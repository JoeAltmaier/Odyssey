/*
 * ScriptDirector.cpp - Error Injection Script Director DDM
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Revision History:
 *     2/6/1999 Bob Butler: Created
 *
*/
#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <string.h>
//
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
	#include <stdarg.h>
	#include <conio.h>
#endif

#include "DdmScriptDirector.h"
#include "BuildSys.h"
#include "Event.h"
#include "ErrorTrigger.h"

#define ENABLE_ERROR_INJECTION

#include "EISTriggers.h"

extern	"C" int ttyA_in();
extern "C" {
#include "ct_fmt.h"
}

CLASSNAME(DdmScriptDirector, SINGLE);	// Class Link Name used by Buildsys.cpp

// temporary (and simplistic) atoi until the library issue is resolved

int atoi(char *);

int atoi (char *s)
{
	int i = 0;
	while(s && *s && isdigit(*s))
	{
		i = i * 10 + (*s - '0');
		++s;
	}
	return i;
}

char *DdmScriptDirector::aCmd[] = {"UPLOAD", "LOAD", "MODE", "PHASE", "NEXT", "RUN", "STOP", "CLEAR", "IOPLIST", "REPORT", "LOG", "TEST", NULL};
bool DdmScriptDirector::isActive = false;
DdmScriptDirector *DdmScriptDirector::pSD = NULL;

void DdmScriptDirector::SetActive(bool isActive_)
{
	if (!pSD)
	{
		printf("DdmScriptDirector::SetActive -- Not enabled yet\n");
		return;
	}
	isActive = isActive_;
	if (isActive)
	{
		pSD->Print("EIS is now active\n\nEIS>");
		pSD->pTimerMsg = new RqOsTimerStart(10000,10000);
		pSD->Send(pSD->pTimerMsg, REPLYCALLBACK(DdmScriptDirector, ProcessTimerReply));
	}
	else
	{
		pSD->Print("EIS is no longer active\n");
		// stop the timer
		RqOsTimerStop *pMsg = new RqOsTimerStop(pSD->pTimerMsg);
		pSD->Send(pMsg, REPLYCALLBACK(DdmScriptDirector, ProcessTimerStopReply));
	}
}

// DdmScriptDirector -- Constructor ---------------------DdmScriptDirector-
//
DdmScriptDirector::DdmScriptDirector(DID did): Ddm(did),
	bUpload(false), cPendingReplies(0), cPendingClears(0), bProcessing(false)
{
	//SetConfigAddress(&config,sizeof(config));	// tell Ddm:: where my config area is
}
	

// Ctor -- Create ourselves ---------------------------------DdmScriptDirector-
//
Ddm *DdmScriptDirector::Ctor(DID did)
{

	return new DdmScriptDirector(did);
}


STATUS DdmScriptDirector::Enable(Message *pMsg)
{ 
	TRACE_ENTRY("DdmScriptDirector::Enable\n");
	lenCmd = 0;
	pTimerMsg = NULL;
		
	Reply(pMsg, OK);
	pSD = this;
	isActive = false;
	
	return OK;
}

/*STATUS DdmScriptDirector::ReplyDefault(Message *pMsg) 
{
	delete pMsg;
	return OK;
}*/

STATUS DdmScriptDirector::ReplyTMDefault(Message *pMsg) 
{
	--cPendingReplies;
	if (!cPendingReplies)
		Print("\nEIS>");

	delete pMsg;
	return OK;
}

STATUS DdmScriptDirector::ReplyReport(Message *pMsg) 
{
	--cPendingReplies;
	TMReport *pPayload = (TMReport *)pMsg->GetPPayload();
	Print("\n *** Slot %d Status Report ***\n", pPayload->iSlot);
	if (pPayload && pPayload->cTriggers)
	{
		Print("Phase: %d\n", pPayload->iPhase);
		Print("    Total Triggers: %d\n", pPayload->cTriggers);		
		Print("    Armed Triggers: %d\n", pPayload->cArmed);		
		Print(" Disarmed Triggers: %d\n", pPayload->cDisarmed);		
		Print("Triggered Triggers: %d\n", pPayload->cTriggered);		
		Print("  Cleared Triggers: %d\n", pPayload->cCleared);		
	}
	Print(" *** End Slot %d Status Report ***\n\n", pPayload->iSlot);

	if (!cPendingReplies)
		Print("\nEIS>");
		
	delete pMsg;
	return OK;
}

