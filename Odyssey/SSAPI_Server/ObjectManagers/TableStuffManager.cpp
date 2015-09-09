//************************************************************************
// FILE:		TableStuffManager.cpp
//
// PURPOSE:		An object manager that is responsible for the Rambo PnP
//************************************************************************


#include "TableStuffManager.h"
#include "ShadowTable.h"
#include "SsapiTypes.h"
#include "SList.h"
#include "ListenManager.h"
#include "Kernel.h"
// managed objects
#include "TableMetaData.h"
#include "TableStuff.h"
#include "TableStuffRow.h"

#define	MAX_TABLE_ROW_SIZE			4 * 1024

#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS

extern U32   acbDataTypeSize [] ;

TableStuffManager* TableStuffManager::m_pThis = NULL;

//************************************************************************
// 
//************************************************************************

TableStuffManager::TableStuffManager( ListenManager *pLM, DdmServices *pParent )
:ObjectManager( pLM, DesignatorId(RowId(), SSAPI_MANAGER_TYPE_TABLE_MANAGER ), pParent ){

	m_isIniting = true;
	m_shouldRebuild = false;
	SetIsReadyToServiceRequests( false );

	m_pTableOfTables = new ShadowTable(	CT_PTS_TABLE_OF_TABLES,
										this,
										(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(TableStuffManager, TableAdded),
										(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(TableStuffManager, TableRemoved),
										(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(TableStuffManager, TableModified),
										sizeof(rowDef) );

	m_pTableOfTables->Initialize( (pTSCallback_t)METHOD_ADDRESS(TableStuffManager, TableOfTablesInitialized), NULL );

	SSAPI_TRACE( TRACE_L2, "\nStarting 'Mr.Rambo-On-demand'" );
}



//************************************************************************
// 
//************************************************************************

STATUS 
TableStuffManager::TableOfTablesInitialized( void *pContext, STATUS rc ){

	m_pTableOfTables->Enumerate(	&m_pTempBuffer,
									(pTSCallback_t)METHOD_ADDRESS( TableStuffManager, TableOfTablesEnumerated ),
									NULL );

	return OK;
}


STATUS 
TableStuffManager::TableOfTablesEnumerated( void *pContext, STATUS rc ){

	TableMetaData			*pObj;
	CoolVector				container;
	U32						index;
	rowDef					*pRow = (rowDef *)m_pTempBuffer;

	if( m_shouldRebuild ){
		m_shouldRebuild = false;
		delete m_pTempBuffer;
		m_pTableOfTables->Enumerate(	&m_pTempBuffer,
										(pTSCallback_t)METHOD_ADDRESS( TableStuffManager, TableOfTablesEnumerated ),
										NULL );
		return OK;
	}
	
	pObj = new TableMetaData( GetListenManager() );
	for( index = 0; index < m_pTableOfTables->GetNumberOfRows(); index++, pRow++ ){
		// TBDGAI HACK!!!!!!!! Need dynamic SGLs for listen,. so row size is of o importance!
	//	AddNewTable(pRow->tableName, 
	//				MAX_TABLE_ROW_SIZE,
	//				(TMGR_CALLBACK)METHOD_ADDRESS(TableStuffManager, TableAddedWhileInitializing), 
	//				(void *) ((index == (m_pTableOfTables->GetNumberOfRows() - 1))? 1 : 0) ); 

		if( index == 0 )
			AddNewTable(pRow->tableName, 
						MAX_TABLE_ROW_SIZE,
						(TMGR_CALLBACK)METHOD_ADDRESS(TableStuffManager, TableAddedWhileInitializing), 
						(void *) index ); 
		
		pObj->AddTableInfoCell( pRow );	
		Kernel::Reschedule();
	}

	container.Add( (CONTAINER_ELEMENT) pObj );
	AddObjectsIntoManagedObjectsVector( container );

	return OK;

}

//************************************************************************
// 
//************************************************************************

TableStuffManager::~TableStuffManager(){

	delete m_pTableOfTables;
}


//************************************************************************
// Dispatch:
//
// PURPOSE:		Dispatches a request to whoever should service it.
//				
// RECEIVE:		requestParms:		value set with request parms
//				reuestCode:			code of the request
//				pResponder:			a wrapper object that knows how to 
//									respond
//
// NOTE:		All sub-classes that override this method MUST call it on
//				their superclass before processing!!! If this method 
//				returned false, only then should they tray to handle 
//				a request.
//************************************************************************

bool 
TableStuffManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){

	RowId					rid;
	DesignatorId			id;
	UnicodeString			s;
	TableStuffRow			*pRow;

	if( ObjectManager::Dispatch( pRequestParms, requestCode, pResponder ) )
		return true;

	switch( requestCode ){
		case SSAPI_TABLE_MANAGER_MODIFY_FIELD:
			pRequestParms->GetString( SSAPI_TABLE_MANAGER_MODIFY_FIELD_FIELD_NAME, &s );
			pRequestParms->GetRowID( SSAPI_TABLE_MANAGER_MODIFY_FIELD_ROW_ID, (char *)&rid );
			GetDesignatorIdByRowId( rid, id );
			if( !(pRow = (TableStuffRow *) GetManagedObject( &id )) )
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
			ModifyField( *pRow, s, *pRequestParms, pResponder );
			break;

		default:
			ASSERT(0);
			break;
	}

	return true;
}


//************************************************************************
// PTS callback for the PTS's "Table of tables"
//************************************************************************

STATUS 
TableStuffManager::TableAdded( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_shouldRebuild = true;
		return OK;
	}

	rowDef	*pRow = (rowDef *)pRows;
	U32		index;

	for( index = 0; index < numberOfRows; index++, pRow++ ){
		AddNewTable(pRow->tableName, 
					MAX_TABLE_ROW_SIZE,
					(TMGR_CALLBACK)METHOD_ADDRESS(TableStuffManager, DoneAddingNewTable), 
					NULL ); 
		
		GetTableMetaData()->AddTableInfoCell( pRow );	
		SetIsReadyToServiceRequests( false );
	}
	
	return OK;
}


//************************************************************************
//
//************************************************************************

void 
TableStuffManager::DoneAddingNewTable( void *pContext ){

	SetIsReadyToServiceRequests( true );
}

STATUS 
TableStuffManager::TableRemoved( void *pRows, U32 numberOfRows, ShadowTable *pTable ){

	if( m_isIniting ){
		m_shouldRebuild = true;
		return OK;
	}

	rowDef			*pRow = (rowDef *)pRows;
	U32				index, child;
	CoolVector		container;
	RowId			rid;
	DesignatorId	id;
	ManagedObject	*pObj, *pChild;

	for( index = 0; index < numberOfRows; index++, pRow++ ){
		// get the table object
		GetTableMetaData()->GetTableIdByTableName( pRow->tableName, rid );
		if( GetDesignatorIdByRowId( rid, id ) ){
			pObj = GetManagedObject( &id );
			container.Add( (CONTAINER_ELEMENT)pObj );
			// get children
			for( child = 0; child < pObj->GetChildCount(); child++ ){
				id = pObj->GetChildIdAt( child );
				pChild = GetManagedObject( &id );
				container.AddAt( (CONTAINER_ELEMENT)pChild, 0 );
			}
			// purge deleted objects
			DeleteObjectsFromTheSystem( container );

			// purge table info cell from the meta data object
			GetTableMetaData()->RemoveTableInfoCell( pRow, true );
		}
		else
			ASSERT(0);
	}

	return OK;
}


STATUS 
TableStuffManager::TableModified( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_shouldRebuild = true;
		return OK;
	}

	// no processing is necessary - nothing should happen
	ASSERT(0);
	return OK;
}


