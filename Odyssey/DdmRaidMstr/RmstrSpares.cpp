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
// File: RmstrSpares.cpp
// 
// Description:
// Implementation for raid spare operations
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrSpares.cpp $
// 
// 20    9/09/99 1:38p Dpatel
// removed ENABLE_LOGGING ifdef...
// 
// 19    9/07/99 7:39p Dpatel
// 
// 18    9/07/99 7:32p Dpatel
// 
// 17    9/01/99 6:38p Dpatel
// added logging and alarm code..
// 
// 16    8/27/99 5:24p Dpatel
// added event code..
// 
// 15    8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 14    8/14/99 1:37p Dpatel
// Added event logging..
// 
// 13    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 12    8/03/99 5:25p Dpatel
// Removed the service method to modify SRC, used table services..
// 
// 11    8/02/99 3:18p Jtaylor
// fixed warnings
// 
// 10    7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 9     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 8     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 7     7/17/99 1:20p Dpatel
// Queued up commands.
// 
// 6     7/16/99 10:28a Dpatel
// Added DownMember and Commit Spare code. Also removed the reads for
// validation.
// 
// 5     7/09/99 5:26p Dpatel
// 
// 4     7/06/99 4:56p Dpatel
// Added flag for Updating the SRC "fUsed" field.
// 
// 3     6/30/99 11:15a Dpatel
// Changes for Abort Util and Chg Priority.
// 
// 2     6/28/99 5:16p Dpatel
// Implemented new methods, changed headers.
// 
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/


#include "DdmRaidMgmt.h"

// Create Spare States
enum {
	CREATESPARE_SDRECORD_INSERTED = 1,
	CREATESPARE_SPARE_NAME_READ,
	CREATESPARE_SPARE_CLAIMED,
	CREATESPARE_SPARE_INFO_UPDATED_IN_ADT,
	CREATESPARE_ARRAY_NAME_READ,
};

// Create Spare Validation States
enum {
	CREATE_SPARE_VALIDATION_SRCT_RECORD_READ = 100
};



//************************************************************************
//	rmstrCreateTheSpare
//		Create a Dedicated, Pool or Host Pool spare
//
//	handle		- Handle for the cmd
//	_pCmdInfo	- cmd packet for create spare
//	_pADTRecord	- the array to which the dedicated spare is to be
//					added. NULL for other spares
//	_pSRCRecord	- the SRC record for the new spare
//
//************************************************************************
STATUS DdmRAIDMstr::
rmstrCreateTheSpare(
			HANDLE							handle,
			RMSTR_CMND_INFO					*_pCmdInfo,
			RAID_ARRAY_DESCRIPTOR			*_pADTRecord,
			StorageRollCallRecord			*_pSRCRecord)
{
	STATUS							status = RMSTR_SUCCESS;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_CREATE_SPARE_INFO			*pCreateSpareInfo = NULL;
	CONTEXT							*pCmdContext = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;

	STATE_IDENTIFIER				stateIdentifier;

	pCmdContext = new CONTEXT;
	pCmdContext->cmdHandle		= handle;

	// save the cmd info into our context
	pCmdContext->pData			= new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));
	if (_pADTRecord){
		// NULL for POOL SPARES
		pCmdContext->pData1			= new RAID_ARRAY_DESCRIPTOR;
		memcpy(pCmdContext->pData1,_pADTRecord,sizeof(RAID_ARRAY_DESCRIPTOR));
	}
	pCmdContext->pData2			= new StorageRollCallRecord;
	memcpy(pCmdContext->pData2,_pSRCRecord,sizeof(StorageRollCallRecord));

	pCmdInfo				= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pCreateSpareInfo		=
		(RMSTR_CREATE_SPARE_INFO *)&pCmdParams->createSpareInfo;

	pSRCRecord	= (StorageRollCallRecord *)pCmdContext->pData2;

	SetStateIdentifier( 
		&stateIdentifier,
		pCmdInfo->opcode,
		(rowID *)pCmdContext->cmdHandle,
		CREATESPARE_SDRECORD_INSERTED,
		pCmdContext->numProcessed);

	pCmdContext->state = CREATESPARE_SDRECORD_INSERTED;
	// deletes the pData2 and uses pData2 for Spare descriptor
	status = rmstrInsertSpareDescriptor(
					&stateIdentifier,
					pCreateSpareInfo->spareType,
					&pCreateSpareInfo->arrayRowId,
					&pCreateSpareInfo->spareId,
					&pCreateSpareInfo->hostId,
					pSRCRecord->Capacity,
					pSRCRecord->vdnBSADdm,
					TSCALLBACK(DdmRAIDMstr,rmstrProcessCreateSpareReply),
					pCmdContext);

	return status;
}	


