//************************************************************************
// FILE:		ShadowTable.cpp
//
// PURPOSE:		Defines a very cute object that wraps up a PTS table 
//				and reduces the number of PTS listeners we
//				will need and makes PTS interface easier. Essentially,
//				three listeners are registered for all tables:
//					-- Add any row
//					-- Delete any row
//					-- Modify any row
//
// NOTES:		1. Once the table object is instantiated, it must be either 
//				   Initialize()'d or Create()'d so that it has all the infor 
//				   about the table it will be working closely with.
//************************************************************************


#include "ShadowTable.h"
#include "Message.h"
#include "SList.h"
#include "Rows.h"
#include "Fields.h"
#include "SSAPITypes.h"
#include "PtsCommon.h"
#include "CtEvent.h"

#define		NUMBER_ROWS_PADDED						1	// in case we miss row added events in the beginning
#define		MX_NUMBER_OF_ROW_INSERTED_AT_A_TIME		1	// this had better be one, but just in case
#define		MX_NUMBER_OF_COLUMNS					CMAXENTRIES

//************************************************************************
// ShadowTable:
// 
// PURPOSE:		Default destructor
//
// RECEIVE:		tableName:			name of the table
//				pParentDdm:			ptr to the DDM that owns this object
//				insertRowCallback:	method to call when a row is inserted
//				deleteRowCallback:	method to call when a row is deleted
//				modifyRowCallback:	method to call when a row is modified
//************************************************************************

ShadowTable::ShadowTable(StringClass			tableName, 
						 DdmServices			*pParentDdm, 
						 SHADOW_LISTEN_CALLBACK	insertRowCallback, 
						 SHADOW_LISTEN_CALLBACK	deleteRowCallback,
						 SHADOW_LISTEN_CALLBACK	modifyRowCallback,
						 U32					bytesPerRow) : DdmServices( pParentDdm ){
	
	m_isInited			= false;
	m_tableName			= tableName;
	m_pParentDdm		= pParentDdm;	
	m_insertRowCallback	= insertRowCallback;
	m_deleteRowCallback	= deleteRowCallback;
	m_modifyRowCallback	= modifyRowCallback;
	m_pModifiedRecord	= NULL;
	m_numberOfCols		= 0;
	m_numberOfRows		= 0;
	m_bytesPerRow		= bytesPerRow;
	m_pFieldDef			= NULL;
	m_pTableListenerObj	= NULL;
	
}


//************************************************************************
// ~ShadowTable
//
// PURPOSE:		Destructor. Do clean-ups
//************************************************************************

ShadowTable::~ShadowTable(){
	
	if( m_pTableListenerObj )
		m_pTableListenerObj->Stop();
	delete m_pModifiedRecord;
	delete []m_pFieldDef;
}


//************************************************************************
// Initialize:
//
// PURPOSE:		Initializes  the object. MUST be called (or Create() )
//				The callback will signal completion. Check the "rc" for
//				OK or !OK.
//
// FUNCTIONALITY:
//				1. Read table definition
//				2. Init insert, delete, and modify listeners
//				3. Return to the callback to indicate we're done 
//************************************************************************

