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
// File: RmstrChangePriority.cpp
// 
// Description:
// Implementation for the change priority command
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrChangePriority.cpp $
// 
// 10    8/31/99 6:39p Dpatel
// events, util abort processing etc.
// 
// 9     8/14/99 1:37p Dpatel
// Added event logging..
// 
// 8     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 7     7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 6     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 5     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 4     7/17/99 1:20p Dpatel
// Queued up commands.
// 
// 3     7/16/99 10:39a Dpatel
// Removed some reads..
// 
// 2     7/09/99 5:26p Dpatel
// 
// 1     6/30/99 1:33p Dpatel
// Initial creation
// 
//
//
/*************************************************************************/

#include "DdmRaidMgmt.h"


// Change Priority states
enum
{
	CHANGE_PRIORITY_UDT_PRIORITY_UPDATED = 1
};



//************************************************************************
//	ChangeThePriority
//		Update the priority in the UDT
//	Note: The event for priority changed is generated when we
//	get a cmd completion code from the Raid Ddm.
//
//
//	handle		- Handle for the cmd
//	_pCmdInfo	- cmd packet for change util priority
//	_pUtility	- the UDT of the util whose priority is to be changed
//
//************************************************************************
STATUS DdmRAIDMstr::
ChangeThePriority(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo,
			RAID_ARRAY_UTILITY		*_pUtility)
{
	STATUS							status = RMSTR_SUCCESS;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_CHANGE_PRIORITY_INFO		*pChangePriorityInfo = NULL;
	CONTEXT							*pCmdContext = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RAID_ARRAY_UTILITY				*pUtility = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;

	// save the information into our context
	pCmdContext = new CONTEXT;

	pCmdContext->cmdHandle = handle;
	pCmdContext->pData	= new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));

	pCmdContext->pData1 = new RAID_ARRAY_UTILITY;
	memcpy(pCmdContext->pData1,_pUtility,sizeof(RAID_ARRAY_UTILITY));
	pUtility = (RAID_ARRAY_UTILITY *)pCmdContext->pData1;


	pCmdInfo				= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pChangePriorityInfo		=
		(RMSTR_CHANGE_PRIORITY_INFO *)&pCmdParams->changePriorityInfo;


	GetRmstrData(
		RAID_ARRAY,
		&pUtility->targetRID,
		(void **)&pADTRecord);

	if (pADTRecord){
		pCmdContext->pData2 = new RAID_ARRAY_DESCRIPTOR;
		memcpy(pCmdContext->pData2,pADTRecord,sizeof(RAID_ARRAY_DESCRIPTOR));

		pCmdContext->state = CHANGE_PRIORITY_UDT_PRIORITY_UPDATED;
		// save the old priority
		pCmdContext->value = pUtility->priority;
		// set the new priority and modify UDT record
		pUtility->priority = pChangePriorityInfo->newPriority;
		m_pTableServices->TableServiceModifyField(
						RAID_UTIL_DESCRIPTOR_TABLE,
						&pUtility->thisRID,	// row id to modify
						fdPRIORITY,
						&pUtility->priority,
						sizeof(RAID_UTIL_PRIORITY),
						(pTSCallback_t)&DdmRAIDMstr::ProcessChangePriorityReply,
						pCmdContext);
	} else {
		status = RMSTR_ERR_INVALID_COMMAND;
	}
	if (status){
		m_pCmdServer->csrvReportCmdStatus(
				handle,							// handle
				status,							// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCmdInfo
		StopCommandProcessing(true, handle);
		delete pCmdContext;
		pCmdContext = NULL;
	}
	return status;	
}


