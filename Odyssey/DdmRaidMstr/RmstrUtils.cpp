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
// File: RmstrUtils.cpp
// 
// Description:
// Implementation for the raid utilities
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrUtils.cpp $
// 
// 21    12/17/99 5:33p Dpatel
// added hot copy code
// 
// 20    11/03/99 5:00p Dpatel
// added tracing...
// 
// 19    9/07/99 3:40p Dpatel
// removed member hot copy...
// 
// 18    8/27/99 6:47p Dpatel
// start util takes SRC row id instead of ADT row id
// 
// 17    8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 16    8/14/99 1:37p Dpatel
// Added event logging..
// 
// 15    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 14    8/05/99 11:07a Dpatel
// internal delete array, fake raid ddm, hot copy auto break, removed
// array name code..
// 
// 13    8/02/99 3:01p Dpatel
// changes to create array, array FT processing...
// 
// 12    7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 11    7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 10    7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 9     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 8     7/20/99 6:49p Dpatel
// Some bug fixes and changed arrayName in ArrayDescriptor to rowID.
// 
// 7     7/16/99 10:28a Dpatel
// Added DownMember and Commit Spare code. Also removed the reads for
// validation.
// 
// 6     7/09/99 5:26p Dpatel
// 
// 5     6/30/99 11:15a Dpatel
// Changes for Abort Util and Chg Priority.
// 
// 4     6/28/99 5:16p Dpatel
// Implemented new methods, changed headers.
// 
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/

#include "DdmRaidMgmt.h"


// Start Util Validation states
enum
{
	UTIL_DESCRIPTOR_RECORD_INSERTED =1,
	UTIL_INFO_UPDATED_IN_ADT
};




//************************************************************************
//	utilStartValidatedUtility
//		Prepare the source and destination members for the utility.		
//		Insert a new util descriptor record
//
//
//	handle		- Handle for the cmd
//	_pCmdInfo	- cmd packet for start util
//	_pADTRecord	- the array whose preferred read member is to be changed
//
//************************************************************************
STATUS DdmRAIDMstr::
utilStartValidatedUtility(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo,
			RAID_ARRAY_DESCRIPTOR	*_pADTRecord)
{
	STATUS							status = 0;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_START_UTIL_INFO			*pStartUtilityInfo = NULL;
	CONTEXT							*pCmdContext = NULL;
	RAID_UTIL_POLICIES				policy;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER				*pMember = NULL;

	rowID							*pSrcRowIds = NULL;
	rowID							*pDestRowIds = NULL;
	U32								numberSourceMembers = 0;
	U32								numberDestinationMembers = 0;

	pCmdContext = new CONTEXT;
	memset(pCmdContext,0,sizeof(CONTEXT));

	pCmdContext->cmdHandle	= handle;

	pCmdContext->pData		= new (RMSTR_CMND_INFO);
	memcpy(pCmdContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));

	if (_pADTRecord){
		pCmdContext->pData1		= new RAID_ARRAY_DESCRIPTOR;
		memcpy(pCmdContext->pData1,_pADTRecord,sizeof(RAID_ARRAY_DESCRIPTOR));
		pADTRecord	= (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;
	}

	pCmdInfo				= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pStartUtilityInfo		=
		(RMSTR_START_UTIL_INFO *)&pCmdParams->startUtilInfo;

	// if policy says, array operation or INIT, then fill the source/dest
	policy = pStartUtilityInfo->policy;

	if (policy.SpecifyMembersToRunOn == 1){
		numberSourceMembers = pStartUtilityInfo->numberSourceMembers;
		numberDestinationMembers = pStartUtilityInfo->numberDestinationMembers;
		pSrcRowIds = &pStartUtilityInfo->sourceMembers[0];
		pDestRowIds = &pStartUtilityInfo->destinationMembers[0];
	} else {
		switch(pStartUtilityInfo->utilityName){
		case RAID_UTIL_VERIFY:
		case RAID_UTIL_BKGD_INIT:
			// same for Verify/Init, first up member as src 
			// all remaining up members as dest
			PrepareVerifyMembers(
						pADTRecord,
						&pSrcRowIds,
						&pDestRowIds,
						&numberSourceMembers,
						&numberDestinationMembers);
			break;

		case RAID_UTIL_REGENERATE:
			PrepareRegenerateMembers(
						pADTRecord,
						&pSrcRowIds,
						&pDestRowIds,
						&numberSourceMembers,
						&numberDestinationMembers);
			break;

		case RAID_UTIL_LUN_HOTCOPY:
			/**********************************
			Hot copy code returns from here
			***********************************/
			pCmdContext->state		= UTIL_DESCRIPTOR_RECORD_INSERTED;
			status = StartHotCopy(
						pADTRecord,
						TSCALLBACK(DdmRAIDMstr,rmstrProcessStartUtilityReply),
						pCmdContext);
			return status;

		default:
			assert(0);
			break;
		}
	}

	pCmdContext->state		= UTIL_DESCRIPTOR_RECORD_INSERTED;

	STATE_IDENTIFIER			stateIdentifier;
	stateIdentifier.cmdOpcode	= pCmdInfo->opcode;
	stateIdentifier.cmdRowId	= *(rowID *)handle; 
	stateIdentifier.cmdState	= UTIL_DESCRIPTOR_RECORD_INSERTED;
	stateIdentifier.index		= 0;
	status = rmstrInsertUtilDescriptor(
					&stateIdentifier,
					pStartUtilityInfo->utilityName,	// util name
					&pADTRecord->thisRID,			// target array
					pStartUtilityInfo->priority,	// priority
					pStartUtilityInfo->updateRate,	// update rate
					pStartUtilityInfo->policy,		// policies
					pSrcRowIds,						
					numberSourceMembers,								
					pDestRowIds,			
					numberDestinationMembers,
					pADTRecord->memberCapacity,
					TSCALLBACK(DdmRAIDMstr,rmstrProcessStartUtilityReply),
					pCmdContext);
	if (pSrcRowIds){
		delete pSrcRowIds;
	}
	if (pDestRowIds){
		delete pDestRowIds;
	}
	return status;
}



