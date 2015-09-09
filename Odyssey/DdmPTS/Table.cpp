/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This file is the definition of an object interface to the
// DefineTable, GetTableDef, and EnumerateTable API of the
// Table Service.
// 
// $Log: /Gemini/Odyssey/DdmPTS/Table.cpp $
// 
// 37    10/12/99 10:02a Jlane
// Remove unused route by slot code.  Use strlen()+1for name SGLs
// throughout.
// 
// 36    8/30/99 12:17p Jlane
// Make fielddef param const to avoid warning.
// 
// 35    8/30/99 9:21a Sgavarre
// clear getsgl buffer sizes
// 
// 34    8/28/99 5:39p Sgavarre
// add dynamic reply buffers for returned data;  no change to client
// interface
// 
// 33    7/22/99 3:11p Hdo
// 
// 32    7/21/99 7:02p Hdo
// Change the return type of Send() from STATUS to void
// Call the user callback routine when error
// Add error checking
// 
// 31    7/14/99 2:17p Sgavarre
// Add parameter to GetTableDef that specifies the number of field per row
// 
// 30    7/12/99 2:46p Sgavarre
// Update DeleteTable
// 
// 29    7/01/99 7:03p Hdo
// Add TSDeleteTable class
// 
// 28    6/08/99 3:45p Sgavarre
// remove reply payloads
// 
// 27    5/24/99 8:28p Hdo
// Add TSGetTableDef::GetReturnedTableDef()
// 
// 26    5/25/99 2:47a Jhatwich
// fixed the reply callbacks... again
// 
// 25    5/12/99 5:13p Hdo
// Add TSDefineTable::GetTableID()
// Intialize pointers to NULL after delete for pMsg
// 
// 24    5/11/99 10:55a Jhatwich
// win32
// 
// 23    4/15/99 3:22p Jlane
// Delete pMsg in all reply handlers.
// 
// 22    4/07/99 5:09p Ewedel
// Changed to use new payload size argument to Message() constructor.
// 
// 21    4/07/99 12:25p Ewedel
// Added non-obvious cast needed to remove compiler warning.
// 
// 20    4/06/99 11:47a Jlane
// Added const to Define Table params to avoid warnings.
// 
// 19    4/05/99 2:36p Jlane
// Fix erroroneous Send in EnumerateTable.
// 
// 18    4/03/99 6:32p Jlane
// Multiple changes to interface, messaage payloads and message handlers
// to avoid having to add reply payloads as this destroys SGL items.  We
// do this by allocating space for reply data in the msg payload.
// 
// 17    4/02/99 1:52p Jlane
// #if'd out workaround code to send messages to slots since interslot
// route by function wasn't working.
// 
// 16    3/27/99 1:49p Jlane
// Changed all calls to Send() to use the send to slot version if we're
// not on the HBC.  This works around the OS limitation of not corrrectly
// handling route by function code between boards.  This should be
// temporary but should also work indefinitely.
// 
// 15    3/22/99 1:37p Hdo
// Table: Merge w/ EW version and add comments
// 
// 03/22/99	HD  Update the PTS document and clean up
//
// 14    3/19/99 7:07p Ewedel
// Fixed a weirdness in TSEnumTable::HandleEnumTableReply() where the
// count of bytes returned by PTS was not being reported to the caller.
// There is a separate assign (still there) to m_cbDataRetMax, which might
// be wrong.
// Also added vss Log: keyword.
//
// 02/14/99 JFL	Don't delete &pMsg (should be pMsg) in HandleEnumTableReply.
// 02/01/99	JFL	Merged with Sherri's TableMsgs.h Changed the field named
//				pGetTableDefReplyPayload->cFieldDefsRet to pGetTableDefReplyPayload->cbFieldDefsRet
//				and also changed the TSGetTableDef field name for the same value accordingly. 
// 01/21/99	JFL	In TSEnumTable::Initialize: starting row num shouldn't be a reference.
// 01/18/99	JFL	Misc. cleanup & modified to conform to the new TableMsgs.h format.
// 12/28/98 HD	Added the pContext value to all classes
//				Changed parms allowing to pass NULL pointer
// 12/11/98 HD  Converted pointers variables to reference variables
// 11/23/98	HD	Modified TSDefineTable, TSGetTableDef, & TSEnumTable becoming
//				objects with callback facility.
// 11/17/98	JFL	Created.
// 
/*************************************************************************/

