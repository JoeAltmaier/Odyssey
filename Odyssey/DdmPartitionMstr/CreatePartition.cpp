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
// File: PmstrCreatePartition.cpp
// 
// Description:
// Implementation for create partition
// 
// $Log: /Gemini/Odyssey/DdmPartitionMstr/CreatePartition.cpp $
// 
// 13    2/08/00 4:46p Szhang
// CreateVirtual device uses HDM_PART
// 
// 12    1/20/00 11:34a Szhang
// Fixed bugs for Window version
// 
// 11    1/14/00 10:24a Dpatel
// 
// 1     9/15/99 4:00p Dpatel
// Initial creation
// 
//
/*************************************************************************/


#include "DdmPartitionMstr.h"


// Create partition States
enum
{	
	CREATE_PARTITION_SRC_RECORD_READ = 1,	
	CREATE_PARTITION_PDT_RECORD_INSERTED,
	CREATE_PARTITION_NEW_SRC_RECORD_INSERTED,
	CREATE_PARTITION_VD_CREATED,
	CREATE_PARTITION_PDT_SRC_RECORD_UPDATED,
	CREATE_PARTITION_NEW_SRC_RECORD_UPDATED,	
	CREATE_PARTITION_SRC_CLAIMED,
	CREATE_PARTITION_PDT_POINTERS_UPDATED,
	CREATE_PARTITION_EXISTING_PDT_RECORD_MODIFIED,
	CREATE_PARTITION_SRC_CAPACITY_MODIFIED,
	CREATE_PARTITION_NEXT_SRC_RECORD_READ,
	CREATE_PARTITION_PREV_SRC_RECORD_READ,
	CREATE_PARTITION_SRC_NAME_READ,
	CREATE_PARTITION_REMAINDER_SRC_NAME_READ
};

// Create Partition Validation states
enum
{
	CREATE_PARTITION_VALIDATION_SRCT_RECORD_READ = 100
};


//************************************************************************
//	CreatePartitionValidation
//		- Check if not used
//		- check if new partition request is not already a partition
//
//	handle		- the handle for the command
//	pCmdInfo	- the cmd info
//
//************************************************************************
STATUS DdmPartitionMstr::
CreatePartitionValidation(HANDLE h, PMSTR_CMND_INFO *_pCmdInfo)
{
	STATUS							status = PMSTR_SUCCESS;
	PMSTR_CREATE_PARTITION_INFO		*pCreatePartitionInfo = NULL;
	PMSTR_CMND_INFO					*pCmdInfo = NULL;
	PMSTR_CMND_PARAMETERS			*pCmdParams = NULL;

	PARTITION_CONTEXT	*pValidationContext	= new PARTITION_CONTEXT;

	// save the CREATE_PARTITION_INFO and the handle
	pValidationContext->cmdHandle	= h;

	pValidationContext->pData	= new(tZERO) PMSTR_CMND_INFO;
	memcpy(pValidationContext->pData, _pCmdInfo, sizeof(PMSTR_CMND_INFO));

	pCmdInfo			= (PMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pCreatePartitionInfo = 
				(PMSTR_CREATE_PARTITION_INFO *)&pCmdParams->createPartitionInfo;

	pValidationContext->state		= CREATE_PARTITION_VALIDATION_SRCT_RECORD_READ;
	// read the src record, Allocate space for read row data
	pValidationContext->pData1 = new(tZERO) StorageRollCallRecord;

	// read SRCT Record 
	status = m_pTableServices->TableServiceReadRow(
				STORAGE_ROLL_CALL_TABLE,
				&pCreatePartitionInfo->srcToPartition,
				pValidationContext->pData1,
				sizeof(StorageRollCallRecord),
				TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionValidationReply),
				pValidationContext);
	return status;
}



//************************************************************************
//	ProcessCreatePartitionValidationReply
//		Create Partition Command Validation Reply Handler
//
//	If any error is found, then the appropriate error code is returned
//	and the command processing is stopped. If validation is successful
//	then proceed for actually creating the error

