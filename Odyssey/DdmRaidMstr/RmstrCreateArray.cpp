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
// File: RmstrCreateArray.cpp
// 
// Description:
// Implementation for raid create array cmd
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrCreateArray.cpp $
// 
// 32    11/23/99 6:47p Dpatel
// capability check for RAID1, and dummyVDN for array creation for win32
// 
// 31    11/23/99 11:09a Dpatel
// Check for Data block sizes
// 
// 30    10/11/99 6:42p Dpatel
// changed name to HDM_RAID
// 
// 28    9/17/99 10:14a Dpatel
// removed the TEST_TABLES define..
// 
// 27    9/09/99 1:38p Dpatel
// removed ENABLE_LOGGING ifdef...
// 
// 26    8/27/99 5:24p Dpatel
// added event code..
// 
// 25    8/24/99 5:13p Dpatel
// added failover code, create array changes for "array name"
// 
// 24    8/24/99 10:43a Dpatel
// generated event after reporting cmd status..
// 
// 23    8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 22    8/14/99 1:37p Dpatel
// Added event logging..
// 
// 21    8/12/99 1:56p Dpatel
// Added Array offline event processing code.
// 
// 20    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 19    8/06/99 2:09p Dpatel
// set SRC version.
// 
// 18    8/05/99 11:07a Dpatel
// internal delete array, fake raid ddm, hot copy auto break, removed
// array name code..
// 
// 17    8/03/99 6:22p Dpatel
// Check on member down, if source member down then change the source
// 
// 16    8/03/99 5:25p Dpatel
// Removed the service method to modify SRC, used table services..
// 
// 15    8/02/99 3:01p Dpatel
// changes to create array, array FT processing...
// 
// 14    7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 13    7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 12    7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 11    7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 10    7/20/99 6:49p Dpatel
// Some bug fixes and changed arrayName in ArrayDescriptor to rowID.
// 
// 9     7/17/99 1:20p Dpatel
// Queued up commands.
// 
// 8     7/16/99 10:28a Dpatel
// Added DownMember and Commit Spare code. Also removed the reads for
// validation.
// 
// 7     7/09/99 5:26p Dpatel
// 
// 6     7/06/99 4:56p Dpatel
// Added flag for Updating the SRC "fUsed" field.
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


// Create Array States
enum
{
	caARRAY_DESCRIPTOR_RECORD_INSERTED = 1,
	caMEMBER_SRC_RECORD_READ,
	caMEMBER_DESCRIPTOR_RECORD_INSERTED,
	caSPARE_SRC_RECORD_READ,
	caSPARE_DESCRIPTOR_RECORD_INSERTED,
	caMEMBER_CLAIMED,
	caSPARE_CLAIMED,
	caMEMBER_INFO_UPDATED_IN_ADT,
	caUTIL_INFO_UPDATED_IN_ADT,
	caARRAY_VD_CREATED,
	caSRC_ENTRY_INSERTED,		// SRC entry for Array inserted in SRCT
	caARRAY_NAME_READ,
	caSRC_INFO_UPDATED_IN_ADT	// when ADT updated with SRC rid
};

// Create Array Validation states
enum
{
	caREAD_VALIDATION_CAPABILITIES = 100,
	caVALIDATION_SRCT_RECORD_READ,
	caVALIDATION_CAPABILITIES_READ,
	caVALIDATION_MEMBER_SIZE_CHECKED
};


//************************************************************************
//	caCreateArrayValidation
//		Perform Create Array Command Validation
//			Check for duplicate names
//			Check for member sizes
//			Check for spare sizes
//
//	If any error is found, then the appropriate error code is returned
//	and the command processing is stopped. If validation is successful
//	then proceed for actually creating the error

//	handle		- the handle for the command
//	pCmdInfo	- the cmd info
//
//************************************************************************
STATUS DdmRAIDMstr::
caCreateArrayValidation(HANDLE h, RMSTR_CMND_INFO *_pCmdInfo)
{
	STATUS							status = RMSTR_SUCCESS;
	STATE_IDENTIFIER				stateIdentifier;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;

	stateIdentifier.cmdOpcode = _pCmdInfo->opcode;
	stateIdentifier.cmdRowId = *(rowID *)h;
	stateIdentifier.cmdState = 	caARRAY_DESCRIPTOR_RECORD_INSERTED;
	stateIdentifier.index = 0;

	pADTRecord = new RAID_ARRAY_DESCRIPTOR;
	BOOL stateProcessed = CheckIfStateAlreadyProcessed(
								RAID_ARRAY,
								&stateIdentifier,
								(void *)pADTRecord);
	delete pADTRecord;

	if (stateProcessed){
		// proceed directly to insertion
		caInsertArrayDescriptorRecord(h,_pCmdInfo);
	} else {
		CONTEXT	*pValidationContext			= new CONTEXT;

		// save the CREATE ARRAY DEFINITION and the handle
		pValidationContext->cmdHandle	= h;

		pValidationContext->pData	= new RMSTR_CMND_INFO;
		memcpy(pValidationContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));

		pValidationContext->state		= caREAD_VALIDATION_CAPABILITIES;
		// perform validation
		caProcessCreateArrayValidationReply(pValidationContext,OK);
	}
	return status;
}



