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
// File: RmstrProcessStopUtilEvent.cpp
// 
// Description:
// Implemenation for processing a stop utility event
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrProcessStopUtilEvent.cpp $
// 
// 13    9/07/99 3:40p Dpatel
// removed member hot copy...
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
// 8     8/24/99 5:13p Dpatel
// added failover code, create array changes for "array name"
// 
// 7     8/14/99 1:37p Dpatel
// Added event logging..
// 
// 6     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 5     8/06/99 2:07p Dpatel
// handled init/hc complete processing..
// 
// 4     8/05/99 11:07a Dpatel
// internal delete array, fake raid ddm, hot copy auto break, removed
// array name code..
// 
// 3     8/02/99 3:57p Dpatel
// Array Critical event was generated before ADT update
// 
// 2     8/02/99 3:01p Dpatel
// changes to create array, array FT processing...
// 
// 1     7/30/99 6:49p Dpatel
// Initial Creation.
// 
// 
// 
//
/*************************************************************************/

#include "DdmRaidMgmt.h"


enum {
	STOP_UTIL_EVENT_UDT_RECORD_READ = 1,
	STOP_UTIL_EVENT_ADT_RECORD_UPDATED,
	STOP_UTIL_EVENT_UDT_RECORD_DELETED,
	STOP_UTIL_EVENT_MDT_HEALTH_UPDATED,
	STOP_UTIL_EVENT_MDT_RECORDS_UPDATED,
	STOP_UTIL_EVENT_ADT_FAULT_TOLERANT,
	STOP_UTIL_EVENT_ADT_INIT_STATUS_UPDATED,
	STOP_UTIL_ARRAY_NAME_READ
};


//************************************************************************
//	ProcessStopUtilEvent
//		This is an internal command. It is started whenever we rcv a 
//		UTIL_STOPPED event from the RAID DDM.
//
//	h			- handle for the cmd
//	pCmdInfo	- cmd packet for process stop util event
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessStopUtilEvent(HANDLE h, RMSTR_CMND_INFO *_pCmdInfo)
{
	CONTEXT							*pEventContext = NULL;
	STATUS							status = RMSTR_SUCCESS;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;

	RAID_ARRAY_UTILITY				*pUtility = NULL;

	RMSTR_PROCESS_STOP_UTIL_EVENT_INFO	*pProcessStopUtilEventInfo = NULL;


	pEventContext = new CONTEXT;
	pEventContext->cmdHandle	= h;

	// save info in context
	pEventContext->pData = new(tZERO) RMSTR_CMND_INFO;
	memcpy(pEventContext->pData, _pCmdInfo, sizeof(RMSTR_CMND_INFO));
	pCmdInfo = (RMSTR_CMND_INFO *)pEventContext->pData;

	pCmdParams		= &pCmdInfo->cmdParams;
	pProcessStopUtilEventInfo = 
				(RMSTR_PROCESS_STOP_UTIL_EVENT_INFO *)&pCmdParams->processStopUtilEventInfo;
	// allocate data for Reading UDT row
	pEventContext->pData1 = new(tZERO) RAID_ARRAY_UTILITY;


	// first check against our local copy,
	// if the util exists or not (failover)
	pUtility = (RAID_ARRAY_UTILITY *)pEventContext->pData1;
	GetRmstrData(
			RAID_UTILITY,
			&pProcessStopUtilEventInfo->utilRowId,
			(void **)pUtility);
	if (pUtility){
		pEventContext->state = STOP_UTIL_EVENT_UDT_RECORD_READ;
		// utility event, so read the utility row data
		status = m_pTableServices->TableServiceReadRow(
						RAID_UTIL_DESCRIPTOR_TABLE,		// tableName
						&pProcessStopUtilEventInfo->utilRowId,
						pEventContext->pData1,			// buffer to put data
						sizeof(RAID_ARRAY_UTILITY),
						TSCALLBACK(DdmRAIDMstr,ProcessStopUtilEventReply),
						pEventContext);
	} else {
		// utility was already deleted, so
		// Report the status back to CmdSender
		m_pCmdServer->csrvReportCmdStatus(
				h,					// handle
				status,				// completion code
				NULL,				// result Data
				(void *)_pCmdInfo);	// pCmdInfo
		StopCommandProcessing(true, h);
		delete pEventContext;
	}
	return status;
}


