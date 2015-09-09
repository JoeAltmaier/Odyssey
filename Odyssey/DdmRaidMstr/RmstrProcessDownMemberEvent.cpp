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
// File: RmstrProcessDownMemberEvent.cpp
// 
// Description:
// Implementation for Down A Member Command
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrProcessDownMemberEvent.cpp $
// 
// 17    2/11/00 5:26p Dpatel
// Unnecessary log message was being logged when a drive went down.
// (DOWN_BY_USER)
// 
// 16    10/19/99 4:13p Dpatel
// generated member removed event instead of down, since we are removing
// member always...
// 
// 15    10/19/99 4:11p Dpatel
// 
// 14    9/09/99 1:38p Dpatel
// removed ENABLE_LOGGING ifdef...
// 
// 13    9/07/99 1:47p Dpatel
// Checked array and utility data during init to check PTS data
// consistency.
// 
// 12    9/03/99 10:01a Dpatel
// Remitting alarms...
// 
// 11    9/01/99 6:38p Dpatel
// added logging and alarm code..
// 
// 10    8/31/99 6:39p Dpatel
// events, util abort processing etc.
// 
// 9     8/27/99 6:42p Dpatel
// start util now takes target row id as the SRC instead of ADT row id
// 
// 8     8/27/99 5:24p Dpatel
// added event code..
// 
// 7     8/16/99 7:05p Dpatel
// Changes for alarms + using rowID * as handle instead of void*
// 
// 6     8/14/99 1:37p Dpatel
// Added event logging..
// 
// 5     8/12/99 1:56p Dpatel
// Added Array offline event processing code.
// 
// 4     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 3     8/03/99 6:22p Dpatel
// Check on member down, if source member down then change the source
// 
// 2     8/02/99 3:57p Dpatel
// Array Critical event was generated before ADT update
// 
// 1     7/30/99 6:48p Dpatel
// Initial Creation.
// 
// 
// 
//
/*************************************************************************/

#include "DdmRaidMgmt.h"

enum {
	MEMBER_DOWN_MDT_RECORD_READ = 1,
	MEMBER_DOWN_ARRAY_CRITICAL_CHECK_DONE
};

//************************************************************************
//	ProcessDownMemberEvent
//		This is an internal command. It is started whenever we rcv a down
//		member event from the RAID DDM.
//
//	h			- handle for the cmd
//	pCmdInfo	- cmd packet for down member event
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessDownMemberEvent(HANDLE h, RMSTR_CMND_INFO *_pCmdInfo)
{
	CONTEXT							*pCmdContext = NULL;
	STATUS							status;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;

	RAID_ARRAY_DESCRIPTOR			*_pADTRecord = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER				*_pMDTRecord = NULL;
	RAID_ARRAY_MEMBER				*pMDTRecord = NULL;
	RMSTR_PROCESS_MEMBER_DOWN_INFO	*pProcessMemberDownInfo = NULL;


	pCmdContext = new CONTEXT;
	pCmdContext->cmdHandle	= h;

	// save info in context
	pCmdContext->pData = new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));
	pCmdInfo = (RMSTR_CMND_INFO *)pCmdContext->pData;

	pCmdParams		= &pCmdInfo->cmdParams;
	pProcessMemberDownInfo = 
			(RMSTR_PROCESS_MEMBER_DOWN_INFO *)&pCmdParams->processMemberDownInfo;

	// Read the ADT
	GetRmstrData(
		RAID_ARRAY,
		&pProcessMemberDownInfo->arrayRowId,
		(void **)&_pADTRecord);

	pCmdContext->pData1 = new RAID_ARRAY_DESCRIPTOR;
	memcpy(pCmdContext->pData1, _pADTRecord, sizeof(RAID_ARRAY_DESCRIPTOR));
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;

	pCmdContext->state = MEMBER_DOWN_MDT_RECORD_READ;
	// Allocate space for read row data
	pCmdContext->pData2 = new RAID_ARRAY_MEMBER;
	memset(pCmdContext->pData2,0,sizeof(RAID_ARRAY_MEMBER));
	// Read the MDT Record, since RAID DDM will have updated
	// the PTS with member health
	status = m_pTableServices->TableServiceReadRow(
					RAID_MEMBER_DESCRIPTOR_TABLE,
					&pProcessMemberDownInfo->memberRowId,
					pCmdContext->pData2,
					sizeof(RAID_ARRAY_MEMBER),
					(pTSCallback_t)&DdmRAIDMstr::ProcessMemberDownEventReply,
					pCmdContext);
	return status;
}
	