//************************************************************************
//	caCreateArrayValidationReply
//		Create Array Command Validation Reply Handler
//
//	If any error is found, then the appropriate error code is returned
//	and the command processing is stopped. If validation is successful
//	then proceed for actually creating the error

//	pContext	- the validation context
//	status		- the message status
//
//************************************************************************
STATUS DdmRAIDMstr::
caProcessCreateArrayValidationReply(void *_pContext, STATUS status)
{
	CONTEXT							*pValidationContext = NULL;
	RMSTR_CREATE_ARRAY_DEFINITION	*pCreateArrayDefinition = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	STATUS							rc;
	StorageRollCallRecord			*pSRCRecord = NULL;
	BOOL							validationComplete = false;
	RMSTR_CAPABILITY_CODE			capCode;
	RMSTR_CAPABILITY_DESCRIPTOR		*pCapabilityDescriptor = NULL;
	RMSTR_CAPABILITY_RAID_LEVEL		*pRaidLevelCapability = NULL;


	rc					= RMSTR_SUCCESS;
	pValidationContext	= (CONTEXT *)_pContext;

	// pValidationContext->pData = cmdInfo
	// pValidationContext->pData1 = enum data		
	// pData2 = NULL
	// pData3 = NULL
	pCmdInfo			= (RMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pCreateArrayDefinition = 
			(RMSTR_CREATE_ARRAY_DEFINITION *)&pCmdParams->createArrayDefinition;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		validationComplete = true;
	} else {
		switch(pValidationContext->state){
		case caREAD_VALIDATION_CAPABILITIES:
			switch(pCreateArrayDefinition->raidLevel){
				case RAID0:
					capCode = RMSTR_CAPABILITY_RAID0;
					break;
				case RAID1:
					capCode = RMSTR_CAPABILITY_RAID1;
					break;
			}

			pCapabilityDescriptor = new RMSTR_CAPABILITY_DESCRIPTOR;
			pValidationContext->pData1 = pCapabilityDescriptor;
			pValidationContext->state = caVALIDATION_CAPABILITIES_READ;
			m_pRmstrCapability->GetRmstrCapability(
						capCode,
						&pValidationContext->value,		// if cap found or not
						pCapabilityDescriptor,
						TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayValidationReply),
						pValidationContext);
			break;

		case caVALIDATION_CAPABILITIES_READ:
			rc = pValidationContext->value;
			if (rc == true){
				// capability found
				pCapabilityDescriptor = (RMSTR_CAPABILITY_DESCRIPTOR *)pValidationContext->pData1;
				pRaidLevelCapability = (RMSTR_CAPABILITY_RAID_LEVEL *)pCapabilityDescriptor->capabilities;
				rc = CheckRaidLevelCapability(
								pCreateArrayDefinition->raidLevel,
								pRaidLevelCapability,
								&pCreateArrayDefinition->dataBlockSize,
								&pCreateArrayDefinition->parityBlockSize);
			}
			if (pValidationContext->pData1){
				delete pValidationContext->pData1;
				pValidationContext->pData1 = NULL;
			}
			if (rc == RMSTR_SUCCESS){
				// Check for number members, number spares, raid level..
				if (pCreateArrayDefinition->numberMembers > MAX_ARRAY_MEMBERS){
					rc = RMSTR_ERR_INVALID_COMMAND;
				}

				if (pCreateArrayDefinition->numberSpares > MAX_ARRAY_SPARES){
					rc = RMSTR_ERR_INVALID_COMMAND;
				}

				if (pCreateArrayDefinition->raidLevel == RAID1){
					if (pCreateArrayDefinition->numberMembers == 1){
						rc = RMSTR_ERR_INVALID_COMMAND;
					}
				}

				// If Hot copy, check RAID Level
				if ((pCreateArrayDefinition->createPolicy.StartHotCopyWithManualBreak) ||
					(pCreateArrayDefinition->createPolicy.StartHotCopyWithAutoBreak)){
					if (pCreateArrayDefinition->raidLevel != RAID1){
						rc = RMSTR_ERR_INVALID_COMMAND;
					}
					if (pCreateArrayDefinition->numberSpares != 0){
						rc = RMSTR_ERR_INVALID_COMMAND;
					}
				}
			} else {
				validationComplete = true;
			}

			if (rc == RMSTR_SUCCESS){
				// read the SRC to check size and fUsed
				pValidationContext->state = caVALIDATION_SRCT_RECORD_READ;
				// reads SRC record into pData3
				rc = caReadSRCTRecord(
						&pCreateArrayDefinition->arrayMembers[pValidationContext->numProcessed],
						TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayValidationReply),
						pValidationContext);
			} else {
				validationComplete = true;
			}
			break;

		case caVALIDATION_SRCT_RECORD_READ:
			// pValidationContext->pData = cmdInfo
			// pValidationContext->pData1 = NULL
			// pData2 = NULL
			// pData3 = SRC Record data
			pSRCRecord = (StorageRollCallRecord *)pValidationContext->pData3;

			rc = rmstrServiceCheckSize(
				(pValidationContext->numProcessed > pCreateArrayDefinition->numberMembers)?false:true,
				pSRCRecord,
				pCreateArrayDefinition->memberCapacity);

			// if member is hot copy source, then ignore error
			// if fUsed for SRC
			if ((pCreateArrayDefinition->createPolicy.StartHotCopyWithManualBreak) ||
				(pCreateArrayDefinition->createPolicy.StartHotCopyWithAutoBreak)){
				if (pCreateArrayDefinition->sourceMemberIndex ==
						pValidationContext->numProcessed){
					if (rc == RMSTR_ERR_STORAGE_ELEMENT_IN_USE){
						rc = RMSTR_SUCCESS;
					}
				}
			}

			if (!rc){
				// check if more members need to be checked
				pValidationContext->numProcessed++;			
				if (pValidationContext->numProcessed <
					pCreateArrayDefinition->numberMembers + pCreateArrayDefinition->numberSpares){
					pValidationContext->state = caVALIDATION_SRCT_RECORD_READ;
					rc = caReadSRCTRecord(
							&pCreateArrayDefinition->arrayMembers[pValidationContext->numProcessed],
							TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayValidationReply),
							pValidationContext);
				} else {
					// all error checking is done, issue the create array cmd
					// pValidationContext->pData1 contains the cmdInfo
					caInsertArrayDescriptorRecord(
						pValidationContext->cmdHandle,
						pCmdInfo);
					validationComplete = true;
				}
			} else {
				validationComplete = true;
			}			
			break;

		default:
			rc = RMSTR_ERR_INVALID_COMMAND;
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
//	caProcessCreateArrayReply
//		Process the different states for create array.
//			Insert Array Descriptor
//			Insert Members and mark as claimed in SRC
//			Insert Spares and mark as claimed in SRC
//			Add member and spare info to ADT
//			Create new SRC entry for Array
//			Update the ADT with the new SRC row id.
//
//	pContext	- our context data for create array messages
//	status		- status of the message
//
//************************************************************************
STATUS DdmRAIDMstr::
caProcessCreateArrayReply(void *_pContext, STATUS status)
{
	CONTEXT					*pCmdContext = (CONTEXT *)_pContext;
	STATUS					rc;
	RMSTR_CREATE_ARRAY_DEFINITION	*pCreateArrayDefinition = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER				*pMember = NULL;
	RAID_SPARE_DESCRIPTOR			*pSpare = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;
	BOOL							cmdComplete = false;
	RMSTR_EVT_ARRAY_ADDED_STATUS	*pEvtArrayAdded = NULL;
	VirtualDeviceRecord				*pVDRecord = NULL;


	RaidRequest						*pRaidRequest = NULL;

	U32 i=0;
	U32 upMembers = 0;


	rc = RMSTR_SUCCESS;


	// pCmdContext->pData = CmdInfo
	// pCmdContext->pData1 = ADT Record
	// pCmdContext->pData2 = member, util, spare descriptors
	// pCmdContext->pData3 = SRC record (after SRC read);
	pCmdInfo		= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pCreateArrayDefinition = 
		(RMSTR_CREATE_ARRAY_DEFINITION *)&pCmdParams->createArrayDefinition;
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;
	pVDRecord = (VirtualDeviceRecord *)pCmdContext->pData4;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){
		case caARRAY_DESCRIPTOR_RECORD_INSERTED:
			AddRmstrData(RAID_ARRAY, &pADTRecord->thisRID, pADTRecord);

			pCmdContext->state = caMEMBER_SRC_RECORD_READ;
			rc = caReadSRCTRecord(
						&pCreateArrayDefinition->arrayMembers[pCmdContext->numProcessed],
						TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
						pCmdContext);
			break;

		case caMEMBER_SRC_RECORD_READ:
			// SRC should have been read
			pSRCRecord = (StorageRollCallRecord *)pCmdContext->pData3;

			// delete the raid array member info (if any)
			if (pCmdContext->pData2){
				delete pCmdContext->pData2;
				pCmdContext->pData2 = NULL;
			}
			pMember = new (tZERO) RAID_ARRAY_MEMBER;
			pCmdContext->pData2 = pMember;

			pMember->policy.SourcePrimary = 0;
			pMember->policy.ReadPreference = READ_PREFERENCE_MEDIUM;
			pMember->memberHealth = RAID_STATUS_UP;
			// set the source member and preferred Member, for RAID 1
			if (pCreateArrayDefinition->raidLevel == RAID1){
				// set the preferred Member
				// Resolve: Need logic to set the correct preferred member
				if (pCreateArrayDefinition->preferredMemberIndex ==
											pCmdContext->numProcessed){	
					pMember->policy.ReadPreference = READ_PREFERENCE_HIGH;
				}

				if ((pCreateArrayDefinition->createPolicy.StartHotCopyWithManualBreak) ||
						(pCreateArrayDefinition->createPolicy.StartHotCopyWithAutoBreak)){				
					// set the source member
					if (pCreateArrayDefinition->sourceMemberIndex ==
											pCmdContext->numProcessed){
						pMember->policy.SourcePrimary = 1;
						pMember->memberHealth = RAID_STATUS_UP;
					} else {
						pMember->memberHealth = RAID_STATUS_DOWN;
					}
					// set the hot copy export member
					if (pCreateArrayDefinition->hotCopyExportMemberIndex ==
											pCmdContext->numProcessed){
						pMember->policy.HotCopyExportMember = 1;
					}
				} else {
					// regular RAID 1 mirror
					if (pCreateArrayDefinition->sourceMemberIndex ==
											pCmdContext->numProcessed){
						pMember->policy.SourcePrimary = 1;
					}
				}
			}
			// set the state identifier
			pMember->stateIdentifier.cmdOpcode = pCmdInfo->opcode;
			pMember->stateIdentifier.cmdRowId = *(rowID *)pCmdContext->cmdHandle;
			pMember->stateIdentifier.cmdState = caMEMBER_DESCRIPTOR_RECORD_INSERTED;
			pMember->stateIdentifier.index = pCmdContext->numProcessed;
			pCmdContext->state = caMEMBER_DESCRIPTOR_RECORD_INSERTED;
			caInsertMemberDescriptorRecords(
						pADTRecord,
						pSRCRecord,
						pMember,
						pCmdContext->numProcessed,		// Member index
						pCmdContext);
			// Now delete the SRC data
			if (pCmdContext->pData3){
				delete pCmdContext->pData3;
				pCmdContext->pData3 = NULL;
			}
			break;

		case caMEMBER_DESCRIPTOR_RECORD_INSERTED:
			// modify the SRC entry to say member is used (claimed)
			// go to member claimed state
			pMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData2;

			AddRmstrData(RAID_MEMBER, &pMember->thisRID, pMember);

			pCmdContext->state = caMEMBER_CLAIMED;
			m_SRCIsUsed = SRC_USED;
			status = m_pTableServices->TableServiceModifyField(
							STORAGE_ROLL_CALL_TABLE,
							&pMember->memberRID,		// SRC row id
							fdSRC_FUSED,				// field name of field to be modifiied
							&m_SRCIsUsed,				// set to true
							sizeof(U32),
							TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
							pCmdContext);
			break;

		case caSPARE_SRC_RECORD_READ:
			caInsertSpareDescriptorRecords(pCmdContext);
			break;

		case caSPARE_DESCRIPTOR_RECORD_INSERTED:
			pSpare = (RAID_SPARE_DESCRIPTOR *)pCmdContext->pData2;

			AddRmstrData(RAID_SPARE, &pSpare->thisRID, pSpare);

			// modify the SRC entry to say member is used.
			pCmdContext->state = caSPARE_CLAIMED;
			m_SRCIsUsed = SRC_USED;
			status = m_pTableServices->TableServiceModifyField(
							STORAGE_ROLL_CALL_TABLE,
							&pCreateArrayDefinition->arrayMembers[pCmdContext->numProcessed],	//key
							fdSRC_FUSED,			// field name of field to be modifiied
							&m_SRCIsUsed,			// set to USED
							sizeof(U32),
							TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
							pCmdContext);

			break;

		case caMEMBER_CLAIMED:
			pMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData2;

			pADTRecord->members[pCmdContext->numProcessed] = 
					pMember->thisRID;
			// update the state identifier..
			pADTRecord->stateIdentifier.cmdState = caMEMBER_INFO_UPDATED_IN_ADT;
			pADTRecord->stateIdentifier.index = pCmdContext->numProcessed;

			pCmdContext->state = caMEMBER_INFO_UPDATED_IN_ADT;
			// if state already processed, then will read the new
			// data into pADTRecord, else will modify
			CheckAndModifyRow(
						RAID_ARRAY,
						&pADTRecord->stateIdentifier,
						RAID_ARRAY_DESCRIPTOR_TABLE,
						&pADTRecord->thisRID,	// row id to modify
						pADTRecord,
						sizeof(RAID_ARRAY_DESCRIPTOR),
						&pADTRecord->thisRID,
						TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
						pCmdContext);
			break;

		case caSPARE_CLAIMED:
			pSpare = (RAID_SPARE_DESCRIPTOR *)pCmdContext->pData2;

			// modify the ADT to fill in the RID of new member/spare
			// will go to ADT_UPDATED
			pCmdContext->state = caMEMBER_INFO_UPDATED_IN_ADT;
			pADTRecord->spares[pCmdContext->numProcessed-pADTRecord->numberMembers] = 
					pSpare->thisRID;
			// update the state identifier..
			pADTRecord->stateIdentifier.cmdState = caMEMBER_INFO_UPDATED_IN_ADT;
			pADTRecord->stateIdentifier.index = pCmdContext->numProcessed;

			// if state already processed, then will read the new
			// data into pADTRecord, else will modify
			CheckAndModifyRow(
						RAID_ARRAY,
						&pADTRecord->stateIdentifier,
						RAID_ARRAY_DESCRIPTOR_TABLE,
						&pADTRecord->thisRID,	// row id to modify
						pADTRecord,
						sizeof(RAID_ARRAY_DESCRIPTOR),
						&pADTRecord->thisRID,
						TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
						pCmdContext);
			break;

		case caMEMBER_INFO_UPDATED_IN_ADT:

			ModifyRmstrData(
						RAID_ARRAY, 
						&pADTRecord->thisRID,
						pADTRecord);

			pCmdContext->numProcessed++;
			// continue to insert until all members are done			
			if (pCmdContext->numProcessed < pADTRecord->numberMembers){
				pCmdContext->state = caMEMBER_SRC_RECORD_READ;
				rc = caReadSRCTRecord(
						&pCreateArrayDefinition->arrayMembers[pCmdContext->numProcessed],
						TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
						pCmdContext);
			} else if (pCmdContext->numProcessed < 
						(pADTRecord->numberMembers+pADTRecord->numberSpares)){
					// try to insert spares if any
					pCmdContext->state = caSPARE_SRC_RECORD_READ;
					rc = caReadSRCTRecord(
							&pCreateArrayDefinition->arrayMembers[pCmdContext->numProcessed],
							TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
							pCmdContext);
			} else {
				// members and spares inserted so go for new VD creation
				pVDRecord = new VirtualDeviceRecord(
								"HDM_RAID",
								IOP_LOCAL,		// primary slot
								IOP_LOCAL,		// sec slot
								true,			// auto instantiate
								RowId(pADTRecord->thisRID));
				pVDRecord->ridVDOwnerUse = RowId(*(rowID *)pCmdContext->cmdHandle);
				// set it to pData4
				pCmdContext->pData4 = pVDRecord;

				pCmdContext->state = caARRAY_VD_CREATED;
				m_pHelperServices->CreateVirtualDevice(
					pVDRecord,
					TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
					pCmdContext);
			}
			break;

		case caARRAY_VD_CREATED:
			// new VD created, update our ADT
#ifdef WIN32
			pADTRecord->arrayVDN = m_dummyVDN++;
#else
			pADTRecord->arrayVDN = ((RowId &)pVDRecord->rid).GetRow();
#endif
			pCmdContext->state = caSRC_ENTRY_INSERTED;

			// now add a new SRC entry for our array
			pSRCRecord = new (tZERO) StorageRollCallRecord;
			pCmdContext->pData2 = pSRCRecord;
			caPrepareSRCEntryForArray(
					pSRCRecord,
					pCreateArrayDefinition,
					pADTRecord);

			// check if we have a same SRC entry and then only insert
			caCheckAndInsertSRCEntryForArray(
					pSRCRecord,
					pCmdContext,
					TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply));
			break;

		case caSRC_ENTRY_INSERTED:
			pSRCRecord = (StorageRollCallRecord *)pCmdContext->pData2;
			pCmdContext->state = caARRAY_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
				&pCmdContext->ucArrayName,
				NULL,		// we know that there is no slot id for arrays
				&pSRCRecord->rid,
				TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
				pCmdContext);
			break;

		case caARRAY_NAME_READ:
			// Now that a new SRC entry is created, 
			// and array name is read, update the ADT
			// with the new SRC rid
			pSRCRecord = (StorageRollCallRecord *)pCmdContext->pData2;
			pADTRecord->SRCTRID = pSRCRecord->rid;

			pCmdContext->state = caSRC_INFO_UPDATED_IN_ADT;

			// This modify should be harmless, since we are just
			// updating the new SRC and the new VD
			m_pTableServices->TableServiceModifyRow(
					RAID_ARRAY_DESCRIPTOR_TABLE,
					&pADTRecord->thisRID,	// row id to modify
					pADTRecord,
					sizeof(RAID_ARRAY_DESCRIPTOR),
					&pADTRecord->thisRID,
					TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
					pCmdContext);
			break;

		case caSRC_INFO_UPDATED_IN_ADT:

			ModifyRmstrData(
						RAID_ARRAY, 
						&pADTRecord->thisRID,
						pADTRecord);

			switch(pCreateArrayDefinition->raidLevel){
				case RAID0:
					cmdComplete = true;
					break;
				case RAID1:
					cmdComplete = true;
					break;
			}
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
		if (rc == RMSTR_SUCCESS){
			switch (pCreateArrayDefinition->raidLevel){
				case RAID1:
					// Before we stop the create array command
					// insert the INTERNAL_CMD to start hot copy
					if ((pCreateArrayDefinition->createPolicy.StartHotCopyWithManualBreak) ||
						(pCreateArrayDefinition->createPolicy.StartHotCopyWithAutoBreak)){
						StartInternalHotCopy(pADTRecord, pCreateArrayDefinition->hotCopyPriority);
					} else {
						// Resolve:
						//StartInternalBkgdInit(pADTRecord);
					}
					// Now stop the Create Array command, so that our Internal cmd
					// gets started immediately, 
					StopCommandProcessing(
							false,		// dont start next cmd after stopping
							pCmdContext->cmdHandle);
					break;
				case RAID0:
					StopCommandProcessing(
							true,		// dont start next cmd after stopping
							pCmdContext->cmdHandle);
			}

			LogEventWithArrayName(
					CTS_RMSTR_ARRAY_ADDED,
					&pADTRecord->SRCTRID);
		}

		// Report the status of the create Array back 
		m_pCmdServer->csrvReportCmdStatus(
				pCmdContext->cmdHandle,		// handle
				rc,							// completion code
				NULL,						// result Data
				(void *)pCmdInfo);			// Orig cmd info

		if (rc == RMSTR_SUCCESS){
			// Generate event for Array Added
			pEvtArrayAdded = new (tZERO) RMSTR_EVT_ARRAY_ADDED_STATUS;

			pSRCRecord = (StorageRollCallRecord *)pCmdContext->pData2;
			pEvtArrayAdded->SRCData = *pSRCRecord;
			pEvtArrayAdded->arrayData = *pADTRecord;
			// copy the array name..
			pCmdContext->ucArrayName.CString(
				pEvtArrayAdded->arrayName, 
				sizeof(UnicodeString32));
			m_pCmdServer->csrvReportEvent(
					RMSTR_EVT_ARRAY_ADDED,		// completion code
					pEvtArrayAdded);			// event Data
			delete pEvtArrayAdded;
			pEvtArrayAdded = NULL;
		}

		if (pCmdContext) {
			delete pCmdContext;
			pCmdContext = NULL;
		}
	}
	return status;
}