#include <cstring>
#include <stdio.h>

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_PTS
#include "Odyssey_Trace.h"

#include "Table.h"
#include "Ddm.h"
	

/**********************************************************************/
// TSDefineTable Class Interface - Create a new table.
//
/**********************************************************************/
// Initialize - Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	rgbTableName	- Null terminated name of the table to create.
//	prgFieldDefs	- Pointer to FieldDef array that defines the fields
//	cbrgFieldDefs	- size of the above rgFieldDefs array in bytes.
//	cEntriesRsv		- Number of table entries to create initially.
//	PersistFlags	- pointer to returned flag indicating if table is persistant.
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**********************************************************************/
STATUS TSDefineTable::Initialize (	DdmServices*	pDdmServices,
									String64		rgbTableName,
									const fieldDef*	prgFieldDefs,
									U32				cbrgFieldDefs,
									U32				cEntriesRsv,
									U32				PersistFlags,
									pTSCallback_t	pCallback,
									void*			pContext )
{
	TRACE_ENTRY(TSDefineTable::Initialize);

	// Initialize our base class.
	if ((pDdmServices == NULL) || (rgbTableName == NULL) || (prgFieldDefs == NULL) ||
		(cbrgFieldDefs == 0))
		return ercBadParameter;

	SetParentDdm(pDdmServices);

	// Stash parameters in member variables.
	m_pDdmServices	= pDdmServices;
	strcpy(m_rgbTableName, (char *)rgbTableName);
	m_prgFieldDefs	= prgFieldDefs;
	m_cbrgFieldDefs	= cbrgFieldDefs;

	m_cEntriesRsv	= cEntriesRsv;
	m_PersistFlags	= PersistFlags;
	m_pCallback		= pCallback;	
	m_pContext		= pContext;

	return OS_DETAIL_STATUS_SUCCESS;
}

/**********************************************************************/
// Send - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (HandleDefineTableReply) to unpack the reply.
//
// Parameter: None
/**********************************************************************/
void TSDefineTable::Send()
{
	TRACE_ENTRY(TSDefineTable::Send);

	// This is the message payload structure.
	DefineTablePayload		myDefineTablePayload;
	STATUS					status;
	Message					*pMsg;
	
	// Initialize our payload structure from parameters set in Initialize.
	myDefineTablePayload.cEntriesRsv = m_cEntriesRsv;
	myDefineTablePayload.persistFlags = m_PersistFlags;
	
	// Allocate a new private (define table) message to send to the table service.
	pMsg = new Message(TS_FUNCTION_DEFINE_TABLE, sizeof(DefineTablePayload));
	
	// Add payload structure to the message.
	pMsg->AddPayload (&myDefineTablePayload, sizeof(myDefineTablePayload));
	
	// Add the table name to the message as a scatter gather list item.
	pMsg->AddSgl (DEFINE_TABLE_MSG_TABLENAME_SGI,
				  m_rgbTableName, 
				  strlen(m_rgbTableName)+1,
				  SGL_SEND);
	
	// Add the return buffer to the message as a scatter gather list item.
	pMsg->AddSgl (DEFINE_TABLE_MSG_FIELDDEFS_SGI,
				  (void*)m_prgFieldDefs, 
				  m_cbrgFieldDefs,
				  SGL_SEND );

	// Send the message to the Table Service.  Pass our object pointer as pContext
	// and specify our own callback handler to be called upon reply.
	status = DdmServices::Send(pMsg, NULL, REPLYCALLBACK(TSDefineTable, HandleDefineTableReply));
	
	// If the send failed delete the message and object before returning.
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSDefineTable::Send status=%d\n", status));
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		delete pMsg;
		delete this;
	}
}

