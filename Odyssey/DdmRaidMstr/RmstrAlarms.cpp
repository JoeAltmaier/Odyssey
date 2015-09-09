/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: RmstrAlarms.cpp
// 
// Description:
//	implementation of various alarm routines
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrAlarms.cpp $
// 
// 2     9/08/99 11:01a Dpatel
// 
// 1     9/03/99 9:51a Dpatel
// Initial creation..
// 
// 
//
/*************************************************************************/


#include "DdmRaidMgmt.h"

enum {
	ALARM_ARRAY_NAME_READ = 1,
};

//************************************************************************
//	RmstrSubmitAlarm
//
//************************************************************************
void DdmRAIDMstr::
RmstrSubmitAlarm(
		U32						eventCode,
		U32						type,
		rowID					*pRowId)
{
	ALARM_CONTEXT				*pOutstandingAlarmContext = NULL;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	rowID						SRCTRID;
	rowID						*pAlarmSourceRowId = NULL;

	assert(eventCode);
	assert(pRowId);

	// Check if the same alarm on same array already exists or not
	RmstrGetOutstandingAlarm(
					eventCode,
					pRowId,
					&pOutstandingAlarmContext);
	if (pOutstandingAlarmContext){
		// alarm already there
		return;
	}

	switch(type){
	case RAID_ARRAY:
		GetRmstrData(
			type,
			pRowId,
			(void **)&pADTRecord);
		if (pADTRecord == NULL){
			return;
		} else {
			SRCTRID = pADTRecord->SRCTRID;
			pAlarmSourceRowId = new(tZERO) rowID;
			*pAlarmSourceRowId = pADTRecord->thisRID;
		}
		break;
	default:
		// we handle only raid array related alarms..
		assert(0);
	}


	CONTEXT						*pContext = NULL;
	pContext = new CONTEXT;
	pContext->state = ALARM_ARRAY_NAME_READ;
	pContext->newRowId = SRCTRID;
	pContext->pData = (void *)pAlarmSourceRowId;
	pContext->value = eventCode;

	m_pHelperServices->ReadStorageElementName(
					&pContext->ucArrayName,
					NULL,
					&pContext->newRowId,
					TSCALLBACK(DdmRAIDMstr,ProcessRmstrSubmitAlarmReply),
					pContext);
}


//************************************************************************
//	ProcessRmstrSubmitAlarmReply
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessRmstrSubmitAlarmReply(
		void					*_pContext, 
		STATUS					status)
{
	CONTEXT						*pContext = (CONTEXT *)_pContext;
	StringClass					scArrayName;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	BOOL						cmdComplete = false;
	U32							eventCode = 0;
	Event						*pEvt = NULL;
	ALARM_CONTEXT				*pAlarmContext = NULL;
	rowID						*pAlarmSourceRowId = NULL;

	eventCode = pContext->value;
	pAlarmSourceRowId = (rowID *)pContext->pData;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		cmdComplete = true;
	} else {
		switch(pContext->state){
		case ALARM_ARRAY_NAME_READ:
			// save the alarm context data, so that you can use it
			// to remit the alarm later
			pContext->ucArrayName.GetAsciiString(scArrayName);			
			pEvt = new Event(eventCode, GetDid(), GetVdn());
			pEvt->AddEventParameter(scArrayName.CString());

			pAlarmContext = new(tZERO) ALARM_CONTEXT;
			pAlarmContext->alarmSourceRowId = *pAlarmSourceRowId;
			pAlarmContext->eventCode = eventCode;

			AddRmstrData(
				RAID_ALARM,
				&pAlarmContext->alarmSourceRowId,
				pAlarmContext);
			delete pAlarmContext;
			pAlarmContext = NULL;

			RmstrGetOutstandingAlarm(
						eventCode,
						pAlarmSourceRowId,
						&pAlarmContext);
			SubmitAlarm(
					pEvt,
					sizeof(ALARM_CONTEXT), 
					pAlarmContext);
			delete pEvt;
			cmdComplete = true;
			break;
		default:
			assert(0);
		}	
	}
	if (cmdComplete){
		delete pContext;
	}
	return status;
}


