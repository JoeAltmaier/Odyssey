/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdTables.cpp
// 
// Description:
// This module is the SSD Table Handler
// 
// $Log: /Gemini/Odyssey/DdmSSD/SsdTables.cpp $
// 
// 10    1/12/00 9:53a Jfrandeen
// Initialize the m_p_SSD_descriptor; checkin for Huy
// 
// 9     12/10/99 1:25p Hdo
// Modify capacity in SSD_Descriptor and StorageRollCall tables after
// Enable
// Remove fieldDefs variables and new(tPCI)
// Add rid_NameString, rid_Status and rid_Performance
// 
// 8     11/22/99 10:47a Jfrandeen
// Use new FlashStorage in place of FlashFile
// 
// 7     11/09/99 5:37p Hdo
// Change in Ssd_DescTable_InsertRow to match with new FF_CONFIG
// 
// 6     11/09/99 2:12p Hdo
// 
// 5     10/26/99 3:37p Hdo
// Add code to support ridIopStatusTable
// 
// 4     10/22/99 11:33a Hdo
// Remove table update handle and listenr methods
// 
// 2     9/08/99 7:55p Hdo
// Modify and add the Table update method
//
// 07/27/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD1
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"

#include "SsdDdm.h"
#include "FlashMonitorIsm.h"

#define MAX_NUMBER_BOARD	16

/*************************************************************************/
// Ssd_Table_Modify_Capacity
// Send our capacity to the table service.  This happens during the Enable
// state.
/*************************************************************************/
STATUS SSD_Ddm::Ssd_Table_Modify_Capacity()
{
	TRACE_ENTRY(SSD_Ddm::Ssd_Table_Modify_Capacity);

	// Modify the SSD_Descriptor record and send back
	m_p_modify_row = new TSModifyRow();

	m_p_SSD_descriptor->Capacity = m_capacity;

	STATUS status = m_p_modify_row->Initialize(
		this,							// *pDdmServices
		SSD_DESCRIPTOR_TABLE,			// Table name
		"rid",							// rgbKeyFieldName
		&m_p_SSD_descriptor->rid,		// *pKeyFieldValue
		sizeof(rowID),					// cbKeyFieldValue
		m_p_SSD_descriptor,
		sizeof(SSD_Descriptor),
		1,								// 1 row
		NULL,							// *pcRowsModifiedRet
		&m_row_ID_SSD_descriptor,		// returned rowID
		sizeof(rowID),					// cbMaxRowID
		NULL,
		NULL							// pContext
	);

	if( status == ercOK )
		m_p_modify_row->Send();

	// Modify the StorageRollCall table
	m_p_modify_field = new TSModifyField();

	status = m_p_modify_field->Initialize(
		this,							// ClientDdm
		STORAGE_ROLL_CALL_TABLE,
		"rid",							// rgbKeyFieldName
		&m_row_ID_SRC,					// *pKeyFieldValue
		sizeof(rowID),					// cbKeyFieldValue
		fdSRC_CAPACITY,					// rgbFieldName
		&m_capacity,					// *pbFieldValue
		sizeof(m_capacity),				// cbFieldValue
		0,								// cRowsToModify; 0 = ALL
		NULL,							// pcRowsModifiedRet
		NULL,							// *pRowIDRet
		0,								// cbMaxRowID
		NULL,							// pTSCallback_t pCallback,
		NULL							// pContext
	);

	if (status == ercOK)
		m_p_modify_field->Send();

	return status;

} // Ssd_Table_Modify_Capacity