STATUS 
ShadowTable::Initialize( pTSCallback_t pCallback, void *pCallersContext ){

	if( m_isInited )
		return !OK;

	STATUS				rc;
	String64			name;
	CONTEXT				*pContext = new CONTEXT;
	TSListen			*pObjI = m_pTableListenerObj = new TSListen();
	TSGetTableDef		*pObj = new TSGetTableDef;


	// first create a listener 
	pContext->state				= INIT_LISTENERS; 
	pContext->pCallersContext	= pCallersContext;			
	pContext->pCallback			= pCallback;
	m_sizeOfModifiedRecord		= MX_NUMBER_OF_ROW_INSERTED_AT_A_TIME * m_bytesPerRow;
	m_pModifiedRecord			= new char[m_sizeOfModifiedRecord];
	rc = PopulateAddListenerObject( pObjI, ListenOnInsertRow |  ListenOnDeleteAnyRow | ListenOnModifyAnyRowAnyField , &m_listener, pContext ); 
	if( rc != OK ){
		Trace((StringClass)"Couldn't init listeners", ST_CRITICAL );
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc );
		return !OK;
	}
	// send this dude!
	pObjI->Send();
	

	// now get table def	
	pContext = new CONTEXT;
	pContext->state				= READ_TABLE_DEF;
	pContext->pCallersContext	= pCallersContext;			
	pContext->pCallback			= pCallback;
	pContext->pNumOfCols		= new U32;
	pContext->pBytesPerRow		= new U32;
	pContext->pNumOfRows		= new U32;
	pContext->pFieldDefSize		= new U32;
	pContext->pFieldDef			= new fieldDef[ MX_NUMBER_OF_COLUMNS ];
	
	m_tableName.CString( (char *)&name, sizeof( name ) );
	
	rc = pObj->Initialize(	this,
							name,
							(fieldDef *)pContext->pFieldDef,	
							sizeof(fieldDef) * MX_NUMBER_OF_COLUMNS,
							pContext->pFieldDefSize,
							pContext->pBytesPerRow,
							pContext->pNumOfRows,
							pContext->pNumOfCols,
							NULL,
							(pTSCallback_t)&ShadowTable::InitializeReplyHandler,
							pContext );
	if( rc != OK ){
		delete pContext;
		delete pObj;
		return rc;
	}			  	
	
	pObj->Send();

	return OK;
}


//************************************************************************
// InitializeReplyHandler:
//
// PURPOSE:		Responsible for handling reply from Initialize() and itself
//************************************************************************

STATUS
ShadowTable::InitializeReplyHandler( void *pContext_, STATUS rc ){
	
	CONTEXT				*pContext = (CONTEXT *)pContext_;
	
	switch( pContext->state ) {
		case READ_TABLE_DEF:	
			// collect data and do clean-up
			if( rc != OK ){
				Trace( (StringClass)"GetTableDef failed", ST_CRITICAL );	
				(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc );
				delete pContext->pNumOfCols;
				delete pContext->pBytesPerRow;
				delete pContext->pNumOfRows;
				delete pContext->pFieldDef;
				delete pContext->pFieldDefSize;
				delete pContext;
				LogFailure( "ReadTableDef", rc );
				break;
			}
	
			m_numberOfCols	= *pContext->pNumOfCols;
			delete pContext->pNumOfCols;
			m_bytesPerRow = *pContext->pBytesPerRow;
			delete pContext->pBytesPerRow;
			m_numberOfRows	= *pContext->pNumOfRows;
			delete pContext->pNumOfRows;

			m_pFieldDef = new fieldDef[ m_numberOfCols ];
			memcpy( m_pFieldDef, pContext->pFieldDef, *pContext->pFieldDefSize );
			delete pContext->pFieldDef;
			delete pContext->pFieldDefSize;

			// callback to report the GOOOOOOD news :-)
			m_isInited = true;
			(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, OK );

			delete pContext;

			break;
			
		case INIT_LISTENERS:

			if( !m_areListenersInited ){ 
				// report status
				if( rc != OK ){
					Trace( (StringClass)"Register listeners failed", ST_CRITICAL );	
					(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc );
					delete pContext;
					LogFailure("RegisterListener", rc );
					break;
				}
						
				// do final clean-up
				m_areListenersInited = true;
				break;
			}
			else
				;
		default:	// PTS listen reply

		// TBDGAI:
		// this whole thing should be redone on real system.
		// make sure the memory is allocated by the transport, not me
		// and calculate the number of rows affected, now it's hardcoded to 1.
		if( *m_pListenTypeRet & ListenOnInsertRow ){	
			if( m_isInited )
				m_numberOfRows++;		
			(m_pParentDdm->*m_insertRowCallback)( m_pModifiedRecord, 1, this);
		}
		else if( *m_pListenTypeRet & ListenOnDeleteAnyRow ){
			if( m_isInited )
				m_numberOfRows--;
			(m_pParentDdm->*m_deleteRowCallback)( m_pModifiedRecord, 1, this);
		}
		else if( *m_pListenTypeRet & ListenOnModifyAnyRowAnyField ){
			m_numberOfRows = m_numberOfRows;
#if 0
			// find the new rows by the row ids of the old ones and return them\
			// TBDGAI: 1 is a hack now! But NOT the 1 when we do callback, this one is correct,
			// we do one at a time on this listen type
			rowID	*pOldId = (rowID *)m_pModifiedRecord, *pNewId;
			for( U32 ridNum = 0; ridNum < 1; ridNum++, pOldId = (rowID *)((char *)pOldId + m_bytesPerRow ) ){
				pNewId = (rowID *)m_pTableDataRet;
				for( U32  i = 0; i < m_sizeOfTableDataRet / m_bytesPerRow; i++, pNewId = (rowID *)( (char *)pNewId + m_bytesPerRow ) ){
					if( RowId(*pNewId) == RowId(*pOldId) )
					
				}
			}
#else
			(m_pParentDdm->*m_modifyRowCallback)( (void *)m_pModifiedRecord, 1, this);
#endif
			
#ifndef WIN32
			// I had problems with this....it seems that there some overlap 
			// between this memory and memory allocated for objects elsewhere...
			// let's see if this is present on the eval or...if I van survive without using it.
			delete m_pTableDataRet;
#endif
		}
		break;	

	}	

	return OK;
}


