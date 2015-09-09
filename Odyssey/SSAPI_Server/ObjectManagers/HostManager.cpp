//************************************************************************
// FILE:		HostManager.cpp
//
// PURPOSE:		Defines a class whose instance will be managing Host
//				objects in the O2K product.
//************************************************************************


#include "HostManager.h" 
#include "ShadowTable.h"
#include "CoolVector.h"
#include "SsapiLocalResponder.h"
#include "ClassTypeMap.h"
#include "DdmSsapi.h"
#include "SSAPIEvents.h"
#include "ConnectionManager.h"
#include "StringResourceManager.h"

#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS

HostManager* HostManager::m_pThis	= NULL;

//************************************************************************
// HostManager:
//
 // PURPOSE:		The default constructor
//************************************************************************


HostManager::HostManager( ListenManager *pListenManager, DdmServices *pParent, StringResourceManager *pSRM )
:ObjectManager( pListenManager, DesignatorId(RowId(), SSAPI_MANAGER_CLASS_TYPE_HOST_MANAGER), pParent ){

	m_isIniting					= true;
	m_tablesToRebuild			= 0;
	m_requestsOutstanding		= 0;
	m_pStringResourceManager	= pSRM;
	m_isModifyingConnOnHost		= false;

	SetIsReadyToServiceRequests( false );

	m_pTable = new SSAPI_HOST_MGR_JUMP_TABLE_RECORD[SSAPI_HM_NUMBER_OF_TABLES_USED];


	(m_pTable)->pTableName				= HOST_DESCRIPTOR_TABLE_NAME;
	(m_pTable)->tableMask				= HM_HOST_TABLE;
	(m_pTable)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(HostManager,HostTableRowInserted);
	(m_pTable)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(HostManager,HostTableRowDeleted);
	(m_pTable)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(HostManager,HostTableRowModified);	
	(m_pTable)->pCreateObjectsFromRow	= (HostManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(HostManager,CreateObjectsFromHostRow);
	(m_pTable)->rowSize					= sizeof(HostDescriptorRecord);
	(m_pTable)->pFieldDef				= (fieldDef*)HostDescriptorTable_FieldDefs;
	(m_pTable)->fieldDefSize			= cbHostDescriptor_FieldDefs;


	for( int i = 0; i < SSAPI_HM_NUMBER_OF_TABLES_USED; i++ )
		(m_pTable + i)->pShadowTable	= new ShadowTable(	(m_pTable + i)->pTableName,
															this,
															(m_pTable + i)->pRowInsertedCallback,
															(m_pTable + i)->pRowDeletedCallback,
															(m_pTable + i)->pRowModifiedCallback,
															(m_pTable + i)->rowSize );

	m_pObjAddedResponder = new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(HostManager,ObjectAddedEventCallback) ); 
	GetListenManager()->AddListenerForObjectAddedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjAddedResponder->GetSessionID(), CALLBACK_METHOD(m_pObjAddedResponder, 1) );
	
	m_pObjModifiedResponder = new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(HostManager,ObjectModifiedEventCallback) ); 
	GetListenManager()->AddListenerForObjectModifiedEventForManagers( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjModifiedResponder->GetSessionID(), CALLBACK_METHOD(m_pObjModifiedResponder, 1) );
	
	SSAPI_TRACE( TRACE_L2, "\nHostManager: Initializing..." );

	DefineAllTables(); 
	AddOutstandingReq();
}


//************************************************************************
// ~HostManager:
//
// PURPOSE:		The destructor
//************************************************************************

