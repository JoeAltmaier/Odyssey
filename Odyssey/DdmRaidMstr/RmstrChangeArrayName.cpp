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
// File: RmstrChangeArrayName.cpp
// 
// Description:
// Implementation for the change array name command
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrChangeArrayName.cpp $
// 
// 12    8/14/99 1:37p Dpatel
// Added event logging..
// 
// 11    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 10    7/30/99 6:47p Dpatel
// Removed INSUFFICIENT CAP and INVALID STORAGE_ELEMENT
// 
// 9     7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 8     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 7     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 6     7/20/99 6:49p Dpatel
// Some bug fixes and changed arrayName in ArrayDescriptor to rowID.
// 
// 5     7/17/99 1:20p Dpatel
// Queued up commands.
// 
// 4     7/16/99 10:28a Dpatel
// Added DownMember and Commit Spare code. Also removed the reads for
// validation.
// 
// 3     7/09/99 5:26p Dpatel
// 
// 2     6/28/99 5:16p Dpatel
// Implemented new methods, changed headers.
// 
//
// 06/11/99 Dipam Patel: Create file
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

#if 0
// Currently we are not using this API anymore...


// Change Array Name Validaiton States
enum
{
	CHANGE_NAME_ADT_RECORD_UPDATED = 1,
	CHANGE_NAME_ARRAY_NAME_READ,
	CHANGE_NAME_OLD_NAME_DELETED,
	CHANGE_NAME_NEW_NAME_WRITTEN
};

// Change Array Name Validaiton States
enum
{
	CHANGE_NAME_VALIDATION_ADT_ENUMERATED = 100,
	CHANGE_NAME_VALIDATION_DUPLICATE_NAME_CHECKED
};


//////////////////////////////////////////////////////////////////////
//
//	Change The Array Name
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstr::
ChangeArrayName(HANDLE h, RMSTR_CMND_INFO *pCmndInfo)
{
	return ChangeArrayNameValidation(h, pCmndInfo);
}	


//////////////////////////////////////////////////////////////////////
//
//	VALIDATION DONE, now change the actual name
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstr::
ChangeTheArrayName(
			HANDLE						handle,
			RMSTR_CMND_INFO				*_pCmdInfo,
			RAID_ARRAY_DESCRIPTOR		*_pADTRecord)
{
	STATUS							status = 0;
	CONTEXT							*pCmdContext = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_CHANGE_ARRAY_NAME_INFO	*pChangeArrayNameInfo = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;

	pCmdContext = new CONTEXT;

	// copy the cmd info into context
	pCmdContext->cmdHandle = handle;

	pCmdContext->pData = new RMSTR_CMND_INFO;
	memcpy(pCmdContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));

	pCmdContext->pData1 = new RAID_ARRAY_DESCRIPTOR;
	memcpy(pCmdContext->pData1,_pADTRecord,sizeof(RAID_ARRAY_DESCRIPTOR));
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;

	pCmdInfo				= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pChangeArrayNameInfo	=
		(RMSTR_CHANGE_ARRAY_NAME_INFO *)&pCmdParams->changeArrayNameInfo;

	pCmdContext->state = CHANGE_NAME_ARRAY_NAME_READ;
	status = m_pStringResourceManager->ReadString(
					&m_ucArrayName,
					pADTRecord->arrayNameRID,
					(pTSCallback_t)&DdmRAIDMstr::ProcessChangeArrayNameReply,
					pCmdContext);
	return status;
}	




