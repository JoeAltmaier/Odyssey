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
// File: RmstrProcessArrayOfflineEvent.cpp
// 
// Description:
// Implementation for Processing array offline event
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrProcessArrayOfflineEvent.cpp $
// 
// 3     9/07/99 1:47p Dpatel
// Checked array and utility data during init to check PTS data
// consistency.
// 
// 2     9/03/99 10:01a Dpatel
// Remitting alarms...
// 
// 1     8/12/99 4:21p Dpatel
// Initial creation..
// 
// 
// 
//
/*************************************************************************/

#include "DdmRaidMgmt.h"

enum {
	ARRAY_OFFLINE_MDT_RECORD_READ = 1,
	ARRAY_OFFLINE_ADT_RECORD_UPDATED
};

//************************************************************************
//	ProcessArrayOfflineEvent
//		This is an internal command. It is started whenever we rcv a 
//		array offline event from the RAID DDM.
//
//	h			- handle for the cmd
//	pCmdInfo	- cmd packet for array offline event
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessArrayOfflineEvent(HANDLE h, RMSTR_CMND_INFO *_pCmdInfo)
{
	CONTEXT							*pCmdContext = NULL;
	STATUS							status = RMSTR_SUCCESS;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;

	RAID_ARRAY_DESCRIPTOR			*_pADTRecord = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER				*_pMDTRecord = NULL;
	RAID_ARRAY_MEMBER				*pMDTRecord = NULL;
	RMSTR_PROCESS_ARRAY_OFFLINE_INFO	*pProcessArrayOfflineInfo = NULL;


	pCmdContext = new CONTEXT;
	pCmdContext->cmdHandle	= h;

	// save info in context
	pCmdContext->pData = new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));
	pCmdInfo = (RMSTR_CMND_INFO *)pCmdContext->pData;

	pCmdParams		= &pCmdInfo->cmdParams;
	pProcessArrayOfflineInfo = 
			(RMSTR_PROCESS_ARRAY_OFFLINE_INFO *)&pCmdParams->processArrayOfflineInfo;

	// Read the ADT
	GetRmstrData(
		RAID_ARRAY,
		&pProcessArrayOfflineInfo->arrayRowId,
		(void **)&_pADTRecord);
	// if array is already marked offline, we dont need to do any
	// processing
	if (_pADTRecord->health == RAID_OFFLINE){
		ProcessArrayOfflineEventReply(pCmdContext, 1);
	} else {
		pCmdContext->pData1 = new RAID_ARRAY_DESCRIPTOR;
		memcpy(pCmdContext->pData1, _pADTRecord, sizeof(RAID_ARRAY_DESCRIPTOR));
		pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;

		pCmdContext->state = ARRAY_OFFLINE_MDT_RECORD_READ;
		// Allocate space for read row data
		pCmdContext->pData2 = new RAID_ARRAY_MEMBER;
		memset(pCmdContext->pData2,0,sizeof(RAID_ARRAY_MEMBER));
		// Read the MDT Record, since RAID DDM will have updated
		// the PTS with member health
		status = m_pTableServices->TableServiceReadRow(
						RAID_MEMBER_DESCRIPTOR_TABLE,
						&pProcessArrayOfflineInfo->memberRowId,
						pCmdContext->pData2,
						sizeof(RAID_ARRAY_MEMBER),
						(pTSCallback_t)&DdmRAIDMstr::ProcessArrayOfflineEventReply,
						pCmdContext);
	}
	return status;
}
	


//************************************************************************
//	ProcessArrayOfflineEventReply
//		- Read the down member record and update our local copy
//		- Change the array state in the ADT to OFFLINE.
//		- Generate event for array offline
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessArrayOfflineEventReply(void *_pContext, STATUS status)
{
	CONTEXT							*pArrayOfflineContext = NULL;
	RMSTR_CMND_INFO					*pCmdInfo;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER				*pMember = NULL;
	RMSTR_EVT_ARRAY_OFFLINE_STATUS	*pEvtArrayOfflineStatus = NULL;
	BOOL							eventComplete = false;
	STATUS							rc = RMSTR_SUCCESS;

	pArrayOfflineContext = (CONTEXT *)_pContext;
	pCmdInfo = (RMSTR_CMND_INFO *)pArrayOfflineContext->pData;
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *) pArrayOfflineContext->pData1;
	pMember = (RAID_ARRAY_MEMBER *)pArrayOfflineContext->pData2;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		eventComplete = true;
	} else {
		switch(pArrayOfflineContext->state){
		case ARRAY_OFFLINE_MDT_RECORD_READ:
			ModifyRmstrData(
				RAID_MEMBER,
				&pMember->thisRID,
				pMember);

			// Modify Array State
			pArrayOfflineContext->state = ARRAY_OFFLINE_ADT_RECORD_UPDATED;
			pADTRecord->health = RAID_OFFLINE;
			m_pTableServices->TableServiceModifyRow(
						RAID_ARRAY_DESCRIPTOR_TABLE,
						&pADTRecord->thisRID,	// row id to modify
						pADTRecord,
						sizeof(RAID_ARRAY_DESCRIPTOR),
						&pADTRecord->thisRID,
						(pTSCallback_t)&DdmRAIDMstr::ProcessArrayOfflineEventReply,
						pArrayOfflineContext);

			break;

		case ARRAY_OFFLINE_ADT_RECORD_UPDATED:
			// Modify our local copy,
			ModifyRmstrData(
				RAID_ARRAY,
				&pADTRecord->thisRID,
				(void **)pADTRecord);
			eventComplete = true;
			break;

		default:
			assert(0);
		}
	}
	if (eventComplete){
		if (rc == RMSTR_SUCCESS){
			// Generate Event
			RMSTR_EVT_ARRAY_OFFLINE_STATUS *pEvtArrayOfflineStatus = 
							new RMSTR_EVT_ARRAY_OFFLINE_STATUS;
			memcpy(
				&pEvtArrayOfflineStatus->arrayData,
				pADTRecord,
				sizeof(RAID_ARRAY_DESCRIPTOR));
			memcpy(
				&pEvtArrayOfflineStatus->memberData,
				pMember,
				sizeof(RAID_ARRAY_MEMBER));

			m_pCmdServer->csrvReportEvent(
					RMSTR_EVT_ARRAY_OFFLINE,
					pEvtArrayOfflineStatus);		// event Data
			delete pEvtArrayOfflineStatus;
			pEvtArrayOfflineStatus = NULL;

			// Raise alarm
			RmstrSubmitAlarm(
				CTS_RMSTR_ARRAY_OFFLINE,
				RAID_ARRAY,
				&pADTRecord->thisRID);

			// Log Event
			LogEventWithArrayName(
				CTS_RMSTR_ARRAY_OFFLINE, 
				&pADTRecord->SRCTRID);

		}
		// Report status to Cmd Sender
		m_pCmdServer->csrvReportCmdStatus(
				pArrayOfflineContext->cmdHandle,	// handle
				status,					// completion code
				NULL,					// result Data
				(void *)pCmdInfo);		// pCmdInfo
		StopCommandProcessing(true, pArrayOfflineContext->cmdHandle);
		delete pArrayOfflineContext;
		pArrayOfflineContext = NULL;
	}
	return status;
}





