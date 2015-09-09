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
// File: RmstrRemoveMember.cpp
// 
// Description:
// Implementation for Remove a Member operation
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrRemoveMember.cpp $
// 
// 2     7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 1     7/28/99 6:36p Dpatel
// Initial Creation
// 
//
/*************************************************************************/

#include "OsTypes.h"
#include "Message.h"
#include "CTTypes.h"
#include "OsStatus.h"
#include "Ddm.h"
#include "Fields.h"
#include "Rows.h"
#include "Listen.h"
#include "ReadTable.h"
#include "Table.h"
#include "PTSCommon.h"
#include "RequestCodes.h"

#include "DdmRaidMgmt.h"

#include "StorageRollCallTable.h"
#include "RaidUtilTable.h"
#include "ArrayDescriptor.h"

#include "RMgmtPolicies.h"
#include "RaidDefs.h"
#include "RmstrCmnds.h"


// Remove Member States
enum {
	REMOVE_MEMBER_SRCT_RECORD_READ = 1,
	REMOVE_MEMBER_MEMBER_ADDED
};


enum {
	REMOVE_MEMBER_VALIDATION_SRCT_RECORD_READ = 100,
};


//////////////////////////////////////////////////////////////////////
//
//	Remove An Existing Member
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstr::
RemoveMember(HANDLE h, RMSTR_CMND_INFO *pCmndInfo)
{
	return RemoveMemberValidation(h, pCmndInfo);
}	


//////////////////////////////////////////////////////////////////////
//
//	Remove A Member, AFTER VALIDATION
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstr::
RemoveTheMember(
			HANDLE							handle,
			RMSTR_CMND_INFO					*_pCmdInfo,
			RAID_ARRAY_DESCRIPTOR			*_pADTRecord,
			RAID_ARRAY_MEMBER				*_pMember)
{
	STATUS							status;
	CONTEXT							*pCmdContext = NULL;
	RAID_ARRAY_MEMBER				*pMember = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	StorageRollCallRecord			*pSRCRecord = NULL;
	RaidRequest						*pRaidRequest = NULL;
	RaidRemoveMemberStruct			*pRaidRemoveMemberStruct = NULL;

	pCmdContext = new CONTEXT;

	pCmdContext->cmdHandle		= handle;

	// save the cmd info into our context
	pCmdContext->pData			= new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));


	pCmdContext->pData1			= new RAID_ARRAY_MEMBER;
	memcpy(pCmdContext->pData1, _pMember, sizeof(RAID_ARRAY_MEMBER));
	pMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData1;

	pCmdContext->pData2			= new RAID_ARRAY_DESCRIPTOR;
	memcpy(pCmdContext->pData2,_pADTRecord,sizeof(RAID_ARRAY_DESCRIPTOR));
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData2;

	//Prepare and issue a RAID_DDM_CMD for "Remove Member"
	pRaidRequest = new RaidRequest;
	memset(pRaidRequest,0,sizeof(RaidRequest));

	pRaidRemoveMemberStruct = &pRaidRequest->Data.RemoveData;


	pRaidRequest->Opcode = 	RAID_REQUEST_REMOVE_MEMBER;
	pRaidRequest->RaidVDN = pADTRecord->arrayVDN;

	pRaidRemoveMemberStruct->MemberRowID = pMember->thisRID;

	status = m_pRaidCmdSender->csndrExecute(
							pRaidRequest,
							(pCmdCompletionCallback_t)&DdmRAIDMstr::RaidDdmCommandCompletionReply,
							pCmdContext);
	delete pRaidRequest;
	pRaidRequest = NULL;

#ifdef RMSTR_SSAPI_TEST
	// Generate a fake event
	if (pMember){
		RMSTR_EVT_MEMBER_REMOVED_STATUS	*pEvtMemberRemovedStatus;
		pEvtMemberRemovedStatus = new RMSTR_EVT_MEMBER_REMOVED_STATUS;
		memcpy(
			&pEvtMemberRemovedStatus->memberData,
			pMember,
			sizeof(RAID_ARRAY_MEMBER));
		m_pCmdServer->csrvReportEvent(
				RMSTR_EVT_MEMBER_REMOVED,			// event Code
				pEvtMemberRemovedStatus);			// event Data
		delete pEvtMemberRemovedStatus;
	}
	m_pCmdServer->csrvReportCmdStatus(
			pCmdContext->cmdHandle,		// handle
			status,						// completion code
			NULL,						// result Data
			(void *)pCmdContext->pData);// Orig cmd info
	StopCommandProcessing(true,pCmdContext->cmdHandle);
	if (pCmdContext) {
			delete pCmdContext;
			pCmdContext = NULL;
	}
#endif

	return status;
}	





//////////////////////////////////////////////////////////////////////
//
//	Remove Member VALIDATION
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstr::
RemoveMemberValidation(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo)
{
	STATUS							rc = RMSTR_SUCCESS;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_REMOVE_MEMBER_INFO		*pRemoveMemberInfo = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER				*pMember = NULL;

	CONTEXT	*pValidationContext			= new CONTEXT;
	memset((void *)pValidationContext,0,sizeof(CONTEXT));

	// save the Cmd Info and the handle
	pValidationContext->cmdHandle	= handle;
	pValidationContext->pData		= new RMSTR_CMND_INFO;
	memcpy(pValidationContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));

	pCmdInfo			= (RMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pRemoveMemberInfo	= 
			(RMSTR_REMOVE_MEMBER_INFO *)&pCmdParams->removeMemberInfo;

	// First check the array state
	GetRmstrData(
		RAID_ARRAY,
		&pRemoveMemberInfo->arrayRowId,
		(void **)&pADTRecord);
	if (pADTRecord){
		if (pADTRecord->raidLevel != RAID1){
			rc = RMSTR_ERR_INVALID_COMMAND;
		}
		if (pADTRecord->health == RAID_OFFLINE){
			rc = RMSTR_ERR_ARRAY_OFFLINE;
		}
		if (pADTRecord->health == RAID_CRITICAL){
			rc = RMSTR_ERR_ARRAY_CRITICAL;
		}
		GetRmstrData(
			RAID_MEMBER,
			&pRemoveMemberInfo->memberRowId,
			(void **)&pMember);
		if (pMember == NULL){
			rc = RMSTR_ERR_INVALID_COMMAND;
		}


	} else {
		rc = RMSTR_ERR_INVALID_COMMAND;
	}

	if (rc == RMSTR_SUCCESS){
			RemoveTheMember(
					pValidationContext->cmdHandle,
					pCmdInfo,
					pADTRecord,
					pMember);
	} else {
			m_pCmdServer->csrvReportCmdStatus(
				handle,					// handle
				rc,						// completion code
				NULL,					// result Data
				(void *)pCmdInfo);		// pCmdInfo
			StopCommandProcessing(true,handle);
	}
	delete pValidationContext;
	pValidationContext = NULL;
	return rc;
}






