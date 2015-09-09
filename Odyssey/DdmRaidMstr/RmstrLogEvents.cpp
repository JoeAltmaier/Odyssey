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
// File: RmstrLogEvents.cpp
// 
// Description:
//	implementation of various logging routines
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrLogEvents.cpp $
// 
// 4     9/03/99 10:01a Dpatel
// Remitting alarms...
// 
// 3     9/01/99 6:38p Dpatel
// added logging and alarm code..
// 
// 2     8/31/99 6:39p Dpatel
// events, util abort processing etc.
// 
// 1     8/27/99 5:23p Dpatel
// Initial creation.
// 
// 
//
/*************************************************************************/


#include "DdmRaidMgmt.h"



//************************************************************************
//	LogSpareAddedEvent
//		Logs the spare added event, with arrayName and spareName 
//		(if available), else logs the spare unique id (e.g disks)
//
//************************************************************************
void DdmRAIDMstr::
LogSpareAddedEvent(CONTEXT *pCmdContext, RAID_SPARE_DESCRIPTOR *pSpare)
{
	StringClass					scArrayName;
	StringClass					scSpareName;
	Event						*pEvent = NULL;

	if (pCmdContext->ucArrayName.GetLength()){
		pCmdContext->ucArrayName.GetAsciiString(scArrayName);
	} 
	if (pCmdContext->ucMemberName.GetLength()){
		// Log event with name of member
		pCmdContext->ucMemberName.GetAsciiString(scSpareName);
		switch(pSpare->spareType){
			case RAID_DEDICATED_SPARE:
				LogEvent(
					CTS_RMSTR_DEDICATED_SPARE_ADDED,
					scSpareName.CString(),
					scArrayName.CString());
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<Dedicated Spare %s added to array %s.>>\n", scSpareName.CString(), scArrayName.CString()));
				break;

			case RAID_HOST_POOL_SPARE:
				LogEvent(
					CTS_RMSTR_HOST_POOL_SPARE_ADDED, 
					scSpareName.CString());
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<Host Pool Spare %s added to Host %s.>>\n", scSpareName.CString(), "???"));
				break;

			case RAID_GENERAL_POOL_SPARE:
				LogEvent(
					CTS_RMSTR_SYSTEM_POOL_SPARE_ADDED,
					scSpareName.CString());
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<System Pool Spare %s added.>>\n", scSpareName.CString()));
				break;
			}
	} else {
		// Log event with unique id (for disks without names)
		switch(pSpare->spareType){
			case RAID_DEDICATED_SPARE:
				LogEvent(
					CTS_RMSTR_DEDICATED_SPARE_DISK_ADDED,
					pCmdContext->SlotID,
					scArrayName.CString());
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<Dedicated Spare [Slot=%d] added to array %s.>>\n", 
							pCmdContext->SlotID,scArrayName.CString()));
				break;

			case RAID_HOST_POOL_SPARE:
				LogEvent(
					CTS_RMSTR_HOST_POOL_SPARE_DISK_ADDED, 
					pCmdContext->SlotID);
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<Host Pool Spare [Slot=%d] added to Host %s.>>\n", 
							pCmdContext->SlotID, "???"));
				break;

			case RAID_GENERAL_POOL_SPARE:
				LogEvent(
					CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_ADDED,
					pCmdContext->SlotID);
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<System Pool Spare [Slot=%d] added.>>\n", 
							pCmdContext->SlotID));
				break;
			}
	}
}



