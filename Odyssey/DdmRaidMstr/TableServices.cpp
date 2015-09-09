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
// File: TableServices.cpp
// 
// Description:
//	Provides a wrapper for the PTS table operations.
// 
// $Log: /Gemini/Odyssey/DdmPartitionMstr/TableServices.cpp $
// 
// 7     1/21/00 12:04p Szhang
// Added TableServiceDeleteAllMatchedRows method to allow delete more
// than one row at a time.
// 
// 6     10/05/99 5:48p Dpatel
// 
// 5     8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 4     8/16/99 2:53p Dpatel
// changes for PTS interface changes to ModifyRow/Field and deleterow.
// 
// 3     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
//
/*************************************************************************/

#include "TableServices.h"


//************************************************************************
//	CONSTRUCTOR
//		Store the parent ddm services pointer. This will be used to call
//		the callback.
//************************************************************************
TableServices::TableServices(DdmServices *pParentDdm)
				: DdmServices( pParentDdm ){
	m_pParentDdm = pParentDdm;
}

//************************************************************************
//	DESTRUCTOR
//
//************************************************************************
TableServices::~TableServices(){
}




typedef enum {
	SERVICE_READ_TABLE_DEF = 1
}TABLE_SERVICE_STATUS;

//*********************************************************
//	TableServiceEnumTable
//		Enums the table specified and returns the number of rows
//		and the data in those rows.
//
//	tableName		- table whose rows are to be enumed
//	ppTableDataRet	- where you want data - caller is supposed to delete
//	pSizeofTableDataRet	- where you want size of ret data
//	pNumberOfRows		- where you want no.of rows returned
//
//	pOriginalContext	- any context data
//	pCallback			- the callback on completion of enum
//
//********************************************************
STATUS TableServices::
TableServiceEnumTable(
			char			*tableName,
			void			**ppTableDataRet,		// where you want data
			U32				*pSizeofTableDataRet,	// where you want size of ret data
			U32				*pNumberOfRows,
			void			*pOriginalContext,
			pTSCallback_t	pCallback)
{
	TSGetTableDef		*pTableDef = new TSGetTableDef;
	STATUS				status;

	TABLE_CONTEXT		*pContext = new TABLE_CONTEXT;
	memset(pContext,0,sizeof(TABLE_CONTEXT));
	
	pContext->state				= SERVICE_READ_TABLE_DEF;
	pContext->pParentContext	= pOriginalContext;			
	pContext->pCallback			= pCallback;
	pContext->pData				= new String64;
	strcpy((char *)pContext->pData,tableName);
	pContext->pData1			= ppTableDataRet;
	pContext->pData2			= (void *)pSizeofTableDataRet;
	pContext->pData3			= (void *)pNumberOfRows;
	
	status = pTableDef->Initialize(	
							this,
							(char *)pContext->pData,
							NULL,		
							0,
							&pContext->value,	// num cols
							&pContext->value1,	// bytes/row
							&pContext->value2,	// no of rows
							&pContext->value3,	// count of fields in a row
							NULL,
							(pTSCallback_t)&TableServices::TableServiceEnumerate,
							pContext );
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTableDef->Send();
	return status;
}




//*********************************************************
//	TableServiceEnumerate
//		This method is called after we get the table def for
//		the table and determine the number of rows and bytes per
//		row. We allocate the total memory required to hold the table
//		data and call the PTS enum table method to actually read
//		the table data.
//
//********************************************************
STATUS TableServices::
TableServiceEnumerate(void	*_pContext, STATUS status)
{
	U32					numberOfRows = 0;
	U32					bytesPerRow = 0;
	void				**ppTableDataRet = NULL;
	U32					*pSizeofTableDataRet =NULL;
	U32					*pNumberOfRows = NULL;
	char				*pTableName;
	TABLE_CONTEXT		*pContext = (TABLE_CONTEXT *)_pContext;
	
	if (status != OS_DETAIL_STATUS_SUCCESS){
		// we report back to the caller with the error.
		TableServiceEnumReplyHandler (pContext,status);
	} else {
		numberOfRows = pContext->value2;
		bytesPerRow = pContext->value1;

		// Fill in the information for the original context
		pNumberOfRows = (U32 *)pContext->pData3;
		*pNumberOfRows = numberOfRows;


		ppTableDataRet = (void **)pContext->pData1;
		pSizeofTableDataRet = (U32 *)pContext->pData2;

		// this is where the user had wanted his data, caller will delete
		*ppTableDataRet	= new char[numberOfRows * bytesPerRow];
		memset(*ppTableDataRet, 0, numberOfRows * bytesPerRow );

		pTableName				= new String64;
		strcpy(pTableName, (char *)pContext->pData);

		// the handler will return the enum data to the caller
		// with the state of the orignalContext.
		TSEnumTable			*pEnumTable = new TSEnumTable();
		status = pEnumTable->Initialize(
							this,
							(char *)pTableName,		
							0,						// start row
							*ppTableDataRet,
							numberOfRows * bytesPerRow,
							(U32 *)pSizeofTableDataRet,
							(pTSCallback_t)&TableServices::TableServiceEnumReplyHandler,
							pContext);
		if (status == OS_DETAIL_STATUS_SUCCESS)
			pEnumTable->Send();
		// pEnumTable->Initialize() does a strcpy, so we can delete.
		delete pTableName;
	}
	return status;
}


