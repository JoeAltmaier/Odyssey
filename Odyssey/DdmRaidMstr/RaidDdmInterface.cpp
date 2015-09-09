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
// File: RaidDdmInterface.cpp
// 
// Description:
// Implementation for raid ddm cmnds and events
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RaidDdmInterface.cpp $
// 
// 24    11/03/99 3:32p Dpatel
// added tracing..
// 
// 23    10/15/99 11:48a Dpatel
// change to put adt rid in spare_activated evt..
// 
// 22    9/07/99 1:47p Dpatel
// Checked array and utility data during init to check PTS data
// consistency.
// 
// 21    8/31/99 6:39p Dpatel
// events, util abort processing etc.
// 
// 20    8/27/99 5:24p Dpatel
// added event code..
// 
// 19    8/24/99 5:13p Dpatel
// added failover code, create array changes for "array name"
// 
// 18    8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 17    8/14/99 1:37p Dpatel
// Added event logging..
// 
// 16    8/12/99 1:56p Dpatel
// Added Array offline event processing code.
// 
// 15    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 14    8/03/99 3:21p Jtaylor
// pADTRecord was not set properly for ADD member cmd reply
// 
// 13    8/03/99 10:15a Jtaylor
// added RMSTR_RAID_DDM test ifdefs
// 
// 12    8/02/99 3:01p Dpatel
// changes to create array, array FT processing...
// 
// 11    7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 10    7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 9     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 8     7/20/99 10:18a Dpatel
// pADTrecord was uniitialized before modify.
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
// 4     7/06/99 4:57p Dpatel
// fixed bugs found in the Utility testing process.
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



enum {
	evtUDT_RECORD_READ =1,
	evtUDT_RECORD_UPDATED
};



//************************************************************************
//	RaidCmdSenderInitializedReply
//		This is the reply for the Raid DDM Cmd Sender object initialization
//		completed. If the completion is successful then set our flag to 
//		indicate that our Cmd Sender is initialized successfully.
//
//************************************************************************
void DdmRAIDMstr
::RaidCmdSenderInitializedReply(STATUS status)
{
	if (status == OK) {
		m_RaidCmdSenderInitialized = true;
	} else {
		assert(0);
	}
}



//************************************************************************
//	RaidDdmCommandCompletionReply
//		This method is called whenever we receive a status for any cmd we
//		submitted to the Raid DDM.
//		We check if the cmd completed successfully or not. If cmd completed
//		successfully, then we report the status of the cmd to the Cmd Sender
//		for the RMSTR cmd. Also we need to check if any events need to be
//		generated for the cmd completion e.g change priority etc
//
//************************************************************************
void DdmRAIDMstr
::RaidDdmCommandCompletionReply(
			STATUS			completionCode,
			void			*pStatusData,
			void			*pCmdData,
			void			*_pRmstrCmdContext)
{
	CONTEXT			*pRmstrCmdContext = (CONTEXT *)_pRmstrCmdContext;
	RaidRequest		*pRaidRequest = (RaidRequest *)pCmdData;

	TRACEF(TRACE_L2,("\tRaidDdmCmdCompletionReply: Enter\n"));

	switch(completionCode){
		case RMSTR_SUCCESS:
			break;
		default:
			// Resolve:
			// Log that the cmd failed to start
			TRACEF(TRACE_L1, ("\tRaidDdmCmdCompletionReply: ERROR CODE = 0x%x\n", completionCode));		
			m_pCmdServer->csrvReportCmdStatus(
				pRmstrCmdContext->cmdHandle,	// handle
				completionCode,					// completion code
				NULL,							// result Data
				(void *)pRmstrCmdContext->pData);// Orig cmd info			
			delete pRmstrCmdContext;
			return;			
	}
	if (pRmstrCmdContext->cmdHandle) {
		// Report the status to cmdSender, that cmd 
		// was submitted
		// Event will be reported when the RAID DDM reports back
		TRACEF(TRACE_L2,("\tRaidDdmCmdCompletionReply: Reporting status back to cmd sender\n"));		
		m_pCmdServer->csrvReportCmdStatus(
				pRmstrCmdContext->cmdHandle,	// handle
				RMSTR_SUCCESS,					// completion code
				NULL,							// result Data
				(void *)pRmstrCmdContext->pData);// Orig cmd info
		// check if we need to generate any special events
		// e.g for change priority
		CheckForEventsToBeGeneratedOnCmdCompletion(
							pRmstrCmdContext);
		StopCommandProcessing(
			true,
			pRmstrCmdContext->cmdHandle);
	} else {
		assert(pRmstrCmdContext->cmdHandle != NULL);
	}

	if (pRmstrCmdContext) {
		delete pRmstrCmdContext;
		pRmstrCmdContext = NULL;
	}
}




