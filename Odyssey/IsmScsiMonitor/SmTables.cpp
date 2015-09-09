/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SmTables.cpp
// 
// Description:
// This module is the ScsiMonitor Table Handler
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmScsiMonitor/SmTables.cpp $
// 
// 2     10/19/99 6:36p Cchan
// Fixed uninitialized PTS variable SM_TS_Disk_Desc
// 
// 1     10/11/99 5:35p Cchan
// HSCSI (QL1040B) version of the Drive Monitor
// 
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"

// Tables referenced
#include "DiskDescriptor.h"
#include "StorageRollCallTable.h"

#include "ScsiMonitorIsm.h"
#include "SmCommon.h"

#include "Message.h"
#include "HscsiMessageFormats.h"
#include "HscsiMessageStatus.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"

// Table method context
typedef struct _SM_TBL_CONTEXT {
	Message					*pMsg;				// saved Init message
	U32						index;				// drive number index
	SM_CONTEXT				*pDmc;				// ScsiMonitor internal context
	DiskDescriptor			*pDDTableRow;
	StorageRollCallRecord	*pRCTableRow;
} SM_TBL_CONTEXT, *PSM_TBL_CONTEXT;

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/
// Default Table Data for test (defined in BuildSys.cpp)
//extern DiskDescriptor			DiskDesc[];		// one for each slot

/*************************************************************************/
// SmTableInitialize
// Start of the table initialization state machine, called from the Ddm Init
// Creates these tables if they do not exist:
//	DiskDescriptor
//	StorageRollCallTable
// Reads these tables:
//	DiskDescriptor
/*************************************************************************/
STATUS	ScsiMonitorIsm::SmTableInitialize(Message *pMsg)
{
	TRACE_ENTRY(SmTableInitialize);
	
	STATUS			status;
	SM_TBL_CONTEXT *pTC = new SM_TBL_CONTEXT;
	
	pTC->pMsg = pMsg;
	
	// This is the code to create the DiskDescriptor Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.

	fieldDef *pciDiskDescriptorTable_FieldDefs = 
				(fieldDef*)new(tPCI) char[cbDiskDescriptorTable_FieldDefs];

	memcpy( (char*)pciDiskDescriptorTable_FieldDefs,
			(char*)DiskDescriptorTable_FieldDefs,
			cbDiskDescriptorTable_FieldDefs
		  ); 
		  
	m_pDefineTable = new(tUNCACHED) TSDefineTable;

	status = m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		DISK_DESC_TABLE,					// String64 prgbTableName,
		pciDiskDescriptorTable_FieldDefs,	// fieldDef* prgFieldDefsRet,
		cbDiskDescriptorTable_FieldDefs,	// U32 cbrgFieldDefs,
		config.num_drives,					// U32 cEntriesRsv,
		false,								// bool* pfPersistant,
		(pTSCallback_t)&SmTblReply1,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	if (status == ercOK)
		m_pDefineTable->Send();

	return status;
} // SmTableInitialize

/*************************************************************************/
// SmTblReply1
// Reply from creating the DiskDescriptor Table. If it already exist,
// read all the rows.  If it does not, create some dummy entries.
/*************************************************************************/
STATUS ScsiMonitorIsm::SmTblReply1(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(SmTblReply1);
	
	// if the table exist, do not load dummy values
	if (status != ercOK)
	{
		TRACE_STRING(TRACE_L8, "\n\rSM: DiskDesc table already defined");
	
		status = SmTblReply2(pTC, ercOK);
		return (status);
	}
	
	// Table did not exist, it does now, so load it with default data from
	// the BuildSys.cpp file

	TRACE_STRING(TRACE_L8, "\n\rSM: loading DiskDesc table");
	
	char* pcinewDiskDescRecord = (char*)new(tPCI) 
					char[sizeof(DiskDescriptor) * config.num_drives];

	// fill in the blanks to generate a default table
	DiskDescriptor *pDD = (DiskDescriptor *)pcinewDiskDescRecord;

	for (int i = 0; i < config.num_drives; i++, pDD++)
	{
		memset(pDD, 0, sizeof(DiskDescriptor));
		pDD->version = DISK_DESC_VERSION;
		pDD->size = sizeof(DiskDescriptor);
		pDD->SlotID = i;
		pDD->FCTargetID = config.xlate[i];
		pDD->CurrentStatus = DriveInvalid;
	}
	
	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the DiskDescTable.
	m_pInsertRow = new(tUNCACHED) TSInsertRow;
	
	status = m_pInsertRow->Initialize(
		this,							// Ddm* ClientDdm
		DISK_DESC_TABLE,				// prgbTableName
		pcinewDiskDescRecord,			// prgbRowData
		sizeof(DiskDescriptor)*config.num_drives,		// cbRowData
		&m_RowID1,						// *pRowIDRet
		(pTSCallback_t)&SmTblReply2,	// pTSCallback_t pCallback,
		(void*)pTC						// pContext
	);
	
	if (status == ercOK)
		m_pInsertRow->Send();

	return status;
} // SmTblReply1