//////////////////////////////////////////////////////////////////////
//
//	Change Array Name Reply Handler
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstr::
ProcessChangeArrayNameReply(void *_pCmdContext, STATUS status)
{
	RMSTR_CHANGE_ARRAY_NAME_INFO	*pChangeArrayNameInfo = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	STATUS							rc = RMSTR_SUCCESS;
	BOOL							cmdComplete = false;	
	CONTEXT							*pCmdContext = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RMSTR_EVT_ARRAY_NAME_CHANGED_STATUS	*pEvtArrayNameChanged = NULL;
	UnicodeString					ucNewArrayName;



	pCmdContext				= (CONTEXT *)_pCmdContext;
	pCmdInfo				= (RMSTR_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pChangeArrayNameInfo		=
		(RMSTR_CHANGE_ARRAY_NAME_INFO *)&pCmdParams->changeArrayNameInfo;
	pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pCmdContext->pData1;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){

		case CHANGE_NAME_ARRAY_NAME_READ:
			// old array name is read into m_ucArrayName
			
			// Delete the old name from the table
			pCmdContext->state = CHANGE_NAME_OLD_NAME_DELETED;
			m_pStringResourceManager->DeleteString(
						RowId(pADTRecord->arrayNameRID),
						(pTSCallback_t)&DdmRAIDMstr::ProcessChangeArrayNameReply,
						pCmdContext);
			break;

		case CHANGE_NAME_OLD_NAME_DELETED:
			// Now write the new array name into the table
			ucNewArrayName = UnicodeString(
									pChangeArrayNameInfo->newName);
			pCmdContext->state = CHANGE_NAME_NEW_NAME_WRITTEN;
			m_pStringResourceManager->WriteString(
						ucNewArrayName,
						&pADTRecord->arrayNameRID,
						(pTSCallback_t)&DdmRAIDMstr::ProcessChangeArrayNameReply,
						pCmdContext);
			break;

		case CHANGE_NAME_NEW_NAME_WRITTEN:
			pCmdContext->state = CHANGE_NAME_ADT_RECORD_UPDATED;
			status = m_pTableServices->TableServiceModifyRow(
						RAID_ARRAY_DESCRIPTOR_TABLE,
						&pADTRecord->thisRID,	// row id to modify
						pADTRecord,
						sizeof(RAID_ARRAY_DESCRIPTOR),
						&pADTRecord->thisRID,
						(pTSCallback_t)&DdmRAIDMstr::ProcessChangeArrayNameReply,
						pCmdContext);
			break;

		case CHANGE_NAME_ADT_RECORD_UPDATED:
			// cmd is complete, report the status and generate event
			ModifyRmstrData(
				RAID_ARRAY,
				&pADTRecord->thisRID,
				pADTRecord);
			cmdComplete = true;
			break;

		default:
			break;
		}
	}
	if (cmdComplete){
		// Report the status of the change array name
		m_pCmdServer->csrvReportCmdStatus(
			pCmdContext->cmdHandle,		// handle
			rc,							// completion code
			NULL,						// result Data
			(void *)pCmdInfo);			// Orig cmd info

		if (rc == RMSTR_SUCCESS){
			// Generate event for Array Name Changed
			pEvtArrayNameChanged = new RMSTR_EVT_ARRAY_NAME_CHANGED_STATUS;
			memset(pEvtArrayNameChanged,0,sizeof(RMSTR_EVT_ARRAY_NAME_CHANGED_STATUS));

			pEvtArrayNameChanged->arrayRowId = pChangeArrayNameInfo->arrayRowId;

			// Copy the old and new name back to the event
			// Copy the Array Name back in the event
			m_ucArrayName.CString(
					pEvtArrayNameChanged->oldName, 
					sizeof(UnicodeString32));		
			UnicodeString ucNewArrayName = UnicodeString(
									pChangeArrayNameInfo->newName);
			ucNewArrayName.CString(
					pEvtArrayNameChanged->newName, 
					sizeof(UnicodeString32));		

			m_pCmdServer->csrvReportEvent(
				RMSTR_EVT_ARRAY_NAME_CHANGED,	// completion code
				pEvtArrayNameChanged);				// event Data
			delete pEvtArrayNameChanged;
			pEvtArrayNameChanged = NULL;
		}
		StopCommandProcessing(true, pCmdContext->cmdHandle);
		delete pCmdContext;
	}
	return status;
}



