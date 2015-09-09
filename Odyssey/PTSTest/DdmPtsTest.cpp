/* DdmPtsTest.cpp -- Test PTS and Interface classes DDM
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
//
// Revision History:
// $Log: /Gemini/Odyssey/PTSTest/DdmPtsTest.cpp $
// 
// 17    8/18/99 2:19p Legger
// Updated to reflect changes in row, del row interface in PTS.
// 
// 16    7/28/99 10:20a Sgavarre
// fix memory leaks;  remove return paramter to send
// 
// 15    7/22/99 3:08p Hdo
// Change to match with new TS Object
// 
// 14    7/14/99 3:28p Sgavarre
// fix DeleteRow for parameter change
// 
// 13    5/20/99 8:34a Legger
// Changed ReplyA so that it doesn't loop repeatedly and cause listen code
// to exceed the heap memory available.
// 
// 12    5/14/99 2:55p Cwohlforth
// Changed  to #include "Odyssey_Trace.h"
// 
// 11    5/13/99 2:23p Legger
// Added comments
// 
// 10    5/13/99 2:18p Legger
// Added TSDeleteRow functionality.
// Also added infinite loop to test memory leaks, etc.
// 
// 8     4/22/99 8:57a Jlane
// Enhanced for new Listen paramaters.
// 
// 7     4/16/99 6:16p Jlane
// Modifications made debugging Listen.
// 
// 6     4/05/99 3:25p Legger
// Added "SINGLE" parameter to CLASSNAME chingus.
// 
// 5     4/05/99 3:21p Legger
// Added Table.RowID value to Table definitions
// 
// 4     3/01/99 7:44p Ewedel
// Cleaned up various compile warnings.
//     02/01/99 JFL	Created.
//
 *
**/

#include <String.h>
#define _DEBUG
#include "OsTypes.h"
#include "Odyssey_Trace.h"
//#include "DdmScc.h"                           Removed by LWE. We don't appear to use this anymore
#include "DdmPtsTest.h"
#include "BuildSys.h"

#include "Table.h"
#include "Rows.h"

	
CLASSNAME(DdmPtsTest, SINGLE);	// Class Link Name used by Buildsys.cpp


// DdmPtsTest -- Constructor -------------------------------------------DdmPtsTest-
//
DdmPtsTest::DdmPtsTest(DID did): Ddm(did), services(this)
{
	Tracef("DdmPtsTest::DdmPtsTest()\n");
	SetConfigAddress(NULL,0);	// tell Ddm:: where my config area is
}
	

// Ctor -- Create ourselves --------------------------------------------DdmPtsTest-
//
Ddm *DdmPtsTest::Ctor(DID did)
{
	return new DdmPtsTest(did);
}


// Initialize -- Do post-construction initialization -------------------DdmBootMgr-
//
STATUS DdmPtsTest::Initialize(Message *pMsg)
{
	Tracef("DdmPtsTest::Initialize()\n");
	
	return Ddm::Initialize(pMsg);
}


// Enable -- Start-it-up -----------------------------------------------DdmPtsTest-
//
STATUS DdmPtsTest::Enable(Message *pMsg)
{
STATUS	status;

	Tracef("DdmPtsTest::Enable() config.vd=%u\n",config.vd);
	
	status = services.Start(config.id,config.vd);	
	Tracef("DdmPtsTest::Enable().start status = %u.\n", status);

	return Ddm::Enable(pMsg);
}

// Start -- Open the gates -------------------------------------------TestServices-
//
STATUS TestServices::Start(U16 _id,VDN _vdn)
{
	STATUS status;
	
	vdn = _vdn;
	id = _id;
	Tracef("In Test::Start()\n");
	
	// This is the code to create the DiskStattus Table.
	fieldDef	DiskStatusTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									8,	ROWID_FT, Persistant_PT,
		"version",									4,	U32_FT, Persistant_PT,
		"size",										4,	U32_FT, Persistant_PT,
		"key",										4,	U32_FT, Persistant_PT,
		"RefreshRate",								4,	U32_FT, Persistant_PT,
		"ridSRCRecord",								8,	ROWID_FT, Persistant_PT,
		"num_recoverable_media_errors_no_delay",	4,	U32_FT, Persistant_PT,
		"num_recoverable_media_errors_delay",		4,	U32_FT, Persistant_PT,
		"num_recoverable_media_errors_by_retry",	4,	U32_FT, Persistant_PT,
		"num_recoverable_media_errors_by_ecc",		4,	U32_FT, Persistant_PT,
		"num_recoverable_nonmedia_errors",			4,	U32_FT, Persistant_PT,
		"num_bytesprocessedtotal",					4,	U32_FT, Persistant_PT,
		"num_unrecoverable_media_errors",			4,	U32_FT, Persistant_PT,
	};
	
	m_pciDiskStatusTable_FieldDefs = (fieldDef*)new(tPCI) char(sizeof(DiskStatusTable_FieldDefs));
	memcpy( (char*)m_pciDiskStatusTable_FieldDefs,
			(char*)DiskStatusTable_FieldDefs,
			sizeof(DiskStatusTable_FieldDefs)
		  ); 
		  
	m_pDefineTable = new TSDefineTable;

	status = m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		DISK_STATUS_TABLE,					// String64 prgbTableName,