//************************************************************************
//	ProcessStopUtilEventReply
//		- Read the UDT record (we need it for generating event)
//		- Modify the ADT to remove the util row id from array
//		- Check and update the array and init state (if it needs to be changed) 
//		depending on which util stopped (INIT, HOT COPY or REGENERATE)
//		- Delete the UDT record
//		- Generate/Log event for Util stopped
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessStopUtilEventReply(void *_pContext, STATUS status)
{
	CONTEXT						*pEventContext = NULL;
	RAID_ARRAY_UTILITY			*pUtility = NULL;
	void						*pRowData = NULL;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	RMSTR_CMND_INFO				*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS		*pCmdParams = NULL;
	RMSTR_PROCESS_STOP_UTIL_EVENT_INFO	*pProcessStopUtilEventInfo = NULL;
	STATUS						rc = RMSTR_SUCCESS;
	BOOL						eventComplete = false;

	// RMSTR Event
	RMSTR_EVT_UTIL_STOPPED_STATUS	*pEvtUtilStoppedStatus;

	pEventContext = (CONTEXT *)_pContext;

	// pEventContext->pData = Cmd Info
	// pEventContext->pData1 = UDT Record
	// pEventContext->pData2 = ADT Record (some states only)

	pCmdInfo = (RMSTR_CMND_INFO *)pEventContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pProcessStopUtilEventInfo = 
			(RMSTR_PROCESS_STOP_UTIL_EVENT_INFO *)&pCmdParams->processStopUtilEventInfo;


	pUtility = (RAID_ARRAY_UTILITY *)pEventContext->pData1;
	assert (pUtility != NULL);

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		eventComplete = true;
	} else {
		switch(pEventContext->state){
		case STOP_UTIL_EVENT_UDT_RECORD_READ:
			// Modify our data, since RAID DDM updates it..
			ModifyRmstrData(
				RAID_UTILITY,
				&pUtility->thisRID,
				pUtility);
			// Read the ADT
			GetRmstrData(
					RAID_ARRAY,
					&pUtility->targetRID,
					(void **)&pRowData);
			pEventContext->pData2 = new(tZERO) RAID_ARRAY_DESCRIPTOR;
			memcpy(
				pEventContext->pData2, 
				pRowData, 
				sizeof(RAID_ARRAY_DESCRIPTOR));
			pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pEventContext->pData2;

			// find the Util that matches the util rowID and zero it
			rmstrServiceDeleteMemberSpareUtilityFromADT(
							RAID_UTILITY,
							&pUtility->thisRID,
							pADTRecord);

			// Set the new state identifier
			SetStateIdentifier(
					&pADTRecord->stateIdentifier,
					pCmdInfo->opcode,
					(rowID *)pEventContext->cmdHandle,
					STOP_UTIL_EVENT_ADT_RECORD_UPDATED,
					0);

			pEventContext->state = STOP_UTIL_EVENT_ADT_RECORD_UPDATED;
			// modify the ADT Row, if state not already processed
			status = CheckAndModifyRow(
					RAID_ARRAY,
					&pADTRecord->stateIdentifier,
					RAID_ARRAY_DESCRIPTOR_TABLE,
					&pADTRecord->thisRID,	// row id to modify
					pADTRecord,
					sizeof(RAID_ARRAY_DESCRIPTOR),
					&pADTRecord->thisRID,
					TSCALLBACK(DdmRAIDMstr,ProcessStopUtilEventReply),
					pEventContext);
			break;

		case STOP_UTIL_EVENT_ADT_RECORD_UPDATED:
			pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pEventContext->pData2;
			// modify ADT Row done, now delete the UDT record
			ModifyRmstrData(
				RAID_ARRAY,
				&pADTRecord->thisRID,
				pADTRecord);

			// failover: we can assume that this row exists since
			// we checked it during validation
			// Delete the util from UDT
			pEventContext->state = STOP_UTIL_EVENT_UDT_RECORD_DELETED;
			m_pTableServices->TableServiceDeleteRow(
					RAID_UTIL_DESCRIPTOR_TABLE,
					&pUtility->thisRID,	// row id to modify
					TSCALLBACK(DdmRAIDMstr,ProcessStopUtilEventReply),
					pEventContext);
			break;

		case STOP_UTIL_EVENT_UDT_RECORD_DELETED:
			// util stopped/aborted processing done
			RemoveRmstrData(
				RAID_UTILITY,
				&pUtility->thisRID);
			pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pEventContext->pData2;
			pEventContext->state = STOP_UTIL_ARRAY_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
				&pEventContext->ucArrayName,
				NULL,
				&pADTRecord->SRCTRID,
				TSCALLBACK(DdmRAIDMstr,ProcessStopUtilEventReply),
				pEventContext);
			break;

		case STOP_UTIL_ARRAY_NAME_READ:
			eventComplete = true;
			break;

		default:
			break;
		}
	}

	if (eventComplete){
		if (rc == RMSTR_SUCCESS){
			pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pEventContext->pData2;
			// Generate event for Util Stopped
			pEvtUtilStoppedStatus = new(tZERO) RMSTR_EVT_UTIL_STOPPED_STATUS;
			// fill in the event data
			pEvtUtilStoppedStatus->utilityData = *pUtility;
			pEvtUtilStoppedStatus->miscompareCount = pProcessStopUtilEventInfo->miscompareCount;
			pEvtUtilStoppedStatus->reason = pProcessStopUtilEventInfo->reason;
			m_pCmdServer->csrvReportEvent(
					RMSTR_EVT_UTIL_STOPPED,		// completion code
					pEvtUtilStoppedStatus);		// result Data
			delete pEvtUtilStoppedStatus;
			pEvtUtilStoppedStatus = NULL;

			// Log the util abort/complete event
			StringClass					scArrayName;			
			pEventContext->ucArrayName.GetAsciiString(scArrayName);
			switch(pUtility->utilityCode)
			{
				case RAID_UTIL_VERIFY:
					
					switch(pUtility->status){
						case RAID_UTIL_ABORTED:
							LogEvent(
								CTS_RMSTR_VERIFY_ABORTED_BY_USER,
								scArrayName.CString(),
								pProcessStopUtilEventInfo->miscompareCount);
							TRACEF_NF(TRACE_RMSTR,
								("\nEvent: <<Verify aborted on array %s by user, with %d miscompares.>>\n\n",
								scArrayName.CString(),
								pProcessStopUtilEventInfo->miscompareCount));
							break;
						case RAID_UTIL_ABORTED_IOERROR:
							LogEvent(
								CTS_RMSTR_VERIFY_ABORTED_IOERROR,
								scArrayName.CString(),
								pProcessStopUtilEventInfo->miscompareCount);
							TRACEF_NF(TRACE_RMSTR,
								("\nEvent: <<Verify aborted on array %s due to I/O Error, with %d miscompares.>>\n\n",
								scArrayName.CString(),
								pProcessStopUtilEventInfo->miscompareCount));
							break;
						case RAID_UTIL_COMPLETE:
							LogEvent(
								CTS_RMSTR_VERIFY_COMPLETED, 
								scArrayName.CString(),
								pProcessStopUtilEventInfo->miscompareCount);
							TRACEF_NF(TRACE_RMSTR,
								("\nEvent: <<Verify completed on array %s with %d miscompares.>>\n\n",
								scArrayName.CString(),
								pProcessStopUtilEventInfo->miscompareCount));
							break;
					}
					break;
				case RAID_UTIL_BKGD_INIT:
					switch(pUtility->status){
						case RAID_UTIL_ABORTED_IOERROR:
							LogEvent(
								CTS_RMSTR_BKGD_INIT_ABORTED_IOERROR,
								scArrayName.CString());
							TRACEF_NF(TRACE_RMSTR,
								("\nEvent: <<Bkgd Init aborted due to I/O Error on array %s.>>\n\n", scArrayName.CString()));
							break;
						case RAID_UTIL_COMPLETE:
							LogEvent(
								CTS_RMSTR_BKGD_INIT_COMPLETED, 
								scArrayName.CString());
							TRACEF_NF(TRACE_RMSTR,
								("\nEvent: <<Bkgd Init completed on array %s.>>\n\n", scArrayName.CString()));
							break;
					}
					break;
				case RAID_UTIL_LUN_HOTCOPY:
					switch(pUtility->status){
						case RAID_UTIL_ABORTED:
							LogEvent(
								CTS_RMSTR_HOTCOPY_ABORTED_BY_USER, 
								scArrayName.CString());
							TRACEF_NF(TRACE_RMSTR,
								("\nEvent: <<Hot copy aborted by user on array %s.>>\n\n", scArrayName.CString()));
							break;
						case RAID_UTIL_ABORTED_IOERROR:
							LogEvent(
								CTS_RMSTR_HOTCOPY_ABORTED_IOERROR, 
								scArrayName.CString());
							TRACEF_NF(TRACE_RMSTR,
								("\nEvent: <<Hot copy aborted due to I/O Error on array %s.>>\n\n", scArrayName.CString()));
							break;
						case RAID_UTIL_COMPLETE:
							LogEvent(
								CTS_RMSTR_HOTCOPY_COMPLETED,
								scArrayName.CString());
							TRACEF_NF(TRACE_RMSTR,
								("\nEvent: <<Hot copy completed on array %s.>>\n\n", scArrayName.CString()));
							break;
					}
					break;
				case RAID_UTIL_REGENERATE:
					switch(pUtility->status){
						case RAID_UTIL_ABORTED:
							LogEvent(
								CTS_RMSTR_REGENERATE_ABORTED_BY_USER, 
								scArrayName.CString());
							TRACEF_NF(TRACE_RMSTR,
								("\nEvent: <<Regenerate aborted by user on array %s.>>\n\n", scArrayName.CString()));
							break;
						case RAID_UTIL_ABORTED_IOERROR:
							LogEvent(
								CTS_RMSTR_REGENERATE_ABORTED_IOERROR,
								scArrayName.CString());
							TRACEF_NF(TRACE_RMSTR,
								("\nEvent: <<Regenerate aborted due to I/O Error on array %s.>>\n\n", scArrayName.CString()));
							break;
						case RAID_UTIL_COMPLETE:
							LogEvent(
								CTS_RMSTR_REGENERATE_COMPLETED,
								scArrayName.CString());
							TRACEF_NF(TRACE_RMSTR,
								("\nEvent: <<Regenerate completed on array %s.>>\n\n", scArrayName.CString()));
							break;
					}
					break;
			}	// end switch

			// Resolve:
			// We need to have a separate method to update the array state and
			// generate events if the util was aborted due to I/O error.

			// Check for regenerate complete, if array state changed
			// from critical to fault tolerant
			// update any mdt records if required
			// (For failover: We are just using modify field according to
			// the most current state, so the operations are harmless, if 
			// repeated)
			UpdateArrayStateAndGenerateEvents(
					pADTRecord, 
					pUtility);
		}	// rc == RMSTR_SUCCESS

		// Report the status back to CmdSender
		m_pCmdServer->csrvReportCmdStatus(
				pEventContext->cmdHandle,		// handle
				rc,								// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCmdInfo
		StopCommandProcessing(true, pEventContext->cmdHandle);

		delete pEventContext;
		pEventContext = NULL;
	} // event complete
	return status;
}