//************************************************************************
//	RmstrRemitAlarm
//
//************************************************************************
void DdmRAIDMstr::
RmstrRemitAlarm(
		U32						eventCode,
		rowID					*pRowId)
{
	ALARM_CONTEXT		*pAlarmContext = NULL;

	RmstrGetOutstandingAlarm(
			eventCode,
			pRowId,
			&pAlarmContext);
	if (pAlarmContext){
		RemitAlarm(pAlarmContext);
		RemoveRmstrData(
			RAID_ALARM,
			&pAlarmContext->alarmSourceRowId);
	}
}


//************************************************************************
//	RmstrGetOutstandingAlarm
//
//************************************************************************
void DdmRAIDMstr::
RmstrGetOutstandingAlarm(
		U32						eventCode,
		rowID					*pRowId,
		ALARM_CONTEXT			**_ppAlarmContext)
{
	ALARM_CONTEXT		*pAlarmContext = NULL;

	TraverseRmstrData(
				RAID_ALARM,
				NULL,
				(void **)&pAlarmContext);
	while (pAlarmContext){
		if ( pAlarmContext->alarmSourceRowId == *pRowId){
			if (pAlarmContext->eventCode == eventCode){
				*_ppAlarmContext = pAlarmContext;
				break;
			}
		}
		TraverseRmstrData(
					RAID_ALARM,
					&pAlarmContext->alarmSourceRowId,
					(void **)&pAlarmContext);
	}
}


//************************************************************************
//	cbRecoverAlarm
//		RecoverAlarms called as part of initialization.
//		Checks for each outstanding alarm, if the condition which raised
//		the alarm in the first place still exists or not.
//		If alarm condition does not exist, then the alarm is remitted.
//
//************************************************************************
void DdmRAIDMstr::
cbRecoverAlarm( void *_pAlarmContext, STATUS status)
{
	// We got back a recovered alarm
	ALARM_CONTEXT			*pAlarmContext = (ALARM_CONTEXT *)_pAlarmContext;
	BOOL					isArrayCritical = false;
	RAID_ARRAY_DESCRIPTOR	*pADTRecord = NULL;

	switch(pAlarmContext->eventCode){
	case CTS_RMSTR_ARRAY_CRITICAL:
		GetRmstrData(
			RAID_ARRAY,
			&pAlarmContext->alarmSourceRowId,
			(void **)&pADTRecord);
		if (pADTRecord){
			switch(pADTRecord->health){
				case RAID_CRITICAL:
				case RAID_OFFLINE:
					// alarm still required, so add it to our list
					AddRmstrData(
						RAID_ALARM,
						&pAlarmContext->alarmSourceRowId,
						pAlarmContext);
					break;
				case RAID_FAULT_TOLERANT:
					RemitAlarm(pAlarmContext);
					RemoveRmstrData(
						RAID_ALARM,
						&pAlarmContext->alarmSourceRowId);
					break;
			}
		} else {
			RemitAlarm(pAlarmContext);
			RemoveRmstrData(
				RAID_ALARM,
				&pAlarmContext->alarmSourceRowId);
		}
		break;

	case CTS_RMSTR_ARRAY_OFFLINE:
		GetRmstrData(
			RAID_ARRAY,
			&pAlarmContext->alarmSourceRowId,
			(void **)&pADTRecord);
		if (pADTRecord){
			switch(pADTRecord->health){
				case RAID_CRITICAL:
				case RAID_OFFLINE:
					break;
			}
		} else {
			// array is probably deleted so remit alarm
			RemitAlarm(pAlarmContext);
			RemoveRmstrData(
				RAID_ALARM,
				&pAlarmContext->alarmSourceRowId);
		}
		break;
	default:
		assert(0);
	}
}
