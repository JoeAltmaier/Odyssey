/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmTableInit.cpp
// 
// Description:
// This module handles Table Initialization and initial startup.
// 

// Update Log:
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DmTableInit.cpp $
// 
// 9     1/11/00 10:33p Mpanas
// fix StorageRollCall Ghosting
// (missed this change in merge)
// 
// 8     1/11/00 7:27p Mpanas
// New PathTable changes
// 
// 7     12/21/99 12:51p Mpanas
// Fix downstream ghosting problem
// 
// 6     11/03/99 2:52p Jlane
// Make all tables persistant.
// 
// 5     10/21/99 7:09p Mpanas
// Fix typo in cut and paste code
// 
// 4     10/11/99 7:20p Mpanas
// Second part of BSA VD Create, check for prior
// BSA device entries (like in BuildSys) 
// Note: this change needs the fix in VirtualDeviceTable.h
// Initialize to zero pad the class string.
// 
// 3     10/07/99 8:09p Mpanas
// First cut of BSA VD Create
// 
// 2     9/30/99 2:44p Mpanas
// Fix DriveMonitor / LoopMonitor race to update LoopDescriptor
// make sure we use row ID as key, copy listen data to local data
// 
// 1     9/14/99 8:40p Mpanas
// Complete re-write of DriveMonitor
// - Scan in sequence
// - Start Motors in sequence
// - LUN Scan support
// - Better table update
// - Re-organize sources
// 
// 
// 08/23/99 Michael G. Panas: Create file
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"
#include "Scsi.h"

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
	Message					*pMsg;				// saved Init message
	U32						index;				// drive number index
	DM_CONTEXT				*pDmc;				// Drive Monitor internal context
	DM_DEVICE_STATE			*pDMState;
	PathDescriptor			*pPDTableRow;
	StorageRollCallRecord	*pRCTableRow;
	String32				 classname;			// space for static class name
} DM_TBL_CONTEXT, *PDM_TBL_CONTEXT;

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// DmTableInitialize
// Start of the table initialization state machine, called from the Ddm Init
// Creates these tables if they do not exist:
//	PathDescriptor
//	DiskDescriptor
//	DeviceDescriptor
//	StorageRollCallTable
//  BsaConfig
// Reads these tables:
//	PathDescriptor
//	DiskDescriptor
//	DeviceDescriptor
//  BsaConfig
//	VDT
//
/*************************************************************************/
STATUS	DriveMonitorIsm::DmTableInitialize(Message *pMsg)
{
	TRACE_ENTRY(DmTableInitialize);
	
	STATUS			status = ercOK;
	DM_TBL_CONTEXT *pTC = new DM_TBL_CONTEXT;
	
	pTC->pMsg = pMsg;
	
	// pre-set some defaults
	m_nVdtTableRows = 0;
	m_nBsaTableRows = 0;
	m_nPaths = 0;
	
	// start the statemachine
	status = DmTblDoRollCall(pTC, ercOK);
	
	return status;
} // DmTableInitialize


/*************************************************************************/
// DmTblDoRollCall
// Create the StorageRollCall table, since we need it later. Make sure
// there is a minimum of 100 entries.  Rows are loaded on the fly to
// this table.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblDoRollCall(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblDoRollCall);
	
	// This is the code to create the StorageRollCall Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.

	m_pDefineTable = new(tUNCACHED) TSDefineTable;

	status = m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		STORAGE_ROLL_CALL_TABLE,			// String64 prgbTableName,
		StorageRollCallRecord::FieldDefs(),			// fieldDef* prgFieldDefsRet,
		StorageRollCallRecord::FieldDefsSize(),		// U32 cbrgFieldDefs,
		100,								// U32 cEntriesRsv,  (default 100 entries)
		Persistant_PT,						// U32 PersistFlags,
		(pTSCallback_t)&DmTblDoVDT,			// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	if (status == ercOK)
		m_pDefineTable->Send();

	return status;
	
} // DmTblDoRollCall