//************************************************************************
//	RaidDdmEventHandler
//		This method is called whenever the Raid DDM generates an event.
//		submitted to the Raid DDM.
//		We process UTILITY events, ARRAY offline events etc
//
//************************************************************************
void DdmRAIDMstr
::RaidDdmEventHandler(
			STATUS			eventCode,
			void			*pStatusData)
{
	TRACEF_NF(TRACE_RMSTR,("\tRaidDdmEventHandler Enter:\n"));
	RaidEvent				*pRaidEvent = NULL;
	RaidMemberDownEvent		*pRaidMemberDownEvent = NULL;
	RaidOfflineEvent		*pRaidOfflineEvent = NULL;

	TRACEF(TRACE_L2,("\tRaidDdmEventHandler: Enter\n"));
	switch(eventCode){
	case RAID_EVT_UTIL_STARTED:
	case RAID_EVT_UTIL_STOPPED:
	case RAID_EVT_UTIL_PERCENT_COMPLETE:
		ProcessUtilityEvents(eventCode,pStatusData);
		break;

	case RAID_EVT_MEMBER_DOWN:
		pRaidEvent = (RaidEvent *)pStatusData;
		pRaidMemberDownEvent = &pRaidEvent->Event.MemberDown;
		TRACEF(TRACE_L2,("\tRaidDdmEventHandler: Member down event recvd\n"));		
		StartInternalProcessMemberDownEvent(
					&pRaidMemberDownEvent->ArrayRowID,
					&pRaidMemberDownEvent->MemberRowID,
					pRaidMemberDownEvent->Reason);
		break;

	case RAID_EVT_ARRAY_OFFLINE:
		pRaidEvent = (RaidEvent *)pStatusData;
		pRaidOfflineEvent = &pRaidEvent->Event.RaidOffline;
		TRACEF(TRACE_L2,("\tRaidDdmEventHandler: Array offline event recvd\n"));	
		StartInternalProcessArrayOfflineEvent(
					&pRaidOfflineEvent->ArrayRowID,
					&pRaidOfflineEvent->MemberRowID,
					pRaidOfflineEvent->Reason);
		break;

#ifdef RMSTR_RAID_DDM_TEST
	case RAID_EVT_ENABLED:
		// This is temporary. Raid DDM lets us know that it is enabled and that it
		// has populated the ADT, MDT tables, so we can read them
		InitializeRmstrData();
		break;
#endif		
	default:
		break;
	}
	return;
}


