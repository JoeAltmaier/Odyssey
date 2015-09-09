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
// File: RmstrDownAMember.cpp
// 
// Description:
// Implementation for Down A Member Command
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrDownAMember.cpp $
// 
// 11    8/24/99 5:13p Dpatel
// added failover code, create array changes for "array name"
// 
// 10    8/14/99 1:37p Dpatel
// Added event logging..
// 
// 9     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 8     7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 7     7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 6     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 5     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 4     7/20/99 10:18a Dpatel
// Fixes for Down Member
// 
// 3     7/17/99 1:20p Dpatel
// Queued up commands.
// 
// 2     7/16/99 10:28a Dpatel
// Added DownMember and Commit Spare code. Also removed the reads for
// validation.
// 
// 1     7/09/99 5:29p Dpatel
// Initial Creation
// 
// 
//
/*************************************************************************/

#include "DdmRaidMgmt.h"



//************************************************************************
//	DownAMember
//		Prepare a Raid Request and send it to the Raid DDM.
//		Rmstr will generate the down event and process the down member
//		when we receive an event from the Raid DDM.
//
//	handle		- Handle for the cmd
//	_pCmdInfo	- cmd packet for down a member info
//	_pADTRecord	- the array to which the member belongs
//	_pMDTRecord	- the member which is to be taken down
//
//************************************************************************
STATUS DdmRAIDMstr::
DownAMember(
		HANDLE					handle,
		RMSTR_CMND_INFO			*_pCmdInfo,
		RAID_ARRAY_DESCRIPTOR	*_pADTRecord,
		RAID_ARRAY_MEMBER		*_pMDTRecord)
{
	CONTEXT							*pCmdContext = NULL;
	STATUS							status;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER				*pMDTRecord = NULL;
	RaidRequest						*pRaidRequest = NULL;
	RaidDownMemberStruct			*pRaidDownMemberStruct = NULL;


	pCmdContext = new CONTEXT;
	pCmdContext->cmdHandle	= handle;

	// save info in context
	pCmdContext->pData = new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));

	pCmdContext->pData1 = new RAID_ARRAY_DESCRIPTOR;
	memcpy(pCmdContext->pData1, _pADTRecord, sizeof(RAID_ARRAY_DESCRIPTOR));

	pCmdContext->pData2 = new RAID_ARRAY_MEMBER;
	memcpy(pCmdContext->pData2, _pMDTRecord, sizeof(RAID_ARRAY_MEMBER));

	

	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;
	pMDTRecord = (RAID_ARRAY_MEMBER *)pCmdContext->pData2;

	//Prepare a RAID_DDM_CMD for Member down
	pRaidRequest = new RaidRequest;
	pRaidDownMemberStruct = new RaidDownMemberStruct;
	memset(pRaidRequest,0,sizeof(RaidRequest));
	memset(pRaidDownMemberStruct,0,sizeof(RaidDownMemberStruct));

	pRaidRequest->Opcode = 	RAID_REQUEST_DOWN_MEMBER;
	pRaidRequest->RaidVDN = pADTRecord->arrayVDN;

	pRaidDownMemberStruct->MemberRowID = pMDTRecord->thisRID;
	memcpy(
		&pRaidRequest->Data.DownData,
		pRaidDownMemberStruct,
		sizeof(RaidDownMemberStruct));		
	//status = m_pRaidCmdSender->csndrExecute(
	// Execute cmd only if cmd is not executed..
	status = m_pRaidCmdSender->csndrCheckAndExecute(
		pRaidRequest,
		(pCmdCompletionCallback_t)&DdmRAIDMstr::RaidDdmCommandCompletionReply,
		pCmdContext);
	delete pRaidRequest;
	delete pRaidDownMemberStruct;	
	return status;
}




//************************************************************************
//	DownAMemberValidation
//		Check array state, if member down is allowed or not. Make sure
//		that the member to be downed is currently up.
//
//************************************************************************
STATUS DdmRAIDMstr::
DownAMemberValidation(HANDLE handle, RMSTR_CMND_INFO *pCmdInfo)
{
	STATUS						status = RMSTR_SUCCESS;
	RMSTR_DOWN_A_MEMBER_INFO	*pDownAMemberInfo = NULL;
	RMSTR_CMND_PARAMETERS		*pCmdParams = NULL;
	void						*pRowData = NULL;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	RAID_ARRAY_MEMBER			*pMember = NULL;

	pCmdParams		= &pCmdInfo->cmdParams;
	pDownAMemberInfo = 
			(RMSTR_DOWN_A_MEMBER_INFO *)&pCmdParams->downAMemberInfo;

	// Read the ADT
	GetRmstrData(
		RAID_ARRAY,
		&pDownAMemberInfo->arrayRowId,
		&pRowData);
	if (pRowData){
		// Allocate space for read row data
		pADTRecord = new RAID_ARRAY_DESCRIPTOR;
		memcpy(pADTRecord,pRowData,sizeof(RAID_ARRAY_DESCRIPTOR));

		switch(pADTRecord->health){
			case RAID_CRITICAL:
				status = RMSTR_ERR_ARRAY_CRITICAL;
				break;

			case RAID_OFFLINE:
				status = RMSTR_ERR_ARRAY_OFFLINE;
				break;

			default:
				// read the MDT to make sure member is up!
				GetRmstrData(
					RAID_MEMBER,
					&pDownAMemberInfo->memberRowId,
					&pRowData);
				if (pRowData){
					pMember	= new RAID_ARRAY_MEMBER;
					memcpy(pMember,pRowData,sizeof(RAID_ARRAY_MEMBER));

					if (pMember->memberHealth == RAID_STATUS_DOWN){
						status = RMSTR_ERR_MEMBER_ALREADY_DOWN;
					}
					if (pMember->memberHealth == RAID_STATUS_REGENERATING){
						status = RMSTR_ERR_UTIL_RUNNING;
					}
					if (status == RMSTR_SUCCESS){
						// Proceed to Down the member
						DownAMember(
							handle,
							pCmdInfo,
							pADTRecord,
							pMember);
					}
				} else {
					status = RMSTR_ERR_INVALID_COMMAND;
				}
				break;
		}
	} else {
		status = RMSTR_ERR_INVALID_COMMAND;
	}
	if(status){
			// Report error to Cmd Sender
			m_pCmdServer->csrvReportCmdStatus(
				handle,					// handle
				status,					// completion code
				NULL,					// result Data
				(void *)pCmdInfo);		// pCmdInfo
			StopCommandProcessing(true, handle);
	}
	if (pADTRecord){
		delete pADTRecord;
	}
	if (pMember){
		delete pMember;
	}

	return status;
}












