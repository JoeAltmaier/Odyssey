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
// File: RmstrDeleteArray.cpp
// 
// Description:
// Implementation for raid Delete array cmd
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrDeleteArray.cpp $
// 
// 19    12/17/99 5:33p Dpatel
// added hot copy code
// 
// 17    9/09/99 1:38p Dpatel
// removed ENABLE_LOGGING ifdef...
// 
// 16    9/03/99 10:01a Dpatel
// Remitting alarms...
// 
// 15    8/31/99 7:17p Dpatel
// bug fix
// 
// 14    8/30/99 10:46a Dpatel
// checked for storage element in use..
// 
// 13    8/27/99 5:24p Dpatel
// added event code..
// 
// 12    8/14/99 1:37p Dpatel
// Added event logging..
// 
// 11    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 10    8/05/99 11:07a Dpatel
// internal delete array, fake raid ddm, hot copy auto break, removed
// array name code..
// 
// 9     8/03/99 5:25p Dpatel
// Removed the service method to modify SRC, used table services..
// 
// 8     7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 7     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 6     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 5     7/20/99 6:49p Dpatel
// Some bug fixes and changed arrayName in ArrayDescriptor to rowID.
// 
// 4     7/17/99 1:20p Dpatel
// Queued up commands.
// 
// 3     7/16/99 10:28a Dpatel
// Added DownMember and Commit Spare code. Also removed the reads for
// validation.
// 
// 2     7/09/99 5:26p Dpatel
// 
// 1     7/06/99 5:05p Dpatel
// Initial Creation.
// 
//
/*************************************************************************/


#include "DdmRaidMgmt.h"


// Delete Array States
enum
{
	DELETE_ARRAY_MEMBERS_DELETED = 1,
	DELETE_ARRAY_SPARES_DELETED,
	DELETE_ARRAY_UTILITIES_DELETED,
	DELETE_ARRAY_ADT_RECORD_DELETED,
	DELETE_ARRAY_TABLE_ROW_DELETED,
	DELETE_ARRAY_SRCT_RECORD_UPDATED,
	DELETE_ARRAY_SRCT_RECORD_DELETED,
	DELETE_ARRAY_ARRAY_NAME_READ,
	DELETE_ARRAY_ARRAY_NAME_DELETED,
	DELETE_ARRAY_HOTCOPY_UNEXPORT,
	DELETE_ARRAY_HOTCOPY_EXPORT,
	DELETE_ARRAY_EXPORT_RECORD_READ
};

// Delete Array Validation states
enum
{
	DELETE_ARRAY_VALIDATION_ADT_RECORD_READ = 100,
	DELETE_ARRAY_VALIDATION_SRCT_RECORD_READ
};





//************************************************************************
//	DeleteTheArray
//		Remove Members from the MDT and update its SRC entry to free
//		Remove Dedicated Spares(if any)from SDT and free its SRC entry
//		Remove Array descriptor from ADT
//		Remove the SRC entry for the Array
//
//	handle		- the handle for delete array cmd
//	_pCmdInfo	- the delete array cmd info packet
//	pADTRecord	- the Array descriptor for the array to delete
//
//************************************************************************
STATUS DdmRAIDMstr::
DeleteTheArray(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo,
			RAID_ARRAY_DESCRIPTOR	*pADTRecord)

{
	RMSTR_DELETE_ARRAY_INFO			*pDeleteArrayInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	CONTEXT							*pCmdContext = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	STATUS							status = RMSTR_SUCCESS;

	pCmdContext = new CONTEXT;


	pCmdContext->cmdHandle	= handle;
	pCmdContext->pData		= new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));
	

	pCmdContext->pData1		= new RAID_ARRAY_DESCRIPTOR;
	memcpy(pCmdContext->pData1,pADTRecord,sizeof(RAID_ARRAY_DESCRIPTOR));

	pCmdInfo		= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pDeleteArrayInfo = 
			(RMSTR_DELETE_ARRAY_INFO *)&pCmdParams->deleteArrayInfo;

	// Remit any outstanding Alarm for array offline or critical
	RmstrRemitAlarm(
			CTS_RMSTR_ARRAY_CRITICAL,
			&pADTRecord->thisRID);
	RmstrRemitAlarm(
			CTS_RMSTR_ARRAY_OFFLINE,
			&pADTRecord->thisRID);

	// if break hot copy mirror, then unexport the array first
	if (pDeleteArrayInfo->policy.BreakHotCopyMirror){
		// first unexport the hot copy array
		pCmdContext->state = DELETE_ARRAY_HOTCOPY_UNEXPORT;
		ProcessDeleteArrayReply(pCmdContext, OK);
	} else {	
		// Remove MEMBERS & UPDATE SRCT, REMOVE SPARES & UPDATE SRCT, REMOVE UTILS, 
		// REMOVE ADT & DELETE SRCT
		pCmdContext->state = DELETE_ARRAY_MEMBERS_DELETED;
		status = DeleteTableRowsForArrayElements(
			RAID_MEMBER,
			pADTRecord,
			TSCALLBACK(DdmRAIDMstr,ProcessDeleteArrayReply),
			pCmdContext);
	}
	return status;
}	


