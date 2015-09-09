/*************************************************************************/
// Copyright (C) ConvergeNet Technologies, 1998 
//
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DdmPnP.cpp -- DDM
// 
// Description:
// 	. 
// 
// $Log: /Gemini/Odyssey/DdmPnP/DdmPnP.cpp $
// 
// 22    9/03/99 11:03a Hdo
// Add tUNCACHED to all 'new'
//
// Revision History:
// 
//		05/23/99 Huy Do: Fix bugs - a lot of bugs!
//		05/20/99 Huy Do: Add InsertRow and HandleInsertRow
//		05/12/99 Huy Do: modify HandleReplie(s); remove pciFieldDef
//		05/10/99 Huy Do: change from payload to SGL entries
//		04/20/99 Huy Do: add error checking for HandleReplie(s)
//		03/19/99 Huy Do: Created
//		03/30/99 Huy Do: CreateTable, GetTableDef, EnumTable
//		03/10/99 Huy Do: Add pciFieldDef
/*************************************************************************/

#define _DEBUG

#include "BuildSys.h"
#include "Trace_Index.h"
#define TRACE_INDEX TRACE_PNP
#include "Odyssey_Trace.h"

#include "DdmPnP.h"
#include "ErrorLog.h"
#include "SerialDDM.h"

CLASSNAME(DdmPnP, SINGLE);	// Class Link Name used by Buildsys.cpp

SERVELOCAL( DdmPnP, PNP_CREATE_TABLE);
SERVELOCAL( DdmPnP, PNP_GET_TABLE_DEF);
SERVELOCAL( DdmPnP, PNP_ENUM_TABLE  );
SERVELOCAL( DdmPnP, PNP_INSERT_ROW  );
SERVELOCAL( DdmPnP, PNP_DELETE_TABLE);
SERVELOCAL( DdmPnP, PNP_MODIFY_ROW  );
SERVELOCAL( DdmPnP, PNP_READ_ROW    );
SERVELOCAL( DdmPnP, PNP_DELETE_ROW  );
SERVELOCAL( DdmPnP, PNP_MODIFY_FIELD);
SERVELOCAL( DdmPnP, PNP_LISTEN		);
SERVELOCAL( DdmPnP, PNP_CONNECT		);


// DdmPnP -- Constructor ------------------------------------------DdmPnP-
//
DdmPnP::DdmPnP(DID did): Ddm(did)
{
	TRACE_ENTRY(DdmPnP::DdmPnP);

	m_pDefTbl= NULL;

	m_pGetTblDef = NULL;
	m_GetTableDefReplyPayload.pFieldDefsRet = NULL;
	m_GetTableDefReplyPayload.numFieldDefsRet = 0;
	m_GetTableDefReplyPayload.cbRowRet = 0;
	m_GetTableDefReplyPayload.cRowsRet = 0;
	m_GetTableDefReplyPayload.persistent = 0;

	m_pEnumTbl = NULL;

	m_pInsertRow = NULL;
	m_pDeleteTable = NULL;
	m_pReadRow = NULL;
	m_pDeleteRow = NULL;
	m_pModifyRow = NULL;
	m_pModifyField = NULL;
	m_pListen = NULL;
}
	

// Ctor -- Create ourselves ---------------------------------------DdmPnP-
//
Ddm *DdmPnP::Ctor(DID did)
{
	TRACE_ENTRY(DdmPnP::Ctor);

	return (new DdmPnP(did));
}

// Initialize -- Just what it says --------------------------------DdmPnP-
//
// Precondition : the SerialDDM is up
// Postcondition: DdmPnP is ready to serve
STATUS DdmPnP::Initialize(Message *pMsg)	// virtual
{ 
	TRACE_ENTRY(DdmPnP::Initialize);

	DispatchRequest(PNP_CREATE_TABLE , (RequestCallback) &CreateTable);
	DispatchRequest(PNP_GET_TABLE_DEF, (RequestCallback) &GetTableDef);
	DispatchRequest(PNP_ENUM_TABLE   , (RequestCallback) &EnumTable  );
	DispatchRequest(PNP_INSERT_ROW   , (RequestCallback) &InsertRow  );
	DispatchRequest(PNP_DELETE_TABLE , (RequestCallback) &DeleteTable);
	DispatchRequest(PNP_MODIFY_ROW   , (RequestCallback) &ModifyRow);
	DispatchRequest(PNP_READ_ROW     , (RequestCallback) &ReadRow);
	DispatchRequest(PNP_DELETE_ROW   , (RequestCallback) &DeleteRow);
	DispatchRequest(PNP_MODIFY_FIELD , (RequestCallback) &ModifyField);
	DispatchRequest(PNP_LISTEN       , (RequestCallback) &Listen);
	DispatchRequest(PNP_CONNECT      , (RequestCallback) &Connect);

	return Ddm::Initialize(pMsg);
}

