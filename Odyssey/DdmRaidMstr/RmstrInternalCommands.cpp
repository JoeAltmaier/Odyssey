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
// File: RmstrInternalCommands.cpp
// 
// Description:
// Implementation for the abort utility command
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrInternalCommands.cpp $
// 
// 14    12/17/99 5:33p Dpatel
// added hot copy code
// 
// 13    9/07/99 7:31p Dpatel
// 
// 12    9/07/99 2:00p Dpatel
// 
// 11    8/31/99 6:39p Dpatel
// events, util abort processing etc.
// 
// 10    8/27/99 6:42p Dpatel
// start util now takes target row id as the SRC instead of ADT row id
// 
// 9     8/12/99 1:56p Dpatel
// Added Array offline event processing code.
// 
// 8     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 7     8/05/99 2:39p Dpatel
// remove warning..
// 
// 6     8/05/99 11:07a Dpatel
// internal delete array, fake raid ddm, hot copy auto break, removed
// array name code..
// 
// 5     8/03/99 6:22p Dpatel
// Check on member down, if source member down then change the source
// 
// 4     8/02/99 3:01p Dpatel
// changes to create array, array FT processing...
// 
// 3     7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 2     7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 1     7/23/99 5:57p Dpatel
// Initial creation
// 
//
/*************************************************************************/

#include "DdmRaidMgmt.h"

//************************************************************************
//	StartInternalDownMember
//
//************************************************************************
STATUS DdmRAIDMstr::
StartInternalDownMember(RAID_ARRAY_DESCRIPTOR	*pADTRecord)
{
	
	RMSTR_CMND_INFO				*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS		*pCmdParams = NULL;
	RMSTR_DOWN_A_MEMBER_INFO	*pDownMemberInfo = NULL;


	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_DOWN_A_MEMBER;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pDownMemberInfo = 
			(RMSTR_DOWN_A_MEMBER_INFO *) (&pCmdParams->downAMemberInfo);

	pDownMemberInfo->arrayRowId = pADTRecord->thisRID;	
	pDownMemberInfo->memberRowId = pADTRecord->members[0];
	STATUS status = m_pInternalCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstr::rmstrInternalCmdCompletionReply,
		NULL);
	delete pCmdInfo;
	return status;
}	


//************************************************************************
//	StartInternalHotCopy
//		Start an internal cmd for the hot copy operation. The
//		hot copy is to be run on default src / destination members i.e
//		the "designated source" as our source and all down members as the
//		destination.
//
//	pADTRecord		- the array to start the hot copy on
//	priority		- the priority of the hot copy util
//
//************************************************************************
STATUS DdmRAIDMstr::
StartInternalHotCopy(
		RAID_ARRAY_DESCRIPTOR			*pADTRecord,
		RAID_UTIL_PRIORITY				priority)
{
	
	RMSTR_CMND_INFO				*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS		*pCmdParams = NULL;
	RMSTR_START_UTIL_INFO		*pStartUtilInfo = NULL;
	RAID_UTIL_POLICIES			utilPolicy;


	pCmdInfo = new(tZERO) RMSTR_CMND_INFO;

	pCmdInfo->opcode = RMSTR_INTERNAL_CMND_START_UTIL;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pStartUtilInfo = 
			(RMSTR_START_UTIL_INFO *) (&pCmdParams->startUtilInfo);

	pStartUtilInfo->targetRowId = pADTRecord->SRCTRID;	// target ADT row id

	pStartUtilInfo->utilityName = RAID_UTIL_LUN_HOTCOPY;
	pStartUtilInfo->priority = priority;
	pStartUtilInfo->updateRate = 1;

	memset(&utilPolicy, 0, sizeof(RAID_UTIL_POLICIES));


	STATUS status = m_pInternalCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstr::rmstrInternalCmdCompletionReply,
		NULL);
	delete pCmdInfo;
	return status;
}	



