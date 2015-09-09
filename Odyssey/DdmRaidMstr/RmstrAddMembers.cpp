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
// File: RmstrAddMembers.cpp
// 
// Description:
// Implementation for Add a Member operation
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrAddMembers.cpp $
// 
// 8     8/14/99 1:37p Dpatel
// Added event logging..
// 
// 7     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 6     8/03/99 5:25p Dpatel
// Removed the service method to modify SRC, used table services..
// 
// 5     8/02/99 3:18p Jtaylor
// fixed warnings
// 
// 4     7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 3     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 2     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 1     7/20/99 6:51p Dpatel
// Initial creation.
//
/*************************************************************************/


#include "DdmRaidMgmt.h"



// Add Member States
enum {
	ADD_MEMBER_SRCT_RECORD_READ = 1,
	ADD_MEMBER_MDT_RECORD_INSERTED,
	ADD_MEMBER_SRCT_RECORD_UPDATED,
	ADD_MEMBER_ADT_RECORD_UPDATED
};


enum {
	ADD_MEMBER_VALIDATION_SRCT_RECORD_READ = 100,
};




//************************************************************************
//	AddTheMember
//		Add a new member to an existing array
//	handle			- handle for the cmd
//	*_pCmdInfo		- Cmd info for add member
//	*_pADTRecord	- array to add the member to
//	*_pSRCRecord	- the SRC record for the new member
//
//************************************************************************
STATUS DdmRAIDMstr::
AddTheMember(
			HANDLE							handle,
			RMSTR_CMND_INFO					*_pCmdInfo,
			RAID_ARRAY_DESCRIPTOR			*_pADTRecord,
			StorageRollCallRecord			*_pSRCRecord)
{
	STATUS							status;
	CONTEXT							*pCmdContext = NULL;
	RAID_ARRAY_MEMBER				*pMember	= NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;
	RAID_MEMBER_POLICIES			policy;

	pCmdContext = new CONTEXT;

	pCmdContext->cmdHandle		= handle;

	// save the cmd info into our context
	pCmdContext->pData			= new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));

	pCmdContext->pData1			= new RAID_ARRAY_DESCRIPTOR;
	memcpy(pCmdContext->pData1,_pADTRecord,sizeof(RAID_ARRAY_DESCRIPTOR));
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;

	pCmdContext->pData2			= new StorageRollCallRecord;
	memcpy(pCmdContext->pData2, _pSRCRecord, sizeof(StorageRollCallRecord));
	pSRCRecord = (StorageRollCallRecord *)pCmdContext->pData2;

	pCmdContext->pData3			= new RAID_ARRAY_MEMBER;
	memset(pCmdContext->pData3, 0, sizeof(RAID_ARRAY_MEMBER));

	pMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData3;
	memset(&policy,0,sizeof(RAID_MEMBER_POLICIES));

	policy.SourcePrimary = 0;
	policy.ReadPreference = READ_PREFERENCE_MEDIUM;
	rmstrServicePrepareMemberInformation(
					pMember,
					&pADTRecord->thisRID,			// array rid
					&pSRCRecord->rid,			// Src rid
					RAID_STATUS_DOWN,			// health
					pADTRecord->numberMembers,// Member Index
					pSRCRecord->Capacity,		// end lba
					0,							// start LBA
					pSRCRecord->vdnBSADdm,		// member VD
					3,							// max retry cnt
					RAID_QUEUE_ELEVATOR,
					5,
					policy);

	pCmdContext->state = ADD_MEMBER_MDT_RECORD_INSERTED;
	status = m_pTableServices->TableServiceInsertRow(
				RAID_MEMBER_DESCRIPTOR_TABLE,
				pMember,
				sizeof(RAID_ARRAY_MEMBER),
				&pMember->thisRID,
				(pTSCallback_t)&DdmRAIDMstr::ProcessAddMemberReply,
				pCmdContext);
	return status;
}	