/*************************************************************************/
// Ssd_Table_Get_ridIopStatus
// Read the IopStatusTable and find the board
/*************************************************************************/
STATUS SSD_Ddm::Ssd_Table_Get_ridIopStatus()
{
	TRACE_ENTRY(SSD_Ddm::Ssd_Table_Get_ridIopStatus);

	STATUS status;
	TySlot	mySlot = Address::GetSlot();

	m_p_IopStatus = new(tZERO) IOPStatusRecord();

	// Allocate an ReadRow object.
	m_p_IOP_ReadRow = new TSReadRow();

	// Initialize the ReadRow operation.
	status = m_p_IOP_ReadRow->Initialize(
		this,
		CT_IOPST_TABLE_NAME,		// Name of table to read
		CT_IOPST_SLOT,				// rgbKeyFieldName
		&mySlot,					// *pKeyFieldValue
		sizeof(TySlot),				// cbKeyFieldValue
		m_p_IopStatus,				// Returned data buffer.
		sizeof(IOPStatusRecord),	// max size of returned data.
		&m_nIopRowsRead,			// returned number of row.
		(pTSCallback_t) &Ssd_Get_ridIopStatus_Reply,
		NULL
		);

	if( status == ercOK )
		m_p_IOP_ReadRow->Send();

	return status;

} // Ssd_Table_Get_ridIopStatus

/*************************************************************************/
// Ssd_Get_ridIopStatus_Reply
// 
/*************************************************************************/
STATUS SSD_Ddm::Ssd_Get_ridIopStatus_Reply(void *pContext, STATUS status)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_Get_ridIopStatus_Reply);

	if( status != ercOK || m_nIopRowsRead == 0 )
	{
		;	// Serious error
	}

	return status;

} // Ssd_Get_ridIopStatus_Reply

/*************************************************************************/
// Ssd_Table_Initialize
// Start of the table initialization state machine, called from the DdmInit
// Creates these tables if they do not exist:
//	SSD_Descriptor
//  StorageRollCallTable
// Reads these tables:
//	SSD_Descriptor
//  StorageRollCallTable
/*************************************************************************/
STATUS	SSD_Ddm::Ssd_Table_Initialize(Message *pMsg)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_Table_Initialize);

	STATUS status;

	// Get the rowID for this board from the IopStatusTable
	status = Ssd_Table_Get_ridIopStatus();

	if ( status != ercOK )
	{
		Reply(pMsg, status);
		return status;
	}

	// Initialize the rowID structures
	memset(&m_rid_StringName, 0, sizeof(rowID));
	memset(&m_rid_Status, 0, sizeof(rowID));
	memset(&m_rid_Performance, 0, sizeof(rowID));
	// Initialize the m_p_SSD_descriptor
	m_p_SSD_descriptor = NULL;

	m_p_define_table = new TSDefineTable();

	status = m_p_define_table->Initialize
	(
		this,								// *pDdm,
		SSD_DESCRIPTOR_TABLE,				// prgbTableName,
		SSD_descriptor_table_field_defs,	// *prgFieldDefsRet,
		cb_SSD_descriptor_table_field_defs,	// cbrgFieldDefs,
		MAX_NUMBER_BOARD,					// number of devices
		TRUE,								// persistant,
		(pTSCallback_t)&Ssd_DescTable_InsertRow,// pCallback,
		(void*)pMsg							// *pContext
	);

	if( status == ercOK )
		m_p_define_table->Send();

	return status;

} // Ssd_Table_Initialize

