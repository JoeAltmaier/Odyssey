//************************************************************************
// FILE:		ObjectManager.cpp
//
// PURPOSE:		Implements base class ObjectManager.
//************************************************************************



#include "ObjectManager.h"
#include "ListenManager.h"
#include "ManagedObject.h"
#include "..\SSAPI_Codes.h"
#include "SSAPIAssert.h"
#include "SsapiLocalResponder.h"
#include "SList.h"
#include "SsapiEvents.h"
#include "DdmSSAPI.h"
#include "UserManager.h"
#include "SSAPITypes.h"
#include "FilterSet.h"
#include "ConfigIdManager.h"
#include "SsapiAlarms.h"

#include "..\msl\Osheap.h"


// TRACE Facility hook-up
#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS


//************************************************************************
// ObjectManager:
//
// PURPOSE:		The destructor
//************************************************************************

ObjectManager::~ObjectManager(){

	ManagedObject		*pManagedObject;
	QueuedRequest		*pRequest;

	while( m_managedObjects.GetCount() ){
		m_managedObjects.GetAt( pManagedObject, 0 );
		m_managedObjects.RemoveAt( 0 );
		delete pManagedObject;
	}

	while( m_pRequestQueue->Count() ){
		m_pRequestQueue->GetAt( (CONTAINER_ELEMENT&)pRequest, 0);
		m_pRequestQueue->RemoveAt( 0 );
		delete pRequest;
	}
	delete m_pRequestQueue;
	
	m_pListenManager->DeleteListenerForObjectDeletedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pLocalResponder->GetSessionID() );
	delete m_pLocalResponder;
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
ObjectManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){

	bool				rc = false;
	ManagedObject		*pManagedObject;
	DesignatorId		id;
	ObjectManager		*pUserManager = ((DdmSSAPI *)pParentDdmSvs)->GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_USER_MANAGER);

	// readiness check
	if( !IsReadyToServiceRequests() ){
		InqueueRequest( pRequestParms, pResponder, requestCode );
		return true;	// will be serviced as soon as the manager's ready
	}
	
	// dispatch!
	switch( requestCode ){
		case SSAPI_OBJECT_MANAGER_LIST:
			rc = ListObjects( pRequestParms, pResponder ); 
			break;

		case SSAPI_OBJECT_MANAGER_PAGED_LIST:
			rc = ListObjectsPaged( pRequestParms, pResponder );
			break;

		case SSAPI_OBJECT_MANAGER_ADD_LISTENER:
			rc = AddListener( pRequestParms, pResponder );
			break;

		case SSAPI_OBJECT_MANAGER_DELETE_LISTENER:
			rc = DeleteListener( pRequestParms, pResponder );
			break;

		case SSAPI_MANAGED_OBJECT_ADD_LISTENER:
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_MANAGED_OBJECT_ADD_LISTENER_OBJECT_ID);
			pManagedObject = GetManagedObject( &id );

			if( pManagedObject )
				rc = pManagedObject->AddListener( pRequestParms, pResponder );
			else 
				rc = pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION, true );

			break;

		case SSAPI_MANAGED_OBJECT_DELETE_LISTENER:
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_MANAGED_OBJECT_DELETE_LISTENER_OBJECT_ID);
			pManagedObject = GetManagedObject( &id );

			if( pManagedObject )
				rc = pManagedObject->DeleteListener( pRequestParms, pResponder );
			else 
				rc = pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION, true );
			break;

		case SSAPI_OBJECT_MANAGER_ADD_OBJECT:
			if( pRequestParms->GetValue( SSAPI_OBJECT_MANAGER_ADD_OBJECT_OBJECT ) ){
				rc = AddObject( (ValueSet &)*pRequestParms->GetValue( SSAPI_OBJECT_MANAGER_ADD_OBJECT_OBJECT ), pResponder );
			}
			else{
				rc = pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION, true );
			}
			break;

		case SSAPI_OBJECT_MANAGER_MODIFY_OBJECT:
			if( pRequestParms->GetValue( SSAPI_OBJECT_MANAGER_MODIFY_OBJECT_OBJECT ) )
				if (((ValueSet *)pRequestParms->GetValue( SSAPI_OBJECT_MANAGER_MODIFY_OBJECT_OBJECT ))->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID ) )
					if( (pManagedObject = GetManagedObject( &id ) ) != NULL  ){
						rc = pManagedObject->ModifyObject( (ValueSet &)*pRequestParms->GetValue( SSAPI_OBJECT_MANAGER_MODIFY_OBJECT_OBJECT ), pResponder );
						break;
					}

			rc = pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION, true );
			break;

		case SSAPI_OBJECT_MANAGER_DELETE_OBJECT:
			if( pRequestParms->GetValue( SSAPI_OBJECT_MANAGER_DELETE_OBJECT_OBJECT ) )
				if (((ValueSet *)pRequestParms->GetValue( SSAPI_OBJECT_MANAGER_DELETE_OBJECT_OBJECT ))->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID ) )
					if( (pManagedObject = GetManagedObject( &id ) ) != NULL  ){
						rc = pManagedObject->DeleteObject( (ValueSet &)*pRequestParms->GetValue( SSAPI_OBJECT_MANAGER_DELETE_OBJECT_OBJECT ), pResponder );
						break;
					}

			rc = pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION, true );
			break;


		default:
			rc = false;
			break;
	}

	return rc;
}