//************************************************************************
//	rmstrProcessCreateSpareReply
//		Insert a new Spare Descriptor record
//		Mark the SRC as USED
//		Update the ADT (for Dedicated Spares)
//	Generate an event for the new spare added. Also check if any array is
//	critical and if we can use this spare to regenerate.
//
//************************************************************************
STATUS DdmRAIDMstr::
rmstrProcessCreateSpareReply(void *_pContext, STATUS status)
{
	CONTEXT							*pCmdContext = (CONTEXT *)_pContext;
	STATUS							rc;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_SPARE_DESCRIPTOR			*pSpare = NULL;
	RMSTR_CREATE_SPARE_INFO			*pCreateSpareInfo = NULL;
	BOOL							cmdComplete = false;
	RMSTR_EVT_SPARE_ADDED_STATUS	*pEvtSpareAdded = NULL;

	rc = RMSTR_SUCCESS;

	// pCmdContext->pData = cmdInfo
	// pCmdContext->pData1 = ADT Record (for Dedicated Spares only, else NULL)
	// pCmdContext->pData2 = spare info
	pCmdInfo		= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pCreateSpareInfo = 
		(RMSTR_CREATE_SPARE_INFO *)&pCmdParams->createSpareInfo;
	pSpare = (RAID_SPARE_DESCRIPTOR *)pCmdContext->pData2;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){
		case CREATESPARE_SDRECORD_INSERTED:
			AddRmstrData(RAID_SPARE, &pSpare->thisRID, pSpare);		

			// read the spare name
			pCmdContext->state = CREATESPARE_SPARE_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
				&pCmdContext->ucMemberName,
				&pCmdContext->SlotID,
				&pSpare->SRCTRID,
				TSCALLBACK(DdmRAIDMstr,rmstrProcessCreateSpareReply),
				pCmdContext);
			break;

		case CREATESPARE_SPARE_NAME_READ:
			// modify the SRC entry to say member is used.
			pCmdContext->state = CREATESPARE_SPARE_CLAIMED;
			m_SRCIsUsed = SRC_USED;
			status = m_pTableServices->TableServiceModifyField(
							STORAGE_ROLL_CALL_TABLE,
							&pCreateSpareInfo->spareId,		// SRC row id
							fdSRC_FUSED,			// field name of field to be modifiied
							&m_SRCIsUsed,			// set to false
							sizeof(U32),
							TSCALLBACK(DdmRAIDMstr,rmstrProcessCreateSpareReply),
							pCmdContext);
			break;

		case CREATESPARE_SPARE_CLAIMED:
			if (pCreateSpareInfo->spareType == RAID_DEDICATED_SPARE){
				pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;
				// modify the ADT to fill in the RID of new member/spare
				// will go to ADT_UPDATED					
				pCmdContext->state = CREATESPARE_SPARE_INFO_UPDATED_IN_ADT;
				pADTRecord->spares[pADTRecord->numberSpares] = 
						pSpare->thisRID;
				pADTRecord->numberSpares++;

				SetStateIdentifier(
						&pADTRecord->stateIdentifier,
						pCmdInfo->opcode,
						(rowID *)pCmdContext->cmdHandle,
						CREATESPARE_SPARE_INFO_UPDATED_IN_ADT,
						0);

				status = CheckAndModifyRow(
							RAID_ARRAY,
							&pADTRecord->stateIdentifier,
							RAID_ARRAY_DESCRIPTOR_TABLE,
							&pADTRecord->thisRID,	// row id to modify
							pADTRecord,
							sizeof(RAID_ARRAY_DESCRIPTOR),
							&pADTRecord->thisRID,
							TSCALLBACK(DdmRAIDMstr,rmstrProcessCreateSpareReply),
							pCmdContext);
			} else {
				cmdComplete = true;
			}
			break;

		case CREATESPARE_SPARE_INFO_UPDATED_IN_ADT:
			// set the pADT Record
			pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;
			ModifyRmstrData(
						RAID_ARRAY, 
						&pADTRecord->thisRID,
						pADTRecord);
			// read the array name
			pCmdContext->state = CREATESPARE_ARRAY_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
				&pCmdContext->ucArrayName,
				NULL,
				&pADTRecord->SRCTRID,
				TSCALLBACK(DdmRAIDMstr,rmstrProcessCreateSpareReply),
				pCmdContext);
			break;

		case CREATESPARE_ARRAY_NAME_READ:
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
		// Report the status of the create spare back 
		m_pCmdServer->csrvReportCmdStatus(
				pCmdContext->cmdHandle,		// handle
				rc,							// completion code
				NULL,						// result Data
				(void *)pCmdInfo);			// Orig cmd info
		if (rc == RMSTR_SUCCESS) {
			// Generate event for Spare Added
			pEvtSpareAdded = new (tZERO) RMSTR_EVT_SPARE_ADDED_STATUS;

			pEvtSpareAdded->spareData = *pSpare;
			m_pCmdServer->csrvReportEvent(
					RMSTR_EVT_SPARE_ADDED,		// completion code
					pEvtSpareAdded);			// event Data
			delete pEvtSpareAdded;
			pEvtSpareAdded = NULL;

			LogSpareAddedEvent(pCmdContext,pSpare);

				// Check array state to commit spare, if critical spare is committed
			BOOL isSpareCommited = CheckForAllCriticalArrays(pSpare);
			if (isSpareCommited == false){
				// Should check for next command
				StopCommandProcessing(true, pCmdContext->cmdHandle);
			} else {
				// spare is committed, so dont start next cmd immediately
				StopCommandProcessing(false, pCmdContext->cmdHandle);
			}
		} else {
			StopCommandProcessing(true, pCmdContext->cmdHandle);
		}
		if (pCmdContext) {
			delete pCmdContext;
			pCmdContext = NULL;
		}
	}
	return status;
}


