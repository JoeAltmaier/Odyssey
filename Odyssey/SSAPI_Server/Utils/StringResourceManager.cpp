//************************************************************************
// FILE:		StringResourceManager.cpp
//
// PURPOSE:		Implements the object that can intellegently manage string 
//				resources. The object will store strings so that they
//				use up the least amount of space possible.
//************************************************************************


#include "StringResourceManager.h"
#include "SSAPITypes.h"
#include "UnicodeString16Table.h"
#include "UnicodeString32Table.h"
#include "UnicodeString64Table.h"
#include "UnicodeString128Table.h"
#include "UnicodeString256Table.h"


SRM_TABLE_JUMP_TABLE	jumpTable[] = {
	{	
		TABLE_16,
		"UnicodeString16Table",
		UnicodeString16Table_FieldDefs,
		cbUnicodeString16Table_FieldDefs,
		sizeof(UnicodeString16Record),
		32,
		UNICODE_STRING_16TABLE_VERSION,
	},
	{	
		TABLE_32,
		"UnicodeString32Table",
		UnicodeString32Table_FieldDefs,
		cbUnicodeString32Table_FieldDefs,
		sizeof(UnicodeString32Record),
		64,
		UNICODE_STRING_32TABLE_VERSION,
	},
	{	
		TABLE_64,
		"UnicodeString64Table",
		UnicodeString64Table_FieldDefs,
		cbUnicodeString64Table_FieldDefs,
		sizeof(UnicodeString64Record),
		128,
		UNICODE_STRING_64TABLE_VERSION,
	},
	{	
		TABLE_128,
		"UnicodeString128Table",
		UnicodeString128Table_FieldDefs,
		cbUnicodeString128Table_FieldDefs,
		sizeof(UnicodeString128Record),
		256,
		UNICODE_STRING_128TABLE_VERSION,
	},
	{	
		TABLE_256,
		"UnicodeString256Table",
		UnicodeString256Table_FieldDefs,
		cbUnicodeString256Table_FieldDefs,
		sizeof(UnicodeString256Record),
		512,
		UNICODE_STRING_256TABLE_VERSION,
	},

};

//************************************************************************
// StringResourceManager:
//
// PURPOSE:		Default constructor
//
// RECEIVE:		pParent:				DDM to use for message sending/receiving
//				pObjectReadyCallback:	Method to be called when the manager
//										is ready.
//************************************************************************

StringResourceManager::StringResourceManager( DdmServices *pParent, pTSCallback_t pObjectReadyCallback )
					  :DdmServices( pParent ){
	

	m_isInited				= false;
	m_pTables				= new P_ShadowTable [ NUMBER_OF_TABLES ];
	m_pInitCompleteCallback	= pObjectReadyCallback;

	for( U32 i = 0; i < NUMBER_OF_TABLES; i++ ){
		m_pTables[ i ] = new ShadowTable(	jumpTable[ i ].name,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StringResourceManager, DummyCallbackU32),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StringResourceManager, DummyCallbackU32),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StringResourceManager, DummyCallbackU32),
											jumpTable[ i ].bytesPerRow );

		m_pTables[ i ]->DefineTable(	jumpTable[ i ].tableDef,
										jumpTable[ i ].tableDefSize,
										64,
										(pTSCallback_t)METHOD_ADDRESS( StringResourceManager, DummyCallback ), 
										NULL );

		m_pTables[ i ]->Initialize(		(pTSCallback_t)METHOD_ADDRESS( StringResourceManager, InitializeTableCallback ),
										(void *) ((i == NUMBER_OF_TABLES - 1)? 1 : 0) );
	}
}


//************************************************************************
// ~StringResourceManager:
//
// PURPOSE:		The destructor
//************************************************************************

StringResourceManager::~StringResourceManager(){

	for( U32 i = 0; i < NUMBER_OF_TABLES; i++ )
		delete m_pTables[ i ];

	delete [] m_pTables;

}


//************************************************************************
// WriteString: (multiple overloads)
//
// PURPOSE:		Inserts a string into the PTS. The table used depends
//				on the string size
//	
// RECEIVE:		string:					the resource in different types
//				pCompletetionCallback	the method called when the operation
//										has completed.
//				*pRid					ptr to location where the row id
//										of the string will be put on completion
//				pContext:				any context
//			
// RETURN:		true:					SEND() was successful
//************************************************************************