//************************************************************************
// ObjectManager:
//
// PURPOSE:		Default constructor
//************************************************************************

ObjectManager::ObjectManager(	ListenManager *pListenManager, DesignatorId id,
								DdmServices *pParent)
				:DdmServices(	pParent ){

	m_pListenManager = pListenManager;
	m_id			 = id;
	m_configId		 = 0;
	m_pRequestQueue	 = new SList();
	m_isReadyToServiceRequests = false;

	if( m_id.GetClassId() != SSAPI_MANAGER_CLASS_TYPE_CONFIG_ID_MANAGER )
		m_pConfigIdManager = (ConfigIdManager *)((DdmSSAPI *)pParent)->GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_CONFIG_ID_MANAGER);

	m_pLocalResponder = new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(ObjectManager, ObjectDeletedCallbackHandler) ); 
	m_pListenManager->AddListenerForObjectDeletedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pLocalResponder->GetSessionID(), CALLBACK_METHOD(m_pLocalResponder, 1) );
	//m_pListenManager->AddListenerForObjectAddedEvent( m_id, m_pLocalResponder->GetSessionID(), CALLBACK_METHOD(m_pLocalResponder, 1) );									

	m_lastObjectCount = 0;
	ASSERT( m_pListenManager );
}


//************************************************************************
// ListObjects:
// 
// PURPOSE:		Services a LIST OBJECTS request. 
//
// RECEIVE:		pParms:			in-parms
//				pResponder:		ptr to a responder object
//************************************************************************

