/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmTables.cpp
// 
// Description:
// This module is the Drive Monitor Table Update Handler. 
// Update methods are included for:
//	DiskDescriptor Table
//	StorageRollcall Table
//	TapeDescriptor Table
//	SESDescriptor Table
// 

// Update Log:
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DmTables.cpp $
// 
// 14    2/01/00 7:19p Mpanas
// Make sure we don't create a BSA vdn for SES or Tape
// 
// 13    1/11/00 7:27p Mpanas
// New PathTable changes
// 
// 12    10/12/99 8:33p Mpanas
// Add support for removed drives
// - Update DiskDescriptor with DriveRemoved
// - correct several bugs, flags not inverted correctly
// - Flag drives that have been changed (with a trace message only)
// 
// 11    10/07/99 8:09p Mpanas
// First cut of BSA VD Create
// 
// 10    9/14/99 8:41p Mpanas
// Complete re-write of DriveMonitor
// - Scan in sequence
// - Start Motors in sequence
// - LUN Scan support
// - Better table update
// - Re-organize sources
// 
// 9     8/16/99 1:54p Mpanas
// Update to latest PTS model
// 
// 8     8/14/99 9:37p Mpanas
// Support for new LoopMonitor
// 
// 7     7/21/99 10:13p Mpanas
// resolve latest PTS changes
// 
// 6     7/15/99 11:53p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Message front end for the DriveMonitor
// - remove all external entry points
// 
// 5     5/28/99 12:59p Mpanas
// match new StorageRollCall struct
// 
// 4     5/18/99 12:25p Mpanas
// Make all memory buffers UNCACHED
// 
// 3     5/10/99 5:37p Mpanas
// Fix all the new() types
// 
// 2     4/01/99 2:09p Mpanas
// Changes to match with new CHAOS
// 
// 1     3/22/99 8:19p Mpanas
// Initial Checkin
// Drive Monitor PTS support code
// 
// 03/04/99 Michael G. Panas: Create file
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

#include "DriveMonitorIsm.h"
#include "DmCommon.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"


// Table method context
typedef struct _DM_TBL_CONTEXT {
	U32								 flags;			// execution flags
	U32								 index;			// drive number index
	DM_CONTEXT						*pDmc;			// Drive Monitor internal context
	PathDescriptor					*pPDTableRow;
	DiskDescriptor					*pDDTableRow;
	DeviceDescriptor				*pTDTableRow;
	StorageRollCallRecord			*pRCTableRow;
	DM_DEVICE_STATE					*pDMState;		// device state
	DriveMonitorIsm::pDMCallback_t	 Callback;		// saved Callback address
} DM_TBL_CONTEXT, *PDM_TBL_CONTEXT;

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/

/*************************************************************************/
// DM_Check_PTS_Update_OK
// Check if we OK to update the PTS data.  If we are secondary and primary
// is active, no update.  Updates are always OK if, the DriveMonitor is
// not in Failover mode.
// Check to see if we are in our secondary failover slot and the
// DriveMonitor is in Failover Mode,
// return 1 if we are, 0 otherwise.
/*************************************************************************/
U32 DriveMonitorIsm::DM_Check_PTS_Update_OK(void)
{
	TRACE_ENTRY(DM_Check_PTS_Update_OK);

	if (config.flags & DM_FLAGS_REDUNDANT)
	{
		// check if we are secondary and primary is still active
		// TODO:
		// see if there is a better way to find this out using the OS
		if (config.flags & DM_FLAGS_SECONDARY)
		{
			return(0);
		}
	}

	// Update is OK
	return(1);

} // DM_Check_PTS_Update_OK




/*************************************************************************/
// DmTableUpdatePD
// Updates a PathDescriptor table row entry that has changed or been added.
// Build the context and call the first state in the state machine.
/*************************************************************************/
STATUS	DriveMonitorIsm::DmTableUpdatePD(DM_CONTEXT *pDmc, pDMCallback_t Callback)
{
	STATUS			status;
	DM_TBL_CONTEXT *pTC = new DM_TBL_CONTEXT;
	
	TRACE_ENTRY(DmTableUpdatePD);
	
	pTC->pDmc = pDmc;
	pTC->pDMState = pDmc->pDMState;
	pTC->index = pDmc->index;
	pTC->Callback = Callback;
	pTC->flags = 0;
	
	// make someplace to store the record we are reading
	pTC->pPDTableRow = new(tUNCACHED) PathDescriptor;
	pTC->pRCTableRow = NULL;
	
	// check if we need to add the row first
	if (pTC->pDMState->state & DEVICE_STATE_ADD_PATH)
	{
		status = DmTblUpdAddPDDesc(pTC, ercOK);
		
		return status;
	}
	
	// read/modify this row
	status = DmTblUpdReadPDDesc(pTC, ercOK);
	
	return status;
	
} // DmTableUpdatePD

/*************************************************************************/
// DmTblUpdReadPDDesc
// Read the PathDescriptor row that needs to be updated
/*************************************************************************/
STATUS 	DriveMonitorIsm::DmTblUpdReadPDDesc(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;

	TRACE_ENTRY(DmTblUpdReadPDDesc);
		
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblUpdReadPDDesc: status = ", status);
		
		// must have failed, don't retry
		status = DmTableUpdatePDEnd(pTC, status);
		return status;
	}
	
	// Allocate a ReadRow object for the PathDescriptor Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		PATH_DESC_TABLE,					// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		(void*)&pDMState->ridPD,			// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pTC->pPDTableRow,					// void* prgbRowDataRet,
		sizeof(PathDescriptor),				// U32 cbRowDataRetMax,
		&m_nTableRows,						// U32 *pcRowsReadRet,
		(pTSCallback_t)&DmTblUpdModifyPDDesc,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // DmTblUpdReadPDDesc