//************************************************************************
//	ProcessUtilityEvents
//		Update tables and generate RMSTR events for
//		RAID_EVT_UTIL_STOPPED	- send internal cmd	
//		RAID_EVT_UTIL_STARTED	- generate rmstr event
//		RAID_EVT_UTIL_%_COMPLETE- generate rmstr event and modify % complete
//	
//
//************************************************************************
void DdmRAIDMstr
::ProcessUtilityEvents(
			STATUS			eventCode,
			void			*pStatusData)
{
	RaidEvent					*pRaidEvent = NULL;
	RaidStartUtilEvent			*pRaidStartUtilEvent = NULL;	
	RaidStopUtilEvent			*pRaidStopUtilEvent = NULL;	
	RaidPercentEvent			*pRaidPercentEvent = NULL;	
	CONTEXT						*pEventContext = NULL;
	rowID						*pUtilRowId = NULL;

	if (eventCode == RAID_EVT_UTIL_STOPPED){
		// Issue an internal command to process this event
		// since it changes a lot of tables which could affect other
		// cmds..
		pRaidEvent = (RaidEvent *)pStatusData;
		pRaidStopUtilEvent = &pRaidEvent->Event.StopUtil;
		TRACEF(TRACE_L2,("\tRaidDdmEventHandler: Util stopped event recvd\n"));			
		StartInternalProcessStopUtilEvent(
						&pRaidStopUtilEvent->UtilRowID,
						pRaidStopUtilEvent->MiscompareCnt,
						pRaidStopUtilEvent->Reason);
	} else {
		pEventContext = new CONTEXT;
		memset(pEventContext,0,sizeof(CONTEXT));

		// save the Raid Event data
		pEventContext->value = eventCode;
		pEventContext->pData = new RaidEvent;
		pRaidEvent = (RaidEvent *)pEventContext->pData;
		memcpy(pRaidEvent,pStatusData,sizeof(RaidEvent));


		switch(eventCode){
		case RAID_EVT_UTIL_STARTED:
			// Read the utility row id to fill in the array row id
			pRaidStartUtilEvent = &pRaidEvent->Event.StartUtil;
			pUtilRowId = &pRaidStartUtilEvent->UtilRowID;
			TRACEF(TRACE_L2,("\tRaidDdmEventHandler: Util started event recvd\n"));			
			break;

		case RAID_EVT_UTIL_PERCENT_COMPLETE:
			pRaidPercentEvent = &pRaidEvent->Event.PercentUtil;
			pUtilRowId = &pRaidPercentEvent->UtilRowID;
			TRACEF(TRACE_L2,("\tRaidDdmEventHandler: Util % comp event recvd\n"));						
			break;

		default:
			return;
		}
		if (pUtilRowId){
			// allocate data for Reading UDT row
			pEventContext->pData1 = new RAID_ARRAY_UTILITY;
			pEventContext->state = evtUDT_RECORD_READ;

			// utility event, so read the utility row data
			m_pTableServices->TableServiceReadRow(
					RAID_UTIL_DESCRIPTOR_TABLE,		// tableName
					pUtilRowId,
					pEventContext->pData1,			// buffer to put data
					sizeof(RAID_ARRAY_UTILITY),
					(pTSCallback_t)&DdmRAIDMstr::ProcessUtilityEventsReply,
					pEventContext);
		}
	}
}