//************************************************************************
//	rmstrInsertSpareDescriptor
//		Insert a Spare Descriptor Record
//		pData2 will be used for the spare data
//
//************************************************************************
STATUS DdmRAIDMstr::
rmstrInsertSpareDescriptor(
			STATE_IDENTIFIER	*pStateIdentifier,
			RAID_SPARE_TYPE		spareType,
			rowID				*pArrayRID,
			rowID				*pSRCTRID,
			rowID				*pHostRID,
			I64					capacity,
			VDN					bsaVdn,
			pTSCallback_t		cb,
			CONTEXT				*pCmdContext)
{
	STATUS			status = 0;

	// delete the raid array member/spare info 
	if (pCmdContext->pData2){
		delete pCmdContext->pData2;
		pCmdContext->pData2 = NULL;
	}

	RAID_SPARE_DESCRIPTOR *pSpare = new RAID_SPARE_DESCRIPTOR;
	pCmdContext->pData2 = pSpare;

	pSpare->version = RAID_SPARE_DESCRIPTOR_TABLE_VERSION;
	pSpare->size = sizeof(RAID_SPARE_DESCRIPTOR);
	// Fill in the spare information
	pSpare->arrayRID = *pArrayRID;
	pSpare->SRCTRID = *pSRCTRID;
	if (pHostRID){
		// can be null if dedicated or system pool spare
		pSpare->hostRID = *pHostRID;
	}
	pSpare->spareType = spareType;
	pSpare->capacity = capacity;
	pSpare->bsaVdn = bsaVdn;

	// copy the state identfier for the spare insert state
	pSpare->stateIdentifier = *pStateIdentifier;

	// if state was already processed, 
	// pSpare will be filled with last data from PTS
	status = CheckAndInsertRow(
				RAID_SPARE,
				&pSpare->stateIdentifier,
				RAID_SPARE_DESCRIPTOR_TABLE,
				pSpare,
				sizeof(RAID_SPARE_DESCRIPTOR),
				&pSpare->thisRID,
				cb,
				pCmdContext);
	return status;
}