/*************************************************************************/
// SmTblReply2
// The DiskDescriptor table now exists.  Read how many entries (rows) 
// there current are.  Use this count to to build the array for local use.
/*************************************************************************/
STATUS ScsiMonitorIsm::SmTblReply2(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(SmTblReply2);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rSmTblReply2: status = ", status);
	}
	
	m_nTableRows = 0;

	// Alloc, init and send off a read row object for the new
	// DiskStatusTable record.
	
	// Allocate an Get Table Defs object for the DiskDescTable.
	m_pTSGetTableDef = new(tUNCACHED) TSGetTableDef;

	// Initialize the enumerate table operation.
	status = m_pTSGetTableDef->Initialize( 
		this,
		DISK_DESC_TABLE,
		NULL,						// rgFieldDefsRet
		0,							// cbrgFieldDefsRetMax
		NULL,						// pcFieldDefsRet
		NULL,						// pcbRowRet
		&m_nTableRows,				// pcRowsRet Returned data buffer (row count).
		NULL,						// pcFieldsRet
		NULL,						// pPersistFlagsRet
		(pTSCallback_t)&SmTblReply3,		// pTSCallback_t pCallback,
		(void*)pTC					// pContext
	  );

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pTSGetTableDef->Send();
	
	return status;
} // SmTblReply2

/*************************************************************************/
// SmTblReply3
// Number of rows is now read into m_nTableRows.  Use this count to
// create the local DiscDesc table. And start the read of all the entries.
/*************************************************************************/
STATUS ScsiMonitorIsm::SmTblReply3(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(SmTblReply3);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rSmTblReply3: status = ", status);
	}
	
	// check for a prior table
	if (SM_TS_Disk_Desc)
	{
		delete SM_TS_Disk_Desc;
	}
	
	TRACE_HEX(TRACE_L8, "\n\rSM: num of disk desc rows = ", m_nTableRows);
	
	// Allocate the Local Disk Descriptor Table
	SM_TS_Disk_Desc = new(tUNCACHED) DiskDescriptor[m_nTableRows];
		
	// Allocate an Enumerate Table object for the DiskDescTable.
	m_EnumTable = new(tPCI) TSEnumTable;

	// Initialize the enumerate table operation.
	status = m_EnumTable->Initialize(
		this,
		DISK_DESC_TABLE,				// Name of table to read.
		0,								// Starting row number.
		SM_TS_Disk_Desc,				// Returned data buffer.
		sizeof(DiskDescriptor) * m_nTableRows,	// max size of returned data.
		&m_numBytesEnumed,				// pointer to # of returned bytes.
//		(pTSCallback_t)&SmTblReply4,		// pTSCallback_t pCallback,
		(pTSCallback_t)&SmTblReplyLast,	// pTSCallback_t pCallback,
		(void*)pTC						// pContext
						  );

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_EnumTable->Send();
	
	return status;
} // SmTblReply3

/*************************************************************************/
// SmTblReply4
// Create the StorageRollCall table, since we need it later. Make sure
// there is a minimum of 100 entries.  Rows are loaded on the fly to
// this table.
/*************************************************************************
STATUS ScsiMonitorIsm::SmTblReply4(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(SmTblReply4);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rSmTblReply4: status = ", status);
	}
	
	// This is the code to create the StorageRollCall Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.

	fieldDef *pciStorageRollCallTable_FieldDefs = 
				(fieldDef*)new(tPCI) char[cbStorageRollCallTable_FieldDefs];

	memcpy( (char*)pciStorageRollCallTable_FieldDefs,
			(char*)StorageRollCallTable_FieldDefs,
			cbStorageRollCallTable_FieldDefs
		  ); 
		  
	m_pDefineTable = new(tUNCACHED) TSDefineTable;

	m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		STORAGE_ROLL_CALL_TABLE,			// String64 prgbTableName,
		pciStorageRollCallTable_FieldDefs,	// fieldDef* prgFieldDefsRet,
		cbStorageRollCallTable_FieldDefs,	// U32 cbrgFieldDefs,
		100,								// U32 cEntriesRsv,  (default 100 entries)
		false,								// bool* pfPersistant,
		(pTSCallback_t)&SmTblReplyLast,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	status = m_pDefineTable->Send();

	return status;
} // SmTableInitialize
*/

