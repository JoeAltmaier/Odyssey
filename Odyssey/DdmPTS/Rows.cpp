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
// Description:
// This file is the declaration of an object interface to the Row(s)
// API of the Table Service.
// 
// $Log: /Gemini/Odyssey/DdmPTS/Rows.cpp $
// 
// 36    10/27/99 12:06p Ewedel
// Changed Initialize() members to take CPtsRecordBase* argument as row
// data specifier.  For non-varfield classes, this is added as an
// overload.  For varfield classes, this is now the *only* way of
// specifying row data (i.e., varfield table rows must be derived from
// CPtsRecordBase).
// 
// 35    10/12/99 10:02a Jlane
// Remove unused route by slot code.  Use strlen()+1for name SGLs
// throughout.
// 
// 34    10/06/99 3:27p Sgavarre
// add variable length fields
// 
// 32    8/30/99 9:13a Sgavarre
// clear getsgl size
// 
// 31    8/28/99 5:39p Sgavarre
// add dynamic reply buffers for returned data;  no change to client
// interface
// 
// 30    8/13/99 4:58p Sgavarre
// Update ModifyRow, DeleteRows for multiRow operation
// 
// 29    7/22/99 3:11p Hdo
// 
// 28    7/21/99 7:02p Hdo
// Change the return type of Send() from STATUS to void
// Call the user callback routine when error
// Add error checking
// 
// 27    7/19/99 9:12a Sgavarre
// remove pcRowsModified as not being used by interface
// 
// 26    7/13/99 10:04a Sgavarre
// Fix DeleteRow parameter list: does not return rowId 
// 
// 25    7/12/99 5:00p Sgavarre
// fix bugs in ModifyRow params
// 
// 24    6/30/99 12:54p Mpanas
// Change Tracef to TRACEF and set level 8
// for real trace info
// 
// 23    6/22/99 3:55p Jhatwich
// updated for windows
// 
// 22    6/08/99 3:45p Sgavarre
// remove reply payloads
// 
// 21    6/07/99 8:05p Tnelson
// Fix all warnings...
// 
// 20    5/24/99 8:29p Hdo
// 
// 19    5/24/99 10:41a Hdo
// Remove the GetSgl call in HandlInsertRowReply
// 
// 18    5/18/99 1:59p Jlane
// In ModifyRow::Send() cbKeyFieldvalue and cbRowData were being reversed
// in SGL creation.  Don't do that.
// 
// 17    5/11/99 10:55a Jhatwich
// win32
// 
// 16    4/15/99 3:21p Jlane
// Delete pMsg in all reply handlers.
// 
// 15    4/07/99 5:16p Ewedel
// Changed to use new payload size argument to Message() constructor.
//
// 3/22/99	HD  Update the PTS document and clean up
// 02/20/99 JL	Pass sizeof(rowID) instead of sizeof(rowID*) in InsertRow().
// 01/13/99 HD  Implement TSModifyRow, TSReadRow, TSDeleteRow
//				Modify TSInsert converting message payload to SGL
// 12/14/98 HD  modify the header file and implement it
// 11/23/98	HD	Created.
//
/*************************************************************************/

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_PTS
#include "Odyssey_Trace.h"

#include "Rows.h"
/**********************************************************************/
// TSInsertRow Class Interface - Insert a new row into a table
//
/**********************************************************************/
// Initialize - Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	rgbTableName	- Null terminated name of the table to create.
//	prgbRowData		- Pointer to the row data to insert.
//	cbRowData		- size of the row data in bytes.
//	pRowIDRet		- Pointer to returned RowID for newly inserted row.
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**********************************************************************/
STATUS TSInsertRow::Initialize(	DdmServices 	*pDdmServices,
								String64		rgbTableName,
								void			*prgbRowData,
								U32				cbRowData,
								rowID			*pRowIDRet,
								pTSCallback_t	pCallback,
								void			*pContext )
{
	TRACE_ENTRY(TSInsertRow::Initialize);

	// Check for null in required parameters.
	if (( pDdmServices == NULL ) || ( rgbTableName == NULL )  || ( prgbRowData == NULL ) ||
		( cbRowData == 0 ))
		return ercBadParameter;

	// Initialize our base class.
	SetParentDdm(pDdmServices);
	m_pDdmServices	= pDdmServices;

	// Stash parameters in member variables.
	
	strcpy(m_rgbTableName, rgbTableName);
 	m_prgbRowData	= prgbRowData;
	m_cbRowData		= cbRowData;

	m_pRowIDRet		= pRowIDRet;
	m_pCallback		= pCallback;
	m_pContext		= pContext;

	return OS_DETAIL_STATUS_SUCCESS;
}