//************************************************************************
//	caInsertArrayDescriptorRecord
//		Convert the Array Def from user to Array Descriptor and insert record
//		Called after validation is done
//
//************************************************************************
STATUS DdmRAIDMstr::
caInsertArrayDescriptorRecord(
		HANDLE					handle,
		RMSTR_CMND_INFO			*_pCmdInfo)
{
	STATUS							status = 0;
	CONTEXT							*pCmdContext = NULL;
	RMSTR_CREATE_ARRAY_DEFINITION	*pCreateArrayDefinition = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	

	pCmdContext = new CONTEXT;

	pCmdContext->cmdHandle = handle;
	// save the info into our context
	pCmdContext->pData = new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));
	

	pCmdInfo		= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pCreateArrayDefinition = 
		(RMSTR_CREATE_ARRAY_DEFINITION *)&pCmdParams->createArrayDefinition;


	RAID_ARRAY_DESCRIPTOR *pADTRecord = new (tZERO) RAID_ARRAY_DESCRIPTOR;

	pADTRecord->version = RAID_ARRAY_DESCRIPTOR_TABLE_VERSION;
	pADTRecord->size = sizeof(RAID_ARRAY_DESCRIPTOR);
	pADTRecord->totalCapacity = pCreateArrayDefinition->totalCapacity;
	pADTRecord->memberCapacity = pCreateArrayDefinition->memberCapacity; 

	pADTRecord->dataBlockSize = pCreateArrayDefinition->dataBlockSize;
	pADTRecord->parityBlockSize = pCreateArrayDefinition->parityBlockSize;
	pADTRecord->raidLevel = pCreateArrayDefinition->raidLevel;
	pADTRecord->peckingOrder = pCreateArrayDefinition->peckingOrder;

	pADTRecord->numberMembers = pCreateArrayDefinition->numberMembers;

	pADTRecord->numberSpares = pCreateArrayDefinition->numberSpares;
	pADTRecord->arrayPolicy = pCreateArrayDefinition->arrayPolicy;
	pADTRecord->hostForSparePool = pCreateArrayDefinition->hostForSparePool;

	pADTRecord->sourceMemberIndex = pCreateArrayDefinition->sourceMemberIndex;
	pADTRecord->preferredMemberIndex = pCreateArrayDefinition->preferredMemberIndex;
	pADTRecord->hotCopyExportMemberIndex = pCreateArrayDefinition->hotCopyExportMemberIndex;

	pADTRecord->createPolicy = pCreateArrayDefinition->createPolicy;
	// The following value is filled later, when we Create Virtual Device
	pADTRecord->arrayVDN = 0;
	

	switch(pCreateArrayDefinition->raidLevel){
	case RAID0:
		pADTRecord->health = RAID_OKAY;
		pADTRecord->initStatus = RAID_INIT_COMPLETE;	// not req for RAID0
		break;
	case RAID1:
		if ((pCreateArrayDefinition->createPolicy.StartHotCopyWithManualBreak) ||
			(pCreateArrayDefinition->createPolicy.StartHotCopyWithAutoBreak)){
					pADTRecord->health = RAID_CRITICAL;
					// Resolve:
					//pADTRecord->initStatus = RAID_INIT_HOT_COPYING;
		} else {
			pADTRecord->health = RAID_FAULT_TOLERANT;
			pADTRecord->initStatus = RAID_INIT_NOT_STARTED;
		}
		
		break;
	}
	pADTRecord->numberUtilities = 0;		// Filled later when UDT inserted
	// Resolve:
#ifndef WIN32
	pADTRecord->serialNumber = Kernel::Time_Stamp();
#else
	time((time_t *)&pADTRecord->serialNumber);
	time((time_t *)&pADTRecord->creationDate);
	time((time_t *)&pADTRecord->timeStamp);
#endif


	// context data1 now contains ADT record, pData contains array def
	pCmdContext->pData1 = pADTRecord;
	pCmdContext->state = caARRAY_DESCRIPTOR_RECORD_INSERTED;

	pADTRecord->stateIdentifier.cmdOpcode = pCmdInfo->opcode;
	pADTRecord->stateIdentifier.cmdRowId = *(rowID *)handle;
	pADTRecord->stateIdentifier.cmdState = caARRAY_DESCRIPTOR_RECORD_INSERTED;
	pADTRecord->stateIdentifier.index = 0;

	// if state was already processed, 
	// pADTRecord will be filled with last data from PTS
	status = CheckAndInsertRow(
				RAID_ARRAY,
				&pADTRecord->stateIdentifier,
				RAID_ARRAY_DESCRIPTOR_TABLE,
				pADTRecord,
				sizeof(RAID_ARRAY_DESCRIPTOR),
				&pADTRecord->thisRID,
				TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
				pCmdContext);
	return status;
}



//************************************************************************
//
//	INSERT MEMBER DESCRIPTOR RECORDS
//
//************************************************************************
STATUS DdmRAIDMstr::
caInsertMemberDescriptorRecords(
	RAID_ARRAY_DESCRIPTOR			*pADTRecord,
	StorageRollCallRecord			*pSRCRecord,
	RAID_ARRAY_MEMBER				*pMember,
	U32								memberIndex,
	CONTEXT							*pCmdContext)
{
	STATUS							status = 0;

	rmstrServicePrepareMemberInformation(
				pMember,
				&pADTRecord->thisRID,		// array rid
				&pSRCRecord->rid,			// Src rid
				pMember->memberHealth,		// health
				pCmdContext->numProcessed,	// Member Index
				pSRCRecord->Capacity,		// end lba
				0,							// start LBA
				pSRCRecord->vdnBSADdm,		// member VD
				3,							// max retry cnt
				RAID_QUEUE_ELEVATOR,
				5,
				pMember->policy);							// max outstanding

	// if state was already processed, 
	// pADTRecord will be filled with last data from PTS
	status = CheckAndInsertRow(
				RAID_MEMBER,
				&pMember->stateIdentifier,
				RAID_MEMBER_DESCRIPTOR_TABLE,
				pMember,
				sizeof(RAID_ARRAY_MEMBER),
				&pMember->thisRID,
				TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
				pCmdContext);
	return status;
}



