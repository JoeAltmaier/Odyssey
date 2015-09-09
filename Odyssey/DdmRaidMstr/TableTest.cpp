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
// $Log: /Gemini/Odyssey/DdmRaidMstr/TableTest.cpp $
// 
// 12    8/27/99 5:55p Dpatel
// removed code for vd..
// 
// 11    8/24/99 9:24a Dpatel
// VD table commented..
// 
// 10    8/16/99 2:51p Dpatel
// ReplyContinuous | ReplyWithRow ...
// 
// 9     8/14/99 1:37p Dpatel
// Added event logging..
// 
// 8     8/06/99 2:07p Dpatel
// Test changes..
// 
// 7     8/02/99 3:18p Jtaylor
// fixed warnings
// 
// 6     7/26/99 3:00p Dpatel
// removed the status from Send().
// 
// 5     7/21/99 5:05p Dpatel
// Set the size of dummy src records to FFFFEEEE
// 
// 4     7/06/99 9:42a Dpatel
// memset SRC records to 0 after new.
// 
// 3     6/28/99 5:16p Dpatel
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
#include "RequestCodes.h"

#include "DdmRaidMgmt.h"

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
STATUS DdmRAIDMstr::
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
		RMSTR_TBL_RSRV_ENTRIES,											// U32 cEntriesRsv,
		true,											// bool* pfPersistant,
		(pTSCallback_t)&DdmRAIDMstr::SRCTableDefineReply,			// pTSCallback_t pCallback,
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
STATUS DdmRAIDMstr
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
		(pTSCallback_t)&DdmRAIDMstr::ListenSRCInsertRowReply,	// pTSCallback_t pCallback,
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
		(pTSCallback_t)&DdmRAIDMstr::SRCInsertRowReply,
		this
	);
	pInsertRow->Send();
	return status;
}

STATUS DdmRAIDMstr
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
		(pTSCallback_t)&DdmRAIDMstr::SRCInsertRowReply,
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
STATUS DdmRAIDMstr
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

STATUS DdmRAIDMstr
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
			(pTSCallback_t)&DdmRAIDMstr::SRCReadRowReply,
			this
			);
	pReadRow->Send();
	return status;
}


STATUS DdmRAIDMstr
::SRCReadRowReply( void* _pContext, STATUS status )
{
	UNUSED(_pContext);
	U32 version;
	version = m_pNewSRCRecord->version;
	delete m_pNewSRCRecord;
	return status;
}


STATUS DdmRAIDMstr
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

STATUS DdmRAIDMstr
::CreateNewVirtualDevice( )
{
#if 0
	CONTEXT					*pContext = new CONTEXT();

	VirtualDeviceRecord		*pNewVDTRecord = NULL;
	STATUS					status = 0;

	pContext->pData = new (tZERO) VirtualDeviceRecord;
	pNewVDTRecord = (VirtualDeviceRecord *)pContext->pData;
		
	// Otherwise fill out a new Virtual Device Table Record and insert it.
	pNewVDTRecord->version = VIRTUAL_DEVICE_TABLE_VERSION;
	pNewVDTRecord->size = sizeof(VirtualDeviceRecord);

	// Mark the VDT Record with the rowID of the command that created it.
	//pNewVDTRecord.ridVDOwnerUse = pCmdInfo->pRequest->rid;

	pNewVDTRecord->eVDFlags = eVDFlagMask_Redundant;
	pNewVDTRecord->eVDStateActual = eVDState_Defined;		// Current state of the Virtual Device
	pNewVDTRecord->eVDStateDesired = eVDState_Configured;	// Desired state of the Virtual Device

	strcpy(pNewVDTRecord->stClassName, "DdmSTS" );

	pNewVDTRecord->sStackSize = 10240;			// Stack size 10K default.
	// Resolve: pNewVDTRecord->ridDdmCfgRec = m_SCSITargetConfigRec.rid;	// Configuration data for the VD.
	pNewVDTRecord->didPrimary = 0;	// Resolve: What value to put??
	pNewVDTRecord->didStandby = 0; // Resolve: What value to put??
	
	TSInsertRow *pInsertVDTRec = new TSInsertRow;
	status = pInsertVDTRec->Initialize(
			this,									// DdmServices *pDdmServices,
			VIRTUAL_DEVICE_TABLE,					// String64 rgbTableName,
			pNewVDTRecord,							// void *prgbRowData,
			sizeof(VirtualDeviceRecord),				// U32 cbRowData,
			&pNewVDTRecord->rid,					// rowID *prowIDRet,
			(pTSCallback_t)&DdmRAIDMstr::CreateVirtualDeviceReply,	// pTSCallback_t pCallback,
			pContext								// void* pContext
		);
	
	if (status == OK)
		pInsertVDTRec->Send();

	return status;
#endif
	return OK;
}

STATUS DdmRAIDMstr
::CreateVirtualDeviceReply( void* _pContext, STATUS status )
{
#if 0
	CONTEXT					*pContext = (CONTEXT *)_pContext;
	VirtualDeviceRecord		*pNewVDTRecord = NULL;

	pNewVDTRecord = (VirtualDeviceRecord *)pContext->pData;
	delete pContext;
#endif
	return status;
}


#endif