//*********************************************************
//	TableServiceEnumReplyHandler
//		The reply for the EnumTable->Send(). Here we just call
//		the users callback with their originalContext and the
//		status.
//
//********************************************************
STATUS TableServices::
TableServiceEnumReplyHandler(void *_pContext, STATUS status)
{
	TABLE_CONTEXT		*pOriginalContext = NULL;
	pTSCallback_t		cb;

	TABLE_CONTEXT		*pContext = (TABLE_CONTEXT *)_pContext;

	pOriginalContext = (TABLE_CONTEXT *)pContext->pParentContext;
	cb = (pTSCallback_t)pContext->pCallback,	// callback
	(m_pParentDdm->*cb)(pOriginalContext,status);
	// now you can delete pContext, but we dont need to delete
	// any data that we allocated since user will delete it.
	pContext->pData1 = NULL;
	pContext->pData2 = NULL;
	pContext->pData3 = NULL;
	delete pContext;
	pContext = NULL;
	return status;
}




//*********************************************************
//	TableServiceReadRow
//
//********************************************************
STATUS TableServices::
TableServiceReadRow(
			String64		tableName,
			rowID			*pRowToRead,
			void			*buffer,
			U32				sizeofData,
			pTSCallback_t	pCallback,
			void			*pOriginalContext)
{
	STATUS			status;
	TSReadRow		*pReadRow;
	TABLE_CONTEXT	*pContext;

	pContext = new TABLE_CONTEXT;
	pContext->pCallback = pCallback;
	pContext->pParentContext = pOriginalContext;

	pReadRow = new TSReadRow;
	status = pReadRow->Initialize(
			this,
			tableName,
			CT_PTS_RID_FIELD_NAME,
			pRowToRead,
			sizeof(rowID),
			buffer,
			sizeofData,
			NULL,
			(pTSCallback_t)&TableServices::TableServiceOperationsReplyHandler,
			pContext
			);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pReadRow->Send();
	return status;
}




//*********************************************************
//	TableServiceInsertRow
//
//********************************************************
STATUS TableServices::
TableServiceInsertRow(
			String64		tableName,
			void			*buffer,
			U32				sizeofData,
			rowID			*pNewRowIdRet,
			pTSCallback_t	cb,
			void			*pOriginalContext)
{
	STATUS				status = 0;
	TSInsertRow			*pInsertRow = NULL;
	TABLE_CONTEXT	*pContext;

	pContext = new TABLE_CONTEXT;
	pContext->pCallback = cb;
	pContext->pParentContext = pOriginalContext;


	pInsertRow = new TSInsertRow;
	status = pInsertRow->Initialize(
				this,
				tableName,
				buffer,
				sizeofData,
				pNewRowIdRet,
				(pTSCallback_t)&TableServices::TableServiceOperationsReplyHandler,
				pContext);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pInsertRow->Send();
	return status;
}


//*********************************************************
//	TableServiceModifyRow
//
//********************************************************
STATUS TableServices::
TableServiceModifyRow(
			String64		tableName,
			rowID			*pRowToModify,
			void			*buffer,
			U32				sizeofData,
			rowID			*pNewRowIdRet,
			pTSCallback_t	cb,
			void			*pOriginalContext)
{
	STATUS			status;
	TSModifyRow		*pModifyRow;
	TABLE_CONTEXT	*pContext;

	pContext = new TABLE_CONTEXT;
	pContext->pCallback = cb;
	pContext->pParentContext = pOriginalContext;

	// Update ADT information
	pModifyRow = new TSModifyRow;

	status = pModifyRow->Initialize(
		this,
		tableName,
		CT_PTS_RID_FIELD_NAME,		// use the row id as the key
		pRowToModify,				// key
		sizeof(rowID),				// key size
		buffer,						// buffer containing new row data
		sizeofData,					// size of row
		1,							// count of rows to modify
		&pContext->value,			// ptr to no.of rows actually modified
		pNewRowIdRet,				// returned row id
		sizeof(rowID),				// size of buffer to return row ids
		(pTSCallback_t)&TableServices::TableServiceOperationsReplyHandler,
		pContext);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pModifyRow->Send();
	return status;
}