//************************************************************************
// PTS callback for the ALL tables. 
//
// NOTE:		Use the ptr to a ShadowTable object to determine which
//				table has been modified.
//************************************************************************

STATUS 
TableStuffManager::RowAdded( void *pRows, U32 numberOfRows, ShadowTable *pTable ){

	if( pTable == m_pTableBeingAdded ){
		m_wasTableUpdatedWhileAdding = true;
		return OK;
	}

	U32				index;
	char			*pCurr = (char *)pRows;
	TableStuff		*pTableObj;
	TableStuffRow	*pTableRow;
	RowId			rid;
	DesignatorId	id;
	CoolVector		container;
	
	GetTableMetaData()->GetTableIdByTableName( pTable->GetName(), rid );
	GetDesignatorIdByRowId( rid, id );
	pTableObj = (TableStuff *)GetManagedObject( &id );

	for( index = 0; index < numberOfRows; index++ ){
		pTableRow = new TableStuffRow(	GetListenManager(),
										this,
										pTable,
										pCurr,
										pTable->GetBytesPerRow() );

		container.Add( (CONTAINER_ELEMENT) pTableRow );
		pTableRow->AddParentId( pTableObj, false );
		AddObjectsIntoManagedObjectsVector( container );
		pTableObj->AddChildId( pTableRow, true );
		pCurr += pTable->GetBytesPerRow();
	}

	return OK;
}