/*************************************************************************/
// Ssd_DescTable_InsertRow
// Reply from creating the SSD_Descriptor Table. If it already exists,
// read all the rows.  If it does not, create an entry.
/*************************************************************************/
STATUS SSD_Ddm::Ssd_DescTable_InsertRow(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_DescTable_InsertRow);

	// If the table exists, read howany entries
	if (status == ercTableExists)
	{
		TRACE_STRING(TRACE_L8, "\n\rSSDDesc table already defined");

		status = Ssd_DescTable_ReadNumRow(pClientContext, ercOK);
		return (status);
	}

	// Table did not exist, it does now, so load it with default data
	TRACE_STRING(TRACE_L8, "\n\rLoading SSDDesc table");

	m_p_SSD_descriptor = new(tZERO) SSD_Descriptor();

	// fill in the blanks to generate a default table
	m_p_SSD_descriptor->version = SSD_DESC_VERSION;
	m_p_SSD_descriptor->size = sizeof(SSD_Descriptor);

	m_p_SSD_descriptor->Capacity = m_config.capacity;
	m_p_SSD_descriptor->vdnBSADdm = m_config.vdnBSADdm;
	strcpy(m_p_SSD_descriptor->SerialNumber, m_config.SerialNumber);
	m_p_SSD_descriptor->ridIopStatus = m_p_IopStatus->rid;
	m_p_SSD_descriptor->vdnMonitor = m_config.vdnMonitor;
	m_p_SSD_descriptor->memory_size = MEMORY_FOR_SSD;
	m_p_SSD_descriptor->callback_memory_size = MEMORY_FOR_CALLBACKS;

	// Initialize cache config fields
	m_p_SSD_descriptor->cache_config.version = CM_CONFIG_VERSION;

	// Size need not be filled in -- only used when CM returns cache config.
	m_p_SSD_descriptor->cache_config.size = 0;

	// page_size is ignored by the cache manager; device page size is used.
	m_p_SSD_descriptor->cache_config.page_size = m_config.page_size;

	// page_table_size specifies the number of entries in the page table 
	// for a cache that uses linear mapping.
	// This will be overridden by the actual number of user pages.
	m_p_SSD_descriptor->cache_config.page_table_size = SSD_NUM_VIRTUAL_PAGES;

	// Number of pages in the cache.
	// If the user does not fill in the memory parameters for the 
	// cache config, the left over memory will be used for the cache manager.
	m_p_SSD_descriptor->cache_config.num_pages = 0;

	// Pointer to memory to be used for pages.
	// This will be filled in by the flash storage manager.
	m_p_SSD_descriptor->cache_config.p_page_memory = 0;

	// Number of pages in the secondary cache.
	// We don't use a secondary cache.
	m_p_SSD_descriptor->cache_config.num_pages_secondary = 0;
	m_p_SSD_descriptor->cache_config.p_page_memory_secondary = 0;

	// Pointer to memory to be used for internal cache tables
	// This will be filled in by the flash storage manager.
	m_p_SSD_descriptor->cache_config.p_table_memory = 0;

	// page_table_size and hash_table_size are mutually exclusive.
	m_p_SSD_descriptor->cache_config.hash_table_size =
		m_config.hash_table_size;
	m_p_SSD_descriptor->cache_config.num_reserve_pages =
		m_config.num_reserve_pages;
	m_p_SSD_descriptor->cache_config.dirty_page_writeback_threshold =
		m_config.dirty_page_writeback_threshold;
	m_p_SSD_descriptor->cache_config.num_prefetch_forward =
		m_config.num_prefetch_forward;
	m_p_SSD_descriptor->cache_config.num_prefetch_backward =
		m_config.num_prefetch_backward;

	// Initialize flash config fields
	m_p_SSD_descriptor->flash_config.version = FF_CONFIG_VERSION;

	// Size need not be filled in -- only used when flash manager returns flash config.
	m_p_SSD_descriptor->flash_config.size = 0; 

	// Pointer to flash device will be filled in later.
	m_p_SSD_descriptor->flash_config.p_device = 0; 

	// Pointer to memory will be filled in later.
	m_p_SSD_descriptor->flash_config.p_memory = 0; 

	// Amount of memory to be allocated for the flash storage system.
	// If the user does not fill in the memory parameters for the 
	// cache config, the left over memory will be used for the cache manager.
	m_p_SSD_descriptor->flash_config.memory_size = MEMORY_FOR_SSD;

	m_p_SSD_descriptor->flash_config.verify_write = m_config.verify_write_level;
	m_p_SSD_descriptor->flash_config.verify_erase = m_config.verify_erase_level;
	m_p_SSD_descriptor->flash_config.verify_page_erased_before_write = 1;
	m_p_SSD_descriptor->flash_config.wear_level_threshold = 10;

	// Format configuration parameters begin here.
	m_p_SSD_descriptor->flash_config.percentage_erased_pages =
		m_config.percentage_erase_level;
	m_p_SSD_descriptor->flash_config.percentage_replacement_pages =
		m_config.percentage_replacement_pages;
	m_p_SSD_descriptor->flash_config.replacement_page_threshold =
		m_config.replacement_page_threshold;
	m_p_SSD_descriptor->flash_config.erase_all_pages =
		m_config.erase_all_pages;

	// Debugging parameters here.
	m_p_SSD_descriptor->flash_config.verify_structures =
		m_config.verify_structures;

	// Error injection parameters begin here.
	m_p_SSD_descriptor->flash_config.test_all_random = 0;
	m_p_SSD_descriptor->flash_config.test_all_static = 0;
	m_p_SSD_descriptor->flash_config.write_error_frequency_value = 0;
	m_p_SSD_descriptor->flash_config.erase_error_frequency_value = 0;
	m_p_SSD_descriptor->flash_config.read_error_frequency_value = 0;

	m_p_insert_row = new TSInsertRow();

	status = m_p_insert_row->Initialize(
		this,							// *ClientDdm
		SSD_DESCRIPTOR_TABLE,			// rgbTableName
		m_p_SSD_descriptor,				// pbRowData
		sizeof(SSD_Descriptor),			// cbRowData
		&m_row_ID_SSD_descriptor,		// *pRowIDRet
		(pTSCallback_t)&Ssd_DescTable_Reply_Last,// *pCallback,
		(void*)pClientContext				// *pContext
	);

	if( status == ercOK )
		m_p_insert_row->Send();

	return status;

} // Ssd_DescTable_InsertRow

