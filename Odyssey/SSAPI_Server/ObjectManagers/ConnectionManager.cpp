//************************************************************************
// FILE:		ConnectionManager.cpp
//
// PURPOSE:		Implements the ConnectionManager object which will be responsible
//				for connection and data path management in the O2K
//				products.
//************************************************************************

#include "ConnectionManager.h"
#include "UpstreamConnection.h"
#include "DownstreamConnection.h"
#include "RedundantDataPath.h"
#include "ClusteredDataPath.h"
#include "StringResourceManager.h"
#include "SsapiLocalResponder.h"
#include "ListenManager.h"
#include "DeviceManager.h"
#include "ClassTypeMap.h"
#include "SsapiEvents.h"
#include "DdmSsapi.h"

ConnectionManager* ConnectionManager::m_pThis = NULL;



//************************************************************************
// ConnectionManager:
//
// PURPOSE:		Default constructor
//************************************************************************

ConnectionManager::ConnectionManager( ListenManager *pListenManager, DdmServices *pParent, StringResourceManager *pSRM )									 
:ObjectManager( pListenManager, DesignatorId(RowId(), SSAPI_MANAGER_CLASS_TYPE_CONNECTION_MANAGER), pParent ){

	m_isIniting						= true;
	m_tablesToRebuild				= 0;
	m_requestsOutstanding			= 0;
	m_pStringResourceManager		= pSRM;
	m_updatesForHostConnPending		= 0;
	m_isAnotherStringUpdatePending	= false;
	m_pAddPathStringsCell			= NULL;

	SetIsReadyToServiceRequests( false );

	m_pTable = new SSAPI_CONN_MGR_JUMP_TABLE_RECORD[SSAPI_CM_NUMBER_OF_TABLES_USED];

	(m_pTable + 0)->pTableName				= FC_PORT_DATABASE_TABLE_NAME;
	(m_pTable + 0)->tableMask				= CM_FC_DB_TABLE;
	(m_pTable + 0)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ConnectionManager,FcDbTableRowInserted);
	(m_pTable + 0)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ConnectionManager,FcDbTableRowDeleted);
	(m_pTable + 0)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ConnectionManager,FcDbTableRowModified);
	(m_pTable + 0)->pCreateObjectsFromRow	= (ConnectionManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(ConnectionManager,CreateObjectsFromFcDbRow );
	(m_pTable + 0)->rowSize					= sizeof(FCPortDatabaseRecord);
	(m_pTable + 0)->pFieldDef				= (fieldDef*)FCPortDatabaseTable_FieldDefs;
	(m_pTable + 0)->fieldDefSize			= cbFCPortDatabase_FieldDefs;

	(m_pTable + 1)->pTableName				= HOST_CONNECTION_DESCRIPTOR_TABLE_NAME;
	(m_pTable + 1)->tableMask				= CM_PATH_TABLE; 
	(m_pTable + 1)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ConnectionManager,PathTableRowInserted);
	(m_pTable + 1)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ConnectionManager,PathTableRowDeleted);
	(m_pTable + 1)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ConnectionManager,PathTableRowModified);
	(m_pTable + 1)->pCreateObjectsFromRow	= (ConnectionManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(ConnectionManager,CreateObjectsFromPathRow);
	(m_pTable + 1)->rowSize					= sizeof(HostConnectionDescriptorRecord);
	(m_pTable + 1)->pFieldDef				= (fieldDef*)HostConnectionDescriptorTable_FieldDefs;
	(m_pTable + 1)->fieldDefSize			= cbHostConnectionDescriptor_FieldDefs;


	for( int i = 0; i < SSAPI_CM_NUMBER_OF_TABLES_USED; i++ )
		(m_pTable + i)->pShadowTable	= new ShadowTable(	(m_pTable + i)->pTableName,
															this,
															(m_pTable + i)->pRowInsertedCallback,
															(m_pTable + i)->pRowDeletedCallback,
															(m_pTable + i)->pRowModifiedCallback,
															(m_pTable + i)->rowSize );

	m_pObjAddedResponder = new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(ConnectionManager,ObjectAddedEventCallback) ); 
	GetListenManager()->AddListenerForObjectAddedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjAddedResponder->GetSessionID(), CALLBACK_METHOD(m_pObjAddedResponder, 1) );
	
	m_pObjModifiedResponder = new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(ConnectionManager,ObjectModifiedEventCallback) ); 
	GetListenManager()->AddListenerForObjectModifiedEventForManagers( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjModifiedResponder->GetSessionID(), CALLBACK_METHOD(m_pObjModifiedResponder, 1) );
	
	SSAPI_TRACE( TRACE_L2, "\nConnectionManager: Initializing..." );

	DefineAllTables(); 
	AddOutstandingReq();

}


//************************************************************************
// ~ConnectionManager:
//
// PURPOSE:		The destructor
//************************************************************************

ConnectionManager::~ConnectionManager(){


	GetListenManager()->DeleteListenerForObjectAddedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjAddedResponder->GetSessionID() );
	delete m_pObjAddedResponder;
	
	GetListenManager()->DeleteListenerForObjectModifiedEventForManagers( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjModifiedResponder->GetSessionID() );
	delete m_pObjModifiedResponder;

	for( int i = 0; i < SSAPI_CM_NUMBER_OF_TABLES_USED; i++ )
		delete (m_pTable + i)->pShadowTable;

	delete[] m_pTable;
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
ConnectionManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){
	
	DataPath				*pPath;
	DesignatorId			ownerId, id;
	UpstreamConnection		*pConn;
	ValueSet				*pVs;

	if( ObjectManager::Dispatch( pRequestParms, requestCode, pResponder ) )
		return true;

	switch( requestCode ){

		case SSAPI_CM_SET_CONNECTION_IDS:
			pRequestParms->GetGenericValue( (char *)&ownerId, sizeof(ownerId), SSAPI_CM_SET_CONNECTION_IDS_OWNER_ID );		
			pVs = (ValueSet *)pRequestParms->GetValue(SSAPI_CM_SET_CONNECTION_IDS_CONNECTION_ID_VECTOR);
			pPath = (DataPath *)GetManagedObject( &ownerId );

			// validate parms
			if( !pPath || !pVs )
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			else
				SetConnectionIds( pPath, pVs, pResponder );
			break;

		case SSAPI_PATH_MODIFY_STRINGS:
			ModifyPathStrings( *pRequestParms, pResponder );
			break;

		case SSAPI_CM_ADD_CONNECTION_ID:
			pRequestParms->GetGenericValue( (char *)&ownerId, sizeof(ownerId), SSAPI_CM_ADD_CONNECTION_ID_OWNER_ID );		
			pPath = (DataPath *)GetManagedObject( &ownerId );
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_CM_ADD_CONNECTION_ID_CONNECTION_ID );
			pConn = (UpstreamConnection *)GetManagedObject( &id );

			if( !pConn ){
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_CONNECTION_ID_INALID);
				return true;
			}

			if( pPath )
				AddConnIdToDataPath( pPath, pConn, pResponder, 0 );
			else
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);

			break;

		case SSAPI_CM_REMOVE_CONNECTION_ID:
			pRequestParms->GetGenericValue( (char *)&ownerId, sizeof(id), SSAPI_CM_REMOVE_CONNECTION_ID_OWNER_ID );		
			pPath = (DataPath *)GetManagedObject( &ownerId );
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_CM_REMOVE_CONNECTION_ID_CONNECTION_ID );

			if( pPath )
				RemoveConnIdFromDataPath( pPath, id, pResponder );
			else
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);

			break;

		default:
			ASSERT(0);
			break;
	}

	return true;
}