//************************************************************************
//
//	INSERT SPARE DESCRIPTOR RECORDS
//
//************************************************************************//
STATUS DdmRAIDMstr::
caInsertSpareDescriptorRecords(CONTEXT *pCmdContext)
{
	STATUS							status = RMSTR_SUCCESS;

	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_CREATE_ARRAY_DEFINITION	*pCreateArrayDefinition = NULL;	
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;

	STATE_IDENTIFIER					stateIdentifier;

	// pData contains RMSTR_CMND_INFO
	// pData1 contains the ADT 
	pCmdInfo				= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pCreateArrayDefinition	=
		(RMSTR_CREATE_ARRAY_DEFINITION *)&pCmdParams->createArrayDefinition;

	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;


	pSRCRecord = (StorageRollCallRecord *)pCmdContext->pData3;
	pCmdContext->state = caSPARE_DESCRIPTOR_RECORD_INSERTED;

	// Prepare the state Identifier
	stateIdentifier.cmdOpcode = pCmdInfo->opcode;
	stateIdentifier.cmdRowId = *(rowID *)pCmdContext->cmdHandle;
	stateIdentifier.cmdState = caSPARE_DESCRIPTOR_RECORD_INSERTED;;
	stateIdentifier.index = pCmdContext->numProcessed;

	status = rmstrInsertSpareDescriptor(
					&stateIdentifier,
					RAID_DEDICATED_SPARE,
					&pADTRecord->thisRID,
					&pCreateArrayDefinition->arrayMembers[pCmdContext->numProcessed],
					NULL,			// no host rid for dedicated spares
					pSRCRecord->Capacity,
					pSRCRecord->vdnBSADdm,
					TSCALLBACK(DdmRAIDMstr,caProcessCreateArrayReply),
					pCmdContext);
	// Now delete the SRC data
	if (pCmdContext->pData3){
		delete pCmdContext->pData3;
		pCmdContext->pData3 = NULL;
	}
	return status;
}