/*************************************************************************/
// DmTblDoVDT
// Reply from creating the RollCall Table.
// Read how many entries (rows) there currently are in the VDT table.
// Use this count to build the array for local use.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblDoVDT(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblDoVDT);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblDoVDT: status = ", status);
	}
	
	m_nVdtTableRows = 0;

	// Allocate an Get Table Defs object for the VirtualDevice table.
	m_pTSGetTableDef = new(tUNCACHED) TSGetTableDef;

	// Initialize the gettabledef table operation.
	status = m_pTSGetTableDef->Initialize( 
		this,
		PTS_VIRTUAL_DEVICE_TABLE,
		NULL,						// rgFieldDefsRet
		0,							// cbrgFieldDefsRetMax
		NULL,						// pcFieldDefsRet
		NULL,						// pcbRowRet
		&m_nVdtTableRows,			// pcRowsRet Returned data buffer (row count).
		NULL,						// pcFieldsRet
		NULL,						// pPersistFlagsRet
		(pTSCallback_t)&DmTblVdtReply1,		// pTSCallback_t pCallback,
		(void*)pTC					// pContext
	  );

	// Initiate the gettabledef table operation.
	if (status == ercOK)
		m_pTSGetTableDef->Send();
	
	return status;
	
} // DmTblDoVDT

/*************************************************************************/
// DmTblVdtReply1
// Number of rows is now read into m_nVdtTableRows.  Use this count to
// create the local VirtualDevice table. Start the read of all the entries
// that match class "HDM_BSA".
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblVdtReply1(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblVdtReply1);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblVdtReply1: status = ", status);
	}
	
	// check if any entries were read
	if (m_nVdtTableRows == 0)
	{
		// no- skip the read
		status = DmTblDoBsaConfig(pTC, ercOK);
		return (status);
	}
	
	TRACE_HEX(TRACE_L8, "\n\rDmTblVdtReply1: num of VDT rows max = ", m_nVdtTableRows);
	
	// use a static zero filled array to access the class name
	strncpy(pTC->classname, "HDM_BSA", sCLASSNAME);
	
	// Allocate space for the Local VirtualDevice Table
	pVdt = new(tUNCACHED) VirtualDeviceRecord[m_nVdtTableRows+2];
		
	// Allocate a ReadRow object for the VirtualDevice Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		PTS_VIRTUAL_DEVICE_TABLE,			// String64 rgbTableName,
		VDT_CLASSNAME_FIELD,				// String64 prgbKeyFieldName,
		&pTC->classname,					// void* pKeyFieldValue,
		sizeof(String32),					// U32 cbKeyFieldValue,
		pVdt,								// void* prgbRowData,
		sizeof(VirtualDeviceRecord) * (m_nVdtTableRows +2), // U32 cbRowDataRetMax, max size
		&m_nVdtTableRows,					// U32 *pcRowsReadRet,
		(pTSCallback_t)&DmTblDoBsaConfig,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the ReadRow table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // DmTblVdtReply1



/*************************************************************************/
// DmTblDoBsaConfig
// Create the BsaConfig table, since we need it later. Make sure
// there is a minimum of 100 entries.  Rows are loaded on the fly to
// this table.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblDoBsaConfig(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblDoBsaConfig);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblDoBsaConfig: status = ", status);
	}
	
	// This is the code to create the BsaConfig Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.
 
	m_pDefineTable = new(tUNCACHED) TSDefineTable;

	status = m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		BSA_CONFIG_TABLE_NAME,				// String64 prgbTableName,
		BSA_CONFIG::FieldDefs(),			// fieldDef* prgFieldDefsRet,
		BSA_CONFIG::FieldDefsSize(),		// U32 cbrgFieldDefs,
		100,								// U32 cEntriesRsv,  (default 100 entries)
		Persistant_PT,						// U32 PersistFlags,
		(pTSCallback_t)&DmTblBsaReply1,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	if (status == ercOK)
		m_pDefineTable->Send();

	return status;
	
} // DmTblDoBsaConfig

/*************************************************************************/
// DmTblBsaReply1
// Reply from creating the BsaConfig Table. If it already exist,
// read all the rows that match our Initiator VDN.  If it does not, skip
// the read and move on to next. Read how many entries (rows) there
// currently are.  Use this count to build the array for local use.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblBsaReply1(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblBsaReply1);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblBsaReply1: status = ", status);
	}
	
	// if the table exist, read all the entries
	if (status == ercOK)
	{
		// Table did not exist, skip the read
		status = DmTblDoDescriptors(pTC, ercOK);
		return (status);
	}
	
	TRACE_STRING(TRACE_L8, "\n\rDmTblBsaReply1: BsaConfig table already defined, reading");
	
	m_nBsaTableRows = 0;

	// Allocate an Get Table Defs object for the BsaConfig table.
	m_pTSGetTableDef = new(tUNCACHED) TSGetTableDef;

	// Initialize the Get Table Defs table operation.
	status = m_pTSGetTableDef->Initialize( 
		this,
		BSA_CONFIG_TABLE_NAME,
		NULL,						// rgFieldDefsRet
		0,							// cbrgFieldDefsRetMax
		NULL,						// pcFieldDefsRet
		NULL,						// pcbRowRet
		&m_nBsaTableRows,			// pcRowsRet Returned data buffer (row count).
		NULL,						// pcFieldsRet
		NULL,						// pPersistFlagsRet
		(pTSCallback_t)&DmTblBsaReply2,		// pTSCallback_t pCallback,
		(void*)pTC					// pContext
	  );

	// Initiate the Get Table Defs table operation.
	if (status == ercOK)
		m_pTSGetTableDef->Send();
	
	return status;
	
} // DmTblBsaReply1