//************************************************************************
// DefineAllTables:
//
// PURPOSE:		Issues requests to the PTS to define all tables if not yet
//				defined
//************************************************************************

STATUS 
ConnectionManager::DefineAllTables(){

	STATUS	status = OK;

	m_builtTablesMask = 0;
	for( int i = 0; i < SSAPI_CM_NUMBER_OF_TABLES_USED; i++ )
		status	|= (m_pTable + i)->pShadowTable->DefineTable(	(m_pTable + i)->pFieldDef,
																(m_pTable + i)->fieldDefSize,
																32,
																(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,DefineAllTablesReplyHandler),
																(void *)(m_pTable + i)->tableMask );

	if( status != OK ){
	}	// GAIEVENT

	return OK;
}


//************************************************************************
// DefineAllTablesReplyHandler:
//
// PURPOSE:		Handles replies to requests issued in DefineAllTables()
//************************************************************************

STATUS 
ConnectionManager::DefineAllTablesReplyHandler( void *pContext, STATUS status ){

	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & CM_ALL_TABLES) == CM_ALL_TABLES ){
		m_builtTablesMask = 0;
		InitializeAllTables();
	}

	return OK;
}


//************************************************************************
// InitializeAllTables:
//
// PURPOSE:		Sends requests to the PTS to init ShadowTable objects
//				for all the tables used.
//************************************************************************

STATUS 
ConnectionManager::InitializeAllTables(){

	STATUS status = OK;

	m_builtTablesMask = 0;
	for( int i = 0; i < SSAPI_CM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Initialize(	(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,InitializeAllTablesReplyHandler), 
															(void *)(m_pTable + i)->tableMask );

	if( status != OK ){
	}	// GAIEVENT

	return OK;
}


//************************************************************************
// InitializeAllTablesReplyHandler:
//
// PURPOSE:		Handles replies to requests to the PTS sent from 
//				InitializeAllTables()
//************************************************************************

STATUS 
ConnectionManager::InitializeAllTablesReplyHandler( void *pContext, STATUS status ){
	
	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & CM_ALL_TABLES) == CM_ALL_TABLES ){
		m_builtTablesMask = 0;
		EnumerateAllTables();
	}

	return OK;
}


//************************************************************************
// EnumerateAllTables:
//
// PUPROSE:		Issues requests to the PTS to enumerate all tables used
//************************************************************************

STATUS 
ConnectionManager::EnumerateAllTables(){

	STATUS		status = OK;
	
	m_builtTablesMask = 0;

	for( int i = 0; i < SSAPI_CM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable,
															(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,EnumerateAllTablesReplyHandler),
															(void *)(m_pTable + i)->tableMask);

	if( status != OK ){
	}	// GAIEVENT

	return OK;
}


//************************************************************************
// EnumerateAllTablesReplyHandler:
//
// PURPOSE:		Handles replies to requests sent by EnumerateAllTables().
//				When all table have been enumerated, sets "ready for 
//				service flag"
//************************************************************************

STATUS 
ConnectionManager::EnumerateAllTablesReplyHandler( void *pContext, STATUS rc ){

	U32				mask = (U32)pContext, index;
	CoolVector		container;

	for( int i = 0; i < SSAPI_CM_NUMBER_OF_TABLES_USED; i++ ){
		if( mask == (m_pTable + i)->tableMask ){
			if( m_tablesToRebuild & (m_pTable + i)->tableMask ){
				m_tablesToRebuild &= ~(m_pTable + i)->tableMask;
				delete (m_pTable + i)->pTempTable;
				return (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable, (pTSCallback_t)METHOD_ADDRESS(ConnectionManager,EnumerateAllTablesReplyHandler), (void *)(m_pTable + i)->tableMask);
			}
			m_builtTablesMask |= mask;
			if( rc == OK ){
				for( index = 0; index < (m_pTable + i)->pShadowTable->GetNumberOfRows(); index++ )
					(this->*(m_pTable + i)->pCreateObjectsFromRow)( ((char *)(m_pTable + i)->pTempTable) + index*(m_pTable + i)->rowSize, container );

				delete (m_pTable + i)->pTempTable;
				AddObjectsIntoManagedObjectsVector( container );
			}
			else{
				delete (m_pTable + i)->pTempTable;
				return OK;
			}
		}
	}

	if( ((m_builtTablesMask & CM_ALL_TABLES) == CM_ALL_TABLES) )
		RemoveOutstandingReq();	

	return OK;
}


//************************************************************************
// Table callbacks for the connection table
//************************************************************************

STATUS 
ConnectionManager::PathTableRowInserted( void *pContext, U32 numberOfRows, ShadowTable *){

	if( m_isIniting ){
		m_tablesToRebuild	|= CM_PATH_TABLE;
		return OK;
	}

	HostConnectionDescriptorRecord		*pRow = (HostConnectionDescriptorRecord *)pContext;
	Container							*pContainer;
	DataPath							*pPath;
	
	SSAPI_TRACE( TRACE_L3, "\nConnectionManager::ConnTableRowInserted" );
	pContainer = new CoolVector;

	for( int index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromPathRow( pRow, *pContainer, true );
	
	if( m_pAddPathStringsCell ){
		ValueSet		*pVs = new ValueSet, *pR = new ValueSet;
		DesignatorId id;
		
		// find the object we have just added
		pRow = (HostConnectionDescriptorRecord *)pContext;
		GetDesignatorIdByRowId( pRow->rid, id );
		pPath = (DataPath *)GetManagedObject( &id );
		id = pPath->GetDesignatorId();

		pVs->AddGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID);
		pVs->AddString( &m_pAddPathStringsCell->name, SSAPI_DATA_PATH_FID_NAME);
		pVs->AddString( &m_pAddPathStringsCell->description, SSAPI_DATA_PATH_FID_DESCRIPTION);
		pR->AddValue( pVs, SSAPI_PATH_MODIFY_STRINGS_PATH_OBJECT);
		ModifyPathStrings( *pR, new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(ConnectionManager, Dummy ) ) );
		delete pVs;
		delete pR;
		delete m_pAddPathStringsCell;
		m_pAddPathStringsCell = NULL;
		RemoveOutstandingReq();
	}

	BumpUpConfigId();

	delete pContainer;
	return OK;
}