//*********************************************************
//	TableServiceDeleteRow
//
//********************************************************
STATUS TableServices::
TableServiceDeleteRow(
			String64		tableName,
			rowID			*pRowId,
			pTSCallback_t	cb,
			void			*pOriginalContext)
{
	STATUS		status;
	TABLE_CONTEXT	*pContext;

	pContext = new TABLE_CONTEXT;
	pContext->pCallback = cb;
	pContext->pParentContext = pOriginalContext;

	TSDeleteRow *pTSDeleteRow = new TSDeleteRow; 

	
	status = pTSDeleteRow->Initialize(
		this,
		tableName,
		CT_PTS_RID_FIELD_NAME,
		pRowId,
		sizeof(rowID),
		1,						// no of rows to delete
		&pContext->value,		// rows deleted
		(pTSCallback_t)&TableServices::TableServiceOperationsReplyHandler,
		pContext
	);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDeleteRow->Send();
	return status;
}

//*********************************************************
//	TableServiceDeleteAllMatchedRows
//
//********************************************************
STATUS TableServices::
TableServiceDeleteAllMatchedRows(
			String64		tableName,
			String64		KeyField,
			rowID			*pKeyFieldValue,
			pTSCallback_t	cb,
			void			*pOriginalContext)
{
	STATUS		status;
	TABLE_CONTEXT	*pContext;

	pContext = new TABLE_CONTEXT;
	pContext->pCallback = cb;
	pContext->pParentContext = pOriginalContext;

	TSDeleteRow *pTSDeleteRow = new TSDeleteRow; 

	
	status = pTSDeleteRow->Initialize(
		this,
		tableName,
		KeyField,
		pKeyFieldValue,
		sizeof(rowID),
		0,						// no of rows to delete
		&pContext->value,		// rows deleted
		(pTSCallback_t)&TableServices::TableServiceOperationsReplyHandler,
		pContext
	);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDeleteRow->Send();
	return status;
}


//*********************************************************
//	TableServiceModifyField
//
//********************************************************
STATUS TableServices::
TableServiceModifyField(
			String64			tableName,
			rowID				*pRowIdToModify,
			String64			fieldNameToModify,
			void				*pNewFieldValue,		
			U32					sizeofNewFieldValue,	
			pTSCallback_t		cb,
			void				*pOriginalContext)
{
	STATUS					status = 0;
	TSModifyField			*pModifyField = NULL;
	TABLE_CONTEXT			*pContext = NULL;

	pContext = new TABLE_CONTEXT;
	pContext->pCallback = cb;
	pContext->pParentContext = pOriginalContext;
	pContext->pData = new char[sizeofNewFieldValue];
	memcpy(pContext->pData, pNewFieldValue, sizeofNewFieldValue);

	pModifyField = new TSModifyField;
	status = pModifyField->Initialize(
		this,
		tableName,
		CT_PTS_RID_FIELD_NAME,		// use the row id as the key
		pRowIdToModify,				//key
		sizeof(rowID),				// key size
		fieldNameToModify,			// field name of field to be modifiied
		pContext->pData,			// new field value
		sizeofNewFieldValue,		// size of field
		1,							// no of rows to modify
		&pContext->value,			// no of rows actually modified
		&pContext->newRowId,		// returned row id
		sizeof(rowID),				//
		(pTSCallback_t)&TableServices::TableServiceOperationsReplyHandler,
		pContext
	);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pModifyField->Send();
	return status;
}


//*********************************************************
//	TableServiceOperationsReplyHandler
//		Call the users callback with the original context 
//		and status
//
//********************************************************
STATUS TableServices::
TableServiceOperationsReplyHandler(void *_pContext, STATUS status)
{
	TABLE_CONTEXT		*pOriginalContext = NULL;
	pTSCallback_t		cb;

	TABLE_CONTEXT		*pContext = (TABLE_CONTEXT *)_pContext;

	pOriginalContext = (TABLE_CONTEXT *)pContext->pParentContext;
	cb = (pTSCallback_t)pContext->pCallback,	// callback
	(m_pParentDdm->*cb)(pOriginalContext,status);
	delete pContext;
	pContext = NULL;
	return status;
}