bool 
ObjectManager::ListObjects( ValueSet *pParms, SsapiResponder *pResponder ){
	
	ManagedObject			*pManagedObject;
	U32						index;
	ValueSet				*pObjects = new ValueSet();
	ValueSet				*pReturnSet = new ValueSet();
	ValueSet				*pFilterSet;
	DesignatorIdVector		filteredSet;
	
	pFilterSet	= (ValueSet *)pParms->GetValue( SSAPI_OBJECT_MANAGER_LIST_FILTER_SET );

	if( !pFilterSet || !pFilterSet->GetCount() ){	// get all objects
		for( index = 0; index < m_managedObjects.GetCount(); index++ ){
			m_managedObjects.GetAt( pManagedObject, index );
			pManagedObject->BuildYourValueSet();
			pObjects->AddValue( pManagedObject, index );
			pManagedObject->Clear();
		}
	}
	else {											// build a filtered set

		FilterSet		*pFS = new FilterSet( *pFilterSet );
		
		if(!pFS->BuildFilteredSet( m_managedObjects, filteredSet )) {
			delete pFS;
			delete pObjects;
			delete pReturnSet;
			return pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION, true );
		}

		for( index = 0; index < filteredSet.GetCount(); index++ ){
			filteredSet.GetAt( pManagedObject, index );
			pManagedObject->BuildYourValueSet();
			pObjects->AddValue( pManagedObject, index );
			pManagedObject->Clear();
		}

		delete pFS;
	}
	
	index = SSAPI_RC_SUCCESS;	
	ValueSet	*pRcSet = new ValueSet;

	pRcSet->AddInt(index, SSAPI_RETURN_STATUS );
	pReturnSet->AddValue( pRcSet, SSAPI_RETURN_STATUS_SET );
	pReturnSet->AddValue( pObjects, SSAPI_OBJECT_MANAGER_LIST_OBJECT_VECTOR );
	
	pResponder->Respond( pReturnSet, TRUE );

	delete pObjects;
	delete pReturnSet;
	delete pRcSet;

	return true;
}


//************************************************************************
// ListObjectsPaged:
// 
// PURPOSE:		Services a PAGED LIST OBJECTS request. 
//
// RECEIVE:		pParms:			set of parms 
//				pResponder:		ptr to a responder object
//************************************************************************

bool 
ObjectManager::ListObjectsPaged( ValueSet *pParms, SsapiResponder *pResponder ){

	return false;
}


//************************************************************************
// FireObjectAddedEvent:
//
// PURPOSE:		Informs ListenManager about OBJECT_ADDED event
//************************************************************************

bool 
ObjectManager::FireObjectAddedEvent( ManagedObject *pAddedObject ){
	
	return m_pListenManager->PropagateObjectAddedEvent( m_id, pAddedObject );

}


//************************************************************************
// FireObjectDeletedEvent:
//
// PURPOSE:		Informs ListenManager of OBJECT_DELETED event
//************************************************************************

bool 
ObjectManager::FireObjectDeletedEvent( ManagedObject *pDeletedObject ){

	return m_pListenManager->PropagateObjectDeletedEvent( m_id, pDeletedObject );
}


//************************************************************************
// AddListener:
//
// PURPOSE:		Adds a listener for any of OBJECT_ADDED, OBJECT_DELETED,
//				OBJECT_MODIFIED;
//
// RECEIVE:		listener:		a value set with parms
//				pResponder:		a wrapper to respond
//
// NOTE:		Will fail if a listener for this owner and sessionId 
//				already exists.
//************************************************************************

bool 
ObjectManager::AddListener( ValueSet *pListener, SsapiResponder *pResponder ){

	U32				listenerType;
	bool			rc = true;

	// get parms
	rc &= pListener->GetInt( SSAPI_OBJECT_MANAGER_ADD_LISTENER_LISTENER_TYPE, (int *)&listenerType )? true : false;

	// do work
	if( listenerType & SSAPI_LISTENER_TYPE_OBJECT_ADDED )
		rc &= m_pListenManager->AddListenerForObjectAddedEvent( m_id, pResponder->GetSessionID(), CALLBACK_METHOD(pResponder, SSAPI_OBJECT_MANAGER_ADD_LISTENER_EVENT_OBJECT ) );
	
	if( listenerType & SSAPI_LISTENER_TYPE_OBJECT_DELETED )
		rc &= m_pListenManager->AddListenerForObjectDeletedEvent( m_id, pResponder->GetSessionID(), CALLBACK_METHOD(pResponder, SSAPI_OBJECT_MANAGER_ADD_LISTENER_EVENT_OBJECT ) );

	if( listenerType & SSAPI_LISTENER_TYPE_OBJECT_MODIFIED )
		rc &= m_pListenManager->AddListenerForObjectModifiedEventForManagers( m_id, pResponder->GetSessionID(), CALLBACK_METHOD(pResponder, SSAPI_OBJECT_MANAGER_ADD_LISTENER_EVENT_OBJECT ) );

	if( listenerType & SSAPI_LISTENER_TYPE_CHILD_ADDED )
		rc &= m_pListenManager->AddListenerForChildAddedEventForManagers( m_id, pResponder->GetSessionID(), CALLBACK_METHOD(pResponder, SSAPI_OBJECT_MANAGER_ADD_LISTENER_EVENT_OBJECT ) );

	if( listenerType & SSAPI_LISTENER_TYPE_CHILD_DELETED )
		rc &= m_pListenManager->AddListenerForChildDeletedEventForManagers( m_id, pResponder->GetSessionID(), CALLBACK_METHOD(pResponder, SSAPI_OBJECT_MANAGER_ADD_LISTENER_EVENT_OBJECT ) );
	
	// reply
	pResponder->RespondToRequest(	rc? SSAPI_RC_SUCCESS : SSAPI_EXCEPTION_INTERNAL,
									rc? 0 : CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_ADD_LISTENER,
									rc? FALSE :	TRUE );

	return true;
}