STATUS 
ConnectionManager::PathTableRowDeleted( void *pContext, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild	|= CM_PATH_TABLE;
		return OK;
	}

	Container						*pContainer; 
	HostConnectionDescriptorRecord	*pRow = (HostConnectionDescriptorRecord *)pContext;
	U32								index;
	ManagedObject					*pObj;

	SSAPI_TRACE( TRACE_L3, "\nConnectionManager::ConnTableRowDeleted" );
	pContainer = new CoolVector;

	for( index = 0; index < numberOfRows; index++, pRow++ ){
		// delete strings
		if( RowId(pRow->ridDescription).IsClear() == false ) 
			m_pStringResourceManager->DeleteString( pRow->ridDescription,
													(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,DoNothing),
													NULL );

		if( RowId(pRow->ridName).IsClear() == false ) 
			m_pStringResourceManager->DeleteString( pRow->ridName,
													(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,DoNothing),
													NULL );

		// clear up string ids so we do not try to read them
		pRow->ridDescription = RowId();
		pRow->ridName = RowId();

		CreateObjectsFromPathRow( pRow, *pContainer );
	}
	

	DeleteObjectsFromTheSystem( *pContainer );

	BumpUpConfigId();
	
	// clean-up
	while( pContainer->Count() ){
		pContainer->GetAt( (CONTAINER_ELEMENT &)pObj, 0 );
		pContainer->RemoveAt( 0 );
		delete pObj;
	}

	delete pContainer;

	return OK;
}


STATUS 
ConnectionManager::PathTableRowModified( void *pContext, U32 numberOfRows, ShadowTable *p){

	DataPath						*pPath;
	DesignatorId					id, childId;
	HostConnectionDescriptorRecord	*pRow = (HostConnectionDescriptorRecord *)pContext;
	UpstreamConnection				*pConn;
	U32								rowNum, index, childNum;
	bool							found;

	if( m_isIniting ){
		m_tablesToRebuild	|= CM_PATH_TABLE;
		return OK;
	}

	if( m_updatesForHostConnPending > 1){
		m_updatesForHostConnPending--;
		if( m_requestsOutstanding )
			RemoveOutstandingReq();
		SSAPI_TRACE( TRACE_L2, "\nConnectionManager: ingoring redundant table update -> PathDescr");
		return OK;
	}
	else if( m_updatesForHostConnPending )
		m_updatesForHostConnPending--;

	for( rowNum = 0; rowNum < numberOfRows; rowNum++, pRow++ ){
		this->GetDesignatorIdByRowId( pRow->rid, id );
		pPath = (DataPath *)GetManagedObject( &id );
		ASSERT( pPath );
		// find and purge ids that were deleted
		for( childNum = 0; childNum < pPath->GetChildCount(); childNum++ ){
			childId = pPath->GetChildIdAt( childNum );
			for( index = 0, found = false; index < pRow->ridEIPCount; index++ ){
				if( RowId(pRow->ridEIPs[index]) == childId.GetRowId() ){
					found = true;
					break;
				}
			}
			if( !found ){ // purge!!!
				pConn = (UpstreamConnection *)GetManagedObject( &childId );
				ASSERT(pConn);
				pPath->DeleteChildId( pConn );
				pConn->DeleteParentId( pPath->GetDesignatorId() );
				SSAPI_TRACE( TRACE_L2, "\nConnectionManager:: Purged Connection id from PathDescr");
			}
		}
	}

	PathTableRowInserted( pContext, numberOfRows, p );

	if( m_requestsOutstanding )
		RemoveOutstandingReq();

	return OK;
}


bool 
ConnectionManager::CreateObjectsFromPathRow( HostConnectionDescriptorRecord *pRow, Container &container, bool autoAdd ){

	DataPath			*pPath, *pOldPath;
	DeviceManager		*pManager = (DeviceManager *)((DdmSSAPI*)pParentDdmSvs)->GetObjectManager( SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );
	U32					i;
	DesignatorId		id;
	ConnectionBase		*pConn;
	CoolVector			purgeIds, autoAddContainer;
	
	switch( pRow->eHostConnectionMode ){
		case ePATH_TYPE_REDUNDANT:
			pPath = new RedundantDataPath( GetListenManager(), this );
			break;

		case ePATH_TYPE_CLUSTERED:
			pPath = new ClusteredDataPath( GetListenManager(), this );
			break;

		default:
			ASSERT(0);
			return false;
	}

	pPath->BuildYourselfFromPtsRow( pRow );
	id = pPath->GetDesignatorId();
	pOldPath = (DataPath *)GetManagedObject( &id );
	if( pOldPath ){
		pPath->SetRidDescription( pOldPath->GetRidDescription() );
		pPath->SetRidName( pOldPath->GetRidName() );
		pPath->SetName( pOldPath->GetName(), false );
		pPath->SetDescription( pOldPath->GetDescription(), false );
	}

	// add children/parent ids
	for( i = 0; i < pRow->ridEIPCount; i++ ){
		GetDesignatorIdByRowId( pRow->ridEIPs[i], id );
		pConn = (ConnectionBase *)GetManagedObject( &id );
		if( pConn ){
			if( !pOldPath )
				pPath->AddChildId( pConn, false );
			else if( pOldPath->IsYourChild( pConn->GetDesignatorId() ) )
				pPath->AddChildId( pConn, false );
			else
				pPath->AddChildId( pConn );
		}
		else
			purgeIds.Add( i, i );
	}

	if( autoAdd ){
		autoAddContainer.Add( (CONTAINER_ELEMENT)pPath );
		AddObjectsIntoManagedObjectsVector( autoAddContainer );
		autoAddContainer.RemoveAll();
	}

	container.Add( (CONTAINER_ELEMENT) pPath );

	// now, add parent id to the connections...
	for( i = 0; i < pRow->ridEIPCount; i++ ){
		GetDesignatorIdByRowId( pRow->ridEIPs[i], id );
		pConn = (ConnectionBase *)GetManagedObject( &id );
		if( pConn )
			pConn->AddParentId( pPath, true );
	}


	// ooops, some ids are of non-existing  objects....purge 'em
	if( purgeIds.Count() )
		PurgeConnIdsFromPathRow( pRow, purgeIds );

	// read the strings
	if( (RowId(pRow->ridDescription).IsClear() == false) && m_isIniting ){
		READ_STRING_CELL	*pCell = new READ_STRING_CELL(this);
		bool				rc;

		pCell->id = pPath->GetDesignatorId();
		pCell->pName = new UnicodeString;
		pCell->context = STRING_OP_CONTEXT_PATH_DESCRIPTION;	
		rc =m_pStringResourceManager->ReadString(	pCell->pName,
													pRow->ridDescription,
													(pTSCallback_t)METHOD_ADDRESS(ConnectionManager, ReadPathStringCallback),
													pCell );
		if( rc )
			AddOutstandingReq();
		else
			ASSERT(0);

	}

	if( (RowId(pRow->ridName).IsClear() == false) && m_isIniting ){
		READ_STRING_CELL	*pCell = new READ_STRING_CELL(this);
		bool				rc;

		pCell->id = pPath->GetDesignatorId();
		pCell->pName = new UnicodeString;
		pCell->context = STRING_OP_CONTEXT_PATH_NAME;	
		rc =m_pStringResourceManager->ReadString(	pCell->pName,
													pRow->ridName,
													(pTSCallback_t)METHOD_ADDRESS(ConnectionManager, ReadPathStringCallback),
													pCell );
		if( rc )
			AddOutstandingReq();
		else
			ASSERT(0);

	}

	return true;
}