//************************************************************************
//	ProcessChangePriorityReply
//		Update the priority in the UDT,
//		Send a RaidRequest to the Raid DDM.
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessChangePriorityReply(void *_pContext, STATUS status)
{
	CONTEXT							*pCmdContext;
	RMSTR_CMND_INFO					*pCmdInfo;
	RMSTR_CMND_PARAMETERS			*pCmdParams;
	STATUS							rc = RMSTR_SUCCESS;
	RMSTR_CHANGE_PRIORITY_INFO		*pChangePriorityInfo;
	RAID_ARRAY_UTILITY				*pUtility;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord;
	RaidRequest						*pRaidRequest = NULL;
	RaidSetPriorityStruct			*pRaidSetPriorityStruct = NULL;


	rc					= RMSTR_SUCCESS;
	pCmdContext			= (CONTEXT *)_pContext;
	pCmdInfo			= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pChangePriorityInfo = 
			(RMSTR_CHANGE_PRIORITY_INFO *)&pCmdParams->changePriorityInfo;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
	} else {
		// pData contains cmdInfo (RMSTR_CHANGE_PRIORITY_INFO)
		// pData1  contains the UDT 
		// pData2 = contains ADT
		// pData3 = NULL
		pUtility = (RAID_ARRAY_UTILITY *)pCmdContext->pData1;
		pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData2;

		switch(pCmdContext->state){

			case CHANGE_PRIORITY_UDT_PRIORITY_UPDATED:

				ModifyRmstrData(
					RAID_UTILITY,
					&pUtility->thisRID,
					pUtility);

				//UDT is updated, send cmd to RAID DDM
				pRaidRequest = new RaidRequest;
				pRaidSetPriorityStruct = new RaidSetPriorityStruct;
				memset(pRaidRequest,0,sizeof(RaidRequest));
				memset(pRaidSetPriorityStruct,0,sizeof(RaidSetPriorityStruct));

				pRaidRequest->Opcode = 	RAID_REQUEST_CHG_PRIORITY;
				pRaidRequest->RaidVDN = pADTRecord->arrayVDN;
				pRaidSetPriorityStruct->UtilRowID = pUtility->thisRID;
				pRaidSetPriorityStruct->Priority = pUtility->priority;
				memcpy(
					&pRaidRequest->Data.PriorityData,
					pRaidSetPriorityStruct,
					sizeof(RaidSetPriorityStruct));		
				status = m_pRaidCmdSender->csndrExecute(
					pRaidRequest,
					(pCmdCompletionCallback_t)&DdmRAIDMstr::RaidDdmCommandCompletionReply,
					pCmdContext);
				delete pRaidRequest;
				delete pRaidSetPriorityStruct;
				break;

			default:
				rc = RMSTR_ERR_INVALID_COMMAND;
				break;
		}
	}
	if (rc){
		m_pCmdServer->csrvReportCmdStatus(
				pCmdContext->cmdHandle,			// handle
				rc,								// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCmdInfo
		StopCommandProcessing(true, pCmdContext->cmdHandle);
		delete pCmdContext;
	}
	return status;
}


//************************************************************************
//	ChangePriorityValidation
//		Check if util running
//
//************************************************************************
STATUS DdmRAIDMstr::
ChangePriorityValidation(HANDLE h, RMSTR_CMND_INFO *pCmdInfo)
{
	STATUS							rc = RMSTR_SUCCESS;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_CHANGE_PRIORITY_INFO		*pChangePriorityInfo = NULL;
	rowID							*pUtilRowId = NULL;
	RAID_ARRAY_UTILITY				*pUtility = NULL;


	// get the util
	pCmdParams		= &pCmdInfo->cmdParams;
	pChangePriorityInfo = 
			(RMSTR_CHANGE_PRIORITY_INFO *)&pCmdParams->changePriorityInfo;
	pUtilRowId = &pChangePriorityInfo->utilRowId;

	GetRmstrData(
		RAID_UTILITY,
		pUtilRowId,
		(void **)&pUtility);

	if (pUtility){
		if (rc == RMSTR_SUCCESS){
			if (pUtility->status != RAID_UTIL_RUNNING){
				rc = RMSTR_ERR_UTIL_NOT_RUNNING;
			}
		}
	} else {
		rc = RMSTR_ERR_INVALID_COMMAND;
	}
		
	if (rc == RMSTR_SUCCESS){
				ChangeThePriority(
					h,
					pCmdInfo,
					pUtility);
	} else {
		m_pCmdServer->csrvReportCmdStatus(
				h,					// handle
				rc,					// completion code
				NULL,				// result Data
				(void *)pCmdInfo);	// pCmdInfo
		StopCommandProcessing(true,h);
	}

	return rc;
}