//************************************************************************
//	StartInternalBkgdInit
//		Start an internal cmd for the bkgd init operation. The
//		init is to be run on default src / destination members i.e
//		the "designated source" as our source and all other up members as
//		destination.
//
//	pADTRecord		- the array to start the init on
//
//************************************************************************
STATUS DdmRAIDMstr::
StartInternalBkgdInit(RAID_ARRAY_DESCRIPTOR *pADTRecord)
{
	
	RMSTR_CMND_INFO				*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS		*pCmdParams = NULL;
	RMSTR_START_UTIL_INFO		*pStartUtilInfo = NULL;
	RAID_UTIL_POLICIES			utilPolicy;


	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_INTERNAL_CMND_START_UTIL;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pStartUtilInfo = 
			(RMSTR_START_UTIL_INFO *) (&pCmdParams->startUtilInfo);

	//pStartUtilInfo->targetRowId = pADTRecord->thisRID;	// target ADT row id
	pStartUtilInfo->targetRowId = pADTRecord->SRCTRID;	// target ADT row id

	pStartUtilInfo->utilityName = RAID_UTIL_BKGD_INIT;
	pStartUtilInfo->priority = PRIORITY_MEDIUM;
	pStartUtilInfo->updateRate = 5;

	memset(&utilPolicy, 0, sizeof(RAID_UTIL_POLICIES));

	STATUS status = m_pInternalCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstr::rmstrInternalCmdCompletionReply,
		NULL);
	delete pCmdInfo;
	return status;
}	



//************************************************************************
//	StartInternalRegenerate
//		Start an internal cmd for the regenerate operation. The
//		regenerate is to be run on default src / destination members i.e
//		the "designated source" as our source and all down members as the
//		destination.
//
//	pADTRecord		- the array to start the regenerate on
//
//************************************************************************
STATUS DdmRAIDMstr::
StartInternalRegenerate(RAID_ARRAY_DESCRIPTOR *pADTRecord)
{
	
	RMSTR_CMND_INFO				*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS		*pCmdParams = NULL;
	RMSTR_START_UTIL_INFO		*pStartUtilInfo = NULL;
	RAID_UTIL_POLICIES			utilPolicy;


	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_INTERNAL_CMND_START_UTIL;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pStartUtilInfo = 
			(RMSTR_START_UTIL_INFO *) (&pCmdParams->startUtilInfo);

	//pStartUtilInfo->targetRowId = pADTRecord->thisRID;	// target ADT row id
	pStartUtilInfo->targetRowId = pADTRecord->SRCTRID;	// target ADT row id

	pStartUtilInfo->utilityName = RAID_UTIL_REGENERATE;
	pStartUtilInfo->priority = PRIORITY_HIGH;
	pStartUtilInfo->updateRate = 5;

	memset(&utilPolicy, 0, sizeof(RAID_UTIL_POLICIES));

	STATUS status = m_pInternalCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstr::rmstrInternalCmdCompletionReply,
		NULL);
	delete pCmdInfo;
	return status;
}	





//************************************************************************
//	StartInternalDeleteArray
//		This routine currently just supports deleting an array
//		(RAID 1) which was created for hot copy. The delete policy
//		is set to break hot copy mirror and the export member (which was
//		set during create) is filled in the deleteArrayInfo.
//
//	pADTRecord		- the array to delete
//	breakHotCopy	- true (break hot copy), false - not supported currently
//	useSourceAsExport - true (always export source) - used when
//						hotcopy is aborted by user
//
//************************************************************************
STATUS DdmRAIDMstr::
StartInternalDeleteArray(
		RAID_ARRAY_DESCRIPTOR				*pADTRecord,
		BOOL								breakHotCopy,
		BOOL								useSourceAsExport)
{
	
	RMSTR_CMND_INFO				*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS		*pCmdParams = NULL;
	RMSTR_DELETE_ARRAY_INFO		*pDeleteArrayInfo = NULL;
	RAID_DELETE_POLICIES		deletePolicy;
	RAID_ARRAY_MEMBER			*pMember = NULL;
	BOOL						found = false;

	assert(breakHotCopy == true);

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_INTERNAL_CMND_DELETE_ARRAY;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pDeleteArrayInfo = 
			(RMSTR_DELETE_ARRAY_INFO *) (&pCmdParams->deleteArrayInfo);

	memset(&deletePolicy, 0, sizeof(RAID_DELETE_POLICIES));

	if (breakHotCopy){
		deletePolicy.BreakHotCopyMirror = 1;
		// find the export member row Id, fill it in the deleteInfo
		for (U32 i=0; i < pADTRecord->numberMembers; i++){
			GetRmstrData(
				RAID_MEMBER,
				&pADTRecord->members[i],
				(void **)&pMember);
			if (pMember){
				if (useSourceAsExport){
					if (pMember->policy.SourcePrimary){
						found = true;
						break;
					} else {
					}
				} else {
					if (pMember->policy.HotCopyExportMember){
						found = true;
						break;
					}
				}
			}
		}
	}

	if (found){
		pDeleteArrayInfo->arrayRowId = pADTRecord->thisRID;	
		pDeleteArrayInfo->hotCopyExportMemberRowId = pMember->thisRID;
		pDeleteArrayInfo->policy = deletePolicy;	

		STATUS status = m_pInternalCmdSender->csndrExecute(
			pCmdInfo,
			(pCmdCompletionCallback_t)&DdmRAIDMstr::rmstrInternalCmdCompletionReply,
			NULL);
	}
	delete pCmdInfo;
	return found;
}	