/*************************************************************************/
// Ssd_DescTable_ReadNumRow
// The SsdDescriptor table now exists.  Read number of rows in the table.
// Use this count to to build the array for local use.
/*************************************************************************/
STATUS SSD_Ddm::Ssd_DescTable_ReadNumRow(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_DescTable_ReadNumRow);

	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\Ssd_DescTable_ReadNumRow: status = ", status);

		status = Ssd_SRCTable_Initialize(pClientContext, ercOK);
		return status;
	}

	m_num_table_rows = 0;

	// Allocate a Get Table Defs object for the DiskDescTable.
	m_p_TSGetTableDef = new TSGetTableDef();

	// Initialize the GetTableDef operation.
	status = m_p_TSGetTableDef->Initialize( 
		this,
		SSD_DESCRIPTOR_TABLE,
		NULL,						// rgFieldDefsRet
		0,							// cbrgFieldDefsRetMax
		NULL,						// pcFieldDefsRet
		NULL,						// pcbRowRet
		&m_num_table_rows,			// Returned data buffer (row count).
		NULL,						// pcFieldsRet
		NULL,						// pPersistFlagsRet
		(pTSCallback_t)&Ssd_DescTable_Read_Rows,
		(void*)pClientContext
	  );

	if( status == ercOK )
		m_p_TSGetTableDef->Send();
	
	return status;
	
} // Ssd_DescTable_ReadNumRow