// Enable -- Start-it-up ------------------------------------------DdmPnP-
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::Enable(Message *pMsg)	// virtual
{ 
	TRACE_ENTRY(DdmPnP::Enable);

	return Ddm::Enable(pMsg);
}

// Connect -- Start-it-up ------------------------------------------DdmPnP-
// Message handler for an PNP_CONNECT message
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::Connect(Message *pReqMsg)
{ 
	TRACE_ENTRY(DdmPnP::Connect);

	Reply(pReqMsg, ercOK);
	return ercOK;
}

//  ------------------------------------------DdmPnP-
// Message handler for an PNP_CREATE_TABLE message
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::CreateTable(Message *pReqMsg)
{
	TRACE_ENTRY(DdmPnP::CreateTable);
	CT_ASSERT( pReqMsg != NULL, CreateTable);

	m_pDefTbl = new(tZERO|tUNCACHED) TSDefineTable();
	CT_ASSERT( m_pDefTbl != NULL, CreateTable);
	U32 cbData;
	SP_PAYLOAD *pPayload;
	STATUS		status;

	pReqMsg->GetSgl(CREATE_TABLE_MSG_SGL, &pPayload, &cbData);
	CT_ASSERT( pPayload != NULL, CreateTable);

	// initialize the TSDefineTable object
	status = m_pDefTbl->Initialize(this,
					 pPayload->Data.ct.TableName,
					 pPayload->Data.ct.pFieldDefs,
					 pPayload->Data.ct.cbFieldDefs,
					 pPayload->Data.ct.cEntriesRsv,
					 pPayload->Data.ct.persistent,
					 (pTSCallback_t)&HandleCreateTable,
					 pReqMsg);

	// Send it to the Table service, speicfy the callback handler
	// pDefTbl will destroy itself inside Send()
	if( status == ercOK )
		m_pDefTbl->Send();
	else
		HandleCreateTable(pReqMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for the callback in CreateTable
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::HandleCreateTable(void *pContext, STATUS status)
{
	TRACE_ENTRY(DdmPnP::HandleCreateTable);
	CT_ASSERT( pContext != NULL, HandleCreateTable);

	Message* pInitMsg = (Message*)pContext;
	rowID *pReplyRowID;
	U32 cbReplyRowID;

	pInitMsg->GetSgl(CREATE_TABLE_REPLY_SGL,
					&pReplyRowID, &cbReplyRowID);

	if( status == ercOK )
		*pReplyRowID = m_pDefTbl->GetTableID();
	else
		if( status == ercTableExists )	// table exist! Get the tableID
			*pReplyRowID = m_pDefTbl->GetTableID();
		else
		{
			pReplyRowID->Table = 0xFF;
			pReplyRowID->LoPart = 0xFF;
			pReplyRowID->HiPart = 0xFFFF;

			delete m_pDefTbl;
		}

	Reply(pInitMsg, status);

	return status;
}

// ------------------------------------------DdmPnP-
// Message handler for an PNP_GET_TABLE_DEF message
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::GetTableDef(Message *pReqMsg)
{
	TRACE_ENTRY(DdmPnP::GetTableDef);
	CT_ASSERT( pReqMsg != NULL, GetTableDef);

	U32 cbData;
	SP_PAYLOAD *pPayload;
	STATUS		status;

	m_pGetTblDef = new(tZERO|tUNCACHED) TSGetTableDef();
	CT_ASSERT( m_pGetTblDef != NULL, GetTableDef);

	pReqMsg->GetSgl(GET_TABLE_DEF_MSG_SGL, &pPayload, &cbData);
	CT_ASSERT( pPayload != NULL, GetTableDef);

	m_GetTableDefReplyPayload.pFieldDefsRet = 
			new(tZERO|tUNCACHED) fieldDef[pPayload->Data.gt.FieldDefRetMax / sizeof(fieldDef)];
	CT_ASSERT( m_GetTableDefReplyPayload.pFieldDefsRet != NULL, GetTableDef);

	if( m_GetTableDefReplyPayload.pFieldDefsRet == NULL )
		HandleGetTableDef(pReqMsg, ercNoMoreHeap);

	// initialize the TSGetTableDef object
	status = m_pGetTblDef->Initialize(this,
					 pPayload->Data.gt.TableName,
					 m_GetTableDefReplyPayload.pFieldDefsRet,
					 pPayload->Data.gt.FieldDefRetMax,
					 &m_GetTableDefReplyPayload.cbFieldDefs,
					 &m_GetTableDefReplyPayload.cbRowRet,
					 &m_GetTableDefReplyPayload.cRowsRet,
					 &m_GetTableDefReplyPayload.numFieldDefsRet,
					 &m_GetTableDefReplyPayload.persistent,
					 (pTSCallback_t)&HandleGetTableDef,
					 pReqMsg);
	
	// Send it to the Table service, speicfy the callback handler
	// pGetTblDef will destroy itself inside Send()
	if( status == ercOK )
		m_pGetTblDef->Send();
	else
		HandleGetTableDef(pReqMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for the callback in GetTableDef
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::HandleGetTableDef(void *pContext, STATUS status)
{
	TRACE_ENTRY(DdmPnP::HandleGetTableDef);

	Message* pInitMsg = (Message*)pContext;
	GET_TABLE_DEF_REPLY_PAYLOAD *pGetDefReply;
	U32 cbReplyGetDef;

	pInitMsg->GetSgl(GET_TABLE_DEF_REPLY_SGL,
					&pGetDefReply, &cbReplyGetDef);
	if( status == ercOK )
		*pGetDefReply = m_GetTableDefReplyPayload;
	else
	{
		m_GetTableDefReplyPayload.pFieldDefsRet = NULL;
		m_GetTableDefReplyPayload.cbFieldDefs = 0;
		m_GetTableDefReplyPayload.numFieldDefsRet = 0;
		m_GetTableDefReplyPayload.cbRowRet = 0;
		m_GetTableDefReplyPayload.cRowsRet = 0;
		m_GetTableDefReplyPayload.persistent = 0;
		*pGetDefReply = m_GetTableDefReplyPayload;

		delete m_pGetTblDef;
	}

	Reply(pInitMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for an PNP_ENUM_TABLE message
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::EnumTable(Message *pReqMsg)
{
	TRACE_ENTRY(DdmPnP::EnumTable);
	CT_ASSERT( pReqMsg != NULL, EnumTable);

	U32 cbData;
	SP_PAYLOAD *pPayload;
	STATUS		status;

	m_pEnumTbl = new(tZERO|tUNCACHED) TSEnumTable();
	CT_ASSERT( m_pEnumTbl != NULL, EnumTable);

	pReqMsg->GetSgl(ENUM_TABLE_MSG_SGL, &pPayload, &cbData);
	CT_ASSERT( pPayload != NULL, EnumTable);

	m_EnumTableReplyPayload.pRowsDataRet = new(tZERO|tUNCACHED) char [pPayload->Data.et.cbDataRetMax];
	CT_ASSERT( m_EnumTableReplyPayload.pRowsDataRet != NULL, EnumTable);

	if( m_EnumTableReplyPayload.pRowsDataRet == NULL )
		HandleEnumTable(pReqMsg, ercNoMoreHeap);

	// initialize the TSEnumTable object
	status = m_pEnumTbl->Initialize(this,
					 pPayload->Data.et.TableName,
					 pPayload->Data.et.startRow,
					 m_EnumTableReplyPayload.pRowsDataRet,
					 pPayload->Data.et.cbDataRetMax,
					 &m_EnumTableReplyPayload.cbRowsDataRet,
					 (pTSCallback_t)&HandleEnumTable,
					 pReqMsg);

	// Send it to the Table service, speicfy the callback handler
	// m_pEnumTbl will destroy itself inside Send()
	if( status == ercOK )
		m_pEnumTbl->Send();
	else
		HandleEnumTable(pReqMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for the callback in EnumTable
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::HandleEnumTable(void *pContext, STATUS status)
{
	TRACE_ENTRY(DdmPnP::HandleEnumTable);

	Message* pInitMsg = (Message*)pContext;
	ENUM_TABLE_REPLY_PAYLOAD *pReplyEnumPayload;
	U32 cbReplyEnum;

	pInitMsg->GetSgl(ENUM_TABLE_MSG_REPLY_SGL,
					&pReplyEnumPayload, &cbReplyEnum);
	if( status == ercOK )
		*pReplyEnumPayload = m_EnumTableReplyPayload;
	else
	{
		m_EnumTableReplyPayload.pRowsDataRet = NULL;
		m_EnumTableReplyPayload.cbRowsDataRet = 0;
		*pReplyEnumPayload = m_EnumTableReplyPayload;

		delete m_pEnumTbl;
	}
	Reply(pInitMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for an PNP_ISERT_ROW message
//
// Precondition : 
// Postcondition:
STATUS DdmPnP::InsertRow(Message *pReqMsg)
{
	TRACE_ENTRY(DdmPnP::InsertRow);
	CT_ASSERT( pReqMsg != NULL, InsertRow);

	m_pInsertRow = new(tZERO|tUNCACHED) TSInsertRow();
	CT_ASSERT( m_pInsertRow != NULL, InsertRow);

	U32 cbData;
	SP_PAYLOAD	*pPayload;
	STATUS		status;

	pReqMsg->GetSgl(INSERT_ROW_MSG_SGL, &pPayload, &cbData);
	CT_ASSERT( pPayload != NULL, InsertRow);

	// initialize the TSEnumTable object
	status = m_pInsertRow->Initialize(this,
					 pPayload->Data.in.TableName,
					 pPayload->Data.in.pRowData,
					 pPayload->Data.in.cbRowData,
					 &m_ReplyRowID,
					 (pTSCallback_t)&HandleInsertRow,
					 pReqMsg);

	// Send it to the Table service, speicfy the callback handler
	// m_pInsertRow will destroy itself inside Send()
	if( status == ercOK )
		m_pInsertRow->Send();
	else
		HandleInsertRow(pReqMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for the callback in InsertRow
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::HandleInsertRow(void *pContext, STATUS status)
{
	TRACE_ENTRY(DdmPnP::HandleInsertRow);

	Message* pInitMsg = (Message*)pContext;
	rowID *pReplyRowID;
	U32 cbReplyRowID;

	pInitMsg->GetSgl(INSERT_ROW_REPLY_SGL,
					&pReplyRowID, &cbReplyRowID);
	if( status == ercOK )
		*pReplyRowID = m_ReplyRowID;
	else
	{
		pReplyRowID->Table = 0xFF;
		pReplyRowID->LoPart = 0xFF;
		pReplyRowID->HiPart = 0xFFFF;

		delete m_pInsertRow;
	}
	Reply(pInitMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for an PNP_DELETE_TABLE message
//
// Precondition : 
// Postcondition: pReqMsg will be delete
STATUS DdmPnP::DeleteTable(Message *pReqMsg)
{
	TRACE_ENTRY(DdmPnP::DeleteTable);
	CT_ASSERT( pReqMsg != NULL, DeleteTable);

	U32 cbData;
	SP_PAYLOAD *pPayload;
	STATUS		status;

	m_pDeleteTable = new(tZERO|tUNCACHED) TSDeleteTable();
	CT_ASSERT( m_pDeleteTable != NULL, DeleteTable);

	pReqMsg->GetSgl(DELETE_TABLE_MSG_SGL, &pPayload, &cbData);
	CT_ASSERT( pPayload != NULL, DeleteTable);

	// initialize the TSDeleteTable object
	status = m_pDeleteTable->Initialize(this,
					 pPayload->Data.et.TableName,
					 m_ReplyRowID,
					 (pTSCallback_t)&HandleDeleteTable,
					 pReqMsg);

	// Send it to the Table service, speicfy the callback handler
	// m_pDeleteTable will destroy itself inside Send()
	if( status == ercOK )
		m_pDeleteTable->Send();
	else
		HandleDeleteTable(pReqMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for the callback in DeleteTable
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::HandleDeleteTable(void *pContext, STATUS status)
{
	TRACE_ENTRY(DdmPnP::HandleDeleteTable);

	Message* pInitMsg = (Message*)pContext;

	if( status != ercOK )
		delete m_pDeleteTable;
	Reply(pInitMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for an PNP_READ_ROW message
//
// Precondition : 
// Postcondition:
STATUS DdmPnP::ReadRow(Message *pReqMsg)
{
	TRACE_ENTRY(DdmPnP::ReadRow);
	CT_ASSERT( pReqMsg != NULL, ReadRow);

	U32 cbData;
	SP_PAYLOAD *pPayload;
	STATUS		status;

	m_pReadRow = new(tZERO|tUNCACHED) TSReadRow();
	CT_ASSERT( m_pReadRow != NULL, ReadRow);

	pReqMsg->GetSgl(READ_ROW_MSG_SGL, &pPayload, &cbData);
	CT_ASSERT( pPayload != NULL, ReadRow);

	m_ReadRowReplyPayload.pRowDataRet = new(tZERO|tUNCACHED) char [pPayload->Data.rr.cbRowRetMax];
	CT_ASSERT( m_ReadRowReplyPayload.pRowDataRet != NULL, ReadRow);

	if( m_ReadRowReplyPayload.pRowDataRet == NULL )
		HandleReadRow(pReqMsg, ercNoMoreHeap);

	// initialize the TSEnumTable object
	status = m_pReadRow->Initialize(this,
					 pPayload->Data.rr.TableName,
					 pPayload->Data.rr.KeyFieldName,
					 pPayload->Data.rr.pKeyFieldValue,
					 pPayload->Data.rr.cbKeyFieldValue,
					 m_ReadRowReplyPayload.pRowDataRet,
					 pPayload->Data.rr.cbRowRetMax,
					 &m_ReadRowReplyPayload.numRowsRet,
					 (pTSCallback_t)&HandleReadRow,
					 pReqMsg);

	// Send it to the Table service, speicfy the callback handler
	// m_pReadRow will destroy itself inside Send()
	if( status == ercOK )
		m_pReadRow->Send();
	else
		HandleReadRow(pReqMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for the callback in ReadRow
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::HandleReadRow(void *pContext, STATUS status)
{
	TRACE_ENTRY(DdmPnP::HandleReadRow);

	Message* pInitMsg = (Message*)pContext;
	READ_ROW_REPLY_PAYLOAD *pReplyReadRowPayload;
	U32 cbReplyReadRow;

	pInitMsg->GetSgl(READ_ROW_REPLY_SGL,
					&pReplyReadRowPayload, &cbReplyReadRow);

	if( status == ercOK )
		*pReplyReadRowPayload = m_ReadRowReplyPayload;
	else
	{
		m_ReadRowReplyPayload.pRowDataRet = NULL;
		m_ReadRowReplyPayload.cbRowDataRet = 0;
		m_ReadRowReplyPayload.numRowsRet = 0;
		*pReplyReadRowPayload = m_ReadRowReplyPayload;

		delete m_pReadRow;
	}
	Reply(pInitMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for an PNP_DELETE_ROW message
//
// Precondition : 
// Postcondition:
STATUS DdmPnP::DeleteRow(Message *pReqMsg)
{
	TRACE_ENTRY(DdmPnP::DeleteRow);
	CT_ASSERT( pReqMsg != NULL, DeleteRow);

	U32 cbData;
	SP_PAYLOAD *pPayload;
	STATUS		status;

	m_pDeleteRow = new(tZERO|tUNCACHED) TSDeleteRow();
	CT_ASSERT( m_pDeleteRow != NULL, DeleteRow);

	pReqMsg->GetSgl(DELETE_ROW_MSG_SGL, &pPayload, &cbData);
	CT_ASSERT( pPayload != NULL, DeleteRow);

	// initialize the TSEnumTable object
	status = m_pDeleteRow->Initialize(this,
					 pPayload->Data.dr.TableName,
					 pPayload->Data.dr.KeyFieldName,
					 pPayload->Data.dr.pKeyFieldValue,
					 pPayload->Data.dr.cbKeyFieldValue,
					 pPayload->Data.dr.cRowsDelete,
					 &m_cRowsDeleted,
					 (pTSCallback_t)&HandleDeleteRow,
					 pReqMsg);

	// Send it to the Table service, speicfy the callback handler
	// m_pDeleteRow will destroy itself inside Send()
	if( status == ercOK )
		m_pDeleteRow->Send();
	else
		HandleDeleteRow(pReqMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for the callback in DeleteRow
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::HandleDeleteRow(void *pContext, STATUS status)
{
	TRACE_ENTRY(DdmPnP::HandleDeleteRow);

	Message* pInitMsg = (Message*)pContext;
	U32 *pcRowsDel;
	U32 cbReplyRowID;

	pInitMsg->GetSgl(DELETE_ROW_REPLY_SGL,
					&pcRowsDel, &cbReplyRowID);
	if( status == ercOK )
		*pcRowsDel = m_cRowsDeleted;
	else
	{
		*pcRowsDel = 0;

		delete m_pDeleteRow;
	}
	Reply(pInitMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for an PNP_MODIFY_ROW message
//
// Precondition : 
// Postcondition:
STATUS DdmPnP::ModifyRow(Message *pReqMsg)
{
	TRACE_ENTRY(DdmPnP::ModifyRow);
	CT_ASSERT( pReqMsg != NULL, ModifyRow);

	U32 cbData;
	SP_PAYLOAD *pPayload;
	STATUS		status;

	m_pModifyRow = new(tZERO|tUNCACHED) TSModifyRow();
	CT_ASSERT( m_pModifyRow != NULL, ModifyRow);

	pReqMsg->GetSgl(MODIFY_ROW_MSG_SGL, &pPayload, &cbData);
	CT_ASSERT( pPayload != NULL, ModifyRow);

	// initialize the TSModifyRow object
	status = m_pModifyRow->Initialize(this,
					 pPayload->Data.mr.TableName,
					 pPayload->Data.mr.KeyFieldName,
					 pPayload->Data.mr.pKeyFieldValue,
					 pPayload->Data.mr.cbKeyFieldValue,
					 pPayload->Data.mr.pRowData,
					 pPayload->Data.mr.cbRowData,
					 pPayload->Data.mr.cRowsModify,
					 &m_cRowsModified,
					 m_pReturnedRowID,
					 sizeof(rowID)*pPayload->Data.mr.cRowsModify,
					 (pTSCallback_t)&HandleModifyRow,
					 pReqMsg);

	// Send it to the Table service, speicfy the callback handler
	// m_pModifyRow will destroy itself inside Send()
	if( status == ercOK )
		m_pModifyRow->Send();
	else
		HandleModifyRow(pReqMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for the callback in ModifyRow
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::HandleModifyRow(void *pContext, STATUS status)
{
	TRACE_ENTRY(DdmPnP::HandleModifyRow);

	Message* pInitMsg = (Message*)pContext;
	//rowID *pReplyRowID;
	U32		*pcRowModified;
	U32 	cbReply;

	pInitMsg->GetSgl(MODIFY_ROW_REPLY_SGL,
					&pcRowModified, &cbReply);
	if( status == ercOK )
		//*pReplyRowID = m_ReplyRowID;
		*pcRowModified = m_cRowsModified;
	else
	{
		*pcRowModified = 0;
		//pReplyRowID->Table = 0xFF;
		//pReplyRowID->LoPart = 0xFF;
		//pReplyRowID->HiPart = 0xFFFF;

		delete m_pModifyRow;
	}
	Reply(pInitMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for an PNP_MODIFY_FIELD message
//
// Precondition : 
// Postcondition:
STATUS DdmPnP::ModifyField(Message *pReqMsg)
{
	TRACE_ENTRY(DdmPnP::ModifyField);
	CT_ASSERT( pReqMsg != NULL, ModifyField);

	U32 cbData;
	SP_PAYLOAD *pPayload;
	STATUS		status;

	m_pModifyField = new(tZERO|tUNCACHED) TSModifyField();
	CT_ASSERT( m_pModifyField != NULL, DeleteRow);

	pReqMsg->GetSgl(MODIFY_FIELD_MSG_SGL, &pPayload, &cbData);
	CT_ASSERT( pPayload != NULL, ModifyField);

	// initialize the TSModifyField object
	status = m_pModifyField->Initialize(this,
					 pPayload->Data.mf.TableName,
					 pPayload->Data.mf.KeyFieldName,
					 pPayload->Data.mf.pKeyFieldValue,
					 pPayload->Data.mf.cbKeyFieldValue,
					 pPayload->Data.mf.FieldName,
					 pPayload->Data.mf.pFieldValue,
					 pPayload->Data.mf.cbFieldValue,
					 pPayload->Data.mf.cRowsModify,
					 &m_cRowsModified,
					 m_pReturnedRowID,
					 sizeof(rowID)*pPayload->Data.mf.cRowsModify,
					 (pTSCallback_t)&HandleModifyField,
					 pReqMsg);

	// Send it to the Table service, speicfy the callback handler
	// m_pModifyField will destroy itself inside Send()
	if( status == ercOK )
		m_pModifyField->Send();
	else
		HandleModifyField(pReqMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for the callback in ModifyField
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::HandleModifyField(void *pContext, STATUS status)
{
	TRACE_ENTRY(DdmPnP::HandleModifyField);

	Message* pInitMsg = (Message*)pContext;
	//rowID *pReplyRowID;
	U32	*pcRowsModified;
	U32 cbReply;

	pInitMsg->GetSgl(MODIFY_FIELD_REPLY_SGL,
					&pcRowsModified, &cbReply);
	if( status == ercOK )
		//*pReplyRowID = m_ReplyRowID;
		*pcRowsModified = m_cRowsModified;
	else
	{
		*pcRowsModified = 0;
		//pReplyRowID->Table = 0xFF;
		//pReplyRowID->LoPart = 0xFF;
		//pReplyRowID->HiPart = 0xFFFF;

		delete m_pModifyField;
	}
	Reply(pInitMsg, status);

	return status;
}

//  ------------------------------------------DdmPnP-
// Message handler for an PNP_LISTEN message
//
// Precondition : 
// Postcondition:
STATUS DdmPnP::Listen(Message *pReqMsg)
{
	TRACE_ENTRY(DdmPnP::Listen);

	CT_ASSERT( pReqMsg != NULL, Listen);
	U32 cbData;
	SP_PAYLOAD *pPayload;
	STATUS		status;

	m_pListen = new(tZERO|tUNCACHED) TSListen();
	CT_ASSERT( m_pListen != NULL, Listen);

	pReqMsg->GetSgl(LISTEN_MSG_SGL, &pPayload, &cbData);
	CT_ASSERT( pPayload != NULL, Listen);

	status = m_pListen->Initialize(this,
							pPayload->Data.lt.ListenType,
							pPayload->Data.lt.TableName,
							pPayload->Data.lt.RowKeyFieldName,
							pPayload->Data.lt.pRowKeyFieldValue,
							pPayload->Data.lt.cbRowKeyFieldValue,
							pPayload->Data.lt.FieldName,
							pPayload->Data.lt.pFieldValue,
							pPayload->Data.lt.cbFieldValue,
							pPayload->Data.lt.ReplyMode,
							NULL,//ppTableDataRet		All NULL for now
							NULL,//pcbTableDataRet
							NULL,//pListenerIDRet
							NULL,//ppListenTypeRet
							NULL,//ppModifiedRecordRet
							NULL,//pcbModifiedRecordRet
							(pTSCallback_t)&HandleListen,
							pReqMsg);
	// Send it to the Table service, speicfy the callback handler
	// m_pListen will destroy itself inside Send()
	if( status == ercOK )
		m_pListen->Send();
	else
		HandleListen(pReqMsg, status);
	return ercOK;
}

//  ------------------------------------------DdmPnP-
// Message handler for the callback in Listen
//
// Precondition : 
// Postcondition: 
STATUS DdmPnP::HandleListen(void *pContext, STATUS status)
{
	TRACE_ENTRY(DdmPnP::HandleListen);

	Message* pInitMsg = (Message*)pContext;
	if( status == ercOK )
	{
	}
	else
	{
		delete m_pListen;
	}
	return status;
}

// End of file