//************************************************************************
//	StartInternalCommitSpare
//		Start an internal command to commit spare. 
//
//	pSpare			- the spare to be committed.
//	pADTRecord		- the array to commit the spare to
//	pMember			- the member to replace with the spare
//
//************************************************************************
void DdmRAIDMstr::
StartInternalCommitSpare(
			RAID_SPARE_DESCRIPTOR		*pSpare,
			RAID_ARRAY_DESCRIPTOR		*pADTRecord,
			RAID_ARRAY_MEMBER			*pMember)
{
	RMSTR_CMND_INFO				*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS		*pCmdParams = NULL;
	RMSTR_COMMIT_SPARE_INFO		*pCommitSpareInfo = NULL;


	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_INTERNAL_CMND_COMMIT_SPARE;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pCommitSpareInfo = 
			(RMSTR_COMMIT_SPARE_INFO *) (&pCmdParams->commitSpareInfo);

	pCommitSpareInfo->arrayRowId = pADTRecord->thisRID;	// target ADT row id
	pCommitSpareInfo->spareRowId = pSpare->thisRID;	// target ADT row id
	pCommitSpareInfo->memberRowId = pMember->thisRID;	// target ADT row id

	STATUS status = m_pInternalCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstr::rmstrInternalCmdCompletionReply,
		NULL);
	delete pCmdInfo;
}



//************************************************************************
//	StartInternalProcessMemberDownEvent
//		Start an internal command to process the member down event
//
//	pArrayRowId		- the array on which the member went down
//	pMemberRowId	- the member which went down
//	reason			- the reason for the member to go down (user or I/O err)
//
//************************************************************************
void DdmRAIDMstr
::StartInternalProcessMemberDownEvent(
			rowID			*pArrayRowId,
			rowID			*pMemberRowId,
			U32				reason)
{
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_PROCESS_MEMBER_DOWN_INFO	*pProcessMemberDownInfo = NULL;


	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_INTERNAL_CMND_PROCESS_MEMBER_DOWN_EVENT;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pProcessMemberDownInfo = 
			(RMSTR_PROCESS_MEMBER_DOWN_INFO *) (&pCmdParams->processMemberDownInfo);

	pProcessMemberDownInfo->arrayRowId = *pArrayRowId;
	pProcessMemberDownInfo->memberRowId = *pMemberRowId;
	pProcessMemberDownInfo->reason = reason;

	STATUS status = m_pInternalCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstr::rmstrInternalCmdCompletionReply,
		NULL);
	delete pCmdInfo;
}




//************************************************************************
//	StartInternalProcessArrayOfflineEvent
//		Start an internal command to process the array offline event
//
//	pArrayRowId		- the array which went offline
//	pMemberRowId	- the member which went down
//	reason			- the reason for the member to go down (user or I/O err)
//
//************************************************************************
void DdmRAIDMstr
::StartInternalProcessArrayOfflineEvent(
			rowID			*pArrayRowId,
			rowID			*pMemberRowId,
			U32				reason)
{
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_PROCESS_ARRAY_OFFLINE_INFO	*pProcessArrayOfflineInfo = NULL;


	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_INTERNAL_CMND_PROCESS_ARRAY_OFFLINE_EVENT;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pProcessArrayOfflineInfo = 
			(RMSTR_PROCESS_ARRAY_OFFLINE_INFO *) (&pCmdParams->processArrayOfflineInfo);

	pProcessArrayOfflineInfo->arrayRowId = *pArrayRowId;
	pProcessArrayOfflineInfo->memberRowId = *pMemberRowId;
	pProcessArrayOfflineInfo->reason = reason;

	STATUS status = m_pInternalCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstr::rmstrInternalCmdCompletionReply,
		NULL);
	delete pCmdInfo;
}