/**********************************************************************/
// STATUS Send() - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (HandleInsertRowsReply) to unpack the reply.
//
/**********************************************************************/
void TSInsertRow::Send()
{
	TRACE_ENTRY(TSInsertRow::Send);

	STATUS					status;

	// Allocate a message to send to the table service.
	Message *pMsg = new Message(TS_FUNCTION_INSERT_ROW,
                               sizeof(InsertRowsPayload));

	// Build the SGL, first insert TableName
	pMsg->AddSgl(	INSERT_ROWS_MSG_TABLENAME_SGI,
					m_rgbTableName,
					strlen(m_rgbTableName)+1,
					SGL_SEND);

	// Insert the row data
	pMsg->AddSgl(	INSERT_ROWS_MSG_DATA_BUFFER_SGI,
   				m_prgbRowData,
	   			m_cbRowData,
		   		SGL_SEND);

	// Add the return buffer to the message as SGL
	pMsg->AddSgl(	INSERT_ROWS_REPLY_ROWIDS_SGI,
					NULL,			//m_pRowIDRet,
					NULL,			//sizeof(rowID),
					SGL_DYNAMIC_REPLY );	//SGL_REPLY);

	// Call the Table Service to insert the row.  Pass our object pointer as pContext 
	// and specify our own callback handler to be called upon reply.
	status = DdmOsServices::Send(pMsg, NULL, REPLYCALLBACK(TSInsertRow, HandleInsertRowsReply));

	// If the Send failed delete the message before returning
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSInsertRow::Send status=%d\n", status));
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		delete pMsg;
		delete this;
	}
}	// TSInsertRow::Send()