bool 
StringResourceManager::WriteString(	UnicodeString	string,	 
									rowID			*pRid,
									pTSCallback_t	pCompletetionCallback,
									void			*pContext ){

	void		*pData = new char[ string.GetSize() ];
	bool		rc;

	string.CString( pData, string.GetSize() );

	rc = WriteDataToTable( pData, string.GetSize(), pRid, pCompletetionCallback, pContext );

	delete pData;
	return rc;
}


bool 
StringResourceManager::WriteString(	StringClass		string,	
									rowID			*pRid,
									pTSCallback_t	pCompletetionCallback,
									void			*pContext ){

	void	*pData = string.CString();
	bool	rc;	
	
	rc = WriteDataToTable( pData, string.SLength() + 1, pRid, pCompletetionCallback, pContext );

	delete pData;
	return rc;
}


//************************************************************************
// WriteDataToTable:
//
// PURPOSE:		Performs the actual write to a table. Selects the right
//				table based on the data size.
//************************************************************************

bool 
StringResourceManager::WriteDataToTable(	void			*pData, 
											U32				dataSize,
											rowID			*pRid, 
											pTSCallback_t	pCompletionCallback,
											void			*pContext ){

	ShadowTable						*pTable = NULL;
	PTS_TRANSACTION_WRITE_CONTEXT	*pCell;
	STATUS							rc;
	void							*pRow;

	// determine which table to use
	for( U32 i = 0; i < NUMBER_OF_TABLES; i++ ){
		if( jumpTable[ i ].maxResourceSize >= dataSize ){
			pTable = m_pTables[ i ];
			pRow = new char[jumpTable[ i ].bytesPerRow];
			((UnicodeString16Record *)pRow)->size = jumpTable[ i ].bytesPerRow;
			((UnicodeString16Record *)pRow)->version = jumpTable[ i ].tableVersion;
			memcpy( &((UnicodeString16Record *)pRow)->string, pData, dataSize );
			break;
		}
	}

	if( !pTable ) // the string's toooooooo long!
		return false;

	pCell = new PTS_TRANSACTION_WRITE_CONTEXT;
	pCell->pContext = pContext;
	pCell->pCompletionCallback = pCompletionCallback;

	rc = pTable->InsertRow(	pRow, 
							pRid, 
							(pTSCallback_t)METHOD_ADDRESS(StringResourceManager, WriteDataToTableReplyCallback), 
							pCell );

	delete pRow;

	return (rc == OK)? true : false;
}


//************************************************************************
// WriteDataToTableReplyCallback:
//
// PURPOSE:		Handles PTS reply and responds to the user
//************************************************************************

STATUS 
StringResourceManager::WriteDataToTableReplyCallback( void *pContext, STATUS status ){

	PTS_TRANSACTION_WRITE_CONTEXT	*pCell = (PTS_TRANSACTION_WRITE_CONTEXT *)pContext;

	(pParentDdmSvs->*(pCell->pCompletionCallback)) ( pCell->pContext, status );

	delete pCell;

	return OK;
}


//************************************************************************
// ReadString:	(multiple overloads)
//
// PURPOSE:		Reads a string from a PTS table and stores it in the format
//				requested
//
// RECEIVE:		string:					string resource
//				pCompletionCallback:	method called on completion
//				rid:					row id of the strig resource
//				pContext:				any context				
//************************************************************************

bool 
StringResourceManager::ReadString(	UnicodeString	*pString,
									RowId			rid,
									pTSCallback_t	pCompletionCallback,
									void			*pContext ){

	PTS_TRANSACTION_READ_CONTEXT	*pCell = new PTS_TRANSACTION_READ_CONTEXT;
	bool							rc;

	pCell->pStringObj			= (void *)pString;
	pCell->dataType				= SRM_UNICODE_STRING;
	pCell->pContext				= pContext;
	pCell->pCompletionCallback	= pCompletionCallback;

	rc = ReadDataFromTable( pCell, rid );

	if( !rc )
		delete pCell;

	return rc;
}
	

bool 
StringResourceManager::ReadString(	StringClass		*pString,
									RowId			rid,
									pTSCallback_t	pCompletionCallback,
									void			*pContext ){

	PTS_TRANSACTION_READ_CONTEXT	*pCell = new PTS_TRANSACTION_READ_CONTEXT;
	bool							rc;

	pCell->pStringObj			= (void *)pString;
	pCell->dataType				= SRM_ASCII_STRING;
	pCell->pContext				= pContext;
	pCell->pCompletionCallback	= pCompletionCallback;

	rc = ReadDataFromTable( pCell, rid );

	if( !rc )
		delete pCell;

	return rc;
}