//************************************************************************
//	UpdateArrayStateAndGenerateEvents
//	if utility completed successfully:
//		- For Bkgd Init, update the init status
//		- For Regnerate and Hot Copy with Manual Break, generate FT event
//		- For Hot copy with Auto break, delete array
//	if utility was aborted
//		- For init,??
//		- For regenerate, check for reporting array FT event
//		- for hotcopy, if user aborted then clean hot copy mirror
//			if aborted by i/o error, leave mirror
//
//************************************************************************
void DdmRAIDMstr::
UpdateArrayStateAndGenerateEvents(
			RAID_ARRAY_DESCRIPTOR		*_pADTRecord, 
			RAID_ARRAY_UTILITY			*_pUtility)
{
	RAID_ARRAY_MEMBER				*pMember = NULL;
	CONTEXT							*pContext;
	BOOL							error = false;
	BOOL							generateFaultTolerantEvent = false;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_UTILITY				*pUtility = NULL;


	pContext = new CONTEXT;

	pContext->pData = new(tZERO) RAID_ARRAY_DESCRIPTOR;
	pContext->pData1 = new(tZERO) RAID_ARRAY_UTILITY;
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pContext->pData; 
	pUtility = (RAID_ARRAY_UTILITY *)pContext->pData1;

	*pADTRecord = *_pADTRecord;
	*pUtility = *_pUtility;

	GetRmstrData(
			RAID_MEMBER,
			&pUtility->destinationRowIds[0],
			(void **)&pMember);
	if (!pMember){
		error = true;
	}


	if (!error){
		switch(pUtility->utilityCode){
			case RAID_UTIL_BKGD_INIT:
				if (pUtility->status == RAID_UTIL_COMPLETE){
					// set the array init status
					pContext->state = STOP_UTIL_EVENT_ADT_INIT_STATUS_UPDATED;
					pADTRecord->initStatus = RAID_INIT_COMPLETE;
					m_pTableServices->TableServiceModifyField(
										RAID_ARRAY_DESCRIPTOR_TABLE,
										&pADTRecord->thisRID,	// row id to modify
										fdINIT_STATUS,
										&pADTRecord->initStatus,
										sizeof(RAID_INIT_STATUS),
										TSCALLBACK(DdmRAIDMstr,ProcessUpdateArrayStateAndGenerateEventsReply),
										pContext);
				}
				break;

			case RAID_UTIL_REGENERATE:
				if (pUtility->status == RAID_UTIL_COMPLETE){
					generateFaultTolerantEvent = true;
				} else {
					// if aborted by user - we just leave regenerating member as down 
					// if aborted  i/o error - we should have got a member down
					// on the regenerating member, which will look for other spares 
					// to regenerate
				}
				break;

			case RAID_UTIL_LUN_HOTCOPY:
				switch(pUtility->status){
					case RAID_UTIL_COMPLETE:
						// If array was created for hot copy with auto break, 
						// then break the mirror
						if (pADTRecord->createPolicy.StartHotCopyWithAutoBreak){
								StartInternalDeleteArray(
									pADTRecord, 
									true,	// break hot copy
									false);	// use src as export = false
						} else if(pADTRecord->createPolicy.StartHotCopyWithManualBreak){
							// if manual hot copy break, then generate the fault tolerant event
							generateFaultTolerantEvent = true;
						} 
						break;
					case RAID_UTIL_ABORTED:
						// aborted by user, do auto cleanup with exporting the src
						// because dest is not copied yet.
						// doesnt matter if auto break or not..
						StartInternalDeleteArray(
								pADTRecord,
								true,	// break mirror
								true);	// use src as export
						break;
					case RAID_UTIL_ABORTED_IOERROR:
						// dont do anything, user who started hotcopy
						// will clean up
						break;
					default:
						assert(0);
				}
				break;
			default:
				break;
		}
	} 

	if (generateFaultTolerantEvent){
		// use value to set the status of member to UP
		pContext->value = RAID_STATUS_UP;
		// save the state of the array before changing member status
		pContext->value1 = CheckIfArrayCritical(pADTRecord, false);

		pContext->pData2 = new(tZERO) RAID_ARRAY_MEMBER;
		memcpy(pContext->pData2, pMember, sizeof(RAID_ARRAY_MEMBER));
		pContext->state = STOP_UTIL_EVENT_MDT_HEALTH_UPDATED;
		m_pTableServices->TableServiceModifyField(
								RAID_MEMBER_DESCRIPTOR_TABLE,
								&pMember->thisRID,	// row id to modify
								fdMEMBER_HEALTH,
								&pContext->value,
								sizeof(RAID_MEMBER_STATUS),
								TSCALLBACK(DdmRAIDMstr,ProcessUpdateArrayStateAndGenerateEventsReply),
								pContext);
	}
}