/**********************************************************************/
// HandleDefineTableReply - This is the function specified as the 
// callback in the Send method.  It calls the user's callback and upon
// return, deletes the message and the object. 
//
// Parameters:
//	pMsg	- The replied message
/**********************************************************************/
STATUS TSDefineTable::HandleDefineTableReply( Message* pMsg )
{
	TRACE_ENTRY(TSDefineTable::HandleDefineTableReply);

	STATUS	status = pMsg->DetailedStatusCode;
	DefineTablePayload *pPayload = (DefineTablePayload *)pMsg->GetPPayload();

	// Save the returned rowID
	m_rowIDRet = pPayload->tableIdRet;

	// Call the user's callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );
	
	// Delete our message delete ourselves.
	delete pMsg;
	delete this;
	
	return status;
}

/**********************************************************************/
// GetTableID - Return the rowID of the newly define table 
//
// Parameters: None
// Return:	rowID
/**********************************************************************/
rowID TSDefineTable::GetTableID()
{
	return m_rowIDRet;
}


/**********************************************************************/
// TSGetTableDef - Get the definition data of an existing table
//
/**********************************************************************/
// Initialize -	Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	rgbTableName	- Null terminated name of the table.
//	prgFieldDefsRet	- pointer to buffer where the array of fieldDef is returned
//  cbrgFieldDefsRetMax - Size of prgFieldDefsRet buffer.
//	pcbFieldDefsRet	- pointer to actual number of bytes returned in FieldDef array.
//	pcbRowRet		- pointer to returned number of bytes per row.
//	pcRowsRet		- pointer to returned count of rows in table
//	pcFieldsRet		- pointer to returned count of fields in a row
//	pPersistFlagsRet- pointer to returned flag indicating if table is persistant.
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**********************************************************************/
STATUS TSGetTableDef::Initialize(	DdmServices		*pDdmServices,
									String64		rgbTableName,
									fieldDef		*prgFieldDefsRet,
									U32				cbrgFieldDefsMax,
									U32				*pcbFieldDefsRet,
									U32				*pcbRowRet,
									U32				*pcRowsRet,
									U32				*pcFieldsRet,
									U32				*pPersistFlagsRet,
									pTSCallback_t	pCallback,
									void			*pContext
								 )
{
	TRACE_ENTRY(TSGetTableDef::Initialize);

	// Initialize our base class.
	if ((pDdmServices == NULL) || (rgbTableName == NULL))
		return ercBadParameter;

	SetParentDdm(pDdmServices);
	
	// Stash parameters in member variables.
	m_pDdmServices			= pDdmServices;
	strcpy(m_rgbTableName, rgbTableName);

	m_prgFieldDefsRet	= prgFieldDefsRet;
	m_cbrgFieldDefsMax	= cbrgFieldDefsMax;
	m_pcbFieldDefsRet	= pcbFieldDefsRet;
	m_pcbRowRet			= pcbRowRet;
	m_pcRowsRet			= pcRowsRet;
	m_pcFieldsRet		= pcFieldsRet;
	m_pPersistFlagsRet	= pPersistFlagsRet;
	m_pCallback			= pCallback;
	m_pContext			= pContext;
	return OS_DETAIL_STATUS_SUCCESS;
} // TSGetTableDef::Initialize


