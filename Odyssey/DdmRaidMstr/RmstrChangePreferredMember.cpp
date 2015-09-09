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
// File: RmstrChangePreferred.cpp
// 
// Description:
// Implementation for changing "preferred read" member of an array.
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrChangePreferredMember.cpp $
// 
// 6     9/07/99 4:15p Dpatel
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
// 2     8/02/99 3:01p Dpatel
// changes to create array, array FT processing...
// 
// 1     7/30/99 6:48p Dpatel
// Initial Creation.
// 
//
/*************************************************************************/


#include "DdmRaidMgmt.h"


// Change Preferred Member States
enum {
	CHANGE_PREFERRED_OLD_MDT_RECORD_UPDATED = 1,
	CHANGE_PREFERRED_NEW_MDT_RECORD_UPDATED,
	CHANGE_PREFERRED_ADT_RECORD_UPDATED,
	CHANGE_PREFERRED_ADT_RECORD_READ
};



//************************************************************************
//	ChangeThePreferredMember
//		Change the preferred read member of an array		
//
//	handle		- Handle for the cmd
//	_pCmdInfo	- cmd packet for change preferred member
//	_pADTRecord	- the array whose preferred read member is to be changed
//	_pMember	- the member which is to be set as the new preferred member
//
//************************************************************************
STATUS DdmRAIDMstr::
ChangeThePreferredMember(
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
	RAID_ARRAY_MEMBER				*pNewPreferredMember = NULL;
	RAID_ARRAY_MEMBER				*pOldPreferredMember = NULL;
	RMSTR_CHANGE_PREFERRED_MEMBER_INFO	*pChangePreferredMemberInfo = NULL;
	rowID							oldPreferredMemberRowId;
	

	pCmdContext = new CONTEXT;

	pCmdContext->cmdHandle	= handle;
	pCmdContext->pData		= new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));

	pCmdContext->pData1		= new RAID_ARRAY_DESCRIPTOR;
	memcpy(pCmdContext->pData1, _pADTRecord, sizeof(RAID_ARRAY_DESCRIPTOR));
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;

	pCmdContext->pData2		= new RAID_ARRAY_MEMBER;
	memcpy(pCmdContext->pData2, _pMember, sizeof(RAID_ARRAY_MEMBER));
	pNewPreferredMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData2;

	oldPreferredMemberRowId = pADTRecord->members[pADTRecord->preferredMemberIndex];

	GetRmstrData(
		RAID_MEMBER,
		&oldPreferredMemberRowId,
		(void **)&pOldPreferredMember);

	pCmdContext->pData3		= new RAID_ARRAY_MEMBER;
	memcpy(pCmdContext->pData3, pOldPreferredMember, sizeof(RAID_ARRAY_MEMBER));
	pOldPreferredMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData3;


	pCmdInfo				= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pChangePreferredMemberInfo		=
		(RMSTR_CHANGE_PREFERRED_MEMBER_INFO *)&pCmdParams->changePreferredMemberInfo;


	// Update the old member's preferred member
	pCmdContext->state = CHANGE_PREFERRED_OLD_MDT_RECORD_UPDATED;
	pOldPreferredMember->policy.ReadPreference = READ_PREFERENCE_MEDIUM;
	status = m_pTableServices->TableServiceModifyField(
				RAID_MEMBER_DESCRIPTOR_TABLE,
				&pOldPreferredMember->thisRID,			// row id to modify
				fdMEM_POLICY,
				&pOldPreferredMember->policy,
				sizeof(RAID_MEMBER_POLICIES),
				(pTSCallback_t)&DdmRAIDMstr::ProcessChangePreferredMemberReply,
				pCmdContext);
	return status;
}	


