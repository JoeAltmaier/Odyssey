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
// File: RmstrChangeSourceMember.cpp
// 
// Description:
// Implementation for changing Source member of a RAID 1 array
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrChangeSourceMember.cpp $
// 
// 7     10/12/99 5:40p Agusev
// no validation for internal cmds. - dp
// 
// 6     9/07/99 1:47p Dpatel
// Checked array and utility data during init to check PTS data
// consistency.
// 
// 5     8/31/99 6:39p Dpatel
// events, util abort processing etc.
// 
// 4     8/14/99 1:37p Dpatel
// Added event logging..
// 
// 3     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 2     8/03/99 6:22p Dpatel
// Check on member down, if source member down then change the source
// 
// 1     8/02/99 2:59p Dpatel
// Initial creation..
// 
//
/*************************************************************************/


#include "DdmRaidMgmt.h"


// Change Source Member States
enum {
	CHANGE_SOURCE_OLD_MDT_RECORD_UPDATED = 1,
	CHANGE_SOURCE_NEW_MDT_RECORD_UPDATED,
	CHANGE_SOURCE_ADT_RECORD_UPDATED,
	CHANGE_SOURCE_ADT_RECORD_READ
};



//************************************************************************
//	ChangeTheSourceMember
//		Change the source member for RAID 1 arrays
//
//	handle		- Handle for the cmd
//	_pCmdInfo	- cmd packet for change source member
//	_pADTRecord	- the array whose source member is to be changed
//	_pMember	- the member which is to be set as the new source
//
//************************************************************************
STATUS DdmRAIDMstr::
ChangeTheSourceMember(
		HANDLE						handle,
		RMSTR_CMND_INFO				*_pCmdInfo,
		RAID_ARRAY_DESCRIPTOR		*_pADTRecord,
		RAID_ARRAY_MEMBER			*_pMember)
{
	STATUS							status = RMSTR_SUCCESS;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	CONTEXT							*pCmdContext = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER				*pNewSourceMember = NULL;
	RAID_ARRAY_MEMBER				*pOldSourceMember = NULL;
	RMSTR_CHANGE_SOURCE_MEMBER_INFO	*pChangeSourceMemberInfo = NULL;
	rowID							oldSourceMemberRowId;
	

	pCmdContext = new CONTEXT;

	pCmdContext->cmdHandle	= handle;
	pCmdContext->pData		= new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));

	pCmdContext->pData1		= new RAID_ARRAY_DESCRIPTOR;
	memcpy(pCmdContext->pData1, _pADTRecord, sizeof(RAID_ARRAY_DESCRIPTOR));
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;

	pCmdContext->pData2		= new RAID_ARRAY_MEMBER;
	memcpy(pCmdContext->pData2, _pMember, sizeof(RAID_ARRAY_MEMBER));
	pNewSourceMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData2;

	oldSourceMemberRowId = pADTRecord->members[pADTRecord->sourceMemberIndex];

	GetRmstrData(
		RAID_MEMBER,
		&oldSourceMemberRowId,
		(void **)&pOldSourceMember);

	pCmdContext->pData3		= new RAID_ARRAY_MEMBER;
	memcpy(pCmdContext->pData3, pOldSourceMember, sizeof(RAID_ARRAY_MEMBER));
	pOldSourceMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData3;

	pCmdInfo				= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pChangeSourceMemberInfo		=
		(RMSTR_CHANGE_SOURCE_MEMBER_INFO *)&pCmdParams->changeSourceMemberInfo;

	// Update the old member's Source member
	pCmdContext->state = CHANGE_SOURCE_OLD_MDT_RECORD_UPDATED;
	pOldSourceMember->policy.SourcePrimary = 0;
	status = m_pTableServices->TableServiceModifyField(
				RAID_MEMBER_DESCRIPTOR_TABLE,
				&pOldSourceMember->thisRID,			// row id to modify
				fdMEM_POLICY,
				&pOldSourceMember->policy,
				sizeof(RAID_MEMBER_POLICIES),
				(pTSCallback_t)&DdmRAIDMstr::ProcessChangeSourceMemberReply,
				pCmdContext);
	return status;
}	


