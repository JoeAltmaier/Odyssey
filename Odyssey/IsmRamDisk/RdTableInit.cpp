/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RdTableInit.cpp
// 
// Description:
// This module handles Table Initialization and initial startup for
// the Ram Disk Module.  This code adds an entry in the StorageRollCall
// for this RamDisk.
// 
//
// Update Log:
//	$Log:$
// 
// 
// 09/24/99 Michael G. Panas: Create file
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"
#include "Scsi.h"

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_RAM_DISK
#include "Odyssey_Trace.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

// Tables referenced
#include "StorageRollCallTable.h"

#include "RdIsm.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"


// Table method context
typedef struct _RD_TBL_CONTEXT {
	Message					*pMsg;				// saved Init message
	U32						flags;				// flags
	StorageRollCallRecord	*pRCTableRow;
} RD_TBL_CONTEXT, *PRD_TBL_CONTEXT;

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// RdTableInitialize
// Start of the table initialization state machine, called from the Ddm Init
//	Add an entry in the StorageRollCallTable for this RamDisk, these entries
// are keyed by the VDN of the Ramdisk (stored in the vdnBSADdm field).
//
/*************************************************************************/
STATUS	RdIsm::RdTableInitialize(Message *pMsg)
{
	TRACE_ENTRY(RdTableInitialize);
	
	STATUS			status = ercOK;
	RD_TBL_CONTEXT *pTC = new RD_TBL_CONTEXT;
	
	pTC->pMsg = pMsg;
	
	pTC->flags = 0;
	
	// need someplace to put the Rollcall Table when we read it
	pTC->pRCTableRow = new(tUNCACHED) StorageRollCallRecord;

	// Read/Modify
	status = RdTblUpdReadRollCall(pTC, ercOK);
	
	return status;
	
} // RdTableInitialize

/*************************************************************************/
// RdTblUpdReadRollCall
// Check for a RollCall entry by reading it, add it if it does not exist
/*************************************************************************/
STATUS 	RdIsm::RdTblUpdReadRollCall(void *pClientContext, STATUS status)
{
	RD_TBL_CONTEXT 		*pTC = (RD_TBL_CONTEXT *)pClientContext;
	rowID				*pRI;

	TRACE_ENTRY(RdTblUpdReadRollCall);
	
	// Allocate an ReadRow object for the StorageRollCall table.
	m_pSRCReadRow = new(tPCI) TSReadRow;
	
	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		STORAGE_ROLL_CALL_TABLE,			// String64 rgbTableName,
		"vdnBSADdm",						// String64 prgbKeyFieldName,
		&myVd,								// void* pKeyFieldValue,
		sizeof(VDN),						// U32 cbKeyFieldValue,
		pTC->pRCTableRow,					// void* prgbRowDataRet,
		sizeof(StorageRollCallRecord),		// U32 cbRowDataRetMax,
		NULL,								// rowID *pcRowsReadRet,
		(pTSCallback_t)&RdTblUpdModifyRollCall,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // RdTblUpdReadRollCall

/*************************************************************************/
// RdTblUpdModifyRollCall
// Modify a table row entry with the table data in the DM_Disk_Desc[]
// that has just been built.
/*************************************************************************/
STATUS 	RdIsm::RdTblUpdModifyRollCall(void *pClientContext, STATUS status)
{
	RD_TBL_CONTEXT 			*pTC = (RD_TBL_CONTEXT *)pClientContext;
	StorageRollCallRecord	*pRCR = pTC->pRCTableRow;

	TRACE_ENTRY(RdTblUpdModifyRollCall);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rRdTblUpdModifyRollCall: status = ", status);
		if ((status == ercEOF) || (status == ercKeyNotFound))
		{
			status = RdTblUpdAddRollCall(pTC, ercOK);
			return status;
		}
	}
	
	// fill in fields used for RAM disks
	pRCR->Capacity = num_sectors;
	pRCR->vdnBSADdm = myVd;

	// Allocate an Modify Row object for the DiskStatusTable.
	m_ModifyRow = new(tUNCACHED) TSModifyRow;

	// Initialize the modify row operation.
	status = m_ModifyRow->Initialize(
		this,								// DdmServices pDdmServices,
		STORAGE_ROLL_CALL_TABLE,			// String64 rgbTableName,
		"vdnBSADdm",						// String64 prgbKeyFieldName,
		(void*)&myVd,						// void* pKeyFieldValue,
		sizeof(VDN),						// U32 cbKeyFieldValue,
		pTC->pRCTableRow,					// void* prgbRowData,
		sizeof(StorageRollCallRecord),		// U32 cbRowData,
		0,									// U32 cRowsToModify,
		NULL,								// U32 *pcRowsModifiedRet,
		NULL,								// rowID *pRowIDRet,
		0,									// U32 cbMaxRowID,
		(pTSCallback_t)&RdTableUpdateRCEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_ModifyRow->Send();
	
	return status;
	
} // RdTblUpdModifyRollCall

/*************************************************************************/
// RdTblUpdAddRollCall
// Row did not exist, so now we must add it to the StorageRollCall table.
/*************************************************************************/
STATUS RdIsm::RdTblUpdAddRollCall(void *pClientContext, STATUS status)
{
	RD_TBL_CONTEXT 			*pTC = (RD_TBL_CONTEXT *)pClientContext;
	StorageRollCallRecord	*pRCR = pTC->pRCTableRow;

	TRACE_ENTRY(RdTblUpdAddRollCall);
	
	memset(pRCR, 0, sizeof(StorageRollCallRecord));
	
	// fill in all the common RollCall entries
	pRCR->version = STORAGE_ROLL_CALL_TABLE_VERSION;
	pRCR->size = sizeof(StorageRollCallRecord);
	pRCR->vdnMonitor = myVd;
	pRCR->fUsed = 0;
	
	// fill in fields used for disks
	pRCR->Capacity = num_sectors;
	pRCR->vdnBSADdm = myVd;
	
	pRCR->storageclass = SRCTypeRamDisk;

	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the StorageRollCallTable.
	m_pInsertRow = new(tUNCACHED) TSInsertRow;
	
	status = m_pInsertRow->Initialize(
		this,							// Ddm* ClientDdm
		STORAGE_ROLL_CALL_TABLE,		// prgbTableName
		pRCR,							// prgbRowData
		sizeof(StorageRollCallRecord),	// cbRowData
		NULL,							// *pRowIDRet
		(pTSCallback_t)&RdTableUpdateRCEnd,	// pTSCallback_t pCallback,
		(void*)pTC						// pContext
	);
	
	if (status == ercOK)
		m_pInsertRow->Send();

	return status;
} // RdTblUpdAddRollCall

/*************************************************************************/
// RdTableUpdateRCEnd
// Complete the table row entry update.
/*************************************************************************/
STATUS	RdIsm::RdTableUpdateRCEnd(void *pClientContext, STATUS status)
{
	RD_TBL_CONTEXT 		*pTC = (RD_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(RdTableUpdateRCEnd);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rRdTableUpdateRCEnd: status = ", status);
	}

	// don't return the Initialize cookie until the last TS message
	Reply(pTC->pMsg); 
	
	// done with these
	if (pTC->pRCTableRow)
		delete pTC->pRCTableRow;
	delete pTC;
	
	return ercOK;
	
} // RdTableUpdateRCEnd