//************************************************************************
//	ProcessDeleteArrayReply
//		Handle the different states for delete array. Generate an
//		event for delete array if cmd completes successfully.
//
//	_pContext	- the context for delete array 
//	status		- the status
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessDeleteArrayReply(void *_pContext, STATUS status)
{
	RMSTR_DELETE_ARRAY_INFO			*pDeleteArrayInfo = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	CONTEXT							*pCmdContext = NULL;

	RAID_ARRAY_MEMBER				*pMember = NULL;
	RAID_SPARE_DESCRIPTOR			*pSpare = NULL;
	RAID_ARRAY_UTILITY				*pUtility = NULL;

	BOOL							cmdComplete = false;
	RMSTR_EVT_ARRAY_DELETED_STATUS	*pEvtArrayDeleted = NULL;
	STATUS							rc;


	rc = RMSTR_SUCCESS;

	pCmdContext = (CONTEXT *)_pContext;
	// pCmdContext->pData = CmdInfo
	// pCmdContext->pData1 = ADT Record
	// pCmdContext->pData2 = NULL
	// pCmdContext->pData3 = NULL
	pCmdInfo		= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pDeleteArrayInfo = 
		(RMSTR_DELETE_ARRAY_INFO *)&pCmdParams->deleteArrayInfo;
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;


	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){
		case DELETE_ARRAY_HOTCOPY_UNEXPORT:
			// Modify the Virtual Circuit,
			//		Remove the array from the head and export
			//		export the leg of the mirror which the cmd requests
			pCmdContext->state = DELETE_ARRAY_HOTCOPY_EXPORT;
			GetRmstrData(
					RAID_MEMBER,
					&pDeleteArrayInfo->hotCopyExportMemberRowId,
					(void **)&pMember);
			if (pMember){
				BreakHotCopy(
						pADTRecord,				// HC array to be broken
						pMember,				// member (leg) to be exported
						TSCALLBACK(DdmRAIDMstr,ProcessDeleteArrayReply),
						pCmdContext);
			} else {
				// we did'nt find member, so we had already done this
				// step before (failover)
				ProcessDeleteArrayReply(pCmdContext, OK);
			}
			break;

		case DELETE_ARRAY_HOTCOPY_EXPORT:
			// Remove MEMBERS & UPDATE SRCT, REMOVE SPARES & UPDATE SRCT, REMOVE UTILS, 
			// REMOVE ADT & DELETE SRCT
			pCmdContext->state = DELETE_ARRAY_MEMBERS_DELETED;
			status = DeleteTableRowsForArrayElements(
				RAID_MEMBER,
				pADTRecord,
				TSCALLBACK(DdmRAIDMstr,ProcessDeleteArrayReply),
				pCmdContext);
				break;

		case DELETE_ARRAY_MEMBERS_DELETED:
			pCmdContext->state = DELETE_ARRAY_SPARES_DELETED;
			status = DeleteTableRowsForArrayElements(
				RAID_SPARE,
				pADTRecord,
				TSCALLBACK(DdmRAIDMstr,ProcessDeleteArrayReply),
				pCmdContext);
			break;

		case DELETE_ARRAY_SPARES_DELETED:
			pCmdContext->state = DELETE_ARRAY_UTILITIES_DELETED;
			status = DeleteTableRowsForArrayElements(
				RAID_UTILITY,
				pADTRecord,
				TSCALLBACK(DdmRAIDMstr,ProcessDeleteArrayReply),
				pCmdContext);
			break;

		case DELETE_ARRAY_UTILITIES_DELETED:
			pCmdContext->state = DELETE_ARRAY_ADT_RECORD_DELETED;
			status = m_pTableServices->TableServiceDeleteRow(
							RAID_ARRAY_DESCRIPTOR_TABLE,
							&pADTRecord->thisRID,
							TSCALLBACK(DdmRAIDMstr,ProcessDeleteArrayReply),
							pCmdContext);
			break;

		case DELETE_ARRAY_ADT_RECORD_DELETED:
			RemoveRmstrData(
				RAID_ARRAY,
				&pADTRecord->thisRID);

			pCmdContext->state = DELETE_ARRAY_ARRAY_NAME_READ;
			m_pHelperServices->ReadStorageElementName(
				&pCmdContext->ucArrayName,
				NULL,
				&pADTRecord->SRCTRID,
				TSCALLBACK(DdmRAIDMstr,ProcessDeleteArrayReply),
				pCmdContext);
			break;

		case DELETE_ARRAY_ARRAY_NAME_READ:
			pCmdContext->state = DELETE_ARRAY_ARRAY_NAME_DELETED;
			m_pHelperServices->DeleteStorageElementName(
					&pADTRecord->SRCTRID,
					TSCALLBACK(DdmRAIDMstr,ProcessDeleteArrayReply),
					pCmdContext);
			break;
		
		case DELETE_ARRAY_ARRAY_NAME_DELETED:
			pCmdContext->state = DELETE_ARRAY_SRCT_RECORD_DELETED;
			status = m_pTableServices->TableServiceDeleteRow(
							STORAGE_ROLL_CALL_TABLE,
							&pADTRecord->SRCTRID,
							TSCALLBACK(DdmRAIDMstr,ProcessDeleteArrayReply),
							pCmdContext);
			break;

		case DELETE_ARRAY_SRCT_RECORD_DELETED:
			cmdComplete = true;
			break;

		default:
			cmdComplete = true;
			break;
		}
	}
	if (cmdComplete){
		// Report the status of the Delete Array back 
		m_pCmdServer->csrvReportCmdStatus(
			pCmdContext->cmdHandle,		// handle
			rc,							// completion code
			NULL,						// result Data
			(void *)pCmdInfo);			// Orig cmd info

		if (rc == RMSTR_SUCCESS){
			// You have to use the stored name in ucArrayName,
			// since the actual entry with StringResourceManager
			// might have been deleted
			StringClass					scArrayName;
			pCmdContext->ucArrayName.GetAsciiString(scArrayName);
			LogEvent(CTS_RMSTR_ARRAY_DELETED, scArrayName.CString());
			TRACEF_NF(TRACE_RMSTR,("\n<<Event: Array %s deleted!>>\n\n", scArrayName.CString()));

			// Generate event for Array Deleted
			pEvtArrayDeleted = new(tZERO) RMSTR_EVT_ARRAY_DELETED_STATUS;

			pEvtArrayDeleted->SRCTRowId = pADTRecord->SRCTRID;
			pEvtArrayDeleted->arrayData = *pADTRecord;
			m_pCmdServer->csrvReportEvent(
				RMSTR_EVT_ARRAY_DELETED,	// event Code
				pEvtArrayDeleted);			// event Data
			delete pEvtArrayDeleted;
			pEvtArrayDeleted = NULL;
		}

		StopCommandProcessing(true, pCmdContext->cmdHandle);

		if (pCmdContext) {
			delete pCmdContext;
			pCmdContext = NULL;
		}
	}
	return status;
}