/**********************************************************************/
// Send - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (DispatchGetTableDefReplpy) to unpack the reply.
/**********************************************************************/
void TSGetTableDef::Send()
{
	TRACE_ENTRY(TSGetTableDef::Send);

	STATUS		status;
	Message		*pMsg;
	
	// Allocate a new private (get table def) message to send to the table service.
	pMsg = new Message(TS_FUNCTION_GET_TABLE_DEF, sizeof(GetTableDefPayload));
	
	// Add the table name to the message as a scatter gather list item.
	pMsg->AddSgl (GET_TABLE_DEF_MSG_TABLENAME_SGI,
				  m_rgbTableName, 
				  strlen(m_rgbTableName)+1,
				  SGL_SEND );
	
	// Add the return buffer to the message as a dynamically allocated reply.
	pMsg->AddSgl (GET_TABLE_DEF_REPLY_FIELDDEFS_SGI,
				  NULL,						//m_prgFieldDefsRet, 
				  NULL,						//m_cbrgFieldDefsMax,
				  SGL_DYNAMIC_REPLY );
	
	// Add the return buffer to the message as a scatter gather list item.
	pMsg->AddSgl (GET_TABLE_DEF_REPLY_TABLEDEF_SGI,
				  (void*)&m_TableDef, 
				  sizeof(m_TableDef),
				  SGL_REPLY );

	// Send the message to the Table Service.  Pass our object pointer as pContext 
	// and specify our own callback handler to be called upon reply.
	status = DdmServices::Send(pMsg, NULL, REPLYCALLBACK(TSGetTableDef, HandleGetTableDefReply));

	// If the send failed delete the message before returning.
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSDefineTable::Send status=%d\n", status));
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		delete pMsg;
		delete this;
	}
} // TSGetTableDef::Send()

/**********************************************************************/
// HandleGetTableDefReply - This is the function specified as the 
// callback in the Send method.
// It unpacks the table Service reply returning data to where the user 
// wanted it put.  It then deletes the message and calls the user's
// callback. 
//
// Parameters:
//	pMsg	- The replied message
/**********************************************************************/
STATUS TSGetTableDef::HandleGetTableDefReply( Message* pMsg )
{
	TRACE_ENTRY(TSGetTableDef::HandleGetTableDefReply);

	// This is the reply payload structure.
	GetTableDefPayload		*pGetTableDefReplyPayload;
	STATUS					status = pMsg->DetailedStatusCode;
	void					*prgFieldDefsRet;
	U32						cbrgFieldDefsMax = 0;

	// Get a pointer to the reply payload and cast it to our reply structure.
	pGetTableDefReplyPayload = (GetTableDefPayload*)pMsg->GetPPayload();
	 
	// Return the number of bytes in the fielddef array.
	if (m_pcbFieldDefsRet)
		*m_pcbFieldDefsRet = pGetTableDefReplyPayload->cbFieldDefsRet;
	
	// Return if this table is persistant?
	if (m_pPersistFlagsRet)
		*m_pPersistFlagsRet = pGetTableDefReplyPayload->persistFlagsRet;
	
	// Return the size of a row.
	if (m_pcbRowRet)
		*m_pcbRowRet = m_TableDef.cbRow;
	
	// Return the number of rows in the table.
	if (m_pcRowsRet)
		*m_pcRowsRet = m_TableDef.cRows;
	
	// Return the number of fields in the row.
	if (m_pcFieldsRet)
		*m_pcFieldsRet = m_TableDef.cFields;
	
	if (m_prgFieldDefsRet != NULL)
	{
		pMsg->GetSgl( GET_TABLE_DEF_REPLY_FIELDDEFS_SGI,
					  &prgFieldDefsRet, &cbrgFieldDefsMax );

 		if (cbrgFieldDefsMax > m_cbrgFieldDefsMax)				// only copy as many as will fit 
			cbrgFieldDefsMax = m_cbrgFieldDefsMax;
							
		memcpy (m_prgFieldDefsRet, prgFieldDefsRet, cbrgFieldDefsMax);	// copy the rowIDs into the user's buffer 
	}

	// Call the user's specified callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );

	// Delete our message delete this object.
	delete pMsg;
	delete this;
	
	return status;
}	// TSGetTabledef

//
tableDef& TSGetTableDef::GetReturnedTableDef()
{
	return m_TableDef;
}