/*************************************************************************/
// DmTblUpdModifyPDDesc
// Modify a table row entry with the table data in the DM_DEVICE_STATE
// that has just been built.
/*************************************************************************/
STATUS 	DriveMonitorIsm::DmTblUpdModifyPDDesc(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 			*pTC = (DM_TBL_CONTEXT *)pClientContext;
	PathDescriptor			*pPD1, *pPD = pTC->pDMState->pPD;

	TRACE_ENTRY(DmTblUpdModifyPDDesc);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblUpdModifyPDDesc: status = ", status);
		if ((status == ercEOF) || (status == ercKeyNotFound))
		{
			status = DmTblUpdAddPDDesc(pTC, ercOK);
			return status;
		}
	}
	
	// PTS view of descriptor
	pPD1 = pTC->pPDTableRow;

	// Modify fields that changed (maybe)
	pPD1->CurrentStatus = pTC->pDMState->pDD->CurrentStatus;
	pPD1->vdnDdm = pPD->vdnDdm;
	// TODO: fix this later
	//pPD1->ridActiveDesc = pPD->ridActiveDesc;
	
	// Allocate an Modify Row object for the PathDescriptor table
	m_ModifyRow = new TSModifyRow;

	// Initialize the modify row operation.
	status = m_ModifyRow->Initialize(
		this,								// DdmServices pDdmServices,
		PATH_DESC_TABLE,					// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		(void*)&pTC->pDMState->ridPD,		// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pTC->pPDTableRow,					// void* prgbRowData,
		sizeof(PathDescriptor),				// U32 cbRowData,
		0,									// U32 cRowsToModify,
		NULL,								// U32 *pcRowsModifiedRet,
		NULL,								// rowID *pRowIDRet,
		0,									// U32 cbMaxRowID,
		(pTSCallback_t)&DmTableUpdatePDEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_ModifyRow->Send();
	
	return status;
	
} // DmTblUpdModifyPDDesc

/*************************************************************************/
// DmTblUpdAddPDDesc
// Tried to modify a row that did not exist, so now we must add it to
// the PathDescriptior table
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblUpdAddPDDesc(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;
	PathDescriptor		*pPD = pDMState->pPD;

	TRACE_ENTRY(DmTblUpdAddPDDesc);
	
	// row will be added
	pTC->flags = 1;
	
	// Fill in the known fields
	pPD->version = PATH_DESC_VERSION;
	pPD->size = sizeof(PathDescriptor);
	pPD->FCInstance = config.FC_instance;
	pPD->FCTargetID = config.xlate[pTC->pDmc->drive_number];
	pPD->FCTargetLUN = pTC->pDmc->lun_number;
	pPD->vdnMonitor = MyVd;
	pPD->InqType = pDMState->type;
		
	// disk/device descriptor should already be written to the PTS
	if ((pPD->InqType == SCSI_DEVICE_TYPE_DIRECT) ||
				(pPD->InqType == SCSI_DEVICE_TYPE_ARRAY_CONT))
	{
		pPD->ridDescriptor = pDMState->pDD->rid;

		// check to see if we found a BSA for this device already
		if (pDMState->state & DEVICE_STATE_VDN_FOUND)
		{
			// clear the state flag, since we only need to do this once
			pDMState->state &= ~DEVICE_STATE_VDN_FOUND;
			
			// add the BSA VDN to the PathDescriptor record
			pPD->vdnDdm = pDMState->Vdn;
		}
	
	}
	else
	{
		pPD->ridDescriptor = pDMState->pDevice->rid;
		
		// TODO:
		// do the same vdn thing for devices
	}
	
	// TODO:
	// Find a better way to do this...	
	// Active descriptor is the rid for our (Disk/Device)Descriptor if:
	// 1. we are non-redundant
	// 2. we are redundant and we are the primary
	if (((config.flags & DM_FLAGS_REDUNDANT) == 0) ||
			((config.flags & DM_FLAGS_REDUNDANT) &&
				((config.flags & DM_FLAGS_SECONDARY) == 0)))
	{
		pPD->ridActiveDesc = pPD->ridDescriptor;
	}
	
	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the PathDescriptor Table.
	m_pInsertRow = new TSInsertRow;
	
	status = m_pInsertRow->Initialize(
		this,							// Ddm* ClientDdm
		PATH_DESC_TABLE,				// prgbTableName
		pPD,							// prgbRowData
		sizeof(PathDescriptor),			// cbRowData
		&pPD->rid,						// *pRowIDRet
		(pTSCallback_t)&DmTableUpdatePDEnd,	// pTSCallback_t pCallback,
		(void*)pTC						// pContext
	);
	
	if (status == ercOK)
		m_pInsertRow->Send();

	return status;
	
} // DmTblUpdAddPDDesc

/*************************************************************************/
// DmTableUpdatePDEnd
// Last CallBack, check for add, do a callback if needed, delete data areas
// used and delete our context.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTableUpdatePDEnd(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;
	pDMCallback_t 		 Callback = pTC->Callback;
	
	TRACE_ENTRY(DmTableUpdatePDEnd);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L3, "\n\rDmTableUpdatePDEnd: status = ", status);
	}
	else
	{
		// if row was added, we need to save the entry in our DEVICE_STATE,
		// so we have the row ID for future updates
		if (pTC->flags)
		{
			// clear the add flag
			pDMState->state &= ~DEVICE_STATE_ADD_PATH;
			
			// save the row ID
			memcpy(&pDMState->ridPD, &pDMState->pPD->rid,
							sizeof(rowID));
		}
		
		// check for a BSA Virtual Device on storage only, create
		// one if it does not exist
		// TODO:  create the ES PassThru device for non-disk devices
		if ((pDMState->pPD->vdnDdm == 0) && 
				(pDMState->pPD->InqType == SCSI_DEVICE_TYPE_DIRECT) ||
				(pDMState->pPD->InqType == SCSI_DEVICE_TYPE_ARRAY_CONT))
		{
			// start the create statemachine, when done, come back
			// and update the PathDescriptor again with the new Vdn
			DM_Create_Bsa_Device(pTC, pDMState, &DmTblUpdReadPDDesc);
			return(ercOK);
		}
	}
			
	// Do a callback if there is one
	if (Callback)
		(this->*Callback)((void *)pTC->pDmc, status);
	
	// done with this
	if (pTC->pPDTableRow)
		delete pTC->pPDTableRow;

	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // DmTableUpdatePDEnd