//************************************************************************
// DeleteListener
//
// PURPOSE:		Deletes a listener for any of OBJECT_ADDED, OBJECT_DELETED,
//				OBJECT_MODIFIED. 
//
// RECEIVE:		listener:		a value set with parms
//				pResponder:		a wrapper to respond
//
// NOTE:		Will fail if no such listener existed
//************************************************************************

bool 
ObjectManager::DeleteListener( ValueSet *pListener, SsapiResponder *pResponder ){

	U32				listenerType;
	bool			rc = true;

	// get parms
	rc &= pListener->GetInt( SSAPI_OBJECT_MANAGER_DELETE_LISTENER_LISTENER_TYPE, (int *)&listenerType )? true : false;

	// do work
	if( listenerType & SSAPI_LISTENER_TYPE_OBJECT_ADDED )
		rc &= m_pListenManager->DeleteListenerForObjectAddedEvent( m_id, pResponder->GetSessionID() );

	if( listenerType & SSAPI_LISTENER_TYPE_OBJECT_DELETED )
		rc &= m_pListenManager->DeleteListenerForObjectDeletedEvent( m_id, pResponder->GetSessionID() );

	if( listenerType & SSAPI_LISTENER_TYPE_OBJECT_MODIFIED )
		rc &= m_pListenManager->DeleteListenerForObjectModifiedEventForManagers( m_id, pResponder->GetSessionID() );

	if( listenerType & SSAPI_LISTENER_TYPE_CHILD_ADDED )
		rc &= m_pListenManager->DeleteListenerForChildAddedEventForManagers( m_id, pResponder->GetSessionID());

	if( listenerType & SSAPI_LISTENER_TYPE_CHILD_DELETED )
		rc &= m_pListenManager->DeleteListenerForChildDeletedEventForManagers( m_id, pResponder->GetSessionID() );

	// reply
	pResponder->RespondToRequest(	rc? SSAPI_RC_SUCCESS : SSAPI_EXCEPTION_INTERNAL,
									rc? 0 : CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_DELETE_LISTENER,
									rc? TRUE :	FALSE );

	return true;
}


//************************************************************************
// GetManagedObject:
//
// PURPOSE:		Performs a lookup on the collection of all managed objects
//				and returns ptr to the object, NULL if object no found
//************************************************************************

ManagedObject*
ObjectManager::GetManagedObject( DesignatorId	*pId ){

	ManagedObject		*pManagedObject;
#if 0
	for( U32 index = 0; index < m_managedObjects.GetCount(); index++ ){
		m_managedObjects.GetAt( pManagedObject, index );
		if( pManagedObject->GetDesignatorId() == *pId )
			return pManagedObject;
	}
#endif
	m_managedObjects.Get( pManagedObject, *pId );

	return pManagedObject;
}