//************************************************************************
//	ProcessUtilityEventsReply
//		Update tables and generate RMSTR events for
//		RAID_EVT_UTIL_STOPPED	- send internal cmd	
//		RAID_EVT_UTIL_STARTED	- generate rmstr event
//		RAID_EVT_UTIL_%_COMPLETE- generate rmstr event and modify % complete
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessUtilityEventsReply(void *_pContext, STATUS status)
{
	CONTEXT						*pEventContext = NULL;
	RaidEvent					*pRaidEvent = NULL;
	RaidStartUtilEvent			*pRaidStartUtilEvent = NULL;
	RaidPercentEvent			*pRaidPercentEvent=NULL;	
	STATUS						eventCode;
	RAID_ARRAY_UTILITY			*pUtility = NULL;
	BOOL						eventComplete = false;


	if (status != OS_DETAIL_STATUS_SUCCESS){
		return status;
	}
	// RMSTR Event
	RMSTR_EVT_UTIL_STARTED_STATUS	*pEvtUtilStartedStatus;
	RMSTR_PERCENT_COMPLETE_STATUS	*pPercentCompleteStatus;

	pEventContext = (CONTEXT *)_pContext;

	// pEventContext->pData = RaidEvent
	// pEventContext->pData1 = UDT Record
	// pEventContext->pData2 = ADT Record (some states only)

	pRaidEvent = (RaidEvent *)pEventContext->pData;
	eventCode = pEventContext->value;

	pUtility = (RAID_ARRAY_UTILITY *)pEventContext->pData1;
	
	switch(pEventContext->state){
	case evtUDT_RECORD_READ:
		assert (pUtility != NULL);
		ModifyRmstrData(
			RAID_UTILITY,
			&pUtility->thisRID,
			pUtility);
		switch(eventCode){
			case RAID_EVT_UTIL_STARTED:
				eventComplete = true;
				break;

			case RAID_EVT_UTIL_PERCENT_COMPLETE:
				// Update the Util table to reflect the new %
				pRaidPercentEvent = &pRaidEvent->Event.PercentUtil;
				pEventContext->state = evtUDT_RECORD_UPDATED;
				pUtility->percentComplete = 
					pRaidPercentEvent->Percent;

				status = m_pTableServices->TableServiceModifyField(
							RAID_UTIL_DESCRIPTOR_TABLE,
							&pUtility->thisRID,	// row id to modify
							fdPERCENT_DONE,
							&pRaidPercentEvent->Percent,
							sizeof(U32),
							(pTSCallback_t)&DdmRAIDMstr::ProcessUtilityEventsReply,
							pEventContext);
				break;

			default:
				break;
		}
		break;

	case evtUDT_RECORD_UPDATED:
		// new percent in PTS
		ModifyRmstrData(
			RAID_UTILITY,
			&pUtility->thisRID,
			pUtility);

		eventComplete = true;
		break;

	default:
		break;
	}

	if (eventComplete){
		RAID_ARRAY_DESCRIPTOR	*pADTRecord = NULL;
		// Report the event
		switch(eventCode){
			case RAID_EVT_UTIL_STARTED:
				pRaidStartUtilEvent = &pRaidEvent->Event.StartUtil;
				// Generate event for Util Started
				pEvtUtilStartedStatus = new (tZERO) RMSTR_EVT_UTIL_STARTED_STATUS;
				pEvtUtilStartedStatus->utilityData = *pUtility;
				m_pCmdServer->csrvReportEvent(
						RMSTR_EVT_UTIL_STARTED,		// completion code
						pEvtUtilStartedStatus);		// result Data
				delete pEvtUtilStartedStatus;
				pEvtUtilStartedStatus = NULL;

				// Log the util started event
				GetRmstrData(
						RAID_ARRAY,
						&pUtility->targetRID,
						(void **)&pADTRecord);
				switch(pUtility->utilityCode){
					case RAID_UTIL_VERIFY:
						LogEventWithArrayName(
							CTS_RMSTR_VERIFY_STARTED, 
							&pADTRecord->SRCTRID);
						break;
					case RAID_UTIL_BKGD_INIT:
						LogEventWithArrayName(
							CTS_RMSTR_BKGD_INIT_STARTED, 
							&pADTRecord->SRCTRID);
						break;
					case RAID_UTIL_MEMBER_HOTCOPY:
					case RAID_UTIL_LUN_HOTCOPY:
						LogEventWithArrayName(
							CTS_RMSTR_HOTCOPY_STARTED,
							&pADTRecord->SRCTRID);
						break;
					case RAID_UTIL_REGENERATE:
						LogEventWithArrayName(
							CTS_RMSTR_REGENERATE_STARTED, 
							&pADTRecord->SRCTRID);
						break;
				}
				break;

			case RAID_EVT_UTIL_PERCENT_COMPLETE:
				pRaidPercentEvent = &pRaidEvent->Event.PercentUtil;
				pPercentCompleteStatus = new (tZERO) RMSTR_PERCENT_COMPLETE_STATUS;
				// fill in the event data
				pPercentCompleteStatus->utilityData = *pUtility;
				pPercentCompleteStatus->percentComplete = pRaidPercentEvent->Percent;
				m_pCmdServer->csrvReportEvent(
						RMSTR_EVT_UTIL_PERCENT_COMPLETE,	// completion code
						pPercentCompleteStatus);		// result Data
				delete pPercentCompleteStatus;
				pPercentCompleteStatus = NULL;
				break;

			default:
				break;
		}

		delete pEventContext;
		pEventContext = NULL;
	} // event complete
	return status;
}