//			11,									// U32 crgFieldDefs,
		m_pciDiskStatusTable_FieldDefs	,		// fieldDef* prgFieldDefsRet,
		sizeof(DiskStatusTable_FieldDefs),	// U32 cbrgFieldDefs,
		20,									// U32 cEntriesRsv,
		false,								// bool* pfPersistant,
		(pTSCallback_t)&Reply1,				// pTSCallback_t pCallback,
		NULL								// void* pContext
	);

	//  status checks need to be done on all initializes
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pDefineTable->Send();
	else
		Tracef("TestServices::DefineTableInit() status=%u\n",status);
	
	return status;
}

// Reply -- Handle Replies to keep things going ----------------------TestServices-
//
STATUS TestServices::Reply1(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	Tracef("TestServices::Reply1() status=%u\n",status);
	delete (m_pciDiskStatusTable_FieldDefs);

	// Declare and initialize a Disk Status Record with default values. 
	DiskStatusRecord	newDiskStatusRecord = {
	// 	Default Value				FieldName									  Size   Type
		0,0,0,						// "rowID.HiPart, rowID.LoPart",				8,	RowID_FT, Persistant_PT,
		1,							// "Version",									4,	U32_FT, Persistant_PT,
		sizeof(DiskStatusRecord),	// "Size",										4,	U32_FT, Persistant_PT,
		0,							// "Key",										4,	U32_FT, Persistant_PT,
		1000000,					// "RefreshRate",								4,	U32_FT, Persistant_PT,
		0,0,0,						// "ridSRCRecord.HiPart, ridSRCRecord.LoPart",	8,	ROWID_FT, Persistant_PT,
		1,							// "num_recoverable_media_errors_no_delay",		4,	U32_FT, Persistant_PT,
		1,							// "num_recoverable_media_errors_delay",		4,	U32_FT, Persistant_PT,
		1,							// "num_recoverable_media_errors_by_retry",		4,	U32_FT, Persistant_PT,
		1,							// "num_recoverable_media_errors_by_ecc",		4,	U32_FT, Persistant_PT,
		1,							// "num_recoverable_nonmedia_errors",			4,	U32_FT, Persistant_PT,
		1,							// "num_bytesprocessedtotal",					4,	U32_FT, Persistant_PT,
		1							// "num_unrecoverable_media_errors",			4,	U32_FT, Persistant_PT,
	};
	
	m_pcinewDiskStatusRecord = (char*)new(tPCI) char(sizeof(newDiskStatusRecord));
	memcpy( (char*)m_pcinewDiskStatusRecord,
			(char*)&newDiskStatusRecord,
			sizeof(newDiskStatusRecord)
		  ); 

	// Create a new InsertRow Object, Initialize it with our pramaeters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the DiskStatusTable.
	m_pInsertRow = new TSInsertRow;
	
	status = m_pInsertRow->Initialize(
		this,							// Ddm* ClientDdm
		DISK_STATUS_TABLE,				// prgbTableName
		m_pcinewDiskStatusRecord,			// prgbRowData
		sizeof(newDiskStatusRecord),	// cbRowData
		&m_RowID1,						// *pRowIDRet
		(pTSCallback_t)&Reply2,			// pTSCallback_t pCallback,
		this							// pContext
	);
	
	m_pInsertRow->Send();

	return status;
}