/*************************************************************************/
// Ssd_DescTable_Read_Rows
// Number of rows is now read into m_num_table_rows.  Use this count to
// create the local SSD_Descriptor table. And start the read of matched
// entries.
/*************************************************************************/
STATUS SSD_Ddm::Ssd_DescTable_Read_Rows(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_DescTable_Read_Rows);
	
	if (status != ercOK || m_num_table_rows == 0 )
	{
		TRACE_HEX(TRACE_L8, "\n\Ssd_DescTable_Read_Rows: status = ", status);
	
		status = Ssd_DescTable_InsertRow(pClientContext, ercOK);
		return status;
	}

	TRACE_HEX(TRACE_L8, "\n\rSSD_Ddm: num of descriptor rows = ", m_num_table_rows);
	
	// Allocate, init and send off a read row object for the SSD_Descriptor

	// Allocate the Local SSD Descriptor Table
	if ( m_p_SSD_descriptor )
	{
		// Delete the old descriptor record
		delete [] m_p_SSD_descriptor;
	}

	// Allocate a new buffer enough to hold all the SSD type records
	m_p_SSD_descriptor = new(tZERO) SSD_Descriptor[m_num_table_rows];
		
	// Allocate an ReadRow object.
	m_p_ReadRow = new TSReadRow();

	// Initialize the read row operation.
	status = m_p_ReadRow->Initialize(
		this,
		SSD_DESCRIPTOR_TABLE,				// Name of table to read
		"SerialNumber",						// KeyFieldName
		m_config.SerialNumber,				// KeyFieldValue
		sizeof(String32),					// cbKeyFieldValue
		m_p_SSD_descriptor,					// Returned data buffer.
		sizeof(SSD_Descriptor) * m_num_table_rows,	// max size of returned data.
		&m_nTableRowsRead,					// returned number of row.
		(pTSCallback_t)&Ssd_DescTable_ReadRow_Reply,
		pClientContext
		);

	if( status == ercOK )
		m_p_ReadRow->Send();
	
	return status;
} // Ssd_DescTable_Read_Rows
	
/*************************************************************************/
// Ssd_DescTable_ReadRow_Reply
// The matched rows is read in.  If no rows matched, insert a new row.  If
// more than one row match, update the first one, and delete the rest.  If
// exact one row is matched, update the table.
/*************************************************************************/
STATUS SSD_Ddm::Ssd_DescTable_ReadRow_Reply(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_DescTable_ReadRow_Reply);

	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\Ssd_DescTable_ReadRow_Reply: status = ", status);
	}

	if( m_nTableRowsRead == 0 )
	{
		status = Ssd_DescTable_InsertRow(pClientContext, ercOK);

		return status;
	}

	// If more than one entry returned, modify the first and delete the rest
	// TODO: get the first one, delete other entries
	if( m_nTableRowsRead > 1 )
	{
		// Skip the first one
		SSD_Descriptor	*p_SSD_Desc = m_p_SSD_descriptor++;
		for (U32 rows = 1; rows < m_nTableRowsRead; rows++)
		{
			TSDeleteRow *p_DelRow = new TSDeleteRow();
			status = p_DelRow->Initialize(
				this,
				SSD_DESCRIPTOR_TABLE,				// Table name
				"rid",
				&p_SSD_Desc->rid,
				sizeof(rowID),
				0,
				NULL,
				NULL,
				NULL
			);
			p_SSD_Desc++;
		}
	}

	m_p_SSD_descriptor->Capacity = m_config.capacity;
	m_p_SSD_descriptor->vdnBSADdm = m_config.vdnBSADdm;
	m_p_SSD_descriptor->ridIopStatus = m_p_IopStatus->rid;
	m_p_SSD_descriptor->vdnMonitor = m_config.vdnMonitor;

	// Modify the SSD_Descriptor record and send back
	m_p_modify_row = new TSModifyRow();
	status = m_p_modify_row->Initialize(
		this,								// *pDdmServices
		SSD_DESCRIPTOR_TABLE,				// Table name
		"rid",								// rgbKeyFieldName
		&m_p_SSD_descriptor->rid,			// *pKeyFieldValue
		sizeof(rowID),						// cbKeyFieldValue
		m_p_SSD_descriptor,
		sizeof(SSD_Descriptor),
		1,									// 1 row
		NULL,								// *pcRowsModifiedRet
		&m_row_ID_SSD_descriptor,			// returned rowID
		sizeof(rowID),						// cbMaxRowID
		(pTSCallback_t)&Ssd_DescTable_Reply_Last,
		(void*)pClientContext				// pContext
	);

	if( status == ercOK )
		m_p_modify_row->Send();

	return status;

} // Ssd_DescTable_Reply_Last