/**************************************************************************/
// STATUS HandleInsertRowReply() - This is the function specified as the 
// callback in the Send method.  It deletes the message and calls the
// user's callback. 
//
// Parameters:
//	pMsg	- The replied message
/**************************************************************************/
STATUS TSInsertRow::HandleInsertRowsReply(MessageReply *pMsg )
{
	TRACE_ENTRY(TSInsertRow::HandleInsertRowsReply);

	InsertRowsPayload		*pInsertRowsReplyPayload;
	STATUS					status = pMsg->DetailedStatusCode;
	void					*pRowIDRet;
	U32						cbMaxRowID = 0;

	// Get a pointer to the reply payload and cast it to our reply structure.
	pInsertRowsReplyPayload = (InsertRowsPayload*)pMsg->GetPPayload();

	m_cRowsInsertedRet = pInsertRowsReplyPayload->cIDsRet;		// currently not returned to user

	if (m_pRowIDRet != NULL)
	{
		// Add the return buffer to the message as SGL
		pMsg->GetSgl ( INSERT_ROWS_REPLY_ROWIDS_SGI,
						&pRowIDRet,
						&cbMaxRowID);
		if (cbMaxRowID > (sizeof(rowID)))						// only copy as many as will fit 
			cbMaxRowID = sizeof(rowID);							// currently, interface only allows one
							
		memcpy (m_pRowIDRet, pRowIDRet, cbMaxRowID);			// copy the rowIDs into the user's buffer 
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
// TSInsertVLRow Class Interface - Insert a new variable length row into a table
//
/**********************************************************************/
// Initialize - Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	rgbTableName	- Null terminated name of the table to create.
//	pRowData		- Pointer to the row data to insert.  This must be
//                    a CPtsRecordBase derivative.
//	pRowIDRet		- Pointer to returned RowID for newly inserted row.
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**********************************************************************/
STATUS TSInsertVLRow::Initialize (  DdmServices 	*pDdmServices,
									String64		rgbTableName,
									const CPtsRecordBase *pRowData,
									rowID			*pRowIDRet,
									pTSCallback_t	pCallback,
									void			*pContext )
{
	TRACE_ENTRY(TSInsertVLRow::Initialize);

	// Check for null in required parameters.
	if ((pDdmServices == NULL) || (rgbTableName == NULL) || 
		(pRowData == NULL))
		return ercBadParameter;

	// Initialize our base class.
	SetParentDdm(pDdmServices);
	m_pDdmServices	= pDdmServices;

	// Stash parameters in member variables.
	
	strcpy(m_rgbTableName, rgbTableName);

	m_pRowData	= pRowData;
	m_pRowIDRet = pRowIDRet;
	m_pCallback	= pCallback;
	m_pContext	= pContext;
	
	return OS_DETAIL_STATUS_SUCCESS;
}

/**********************************************************************/
// STATUS Send() - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (HandleInsertRowsReply) to unpack the reply.
//
/**********************************************************************/
void TSInsertVLRow::Send()
{
	TRACE_ENTRY(TSInsertVLRow::Send);

	STATUS			status;

	// Allocate a message to send to the table service.
	Message *pMsg = new Message(TS_FUNCTION_INSERT_VLROW,
                               sizeof(InsertVarLenRowPayload));

	// Build the SGL, first insert TableName
	pMsg->AddSgl (INSERT_VLROW_MSG_TABLENAME_SGI,
				  m_rgbTableName, 
				  strlen(m_rgbTableName)+1,
				  SGL_SEND);

	//  Insert the (potentially) variable length row data
    //  (does conversion from varfield pointers into offsets)
    m_pRowData->WriteRecordAsSgl(pMsg, INSERT_VLROW_MSG_DATA_SGI);

	// Call the Table Service to insert the row.  Pass our object pointer as pContext 
	// and specify our own callback handler to be called upon reply.
	status = DdmOsServices::Send(pMsg, NULL, REPLYCALLBACK(TSInsertRow, HandleInsertVarLenRowReply));

	// If the Send failed delete the message before returning
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSInsertRow::Send status=%d\n", status));
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		delete pMsg;
		delete this;
	}
}	// TSInsertVLRow::Send()

/**************************************************************************/
// STATUS HandleInsertVarLenRowReply() - This is the function specified as the 
// callback in the Send method.  It deletes the message and calls the
// user's callback. 
//
// Parameters:
//	pMsg	- The replied message
/**************************************************************************/
STATUS TSInsertVLRow::HandleInsertVarLenRowReply(MessageReply *pMsg )
{
	TRACE_ENTRY(TSInsertVLRow::HandleInsertVLRowReply);

	InsertVarLenRowPayload	*pInsertVLRowPayload;
	STATUS					status = pMsg->DetailedStatusCode;

	// Get a pointer to the reply payload and cast it to our reply structure.
	pInsertVLRowPayload = (InsertVarLenRowPayload*)pMsg->GetPPayload();
	if (m_pRowIDRet != NULL)
		*m_pRowIDRet = pInsertVLRowPayload->rowId;

	// Call the user's specified callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );

	// Delete our message, and delete ourselves.
	delete pMsg;
	delete this;

	return status;
}


/**************************************************************************/
// TSModifyRow Class Interface - Modify the contents of a row in a table.
//
/**************************************************************************/
// Initialize()			--> Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	rgbTableName	- Null terminated name of the table.
//	rgbKeyFieldName	- The key field name used to identify the row to modify
//	pKeyFieldValue	- The key field value used to search for the row to modify.
//	cbKeyFieldValue	- the size of the key field value in bytes.
//	prgbRowData		- Pointer to the row data to insert.
//	cbRowData		- size of the row data in bytes.
//	cRowsToModify	- count of rows to modify;  '0' means ALL that match
//	pcRowsModifiedRet - pointer to the U32 that will have the # of rows modified.
//	pRowIDRet		- Pointer to returned RowID for newly modified row.
//	cbMaxRowIDs		- size of buffer for rowIDs
//	pcRowsModifiedRet - pointer to the U32 that will have the # of rows modified.//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**************************************************************************/
STATUS TSModifyRow::Initialize(	DdmServices 	*pDdmServices,
								String64		rgbTableName,
								String64		rgbKeyFieldName,
								void			*pKeyFieldValue,
								U32				cbKeyFieldValue,
								void			*prgbRowData,
								U32				cbRowData,
								U32				cRowsToModify,
								U32				*pcRowsModifiedRet,
								rowID			*pRowIDRet,
								U32				cbMaxRowID,
								pTSCallback_t	pCallback,
								void*			pContext )
{
	TRACE_ENTRY(TSModifyRow::Initialize);

	if ((pDdmServices == NULL) || (rgbTableName == NULL) || (rgbKeyFieldName == NULL) ||
		 (prgbRowData == NULL) || (cbRowData == 0))
		return ercBadParameter;

	// Initialize our base class.
	SetParentDdm(pDdmServices);
	
	m_pDdmServices	= pDdmServices;
 	strcpy(m_rgbTableName, rgbTableName);
	strcpy(m_rgbKeyFieldName, rgbKeyFieldName);
	m_pKeyFieldValue	= pKeyFieldValue;
	m_cbKeyFieldValue	= cbKeyFieldValue;
	m_prgbRowData		= prgbRowData;
	m_cbRowData			= cbRowData;
	m_cRowsToModify		= cRowsToModify;
	m_pcRowsModifiedRet	= pcRowsModifiedRet;
	m_cbMaxRowID		= cbMaxRowID;

	m_pRowIDRet			= pRowIDRet;
	m_pCallback			= pCallback;
	m_pContext			= pContext;

    //  we assume that we are given just a raw buffer, unless told otherwise
    m_fHasPtsRecBase    = FALSE;

	return OS_DETAIL_STATUS_SUCCESS;
} // TSModifyRow::Initialize()

/**********************************************************************/
// STATUS Send() - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (HandleInsertRowsReply) to unpack the reply.
//
/**********************************************************************/
void TSModifyRow::Send()
{
	TRACE_ENTRY(TSModifyRow::Send);

	STATUS					status;
	ModifyRowsPayload		myModifyRowsPayload;

	// Allocate a message to send to the table service.
	Message *pMsg=new Message(TS_FUNCTION_MODIFY_ROW,
                             sizeof (ModifyRowsPayload));

	myModifyRowsPayload.cRowsToModify = m_cRowsToModify;

	// Add payload structure to the message.
	pMsg->AddPayload( &myModifyRowsPayload, sizeof(myModifyRowsPayload));


	// Add the Table Name to SGL
	pMsg->AddSgl(	MODIFY_ROWS_MSG_TABLENAME_SGI,
					m_rgbTableName,
					strlen(m_rgbTableName)+1,
					SGL_SEND);

	// Insert the key field name
	pMsg->AddSgl(	MODIFY_ROWS_MSG_KEY_NAMES_SGI,
					m_rgbKeyFieldName,
					strlen(m_rgbKeyFieldName)+1,
					SGL_SEND);

	// Insert the key field value
	pMsg->AddSgl(	MODIFY_ROWS_MSG_KEY_VALUES_SGI,
					m_pKeyFieldValue,
					m_cbKeyFieldValue,
					SGL_SEND);

	// Insert the data
    if (m_fHasPtsRecBase)
       {
       //  write standard varfield-aware record to SGL
       ((CPtsRecordBase *) m_prgbRowData)->
                  WriteRecordAsSgl(pMsg, MODIFY_ROWS_MSG_DATA_BUFFER_SGI);
       }
    else
       {
       //  write caller's data as raw buffer
       pMsg->AddSgl(	MODIFY_ROWS_MSG_DATA_BUFFER_SGI,
                       m_prgbRowData,
                       m_cbRowData,
                       SGL_SEND);
       }

	// Add the return buffer to the message as SGL
	pMsg->AddSgl(	MODIFY_ROWS_REPLY_ROWIDS_SGI,
					NULL,					//m_pRowIDRet,
					NULL,					//m_cbMaxRowID,
					SGL_DYNAMIC_REPLY );

	// Call the Table Service to modify the row.  Pass our object pointer as pContext 
	// and specify our own callback handler to be called upon reply.
	status = DdmServices::Send( pMsg, NULL, REPLYCALLBACK(TSModifyRow, HandleModifyRowsReply));

	// If the Send failed delete the message before returning
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSModifyRow::Send status=%d\n", status));
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		delete pMsg;
		delete this;
	}
} // TSModifyRow::Send()