/*************************************************************************/
// DmTableUpdateDD
// Updates a DiskDescriptor table row entry that has changed or been added.
// Build the context and call the first state in the state machine.
/*************************************************************************/
STATUS	DriveMonitorIsm::DmTableUpdateDD(DM_CONTEXT *pDmc, pDMCallback_t Callback)
{
	STATUS			status;
	DM_TBL_CONTEXT *pTC = new DM_TBL_CONTEXT;
	
	TRACE_ENTRY(DmTableUpdateDD);
	
	pTC->pDmc = pDmc;
	pTC->pDMState = pDmc->pDMState;
	pTC->index = pDmc->index;
	pTC->Callback = Callback;
	pTC->flags = 0;
	
	// make someplace to store the record we are reading
	pTC->pDDTableRow = new(tUNCACHED) DiskDescriptor;
	pTC->pRCTableRow = NULL;
	
	// check if we need to add the row first
	if (pTC->pDMState->state & DEVICE_STATE_ADD_DD)
	{
		status = DmTblUpdAddDesc(pTC, ercOK);
		
		return status;
	}
	
	// read/modify this row
	status = DmTblUpdReadDesc(pTC, ercOK);
	
	return status;
	
} // DmTableUpdateDD

/*************************************************************************/
// DmTblUpdReadDesc
// Read the DiskDescriptor row that needs to be updated
/*************************************************************************/
STATUS 	DriveMonitorIsm::DmTblUpdReadDesc(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;

	TRACE_ENTRY(DmTblUpdReadDesc);
		
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblUpdReadDesc: status = ", status);
		
		// Create Virtual Device must have failed, don't retry
		status = DmTableUpdateDDEnd(pTC, status);
		return status;
	}
	
	// Allocate a ReadRow object for the DiskStatusTable.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		DISK_DESC_TABLE,					// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		(void*)&pDMState->ridDD,			// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pTC->pDDTableRow,					// void* prgbRowDataRet,
		sizeof(DiskDescriptor),				// U32 cbRowDataRetMax,
		&m_nTableRows,						// U32 *pcRowsReadRet,
		(pTSCallback_t)&DmTblUpdModifyDesc,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // DmTblUpdReadDesc

/*************************************************************************/
// DmTblUpdModifyDesc
// Modify a table row entry with the table data in the DM_Disk_Desc[]
// that has just been built.
/*************************************************************************/
STATUS 	DriveMonitorIsm::DmTblUpdModifyDesc(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 			*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DiskDescriptor			*pDD1, *pDD = pTC->pDMState->pDD;

	TRACE_ENTRY(DmTblUpdModifyDesc);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblUpdModifyDesc: status = ", status);
		if ((status == ercEOF) || (status == ercKeyNotFound))
		{
			status = DmTblUpdAddDesc(pTC, ercOK);
			return status;
		}
	}
	
	// PTS view of descriptor
	pDD1 = pTC->pDDTableRow;

	if (pDD->CurrentStatus == DriveReady)
	{
		// check for any change in drive serial number if drive is ready
		// old serial is in: pDD1
		// new serial is in: pDD
		if (strncmp(pDD->SerialNumber, pDD1->SerialNumber, sizeof(String32)))
		{
			// serial number does not match, for now just scream at operator
			// maybe later send a LogEvent()
			TRACEF(TRACE_L3, ("\nSerial Number Mismatch, Slot %d, Id %d, LUN %d",
						pDD->SlotID,
						pTC->pDMState->pPD->FCTargetID,
						pTC->pDMState->pPD->FCTargetLUN));
			TRACEF(TRACE_L3, ("\n  Old: [%s]\n  New: [%s]",
						pDD1->SerialNumber, pDD->SerialNumber));
			
			// overwrite the old serial number with the new one
			memcpy(pDD1->SerialNumber, pDD->SerialNumber, sizeof(String32));
		}
	}
	
	// Modify fields that changed (maybe)
	pDD1->CurrentStatus = pDD->CurrentStatus;
	pDD1->Capacity = pDD->Capacity;
	pDD1->InqData = pDD->InqData;
	strcpy(pDD1->SerialNumber, pDD->SerialNumber);
	pDD1->LockState = pDD->LockState;
	
	// Allocate an Modify Row object for the DiskDescriptor Table.
	m_ModifyRow = new TSModifyRow;

	// Initialize the modify row operation.
	status = m_ModifyRow->Initialize(
		this,								// DdmServices pDdmServices,
		DISK_DESC_TABLE,					// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		(void*)&pTC->pDMState->ridDD,		// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pTC->pDDTableRow,					// void* prgbRowData,
		sizeof(DiskDescriptor),				// U32 cbRowData,
		0,									// U32 cRowsToModify,
		NULL,								// U32 *pcRowsModifiedRet,
		NULL,								// rowID *pRowIDRet,
		0,									// U32 cbMaxRowID,
		(pTSCallback_t)&DmTableUpdateDDEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_ModifyRow->Send();
	
	return status;
	
} // DmTblUpdModify

/*************************************************************************/
// DmTblUpdAddDesc
// Tried to modify a row that did not exist, so now we must add it to
// the DiskDescriptior table
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblUpdAddDesc(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;
	DiskDescriptor	*pDD = pTC->pDMState->pDD;

	TRACE_ENTRY(DmTblUpdAddDesc);
	
	// row will be added
	pTC->flags = 1;
	
	// Fill in the known fields
	pDD->version = DISK_DESC_VERSION;
	pDD->size = sizeof(DiskDescriptor);
	pDD->SlotID = pTC->pDmc->drive_number;
	pDD->LockState = LOCK_STATE_UNKNOWN;
	
	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the DiskDescriptor Table.
	m_pInsertRow = new(tUNCACHED) TSInsertRow;
	
	status = m_pInsertRow->Initialize(
		this,							// Ddm* ClientDdm
		DISK_DESC_TABLE,				// prgbTableName
		pDD,							// prgbRowData
		sizeof(DiskDescriptor),			// cbRowData
		&pDD->rid,						// *pRowIDRet
		(pTSCallback_t)&DmTableUpdateDDEnd,	// pTSCallback_t pCallback,
		(void*)pTC						// pContext
	);
	
	if (status == ercOK)
		m_pInsertRow->Send();

	return status;
	
} // DmTblUpdAddDesc