STATUS TestServices::Reply2(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	Tracef("TestServices::Reply2() status=%u\n",status);
	delete (m_pcinewDiskStatusRecord);						// this is associated with the previous send;
															// if handlereply routine changed, needs to move

	// Declare and initialize a Disk Status Record with default values. 
	DiskStatusRecord	newDiskStatusRecord = {
	// 	Default Value				FieldName									  Size   Type
	// 	Default Value				FieldName									  Size   Type
		0,0,0,						// "rowID.HiPart, rowID.LoPart",				8,	RowID_FT, Persistant_PT,
		1,							// "Version",									4,	U32_FT, Persistant_PT,
		sizeof(DiskStatusRecord),	// "Size",										4,	U32_FT, Persistant_PT,
		0,							// "Key",										4,	U32_FT, Persistant_PT,
		1000000,					// "RefreshRate",								4,	U32_FT, Persistant_PT,
		0,0,0,						// "ridSRCRecord.HiPart, ridSRCRecord.LoPart",	8,	ROWID_FT, Persistant_PT,
		2,							// "num_recoverable_media_errors_no_delay",		4,	U32_FT, Persistant_PT,
		2,							// "num_recoverable_media_errors_delay",		4,	U32_FT, Persistant_PT,
		2,							// "num_recoverable_media_errors_by_retry",		4,	U32_FT, Persistant_PT,
		2,							// "num_recoverable_media_errors_by_ecc",		4,	U32_FT, Persistant_PT,
		2,							// "num_recoverable_nonmedia_errors",			4,	U32_FT, Persistant_PT,
		2,							// "num_bytesprocessedtotal",					4,	U32_FT, Persistant_PT,
		2							// "num_unrecoverable_media_errors",			4,	U32_FT, Persistant_PT,
	};
	
	m_pcinewDiskStatusRecord2 = (char*)new(tPCI) char(sizeof(newDiskStatusRecord));
	memcpy( (char*)m_pcinewDiskStatusRecord,
			(char*)&newDiskStatusRecord,
			sizeof(newDiskStatusRecord)
		  ); 

	// Create a new InsertRow Object, Initialize it with our pramaeters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the DiskStatusTable.
	m_pInsertRow = new TSInsertRow;
	
	m_pInsertRow->Initialize(
		this,							// Ddm* ClientDdm
		DISK_STATUS_TABLE,				// prgbTableName
		m_pcinewDiskStatusRecord,			// prgbRowData
		sizeof(newDiskStatusRecord),	// cbRowData
		&m_RowID2,						// *pRowIDRet
		(pTSCallback_t)&Reply3,			// pTSCallback_t pCallback,
		this							// pContext
	);
	
	m_pInsertRow->Send();

	return status;
}


STATUS TestServices::Reply3(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	Tracef("TestServices::Reply3() status=%u\n",status);
 	delete (m_pcinewDiskStatusRecord2);


	// Alloc, init and send off a read row object for the new
	// Storage Roll Call record.
	TSReadRow*	m_pSRCReadRow = new  TSReadRow;
	
	status = m_pSRCReadRow->Initialize(	
		this,							// Ddm* ClientDdm
		DISK_STATUS_TABLE,				// Name of table to read.
		CT_PTS_RID_FIELD_NAME,			// String64 prgbKeyFieldName,
		&m_RowID1,						// void* pKeyFieldValue,
		sizeof(rowID),					// U32 cbKeyFieldValue,
		&m_DiskStatusRecord1,			// void* prgbRowDataRet,
		sizeof(m_DiskStatusRecord1),	// U32 cbRowDataRetMax,
		NULL,							// rowID* pRowIDRet = 0,
		(pTSCallback_t)&Reply4,			// pTSCallback_t pCallback,
		this
	);
	
	m_pSRCReadRow->Send();

	return status;
}


STATUS TestServices::Reply4(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	Tracef("TestServices::Reply4() status=%u\n",status);
	m_EnumCount = 0;

	// Alloc, init and send off a read row object for the new
	// DiskStatusTable record.
	
	// Allocate an Enumerate Table object for the DiskStatusTable.
	m_pEnumTable = new TSEnumTable;

	// Initialize the enumerate table operation.
	status = m_pEnumTable->Initialize( this,
							DISK_STATUS_TABLE,
							0,							// Starting row number.
							&m_DiskStatusTable,			// Returned data buffer.
							sizeof(m_DiskStatusTable),	// max size of returned data.
							NULL,						// pointer to # of returned bytes.
							(pTSCallback_t)&Reply5,		// pTSCallback_t pCallback,
							this
						  );

	// Initiate the enumerate table operation.
	m_pEnumTable->Send();

	return status;
}