//************************************************************************
//	ProcessChangeSourceMemberReply
//		Modify the current member's policy to remove as source member
//		Modify the new member's policy to set it as new source member
//		Update the ADT
//		Generate Event for source member changed
//
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessChangeSourceMemberReply(void *_pContext, STATUS status)
{
	CONTEXT							*pCmdContext = (CONTEXT *)_pContext;
	STATUS							rc;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER					*pOldSourceMember = NULL;
	RAID_ARRAY_MEMBER					*pNewSourceMember = NULL;
	RMSTR_CHANGE_SOURCE_MEMBER_INFO		*pChangeSourceMemberInfo = NULL;

	BOOL							cmdComplete = false;

	rc = RMSTR_SUCCESS;

	// pCmdContext->pData = cmdInfo
	// pCmdContext->pData1 = ADT Record (for Dedicated Spares only, else NULL)
	// pCmdContext->pData2 = new MDT Record
	// pCmdContext->pData3 = old MDT Record
	pCmdInfo				= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pChangeSourceMemberInfo		=
		(RMSTR_CHANGE_SOURCE_MEMBER_INFO *)&pCmdParams->changeSourceMemberInfo;

	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;
	pNewSourceMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData2;
	pOldSourceMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData3;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){
		case CHANGE_SOURCE_OLD_MDT_RECORD_UPDATED:
			ModifyRmstrData(
				RAID_MEMBER,
				&pOldSourceMember->thisRID,
				pOldSourceMember);

			pCmdContext->state = CHANGE_SOURCE_NEW_MDT_RECORD_UPDATED;
			pNewSourceMember->policy.SourcePrimary = 1;
			status = m_pTableServices->TableServiceModifyField(
						RAID_MEMBER_DESCRIPTOR_TABLE,
						&pNewSourceMember->thisRID,			// row id to modify
						fdMEM_POLICY,
						&pNewSourceMember->policy,
						sizeof(RAID_MEMBER_POLICIES),
						(pTSCallback_t)&DdmRAIDMstr::ProcessChangeSourceMemberReply,
						pCmdContext);
			break;

		case CHANGE_SOURCE_NEW_MDT_RECORD_UPDATED:
			ModifyRmstrData(
				RAID_MEMBER,
				&pNewSourceMember->thisRID,
				pNewSourceMember);

			pADTRecord->sourceMemberIndex = pNewSourceMember->memberIndex;
			pCmdContext->state = CHANGE_SOURCE_ADT_RECORD_UPDATED;
			status = m_pTableServices->TableServiceModifyField(
						RAID_ARRAY_DESCRIPTOR_TABLE,
						&pADTRecord->thisRID,			// row id to modify
						fdSOURCE_MEMBER,
						&pADTRecord->sourceMemberIndex,
						sizeof(U32),
						(pTSCallback_t)&DdmRAIDMstr::ProcessChangeSourceMemberReply,
						pCmdContext);
			break;

		case CHANGE_SOURCE_ADT_RECORD_UPDATED:
			ModifyRmstrData(
					RAID_ARRAY,
					&pADTRecord->thisRID,
					pADTRecord);
			// for test only...read to make sure if modified correcltly
			pCmdContext->state = CHANGE_SOURCE_ADT_RECORD_READ;
			pADTRecord->numberMembers = 0;
			pADTRecord->sourceMemberIndex = 0;
			m_pTableServices->TableServiceReadRow(
					RAID_ARRAY_DESCRIPTOR_TABLE,		// tableName
					&pADTRecord->thisRID,
					pADTRecord,						// buffer to put data
					sizeof(RAID_ARRAY_DESCRIPTOR),
					(pTSCallback_t)&DdmRAIDMstr::ProcessChangeSourceMemberReply,
					pCmdContext);
			break;

		case CHANGE_SOURCE_ADT_RECORD_READ:
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
			// Generate event for Source Member changed
			RMSTR_EVT_SOURCE_MEMBER_CHANGED_STATUS *pEvent;
			pEvent = new RMSTR_EVT_SOURCE_MEMBER_CHANGED_STATUS;
			memcpy(
				&pEvent->arrayData,
				pADTRecord,
				sizeof(RAID_ARRAY_DESCRIPTOR));
			m_pCmdServer->csrvReportEvent(
				RMSTR_EVT_SOURCE_MEMBER_CHANGED,		// event Code
				pEvent);		// event Data
			delete pEvent;

			LogMemberEvent(
				CTS_RMSTR_SOURCE_MEMBER_CHANGED,
				pADTRecord,
				pNewSourceMember);
		}
		StopCommandProcessing(true, pCmdContext->cmdHandle);
		delete pCmdContext;
		pCmdContext = NULL;
	}
	return status;
}


//************************************************************************
//	ChangeSourceMemberValidation
//		Check if array and member are present.
//		Check for appropriate Raid Level
//
//************************************************************************
STATUS DdmRAIDMstr::
ChangeSourceMemberValidation(HANDLE h, RMSTR_CMND_INFO *_pCmdInfo)
{
	STATUS								status = RMSTR_SUCCESS;
	RMSTR_CMND_PARAMETERS				*pCmdParams = NULL;
	RMSTR_CHANGE_SOURCE_MEMBER_INFO	*pChangeSourceMemberInfo = NULL;
	RMSTR_CMND_INFO						*pCmdInfo =NULL;
	RAID_ARRAY_DESCRIPTOR				*pADTRecord = NULL;
	RAID_ARRAY_MEMBER					*pMember = NULL;

	CONTEXT	*pValidationContext			= new CONTEXT;
	memset((void *)pValidationContext,0,sizeof(CONTEXT));

	// save the Cmd Info and the handle
	pValidationContext->cmdHandle	= h;

	pValidationContext->pData		= new RMSTR_CMND_INFO;
	memcpy(pValidationContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));


	pCmdInfo			= (RMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pChangeSourceMemberInfo	= 
		(RMSTR_CHANGE_SOURCE_MEMBER_INFO *)&pCmdParams->changeSourceMemberInfo;

	// first get the ADT record
	GetRmstrData(
			RAID_ARRAY,
			&pChangeSourceMemberInfo->arrayRowId,
			(void **)&pADTRecord);
	GetRmstrData(
			RAID_MEMBER,
			&pChangeSourceMemberInfo->newMemberRowId,
			(void **)&pMember);

	if (pADTRecord && pMember) {
		if (pCmdInfo->opcode != RMSTR_INTERNAL_CMND_CHANGE_SOURCE_MEMBER){
			// no validation check for our internal cmds
			if (pADTRecord->numberUtilities){
				// Resolve: should check,  if init then no error??
				status = RMSTR_ERR_UTIL_RUNNING;
			}			
		} 
		if (status == RMSTR_SUCCESS) {
			if (pADTRecord->raidLevel == RAID1){
				ChangeTheSourceMember(
						pValidationContext->cmdHandle,
						pCmdInfo,
						pADTRecord,
						pMember);
			} else {
				status = RMSTR_ERR_INVALID_RAID_LEVEL;
			}
		}
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