/*************************************************************************/
// DmTableUpdateDDEnd
// Last CallBack, check for add, do a callback if needed, delete data areas
// used and delete our context.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTableUpdateDDEnd(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;
	pDMCallback_t 		 Callback = pTC->Callback;
	
	TRACE_ENTRY(DmTableUpdateDDEnd);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L3, "\n\rDmTableUpdateDDEnd: status = ", status);
	}
	else
	{
		// if row was added, we need to save the entry in our DEVICE_STATE,
		// so we have the row ID for future updates
		if (pTC->flags)
		{
			// clear the add flag
			pDMState->state &= ~DEVICE_STATE_ADD_DD;
			
			// save the row ID
			memcpy(&pDMState->ridDD, &pDMState->pDD->rid,
							sizeof(rowID));
		}
	}
			
	// Do a callback if there is one
	if (Callback)
		(this->*Callback)((void *)pTC->pDmc, status);
	
	// done with this
	if (pTC->pDDTableRow)
		delete pTC->pDDTableRow;

	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // DmTableUpdateDDEnd




/*************************************************************************/
// DmTableUpdateDeviceDesc
// Updates a DeviceDescriptor table row entry that has changed or been added.
// Build the context and call the first state in the state machine.
/*************************************************************************/
STATUS	DriveMonitorIsm::DmTableUpdateDeviceDesc(DM_CONTEXT *pDmc, pDMCallback_t Callback)
{
	STATUS			status;
	DM_TBL_CONTEXT *pTC = new DM_TBL_CONTEXT;
	
	TRACE_ENTRY(DmTableUpdateDeviceDesc);
	
	pTC->pDmc = pDmc;
	pTC->pDMState = pDmc->pDMState;
	pTC->index = pDmc->index;
	pTC->Callback = Callback;
	pTC->flags = 0;
	
	// make someplace to store the record we are reading
	pTC->pTDTableRow = new(tUNCACHED) DeviceDescriptor;
	
	// check if we need to add the row first
	if (pTC->pDMState->state & DEVICE_STATE_ADD_DD)
	{
		status = DmTblUpdAddDevDesc(pTC, ercOK);
		
		return status;
	}
	
	// read/modify this row
	status = DmTblUpdReadDevDesc(pTC, ercOK);
	
	return status;
	
} // DmTableUpdateDeviceDesc

/*************************************************************************/
// DmTblUpdReadDevDesc
// Read the DeviceDescriptor row that needs to be updated
/*************************************************************************/
STATUS 	DriveMonitorIsm::DmTblUpdReadDevDesc(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;

	TRACE_ENTRY(DmTblUpdReadDevDesc);
		
	// Allocate a ReadRow object for the Tape Descriptor Table.
	m_pSRCReadRow = new(tUNCACHED) TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		DEVICE_DESC_TABLE,					// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		(void*)&pDMState->rid,				// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pTC->pTDTableRow,					// void* prgbRowDataRet,
		sizeof(DeviceDescriptor),				// U32 cbRowDataRetMax,
		&m_nTableRows,						// U32 *pcRowsReadRet,
		(pTSCallback_t)&DmTblUpdModifyDevDesc,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // DmTblUpdReadDevDesc

/*************************************************************************/
// DmTblUpdModifyDevDesc
// Modify a table row entry with the table data in the Container that may
// have been updated.
/*************************************************************************/
STATUS 	DriveMonitorIsm::DmTblUpdModifyDevDesc(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 			*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DeviceDescriptor		*pTD1, *pTD = (DeviceDescriptor *)pTC->pDMState->pDevice;

	TRACE_ENTRY(DmTblUpdModifyDevDesc);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblUpdModifyDevDesc: status = ", status);
		if ((status == ercEOF) || (status == ercKeyNotFound))
		{
			status = DmTblUpdAddDevDesc(pTC, ercOK);
			return status;
		}
	}
	
	// Modify fields that changed (maybe)
	pTD1 = pTC->pTDTableRow;
	pTD1->CurrentStatus = pTD->CurrentStatus;
	pTD1->InqData = pTD->InqData;
	strcpy(pTD1->SerialNumber, pTD->SerialNumber);
	
	// Allocate an Modify Row object for the DeviceDescriptor Table.
	m_ModifyRow = new(tUNCACHED) TSModifyRow;

	// Initialize the modify row operation.
	status = m_ModifyRow->Initialize(
		this,								// DdmServices pDdmServices,
		DEVICE_DESC_TABLE,					// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		(void*)&pTC->pDMState->rid,			// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pTC->pTDTableRow,					// void* prgbRowData,
		sizeof(DeviceDescriptor),			// U32 cbRowData,
		0,									// U32 cRowsToModify,
		NULL,								// U32 *pcRowsModifiedRet,
		NULL,								// rowID *pRowIDRet,
		0,									// U32 cbMaxRowID,
		(pTSCallback_t)&DmTableUpdateDevDEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_ModifyRow->Send();
	
	return status;
	
} // DmTblUpdModifyDevDesc

/*************************************************************************/
// DmTblUpdAddDevDesc
// Tried to modify a row that did not exist, so now we must add it to
// the DeviceDescriptior table
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblUpdAddDevDesc(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;
	DeviceDescriptor	*pTD = (DeviceDescriptor *)pTC->pDMState->pDevice;

	TRACE_ENTRY(DmTblUpdAddDevDesc);
	
	// row will be added
	pTC->flags = 1;
	
	// Fill in the known fields
	pTD->version = DEVICE_DESC_VERSION;
	pTD->size = sizeof(DeviceDescriptor);
	pTD->SlotID = pTC->pDmc->drive_number;
	
	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the DeviceDescriptor Table.
	m_pInsertRow = new(tUNCACHED) TSInsertRow;
	
	status = m_pInsertRow->Initialize(
		this,								// Ddm* ClientDdm
		DEVICE_DESC_TABLE,					// prgbTableName
		pTD,								// prgbRowData
		sizeof(DeviceDescriptor),			// cbRowData
		&pTD->rid,							// *pRowIDRet
		(pTSCallback_t)&DmTableUpdateDevDEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// pContext
	);
	
	if (status == ercOK)
		m_pInsertRow->Send();

	return status;
	
} // DmTblUpdAddDevDesc

/*************************************************************************/
// DmTableUpdateDevDEnd
// Last CallBack, check for add, do a callback if needed, delete data areas
// used and delete our context.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTableUpdateDevDEnd(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;
	pDMCallback_t 		 Callback = pTC->Callback;
	
	TRACE_ENTRY(DmTableUpdateDevDEnd);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L3, "\n\rDmTableUpdateDevDEnd: status = ", status);
	}
	else
	{
		// if row was added, we need to save the entry in our DEVICE_STATE,
		// so we have the row ID for future updates
		if (pTC->flags)
		{
			// clear the add flag
			pDMState->state &= ~DEVICE_STATE_ADD_DD;
			
			// save the row ID
			memcpy(&pDMState->rid, &((DeviceDescriptor *)pDMState->pDevice)->rid,
							sizeof(rowID));
		}
	}
			
	// Do a callback if there is one
	if (Callback)
		(this->*Callback)((void *)pTC->pDmc, status);
	
	// done with this
	if (pTC->pTDTableRow)
		delete pTC->pTDTableRow;

	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // DmTableUpdateDevDEnd