STATUS TestServices::Reply5(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	Tracef("TestServices::Reply5() status=%u\n",status);


	// Alloc, init and send off a read row object for the new
	// Storage Roll Call record.
	TSReadRow*	m_pSRCReadRow = new  TSReadRow;
	
	status = m_pSRCReadRow->Initialize(	
		this,							// Ddm* ClientDdm
		DISK_STATUS_TABLE,				// Name of table to read.
		CT_PTS_RID_FIELD_NAME,			// String64 prgbKeyFieldName,
		&m_RowID1,						// void* pKeyFieldValue,
		sizeof(rowID),					// U32 cbKeyFieldValue,
		&m_DiskStatusRecord1,			// void* prgbRowDataRet,
		sizeof(m_DiskStatusRecord1),	// U32 cbRowDataRetMax,
		NULL,							// rowID* pRowIDRet = 0,
		(pTSCallback_t)&Reply6,			// pTSCallback_t pCallback,
		this
	);
	
	m_pSRCReadRow->Send();

	return status;
}



STATUS TestServices::Reply6(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	U32 num_rows_modified;

	Tracef("TestServices::Reply6() status=%u\n",status);

	m_DiskStatusRecord1.version = 2;
	m_DiskStatusRecord1.size = sizeof(DiskStatusRecord);
	m_DiskStatusRecord1.key = 5;
	m_DiskStatusRecord1.RefreshRate = 5;
	m_DiskStatusRecord1.num_recoverable_media_errors_no_delay = 5;
	m_DiskStatusRecord1.num_recoverable_media_errors_delay = 5;
	m_DiskStatusRecord1.num_recoverable_media_errors_by_retry = 5;
	m_DiskStatusRecord1.num_recoverable_media_errors_by_ecc = 5;
	m_DiskStatusRecord1.num_recoverable_nonmedia_errors = 5;
	m_DiskStatusRecord1.num_bytes_processed_total = 5;
	m_DiskStatusRecord1.num_unrecoverable_media_errors = 5;
		
	// Allocate an Enumerate Table object for the DiskStatusTable.
	m_pModifyRow = new TSModifyRow;

	// Initialize the modify row operation.
	status = m_pModifyRow->Initialize(	this,					// DdmServices pDdmServices,
								DISK_STATUS_TABLE,				// String64 rgbTableName,
								CT_PTS_RID_FIELD_NAME,			// String64 prgbKeyFieldName,
								&m_RowID1,						// void* pKeyFieldValue,
								sizeof(rowID),					// U32 cbKeyFieldValue,
								&m_DiskStatusRecord1,			// void* prgbRowData,
								sizeof(m_DiskStatusRecord1),	// U32 cbRowData,
								1,								// U32 cRowsToModify,
								&num_rows_modified,				// U32 *pcRowsModifiedRet
								&m_RowID1,						// rowID *pRowIDRet,
								sizeof (rowID),					// U32 cbMaxRowID,
								(pTSCallback_t)&Reply7,			// pTSCallback_t pCallback,
								this
							);

	// Initiate the enumerate table operation.
	m_pModifyRow->Send();

	return status;
}

STATUS TestServices::Reply7(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	Tracef("TestServices::Reply7() status=%u\n",status);

	// Allocate an Enumerate Table object for the DiskStatusTable.
	m_pReadTable = new TSReadTable;

	m_pTableRows = NULL;
	m_nTableRows = 0;

	// Initialize the modify row operation.
	status = m_pReadTable->Initialize(	this,							// DdmServices pDdmServices,
								DISK_STATUS_TABLE,				// String64 rgbTableName,
								&m_pTableRows,					// void* &ppTableDataRet, returned table data.
								&m_nTableRows,					// U32 *pcRowsRet,			// returned # of rows read
								(pTSCallback_t)&Reply8,			// pTSCallback_t pCallback,
								this
							);

	// Initiate the enumerate table operation.
	m_pReadTable->Send();
	
	return status;
}

