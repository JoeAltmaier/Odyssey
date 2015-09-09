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
// File: RmstrCommitSpare.cpp
// 
// Description:
// Implementation for Down A Member Command
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrCommitSpare.cpp $
// 
// 13    2/11/00 5:21p Dpatel
// GetValidSpare() was returning the incorrect spare since ppSpare was not
// set properly. - DFCT13014
// 
// 12    9/07/99 1:47p Dpatel
// Checked array and utility data during init to check PTS data
// consistency.
// 
// 11    8/27/99 5:24p Dpatel
// added event code..
// 
// 10    8/24/99 5:13p Dpatel
// added failover code, create array changes for "array name"
// 
// 9     8/14/99 1:37p Dpatel
// Added event logging..
// 
// 8     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 7     8/03/99 11:09a Dpatel
// Used TableServices modify field instead of rmstrServiceModify...
// 
// 6     8/03/99 10:15a Jtaylor
// added RMSTR_RAID_DDM test ifdefs
// 
// 5     7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
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
// 1     7/15/99 6:10p Dpatel
// Initial Creation
// 
// 
//
/*************************************************************************/


#include "DdmRaidMgmt.h"


enum {
	COMMIT_SPARE_MDT_RECORD_DELETED = 100,
	COMMIT_SPARE_MDT_RECORD_INSERTED,
	COMMIT_SPARE_SDT_RECORD_DELETED,
	COMMIT_SPARE_ADT_RECORD_UPDATED,
	COMMIT_SPARE_MEMBER_SRCT_RECORD_UNUSED,
};



//************************************************************************
//	CommitSpare
//		This is an Rmstr internal cmd. It reads the ADT, MDT and SDT and
//		then calls the method to actually commit the spare.
//
//************************************************************************
void DdmRAIDMstr::
CommitSpare(
		HANDLE						handle,
		RMSTR_CMND_INFO				*pCmdInfo)
{
	STATUS						status = RMSTR_SUCCESS;
	RMSTR_COMMIT_SPARE_INFO		*pCommitSpareInfo = NULL;
	RMSTR_CMND_PARAMETERS		*pCmdParams = NULL;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	RAID_ARRAY_MEMBER			*pMember = NULL;
	RAID_SPARE_DESCRIPTOR		*pSpare = NULL;

	pCmdParams		= &pCmdInfo->cmdParams;
	pCommitSpareInfo = 
			(RMSTR_COMMIT_SPARE_INFO *)&pCmdParams->commitSpareInfo;

	// Read the ADT
	GetRmstrData(
		RAID_ARRAY,
		&pCommitSpareInfo->arrayRowId,
		(void **)&pADTRecord);

	GetRmstrData(
		RAID_MEMBER,
		&pCommitSpareInfo->memberRowId,
		(void **)&pMember);

	GetRmstrData(
		RAID_SPARE,
		&pCommitSpareInfo->spareRowId,
		(void **)&pSpare);
	CommitTheSpare(handle, pCmdInfo,pSpare,pADTRecord,pMember);
}


//************************************************************************
//	CommitTheSpare
//		Remove the MDT record for the member to be replaced
//		Mark the member's SRC record as UNUSED
//		Insert a new MDT record for the new member
//		Delete the SDT record
//		Update ADT with the new row id of the member
//
//************************************************************************
STATUS DdmRAIDMstr::
CommitTheSpare(
		HANDLE						handle,
		RMSTR_CMND_INFO				*_pCmdInfo,
		RAID_SPARE_DESCRIPTOR		*_pSpare,
		RAID_ARRAY_DESCRIPTOR		*_pADTRecord,
		RAID_ARRAY_MEMBER			*_pMember)
{
	CONTEXT				*pContext = NULL;
	STATUS				status = RMSTR_SUCCESS;
	RAID_ARRAY_MEMBER	*pMember = NULL;

	pContext = new CONTEXT;

	pContext->cmdHandle = handle;

	pContext->pData = new RMSTR_CMND_INFO;
	memcpy(pContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));


	pContext->pData1 = new RAID_ARRAY_DESCRIPTOR;
	memcpy(pContext->pData1, _pADTRecord, sizeof(RAID_ARRAY_DESCRIPTOR));

	if (_pMember){
		// Down member is present
		pContext->pData3 = new RAID_ARRAY_MEMBER;
		memcpy(pContext->pData3, _pMember, sizeof(RAID_ARRAY_MEMBER));
		pMember = (RAID_ARRAY_MEMBER *)pContext->pData3;
		// Save the old member rid
		pContext->newRowId = pMember->thisRID;
	}

	// if we have spare, then we have member (according to state m/c
	// so we start cmd from step 1
	if (_pSpare) {
		pContext->pData2 = new RAID_SPARE_DESCRIPTOR;
		memcpy(pContext->pData2, _pSpare, sizeof(RAID_SPARE_DESCRIPTOR));
		// modify the SRC entry to say member is now free
		pContext->state = COMMIT_SPARE_MEMBER_SRCT_RECORD_UNUSED;
		pContext->value = false;
		status = m_pTableServices->TableServiceModifyField(
							STORAGE_ROLL_CALL_TABLE,
							&pMember->memberRID,		// SRC row id
							fdSRC_FUSED,				// field name of field to be modifiied
							&pContext->value,		// set to false
							sizeof(U32),
							(pTSCallback_t)&DdmRAIDMstr::ProcessCommitSpareReply,
							pContext);
	} else {
		// if we still have member, but no spare then cmd was sent to
		// RAID DDM, so we just remove old member
		pContext->state = COMMIT_SPARE_MDT_RECORD_DELETED;
		status = m_pTableServices->TableServiceDeleteRow(
								RAID_MEMBER_DESCRIPTOR_TABLE,
								&pContext->newRowId, // we saved the old member id here
								(pTSCallback_t)&DdmRAIDMstr::ProcessCommitSpareReply,
								pContext);
	}
	return status;
}