/*************************************************************************/
// DmTableUpdateRC
// Updates a StorageRollCall table row entry that has changed or been added.
// Build the context and call the first state in the state machine.
/*************************************************************************/
STATUS	DriveMonitorIsm::DmTableUpdateRC(DM_CONTEXT *pDmc, pDMCallback_t Callback)
{
	STATUS			 status;
	DM_TBL_CONTEXT	*pTC = new DM_TBL_CONTEXT;
	
	TRACE_ENTRY(DmTableUpdateRC);
	
	pTC->pDmc = pDmc;
	pTC->pDMState = pDmc->pDMState;
	pTC->index = pDmc->index;
	pTC->Callback = Callback;
	pTC->flags = 0;
	
	pTC->pDDTableRow = NULL;
	
	// need someplace to put the Rollcall Table
	pTC->pRCTableRow = new(tUNCACHED) StorageRollCallRecord;

	// check if we need to add the row first
	if (pTC->pDMState->state & DEVICE_STATE_ADD_RC)
	{
		status = DmTblUpdAddRollCall(pTC, ercOK);
		
		return status;
	}
	
	// Read/Modify
	status = DmTblUpdReadRollCall(pTC, ercOK);
	
	return status;
	
} // DmTableUpdateRC

/*************************************************************************/
// DmTblUpdReadRollCall
// Check for a RollCall entry by reading it, add it if it does not exist
/*************************************************************************/
STATUS 	DriveMonitorIsm::DmTblUpdReadRollCall(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;
	rowID				*pRI;

	TRACE_ENTRY(DmTblUpdReadRollCall);
	
	// Allocate an ReadRow object for the StorageRollCall table.
	m_pSRCReadRow = new(tPCI) TSReadRow;
	
	// check if we need to do the initial read to get the row ID
	if (pDMState->state & DEVICE_STATE_READ_RC)
	{
		// figure out what descriptor to use as the key
		// Key must in a static data area
		switch(pTC->pDMState->type)
		{
			case SCSI_DEVICE_TYPE_DIRECT:
			case SCSI_DEVICE_TYPE_ARRAY_CONT:
			{
				DiskDescriptor		*pDD = pDMState->pDD;
				
				pRI = &pDD->rid;
			}
				break;
			
			case SCSI_DEVICE_TYPE_SEQUENTIAL:
			{
				pRI = &pDMState->pDevice->rid;
			}
				break;
			
			case SCSI_DEVICE_TYPE_ENCLOSURE_SERVICES:
			{
				pRI = &pDMState->pDevice->rid;
			}
				break;
			
			default:
				// don't know what this is
				status = DmTableUpdateRCEnd(pTC, ercOK);
				return status;
				break;
			
		}
	
		// Initialize the read row operation.
		status = m_pSRCReadRow->Initialize(
			this,								// DdmServices pDdmServices,
			STORAGE_ROLL_CALL_TABLE,			// String64 rgbTableName,
			"ridDescriptorRecord",				// String64 prgbKeyFieldName,
			(void *)pRI,						// void* pKeyFieldValue,
			sizeof(rowID),						// U32 cbKeyFieldValue,
			pTC->pRCTableRow,					// void* prgbRowDataRet,
			sizeof(StorageRollCallRecord),		// U32 cbRowDataRetMax,
			NULL,								// U32 *pcRowsReadRet,
			(pTSCallback_t)&DmTblUpdModifyRollCall,	// pTSCallback_t pCallback,
			(void*)pTC							// void* pContext
		);
		
	}
	else
	{
		// already have the rowID
		// Initialize the read row operation.
		status = m_pSRCReadRow->Initialize(
			this,								// DdmServices pDdmServices,
			STORAGE_ROLL_CALL_TABLE,			// String64 rgbTableName,
			"rid",								// String64 prgbKeyFieldName,
			&pDMState->ridRC,					// void* pKeyFieldValue,
			sizeof(rowID),						// U32 cbKeyFieldValue,
			pTC->pRCTableRow,					// void* prgbRowDataRet,
			sizeof(StorageRollCallRecord),		// U32 cbRowDataRetMax,
			NULL,								// rowID *pcRowsReadRet,
			(pTSCallback_t)&DmTblUpdModifyRollCall,	// pTSCallback_t pCallback,
			(void*)pTC							// void* pContext
		);
		
	}

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // DmTblUpdReadRollCall

/*************************************************************************/
// DmTblUpdModifyRollCall
// Modify a table row entry with the table data in the DM_Disk_Desc[]
// that has just been built.
/*************************************************************************/
STATUS 	DriveMonitorIsm::DmTblUpdModifyRollCall(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 			*pTC = (DM_TBL_CONTEXT *)pClientContext;
	rowID					*pRI;
	DiskDescriptor			*pDD = pTC->pDMState->pDD;
	StorageRollCallRecord	*pRCR = pTC->pRCTableRow;

	TRACE_ENTRY(DmTblUpdModifyRollCall);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblUpdModifyRollCall: status = ", status);
		if ((status == ercEOF) || (status == ercKeyNotFound))
		{
			status = DmTblUpdAddRollCall(pTC, ercOK);
			return status;
		}
	}
	
	// check if we did an initial read of the StorageRollCall entry
	if (pTC->pDMState->state & DEVICE_STATE_READ_RC)
	{
		// clear the read flag
		pTC->pDMState->state &= ~DEVICE_STATE_READ_RC;
		
		// copy the data to the local container
		memcpy(pTC->pDMState->pRC, pTC->pRCTableRow, sizeof(StorageRollCallRecord));
		memcpy(&pTC->pDMState->ridRC, &pTC->pRCTableRow->rid, sizeof(rowID));
	}	

	// Modify fields that changed (maybe)
	switch(pTC->pDMState->type)
	{
		case SCSI_DEVICE_TYPE_DIRECT:
		case SCSI_DEVICE_TYPE_ARRAY_CONT:
		{
			DiskDescriptor			*pDD = pTC->pDMState->pDD;
			
			// fill in fields used for disks
			pRI = &pDD->rid;
			pRCR->Capacity = pDD->Capacity.LowPart;
		}
			break;
		
		case SCSI_DEVICE_TYPE_SEQUENTIAL:
		{
			// only the row ID for this descriptor is needed
			pRI = &pTC->pDMState->pDevice->rid;

			pRCR->storageclass = SRCTypeTape;
		}
			break;
		
		case SCSI_DEVICE_TYPE_ENCLOSURE_SERVICES:
			// not really a storage type, but needed for
			// configuration purposes
		{
			// only the row ID for this descriptor is needed
			pRI = &pTC->pDMState->pDevice->rid;

			pRCR->storageclass = SRCTypeSES;
		}
			break;
		
		default:
			// don't know what this is
			pRCR->storageclass = (SRCStorageTypes)-1;
			break;
		
	}

	// Allocate an Modify Row object for the DiskStatusTable.
	m_ModifyRow = new(tUNCACHED) TSModifyRow;

	// Initialize the modify row operation.
	status = m_ModifyRow->Initialize(
		this,								// DdmServices pDdmServices,
		STORAGE_ROLL_CALL_TABLE,			// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		(void*)&pTC->pDMState->ridRC,		// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pTC->pRCTableRow,					// void* prgbRowData,
		sizeof(StorageRollCallRecord),		// U32 cbRowData,
		0,									// U32 cRowsToModify,
		NULL,								// U32 *pcRowsModifiedRet,
		NULL,								// rowID *pRowIDRet,
		0,									// U32 cbMaxRowID,
		(pTSCallback_t)&DmTableUpdateRCEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_ModifyRow->Send();
	
	return status;
	
} // DmTblUpdModifyRollCall

/*************************************************************************/
// DmTblUpdAddRollCall
// Row did not exist, so now we must add it to the StorageRollCall table.
// Only three device types are supported so far: Disk, Tape and SES.  Each
// of these devices have a different descriptor, so are handled seperately.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblUpdAddRollCall(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 			*pTC = (DM_TBL_CONTEXT *)pClientContext;
	StorageRollCallRecord	*pRCR = pTC->pDMState->pRC;

	TRACE_ENTRY(DmTblUpdAddRollCall);
	
	// fill in all the common RollCall entries
	pRCR->version = STORAGE_ROLL_CALL_TABLE_VERSION;
	pRCR->size = sizeof(StorageRollCallRecord);
	pRCR->vdnMonitor = MyVd;
	pRCR->fUsed = 0;
	// could be BSA or EchoScsi vdn
	pRCR->vdnBSADdm = pTC->pDMState->pPD->vdnDdm;
	
	// decide what storage type to specify
	switch(pTC->pDMState->type)
	{
		case SCSI_DEVICE_TYPE_DIRECT:
		case SCSI_DEVICE_TYPE_ARRAY_CONT:
		{
			DiskDescriptor			*pDD = pTC->pDMState->pDD;
			
			// fill in fields used for disks
			pRCR->ridDescriptorRecord = pDD->rid;
			pRCR->Capacity = pDD->Capacity.LowPart;
			
			if (pDD->DiskType == TypeFCDisk)
			{
				pRCR->storageclass = SRCTypeFCDisk;
			}
			else if (pDD->DiskType == TypeExternalFCDisk)
			{
				pRCR->storageclass = SRCTypeExternalFCDisk;
			}
			else
				pRCR->storageclass = SRCTypeUnknown;
		}
			break;
		
		case SCSI_DEVICE_TYPE_SEQUENTIAL:
		{
			DeviceDescriptor			*pDvD = (DeviceDescriptor *)pTC->pDMState->pDevice;
			
			// only the row ID for this descriptor is needed
			pRCR->ridDescriptorRecord = pDvD->rid;

			pRCR->storageclass = SRCTypeTape;
		}
			break;
		
		case SCSI_DEVICE_TYPE_ENCLOSURE_SERVICES:
			// not really a storage type, but needed for
			// configuration purposes
		{
			DeviceDescriptor			*pDvD = (DeviceDescriptor *)pTC->pDMState->pDevice;
			
			// only the row ID for this descriptor is needed
			pRCR->ridDescriptorRecord = pDvD->rid;

			pRCR->storageclass = SRCTypeSES;
		}
			break;
		
		default:
			// don't know what this is
			pRCR->storageclass = (SRCStorageTypes)-1;
			break;
		
	}

	// row will be added
	pTC->flags = 1;
	
	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the StorageRollCallTable.
	m_pInsertRow = new(tUNCACHED) TSInsertRow;
	
	status = m_pInsertRow->Initialize(
		this,							// Ddm* ClientDdm
		STORAGE_ROLL_CALL_TABLE,		// prgbTableName
		pRCR,							// prgbRowData
		sizeof(StorageRollCallRecord),	// cbRowData
		&pRCR->rid,						// *pRowIDRet
		(pTSCallback_t)&DmTableUpdateRCEnd,	// pTSCallback_t pCallback,
		(void*)pTC						// pContext
	);
	
	if (status == ercOK)
		m_pInsertRow->Send();

	return status;
} // DmTblUpdAddRollCall