STATUS TestServices::Reply8(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	Tracef("TestServices::Reply8() status=%u\n",status);
	m_EnumCount++;

	if (m_pTableRows)
		delete m_pTableRows;
	m_nTableRows = 0;
	
	// Alloc, init and send off a read row object for the new
	// DiskStatusTable record.
	
	// Allocate an Enumerate Table object for the DiskStatusTable.
	m_pEnumTable = new TSEnumTable;

	// Initialize the enumerate table operation.
	status = m_pEnumTable->Initialize( this,
							DISK_STATUS_TABLE,
							0,							// Starting row number.
							&m_DiskStatusTable,			// Returned data buffer.
							sizeof(m_DiskStatusTable),	// max size of returned data.
							&m_numBytesEnumed,			// pointer to # of returned bytes.
							(pTSCallback_t)&Reply9,		// pTSCallback_t pCallback,
							this
						  );

	// Initiate the enumerate table operation.
	m_pEnumTable->Send();

	return status;
}

STATUS TestServices::Reply9(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	Tracef("TestServices::Reply9() status=%u\n",status);

	if (m_pTableRows)
		delete m_pTableRows;
	m_nTableRows = 0;
	
	// Alloc, init and send off a read row object for the new
	// DiskStatusTable record.
	
	// Allocate an Enumerate Table object for the DiskStatusTable.
	m_pListen = new TSListen;

	// Initialize the m_cbModifiedRecord with the size of the DiskStatusRecord.
	// This is used to initialize the size of the associated sgl item since
	// true dynamic sgl items aren't supported yet.
	m_cbModifiedDSTRecord = sizeof(DiskStatusRecord);
	
	// Initialize all parameters necessary to Listen.
	status = m_pListen->Initialize(
		this,									// DdmServices* pDdmServices,
		ListenOnModifyAnyRowAnyField,			// U32 ListenType,
		DISK_STATUS_TABLE,						// String64 prgbTableName,
		NULL,									// String64 prgbRowKeyFieldName,
		NULL,									// void* prgbRowKeyFieldValue,
		0,										// U32 cbRowKeyFieldValue,
		NULL,									// String64 prgbFieldName,
		NULL,									// void* prgbFieldValue,
		0,										// U32 cbFieldValue,
		ReplyContinuous,						// U32 ReplyMode,
		&m_pDiskStatusTable,					// void** ppTableDataRet,
		&m_cbDiskStatusTable,					// U32* pcbTableDataRet,
		&m_ListenerID,							// U32* pListenerIDRet,
		&m_pListenReplyType,					// U32** ppListenTypeRet,
		&m_pModifiedDSTRecord,					// void** ppModifiedRecordRet,
		&m_cbModifiedDSTRecord,					// U32* pcbModifiedRecordRet,
		(pTSCallback_t)&ListenReply,			// pTSCallback_t pCallback,
		NULL									// void* pContext
	);

	// Initiate the listen operation.
	m_pListen->Send();

	// Now read a row to modify.

	// Alloc, init and send off a read row object for the new
	// Storage Roll Call record.
	TSReadRow*	m_pSRCReadRow = new  TSReadRow;
	
	status = m_pSRCReadRow->Initialize(	
		this,							// Ddm* ClientDdm
		DISK_STATUS_TABLE,				// Name of table to read.
		CT_PTS_RID_FIELD_NAME,			// String64 prgbKeyFieldName,
		&m_RowID1,						// void* pKeyFieldValue,
		sizeof(rowID),					// U32 cbKeyFieldValue,
		&m_DiskStatusRecord1,			// void* prgbRowDataRet,
		sizeof(m_DiskStatusRecord1),	// U32 cbRowDataRetMax,
		NULL,							// rowID* pRowIDRet = 0,
		(pTSCallback_t)&ReplyA,			// pTSCallback_t pCallback,
		this
	);
	
	m_pSRCReadRow->Send();

	// Initialize for ReplyA so he won't try to free as yet non-existant
	// TSModify Message.
	m_pModifyRow = NULL;

	return status;
}