/*************************************************************************/
// DmTblBsaReply2
// Number of rows is now read into m_nBsaTableRows.  Use this count to
// create the local BsaConfig table. And start the read of all the entries
// that match our Initiator VDN.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblBsaReply2(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblBsaReply2);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblBsaReply2: status = ", status);
	}
	
	// check if any entries were read
	if (m_nBsaTableRows == 0)
	{
		// no- skip the read
		status = DmTblDoDescriptors(pTC, ercOK);
		return (status);
	}
		
	TRACE_HEX(TRACE_L8, "\n\rDmTblBsaReply2: num of BsaConfig rows max = ", m_nBsaTableRows);
	
	// Allocate space for the Local Bsa Config Table
	pBSAConfig = new(tUNCACHED) BSA_CONFIG[m_nBsaTableRows+2];
		
	// Allocate a ReadRow object for the BSA_CONFIG Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		BSA_CONFIG_TABLE_NAME,				// String64 rgbTableName,
		"initVD",							// String64 prgbKeyFieldName,
		&config.vd,							// void* pKeyFieldValue,
		sizeof(U32),						// U32 cbKeyFieldValue,
		pBSAConfig,							// void* prgbRowData,
		sizeof(BSA_CONFIG) * (m_nBsaTableRows +2), // U32 cbRowDataRetMax, max size
		&m_nBsaTableRows,					// U32 *pcRowsReadRet,
		(pTSCallback_t)&DmTblBsaReply3,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // DmTblBsaReply2

/*************************************************************************/
// DmTblBsaReply3
// check for a set of an existing BsaConfig entries.  If we have some
// that match our Initiator VDN, create container entries for each one. If no
// entries, skip to next.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblBsaReply3(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState;
	U32					 rows, loop;
	BSA_CONFIG			*pBC;
	VirtualDeviceRecord	*pVdr;
	union {
		IDLUN		 idlun;
		long		 l;
	} sig;

	TRACE_ENTRY(DmTblBsaReply3);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblBsaReply3: status = ", status);
	}
	
	// create a container list of all the existing devices that match
	// our LoopInstance
	
	// any rows read?
	if ((m_nBsaTableRows) && (m_nVdtTableRows) && (status == ercOK))
	{
		pBC = (BSA_CONFIG *) pBSAConfig;
		
		// check the BsaConfig list for any IDs that need to be
		// added to our container list
		for (rows = m_nBsaTableRows ;rows--; pBC++)
		{
			TRACEF(TRACE_L3, ("\n BSA: id %d, lun %d", pBC->ID, pBC->LUN));
	
			pVdr = (VirtualDeviceRecord *) pVdt;
			
			// find the VirtualDeviceRecord for this BsaConfig
			for (loop = m_nVdtTableRows; loop--; pVdr++)
			{
				// match the rid of the BsaConfig to the VDT ridDdmCfgRec entry
				// these rowIDs are different sizes so, we need a memcmp() to
				// compare only the first 8 bytes
				if (memcmp(&pBC->rid, &pVdr->ridDdmCfgRec, sizeof(rowID)) == 0)
					break;
			}
			
			// found VDT?
			if (loop == -1)
			{
				TRACEF(TRACE_L3, (" No VDT! ID:%d, LUN:%d", pBC->ID, pBC->LUN));
				continue;
			}
			
			TRACEF(TRACE_L3, (" VDN: %d", pVdr->rid.LoPart));
	
		    // build the device key
		    sig.idlun.id = (U8)pBC->ID;
		    sig.idlun.LUN = (U16)pBC->LUN;
		  	sig.idlun.HostId = 0;
			
			// see if we already have it
			if ((pDD->Get((CONTAINER_ELEMENT &)pDMState, (CONTAINER_KEY)sig.l)) == FALSE)
			{
				// did not exist
				pDMState = new DM_DEVICE_STATE;
				memset(pDMState, 0, sizeof(DM_DEVICE_STATE));
				
				// set inital state and type
				pDMState->state = DEVICE_STATE_READ_RC|DEVICE_STATE_VDN_FOUND;  // need to read all entries
				pDMState->type = -1;
				
				pDMState->Vdn = pVdr->rid.LoPart;

				// add to the list
				pDD->Add((CONTAINER_ELEMENT)pDMState, (CONTAINER_KEY)sig.l);
			}
			else
			{
				TRACEF(TRACE_L3, (" Container existed!"));
			}
		}
	}
	
	// finished - delete memory and do next
	delete DM_TS_Disk_Desc;
	
	status = DmTblDoPathDescriptor(pTC, ercOK);

	return status;

} // DmTblDDReply3