//	pContext	- the validation context
//	status		- the message status
//
//  Validation State Machine for Create:
//		Read SRC
//			SRC->fUsed?
//			SRC Capacity <= patition size?
//************************************************************************
STATUS DdmPartitionMstr::
ProcessCreatePartitionValidationReply(void *_pContext, STATUS status)
{
	PARTITION_CONTEXT							*pValidationContext = NULL;
	PMSTR_CREATE_PARTITION_INFO		*pCreatePartitionInfo = NULL;
	PMSTR_CMND_INFO					*pCmdInfo = NULL;
	PMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	STATUS							rc;
	StorageRollCallRecord			*pSRCRecord = NULL;
	BOOL							validationComplete = false;


	rc					= PMSTR_SUCCESS;
	pValidationContext	= (PARTITION_CONTEXT *)_pContext;

	// pValidationContext->pData = cmdInfo
	// pData1 = SRC Record
	// pData2 = Partition record (maybe)
	// pData3 = NULL
	pCmdInfo			= (PMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pCreatePartitionInfo = 
			(PMSTR_CREATE_PARTITION_INFO *)&pCmdParams->createPartitionInfo;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = PMSTR_ERR_INVALID_COMMAND;
		validationComplete = true;
	} else {
		switch(pValidationContext->state){
		case CREATE_PARTITION_VALIDATION_SRCT_RECORD_READ:
			pSRCRecord = (StorageRollCallRecord *)pValidationContext->pData1;
			if (pSRCRecord->fUsed){
				rc = PMSTR_ERR_STORAGE_ELEMENT_USED;							
			} 

			if (rc == PMSTR_SUCCESS){
				// check if SRC type is partition, if partitioning
				// a partition, then modify existing partition for size and then
				// add a new partition
				if (pSRCRecord->storageclass == SRCTypePartition){
					// all error checking is done, issue create part of part cmd
					// we will check the size later, since if there is a failover
					// our sizes could have been changed, so we need to take our
					// state identifier into account
					CreatePartitionOfPartition(
						pValidationContext->cmdHandle,
						pCmdInfo,
						pSRCRecord);
				} else {
					I64		srcCapacity = 0;
					I64		partitionSize = 0;
					memcpy(&srcCapacity, &pSRCRecord->Capacity, sizeof(I64));
					memcpy(&partitionSize, &pCreatePartitionInfo->partitionSize, sizeof(I64));					
				
					if (srcCapacity <= partitionSize){
						rc = PMSTR_ERR_INVALID_PARTITION_SIZE;
					}

					// all error checking is done, issue the create partition cmd
					CreatePartition(
						pValidationContext->cmdHandle,
						pCmdInfo,
						pSRCRecord);
				}
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
//	CreatePartition
//		Prepare and insert the partition descriptor
//
//************************************************************************
STATUS DdmPartitionMstr::
CreatePartition(
		HANDLE					handle,
		PMSTR_CMND_INFO			*_pCmdInfo,
		StorageRollCallRecord	*_pSRCRecord)
{
	STATUS							status = 0;
	
	PMSTR_CREATE_PARTITION_INFO		*pCreatePartitionInfo = NULL;
	PMSTR_CMND_INFO					*pCmdInfo = NULL;
	PMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	

	PARTITION_CONTEXT				*pCmdContext = new PARTITION_CONTEXT;

	pCmdContext->cmdHandle = handle;
	// save the info into our context
	pCmdContext->pData = new(tZERO) PMSTR_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(PMSTR_CMND_INFO));
	
	
	pCmdInfo			= (PMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pCreatePartitionInfo = 
			(PMSTR_CREATE_PARTITION_INFO *)&pCmdParams->createPartitionInfo;

	PARTITION_DESCRIPTOR *pPDTRecord = (PARTITION_DESCRIPTOR *)
			new(tZERO) PARTITION_DESCRIPTOR;

	I64 partitionSize = 0;
	memcpy(&partitionSize, &pCreatePartitionInfo->partitionSize, sizeof(I64));
	PreparePartitionDescriptor(
			pPDTRecord,
			PARTITION_DESCRIPTOR_TABLE_VERSION,
			sizeof(PARTITION_DESCRIPTOR),
			0,								// Resolve VDN
			&pCreatePartitionInfo->srcToPartition,
			_pSRCRecord->vdnBSADdm,
			0,								// startLBA is from 0 for 1st part
			partitionSize);

	// context data1 now contains PDT record
	pCmdContext->pData1 = pPDTRecord;
	pCmdContext->state = CREATE_PARTITION_PDT_RECORD_INSERTED;

	// copy SRC data into context pData2 
	pCmdContext->pData2 = new (tZERO) StorageRollCallRecord;
	memcpy(pCmdContext->pData2, _pSRCRecord, sizeof(StorageRollCallRecord));

	m_pDataQueue->SetStateIdentifier(
						&pPDTRecord->stateIdentifier,
						pCmdInfo->opcode,
						(rowID *)handle,
						CREATE_PARTITION_PDT_RECORD_INSERTED,
						pCmdContext->numProcessed);
	
	//m_pTableServices->TableServiceInsertRow(
	m_pDataQueue->CheckAndInsertRow(
				PMSTR_PARTITION,
				&pPDTRecord->stateIdentifier,
				PARTITION_DESCRIPTOR_TABLE,
				pPDTRecord,
				sizeof(PARTITION_DESCRIPTOR),
				&pPDTRecord->rid,
				TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionReply),
				pCmdContext);
	return status;
}



//************************************************************************
//	ProcessCreatePartitionReply
//		Process the different states for create partition
//			Insert Partition Descriptor
//
//	pContext	- our context data for create array messages
//	status		- status of the message
//	State Machine for Create:
//		Insert PDT
//		Insert SRC
//		Create Virtual Device
//		Update PDT & SRC
//		Update new SRC
//		Update PDT's pointers
//		Claime SRC
//		SRC read
//		Remainder read
//************************************************************************
STATUS DdmPartitionMstr::
ProcessCreatePartitionReply(void *_pContext, STATUS status)
{
	PARTITION_CONTEXT							*pCmdContext = (PARTITION_CONTEXT *)_pContext;
	STATUS							rc;
	PMSTR_CREATE_PARTITION_INFO		*pCreatePartitionInfo = NULL;
	PMSTR_CMND_INFO					*pCmdInfo = NULL;
	PMSTR_CMND_PARAMETERS			*pCmdParams = NULL;

	PARTITION_DESCRIPTOR			*pPDTRecord = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;
	StorageRollCallRecord			*pSRCNewPartitionRecord = NULL;
	BOOL							cmdComplete = false;
	PMSTR_EVT_PARTITION_CREATED_STATUS *pEvtPartitionCreated = NULL;
	VirtualDeviceRecord				 *pVDRecord = NULL;
	rowID							tempRowId;

	rc = PMSTR_SUCCESS;


	// pCmdContext->pData = CmdInfo
	// pCmdContext->pData1 = partition descriptor 
	// pData2 = SRC record to be partitioned;
	// pData3 = new SRC entry for partition;
	pCmdInfo			= (PMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pCreatePartitionInfo = 
			(PMSTR_CREATE_PARTITION_INFO *)&pCmdParams->createPartitionInfo;
	pPDTRecord = (PARTITION_DESCRIPTOR *)pCmdContext->pData1;
	pSRCRecord = (StorageRollCallRecord *)pCmdContext->pData2;
	pSRCNewPartitionRecord = (StorageRollCallRecord *)pCmdContext->pData3;
	pEvtPartitionCreated = (PMSTR_EVT_PARTITION_CREATED_STATUS *)pCmdContext->pData4;
	pVDRecord = (VirtualDeviceRecord *)pCmdContext->pData5;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = PMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){
		case CREATE_PARTITION_PDT_RECORD_INSERTED:
			m_pDataQueue->Add(
							PMSTR_PARTITION, 
							&pPDTRecord->rid,
							&pPDTRecord->stateIdentifier,
							pPDTRecord,
							pPDTRecord->size);

			// delete any old data
			if (pCmdContext->pData3){
				delete pCmdContext->pData3;
				pCmdContext->pData3 = NULL;
			}

			// Create a new SRC entry for this partition and insert it
			pSRCNewPartitionRecord = new(tZERO) StorageRollCallRecord;
			pCmdContext->pData3 = pSRCNewPartitionRecord;
			
			I64 partitionSize;
			memcpy(&partitionSize, &pPDTRecord->partitionSize, sizeof(I64));
			PrepareSRCEntryForPartition(
					pSRCNewPartitionRecord,
					0,					// fill vdn once its created
					partitionSize,
					&pPDTRecord->rid,
					(pCmdContext->numProcessed == 0) ? &pCreatePartitionInfo->partitionNameRowId : &pCreatePartitionInfo->remainderPartitionNameRowId);

			// reads existing SRC records for our PDT row id, if found then
			// returns the existing SRC record into our pSRCNewPartitionRecord
			// if not found, then new record is inserted
			pCmdContext->state = CREATE_PARTITION_NEW_SRC_RECORD_INSERTED;
			CheckAndInsertNewSRCEntry(
					&pPDTRecord->rid,
					pSRCNewPartitionRecord,
					TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionReply),
					pCmdContext);
			break;

		case CREATE_PARTITION_NEW_SRC_RECORD_INSERTED:
			// Now modify the PDT record to reflect its SRC entry
			pPDTRecord->SRCTRID = pSRCNewPartitionRecord->rid;

			// obtain a new Virtual Device
			if (pCmdContext->pData5){
				delete pCmdContext->pData5;
				pCmdContext->pData5 = NULL;
			}
			pVDRecord = new VirtualDeviceRecord(
								"HDM_PART",
								IOP_LOCAL,		// primary slot
								IOP_LOCAL,		// sec slot
								true,			// auto instantiate
								RowId(pPDTRecord->rid));

			// We set the HiPart as numProcessed to make it unique
			// since for the same cmd row id we are creating two vd's
			tempRowId = *(rowID *)pCmdContext->cmdHandle;
			tempRowId.HiPart = pCmdContext->numProcessed;
			pVDRecord->ridVDOwnerUse = RowId(tempRowId);
			// set it to pData5
			pCmdContext->pData5 = pVDRecord;

			pCmdContext->state = CREATE_PARTITION_VD_CREATED;
			m_pHelperServices->CreateVirtualDevice(
					pVDRecord,
					TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionReply),
					pCmdContext);
			break;

		case CREATE_PARTITION_VD_CREATED:
			// now modify our pdt record with the VDN 
			pPDTRecord->partitionVD = ((RowId &)pVDRecord->rid).GetRow();

			// modify row since VDN and SRC both need to be updated
			pCmdContext->state = CREATE_PARTITION_PDT_SRC_RECORD_UPDATED;
			m_pTableServices->TableServiceModifyRow(
					PARTITION_DESCRIPTOR_TABLE,
					&pPDTRecord->rid,	// row id to modify
					pPDTRecord,
					sizeof(PARTITION_DESCRIPTOR),
					&pPDTRecord->rid,
					TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionReply),
					pCmdContext);
			break;

		case CREATE_PARTITION_PDT_SRC_RECORD_UPDATED:
			// update our local copy with the src entry and vdn
			m_pDataQueue->Modify(
							PMSTR_PARTITION, 
							&pPDTRecord->rid,
							&pPDTRecord->stateIdentifier,
							pPDTRecord,
							pPDTRecord->size);

			pCmdContext->state = CREATE_PARTITION_NEW_SRC_RECORD_UPDATED;
			// also update the partition src with the new vdn
			pSRCNewPartitionRecord->vdnBSADdm = pPDTRecord->partitionVD;
			status = m_pTableServices->TableServiceModifyField(
							STORAGE_ROLL_CALL_TABLE,
							&pSRCNewPartitionRecord->rid,		// SRC row id
							fdSRC_VDNBSADDM,					// field name of field to be modifiied
							&pSRCNewPartitionRecord->vdnBSADdm,
							sizeof(VDN),
							TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionReply),
							pCmdContext);
			break;

		case CREATE_PARTITION_NEW_SRC_RECORD_UPDATED:
		//  Failover is OK Between creating the new and the remainder, since SRC still can
		//	be read, SRC->fUsed is faulse and SRC->Capasity is not be changed.
			pCmdContext->numProcessed++;
			if (pCmdContext->numProcessed == 1){
				// save info about partition to Generate event later
				pCmdContext->pData4 =
						new(tZERO) PMSTR_EVT_PARTITION_CREATED_STATUS;
				pEvtPartitionCreated = 
					(PMSTR_EVT_PARTITION_CREATED_STATUS *)pCmdContext->pData4;
				pEvtPartitionCreated->SRCData = *pSRCNewPartitionRecord;
				pEvtPartitionCreated->partitionData = *pPDTRecord;
			}

			if (pCmdContext->numProcessed < 2){
				// set the LBA to whereever the previous one left ends
				I64 partitionSize = 0;
				I64 partitionSize2 = 0;
				I64	srcCapacity = 0;
				
				memcpy(&partitionSize, &pPDTRecord->partitionSize, sizeof(I64));
				memcpy(&partitionSize2, &pCreatePartitionInfo->partitionSize, sizeof(I64));
				memcpy(&srcCapacity, &pSRCRecord->Capacity, sizeof(I64));
				pPDTRecord->startLBA = partitionSize;
				// set the size of the next partition
				partitionSize = srcCapacity -
								partitionSize2;
				memcpy(&pPDTRecord->partitionSize,&partitionSize, sizeof(I64));
				
				pCmdContext->state = CREATE_PARTITION_PDT_RECORD_INSERTED;

				m_pDataQueue->SetStateIdentifier(
						&pPDTRecord->stateIdentifier,
						pCmdInfo->opcode,
						(rowID *)pCmdContext->cmdHandle,
						pCmdContext->state,
						pCmdContext->numProcessed);

				//m_pTableServices->TableServiceInsertRow(
				m_pDataQueue->CheckAndInsertRow(
						PMSTR_PARTITION,
						&pPDTRecord->stateIdentifier,
						PARTITION_DESCRIPTOR_TABLE,
						pPDTRecord,
						sizeof(PARTITION_DESCRIPTOR),
						&pPDTRecord->rid,
						TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionReply),
						pCmdContext);
			} else {
				// all inserts updates done, so now update the ptrs
				// this is a harmless update (so okay for failover)
				pCmdContext->state = CREATE_PARTITION_PDT_POINTERS_UPDATED;
				UpdatePartitionPointers(
					&pCreatePartitionInfo->srcToPartition,				
					pPDTRecord,
					TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionReply),
					pCmdContext);
			}
			break;

		case CREATE_PARTITION_PDT_POINTERS_UPDATED:
			// all inserts/updates done, so mark original SRC entry as used
			// Failover is OK after updating pointers
			// if (SimulateFailover(pCmdContext)){
			//	return status;
			// } 

			pCmdContext->state = CREATE_PARTITION_SRC_CLAIMED;
			m_SRCIsUsed = SRC_USED;
			status = m_pTableServices->TableServiceModifyField(
								STORAGE_ROLL_CALL_TABLE,
								&pSRCRecord->rid,			// SRC row id
								fdSRC_FUSED,				// field name of field to be modifiied
								&m_SRCIsUsed,				// set to true
								sizeof(U32),
								TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionReply),
								pCmdContext);
			break;

		case CREATE_PARTITION_SRC_CLAIMED:
		//	Failover is OK after changing SRC_Used to true. It's OK to reject the same
		//	command, since create is done.
			pCmdContext->state = CREATE_PARTITION_SRC_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
				&pCmdContext->ucPartitionName,
				&pCmdContext->SlotID,
				&pEvtPartitionCreated->SRCData.rid,
				TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionReply),
				(void *)pCmdContext);
			break;

		case CREATE_PARTITION_SRC_NAME_READ:
			pCmdContext->state = CREATE_PARTITION_REMAINDER_SRC_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
				&pCmdContext->ucRemainderPartitionName,
				&pCmdContext->SlotID,
				&pSRCNewPartitionRecord->rid,
				TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionReply),
				(void *)pCmdContext);
			break;

		case CREATE_PARTITION_REMAINDER_SRC_NAME_READ:
			cmdComplete = true;
			break;

		default:
			break;
		}
	}
	if (cmdComplete){
		// Report the status of the create Array back 
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
			// Generate event for 1st partition created
			// copy the new partition name.., rest of partition info
			// was added in our state machine itself
			pCmdContext->ucPartitionName.CString(
					pEvtPartitionCreated->partitionName, 
					sizeof(UnicodeString32));
			pEvtPartitionCreated->partitionNameRowId =
				pCreatePartitionInfo->partitionNameRowId;

			// Read our partition from our local copy, if
			// any of the partition pointers have been updated
			PARTITION_DESCRIPTOR	*pTempPDTRecord = NULL;
			m_pDataQueue->Get(
					PMSTR_PARTITION, 
					&pEvtPartitionCreated->partitionData.rid,
					(void **)&pTempPDTRecord,
					sizeof(PARTITION_DESCRIPTOR));
			pEvtPartitionCreated->partitionData = *pTempPDTRecord;

			m_pCmdServer->csrvReportEvent(
					PMSTR_EVT_PARTITION_CREATED,	// completion code
					pEvtPartitionCreated);			// event Data

			// Generate event for 2nd partition created
			memset(pEvtPartitionCreated->partitionName, 0, sizeof(UnicodeString32));
			pCmdContext->ucRemainderPartitionName.CString(
					pEvtPartitionCreated->partitionName, 
					sizeof(UnicodeString32));
			pEvtPartitionCreated->partitionNameRowId = 
				pCreatePartitionInfo->remainderPartitionNameRowId;
			pEvtPartitionCreated->SRCData = *pSRCNewPartitionRecord;
			pEvtPartitionCreated->partitionData = *pPDTRecord;
			m_pCmdServer->csrvReportEvent(
					PMSTR_EVT_PARTITION_CREATED,	// completion code
					pEvtPartitionCreated);			// event Data
		}

		// clean up
		delete pEvtPartitionCreated;

		if (pCmdContext) {
			delete pCmdContext;
			pCmdContext = NULL;
		}

	}
	return status;
}