//************************************************************************
//	DeleteArrayValidation
//		Validate the Delete array cmd. Check if array exists and that
//		no utilities are running on the array.
//
//************************************************************************
STATUS DdmRAIDMstr::
DeleteArrayValidation(HANDLE h, RMSTR_CMND_INFO *_pCmdInfo)
{
	STATUS						status = RMSTR_SUCCESS;
	RMSTR_DELETE_ARRAY_INFO		*pDeleteArrayInfo = NULL;
	RMSTR_CMND_PARAMETERS		*pCmdParams = NULL;
	RMSTR_CMND_INFO				*pCmdInfo = NULL;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;

	// check if duplicate name
	CONTEXT	*pValidationContext			= new CONTEXT;

	// save the Delete ARRAY DEFINITION and the handle
	pValidationContext->cmdHandle	= h;
	pValidationContext->state		= DELETE_ARRAY_VALIDATION_ADT_RECORD_READ;

	pValidationContext->pData		= new RMSTR_CMND_INFO;
	memcpy(pValidationContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));

	pCmdInfo		= (RMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams		= &pCmdInfo->cmdParams;
	pDeleteArrayInfo = 
			(RMSTR_DELETE_ARRAY_INFO *)&pCmdParams->deleteArrayInfo;


	// first Get the ADT record
	GetRmstrData(
			RAID_ARRAY, 
			&pDeleteArrayInfo->arrayRowId,
			(void **)&pADTRecord);
	if (pADTRecord) {
		// Allocate space for read row data
		pValidationContext->pData1 = new RAID_ARRAY_DESCRIPTOR;
		memcpy(pValidationContext->pData1,pADTRecord,sizeof(RAID_ARRAY_DESCRIPTOR));
		ProcessDeleteArrayValidationReply(pValidationContext,OK);
	} else {
		status = RMSTR_ERR_INVALID_COMMAND;
	}
	if(status){
		// Report error to Cmd Sender
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

//************************************************************************
//	ProcessDeleteArrayValidationReply
//		Process replies for delete array validation.
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessDeleteArrayValidationReply(void *_pContext, STATUS status)
{
	CONTEXT							*pValidationContext = NULL;
	RMSTR_DELETE_ARRAY_INFO			*pDeleteArrayInfo = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	STATUS							rc;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	BOOL							validationComplete = false;
	StorageRollCallRecord			*pSRCRecord = NULL;

	pValidationContext	= (CONTEXT *)_pContext;
	pCmdInfo			= (RMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;


	if (status != OS_DETAIL_STATUS_SUCCESS){
		// return appropriate error
		rc = RMSTR_ERR_INVALID_COMMAND;
		validationComplete = true;
	} else {
		rc					= RMSTR_SUCCESS;
		pDeleteArrayInfo = 
				(RMSTR_DELETE_ARRAY_INFO *)&pCmdParams->deleteArrayInfo;
		pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pValidationContext->pData1;

		switch(pValidationContext->state){
		case DELETE_ARRAY_VALIDATION_ADT_RECORD_READ:
			// pValidationContext->pData = cmdInfo
			// pValidationContext->pData1 = ADT Record data
			// pData2 = NULL
			// pData3 = NULL
			if (pADTRecord->numberUtilities){
				rc = RMSTR_ERR_UTIL_RUNNING;
			} 
			if (rc == RMSTR_SUCCESS){
				// Allocate space for read row data
				pValidationContext->state		= DELETE_ARRAY_VALIDATION_SRCT_RECORD_READ;
				pValidationContext->pData2 = new (tZERO) StorageRollCallRecord;

				// read SRCT Record and check if used or not
				status = m_pTableServices->TableServiceReadRow(
						STORAGE_ROLL_CALL_TABLE,
						&pADTRecord->SRCTRID,
						pValidationContext->pData2,
						sizeof(StorageRollCallRecord),
						TSCALLBACK(DdmRAIDMstr,ProcessDeleteArrayValidationReply),
						pValidationContext);
			}else{	
				validationComplete = true;
			}
			break;

		case DELETE_ARRAY_VALIDATION_SRCT_RECORD_READ:
			pSRCRecord = (StorageRollCallRecord *)pValidationContext->pData2;
			if (pDeleteArrayInfo->policy.BreakHotCopyMirror){
				DeleteTheArray(
					pValidationContext->cmdHandle,
					pCmdInfo,
					pADTRecord);
			} else {
				if (pSRCRecord->fUsed){
					rc = RMSTR_ERR_STORAGE_ELEMENT_IN_USE;
				}else {
					DeleteTheArray(
						pValidationContext->cmdHandle,
						pCmdInfo,
						pADTRecord);
				}
			}
			validationComplete = true;
		default:
			break;
		}
	}
	if (validationComplete){
		if(rc){
			// Report error to Cmd Sender
			m_pCmdServer->csrvReportCmdStatus(
				pValidationContext->cmdHandle,	// handle
				rc,								// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCmdInfo
			StopCommandProcessing(true, pValidationContext->cmdHandle);
		}
		delete pValidationContext;
		pValidationContext = NULL;
	}
	return status;
}



//************************************************************************
//	DeleteTableRowsForArrayElements
//		A convinience method to delete either the Members, Spares or
//		utilities belonging to a particular array. The SRC entry for
//		members and spares is appropriately marked as UNUSED.
//
//	type		- RAID_MEMBER, RAID_SPARE, RAID_UTILITY
//	_pADTRecord	- the array on which the operation is to be performed
//	cb			- callback to call when rows deleted
//	pCmdContext	- the original context, returned on cmd completion
//
//************************************************************************
STATUS DdmRAIDMstr::
DeleteTableRowsForArrayElements(
		U32						type,
		RAID_ARRAY_DESCRIPTOR	*_pADTRecord,
		pTSCallback_t			cb,
		CONTEXT					*pCmdContext)
{
	String64			tableName;
	STATUS				status = 0;
	rowID				*pADTRows = NULL;
	RAID_ARRAY_MEMBER	*pMember = NULL;
	RAID_ARRAY_UTILITY	*pUtility = NULL;
	RAID_SPARE_DESCRIPTOR	*pSpare = NULL;
	RAID_ARRAY_DESCRIPTOR	*pADTRecord = NULL;
	BOOL					cmdComplete = false;


	CONTEXT			*pEnumContext = new CONTEXT;

	// save the users info into enum context
	pEnumContext->state = DELETE_ARRAY_TABLE_ROW_DELETED;
	pEnumContext->pCallback = cb;
	pEnumContext->pParentContext = pCmdContext;
	pEnumContext->value1 = type;
	pEnumContext->pData = new RAID_ARRAY_DESCRIPTOR;
	memcpy(pEnumContext->pData, _pADTRecord, sizeof(RAID_ARRAY_DESCRIPTOR));
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pEnumContext->pData;


	switch (type){
	case RAID_MEMBER:
		strcpy(tableName,RAID_MEMBER_DESCRIPTOR_TABLE);
		pADTRows = pADTRecord->members;
		if (pADTRecord->numberMembers){
				status = m_pTableServices->TableServiceDeleteRow(
							tableName,
							&pADTRows[0],
							TSCALLBACK(DdmRAIDMstr,ProcessDeleteTableRowsReply),
							pEnumContext);
		}else {
			cmdComplete = true;
		}
		break;

	case RAID_SPARE:
		strcpy(tableName,RAID_SPARE_DESCRIPTOR_TABLE);
		pADTRows = pADTRecord->spares;
		if (pADTRecord->numberSpares){
			status = m_pTableServices->TableServiceDeleteRow(
						tableName,
						&pADTRows[0],
						TSCALLBACK(DdmRAIDMstr,ProcessDeleteTableRowsReply),
						pEnumContext);
		} else {
			cmdComplete = true;
		}
		break;
	case RAID_UTILITY:
		strcpy(tableName,RAID_UTIL_DESCRIPTOR_TABLE);
		pADTRows = pADTRecord->utilities;
		if (pADTRecord->numberUtilities){
				status = m_pTableServices->TableServiceDeleteRow(
							tableName,
							&pADTRows[0],
							TSCALLBACK(DdmRAIDMstr,ProcessDeleteTableRowsReply),
							pEnumContext);
		} else {
			cmdComplete = true;
		}
		break;
	}
	if (cmdComplete){
		//pEnumContext->clean();
		delete pEnumContext;
		(this->*cb)(
				pCmdContext,	
				OK);		

	}
	return status;
}

//************************************************************************
//	ProcessDeleteTableRowsReply
//		Process replies for deleting table rows
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessDeleteTableRowsReply(void *_pContext, STATUS status)
{
	CONTEXT							*pEnumContext;
	CONTEXT							*pCmdContext;
	U32								type;
	pTSCallback_t					cb;
	RAID_ARRAY_MEMBER				*pMember = NULL;
	RAID_SPARE_DESCRIPTOR			*pSpare = NULL;
	RAID_ARRAY_UTILITY				*pUtility = NULL;
	BOOL							cmdComplete = false;
	rowID							*pADTRows = NULL;
	String64						tableName;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	U32								count =0;



	if (status != OS_DETAIL_STATUS_SUCCESS){
		return status;
	}
	pEnumContext	= (CONTEXT *)_pContext;

	// Read the original cmd data from context
	type = pEnumContext->value1;
	cb = pEnumContext->pCallback;
	pCmdContext = pEnumContext->pParentContext;

	// pEnumContext->pData = pADTRecord;
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pEnumContext->pData;

	switch (type){
	case RAID_MEMBER:
		strcpy(tableName,RAID_MEMBER_DESCRIPTOR_TABLE);
		pADTRows = pADTRecord->members;
		count = pADTRecord->numberMembers;
		break;
	case RAID_SPARE:
		strcpy(tableName,RAID_SPARE_DESCRIPTOR_TABLE);
		pADTRows = pADTRecord->spares;
		count = pADTRecord->numberSpares;
		break;
	case RAID_UTILITY:
		strcpy(tableName,RAID_UTIL_DESCRIPTOR_TABLE);
		pADTRows = pADTRecord->utilities;
		count = pADTRecord->numberUtilities;
		break;
	}

	switch(pEnumContext->state){
	case DELETE_ARRAY_TABLE_ROW_DELETED:
		pEnumContext->state = DELETE_ARRAY_SRCT_RECORD_UPDATED;
		switch(type){
			case RAID_MEMBER:
				GetRmstrData(
					type,
					&pADTRows[pEnumContext->numProcessed],
					(void **)&pMember);
				if (pMember){
					if (pMember->policy.HotCopyExportMember){
						// leave the member used
						m_SRCIsUsed = SRC_USED;
					} else {
						m_SRCIsUsed = SRC_UNUSED;
					}
					status = m_pTableServices->TableServiceModifyField(
									STORAGE_ROLL_CALL_TABLE,
									&pMember->memberRID,		// SRC row id
									fdSRC_FUSED,				// field name of field to be modifiied
									&m_SRCIsUsed,				// set to false
									sizeof(U32),
									TSCALLBACK(DdmRAIDMstr,ProcessDeleteTableRowsReply),
									pEnumContext);
				}
				break;

			case RAID_SPARE:
				GetRmstrData(
					type,
					&pADTRows[pEnumContext->numProcessed],
					(void **)&pSpare);
				if (pSpare){
					m_SRCIsUsed = SRC_UNUSED;
					status = m_pTableServices->TableServiceModifyField(
									STORAGE_ROLL_CALL_TABLE,
									&pSpare->SRCTRID,
									fdSRC_FUSED,			// field name of field to be modifiied
									&m_SRCIsUsed,			// set to false
									sizeof(U32),
									TSCALLBACK(DdmRAIDMstr,ProcessDeleteTableRowsReply),
									pEnumContext);
				}
				break;

			case RAID_UTILITY:
				// utilities, no need to update SRCT, check for next util row to delete
				RemoveRmstrData(
					type,
					&pADTRows[pEnumContext->numProcessed]);

				pEnumContext->numProcessed++;
				if (pEnumContext->numProcessed < pEnumContext->value2){
					pEnumContext->state = DELETE_ARRAY_TABLE_ROW_DELETED;
					status = m_pTableServices->TableServiceDeleteRow(						tableName,
								&pADTRows[pEnumContext->numProcessed],
								TSCALLBACK(DdmRAIDMstr,ProcessDeleteTableRowsReply),
								pEnumContext);
				} else {
					cmdComplete = true;
				}
				break;
			default:
				break;
		}
		break;

	case DELETE_ARRAY_SRCT_RECORD_UPDATED:
		RemoveRmstrData(
			type,
			&pADTRows[pEnumContext->numProcessed]);

		pEnumContext->numProcessed++;
		if (pEnumContext->numProcessed < count){
			pEnumContext->state = DELETE_ARRAY_TABLE_ROW_DELETED;
			status = m_pTableServices->TableServiceDeleteRow(
						tableName,
						&pADTRows[pEnumContext->numProcessed],
						TSCALLBACK(DdmRAIDMstr,ProcessDeleteTableRowsReply),
						pEnumContext);
		} else {
			cmdComplete = true;
		}
		break;

	default:
		break;
	}
	if (cmdComplete){
		delete pEnumContext;
		pEnumContext = NULL;

		// call the original cb with cmdContext
		(this->*cb)(
				pCmdContext,	
				OK);		
	}
	return status;
}








//************************************************************************
//	Export the member on deleting array (if hot copy mirror)
//
//************************************************************************
void DdmRAIDMstr::
BreakHotCopy(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_ARRAY_MEMBER			*pMember,
		pTSCallback_t				cb,
		void						*_pContext)
{
	STATUS			status;
	CONTEXT			*pModifyContext	= new CONTEXT;
	pModifyContext->pParentContext	= (CONTEXT *)_pContext;
	pModifyContext->pCallback		= cb;
	
	pModifyContext->pData	= new (tZERO) ExportTableEntry;
	pModifyContext->pData1	= new (tZERO) RAID_ARRAY_DESCRIPTOR;
	pModifyContext->pData2	= new (tZERO) RAID_ARRAY_MEMBER;
	memcpy(pModifyContext->pData1, pADTRecord, sizeof(RAID_ARRAY_DESCRIPTOR));
	memcpy(pModifyContext->pData2, pMember, sizeof(RAID_ARRAY_MEMBER));
	pModifyContext->state = DELETE_ARRAY_EXPORT_RECORD_READ;

	// read the export table, and find the VCrowid matching our
	// src entry, also, make sure that the entry is not already exported
	TSReadRow *pReadRow = new TSReadRow;
	status = pReadRow->Initialize(
			this,
			EXPORT_TABLE,
			"ridSRC",
			&pADTRecord->SRCTRID,
			sizeof(rowID),
			pModifyContext->pData,
			sizeof(ExportTableEntry),
			NULL,
			TSCALLBACK(DdmRAIDMstr,ProcessBreakHotCopyReply),
			pModifyContext
			);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pReadRow->Send();
	else 
		ProcessBreakHotCopyReply(pModifyContext, status);
}	


//************************************************************************
//	Export the member reply
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessBreakHotCopyReply(
		void					*_pContext,
		STATUS					status)
{
	CONTEXT					*pOriginalContext = NULL;
	pTSCallback_t			cb;
	ExportTableEntry		*pExportEntry = NULL;
	RAID_ARRAY_DESCRIPTOR	*pADTRecord = NULL;
	RAID_ARRAY_MEMBER		*pMember = NULL;
	
	// VC Modify stuff
	VCRequest						*pVCRequest = NULL;
	VCCommand						*pVcCmd;
	VCModifyCtlCommand				*pVCModifyCtl;
	MsgVCMModifyVC					*pMsg;

	// Restore back data from the context
	CONTEXT				*pModifyContext = (CONTEXT *)_pContext;
	pOriginalContext	= pModifyContext->pParentContext;
	cb					= pModifyContext->pCallback;
	pExportEntry		= (ExportTableEntry *)pModifyContext->pData;
	pADTRecord			= (RAID_ARRAY_DESCRIPTOR *)pModifyContext->pData1;
	pMember				= (RAID_ARRAY_MEMBER *)pModifyContext->pData2;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		delete pModifyContext;
		(this->*cb)(pOriginalContext,status);
	} else {

		switch(pModifyContext->state){
		case DELETE_ARRAY_EXPORT_RECORD_READ:
			// Issue a VC Modify for the VC Id in the export record
			pVCRequest = new (tZERO) VCRequest;
			pModifyContext->pData1 = pVCRequest;

			pVcCmd = (VCCommand *)&pVCRequest->eCommand;
			pVCModifyCtl = (VCModifyCtlCommand *)&pVCRequest->u.VCModifyParms;

			*pVcCmd = k_eModifyVC;
			pVCModifyCtl->vdNext = pMember->memberVD;
			pVCModifyCtl->srcNext = pMember->memberRID;
			pVCModifyCtl->ridVcId = pExportEntry->ridVcId;


			pMsg = new MsgVCMModifyVC(pVCRequest);
			status = Send(
						pMsg,
						pModifyContext,
						REPLYCALLBACK(DdmRAIDMstr,DeleteArray_VCModifyMessageReply));
			break;

		default:
			break;
		} // end switch
	} // end else		

	if (status){
		delete pModifyContext;
		(this->*cb)(pOriginalContext,status);
	}
	return status;
}



//************************************************************************
//	VCMCommandCompletionReply
//
//************************************************************************
STATUS DdmRAIDMstr
::DeleteArray_VCModifyMessageReply(Message *pMsg)
{
	STATUS				status = pMsg->Status();

	CONTEXT				*pModifyContext = (CONTEXT *)pMsg->GetContext();
	CONTEXT				*pOriginalContext = NULL;
	pTSCallback_t		cb;
	
	pOriginalContext	= pModifyContext->pParentContext;
	cb					= pModifyContext->pCallback;

	delete pModifyContext;
	(this->*cb)(pOriginalContext,status);
	delete pMsg;
	return status;
}

