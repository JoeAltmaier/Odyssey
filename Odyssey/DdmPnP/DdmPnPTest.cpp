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
// File: DdmPnPTest.cpp
// 
// Description:
//    This file contains a test DDM used to exercise the PnPDDM.
// 
//     3/19/99 7:37p HDo: Create.
//
/*************************************************************************/


#define _TRACEF
#include "BuildSys.h"
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
//#include  "stdio.h"
#include  "DdmPnPTest.h"

#include "PnPMsg.h"

CLASSNAME(DdmPnPTest, SINGLE);  // Class Link Name used by Buildsys.cpp


// Ctor -- Create ourselves ---------------------------------DdmPnPTest-
//
Ddm *DdmPnPTest::Ctor (DID did)
{
   return (new DdmPnPTest(did));
}  /* end of DdmPnPTest::Ctor */


// DdmPnPTest -- Constructor --------------------------------DdmPnPTest-
//
DdmPnPTest::DdmPnPTest (DID did) : Ddm(did)
{
	TRACEF(TRACE_L8, ("DdmPnPTest::DdmPnPTest()\n"));
	SetConfigAddress(&config,sizeof(config));
}  /* end of DdmPnPTest::DdmPnPTest */


// Initialize --  -------------------------------------DdmPnPTest-
//
STATUS DdmPnPTest::Initialize(Message *pMsg)
{
	TRACEF(TRACE_L8, ("DdmPnPTest::Initialize()\n"));

	Reply(pMsg, OK);
	return OK;
}  /* end of DdmPnPTest::Initialize */

// Enable -- Start-it-up -------------------------------------DdmPnPTest-
//
STATUS DdmPnPTest::Enable(Message *pMsg)
{
	TRACEF(TRACE_L8, ("DdmPnPTest::Enable()\n"));

	Reply(pMsg, OK);
	return OK;
}  /* end of DdmPnPTest::Enable */

#if 0
// DdmPnPTest::TestCreateTable()
// Called by Enable().  It sends the first message to the PnPDdm
// Test CreateTable command.
//
STATUS DdmPnPTest::TestCreateTable(void)
{
	Message* pMsg;
	SP_PAYLOAD *myspPayload = new SP_PAYLOAD;

	// This is the code to create the DiskStattus Table.
	fieldDef	DiskStatusTable_FieldDefs[] = {
		// FieldName							  Size   Type
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

	fieldDef* pciDiskStatusTable_FieldDefs = (fieldDef*)new(tPCI)char(sizeof(DiskStatusTable_FieldDefs));
	memcpy( (char*)pciDiskStatusTable_FieldDefs,
			(char*)DiskStatusTable_FieldDefs,
			sizeof(DiskStatusTable_FieldDefs) );
	myspPayload->cmd = CREATE_TABLE;

	// SGL
	myspPayload->chID = 0xDEADDEAD;
	strcpy(myspPayload->Data.ct.TableName, "DISK_STATUS_TABLE");
	myspPayload->Data.ct.pFieldDefs = pciDiskStatusTable_FieldDefs;
	myspPayload->Data.ct.cbFieldDefs = sizeof(DiskStatusTable_FieldDefs);
	myspPayload->Data.ct.cEntriesRsv = 10;
	//myspPayload->Data.ct.persistent = FALSE;
	myspPayload->Data.ct.cRow = 1;

	myspPayload->cbData = sizeof(*myspPayload) + sizeof(DiskStatusTable_FieldDefs);

	pMsg = new Message(PNP_CREATE_TABLE);
	pMsg->AddSgl(CREATE_TABLE_MSG_SGL,
				myspPayload, myspPayload->cbData, SGL_REPLY);
	return Send(pMsg);
}

//
// DdmPnPTest::TestInsertRow
//
STATUS DdmPnPTest::TestInsertRow(void)
{
	Message* pMsg;
	SP_PAYLOAD *myspPayload = new SP_PAYLOAD;

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

	char *pcinewDiskStatusRecord = (char *) new(tPCI) char(sizeof(newDiskStatusRecord));
	memcpy( (char *)pcinewDiskStatusRecord,
			(char *)&newDiskStatusRecord,
			sizeof(newDiskStatusRecord));

	myspPayload->cmd = INSERT_ROW;

	// SGL
	myspPayload->chID = 0xDEADDEAD;
	strcpy(myspPayload->Data.in.TableName, "DISK_STATUS_TABLE");
	myspPayload->Data.in.pRowData = pcinewDiskStatusRecord;
	myspPayload->Data.in.cbRowData = sizeof(newDiskStatusRecord);

	myspPayload->cbData = sizeof(myspPayload) + sizeof(newDiskStatusRecord);

	pMsg = new Message(PNP_INSERT_ROW);
	pMsg->AddSgl(INSERT_ROW_MSG_SGL,
				myspPayload, myspPayload->cbData);
	return Send(pMsg);
}

//
// DdmPnPTest::TestGetTableDef
//
STATUS DdmPnPTest::TestGetTableDef(void)
{
	Message* pMsg;
	SP_PAYLOAD *myspPayload = new SP_PAYLOAD;

	myspPayload->cmd = GET_TABLE_DEF;

	// Payload
	myspPayload->chID = 0xDEADDEAD;
	strcpy(myspPayload->Data.gt.TableName, "DISK_STATUS_TABLE");

	myspPayload->cbData = sizeof(*myspPayload);

	pMsg = new Message(PNP_GET_TABLE_DEF);
	pMsg->AddSgl(GET_TABLE_DEF_MSG_SGL,
				myspPayload, myspPayload->cbData);
	return Send(pMsg);
}

//
// DdmPnPTest::TestEnumTable
//
STATUS DdmPnPTest::TestEnumTable(void)
{
	Message* pMsg;
	SP_PAYLOAD *myspPayload = new SP_PAYLOAD;

	myspPayload->cmd = ENUM_TABLE;

	// Payload
	myspPayload->chID = 0xDEADDEAD;
	strcpy(myspPayload->Data.et.TableName, "DISK_STATUS_TABLE");
	myspPayload->Data.et.startRow = 0;
	myspPayload->Data.et.cbDataRetMax = sizeof(SP_PAYLOAD);

	myspPayload->cbData = sizeof(*myspPayload);

	pMsg = new Message(PNP_ENUM_TABLE);
	pMsg->AddSgl(ENUM_TABLE_MSG_SGL,
				myspPayload, myspPayload->cbData);
	return Send(pMsg);
}
#endif