/**************************************************************************/
// STATUS HandleModifyRowsReply() - This is the function specified as the 
// callback in the Send method.  It deletes the message and calls the
// user's callback. 
//
// Parameters:
//	pMsg	- The replied message
/**************************************************************************/
STATUS TSModifyRow::HandleModifyRowsReply(MessageReply *pMsg)
{
	TRACE_ENTRY(TSModifyRow::HandleModifyRowsReply);

	ModifyRowsPayload		*pModifyRowsReplyPayload;
	STATUS					status = pMsg->DetailedStatusCode;
	void					*pRowIDRet;
	U32						cbMaxRowID = 0;


	// Get a pointer to the reply payload and cast it to our reply structure.
	pModifyRowsReplyPayload = (ModifyRowsPayload*)pMsg->GetPPayload();
	if (m_pcRowsModifiedRet != NULL)
		*m_pcRowsModifiedRet = pModifyRowsReplyPayload->cRowsModifiedRet;

	if (m_pRowIDRet != NULL )
	{
		pMsg->GetSgl(	MODIFY_ROWS_REPLY_ROWIDS_SGI,
						&pRowIDRet,
						&cbMaxRowID	 );
		if (cbMaxRowID > m_cbMaxRowID)							// only copy as many as will fit 
			cbMaxRowID = m_cbMaxRowID;
							
		memcpy (m_pRowIDRet, pRowIDRet, cbMaxRowID);			// copy the rowIDs into the user's buffer 
	}

	// Call the user's specified callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );
	
	// Delete our message, delete ourselves.
	delete pMsg;
	delete this;

	return status;
}	// TSModifyRow::HandleModifyRowReply()
						