//************************************************************************
//	rmstrCreateSpareValidation
//		Validate the new spare by checking size and max spares for Dedicated
//		spares. For pool spares, just add the spare.
//
//************************************************************************
STATUS DdmRAIDMstr::
rmstrCreateSpareValidation(HANDLE h, RMSTR_CMND_INFO *_pCmdInfo)
{
	STATUS							status = RMSTR_SUCCESS;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_CREATE_SPARE_INFO			*pCreateSpareInfo = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;


	CONTEXT	*pValidationContext			= new CONTEXT;

	// save the Cmd Info and the handle
	pValidationContext->cmdHandle	= h;
	pValidationContext->pData		= new RMSTR_CMND_INFO;
	memcpy(pValidationContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));


	pCmdInfo			= (RMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pCreateSpareInfo	= 
		(RMSTR_CREATE_SPARE_INFO *)&pCmdParams->createSpareInfo;

	// first read the SRC entry to make sure Element is not used
	pValidationContext->pData2 = new (tZERO) StorageRollCallRecord;

	pValidationContext->state = CREATE_SPARE_VALIDATION_SRCT_RECORD_READ;
	// read SRCT Record and check size
	status = m_pTableServices->TableServiceReadRow(
			STORAGE_ROLL_CALL_TABLE,
			&pCreateSpareInfo->spareId,
			pValidationContext->pData2,
			sizeof(StorageRollCallRecord),
			TSCALLBACK(DdmRAIDMstr,rmstrProcessCreateSpareValidationReply),
			pValidationContext);
	return status;
}

