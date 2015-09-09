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
// File: PmstrMergePartition.cpp
// 
// Description:
// Implementation for create partition
// 
// $Log: /Gemini/Odyssey/DdmPartitionMstr/MergePartition.cpp $
// 
// 6     1/21/00 12:00p Szhang
// Changed State Machine to handle several failover.
// 
// 1     9/15/99 4:00p Dpatel
// Initial creation
// 
//
/*************************************************************************/


#include "DdmPartitionMstr.h"


// Merge partition States
enum
{	
	MERGE_PARTITION_PDT1_SIZE_MODIFIED = 1,
	MERGE_PARTITION_SRC1_CAPACITY_MODIFIED,
	MERGE_PARTITION_PDT_POINTERS_UPDATED,
	MERGE_PARTITION_PARENT_SRC_UNCLAIMED,
	MERGE_PARTITION_SRC2_RECORD_DELETED1,
	MERGE_PARTITION_SRC1_RECORD_DELETED,
	MERGE_PARTITION_PDT1_PDT2_RECORD_DELETED,
	MERGE_PARTITION_NEXT_SRC_RECORD_READ,
	MERGE_PARTITION_PREV_SRC_RECORD_READ,
	MERGE_PARTITION_SRC2_RECORD_DELETED2,
	MERGE_PARTITION_PDT2_RECORD_DELETED,
	MERGE_PARTITION_PDT1_RECORD_DELETED,
	MERGE_PARTITION_SRC2_RECORD_DELETED,
	MERGE_PARTITION_SRC1_RECORD_DELETED_CLEANUP,
	MERGE_PARTITION_PDT1_RECORD_DELETED_CLEANUP
};

// Merge Partition Validation states
enum
{
	MERGE_PARTITION_VALIDATION_SRCT1_RECORD_READ = 100,
	MERGE_PARTITION_VALIDATION_SRCT2_RECORD_READ
};


//************************************************************************
//	MergePartitionValidation
//		- Check if not used
//		- check if new partition request is not already a partition
//
//	handle		- the handle for the command
//	pCmdInfo	- the cmd info
//
//************************************************************************
STATUS DdmPartitionMstr::
MergePartitionValidation(HANDLE h, PMSTR_CMND_INFO *_pCmdInfo)
{
	STATUS							status = PMSTR_SUCCESS;
	PMSTR_MERGE_PARTITION_INFO		*pMergePartitionInfo = NULL;
	PMSTR_CMND_INFO					*pCmdInfo = NULL;
	PMSTR_CMND_PARAMETERS			*pCmdParams = NULL;

	PARTITION_CONTEXT	*pValidationContext	= new PARTITION_CONTEXT;

	// save the MERGE_PARTITION_INFO and the handle
	pValidationContext->cmdHandle	= h;

	pValidationContext->pData	= new(tZERO) PMSTR_CMND_INFO;
	memcpy(pValidationContext->pData, _pCmdInfo, sizeof(PMSTR_CMND_INFO));

	pCmdInfo			= (PMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pMergePartitionInfo = 
				(PMSTR_MERGE_PARTITION_INFO *)&pCmdParams->mergePartitionInfo;

	pValidationContext->state		= MERGE_PARTITION_VALIDATION_SRCT1_RECORD_READ;
	// read the src record, Allocate space for read row data
	pValidationContext->pData1 = new(tZERO) StorageRollCallRecord;

	// read SRC1 Record 
	status = m_pTableServices->TableServiceReadRow(
				STORAGE_ROLL_CALL_TABLE,
				&pMergePartitionInfo->srcPartitionRowId1,
				pValidationContext->pData1,
				sizeof(StorageRollCallRecord),
				TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionValidationReply),
				pValidationContext);
	return status;
}



//************************************************************************
//	ProcessMergePartitionValidationReply
//		Create Partition Command Validation Reply Handler
//
//	If any error is found, then the appropriate error code is returned
//	and the command processing is stopped. If validation is successful
//	then proceed for actually creating the error