/**************************************************************************/
// TSReadRow Class Interface - Read a specified row from the specified table
//
/**************************************************************************/
// Initialize - Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	rgbTableName	- Null terminated name of the table.
//	rgbKeyFieldName	- The key field name used to identify the row to modify
//	pKeyFieldValue	- The key field value used to search for the row to modify.
//	cbKeyFieldValue	- the size of the key field value in bytes.
//	prgbRowDataRet	- Pointer to the row data to insert.
//	cbRowDataRetMax	- size of the row data in bytes.
//	pcRowsReadRet	- pointer to count of rows read.
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**************************************************************************/
STATUS TSReadRow::Initialize(	DdmServices		*pDdmServices,
								String64		rgbTableName,
								String64		prgbKeyFieldName,
								void			*pKeyFieldValue,
								U32				cbKeyFieldValue,
								void			*prgbRowDataRet,
								U32				cbRowDataRetMax,
								U32				*pcRowsReadRet,
								pTSCallback_t	pCallback,
								void			*pContext )
								
{
	TRACE_ENTRY(TSReadRow::Initialize);

	if ((pDdmServices == NULL) || (rgbTableName == NULL) || (prgbKeyFieldName == NULL))
		return ercBadParameter;
	
	// Initialize our base class.
	SetParentDdm(pDdmServices);
	
	// Stash parameters in member variables.
	m_pDdmServices		= pDdmServices;
	strcpy(m_rgbTableName, rgbTableName);
	strcpy(m_rgbKeyFieldName, prgbKeyFieldName);
	m_pKeyFieldValue	= pKeyFieldValue;
	m_cbKeyFieldValue	= cbKeyFieldValue;

	m_prgbRowDataRet	= prgbRowDataRet;
	m_cbRowDataRetMax	= cbRowDataRetMax;
	m_pcRowsReadRet	= pcRowsReadRet;
	m_pCallback			= pCallback;
	m_pContext			= pContext;
	
	return OS_DETAIL_STATUS_SUCCESS;
} // TSReadRow::Initialize

/**********************************************************************/
// STATUS Send() - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (HandleReadRowsReply) to unpack the reply.
//
/**********************************************************************/
void TSReadRow::Send()
{
	TRACE_ENTRY(TSReadRow::Send);

	STATUS				status;

	// Allocate a message to send to the table service.
	Message *pMsg=new Message(TS_FUNCTION_READ_ROW,
                             sizeof(ReadRowsPayload));

	// Add the Table Name to SGL
	pMsg->AddSgl(	READ_ROWS_MSG_TABLENAME_SGI,
					m_rgbTableName,
					strlen(m_rgbTableName)+1,
					SGL_SEND);

	// Insert the key field name
	pMsg->AddSgl(	READ_ROWS_MSG_KEY_NAMES_SGI,
					m_rgbKeyFieldName,
					strlen(m_rgbKeyFieldName)+1,
					SGL_SEND);

	// Insert the key field value
	pMsg->AddSgl(	READ_ROWS_MSG_KEY_VALUES_SGI,
					m_pKeyFieldValue,
					m_cbKeyFieldValue,
					SGL_SEND);

	// Add the returned data buffer to the message as a SGL item.
	pMsg->AddSgl( READ_ROWS_REPLY_DATA_BUFFER_SGI,
				  NULL,				// m_prgbRowDataRet, 
				  NULL,				// m_cbRowDataRetMax,
				  SGL_DYNAMIC_REPLY );

	// Call the Table Service to read the table.  Pass our object pointer as pContext 
	// and specify our own callback handler to be called upon reply.
	status = DdmServices::Send(pMsg, NULL, REPLYCALLBACK(TSReadRow, HandleReadRowsReply));

	// If the Send failed delete the message before returning
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSReadRow::Send status=%d\n", status));
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		delete pMsg;		
		delete this;
	}
} // TSReadRow::Send