//************************************************************************
//	ProcessDownMemberEventReply
//		- Read the down member record
//		- Check if its the primary/source member, if so then chg the primary
//		to next up member.
//		- Generate event for member down.
//		- Check if array state critical, modify ADT and generate event
//		- Check for any valid spares and commit spare for the down member
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessMemberDownEventReply(void *_pContext, STATUS status)
{
	CONTEXT							*pMemberDownContext = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;

	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER				*pMember = NULL;
	RMSTR_EVT_MEMBER_REMOVED_STATUS	*pEvtMemberRemovedStatus = NULL;
	BOOL							eventComplete = false;
	BOOL							isArrayCritical = false;
	RMSTR_PROCESS_MEMBER_DOWN_INFO	*pProcessMemberDownInfo = NULL;


	pMemberDownContext = (CONTEXT *)_pContext;
	pCmdInfo = (RMSTR_CMND_INFO *)pMemberDownContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pProcessMemberDownInfo = 
			(RMSTR_PROCESS_MEMBER_DOWN_INFO *)&pCmdParams->processMemberDownInfo;


	pADTRecord = (RAID_ARRAY_DESCRIPTOR *) pMemberDownContext->pData1;
	pMember = (RAID_ARRAY_MEMBER *)pMemberDownContext->pData2;

	// we should not be getting member down event for raid 0
	assert(pADTRecord->raidLevel != RAID0);

	if (status != OS_DETAIL_STATUS_SUCCESS){
		eventComplete = true;
	} else {
		switch(pMemberDownContext->state){
		case MEMBER_DOWN_MDT_RECORD_READ:
			// Resolve: what if array state changes to Offline,
			// multiple member down events..Regenerate failed to start..

			ModifyRmstrData(
				RAID_MEMBER,
				&pMember->thisRID,
				pMember);
			// If primary member (RAID 1 only) was down, then
			// find next up member and make it primary..
			if (pADTRecord->raidLevel == RAID1){
				if (pMember->policy.SourcePrimary == 1){
					StartInternalChangeSourceMember(pADTRecord);
				}
			}

			// Generate the member removed event, since we are removing
			// all down members
			pEvtMemberRemovedStatus = new RMSTR_EVT_MEMBER_REMOVED_STATUS;
			pEvtMemberRemovedStatus->memberData = *pMember;
			m_pCmdServer->csrvReportEvent(
				RMSTR_EVT_MEMBER_REMOVED,			// event Code
				pEvtMemberRemovedStatus);			// event Data
			delete pEvtMemberRemovedStatus;
			pEvtMemberRemovedStatus = NULL;

			// Resolve: Reason for down member
			switch(pProcessMemberDownInfo->reason){
				default:
				// Now there is no member down by user, so only due to i/o err
				LogMemberEvent(
					CTS_RMSTR_MEMBER_DOWN_IOERROR, 
					pADTRecord,
					pMember);
				break;
			}


			pMemberDownContext->state = MEMBER_DOWN_ARRAY_CRITICAL_CHECK_DONE;

			// will check if array state is critical,
			// does not generate event
			isArrayCritical = CheckIfArrayCritical(
									pADTRecord,
									false);
			if (isArrayCritical){
				// Modify Array State
				pADTRecord->health = RAID_CRITICAL;
				m_pTableServices->TableServiceModifyRow(
						RAID_ARRAY_DESCRIPTOR_TABLE,
						&pADTRecord->thisRID,	// row id to modify
						pADTRecord,
						sizeof(RAID_ARRAY_DESCRIPTOR),
						&pADTRecord->thisRID,
						(pTSCallback_t)&DdmRAIDMstr::ProcessMemberDownEventReply,
						pMemberDownContext);
			} else {
				ProcessMemberDownEventReply(pMemberDownContext,OK);
			}

			break;

		case MEMBER_DOWN_ARRAY_CRITICAL_CHECK_DONE:
			// Modify the data, even if it did not change.
			ModifyRmstrData(
				RAID_ARRAY,
				&pADTRecord->thisRID,
				(void **)pADTRecord);
			// Check again, since ADT should have been updated now
			// Generate event if array was indeed critical!!!
			CheckIfArrayCritical(
						pADTRecord,
						true);
			// Commit the first valid spare, even if array not crtical
			CommitFirstValidSpare(pADTRecord, pMember);
			eventComplete = true;
			break;

		default:
			break;
		}
	}
	if (eventComplete){
#if 0
		if (SimulateFailover(pMemberDownContext)){
			return status;
		}
#endif
		// Report status to Cmd Sender
		m_pCmdServer->csrvReportCmdStatus(
				pMemberDownContext->cmdHandle,	// handle
				status,					// completion code
				NULL,					// result Data
				(void *)pCmdInfo);		// pCmdInfo
		StopCommandProcessing(true, pMemberDownContext->cmdHandle);
		delete pMemberDownContext;
		pMemberDownContext = NULL;
	}
	return status;
}