//************************************************************************
//	rmstrProcessStartUtilityReply
//		Insert a new util descriptor record
//		Update the ADT to reflect the new util row id
//
//************************************************************************
STATUS DdmRAIDMstr::
rmstrProcessStartUtilityReply(void *_pContext, STATUS status)
{
	CONTEXT							*pCmdContext = (CONTEXT *)_pContext;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_UTILITY				*pUtility = NULL;
	RMSTR_START_UTIL_INFO			*pStartUtilityInfo = NULL;
	RaidRequest						*pRaidRequest = NULL;
	BOOL							rc = RMSTR_SUCCESS;
	BOOL							cmdComplete = false;

	STATE_IDENTIFIER				stateIdentifier;


	// pCmdContext->pData = CmdInfo
	// pCmdContext->pData1 = ADT Record
	// pCmdContext->pData2 = Context specific data for util
	pCmdInfo		= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pStartUtilityInfo = 
		(RMSTR_START_UTIL_INFO *)&pCmdParams->startUtilInfo;
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;
	pUtility = (RAID_ARRAY_UTILITY *)pCmdContext->pData2;


	if (status != OS_DETAIL_STATUS_SUCCESS){
		TRACEF(TRACE_L1, ("Start Utility: Invalid command\n"));
		rc = RMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){
		case 	UTIL_DESCRIPTOR_RECORD_INSERTED:
			TRACEF(TRACE_L1, ("Start Utility: Row inserted for utility=0x%x\n", 
				pUtility->utilityCode));
		
			// Add the Utility to Rmstr Data
			AddRmstrData(
					RAID_UTILITY,
					&pUtility->thisRID,
					pUtility);

			// Update the ADT to put the RID of the util
			pADTRecord->utilities[pADTRecord->numberUtilities] = 
					pUtility->thisRID;
			pADTRecord->numberUtilities++;
			// update the state id in ADT
			pADTRecord->stateIdentifier.cmdOpcode = pCmdInfo->opcode;
			pADTRecord->stateIdentifier.cmdRowId = *(rowID *)pCmdContext->cmdHandle;
			pADTRecord->stateIdentifier.cmdState = UTIL_INFO_UPDATED_IN_ADT;
			pADTRecord->stateIdentifier.index = 0;

			pCmdContext->state = UTIL_INFO_UPDATED_IN_ADT;

			// prepare the state identifier
			stateIdentifier.cmdOpcode = pCmdInfo->opcode;
			stateIdentifier.cmdRowId = *(rowID *)pCmdContext->cmdHandle;
			stateIdentifier.cmdState = UTIL_INFO_UPDATED_IN_ADT;
			stateIdentifier.index = 0;

			// if state already processed, then will read the new
			// data into pADTRecord, else will modify
			CheckAndModifyRow(
						RAID_ARRAY,
						&stateIdentifier,
						RAID_ARRAY_DESCRIPTOR_TABLE,
						&pADTRecord->thisRID,	// row id to modify
						pADTRecord,
						sizeof(RAID_ARRAY_DESCRIPTOR),
						&pADTRecord->thisRID,
						TSCALLBACK (DdmRAIDMstr,rmstrProcessStartUtilityReply),
						pCmdContext);

			break;

		case 	UTIL_INFO_UPDATED_IN_ADT:

			ModifyRmstrData(
				RAID_ARRAY,
				&pADTRecord->thisRID,
				pADTRecord);

			TRACEF(TRACE_L1, ("Start Utility: util info updated in ADT\n")); 
			pRaidRequest = new RaidRequest;
			memset(pRaidRequest,0,sizeof(RaidRequest));
			pRaidRequest->Opcode = 	RAID_REQUEST_START_UTIL;
			pRaidRequest->RaidVDN = pADTRecord->arrayVDN;
			pRaidRequest->Data.UtilityData = *pUtility;
			status = m_pRaidCmdSender->csndrCheckAndExecute(
				pRaidRequest,
				(pCmdCompletionCallback_t)&DdmRAIDMstr::RaidDdmCommandCompletionReply,
				pCmdContext);
			delete pRaidRequest;
			cmdComplete = true;
			break;

		default:
			rc = RMSTR_ERR_INVALID_COMMAND;
			cmdComplete = true;
			break;
		}
	}
	if (cmdComplete){
		TRACEF(TRACE_L1, ("Start Utility : cmd sent to Raid ddm\n")); 
		// only if the cmd fails, report status, else status is
		// reported back when we hear from the raid ddm
		if (rc) {
			m_pCmdServer->csrvReportCmdStatus(
				pCmdContext->cmdHandle,			// handle
				rc,								// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCQRecord
			StopCommandProcessing(true, pCmdContext->cmdHandle);
			delete pCmdContext;
		}
	}
	return status;
}