/*************************************************************************/
// DmTableUpdateRCEnd
// Complete the table row entry update. Call the Finish routine
/*************************************************************************/
STATUS	DriveMonitorIsm::DmTableUpdateRCEnd(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;
	pDMCallback_t 		 Callback = pTC->Callback;

	TRACE_ENTRY(DmTableUpdateRCEnd);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTableUpdateRCEnd: status = ", status);
	}
	else
	{
		// if row was added, we need to copy it to the DM_DEVICE_STATE,
		// so we have the rowID and we can compare changed fields later
		if (pTC->flags)
		{
			// clear the add flag
			pDMState->state &= ~DEVICE_STATE_ADD_RC;
			
			// save the row ID
			memcpy(&pDMState->ridRC, &pDMState->pRC->rid,
							sizeof(rowID));
		}
	}
				
	// Do a callback if there is one
	if (Callback)
		(this->*Callback)((void *)pTC->pDmc, status);
	
	// done with these
	if (pTC->pRCTableRow)
		delete pTC->pRCTableRow;
	delete pTC;
	
	return ercOK;
	
} // DmTableUpdateRCEnd




/*************************************************************************/
// DmFindDescriptor
// Find the Disk or Device Descriptor that matches the current Inquiry 
// Serial Number.  Read the whole descriptor table if the number of
// entries has changed.
/*************************************************************************/
STATUS DriveMonitorIsm::DmFindDescriptor(DM_CONTEXT *pDmc, pDMCallback_t Callback)
{
	STATUS				 status;
	DM_TBL_CONTEXT 		*pTC = new DM_TBL_CONTEXT;
	DM_DEVICE_STATE		*pDMState = pDmc->pDMState;
	
	TRACE_ENTRY(DmFindDescriptor);
	
	pTC->pDmc = pDmc;
	pTC->pDMState = pDMState;
	pTC->Callback = Callback;
	pTC->flags = 0;
	
	// check the type of device
	if ((pDMState->pPD->InqType == SCSI_DEVICE_TYPE_DIRECT) ||
				(pDMState->pPD->InqType == SCSI_DEVICE_TYPE_ARRAY_CONT))
	{
		status = DmFindDiskDescriptor(pTC, ercOK);
		
		return status;
	}
	
	// handle other devices
	status = DmFindDeviceDescriptor(pTC, ercOK);
	
	return status;
} // DmFindDescriptor