//************************************************************************
//	CheckIfArrayCritical
//		Checks the current state of any array. If array is critical
//		then returns TRUE, else returns FALSE.
//	
//	pADTRecord		- the array whose state is to be checked
//	generateEvent	- if array is critical, generate an event
//
//************************************************************************
BOOL DdmRAIDMstr::
CheckIfArrayCritical(
		RAID_ARRAY_DESCRIPTOR			*pADTRecord,
		BOOL							generateEvent)
{
	STATUS					isArrayCritical = false;
	RAID_ARRAY_MEMBER		*pMember = NULL;
	U32						i=0;
	U32						upMembers = 0;


	if (pADTRecord->raidLevel != RAID0){
		switch(pADTRecord->health){
			case RAID_CRITICAL:
				// array was already critical
				isArrayCritical = true;
				break;

			case RAID_FAULT_TOLERANT:
				switch(pADTRecord->raidLevel){
					case RAID1:
						for (i=0; i < pADTRecord->numberMembers; i++){
							GetRmstrData(
									RAID_MEMBER,
									&pADTRecord->members[i],
									(void **)&pMember);
							if (pMember){
								if (pMember->memberHealth == RAID_STATUS_UP){
									upMembers++;
								}
							}
						}
						if (upMembers == 1){
							isArrayCritical = true;
						}
						break;

					case RAID5:
						assert(0);
						break;

					default:
						assert(0);
						break;
				}
			default:
				break;
		}
	}
	if (isArrayCritical){
		if (generateEvent){
			// Generate Array Critical Event
			RMSTR_EVT_ARRAY_CRITICAL_STATUS *pEvtArrayCriticalStatus = 
							new RMSTR_EVT_ARRAY_CRITICAL_STATUS;
			pEvtArrayCriticalStatus->arrayData = *pADTRecord;
			m_pCmdServer->csrvReportEvent(
					RMSTR_EVT_ARRAY_CRITICAL,
					pEvtArrayCriticalStatus);		// event Data
			delete pEvtArrayCriticalStatus;
			pEvtArrayCriticalStatus = NULL;

			RmstrSubmitAlarm(
				CTS_RMSTR_ARRAY_CRITICAL,
				RAID_ARRAY,
				&pADTRecord->thisRID);

			LogEventWithArrayName(
				CTS_RMSTR_ARRAY_CRITICAL, 
				&pADTRecord->SRCTRID);

		}
	}
	return isArrayCritical;
}


//************************************************************************
//	CheckIfArrayOffline
//		Checks the current state of any array. If array is offline
//		then returns TRUE, else returns FALSE.
//	
//	pADTRecord		- the array whose state is to be checked
//
//************************************************************************
BOOL DdmRAIDMstr::
CheckIfArrayOffline(RAID_ARRAY_DESCRIPTOR	*pADTRecord)
{
	STATUS					isArrayOffline = false;
	RAID_ARRAY_MEMBER		*pMember = NULL;
	U32						i=0;
	U32						upMembers = 0;


	if (pADTRecord->raidLevel != RAID0){
		switch(pADTRecord->health){
			case RAID_OFFLINE:
				// array was already Offline
				isArrayOffline = true;
				break;

			case RAID_FAULT_TOLERANT:
			case RAID_CRITICAL:
				switch(pADTRecord->raidLevel){
					case RAID1:
						for (i=0; i < pADTRecord->numberMembers; i++){
							GetRmstrData(
									RAID_MEMBER,
									&pADTRecord->members[i],
									(void **)&pMember);
							if (pMember){
								if (pMember->memberHealth == RAID_STATUS_UP){
									upMembers++;
								}
							}
						}
						if (upMembers == 0){
							isArrayOffline = true;
						}
						break;

					case RAID5:
						assert(0);
						break;

					default:
						assert(0);
						break;
				}
			default:
				break;
		}
	}
	return isArrayOffline;
}