//************************************************************************
//	rmstrInsertUtilDescriptor
//		Insert a new util descriptor record
//
//************************************************************************
STATUS DdmRAIDMstr::
rmstrInsertUtilDescriptor(
			STATE_IDENTIFIER	*pStateIdentifier,
			RAID_UTIL_NAME		utilityCode,
			rowID				*pArrayRID,
			RAID_UTIL_PRIORITY	priority,
			U32					percentCompleteUpdateRate,
			RAID_UTIL_POLICIES	policy,
			rowID				*pSourceRID,
			U32					numSourceMembers,
			rowID				*pDestinationRIDs,
			U32					numDestinationMembers,
			I64					endLBA,
			pTSCallback_t		cb,
			CONTEXT				*pCmdContext)
{
	STATUS				status = RMSTR_SUCCESS;
	U32					i=0;

	if (pCmdContext->pData2){
		delete pCmdContext->pData2;
		pCmdContext->pData2 = NULL;
	}
	RAID_ARRAY_UTILITY *pUtility = new (tZERO) RAID_ARRAY_UTILITY;

	pCmdContext->pData2 = pUtility;

	pUtility->version = RAID_UTIL_DESCRIPTOR_TABLE_VERSION;
	pUtility->size = sizeof(RAID_ARRAY_UTILITY);

	// Fill in the utility information
	if (pArrayRID)
		pUtility->targetRID = *pArrayRID;
	pUtility->utilityCode = utilityCode;
	pUtility->priority = priority;
	pUtility->percentCompleteUpdateRate = percentCompleteUpdateRate;
	pUtility->status = RAID_UTIL_NOT_RUNNING;
	pUtility->policy = policy;
	pUtility->endLBA = endLBA;

	for(i=0; i<numSourceMembers; i++){
		pUtility->sourceRowIds[i] = pSourceRID[i];
	}
	for(i=0; i<numDestinationMembers; i++){
		pUtility->destinationRowIds[i] = pDestinationRIDs[i];
	}

	// copy the new state identifier into the Utility record
	// to be inserted
	pUtility->stateIdentifier = *pStateIdentifier;

	// if state was already processed, pUtil should have UDT_INSERTED state or greater
	status = CheckAndInsertRow(
				RAID_UTILITY,
				pStateIdentifier,
				RAID_UTIL_DESCRIPTOR_TABLE,
				pCmdContext->pData2,
				sizeof(RAID_ARRAY_UTILITY),
				&pUtility->thisRID,
				cb,
				pCmdContext);
	return status;
}