//************************************************************************
// DefineTable:
//
// PURPOSE:		Issues a request to define a table
//
// RECEIVE:		pDef:		field defintion of the table
//				sizeOfDef:	size of the defintion
//				pCallback:	routine to call back to
//				pContext:	for caller's context
//************************************************************************

STATUS 
ShadowTable::DefineTable(	fieldDef *pDef,				U32 sizeOfDef, U32 entriesToReserve,
							pTSCallback_t pCallback,	void *pCallersContext ) {

	CONTEXT					*pContext	= new CONTEXT;
	TSDefineTable			*pObj		= new TSDefineTable();
	STATUS					rc;
	String64				name;

	m_tableName.CString( (char *)&name, sizeof( name ) );

	pContext->state				= DEFINE_TABLE;
	pContext->pCallersContext	= pCallersContext;
	pContext->pCallback			= pCallback;
	m_pDefineTableObject		= pObj;

	rc = pObj->Initialize(	this,
							name,
							pDef,
							sizeOfDef,
							entriesToReserve,
							true,
							(pTSCallback_t)&ShadowTable::DefineTableReplyHandler,
							pContext );
	if( rc != OK ){
		delete pContext;
		delete pObj;
		return !OK;
	}

	pObj->Send();

	return OK;
}


//************************************************************************
// DefineTableReplyHandler:
//
// PURPOSE:		Resoponsible for the handling if reply to define a table
//************************************************************************

STATUS 
ShadowTable::DefineTableReplyHandler( void *pContext_, STATUS rc ){

	CONTEXT		*pContext = (CONTEXT *)pContext_;

	m_tableRowId	=	m_pDefineTableObject->GetTableID();

	if( rc != OK ){
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
		//LogFailure("DefineTable", rc );
	}
	else {
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
	}
	delete pContext;

	return OK;
}

//************************************************************************
// InsertRow:
//
// PURPOSE:		Inserts a row into the table
//
// RECEIVE:		pRow:		ptr to the buffer representing a row
//				pRowId:		ptr of the cell where rowId will be put
//
//************************************************************************