//************************************************************************
//	ProcessCommitSpareReply
//		Remove the MDT record for the member to be replaced
//		Mark the member's SRC record as UNUSED
//		Insert a new MDT record for the new member
//		Delete the SDT record
//		Update ADT with the new row id of the member
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessCommitSpareReply(void *_pContext, STATUS status)
{
	CONTEXT								*pContext = (CONTEXT *)_pContext;
	RAID_ARRAY_DESCRIPTOR				*pADTRecord = NULL;
	RAID_ARRAY_MEMBER					*pMember = NULL;
	RAID_SPARE_DESCRIPTOR				*pSpare = NULL;
	BOOL								commitSpareComplete = false;
	STATUS								rc;
	RMSTR_CMND_INFO						*pCmdInfo = NULL;
	RaidRequest							*pRaidRequest = NULL;
	RaidReplaceMemberStruct				*pRaidReplaceMemberStruct = NULL;

	rc = RMSTR_SUCCESS;

	pCmdInfo = (RMSTR_CMND_INFO *)pContext->pData;
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pContext->pData1;
	pSpare = (RAID_SPARE_DESCRIPTOR *)pContext->pData2;
	pMember = (RAID_ARRAY_MEMBER *)pContext->pData3;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		commitSpareComplete = true;
		rc = RMSTR_ERR_INVALID_COMMAND;
	} else {

		switch (pContext->state){
		case COMMIT_SPARE_MEMBER_SRCT_RECORD_UNUSED:
			// Update the Member info with info from spare
			pMember->policy.SourcePrimary = 0;
			pMember->endLBA = pSpare->capacity;
			pMember->memberVD = pSpare->bsaVdn;
			pMember->memberRID = pSpare->SRCTRID;
			memset(&pMember->thisRID, 0, sizeof(rowID));
		
			pContext->state = COMMIT_SPARE_MDT_RECORD_INSERTED;
			// Now Insert the spare as the new member
			SetStateIdentifier(
						&pMember->stateIdentifier,
						pCmdInfo->opcode,
						(rowID *)pContext->cmdHandle,
						COMMIT_SPARE_MDT_RECORD_INSERTED,
						0);
			rc = CheckAndInsertRow(
					RAID_MEMBER,
					&pMember->stateIdentifier,
					RAID_MEMBER_DESCRIPTOR_TABLE,
					pMember,
					sizeof(RAID_ARRAY_MEMBER),
					&pMember->thisRID,
					(pTSCallback_t)&DdmRAIDMstr::ProcessCommitSpareReply,
					pContext);
			break;

		case COMMIT_SPARE_MDT_RECORD_INSERTED:
			AddRmstrData(
				RAID_MEMBER,
				&pMember->thisRID,
				pMember);

			// Modify the ADT to update the member info
			pADTRecord->members[pMember->memberIndex] = pMember->thisRID;
			// Modify the ADT to update the spare info
			if (pSpare->spareType == RAID_DEDICATED_SPARE){
				rmstrServiceDeleteMemberSpareUtilityFromADT(
							RAID_SPARE,
							&pSpare->thisRID,
							pADTRecord);
			}
			SetStateIdentifier(
						&pADTRecord->stateIdentifier,
						pCmdInfo->opcode,
						(rowID *)pContext->cmdHandle,
						COMMIT_SPARE_ADT_RECORD_UPDATED,
						0);
			pContext->state = COMMIT_SPARE_ADT_RECORD_UPDATED;
			rc = CheckAndModifyRow(
						RAID_ARRAY,
						&pADTRecord->stateIdentifier,
						RAID_ARRAY_DESCRIPTOR_TABLE,
						&pADTRecord->thisRID,	// row id to modify
						pADTRecord,
						sizeof(RAID_ARRAY_DESCRIPTOR),
						&pADTRecord->thisRID,
						(pTSCallback_t)&DdmRAIDMstr::ProcessCommitSpareReply,
						pContext);
			break;


		case COMMIT_SPARE_ADT_RECORD_UPDATED:
			ModifyRmstrData(
					RAID_ARRAY,
					&pADTRecord->thisRID,
					pADTRecord);

			//Prepare a RAID_DDM_CMD for Replace Member
			pRaidRequest = new RaidRequest;
			pRaidReplaceMemberStruct = &pRaidRequest->Data.ReplaceData;
			memset(pRaidRequest,0,sizeof(RaidRequest));

			pRaidRequest->Opcode = 	RAID_REQUEST_REPLACE_MEMBER;
			pRaidRequest->RaidVDN = pADTRecord->arrayVDN;

			pRaidReplaceMemberStruct->OldMemberRowID = pContext->newRowId;
			memcpy(
				&(pRaidReplaceMemberStruct->Member),
				pMember,
				sizeof(RAID_ARRAY_MEMBER));		
			status = m_pRaidCmdSender->csndrCheckAndExecute(
				pRaidRequest,
				(pCmdCompletionCallback_t)&DdmRAIDMstr::RaidDdmCommandCompletionReply,
				pContext);
			delete pRaidRequest;

			// Now Delete the Spare
			pContext->state = COMMIT_SPARE_SDT_RECORD_DELETED;
			rc = m_pTableServices->TableServiceDeleteRow(
						RAID_SPARE_DESCRIPTOR_TABLE,
						&pSpare->thisRID,			// row id to delete
						(pTSCallback_t)&DdmRAIDMstr::ProcessCommitSpareReply,
						pContext);
			break;

		case COMMIT_SPARE_SDT_RECORD_DELETED:
			RemoveRmstrData(
					RAID_SPARE,
					&pSpare->thisRID);

			// delete old member now
			pContext->state = COMMIT_SPARE_MDT_RECORD_DELETED;
			status = m_pTableServices->TableServiceDeleteRow(
								RAID_MEMBER_DESCRIPTOR_TABLE,
								&pContext->newRowId, // we saved the old member id here
								(pTSCallback_t)&DdmRAIDMstr::ProcessCommitSpareReply,
								pContext);
			break;

		case COMMIT_SPARE_MDT_RECORD_DELETED:
			RemoveRmstrData(
					RAID_MEMBER,
					&pContext->newRowId);
			commitSpareComplete = true;
			break;

		default:
			break;
		}
	}
	if (commitSpareComplete){
		if (rc){
			m_pCmdServer->csrvReportCmdStatus(
				pContext->cmdHandle,	// handle
				rc,						// completion code
				NULL,					// result Data
				(void *)pCmdInfo);		// pCmdInfo
			StopCommandProcessing(true,pContext->cmdHandle);
			delete pContext;
		}
	}
	return status;
}