//************************************************************************
// InqueueRequest:
//
// PURPOSE:		Adds a request into request q
//************************************************************************

bool 
ObjectManager::InqueueRequest( ValueSet *pRequestSet, SsapiResponder *pResponder, U32 requestCode ){

	return m_pRequestQueue->AddAt( (CONTAINER_ELEMENT) new QueuedRequest( pRequestSet, pResponder, requestCode ), 
									m_pRequestQueue->Count() ) ?
			true : false; 
}


//************************************************************************
// DequeueRequest:
//
// PURPOSE:		Removes request from request q
//************************************************************************

bool 
ObjectManager::DequeueRequest( ValueSet* &pRequestSet, SsapiResponder* &pResponder, U32 &requestCode ){

	QueuedRequest			*pRequest;

	if( m_pRequestQueue->Count() == 0 ){
		ASSERT(0);
		return false;
	}

	m_pRequestQueue->GetAt( (CONTAINER_ELEMENT&)pRequest, m_pRequestQueue->Count() - 1 );
	pRequestSet		= pRequest->pRequestSet;
	pResponder		= pRequest->pResponder;
	requestCode		= pRequest->requestCode;
	m_pRequestQueue->RemoveAt( m_pRequestQueue->Count() - 1 );

	return true;
}

//************************************************************************
// SetIsReadyToServiceRequests:
//
// PURPOSE:		Used by derived classes to say when they are
//				ready for service. This will start a retroactive request
//				execution of those requests that got queued up.
//************************************************************************

void 
ObjectManager::SetIsReadyToServiceRequests( bool isReady ){

	ValueSet		*pRequestSet;
	SsapiResponder	*pResponder;
	U32				requestCode;
	
	m_isReadyToServiceRequests = isReady;

	if( m_isReadyToServiceRequests ){
		// service requests on the priprietory queue
		while( m_pRequestQueue->Count() && m_isReadyToServiceRequests ){
			DequeueRequest( pRequestSet, pResponder, requestCode );
			Dispatch( pRequestSet, requestCode, pResponder );
		}
	}

	((DdmSSAPI *)pParentDdmSvs)->SetManagerReady( m_id.GetClassId(), m_isReadyToServiceRequests );
}

//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		Called by the ListenManager to inform about an event.
//				For object deleted event, the method will purge all 
//				references in the designator id vectors. 
//				When done, ObjectDeletedCallbackHandler() is called to
//				other managers ability to habdle object deleted event
//************************************************************************

void 
ObjectManager::ObjectDeletedCallbackHandler( ValueSet *pVs, bool isLast, int eventObjectId ){

	SSAPIEvent		*pEvent;
	int				eventType;
	ManagedObject	*pManagedObject;
	ValueSet		*pChild;
	DesignatorId	id;
	U32				index;

	((ValueSet *)pVs->GetValue( eventObjectId ))->GetInt( SSAPI_EVENT_FID_EVENT_TYPE, &eventType );
	switch( eventType ){
		case SSAPI_EVENT_OBJECT_DELETED:
			pEvent = new SSAPIEventObjectDeleted( NULL );
			pChild = new ValueSet;
			*pEvent = *(ValueSet *)pVs->GetValue( eventObjectId );
			*pChild = *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
			pChild->GetGenericValue( (char *)&id, sizeof( id ), SSAPI_OBJECT_FID_ID );
			delete pChild;
			
			// notify the super class
			ObjectDeletedCallbackHandler( pEvent, isLast );

			//traverse all objects
			for( index = 0; index < m_managedObjects.GetCount(); index++ ){
				m_managedObjects.GetAt( pManagedObject, index );
				PurgeAllIdsFromChildrenVector( id, &pManagedObject->m_children, pManagedObject->GetDesignatorId() );
				PurgeAllIdsFromIdVector( id, pManagedObject->m_parents, pManagedObject->GetDesignatorId() );
				PurgeAllIdsFromIdVector( id, pManagedObject->m_phsDataObjects, pManagedObject->GetDesignatorId() );
			}
			break;

		default:
			return;
	}

	delete pEvent;
}