/**********************************************************************/
// TSEnumTable Class Interface - Read Sequential records from a table.
//
/**********************************************************************/
// Initialize - Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//  prgbTableName	- Null terminated name of the table.
//  uStartRowNum	- Starting row number (0 based).
//  pbRowDataRet	- Pointer to the returned data buffer.
//  cbDataRetMax	- Max size of returned RowDataRet buffer.
//					- limited by 8K transfer buffer size.
//  pcbRowDataRet	- Number of bytes returned.
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**********************************************************************/
STATUS TSEnumTable::Initialize( DdmServices*	pDdmServices,
								String64		prgbTableName,
								U32				uStartRowNum,
								void			*pbRowDataRet,
								U32				cbDataRetMax,
								U32*			pcbRowDataRet,
								pTSCallback_t	pCallback,
								void*			pContext
						  	   )

{
	TRACE_ENTRY(TSEnumTable::Initialize);

	// Initialize our base class.
	if ((pDdmServices == NULL) || (prgbTableName == NULL))
		return ercBadParameter;
	SetParentDdm(pDdmServices);
	
	// Stash parameters in member variables.
	m_pDdmServices	= pDdmServices;
	strcpy(m_rgbTableName, prgbTableName);
	m_uStartRowNum	= uStartRowNum;
	m_pbRowDataRet	= pbRowDataRet;
	m_cbDataRetMax	= cbDataRetMax;
	m_pcbRowDataRet	= pcbRowDataRet;
	m_pCallback		= pCallback;	
	m_pContext		= pContext;

	return OS_DETAIL_STATUS_SUCCESS;
}


/**********************************************************************/
// Send - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (DispatchGetTableDefReplpy) to unpack the reply.
/**********************************************************************/
void TSEnumTable::Send()
{
	TRACE_ENTRY(TSEnumTable::Send);

	EnumerateTablePayload	myEnumerateTablePayload;
	STATUS					status;
	Message					*pMsg;

	// Allocate a new private (enumerate table) message to send to the table service.
	pMsg = new Message(TS_FUNCTION_ENUMERATE_TABLE, sizeof(EnumerateTablePayload));
	
	// Initialize our payload structure.
	myEnumerateTablePayload.uStartingRow = m_uStartRowNum;
	
	// Add payload structure to the message.
	pMsg->AddPayload (&myEnumerateTablePayload, sizeof(myEnumerateTablePayload));
	
	// Add the table name to the message as a scatter gather list item.
	pMsg->AddSgl (ENUMERATE_TABLE_MSG_TABLENAME_SGI,
				  m_rgbTableName, 
				  strlen(m_rgbTableName)+1,
				  SGL_SEND );
	
	// Add the return buffer to the message as a dynamically allocated buffer.
	pMsg->AddSgl (ENUMERATE_TABLE_REPLY_SGI,
				  NULL,						// m_pbRowDataRet, 
				  NULL,						// m_cbDataRetMax,
				  SGL_DYNAMIC_REPLY );

	// Send the message to the Table Service.  Pass our object pointer as pContext 
	// and specify our own callback handler to be called upon reply.
	status = DdmServices::Send(pMsg, NULL, REPLYCALLBACK(TSEnumTable, HandleEnumTableReply));

	// If the send failed delete the message before returning.
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSEnumTable::Send status=%d\n", status));
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		delete pMsg;
		delete this;
	}
}