/*************************************************************************/
// DmTblDoPathDescriptor
// Create the PathDescriptor Table if it does not exist, read existing
// entries if it does.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblDoPathDescriptor(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblDoPathDescriptor);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblDoPathDescriptor: status = ", status);
	}
	
	// This code will create the PathDescriptor Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.

	m_pDefineTable = new(tUNCACHED) TSDefineTable;

	status = m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		PATH_DESC_TABLE,					// String64 prgbTableName,
		PathDescriptor::FieldDefs(),		// fieldDef* prgFieldDefsRet,
		PathDescriptor::FieldDefsSize(),	// U32 cbrgFieldDefs,
		config.num_drives * 2,				// U32 cEntriesRsv,
		Persistant_PT,						// U32 PersistFlags,
		(pTSCallback_t)&DmTblPDReply1,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	if (status == ercOK)
		m_pDefineTable->Send();

	return status;
} // DmTblDoPathDescriptor

/*************************************************************************/
// DmTblPDReply1
// Reply from creating the PathDescriptor Table. If it already exist,
// read all the rows.  If it does not, skip the read and move on to next.
// Read how many entries (rows) there currently are.  Use this count to
// build the array for local use.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblPDReply1(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblPDReply1);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblPDReply1: status = ", status);
	}
	
	// if the table exist, read all the entries
	if (status == ercOK)
	{
		// Table did not exist, skip the read, no entries
		status = DmTblDoDescriptors(pTC, ercOK);
		return (status);
	}
	
	TRACE_STRING(TRACE_L8, "\n\rDmTblPDReply1: PathDesc table already defined");
	
	m_nTableRows = 0;

	// Allocate an Get Table Defs object for the PathDescTable.
	m_pTSGetTableDef = new(tUNCACHED) TSGetTableDef;

	// Initialize the enumerate table operation.
	status = m_pTSGetTableDef->Initialize( 
		this,
		PATH_DESC_TABLE,
		NULL,						// rgFieldDefsRet
		0,							// cbrgFieldDefsRetMax
		NULL,						// pcFieldDefsRet
		NULL,						// pcbRowRet
		&m_nTableRows,				// pcRowsRet Returned data buffer (row count).
		NULL,						// pcFieldsRet
		NULL,						// pPersistFlagsRet
		(pTSCallback_t)&DmTblPDReply2,	// pTSCallback_t pCallback,
		(void*)pTC					// pContext
	  );

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pTSGetTableDef->Send();
	
	return status;
	
} // DmTblPDReply1

/*************************************************************************/
// DmTblPDReply2
// Number of rows is now read into m_nTableRows.  Use this count to
// create the local PathDescRecord array. And start the read of all the entries
// that match our LoopInstance.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblPDReply2(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblPDReply2);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblPDReply2: status = ", status);
	}
	
	// check if any entries were read
	if (m_nTableRows == 0)
	{
		// no- skip the read
		status = DmTblDoDescriptors(pTC, ercOK);
		return (status);
	}
	
	// check for a prior table
	if (DM_TS_Path_Desc)
	{
		delete DM_TS_Path_Desc;
	}
	
	TRACE_HEX(TRACE_L8, "\n\rDmTblDDReply2: num of path desc rows max = ", m_nTableRows);
	
	// Allocate space for the Local Path Descriptor Table
	DM_TS_Path_Desc = new(tUNCACHED) PathDescriptor[m_nTableRows+2];
		
	// Allocate a ReadRow object for the DiskDescriptor Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		PATH_DESC_TABLE,					// String64 rgbTableName,
		"FCInstance",						// String64 prgbKeyFieldName,
		&config.FC_instance,				// void* pKeyFieldValue,
		sizeof(U32),						// U32 cbKeyFieldValue,
		DM_TS_Path_Desc,					// void* prgbRowData,
		sizeof(PathDescriptor) * (m_nTableRows +2), // U32 cbRowDataRetMax, max size
		&m_nTableRowsRead,					// U32 *pcRowsReadRet,
		(pTSCallback_t)&DmTblPDReply3,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the read row table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // DmTblPDReply2