STATUS 
TableStuffManager::RowRemoved( void *pRows, U32 numberOfRows, ShadowTable *pTable ){

	if( pTable == m_pTableBeingAdded ){
		m_wasTableUpdatedWhileAdding = true;
		return OK;
	}

	U32				index;
	char			*pCurr = (char *)pRows;
	TableStuffRow	*pTableRow;
	CoolVector		container;
	DesignatorId	id;
	TableStuff		*pTableObj;
	RowId			rid;

	GetTableMetaData()->GetTableIdByTableName( pTable->GetName(), rid );
	GetDesignatorIdByRowId( rid, id );
	pTableObj = (TableStuff *)GetManagedObject( &id );
	
	for( index = 0; index < numberOfRows; index++ ){
		this->GetDesignatorIdByRowId( *((RowId *)pCurr), id );
		pTableRow = (TableStuffRow *)GetManagedObject( &id );
		pCurr += pTable->GetBytesPerRow();
		container.Add( (CONTAINER_ELEMENT)pTableRow );
	}
	DeleteObjectsFromTheSystem( container );
	return OK;
}


STATUS 
TableStuffManager::RowModified( void *pRows, U32 numberOfRows, ShadowTable *pTable ){

	if( pTable == m_pTableBeingAdded ){
		m_wasTableUpdatedWhileAdding = true;
		return OK;
	}

	U32				index;
	char			*pCurr = (char *)pRows;
	TableStuffRow	*pTableRow;
	CoolVector		container;
	DesignatorId	id;
	
	for( index = 0; index < numberOfRows; index++ ){
		pTableRow = new TableStuffRow(	GetListenManager(),
										this,
										pTable,
										pCurr,
										pTable->GetBytesPerRow() );
		pCurr += pTable->GetBytesPerRow();
		RecoverParentIds( pTableRow );
		container.Add( (CONTAINER_ELEMENT)pTableRow );
	}
	
	AddObjectsIntoManagedObjectsVector( container );
	return OK;
}


//************************************************************************
//
//************************************************************************

void 
TableStuffManager::StuffContainerWithRowObjectsForTable( U16 tableId, Container &container ){

	ManagedObject		*pObj;
	U32					i, count;

	container.RemoveAll();
	for( i = count = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( pObj->GetClassType() == SSAPI_OBJECT_CLASS_TYPE_PTS_TABLE_ROW )
			if( pObj->GetDesignatorId().GetRowId().Table == tableId )
				container.Add( (CONTAINER_ELEMENT) pObj );
	}
}


//************************************************************************
// AddNewTable:
//************************************************************************