//************************************************************************
//	CommitFirstValidSpare
//		A convinience method to find the first valid spare and 
//		replace the down member with this spare. If valid spare is
//		found, an internal command to commitSpare is started. If
//		no valid spare is found a warning event is generated.
//
//	_pADTRecord	- the array to which the spare is to be committed
//	_pMember	- the member which is to be replaced with the spare
//
//************************************************************************
STATUS DdmRAIDMstr::
CommitFirstValidSpare(
		RAID_ARRAY_DESCRIPTOR			*pADTRecord,
		RAID_ARRAY_MEMBER				*pMember)
{
	BOOL					spareCommited = false;
	RAID_SPARE_DESCRIPTOR	*pSpare = NULL;

	GetValidSpare(
			pADTRecord, 
			&pSpare);	
	if (pSpare) {
		StartInternalCommitSpare(
				pSpare,
				pADTRecord,
				pMember); // commits dedicated or pool spare
		spareCommited = true;
	} else {
		// if none found, member will be down (marked by RAID DDM)
		// array state (if critical should have been marked)
		LogEventWithArrayName(
			CTS_RMSTR_REGENERATE_NOT_STARTED,
			&pADTRecord->SRCTRID);
	}
	return spareCommited;
}


//************************************************************************
//	GetValidSpare
//		A convinience method to find a spare that fits for the array.
//		First all dedicated spares are checked and then pool spares
//
//	_pADTRecord	- the array to which the spare is to be committed
//	_ppSpare	- where the valid spare (if found) is returned, else NULL
//
//************************************************************************
STATUS DdmRAIDMstr::
GetValidSpare(
		RAID_ARRAY_DESCRIPTOR			*pADTRecord,
		RAID_SPARE_DESCRIPTOR			**ppSpare)
{
	RAID_SPARE_DESCRIPTOR		*pSpare = *ppSpare;
	BOOL						spareFound = false;

	// Check for dedicated spares
	if (pADTRecord->numberSpares){
		GetRmstrData(
			RAID_SPARE,
			&pADTRecord->spares[0],
			(void **)ppSpare);
		spareFound = true;
	} else {
		// Check for Pool Spares
		TraverseRmstrData(
			RAID_SPARE,
			NULL,
			(void **)ppSpare);
		while (pSpare != NULL){
			if (pSpare->spareType != RAID_DEDICATED_SPARE){
				if (pSpare->capacity > pADTRecord->memberCapacity){
					spareFound=true;
					break;
				}
			}
			TraverseRmstrData(
				RAID_SPARE,
				&pSpare->thisRID,
				(void **)&pSpare);
		}
	} 
	if (!spareFound){
		*ppSpare = NULL;
	}
	return spareFound;
}