/*************************************************************************/
// DmTblPDReply3
// check for a set of an existing PathDescriptor entries.  If we have some
// that match our FC Loop, create container entries for each one.  If no
// entries, skip to next.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblPDReply3(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState;
	U32					 rows;
	PathDescriptor		*pPDr;
	union {
		IDLUN		 idlun;
		long		 l;
	} sig;

	TRACE_ENTRY(DmTblPDReply3);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblPDReply3: status = ", status);
	}
	
	// create a container list of all the existing devices that match
	// our LoopInstance
	
	// any rows read?
	if ((m_nTableRowsRead) && (status == ercOK))
	{
		pPDr = (PathDescriptor *) DM_TS_Path_Desc;
		
		m_nPaths = m_nTableRowsRead;
	
		// check the Descriptor list for any IDs that need to be
		// added to our container list
		for (rows = m_nTableRowsRead ;rows--; pPDr++)
		{
		    // build the device key
		    sig.idlun.id = (U8)pPDr->FCTargetID;
		    sig.idlun.LUN = (U16)pPDr->FCTargetLUN;
		  	sig.idlun.HostId = 0;
			
			// see if we already have it
			if ((pDD->Get((CONTAINER_ELEMENT &)pDMState, (CONTAINER_KEY)sig.l)) == FALSE)
			{
				// did not exist
				pDMState = new DM_DEVICE_STATE;
				memset(pDMState, 0, sizeof(DM_DEVICE_STATE));
				pDMState->pPD = new PathDescriptor;
				
				// set inital state and type
				pDMState->state = DEVICE_STATE_READ_RC;
				pDMState->type = pPDr->InqType;
				
				// Add new entry
				memcpy(pDMState->pPD, pPDr, sizeof(PathDescriptor));
				memcpy(&pDMState->ridPD, &pPDr->rid, sizeof(rowID));
												
				// add to the list
				pDD->Add((CONTAINER_ELEMENT)pDMState, (CONTAINER_KEY)sig.l);
			}
			else
			{
				// did exist, see if we still need to copy path descriptor
				if (pDMState->pPD == NULL)
				{
					pDMState->pPD = new PathDescriptor;

					// Add new entry
					memcpy(pDMState->pPD, pPDr, sizeof(PathDescriptor));
					memcpy(&pDMState->ridPD, &pPDr->rid, sizeof(rowID));
												
					pDMState->type = pPDr->InqType;
					
					pDMState->state &= ~DEVICE_STATE_ADD_PATH;
				}
			}
		}
	}

	// finished - delete memory and do next
	delete DM_TS_Path_Desc;
	DM_TS_Path_Desc = NULL;
	
	status = DmTblDoDescriptors(pTC, ercOK);

	return status;

} // DmTblPDReply3




/*************************************************************************/
// DmTblDoDescriptors
// Create the DeviceDescriptor Table if it does not exist, read existing
// entries if it does.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblDoDescriptors(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblDoDescriptors);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblDoDescriptors: status = ", status);
	}
	
	// This code will create the DeviceDescriptor Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.

	m_pDefineTable = new(tUNCACHED) TSDefineTable;

	status = m_pDefineTable->Initialize
	(
		this,									// Ddm* pDdm,
		DEVICE_DESC_TABLE,						// String64 prgbTableName,
		DeviceDescriptor::FieldDefs(),			// fieldDef* prgFieldDefsRet,
		DeviceDescriptor::FieldDefsSize(),		// U32 cbrgFieldDefs,
		config.num_drives,						// U32 cEntriesRsv,
		Persistant_PT,							// U32 PersistFlags,
		(pTSCallback_t)&DmTblDoDiskDescriptor,	// pTSCallback_t pCallback,
		(void*)pTC								// void* pContext
	);
	
	if (status == ercOK)
		m_pDefineTable->Send();

	return status;
	
} // DmTblDoDescriptors