STATUS 
ShadowTable::InsertRow( void *pRow,					rowID *pRowId, 
						pTSCallback_t pCallback,	void  *pCallersContext ){

	if( !m_isInited )
		return !OK;

	CONTEXT				*pContext = new CONTEXT;
	char				*pName = m_tableName.CString();						
	TSInsertRow			*pObj = new TSInsertRow();
	STATUS				rc;
	
	pContext->state				= INSERT_ROW;
	pContext->pCallersContext	= pCallersContext;			
	pContext->pCallback			= pCallback;
	pContext->pRow				= new char[m_bytesPerRow];
	memcpy( pContext->pRow, pRow, m_bytesPerRow );

	rc = pObj->Initialize(	this,
							pName,
							pContext->pRow,
							m_bytesPerRow,
							pRowId,
							(pTSCallback_t)&ShadowTable::InsertRowReplyHandler,
							pContext );
	delete pName;

	if( rc != OK ){
		delete pContext;
		return !OK;
	}
	
	pObj->Send();

	return OK;
}


//************************************************************************
// InsertRowReplyHandler:
//
// PURPOSE:		Responsible for handling replies to InsertRow operation
//************************************************************************

STATUS 
ShadowTable::InsertRowReplyHandler( void *pContext_, STATUS rc ){

	CONTEXT			*pContext	= (CONTEXT *)pContext_;
	
	if( rc != OK ){
		Trace( (StringClass)"\nInsert_Row failed", ST_CRITICAL );
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
		LogFailure("InsertRow", rc );
	}
	else {
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
	}
	delete pContext->pRow;
	delete pContext;

	return OK;
}


//************************************************************************
// DeleteRow:
//
// PURPOSE:		Deletes row from the table. 
//
// RECEIVE:		rowId:	row id of the row to be deleted
//************************************************************************

STATUS 
ShadowTable::DeleteRow( rowID rowId, pTSCallback_t pCallback, void *pCallersContext){

	if( !m_isInited )
		return !OK;

	CONTEXT				*pContext = new CONTEXT;
	char				*pName = m_tableName.CString();						
	TSDeleteRow			*pObj = new TSDeleteRow();
	STATUS				rc;
	
	pContext->state				= DELETE_ROW;
	pContext->pCallersContext	= pCallersContext;			
	pContext->pCallback			= pCallback;
	pContext->pRowId			= new rowID;
	*pContext->pRowId			= rowId;

	rc = pObj->Initialize(	this,
							pName,
							CT_PTS_RID_FIELD_NAME,
							pContext->pRowId,
							sizeof(rowID),
							1,
							NULL,
							(pTSCallback_t)&ShadowTable::DeleteRowReplyHandler,
							pContext );
	delete pName;

	if( rc != OK ){
		delete pContext;
		return !OK;
	}
	
	pObj->Send();

	return OK;
}



//************************************************************************
// DeleteRowReplyHandler:
//
// PURPOSE:		Responsible for handling reply to DeleteRow cmd
//************************************************************************

STATUS 
ShadowTable::DeleteRowReplyHandler( void *pContext_, STATUS rc ){

	CONTEXT			*pContext	= (CONTEXT *)pContext_;
	
	if( rc != OK ){
		Trace( (StringClass)"\nDelete_Row failed", ST_CRITICAL );
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
		LogFailure("DeleteRow", rc );
	}
	else {
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
	}
	delete pContext->pRowId;
	delete pContext;

	return OK;
}


//************************************************************************
// DeleteTable:
//
// PURPOSE:		Deletes the table from the PTS
//************************************************************************

void 
ShadowTable::DeleteTable( pTSCallback_t pCallback, void *pContext_ ){

	TSDeleteTable	*pObj = new TSDeleteTable;
	char			*pName = m_tableName.CString();
	CONTEXT			*pContext = new CONTEXT;
	RowId			rid;

	pContext->pCallersContext	= pContext_;			
	pContext->pCallback			= pCallback;

	pObj->Initialize(	this,
						pName,
						rid,
						(pTSCallback_t)&ShadowTable::DeleteTableReplyHandler,
						pContext );

	pObj->Send();

	delete pName;
}