HostManager::~HostManager(){

	GetListenManager()->DeleteListenerForObjectAddedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjAddedResponder->GetSessionID() );
	delete m_pObjAddedResponder;
	
	GetListenManager()->DeleteListenerForObjectModifiedEventForManagers( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjModifiedResponder->GetSessionID() );
	delete m_pObjModifiedResponder;

	for( int i = 0; i < SSAPI_HM_NUMBER_OF_TABLES_USED; i++ )
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
HostManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){

	DesignatorId			id, ownerId;
	Host					*pHost;
	ValueSet				*pVs;

	if( ObjectManager::Dispatch( pRequestParms, requestCode, pResponder ) )
		return true;

	switch( requestCode ){

		case SSAPI_HM_SET_CONNECTION_IDS:
			pRequestParms->GetGenericValue( (char *)&ownerId, sizeof(ownerId), SSAPI_HM_SET_CONNECTION_IDS_OWNER_ID );
			pVs = (ValueSet *)pRequestParms->GetValue( SSAPI_HM_SET_CONNECTION_IDS_CONNECTION_ID_VECTOR );
			pHost = (Host *)GetManagedObject( &ownerId );

			// validate parms
			if( !pHost || !pVs )
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			else
				SetConnectionIds( pHost, pVs, pResponder );

			break;

		case SSAPI_HM_ADD_CONNECTION_ID:
			pRequestParms->GetGenericValue( (char *)&ownerId, sizeof(ownerId), SSAPI_HM_ADD_CONNECTION_ID_OWNER_ID );		
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_HM_ADD_CONNECTION_ID_CONNECTION_ID );
			pHost = (Host *)GetManagedObject( &ownerId );

			if( pHost )
				AddConnIdToHost( pHost, id, pResponder );
			else
				pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);

			break;

		case SSAPI_HM_REMOVE_CONNECTION_ID:
			pRequestParms->GetGenericValue( (char *)&ownerId, sizeof(id), SSAPI_HM_REMOVE_CONNECTION_ID_OWNER_ID );		
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_HM_ADD_CONNECTION_ID_CONNECTION_ID );
			pHost = (Host *)GetManagedObject( &ownerId );

			if( pHost )
				RemoveConnIdFromHost( pHost, id, pResponder );
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
// AddHost:
//
// PURPOSE:		Adds a new host to the system
//************************************************************************

bool 
HostManager::AddHost( ValueSet &objectValues, SsapiResponder *pResponder ){

	Host					*pHost	= new Host( GetListenManager(), this );
	HostDescriptorRecord	row;
	STATUS					status;
	DesignatorId			id;
	ValueSet				*pConnections;

	*pHost = objectValues;
	pHost->BuildYourselfFromYourValueSet();

	if( !pHost->AreThereAnyTooLongStrings( pResponder ) ){
		delete pHost;
		return true;
	}
	
	pHost->WriteYourSelfIntoHostDescriptorRow( &row );

	row.eipCount = 0;
	pConnections = (ValueSet *)objectValues.GetValue(SSAPI_HOST_FID_CONNECTION_ID_VECTOR);
	// verify that all Connections are valid, and insert them into the row
	for( U32 i = 0; pConnections && (i < pConnections->GetCount()); i++ ){
		pConnections->GetGenericValue( (char *)&id, sizeof(id), i );
		if( !IsValidConnection(id) ){
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_CONNECTION_ID_INALID );
			delete pHost;
			return true;
		}
		else{
			row.eip[row.eipCount++] = id.GetRowId().GetRowID();
		}
	}

	

	status = GetShadowTable( HM_HOST_TABLE )->InsertRow( &row, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS( HostManager, ObjectAddedDeletedModifiedReplyCallback ), pResponder );
	if( status != OK )
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	delete pHost;
	return true;
}



//************************************************************************
// DeleteHost:
//
// PURPOSE:		Deletes the host
//************************************************************************

bool 
HostManager::DeleteHost( DesignatorId id, SsapiResponder *pResponder ){

	Host			*pHost = (Host *)GetManagedObject( &id );
	DesignatorId	childId;
	
	STATUS rc = GetShadowTable( HM_HOST_TABLE )->DeleteRow(	id.GetRowId(),
															(pTSCallback_t)METHOD_ADDRESS( HostManager, ObjectAddedDeletedModifiedReplyCallback ),
															pResponder );
	if( rc != OK )
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	
	return true;
}


//************************************************************************
// ModifyHost: 
//
// PURPOSE:		Modifies host data. The value set may contain an incomplete
//				object, but the ID must be there!!!
//************************************************************************