STATUS DdmScriptDirector::ReplyTMAllCleared(Message *pMsg) 
{
	--cPendingClears;
	if (!cPendingClears)
		Print("\nPhase %d complete, all Triggers have cleared.\n\nEIS>", phCurr);
	delete pMsg;
	return OK;
}



STATUS DdmScriptDirector::ProcessTimerReply(Message *pMsg) 
{
	/* 	We don't want to be reading input while we are in the middle of 
		processing a command or communicating with the TriggerManager DDMs, 
		so we'll ignore timer messages if either is happening */
	if (!cPendingReplies && !bProcessing)
		GetNextChar();

	delete pMsg;
	return OK;
}

STATUS DdmScriptDirector::ProcessTimerStopReply(Message *pMsg) 
{
	delete pMsg;
	return OK;
}


STATUS DdmScriptDirector::GetNextChar()
{
		
	char ch = ttyA_in();
	if (ch == -1)
		return OK;
	if (ch == '\n' || ch == '\r')
	{
		bProcessing = true;
		sCmd[lenCmd] = 0;
		if (lenCmd)
		{
			Print("\n");
			if (bUpload)
			{
				if (AddScriptLine(sCmd) && pEMsg)
					Print("%s\n", pEMsg);
				delete pEMsg;
			}
			else
				ProcessCommand(sCmd);
			if (!bUpload && !cPendingReplies)
				Print("\nEIS>");
		}
		lenCmd = 0;
		bProcessing = false;
	}
	if (ch == '\b' && lenCmd > 0)
	{
	   	--lenCmd;
	   	Print("\b \b");
	 }
	else if (isprint(ch))
	{
		sCmd[lenCmd++] = ch;
		Print("%c",  ch);
	}
		
	return OK;
}

DdmScriptDirector::eCmd DdmScriptDirector::Command(char *_sCmd)
{
	char *_ucCmd = new char[strlen(_sCmd) + 1];
	char *_p = _ucCmd; 
	*_p = 0;
	while (*_sCmd) *_p++ = toupper(*_sCmd++);
		
	
	for (U16 i = 0; aCmd[i] != NULL; ++i)
		if (strcmp(_ucCmd, aCmd[i]) == 0) 
			return (eCmd)i;
	return _ERROR_;
}

void DdmScriptDirector::ProcessCommand(char *_sCmdLine)
{
	printf("Processing %s\n", _sCmdLine);
	
	char *_sCmd = strtok(_sCmdLine, " ");

	switch (Command(_sCmd))
	{
	case UPLOAD:
		UploadScript();
		break;
	case LOAD_SCRIPT:
		LoadScript(strtok(NULL, " "));
		break;
	case MODE:
		SetMode(strtok(NULL, " "), strtok(NULL, " "), strtok(NULL, " "));
		break;
	case PHASE:
		SetPhase(strtok(NULL, " "));
		break;
	case NEXT:
		SetPhase(phCurr + 1);
		break;
	case RUN:
		RunScript();
		break;
	case STOP:
		StopScript();
	case CLEAR:
		ResetEverything();
		break;
	case IOPLIST:
		if (strlen(_sCmdLine) > strlen(_sCmd))
			SetIOPList (_sCmdLine + strlen(_sCmd) + 1);
		else
			SetIOPList ("ALL");
		break;
	case REPORT:
		Report();
		break;
	case LOG:
		Print("Logging: %s\n", _sCmdLine + strlen(_sCmd) + 1);
		break;
	case TEST:
		{
		char *pClear = strtok(NULL, " ");
		if (pClear && strcmp(pClear, "clear") == 0)
			TriggerClassClear(EIS_TEST);
		else
			Test();
		}
		break;

	default:
		Print("Unrecognized command: %s\n", _sCmd);
	}
}