//************************************************************************
// DeleteTableReplyHandler:
//
// PURPOSE:		Responsible for handling a callback to the DeleteTable()
//************************************************************************

STATUS 
ShadowTable::DeleteTableReplyHandler( void *pContext_, STATUS rc ){

	CONTEXT			*pContext	= (CONTEXT *)pContext_;

	if( rc != OK ){
		Trace( (StringClass)"\nDelete_Table failed", ST_CRITICAL );
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
		LogFailure("DeleteTable", rc );
	}
	else {
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
	}

	delete pContext;

	return OK;
}

//************************************************************************
// ModifyRow:
//
// PURPOSE:		Modifes a row.
//
// RECEIVE:		rowId:		the row id of the row to modify
//				pRow:		ptr to memory that contains new row data 
//************************************************************************

STATUS 
ShadowTable::ModifyRow( rowID rowId, void* pRow,  pTSCallback_t pCallback, void *pCallersContext ){

	CONTEXT				*pContext = new CONTEXT;
	char				*pName = m_tableName.CString();						
	TSModifyRow			*pObj = new TSModifyRow();
	STATUS				rc;
	
	pContext->state				= MODIFY_ROW;
	pContext->pCallersContext	= pCallersContext;			
	pContext->pCallback			= pCallback;
	pContext->pRowId			= new rowID;
	pContext->pRow				= new char[m_bytesPerRow];

	memcpy( pContext->pRow, pRow, m_bytesPerRow );
	*pContext->pRowId			= rowId;

	rc = pObj->Initialize(	this,
							pName,
							CT_PTS_RID_FIELD_NAME,
							pContext->pRowId,
							sizeof(rowID),
							pContext->pRow,
							m_bytesPerRow,
							1,
							NULL,
							(rowID*)&m_tempRowId,
							sizeof(rowID),
							(pTSCallback_t)&ShadowTable::ModifyRowReplyHandler,
							 pContext );
	delete pName;

	if( rc != OK ){
		delete pContext->pRow;
		delete pContext->pRowId;
		delete pContext;
		return !OK;
	}
	
	pObj->Send();

	return OK;
}



//************************************************************************
// ModifyRowReplyHandler:
//
// PURPOSE:		Responsible for handling replies to ModifyRow()
//************************************************************************

STATUS 
ShadowTable::ModifyRowReplyHandler( void *pContext_, STATUS rc ){

	CONTEXT			*pContext	= (CONTEXT *)pContext_;
	
	if( rc != OK ){
		Trace( (StringClass)"Modify_Row failed", ST_CRITICAL );
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
		LogFailure("ModifyRow", rc );
	}
	else {
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
	}
	delete pContext->pRow;
	delete pContext->pRowId;
	delete pContext;

	return OK;
}



//************************************************************************
// Enumerate:
//
// PURPOSE:		Reads the table into buffer specified
//
// RECEIVE:		pTableData:	address of the pointer that will point to
//							the memory with table data.
//
// NOTE:		the caller must delete 'ppTableData' !!!
//************************************************************************

STATUS 
ShadowTable::Enumerate( void **ppTableData, pTSCallback_t pCallback, void *pCallersContext ){

	if( !m_isInited )
		return !OK;

	CONTEXT				*pContext = new CONTEXT;
	char				*pName = m_tableName.CString();						
	TSEnumTable			*pObj = new TSEnumTable();
	STATUS				rc;
	
	pContext->state				= READ_TABLE;
	pContext->pCallersContext	= pCallersContext;			
	pContext->pCallback			= pCallback;
	*ppTableData = pContext->pTableData		= new char[(m_numberOfRows + NUMBER_ROWS_PADDED) * m_bytesPerRow];
	memset( pContext->pTableData, 0, m_numberOfRows * m_bytesPerRow );

	rc = pObj->Initialize(	this,
							pName,
							0,		// start row
							pContext->pTableData,
							(m_numberOfRows + NUMBER_ROWS_PADDED) * m_bytesPerRow,
							&m_sizeOfTableDataRet,
							(pTSCallback_t)&ShadowTable::EnumerateReplyHandler,
							pContext );
	delete pName;

	if( rc != OK ){
		delete pContext;
		return !OK;
	}
	
	pObj->Send();

	return OK;
}



