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
// File: RmstrDeleteSpare.cpp
// 
// Description:
// Implementation for raid delete spare operation
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrDeleteSpare.cpp $
// 
// 17    11/03/99 3:32p Dpatel
// added tracing
// 
// 16    10/15/99 3:54p Dpatel
// FindFirstArrayThatFitsforSpare, no assignment to pADTRecord was being
// made...
// 
// 14    9/09/99 1:38p Dpatel
// removed ENABLE_LOGGING ifdef...
// 
// 13    8/27/99 5:24p Dpatel
// added event code..
// 
// 12    8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 11    8/14/99 1:37p Dpatel
// Added event logging..
// 
// 10    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 9     8/03/99 5:25p Dpatel
// Removed the service method to modify SRC, used table services..
// 
// 8     7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 7     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 6     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 5     7/20/99 6:49p Dpatel
// Some bug fixes and changed arrayName in ArrayDescriptor to rowID.
// 
// 4     7/17/99 1:20p Dpatel
// Queued up commands.
// 
// 3     7/16/99 10:28a Dpatel
// Added DownMember and Commit Spare code. Also removed the reads for
// validation.
// 
// 2     7/09/99 5:26p Dpatel
// 
// 1     7/06/99 5:05p Dpatel
// Initial Creation.
// 
//
/*************************************************************************/


#include "DdmRaidMgmt.h"


// Delete Spare States
enum {
	DELETE_SPARE_SRCT_RECORD_UPDATED = 1,
	DELETE_SPARE_ADT_RECORD_UPDATED,
	DELETE_SPARE_SDT_RECORD_DELETED,
	DELETE_SPARE_ARRAY_NAME_READ,
	DELETE_SPARE_SPARE_NAME_READ
};




//************************************************************************
//	DeleteTheSpare
//		Delete a spare, dedicated, pool or host pool.
//
//	handle		- Handle for the cmd
//	_pCmdInfo	- cmd packet for create spare
//	_pSpare		- the SDT record for the spare to delete
//
//************************************************************************
STATUS DdmRAIDMstr::
DeleteTheSpare(
		HANDLE						handle,
		RMSTR_CMND_INFO				*_pCmdInfo,
		RAID_SPARE_DESCRIPTOR		*_pSpare)
{
	STATUS							status = RMSTR_SUCCESS;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_DELETE_SPARE_INFO			*pDeleteSpareInfo = NULL;
	CONTEXT							*pCmdContext = NULL;
	RAID_SPARE_DESCRIPTOR			*pSpare = NULL;

	pCmdContext = new CONTEXT;

	pCmdContext->cmdHandle	= handle;
	pCmdContext->pData		= new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));

	pCmdContext->pData1		= new RAID_SPARE_DESCRIPTOR;
	memcpy(pCmdContext->pData1, _pSpare, sizeof(RAID_SPARE_DESCRIPTOR));


	pCmdInfo				= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pDeleteSpareInfo		=
		(RMSTR_DELETE_SPARE_INFO *)&pCmdParams->deleteSpareInfo;

	pSpare = (RAID_SPARE_DESCRIPTOR *)pCmdContext->pData1;

	// First mark the spare as unused,
	pCmdContext->state = DELETE_SPARE_SRCT_RECORD_UPDATED;
	m_SRCIsUsed = SRC_UNUSED;
	status = m_pTableServices->TableServiceModifyField(
							STORAGE_ROLL_CALL_TABLE,
							&pSpare->SRCTRID,
							fdSRC_FUSED,		// field name of field to be modifiied
							&m_SRCIsUsed,		// set to unused
							sizeof(U32),
							TSCALLBACK(DdmRAIDMstr,ProcessDeleteSpareReply),
							pCmdContext);
	return status;
}	


