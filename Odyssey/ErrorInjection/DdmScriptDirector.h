/*
 * ScriptDirector.h - Error Injection Script Director DDM
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

#ifndef __DdmScriptDirector_H
#define __DdmScriptDirector_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "Array_t.h"
#include "ErrorTrigger.h"
#include "RqOsTimer.h"

/*
 * The Script Director DDM resides in the HBC and is responsible for
 * providing the user interface to the Error Injection Service.
 *
*/

	

class DdmScriptDirector : public Ddm
{
public:
	DdmScriptDirector(DID did);
	static Ddm *Ctor(DID did);
	STATUS Enable(Message *pMsg);
	void ProcessCommand(char *sCmdLine);
	static bool isActive;
	static void SetActive(bool isActive);
	
private:
	STATUS GetNextChar();
	STATUS ProcessTimerReply(Message *pMsg);
	STATUS ProcessTimerStopReply(Message *pMsg);
	STATUS ReplyTMDefault(Message *pMsg);
	STATUS ReplyReport(Message *pMsg);
	STATUS ReplyTMAllCleared(Message *pMsg);
	static enum eMode {AUTO, STEP};
	static enum eState { INACTIVE, ACTIVE, STOPPED };
	// LOAD_SCRIPT is for the LOAD command -- Metrowerks complains if you use LOAD
	static enum eCmd { UPLOAD, LOAD_SCRIPT, MODE, PHASE, NEXT, RUN, STOP, CLEAR, IOPLIST, REPORT, LOG, TEST, _ERROR_};
	static char *aCmd[];  // array of commands, needs to match eCmd above
	
	static DdmScriptDirector *pSD;
	char sCmd[255];  // the command currently being typed
	
	
	int lenCmd;

	char * pEMsg;
	
	Array_T< ErrorTrigger > aET;
	Array_T< Array_T<ErrorTrigger> > aIopTriggers;
	RqOsTimerStart *pTimerMsg;
	
	char *sScriptName;
	U32 cLine;
	U32 cPendingReplies;
	U32 cPendingClears;
	
	U32 phStart;
	U32 phStop;
	U32 phCurr;
	BOOL bUpload;
	BOOL bProcessing;
	
	eMode mode;
	eState state;


	eCmd Command(char *sCmd);
	char *Read();
	void Print(char *, ...);
//	void Log(char *, ...);
	BOOL LoadScript (char *sName);
	BOOL UploadScript ();	
	BOOL AddScriptLine(char *line);
	BOOL SetMode(char *_sMode, char *_sPhStart, char *_sPhStop);
	BOOL SetPhase(char *_sPhCurr);
	BOOL SetPhase(U32 _phCurr);
	BOOL SetIOPList(char *_sIOPList);
	BOOL RunScript();
	BOOL StopScript();
	BOOL ResetEverything();
	BOOL Report();
	BOOL DistributeScript(U32 _phCurr);
	void Test();

};

#endif