//************************************************************************
// EnumerateReplyHandler:
//
// PURPOSE:		REsponsible for handling the reply to Enumerate()
//************************************************************************

STATUS 
ShadowTable::EnumerateReplyHandler( void *pContext_, STATUS rc ){

	CONTEXT			*pContext	= (CONTEXT *)pContext_;
	
	if( rc != OK ){
		Trace( (StringClass)"\nEnumerate failed", ST_CRITICAL );
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
		LogFailure("EnumerateTable", rc );
	}
	else {
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
	}
	delete pContext;

	return OK;
}


//************************************************************************
// ReadRow:
//
// PURPOSE:		Read a row
//
// RECEIVE:		rowId:		the id of the row to read
//				ppRow:		address of the pointer to a read row. allocated 
//							by the callee
//	
// NOTE:		ppRow must be delete()d after the callback returns.	
//************************************************************************

STATUS 
ShadowTable::ReadRow( rowID rowId, void **ppRow, pTSCallback_t pCallback, void *pCallersContext ){

	CONTEXT				*pContext = new CONTEXT;
	char				*pName = m_tableName.CString();						
	TSReadRow			*pObj = new TSReadRow();
	STATUS				rc;
	
	pContext->state				= READ_ROW;
	pContext->pCallersContext	= pCallersContext;			
	pContext->pCallback			= pCallback;
	pContext->pRowId			= new rowID;
	*ppRow= pContext->pRow		= new char[m_bytesPerRow];

	*pContext->pRowId			= rowId;

	rc = pObj->Initialize(	this,
							pName,
							CT_PTS_RID_FIELD_NAME,
							pContext->pRowId,
							sizeof(rowID),
							pContext->pRow,
							m_bytesPerRow,
							NULL,
							(pTSCallback_t)&ShadowTable::ReadRowReplyHandler,
							 pContext );
	delete pName;

	if( rc != OK ){
		delete pContext->pRow;
		delete pContext->pRowId;
		delete pContext;
		return !OK;
	}
	
	pObj->Send();

	return OK;
}



//************************************************************************
// ReadRowReplyHandler:
//
// PURPOSE:		Responsible for handling replies to ReadRow()
//************************************************************************

STATUS 
ShadowTable::ReadRowReplyHandler( void *pContext_, STATUS rc ){

	CONTEXT			*pContext	= (CONTEXT *)pContext_;
	
	if( rc != OK ){
		Trace( (StringClass)"\nRead_Row failed", ST_CRITICAL );
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
		LogFailure("ReadRow", rc );
	}
	else {
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
	}
	delete pContext;

	return OK;
}


//************************************************************************
// PopulateAddListenerObject:
//
// PURPOSE:		Fills up a PTS add listener object by calling its
//				Initialize()
//
// RECEIVE:		pListener:		ptr to the object to populate
//				listenerType:	type as defined in the PTS API
//				pListenerId:	ptr to cell where the new listener id should be put
//************************************************************************