//************************************************************************
//	rmstrProcessCreateSpareValidationReply
//		Validate the new spare by checking size and max spares for Dedicated
//		spares. For pool spares, just add the spare.
//
//************************************************************************
STATUS DdmRAIDMstr
::rmstrProcessCreateSpareValidationReply(void *_pContext, STATUS status)
{
	CONTEXT							*pValidationContext=NULL;
	RMSTR_CMND_INFO					*pCmdInfo=NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	STATUS							rc;
	RMSTR_CREATE_SPARE_INFO			*pCreateSpareInfo = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;
	rowID							*pTargetArrayRowId = NULL;
	BOOL							validationComplete = false;

	STATE_IDENTIFIER				stateIdentifier;
	BOOL							stateProcessed = false;
	RAID_SPARE_DESCRIPTOR			*pFailoverSpare = NULL;

	rc					= RMSTR_SUCCESS;
	pValidationContext	= (CONTEXT *)_pContext;

	// pValidationContext->pData = cmdInfo
	// pValidationContext->pData1 = ADT Row data
	// pData2 = SRC row data
	pCmdInfo		= (RMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pCreateSpareInfo = 
		(RMSTR_CREATE_SPARE_INFO *)&pCmdParams->createSpareInfo;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		validationComplete = true;
	} else {
		pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pValidationContext->pData1;
		switch(pValidationContext->state){
		case CREATE_SPARE_VALIDATION_SRCT_RECORD_READ:
			// pData2 is SRC Record
			pSRCRecord = (StorageRollCallRecord *)pValidationContext->pData2;

			// first check if failover case..
			stateIdentifier.cmdOpcode	= pCmdInfo->opcode;
			stateIdentifier.cmdRowId	= *(rowID *)pValidationContext->cmdHandle;
			stateIdentifier.cmdState	= CREATESPARE_SDRECORD_INSERTED;
			stateIdentifier.index		= 0;
			pFailoverSpare = new RAID_SPARE_DESCRIPTOR;
			stateProcessed = CheckIfStateAlreadyProcessed(
										RAID_SPARE,
										&stateIdentifier,
										(void *)pFailoverSpare);
			GetRmstrData(
				RAID_ARRAY,
				&pFailoverSpare->arrayRID,
				(void **)&pADTRecord);
			delete pFailoverSpare;

			if (stateProcessed){
				// proceed directly to creation of spare
				rmstrCreateTheSpare(
							pValidationContext->cmdHandle,
							pCmdInfo,
							pADTRecord,
							pSRCRecord);
				validationComplete = true;
			} else {
				switch(pCreateSpareInfo->spareType){
					case RAID_DEDICATED_SPARE:
						if (pSRCRecord->fUsed){	
							rc = RMSTR_ERR_STORAGE_ELEMENT_IN_USE;
						} else {
							// get the array row id
							pCmdParams		= &pCmdInfo->cmdParams;
							pCreateSpareInfo = 
									(RMSTR_CREATE_SPARE_INFO *)&pCmdParams->createSpareInfo;
							pTargetArrayRowId = &pCreateSpareInfo->arrayRowId;

							GetRmstrData(
									RAID_ARRAY,
									pTargetArrayRowId,
									(void **)&pADTRecord);
							if (pADTRecord){
								if (pADTRecord->numberSpares >= MAX_ARRAY_SPARES){
									rc = RMSTR_ERR_MAX_SPARES_ALREADY_CREATED;
									validationComplete = true;
								} 
								if (!rc){
									// pData2 is SRC Record
									pSRCRecord = (StorageRollCallRecord *)pValidationContext->pData2;
									rc = rmstrServiceCheckSize(
												false,			// check for spare
												pSRCRecord,
												pADTRecord->memberCapacity);
									if (!rc){
										rmstrCreateTheSpare(
											pValidationContext->cmdHandle,
											pCmdInfo,
											pADTRecord,
											pSRCRecord);
									}
								}
							}						
						}
						validationComplete = true;
						break;

					case RAID_HOST_POOL_SPARE:
					case RAID_GENERAL_POOL_SPARE:
						// no need to check size, just check if not used and proceed for
						// creation
						if (pSRCRecord->fUsed){	
							rc = RMSTR_ERR_STORAGE_ELEMENT_IN_USE;
						} else {
							rmstrCreateTheSpare(
								pValidationContext->cmdHandle,
								pCmdInfo,
								NULL,
								pSRCRecord);
						}
						validationComplete = true;
						break;
				}	// end of switch spare type	
			} // stateProcessed
			break;	// end of SRCT record read

		default:
			rc = RMSTR_ERR_INVALID_COMMAND;
			validationComplete = true;
			break;
		}
	}
	if (validationComplete){
		if(rc){
			// Report error 
			m_pCmdServer->csrvReportCmdStatus(
				pValidationContext->cmdHandle,	// handle
				rc,								// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCmdInfo
			StopCommandProcessing(true, pValidationContext->cmdHandle);
		}		
		delete pValidationContext;
		pValidationContext = NULL;
	}
	return rc;
}



//************************************************************************
//	CheckForAllCriticalArrays
//		Check for all array's that are critical
//		Commit the spare to the first valid array with the highest pecking order
//
//************************************************************************
BOOL DdmRAIDMstr::
CheckForAllCriticalArrays(
		RAID_SPARE_DESCRIPTOR			*pSpare)
{
	RAID_ARRAY_DESCRIPTOR	*pADTRecord = NULL;
	RAID_ARRAY_MEMBER		*pMember = NULL;
	BOOL					isArrayCritical = false;
	RAID_UTIL_POLICIES		policy;
	BOOL					isSpareCommited = false;

	memset(&policy,0,sizeof(RAID_UTIL_POLICIES));

	// Traverse all arrays in the system
	TraverseRmstrData(
			RAID_ARRAY,
			NULL,				// get first entry
			(void **)&pADTRecord);
	while (pADTRecord != NULL){
		isArrayCritical = CheckIfArrayCritical(
								pADTRecord,
								false);		
		if (isArrayCritical){
			FindFirstDownMember(pADTRecord,(void **)&pMember);
			if (!CheckIfUtilAlreadyRunning(
								RAID_UTIL_REGENERATE,
								policy,
								1,
								&pMember->thisRID,
								pADTRecord)){
				isSpareCommited = true;
				StartInternalCommitSpare(pSpare, pADTRecord, pMember);
				break;
			}
		}
		TraverseRmstrData(
			RAID_ARRAY,
			&pADTRecord->thisRID,	// get next array
			(void **)&pADTRecord);
	}
	return isSpareCommited;
}


//************************************************************************
//	FindFirstDownMember
//
//	pADTRecord		- the array whose down member is to be found
//	ppMember		- where the down member data is to be returned
//					(this data should not be deleted)
//
//************************************************************************
void DdmRAIDMstr::
FindFirstDownMember(
		RAID_ARRAY_DESCRIPTOR			*pADTRecord,
		void							**ppMember)
{
	U32						i=0;
	RAID_ARRAY_MEMBER		*pRowData = NULL;

	switch(pADTRecord->raidLevel){
		case RAID0:
			break;

		case RAID1:
			for (i=0; i < pADTRecord->numberMembers; i++){
				GetRmstrData(
					RAID_MEMBER,
					&pADTRecord->members[i],
					(void **)&pRowData);
				if (pRowData){
					if (pRowData->memberHealth == RAID_STATUS_DOWN){
						*ppMember = pRowData;
						break;
					}
				}
			}
			break;

		case RAID5:
			assert(0);
			break;

		default:
			assert(0);
			break;
	}
}