/**********************************************************************/
// HandleEnumTableReply - This is the function specified as the 
// callback in the Send method. 
// It unpacks the table Service reply returning data to where the user 
// wanted it put.  It then deletes the message and calls the user's
// callback. 
//
// Parameters:
//	pMsg	- The replied message
/**********************************************************************/
STATUS TSEnumTable::HandleEnumTableReply( Message* pMsg )
{
	TRACE_ENTRY(TSEnumTable::HandleEnumTableReply);

	// This is the reply payload structure.
	EnumerateTablePayload	*pEnumerateTableReplyPayload;
	STATUS					status = pMsg->DetailedStatusCode;
	void					*pbRowDataRet;
	U32						cbDataRetMax = 0;

	// Get a pointer to the reply payload and cast it to our reply structure.
	pEnumerateTableReplyPayload = (EnumerateTablePayload*)pMsg->GetPPayload();

	// Return the number of entries in the fielddef array.
	if (m_pcbRowDataRet)
		*m_pcbRowDataRet = pEnumerateTableReplyPayload->cbDataRet;
	
	if (m_pbRowDataRet != NULL)
	{
		pMsg->GetSgl( ENUMERATE_TABLE_REPLY_SGI,			// get the dynamic buffer
					  &pbRowDataRet, &cbDataRetMax );

		if (cbDataRetMax > m_cbDataRetMax)					// only copy as many as will fit 
			cbDataRetMax = m_cbDataRetMax;
							
		memcpy (m_pbRowDataRet, pbRowDataRet, cbDataRetMax); 
	}
	
	// Call the user's specified callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );

	// Delete our message delete ourselves.
	delete pMsg;
	delete this;

	return status;
}

/**********************************************************************/
// TSDeleteTable Class Interface - Delete a table using Table or TableId.
//
/**********************************************************************/
// Initialize - Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//  prgbTableName	- Null terminated name of the table.
//	tableId			- optional rowID that represents table, instead of tablename.
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**********************************************************************/
STATUS TSDeleteTable::Initialize (DdmServices	*pDdmServices,
								  String64		rgbTableName,
								  rowID			tableId,
								  pTSCallback_t	pCallback,
								  void			*pContext)

{
	TRACE_ENTRY(TSDeleteTable::Initialize);

	// Initialize our base class.
	if( pDdmServices == NULL )
		return ercBadParameter;
	SetParentDdm(pDdmServices);

	// Stash parameters in member variables.
	m_pDdmServices	= pDdmServices;
	strcpy(m_rgbTableName, rgbTableName);
	m_pCallback = pCallback;
	m_pContext	= pContext;
	m_tableId	= tableId;

	return OS_DETAIL_STATUS_SUCCESS;
}


/**********************************************************************/
// Send - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (DispatchGetTableDefReplpy) to unpack the reply.
/**********************************************************************/
void TSDeleteTable::Send()
{
	TRACE_ENTRY(TSDeleteTable::Send);

	STATUS				status;
	DeleteTablePayload	myDeleteTablePayload;
	Message				*pMsg;

	// Allocate a new private (delete table) message to send to the table service.
	pMsg = new Message(TS_FUNCTION_DELETE_TABLE, sizeof(DeleteTablePayload));
                     
	// Initialize our payload structure.
	myDeleteTablePayload.tableId = m_tableId;
	
	// Add payload structure to the message.
	pMsg->AddPayload( &myDeleteTablePayload, sizeof(myDeleteTablePayload));
	
	// Add the table name to the message as a scatter gather list item.
	pMsg->AddSgl( DELETE_TABLE_MSG_TABLENAME_SGI,
				  m_rgbTableName,
				  strlen(m_rgbTableName)+1,
				  SGL_SEND );

	// Send the message to the Table Service.  Pass our object pointer as pContext 
	// and specify our own callback handler to be called upon reply.
	status = DdmServices::Send(pMsg, NULL, REPLYCALLBACK(TSDeleteTable, HandleDeleteTableReply));

	// If the send failed delete the message before returning.
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSDeleteTable::Send status=%d\n", status));
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		delete pMsg;
		delete this;
	}
}

/**********************************************************************/
// HandleDeleteTableReply - This is the function specified as the 
// callback in the Send method. 
// It unpacks the table Service reply returning data to where the user 
// wanted it put.  It then deletes the message and calls the user's
// callback. 
//
// Parameters:
//	pMsg	- The replied message
/**********************************************************************/
STATUS TSDeleteTable::HandleDeleteTableReply( Message* pMsg )
{
	TRACE_ENTRY(TSDeleteTable::HandleDeleteTableReply);

	STATUS		status = pMsg->DetailedStatusCode;

	// Call the user's specified callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );

	// Delete our message delete ourselves.
	delete pMsg;
	delete this;

	return status;
}

// end of file