//************************************************************************
//	CreatePartitionOfPartition
//		Prepare and insert the partition descriptor
//
//************************************************************************
STATUS DdmPartitionMstr::
CreatePartitionOfPartition(
		HANDLE					handle,
		PMSTR_CMND_INFO			*_pCmdInfo,
		StorageRollCallRecord	*_pSRCRecord)
{
	STATUS							status = 0;
	PARTITION_CONTEXT				*pCmdContext = NULL;
	PMSTR_CREATE_PARTITION_INFO		*pCreatePartitionInfo = NULL;
	PMSTR_CMND_INFO					*pCmdInfo = NULL;
	PMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	PARTITION_DESCRIPTOR			*_pPDTRecord = NULL;
	PARTITION_DESCRIPTOR			*pExistingPDTRecord = NULL;
	PARTITION_DESCRIPTOR			*pNewPDTRecord = NULL;

	pCmdContext = new PARTITION_CONTEXT;

	pCmdContext->cmdHandle = handle;
	// save the info into our context
	pCmdContext->pData = new PMSTR_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(PMSTR_CMND_INFO));
	

	pCmdInfo			= (PMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pCreatePartitionInfo = 
			(PMSTR_CREATE_PARTITION_INFO *)&pCmdParams->createPartitionInfo;

	
	// read the existing partition record
	m_pDataQueue->Get(
					PMSTR_PARTITION, 
					&_pSRCRecord->ridDescriptorRecord,
					(void **)&_pPDTRecord,
					sizeof(PARTITION_DESCRIPTOR));

	// if existing PDT record's state identifier's cmd row id is the same
	// as this cmd, then it was a repeat cmd. So ignore validation
	// This can handle the failover after changing the existing partition's capacity
	I64 partitionSize = 0;
	I64	srcCapacity = 0;
	memcpy(&partitionSize, &pCreatePartitionInfo->partitionSize, sizeof(I64));
	memcpy(&srcCapacity, &_pSRCRecord->Capacity, sizeof(I64));
	if ( RowId(_pPDTRecord->stateIdentifier.cmdRowId) !=
		RowId((*(rowID *)handle))){
		if (srcCapacity <= partitionSize){
			status = PMSTR_ERR_INVALID_PARTITION_SIZE;
		}
	}

	if (status){
		ProcessCreatePartitionOfPartitionReply(pCmdContext, status);
		return status;
	}

	// save the existing partition in our context
	pCmdContext->pData1 = new(tZERO) PARTITION_DESCRIPTOR;
	memcpy(pCmdContext->pData1, _pPDTRecord, sizeof(PARTITION_DESCRIPTOR));
	pExistingPDTRecord = (PARTITION_DESCRIPTOR *)pCmdContext->pData1;

	// prepare a new partition entry for insertion
	pCmdContext->pData2 = new(tZERO) PARTITION_DESCRIPTOR;
	pNewPDTRecord = (PARTITION_DESCRIPTOR *)pCmdContext->pData2;

	PreparePartitionDescriptor(
			pNewPDTRecord,
			PARTITION_DESCRIPTOR_TABLE_VERSION,
			sizeof(PARTITION_DESCRIPTOR),
			0,								// Resolve VDN
			&pExistingPDTRecord->parentSRCTRID,
			pExistingPDTRecord->parentVDN,
			pExistingPDTRecord->startLBA,	// startLBA is from start lba of existing part
			partitionSize);	

	// copy SRC data into context pData3
	pCmdContext->pData3 = new (tZERO) StorageRollCallRecord;
	memcpy(pCmdContext->pData3, _pSRCRecord, sizeof(StorageRollCallRecord));

	m_pDataQueue->SetStateIdentifier(
						&pNewPDTRecord->stateIdentifier,
						pCmdInfo->opcode,
						(rowID *)handle,
						CREATE_PARTITION_PDT_RECORD_INSERTED,
						pCmdContext->numProcessed);
	
	//m_pTableServices->TableServiceInsertRow(
	pCmdContext->state = CREATE_PARTITION_PDT_RECORD_INSERTED;
	m_pDataQueue->CheckAndInsertRow(
				PMSTR_PARTITION,
				&pNewPDTRecord->stateIdentifier,
				PARTITION_DESCRIPTOR_TABLE,
				pNewPDTRecord,
				sizeof(PARTITION_DESCRIPTOR),
				&pNewPDTRecord->rid,
				TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
				pCmdContext);
	return status;
}