/*************************************************************************/
// Ssd_Table_Reply_Last
// Last CallBack, clean up and call the initialization of
// the StorageRollCall table.
/*************************************************************************/
STATUS SSD_Ddm::Ssd_DescTable_Reply_Last(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_DescTable_Reply_Last);

	TRACE_HEX(TRACE_L8, "\n\rSsd_Table_Reply_Last: status = ", status);

	// Call the Ssd_SRCTable_Initialize
	status = Ssd_SRCTable_Initialize(pClientContext, ercOK);

	return status;

} // Ssd_DescTable_Reply_Last


/*************************************************************************/
// Ssd_SRCTable_Initialize
// Create the StorageRollCall table.  Make sure there is a minimum of 100
// entries. Rows are loaded on the fly to this table using Table update methods
/*************************************************************************/
STATUS SSD_Ddm::Ssd_SRCTable_Initialize(void *pContext, STATUS status)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_SRC_Table_Initialize);

	m_p_define_SRC_table = new TSDefineTable();

	if( m_p_define_SRC_table == NULL )
		return ercNoMoreHeap;

	status = m_p_define_SRC_table->Initialize(
		this,
		STORAGE_ROLL_CALL_TABLE,
		StorageRollCallTable_FieldDefs,
		cbStorageRollCallTable_FieldDefs,
		100,
		FALSE,
		(pTSCallback_t)&Ssd_SRCTable_InsertRow,
		(void *)pContext
	);

	if( status == ercOK )
		m_p_define_SRC_table->Send();

	return status;
	
} // Ssd_SRCTable_Initialize

/*************************************************************************/
// Ssd_SRCTable_InsertRow
// Reply from creating the SRC table. If it already exists, read all rows.
// If it does not, create an entry for it.
/*************************************************************************/
STATUS SSD_Ddm::Ssd_SRCTable_InsertRow(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_SRCTable_InsertRow);

	// If the table exists, read howany entries
	if (status == ercTableExists)
	{
		TRACE_STRING(TRACE_L8, "\n\rSRC Table already defined");

		status = Ssd_SRCTable_ReadTable(pClientContext, ercOK);
		return status;
	}

	// Allocate a storage roll call record.
	m_p_SRC = new StorageRollCallRecord();

	// fill in all the StorageRollCall entries
	m_p_SRC->version = STORAGE_ROLL_CALL_TABLE_VERSION;
	m_p_SRC->size = sizeof(StorageRollCallRecord);
	m_p_SRC->Capacity = m_capacity;
	m_p_SRC->fUsed = FALSE;
	m_p_SRC->storageclass = SRCTypeSSD;
	m_p_SRC->vdnBSADdm = GetVdn();
	m_p_SRC->ridDescriptorRecord = m_row_ID_SSD_descriptor;

	// TODO: need to get the rid from the Status table
	// as well as the Performance, and the StringName table
	m_p_SRC->ridStatusRecord = m_rid_Status;
	m_p_SRC->ridPerformanceRecord = m_rid_Performance;
	m_p_SRC->vdnMonitor = m_config.vdnMonitor;
	m_p_SRC->ridName = m_rid_StringName;

	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the StorageRollCallTable.
	m_p_insert_row_SRC = new TSInsertRow();
	
	status = m_p_insert_row_SRC->Initialize(
		this,							// Ddm* ClientDdm
		STORAGE_ROLL_CALL_TABLE,		// prgbTableName
		m_p_SRC,						// prgbRowData
		sizeof(StorageRollCallRecord),	// cbRowData
		&m_row_ID_SRC,					// *pRowIDRet
		(pTSCallback_t)&Ssd_SRCTable_Reply_Last,
		pClientContext
	);
	
	if (status == ercOK)
		m_p_insert_row_SRC->Send();

	return status;
	
} // Ssd_SRCTable_InsertRow