//************************************************************************
//
//	CHECK AND INSERT SRC ENTRY FOR NEW ARRAY
//		Read the SRC entry with bsaVDN == vdn of pSRCRecord to be inserted
//		If we find same SRC entry then we dont need to insert another one
//		but just return the entry just read
//
//************************************************************************
STATUS DdmRAIDMstr::
caCheckAndInsertSRCEntryForArray(
		StorageRollCallRecord			*pSRCRecord,
		CONTEXT							*pParentContext,
		pTSCallback_t					cb)
{
	STATUS							status = RMSTR_SUCCESS;

	CONTEXT			*pContext = new CONTEXT;

	pContext->pData = pSRCRecord;
	pContext->pCallback = cb;
	pContext->pParentContext = pParentContext;

	// make sure that we read any SRC row flagged by our VD
	TSReadRow *pReadSRC = new TSReadRow;
	// allocate space for SRC record
	status = pReadSRC->Initialize( 
				this,									// DdmServices *pDdmServices,
				STORAGE_ROLL_CALL_TABLE,				// String64 rgbTableName,
				fdSRC_VDNBSADDM,						// String64 rgbKeyFieldName,
				&pSRCRecord->vdnBSADdm,					// For internal use by VD Owner/Creator. ** KEY FIELD **
				sizeof(VDN),							// U32 cbKeyFieldValue,
				pSRCRecord,								// void *prgbRowDataRet,
				sizeof(StorageRollCallRecord),			// U32 cbRowDataRetMax,
				&pContext->value,									// U32 *pcRowsReadRet,
				TSCALLBACK(DdmRAIDMstr,caCheckAndInsertSRCEntryForArrayReply),	// pTSCallback_t pCallback,
				pContext);	// void* pContext
	if (status == OK){
		pReadSRC->Send();
	} else {
		// clean up
		pContext->pData = NULL;
		delete pContext;
		(this->*cb)(pParentContext, status);
	}
	return status;
}