//************************************************************************
//	LogSpareDeletedEvent
//		Logs the spare deleted event, with arrayName and spareName 
//		(if available), else logs the spare unique id (e.g disks)
//
//************************************************************************
void DdmRAIDMstr::
LogSpareDeletedEvent(
			CONTEXT					*pCmdContext, 
			RAID_SPARE_DESCRIPTOR	*pSpare)
{
	StringClass					scArrayName;
	StringClass					scSpareName;
	Event						*pEvent = NULL;

	if (pCmdContext->ucArrayName.GetLength()){
		pCmdContext->ucArrayName.GetAsciiString(scArrayName);
	} 
	if (pCmdContext->ucMemberName.GetLength()){
		// Log event with name of member
		pCmdContext->ucMemberName.GetAsciiString(scSpareName);
		switch(pSpare->spareType){
			case RAID_DEDICATED_SPARE:
				LogEvent(
					CTS_RMSTR_DEDICATED_SPARE_DELETED, 
					scSpareName.CString(),
					scArrayName.CString());
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<Dedicated Spare %s deleted from array %s.>>\n", scSpareName.CString(), scArrayName.CString()));
				break;
			case RAID_HOST_POOL_SPARE:
				LogEvent(
					CTS_RMSTR_HOST_POOL_SPARE_DELETED, 
					scSpareName.CString(),
					"???");
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<Host Pool Spare %s deleted from host.>>\n", scSpareName.CString(), "???"));
				break;
			case RAID_GENERAL_POOL_SPARE:
				LogEvent(
					CTS_RMSTR_SYSTEM_POOL_SPARE_DELETED,
					scSpareName.CString());
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<System Pool Spare %s deleted.>>\n", scSpareName.CString()));
				break;
		}
	} else {
		switch(pSpare->spareType){
			case RAID_DEDICATED_SPARE:
				LogEvent(
					CTS_RMSTR_DEDICATED_SPARE_DISK_DELETED, 
					pCmdContext->SlotID,
					scArrayName.CString());
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<Dedicated Spare [Slot=%d] deleted from array %s.>>\n", 
							pCmdContext->SlotID, scArrayName.CString()));
				break;
			case RAID_HOST_POOL_SPARE:
				LogEvent(
					CTS_RMSTR_HOST_POOL_SPARE_DISK_DELETED, 
					pCmdContext->SlotID);
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<Host Pool Spare [Slot=%d] deleted from Host %s.>>\n", 
							pCmdContext->SlotID, "???"));
				break;
			case RAID_GENERAL_POOL_SPARE:
				LogEvent(
					CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_DELETED,
					pCmdContext->SlotID);
				TRACEF_NF(
					TRACE_RMSTR,
					("\nEvent <<System Pool Spare [Slot=%d] deleted.>>\n", 
							pCmdContext->SlotID));
				break;
		}
	}
}




enum {
	LOG_EVT_ARRAY_NAME_READ = 1,
	LOG_EVT_SPARE_NAME_READ,
	LOG_EVT_MEMBER_NAME_READ
};
//************************************************************************
//	LogSpareActivatedEvent
//		Dedicated Spare:
//			Check if any other spare is protecting this array, 
//			if not log event
//		Pool Spare:
//			Traverse all arrays, Check if any other pool spare can 
//			protect the array. If not log event.
//
//************************************************************************
void DdmRAIDMstr::
LogSpareActivatedEvent(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_SPARE_DESCRIPTOR		*pSpare)
{
	CONTEXT						*pContext = NULL;
	pContext = new CONTEXT;
	pContext->state = LOG_EVT_ARRAY_NAME_READ;
	pContext->newRowId = pADTRecord->SRCTRID;
	pContext->pData = new RAID_ARRAY_DESCRIPTOR;
	pContext->pData1 = new RAID_SPARE_DESCRIPTOR;
	memcpy(pContext->pData, pADTRecord, sizeof(RAID_ARRAY_DESCRIPTOR));
	memcpy(pContext->pData1, pSpare, sizeof(RAID_SPARE_DESCRIPTOR));
	m_pHelperServices->ReadStorageElementName(
					&pContext->ucArrayName,
					NULL,
					&pContext->newRowId,
					TSCALLBACK(DdmRAIDMstr,ProcessLogSpareActivatedEventReply),
					pContext);
}