//************************************************************************
//	StartInternalProcessStopUtilEvent
//		Start an internal command to process the util stopped event
//
//	pUtilRowId		- the util which was stopped
//	miscompareCount	- valid only for verify stopped/aborted
//	reason			- the reason for util stopped (I/O error, user abort)
//
//************************************************************************
void DdmRAIDMstr
::StartInternalProcessStopUtilEvent(
				rowID				*pUtilRowId,
				U32					miscompareCount,
				RAID_UTIL_STATUS	reason)
{
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_PROCESS_STOP_UTIL_EVENT_INFO	*pProcessStopUtilEventInfo = NULL;


	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_INTERNAL_CMND_PROCESS_STOP_UTIL_EVENT;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pProcessStopUtilEventInfo = 
			(RMSTR_PROCESS_STOP_UTIL_EVENT_INFO *) (&pCmdParams->processStopUtilEventInfo);


	pProcessStopUtilEventInfo->utilRowId = *pUtilRowId;
	pProcessStopUtilEventInfo->miscompareCount = miscompareCount;
	pProcessStopUtilEventInfo->reason = reason;

	STATUS status = m_pInternalCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstr::rmstrInternalCmdCompletionReply,
		NULL);
	delete pCmdInfo;
}


//************************************************************************
//	StartInternalChangeSourceMember
//		Start an internal command to change the source member. This method
//		is called when a member which was the source member goes down. So
//		we find the next "UP" member and assign it as source.
//
//	pADTRecord			- the array whose source member is to be changed
//
//************************************************************************
STATUS DdmRAIDMstr
::StartInternalChangeSourceMember(RAID_ARRAY_DESCRIPTOR	*pADTRecord)
{
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_CHANGE_SOURCE_MEMBER_INFO	*pChangeSourceMemberInfo = NULL;
	RAID_ARRAY_MEMBER				*pMember = NULL;
	BOOL							found = false;

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = 	RMSTR_INTERNAL_CMND_CHANGE_SOURCE_MEMBER;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pChangeSourceMemberInfo = 
			(RMSTR_CHANGE_SOURCE_MEMBER_INFO *) (&pCmdParams->changeSourceMemberInfo);

	pChangeSourceMemberInfo->arrayRowId = pADTRecord->thisRID;
	// find the first member that is not source and is UP..
	for (U32 i=0; i < pADTRecord->numberMembers; i++){
		if (i == pADTRecord->sourceMemberIndex){
			continue;
		}
		GetRmstrData(
			RAID_MEMBER,
			&pADTRecord->members[i],
			(void **)&pMember);
		if (pMember){
			if (pMember->memberHealth == RAID_STATUS_UP){
				found = true;
				break;
			}
		}
	}
	if (found){
		pChangeSourceMemberInfo->newMemberRowId = pMember->thisRID;
		STATUS status = m_pInternalCmdSender->csndrExecute(
			pCmdInfo,
			(pCmdCompletionCallback_t)&DdmRAIDMstr::rmstrInternalCmdCompletionReply,
			NULL);
	}
	delete pCmdInfo;
	return found;
}


//************************************************************************
//	rmstrInternalCmdCompletionReply
//		This is the callback when our internal cmd sender sends a
//		command to the Raid Master. When the Rmstr reports the cmd status
//		this callback gets called. We dont need to do any processing here.
//	Note: We also dont register for any events, since we dont need to do any
//	processing on events.
//
//************************************************************************
void DdmRAIDMstr::
rmstrInternalCmdCompletionReply (
			STATUS			completionCode,
			void			*pResultData,
			void			*pCmdData,
			void			*pCmdContext)
{
	RMSTR_CMND_INFO				*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS		*pCmdParams = NULL;
	RMSTR_START_UTIL_INFO		*pStartUtilInfo = NULL;


	pCmdInfo = (RMSTR_CMND_INFO *)pCmdData; 
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);

	if (completionCode == RMSTR_SUCCESS){
		switch (pCmdInfo->opcode){
		case RMSTR_INTERNAL_CMND_START_UTIL:
			pStartUtilInfo = 
				(RMSTR_START_UTIL_INFO *) (&pCmdParams->startUtilInfo);			
			break;
		default:
			break;
		}
	} else {
	}
}