/**************************************************************************/
// STATUS HandleReadRowsReply() - This is the function specified as the 
// callback in the Send method.  It deletes the message and calls the
// user's callback. 
//
// Parameters:
//	pMsg	- The replied message
/**************************************************************************/
STATUS TSReadRow::HandleReadRowsReply(MessageReply *pMsg)
{
	TRACE_ENTRY(TSReadRow::HandleReadRowsReply);

	ReadRowsPayload	*pReadRowsPayload;
	STATUS			status = pMsg->DetailedStatusCode;
	U32				cbRowDataRetMax = 0;


	// Get a pointer to the reply payload and cast it to our reply structure.
	pReadRowsPayload = (ReadRowsPayload*)pMsg->GetPPayload();

	// Return the number of rows read if the caller so desires.  
	if (m_pcRowsReadRet)
		*m_pcRowsReadRet = pReadRowsPayload->cRowsReadRet;
	
	if (m_prgbRowDataRet != NULL)
	{
        //  copy the caller's returned data into the caller's own buffer

        //  find out how big SGL data is
        cbRowDataRetMax = pMsg->GetSglDataSize (READ_ROWS_REPLY_DATA_BUFFER_SGI);

        //  only copy as much as will fit in caller's buffer
		if (cbRowDataRetMax > m_cbRowDataRetMax)
			cbRowDataRetMax = m_cbRowDataRetMax;

        pMsg->CopyFromSgl (READ_ROWS_REPLY_DATA_BUFFER_SGI, 0,
                           m_prgbRowDataRet, cbRowDataRetMax);
	}
 
	// Call the user's specified callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );

	// Delete our message delete ourselves.
	delete pMsg;
	delete this;

	return status;
}	// TSReadRow::HandleTSReadRowReply
						
 
/**********************************************************************/
// TSReadVLRow Class Interface - Read a new variable length row into a table
//
/**********************************************************************/
// Initialize - Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	rgbTableName	- Null terminated name of the table to create.
//	rgbKeyFieldName	- The key field name used to identify the row to modify
//	pKeyFieldValue	- The key field value used to search for the row to modify.
//	cbKeyFieldValue	- the size of the key field value in bytes.
//	ppRowDataRet	- Pointer to user pointer where address of the row data is returned.
//	pcbRowDataRet	- Return pointer where size of the row data is written.
//	pcRowsReadRet	- pointer to count of rows read.
//  
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**********************************************************************/
STATUS TSReadVLRow::Initialize	 (  DdmServices 	*pDdmServices,
									String64		rgbTableName,
									String64		prgbKeyFieldName,
									void			*pKeyFieldValue,
									U32				cbKeyFieldValue,
									CPtsRecordBase  **ppRowDataRet,
									U32				*pcbRowDataRet,
									U32				*pcRowsReadRet,
									pTSCallback_t	pCallback,
									void			*pContext )
{
	TRACE_ENTRY(TSReadVLRow::Initialize);

	// Check for null in required parameters.
	if ((pDdmServices == NULL) || (rgbTableName == NULL) || (prgbKeyFieldName == NULL))
 		return ercBadParameter;

	// Initialize our base class.
	SetParentDdm(pDdmServices);
	m_pDdmServices	= pDdmServices;
	
	strcpy(m_rgbTableName, rgbTableName);
	strcpy(m_rgbKeyFieldName, prgbKeyFieldName);
	m_pKeyFieldValue	= pKeyFieldValue;
	m_cbKeyFieldValue	= cbKeyFieldValue;

	m_ppRowDataRet  = ppRowDataRet;
	m_pcbRowDataRet = pcbRowDataRet;
 	m_pcRowsReadRet	= pcRowsReadRet;

	m_pCallback	= pCallback;
	m_pContext	= pContext;
	
	return OS_DETAIL_STATUS_SUCCESS;
}

/**********************************************************************/
// Send() - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (HandleInsertRowsReply) to unpack the reply.
//
/**********************************************************************/
void TSReadVLRow::Send()
{
	TRACE_ENTRY(TSReadVLRow::Send);

	STATUS			status;

	// Allocate a message to send to the table service.
	Message *pMsg = new Message(TS_FUNCTION_READ_VARLEN_ROW,
                               sizeof(ReadVarLenRowsPayload));

	// Build the SGL, first insert TableName
	pMsg->AddSgl (READ_VLROWS_MSG_TABLENAME_SGI,
				  m_rgbTableName, 
				  strlen(m_rgbTableName)+1,
				  SGL_SEND);

 	// Insert the key field name
	pMsg->AddSgl (READ_VLROWS_MSG_KEY_NAMES_SGI,
				  m_rgbKeyFieldName,
				  strlen(m_rgbKeyFieldName)+1,
				  SGL_SEND);

	// Insert the key field value
	pMsg->AddSgl (READ_VLROWS_MSG_KEY_VALUES_SGI,
				  m_pKeyFieldValue,
				  m_cbKeyFieldValue,
				  SGL_SEND);

	if (m_ppRowDataRet != NULL ) 
	{
		// Add the return row data SGL item
		// NULL pb indicates data buffer to be alloc'd by transport.
		// NULL cb will tell transport to get allocate size when the
		// PTS calls GetSGL.	READ_VLROWS_REPLY_ROWDATA_SGI
		pMsg->AddSgl (	READ_VLROWS_REPLY_ROWDATA_SGI,
						NULL,
						NULL,
						SGL_DYNAMIC_REPLY );
	}

    //  no support for separate varfield descriptors SGL now
//*	if (m_ppVarLenFieldsRet != NULL )
//*	{	// Add the return ID and optional row data buffer as an SGL item
//*		// NULL pb indicates data buffer to be alloc'd by transport.
//*		pMsg->AddSgl (	READ_VLROWS_REPLY_VLFS_SGI,
//*						NULL,								
//*						NULL, 
//*						SGL_DYNAMIC_REPLY);  
//*	}

	// Call the Table Service to insert the row.  Pass our object pointer as pContext 
	// and specify our own callback handler to be called upon reply.
	status = DdmOsServices::Send(pMsg, NULL, REPLYCALLBACK(TSInsertRow, HandleReadVarLenRowReply));

	// If the Send failed delete the message before returning
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSInsertRow::Send status=%d\n", status));
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		delete pMsg;
		delete this;
	}
}	// TSReadVLRow::Send()