//************************************************************************
//	PrepareVerifyMembers
//		This is a convinience method to prepare the source and destination
//		for verify and initialize utilities.
//	pADTRecord		- array to run the util on
//	**ppSrcRowIds	- where the src row ids are filled and returned
//	**ppDestRowIds	- where the dest row ids are filled and returned
//	*pNumberSourceMembers		- where no. of src members are returned
//	*pNumberDestinationMembers	- where no. of dest members are returned
//
//************************************************************************
void DdmRAIDMstr::
PrepareVerifyMembers(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		rowID						**ppSrcRowIds,
		rowID						**ppDestRowIds,
		U32							*pNumberSourceMembers,
		U32							*pNumberDestinationMembers)
{
	U32							i=0;
	U32							k=0;
	BOOL						srcMembersDone = false;
	RAID_ARRAY_MEMBER			*pDestMembers[MAX_ARRAY_MEMBERS] = {0,0,0};

	// Find the source primary and assign it to src
	// Find All Other Up Members and assign them to destination
	for (i = 0;i < pADTRecord->numberMembers; i++){
		GetRmstrData(
			RAID_MEMBER,
			&pADTRecord->members[i],
			(void **)&pDestMembers[k]);
		if (pDestMembers[k]){
			if (pDestMembers[k]->policy.SourcePrimary){
					*ppSrcRowIds = new rowID;
					**ppSrcRowIds = pDestMembers[k]->thisRID;
					*pNumberSourceMembers = 1;
					srcMembersDone = true;
			} else {
				if (pDestMembers[k]->memberHealth == RAID_STATUS_UP){
					(*pNumberDestinationMembers)++;
					k++;
				}
			}
		}
	}

	assert(*pNumberSourceMembers);
	assert(*pNumberDestinationMembers);

	// prepare the destination row ids
	U32		j = 0;
	rowID	*pTemp = NULL;

	if (k != 0){
		*ppDestRowIds = (rowID *)new char[sizeof(rowID) * k];
		// save the starting ptr
		pTemp = *ppDestRowIds;
	}

	for (j=0; j < k; j++){
		**ppDestRowIds = pDestMembers[j]->thisRID;
		(*ppDestRowIds)++;
	}
	if (*ppDestRowIds){
		// restore the ptr to starting of row Ids
		*ppDestRowIds = pTemp;
	}
}