//************************************************************************
//	ProcessDeleteSpareReply
//		Delete the Spare Descriptor Record,
//		Update the SRC record for the spare
//		Delete spare from the ADT and Modify the ADT
//		Generate Event for spare deleted.
//		Check if any arrays are affected by the delete of this spare
//			and generate warning event.
//
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessDeleteSpareReply(void *_pContext, STATUS status)
{
	CONTEXT							*pCmdContext = (CONTEXT *)_pContext;
	STATUS							rc;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_SPARE_DESCRIPTOR			*pSpare = NULL;
	RMSTR_DELETE_SPARE_INFO			*pDeleteSpareInfo = NULL;
	BOOL							cmdComplete = false;
	RMSTR_EVT_SPARE_DELETED_STATUS	*pEvtSpareDeleted = NULL;
	void							*pRowData = NULL;

	rc = RMSTR_SUCCESS;

	// pCmdContext->pData = cmdInfo
	// pCmdContext->pData1 = ADT Record (for Dedicated Spares only, else NULL)
	// pCmdContext->pData2 = spare info
	pCmdInfo		= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pDeleteSpareInfo = 
		(RMSTR_DELETE_SPARE_INFO *)&pCmdParams->deleteSpareInfo;
	pSpare = (RAID_SPARE_DESCRIPTOR *)pCmdContext->pData1;


	if (status != OS_DETAIL_STATUS_SUCCESS){
		TRACEF(TRACE_L1, ("Delete Spare: Invalid Cmd!!\n"));					
		rc = RMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){
		case DELETE_SPARE_SRCT_RECORD_UPDATED:
			// if dedicated spare, update the ADT to remove spare
			if (pSpare->spareType == RAID_DEDICATED_SPARE){
				TRACEF(TRACE_L2, ("Delete Dedicated Spare: SRC record updated\n"));
				// First read the ADT record, so you can update spare info
				GetRmstrData(
						RAID_ARRAY,
						&pSpare->arrayRID,
						&pRowData);
				pCmdContext->pData2 = new RAID_ARRAY_DESCRIPTOR;
				memcpy(pCmdContext->pData2, pRowData, sizeof(RAID_ARRAY_DESCRIPTOR));
				pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData2;

				// Change the info in pADTRecord
				rmstrServiceDeleteMemberSpareUtilityFromADT(
									RAID_SPARE,
									&pSpare->thisRID,
									pADTRecord);
				// Now Modify the ADT Record in PTS
				pCmdContext->state = DELETE_SPARE_ADT_RECORD_UPDATED;

				// update state identifier
				SetStateIdentifier(
					&pADTRecord->stateIdentifier,
					pCmdInfo->opcode,
					(rowID *)pCmdContext->cmdHandle,
					DELETE_SPARE_ADT_RECORD_UPDATED,
					0);
				status = CheckAndModifyRow(
							RAID_ARRAY,
							&pADTRecord->stateIdentifier,
							RAID_ARRAY_DESCRIPTOR_TABLE,
							&pADTRecord->thisRID,	// row id to modify
							pADTRecord,
							sizeof(RAID_ARRAY_DESCRIPTOR),
							&pADTRecord->thisRID,
							TSCALLBACK(DdmRAIDMstr,ProcessDeleteSpareReply),
							pCmdContext);
			} else {
				if (SimulateFailover(pCmdContext)){
					return status;
				}
				TRACEF(TRACE_L2, ("Delete Pool Spare: SRC record updated\n"));				
				// Delete the Spare from SDT
				pCmdContext->state = DELETE_SPARE_SDT_RECORD_DELETED;
				status = m_pTableServices->TableServiceDeleteRow(
							RAID_SPARE_DESCRIPTOR_TABLE,
							&pSpare->thisRID,			// row id to delete
							TSCALLBACK(DdmRAIDMstr,ProcessDeleteSpareReply),
							pCmdContext);
			}
			break;

		case DELETE_SPARE_ADT_RECORD_UPDATED:
			pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData2;

			TRACEF(TRACE_L2, ("Delete Spare: ADT record updated\n"));
			ModifyRmstrData(
					RAID_ARRAY,
					&pADTRecord->thisRID,
					pADTRecord);

			pCmdContext->state = DELETE_SPARE_ARRAY_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
				&pCmdContext->ucArrayName,
				NULL,
				&pADTRecord->SRCTRID,
				TSCALLBACK(DdmRAIDMstr,ProcessDeleteSpareReply),
				pCmdContext);

			break;

		case DELETE_SPARE_ARRAY_NAME_READ:
			// Delete the Spare row id
			TRACEF(TRACE_L2, ("Delete Spare: Spare name read\n"));			
			pCmdContext->state = DELETE_SPARE_SDT_RECORD_DELETED;
			status = m_pTableServices->TableServiceDeleteRow(
						RAID_SPARE_DESCRIPTOR_TABLE,
						&pSpare->thisRID,			// row id to delete
						TSCALLBACK(DdmRAIDMstr,ProcessDeleteSpareReply),
						pCmdContext);
			break;

		case DELETE_SPARE_SDT_RECORD_DELETED:
			TRACEF(TRACE_L2, ("Delete Spare: SDT Record deleted!!\n"));					
			RemoveRmstrData(RAID_SPARE, &pSpare->thisRID);
			pCmdContext->state = DELETE_SPARE_SPARE_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
				&pCmdContext->ucMemberName,
				&pCmdContext->SlotID,
				&pSpare->SRCTRID,
				TSCALLBACK(DdmRAIDMstr,ProcessDeleteSpareReply),
				pCmdContext);
			break;

		case DELETE_SPARE_SPARE_NAME_READ:
			cmdComplete = true;
			break;

		default:
			break;
		}
	}
	if (cmdComplete){
		// Report the status of the Delete spare back 
		m_pCmdServer->csrvReportCmdStatus(
			pCmdContext->cmdHandle,		// handle
			rc,							// completion code
			NULL,						// result Data
			(void *)pCmdInfo);			// Orig cmd info
		if (rc == RMSTR_SUCCESS){
			TRACEF(TRACE_L1, ("Delete Spare: Cmd completed successfully!!\n"));					
		
			// Generate event for Spare Deleted
			pEvtSpareDeleted = new (tZERO) RMSTR_EVT_SPARE_DELETED_STATUS;

			pEvtSpareDeleted->spareData = *pSpare;
			m_pCmdServer->csrvReportEvent(
				RMSTR_EVT_SPARE_DELETED,	// completion code
				pEvtSpareDeleted);			// event Data
			delete pEvtSpareDeleted;
			pEvtSpareDeleted = NULL;

			LogSpareDeletedEvent(pCmdContext,pSpare);
			if (pSpare->spareType == RAID_DEDICATED_SPARE){
				pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData2;				
			}
			LogNoMoreSparesEvent(pADTRecord, pSpare);
		}
		StopCommandProcessing(true, pCmdContext->cmdHandle);
		delete pCmdContext;
		pCmdContext = NULL;
	}
	return status;
}