//************************************************************************
// ReadPathStringCallback:
//
// PURPOSE:		Called when a string for a path is read
//************************************************************************

STATUS 
ConnectionManager::ReadPathStringCallback( void *pContext, STATUS rc ){

	READ_STRING_CELL	*pCell = (READ_STRING_CELL *)pContext;
	DataPath			*pPath;

	if( rc == OK ){
		pPath = (DataPath *)pCell->pThis->GetManagedObject( &pCell->id );
		if( pPath ){
			switch( pCell->context ){
				case STRING_OP_CONTEXT_PATH_DESCRIPTION:
					pPath->SetDescription( *pCell->pName );
					break;

				case STRING_OP_CONTEXT_PATH_NAME:
					pPath->SetName( *pCell->pName );
					break;

				default:
					ASSERT(0);
					break;
			}	
		}
	}

	pCell->pThis->RemoveOutstandingReq();
	delete pCell->pName;
	delete pCell;

	return OK;
}


//************************************************************************
// RemoveOutstandingReq:
//
// PURPOSE:	
//************************************************************************

void 
ConnectionManager::RemoveOutstandingReq(){ 

	if( (m_requestsOutstanding == 1) && m_isIniting ){
		m_isIniting = false;
		ReconcileObjectStates();
		SSAPI_TRACE( TRACE_L2, "\nConnectionManager: ...Done! Objects built: ", GetManagedObjectCount() );
	}

	if( !m_requestsOutstanding || !--m_requestsOutstanding ) 
		SetIsReadyToServiceRequests( true ); 
}


//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

void 
ConnectionManager::ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ){
#if 0
	ValueSet				*pObjVs = new ValueSet;
	int						classType;
	ClassTypeMap			map;
	DesignatorId			id;

	*pObjVs	= *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	pObjVs->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );


	delete pObjVs;
#endif
}


//************************************************************************
// AddObject:
//
// PURPOSE:		Adds an object to the system
//
// NOTE:		Override default implementation by calling to AddHost()
//************************************************************************

bool 
ConnectionManager::AddObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	int				classType;
	bool			rc = true;
	ClassTypeMap	map;

	if( !objectValues.GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType ) )
		ASSERT(0);;

	if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_DATA_PATH, classType, true ) )
		rc = AddDataPath( &objectValues, pResponder );
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );	

	return rc;
}	


//************************************************************************
// ObjectAddedDeletedModifiedReplyCallback:
//
// PURPOSE:		Responsible for handling the callback and replying to UI
//************************************************************************

STATUS 
ConnectionManager::ObjectAddedDeletedModifiedReplyCallback( void *pContext, STATUS rc ){

	SsapiResponder	*pResponder = (SsapiResponder *)pContext;

	if( rc == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	return OK;
}


//************************************************************************
// AddDataPath:
//
// PURPOSE:		Attempts to add a new host connection object
//************************************************************************

bool 
ConnectionManager::AddDataPath( ValueSet *pValueSet, SsapiResponder *pResponder ){

	DataPath						*pPath;
	HostConnectionDescriptorRecord	row;
	DesignatorId					id;
	ValueSet						&objectValues = *pValueSet;
	STATUS							status;
	int								classType;
	U32								errorString;

	if( !objectValues.GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType ) )
		ASSERT(0);;

	switch( classType ){
		case SSAPI_OBJECT_CLASS_TYPE_REDUNDANT_DATA_PATH:
			pPath = new RedundantDataPath( GetListenManager(), this );
			break;
		case SSAPI_OBJECT_CLASS_TYPE_CLUSTERED_DATA_PATH:
			pPath = new ClusteredDataPath( GetListenManager(), this );
			break;
		default:
			ASSERT(0);
			return false;
	}

	*pPath = *pValueSet;
	pPath->BuildYourselfFromYourValueSet();
	pPath->WriteYourselfIntoPtsRow( &row );
	
	if( !pPath->IsAValidConfiguration( errorString ) ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, errorString );
		delete pPath;
		return false;
	}

	m_pAddPathStringsCell = new ADD_PATH_STRINGS_CELL;
	m_pAddPathStringsCell->name = pPath->GetName();
	m_pAddPathStringsCell->description = pPath->GetDescription();
	m_pAddPathStringsCell->id = pPath->GetDesignatorId();

	status = GetShadowTable( CM_PATH_TABLE )->InsertRow( &row, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS( ConnectionManager, ObjectAddedDeletedModifiedReplyCallback ), pResponder);
	if( status != OK ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
		delete m_pAddPathStringsCell;
	}
	else
		AddOutstandingReq();

	delete pPath;

	return true;
}


//************************************************************************
// GetShadowTable:
//
// PURPOSE:		Performs a look up of the table by the table mask
//************************************************************************

ShadowTable* 
ConnectionManager::GetShadowTable( U32 tableMask ){

	for( U32 i = 0; i < SSAPI_CM_NUMBER_OF_TABLES_USED; i++ )
		if( m_pTable[i].tableMask == tableMask )
			return m_pTable[i].pShadowTable;


	ASSERT(0);
	return NULL;
}


//************************************************************************
// DeleteDataPath:
//
// PURPOSE:		Attempts to delete a host connection object
//************************************************************************