/*************************************************************************/
// DmFindDiskDescriptor
// Use a ReadRow to match our serial number in the DiskDescriptor table
/*************************************************************************/
STATUS DriveMonitorIsm::DmFindDiskDescriptor(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;

	TRACE_ENTRY(DmFindDiskDescriptor);
	
	m_nTableRowsRead = 0;

	// check for a prior table
	if (DM_TS_Disk_Desc)
	{
		delete DM_TS_Disk_Desc;
	}
	
	// Allocate space for the Local DiskDescriptor Table
	DM_TS_Disk_Desc = new DiskDescriptor;
		
	// Allocate a ReadRow object for the DiskDescriptor Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		DISK_DESC_TABLE,					// String64 rgbTableName,
		"SerialNumber",						// String64 prgbKeyFieldName,
		&pDMState->pDD->SerialNumber,		// void* pKeyFieldValue,
		sizeof(String32),					// U32 cbKeyFieldValue,
		DM_TS_Disk_Desc,					// void* prgbRowData,
		sizeof(DiskDescriptor), 			// U32 cbRowDataRetMax, max size
		&m_nTableRowsRead,					// U32 *pcRowsReadRet,
		(pTSCallback_t)&DmFindDiskDescriptor1,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the ReadRow table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	else
		status = DmFindDiskDescriptor1(pTC, status);
	
	return status;
	
} // DmFindDiskDescriptor



/*************************************************************************/
// DmFindDiskDescriptor1
// check the entire set of an existing DiskDescriptor entries.  If we have
// one that matchs our seearch serial number, read all the PathDescriptors
// match this rid.  Otherwise add a new DiskDescriptor for our entry.
/*************************************************************************/
STATUS DriveMonitorIsm::DmFindDiskDescriptor1(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;

	TRACE_ENTRY(DmFindDiskDescriptor1);
	
	if (status != ercOK)
	{
		// No match on our serial number, show the error
		TRACE_HEX(TRACE_L8, "\n\rDmFindDiskDescriptor1: status = ", status);
		
		// and skip the compare
		status = DmFindDescriptorEnd(pTC, ercEOF);
		return (status);
	}
	
	// found a serial number match, copy descriptor to our DEVICE_STATE
	memcpy(pDMState->pDD, DM_TS_Disk_Desc, sizeof(DiskDescriptor));

	// copy the DiskDescriptor rowID data just read
	memcpy(&pDMState->ridDD, &DM_TS_Disk_Desc->rid, sizeof(rowID));									

	// descriptor now read
	pDMState->state &= ~DEVICE_STATE_ADD_DD;
	
	// now read all Paths to the descriptor just found
	
	status = DmFindPathDescriptors(pTC, 0);
	return status;

} // DmFindDiskDescriptor1