/**************************************************************************/
// STATUS HandleReadVarLenRowReply() - This is the function specified as the 
// callback in the Send method.  It deletes the message and calls the
// user's callback. 
//
// Parameters:
//	pMsg	- The replied message
/**************************************************************************/

STATUS TSReadVLRow::HandleReadVarLenRowReply(MessageReply *pMsg )
{
	TRACE_ENTRY(TSReadVLRow::HandleReadVLRowReply);

	ReadVarLenRowsPayload	*pReadVLRowPayload;
	STATUS					status = pMsg->DetailedStatusCode;
    U8                      *pLocalBuf;   // buffer to free, sometimes
    U8                      *pbRowData;   // pointer to contig. row data
    U32                     cbRowData;    // count of bytes in *pbRowData
    CPtsRecordBase          *pCurRec;
    U32                     cbTotalRecSize;

	// Get a pointer to the reply payload and cast it to our reply structure.
	pReadVLRowPayload = (ReadVarLenRowsPayload*)pMsg->GetPPayload();
 	// Return the number of rows read if the caller so desires.  
	if (m_pcRowsReadRet)
		*m_pcRowsReadRet = pReadVLRowPayload->cRowsReadRet;

	// Get the return record buffer if the user wanted it.
	if ((m_ppRowDataRet != NULL) && (m_pcbRowDataRet != NULL))
	{
        //  user wants data, figure out what we have to do to give it to them.

        //  we have to do different stuff, based on whether the returned
        //  SGL data is large enough to be fragmented
        if (pMsg->GetCFragment (READ_VLROWS_REPLY_ROWDATA_SGI) <= 1)
        {
           //  cool, all data fits in a single, contiguous SGL fragment.
           //  Just return a pointer to that fragment's own buffer.
           pMsg->GetSgl ( READ_VLROWS_REPLY_ROWDATA_SGI,
                          (void **) m_ppRowDataRet, &cbRowData);

           //  indicate that we have no allocation to free.
           pLocalBuf = NULL;
        }
        else
        {
           //  whoops, SGL consists of multiple fragments.  So we make
           //  a copy, which uses a single contiguous buffer.
           pLocalBuf = (U8 *) pMsg->GetSglDataCopy (READ_VLROWS_REPLY_ROWDATA_SGI,
                                                    &cbRowData);

           //  return allocated buf ptr to requestor also
           *m_ppRowDataRet = (CPtsRecordBase *) pLocalBuf;
        }

        //  if user wanted data size, let 'em know
        if (m_pcbRowDataRet != NULL)
        {
           *m_pcbRowDataRet = cbRowData;
        }

        //  by hook or by crook, we have a contiguous buffer
        //  containing the SGL's complete data.

        //  now we scoot through it, converting each enclosed record
        //  into pointer-normal form.
        assert (*m_ppRowDataRet != NULL);

        pbRowData = (U8 *) *m_ppRowDataRet;
        while (cbRowData > sizeof (CPtsRecordBase))
        {
           pCurRec = (CPtsRecordBase *) pbRowData;

           //  returned records should always be complete
           cbTotalRecSize = pCurRec->TotalRecSize();
           assert (cbRowData >= cbTotalRecSize);

           if (cbRowData >= cbTotalRecSize)
           {
              //  convert this record's offsets into pointers
              pCurRec->ConvertVarFieldOffsetsToPointers(cbTotalRecSize);
           }
           else
           {
              //  whoops, all done.
              break;
           }

           //  advance to next row
           cbRowData -= cbTotalRecSize;
           pbRowData += cbTotalRecSize;
        }

        //  shouldn't have any left over bits
        assert (cbRowData == 0);
	}
    else
    {
       //  user doesn't want row data (how peculiar!), so flag that we have
       //  no buffer to delete
       pLocalBuf = NULL;
    }

    //  right now, we ignore the separate varfield descriptors SGL item
//*	if ((m_ppVarLenFieldsRet != NULL) && (m_pcbVarLenFieldsRet))
//*	{
//*		// Add the return buffer to the message as SGL
//*		pMsg->GetSgl (	READ_VLROWS_REPLY_VLFS_SGI,
//*						m_ppVarLenFieldsRet,
//*						m_pcbVarLenFieldsRet);
//*	}

 	// Call the user's specified callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );

	// Delete our message, our optional row data buffer, and delete ourselves.
	delete pMsg;
    delete [] pLocalBuf;
	delete this;

	return status;
}