/*************************************************************************/
// DmTblDoDiskDescriptor
// Create the DiskDescriptor Table if it does not exist, read existing
// entries if it does.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblDoDiskDescriptor(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblDoDiskDescriptor);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblDoDiskDescriptor: status = ", status);
	}
	
	// This code will create the DiskDescriptor Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.

	m_pDefineTable = new(tUNCACHED) TSDefineTable;

	status = m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		DISK_DESC_TABLE,					// String64 prgbTableName,
		DiskDescriptor::FieldDefs(),			// fieldDef* prgFieldDefsRet,
		DiskDescriptor::FieldDefsSize(),		// U32 cbrgFieldDefs,
		config.num_drives,					// U32 cEntriesRsv,
		Persistant_PT,						// U32 PersistFlags,
		(pTSCallback_t)&DmTblDDReply1,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	if (status == ercOK)
		m_pDefineTable->Send();

	return status;
	
} // DmTblDoDiskDescriptor

/*************************************************************************/
// DmTblDDReply1
// Reply from creating the DiskDescriptor Table. If it already exist,
// read all the rows that match our paths.  If it does not, skip the read
// and move on to next.
// At this point, we only have the PathDescriptor, so try to Read the row
// that matches the Descriptor RID in the path.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblDDReply1(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblDDReply1);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblDDReply1: status = ", status);
	}
	
	// if the table exist and there are some paths, read all the entries
	if ((status == ercOK) || (m_nPaths == 0))
	{
		// Table did not exist or no path entries, skip the read
		status = DmTblDoLoopDesc(pTC, ercOK);
		return (status);
	}
	
	TRACEF(TRACE_L3, ("\n\rDmTblDDReply1: Found %d Paths", m_nPaths));
	
	m_nTableRows = 0;
	m_next = 0;

	// get the first path
	pDD->GetAt((CONTAINER_ELEMENT &)pTC->pDMState, m_next);

	// check the type	
	if ((pTC->pDMState->pPD->InqType == SCSI_DEVICE_TYPE_DIRECT) ||
				(pTC->pDMState->pPD->InqType == SCSI_DEVICE_TYPE_ARRAY_CONT))
	{
		// find the disk descriptor
		status = DmTblDDReply2(pTC, ercOK);
	}
	else
	{
		// find the device descriptor
		status = DmTblDDReply2a(pTC, ercOK);
	}

	return status;
	
} // DmTblDDReply1

/*************************************************************************/
// DmTblDDReply2
// Read a DiskDescriptor based on the RID in the PathDescriptor.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblDDReply2(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;

	TRACE_ENTRY(DmTblDDReply2);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblDDReply2: status = ", status);
	}
	
	// Allocate space for the Local Disk Descriptor Table
	pDMState->pDD = new(tUNCACHED) DiskDescriptor;
		
	// Allocate a ReadRow object for the DiskDescriptor Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		DISK_DESC_TABLE,					// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		&pDMState->pPD->ridDescriptor,		// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pDMState->pDD,						// void* prgbRowData,
		sizeof(DiskDescriptor), 			// U32 cbRowDataRetMax, max size
		NULL,								// U32 *pcRowsReadRet,
		(pTSCallback_t)&DmTblDDReply3,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // DmTblDDReply2

/*************************************************************************/
// DmTblDDReply2a
// Read a DeviceDescriptor based on the RID in the PathDescriptor.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblDDReply2a(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT 		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;

	TRACE_ENTRY(DmTblDDReply2a);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblDDReply2a: status = ", status);
	}
	
	// Allocate space for the Local Device Descriptor Table
	pDMState->pDevice = new(tUNCACHED) DeviceDescriptor;
		
	// Allocate a ReadRow object for the DeviceDescriptor Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		DEVICE_DESC_TABLE,					// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		&pDMState->pPD->ridDescriptor,		// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pDMState->pDevice,					// void* prgbRowData,
		sizeof(DeviceDescriptor), 			// U32 cbRowDataRetMax, max size
		NULL,								// U32 *pcRowsReadRet,
		(pTSCallback_t)&DmTblDDReply3,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // DmTblDDReply2a