void DdmScriptDirector::Print(char *sFormat, ...)
{
	

	va_list ap;
	va_start(ap, sFormat);
	
	
   
/*   	char *pBuf,buf[512];
	pBuf = buf;
	_fmt(_movech,&pBuf,&sFormat);
	*pBuf = EOS;
*/
	vprintf(sFormat, ap);	
	va_end(ap);	 /* clean things up before leaving */
//	printf(buf);				

}

/*void DdmScriptDirector::Log(char *sFormat, ...)
{
	char buf[512];
   
	va_list ap;
	va_start(ap, sFormat);
	
	
   
	vsprintf(buf, sFormat, ap);	
	va_end(ap);	 /* clean things up before leaving */
//	printf(buf);				


/*   	char *pBuf,buf[512];
	pBuf = buf;
	_fmt(_movech,&pBuf,&sFormat);
	*pBuf = EOS;

	LogEvent((event_code_t)3, (char *)buf);   
}

*/

BOOL DdmScriptDirector::Report()
{
	// notify all of the Trigger Managers
	for (U32 _nSlot = 0; _nSlot < aIopTriggers.Size(); ++_nSlot)
	{
		if (aIopTriggers[_nSlot].NextIndex())  // does this slot have any triggers?
		{
			Message *pMsg = new Message(PRIVATE_REQ_REPORT_TRIGGER_MGR, sizeof(TMReport));
			STATUS s = Send ((TySlot)_nSlot, pMsg, REPLYCALLBACK(DdmScriptDirector, ReplyReport));
			cPendingReplies++;
		}
	}
//	state = ACTIVE;
	return true;
}

BOOL DdmScriptDirector::ResetEverything()
{
	printf("Everything has been reset.");

	DdmScriptDirector::SetActive(false);

	phStart = phStop = phCurr = -1;
	sScriptName = NULL;
	mode = AUTO;
	state = INACTIVE;
	cLine = 0;
	
	aET.Clear();
	aIopTriggers.Clear();
	delete pEMsg;
	pEMsg = NULL;
	
	
	return true;

}

BOOL DdmScriptDirector::LoadScript (char *sName)
{
	Print("Loading script: %s\n", sName);
	Print("Load cannot be implemented until there is a file system, try 'upload' instead.\n");
	return true;

}

BOOL DdmScriptDirector::UploadScript()
{
	Print("Ready to receive the script.\n");
	Print("The last line should be 'END', or type 'END' on a line by itself when complete.\n");
	Print("Begin now...\n");
	ResetEverything();
	bUpload = true;
	return true;
}

BOOL DdmScriptDirector::AddScriptLine(char *_pLine)
{
	if (*_pLine == EOF || bUpload 
		&& (strlen(_pLine) <= 3 && toupper(_pLine[0] == 'E')))
	{
		bUpload = false;
		return true;  // script is loaded
	}
	if (*_pLine == '\'' || *_pLine == ';' || *_pLine == '#')
		return false;  // comment

	ErrorTrigger *_pET = ErrorTrigger::CreateTrigger(_pLine);
	
	++cLine;

	if (!_pET && _pLine && strlen(_pLine))
	{
		int i = 0;
		if (pEMsg)
			i = strlen(pEMsg);
		char *_pTemp = new char[i + strlen(_pLine) + 30];
		if (pEMsg)
			strcat(_pTemp, pEMsg);
			
		sprintf (_pTemp + i, "Error in line %d: %s\n", cLine + 1, _pLine);
		delete pEMsg;
		pEMsg = _pTemp;
	}
	else 
		aET.Append(*_pET);
	return false;  // more to process
}


STATUS DdmScriptDirector::StopScript()
{
	state = STOPPED;
	// Notify all of the Trigger Managers
	for (U32 _nSlot = 0; _nSlot < aIopTriggers.Size(); ++_nSlot)
	{
		if (aIopTriggers[_nSlot].NextIndex())  // does this slot have any triggers?
		{
			Message *pMsg = new Message(PRIVATE_REQ_DISARM_TRIGGER_MGR);
			STATUS s = Send ((TySlot)_nSlot, pMsg, REPLYCALLBACK(DdmScriptDirector, ReplyTMDefault));
			cPendingReplies++;
		}
	}
	return OK;
}