//************************************************************************
//
//		CHECK AND INSERT SRC ENTRY FOR NEW ARRAY REPLY
//
//************************************************************************
STATUS DdmRAIDMstr::
caCheckAndInsertSRCEntryForArrayReply(
		void					*_pContext,
		STATUS					status)
{
	CONTEXT	*pContext = (CONTEXT *)_pContext;
	pTSCallback_t					cb;
	CONTEXT							*pParentContext = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;

	pParentContext = pContext->pParentContext;
	cb = pContext->pCallback;

	if (status == OK){
		// we were able to read the record, so it already exists
		// and we dont need to insert it, call our client back
		(this->*cb)(pParentContext, status);
	} else {
		pSRCRecord = (StorageRollCallRecord *)pContext->pData;
		status = m_pTableServices->TableServiceInsertRow(
				STORAGE_ROLL_CALL_TABLE,
				pSRCRecord,
				sizeof(StorageRollCallRecord),
				&pSRCRecord->rid,
				cb,
				pParentContext);
	}
	// clean up
	pContext->pData = NULL;	// user mem, dont delete
	delete pContext;
	return status;
}


//************************************************************************
//
//		PREPARE THE SRC ENTRY FOR INSERTION
//
//************************************************************************
void DdmRAIDMstr::
caPrepareSRCEntryForArray(
	StorageRollCallRecord				*pSRCRecord,
	RMSTR_CREATE_ARRAY_DEFINITION		*pCreateArrayDefinition,
	RAID_ARRAY_DESCRIPTOR				*pADTRecord)
{

	pSRCRecord->version = STORAGE_ROLL_CALL_TABLE_VERSION;
	pSRCRecord->size = sizeof(StorageRollCallRecord);
	pSRCRecord->Capacity = pADTRecord->totalCapacity;
	pSRCRecord->storageclass = SRCTypeArray;
	pSRCRecord->vdnBSADdm = pADTRecord->arrayVDN;
	pSRCRecord->ridDescriptorRecord = pADTRecord->thisRID;
	pSRCRecord->ridName = pCreateArrayDefinition->arrayNameRowId;	
	//pSRCRecord->ridStatusRecord = Resolve;	
	//pSRCRecord->ridPerformanceRecord = Resolve;
	//pSRCRecord->vdnMonitor = Resolve;
}