STATUS 
ShadowTable::PopulateAddListenerObject( TSListen *pListener, U32 	listenerType, 
										U32 *pListenerId,	 void   *pContext, pTSCallback_t pCallback){
										
	char	*pName = m_tableName.CString();
	STATUS	rc;
	m_pTableDataRet = NULL;
	m_sizeOfTableDataRet = 0;
	rc = pListener->Initialize(		this		,			// DdmServices*	pDdmServices
									listenerType,			// U32			ListenType
									pName,					// String64		prgbTableName
									NULL,					// String64		prgbRowKeyFieldName
									NULL,					// void*		prgbRowKeyFieldValue
									0,						// U32			cbRowKeyFieldValue
									NULL,					// String64		prgbFieldName
									NULL,					// void*		prgbFieldValue
									0,						// U32			cbFieldValue
									ReplyContinuous | ReplyWithRow,		// U32			ReplyMode
									NULL,//&m_pTableDataRet,		// void**		ppTableDataRet
									NULL, //&m_sizeOfTableDataRet,	// U32*			pcbTableDataRet
									pListenerId,			// U32*			pListenerIDRet
									(U32**)&m_pListenTypeRet,// U32**		ppListenTypeRet
									&m_pModifiedRecord,		// void**		ppModifiedRecordRet
									&m_sizeOfModifiedRecord,
									(pTSCallback_t)&ShadowTable::InitializeReplyHandler,
									pContext
								);
	delete pName;
	
	return rc;						
}


//************************************************************************
// ModifyField:
//
// PURPOSE:		Modifies a field in a row
//************************************************************************

STATUS 
ShadowTable::ModifyField(	RowId			rid, 
							char			*pFieldName, 
							void			*pNewValue,
							U32				newValueSize,
							pTSCallback_t	pCallback, 
							void			*pCallersContext ){

	TSModifyField			*pObj = new TSModifyField();
	STATUS					rc;
	char					*pName = GetName().CString();
	rowID					rid_ = rid.GetRowID();
	CONTEXT					*pContext = new CONTEXT;

	pContext->pCallersContext	= pCallersContext;			
	pContext->pCallback			= pCallback;
	pContext->pRowId			= new rowID;
	*pContext->pRowId			= rid_;
	pContext->pNewValue			= new char[newValueSize];

	memcpy( pContext->pNewValue, pNewValue, newValueSize );
	
	rc = pObj->Initialize(	this,
							pName,
							CT_PTS_RID_FIELD_NAME,
							pContext->pRowId,
							sizeof(rid_),
							pFieldName,
							pContext->pNewValue,
							newValueSize,
							1,
							NULL,
							(rowID*)&m_tempRowId,//pContext->pRowId,
							0,
							(pTSCallback_t)METHOD_ADDRESS( ShadowTable, ModifyFieldReplyHandler ),
							pContext );

	delete pName;

	if( rc != OK ){
		delete pObj;
		delete pContext;
		return rc;
	}

	pObj->Send();
	return OK;
}


//************************************************************************
// ModifyFieldReplyHandler:
//
// PURPOSE:		REsponsible for handlinmg the PTS reply and responding 
//				to user
//************************************************************************

STATUS 
ShadowTable::ModifyFieldReplyHandler( void *pContext_, STATUS rc  ){

	CONTEXT		*pContext = (CONTEXT *)pContext_;

	delete pContext->pRowId;
	delete pContext->pNewValue;

	if( rc != OK ){
		Trace( (StringClass)"\nModifyField failed", ST_CRITICAL );
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
		LogFailure("ModifyField", rc );
	}
	else {
		(m_pParentDdm->*(pContext->pCallback))( pContext->pCallersContext, rc);
	}
	delete pContext;
	return OK;
}


//************************************************************************
// LogFailure:
//
// PURPOSE:		Logs a failure in an operation as an internal event
//************************************************************************

void 
ShadowTable::LogFailure( StringClass opName, STATUS rc ){

	char	*pName = m_tableName.CString(),
			*pOp = opName.CString();

	LogEvent(	CTS_SSAPI_SHADOW_TABLE_OP_FAILURE_EVENT,
				pOp,
				pName,
				rc );

	delete [] pName;
	delete [] pOp;
}