/**************************************************************************/
// TSDeleteRow Class Interface - Delete a specified row from the specified table
//
/**************************************************************************/
// Initialize - Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	rgbTableName	- Null terminated name of the table.
//	rgbKeyFieldName	- The key field name used to identify the row to modify
//	pKeyFieldValue	- The key field value used to search for the row to modify.
//	cbKeyFieldValue	- the size of the key field value in bytes.
//	pcRowsDel		- Pointer to returned number of rows deleted
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**************************************************************************/
STATUS TSDeleteRow::Initialize(	DdmServices 	*pDdmServices,
								String64		rgbTableName,
								String64		prgbKeyFieldName,
								void			*pKeyFieldValue,
								U32				cbKeyFieldValue,
								U32				cRowsToDelete,
								U32				*pcRowsDelRet,
								pTSCallback_t	pCallback,
								void			*pContext )
{
	TRACE_ENTRY(TSDeleteRow::Initialize);
	if ((pDdmServices == NULL) || (rgbTableName == NULL) || (prgbKeyFieldName == NULL))
		return ercBadParameter;
		
	// Initialize our base class.
	SetParentDdm(pDdmServices);
	
	// Stash parameters in member variables.
	m_pDdmServices		= pDdmServices;
	strcpy(m_rgbTableName, rgbTableName);
	strcpy(m_rgbKeyFieldName, prgbKeyFieldName);
	
	m_pKeyFieldValue	= pKeyFieldValue;
	m_cbKeyFieldValue	= cbKeyFieldValue;
	m_cRowsToDelete		= cRowsToDelete;
	m_pcRowsDelRet		= pcRowsDelRet;
	m_pCallback			= pCallback;
	m_pContext			= pContext;
	
	return OS_DETAIL_STATUS_SUCCESS;
} // TSDeleteRow::Initialize

/**********************************************************************/
// STATUS Send() - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (HandleDeleteRowsReply) to unpack the reply.
//
/**********************************************************************/
void TSDeleteRow::Send()
{
	TRACE_ENTRY(TSDeleteRow::Send);

	STATUS					status;
	DeleteRowsPayload		myDeleteRowsPayload;

	// Allocate a message to send to the table service.
	Message *pMsg=new Message(TS_FUNCTION_DELETE_ROW,
                             sizeof(DeleteRowsPayload));
                             
	myDeleteRowsPayload.cRowsToDelete = m_cRowsToDelete;
	// Add payload structure to the message.

	pMsg->AddPayload( &myDeleteRowsPayload, sizeof(myDeleteRowsPayload));

	// Add the Table Name to SGL
	pMsg->AddSgl(	DELETE_ROWS_MSG_TABLENAME_SGI,
					m_rgbTableName,
					strlen(m_rgbTableName)+1,
					SGL_SEND);

	// Insert the key field name
	pMsg->AddSgl(	DELETE_ROWS_MSG_KEY_NAMES_SGI,
					m_rgbKeyFieldName,
					strlen(m_rgbKeyFieldName)+1,
					SGL_SEND);

	// Insert the key field value
	pMsg->AddSgl(	DELETE_ROWS_MSG_KEY_VALUES_SGI,
					m_pKeyFieldValue,
					m_cbKeyFieldValue,
					SGL_SEND);

	// Call the Table Service to delete the row.  Pass our object pointer as pContext 
	// and specify our own callback handler to be called upon reply.
	status = DdmServices::Send(pMsg, NULL, REPLYCALLBACK(TSDeleteRow, HandleDeleteRowsReply));

	// If the Send failed delete the message before returning
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSDeleteRow::Send status=%d\n", status));
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		delete pMsg;
		delete this;
	}
} // TSDeleteRow::Send
	
/**************************************************************************/
// STATUS HandleDeleteRowsReply() - This is the function specified as the 
// callback in the Send method.  It deletes the message and calls the
// user's callback. 
//
// Parameters:
//	pMsg	- The replied message
/**************************************************************************/
STATUS TSDeleteRow::HandleDeleteRowsReply(MessageReply *pMsg)
{
	TRACE_ENTRY(TSDeleteRow::HandleDeleteRowsReply);

	DeleteRowsPayload		*pDeleteRowsReplyPayload;
	STATUS					status = pMsg->DetailedStatusCode;

	// Get a pointer to the reply payload and cast it to our reply structure.
	pDeleteRowsReplyPayload = (DeleteRowsPayload*)pMsg->GetPPayload();

	// Return count of rows deleted if the caller so desires.
	if (m_pcRowsDelRet)
		*m_pcRowsDelRet = pDeleteRowsReplyPayload->cRowsDeletedRet;
	
	// Call the user's specified callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );

	// Delete our message delete ourselves.
	delete pMsg;
	delete this;

	return status;
}	// TSDeleteRow::HandleDeleteRowReply

// end of file