bool 
ConnectionManager::DeleteDataPath( DesignatorId &id, SsapiResponder *pResponder ){

	DataPath			*pPath = (DataPath *)GetManagedObject( &id );
	ClassTypeMap		map;
	LunMapManager		*pLM = (LunMapManager *)GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_LUN_MANAGER);

	if( pLM->IsThisPathInUse( id ) ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_HM_PATH_IN_USE );
		return false;
	}

	STATUS rc = GetShadowTable( CM_PATH_TABLE )->DeleteRow(	id.GetRowId(),
															(pTSCallback_t)METHOD_ADDRESS( ConnectionManager, ObjectAddedDeletedModifiedReplyCallback ),
															pResponder );
	if( rc != OK )
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	
	return true;
}


//************************************************************************
// ObjectAddedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_ADDED events 
//				coming from the listen manager.
//
//				We interested in the following things:
//				-- a connection is added -> we need to state of the corresponding connection
//				-- 
//************************************************************************

void 
ConnectionManager::ObjectAddedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId ){

	SSAPIEvent				*pEvent = new SSAPIEventObjectAdded( NULL );
	ValueSet				*pObjVs = new ValueSet;
	ClassTypeMap			map;
	int						classType;
	DesignatorId			id;
	CoolVector				affectedObjects;

	*pEvent = *(ValueSet *)pVs->GetValue( eventObjectId );
	*pObjVs = *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	pObjVs->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );
	pObjVs->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );

	// check if it's a Gemini port
	if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_PORT, classType, true ) ){
		GetAffectedManagedObjects( id, affectedObjects );
		ReconcileObjectStates( &affectedObjects );
	}

	// check if this is a connection object
	if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_CONNECTION, classType, true ) ){
		GetAffectedManagedObjects( id, affectedObjects );
		ReconcileObjectStates( &affectedObjects );
	}
	
	delete pObjVs;
	delete pEvent;

}


//************************************************************************
// ObjectModifiedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_MODIFIED events 
//				coming from the listen manager.
//
//				We are interested in the following things:
//				-- a DataPath is modified -> we need to
//				   reconcile states of the rest
//				-- a Port device is modified, we need to set the state of
//					corresponding data path
//************************************************************************

void 
ConnectionManager::ObjectModifiedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId ){

	SSAPIEvent				*pEvent = new SSAPIEventObjectAdded( NULL );
	ValueSet				*pObj = new ValueSet;
	int						classType;
	ClassTypeMap			map;

	*pEvent = *(ValueSet *)pVs->GetValue( eventObjectId );
	*pObj	= *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	pObj->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );

	// check if this is a port
	ObjectAddedEventCallback( pVs, isLast, eventObjectId );

	// check if it's a data path
	if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_DATA_PATH, classType, true ) )
		ReconcileObjectStates();

	delete pObj;
	delete pEvent;
}


//************************************************************************
// ReconcileObjectStates:
//
// PURPOSE:		Reconciles states of DataPaths
//************************************************************************

void 
ConnectionManager::ReconcileObjectStates( Container *pObjectsToReconcile ){

#if 0
	U32							i;
	ManagedObject				*pObj;
	StatusReporterInterface		*pReporter;
	//TBDGAI

	if( !pObjectsToReconcile ){
		for( i = 0; i < GetManagedObjectCount(); i++ ){
			pObj = GetManagedObject( i );
			pReporter = (StatusReporterInterface *)pObj;
			if( pReporter )
				pReporter->ComposeYourOverallState();
		}
	}
	else{
		for( i = 0; i < pObjectsToReconcile->Count(); i++ ){
			pObjectsToReconcile->GetAt( (CONTAINER_ELEMENT &)pObj, i );
			pReporter = (StatusReporterInterface *) pObj;
			if( pReporter )
				pReporter->ComposeYourOverallState();
		}
	}
#endif
}


//************************************************************************
// Table callbacks for the Fc DB table
//************************************************************************