//************************************************************************
//	CheckForEventsToBeGeneratedOnCmdCompletion
//		Generates any events that might be required (on cmd completion)
//			Change Priority
//			Add member
//			Commit Spare (internal cmd)
//
//************************************************************************
void DdmRAIDMstr::
CheckForEventsToBeGeneratedOnCmdCompletion(CONTEXT *pRmstrCmdContext)
{

	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RAID_ARRAY_UTILITY				*pUtility = NULL;
	RMSTR_PRIORITY_CHANGED_STATUS	*pEvtPriorityChanged = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RMSTR_EVT_MEMBER_ADDED_STATUS	*pEvtMemberAddedStatus = NULL;
	RMSTR_EVT_MEMBER_REMOVED_STATUS	*pEvtMemberRemovedStatus = NULL;
	RAID_ARRAY_MEMBER				*pMember = NULL;
	RMSTR_EVT_SPARE_ACTIVATED_STATUS	*pEvtSpareActivatedStatus = NULL;
	RAID_SPARE_DESCRIPTOR				*pSpare = NULL;
	RAID_SPARE_DESCRIPTOR				*pNextSpare = NULL;


	// pData contains cmdInfo (RMSTR_CHANGE_PRIORITY_INFO)
	// pData1  contains the UDT 
	// pData2 = contains ADT
	// pData3 = NULL
	pCmdInfo		= (RMSTR_CMND_INFO *)pRmstrCmdContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	

	switch(pCmdInfo->opcode){
		case RMSTR_CMND_CHANGE_UTIL_PRIORITY:
			pUtility = (RAID_ARRAY_UTILITY *)pRmstrCmdContext->pData1;

			pEvtPriorityChanged = new (tZERO) RMSTR_PRIORITY_CHANGED_STATUS;
			pEvtPriorityChanged->oldPriority = 
					(RAID_UTIL_PRIORITY)pRmstrCmdContext->value;
			pEvtPriorityChanged->utilityData = *pUtility;
			m_pCmdServer->csrvReportEvent(
					RMSTR_EVT_UTIL_PRIORITY_CHANGED,	// completion code
					pEvtPriorityChanged);		// result Data
			delete pEvtPriorityChanged;
			pEvtPriorityChanged = NULL;
			// Now log the event
			GetRmstrData(
				RAID_ARRAY,
				&pUtility->targetRID,
				(void **)&pADTRecord);
			if (pADTRecord){
				switch(pUtility->utilityCode){
					case RAID_UTIL_VERIFY:
						switch(pUtility->priority){
						case PRIORITY_HIGH:
							LogEventWithArrayName(
								CTS_RMSTR_VERIFY_PRIORITY_CHANGED_HIGH, 
								&pADTRecord->SRCTRID);
							break;
						case PRIORITY_LOW:
							LogEventWithArrayName(
								CTS_RMSTR_VERIFY_PRIORITY_CHANGED_LOW,
								&pADTRecord->SRCTRID);
							break;
						case PRIORITY_MEDIUM:
							LogEventWithArrayName(
								CTS_RMSTR_VERIFY_PRIORITY_CHANGED_MEDIUM, 
								&pADTRecord->SRCTRID);
							break;
						}
						break;
					case RAID_UTIL_MEMBER_HOTCOPY:
					case RAID_UTIL_LUN_HOTCOPY:
						switch(pUtility->priority){
						case PRIORITY_HIGH:
							LogEventWithArrayName(
								CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_HIGH,
								&pADTRecord->SRCTRID);
							break;
						case PRIORITY_LOW:
							LogEventWithArrayName(
								CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_LOW,
								&pADTRecord->SRCTRID);
							break;
						case PRIORITY_MEDIUM:
							LogEventWithArrayName(
								CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_MEDIUM,
								&pADTRecord->SRCTRID);
							break;
						}
						break;
					case RAID_UTIL_REGENERATE:
						switch(pUtility->priority){
						case PRIORITY_HIGH:
							LogEventWithArrayName(
								CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_HIGH,
								&pADTRecord->SRCTRID);
							break;
						case PRIORITY_LOW:
							LogEventWithArrayName(
								CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_LOW,
								&pADTRecord->SRCTRID);
							break;
						case PRIORITY_MEDIUM:
							LogEventWithArrayName(
								CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_MEDIUM,
								&pADTRecord->SRCTRID);
							break;
						}
						break;
				}	// end switch utility code
			} // end pADTRecord
			break;

		case RMSTR_INTERNAL_CMND_COMMIT_SPARE:
			pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pRmstrCmdContext->pData1;
			pSpare = (RAID_SPARE_DESCRIPTOR *)pRmstrCmdContext->pData2;

			
			// Generate Event for spare activated
			pEvtSpareActivatedStatus = new RMSTR_EVT_SPARE_ACTIVATED_STATUS;
			pEvtSpareActivatedStatus->spareData = *pSpare;
			pEvtSpareActivatedStatus->spareData.arrayRID = pADTRecord->thisRID;
			m_pCmdServer->csrvReportEvent(
				RMSTR_EVT_SPARE_ACTIVATED,		// event Code
				pEvtSpareActivatedStatus);		// event Data
			delete pEvtSpareActivatedStatus;
			pEvtSpareActivatedStatus = NULL;
			
			// Start the regenerate...(Resolve: Specify src/dest instead of default)
			// Resolve: should stopcmdprocessing with false
			StartInternalRegenerate(pADTRecord);

			// Log the spare activated event
			LogSpareActivatedEvent(pADTRecord,pSpare);
			break;

		case RMSTR_CMND_ADD_MEMBER:
			pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pRmstrCmdContext->pData1;		
			pMember = (RAID_ARRAY_MEMBER *)pRmstrCmdContext->pData3;
			// Generate the add member event
			pEvtMemberAddedStatus = new (tZERO) RMSTR_EVT_MEMBER_ADDED_STATUS;
			pEvtMemberAddedStatus->memberData = *pMember;
			m_pCmdServer->csrvReportEvent(
				RMSTR_EVT_MEMBER_ADDED,			// event Code
				pEvtMemberAddedStatus);			// event Data
			delete pEvtMemberAddedStatus;
			pEvtMemberAddedStatus = NULL;

			LogMemberEvent(
				CTS_RMSTR_MEMBER_ADDED,
				pADTRecord,
				pMember);

			// Resolve: should stop cmd processing with false
			StartInternalRegenerate(pADTRecord);
			break;

		case RMSTR_CMND_REMOVE_MEMBER:
			// Resolve:
			// This should be handled under RAID_EVT...
			// Also when to remove from ADT, MDT..??
			pMember = (RAID_ARRAY_MEMBER *)pRmstrCmdContext->pData1;
			// Generate the remove member event
			pEvtMemberRemovedStatus = new (tZERO) RMSTR_EVT_MEMBER_REMOVED_STATUS;
			pEvtMemberRemovedStatus->memberData = *pMember;
			m_pCmdServer->csrvReportEvent(
				RMSTR_EVT_MEMBER_REMOVED,			// event Code
				pEvtMemberRemovedStatus);			// event Data
			delete pEvtMemberRemovedStatus;
			//Resolve: (pADTRecord)
			//LogEventWithArrayName(
			//	CTS_RMSTR_MEMBER_REMOVED, 
			//	pADTRecord->SRCTRID);
			break;

		default:
			break;
	}
	// Dont need to delete, the context since it will be deleted
	// by the CmdHandler.
}