//************************************************************************
//	ProcessLogSpareActivatedEventReply
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessLogSpareActivatedEventReply(
		void					*_pContext, 
		STATUS					status)
{
	CONTEXT						*pContext = (CONTEXT *)_pContext;
	StringClass					scArrayName;
	StringClass					scSpareName;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	RAID_SPARE_DESCRIPTOR		*pSpare = NULL;
	BOOL						cmdComplete = false;

	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pContext->pData;
	pSpare = (RAID_SPARE_DESCRIPTOR *)pContext->pData1;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		cmdComplete = true;
	} else {
		switch(pContext->state){
		case LOG_EVT_ARRAY_NAME_READ:
			pContext->state = LOG_EVT_SPARE_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
							&pContext->ucMemberName,
							&pContext->SlotID,
							&pSpare->SRCTRID,
							TSCALLBACK(DdmRAIDMstr,ProcessLogSpareActivatedEventReply),
							pContext);
			break;

		case LOG_EVT_SPARE_NAME_READ:
			// Log Event:
			pContext->ucArrayName.GetAsciiString(scArrayName);			
			if (pContext->ucMemberName.GetLength()){
				// Log event with name of spare
				pContext->ucMemberName.GetAsciiString(scSpareName);
				switch(pSpare->spareType){
					case RAID_DEDICATED_SPARE:
						LogEvent(
							CTS_RMSTR_DEDICATED_SPARE_ACTIVATED, 
							scSpareName.CString(),
							scArrayName.CString());
						TRACEF_NF(
							TRACE_RMSTR,
							("\nEvent <<Dedicated Spare %s used to regenerate array %s.>>\n", scSpareName.CString(), scArrayName.CString()));
						break;
					case RAID_HOST_POOL_SPARE:
						LogEvent(
							CTS_RMSTR_HOST_POOL_SPARE_ACTIVATED, 
							scSpareName.CString(),
							"???",
							scArrayName.CString());
						TRACEF_NF(
							TRACE_RMSTR,
							("\nEvent <<Host Pool Spare %s used to regenerate array %s.>>\n", scSpareName.CString(), scArrayName.CString()));
						break;
					case RAID_GENERAL_POOL_SPARE:
						LogEvent(
							CTS_RMSTR_SYSTEM_POOL_SPARE_ACTIVATED,
							scSpareName.CString(),
							scArrayName.CString());
						TRACEF_NF(
							TRACE_RMSTR,
							("\nEvent <<Pool Spare %s used to regenerate array %s.>>\n", scSpareName.CString(), scArrayName.CString()));
						break;
				}
			} else {
				switch(pSpare->spareType){
					case RAID_DEDICATED_SPARE:
						LogEvent(
							CTS_RMSTR_DEDICATED_SPARE_DISK_ACTIVATED, 
							pContext->SlotID,
							scArrayName.CString());
						TRACEF_NF(
							TRACE_RMSTR,
							("\nEvent <<Dedicated Spare [Slot=%d] used to regenerate array %s.>>\n", 
									pContext->SlotID, scArrayName.CString()));
						break;
					case RAID_HOST_POOL_SPARE:
						LogEvent(
							CTS_RMSTR_HOST_POOL_SPARE_DISK_ACTIVATED, 
							pContext->SlotID,
							scArrayName.CString());
						TRACEF_NF(
							TRACE_RMSTR,
							("\nEvent <<Host Pool Spare [Slot=%d] used to regenerate array %s.>>\n", 
									pContext->SlotID, scArrayName.CString()));
						break;
					case RAID_GENERAL_POOL_SPARE:
						LogEvent(
							CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_ACTIVATED,
							pContext->SlotID,
							scArrayName.CString());
						TRACEF_NF(
							TRACE_RMSTR,
							("\nEvent <<Pool Spare [Slot=%d] used to regenerate array %s.>>\n", 
									pContext->SlotID, scArrayName.CString()));
						break;
				}
			}
			LogNoMoreSparesEvent(pADTRecord, pSpare);
			cmdComplete = true;
			break;

		default:
			assert(0);
		}
	}
	if (cmdComplete == true){
		delete pContext;
	}
	return status;
}