STATUS 
ConnectionManager::FcDbTableRowInserted( void *pContext, U32 numberOfRows, ShadowTable *){

	FCPortDatabaseRecord			*pRow = (FCPortDatabaseRecord *)pContext;
	Container						*pContainer;

	if( m_isIniting ){
		m_tablesToRebuild	|= CM_FC_DB_TABLE;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nConnectionManager::FcDbTableRowInserted" );
	pContainer = new CoolVector;

	for( int index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromFcDbRow( pRow, *pContainer );
	
	AddObjectsIntoManagedObjectsVector( *pContainer );
	BumpUpConfigId();

	delete pContainer;
	return OK;
}


STATUS 
ConnectionManager::FcDbTableRowDeleted( void *pContext, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild	|= CM_FC_DB_TABLE;
		return OK;
	}

	Container						*pContainer; 
	FCPortDatabaseRecord			*pRow = (FCPortDatabaseRecord *)pContext;
	U32								index;
	ManagedObject					*pObj;
	ClassTypeMap					map;
	DesignatorId					id;

	SSAPI_TRACE( TRACE_L3, "\nConnectionManager::FcDbTableRowDeleted" );
	pContainer = new CoolVector;

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromFcDbRow( pRow, *pContainer );

	DeleteObjectsFromTheSystem( *pContainer );
	BumpUpConfigId();
	
	// clean-up
	while( pContainer->Count() ){
		pContainer->GetAt( (CONTAINER_ELEMENT &)pObj, 0 );
		pContainer->RemoveAt( 0 );
		delete pObj;
	}

	delete pContainer;

	return OK;
}


STATUS 
ConnectionManager::FcDbTableRowModified( void *pContext, U32 numberOfRows, ShadowTable *p ){

	if( m_isIniting ){
		m_tablesToRebuild	|= CM_FC_DB_TABLE;
		return OK;
	}

	return FcDbTableRowInserted( pContext, numberOfRows, p );
}


bool 
ConnectionManager::CreateObjectsFromFcDbRow( FCPortDatabaseRecord *pRow, Container &container, bool autoAdd ){

	ConnectionBase			*pConn=NULL, *pOldConn= NULL;
	READ_STRING_CELL		*pCell;	
	U32						i;
	ManagedObject			*pObj;
	DesignatorId			id;
	CoolVector				autoAddContainer;
	
	// skip our own ports
	if( pRow->attribs & FC_PORT_OWNER_INTERNAL )
		return true;
		
	switch( pRow->portType ){

		case FC_PORT_TYPE_TARGET:
			pConn = new DownstreamConnection( GetListenManager(), this );
			break;

		case FC_PORT_TYPE_INITIATOR:
			pConn = new UpstreamConnection( GetListenManager(), this );
			break;

		default:
			SSAPI_TRACE( TRACE_L1, "\nConnectionManager: Unknown Connection type: ", pRow->portType );
			ASSERT(0);
			break;
	}

	if( pConn ){

		pConn->BuildYourselfFromPtsRow( pRow, this );
	
		// find if this is an update and put parents in
		GetDesignatorIdByRowId( pRow->rid, id );
		pOldConn = (ConnectionBase *)GetManagedObject( &id );
		if( pOldConn ){
			for( i = 0; i < pOldConn->GetParentCount(); i++ ){
				id = pOldConn->GetParentIdAt( i );
				pObj = GetManagedObject( &id );
				pConn->AddParentId( pObj, false );
			}
			// check if the connection type changed
			if( pConn->GetClassType() != pOldConn->GetClassType() ){
				CoolVector	v;
				printf("\n Connection type has changed. Deleting the old connection");
				v.Add( (CONTAINER_ELEMENT)pOldConn );
				DeleteObjectsFromTheSystem( v );
			}
		}
	
		container.Add( (CONTAINER_ELEMENT) pConn );
		if( autoAdd ){
			autoAddContainer.Add( (CONTAINER_ELEMENT)pConn );
			AddObjectsIntoManagedObjectsVector( autoAddContainer );
			autoAddContainer.RemoveAll();
		}

		// read the name in 
		if( RowId( pRow->ridName ).IsClear() == false ){
			pCell = new READ_STRING_CELL(this);
			pCell->pName	= new UnicodeString;
			pCell->id		= pConn->GetDesignatorId();
			m_pStringResourceManager->ReadString(	pCell->pName, 
													pRow->ridName,
													(pTSCallback_t)METHOD_ADDRESS( ConnectionManager, ConnectionReadNameCallback ),
													pCell );
			AddOutstandingReq();
		}
	}
	else {
		CoolVector	v;
		if( GetDesignatorIdByRowId( pRow->rid, id ) ){ 
			pOldConn = (ConnectionBase *)GetManagedObject( &id );
			if( pOldConn ){
				printf("\n Looks like a connection's been removed. Deleting the old connection");
				v.Add( (CONTAINER_ELEMENT)pOldConn );
				DeleteObjectsFromTheSystem( v );
			}
		}
	}

	return true;
}

//************************************************************************
// ConnectionReadNameCallback:
//
// PURPOSE:		Callback for ReadString
//************************************************************************

STATUS 
ConnectionManager::ConnectionReadNameCallback( void *pContext, STATUS rc ){

	READ_STRING_CELL		*pCell = (READ_STRING_CELL *)pContext;
	UpstreamConnection		*pConn = (UpstreamConnection *)pCell->pThis->GetManagedObject( &pCell->id );

	if( pConn && (rc == OK) )
		pConn->SetName( *pCell->pName );

	pCell->pThis->RemoveOutstandingReq();
	delete pCell->pName;
	delete pCell;

	return OK;
}


//************************************************************************
// ModifyConnectionName:
//
// PURPOSE:		Modifies the name of the connection object
//************************************************************************

void 
ConnectionManager::ModifyConnectionName( ConnectionBase *pConn, UnicodeString newName, SsapiResponder *pResponder ){

	bool						rc = true;
	ADD_DELETE_PATH_NAME_CELL	*pCell = new ADD_DELETE_PATH_NAME_CELL(this);

	m_pStringResourceManager->DeleteString( pConn->GetRidName(),
											(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,AddDeleteConnectionNameCallback ),
											NULL );
	pCell->connId = pConn->GetDesignatorId();
	m_pStringResourceManager->WriteString(	newName,
											&pCell->rid,
											(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,AddDeleteConnectionNameCallback ),
											pCell);
	AddOutstandingReq();
	pResponder->RespondToRequest( SSAPI_RC_SUCCESS );

}


//************************************************************************
// AddDeleteConnectionNameCallback:
//
// PURPOSE:		Called when coonection name is deleted or added
//				if pContext == NULL --> delete
//				otherwise --> add
//************************************************************************

STATUS 
ConnectionManager::AddDeleteConnectionNameCallback( void *pContext, STATUS rc ){

	ADD_DELETE_PATH_NAME_CELL	*pCell = (ADD_DELETE_PATH_NAME_CELL *)pContext;
	UpstreamConnection			*pConnection;

	if( pContext ){
		pConnection = (UpstreamConnection *)pCell->pThis->GetManagedObject( &pCell->connId );
		if( pConnection ){
			pConnection->SetRidName( pCell->rid );
			pCell->pThis->
			GetShadowTable(CM_FC_DB_TABLE)->ModifyField( pConnection->GetDesignatorId().GetRowId(),
															FCP_PORT_DTB_TABLE_FN_RID_NAME,
															&pCell->rid,
															sizeof(pCell->rid),
															(pTSCallback_t)METHOD_ADDRESS(ConnectionManager, ConnNameFieldModifiedCallback),
															NULL );
		}
	}

	delete pCell;
	return OK;
}


//************************************************************************
// ConnNameFieldModifiedCallback:
//
// PURPOSE:		Called when 'ridName' field on the FC DB descriptor 
//				is modified
//************************************************************************

STATUS 
ConnectionManager::ConnNameFieldModifiedCallback( void *pContext, STATUS rc ){

	RemoveOutstandingReq();
	return OK;
}


//************************************************************************
// ModifyPathStrings:
//************************************************************************

bool 
ConnectionManager::ModifyPathStrings( ValueSet &requestParms, SsapiResponder *pResponder ){

	ValueSet			*pVs = (ValueSet *)requestParms.GetValue(SSAPI_PATH_MODIFY_STRINGS_PATH_OBJECT);
	DataPath			*pNewPath, *pOldPath, *pPath;
	DesignatorId		id;
	RowId				rid = RowId();
	UnicodeString		name, description;	
	bool				nameModified = false, descriptionModified = false;

	if( !pVs ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_PATH_INALID );
		return true;
	}
	else if( !DoesThisPathExist( *pVs, pResponder ) )
		return true;

	pNewPath = new RedundantDataPath( GetListenManager(), this );
	*pNewPath= *pVs;
	pNewPath->BuildYourselfFromYourValueSet();

	id = pNewPath->GetDesignatorId();
	pOldPath = (DataPath *)GetManagedObject( &id );
	pOldPath->BuildYourValueSet();

	pPath = new RedundantDataPath( GetListenManager(), this );
	*((ValueSet *)pPath) = *((ValueSet *)pOldPath);
	pPath->OverrideExistingValues( pNewPath );
	pPath->BuildYourselfFromYourValueSet();


	// delete previous strings
	HostConnectionDescriptorRecord row;
	pOldPath->WriteYourselfIntoPtsRow( &row );

	if( pPath->GetName().GetLength() && ( pPath->GetName() != pOldPath->GetName() ) ){
		WRITE_STRING_CELL *pCell = new WRITE_STRING_CELL(this); // to be deleted in callback
		m_pStringResourceManager->DeleteString( pOldPath->GetRidName(), (pTSCallback_t)METHOD_ADDRESS(ConnectionManager,DoNothing), NULL );
		pCell->id = pOldPath->GetDesignatorId();
		pCell->context = STRING_OP_CONTEXT_PATH_NAME;
		pCell->pRid = new RowId;
		pCell->string = pPath->GetName();
		nameModified = true;
		m_pStringResourceManager->WriteString( pPath->GetName(), pCell->pRid, (pTSCallback_t)METHOD_ADDRESS(ConnectionManager,ModifyPathStringsCallback), pCell );
	}

	if( pPath->GetDescription().GetLength() && (pPath->GetDescription() != pOldPath->GetDescription() ) ){
		WRITE_STRING_CELL *pCell = new WRITE_STRING_CELL(this); // to be deleted in callback
		m_pStringResourceManager->DeleteString( pOldPath->GetRidDescription(), (pTSCallback_t)METHOD_ADDRESS(ConnectionManager,DoNothing), NULL );
		pCell->id = pOldPath->GetDesignatorId();
		pCell->context = STRING_OP_CONTEXT_PATH_DESCRIPTION;
		pCell->pRid = new RowId;
		pCell->string = pPath->GetDescription();
		descriptionModified = true;
		m_pStringResourceManager->WriteString( pPath->GetDescription(), pCell->pRid, (pTSCallback_t)METHOD_ADDRESS(ConnectionManager,ModifyPathStringsCallback), pCell );
	}

	if( descriptionModified && nameModified )
		m_isAnotherStringUpdatePending = true;

	pResponder->RespondToRequest( SSAPI_RC_SUCCESS );

	delete pPath;
	delete pNewPath;
	return true;
}


