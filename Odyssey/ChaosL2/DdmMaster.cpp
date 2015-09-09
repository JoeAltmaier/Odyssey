/*************************************************************************
*
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* File: DdmMaster.cpp
*
* Description:
*	This file contains the source for the DdmMaster class 
* 
* Update Log: 
* 07/02/99 Bob Butler: Create file
*
* 07/30/99 Jaymie Oehler: 
*				Added State RECOVERED to AlarmList
*               Added Tracing capability
*				Modified while loop in RemitAll to advance to the pAL
*				Added switch statement to RemitAlarm to account for
*				  more possible states.
*				Replace call to Find in ProcessRemitReply() with call
*				  to FindAndUnlink.
*				Set state in ProcessRecoverReply() to REMITTED, reset
*				  the pALhead, and set the rowID in the new AlarmList
* 08/11/99 Jaymie Oehler:
*               Changed calls to MsgSubmitAlarm and MsgRecoverAlarms
*               to reflect changes made in AlarmMasterMessages.h
* 08/16/99 Jaymie Oehler:
* 				  Moved definition of AlarmList to DdmMaster.h
* 08/30/99 Jaymie Oehler:
*				Added MsgQueryAlarms() for SSAPI
* 09/01/99 Jaymie Oehler:
*				MsgRecoverAlarm contains a UnicodeString16, not a
*				UnicodeString class
*				
*************************************************************************/

// $Log: /Gemini/Odyssey/ChaosL2/DdmMaster.cpp $
// 
// 15    9/07/99 2:15p Joehler
// Changed userName from UnicodeString16 to UnicodeString32
// 
// 14    9/02/99 11:46a Joehler
// added comments


#include "DdmMaster.h"
#include "AlarmMasterMessages.h"



STATUS 	DdmMaster::SubmitAlarm(const Event *pEvt_, U16 cbContext_, void *pAlarmContext_,
							   BOOL userRemittable)
{
	TRACE_ENTRY(DdmMaster::SubmitAlarm(Event*, U16, void*));

	AlarmList *pAL = new AlarmList(cbContext_, pAlarmContext_, pALhead);
	pALhead = pAL;

	assert(pAL->GetState() == AlarmList::NONE);
	pAL->SetState(AlarmList::SUBMITTING);
	MsgSubmitAlarm *pMsg = new MsgSubmitAlarm(GetVdn(), GetDid(),
		cbContext_, pAlarmContext_, pEvt_, userRemittable);
	return Send(pMsg, pAlarmContext_, REPLYCALLBACK(DdmMaster, ProcessSubmitReply));
}

STATUS  DdmMaster::ProcessSubmitReply(Message *pMsg_)
{
	TRACE_ENTRY(DdmMaster::ProcessSubmitReply(Message*));

	MsgSubmitAlarm *pMsg = (MsgSubmitAlarm *)pMsg_;
	void *pAlarmContext = pMsg->GetContext(); // get the original pointer to the AlarmContext
	assert (pALhead);
	AlarmList *pAL = pALhead->Find(pAlarmContext);
	if (pAL)
	{
		pAL->SetRowId(pMsg->GetRowId());
		if (pAL->GetState() == AlarmList::REMIT_PENDING)
			RemitAlarm(pAlarmContext, pAL->GetUserName());
		else
			pAL->SetState(AlarmList::SUBMITTED);
	}
	
	cbSubmitAlarm(pAlarmContext, pMsg->DetailedStatusCode);
	delete pMsg;
	return OK;
}

// callback function for SubmitAlarm()
void DdmMaster::cbSubmitAlarm(void * /* pAlarmContext_ */, STATUS /* status_ */)
{ 
	// default is to do nothing
} 


STATUS DdmMaster::RemitAll()
{
	TRACE_ENTRY(DdmMaster::RemitAll());

	AlarmList *pAL = pALhead;
	while (pAL) {
		RemitAlarm(pAL->GetAlarmContext());
		pAL = pAL->GetNext();
	}
	return OK;
}

// remit the alarm if it exists
STATUS DdmMaster::RemitAlarm(void *pAlarmContext_, UnicodeString userName)
{
	TRACE_ENTRY(DdmMaster::RemitAlarm(void*));
	UnicodeString32 user;

	AlarmList *pAL = pALhead->Find(pAlarmContext_);
	STATUS erc = OK;
	if (pAL)
	{
		switch (pAL->GetState()) 
		{
		case (AlarmList::SUBMITTING):
			TRACE_STRING(6, "DdmMaster::RemitAlarm - SUBMITTING\n");
			pAL->SetState(AlarmList::REMIT_PENDING); // mark for remission
			pAL->SetUserName(userName);
			break;
		case (AlarmList::SUBMITTED):
		case (AlarmList::RECOVERED):
		case (AlarmList::REMIT_PENDING):
			{
			TRACE_STRING(6, "DdmMaster::RemitAlarm - SUBMITTED or RECOVERED or REMIT_PENDING\n");
			pAL->SetState(AlarmList::REMITTING);
			userName.CString(user, sizeof(user));
			MsgRemitAlarm* pMsg = new MsgRemitAlarm(pAL->GetRowId(), GetVdn(), 
				GetDid(), user);
			erc =  Send(pMsg, pAlarmContext_, REPLYCALLBACK(DdmMaster, ProcessRemitReply));
			break;
			}
		case (AlarmList::REMITTING):
			TRACE_STRING(6, "DdmMaster::RemitAlarm - REMITTING\n");
			break;
		default:
			assert(0);
		}
	}
	else
		erc = ERR;
	return erc;
}