//************************************************************************
// ReadDataFromTable:
//
// PURPOSE:		Does an actual read from a PTS table. The table to use
//				is determined by the row id of the string resource to read.
//************************************************************************

bool 
StringResourceManager::ReadDataFromTable(	PTS_TRANSACTION_READ_CONTEXT *pContext,	RowId rid ){

	ShadowTable			*pTable = NULL;
	STATUS				rc;

	// first, determine which table to read
	for( U32 i = 0; i < NUMBER_OF_TABLES; i++ ){
		if( m_pTables[ i ]->GetTableRowId().Table == rid.Table ){
			pTable = m_pTables[ i ];
			break;
		}
	}

	if( !pTable )	// ooooops, we never wrote this string!!!!
		return false;
	
	rc = pTable->ReadRow(	rid, 
							&pContext->pRowData, 
							(pTSCallback_t)METHOD_ADDRESS(StringResourceManager, ReadDataFromTableReplyCallback),
							(void *)pContext );

	return (rc == OK)? true : false;
}


//************************************************************************
// ReadDataFromTableReplyCallback:
//
// PURPOSE:		Handles PTS reply and responds to the user
//************************************************************************

STATUS 
StringResourceManager::ReadDataFromTableReplyCallback( void *pContext, STATUS status ){

	PTS_TRANSACTION_READ_CONTEXT	*pCell = (PTS_TRANSACTION_READ_CONTEXT *)pContext;
	UnicodeString16Record			*pRow  = (UnicodeString16Record *)pCell->pRowData;

	if( pCell->dataType == SRM_UNICODE_STRING ){
		UnicodeString	*pS = (UnicodeString *)pCell->pStringObj;
		*pS = UnicodeString( (void *) &pRow->string );
	}
	else if( pCell->dataType == SRM_ASCII_STRING ){
		StringClass	*pS = (StringClass *)pCell->pStringObj;
		*pS = StringClass( (char *)pRow->string );
	}

	(pParentDdmSvs->*(pCell->pCompletionCallback)) ( pCell->pContext, status );
	
	delete pCell->pRowData;
	delete pCell;
	return OK;
}


//************************************************************************
// InitializeTableCallback:
//
// PURPOSE:		Called when a shadow table object is inited
//************************************************************************

STATUS 
StringResourceManager::InitializeTableCallback( void *pContext, STATUS status ){

	m_isInited = ((int)pContext == 1 )? true : false;

	if( m_isInited )
		(pParentDdmSvs->*m_pInitCompleteCallback) ( NULL, OK );

	return OK;
}


//************************************************************************
// DeleteString:
//
// PURPOSE:		Deletes the string resource identified by the 'rid'
//
// RETURN:		true:		PTS Send() succeeded
//************************************************************************

bool 
StringResourceManager::DeleteString(	RowId			rid,
										pTSCallback_t	pCompletionCallback,
										void			*pContext ){

	ShadowTable						*pTable = NULL;
	STATUS							rc;
	PTS_TRANSACTION_DELETE_CONTEXT	*pCell;

	// first, find the table
	for( U32 i = 0; i < NUMBER_OF_TABLES; i++ ){
		if( m_pTables[ i ]->GetTableRowId().Table == rid.Table ){
			pTable = m_pTables[ i ];
			break;
		}
	}

	if( !pTable )	// oooops!!!
		return false;

	pCell = new PTS_TRANSACTION_DELETE_CONTEXT;
	pCell->pContext = pContext;
	pCell->pCompletionCallback = pCompletionCallback;

	rc = pTable->DeleteRow(	rid, 
							(pTSCallback_t)METHOD_ADDRESS(StringResourceManager, DeleteStringReplyCallback),
							(void *)pCell );

	return (rc == OK)? true : false;
}


//************************************************************************
// DeleteStringReplyCallback:
//
// PURPOSE:		Handles PTS reply and responds to the user
//************************************************************************

STATUS 
StringResourceManager::DeleteStringReplyCallback( void *pContext, STATUS rc ){
	
	PTS_TRANSACTION_DELETE_CONTEXT	*pCell = (PTS_TRANSACTION_DELETE_CONTEXT *)pContext;

	(pParentDdmSvs->*(pCell->pCompletionCallback)) ( pCell->pContext, rc );

	delete pCell;
	return OK;
}