//	pContext	- the validation context
//	status		- the message status
//  
//	Validation State Machine:
//		Read SRC1
//			SRC1->fUsed?
//			Get PDT1
//		Read SRC2
//		  if SRC2
//			SRC2->fUsed?
//			Get PDT2
//		  else
//			CleanUp (handle failover happened after deleting SRC2) 
//		partitions are contiguous?
//		  PDT1's nextRowId != PDT2's nextRowId (handle failover happend after changing
//		  pointers and before deleting any records from table
//************************************************************************
STATUS DdmPartitionMstr::
ProcessMergePartitionValidationReply(void *_pContext, STATUS status)
{
	PARTITION_CONTEXT				*pValidationContext = NULL;
	PMSTR_MERGE_PARTITION_INFO		*pMergePartitionInfo = NULL;
	PMSTR_CMND_INFO					*pCmdInfo = NULL;
	PMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	STATUS							rc;
	StorageRollCallRecord			*pSRCRecord1 = NULL;
	StorageRollCallRecord			*pSRCRecord2 = NULL;
	PARTITION_DESCRIPTOR			*pPDTRecord1 = NULL;
	PARTITION_DESCRIPTOR			*pPDTRecord2 = NULL;
	PARTITION_DESCRIPTOR			*pTempPDTRecord = NULL;
	BOOL							validationComplete = false;


	rc					= PMSTR_SUCCESS;
	pValidationContext	= (PARTITION_CONTEXT *)_pContext;
	// pValidationContext->pData = cmdInfo
	// pData1 = SRC Record
	// pData2 = Partition record (maybe)
	// pData3 = NULL
	pCmdInfo			= (PMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pMergePartitionInfo = 
			(PMSTR_MERGE_PARTITION_INFO *)&pCmdParams->mergePartitionInfo;

	pSRCRecord1 	= (StorageRollCallRecord *)pValidationContext->pData1;
	pSRCRecord2 	= (StorageRollCallRecord *)pValidationContext->pData2;
	if ((status != ercKeyNotFound) &&
			(status != OS_DETAIL_STATUS_SUCCESS)){
		rc = PMSTR_ERR_INVALID_COMMAND;
		validationComplete = true;
	} else {
		switch(pValidationContext->state){
		case MERGE_PARTITION_VALIDATION_SRCT1_RECORD_READ:
			if(RowId(pSRCRecord1->rid) != 0){
				if (pSRCRecord1->fUsed){
					rc = PMSTR_ERR_STORAGE_ELEMENT_USED;
				} else {
					// Get the PDT associated with the SRC's
					m_pDataQueue->Get(
							PMSTR_PARTITION, 
							&pSRCRecord1->ridDescriptorRecord,
							(void **)&pTempPDTRecord,
							sizeof(PARTITION_DESCRIPTOR));
					if (pTempPDTRecord){
						pValidationContext->pData3 = new (tZERO) PARTITION_DESCRIPTOR;
						memcpy(
							pValidationContext->pData3,
							pTempPDTRecord,
							sizeof(PARTITION_DESCRIPTOR));
					} else {
						rc = PMSTR_ERR_INVALID_COMMAND;
					}
				}
			} else {
				rc = PMSTR_ERR_INVALID_COMMAND;
			}

			pValidationContext->state = MERGE_PARTITION_VALIDATION_SRCT2_RECORD_READ;
			pValidationContext->pData2 = new(tZERO) StorageRollCallRecord;
			// read SRC2 Record 
			status = m_pTableServices->TableServiceReadRow(
						STORAGE_ROLL_CALL_TABLE,
						&pMergePartitionInfo->srcPartitionRowId2,
						pValidationContext->pData2,
						sizeof(StorageRollCallRecord),
						TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionValidationReply),
						pValidationContext);
			break;

		case MERGE_PARTITION_VALIDATION_SRCT2_RECORD_READ:
			
			if (RowId(pSRCRecord2->rid) != 0){
				if (pSRCRecord2->fUsed){
					rc = PMSTR_ERR_STORAGE_ELEMENT_USED;
				} else {
					// Get PDT2
					pTempPDTRecord = NULL;
					m_pDataQueue->Get(
							PMSTR_PARTITION, 
							&pSRCRecord2->ridDescriptorRecord,
							(void **)&pTempPDTRecord,
							sizeof(PARTITION_DESCRIPTOR));
					if (pTempPDTRecord){
						pValidationContext->pData4 = new (tZERO) PARTITION_DESCRIPTOR;
						memcpy(
							pValidationContext->pData4,
							pTempPDTRecord,
							sizeof(PARTITION_DESCRIPTOR));
					} else {
						// we will still try to update ptrs on
						// the first partition (possibly failover)
						;
					}
				}
			}
			// Handle failover happend after deleting SRC2
			// If SRC2 does not exist, if the cmd is same as the previous one,
			// Call CleanUp to delete SRC1 and PDT1 only. PDT2 deleted in Initialization
			// already since SRC2 could not be found.
			else{
				if(memcmp(&((PARTITION_DESCRIPTOR *)(pValidationContext->pData2))->stateIdentifier.cmdRowId,(rowID *)pValidationContext->cmdHandle, sizeof(rowID))){
				    // Resolve: Get SRC2 and PDT2 records from a Table and pass to CleanUp
				    // In CleanUp, use these records to report event to SSAPI.
				    
				    // If can not find SRC2 from that Table, reject this cmd. How can 
				    // we report delete SRC2 event? 
				    
					pValidationContext->state = MERGE_PARTITION_SRC2_RECORD_DELETED; 
					status = CleanUpReply(pValidationContext,OK);
					return status;
				}
				else {
					rc = PMSTR_ERR_INVALID_COMMAND;
				}
			}
			if (rc == PMSTR_SUCCESS){
				pPDTRecord1 = (PARTITION_DESCRIPTOR *)pValidationContext->pData3;
				pPDTRecord2 = (PARTITION_DESCRIPTOR *)pValidationContext->pData4;

				// if both records are there, then only validate
				// else we try to at least update pointers
				if (pPDTRecord1 && pPDTRecord2){
					// check if the partitions supplied are contiguous
					
					// Handle failover happened after changing pointers and before 
					// deleting any records from table.
					if (RowId(pPDTRecord1->nextRowId) != RowId(pPDTRecord2->SRCTRID)&&RowId(pPDTRecord1->nextRowId) != RowId(pPDTRecord2->nextRowId)){
						if (RowId(pPDTRecord2->previousRowId) != RowId(pPDTRecord1->SRCTRID)){
							if (RowId(pPDTRecord1->previousRowId) != RowId(pPDTRecord2->SRCTRID)){
								if (RowId(pPDTRecord2->nextRowId) != RowId(pPDTRecord1->SRCTRID)&&RowId(pPDTRecord2->nextRowId) != RowId(pPDTRecord1->nextRowId)){
									rc = PMSTR_ERR_INVALID_COMMAND;
								}
							}
						}
					}
				}
			}

			if (rc == PMSTR_SUCCESS){
				// all error checking is done, issue the create partition cmd
				MergePartition(
						pValidationContext->cmdHandle,
						pCmdInfo,
						pSRCRecord1,
						pSRCRecord2,
						pPDTRecord1,
						pPDTRecord2);
			} 
			validationComplete = true;
			break;

		default:
			rc = PMSTR_ERR_INVALID_COMMAND;
			validationComplete = true;
			break;
		}
	}

	if (validationComplete){
		if(rc){
			// Report error to CmdSender
			m_pCmdServer->csrvReportCmdStatus(
				pValidationContext->cmdHandle,	// handle
				rc,								// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCmdInfo
			StopCommandProcessing(true, pValidationContext->cmdHandle);
		}
		if (pValidationContext){
			delete pValidationContext;
			pValidationContext = NULL;
		}
	}
	return status;
}
//************************************************************************
//	CleanUpReply
//		finish states after deleting SRC2 and report status
//
//************************************************************************
STATUS DdmPartitionMstr::
CleanUpReply(void * _pContext,STATUS status)
{
	PARTITION_CONTEXT				*pCleanUpContext = NULL;
	PMSTR_CMND_INFO					*pCmdInfo = NULL;
	STATUS							rc;
	BOOL							CleanUpComplete = false;
	StorageRollCallRecord			*pSRCRecord1 = NULL;
	PARTITION_DESCRIPTOR			*pPDTRecord1 = NULL;
	rc					= PMSTR_SUCCESS;
	pCleanUpContext		= (PARTITION_CONTEXT *)_pContext;

	pCmdInfo			= (PMSTR_CMND_INFO *)pCleanUpContext->pData;
	
	pSRCRecord1			= (StorageRollCallRecord *) pCleanUpContext->pData1;
	pPDTRecord1			= (PARTITION_DESCRIPTOR *) pCleanUpContext->pData3;
	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = PMSTR_ERR_INVALID_COMMAND;
		CleanUpComplete = true;
	} 
	else {
		switch(pCleanUpContext->state){
			//finish rest of states after crach 
			case MERGE_PARTITION_SRC2_RECORD_DELETED:
			pCleanUpContext->state = MERGE_PARTITION_SRC1_RECORD_DELETED_CLEANUP;
			status = m_pTableServices->TableServiceDeleteRow(
						STORAGE_ROLL_CALL_TABLE,
						&pSRCRecord1->rid,		
						TSCALLBACK(DdmPartitionMstr,CleanUpReply),
						pCleanUpContext);
			break;

			case MERGE_PARTITION_SRC1_RECORD_DELETED_CLEANUP:
				// We only need delete PDT1, since PDT2 was deleted in Initialization
				// (deleted PDT entries with no associated SRC entries).
				// Since we can not simulate failover which starts the code from 
				// Initialization, we delete PDT1 and PDT2 in one operation now.
				
				// Resolve: Still need find way to report the delete event to SSAPI,
				// If SSAPI can not recognize the crash, we may need save SRC2 and 
				// PDT2 record to a Table before delete SRC2 in MergePartionReply.
				// We can get these records in MergeValidationReply and pass to CleanUP
				// after recognizing the crash.
				
			/*	pCleanUpContext->state = MERGE_PARTITION_PDT1_RECORD_DELETED_CLEANUP;
				status = m_pTableServices->TableServiceDeleteRow(
						PARTITION_DESCRIPTOR_TABLE,
						&pPDTRecord1->rid,// pdt row id
						TSCALLBACK(DdmPartitionMstr,CleanUpReply),
						pCleanUpContext);
			*/
			 
			// Now delete PDT1 and PDT2 in one operation
			pCleanUpContext->state = MERGE_PARTITION_PDT1_PDT2_RECORD_DELETED;
			status = m_pTableServices->TableServiceDeleteAllMatchedRows(
						PARTITION_DESCRIPTOR_TABLE,
						fdPARENT_RID,
						&pPDTRecord1->parentSRCTRID,// pdt row id
						TSCALLBACK(DdmPartitionMstr,CleanUpReply),
						pCleanUpContext);
			break;

			//case MERGE_PARTITION_PDT1_RECORD_DELETED_CLEANUP:
			case MERGE_PARTITION_PDT1_PDT2_RECORD_DELETED:
				// this state is reached only if this was the last
				// remaining partition
				m_pDataQueue->Remove(
							PMSTR_PARTITION, 
							&pPDTRecord1->rid);
			/*	m_pDataQueue->Remove(
							PMSTR_PARTITION, 
							&pPDTRecord2->rid);			
			*/	CleanUpComplete = true;
			break;

			default:
				CleanUpComplete = true;
			break;
		}//switch
	}//else

	if (CleanUpComplete){
	// Report status to CmdSender
			m_pCmdServer->csrvReportCmdStatus(
				pCleanUpContext->cmdHandle,	// handle
				rc,								// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCmdInfo
			StopCommandProcessing(true, pCleanUpContext->cmdHandle);
	
		// if no error then report the event
		if (rc == PMSTR_SUCCESS){
			// first generate a partition deleted event
			/*if (RowId(pSRCRecord2->rid) && RowId(pPDTRecord2->rid)){
				PMSTR_EVT_PARTITION_DELETED_STATUS *pEvtPartitionDeleted = 
							new (tZERO) PMSTR_EVT_PARTITION_DELETED_STATUS;
				pEvtPartitionDeleted->SRCData = *pSRCRecord2;
				pEvtPartitionDeleted->partitionData = *pPDTRecord2;
				m_pCmdServer->csrvReportEvent(
					PMSTR_EVT_PARTITION_DELETED,	// completion code
					pEvtPartitionDeleted);			// event Data
				delete pEvtPartitionDeleted;
			}*/
			// Now generate event for partition modified
			PMSTR_EVT_PARTITION_MODIFIED_STATUS *pEvtPartitionModified = 
						new (tZERO) PMSTR_EVT_PARTITION_MODIFIED_STATUS;
			pEvtPartitionModified->SRCData = *pSRCRecord1;
			pEvtPartitionModified->partitionData = *pPDTRecord1;
			m_pCmdServer->csrvReportEvent(
					PMSTR_EVT_PARTITION_MODIFIED,	// completion code
					pEvtPartitionModified);			// event Data

			// if this was the last partition, then report deleted event
				m_pCmdServer->csrvReportEvent(
					PMSTR_EVT_PARTITION_DELETED,	// completion code
					pEvtPartitionModified);			// event Data
			
			delete pEvtPartitionModified;
		}

		if (pCleanUpContext){
			delete pCleanUpContext;
			pCleanUpContext = NULL;
		}
	}
	return status;
}

//************************************************************************
//	MergePartition
//		Prepare and insert the partition descriptor
//
//************************************************************************
STATUS DdmPartitionMstr::
MergePartition(
		HANDLE					handle,
		PMSTR_CMND_INFO			*_pCmdInfo,
		StorageRollCallRecord	*_pSRCRecord1,
		StorageRollCallRecord	*_pSRCRecord2,
		PARTITION_DESCRIPTOR	*_pPDTRecord1,
		PARTITION_DESCRIPTOR	*_pPDTRecord2)
{
	STATUS							status = 0;
	PARTITION_CONTEXT				*pCmdContext = NULL;
	PMSTR_MERGE_PARTITION_INFO		*pMergePartitionInfo = NULL;
	PMSTR_CMND_INFO					*pCmdInfo = NULL;
	PMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	
	StorageRollCallRecord			*pSRCRecord1 = NULL;
	StorageRollCallRecord			*pSRCRecord2 = NULL;
	PARTITION_DESCRIPTOR			*pPDTRecord1 = NULL;
	PARTITION_DESCRIPTOR			*pPDTRecord2 = NULL;


	pCmdContext = new PARTITION_CONTEXT;

	pCmdContext->cmdHandle = handle;
	// save the info into our context
	pCmdContext->pData = new PMSTR_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(PMSTR_CMND_INFO));
	

	pCmdInfo			= (PMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pMergePartitionInfo = 
			(PMSTR_MERGE_PARTITION_INFO *)&pCmdParams->mergePartitionInfo;

	pCmdContext->pData1 = new(tZERO) StorageRollCallRecord;
	memcpy(pCmdContext->pData1, _pSRCRecord1, sizeof(StorageRollCallRecord));
	pSRCRecord1 = (StorageRollCallRecord *) pCmdContext->pData1;

	pCmdContext->pData2 = new(tZERO) StorageRollCallRecord;
	memcpy(pCmdContext->pData2, _pSRCRecord2, sizeof(StorageRollCallRecord));
	pSRCRecord2 = (StorageRollCallRecord *) pCmdContext->pData2;

	pCmdContext->pData3 = new(tZERO) PARTITION_DESCRIPTOR;
	if (_pPDTRecord1)
		memcpy(pCmdContext->pData3, _pPDTRecord1, sizeof(PARTITION_DESCRIPTOR));
	pPDTRecord1 = (PARTITION_DESCRIPTOR *) pCmdContext->pData3;

	pCmdContext->pData4 = new(tZERO) PARTITION_DESCRIPTOR;
	if (_pPDTRecord2)
		memcpy(pCmdContext->pData4, _pPDTRecord2, sizeof(PARTITION_DESCRIPTOR));
	pPDTRecord2 = (PARTITION_DESCRIPTOR *) pCmdContext->pData4;

	// update PDT record 1 size and start LBA
	// update SRC record 1 Capacity
	// update pointers,(for merged partition and either the next
	// or previous partitions depending on the order)
	// if last partition, delete PDT/SRC record 1
	// delete SRC record 2
	// delete PDT record 2
	
	// update the partition size of the PDT record 1
	if (RowId(pPDTRecord1->rid) && RowId(pPDTRecord2->rid)){
		pPDTRecord1->partitionSize += pPDTRecord2->partitionSize;
		if (pPDTRecord1->nextRowId == pPDTRecord2->SRCTRID){
			pPDTRecord1->nextRowId = pPDTRecord2->nextRowId;
			pCmdContext->value1 = true;
		}
		if (pPDTRecord1->previousRowId == pPDTRecord2->SRCTRID){
			pPDTRecord1->startLBA = pPDTRecord2->startLBA;
			pPDTRecord1->previousRowId = pPDTRecord2->previousRowId;
			pCmdContext->value2 = true;
		}

		m_pDataQueue->SetStateIdentifier(
						&pPDTRecord1->stateIdentifier,
						pCmdInfo->opcode,
						(rowID *)pCmdContext->cmdHandle,
						MERGE_PARTITION_PDT1_SIZE_MODIFIED,
						pCmdContext->numProcessed);

		pCmdContext->state = MERGE_PARTITION_PDT1_SIZE_MODIFIED;
		status = m_pDataQueue->CheckAndModifyRow(
				PMSTR_PARTITION,
				&pPDTRecord1->stateIdentifier,
				PARTITION_DESCRIPTOR_TABLE,
				&pPDTRecord1->rid,
				pPDTRecord1,
				sizeof(PARTITION_DESCRIPTOR),
				&pPDTRecord1->rid,
				TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
				pCmdContext);
	} else {
		// just try to update pointers, since we can assume that
		// pPDT2 is deleted
		pCmdContext->state = MERGE_PARTITION_PDT2_RECORD_DELETED;
		ProcessMergePartitionReply(pCmdContext, status);
	}

	return status;
}



//************************************************************************
//	ProcessMergePartitionReply
//		Process the different states for create partition
//			Insert Partition Descriptor
//
//	pContext	- our context data for create array messages
//	status		- status of the message
//
//	MergePartition State Machine:
//		Modify PDT1's size
//		Modify SRC1's capacity
//		Update PDT pointers
//		if no remaining
//			Mark SRC's parent as FREE
//			Delete SRC2
//			Delete SRC1
//			Delete PDT1 and PDT2 together
//		else
//			Delete SRC2
//			Delete PDT2
//************************************************************************
STATUS DdmPartitionMstr::
ProcessMergePartitionReply(void *_pContext, STATUS status)
{
	PARTITION_CONTEXT				*pCmdContext = (PARTITION_CONTEXT *)_pContext;
	STATUS							rc = PMSTR_SUCCESS;
	PMSTR_MERGE_PARTITION_INFO		*pMergePartitionInfo = NULL;

	BOOL							cmdComplete = false;

	PMSTR_CMND_INFO					*pCmdInfo = NULL;
	PMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	
	StorageRollCallRecord			*pSRCRecord1 = NULL;
	StorageRollCallRecord			*pSRCRecord2 = NULL;
	PARTITION_DESCRIPTOR			*pPDTRecord1 = NULL;
	PARTITION_DESCRIPTOR			*pPDTRecord2 = NULL;

	// PDTNext is the next ptr of the merged partition,
	// PDTNext's previous ptr will have to be updated to point
	// to the merged partition
	StorageRollCallRecord			*pSRCNext = NULL;
	StorageRollCallRecord			*pSRCPrev = NULL;
	BOOL							changeNext = false;
	BOOL							changePrev = false;
	U32								numberOfContiguousPartitionsRemaining = 0;

	PARTITION_DESCRIPTOR			*pTempPDTRecord = NULL;

	pCmdInfo			= (PMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pMergePartitionInfo = 
			(PMSTR_MERGE_PARTITION_INFO *)&pCmdParams->mergePartitionInfo;

	pSRCRecord1			= (StorageRollCallRecord *) pCmdContext->pData1;

	pSRCRecord2			= (StorageRollCallRecord *) pCmdContext->pData2;

	pPDTRecord1			= (PARTITION_DESCRIPTOR *) pCmdContext->pData3;

	pPDTRecord2			= (PARTITION_DESCRIPTOR *) pCmdContext->pData4;

	pSRCNext			= (StorageRollCallRecord *) pCmdContext->pData5;
	pSRCPrev			= (StorageRollCallRecord *) pCmdContext->pData6;
	changeNext			= pCmdContext->value1;
	changePrev			= pCmdContext->value2;
	numberOfContiguousPartitionsRemaining = pCmdContext->value;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = PMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){
		case MERGE_PARTITION_PDT1_SIZE_MODIFIED:
			m_pDataQueue->Modify(
							PMSTR_PARTITION, 
							&pPDTRecord1->rid,
							&pPDTRecord1->stateIdentifier,
							pPDTRecord1,
							pPDTRecord1->size);
			// now modify the SRC1's capacity
			pCmdContext->state = MERGE_PARTITION_SRC1_CAPACITY_MODIFIED;
			pSRCRecord1->Capacity = pPDTRecord1->partitionSize;
			status = m_pTableServices->TableServiceModifyField(
							STORAGE_ROLL_CALL_TABLE,
							&pSRCRecord1->rid,		// SRC row id
							fdSRC_CAPACITY,					// field name of field to be modifiied
							&pSRCRecord1->Capacity,
							sizeof(pSRCRecord1->Capacity),
							TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
							pCmdContext);
		break;

		case MERGE_PARTITION_SRC1_CAPACITY_MODIFIED:
			pCmdContext->state = MERGE_PARTITION_PDT_POINTERS_UPDATED;
			// the return value is the remaining number of partitions 
			// of the same parent
			// if this count is 0 we will have to delete the last
			// partition
			pCmdContext->value = UpdatePartitionPointers(
				&pPDTRecord1->parentSRCTRID,
				pPDTRecord1,
				TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
				pCmdContext);
		break;
		

		case MERGE_PARTITION_PDT_POINTERS_UPDATED:
			if (numberOfContiguousPartitionsRemaining == PMSTR_NONE_MODIFIED){
				// now mark our parent SRC as free and delete PDT1 and SRC1
				pCmdContext->state = MERGE_PARTITION_PARENT_SRC_UNCLAIMED;
				m_SRCIsUsed = SRC_FREE;
				status = m_pTableServices->TableServiceModifyField(
									STORAGE_ROLL_CALL_TABLE,
									&pPDTRecord1->parentSRCTRID,	// SRC row id
									fdSRC_FUSED,					// field name of field to be modifiied
									&m_SRCIsUsed,					// set to true
									sizeof(U32),
									TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
									pCmdContext);
			} else {
				if (numberOfContiguousPartitionsRemaining & PMSTR_NEXT_PARTITION_MODIFIED){
						pCmdContext->state = MERGE_PARTITION_NEXT_SRC_RECORD_READ;
						pCmdContext->pData5 = new (tZERO) StorageRollCallRecord;
						status = m_pTableServices->TableServiceReadRow(
										STORAGE_ROLL_CALL_TABLE,
										&pPDTRecord1->nextRowId,
										pCmdContext->pData5,
										sizeof(StorageRollCallRecord),
										TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
										pCmdContext);
				} else {
					if (numberOfContiguousPartitionsRemaining & PMSTR_PREV_PARTITION_MODIFIED){
						// if there is a previous partition
						// we need to modify the next ptr of the previous partition
						// to point it to the merged partition
						pCmdContext->state = MERGE_PARTITION_PREV_SRC_RECORD_READ;
						pCmdContext->pData6 = new (tZERO) StorageRollCallRecord;
						status = m_pTableServices->TableServiceReadRow(
										STORAGE_ROLL_CALL_TABLE,
										&pPDTRecord1->previousRowId,
										pCmdContext->pData6,
										sizeof(StorageRollCallRecord),
										TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
										pCmdContext);
					}
				}
			}
		break;


		case MERGE_PARTITION_PARENT_SRC_UNCLAIMED:
//	if (SimulateFailover(pCmdContext)){
//	return status;
//	} 
			// Resolve: Still need find way to report the delete event to SSAPI,
			// If SSAPI can not recognize the crash, we may need save SRC2 and 
			// PDT2 record to a Table before delete SRC2 in MergePartionReply.
			
			// Now delete the SRC2 record
			pCmdContext->state = MERGE_PARTITION_SRC2_RECORD_DELETED1;
			if (RowId(pSRCRecord2->rid)){
				status = m_pTableServices->TableServiceDeleteRow(
							STORAGE_ROLL_CALL_TABLE,
							&pSRCRecord2->rid,		
							TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
							pCmdContext);
			} else {
				ProcessMergePartitionReply(pCmdContext, OK);
			}
		break;

		case MERGE_PARTITION_SRC2_RECORD_DELETED1:
		
//	if (SimulateFailover(pCmdContext)){
//		return status;
//	} 

			pCmdContext->state = MERGE_PARTITION_SRC1_RECORD_DELETED;
			status = m_pTableServices->TableServiceDeleteRow(
						STORAGE_ROLL_CALL_TABLE,
						&pSRCRecord1->rid,		
						TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
						pCmdContext);
		break;

		case MERGE_PARTITION_SRC1_RECORD_DELETED:
			// If we have a failover at this point then the repeat cmd after 
			// failover will be rejected by validation (since SRCs) is deleted.
			// So PDTs will remain in the system, so we need to do some table 
			// data consistency check at initialization - which should deleted
			// PDT entries with no associated SRC entries
			
			// This failover can not be simulated by software.
			
			// Now delete PDT1 and PDT2 in one operation
			pCmdContext->state = MERGE_PARTITION_PDT1_PDT2_RECORD_DELETED;
			status = m_pTableServices->TableServiceDeleteAllMatchedRows(
						PARTITION_DESCRIPTOR_TABLE,
						fdPARENT_RID,
						&pPDTRecord1->parentSRCTRID,// pdt row id
						TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
						pCmdContext);
		break;

		case MERGE_PARTITION_PDT1_PDT2_RECORD_DELETED:
			// this state is reached only if this was the last
			// remaining partition
			m_pDataQueue->Remove(
							PMSTR_PARTITION, 
							&pPDTRecord2->rid);

			m_pDataQueue->Remove(
							PMSTR_PARTITION, 
							&pPDTRecord1->rid);

			cmdComplete = true;
		break;

		case MERGE_PARTITION_NEXT_SRC_RECORD_READ:
			changeNext = pCmdContext->value1 = true;
			pCmdContext->state = MERGE_PARTITION_PREV_SRC_RECORD_READ;
			if (numberOfContiguousPartitionsRemaining & PMSTR_PREV_PARTITION_MODIFIED){
				pCmdContext->pData6 = new (tZERO) StorageRollCallRecord;
				status = m_pTableServices->TableServiceReadRow(
								STORAGE_ROLL_CALL_TABLE,
								&pPDTRecord1->previousRowId,
								pCmdContext->pData6,
								sizeof(StorageRollCallRecord),
								TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
								pCmdContext);
			} 
			else {
				ProcessMergePartitionReply(pCmdContext, OK);
			}
		break;

		case MERGE_PARTITION_PREV_SRC_RECORD_READ:
		
//	if (SimulateFailover(pCmdContext)){
//		return status;
//	} 
		
			// Now delete the SRC2
			changePrev = pCmdContext->value2 = true;
			pCmdContext->state = MERGE_PARTITION_SRC2_RECORD_DELETED2;
			if (RowId(pSRCRecord2->rid)){
				status = m_pTableServices->TableServiceDeleteRow(
							STORAGE_ROLL_CALL_TABLE,
							&pSRCRecord2->rid,		
							TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
							pCmdContext);
			} else {
				ProcessMergePartitionReply(pCmdContext, OK);
			}
		break;

		case MERGE_PARTITION_SRC2_RECORD_DELETED2:
			// Failover is OK here, since we don't need delete SRC1 and PDT1 for this
			// case and deleting PDT2 will be handled in Initialization. 
			// Resolve: We need find way (if can not get SRC2 records from that Table, 
			// since we did not save them before failover for this case)to reject this 
			// cmd, since it passed validation for SRC1 and SRC2 and does not neet to 
			// call CleanUp.
	
			// Now delete the PDT2 record
			pCmdContext->state = MERGE_PARTITION_PDT2_RECORD_DELETED;
			if (pPDTRecord2){
				status = m_pTableServices->TableServiceDeleteRow(
							PARTITION_DESCRIPTOR_TABLE,
							&pPDTRecord2->rid,		// pdt row id
							TSCALLBACK(DdmPartitionMstr,ProcessMergePartitionReply),
							pCmdContext);
			} else {
				ProcessMergePartitionReply(pCmdContext, OK);
			}
		break;


		case MERGE_PARTITION_PDT2_RECORD_DELETED:
			if (RowId(pPDTRecord2->rid)){
				m_pDataQueue->Remove(
								PMSTR_PARTITION, 
								&pPDTRecord2->rid);
			}

			
			cmdComplete = true;
		break;
			
		default:
		break;
		}
	}
	if (cmdComplete){
#if 0
		if (SimulateFailover(pCmdContext)){
			return status;
		} 
#endif

		// Report the status of the merge partition back 
		m_pCmdServer->csrvReportCmdStatus(
				pCmdContext->cmdHandle,		// handle
				rc,							// completion code
				NULL,						// result Data
				(void *)pCmdInfo);			// Orig cmd info
		StopCommandProcessing(
				true, 
				pCmdContext->cmdHandle);


		// if no error then report the event
		if (rc == PMSTR_SUCCESS){
			// first generate a partition deleted event
			if (RowId(pSRCRecord2->rid) && RowId(pPDTRecord2->rid)){
				PMSTR_EVT_PARTITION_DELETED_STATUS *pEvtPartitionDeleted = 
							new (tZERO) PMSTR_EVT_PARTITION_DELETED_STATUS;
				pEvtPartitionDeleted->SRCData = *pSRCRecord2;
				//pEvtPartitionDeleted->partitionData = *pPDTRecord2;
				m_pCmdServer->csrvReportEvent(
					PMSTR_EVT_PARTITION_DELETED,	// completion code
					pEvtPartitionDeleted);			// event Data
				delete pEvtPartitionDeleted;
			}
			// first generate a partition deleted event
			if (RowId(pSRCRecord2->rid) && RowId(pPDTRecord2->rid)){
				PMSTR_EVT_PARTITION_DELETED_STATUS *pEvtPartitionDeleted = 
							new (tZERO) PMSTR_EVT_PARTITION_DELETED_STATUS;
				pEvtPartitionDeleted->SRCData = *pSRCRecord2;
				pEvtPartitionDeleted->partitionData = *pPDTRecord2;
				m_pCmdServer->csrvReportEvent(
					PMSTR_EVT_PARTITION_DELETED,	// completion code
					pEvtPartitionDeleted);			// event Data
				delete pEvtPartitionDeleted;
			}

			// Now generate event for partition modified
			PMSTR_EVT_PARTITION_MODIFIED_STATUS *pEvtPartitionModified = 
						new (tZERO) PMSTR_EVT_PARTITION_MODIFIED_STATUS;
			pEvtPartitionModified->SRCData = *pSRCRecord1;
			pEvtPartitionModified->partitionData = *pPDTRecord1;
			m_pCmdServer->csrvReportEvent(
					PMSTR_EVT_PARTITION_MODIFIED,	// completion code
					pEvtPartitionModified);			// event Data

			// if this was the last partition, then report deleted event
			if (numberOfContiguousPartitionsRemaining == 0){
				m_pCmdServer->csrvReportEvent(
					PMSTR_EVT_PARTITION_DELETED,	// completion code
					pEvtPartitionModified);			// event Data
			} else {
				if (changeNext){
					if ((RowId(pPDTRecord1->nextRowId) != 0)){
						// generate event for modify of the next partition
						// whose prev ptr is set to the newly merged partition
						pEvtPartitionModified->SRCData = *pSRCNext;
						m_pDataQueue->Get(
								PMSTR_PARTITION, 
								&pSRCNext->ridDescriptorRecord,
								(void **)&pTempPDTRecord,
								sizeof(PARTITION_DESCRIPTOR));
						pEvtPartitionModified->partitionData = *pTempPDTRecord;
						m_pCmdServer->csrvReportEvent(
							PMSTR_EVT_PARTITION_MODIFIED,	// completion code
							pEvtPartitionModified);			// event Data
					}
				}
				if (changePrev) {
					if ((RowId(pPDTRecord1->previousRowId) != 0)){
						// generate event for modify of the next partition
						// whose prev ptr is set to the newly merged partition
						pEvtPartitionModified->SRCData = *pSRCPrev;
						m_pDataQueue->Get(
								PMSTR_PARTITION, 
								&pSRCPrev->ridDescriptorRecord,
								(void **)&pTempPDTRecord,
								sizeof(PARTITION_DESCRIPTOR));
						pEvtPartitionModified->partitionData = *pTempPDTRecord;
						m_pCmdServer->csrvReportEvent(
							PMSTR_EVT_PARTITION_MODIFIED,	// completion code
							pEvtPartitionModified);			// event Data
					}
				}
			}
			delete pEvtPartitionModified;
		}

		if (pCmdContext) {
			delete pCmdContext;
			pCmdContext = NULL;
		}

	}
	return status;
}