//************************************************************************
//	TraverseArrayThatFitsForThisSpare
//		Convinience method to find any array that fits for this spare.
//		RAID 0 arrays are not checked.
//
//************************************************************************
BOOL DdmRAIDMstr::
TraverseArrayThatFitsForThisSpare(
			RAID_SPARE_DESCRIPTOR		*pSpare,
			RAID_ARRAY_DESCRIPTOR		**ppADTRecord)
{
	BOOL					arrayFound = false;

	RAID_ARRAY_DESCRIPTOR	*pADTRecord = *ppADTRecord;

	if (pADTRecord == NULL){
		TraverseRmstrData(
			RAID_ARRAY,
			NULL,
			(void **)ppADTRecord);
	} else {
		TraverseRmstrData(
			RAID_ARRAY,
			&pADTRecord->thisRID,
			(void **)ppADTRecord);
	}

	while (pADTRecord){
		if (pADTRecord->raidLevel != RAID0){
			if (pSpare->capacity >= pADTRecord->memberCapacity){
				arrayFound = true;
				break;
			}
		}
		TraverseRmstrData(
			RAID_ARRAY,
			&pADTRecord->thisRID,
			(void **)ppADTRecord);
		pADTRecord = *ppADTRecord;
	}
	
	return arrayFound;
}


//************************************************************************
//	DeleteSpareValidation
//		Validates the delete spare command. Checks if the spare exists or
//		not.
//
//************************************************************************
STATUS DdmRAIDMstr::
DeleteSpareValidation(HANDLE h, RMSTR_CMND_INFO *_pCmdInfo)
{
	STATUS							status = RMSTR_SUCCESS;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_DELETE_SPARE_INFO			*pDeleteSpareInfo = NULL;
	RMSTR_CMND_INFO					*pCmdInfo =NULL;
	RAID_SPARE_DESCRIPTOR			*pSpare = NULL;

	CONTEXT	*pValidationContext			= new CONTEXT;

	// save the Cmd Info and the handle
	pValidationContext->cmdHandle	= h;

	pValidationContext->pData		= new RMSTR_CMND_INFO;
	memcpy(pValidationContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));


	pCmdInfo			= (RMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pDeleteSpareInfo	= 
		(RMSTR_DELETE_SPARE_INFO *)&pCmdParams->deleteSpareInfo;

	// first Get the SDT record
	GetRmstrData(RAID_SPARE, &pDeleteSpareInfo->spareId, (void **)&pSpare);
	if (pSpare) {
		DeleteTheSpare(
				pValidationContext->cmdHandle,
				pCmdInfo,
				pSpare);
	} else {
		status = RMSTR_ERR_INVALID_COMMAND;
	}
	if (status){
		// Report error to the cmd Sender
		m_pCmdServer->csrvReportCmdStatus(
				pValidationContext->cmdHandle,	// handle
				status,							// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCmdInfo
		StopCommandProcessing(true, pValidationContext->cmdHandle);
		delete pValidationContext;
		pValidationContext = NULL;
	}
	return status;
}






