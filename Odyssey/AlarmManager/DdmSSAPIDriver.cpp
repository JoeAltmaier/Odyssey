/* DdmSSAPIDriver.cpp 
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
**/

// Revision History:
// $Log: /Gemini/Odyssey/AlarmManager/DdmSSAPIDriver.cpp $
// 
// 19    9/29/99 6:47p Joehler
// modified drivers to test alarm coalescing
// 
// 18    9/14/99 9:33a Joehler
// Fixed bug in testing functionality of user remitted alarms
// 
// 17    9/13/99 3:40p Joehler
// Added testing functionality for user remitted alarms
// 
// 16    9/07/99 2:11p Joehler
// Changes to correctly manage memory associated with AlarmContext
// 
// 15    9/02/99 11:46a Joehler
// added comments
// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_ALARM	// set this modules index to your index	
#include "Odyssey_Trace.h"

//DdmSSAPIDriver.cpp

#include "BuildSys.h"
#include "DdmSSAPIDriver.h"
#include "AlarmEvents.h"
#include "AlarmCmdQueue.h"
#include "DdmMaster.h"
#include "CtEvent.h"

CLASSNAME(DdmSSAPIDriver,SINGLE);  //Class link name used by buildsys

// temp
SSAPIAlarmContext *pSSAPIAlarmContext1;

extern void QuitTest();

DdmSSAPIDriver::DdmSSAPIDriver(DID did) : DdmMaster(did) { }

Ddm* DdmSSAPIDriver::Ctor(DID did) 
{
	
	TRACE_ENTRY(DdmSSAPIDriver::Ctor(DID));
	return (new DdmSSAPIDriver (did));
}

STATUS DdmSSAPIDriver::Initialize(Message* pMsg) 
{
	STATUS status = OK;

	TRACE_ENTRY(DdmSSAPIDriver::Initialize(Message*));

	m_pCmdSender = new CmdSender(
		AMSTR_CMD_QUEUE_TABLE,
		sizeof(AMSTR_CMND_INFO),
		sizeof(AMSTR_EVENT_INFO),
		this
		);

	m_pCmdSender->csndrInitialize((pInitializeCallback_t)
		&DdmSSAPIDriver::cmdsenderInitializedReply);

	InitReplyHandler(ssapidriver_WAITING_FOR_INIT, pMsg);

	return status;
}

void DdmSSAPIDriver::cmdsenderInitializedReply(STATUS status) 
{
	if (!status) {
		// we are initialized
		m_pCmdSender->csndrRegisterForEvents((pEventCallback_t)
			&DdmSSAPIDriver::cmdsenderEventHandler);

		InitReplyHandler(ssapidriver_INITIALIZED, NULL);
	}
}

void DdmSSAPIDriver::InitReplyHandler(U32 state, Message *msg) 
{
	
	TRACE_ENTRY(DdmSSAPIDriver::InitReplyHandler());
	static Message* initMsg;

	switch (state) {
	case ssapidriver_WAITING_FOR_INIT:
		TRACE_STRING(5, ("DdmSSAPIDriver::InitReplyHandler - WAITING_FOR_INIT\n")); 
		initMsg = msg;
		break;
	case ssapidriver_INITIALIZED:
		TRACE_STRING(5, ("DdmSSAPIDriver::InitReplyHandler - INITIALIZED\n")); 
		Reply(initMsg, OK);
		break;
	default:
		assert(0);
	}
}

STATUS DdmSSAPIDriver::Enable(Message* pMsg) 
{
	Reply(pMsg,OK);
	MessageControl();
	return OK;
}
	
void DdmSSAPIDriver::cmdsenderEventHandler(STATUS eventCode,
										   void* pStatusData)
{

	TRACE_ENTRY(DdmSSAPIDriver::cmdsenderEventHandler());
	switch (eventCode) 
	{
	case AMSTR_EVT_ALARM_SUBMITTED:
		TRACE_STRING(5, ("DdmSSAPIDriver::cmdsenderEventHandler - ALARM SUBMITTED\n")); 
		break;
	case AMSTR_EVT_ALARM_REMITTED:
		TRACE_STRING(5, ("DdmSSAPIDriver::cmdsenderEventHandler - ALARM REMITTED\n")); 
		break;
	case AMSTR_EVT_ALARM_ACKNOWLEDGED:
		TRACE_STRING(5, ("DdmSSAPIDriver::cmdsenderEventHandler - ALARM ACKNOWLEDGED\n")); 
		break;
	case AMSTR_EVT_ALARM_UNACKNOWLEDGED:
		TRACE_STRING(5, ("DdmSSAPIDriver::cmdsenderEventHandler - ALARM UNACKNOWLEDGED\n")); 
		break;
	case AMSTR_EVT_ALARM_NOTIFIED:
		TRACE_STRING(5, ("DdmSSAPIDriver::cmdsenderEventHandler - ALARM NOTIFIED\n")); 
		break;
	case AMSTR_EVT_ALARM_KILLED:
		TRACE_STRING(5, ("DdmSSAPIDriver::cmdsenderEventHandler - ALARM KILLED\n")); 
		break;
	default:
		assert(0);
		break;
	}
}