/*************************************************************************/
// DmTblDDReply3
// check for a set of an existing DiskDescriptor entries.  If we have some
// that match our FC Loop, create container entries for each one.  If no
// entries, skip to next.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblDDReply3(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT		*pTC = (DM_TBL_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pTC->pDMState;

	TRACE_ENTRY(DmTblDDReply3);
	
	if (status != ercOK)
	{
		// bad read, need to warn the operator and throw the DM_DEVICE_STATE
		// we built so far away.  Must have been an orphan?
		Tracef("\nDmTblDDReply3: (Disk/Device)Descriptor read failed, status=%x", status);
		Tracef("\n LOOP: %d, ID: %d, LUN: %d", pDMState->pPD->FCInstance,
						pDMState->pPD->FCTargetID,
						pDMState->pPD->FCTargetLUN);
		
		// delete container
		pDD->RemoveAt(m_next);
		
		// Is this needed?
		m_next--;
	}
	else
	{
		// good read
		if ((pDMState->pPD->InqType == SCSI_DEVICE_TYPE_DIRECT) ||
					(pDMState->pPD->InqType == SCSI_DEVICE_TYPE_ARRAY_CONT))
		{
			// copy the DiskDescriptor rowID data just read
			memcpy(&pDMState->ridDD, &pDMState->pDD->rid, sizeof(rowID));									
		}
		else
		{
			// copy the DeviceDescriptor rowID data just read
			memcpy(&pDMState->rid, &pDMState->pDevice->rid, sizeof(rowID));
		}
		// descriptor read
		pDMState->state &= ~DEVICE_STATE_ADD_DD;
	
	}
	
	// check for all paths done
	if (++m_next != m_nPaths)
	{
		// do next path
		// get the next path
		pDD->GetAt((CONTAINER_ELEMENT &)pTC->pDMState, m_next);
		
		// check the type	
		if ((pTC->pDMState->pPD->InqType == SCSI_DEVICE_TYPE_DIRECT) ||
					(pTC->pDMState->pPD->InqType == SCSI_DEVICE_TYPE_ARRAY_CONT))
		{
			// find the disk descriptor
			status = DmTblDDReply2(pTC, ercOK);
		}
		else
		{
			// find the device descriptor
			status = DmTblDDReply2a(pTC, ercOK);
		}
	
		return status;
	}
	
	// finished - do next
	status = DmTblDoLoopDesc(pTC, ercOK);

	return status;

} // DmTblDDReply3



/*************************************************************************/
// DmTblDoLoopDesc
// Read our LoopDescriptor entry, if it available, insert our VDN and
// do a modify.  If it is not, send a listen on this entry and handle
// it later.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblDoLoopDesc(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblDoLoopDesc);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblDoLoopDesc: status = ", status);
	}
	
	// Allocate a ReadRow object for the LoopDescriptor table.
	m_pSRCReadRow = new(tUNCACHED) TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		LOOP_DESCRIPTOR_TABLE,				// String64 rgbTableName,
		fdLD_LOOP_NUM,						// String64 prgbKeyFieldName,
		(void*)&config.FC_instance,			// void* pKeyFieldValue,
		sizeof(U32),						// U32 cbKeyFieldValue,
		DM_Loop_Desc,						// void* prgbRowData,
		sizeof(LoopDescriptorEntry),		// U32 cbRowData,
		NULL,								// rowID *pRowIDRet,
		(pTSCallback_t)&DmTblLpDescReply1, 	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the read row table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // DmTblDoLoopDesc

/*************************************************************************/
// DmTblLpDescReply1
// LoopDescriptor row entry has been read into the DM_Loop_Desc
// Modify this row entry with our VDN an send it back.
/*************************************************************************/
STATUS 	DriveMonitorIsm::DmTblLpDescReply1(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblLpDescReply1);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblLpDescReply1: status = ", status);
		
		// error trying to read our LoopDescriptor, send a listen
		// and exit
		DmTblLpDescReply2(pTC, ercOK);
		
		status = DmTblReplyLast(pTC, ercOK);
		return (status);
	}
	
	// Add our VDN to the LoopDescriptor
	DM_Loop_Desc->vdnDriveMonitor = MyVd;
	
	// Allocate an Modify Row object for the LoopDescriptor.
	m_ModifyRow = new(tUNCACHED) TSModifyRow;

	// Initialize the modify row operation.
	status = m_ModifyRow->Initialize(
		this,								// DdmServices pDdmServices,
		LOOP_DESCRIPTOR_TABLE,				// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		(void*)&DM_Loop_Desc->rid,			// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		DM_Loop_Desc,						// void* prgbRowData,
		sizeof(LoopDescriptorEntry),		// U32 cbRowData,
		0,									// U32 cRowsToModify,
		NULL,								// U32 *pcRowsModifiedRet,
		NULL,								// rowID *pRowIDRet,
		0,									// U32 cbMaxRowID,
		(pTSCallback_t)&DmTblReplyLast,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_ModifyRow->Send();
	
	return status;
	
} // DmTblLpDescReply1