//************************************************************************
//	caReadSRCTRecord
//		Read the SRCT record for each member and spare
//
//	RETURN:
//		0 = success
//		1 = failure
//
//************************************************************************
STATUS DdmRAIDMstr::
caReadSRCTRecord(
		rowID				*pRowToRead,
		pTSCallback_t		cb,
		CONTEXT				*pValidationContext)
{
	STATUS			status		= RMSTR_SUCCESS;

	// delete old pData i.e read row data
	if (pValidationContext->pData3){
		delete pValidationContext->pData3;
		pValidationContext->pData3 = NULL;
	}

	// Allocate space for read row data
	pValidationContext->pData3 = new(tZERO) StorageRollCallRecord;

	// read SRCT Record and check size
	status = m_pTableServices->TableServiceReadRow(
			STORAGE_ROLL_CALL_TABLE,
			pRowToRead,
			pValidationContext->pData3,
			sizeof(StorageRollCallRecord),
			cb,
			pValidationContext);
	return status;
}


//************************************************************************
//	CheckRaidLevelCapability
//		Read the default capability data block size and parity block sizes
//		and copy them into the pointers provided.
//
//		pRaidLevelCapability	- ptr to capability for appropriate raid level
//		pDataBlockSize			- return for default data block size
//		pParityBlockSize		- return for parity block size.
//
//************************************************************************
STATUS DdmRAIDMstr::
CheckRaidLevelCapability(
		RAID_LEVEL						raidLevel,
		RMSTR_CAPABILITY_RAID_LEVEL		*pRaidLevelCapability,
		U32								*pDataBlockSize,
		U32								*pParityBlockSize)
{
	U32		i=0;
	BOOL	found = false;

	switch (raidLevel){
		case RAID0:
			// Check if data block size is valid
			for (i=0; i < 10; i++){
				if (pRaidLevelCapability->validDataBlockSizes[i] == 0xFFFFFFFF)
					break;	
				if (pRaidLevelCapability->validDataBlockSizes[i] == *pDataBlockSize){
					found = true;
					break;
				}
			}
			break;
		case RAID1:
			*pDataBlockSize = pRaidLevelCapability->defaultDataBlockSize;
			found = true;
			break;
	}
	if (found){
		// Now we are just assigning the default size for parity
		*pParityBlockSize = pRaidLevelCapability->defaultParityBlockSize;
		return RMSTR_SUCCESS;
	} else {
		return RMSTR_ERR_INVALID_DATA_BLOCK_SIZE;
	}
}