bool 
HostManager::ModifyHost( ValueSet &objectValues, SsapiResponder *pResponder ){

	Host					*pNewHost = new Host( GetListenManager(), this ), *pOldHost, *pHost= new Host( GetListenManager(), this );
	DesignatorId			id;
	HostDescriptorRecord	row;
	STATUS					rc;

	*pNewHost = objectValues;

	pNewHost->BuildYourselfFromYourValueSet();
	id = pNewHost->GetDesignatorId();

	if( !pNewHost->AreThereAnyTooLongStrings( pResponder ) ){
		delete pHost;
		delete pNewHost;
		return true;
	}

	pOldHost = (Host *)GetManagedObject( &id );
	pOldHost->BuildYourValueSet();
	*((ValueSet *)pHost) = *pOldHost;
	pOldHost->Clear();
	
	pHost->OverrideExistingValues( pNewHost );
	pHost->BuildYourselfFromYourValueSet();
	pHost->WriteYourSelfIntoHostDescriptorRow( &row );

	rc = GetShadowTable( HM_HOST_TABLE )->ModifyRow(pOldHost->GetDesignatorId().GetRowId(),
													&row,
													(pTSCallback_t)METHOD_ADDRESS( HostManager, ObjectAddedDeletedModifiedReplyCallback ),
													pResponder );
	if( rc != OK )
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	else
		AddOutstandingReq();

	delete pNewHost;
	delete pHost;

	return true;
}


//************************************************************************
// Table callbacks for the HostDescriptorTable
//************************************************************************