//************************************************************************
// PurgeAllIdsFromChildrenVector:
//
// PURPOSE:		Walks thru a designator id vector object and deletes 
//				all ids equal to the one specified. Posts CHILD_DELETED
//				event for all deleted ids.
//************************************************************************

void 
ObjectManager::PurgeAllIdsFromChildrenVector( DesignatorId id, Container *pVector, DesignatorId objectInQuestion ){

	DesignatorId						*pId;
	ManagedObject						*pManagedObject;

	for( U32 index = 0; index < pVector->Count(); ){
		pVector->GetAt( (CONTAINER_ELEMENT &)pId, index );
		if( id == *pId ){
			pVector->RemoveAt( index );
			delete pId;
			pManagedObject = GetManagedObject( &id );
			m_pListenManager->PropagateChildDeletedEvent( objectInQuestion, pManagedObject );
		}
		else{
			index++;
		}
	}
}


//************************************************************************
// PurgeAllIdsFromIdVector:
//
// PURPOSE:		Walks thru a vector with ids object and deletes 
//				all ids equal to the one specified. Posts OBJECT_MODIFIED
//				event for all deleted ids.
//************************************************************************

void 
ObjectManager::PurgeAllIdsFromIdVector(	DesignatorId id, Container &vector, 
										DesignatorId objectInQuestion,
										bool shouldDeletePurgedIds ){

	DesignatorId		*pId;
	ManagedObject		*pManagedObject;

	m_managedObjects.Get( pManagedObject, objectInQuestion );

	if( !pManagedObject )
		SSAPI_TRACE( TRACE_L1, ">>>>>>>>>>>>> Could not find the object when purging ids!!!" );


	for( U32 index = 0; index < vector.Count(); ){
		vector.GetAt( (CONTAINER_ELEMENT &)pId, index );
		if( *pId == id ){
			vector.RemoveAt( index );
			if( shouldDeletePurgedIds )
				delete pId;
			m_pListenManager->PropagateObjectModifiedEventForObjects( pManagedObject );
		}
		else{
			index++;
		}
	}
}


//************************************************************************
// AddObjectsIntoManagedObjectsVector:
//
// PURPOSE:		Adds managed objects in the container to the main Vector
//				w/all managed objects. The method checks if such object
//				already exists. If so, it will change it and post 
//				OBJECT_MODIFIED event. If object does not exist, the method
//				will post OBJECT_ADDED event.
//
// NOTE:		The method will remove all objects from the container
//				This is done because some objects may be needed after the
//				call (new ones) and some will not (already existing once )
//************************************************************************

void 
ObjectManager::AddObjectsIntoManagedObjectsVector( Container &container, bool shouldDeleteIfExists ){

	U32				index;
	ManagedObject	*pManagedObject, *pExistingObject;
	DesignatorId	id;

	for( index = 0; index < container.Count(); index++ ){
		container.GetAt( (CONTAINER_ELEMENT&)pManagedObject, index );
		id = pManagedObject->GetDesignatorId();
		pExistingObject = GetManagedObject( &id );

		if( pExistingObject ){
			pManagedObject->m_pManager = this;
			pManagedObject->m_manager = m_id;
			pManagedObject->SetParentDdm( pParentDdmSvs );
			pExistingObject->BuildYourValueSet();
			pManagedObject->BuildYourValueSet();
			if( *pExistingObject == *pManagedObject ){
				if( shouldDeleteIfExists )
					delete pManagedObject;
			}	
			else{
				m_managedObjects.Set( pManagedObject, pManagedObject->GetDesignatorId() );
				pManagedObject->FireEventObjectModifed();
				pManagedObject->Clear();
				if( shouldDeleteIfExists )
					delete pExistingObject;
			}
		}
		else {
			pManagedObject->SetParentDdm( pParentDdmSvs );
			pManagedObject->m_pManager = this;
			pManagedObject->m_manager = m_id;
			m_managedObjects.Add( pManagedObject, pManagedObject->GetDesignatorId() );
			FireObjectAddedEvent( pManagedObject );
		}
	}
	
	if( m_lastObjectCount != GetManagedObjectCount() ){
		m_lastObjectCount = GetManagedObjectCount();
		PrintCurrentObjectCount();
	}
	container.RemoveAll();
}