//************************************************************************
//	ProcessAddMemberReply
//		Insert new MDT record
//		Modify the SRC record for the new member to used
//		Update the ADT record
//		Send command to Raid DDM.
//	The event for member added will be generated when we receive
//	an event from the Raid DDM.
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessAddMemberReply(void *_pContext, STATUS status)
{
	CONTEXT							*pCmdContext = (CONTEXT *)_pContext;
	STATUS							rc;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RMSTR_ADD_MEMBER_INFO			*pAddMemberInfo = NULL;
	BOOL							cmdComplete = false;
	RAID_ARRAY_MEMBER				*pMember = NULL;
	RAID_ARRAY_MEMBER				*pExistingMember = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;
	RaidRequest						*pRaidRequest = NULL;
	RaidAddMemberStruct				*pRaidAddMemberStruct = NULL;


	rc = RMSTR_SUCCESS;

	// pCmdContext->pData = cmdInfo
	// pCmdContext->pData1 = ADT Record 
	// pCmdContext->pData2 = SRC Record
	pCmdInfo		= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pAddMemberInfo = 
		(RMSTR_ADD_MEMBER_INFO *)&pCmdParams->addMemberInfo;
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;
	pSRCRecord = (StorageRollCallRecord *)pCmdContext->pData2;
	pMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData3;
	

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){

		case ADD_MEMBER_MDT_RECORD_INSERTED:
			AddRmstrData(
				RAID_MEMBER, 
				&pMember->thisRID, 
				pMember);

			// modify the SRC entry to say member is used.
			pCmdContext->state = ADD_MEMBER_SRCT_RECORD_UPDATED;
			m_SRCIsUsed = SRC_USED;
			status = m_pTableServices->TableServiceModifyField(
							STORAGE_ROLL_CALL_TABLE,
							&pSRCRecord->rid,		// SRC row id
							fdSRC_FUSED,			// field name of field to be modifiied
							&m_SRCIsUsed,			// set to false
							sizeof(U32),
							(pTSCallback_t)&DdmRAIDMstr::ProcessAddMemberReply,
							pCmdContext);
			break;

		case ADD_MEMBER_SRCT_RECORD_UPDATED:
			// modify the ADT to fill in the RID of new member/Member
			pCmdContext->state = ADD_MEMBER_ADT_RECORD_UPDATED;
			pADTRecord->members[pADTRecord->numberMembers] = 
				pMember->thisRID;
			pADTRecord->numberMembers++;

			m_pTableServices->TableServiceModifyRow(
						RAID_ARRAY_DESCRIPTOR_TABLE,
						&pADTRecord->thisRID,	// row id to modify
						pADTRecord,
						sizeof(RAID_ARRAY_DESCRIPTOR),
						&pADTRecord->thisRID,
						(pTSCallback_t)&DdmRAIDMstr::ProcessAddMemberReply,
						pCmdContext);
			break;

		case ADD_MEMBER_ADT_RECORD_UPDATED:
			ModifyRmstrData(
						RAID_ARRAY, 
						&pADTRecord->thisRID,
						pADTRecord);

			if (pMember){
				//Prepare and issue a RAID_DDM_CMD for "Add Member"
				pRaidRequest = new RaidRequest;
				memset(pRaidRequest,0,sizeof(RaidRequest));

				pRaidAddMemberStruct = &pRaidRequest->Data.AddData;


				pRaidRequest->Opcode = 	RAID_REQUEST_ADD_MEMBER;
				pRaidRequest->RaidVDN = pADTRecord->arrayVDN;

				memcpy(
					&pRaidAddMemberStruct->Member,
					pMember,
					sizeof(RAID_ARRAY_MEMBER));

				STATUS status = m_pRaidCmdSender->csndrExecute(
							pRaidRequest,
							(pCmdCompletionCallback_t)&DdmRAIDMstr::RaidDdmCommandCompletionReply,
							pCmdContext);
				delete pRaidRequest;
				pRaidRequest = NULL;
			} else {
				rc = RMSTR_ERR_INVALID_COMMAND;
			}
			cmdComplete = true;
			break;

		default:
			break;
		}
	}
	if (cmdComplete){
		if (rc){
			m_pCmdServer->csrvReportCmdStatus(
				pCmdContext->cmdHandle,		// handle
				rc,							// completion code
				NULL,						// result Data
				(void *)pCmdContext->pData);// Orig cmd info
			StopCommandProcessing(true,pCmdContext->cmdHandle);
			if (pCmdContext) {
				delete pCmdContext;
				pCmdContext = NULL;
			}
		}
	}
	return status;
}