/*************************************************************************/
// DmTblLpDescReply2
// LoopDescriptor row entry could not be read, send a Listen so we can
// hear when it is available.
/*************************************************************************/
STATUS 	DriveMonitorIsm::DmTblLpDescReply2(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblLpDescReply2);
	
	m_LSize = sizeof(LoopDescriptorEntry);
	
	// Allocate a Listen object for the LoopDescriptor table.
	m_Listen = new(tUNCACHED) TSListen;

	// Initialize the modify row operation.
	status = m_Listen->Initialize(
		this,								// DdmServices pDdmServices,
		(ListenOnInsertRow|ListenOnModifyOneRowAnyField), // U32		ListenType,
		LOOP_DESCRIPTOR_TABLE,				// String64	prgbTableName,
		fdLD_LOOP_NUM,						// String64	prgbRowKeyFieldName,
		(void*)&config.FC_instance,			// void*	prgbRowKeyFieldValue,
		sizeof(U32),						// U32		cbRowKeyFieldValue,
		NULL,								// String64	prgbFieldName,
		NULL,								// void*	prgbFieldValue,
		0,									// U32		cbFieldValue,
		(ReplyOnceOnly|ReplyWithRow),		// U32		ReplyMode, (once)
		NULL,								// void**	ppTableDataRet,
		0,									// U32*		pcbTableDataRet,
		NULL,								// U32*		pListenerIDRet,
		&m_LType,							// U32**	ppListenTypeRet,
		&m_Loop_Desc,						// void**	ppModifiedRecordRet,
		&m_LSize,							// U32*		pcbModifiedRecordRet,
		(pTSCallback_t)&DmTblReplyListenCallback,		// pTSCallback_t pCallback,
		NULL								// void*	pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_Listen->Send();
	
	return status;
	
} // DmTblLpDescReply2

/*************************************************************************/
// DmTblReplyLast
// Last CallBack, just return the Initialize message we saved way back
// when.
/*************************************************************************/
STATUS DriveMonitorIsm::DmTblReplyLast(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmTblReplyLast);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblReplyLast: status = ", status);
	}
	
	// decide what type of scan to do when the SCAN_ALL message comes.
	// the DM_FLAGS_SCAN_ALL flag in our config data will cause all
	// devices to be scanned 0 to 125.  The DM_FLAGS_LEGACY flag will
	// cause us to read the FCPortDatabase for this LoopInstance and build
	// a xlate[] table with all the Targets found on that loop.
	if (config.flags & DM_FLAGS_SCAN_ALL)
	{
		// fill our xlate[] table with all the devices starting with
		// device ID 0 and continuing to ID 125.
		config.num_drives = 126;
		
		for (int i = 0; i < 126; i++)
			config.xlate[i] = i;
	}
	else if (config.flags & DM_FLAGS_LEGACY)
	{
		// TODO:
		// read the FCPortdatabase for our LoopInstance
		// build an xlate[] table with all the target devices found
		//status = DmTblReadDB(pTC, 0);
	}
	
	// don't return the Initialize cookie until the last TS message
	Reply(pTC->pMsg); 
	
	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // DmTblReplyLast

/*************************************************************************/
// DmTblReplyListenCallback
// LoopDescriptor row entry has been read into the m_Loop_Desc pointer,
// copy local then,
// Modify this row entry with our VDN an send it back.
/*************************************************************************/
STATUS 	DriveMonitorIsm::DmTblReplyListenCallback(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(DmTblReplyListenCallback);
	
	// no work on first reply
	if (*m_LType & ListenInitialReply)
	{
		return(0);
	}
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDmTblReplyListenCallback: status = ", status);
	}
	
	// copy the reply data to a local chunk of memory
	memcpy(DM_Loop_Desc, m_Loop_Desc, sizeof(LoopDescriptorEntry));
	
	// Add our VDN to the LoopDescriptor
	DM_Loop_Desc->vdnDriveMonitor = MyVd;
	
	// Allocate an Modify Row object for the LoopDescriptor.
	m_ModifyRow = new(tUNCACHED) TSModifyRow;

	// Initialize the modify row operation.
	status = m_ModifyRow->Initialize(
		this,								// DdmServices pDdmServices,
		LOOP_DESCRIPTOR_TABLE,				// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		(void*)&DM_Loop_Desc->rid,			// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		DM_Loop_Desc,						// void* prgbRowData,
		sizeof(LoopDescriptorEntry),		// U32 cbRowData,
		0,									// U32 cRowsToModify,
		NULL,								// U32 *pcRowsModifiedRet,
		NULL,								// rowID *pRowIDRet,
		0,									// U32 cbMaxRowID,
		NULL,								// pTSCallback_t pCallback,
		NULL								// void* pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_ModifyRow->Send();
	
	return status;
	
} // DmTblReplyListenCallback