//************************************************************************
//	LogNoMoreSparesEvent
//		Dedicated Spare:
//			Check if any other spare is protecting this array, 
//			if not log event
//		Pool Spare:
//			Traverse all arrays, Check if any other pool spare can 
//			protect the array. If not log event.
//
//************************************************************************
void DdmRAIDMstr::
LogNoMoreSparesEvent(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_SPARE_DESCRIPTOR		*pSpare)
{
	RAID_ARRAY_DESCRIPTOR		*pArrayThatFits = NULL;
	RAID_SPARE_DESCRIPTOR		*pNextSpare = NULL;


	if (pSpare->spareType == RAID_DEDICATED_SPARE){
		if (pADTRecord){
			GetValidSpare(
				pADTRecord,
				&pNextSpare);
			if (pNextSpare){
					// Array is protected by some other spare
			} else {
				LogEventWithArrayName(
					CTS_RMSTR_NO_MORE_SPARES,
					&pADTRecord->SRCTRID);
			}
		}
	} else {
		// Start with NULL to get first array,
		// else finds the next array after pArrayThatFits
		TraverseArrayThatFitsForThisSpare(
					pSpare,
					&pArrayThatFits);
		while(pArrayThatFits){
				GetValidSpare(
						pArrayThatFits,
						&pNextSpare);
				if (pNextSpare){
						// Array is protected by some other spare
				} else {
					LogEventWithArrayName(
						CTS_RMSTR_NO_MORE_SPARES,
						&pArrayThatFits->SRCTRID);
				}
				TraverseArrayThatFitsForThisSpare(
					pSpare,
					&pArrayThatFits);
		}
	}
}





//************************************************************************
//	LogEventWithArrayName
//		Only 1 argument for event, i.e just the name of any array
//		This method will not handle cases where name is NULL.
//
//************************************************************************
void DdmRAIDMstr::
LogEventWithArrayName(
		U32					eventCode,
		rowID				*pSRCRowId)
{
	assert(eventCode);
	assert(pSRCRowId);

	CONTEXT *pContext = new CONTEXT;
	pContext->newRowId = *pSRCRowId;
	pContext->value = eventCode;
	m_pHelperServices->ReadStorageElementName(
					&pContext->ucArrayName,
					NULL,
					&pContext->newRowId,
					TSCALLBACK(DdmRAIDMstr,ProcessLogEventWithArrayNameReply),
					pContext);
}