void 
TableStuffManager::AddNewTable( StringClass tableName, U32 rowSize, TMGR_CALLBACK pCallback, void *pContext ){

	ADD_NEW_TABLE_CELL	*pCell = new ADD_NEW_TABLE_CELL;

	pCell->pCompletionCallback	= pCallback;
	pCell->pContext				= pContext;
	pCell->pTable				= new ShadowTable(	tableName,
													this,
													(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(TableStuffManager, RowAdded ),
													(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(TableStuffManager, RowRemoved ),
													(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(TableStuffManager, RowModified ),
													rowSize );

	pCell->pTable->Initialize(	(pTSCallback_t)METHOD_ADDRESS( TableStuffManager, NewTableInitialized ),
								pCell );

	m_pTableBeingAdded = pCell->pTable;
	m_wasTableUpdatedWhileAdding = false;
}


STATUS 
TableStuffManager::NewTableInitialized( void *pContext, STATUS rc ){

	ADD_NEW_TABLE_CELL	*pCell = (ADD_NEW_TABLE_CELL *)pContext;

	pCell->pTable->Enumerate(	&pCell->pTempBuffer,
								(pTSCallback_t)METHOD_ADDRESS( TableStuffManager, NewTableEnumerated ),
								pCell );
	return OK;
}


STATUS 
TableStuffManager::NewTableEnumerated( void *pContext, STATUS rc ){

	ADD_NEW_TABLE_CELL	*pCell = (ADD_NEW_TABLE_CELL *)pContext;

	if( m_wasTableUpdatedWhileAdding ){
		delete pCell->pTempBuffer;
		m_wasTableUpdatedWhileAdding = false;
		pCell->pTable->Enumerate(	&m_pTempBuffer,
									(pTSCallback_t)METHOD_ADDRESS( TableStuffManager, NewTableEnumerated ),
									pCell );
		return OK;
	}
	m_pTableBeingAdded = NULL;
	
	AddTableRowObjects( pCell->pTable, pCell->pTempBuffer );
	AddTableObject( pCell->pTable );

	(this->*(pCell->pCompletionCallback))( pCell->pContext );

	delete pCell->pTempBuffer;
	delete pCell;
	return OK;
}

//************************************************************************
//
//************************************************************************

void 
TableStuffManager::TableAddedWhileInitializing( void *pContext ){

	U32			tableNumber = (U32) pContext;
	TableInfo	*pCell;
	
	if( GetTableMetaData()->GetTableCount() == tableNumber + 1 ){
		m_isIniting = false;
		SetIsReadyToServiceRequests( true );
		SSAPI_TRACE( TRACE_L2, "\n<<<<!!!>>>Mr. Rambo: Done!!! Objects built: ", this->GetManagedObjectCount() );
	}
	else{
		pCell = GetTableMetaData()->GetTableAt( tableNumber + 1 );
		AddNewTable(pCell->m_tableName, 
					MAX_TABLE_ROW_SIZE,
					(TMGR_CALLBACK)METHOD_ADDRESS(TableStuffManager, TableAddedWhileInitializing), 
					(void *) (tableNumber + 1) ); 
	}
}


//************************************************************************
// Methods that build different objects managed by this manager
//************************************************************************

void 
TableStuffManager::AddTableObject( ShadowTable *pTable  ){
	
	RowId			rid;
	TableStuff		*pObj;
	CoolVector		container;

	GetTableMetaData()->GetTableIdByTableName( pTable->GetName(), rid );
	pObj = new TableStuff( GetListenManager(), pTable, DesignatorId( rid, SSAPI_OBJECT_CLASS_TYPE_PTS_TABLE ) );
	container.Add( (CONTAINER_ELEMENT)pObj );
	AddObjectsIntoManagedObjectsVector( container );
}


void 
TableStuffManager::AddTableRowObjects( ShadowTable *pTable, void *pTableData ){

	char			*pCurr = (char *)pTableData;
	U32				i;
	TableStuffRow	*pObj;
	CoolVector		container;

	for( i = 0; i < pTable->GetNumberOfRows(); i++ ){
		pObj = new TableStuffRow(	GetListenManager(),
									this,
									pTable,
									pCurr,
									pTable->GetBytesPerRow() );

		container.Add( (CONTAINER_ELEMENT) pObj );
		pCurr += pTable->GetBytesPerRow();
	}
	AddObjectsIntoManagedObjectsVector( container );
}


//************************************************************************
//
//************************************************************************

TableMetaData* 
TableStuffManager::GetTableMetaData(){

	ManagedObject	*pObj;
	U32				i;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( pObj->GetClassType() == SSAPI_OBJECT_CLASS_TYPE_TABLE_META_DATA )
			return (TableMetaData *)pObj;
	}
	ASSERT(0);
	return NULL;
}


//************************************************************************
//
//************************************************************************

void 
TableStuffManager::DeleteTableRow( TableStuffRow &obj, SsapiResponder *pResponder ){

	DesignatorId		id;
	TableStuffRow		*pRow;
	TableStuff			*pTableStuff;

	obj.GetGenericValue( (char*)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
	pRow = (TableStuffRow *)GetManagedObject( &id );
	ASSERT(pRow);

	id = pRow->GetParentIdAt( 0 );
	pTableStuff = (TableStuff *)GetManagedObject( &id );
	ASSERT( pTableStuff );

	pTableStuff->GetShadowTable()->DeleteRow(	pRow->GetDesignatorId().GetRowId(),
												(pTSCallback_t)METHOD_ADDRESS(TableStuffManager, PtsDefaultCallback),
												NULL );

	pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
}


//************************************************************************
//
//************************************************************************

void 
TableStuffManager::ModifyTableRow( TableStuffRow &obj, SsapiResponder *pResponder ){

	DesignatorId		id;

	obj.GetGenericValue( (char*)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
}


//************************************************************************
//
//************************************************************************

void 
TableStuffManager::DeleteTableStuff( TableStuff &obj, SsapiResponder *pResponder ){

	DesignatorId		id;
	TableStuff			*pTableStuff;

	obj.GetGenericValue( (char*)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
	pTableStuff = (TableStuff *)GetManagedObject( &id );
	ASSERT( pTableStuff );

	pTableStuff->GetShadowTable()->DeleteTable((pTSCallback_t)METHOD_ADDRESS(TableStuffManager, DeleteTableStuffCallback),
												new DesignatorId(pTableStuff->GetDesignatorId()) );

	pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
}


STATUS 
TableStuffManager::DeleteTableStuffCallback( void *pContext, STATUS rc ){

	DesignatorId		*pId = (DesignatorId *)pContext, id;
	TableStuff			*pTableStuff = (TableStuff *)GetManagedObject( pId );
	ManagedObject		*pObj;
	CoolVector			container;
	U32					i;
	
	ASSERT( pTableStuff );
	
	for( i = 0; i < pTableStuff->GetChildCount(); i++ ){
		id = pTableStuff->GetChildIdAt( i );
		pObj = GetManagedObject( &id );
		container.Add( (CONTAINER_ELEMENT)pObj );
	}
	DeleteObjectsFromTheSystem( container );
	container.RemoveAll();
	
	GetTableMetaData()->RemoveTableInfoCell( RowId( 1, 0, pTableStuff->GetDesignatorId().GetRowId().GetTable()), true );

	container.Add( (CONTAINER_ELEMENT)pTableStuff );
	DeleteObjectsFromTheSystem( container );
	
	

	delete pId;
	return OK;
}


//************************************************************************
//************************************************************************

void 
TableStuffManager::ModifyField( TableStuffRow &row, UnicodeString fieldName, 
								ValueSet &newValue,	SsapiResponder *pResponder ){

	DesignatorId			id = row.GetParentIdAt( 0 );
	TableStuff				*pStuff = (TableStuff *)GetManagedObject( &id );
	ShadowTable				&table = row.GetShadowTable();
	StringClass				fName;
	UnicodeString			unicode;
	fieldDef				*pFieldDef = table.GetFieldDefArray();
	U32						i, newValueSize, j, arraySize = 0;
	void					*pNewValue;
	char					*pTemp;
	ValueSet				*pArray;
	int						code = SSAPI_TABLE_MANAGER_MODIFY_FIELD_NEW_VALUE;

	ASSERT( pStuff );
	fieldName.GetAsciiString( fName );

	for( i = 0; i < table.GetNumberOfCols(); i++ ){
		if( fName == pFieldDef[i].name ){
			// check if this is an array, ad get its values if so
			if( (pFieldDef[i].iFieldType!=BINARY_FT) && (acbDataTypeSize[ pFieldDef[i].iFieldType ] != pFieldDef[i].cbField) ){
				pTemp = new char[pFieldDef[i].cbField];
				pArray = (ValueSet *)newValue.GetValue( code );
				for( j=0; j<pArray->GetCount(); j++ ){
					pNewValue = GetValue( &pFieldDef[i], *pArray, j, pResponder, newValueSize );
					arraySize += newValueSize;
					memcpy( (char *)pTemp + j*acbDataTypeSize[ pFieldDef[i].iFieldType ],
							pNewValue, 
							newValueSize );
					delete [] pNewValue;
				}
				pNewValue = pTemp;
				newValueSize = arraySize;
			}
			else{
				// it's just a fiels, fill it
				pNewValue = GetValue(&pFieldDef[i], newValue, code, pResponder, newValueSize );
				break;
			}
		}

	}

	pTemp = fName.CString();
	table.ModifyField(	row.GetDesignatorId().GetRowId(),
						pTemp,
						pNewValue,
						newValueSize,
						(pTSCallback_t)METHOD_ADDRESS(TableStuffManager, ModifyFieldCallback),
						pResponder );
	delete [] pNewValue;
	delete [] pTemp;
	
}


void* 
TableStuffManager::GetValue( fieldDef *pField, ValueSet &newValue, int code, SsapiResponder *pResponder, U32 &newValueSize ){

	void					*pNewValue;
	StringClass				ascii;
	UnicodeString			unicode;

//switch( pFieldDef[i].iFieldType ){
	switch( pField->iFieldType ){
		case BINARY_FT:
			pNewValue = new char[ newValueSize = pField->cbField ];
			if( !newValue.GetGenericValue( (char *)pNewValue, newValueSize, code ) )
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			break;
		case S32_FT:
		case BOOL_FT:
		case VDN_FT:
			pNewValue = new char[ newValueSize = sizeof(int) ];
			newValue.GetInt( code, (int *)pNewValue );
			break;
			case U32_FT:
		case DID_FT:
			pNewValue = new char[ newValueSize = sizeof(U32) ];
			newValue.GetU32( code, (U32 *)pNewValue );
			break;
		case S64_FT:
			pNewValue = new char[ newValueSize = sizeof(I64) ];
			newValue.GetInt64( code, (I64 *)pNewValue );
			break;
		case U64_FT:
			pNewValue = new char[ newValueSize = sizeof(U64) ];
			newValue.GetU64( code, (U64 *)pNewValue );
			break;
			case STRING16_FT:
			newValue.GetASCIIString( code, &ascii );
			if( (ascii.SLength()) > 15 )
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			newValueSize = 16;
			pNewValue = ascii.CString();
			break;
		case STRING32_FT:
			newValue.GetASCIIString( code, &ascii );
			if( (ascii.SLength()) > 31 )
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			newValueSize = 32;
			pNewValue = ascii.CString();
			break;
		case STRING64_FT:
			newValue.GetASCIIString( code, &ascii );
			if( (ascii.SLength()) > 63 )
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			newValueSize = 64;
			pNewValue = ascii.CString();
			break;
		case ROWID_FT:
			pNewValue = new char[ newValueSize = sizeof(RowId) ];
			newValue.GetRowID( code, (char *)pNewValue );
			break;
		case USTRING16_FT:
			newValue.GetString( code, &unicode );
			if( (unicode.GetSize())>32 )
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			newValueSize = 32;
			pNewValue = new char[newValueSize];
			unicode.CString( pNewValue, newValueSize );
			break;
		case USTRING32_FT:
			newValue.GetString( code, &unicode );
			if( (unicode.GetSize())>64 )
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			newValueSize = 64;
			pNewValue = new char[newValueSize];
			unicode.CString( pNewValue, newValueSize );
			break;
		case USTRING64_FT:
			newValue.GetString( code, &unicode );
			if( (unicode.GetSize())>128 )
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			newValueSize = 128;
			pNewValue = new char[newValueSize];
			unicode.CString( pNewValue, newValueSize );
			break;
		case USTRING128_FT:
			newValue.GetString( code, &unicode );
			if( (unicode.GetSize())>256 )
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			newValueSize = 256;
			pNewValue = new char[newValueSize];
			unicode.CString( pNewValue, newValueSize );
			break;
		case USTRING256_FT:
			newValue.GetString( code, &unicode );
			if( (unicode.GetSize())>512 )
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			newValueSize = 512;
			pNewValue = new char[newValueSize];
			unicode.CString( pNewValue, newValueSize );
			break;
		default:
			ASSERT(0);
			pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			break;
	}

	return pNewValue;
}


STATUS 
TableStuffManager::ModifyFieldCallback( void *pContext, STATUS rc ){

	SsapiResponder		*pResponder = (SsapiResponder *)pContext;

	if( rc == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );

	return OK;
}