STATUS 
ConnectionManager::ModifyPathStringsCallback( void *pContext, STATUS rc ){

	WRITE_STRING_CELL				*pCell = (WRITE_STRING_CELL *)pContext;
	DataPath						*pPath = (DataPath *)pCell->pThis->GetManagedObject( &pCell->id );
	HostConnectionDescriptorRecord	row;

	if( pPath ){
		switch( pCell->context ){
			case STRING_OP_CONTEXT_PATH_DESCRIPTION:
				pPath->SetRidDescription( *pCell->pRid );
				pPath->SetDescription( pCell->string, !pCell->pThis->m_isAnotherStringUpdatePending );
				break;

			case STRING_OP_CONTEXT_PATH_NAME:
				pPath->SetRidName( *pCell->pRid );
				pPath->SetName( pCell->string, !pCell->pThis->m_isAnotherStringUpdatePending );
				break;

			default:
				ASSERT(0);
				break;
		}

		if( pCell->pThis->m_isAnotherStringUpdatePending )
			pCell->pThis->m_isAnotherStringUpdatePending = false;

		pPath->WriteYourselfIntoPtsRow( &row );
		pCell->pThis->GetShadowTable(CM_PATH_TABLE)->ModifyRow(	row.rid,
																&row,
																(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,DoNothing),
																NULL );
		pCell->pThis->m_updatesForHostConnPending++;
		pCell->pThis->AddOutstandingReq();
	}

	delete pCell->pRid;
	delete pCell;
	return OK;
}

//************************************************************************
// DoesThisPathExist:
//************************************************************************

bool 
ConnectionManager::DoesThisPathExist( ValueSet &requestParms, SsapiResponder *pResponder ){

	DesignatorId	id;
	int				rc = 1;
	ClassTypeMap	map;

	rc &= requestParms.GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID  );
	rc &= map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_DATA_PATH, id.GetClassId(), true )? 1 : 0;
	rc &= GetManagedObject( &id )? 1 : 0;

	if( !rc ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_PATH_INALID );
		return false;
	}

	return true;
}


//************************************************************************
// PurgeConnIdsFromPathRow:
//
// PURPOSE:		Purges Connection ids from a host connection descriptor
//************************************************************************

void 
ConnectionManager::PurgeConnIdsFromPathRow( HostConnectionDescriptorRecord *pRow, Container &positions ){

	HostConnectionDescriptorRecord	newRow;
	U32								i, newI;
	CONTAINER_ELEMENT				temp;

	memcpy( &newRow, pRow, sizeof(newRow) );
	memset( &newRow.ridEIPs, 0, sizeof(newRow.ridEIPs) );
	for( i = 0, newI = 0; i < pRow->ridEIPCount; i++ ){
		if( positions.Get( temp, (CONTAINER_KEY)i ) )
			;
		else{
			newRow.ridEIPs[newI++] = pRow->ridEIPs[i];
		}
	}
	newRow.ridEIPCount = newI;
	
	SSAPI_TRACE(TRACE_L2, "\nConnectionManager: Purging connections from a datapath..Failover?");

	GetShadowTable(CM_PATH_TABLE)->ModifyRow(	pRow->rid,
												&newRow,
												(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,PurgeConnIdsFromPathRowCallback),
												NULL );

	SSAPI_TRACE( TRACE_L1, "\nConnectionManager purged Conns from a host connection. Ids purged: ", positions.Count() );
	AddOutstandingReq();
	m_updatesForHostConnPending++;
}


STATUS 
ConnectionManager::PurgeConnIdsFromPathRowCallback( void *pContext, STATUS rc ){

	RemoveOutstandingReq();

	return OK;
}



STATUS 
ConnectionManager::AddRemoveConnIdCallback( void *pContext, STATUS rc ){

	SsapiResponder	*pResponder = (SsapiResponder *)pContext;

	if( rc == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);

	return OK;
}



//************************************************************************
// AddConnIdToDataPath:
//
// PURPOSE:		Adds an Id of a Connection to host
//************************************************************************

void 
ConnectionManager::AddConnIdToDataPath( DataPath *pPath, UpstreamConnection *pConn, SsapiResponder *pResponder, U32 flags ){

	HostConnectionDescriptorRecord		row;
	DataPath							*pTempPath;
	U32									errorString;

	// if already added - pull out
	if( pPath->IsYourChild( pConn->GetDesignatorId() ) ){
		printf("\nJosh, stop sending s#$% to me! (Path HAS the conn.)" );
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
		return;
	}

	// 1 -- let's check that if we add a connection, the path will still be valid
	// 2 -- for this, create a temp path object and add the connection to it
	// 3 -- and ask it if it is valid.

	// 1
	pPath->BuildYourValueSet();
	pTempPath = (DataPath *)pPath->CreateInstance();
	((ValueSet &)*pTempPath) = *pPath;
	pTempPath->BuildYourselfFromYourValueSet();
	// 2
	pTempPath->AddChildId( pConn, false );
	// 3
	if( !pTempPath->IsAValidConfiguration( errorString) ){
		delete pTempPath;
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, errorString );
		return;
	}

	// all is fine, update the PTS
	pTempPath->WriteYourselfIntoPtsRow( &row );
	delete pTempPath;

	GetShadowTable(CM_PATH_TABLE)->ModifyRow(	pPath->GetDesignatorId().GetRowId(),
												&row,
												(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,AddRemoveConnIdCallback),
												pResponder );	

	AddOutstandingReq();
	m_updatesForHostConnPending++;
}