//////////////////////////////////////////////////////////////////////
//
//	VALIDATION CODE FOR CHANGE ARRAY NAME
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstr::
ChangeArrayNameValidation(HANDLE h, RMSTR_CMND_INFO *_pCmdInfo)
{
	STATUS			status = RMSTR_SUCCESS;

	// check if duplicate name
	CONTEXT	*pValidationContext			= new CONTEXT;
	memset((void *)pValidationContext,0,sizeof(CONTEXT));

	// save the CHANGE ARRAY NAME INFO and the handle
	pValidationContext->pData		= new RMSTR_CMND_INFO;
	memcpy(pValidationContext->pData,_pCmdInfo,sizeof(RMSTR_CMND_INFO));

	pValidationContext->cmdHandle	= h;
	pValidationContext->state		= CHANGE_NAME_VALIDATION_ADT_ENUMERATED;

	status = m_pTableServices->TableServiceEnumTable(
				RAID_ARRAY_DESCRIPTOR_TABLE,	// tableName
				&pValidationContext->pData1,	// table data returned
				&pValidationContext->value1,	// data returned size
				&pValidationContext->value,		// number of rows returned here
				pValidationContext,				// context
				(pTSCallback_t)&DdmRAIDMstr::ProcessChangeArrayNameValidationReply);
	return status;
}

//////////////////////////////////////////////////////////////////////
//
//	Change Array Name Validation Reply Handler
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstr::
ProcessChangeArrayNameValidationReply(void *_pContext, STATUS status)
{
	CONTEXT							*pValidationContext = NULL;
	RMSTR_CHANGE_ARRAY_NAME_INFO	*pChangeArrayNameInfo = NULL;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	STATUS							rc;
	BOOL							validationComplete = false;

	rc						= RMSTR_SUCCESS;
	pValidationContext		= (CONTEXT *)_pContext;
	pCmdInfo				= (RMSTR_CMND_INFO *)pValidationContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pChangeArrayNameInfo	= 
				(RMSTR_CHANGE_ARRAY_NAME_INFO *)&pCmdParams->changeArrayNameInfo;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = RMSTR_ERR_INVALID_COMMAND;
		validationComplete = true;
	} else {
		switch(pValidationContext->state){
		case CHANGE_NAME_VALIDATION_ADT_ENUMERATED:
			// pContext contains the validation context
			// pValidationContext->pData = cmdInfo
			// pValidationContext->pData1 = enum data		
			// pData2 = NULL
			// pData3 = NULL

			pValidationContext->state = CHANGE_NAME_VALIDATION_DUPLICATE_NAME_CHECKED;
			rc = rmstrServiceCheckDuplicateArrayName(
						pValidationContext->pData1,		// enum data
						pValidationContext->value,		// number rows
						&pValidationContext->value1,	// will return success/failure
						pChangeArrayNameInfo->newName,
						(pTSCallback_t)&DdmRAIDMstr::ProcessChangeArrayNameValidationReply,
						pValidationContext);
			break;

		case CHANGE_NAME_VALIDATION_DUPLICATE_NAME_CHECKED:
			// delete the enum data
			if (pValidationContext->pData1){
				delete pValidationContext->pData1;
				pValidationContext->pData1 = NULL;
			}
			rc = pValidationContext->value1;	// if duplicate name exists
			if (rc == RMSTR_SUCCESS){
				GetRmstrData(
						RAID_ARRAY,
						&pChangeArrayNameInfo->arrayRowId,
						(void **)&pADTRecord);
				if (pADTRecord){	
					if (memcmp(
							&pADTRecord->thisRID,
							&pChangeArrayNameInfo->arrayRowId,
							sizeof(rowID)) == 0) {
								ChangeTheArrayName(
									pValidationContext->cmdHandle,
									pCmdInfo,
									pADTRecord);
					} else {
						rc = RMSTR_ERR_INVALID_COMMAND;
					}
				} else {
					rc = RMSTR_ERR_INVALID_COMMAND;
				}
			}
			validationComplete = true;
			break;

		default:
			break;
		}
	}

	if (validationComplete){
		if(rc){
			// Report error to RMSTR SQ
			m_pCmdServer->csrvReportCmdStatus(
				pValidationContext->cmdHandle,	// handle
				rc,								// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// pCQRecord
			StopCommandProcessing(true,pValidationContext->cmdHandle);
		}

		if (pValidationContext){
			delete pValidationContext;
			pValidationContext = NULL;
		}
	}
	return rc;
}

#endif

