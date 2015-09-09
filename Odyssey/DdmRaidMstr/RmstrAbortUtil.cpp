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
// File: RmstrAbortUtil.cpp
// 
// Description:
// Implementation for the abort utility command
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrAbortUtil.cpp $
// 
// 8     8/14/99 1:37p Dpatel
// Added event logging..
// 
// 7     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 6     7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 5     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 4     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
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
// 06/29/99 Dipam Patel: Create file
//
/*************************************************************************/

#include "DdmRaidMgmt.h"




//************************************************************************
//	AbortTheValidatedUtility
//		Prepare a RAID DDM request RaidRequest for aborting the util
//	Note: We will get a UTIL_STOPPED event from the Raid DDM. We will
//	then generate a RMSTR event for util stopped with a reason indicating
//	that the util was aborted.
//
//
//	handle		- Handle for the cmd
//	_pCmdInfo	- cmd packet for abort util
//	_pUtility	- the UDT of the util to abort
//
//************************************************************************
STATUS DdmRAIDMstr::
AbortTheValidatedUtility(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo,
			RAID_ARRAY_UTILITY		*pUtility)
{
	STATUS							status = RMSTR_SUCCESS;
	CONTEXT							*pCmdContext=NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RaidRequest						*pRaidRequest = NULL;
	RaidAbortStruct					*pRaidAbortStruct = NULL;


	// save info into our context
	pCmdContext = new CONTEXT;

	pCmdContext->cmdHandle = handle;
	pCmdContext->pData = new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));
	pCmdInfo = (RMSTR_CMND_INFO *)pCmdContext->pData;

	pCmdContext->pData1 = new RAID_ARRAY_UTILITY;
	memcpy(pCmdContext->pData1,pUtility,sizeof(RAID_ARRAY_UTILITY));

	GetRmstrData(
		RAID_ARRAY,
		&pUtility->targetRID,
		(void **)&pADTRecord);
	if (pADTRecord){
		pCmdContext->pData2 = new RAID_ARRAY_DESCRIPTOR;
		memcpy(pCmdContext->pData2,pADTRecord,sizeof(RAID_ARRAY_DESCRIPTOR));
		pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData2;

		//Prepare a RAID_DDM_CMD for abort util
		pRaidRequest = new RaidRequest;
		pRaidAbortStruct = new RaidAbortStruct;
		memset(pRaidRequest,0,sizeof(RaidRequest));
		memset(pRaidAbortStruct,0,sizeof(RaidAbortStruct));

		pRaidRequest->Opcode = 	RAID_REQUEST_ABORT_UTIL;
		pRaidRequest->RaidVDN = pADTRecord->arrayVDN;
		pRaidAbortStruct->UtilRowID = pUtility->thisRID;
		memcpy(
			&pRaidRequest->Data.AbortData,
			pRaidAbortStruct,
			sizeof(RaidAbortStruct));		
		status = m_pRaidCmdSender->csndrExecute(
					pRaidRequest,
					(pCmdCompletionCallback_t)&DdmRAIDMstr::RaidDdmCommandCompletionReply,
					pCmdContext);
		delete pRaidRequest;
		delete pRaidAbortStruct;
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
	}
	return status;
}



//************************************************************************
//	AbortUtilValidation
//		Perform validation.
//		Check if policy allows util to be aborted
//		Check if util running
//
//
//************************************************************************
STATUS DdmRAIDMstr::
AbortUtilValidation(HANDLE h, RMSTR_CMND_INFO *pCmdInfo)
{
	STATUS							rc = RMSTR_SUCCESS;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_ABORT_UTIL_INFO			*pAbortUtilInfo = NULL;
	rowID							*pUtilRowId = NULL;
	RAID_ARRAY_UTILITY				*pUtility = NULL;

	// get the abort util info ptr
	pCmdParams		= &pCmdInfo->cmdParams;
	pAbortUtilInfo = 
			(RMSTR_ABORT_UTIL_INFO *)&pCmdParams->abortUtilInfo;
	pUtilRowId = &pAbortUtilInfo->utilRowId;

	GetRmstrData(
			RAID_UTILITY,
			pUtilRowId,
			(void **)&pUtility);

	if (pUtility) {
		if (pUtility->policy.CantAbort == 1){
			rc = RMSTR_ERR_UTIL_ABORT_NOT_ALLOWED;
		}
		if (rc == RMSTR_SUCCESS){
			if (pUtility->status != RAID_UTIL_RUNNING){
				rc = RMSTR_ERR_UTIL_NOT_RUNNING;
			}
		}
	} else {
		rc = RMSTR_ERR_INVALID_COMMAND; 
	}
	if (rc == RMSTR_SUCCESS){
				AbortTheValidatedUtility(
						h,
						pCmdInfo,
						pUtility);
	} else {
		m_pCmdServer->csrvReportCmdStatus(
				h,							// handle
				rc,							// completion code
				NULL,						// result Data
				(void *)pCmdInfo);			// pCmdInfo
		StopCommandProcessing(true,h);
	}
	return rc;
}