STATUS 
HostManager::HostTableRowInserted( void *pContext, U32 numberOfRows, ShadowTable *){

	HostDescriptorRecord		*pRow = (HostDescriptorRecord *)pContext;
	Container					*pContainer;

	if( m_isIniting ){
		m_tablesToRebuild	|= HM_HOST_TABLE;
		return OK;
	}
	
	SSAPI_TRACE( TRACE_L3, "\nHostManager::HostTableRowInserted" );
	pContainer = new CoolVector;

	for( int index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromHostRow( pRow, *pContainer, true );
	
	BumpUpConfigId();

	delete pContainer;
	return OK;
}


STATUS 
HostManager::HostTableRowDeleted( void *pContext, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild	|= HM_HOST_TABLE;
		return OK;
	}

	Container				*pContainer; 
	HostDescriptorRecord	*pRow = (HostDescriptorRecord *)pContext;
	U32						index;
	ManagedObject			*pObj;

	SSAPI_TRACE( TRACE_L3, "\nHostManager::HostTableRowDeleted" );
	pContainer = new CoolVector;

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromHostRow( pRow, *pContainer );

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
HostManager::HostTableRowModified( void *pContext, U32 numberOfRows, ShadowTable *p){

	Host					*pHost;
	DesignatorId			id, childId;
	HostDescriptorRecord	*pRow = (HostDescriptorRecord *)pContext;
	U32						rowNum, index, childNum;
	bool					found;

	if( m_isIniting ){
		m_tablesToRebuild	|= HM_HOST_TABLE;
		return OK;
	}

	for( rowNum = 0; rowNum < numberOfRows; rowNum++, pRow++ ){
		GetDesignatorIdByRowId( pRow->rid, id );
		pHost = (Host *)GetManagedObject( &id );
		ASSERT( pHost );
		// find and purge ids that were deleted
		for( childNum = 0; childNum < pHost->GetConnectionIdCount(); childNum++ ){
			pHost->GetConnectionIdAt( childNum, childId );
			for( index = 0, found = false; index < pRow->eipCount; index++ ){
				if( RowId(pRow->eip[index]) == childId.GetRowId() ){
					found = true;
					break;
				}
			}
			if( !found ){ // purge!!!
				pHost->RemoveConnectionId( childId );
			}
		}
	}

	HostTableRowInserted( pContext, numberOfRows, p );
	if( m_requestsOutstanding )
		RemoveOutstandingReq();

	return OK;
}


bool 
HostManager::CreateObjectsFromHostRow( HostDescriptorRecord *pRow, Container &container, bool autoAdd ){
	
	Host					*pHost = new Host( GetListenManager(), this ), *pOldHost;
	U32						i;
	DesignatorId			id;
	CoolVector				purgeIds;	// ids to purge (in case needed)
	CoolVector				autoAddContainer, autoAddConn;
	
	pHost->BuildYourSelfFromHostDescriptorRow( pRow );
	id = pHost->GetDesignatorId();
	pOldHost = (Host *)GetManagedObject( &id );

	if( !autoAdd )
		container.Add( (CONTAINER_ELEMENT) pHost );

	for( i = 0; i < pRow->eipCount; i++ ){
		if( GetConnIdByRowId(pRow->eip[i], id) )
			pHost->AddConnectionId( id );
		else
			purgeIds.Add((CONTAINER_ELEMENT)i, (CONTAINER_KEY)i );
	}

	if( autoAdd ){
		autoAddContainer.Add( (CONTAINER_ELEMENT) pHost );
		AddObjectsIntoManagedObjectsVector( autoAddContainer );
	}

	if( purgeIds.Count() )
		PurgeConnIdFromHostRow( pRow, purgeIds );

	return true;
}


//************************************************************************
// DefineAllTables:
//
// PURPOSE:		Issues requests to the PTS to define all tables if not yet
//				defined
//************************************************************************

STATUS 
HostManager::DefineAllTables(){

	STATUS	status = OK;

	m_builtTablesMask = 0;
	for( int i = 0; i < SSAPI_HM_NUMBER_OF_TABLES_USED; i++ )
		status	|= (m_pTable + i)->pShadowTable->DefineTable(	(m_pTable + i)->pFieldDef,
																(m_pTable + i)->fieldDefSize,
																32,
																(pTSCallback_t)METHOD_ADDRESS(HostManager,DefineAllTablesReplyHandler),
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
HostManager::DefineAllTablesReplyHandler( void *pContext, STATUS status ){

	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & HM_ALL_TABLES) == HM_ALL_TABLES ){
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
HostManager::InitializeAllTables(){

	STATUS status = OK;

	m_builtTablesMask = 0;
	for( int i = 0; i < SSAPI_HM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Initialize(	(pTSCallback_t)METHOD_ADDRESS(HostManager,InitializeAllTablesReplyHandler), 
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
HostManager::InitializeAllTablesReplyHandler( void *pContext, STATUS status ){
	
	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & HM_ALL_TABLES) == HM_ALL_TABLES ){
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
HostManager::EnumerateAllTables(){

	STATUS		status = OK;
	
	m_builtTablesMask = 0;

	for( int i = 0; i < SSAPI_HM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable,
															(pTSCallback_t)METHOD_ADDRESS(HostManager,EnumerateAllTablesReplyHandler),
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
HostManager::EnumerateAllTablesReplyHandler( void *pContext, STATUS rc ){

	U32				mask = (U32)pContext, index;
	CoolVector		container;

	for( int i = 0; i < SSAPI_HM_NUMBER_OF_TABLES_USED; i++ ){
		if( mask == (m_pTable + i)->tableMask ){
			if( m_tablesToRebuild & (m_pTable + i)->tableMask ){
				m_tablesToRebuild &= ~(m_pTable + i)->tableMask;
				delete (m_pTable + i)->pTempTable;
				return (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable, (pTSCallback_t)METHOD_ADDRESS(HostManager,EnumerateAllTablesReplyHandler), (void *)(m_pTable + i)->tableMask);
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

	if( ((m_builtTablesMask & HM_ALL_TABLES) == HM_ALL_TABLES) )
		RemoveOutstandingReq();	

	return OK;
}


//************************************************************************
// RemoveOutstandingReq:
//
// PURPOSE:	
//************************************************************************

void 
HostManager::RemoveOutstandingReq(){ 

	if( (m_requestsOutstanding == 1) && m_isIniting ){
		m_isIniting = false;
		SSAPI_TRACE( TRACE_L2, "\nHostManager: ...Done! Objects built: ", GetManagedObjectCount() );
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
HostManager::ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ){

	ValueSet				*pObjVs = new ValueSet;
	int						classType;
	ClassTypeMap			map;
	DesignatorId			id;
	Host					*pHost;
	U32						i;

	*pObjVs	= *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	pObjVs->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );
	
	// if this is a connection, check if any host has it
	if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_CONNECTION, classType, true ) ){
		for( i = 0; i < GetManagedObjectCount(); i++ ){
			pHost = (Host *)GetManagedObject( i );
			pObjVs->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
			if( pHost->IsYourConnection( id ) ){
				printf("\nHostManager : detected connection deletion, purging the puppy!" );
				RemoveConnIdFromHost(	pHost, 
										id, 
										new SsapiLocalResponder( this, 
																 (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS( HostManager, Dummy) 	) );
			}
		}
	}
	delete pObjVs;
}


//************************************************************************
// AddObject:
//
// PURPOSE:		Adds an object to the system
//
// NOTE:		Override default implementation by calling to AddHost()
//************************************************************************

bool 
HostManager::AddObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	int				classType;
	bool			rc = true;

	objectValues.GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );

	switch( classType ){

		case SSAPI_OBJECT_CLASS_TYPE_HOST:
			rc = AddHost( objectValues, pResponder );
			break;

		default:
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
			break;
	}
	
	return rc;
}	


//************************************************************************
// ObjectAddedDeletedModifiedReplyCallback:
//
// PURPOSE:		Responsible for handling the callback and replying to UI
//************************************************************************

STATUS 
HostManager::ObjectAddedDeletedModifiedReplyCallback( void *pContext, STATUS rc ){

	SsapiResponder	*pResponder = (SsapiResponder *)pContext;

	if( rc == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	return OK;
}


//************************************************************************
// GetShadowTable:
//
// PURPOSE:		Performs a look up of the table by the table mask
//************************************************************************

ShadowTable* 
HostManager::GetShadowTable( U32 tableMask ){

	for( U32 i = 0; i < SSAPI_HM_NUMBER_OF_TABLES_USED; i++ )
		if( m_pTable[i].tableMask == tableMask )
			return m_pTable[i].pShadowTable;


	ASSERT(0);
	return NULL;
}


//************************************************************************
// ObjectAddedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_ADDED events 
//				coming from the listen manager.
//
//				We interested in the following things:
//				NOTHING currently
//************************************************************************

void 
HostManager::ObjectAddedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId ){

#if 0
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

	delete pObjVs;
	delete pEvent;
#endif

}


//************************************************************************
// ObjectModifiedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_MODIFIED events 
//				coming from the listen manager.
//
//				We are interested in the following things:
//				Connection state is changed -> host state may be affected
//************************************************************************

void 
HostManager::ObjectModifiedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId ){

	SSAPIEvent				*pEvent = new SSAPIEventObjectAdded( NULL );
	ValueSet				*pObj = new ValueSet;
	int						classType;
	ClassTypeMap			map;
	Host					*pHost;
	U32						i;
	DesignatorId			id;

	*pEvent = *(ValueSet *)pVs->GetValue( eventObjectId );
	*pObj	= *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	pObj->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );

	// if this is a connection, check if any host has it
	if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_CONNECTION, classType, true ) ){
		for( i = 0; i < GetManagedObjectCount(); i++ ){
			pHost = (Host *)GetManagedObject( i );
			pObj->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
			if( pHost->IsYourConnection( id ) )
				pHost->ComposeYourOverallState();
		}
	}

	delete pObj;
	delete pEvent;
}


//************************************************************************
// PurgeConnIdFromHostRow:
//
// PURPOSE:		Purges ids from a host descriptor
//************************************************************************

void 
HostManager::PurgeConnIdFromHostRow( HostDescriptorRecord *pRow, Container &positions ){

	HostDescriptorRecord	newRow;
	U32						i, newI;
	CONTAINER_ELEMENT		temp;

	memcpy( &newRow, pRow, sizeof(newRow) );
	memset( &newRow.eip, 0 , sizeof(newRow.eip) );
	for( i = 0, newI = 0; i < pRow->eipCount; i++ ){
		if( positions.Get( temp, (CONTAINER_KEY)i ) )
			;
		else
			newRow.eip[newI++] = pRow->eip[i];
	}
	newRow.eipCount = newI;
	
	SSAPI_TRACE(TRACE_L2, "\nHostManager: Purging Connections from a host...Failover?");
	
	GetShadowTable(HM_HOST_TABLE)->ModifyRow(	pRow->rid,
												&newRow,
												(pTSCallback_t)METHOD_ADDRESS(HostManager,PurgeConnIdFromHostRowCallback),
												NULL );

	SSAPI_TRACE( TRACE_L1, "\nHostManager: purged Connection ids from a host: ", positions.Count() );
	AddOutstandingReq();
}


STATUS 
HostManager::PurgeConnIdFromHostRowCallback( void *pContext, STATUS rc ){

	RemoveOutstandingReq();

	return OK;
}


//************************************************************************
// AddConnIdToHost:
//
// PURPOSE:		Adds an Id of a Connection to host
//************************************************************************

void 
HostManager::AddConnIdToHost( Host *pHost, DesignatorId &connId, SsapiResponder *pResponder ){
	
	HostDescriptorRecord		row;

	// if already assigned -> bark!
	if( !IsAvailableConnection( connId ) ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_HM_CONNECTION_UNAVAILABLE );
		return;
	}

	// check against the max
	if( pHost->GetConnectionIdCount() == HOST_DESCRIPTOR_TABLE_EIP_MAX_COUNT ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_HM_TO_MANY_CONNECTIONS );
		return;
	}
	
	pHost->WriteYourSelfIntoHostDescriptorRow( &row );
	row.eip[row.eipCount++] = connId.GetRowId().GetRowID();
	GetShadowTable(HM_HOST_TABLE)->ModifyRow(	row.rid,
												&row,
												(pTSCallback_t)METHOD_ADDRESS(HostManager,AddRemoveConnIdCallback),
												pResponder );	
	
	AddOutstandingReq();
}


//************************************************************************
// RemoveConnIdFromHost:
//
// PURPOSE:		Removes an Id of a Connection from a  host
//************************************************************************

void 
HostManager::RemoveConnIdFromHost( Host *pHost, DesignatorId connId, SsapiResponder *pResponder ){

	HostDescriptorRecord		row, newRow;
	U32							i;
	bool						wasModified = false;

	// if already removed
	if( !pHost->IsYourConnection( connId ) ){
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
		ASSERT(0);
		return;
	}

	pHost->WriteYourSelfIntoHostDescriptorRow( &row );

	memcpy( &newRow, &row, sizeof(row) );
	memset( &newRow.eip, 0, sizeof(newRow.eip) );
	newRow.eipCount = 0;

	for( i = 0; i < row.eipCount; i++ ){
		if( RowId(row.eip[i]) == connId.GetRowId() )
			wasModified = true;
		else
			newRow.eip[newRow.eipCount++] = row.eip[i];
	}

	if( wasModified ){
		GetShadowTable(HM_HOST_TABLE)->ModifyRow(	newRow.rid,
													&newRow,
													(pTSCallback_t)METHOD_ADDRESS(HostManager,AddRemoveConnIdCallback),
													pResponder );											
		AddOutstandingReq();
	}
	else
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_CONNECTION_ID_INALID);

	
}


STATUS 
HostManager::AddRemoveConnIdCallback( void *pContext, STATUS rc ){

	SsapiResponder	*pResponder = (SsapiResponder *)pContext;

	if( rc == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);

	return OK;
}


//************************************************************************
// IsValidConnection:
//
// PURPOSE:		Checks with the ConnectionManager if the given ID
//				is endeed an ID of some Connection object
//************************************************************************

bool 
HostManager::IsValidConnection( const DesignatorId &id ){

	DesignatorId		temp;
	ConnectionManager	*pM = (ConnectionManager *)GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_CONNECTION_MANAGER);

	if( pM->GetDesignatorIdByRowId( id.GetRowId(), temp ) )
		return true;

	return false;
}