STATUS DdmSSAPIDriver::RemitAlarmFromUser(rowID rid, UnicodeString userName)
{
	TRACE_ENTRY(DdmSSAPIDriver::RemitAlarmFromUser(void*));

	AMSTR_CMND_INFO* pCmdInfo;
	AMSTR_CMND_PARAMETERS* pCmdParams;
	
	pCmdInfo = (AMSTR_CMND_INFO *)new char[sizeof(AMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(AMSTR_CMND_INFO));

	pCmdInfo->opcode = AMSTR_CMND_REMIT_ALARM_FROM_USER;
	pCmdParams = (AMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);

	pCmdParams->rid = rid;
	userName.CString(pCmdParams->userName, sizeof(pCmdParams->userName));

	m_pCmdSender->csndrExecute(pCmdInfo, 
					   (pCmdCompletionCallback_t)&DdmSSAPIDriver::
					   amstrCommandCompletionReply,
					   NULL);
	return OK;
}
// acknowledge the alarm if it exists
STATUS DdmSSAPIDriver::AcknowledgeAlarm(void *pAlarmContext_)
{
	TRACE_ENTRY(DdmSSAPIDriver::AcknowledgeAlarm(void*));

	AlarmList *pAL = pALhead->Find(pAlarmContext_);
	STATUS erc = OK;
	if (pAL)
	{
		AMSTR_CMND_INFO* pCmdInfo;
		AMSTR_CMND_PARAMETERS* pCmdParams;
		UnicodeString notes = UnicodeString(StringClass("this is an acknowledgement note"));
		UnicodeString userName = UnicodeString(StringClass("joehler"));
	
		pCmdInfo = (AMSTR_CMND_INFO *)new char[sizeof(AMSTR_CMND_INFO)];
		memset(pCmdInfo,0,sizeof(AMSTR_CMND_INFO));

		pCmdInfo->opcode = AMSTR_CMND_ACKNOWLEDGE_ALARM;
		pCmdParams = (AMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	
		pCmdParams->rid = pAL->GetRowId();
		notes.CString(pCmdParams->notes, sizeof(pCmdParams->notes));
		userName.CString(pCmdParams->userName, sizeof(pCmdParams->userName));

		m_pCmdSender->csndrExecute(pCmdInfo, 
						   (pCmdCompletionCallback_t)&DdmSSAPIDriver::
						   amstrCommandCompletionReply,
						   NULL);
	}
	else
		erc = ERR;
	return erc;
}

// unacknowledge the alarm if it exists
STATUS DdmSSAPIDriver::UnAcknowledgeAlarm(void *pAlarmContext_)
{
	TRACE_ENTRY(DdmSSAPIDriver::UnAcknowledgeAlarm(void*));

	AlarmList *pAL = pALhead->Find(pAlarmContext_);
	STATUS erc = OK;
	if (pAL)
	{
		AMSTR_CMND_INFO* pCmdInfo;
		AMSTR_CMND_PARAMETERS* pCmdParams;
		UnicodeString notes = UnicodeString(StringClass("this is an unacknowledment string"));
		UnicodeString userName = UnicodeString(StringClass("rbraun"));
	
		pCmdInfo = (AMSTR_CMND_INFO *)new char[sizeof(AMSTR_CMND_INFO)];
		memset(pCmdInfo,0,sizeof(AMSTR_CMND_INFO));

		pCmdInfo->opcode = AMSTR_CMND_UNACKNOWLEDGE_ALARM;
		pCmdParams = (AMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	
		pCmdParams->rid = pAL->GetRowId();
		notes.CString(pCmdParams->notes, sizeof(pCmdParams->notes));
		userName.CString(pCmdParams->userName, sizeof(pCmdParams->userName));

		m_pCmdSender->csndrExecute(pCmdInfo, 
						   (pCmdCompletionCallback_t)&DdmSSAPIDriver::
						   amstrCommandCompletionReply,
						   NULL);
	}
	else
		erc = ERR;
	return erc;
}

// notify the alarm if it exists
STATUS DdmSSAPIDriver::NotifyAlarm(void *pAlarmContext_)
{
	TRACE_ENTRY(DdmSSAPIDriver::NotifyAlarm(void*));

	AlarmList *pAL = pALhead->Find(pAlarmContext_);
	STATUS erc = OK;
	if (pAL)
	{
		AMSTR_CMND_INFO* pCmdInfo;
		AMSTR_CMND_PARAMETERS* pCmdParams;
		UnicodeString notes = UnicodeString(StringClass("this is an notify string"));
		UnicodeString userName = UnicodeString(StringClass("toehler"));
	
		pCmdInfo = (AMSTR_CMND_INFO *)new char[sizeof(AMSTR_CMND_INFO)];
		memset(pCmdInfo,0,sizeof(AMSTR_CMND_INFO));

		pCmdInfo->opcode = AMSTR_CMND_NOTIFY_ALARM;
		pCmdParams = (AMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	
		pCmdParams->rid = pAL->GetRowId();
		notes.CString(pCmdParams->notes, sizeof(pCmdParams->notes));
		userName.CString(pCmdParams->userName, sizeof(pCmdParams->userName));

		m_pCmdSender->csndrExecute(pCmdInfo, 
						   (pCmdCompletionCallback_t)&DdmSSAPIDriver::
						   amstrCommandCompletionReply,
						   NULL);
	}
	else
		erc = ERR;
	return erc;
}
void DdmSSAPIDriver
::amstrCommandCompletionReply(
			STATUS				completionCode,
			void				*pStatusData,
			void				*pCmdData,
			void				*pCmdContext)
{
	AMSTR_CMND_INFO *pCmdInfo = (AMSTR_CMND_INFO*)pCmdData;
	AMSTR_CMND_PARAMETERS *pCmdParams = &pCmdInfo->cmdParams;
	STATUS status = OK;

	switch (pCmdInfo->opcode) {
	case AMSTR_CMND_ACKNOWLEDGE_ALARM:
		// tbd
		break;
	case AMSTR_CMND_UNACKNOWLEDGE_ALARM:
		// tbd
		break;
	case AMSTR_CMND_NOTIFY_ALARM:
		// tbd
		break;
	case AMSTR_CMND_KILL_ALARM:
		// tbd
		break;
	case AMSTR_CMND_REMIT_ALARM_FROM_USER:
		// tbd
		break;
	default:
		assert(0);
		break;
	}

	MessageControl();

}

void DdmSSAPIDriver::SubmitFirstAlarm()
{
	Event* p_SSAPIEvt = new Event(CTS_SSAPI_ALARM_TOO_MANY_WRONG_LOGINS, GetVdn(), GetDid());
	p_SSAPIEvt->AddEventParameter("ssapi");
	pSSAPIAlarmContext1 = new SSAPIAlarmContext;
	pSSAPIAlarmContext1->dummy = 22;
	SubmitAlarm(p_SSAPIEvt, sizeof(SSAPIAlarmContext), pSSAPIAlarmContext1);
}

void DdmSSAPIDriver::AcknowledgeFirstAlarm()
{
	AcknowledgeAlarm(pSSAPIAlarmContext1);
}

void DdmSSAPIDriver::UnAcknowledgeFirstAlarm()
{
	UnAcknowledgeAlarm(pSSAPIAlarmContext1);
}

void DdmSSAPIDriver::NotifyFirstAlarm()
{
	NotifyAlarm(pSSAPIAlarmContext1);
}

void DdmSSAPIDriver::QueryAllAlarms()
{
	QueryAlarms(ALL_ALARMS);
}

void DdmSSAPIDriver::QueryActiveAlarms()
{
	QueryAlarms(ACTIVE_ALARMS);
}

void DdmSSAPIDriver::QueryInactiveAlarms()
{
	QueryAlarms(INACTIVE_ALARMS);
}

void DdmSSAPIDriver::SubmitElevenAlarms()
{
	Event* p_SSAPIEvt;
	SSAPIAlarmContext* pSSAPIAlarmContext;
	U16 i;
	for (i =0; i <=10; i++) 
	{
		p_SSAPIEvt = new Event(CTS_SSAPI_ALARM_TOO_MANY_WRONG_LOGINS, GetVdn(), GetDid());
		p_SSAPIEvt->AddEventParameter(i);
		pSSAPIAlarmContext = new SSAPIAlarmContext;
		pSSAPIAlarmContext->dummy = i;
		SubmitAlarm(p_SSAPIEvt, sizeof(SSAPIAlarmContext), pSSAPIAlarmContext);
	}
}

void DdmSSAPIDriver::SubmitSecondAlarm() 
{
	TRACE_ENTRY(DdmSSAPIDriver::SubmitSecondAlarm());

	Event* p_SSAPIEvt = new Event(CTS_SSAPI_ALARM_TOO_MANY_WRONG_LOGINS, GetVdn(), GetDid());
	p_SSAPIEvt->AddEventParameter("ssapi");
	p_SSAPIEvt->AddEventParameter(32);
	p_SSAPIEvt->AddEventParameter("and another string parameter");
	SSAPIAlarmContext* pSSAPIAlarmContext = new SSAPIAlarmContext;
	pSSAPIAlarmContext->dummy = 11;
	SubmitAlarm(p_SSAPIEvt, sizeof(SSAPIAlarmContext), pSSAPIAlarmContext);
}

void DdmSSAPIDriver::RemitFirstAlarm() 
{
	TRACE_ENTRY(DdmSSAPIDriver::RemitFirstAlarm());

	RemitAlarm(pSSAPIAlarmContext1);
}

void DdmSSAPIDriver::RecoverAllAlarms() 
{
	TRACE_ENTRY(DdmSSAPIDriver::RecoverAllAlarms());

	RecoverAlarms();
}

void DdmSSAPIDriver::DeletePAl()
{
	AlarmList* current;
	while (current = pALhead->GetNext())
	{
		delete pALhead->GetAlarmContext();
		delete pALhead;
		pALhead = current;
	}
	delete pALhead->GetAlarmContext();
	delete pALhead;
	pALhead = NULL;
}

void DdmSSAPIDriver::RemitAllAlarms() 
{
	TRACE_ENTRY(DdmSSAPIDriver::RemitAllAlarms());

	RemitAll();
}

void DdmSSAPIDriver::MessageControl()
{
	TRACE_ENTRY(DdmSSAPIDriver::MessageControl());

	static i = 0;

	switch(i)
	{
		case 0:
			SubmitFirstAlarm();
			SubmitSecondAlarm();
			break;
		case 2:
			RemitFirstAlarm();
			break;
		case 3:
			SubmitFirstAlarm();
			RemitFirstAlarm();
			break;
		case 5:
			SubmitFirstAlarm();
			break;
		case 6:
			AcknowledgeFirstAlarm();
			UnAcknowledgeFirstAlarm();
			NotifyFirstAlarm();
			break;
		case 9:
			QueryAllAlarms();
			break;
		case 10:
			QueryActiveAlarms();
			break;
		case 13:
			QueryInactiveAlarms();
			break;
		case 14:
			DeletePAl();
			RecoverAllAlarms();
			break;
		case 15:
			RemitAllAlarms();
			break;
		case 17:
			SubmitElevenAlarms();
			break;
		case 28:
			DeletePAl();
			RecoverAllAlarms();
			break;
		case 29:
			RemitAllAlarms();
			break;
		case 40:
			Tracef("\nDdmSSAPIDriver - DONE!!!\n");
			QuitTest();
	}
	i++;
}

void DdmSSAPIDriver::cbSubmitAlarm(void * pAlarmContext_, STATUS status_  ) 
{
	TRACE_ENTRY(DdmSSAPIDriver::cbSubmitAlarm(STATUS, void*));
	MessageControl();
}

void DdmSSAPIDriver::cbRemitAlarm( void * pAlarmContext_, STATUS status_ ) 
{
	TRACE_ENTRY(DdmSSAPIDriver::cbRemitAlarm(STATUS, void*));
	delete pAlarmContext_;
	MessageControl();
}

void DdmSSAPIDriver::cbRecoveryComplete( STATUS status_ ) 
{
	TRACE_ENTRY(DdmSSAPIDriver::cbRecoveryComplete(STATUS, void*));
	MessageControl();
}

void DdmSSAPIDriver::cbQueryAlarms(
 		U16 numberOfAlarms,
		AlarmRecord* pAlarmRecords,
		U16 numberOfAlarmLogEntries,
		AlarmLogRecord* pAlarmLogEntries,
		STATUS status_)
{
	U32 i;
	static U16 j = 0;
	UnicodeString us(StringClass("Jaymie"));
	TRACE_ENTRY(DdmSSAPIDriver::cbQueryAlarms(STATUS, void*));
	
	if (j==1)
	{
		for (i = 0; i < numberOfAlarms; i++)
		{
			if ((pAlarmRecords[i].did != this->GetDid()) &&
				(pAlarmRecords[i].clearable==TRUE))
			{
				RemitAlarmFromUser(pAlarmRecords[i].rid, us);
				break;
			}
		}
		for (i = 0; i < numberOfAlarms; i++)
		{
			if ((pAlarmRecords[i].did != this->GetDid()) &&
				(pAlarmRecords[i].clearable==FALSE))
			{
				RemitAlarmFromUser(pAlarmRecords[i].rid, us);
				break;
			}
		}
	}

	MessageControl();
	j++;
}