//************************************************************************
//	PrepareRegenerateMembers
//		This is a convinience method to prepare the source and destination
//		for regenerate and hot copy utils
//	pADTRecord		- array to run the util on
//	**ppSrcRowIds	- where the src row ids are filled and returned
//	**ppDestRowIds	- where the dest row ids are filled and returned
//	*pNumberSourceMembers		- where no. of src members are returned
//	*pNumberDestinationMembers	- where no. of dest members are returned
//
//************************************************************************
void DdmRAIDMstr::
PrepareRegenerateMembers(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		rowID						**ppSrcRowIds,
		rowID						**ppDestRowIds,
		U32							*pNumberSourceMembers,
		U32							*pNumberDestinationMembers)
{
	U32							i=0;
	BOOL						srcMembersDone = false;
	RAID_ARRAY_MEMBER			*pDestMembers[MAX_ARRAY_MEMBERS] = {0,0,0};


	// Find source primary and assign it as source
	// Find All Down Members and assign it to destination

	U32		k=0;	
	for (i=0; i < pADTRecord->numberMembers; i++){
		GetRmstrData(
			RAID_MEMBER,
			&pADTRecord->members[i],
			(void **)&pDestMembers[k]);
		if (pDestMembers[k]){
			if (pDestMembers[k]->policy.SourcePrimary){
					*ppSrcRowIds = new rowID;
					**ppSrcRowIds = pDestMembers[k]->thisRID;
					*pNumberSourceMembers = 1;
					srcMembersDone = true;
			} else {
				if (pDestMembers[k]->memberHealth == RAID_STATUS_DOWN){
					(*pNumberDestinationMembers)++;
					k++;
				}
			}
		}
	}

	assert(*pNumberSourceMembers);
	assert(*pNumberDestinationMembers);

	// prepare the destination
	U32		j = 0;
	rowID	*pTemp = NULL;

	if (k != 0){
		*ppDestRowIds = (rowID *)new char[sizeof(rowID) * k];
		// save the starting ptr
		pTemp = *ppDestRowIds;
	}

	for (j=0; j < k; j++){
		**ppDestRowIds = pDestMembers[j]->thisRID;
		(*ppDestRowIds)++;
	}
	if (*ppDestRowIds){
		// restore the ptr
		*ppDestRowIds = pTemp;
	}

}


//************************************************************************
//	rmstrStartUtilReadTarget
//
//************************************************************************
STATUS DdmRAIDMstr::
rmstrStartUtilReadTarget(
		HANDLE						handle, 
		RMSTR_CMND_INFO				*pCmdInfo)
{
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_START_UTIL_INFO			*pStartUtilInfo = NULL;
	CONTEXT							*pContext = new CONTEXT;

	// get the start utility info
	pCmdParams		= &pCmdInfo->cmdParams;
	pStartUtilInfo	= 
				(RMSTR_START_UTIL_INFO *)&pCmdParams->startUtilInfo;
	
	pContext->newRowId = pStartUtilInfo->targetRowId;
	pContext->cmdHandle = handle;
	pContext->pData = new RMSTR_CMND_INFO;
	memcpy(pContext->pData, pCmdInfo, sizeof(RMSTR_CMND_INFO));
	pContext->pData1 = new StorageRollCallRecord;
	// First read the target id to get the ADT record
	STATUS status = m_pTableServices->TableServiceReadRow(
					STORAGE_ROLL_CALL_TABLE,
					&pContext->newRowId,
					pContext->pData1,
					sizeof(StorageRollCallRecord),
					TSCALLBACK(DdmRAIDMstr,ProcessStartUtilReadTargetReply),
					pContext);
	return status;					
}



//************************************************************************
//	rmstrProcessStartUtilReadTargetReply
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessStartUtilReadTargetReply(void *_pContext, STATUS status)
{
	CONTEXT							*pContext = (CONTEXT *)_pContext;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;
	HANDLE							handle;

	handle = pContext->cmdHandle;

	// pCmdContext->pData = CmdInfo
	// pCmdContext->pData1 = SRC record
	pCmdInfo		= (RMSTR_CMND_INFO *)pContext->pData;
	pSRCRecord		= (StorageRollCallRecord *)pContext->pData1;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		// Report error to RMSTR SQ
		m_pCmdServer->csrvReportCmdStatus(
				handle,							// handle
				RMSTR_ERR_INVALID_COMMAND,			// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCQRecord
		StopCommandProcessing(true, handle);
	} else {
		rmstrStartUtilValidation(
			handle, 
			&pSRCRecord->ridDescriptorRecord,
			pCmdInfo);
	}
	delete pContext;
	return status;
}