//************************************************************************
// GetConnIdByRowId:
//
// PURPOSE:		Queries the ConnectionManager for the id of a Connection
//				object given the RowId.
//
// RETURN:		true:	such object exists and is indeed a Connection object
//				false:	the above condition did not hold for the RowId given
//************************************************************************

bool 
HostManager::GetConnIdByRowId( const RowId &rid, DesignatorId &id ){

	ConnectionManager	*pM = (ConnectionManager *)GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_CONNECTION_MANAGER);
	ClassTypeMap		map;
	
	if( pM->GetDesignatorIdByRowId( rid, id ) ){
		if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_CONNECTION, id.GetClassId(), true ) )
			return true;
	}

	return false;
}


//************************************************************************
// SetConnectionIds:
//
// PURPOSE:		An atomic operation to set a vector of connection ids at
//				one time instead of using add/remove methods
//************************************************************************

void 
HostManager::SetConnectionIds( Host *pHost, ValueSet *pVsIds, SsapiResponder *pResponder ){

	DesignatorId			id;
	HostDescriptorRecord	row;
	U32						i;

	// first off, verify that all connections are available
	for( i = 0; i < pVsIds->GetCount(); i++ ){
		pVsIds->GetGenericValue( (char *)&id, sizeof(id), i );
		if( !pHost->IsYourConnection( id ) && !IsAvailableConnection( id ) ){
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_HM_CONNECTION_UNAVAILABLE );
			return;
		}
	}

	// now check that the # of connections is below available space
	if( pVsIds->GetCount() > HOST_DESCRIPTOR_TABLE_EIP_MAX_COUNT ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_HM_TO_MANY_CONNECTIONS );
		return;
	}

	// looks like we are ready. lets do it!
	pHost->WriteYourSelfIntoHostDescriptorRow( &row );
	row.eipCount = pVsIds->GetCount();
	for( i = 0; i < row.eipCount; i++ ){
		pVsIds->GetGenericValue( (char *)&id, sizeof(id), i );
		row.eip[i] = id.GetRowId().GetRowID();
	}
	
	// send off to the PTS
	GetShadowTable(HM_HOST_TABLE)->ModifyRow(	pHost->GetDesignatorId().GetRowId(),
												&row,
												(pTSCallback_t)METHOD_ADDRESS(HostManager,AddRemoveConnIdCallback),
												pResponder );	
	
	AddOutstandingReq();
}