STATUS DdmScriptDirector::RunScript()
{
	// notify all of the Trigger Managers
	for (U32 _nSlot = 0; _nSlot < aIopTriggers.Size(); ++_nSlot)
	{
		if (aIopTriggers[_nSlot].NextIndex())  // does this slot have any triggers?
		{
			Message *pMsg = new Message(PRIVATE_REQ_ARM_TRIGGER_MGR);
			STATUS s = Send ((TySlot)_nSlot, pMsg, REPLYCALLBACK(DdmScriptDirector, ReplyTMAllCleared));
			cPendingClears++;
		}
	}
	state = ACTIVE;
	return true;
}

BOOL DdmScriptDirector::SetPhase(U32 _phCurr)
{
	BOOL ok = true;
	if (state == ACTIVE)
		ok = StopScript();
	if (ok)
		ok = DistributeScript(_phCurr);
	if (ok)
	{
		mode = STEP;
		phCurr = _phCurr;
		Print("The phase has been set to %d and the EIS is in STEP mode\n", phCurr);
	}
	return ok;
}

BOOL DdmScriptDirector::SetPhase(char *_sPhCurr)
{
	if (_sPhCurr && strlen(_sPhCurr) && isdigit(_sPhCurr[0]))
		return SetPhase(atoi(_sPhCurr));
	return false;
}

BOOL DdmScriptDirector::SetMode(char * _sMode, char *_sPhStart, char *_sPhStop)
{
	printf("Not implemented yet.\n");
	return true;
}

BOOL DdmScriptDirector::SetIOPList(char *_sIOPList)
{
	printf("Not implemented yet.\n");
	return true;
}


BOOL DdmScriptDirector::DistributeScript(U32 _phCurr)
{
	for (U32 _nET = 0; _nET < aET.NextIndex(); ++_nET)
	{
		 // add the trigger to the appropriate slot
		if (aET[_nET].GetPhase() == _phCurr)
				aIopTriggers[aET[_nET].GetSlot()].Append(aET[_nET]);
	}
	// run through the slots with triggers and send a message to the TriggerMgr DDM on that slot
	for (U32 _nSlot = 0; _nSlot < aIopTriggers.Size(); ++_nSlot)
	{
		if (aIopTriggers[_nSlot].NextIndex())  // does this slot have any triggers?
		{
			Message *pMsg = new Message(PRIVATE_REQ_LOAD_TRIGGER_MGR);
			// Send just the constuctor parameters: 5 U32s
			U32 len = aIopTriggers[_nSlot].NextIndex();
			U32 *aPacked = new(tPCI) U32[len * 5]; 
			for (U32 _nTrigger = 0; _nTrigger < len; ++_nTrigger)
				aIopTriggers[_nSlot][_nTrigger].PersistArgs(&(aPacked[_nTrigger * 5]));
			pMsg->AddSgl(0, aPacked, len * 5 * sizeof(U32));
			STATUS s = Send ((TySlot)_nSlot, pMsg, REPLYCALLBACK(DdmScriptDirector, ReplyTMDefault));
			cPendingReplies++;
			
		}
	}
			
	return cPendingReplies > 0;
}

void DdmScriptDirector::Test()
{
	Print("Testing:\n");
	Print("TriggerCheck(EID_EIS_ERROR_1, 0) ");
	if (TriggerCheck(EID_EIS_ERROR_1, 0))
		Print("Triggered\n");
	else
		Print("did not Trigger\n");
	Print("TriggerCheck(EID_EIS_ERROR_1, 1) ");
	if (TriggerCheck(EID_EIS_ERROR_1, 1))
		Print("Triggered\n");
	else
		Print("did not Trigger\n");
	Print("TriggerCheck(EID_EIS_ERROR_2, 2) ");
	if (TriggerCheck(EID_EIS_ERROR_2, 2))
		Print("Triggered\n");
	else
		Print("did not Trigger\n");
	Print("TriggerCheck(EID_EIS_ERROR_3, 0) ");
	if (TriggerCheck(EID_EIS_ERROR_3, 0))
		Print("Triggered\n");
	else
		Print("did not Trigger\n");
}