//************************************************************************
//	AddMemberValidation
//		Check array status
//		Check new member size
//
//************************************************************************
STATUS DdmRAIDMstr::
AddMemberValidation(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo)
{
	STATUS							rc = RMSTR_SUCCESS;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_ADD_MEMBER_INFO			*pAddMemberInfo = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;

	CONTEXT	*pValidationContext			= new CONTEXT;
	memset((void *)pValidationContext,0,sizeof(CONTEXT));

	// save the Cmd Info and the handle
	pValidationContext->cmdHandle	= handle;
	pValidationContext->pData		= new RMSTR_CMND_INFO;
	memcpy(pValidationContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));

	pCmdInfo			= (RMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pAddMemberInfo	= 
			(RMSTR_ADD_MEMBER_INFO *)&pCmdParams->addMemberInfo;

	// First check the array state
	GetRmstrData(
		RAID_ARRAY,
		&pAddMemberInfo->arrayRowId,
		(void **)&pADTRecord);
	if (pADTRecord){
		if (pADTRecord->raidLevel != RAID1){
			rc = RMSTR_ERR_INVALID_COMMAND;
		}
		if (pADTRecord->health == RAID_OFFLINE){
			rc = RMSTR_ERR_ARRAY_OFFLINE;
		}
		if (pADTRecord->numberMembers == MAX_ARRAY_MEMBERS){
			rc = RMSTR_ERR_MAX_ARRAY_MEMBERS;
		}
	} else {
		rc = RMSTR_ERR_INVALID_COMMAND;
	}

	if (rc == RMSTR_SUCCESS){
		// copy pADTRecord to pData1
		pValidationContext->pData1 = new char[sizeof(RAID_ARRAY_DESCRIPTOR)];
		memcpy(pValidationContext->pData1, pADTRecord,sizeof(RAID_ARRAY_DESCRIPTOR));

		// first read the SRC entry to make sure Element is not used
		pValidationContext->pData2 = new char[sizeof(StorageRollCallRecord)];
		memset(pValidationContext->pData2,0,sizeof(StorageRollCallRecord));

		pValidationContext->state = ADD_MEMBER_VALIDATION_SRCT_RECORD_READ;
		// read SRCT Record and check size
		rc = m_pTableServices->TableServiceReadRow(
				STORAGE_ROLL_CALL_TABLE,
				&pAddMemberInfo->newMemberRowId,
				pValidationContext->pData2,
				sizeof(StorageRollCallRecord),
				(pTSCallback_t)&DdmRAIDMstr::ProcessAddMemberValidationReply,
				pValidationContext);
	} else {
			m_pCmdServer->csrvReportCmdStatus(
				handle,					// handle
				rc,						// completion code
				NULL,					// result Data
				(void *)pCmdInfo);		// pCmdInfo
			StopCommandProcessing(true,handle);
			delete pValidationContext;
			pValidationContext = NULL;
	}
	return rc;
}

//************************************************************************
//	ProcessAddMemberValidationReply
//		Check array status
//		Check new member size
//
//************************************************************************
STATUS DdmRAIDMstr
::ProcessAddMemberValidationReply(void *_pContext, STATUS status)
{
	CONTEXT							*pValidationContext=NULL;
	RMSTR_CMND_INFO					*pCmdInfo=NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	STATUS							rc;
	RMSTR_ADD_MEMBER_INFO			*pAddMemberInfo = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;
	rowID							*pTargetArrayRowId = NULL;
	BOOL							validationComplete = false;

	rc					= RMSTR_SUCCESS;
	pValidationContext	= (CONTEXT *)_pContext;

	// pValidationContext->pData = cmdInfo
	// pValidationContext->pData1 = ADT Row data
	// pData2 = SRC row data
	pCmdInfo		= (RMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pAddMemberInfo = 
		(RMSTR_ADD_MEMBER_INFO *)&pCmdParams->addMemberInfo;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		validationComplete = true;
	} else {
		pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pValidationContext->pData1;
		switch(pValidationContext->state){
		case ADD_MEMBER_VALIDATION_SRCT_RECORD_READ:
			// pData2 is SRC Record
			pSRCRecord = (StorageRollCallRecord *)pValidationContext->pData2;
			if (pSRCRecord->fUsed){	
				rc = RMSTR_ERR_STORAGE_ELEMENT_IN_USE;
			} else {
				if (!rc){
					rc = rmstrServiceCheckSize(
							true,				// check for Member
							pSRCRecord,
							pADTRecord->memberCapacity);
				}
			}
			if (rc == RMSTR_SUCCESS){
					AddTheMember(
						pValidationContext->cmdHandle,
						pCmdInfo,
						pADTRecord,
						pSRCRecord);
					validationComplete = true;
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
			// Report error 
			m_pCmdServer->csrvReportCmdStatus(
				pValidationContext->cmdHandle,	// handle
				rc,								// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCmdInfo
			StopCommandProcessing(true,pValidationContext->cmdHandle);
		}		
		delete pValidationContext;
		pValidationContext = NULL;
	}
	return rc;
}