/*************************************************************************/
// DmFindDeviceDescriptor
// Read how many entries (rows) there currently are.  Use this count to
// build the array for local use.
/*************************************************************************/
STATUS DriveMonitorIsm::DmFindDeviceDescriptor(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;

	TRACE_ENTRY(DmFindDeviceDescriptor);
	
	m_nTableRowsRead = 0;

	// check for a prior table
	if (DM_TS_Device_Desc)
	{
		delete DM_TS_Device_Desc;
	}
	
	// Allocate space for the Local DeviceDescriptor Table
	DM_TS_Device_Desc = new DeviceDescriptor;
		
	// Allocate a ReadRow object for the DeviceDescriptor Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		DEVICE_DESC_TABLE,					// String64 rgbTableName,
		"SerialNumber",						// String64 prgbKeyFieldName,
		&pDMState->pDD->SerialNumber,		// void* pKeyFieldValue,
		sizeof(String32),					// U32 cbKeyFieldValue,
		DM_TS_Device_Desc,					// void* prgbRowData,
		sizeof(DeviceDescriptor), 			// U32 cbRowDataRetMax, max size
		&m_nTableRowsRead,					// U32 *pcRowsReadRet,
		(pTSCallback_t)&DmFindDeviceDescriptor1,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the ReadRow table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	else
		status = DmFindDeviceDescriptor1(pTC, status);
	
	return status;
	
} // DmFindDeviceDescriptor


/*************************************************************************/
// DmFindDeviceDescriptor1
// check the entire set of an existing DiskDescriptor entries.  If we have
// one that matchs our search serial number, read all the PathDescriptors
// that match this rid.  Otherwise add a new DiskDescriptor for our entry.
/*************************************************************************/
STATUS DriveMonitorIsm::DmFindDeviceDescriptor1(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;

	TRACE_ENTRY(DmFindDeviceDescriptor1);
	
	if (status != ercOK)
	{
		// No match on our serial number, show the error
		TRACE_HEX(TRACE_L8, "\n\rDmFindDeviceDescriptor1: status = ", status);
		
		// and skip looking for paths
		status = DmFindDescriptorEnd(pTC, ercEOF);
		return (status);
	}
	
	// found a serial number match, copy descriptor to our DEVICE_STATE
	memcpy(pDMState->pDevice, DM_TS_Device_Desc, sizeof(DeviceDescriptor));

	// copy the DeviceDescriptor rowID data just read
	memcpy(&pDMState->rid, &DM_TS_Device_Desc->rid, sizeof(rowID));									

	// descriptor now read
	pDMState->state &= ~DEVICE_STATE_ADD_DD;
	
	// now read all Paths to the descriptor just found
	
	status = DmFindPathDescriptors(pTC, 0);
	return status;

} // DmFindDeviceDescriptor1


/*************************************************************************/
// DmFindPathDescriptors
// Read all the PathDescriptors that match our Disk/Device Descriptor rid.
/*************************************************************************/
STATUS DriveMonitorIsm::DmFindPathDescriptors(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;
	rowID				*prid;

	TRACE_ENTRY(DmFindPathDescriptors);
	
	// pass the correct rid by checking the type of device
	if ((pDMState->pPD->InqType == SCSI_DEVICE_TYPE_DIRECT) ||
				(pDMState->pPD->InqType == SCSI_DEVICE_TYPE_ARRAY_CONT))
	{
		prid = &pDMState->ridDD;
	}
	else
	{
		prid = &pDMState->rid;
	}
	
	// Allocate a ReadRow object for the DiskDescriptor Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation with auto-allocate
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		PATH_DESC_TABLE,					// String64 rgbTableName,
		"ridDescriptor",					// String64 prgbKeyFieldName,
		prid,								// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		&DM_TS_Path_Desc,					// void* prgbRowData,
		0, 									// U32 cbRowDataRetMax, max size
		&m_nTableRowsRead,					// U32 *pcRowsReadRet,
		(pTSCallback_t)&DmFindPathDescriptors1,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the read row table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // DmFindPathDescriptors


/*************************************************************************/
// DmFindPathDescriptors1
/*************************************************************************/
STATUS DriveMonitorIsm::DmFindPathDescriptors1(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;
	pDMCallback_t 		 Callback = pTC->Callback;

	TRACE_ENTRY(DmFindPathDescriptors1);
	
	if ((status != ercOK) || (m_nTableRowsRead == 0))
	{
		// show the error
		TRACE_HEX(TRACE_L8, "\n\rDmFindPathDescriptors1: status = ", status);
		
		// and skip the compare, the same as a not found condition
		status = DmFindDescriptorEnd(pTC, ercEOF);
		return (status);
	}
	
	// found some path entries, get the BSA vdn
	pDMState->pPD->vdnDdm = DM_TS_Path_Desc->vdnDdm;	
	
	// turn on the flags needed to add our new path descriptor
	pDMState->state |= DEVICE_STATE_ADD_PATH;
	
	// Do a callback if there is one
	if (Callback)
		(this->*Callback)((void *)pTC->pDmc, status);
	
	delete pTC;
	
	return status;
	
} // DmFindPathDescriptors1


/*************************************************************************/
// DmFindDescriptorEnd
// No match was found, continue on with the Descriptor update by making
// new entries for the Disk/Device Descriptor, the PathDescriptor and
// the StorageRollCall
/*************************************************************************/
STATUS DriveMonitorIsm::DmFindDescriptorEnd(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;
	pDMCallback_t 		 Callback = pTC->Callback;

	TRACE_ENTRY(DmFindDescriptorEnd);
	
	// turn on the flags needed to add a new device
	pDMState->state |= DEVICE_STATE_ADD_DD|
				DEVICE_STATE_ADD_RC|
				DEVICE_STATE_ADD_PATH;
	
	// Do a callback if there is one
	if (Callback)
		(this->*Callback)((void *)pTC->pDmc, status);
	
	delete pTC;
	
	return status;

} // DmFindDescriptorEnd