//************************************************************************
//	UpdateArrayStateAndGenerateEventsReply
//		- Update the member health
//		- If array was critical, change to FT and generate event
//		- if Member hot copy, then change init status also
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessUpdateArrayStateAndGenerateEventsReply(void *_pContext, STATUS status)
{
	CONTEXT						*pContext;
	RAID_ARRAY_UTILITY			*pUtility = NULL;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	BOOL						cmdComplete = false;
	STATUS						rc = RMSTR_SUCCESS;
	RAID_ARRAY_MEMBER			*pMember = NULL;
	BOOL						wasArrayCritical = false;
	RMSTR_EVT_ARRAY_FAULT_TOLERANT_STATUS		*pEvtArrayFaultTolerant = NULL;
	U32							i=0;


	pContext = (CONTEXT *)_pContext;

	// pContext->pData = ADTRecord
	// pContext->pData1 = UDT Record
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pContext->pData;
	pUtility = (RAID_ARRAY_UTILITY *)pContext->pData1;
	pMember = (RAID_ARRAY_MEMBER *)pContext->pData2;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pContext->state){
		case STOP_UTIL_EVENT_MDT_HEALTH_UPDATED:
			pMember->memberHealth = RAID_STATUS_UP;
			ModifyRmstrData(
				RAID_MEMBER,
				&pUtility->destinationRowIds[pContext->numProcessed],
				pMember);
			// proceed to next member
			pContext->numProcessed++;
			GetRmstrData(
				RAID_MEMBER,
				&pUtility->destinationRowIds[pContext->numProcessed],
				(void **)&pMember);
			if (pMember){
				memcpy(pContext->pData2, pMember, sizeof(RAID_ARRAY_MEMBER));
				status = m_pTableServices->TableServiceModifyField(
								RAID_MEMBER_DESCRIPTOR_TABLE,
								&pMember->thisRID,	// row id to modify
								fdMEMBER_HEALTH,
								&pContext->value,
								sizeof(RAID_MEMBER_STATUS),
								TSCALLBACK(DdmRAIDMstr,ProcessUpdateArrayStateAndGenerateEventsReply),
								pContext);
			} else {
				wasArrayCritical = (BOOL)pContext->value1;
				if (wasArrayCritical){
					pADTRecord->health = RAID_FAULT_TOLERANT;
					pContext->state = STOP_UTIL_EVENT_ADT_FAULT_TOLERANT;
					status = m_pTableServices->TableServiceModifyRow(
								RAID_ARRAY_DESCRIPTOR_TABLE,
								&pADTRecord->thisRID,	// row id to modify
								pADTRecord,
								sizeof(RAID_ARRAY_DESCRIPTOR),
								&pADTRecord->thisRID,
								TSCALLBACK(DdmRAIDMstr,ProcessUpdateArrayStateAndGenerateEventsReply),
								pContext);
				} else {
					cmdComplete = true;
				}
			}
			break;

		case STOP_UTIL_EVENT_ADT_FAULT_TOLERANT:
			// modify ADT Row done, now delete the UDT record
			ModifyRmstrData(
				RAID_ARRAY,
				&pADTRecord->thisRID,
				pADTRecord);
			// Generate event for array fault tolerant
			pEvtArrayFaultTolerant = new(tZERO) RMSTR_EVT_ARRAY_FAULT_TOLERANT_STATUS;
			pEvtArrayFaultTolerant->arrayData = *pADTRecord;
			for (i=0; i < pADTRecord->numberMembers; i++){
				GetRmstrData(
						RAID_MEMBER,
						&pADTRecord->members[i],
						(void **)&pMember);
				if (pMember){
					pEvtArrayFaultTolerant->memberHealth[i] = 
							pMember->memberHealth;
				}
			}
			m_pCmdServer->csrvReportEvent(
					RMSTR_EVT_ARRAY_FAULT_TOLERANT,		// completion code
					pEvtArrayFaultTolerant);		// result Data
			delete pEvtArrayFaultTolerant;
			// Log the event
			LogEventWithArrayName(
				CTS_RMSTR_ARRAY_FAULT_TOLERANT, 
				&pADTRecord->SRCTRID);

			// Remit any outstanding Alarm for array critical
			RmstrRemitAlarm(
				CTS_RMSTR_ARRAY_CRITICAL,
				&pADTRecord->thisRID);

			if(pADTRecord->createPolicy.StartHotCopyWithManualBreak){
				pContext->state = STOP_UTIL_EVENT_ADT_INIT_STATUS_UPDATED;
				pADTRecord->initStatus = RAID_INIT_COMPLETE;
				m_pTableServices->TableServiceModifyField(
									RAID_ARRAY_DESCRIPTOR_TABLE,
									&pADTRecord->thisRID,	// row id to modify
									fdINIT_STATUS,
									&pADTRecord->initStatus,
									sizeof(RAID_INIT_STATUS),
									TSCALLBACK(DdmRAIDMstr,ProcessUpdateArrayStateAndGenerateEventsReply),
									pContext);
			} else {
				cmdComplete =true;
			}
			break;

		case STOP_UTIL_EVENT_ADT_INIT_STATUS_UPDATED:
			// modify ADT Row done
			ModifyRmstrData(
				RAID_ARRAY,
				&pADTRecord->thisRID,
				pADTRecord);
			cmdComplete =true;
			break;
		}
	}
	if (cmdComplete){
		delete pContext;
	}
	return status;
}