/*************************************************************************/
// SmTblReplyLast
// Last CallBack, just return the Initialize message we saved way back
// when.
/*************************************************************************/
STATUS ScsiMonitorIsm::SmTblReplyLast(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(SmTblReplyLast);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rSmTblReplyLast: status = ", status);
	}
	
	// don't return this until the last TS message
	Reply(pTC->pMsg); 
	
	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // SmTblReplyLast


/*************************************************************************/
// SmTableUpdate
// Updates a table row entry that has changed
// Build the context and call the first state in the state machine
/*************************************************************************/
STATUS	ScsiMonitorIsm::SmTableUpdate(Message *pMsg, SM_CONTEXT *pDmc)
{
	STATUS			status;
	SM_TBL_CONTEXT *pTC = new SM_TBL_CONTEXT;
	
	TRACE_ENTRY(SmTableUpdate);
	
	pTC->pMsg = pMsg;
	pTC->pDmc = pDmc;
	pTC->index = pDmc->index;
	pTC->pDDTableRow = new(tUNCACHED) DiskDescriptor;
	pTC->pRCTableRow = NULL;
		
	status = SmTblUpdReadDesc(pTC, ercOK);
	
	return status;
}

/*************************************************************************/
// SmTblUpdReadDesc
// Read the DiskDescriptor row that needs to be updated
/*************************************************************************/
STATUS 	ScsiMonitorIsm::SmTblUpdReadDesc(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(SmTblUpdReadDesc);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rSmTblUpdReadDesc: status = ", status);
		if (status == ercEOF)
		{
			status = SmTblUpdAddDesc(pTC, ercOK);
			return status;
		}
	}
	
	// copy the rowID for the DiskDescriptor to use as a key
	m_RowID1 = SM_TS_Disk_Desc[pTC->index].ridThisRow;
	SM_Disk_Desc[pTC->index].ridThisRow = m_RowID1;
	
	// use the index as a key to find the row
	
	// Allocate a ReadRow object for the DiskStatusTable.
	m_pSRCReadRow = new(tUNCACHED) TSReadRow;

	// Initialize the read row operation.
	m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		DISK_DESC_TABLE,					// String64 rgbTableName,
		"SlotID",							// String64 prgbKeyFieldName,
		(void*)&pTC->index,					// void* pKeyFieldValue,
		sizeof(U32),						// U32 cbKeyFieldValue,
		pTC->pDDTableRow,					// void* prgbRowData,
		sizeof(DiskDescriptor),				// U32 cbRowData,
		NULL,								// rowID *pRowIDRet,
		(pTSCallback_t)&SmTblUpdModifyDesc,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // SmTblUpdReadDesc

/*************************************************************************/
// SmTblUpdModifyDesc
// Modify a table row entry with the table data in the SM_Disk_Desc[]
// that has just been built.
/*************************************************************************/
STATUS 	ScsiMonitorIsm::SmTblUpdModifyDesc(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;
	DiskDescriptor			*pDD;

	TRACE_ENTRY(SmTblUpdModifyDesc);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rSmTblUpdModifyDesc: status = ", status);
		if (status == ercEOF)
		{
			status = SmTblUpdAddDesc(pTC, ercOK);
			return status;
		}
	}
	
	// Modify fields that changed (maybe)
	pDD = pTC->pDDTableRow;
	pDD->CurrentStatus = SM_Disk_Desc[pTC->index].CurrentStatus;
	pDD->Capacity = SM_Disk_Desc[pTC->index].Capacity;
	pDD->InqData = SM_Disk_Desc[pTC->index].InqData;
	strcpy(pDD->SerialNumber, SM_Disk_Desc[pTC->index].SerialNumber);
	
	// copy the rowID
	m_RowID1 = SM_TS_Disk_Desc[pTC->index].ridThisRow;
	
	// use the index as a key to find the row
	
	// Allocate an Modify Row object for the DiskStatusTable.
	m_ModifyRow = new(tUNCACHED) TSModifyRow;

	// Initialize the modify row operation.
	m_ModifyRow->Initialize(
		this,								// DdmServices pDdmServices,
		DISK_DESC_TABLE,					// String64 rgbTableName, "rid"
		"SlotID",							// String64 prgbKeyFieldName,
		(void*)&pTC->index,					// void* pKeyFieldValue, ->pDMState->ridDD
		sizeof(U32),						// U32 cbKeyFieldValue, (rowID)
		pTC->pDDTableRow,					// void* prgbRowData,
		sizeof(DiskDescriptor),				// U32 cbRowData,
		0,									// U32 cRowsToModify,
		NULL,								// U32 *pcRowsModifiedRet
		NULL,								// rowID *pRowIDRet,
		0,									// U32 cbMaxRowID,
//		(pTSCallback_t)&SmTblUpdRollCall,	// pTSCallback_t pCallback,
		(pTSCallback_t)&SmTableUpdateEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_ModifyRow->Send();
	
	return status;
	
} // SmTblUpdModify