//************************************************************************
// GetObjectManager:
//
// PURPOSE:		Performs a look-up of an object manager by class type
//				Spits out a NULL if no such manager is around
//************************************************************************

ObjectManager* 
ObjectManager::GetObjectManager( U32 managerClassType ){

	return ((DdmSSAPI *)pParentDdmSvs)->GetObjectManager( managerClassType );
}


//************************************************************************
// DeleteObjectsFromTheSystem:
//
// PURPOSE:		Deletes objects from the managed objects vector and frees
//				mempory they occupied. Posts OBJECT_DELETED event for each
//				object deleted.
//
// NOTE:		The caller must free up the memory allocated by objects
//				in the 'container' passed
//************************************************************************

void 
ObjectManager::DeleteObjectsFromTheSystem( Container &objects ){

	ManagedObject		*pObjectToDelete, *pManagedObject;
	U32					index;
	DesignatorId		id;

	for( index = 0; index < objects.Count(); index++ ){
		objects.GetAt( (CONTAINER_ELEMENT &)pObjectToDelete, index );
		id = pObjectToDelete->GetDesignatorId();
		pManagedObject = GetManagedObject( &id );
		if( pManagedObject ){
			FireObjectDeletedEvent( pManagedObject );
			m_managedObjects.Remove( id );
			delete pManagedObject;
		}
	}
	
	if( m_lastObjectCount != GetManagedObjectCount() ){
		m_lastObjectCount = GetManagedObjectCount();
		PrintCurrentObjectCount();
	}
}


//************************************************************************
// AddObject:
//
// PURPOSE:		Adds an object to the system
//
// NOTE:		Must be overridden by object managers that can add objects
//************************************************************************

bool 
ObjectManager::AddObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_NOT_SUPPORTED );
}


//************************************************************************
// FreeMemoryForTheContainerWithIds:
//
// PURPOSE:		Deallocates memory taken by elements in the container
//				specified. Elements must be pointers to DesignatorId
//				class. Deletes all elements from the container.
//************************************************************************

void 
ObjectManager::FreeMemoryForTheContainerWithIds( Container &container ){

	DesignatorId			*pId;

	while( container.Count() ){
		container.GetAt( (CONTAINER_ELEMENT &)pId, 0 );
		container.RemoveAt( 0 );
		delete pId;
	}
}

//************************************************************************
// GetManagedObject:
//
// PURPOSE:		Performs a lookup of a managed object the 'position' 
//				specified
//************************************************************************

ManagedObject* 
ObjectManager::GetManagedObject( U32 position ){

	ManagedObject	*pObj;

	m_managedObjects.GetAt( pObj, position );

#ifdef _DEBUG
	if( !pObj )
		ASSERT(0);
#endif

	return pObj;
}


//************************************************************************
// BumpUpConfigId:
//
// PURPOSE:		Bumps up system confgi id and broadcasts OBJECT_MODIFIED
//				event on the config id object
//************************************************************************

void 
ObjectManager::BumpUpConfigId() { 

	m_pConfigIdManager->BumpUpConfigId();
}


//************************************************************************
// GetDesignatorIdByRowId:
//
// PURPOSE:		Finds the first managed object whose designator Id
//				contains the row id specified
//
// RETURN:		true:		object found
//				false:		no such object exists
//************************************************************************

bool 
ObjectManager::GetDesignatorIdByRowId( RowId rid, DesignatorId &designatorId ){

	U32				i;
	ManagedObject	*pObj;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( pObj->GetDesignatorId().GetRowId() == rid ){
			designatorId = pObj->GetDesignatorId();
			return true;
		}
	}