//************************************************************************
// IsAvailableConnection:
//
// PURPOSE:		Checks if this connection is already owned by a host.
//************************************************************************

bool 
HostManager::IsAvailableConnection( const DesignatorId &id ){

	Host		*pHost;
	U32			i;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pHost = (Host *)GetManagedObject( i );
		if( pHost->IsYourConnection( id ) )
			return false;
	}

	return true;
}


//************************************************************************
// DoConnectionsBelongToSameHost:
//
// PURPOSE:		Checks if all the ids specified in the container are 
//				assigned to the same host
//
// NOTE:		The container contains ptrs to DesignatorId objects.
//				This method DOES NOT do any memory clean up.
//************************************************************************

bool 
HostManager::DoConnectionsBelongToSameHost( Container &connectionIds ){

	DesignatorId		*pId;
	Host				*pHost, *pHostFirst = NULL;
	U32					idNum;

	for( idNum = 0; idNum < connectionIds.Count(); idNum++ ){
		connectionIds.GetAt( (CONTAINER_ELEMENT &)pId, idNum );

		if( !pHostFirst ){ // first time here
			if( (pHostFirst = GetHostByConnection( *pId ) ) == NULL ){
				return false;
			}
		}
		else{
			pHost = GetHostByConnection( *pId );
			if( pHost != pHostFirst )
				return false;
		}
	}

	return true;
}


//************************************************************************
// GetHostByConnection:
//
// PURPOSE:		Performs a look up for a host that has the connection
//				specified.
//
// RETURN:		success: ptr to host object
//				failure: NULL (no such host)
//************************************************************************

Host* 
HostManager::GetHostByConnection( const DesignatorId &connId ){

	U32				i;
	Host			*pHost;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pHost = (Host *)GetManagedObject( i );
		if( pHost->IsYourConnection( connId ) )
			return pHost;
	}

	return NULL;
}


//************************************************************************
// GetHostIdByConnectionId:
//
// PURPOSE:		Looks up a host to which the connection is assigned.
//
// OUTPUT:		on success, hostId contains the id of the host object
//
// RETURN:		success:	true
//				failure:	false
//************************************************************************

bool 
HostManager::GetHostIdByConnectionId( const DesignatorId &connId, DesignatorId &hostId ){

	Host	*pHost = GetHostByConnection( connId );

	if( pHost ){
		hostId = pHost->GetDesignatorId();
		return true;
	}

	return false;
}