//************************************************************************
//	ProcessCreatePartitionOfPartitionReply
//		Process the different states for create partition
//			Insert Partition Descriptor
//
//	pContext	- our context data for create array messages
//	status		- status of the message
//		State Machine for PartitionOfPartition:	 
//		Insert PDT
//		Insert SRC
//		Create Virtual Device
//		Update PDT & SRC
//		Update new SRC
//		Modify existing PDT
//		Modify SRC capacity
//		Update PDT's pointers
//		Next SRC read
//		Prev SRC read
//		SRC name read
//************************************************************************
STATUS DdmPartitionMstr::
ProcessCreatePartitionOfPartitionReply(
		void						*_pContext, 
		STATUS						status)
{
	PARTITION_CONTEXT				*pCmdContext = (PARTITION_CONTEXT *)_pContext;
	STATUS							rc;
	PMSTR_CREATE_PARTITION_INFO		*pCreatePartitionInfo = NULL;
	PMSTR_CMND_INFO					*pCmdInfo = NULL;
	PMSTR_CMND_PARAMETERS			*pCmdParams = NULL;

	PARTITION_DESCRIPTOR			*pExistingPDTRecord = NULL;
	PARTITION_DESCRIPTOR			*pNewPDTRecord = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;
	StorageRollCallRecord			*pSRCNewPartitionRecord = NULL;
	BOOL							cmdComplete = false;

	PMSTR_EVT_PARTITION_CREATED_STATUS	*pEvtPartitionCreated = NULL;
	PMSTR_EVT_PARTITION_MODIFIED_STATUS	*pEvtPartitionModified = NULL;

	VirtualDeviceRecord					*pVDRecord = NULL;
	rowID								tempRowId;

	U32									partitionsModified = 0;
	U32									changeNext = false;
	U32									changePrev = false;
	StorageRollCallRecord				*pSRCPrev = NULL;
	StorageRollCallRecord				*pSRCNext = NULL;

	rc = PMSTR_SUCCESS;
	
	
	// pCmdContext->pData = CmdInfo
	// pCmdContext->pData1 = existing partition descriptor 
	// pData2 = new partition record
	// pData3 = SRC record to be partitioned;
	// pData4 = new SRC entry for partition;
	pCmdInfo			= (PMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pCreatePartitionInfo = 
			(PMSTR_CREATE_PARTITION_INFO *)&pCmdParams->createPartitionInfo;
	pExistingPDTRecord = (PARTITION_DESCRIPTOR *)pCmdContext->pData1;
	pNewPDTRecord = (PARTITION_DESCRIPTOR *)pCmdContext->pData2;

	pSRCRecord = (StorageRollCallRecord *)pCmdContext->pData3;
	pSRCNewPartitionRecord = (StorageRollCallRecord *)pCmdContext->pData4;

	
	pVDRecord = (VirtualDeviceRecord *)pCmdContext->pData5;
	pEvtPartitionCreated = (PMSTR_EVT_PARTITION_CREATED_STATUS *)pCmdContext->pData6;

	pSRCNext = (StorageRollCallRecord *)pCmdContext->pData7;
	pSRCPrev = (StorageRollCallRecord *)pCmdContext->pData8;
	partitionsModified = pCmdContext->value;
	changeNext = pCmdContext->value1;
	changePrev = pCmdContext->value2;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = PMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){
		case CREATE_PARTITION_PDT_RECORD_INSERTED:
			m_pDataQueue->Add(
							PMSTR_PARTITION, 
							&pNewPDTRecord->rid,
							&pNewPDTRecord->stateIdentifier,
							pNewPDTRecord,
							pNewPDTRecord->size);
			// delete any old data
			if (pCmdContext->pData4){
				delete pCmdContext->pData4;
				pCmdContext->pData4 = NULL;
			}

			// Create a new SRC entry for this partition and insert it
			pSRCNewPartitionRecord = new(tZERO) StorageRollCallRecord;
			pCmdContext->pData4 = pSRCNewPartitionRecord;
			I64 	partitionSize;
			memcpy(&partitionSize, &pNewPDTRecord->partitionSize, sizeof(I64));
			
			PrepareSRCEntryForPartition(
					pSRCNewPartitionRecord,
					0,					
					partitionSize,
					&pNewPDTRecord->rid,
					(pCmdContext->numProcessed == 0)?(&pCreatePartitionInfo->partitionNameRowId) : NULL);

			// reads existing SRC records for our PDT row id, if found then
			// returns the existing SRC record into our pSRCNewPartitionRecord
			// if not found, then new record is inserted
			pCmdContext->state = CREATE_PARTITION_NEW_SRC_RECORD_INSERTED;
			CheckAndInsertNewSRCEntry(
					&pNewPDTRecord->rid,		// key to check for in the SRC record
					pSRCNewPartitionRecord,
					TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
					pCmdContext);
			break;

		case CREATE_PARTITION_NEW_SRC_RECORD_INSERTED:
			// Now modify the PDT record to reflect its SRC entry
			pNewPDTRecord->SRCTRID = pSRCNewPartitionRecord->rid;

			// obtain a new Virtual Device
			if (pCmdContext->pData5){
				delete pCmdContext->pData5;
				pCmdContext->pData5 = NULL;
			}
			pVDRecord = new VirtualDeviceRecord(
								"HDM_PART",
								IOP_LOCAL,		// primary slot
								IOP_LOCAL,		// sec slot
								true,			// auto instantiate
								RowId(pNewPDTRecord->rid));

			// We set the HiPart as numProcessed to make it unique
			// since for the same cmd row id we are creating two vd's
			tempRowId = *(rowID *)pCmdContext->cmdHandle;
			tempRowId.HiPart = pCmdContext->numProcessed;
			pVDRecord->ridVDOwnerUse = RowId(tempRowId);
			// set it to pData5
			pCmdContext->pData5 = pVDRecord;

			pCmdContext->state = CREATE_PARTITION_VD_CREATED;
			m_pHelperServices->CreateVirtualDevice(
					pVDRecord,
					TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
					pCmdContext);
			break;

		case CREATE_PARTITION_VD_CREATED:
			// now modify our pdt record with the VDN
			pNewPDTRecord->partitionVD = ((RowId &)pVDRecord->rid).GetRow();

			pCmdContext->state = CREATE_PARTITION_PDT_SRC_RECORD_UPDATED;
			m_pTableServices->TableServiceModifyRow(
					PARTITION_DESCRIPTOR_TABLE,
					&pNewPDTRecord->rid,	// row id to modify
					pNewPDTRecord,
					sizeof(PARTITION_DESCRIPTOR),
					&pNewPDTRecord->rid,
					TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
					pCmdContext);
			break;

		case CREATE_PARTITION_PDT_SRC_RECORD_UPDATED:
			// update our local copy with the src entry and vdn
			m_pDataQueue->Modify(
							PMSTR_PARTITION, 
							&pNewPDTRecord->rid,
							&pNewPDTRecord->stateIdentifier,
							pNewPDTRecord,
							pNewPDTRecord->size);

			pCmdContext->state = CREATE_PARTITION_NEW_SRC_RECORD_UPDATED;
			// also update the partition src with the new vdn
			pSRCNewPartitionRecord->vdnBSADdm = pNewPDTRecord->partitionVD;
			status = m_pTableServices->TableServiceModifyField(
							STORAGE_ROLL_CALL_TABLE,
							&pSRCNewPartitionRecord->rid,		// SRC row id
							fdSRC_VDNBSADDM,					// field name of field to be modifiied
							&pSRCNewPartitionRecord->vdnBSADdm,
							sizeof(VDN),
							TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
							pCmdContext);
			break;

		case CREATE_PARTITION_NEW_SRC_RECORD_UPDATED:
			pCmdContext->numProcessed++;
			if (pCmdContext->numProcessed == 1){
				// save info for the event for partition created
				pCmdContext->pData6 =
						new(tZERO) PMSTR_EVT_PARTITION_CREATED_STATUS;
				pEvtPartitionCreated = 
					(PMSTR_EVT_PARTITION_CREATED_STATUS *)pCmdContext->pData6;
				pEvtPartitionCreated->SRCData = *pSRCNewPartitionRecord;
				pEvtPartitionCreated->partitionData = *pNewPDTRecord;
			}

			// Now modify the original partition's size and start lba
			// in the PDT record 

			// set the LBA to whereever the previous one left ends
			partitionSize = 0;
			memcpy(&partitionSize, &pNewPDTRecord->partitionSize, sizeof(I64));
			pExistingPDTRecord->startLBA = 
				partitionSize + pNewPDTRecord->startLBA;

			I64 	partitionSize2;
			memcpy(&partitionSize2, &pExistingPDTRecord->partitionSize, sizeof(I64));
			partitionSize2 = partitionSize2 - partitionSize;
			memcpy(&pExistingPDTRecord->partitionSize, &partitionSize2, sizeof(I64));
			m_pDataQueue->SetStateIdentifier(
								&pExistingPDTRecord->stateIdentifier,
								pCmdInfo->opcode,
								(rowID *)pCmdContext->cmdHandle,
								CREATE_PARTITION_EXISTING_PDT_RECORD_MODIFIED,
								pCmdContext->numProcessed);

			pCmdContext->state = CREATE_PARTITION_EXISTING_PDT_RECORD_MODIFIED;
			m_pDataQueue->CheckAndModifyRow(
						PMSTR_PARTITION,
						&pExistingPDTRecord->stateIdentifier,
						PARTITION_DESCRIPTOR_TABLE,
						&pExistingPDTRecord->rid,
						pExistingPDTRecord,
						sizeof(PARTITION_DESCRIPTOR),
						&pExistingPDTRecord->rid,
						TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
						pCmdContext);
			break;

		case CREATE_PARTITION_EXISTING_PDT_RECORD_MODIFIED:
		// Failover is OK after modifing existing, since the SRC is still can be read,
		// the SRC->fUsed is faulse, and the SRC->Capacity is not be changed.	// update our local copy with the src entry and vdn
			m_pDataQueue->Modify(
							PMSTR_PARTITION, 
							&pExistingPDTRecord->rid,
							&pExistingPDTRecord->stateIdentifier,
							pExistingPDTRecord,
							pExistingPDTRecord->size);

			// now modify the SRC entry of existing partition
			// to change its capacity
			pCmdContext->state = CREATE_PARTITION_SRC_CAPACITY_MODIFIED;
			
			
			I64 srcCapacity;
			memcpy(&srcCapacity, &pSRCRecord->Capacity, sizeof(I64));
			memcpy(&partitionSize2, &pExistingPDTRecord->partitionSize, sizeof(I64));
			srcCapacity = partitionSize2;
			status = m_pTableServices->TableServiceModifyField(
							STORAGE_ROLL_CALL_TABLE,
							&pSRCRecord->rid,		// SRC row id
							fdSRC_CAPACITY,					// field name of field to be modifiied
							&srcCapacity,
							sizeof(srcCapacity),
							TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
							pCmdContext);
			break;

		case CREATE_PARTITION_SRC_CAPACITY_MODIFIED:
		// Failover is handle in Create partition of partition module by comparing 
		// PDTRecord->cmdId with cmdHandle
			pCmdContext->state = CREATE_PARTITION_PDT_POINTERS_UPDATED;
			partitionsModified = pCmdContext->value = UpdatePartitionPointers(
				&pNewPDTRecord->parentSRCTRID,
				pNewPDTRecord,
				TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
				pCmdContext);
			break;

		case CREATE_PARTITION_PDT_POINTERS_UPDATED:
			if (partitionsModified & PMSTR_NEXT_PARTITION_MODIFIED){
				pCmdContext->state = CREATE_PARTITION_NEXT_SRC_RECORD_READ;
				pCmdContext->pData7 = new (tZERO) StorageRollCallRecord;
				status = m_pTableServices->TableServiceReadRow(
										STORAGE_ROLL_CALL_TABLE,
										&pNewPDTRecord->nextRowId,
										pCmdContext->pData7,
										sizeof(StorageRollCallRecord),
										TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
										pCmdContext);
			} else {
				if (partitionsModified & PMSTR_PREV_PARTITION_MODIFIED){
					// if there is a previous partition
					// we need to modify the next ptr of the previous partition
					// to point it to the merged partition
					pCmdContext->state = CREATE_PARTITION_PREV_SRC_RECORD_READ;
					pCmdContext->pData8 = new (tZERO) StorageRollCallRecord;
					status = m_pTableServices->TableServiceReadRow(
										STORAGE_ROLL_CALL_TABLE,
										&pNewPDTRecord->previousRowId,
										pCmdContext->pData8,
										sizeof(StorageRollCallRecord),
										TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
										pCmdContext);
				}
			}
			break;

		case CREATE_PARTITION_NEXT_SRC_RECORD_READ:
			changeNext = pCmdContext->value1 = true;
			if (partitionsModified & PMSTR_PREV_PARTITION_MODIFIED){
				// if there is a previous partition
				// we need to modify the next ptr of the previous partition
				// to point it to the merged partition
				pCmdContext->state = CREATE_PARTITION_PREV_SRC_RECORD_READ;
				pCmdContext->pData8 = new (tZERO) StorageRollCallRecord;
				status = m_pTableServices->TableServiceReadRow(
										STORAGE_ROLL_CALL_TABLE,
										&pNewPDTRecord->previousRowId,
										pCmdContext->pData8,
										sizeof(StorageRollCallRecord),
										TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
										pCmdContext);
			} else {
				pCmdContext->state = CREATE_PARTITION_SRC_NAME_READ;
				m_pHelperServices->ReadStorageElementName(
					&pCmdContext->ucPartitionName,
					&pCmdContext->SlotID,
					&pEvtPartitionCreated->SRCData.rid, 
					TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
					(void *)pCmdContext);
			}

			break;

		case CREATE_PARTITION_PREV_SRC_RECORD_READ:
			changePrev = pCmdContext->value2 = true;
			pCmdContext->state = CREATE_PARTITION_SRC_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
				&pCmdContext->ucPartitionName,
				&pCmdContext->SlotID,
				&pEvtPartitionCreated->SRCData.rid, 
				TSCALLBACK(DdmPartitionMstr,ProcessCreatePartitionOfPartitionReply),
				(void *)pCmdContext);
			break;

		case CREATE_PARTITION_SRC_NAME_READ:
			cmdComplete = true;
			break;

		default:
			break;
		}
	}
	if (cmdComplete){
		if (SimulateFailover(pCmdContext)){
				return status;
		} 

		// Report the status of the create Array back 
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
			// Generate event for partition added
			// copy the new partition name.., rest of partition info
			// was added in our state machine itself
			pCmdContext->ucPartitionName.CString(
					pEvtPartitionCreated->partitionName, 
					sizeof(UnicodeString32));
			pEvtPartitionCreated->partitionNameRowId = 
				pCreatePartitionInfo->partitionNameRowId;
			// Read our partition from our local copy, if
			// any of the partition pointers have been updated
			PARTITION_DESCRIPTOR	*pTempPDTRecord = NULL;
			m_pDataQueue->Get(
					PMSTR_PARTITION, 
					&pEvtPartitionCreated->partitionData.rid,
					(void **)&pTempPDTRecord,
					sizeof(PARTITION_DESCRIPTOR));
			pEvtPartitionCreated->partitionData = *pTempPDTRecord;
			m_pCmdServer->csrvReportEvent(
					PMSTR_EVT_PARTITION_CREATED,	// completion code
					pEvtPartitionCreated);			// event Data

			// Generate event for partition modified
			pEvtPartitionModified = new (tZERO) PMSTR_EVT_PARTITION_MODIFIED_STATUS;
			if (changeNext){
				if ((RowId(pNewPDTRecord->nextRowId) != 0)){
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
			if (changePrev){
				if ((RowId(pNewPDTRecord->previousRowId) != 0)){
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
			delete pEvtPartitionModified;
		}

		// clean up
		delete pEvtPartitionCreated;

		if (pCmdContext) {
			delete pCmdContext;
			pCmdContext = NULL;
		}
	}
	return status;
}





//************************************************************************
//
//		PREPARE SRC ENTRY FOR NEW PARTITION
//
//************************************************************************
void DdmPartitionMstr::
PrepareSRCEntryForPartition(
		StorageRollCallRecord			*pSRCRecord,	// new partition SRC
		VDN								vd,
		I64								capacity,
		rowID							*pDescriptorRowId,
		rowID							*pNameRowId)
{
	pSRCRecord->version = STORAGE_ROLL_CALL_TABLE_VERSION;
	pSRCRecord->size = sizeof(StorageRollCallRecord);

	pSRCRecord->Capacity				= capacity;
	pSRCRecord->storageclass			= SRCTypePartition;
	pSRCRecord->vdnBSADdm				= vd;		
	pSRCRecord->ridDescriptorRecord		= *pDescriptorRowId;
	// name can be null
	if (pNameRowId)
		pSRCRecord->ridName					= *pNameRowId;	
	//pSRCRecord->ridStatusRecord		= Resolve;	
	//pSRCRecord->ridPerformanceRecord	= Resolve;
	//pSRCRecord->vdnMonitor			= Resolve;
}


//************************************************************************
//	PreparePartitionDescriptor
//
//************************************************************************
void DdmPartitionMstr::
PreparePartitionDescriptor(
			PARTITION_DESCRIPTOR	*pPDTRecord,
			U32						version,
			U32						size,
			VDN						vdn,
			rowID					*pSrcToPartition,
			VDN						parentVDN,
			I64						startLBA,
			I64						partitionSize)
{
	pPDTRecord->version = version;
	pPDTRecord->size = size;

	pPDTRecord->parentSRCTRID = *pSrcToPartition;

	// fill in the VD of partition later
	pPDTRecord->partitionVD = vdn;	
	pPDTRecord->parentVDN = parentVDN;	
	pPDTRecord->startLBA = startLBA;		//Resolve: where partition starts
	pPDTRecord->partitionSize = partitionSize;
}






//************************************************************************
//	Update partition pointers
//
//************************************************************************
STATUS DdmPartitionMstr::
UpdatePartitionPointers(
		rowID					*pParentSrcRowId,
		PARTITION_DESCRIPTOR	*pPartitionToCheck,
		pTSCallback_t			cb,
		PARTITION_CONTEXT		*pClientContext)
{
	PARTITION_DESCRIPTOR		*pPDTArray[2];

	// there can be a max of 2 contiguous partitions for
	// the partition which we are checking the startLBA and size

	U32							count = 0;

	PARTITION_DESCRIPTOR		*pPDTRecord = NULL;
	rowID						PDTRowId;
	rowID						*pNextRowId = NULL;

	PARTITION_CONTEXT			*pContext = NULL;

	STATUS						partitionsModified = PMSTR_NONE_MODIFIED;
	I64							partitionSize = 0;
	I64							partitionToCheckSize = 0;

	// prepare a list of all partitions with same parent
	// and contiguous locations
	memset(&PDTRowId, 0, sizeof(rowID));
	memset(pPDTArray, 0, sizeof(pPDTArray));

	m_pDataQueue->Traverse(
					PMSTR_PARTITION,		// type
					NULL,					// start from 1st record
					(void **)&pPDTRecord,	// Return data
					&pNextRowId);			// return key
	while (true){
		if (pNextRowId){
			PDTRowId = *pNextRowId;
			if (pPDTRecord->parentSRCTRID == *pParentSrcRowId){
				memcpy(&partitionSize, &pPDTRecord->partitionSize, sizeof(I64));
				if ((pPDTRecord->startLBA + partitionSize) == pPartitionToCheck->startLBA){
					pPDTRecord->nextRowId = pPartitionToCheck->SRCTRID;
					pPartitionToCheck->previousRowId = pPDTRecord->SRCTRID;
					pPDTArray[count] = new (tZERO) PARTITION_DESCRIPTOR;
					memcpy(pPDTArray[count++], pPDTRecord, sizeof(PARTITION_DESCRIPTOR));
					partitionsModified |= PMSTR_PREV_PARTITION_MODIFIED;
				}
				memcpy(&partitionToCheckSize, &pPartitionToCheck->partitionSize, sizeof(I64));
				if ((pPartitionToCheck->startLBA + partitionToCheckSize) == pPDTRecord->startLBA) {
					pPartitionToCheck->nextRowId = pPDTRecord->SRCTRID;
					pPDTRecord->previousRowId = pPartitionToCheck->SRCTRID;
					pPDTArray[count] = new (tZERO) PARTITION_DESCRIPTOR;
					memcpy(pPDTArray[count++], pPDTRecord, sizeof(PARTITION_DESCRIPTOR));
					partitionsModified |= PMSTR_NEXT_PARTITION_MODIFIED;
				}
				pPDTRecord = NULL;
			}
		} else {
			break;
		}
		m_pDataQueue->Traverse(
					PMSTR_PARTITION,		// type
					&PDTRowId,				// start from 1st record
					(void **)&pPDTRecord,	// Return data
					&pNextRowId);			// return key
	}
	if (count){
		pContext = new PARTITION_CONTEXT;
		pContext->pCallback = cb;
		pContext->pParentContext = pClientContext;


		pContext->pData = new (tZERO) char[sizeof(PARTITION_DESCRIPTOR *) * 2];
		memcpy(pContext->pData, pPDTArray, sizeof(pPDTArray));

		pContext->pData1 = new (tZERO) PARTITION_DESCRIPTOR;
		memcpy(pContext->pData1, pPartitionToCheck, sizeof(PARTITION_DESCRIPTOR));
		pContext->value = count;
		// first modify the partition we were checking for
		m_pTableServices->TableServiceModifyRow(
					PARTITION_DESCRIPTOR_TABLE,
					&pPartitionToCheck->rid,	// row id to modify
					pPartitionToCheck,
					sizeof(PARTITION_DESCRIPTOR),
					&pPartitionToCheck->rid,
					TSCALLBACK(DdmPartitionMstr,ProcessUpdatePartitionPointersReply),
					pContext);
	} else {
		// we need not modify any row
		(this->*cb)(pClientContext, OK);
	}
	return partitionsModified;
}


//************************************************************************
//	Process Update partition pointers reply
//
//************************************************************************
STATUS DdmPartitionMstr::
ProcessUpdatePartitionPointersReply(
		void						*_pContext,
		STATUS						status)
{
	PARTITION_CONTEXT			*pContext = (PARTITION_CONTEXT*)_pContext;
	PARTITION_DESCRIPTOR		*pPDTArray[2];
	BOOL						cmdComplete = false;
	PARTITION_DESCRIPTOR		*pPDTRecord = NULL;
	PARTITION_DESCRIPTOR		*pModifiedPDTRecord = NULL;


	PARTITION_CONTEXT *pOriginalContext = pContext->pParentContext;
	pTSCallback_t cb					= pContext->pCallback;
	U32	count							= pContext->value;
	
	memset(pPDTArray, 0, sizeof(pPDTArray));
	memcpy(pPDTArray, pContext->pData ,sizeof(pPDTArray));
	pModifiedPDTRecord = (PARTITION_DESCRIPTOR *)pContext->pData1;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		cmdComplete = true;
	} else {
		m_pDataQueue->Modify(
					PMSTR_PARTITION, 
					&pModifiedPDTRecord->rid,
					&pModifiedPDTRecord->stateIdentifier,
					pModifiedPDTRecord,
					pModifiedPDTRecord->size);

		if (count){
			pPDTRecord = pPDTArray[count-1];
			m_pTableServices->TableServiceModifyRow(
					PARTITION_DESCRIPTOR_TABLE,
					&pPDTRecord->rid,	// row id to modify
					pPDTRecord,
					sizeof(PARTITION_DESCRIPTOR),
					&pPDTRecord->rid,
					TSCALLBACK(DdmPartitionMstr,ProcessUpdatePartitionPointersReply),
					pContext);
			pContext->value--;
		} else {
			cmdComplete = true;
		}
	}
	if (cmdComplete){
		if (pPDTArray[0]){
			delete pPDTArray[0];
		}
		if (pPDTArray[1]){
			delete pPDTArray[1];
		}

		delete pContext;
		(this->*cb)(pOriginalContext, OK);
	}
	return status;
}



//************************************************************************
//	Check And Insert New SRC entry
//
//************************************************************************
STATUS DdmPartitionMstr::
CheckAndInsertNewSRCEntry(
			rowID					*pRowIdToCompare,
			StorageRollCallRecord	*pNewSRCRecord,
			pTSCallback_t			cb,
			void					*pOriginalContext)
{
	PARTITION_CONTEXT			*pContext = new PARTITION_CONTEXT;

	pContext->pCallback			= cb;
	pContext->pParentContext	= (PARTITION_CONTEXT *)pOriginalContext;

	// save the address of the new src record
	pContext->pData = pNewSRCRecord;


	// First read the existing SRC's to find any SRC entry
	// containing the PDT row id
	TSReadRow *pReadSRCRecord = new TSReadRow;
	STATUS status = pReadSRCRecord->Initialize( 
				this,									// DdmServices *pDdmServices,
				STORAGE_ROLL_CALL_TABLE,				// String64 rgbTableName,
				fdSRC_DESC_RID,							// String64 rgbKeyFieldName,
				pRowIdToCompare,						// key field, descriptor row id
				sizeof(rowID),							// U32 cbKeyFieldValue,
				pContext->pData,						// void *prgbRowDataRet,
				sizeof(StorageRollCallRecord),			// U32 cbRowDataRetMax,
				&pContext->value,						// U32 *pcRowsReadRet,
				(pTSCallback_t) &DdmPartitionMstr::CheckAndInsertNewSRCEntryReply,
				pContext								// void* pContext
			);
	if (status == OK) 
		pReadSRCRecord->Send();

	return status;
}


//************************************************************************
//	Check And Insert New SRC entry Reply
//
//************************************************************************
STATUS DdmPartitionMstr::
CheckAndInsertNewSRCEntryReply(void *_pContext, STATUS status)
{
	PARTITION_CONTEXT					*pContext = (PARTITION_CONTEXT *)_pContext;
	pTSCallback_t						cb;
	PARTITION_CONTEXT					*pOriginalContext = NULL;
	StorageRollCallRecord				*pNewSRCRecord = NULL;

	cb							= pContext->pCallback;
	pOriginalContext			= pContext->pParentContext;
	pNewSRCRecord				= (StorageRollCallRecord *)pContext->pData;

	if (status != OK){
		// we could not read, so insert the record
		status = m_pTableServices->TableServiceInsertRow(
						STORAGE_ROLL_CALL_TABLE,
						pNewSRCRecord,
						sizeof(StorageRollCallRecord),
						&pNewSRCRecord->rid,
						cb,
						pOriginalContext);
	} else {
		// record already exists, 
		pContext->pData = NULL;	// src record, user memory - dont delete
		delete pContext;
		(this->*cb)(pOriginalContext,status);
		return status;
	}
	// if our insert had a bad status
	if (status != OK){
		pContext->pData = NULL;	// src record, user memory - dont delete
		delete pContext;
		(this->*cb)(pOriginalContext,OK);
	}
	return status;
}