//	ASSERT(0);
	return false;
}

//************************************************************************
// RecoverParentIds
//
// PURPOSE:		Recovers parent ids of an object by checking if the object
//				with the same id existed before and copying its parents 
//************************************************************************

void 
ObjectManager::RecoverParentIds( ManagedObject *pNewObj ){

	U32				i;
	DesignatorId	id;
	ManagedObject	*pParent, *pOldObj;

	id = pNewObj->GetDesignatorId();
	pOldObj = GetManagedObject( &id );

	for( i = 0; pOldObj && (i < pOldObj->GetParentCount()); i++ ){
		id = pOldObj->GetParentIdAt( i );
		pParent = GetManagedObject( &id );
		pNewObj->AddParentId( pParent, false );
	}
}


//************************************************************************
// RecoverPhsIds
//
// PURPOSE:		Recovers PHS Data ids of an object by checking if the object
//				with the same id existed before and copying its phs ids 
//************************************************************************

void 
ObjectManager::RecoverPhsIds( ManagedObject *pNewObj ){

	U32				i;
	DesignatorId	id, *pId;
	ManagedObject	*pOldObj;

	id = pNewObj->GetDesignatorId();
	pOldObj = GetManagedObject( &id );

	for( i = 0; pOldObj && (i < pOldObj->m_phsDataObjects.Count() ); i++ ){
		pOldObj->m_phsDataObjects.GetAt( (CONTAINER_ELEMENT &)pId, i );
		pNewObj->AddPhsDataItem( *pId, false );
	}
}


//************************************************************************
// RecoverChildIds
//
// PURPOSE:		Recovers child ids of an object by checking if the object
//				with the same id existed before and copying its children 
//************************************************************************

void 
ObjectManager::RecoverChildIds( ManagedObject *pNewObj ){

	U32				i;
	DesignatorId	id;
	ManagedObject	*pChild, *pOldObj;

	id = pNewObj->GetDesignatorId();
	pOldObj = GetManagedObject( &id );

	for( i = 0; pOldObj && (i < pOldObj->GetChildCount() ); i++ ){
		id = pOldObj->GetChildIdAt( i );
		pChild = GetManagedObject( &id );
		pNewObj->AddChildId( pChild, false );
	}
}


//************************************************************************
// PrintCurrentObjectCount:
//
// PURPOSE:		Prints out the current object count after every change
//************************************************************************

void 
ObjectManager::PrintCurrentObjectCount(){
#if 0
	char			*p;
	StringClass		s = GetName();
	
	s = s + (const StringClass&)StringClass(": object count changed --> ") + StringClass(GetManagedObjectCount());
	p = s.CString();
	
	SSAPI_TRACE( TRACE_L2, "\n");
	SSAPI_TRACE( TRACE_L2, p );

	delete p;
#endif
}


//************************************************************************
// SubmitAlarm:
//
// PURPOSE:		Submits an alarm on behalf of the DdmSSAPI
//************************************************************************

void 
ObjectManager::SubmitAlarm( const Event *pEvent, SsapiAlarmContext *pAlarmContext, bool isUserRemittable ){
	((DdmSSAPI *)pParentDdmSvs)->SubmitAlarm(	pEvent, 
												pAlarmContext->GetSize(), 
												pAlarmContext, 
												isUserRemittable? TRUE : FALSE ); 
}


//************************************************************************
// GetObjectWithPhsId:
//
// PURPOSE:		Searches for the object that has the PHS id specified.
//
// RETURN:		success:	ptr to the object
//				failure:	NULL
//************************************************************************

ManagedObject* 
ObjectManager::GetObjectWithPhsId( DesignatorId &id ){

	ManagedObject	*pObj;
	U32				i;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( pObj->IsYourPhsData( id ) )
			return pObj;
	}
	return NULL;
}