/*************************************************************************/
// SmTblUpdAddDesc
// Tried to modify a row that did not exist, so now we must add it to
// the DiskDescriptior table
/*************************************************************************/
STATUS ScsiMonitorIsm::SmTblUpdAddDesc(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(SmTblUpdAddDesc);
	
	char* pcinewDiskDescRecord = (char*)new(tPCI) 
					char[sizeof(DiskDescriptor)];
	
	memcpy( (char*)pcinewDiskDescRecord,
			(char*)&SM_Disk_Desc[pTC->pDmc->index],
			sizeof(DiskDescriptor)
		  ); 

	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the DiskDescTable.
	m_pInsertRow = new(tUNCACHED) TSInsertRow;
	
	status = m_pInsertRow->Initialize(
		this,							// Ddm* ClientDdm
		DISK_DESC_TABLE,				// prgbTableName
		pcinewDiskDescRecord,			// prgbRowData
		sizeof(DiskDescriptor),			// cbRowData
		&m_RowID1,						// *pRowIDRet
		(pTSCallback_t)&SmTblUpdModifyDesc,	// pTSCallback_t pCallback,
		(void*)pTC						// pContext
	);
	
	if (status == ercOK)
		m_pInsertRow->Send();

	return status;
} // SmTblUpdAddDesc

/*************************************************************************/
// SmTblUpdRollCall
// Check for a RollCall entry by reading it, add it if it does not exist
/*************************************************************************
STATUS 	ScsiMonitorIsm::SmTblUpdRollCall(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(SmTblUpdRollCall);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rSmTblUpdRollCall: status = ", status);
	}
	
	// if the CurrentStatus of this DiskDesc row is not DriveReady,
	// we are done here. If it is DriveReady, add a Storage Roll Call
	// entry for this disk.
	if (pTC->pDDTableRow->CurrentStatus != DriveReady)
	{
		status = SmTableUpdateEnd(pClientContext, ercOK);
		return status;
	}
	
	// need someplace to put the Rollcall Table
	pTC->pRCTableRow = new(tUNCACHED) StorageRollCallRecord;

	// copy the rowID for the DiskDescriptor to use as a key
	m_RowID2 = SM_Disk_Desc[pTC->pDmc->index].ridThisRow;
	
	// Allocate an ReadRow object for the DiskStatusTable.
	m_pSRCReadRow = new(tPCI) TSReadRow;

	// Initialize the read row operation.
	m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		STORAGE_ROLL_CALL_TABLE,			// String64 rgbTableName,
		"ridDescriptorRecord",				// String64 prgbKeyFieldName,
		&m_RowID2,							// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pTC->pRCTableRow,					// void* prgbRowData,
		sizeof(StorageRollCallRecord),		// U32 cbRowData,
		NULL,								// rowID *pRowIDRet,
		(pTSCallback_t)&SmTableUpdateEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the enumerate table operation.
	status = m_pSRCReadRow->Send();
	
	return status;
	
} // SmTblUpdRollCall
*/

