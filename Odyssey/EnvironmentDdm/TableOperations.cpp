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
// File: TableOperation.cpp
// 
// Description:
// 	This file implement the table operations helper methods.
// 
// $Log: /Gemini/Odyssey/EnvironmentDdm/TableOperations.cpp $
// 
// 2     12/13/99 1:34p Vnguyen
// Update for Environment Ddm.
// 
// 1     11/17/99 5:49p Hdo
// First check in
// 
/*************************************************************************/

#include "Trace_Index.h"
//#define TRACE_INDEX TRACE_ENVIRONMENT
#include "Odyssey_Trace.h"

#include "EnvironmentDdm.h"

// EVCStatusTableInitialize()
//
//
void	EnvironmentDdm::EVCStatusTableInitialize(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::EVCStatusTableInitialize);

	STATUS	status;

	m_pDefineTable = new TSDefineTable();

	status = m_pDefineTable->Initialize(
		this,
		EVC_STATUS_TABLE,
		EVCStatusRecord::FieldDefs(),
		EVCStatusRecord::FieldDefsSize(),
		20,									// num PTS rows
		FALSE,								// Table is not persistent
		(pTSCallback_t) &CreateTableReply,
		pMsg
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pDefineTable->Send();
	else
	{
		delete m_pDefineTable;
		// The operation failed, reply to the Initialize message
		Reply(pMsg, status);
	}
}

// CreateTableReply
//
//
STATUS	EnvironmentDdm::CreateTableReply(void *pContext, STATUS status)
{
	TRACE_ENTRY(EnvironmentDdm::CreateTableReply);

	if (status == ercOK)
	{
	TRACEF(TRACE_L8, "Create table for the first time.\n");
		// Table just got created, insert a new dummy row into the table
		status = EVCTable_InsertRow((Message*)pContext);
	}
	else if ( status == ercTableExists )
	{
	TRACEF(TRACE_L8, "Table exists.\n");
		// Table already exists, read in the old record
		status = ReadEVCRecord((Message*)pContext);
	}
	else
	TRACEF(TRACE_L8, ("Create table reply error code %d.\n", status));

	return status;
}

// EVCTable_InsertRow
// Fill in all the initial values for the EVCStatusRecord
//
STATUS	EnvironmentDdm::EVCTable_InsertRow(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::EVCTable_InsertRow);

	STATUS	status;

	// Create a new TSInsertRow object and send it to PTS service
	m_pInsertRow = new TSInsertRow();

	status = m_pInsertRow->Initialize(
		this,
		EVC_STATUS_TABLE,
		(void *)&m_EVCStatusRecord,
		sizeof(EVCStatusRecord),
		&m_rid,
		(pTSCallback_t) &Handle_InsertRow_Reply,
		pMsg
	);

	if ( status == ercOK )
		m_pInsertRow->Send();
	else
	{
		delete m_pInsertRow;
		// The operation failed, reply to the Initialize message
		Reply(pMsg, status);
	}

	return status;
}

// Handle_InsertRow_Reply
//
//
STATUS	EnvironmentDdm::Handle_InsertRow_Reply(void *pContext, STATUS status)
{
	TRACE_ENTRY(EnvironmentDdm::Handle_InsertRow_Reply);

	TRACEF(TRACE_L8, ("Insert row reply status = %X.\n", status));

	// Reply to the Initialize message that passed around as pContext
	Reply((Message *)pContext, status);

	return status;
}

// ReadEVCRecord
// Read the first entry in the table.  If it return NULL then insert
// a new row.
STATUS	EnvironmentDdm::ReadEVCRecord(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::ReadEVCRecord);

	STATUS status;

	// Allocate a new TSReadRow object and send it off to the PTS
	m_pReadRow = new TSReadRow();

	status = m_pReadRow->Initialize(
		this,
		EVC_STATUS_TABLE,
		"rid",
		&m_rid,					// m_rid point to the first entry
		sizeof(rowID),
		&m_EVCStatusRecord,
		sizeof(EVCStatusRecord),
		NULL,
		(pTSCallback_t) &Handle_ReadEVC_Reply,
		pMsg
	);

	if ( status == ercOK )
		m_pReadRow->Send();
	else
	{
		delete m_pReadRow;
		// The operation failed, reply to the Initialize message
		Reply(pMsg, status);
	}

	return status;
}

// Handle_ReadEVC_Reply
// 
//
STATUS	EnvironmentDdm::Handle_ReadEVC_Reply(void *pContext, STATUS status)
{
	TRACE_ENTRY(EnvironmentDdm::Handle_ReadEVC_Reply);
	TRACEF(TRACE_L8, ("ReadEVCRecord reply error code %d.\n", status));

	if ( status == ercOK )
		// Reply to the Initialize message that passed around as pContext
		// with the status
		Reply((Message *)pContext, status);
	else
		// If read return NULL, insert the dummy entry
		EVCTable_InsertRow((Message *)pContext);

	return status;
}

// EVCTable_ModifyRow
//
//
STATUS	EnvironmentDdm::EVCTable_ModifyRow()
{
	TRACE_ENTRY(EnvironmentDdm::EVCTable_ModifyRow);

	STATUS status;

	// Allocate a new TSModifyRow object and send it off to the PTS
	m_pModifyRow = new TSModifyRow();

	status = m_pModifyRow->Initialize(
		this,
		EVC_STATUS_TABLE,
		"rid",
		(void *)&m_rid,
		sizeof(rowID),
		(void *)&m_EVCStatusRecord,
		sizeof(EVCStatusRecord),
		1,
		NULL,
		NULL,
		sizeof(rowID),
		(pTSCallback_t) &Handle_ModifyRow_Reply,
		NULL
	);

	if ( status == ercOK )
		m_pModifyRow->Send();
	else
		delete m_pModifyRow;

	return status;
}

// Handle_ModifyRow_Reply
//
//
STATUS	EnvironmentDdm::Handle_ModifyRow_Reply(void *pContext, STATUS status)
{
	TRACE_ENTRY(EnvironmentDdm::Handle_ModifyRow_Reply);

	if ( status == ercOK )
		;
	else
	TRACEF(TRACE_L8, ("Modify row reply error code %d.\n", status));

	return status;
}


// EVCTable_UpdateKeyswitch
//
//
STATUS	EnvironmentDdm::EVCTable_UpdateKeyswitch(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::EVCTable_UpdateKeyswitch);

	STATUS status;

	// Allocate a new TSModifyRow object and send it off to the PTS
	m_pModifyField = new TSModifyField();

	status = m_pModifyField->Initialize(
		this,
		EVC_STATUS_TABLE,
		"rid",
		(void *)&m_rid,
		sizeof(rowID),
		CT_EVCST_KEY,
		(void *)&m_eKeyswitchPosition,
		sizeof(U32),
		1,									// cRowsToModify
		NULL,								// pcRowsModifiedRet
		NULL,								// pRowIDRet
		0,									// cbMaxRowID
		NULL,								// pCallback
		NULL								// pContext
	);

	if ( status == ercOK )
		m_pModifyField->Send();
	else
		delete m_pModifyField;

	return status;
}