//************************************************************************
//	ProcessChangePreferredMemberReply
//		Modify the current member's policy to remove preferred read member
//		Modify the new preferred member's policy to set it as preferred member
//		Update the ADT
//		Generate Event for preferred member changed
//
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessChangePreferredMemberReply(void *_pContext, STATUS status)
{
	CONTEXT							*pCmdContext = (CONTEXT *)_pContext;
	STATUS							rc;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER					*pOldPreferredMember = NULL;
	RAID_ARRAY_MEMBER					*pNewPreferredMember = NULL;
	RMSTR_CHANGE_PREFERRED_MEMBER_INFO	*pChangePreferredMemberInfo = NULL;

	BOOL							cmdComplete = false;

	rc = RMSTR_SUCCESS;

	// pCmdContext->pData = cmdInfo
	// pCmdContext->pData1 = ADT Record (for Dedicated Spares only, else NULL)
	// pCmdContext->pData2 = new MDT Record
	// pCmdContext->pData3 = old MDT Record
	pCmdInfo				= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pChangePreferredMemberInfo		=
		(RMSTR_CHANGE_PREFERRED_MEMBER_INFO *)&pCmdParams->changePreferredMemberInfo;

	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;
	pNewPreferredMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData2;
	pOldPreferredMember = (RAID_ARRAY_MEMBER *)pCmdContext->pData3;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){
		case CHANGE_PREFERRED_OLD_MDT_RECORD_UPDATED:
			ModifyRmstrData(
				RAID_MEMBER,
				&pOldPreferredMember->thisRID,
				pOldPreferredMember);

			pCmdContext->state = CHANGE_PREFERRED_NEW_MDT_RECORD_UPDATED;
			pNewPreferredMember->policy.ReadPreference = READ_PREFERENCE_HIGH;
			status = m_pTableServices->TableServiceModifyField(
						RAID_MEMBER_DESCRIPTOR_TABLE,
						&pNewPreferredMember->thisRID,			// row id to modify
						fdMEM_POLICY,
						&pNewPreferredMember->policy,
						sizeof(RAID_MEMBER_POLICIES),
						(pTSCallback_t)&DdmRAIDMstr::ProcessChangePreferredMemberReply,
						pCmdContext);
			break;

		case CHANGE_PREFERRED_NEW_MDT_RECORD_UPDATED:
			ModifyRmstrData(
				RAID_MEMBER,
				&pNewPreferredMember->thisRID,
				pNewPreferredMember);

			pADTRecord->preferredMemberIndex = pNewPreferredMember->memberIndex;
			pCmdContext->state = CHANGE_PREFERRED_ADT_RECORD_UPDATED;
			status = m_pTableServices->TableServiceModifyField(
						RAID_ARRAY_DESCRIPTOR_TABLE,
						&pADTRecord->thisRID,			// row id to modify
						fdPREFERRED_MEMBER,
						&pADTRecord->preferredMemberIndex,
						sizeof(U32),
						(pTSCallback_t)&DdmRAIDMstr::ProcessChangePreferredMemberReply,
						pCmdContext);
			break;

		case CHANGE_PREFERRED_ADT_RECORD_UPDATED:
			ModifyRmstrData(
					RAID_ARRAY,
					&pADTRecord->thisRID,
					pADTRecord);
			// Resolve:
			// Send RaidReqest to RAID DDM
			pCmdContext->state = CHANGE_PREFERRED_ADT_RECORD_READ;
			pADTRecord->numberMembers = 0;
			pADTRecord->preferredMemberIndex = 0;
			m_pTableServices->TableServiceReadRow(
					RAID_ARRAY_DESCRIPTOR_TABLE,		// tableName
					&pADTRecord->thisRID,
					pADTRecord,						// buffer to put data
					sizeof(RAID_ARRAY_DESCRIPTOR),
					(pTSCallback_t)&DdmRAIDMstr::ProcessChangePreferredMemberReply,
					pCmdContext);
			break;

		case CHANGE_PREFERRED_ADT_RECORD_READ:
			cmdComplete = true;
			break;

		default:
			break;
		}
	}
	if (cmdComplete){
		// Report the status of the Change preferred member back 
		m_pCmdServer->csrvReportCmdStatus(
			pCmdContext->cmdHandle,		// handle
			rc,							// completion code
			NULL,						// result Data
			(void *)pCmdInfo);			// Orig cmd info
		if (rc == RMSTR_SUCCESS){
			// Generate event for Preferred Member changed
			RMSTR_EVT_PREFERRED_MEMBER_CHANGED_STATUS *pEvent;
			pEvent = new RMSTR_EVT_PREFERRED_MEMBER_CHANGED_STATUS;
			memcpy(
				&pEvent->arrayData,
				pADTRecord,
				sizeof(RAID_ARRAY_DESCRIPTOR));
			m_pCmdServer->csrvReportEvent(
				RMSTR_EVT_PREFERRED_MEMBER_CHANGED,		// event Code
				pEvent);		// event Data
			delete pEvent;
			LogMemberEvent(
				CTS_RMSTR_PREFERRED_MEMBER_CHANGED,
				pADTRecord,
				pNewPreferredMember);
		}
		StopCommandProcessing(true, pCmdContext->cmdHandle);
		delete pCmdContext;
		pCmdContext = NULL;
	}
	return status;
}






//************************************************************************
//	ChangePreferredMemberValidation
//		Check if array and member are present.
//		Check for appropriate Raid Level
//
//************************************************************************
STATUS DdmRAIDMstr::
ChangePreferredMemberValidation(HANDLE h, RMSTR_CMND_INFO *_pCmdInfo)
{
	STATUS								status = RMSTR_SUCCESS;
	RMSTR_CMND_PARAMETERS				*pCmdParams = NULL;
	RMSTR_CHANGE_PREFERRED_MEMBER_INFO	*pChangePreferredMemberInfo = NULL;
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
	pChangePreferredMemberInfo	= 
		(RMSTR_CHANGE_PREFERRED_MEMBER_INFO *)&pCmdParams->changePreferredMemberInfo;

	// first get the ADT record
	GetRmstrData(
			RAID_ARRAY,
			&pChangePreferredMemberInfo->arrayRowId,
			(void **)&pADTRecord);
	GetRmstrData(
			RAID_MEMBER,
			&pChangePreferredMemberInfo->newMemberRowId,
			(void **)&pMember);

	if (pADTRecord && pMember) {
		// Resolve: Check if old member is actually preferred??
		if (pADTRecord->raidLevel != RAID1){
			status = RMSTR_ERR_INVALID_RAID_LEVEL;
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
	} else {
		ChangeThePreferredMember(
				pValidationContext->cmdHandle,
				pCmdInfo,
				pADTRecord,
				pMember);
	}
	return status;
}