//************************************************************************
//	rmstrStartUtilValidation
//		Perform validation if a particular utility can be started
//		or not. Check for
//		- util already running
//		- if util allowed for a particular raid level
//		- if array to run util on is present or not
//
//************************************************************************
STATUS DdmRAIDMstr::
rmstrStartUtilValidation(
		HANDLE						handle, 
		rowID						*_pTargetRowId,
		RMSTR_CMND_INFO				*pCmdInfo)
{
	STATUS							rc = RMSTR_SUCCESS;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_START_UTIL_INFO			*pStartUtilInfo = NULL;
	rowID							*pTargetArrayRowId = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;

	STATE_IDENTIFIER				stateIdentifier;
	BOOL							stateProcessed = false;



	// get the start utility info
	pCmdParams		= &pCmdInfo->cmdParams;
	pStartUtilInfo	= 
				(RMSTR_START_UTIL_INFO *)&pCmdParams->startUtilInfo;

	// if we have inserted a util with the same cmd row id
	// we will proceed directly to insert without validation
	// since this will be a failover case.
	stateIdentifier.cmdRowId = *(rowID *)handle;
	stateIdentifier.cmdOpcode = pCmdInfo->opcode;
	stateIdentifier.cmdState = UTIL_DESCRIPTOR_RECORD_INSERTED;
	stateIdentifier.index = 0;
	RAID_ARRAY_UTILITY	*pUtility = new RAID_ARRAY_UTILITY;
	stateProcessed = CheckIfStateAlreadyProcessed(
							RAID_UTILITY,
							&stateIdentifier,
							pUtility);
	if (stateProcessed){
		GetRmstrData(
			RAID_ARRAY,
			&pUtility->targetRID,
			(void **)&pADTRecord);
		// if util is already started by RAID DDM then we
		// better return..
		if (CheckIfUtilAlreadyRunning(
					pStartUtilInfo->utilityName,
					pStartUtilInfo->policy,
					pStartUtilInfo->numberDestinationMembers,
					&pStartUtilInfo->destinationMembers[0],
					pADTRecord)){
				rc = RMSTR_ERR_UTIL_ALREADY_RUNNING;
		} else {
		}
	} else {
		// get the target row id
		//pTargetArrayRowId = &pStartUtilInfo->targetRowId;
		pTargetArrayRowId = _pTargetRowId;
		if (pTargetArrayRowId) {
			GetRmstrData(
					RAID_ARRAY,
					pTargetArrayRowId,
					(void **)&pADTRecord);
			if (pADTRecord){
				rc = CheckIfUtilAllowed(
									pStartUtilInfo->utilityName,
									pADTRecord);
				if (rc == RMSTR_SUCCESS){
					if (CheckIfUtilAlreadyRunning(
									pStartUtilInfo->utilityName,
									pStartUtilInfo->policy,
									pStartUtilInfo->numberDestinationMembers,
									&pStartUtilInfo->destinationMembers[0],
									pADTRecord)){
						rc = RMSTR_ERR_UTIL_ALREADY_RUNNING;
					}
				} 
			} else {
				// could not read ADT Record
				rc = RMSTR_ERR_INVALID_COMMAND;
			}
		} else {
			// Resolve: case of member hot copy
			// just start hot copy from source to destination
		}
	}
	if (rc){
		// Report error to RMSTR SQ
		TRACEF(TRACE_L1, ("StartUtility: Validation failed!!\n")); 
		m_pCmdServer->csrvReportCmdStatus(
				handle,							// handle
				rc,								// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCQRecord
		StopCommandProcessing(true, handle);
	} else {
		TRACEF(TRACE_L1, ("StartUtility: Validation successful, starting util!!\n")); 	
		utilStartValidatedUtility(
				handle,
				pCmdInfo,
				pADTRecord);
	}
	delete pUtility;
	return rc;
}



//************************************************************************
//	CheckIfUtilAllowed
//		A convinience method to Check if Utility is valid for 
//		the current array state.
//************************************************************************
STATUS DdmRAIDMstr::
CheckIfUtilAllowed(
			RAID_UTIL_NAME			utilityName,
			RAID_ARRAY_DESCRIPTOR	*pADTRecord)
{
	STATUS			status = RMSTR_SUCCESS;


	// do checking based on raid level and health of array
	switch(pADTRecord->raidLevel){
	case RAID0:
		switch (utilityName){
			case RAID_UTIL_BKGD_INIT:
			case RAID_UTIL_VERIFY:
			case RAID_UTIL_REGENERATE:
				status = RMSTR_ERR_UTIL_NOT_SUPPORTED;
				break;
			case RAID_UTIL_LUN_HOTCOPY:
				switch (pADTRecord->health){
					case RAID_OFFLINE:
						status = RMSTR_ERR_ARRAY_STATE_OFFLINE;
						break;
					case RAID_OKAY:
						break;
				}
				break;
		}
		break;

	case RAID1:
		switch (utilityName){
			case RAID_UTIL_BKGD_INIT:
				break;

			case RAID_UTIL_VERIFY:
				switch (pADTRecord->health){
					case RAID_CRITICAL:
						status = RMSTR_ERR_ARRAY_STATE_CRITICAL;
						break;
					case RAID_OFFLINE:
						status = RMSTR_ERR_ARRAY_STATE_OFFLINE;
						break;
					case RAID_FAULT_TOLERANT:
						break;
				}
				break;

			case RAID_UTIL_REGENERATE:
				switch (pADTRecord->health){
					case RAID_OFFLINE:
						status = RMSTR_ERR_ARRAY_STATE_OFFLINE;
						break;
				}
				break;

			case RAID_UTIL_LUN_HOTCOPY:
				break;
			default:
				assert(0);
		}
		break;
	}
	return status;
}



//************************************************************************
//	CheckIfUtilAlreadyRunning
//		A convinience method to Check if same Utility is not already
//		running on the same array/members
//************************************************************************
STATUS DdmRAIDMstr::
CheckIfUtilAlreadyRunning(
			RAID_UTIL_NAME			utilityName,
			RAID_UTIL_POLICIES		utilPolicy,
			U32						numberDestinationMembers,
			rowID					*pDestinationRowIds,
			RAID_ARRAY_DESCRIPTOR	*pADTRecord)
{
	STATUS					isUtilityRunning = false;
	RAID_ARRAY_UTILITY		*pUtility = NULL;
	U32						i=0, k=0;
	BOOL					utilFound = false;

	if (pADTRecord->numberUtilities){
		TraverseRmstrData(
				RAID_UTILITY,
				NULL,
				(void **)&pUtility);
		while(pUtility){
			switch(utilityName){
			case RAID_UTIL_REGENERATE:
			case RAID_UTIL_LUN_HOTCOPY:
				switch(pUtility->utilityCode){
					case RAID_UTIL_REGENERATE:
					case RAID_UTIL_LUN_HOTCOPY:
						utilFound = true;
						break;
				}
				break;
			case RAID_UTIL_VERIFY:
			case RAID_UTIL_BKGD_INIT:
				if (pUtility->utilityCode == utilityName){
					utilFound = true;
				}
			}
			if (utilFound){
				// same utility found on the array
				// Now check if utility running on same members
				if (utilPolicy.SpecifyMembersToRunOn == 0){
					isUtilityRunning = true;
				} else {
					// Check if destination members of new utility
					// are not present in the current utility
					for (i=0; i < MAX_ARRAY_MEMBERS; i++){
						for (k=0; k < numberDestinationMembers; k++){
							if (
								memcmp(
									&pDestinationRowIds[k],
									&pUtility->destinationRowIds[i],
									sizeof(rowID)) == 0){
								isUtilityRunning = true;
								break;
							}
						}
					}
				}
			}
			if (isUtilityRunning){
				break;
			}
			TraverseRmstrData(
				RAID_UTILITY,
				&pUtility->thisRID,
				(void **)&pUtility);
		}
	}
	return isUtilityRunning;
}