/*************************************************************************/
// Ssd_SRCTable_ReadTable
// The SRC table exists.  Start reading all rows with the SRCTypeSSD type
// 
/*************************************************************************/
STATUS SSD_Ddm::Ssd_SRCTable_ReadTable(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_SRCTable_ReadTable);

	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\Ssd_SRCTable_ReadTable: status = ", status);
	}

	SRCStorageTypes tSRCType = SRCTypeSSD;
	// Allocate an ReadRow object
	m_p_ReadRow = new TSReadRow();

	// Initialize the ReadRow operation.
	status = m_p_ReadRow->Initialize(
		this,
		STORAGE_ROLL_CALL_TABLE,			// Name of table to read
		fdSRC_STORAGE_CLASS,				// KeyFieldName
		&tSRCType,							// *pKeyFieldValue
		sizeof(SRCStorageTypes),			// cbKeyFieldValue
		&m_p_SRC,							// Returned data buffer.
		sizeof(StorageRollCallRecord)*MAX_NUMBER_BOARD,// max size of returned data.
		&m_nTableRowsRead,					// returned number of row.
		(pTSCallback_t)&Ssd_SRCTable_Read_Reply,
		(void*)pClientContext
	);

	if( status == ercOK )
		m_p_ReadRow->Send();
	
	return status;
	
} // Ssd_SRCTable_ReadTable

/*************************************************************************/
// Ssd_SRCTable_Read_Reply
// Reply from the SRC_ReadTable
// 
/*************************************************************************/
STATUS SSD_Ddm::Ssd_SRCTable_Read_Reply(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_SRCTable_Read_Reply);

	if (status != ercOK || m_nTableRowsRead == 0 )
	{
		TRACE_HEX(TRACE_L8, "\n\Ssd_SRCTable_Read_Reply: status = ", status);

		status = Ssd_SRCTable_InsertRow(pClientContext, ercOK);
		return status;
	}

	// If more than one entry returned, search for the record that matches
	// the m_p_SSD_descriptor, modify it and delete the rest
	if( m_nTableRowsRead > 1 )
	{
		for (U32 rows = 0; rows < m_nTableRowsRead; m_nTableRowsRead++)
		{
			// TODO
		}
	}

	// fill in all the StorageRollCall entries
	m_p_SRC->version = STORAGE_ROLL_CALL_TABLE_VERSION;
	m_p_SRC->size = sizeof(StorageRollCallRecord);
	m_p_SRC->Capacity = m_capacity;
	m_p_SRC->fUsed = FALSE;
	m_p_SRC->vdnBSADdm = GetVdn();
	m_p_SRC->ridDescriptorRecord = m_row_ID_SSD_descriptor;
	m_p_SRC->ridStatusRecord = m_rid_Status;
	m_p_SRC->ridPerformanceRecord = m_rid_Performance;
	m_p_SRC->vdnMonitor = m_config.vdnMonitor;
	m_p_SRC->ridName = m_rid_StringName;


	// Modify the SRC record and send back
	m_p_modify_row = new TSModifyRow();
	status = m_p_modify_row->Initialize(
		this,								// *pDdmServices
		STORAGE_ROLL_CALL_TABLE,			// Table name
		"rid",								// rgbKeyFieldName
		&m_p_SRC->rid,						// *pKeyFieldValue
		sizeof(rowID),						// cbKeyFieldValue
		m_p_SRC,
		sizeof(StorageRollCallRecord),
		0,									// all matched rows
		NULL,								// pcRowsModifiedRet
		&m_row_ID_SRC,						// returned rowID
		sizeof(rowID),						// cbMaxRowID
		(pTSCallback_t)&Ssd_SRCTable_Reply_Last,
		(void*)pClientContext				// pContext
	);

	if( status == ercOK )
		m_p_modify_row->Send();

	return status;
	
} // Ssd_SRCTable_Read_Reply

/*************************************************************************/
// Ssd_SRCTable_Reply_Last
// Last callback, reply to the saved pMsg
// 
/*************************************************************************/
STATUS SSD_Ddm::Ssd_SRCTable_Reply_Last(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(SSD_Ddm::Ssd_SRCTable_Reply_Last);

	TRACE_HEX(TRACE_L8, "\n\Ssd_SRCTable_Reply_Last: status = ", status);

	Reply((Message*)pClientContext);

	return status;

} // Ssd_SRCTable_Reply_Last