STATUS  DdmMaster::ProcessRemitReply(Message *pMsg_)
{
	TRACE_ENTRY(DdmMaster::ProcessRemitReply(Message*));

	MsgRemitAlarm *pMsg = (MsgRemitAlarm *)pMsg_;
	void *pAlarmContext = pMsg->GetContext(); // get the original pointer to the AlarmContext

	assert (pALhead);
	AlarmList *pAL = pALhead->FindAndUnlink(pAlarmContext);
	if (pAL == pALhead)
		pALhead = pAL->GetNext();
	delete pAL;
	
	cbRemitAlarm(pAlarmContext, pMsg->DetailedStatusCode);

	delete pMsg;
	return OK;
}

// callback function for RemitAlarm()
void DdmMaster::cbRemitAlarm( void *pAlarmContext_, STATUS /*status_*/)
{ 
	TRACE_ENTRY(DdmMaster::cbRemitAlarm(void*, STATUS));
	// delete the alarm context for the alarm that has been remitted.
	delete pAlarmContext_;
}

// send a message to the alarm master to recover any open alarms for
// this VDN
STATUS 	DdmMaster::RecoverAlarms()
{
	TRACE_ENTRY(DdmMaster::RecoverAlarms());

	MsgRecoverAlarms *pMsg = new MsgRecoverAlarms(GetVdn(), GetDid());
	return Send(pMsg, REPLYCALLBACK(DdmMaster, ProcessRecoverReply));
}

// for each reply, pick out the info and build an AlarmList.
// Then, call cbRecoverAlarm with the void * alarm context so derived 
// classes can deal with the recovered alarm.
STATUS  DdmMaster::ProcessRecoverReply(Message *pMsg_)
{
	TRACE_ENTRY(DdmMaster::ProcessRecoverReply(Message*));

	AlarmList* pAL;
	void* pAlarmContext;
	MsgRecoverAlarms *pMsg = (MsgRecoverAlarms *)pMsg_;
	if (pMsg->DetailedStatusCode == OK)
	{
		U16 cbAlarmContext = pMsg->GetAlarmContextSize();
		if (cbAlarmContext != 0)
		{
			pMsg->GetAlarmContext(&pAlarmContext);
			pAL = new AlarmList(cbAlarmContext, pAlarmContext, pALhead);
			pALhead = pAL;

			assert(pAL->GetState() == AlarmList::NONE);
			pAL->SetState(AlarmList::RECOVERED);
			pAL->SetRowId(pMsg->GetRowId());

			cbRecoverAlarm(pAlarmContext, pMsg->DetailedStatusCode);
		}
	}
	if (pMsg->IsLast())
		cbRecoveryComplete(pMsg->DetailedStatusCode);
	delete pMsg;
	return OK;
}

// callback for each recovered alarm
void DdmMaster::cbRecoverAlarm( void * /* pAlarmContext */, STATUS /* status_ */)
{
	// Do nothing
}

// callback when all alarms are recovered
void DdmMaster::cbRecoveryComplete(STATUS /* status_ */)
{
	TRACE_ENTRY(DdmMaster::cbRecoveryComplete(STATUS));
	// default behavior is to remit all open alarms
	RemitAll();
}

// send a message to the alarm master to query for all alarms
// action can be ALL_ALARMS, ACTIVE_ALARMS or INACTIVE_ALARMS
STATUS 	DdmMaster::QueryAlarms(U16 action)
{
	TRACE_ENTRY(DdmMaster::QueryAlarms());

	MsgQueryAlarms *pMsg = new MsgQueryAlarms(action);
	return Send(pMsg, REPLYCALLBACK(DdmMaster, ProcessQueryReply));
}

STATUS  DdmMaster::ProcessQueryReply(Message *pMsg_)
{
	AlarmRecord* pAlarmRecords;
	AlarmLogRecord* pAlarmLogEntries;
	U16 numberOfAlarms, numberOfAlarmLogEntries;
	TRACE_ENTRY(DdmMaster::ProcessQueryReply(Message*));

	MsgQueryAlarms *pMsg = (MsgQueryAlarms *)pMsg_;
	if (pMsg->DetailedStatusCode == OK)
	{
		pMsg->GetAlarms((void**)&pAlarmRecords);
		numberOfAlarms = pMsg->GetNumberOfAlarms();
		pMsg->GetAlarmHistory((void**)&pAlarmLogEntries);
		numberOfAlarmLogEntries = pMsg->GetNumberOfAlarmLogEntries();
		cbQueryAlarms(numberOfAlarms,
			pAlarmRecords,
			numberOfAlarmLogEntries,
			pAlarmLogEntries,
			pMsg->DetailedStatusCode);
	}
	delete pMsg;
	return OK;
}

// callback for queried alarms
void DdmMaster::cbQueryAlarms(
 		U16 numberOfAlarms,
		AlarmRecord* pAlarmRecords,
		U16 numberOfAlarmLogEntries,
		AlarmLogRecord* pAlarmLogEntries,
		STATUS status_)
{
	// Do nothing
}