//************************************************************************
// RemoveConnIdFromDataPath:
//
// PURPOSE:		Adds an Id of a Connection to host
//************************************************************************

void 
ConnectionManager::RemoveConnIdFromDataPath( DataPath *pPath, DesignatorId connId, SsapiResponder *pResponder ){

	HostConnectionDescriptorRecord	row;
	DataPath						*pTempPath;
	U32								errorString;
	ConnectionBase					*pConn;
	
	// check if already not a part of it
	if( !pPath->IsYourChild( connId ) ){
		printf("\nJosh, stop sending s#$% to me! (Path HAS NOT the conn.)" );
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
		return;
	}

	// 1 -- let's check that if we remove a connection, the path will still be valid
	// 2 -- for this, create a temp path object and remove the connection from it
	// 3 -- and ask it if it is valid.

	// 1
	pPath->BuildYourValueSet();
	pTempPath = (DataPath *)pPath->CreateInstance();
	*pTempPath = (const ValueSet &)*pPath;
	pTempPath->BuildYourselfFromYourValueSet();
	// 2
	pConn = (ConnectionBase *)GetManagedObject( &connId );
	pTempPath->DeleteChildId( pConn, false );
	// 3
	if( !pTempPath->IsAValidConfiguration( errorString ) ){
		delete pTempPath;
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, errorString );
		return;
	}
	
	pTempPath->WriteYourselfIntoPtsRow( &row );
	delete pTempPath;

	GetShadowTable(CM_PATH_TABLE)->ModifyRow(	pPath->GetDesignatorId().GetRowId(),
												&row,
												(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,AddRemoveConnIdCallback),
												pResponder );											
	AddOutstandingReq();
	m_updatesForHostConnPending++;
}


//************************************************************************
// DeleteConnection:
//
// PURPOSE:		Deletes the object from the PTS table
//************************************************************************

void 
ConnectionManager::DeleteConnection( ConnectionBase *pConn, SsapiResponder *pResponder ){


		

	// can not delete good Connections!
	if( pConn->GetState() == SSAPI_OBJECT_STATE_GOOD ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_CONNECTION_IS_VALID );
		return;
	}

#if 0	// TBDGAI
	DataPath				*pPath;
	ManagedObject			*pObj;
	U32						i;		
	
	// first delete all references to this Connection
	for( i = 0; i > GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( (pPath = dynamic_cast< DataPath * > (pObj)) != 0 ){
			if( pPath->IsYourConnection( *pConn ) )
				RemoveConnIdFromDataPath( pPath, pConn->GetDesignatorId(), new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(ConnectionManager, Dummy )) );
		}
	}
#endif

	// now, get rid of this sucker
	GetShadowTable(CM_FC_DB_TABLE)->DeleteRow(	pConn->GetDesignatorId().GetRowId(),
												(pTSCallback_t)METHOD_ADDRESS(ConnectionManager, DeleteConnectionCallback),
												pResponder );
	AddOutstandingReq();

}


STATUS 
ConnectionManager::DeleteConnectionCallback( void *pContext, STATUS rc ){

	SsapiResponder		*pResponder = (SsapiResponder *)pContext;

	RemoveOutstandingReq();

	if( rc == OK ){
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	}
	else{
		pResponder->RespondToRequest(  SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED  );
	}

	return OK;
}

//************************************************************************
// GetAffectedManagedObjects:
//
// PURPOSE:		Performs a look up of the affected objects by any change
//				to the MO with 'id' specified. Puts pointers to affected
//				objects into the container.
//
// RETURN:		number of objects affected
//************************************************************************

U32 
ConnectionManager::GetAffectedManagedObjects( DesignatorId id, Container &affectedObjects ){

	ManagedObject			*pObj;
	U32						i;

	affectedObjects.RemoveAll();

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( pObj->IsYourChild( id ) )
			affectedObjects.Add( (CONTAINER_ELEMENT)pObj );
	}

	return affectedObjects.Count();
}


//************************************************************************
// SetConnectionIds:
//
// PURPOSE:		Associates a vector of connection with a data path.
//				This method is provided to handle multiple updates
//				as an atomic operation instead of using a series of
//				add/remove methods
//************************************************************************

void 
ConnectionManager::SetConnectionIds( DataPath *pPath, ValueSet *pVsIds, SsapiResponder *pResponder ){
	
	DataPath						*pTempPath;
	HostConnectionDescriptorRecord	row;
	DesignatorId					id;
	ManagedObject					*pConn;
	U32								i, errorString;

	// 1 -- let's check that if we update connections, the path will still be valid
	// 2 -- for this, create a temp path object and update its connections
	// 3 -- and ask it if it is valid.

	// 1
	pPath->BuildYourValueSet();
	pTempPath = (DataPath *)pPath->CreateInstance();
	*pTempPath = (const ValueSet& )*pPath;
	pTempPath->BuildYourselfFromYourValueSet();
	// 2 -- delete all connections
	for( i = 0; i < pPath->GetChildCount(); i++ ){	
		id = pPath->GetChildIdAt( i );
		pConn = GetManagedObject( &id );
		pTempPath->DeleteChildId( pConn, false );
	}
	// 2 - add the desired ones
	for( i = 0; i < pVsIds->GetCount(); i++ ){
		pVsIds->GetGenericValue( (char *)&id, sizeof(id), i );
		pConn = GetManagedObject( &id );
		if( !pConn ){
			pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_CONNECTION_ID_INALID);
			delete pTempPath;
			return;
		}
		pTempPath->AddChildId( pConn, false );
	}
	// 3
	if( !pTempPath->IsAValidConfiguration( errorString ) ){
		delete pTempPath;
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, errorString );
		return;
	}

	// all is fine, update the PTS
	pTempPath->WriteYourselfIntoPtsRow( &row );
	delete pTempPath;

	GetShadowTable(CM_PATH_TABLE)->ModifyRow(	pPath->GetDesignatorId().GetRowId(),
												&row,
												(pTSCallback_t)METHOD_ADDRESS(ConnectionManager,AddRemoveConnIdCallback),
												pResponder );	

	AddOutstandingReq();
	m_updatesForHostConnPending++;
}