//************************************************************************
//	ProcessLogEventWithArrayNameReply
//		After array name is read, log the event.
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessLogEventWithArrayNameReply(
		void					*_pContext, 
		STATUS					status)
{
	CONTEXT					*pContext = (CONTEXT *)_pContext;
	StringClass				scArrayName;
	U32						eventCode = 0;

	eventCode = pContext->value;
	pContext->ucArrayName.GetAsciiString(scArrayName);
	LogEvent(eventCode, scArrayName.CString());

	// Log Event for tracing:			
	switch(eventCode){
		// Array related event
		case CTS_RMSTR_ARRAY_ADDED:
			TRACEF_NF(TRACE_RMSTR,("\n<<Event: New Array %s created successfully!!>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_ARRAY_CRITICAL:
			TRACEF_NF(TRACE_RMSTR,("\n<<Event: (CRITICAL) Array %s is Critical!!>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_ARRAY_FAULT_TOLERANT:
			TRACEF_NF(TRACE_RMSTR,("\n<<Event: Array %s is now fault tolerant!!>>\n\n", scArrayName.CString()));
			break;

		// util related events				
		case CTS_RMSTR_REGENERATE_NOT_STARTED:
			TRACEF_NF(TRACE_RMSTR,("\n<<Event: No Valid Spare Found. Could not start Regenerate on array %s!!>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_NO_MORE_SPARES:
			TRACEF_NF(TRACE_RMSTR,("\nEvent: <<Warning: No More Valid Spares Available for array %s.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_VERIFY_STARTED:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Verify started on array %s.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_BKGD_INIT_STARTED:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Background Init started on array %s.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_HOTCOPY_STARTED:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Hot Copy started on array %s.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_REGENERATE_STARTED:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Regenerate started on array %s.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_VERIFY_PRIORITY_CHANGED_HIGH:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Priority for Verify running on array %s changed to high.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_VERIFY_PRIORITY_CHANGED_LOW:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Priority for Verify running on array %s changed to low.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_VERIFY_PRIORITY_CHANGED_MEDIUM:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Priority for Verify running on array %s changed to medium.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_HIGH:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Priority for Hot copy running on array %s changed to high.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_LOW:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Priority for Hot copy running on array %s changed to low.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_MEDIUM:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Priority for Hot copy running on array %s changed to medium.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_HIGH:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Priority for Regenerate running on array %s changed to high.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_LOW:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Priority for Regenerate running on array %s changed to low.>>\n\n", scArrayName.CString()));
			break;
		case CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_MEDIUM:
			TRACEF_NF(TRACE_RMSTR,
				("\nEvent: <<Priority for Regenerate running on array %s changed to medium.>>\n\n", scArrayName.CString()));
			break;


		default:
			assert(0);
	}
	delete pContext;
	return status;
}




//************************************************************************
//	LogMemberEvent
//		Used for logging events which have member and array 
//		name as the parameters
//
//************************************************************************
void DdmRAIDMstr::
LogMemberEvent(
		U32							eventCode,
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_ARRAY_MEMBER			*pMember)
{
	assert(eventCode);
	assert(pADTRecord);
	assert(pMember);

	CONTEXT						*pContext = NULL;
	pContext = new CONTEXT;
	pContext->state = LOG_EVT_ARRAY_NAME_READ;
	pContext->newRowId = pADTRecord->SRCTRID;
	pContext->pData = new RAID_ARRAY_DESCRIPTOR;
	pContext->pData1 = new RAID_ARRAY_MEMBER;
	pContext->value = eventCode;

	memcpy(pContext->pData, pADTRecord, sizeof(RAID_ARRAY_DESCRIPTOR));
	memcpy(pContext->pData1, pMember, sizeof(RAID_ARRAY_MEMBER));
	m_pHelperServices->ReadStorageElementName(
					&pContext->ucArrayName,
					NULL,
					&pContext->newRowId,
					TSCALLBACK(DdmRAIDMstr,ProcessLogMemberEventReply),
					pContext);
}


//************************************************************************
//	ProcessLogMemberEventReply
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessLogMemberEventReply(
		void					*_pContext, 
		STATUS					status)
{
	CONTEXT						*pContext = (CONTEXT *)_pContext;
	StringClass					scArrayName;
	StringClass					scMemberName;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	RAID_ARRAY_MEMBER			*pMember = NULL;
	BOOL						cmdComplete = false;
	U32							eventCode = 0;

	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pContext->pData;
	pMember = (RAID_ARRAY_MEMBER *)pContext->pData1;
	eventCode = pContext->value;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		cmdComplete = true;
	} else {
		switch(pContext->state){
		case LOG_EVT_ARRAY_NAME_READ:
			pContext->state = LOG_EVT_MEMBER_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
							&pContext->ucMemberName,
							&pContext->SlotID,
							&pMember->memberRID,
							TSCALLBACK(DdmRAIDMstr,ProcessLogMemberEventReply),
							pContext);
			break;

		case LOG_EVT_MEMBER_NAME_READ:
			// Log Event:
			pContext->ucArrayName.GetAsciiString(scArrayName);
			if (pContext->ucMemberName.GetLength()){
				// Log event with name of member
				pContext->ucMemberName.GetAsciiString(scMemberName);
				switch(eventCode){
				case CTS_RMSTR_MEMBER_ADDED:
					LogEvent(
						CTS_RMSTR_MEMBER_ADDED, 
						scArrayName.CString(),
						scMemberName.CString());
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Member %2 added to array %1.>>\n", 
						scArrayName.CString(), 
						scMemberName.CString()));
					break;

				case CTS_RMSTR_MEMBER_REMOVED:
					LogEvent(
						CTS_RMSTR_MEMBER_REMOVED, 
						scArrayName.CString(),
						scMemberName.CString());
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Member %2 removed from array %1.>>\n", 
						scArrayName.CString(), 
						scMemberName.CString()));
					break;


				case CTS_RMSTR_MEMBER_DOWN_BY_USER:
					LogEvent(
						CTS_RMSTR_MEMBER_DOWN_BY_USER, 
						scArrayName.CString(),
						scMemberName.CString());
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Member %2 on array %1 is downed by user!.>>\n", 
						scArrayName.CString(), 
						scMemberName.CString()));
					break;

				case CTS_RMSTR_MEMBER_DOWN_IOERROR:
					LogEvent(
						CTS_RMSTR_MEMBER_DOWN_IOERROR, 
						scArrayName.CString(),
						scMemberName.CString());
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Member %2 on array %1 is down due to I/O error!.>>\n",
						scArrayName.CString(), 
						scMemberName.CString()));
					break;

				case CTS_RMSTR_SOURCE_MEMBER_CHANGED:
					LogEvent(
						CTS_RMSTR_SOURCE_MEMBER_CHANGED, 
						scArrayName.CString(),
						scMemberName.CString());
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Array %s has its source member changed to %s.>>\n", scArrayName.CString(), scMemberName.CString()));
					break;

				case CTS_RMSTR_PREFERRED_MEMBER_CHANGED:
					LogEvent(
						CTS_RMSTR_PREFERRED_MEMBER_CHANGED, 
						scArrayName.CString(),
						scMemberName.CString());
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Array %s has its preferred member changed to %s.>>\n", scArrayName.CString(), scMemberName.CString()));
					break;
				default:
					assert(0);
				}
			} else {
				switch(eventCode){
				case CTS_RMSTR_MEMBER_ADDED:
					LogEvent(
						CTS_RMSTR_MEMBER_ADDED, 
						scArrayName.CString(),
						pContext->SlotID);
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Member [Slot=%d] added to array %s.>>\n", 
						pContext->SlotID,
						scArrayName.CString()));
					break;

				case CTS_RMSTR_MEMBER_REMOVED:
					LogEvent(
						CTS_RMSTR_MEMBER_REMOVED, 
						scArrayName.CString(),
						pContext->SlotID);
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Member [Slot=%d] removed from array %s.>>\n", 
						pContext->SlotID,
						scArrayName.CString()));
					break;

				case CTS_RMSTR_MEMBER_DOWN_BY_USER:
					LogEvent(
						CTS_RMSTR_MEMBER_DISK_DOWN_BY_USER, 
						scArrayName.CString(),
						pContext->SlotID);
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Member [Slot=%d] on array %s is downed by user!.>>\n", 
						pContext->SlotID,
						scArrayName.CString()));
					break;

				case CTS_RMSTR_MEMBER_DOWN_IOERROR:
					LogEvent(
						CTS_RMSTR_MEMBER_DISK_DOWN_IOERROR, 
						scArrayName.CString(),
						pContext->SlotID);
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Member [Slot=%d] on array %s is down due to I/O error!.>>\n",
						pContext->SlotID,
						scArrayName.CString()));
					break;

				case CTS_RMSTR_SOURCE_MEMBER_CHANGED:
					LogEvent(
						CTS_RMSTR_SOURCE_MEMBER_DISK_CHANGED, 
						scArrayName.CString(),
						pContext->SlotID);
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Array %s has its source member changed to [Slot=%d].>>\n",
						scArrayName.CString(), 
						pContext->SlotID));
					break;

				case CTS_RMSTR_PREFERRED_MEMBER_CHANGED:
					LogEvent(
						CTS_RMSTR_PREFERRED_MEMBER_DISK_CHANGED, 
						scArrayName.CString(),
						pContext->SlotID);
					TRACEF_NF(
						TRACE_RMSTR,
						("\nEvent <<Array %s has its preferred member changed to [Slot=%d].>>\n",
						scArrayName.CString(), 
						pContext->SlotID));
					break;
				default:
					assert(0);
				}
			}
			cmdComplete = true;
			break;

		default:
			assert(0);
		}
	}
	if (cmdComplete == true){
		delete pContext;
	}
	return status;
}