/*************************************************************************/
// SmTblUpdAddRollCall
// Tried to read a row that did not exist, so now we must add it to
// the StorageRollCall table
/*************************************************************************
STATUS ScsiMonitorIsm::SmTblUpdAddRollCall(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;
	StorageRollCallRecord	*pRCR;

	TRACE_ENTRY(SmTblUpdAddRollCall);
	
	pRCR = pTC->pRCTableRow;
	
	// fill in all the RollCall entries
	pRCR->version = STORAGE_ROLL_CALL_TABLE_VERSION;
	pRCR->size = sizeof(StorageRollCallRecord);
	pRCR->ScanState = SRCStateDiscovered;
	pRCR->Capacity = SM_Disk_Desc[pTC->pDmc->index].Capacity.LowPart;
	pRCR->FreeCapacity = SM_Disk_Desc[pTC->pDmc->index].Capacity.LowPart;
	pRCR->UsedCapacity = 0;
	pRCR->NumPartitions = 0;
	pRCR->storageclass = SRCTypeFCDisk;
	pRCR->vdnBSADdm = -1;
	pRCR->ridDescriptorRecord = SM_Disk_Desc[pTC->pDmc->index].ridThisRow;
	pRCR->vdnMonitor = MyVd;
	

	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the StorageRollCallTable.
	m_pInsertRow = new(tUNCACHED) TSInsertRow;
	
	m_pInsertRow->Initialize(
		this,							// Ddm* ClientDdm
		STORAGE_ROLL_CALL_TABLE,		// prgbTableName
		pRCR,							// prgbRowData
		sizeof(StorageRollCallRecord),	// cbRowData
		&m_RowID2,						// *pRowIDRet
		(pTSCallback_t)&SmTableUpdateEnd,	// pTSCallback_t pCallback,
		(void*)pTC						// pContext
	);
	
	status = m_pInsertRow->Send();

	return status;
} // SmTblUpdAddRollCall
*/

/*************************************************************************/
// SmTableUpdateEnd
// Complete the table row entry update. Call the Finish routine
/*************************************************************************/
STATUS	ScsiMonitorIsm::SmTableUpdateEnd(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(SmTableUpdateEnd);
	
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;

	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rSmTableUpdateEnd: status = ", status);

#if 0 // if #1 when using RollCall 	
		if (status == ercEOF)
		{
			status = SmTblUpdAddRollCall(pTC, ercOK);
			return status;
		}
#endif
	}
	
	// Finish this scan
	SM_Do_Finish(pTC->pDmc); 
	
	// No Listen Yet,
	// for now just copy the entry to the global DiskDescriptor table
	SM_TS_Disk_Desc[pTC->index] = SM_Disk_Desc[pTC->index];
	
	// done with these
	if (pTC->pDDTableRow)
		delete pTC->pDDTableRow;
	if (pTC->pRCTableRow)
		delete pTC->pRCTableRow;
	delete pTC;
	
	return ercOK;
	
} // SmTableUpdateEnd


/*************************************************************************/
// SmListenUpdate
// Called when a listen callback happens. This method will update locally
// a table row entry that has changed.
/*************************************************************************/
STATUS	ScsiMonitorIsm::SmListenUpdate(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(SmListenUpdate);
	
	// TODO:
	// find which row changed
	
	// allocate a place for the row
	pTC->pDDTableRow = new(tUNCACHED) DiskDescriptor;
	
	// copy the rowID for the DiskDescriptor to use as a key
	m_RowID1 = SM_Disk_Desc[pTC->pDmc->index].ridThisRow;  // probably wrong XXX
	
	// Allocate an ReadRow object for the DiskStatusTable.
	m_pSRCReadRow = new(tPCI) TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		DISK_DESC_TABLE,					// String64 rgbTableName,
		"ridThisRow",						// String64 prgbKeyFieldName,
		&m_RowID1,							// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pTC->pDDTableRow,					// void* prgbRowData,
		sizeof(DiskDescriptor),				// U32 cbRowData,
		NULL,								// rowID *pRowIDRet,
		(pTSCallback_t)&SmListenUpdateEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the ReadRow table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // SmListenUpdate


/*************************************************************************/
// SmListenUpdateEnd
// Complete a Listen callback
/*************************************************************************/
STATUS	ScsiMonitorIsm::SmListenUpdateEnd(void *pClientContext, STATUS status)
{
	SM_TBL_CONTEXT *pTC = (SM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(SmListenUpdateEnd);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rSmListenUpdateEnd: status = ", status);
	}
	
	delete pTC->pDDTableRow;
	
	return ercOK;
	
} // SmListenUpdateEnd

