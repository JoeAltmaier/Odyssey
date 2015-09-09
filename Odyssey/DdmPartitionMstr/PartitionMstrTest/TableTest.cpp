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
// File: TableTest.cpp
// 
// Description:
// General routines to Test PTS. Temporary Code. 
// 
// $Log: /Gemini/Odyssey/DdmPartitionMstr/PartitionMstrTest/TableTest.cpp $
// 
// 1     9/15/99 4:02p Dpatel
// Initial creation
// 
//
/*************************************************************************/



#include "OsTypes.h"
#include "Message.h"
#include "CTTypes.h"
#include "OsStatus.h"
#include "Ddm.h"
#include "Fields.h"
#include "RequestCodes.h"

#include "PartitionMstrTest.h"

#include "Rows.h"
#include "StorageRollCallTable.h"
#include "Listen.h"
#include "ReadTable.h"
#include "Table.h"
#include "PTSCommon.h"


#ifdef TEST_TABLES
//**************************************************************************
//
//	Create Command Queue and Register Listener for any inserts
//
//**************************************************************************
STATUS PartitionMstrTest::
CreateSRCTable()
{
STATUS status;


	// Allocate an Define Table object for the Virtual Class Descriptor Table.
	m_pSRCDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = m_pSRCDefineTable->Initialize(
		this,											// DdmServices* pDdmServices,
		STORAGE_ROLL_CALL_TABLE,						// String64 prgbTableName,
		StorageRollCallTable_FieldDefs,					// fieldDef* prgFieldDefsRet,
		cbStorageRollCallTable_FieldDefs,				// U32 cbrgFieldDefs,
		10,
		true,											// bool* pfPersistant,
		(pTSCallback_t)&PartitionMstrTest::SRCTableDefineReply,			// pTSCallback_t pCallback,
		NULL											// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pSRCDefineTable->Send();

	return status;
}


//**************************************************************************
//
//	Reply for SRC Table Define
//
//**************************************************************************
STATUS PartitionMstrTest
::SRCTableDefineReply(void *pClientContext, STATUS status)
{
	// First Register a Listen on Inserts
	TSListen			*pTSListenTable;

	// Now register a listener for any inserts into this table

	// We are not supposed to allocate this, PTS will allocate
	// and we have to delete [ but does not work for win32 so alloc]
	m_sizeofSRCRecord = sizeof(StorageRollCallRecord);
	m_pSRCRecord = new StorageRollCallRecord;
	memset(m_pSRCRecord,0,sizeof(StorageRollCallRecord));

	m_sizeofModifiedSRCRecord = sizeof(StorageRollCallRecord);
	m_pModifiedSRCRecord = new StorageRollCallRecord;
	memset(m_pModifiedSRCRecord,0,sizeof(StorageRollCallRecord));

	// Table Return will return the entire table entries,
	// So pass NULL since we are interested in only the  
	// modified record.
	pTSListenTable = new TSListen;
	pTSListenTable->Initialize(
		this,									// DdmServices *pDdmServices,
		ListenOnInsertRow,						// ListenTypeEnum ListenType,
		STORAGE_ROLL_CALL_TABLE,				// String64 prgbTableName,
		NULL,									// String64 prgbRowKeyFieldName,
		NULL,									// void* prgbRowKeyFieldValue,
		0,										// U32 cbRowKeyFieldValue,
		NULL,									// String64 prgbFieldName,
		NULL,									// void* prgbFieldValue,
		0,										// U32 cbFieldValue,
		ReplyContinuous | ReplyWithRow,			// ReplyModeEnum ReplyMode - continuous
		NULL,
		NULL,
		//(void **)&m_pSRCRecord,				// void** ppTableDataRet,
		//&m_sizeofSRCRecord,					// U32* pcbTableDataRet,
		&m_SRCListenerID,						// U32* pListenerIDRet,
		&m_pSRCListenReplyType,					// U32** ppListenTypeRet,
		(void **)&m_pModifiedSRCRecord,			// void** ppModifiedRecordRet,
		&m_sizeofModifiedSRCRecord,				// U32* pcbModifiedRecordRet,
		(pTSCallback_t)&PartitionMstrTest::ListenSRCInsertRowReply,	// pTSCallback_t pCallback,
		this									// void* pContext
	);
	pTSListenTable->Send();


	// Now insert a record
	TSInsertRow		*pInsertRow;

	// Add a Record into the Storage Roll Call (this should be done as
	// a pContext and should be deleted in the InsertReply
	StorageRollCallRecord *pSRCRecord = (StorageRollCallRecord *)
			new char[sizeof(StorageRollCallRecord)];
	memset(pSRCRecord,0,sizeof(StorageRollCallRecord));

	pSRCRecord->version = STORAGE_ROLL_CALL_TABLE_VERSION;
	pSRCRecord->size = sizeof(StorageRollCallRecord);
	pSRCRecord->Capacity = 0xFFFFEEEE;
	pSRCRecord->vdnBSADdm = m_dummyVDN++;
	pSRCRecord->storageclass = SRCTypeSSD;
	pInsertRow = new TSInsertRow;
	pInsertRow->Initialize(
		this,
		STORAGE_ROLL_CALL_TABLE,
		pSRCRecord,
		sizeof(StorageRollCallRecord),
		&m_ridNewSRCRecord,
		(pTSCallback_t)&PartitionMstrTest::SRCInsertRowReply,
		this
	);
	pInsertRow->Send();
	return status;
}

STATUS PartitionMstrTest
::SRCInsertDummySRCRecord()
{
	STATUS			status;
	// Now insert a record
	TSInsertRow		*pInsertRow;

	// Add a Record into the Storage Roll Call (this should be done as
	// a pContext and should be deleted in the InsertReply
	StorageRollCallRecord *pSRCRecord = (StorageRollCallRecord *)
			new char[sizeof(StorageRollCallRecord)];
	memset(pSRCRecord,0,sizeof(StorageRollCallRecord));

	pSRCRecord->version = STORAGE_ROLL_CALL_TABLE_VERSION;
	pSRCRecord->size = sizeof(StorageRollCallRecord);
	pSRCRecord->Capacity = 0xFFFFEEEE;
	pSRCRecord->vdnBSADdm = m_dummyVDN++;
	pSRCRecord->storageclass = SRCTypeSSD;
	pInsertRow = new TSInsertRow;
	status = pInsertRow->Initialize(
		this,
		STORAGE_ROLL_CALL_TABLE,
		pSRCRecord,
		sizeof(StorageRollCallRecord),
		&m_ridNewSRCRecord,
		(pTSCallback_t)&PartitionMstrTest::SRCInsertRowReply,
		this
	);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pInsertRow->Send();
	return status;
}	

//**************************************************************************
//
// Reply for insert row
//
//**************************************************************************
STATUS PartitionMstrTest
::SRCInsertRowReply( void* pContext, STATUS status )
{
	UNUSED(pContext);
	status = SRCReadRow(&m_ridNewSRCRecord);
	m_numberOfDummySRCRecords++;
	if (m_numberOfDummySRCRecords < MAX_DUMMY_SRC_RECORDS){
		SRCInsertDummySRCRecord();
	}
	return status;
}

STATUS PartitionMstrTest
::SRCReadRow(rowID* pRowToRead)
{
	STATUS			status;
	// Record inserted into the command table
	// Now do a read to make sure the entry is there
	TSReadRow *pReadRow = new TSReadRow;

	m_pNewSRCRecord = new StorageRollCallRecord;
	memset(m_pNewSRCRecord,0,sizeof(StorageRollCallRecord));

	status = pReadRow->Initialize(
			this,
			STORAGE_ROLL_CALL_TABLE,
			CT_PTS_RID_FIELD_NAME,
			pRowToRead,
			sizeof(rowID),
			m_pNewSRCRecord,
			sizeof(StorageRollCallRecord),
			NULL,
			(pTSCallback_t)&PartitionMstrTest::SRCReadRowReply,
			this
			);
	pReadRow->Send();
	return status;
}


STATUS PartitionMstrTest
::SRCReadRowReply( void* _pContext, STATUS status )
{
	UNUSED(_pContext);
	U32 version;
	version = m_pNewSRCRecord->version;
	delete m_pNewSRCRecord;
	return status;
}


STATUS PartitionMstrTest
::ListenSRCInsertRowReply( void* _pContext, STATUS status )
{
	UNUSED(_pContext);
	if (*m_pSRCListenReplyType & ListenOnInsertRow)
	{
		if (status == OS_DETAIL_STATUS_SUCCESS){
			status = status;
		}
	}
	return status;
}


#endif