STATUS TestServices::ReplyA(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	U32 num_rows_modified;
	
	Tracef("TestServices::ReplyA() status=%u\n",status);

	// Modify the record's fields.
	m_DiskStatusRecord1.version = 2;
	m_DiskStatusRecord1.size = sizeof(DiskStatusRecord);
	m_DiskStatusRecord1.key++;
	m_DiskStatusRecord1.RefreshRate++;
	m_DiskStatusRecord1.num_recoverable_media_errors_no_delay++;
	m_DiskStatusRecord1.num_recoverable_media_errors_delay++;
	m_DiskStatusRecord1.num_recoverable_media_errors_by_retry++;
	m_DiskStatusRecord1.num_recoverable_media_errors_by_ecc++;
	m_DiskStatusRecord1.num_recoverable_nonmedia_errors++;
	m_DiskStatusRecord1.num_bytes_processed_total++;
	m_DiskStatusRecord1.num_unrecoverable_media_errors++;
		
	// Allocate an Enumerate Table object for the DiskStatusTable.
	m_pModifyRow = new TSModifyRow;

	// Initialize the modify row operation.
	status = m_pModifyRow->Initialize(	this,					// DdmServices pDdmServices,
								DISK_STATUS_TABLE,				// String64 rgbTableName,
								CT_PTS_RID_FIELD_NAME,			// String64 prgbKeyFieldName,
								&m_RowID1,						// void* pKeyFieldValue,
								sizeof(rowID),					// U32 cbKeyFieldValue,
								&m_DiskStatusRecord1,			// void* prgbRowData,
								sizeof(m_DiskStatusRecord1),	// U32 cbRowData,
								1,								// U32 cRowsToModify,
								&num_rows_modified,				// U32 *pcRowsModifiedRet,
								&m_RowID1,						// rowID *pRowIDRet,
								sizeof (rowID),					// U32 cbMaxRowID,
								(pTSCallback_t)&ReplyA,			// pTSCallback_t pCallback,               *** Changed until I add a StopListen to keep the memory from running out.
								this																// Reply to B when StopListen is added    LWE  5/20/99
							);

	// Initiate the enumerate table operation.
	m_pModifyRow->Send();

	return status;
}

/****************************************************************************
**          ReplyB: Added this message handler to do Table row deletion                          **
**          Author: Lee Egger            Date: 5/13/99                                                         **
****************************************************************************/

STATUS TestServices::ReplyB(void *pClientContext, STATUS status)	
{
#pragma unused (pClientContext)

	Tracef("TestServices::ReplyB() status=%u\n",status);

	// Allocate an Enumerate Table object for the DiskStatusTable.
	m_pDeleteRow = new TSDeleteRow;

	// Initialize the enumerate table operation.
	status = m_pDeleteRow->Initialize( this,
							DISK_STATUS_TABLE,
							CT_PTS_RID_FIELD_NAME,		// String64 prgbKeyFieldName,
							&m_RowID1,					// void* pKeyFieldValue,
							sizeof(rowID),				// U32 cbKeyFieldValue,
							1,							// U32 cRowsToDelete,
							&m_RowsDel,					// rowID *pRowIDRet,
							(pTSCallback_t)&ReplyC,		// pTSCallback_t pCallback,
							this
						  );

	// Initiate the enumerate table operation.
	m_pDeleteRow->Send();

	return status;
}


/****************************************************************************
**          ReplyB: Added this message handler to do Table row deletion                          **
**          Note: This member function now "loops" back to the original table insert row     **
** 		function written by JFL. I did this to test for memory leaks, object handling,       **
** 		and other boundary/stress test areas.                                                               **
**          Author: Lee Egger            Date: 5/13/99                                                          **
****************************************************************************/


STATUS TestServices::ReplyC(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	Tracef("TestServices::ReplyC() status=%u\n",status);

	// Allocate an Enumerate Table object for the DiskStatusTable.
	m_pDeleteRow = new TSDeleteRow;

	// Initialize the enumerate table operation.
	status = m_pDeleteRow->Initialize( this,
							DISK_STATUS_TABLE,
							CT_PTS_RID_FIELD_NAME,		// String64 prgbKeyFieldName,
							&m_RowID2,					// void* pKeyFieldValue,
							sizeof(rowID),				// U32 cbKeyFieldValue,
							1,							// U32 cRowsToDelete,
							&m_RowsDel,					// rowID *pRowIDRet,
							(pTSCallback_t)&Reply1,		// pTSCallback_t pCallback,
							this
						  );

	// Initiate the enumerate table operation.
	m_pDeleteRow->Send();
	
	return status;
}


STATUS TestServices::ListenReply(void *pClientContext, STATUS status)
{
#pragma unused (pClientContext)

	//Tracef("TestServices::ListenReply() status=%u\n",status);

//	if (*m_pListenReplyType & ListenInitialReply)
//		Tracef("ListenReply #1");
//	else
//		Tracef("ListenReply #%u\n", m_pModifiedDSTRecord->key);
	
	return status;
}

