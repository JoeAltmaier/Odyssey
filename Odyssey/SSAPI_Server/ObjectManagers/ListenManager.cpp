//******************************************************************************
// FILE:		ListenManager.h
//
// PURPOSE:		Implements ListenManager object that will be repsonsible 
//				for storing listen objects and propagating events
//******************************************************************************

#include "ListenManager.h"
#include "CoolVector.h"
#include "ManagedObject.h"
#include "SsapiLocalResponder.h"
#include "SSAPIEvents.h"

#include "Trace_Index.h"
#define	TRACE_INDEX TRACE_L1

#ifdef _DEBUG
void TraceEvent( SSAPIEvent *pEvent, SsapiResponder *pResponder, Listener *pListener );
#define TRACE_EVENT(pEvent, pResponder, pListener)	TraceEvent( pEvent, pResponder, pListener );
#else
#define TRACE_EVENT(pEvent, pResponder, pListener)
#endif

//******************************************************************************
// ListenManager:
//
// PURPOSE:		Default constructor
//******************************************************************************

ListenManager::ListenManager() {

	m_pObjectAddedListeners					= new CoolVector;
	m_pObjectDeletedListeners				= new CoolVector;
	m_pObjectModifiedListenersForManagers	= new CoolVector;
	m_pChildAddedListeners					= new CoolVector;
	m_pChildDeletedListeners				= new CoolVector;	
	m_pObjectModifiedListenersForObjects	= new CoolVector;
	m_pChildAddedListenersForManagers		= new CoolVector;
	m_pChildDeletedListenersForManagers		= new CoolVector;
}


//******************************************************************************
// ~ObjectManager
// 
// PURPOSE:		The destructor
//******************************************************************************

ListenManager::~ListenManager(){

	FreeCollection(m_pObjectAddedListeners);
	delete m_pObjectAddedListeners;

	FreeCollection(m_pObjectDeletedListeners);
	delete m_pObjectDeletedListeners;

	FreeCollection(m_pObjectModifiedListenersForManagers);
	delete m_pObjectModifiedListenersForManagers;

	FreeCollection(m_pChildAddedListeners);
	delete m_pChildAddedListeners;

	FreeCollection(m_pChildDeletedListeners);
	delete m_pChildDeletedListeners;

	FreeCollection(m_pObjectModifiedListenersForObjects);
	delete m_pObjectModifiedListenersForObjects;

	FreeCollection(m_pChildAddedListenersForManagers);
	delete m_pChildAddedListenersForManagers;

	FreeCollection(m_pChildDeletedListenersForManagers);
	delete m_pChildDeletedListenersForManagers;
}


//******************************************************************************
// AddListenerForChildAddedEventForManagers:
//
// PURPOSE:		Adds a listener object to listen for CHILD_ADDED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to reply to clients
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool 
ListenManager::AddListenerForChildAddedEventForManagers(DesignatorId	owner, 
														SESSION_ID		session,
														CALLBACK_METHOD	callback ){

	
	return AddListenerToCollection(	m_pChildAddedListenersForManagers,
									owner,
									session,
									callback );

}


//******************************************************************************
// DeleteListenerForChildAddedEventForManagers:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool 
ListenManager::DeleteListenerForChildAddedEventForManagers(	DesignatorId owner, 
														   SESSION_ID session ){

	return DeleteListenerFromCollection(	m_pChildAddedListenersForManagers,
											owner,
											session );
}


//******************************************************************************
// AddListenerForChildDeletedEventForManagers:
//
// PURPOSE:		Adds a listener object to listen for CHILD_DELETED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to reply to clients
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool 
ListenManager::AddListenerForChildDeletedEventForManagers(	DesignatorId	owner, 
															SESSION_ID		session,
															CALLBACK_METHOD	callback ){

	
	return AddListenerToCollection(	m_pChildDeletedListenersForManagers,
									owner,
									session,
									callback );
}


//******************************************************************************
// DeleteListenerForChildDeletedEventForManagers:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool 
ListenManager::DeleteListenerForChildDeletedEventForManagers(	DesignatorId owner, 
																SESSION_ID session ){

	return DeleteListenerFromCollection(	m_pChildDeletedListenersForManagers,
											owner,
											session );
}


//******************************************************************************
// AddListenerForObjectAddedEvent:
//
// PURPOSE:		Adds a listener object to listen for OBJECT_ADDED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to send notifications
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool 
ListenManager::AddListenerForObjectAddedEvent(DesignatorId		owner, 
											  SESSION_ID		session,
											  CALLBACK_METHOD	callback ){

	return AddListenerToCollection(	m_pObjectAddedListeners,
									owner,
									session,
									callback );
}


//******************************************************************************
// DeleteListenerForObjectAddedEvent:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool 
ListenManager::DeleteListenerForObjectAddedEvent( DesignatorId owner, SESSION_ID session ){

	return DeleteListenerFromCollection(	this->m_pObjectAddedListeners,
											owner,
											session );
									
}


//******************************************************************************
// AddListenerForObjectDeletedEvent:
//
// PURPOSE:		Adds a listener object to listen for OBJECT_DELTED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to send notifications
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool 
ListenManager::AddListenerForObjectDeletedEvent(DesignatorId	owner, 
												SESSION_ID		session, 
												CALLBACK_METHOD	callback ){

	return AddListenerToCollection(	this->m_pObjectDeletedListeners,
									owner, 
									session,
									callback );
}


//******************************************************************************
// DeleteListenerForObjectDeletedEvent:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool 
ListenManager::DeleteListenerForObjectDeletedEvent( DesignatorId owner, SESSION_ID session ){

	return DeleteListenerFromCollection(	this->m_pObjectDeletedListeners,
											owner,
											session );
}


//******************************************************************************
// AddListenerForObjectModifiedEventForManagers:
//
// PURPOSE:		Adds a listener object to listen for OBJECT_MODIFIED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to send notifications
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool 
ListenManager::AddListenerForObjectModifiedEventForManagers(DesignatorId	owner, 
															SESSION_ID		session,
															CALLBACK_METHOD	callback){

	return AddListenerToCollection(	this->m_pObjectModifiedListenersForManagers,
									owner,
									session,
									callback );
}


//******************************************************************************
// DeleteListenerForObjectModifiedEventForManagers:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool 
ListenManager::DeleteListenerForObjectModifiedEventForManagers(DesignatorId owner, 
															   SESSION_ID session ){

	return DeleteListenerFromCollection(	this->m_pObjectModifiedListenersForManagers,
											owner,
											session );
}


//******************************************************************************
// AddListenerForObjectModifiedEventForObjects:
//
// PURPOSE:		Adds a listener object to listen for OBJECT_MODIFIED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to send notifications
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool 
ListenManager::AddListenerForObjectModifiedEventForObjects(	DesignatorId	owner, 
															SESSION_ID		session,
															CALLBACK_METHOD	callback,
															DesignatorId	objectInQuestion,
															bool			shouldNotifyManager){

	return AddListenerToCollection(	this->m_pObjectModifiedListenersForObjects,
									owner,
									session,
									callback,
									objectInQuestion,
									shouldNotifyManager );
}


//******************************************************************************
// DeleteListenerForObjectModifiedEventForObjects:
//
// PURPOSE:		Deletes a listener object for the given owner
//				session:	the session id of the owning session
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool 
ListenManager::DeleteListenerForObjectModifiedEventForObjects(	DesignatorId	owner,
																SESSION_ID		session,
																DesignatorId	objectInQuestion ){

	return DeleteListenerFromCollection(	this->m_pObjectModifiedListenersForObjects,
											owner,
											session,
											objectInQuestion );
}


//******************************************************************************
// AddListenerForChildAddedEvent:
//
// PURPOSE:		Adds a listener object to listen for CHILD_ADDED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to send notifications
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool 
ListenManager::AddListenerForChildAddedEvent(DesignatorId		owner, 
											 SESSION_ID			session, 
											 CALLBACK_METHOD	callback,
											 DesignatorId		objectInQuestion){

	return AddListenerToCollection(	this->m_pChildAddedListeners,
									owner,
									session,
									callback,
									objectInQuestion );
}


//******************************************************************************
// DeleteListenerForChildAddedEvent:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool 
ListenManager::DeleteListenerForChildAddedEvent(	DesignatorId	owner,
													SESSION_ID		session,
													DesignatorId	objectInQuestion){

	return DeleteListenerFromCollection(	this->m_pChildAddedListeners,
											owner,
											session,
											objectInQuestion );
}


//******************************************************************************
// AddListenerForChildDeletedEvent:
//
// PURPOSE:		Adds a listener object to listen for CHILD_DELETED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to send notifications
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool 
ListenManager::AddListenerForChildDeletedEvent( DesignatorId	owner, 
												SESSION_ID		session,
												CALLBACK_METHOD	callback,
												DesignatorId	objectInQuestion){

	return AddListenerToCollection(	this->m_pChildDeletedListeners,
									owner,
									session,
									callback,
									objectInQuestion );

}


//******************************************************************************
// DeleteListenerForChildDeletedEvent:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool 
ListenManager::DeleteListenerForChildDeletedEvent( DesignatorId owner,
												   SESSION_ID	session,
												   DesignatorId	objectInQuestion){

	return DeleteListenerFromCollection(	this->m_pChildDeletedListeners,
											owner,
											session,
											objectInQuestion );
}


//******************************************************************************
// DeleteAllListeners:
//
// PURPOSE:		Deletes all listeners for a given session
//
// RECEIVE:		session:	the session id
//
// RETURN:		true:		success
//******************************************************************************

bool 
ListenManager::DeleteAllListeners( SESSION_ID session ){

	DeleteAllListeners( this->m_pChildAddedListeners, session );
	DeleteAllListeners( this->m_pChildDeletedListeners, session );
	DeleteAllListeners( this->m_pObjectAddedListeners, session );
	DeleteAllListeners( this->m_pObjectDeletedListeners, session );
	DeleteAllListeners( this->m_pObjectModifiedListenersForObjects, session );
	DeleteAllListeners( this->m_pObjectModifiedListenersForManagers, session );

	return true;
}


//******************************************************************************
// PropagateObjectAddedEvent:
//
// PURPOSE:		Propagates OBJECT_ADDED event to all listeners
//
// RECEIVE:		managerId:			object manager on which the event ocurred
//				pAddedObject:		ptr to the added object
//
// RETURN:		true:				success
//******************************************************************************

bool 
ListenManager::PropagateObjectAddedEvent(	DesignatorId managerId, 
											ManagedObject *pAddedObject ){


	SSAPIEvent	*pEvent = new SSAPIEventObjectAdded( pAddedObject );

	bool rc = PropagateManagerEventToCollection(	this->m_pObjectAddedListeners,
													managerId, 
													pAddedObject,
													pEvent );
	delete pEvent;
	return rc;									
}


//******************************************************************************
// PropagateObjectDeletedEvent:
//
// PURPOSE:		Propagates OBJECT_DELETED event to all listeners
//
// RECEIVE:		managerId:			object manager on which the event ocurred
//				pDeletedObject:		ptr to the deleted object
//
// RETURN:		true:				success
//******************************************************************************

bool 
ListenManager::PropagateObjectDeletedEvent(	DesignatorId managerId, 
											ManagedObject *pDeletedObject ){

	SSAPIEvent	*pEvent = new SSAPIEventObjectDeleted( pDeletedObject );

	bool rc = PropagateManagerEventToCollection(	this->m_pObjectDeletedListeners,
													managerId,
													pDeletedObject, 
													pEvent );

	this->DeleteAllObjectsForObjectInQuestion( pDeletedObject->GetDesignatorId() );
	//this->DeleteAllObjectsForOwner( pDeletedObject->GetDesignatorId() );

	delete pEvent;
	return rc;
}


//******************************************************************************
// PropagateObjectModifiedEventForManagers:
//
// PURPOSE:		Propagates OBJECT_MODIFIED event to all listeners
//
// RECEIVE:		managerId:			object manager on which the event ocurred
//				pModifiedObject:	ptr to the modified object
//
// RETURN:		true:				success
//******************************************************************************

bool 
ListenManager::PropagateObjectModifiedEventForManagers(	DesignatorId  managerId, 
														ManagedObject *pModifiedObject ){

	
	SSAPIEvent	*pEvent = new SSAPIEventObjectModified( pModifiedObject );

	bool rc = PropagateManagerEventToCollection(	this->m_pObjectModifiedListenersForManagers,
													managerId,
													pModifiedObject,
													pEvent );
	delete pEvent;
	return rc;
}


//******************************************************************************
// PropagateChildAddedEvent:
//
// PURPOSE:		Propagates CHILD_ADDED event to all listeners
//
// RECEIVE:		objectInQuestion:	object on which the event ocurred
//				pChild:				ptr to the added child
//
// RETURN:		true:				success		
//******************************************************************************

bool 
ListenManager::PropagateChildAddedEvent(	DesignatorId	objectInQuestion, 
											ManagedObject	*pChild ){
	
	bool					rc = true;

	SSAPIEvent *pEvent = new SSAPIEventChildAdded( pChild, objectInQuestion );

	rc &= PropagateObjectEventToCollection(	this->m_pChildAddedListeners,
											objectInQuestion,
											pChild,
											pEvent );

	ASSERT( pChild->GetManagerId() != DesignatorId() );

	rc &= PropagateManagerEventToCollection(this->m_pChildAddedListenersForManagers,
											pChild->GetManagerId(),
											pChild,
											pEvent );
	
	delete pEvent;
	return rc;
}


//******************************************************************************
// PropagateChildDeletedEvent:
//
// PURPOSE:		Propagates CHILD_DELETED event to all listeners
//
// RECEIVE:		objectInQuestion:	object on which the event ocurred
//				pChild:				ptr to the deleted child
//
// RETURN:		true:				success		
//******************************************************************************

bool 
ListenManager::PropagateChildDeletedEvent(	DesignatorId	objectInQuestion, 
											ManagedObject	*pChild ){

	bool				rc = true;

	SSAPIEvent	*pEvent = new SSAPIEventChildDeleted( pChild, objectInQuestion );

	rc &= PropagateObjectEventToCollection(	this->m_pChildDeletedListeners,
											objectInQuestion,
											pChild,
											pEvent );

	ASSERT( pChild->GetManagerId() != DesignatorId() );

	rc &= PropagateManagerEventToCollection(this->m_pChildDeletedListenersForManagers,
											pChild->GetManagerId(),
											pChild,
											pEvent );

	delete pEvent;
	return rc;
}


//******************************************************************************
// PropagateObjectModifiedEventForObjects:
//
// PURPOSE:		Propagates OBJECT_MODIFIED event to all listeners
//
// RECEIVE:		pModifiedObject:	ptr to the modified object
//
// RETURN:		true:				success
//******************************************************************************

bool 
ListenManager::PropagateObjectModifiedEventForObjects( ManagedObject *pModifiedObject ){

	U32			index, i;
	Listener	*pListener, *pMListener;
	ValueSet	*pVs = new ValueSet;
	SSAPIEvent	*pEvent = new SSAPIEventObjectModified( pModifiedObject );

	for( index = 0; index < m_pObjectModifiedListenersForObjects->Count(); index++ ){
		m_pObjectModifiedListenersForObjects->GetAt( (CONTAINER_ELEMENT&)pListener, index );
		if( pListener->GetObjectInQuestion() == pModifiedObject->GetDesignatorId() ){
			/// Fire off the OBJECT_MODIFIED for Objects event 
			pVs->AddValue( pEvent, pListener->GetCallback().eventObjectCode );

			TRACE_EVENT(pEvent, pListener->GetCallback().pResponder, pListener );

			pListener->GetCallback().pResponder->Respond( pVs, FALSE, pListener->GetCallback().eventObjectCode );
		}
	}
	for(i=0; i < m_pObjectModifiedListenersForManagers->Count(); i++ ){
		m_pObjectModifiedListenersForManagers->GetAt( (CONTAINER_ELEMENT&)pMListener, i);
		if( (pMListener->GetOwner() == pModifiedObject->GetManagerId()) || ( pMListener->GetOwner() == SSAPI_LISTEN_OWNER_ID_ANY ) ){
			pVs->AddValue( pEvent, pMListener->GetCallback().eventObjectCode );

			TRACE_EVENT(pEvent, pMListener->GetCallback().pResponder, pMListener );

			pMListener->GetCallback().pResponder->Respond( pVs, FALSE, pMListener->GetCallback().eventObjectCode );				
		}
	}
	
	delete pEvent;
	delete pVs;
	return true;
}



//******************************************************************************
// DeleteAllObjectsForOwner:
//
// PURPOSE:		Deletes all listeners for a given owner
//
// RECEIVE:		owner:		the owner id
//
// RETURN:		true:		success
//******************************************************************************

bool 
ListenManager::DeleteAllObjectsForOwner( DesignatorId owner ){

	DeleteAllObjectForDesignatorInCollection( this->m_pChildAddedListeners, owner, true );
	DeleteAllObjectForDesignatorInCollection( this->m_pChildDeletedListeners, owner, true );
	DeleteAllObjectForDesignatorInCollection( this->m_pObjectAddedListeners, owner, true );
	DeleteAllObjectForDesignatorInCollection( this->m_pObjectDeletedListeners, owner, true );
	DeleteAllObjectForDesignatorInCollection( this->m_pObjectModifiedListenersForObjects, owner, true );
	DeleteAllObjectForDesignatorInCollection( this->m_pObjectModifiedListenersForManagers, owner, true );

	return true;
}


//******************************************************************************
// DeleteAllObjectsForObjectInQuestion:
//
// PURPOSE:		Deletes all listeners for a given object in question
//
// RECEIVE:		objectInQuestion:		the id of the object in question
//
// RETURN:		true:		success
//******************************************************************************

bool 
ListenManager::DeleteAllObjectsForObjectInQuestion( DesignatorId objectInQuestion ){
	
	DeleteAllObjectForDesignatorInCollection( this->m_pChildAddedListeners, objectInQuestion, false );
	DeleteAllObjectForDesignatorInCollection( this->m_pChildDeletedListeners, objectInQuestion, false );
	DeleteAllObjectForDesignatorInCollection( this->m_pObjectAddedListeners, objectInQuestion, false );
	DeleteAllObjectForDesignatorInCollection( this->m_pObjectDeletedListeners, objectInQuestion, false );
	DeleteAllObjectForDesignatorInCollection( this->m_pObjectModifiedListenersForObjects, objectInQuestion, false );
	DeleteAllObjectForDesignatorInCollection( this->m_pObjectModifiedListenersForManagers, objectInQuestion, false );

	return true;
}



//******************************************************************************
// FreeCollection:
// 
// PURPOSE:		Frees up the colection specified. Deallocates all memory taken
//******************************************************************************

void 
ListenManager::FreeCollection( Container *pCollection ){
	Listener		*pListener;

	while( pCollection->Count() ){
		pCollection->GetAt( (CONTAINER_ELEMENT &)pListener, 0 );
		pCollection->RemoveAt( 0 );
		delete pListener;
	}
}

//******************************************************************************
// AddListenerToCollection:
// 
// PURPOSE:		Creates a listener and adds it into the collection. Fails if
//				a listener with this owner already exists.
//******************************************************************************

bool 
ListenManager::AddListenerToCollection(Container    *pCollection,	DesignatorId owner,
									   SESSION_ID   session,		CALLBACK_METHOD callback,
									   DesignatorId objectInQuestion, 
									   bool         shouldNotifyParent ){
	
	U32				index;
	Listener		*pListener;

	// first, check if such a listener is already there
	for( index = 0; index < pCollection->Count(); index++ ){
		pCollection->GetAt( (CONTAINER_ELEMENT&)pListener, index );
		if( pListener->GetSessionId() == SSAPI_LOCAL_SESSION_ID )
			continue;

		if( (owner == pListener->GetOwner()) && (session == pListener->GetSessionId()) )
			return false;
	}

	// add this sucker!
	pListener = new Listener( owner, objectInQuestion, session, callback, shouldNotifyParent );

	return pCollection->Add( (CONTAINER_ELEMENT)pListener, (CONTAINER_KEY)pListener )? true : false;
}
									   


//******************************************************************************
// DeleteListenerFromCollection:
//
// PURPOSE:		Deletes the listener from the collection. Fails if a listener
//				with this owner does not exist.
//******************************************************************************

bool 
ListenManager::DeleteListenerFromCollection( Container		*pCollection, 
											 DesignatorId	owner,
											 SESSION_ID		session,
											 DesignatorId	objectInQuestion ){

	U32				index;
	Listener		*pListener;

	for( index = 0; index < pCollection->Count(); index++ ){
		pCollection->GetAt( (CONTAINER_ELEMENT&)pListener, index );
		if( (owner == pListener->GetOwner()) && (session == pListener->GetSessionId()) && (pListener->GetObjectInQuestion() == objectInQuestion) ){
			pCollection->RemoveAt( index );
			delete pListener;
			return true;
		}	
	}

	return false;
}


//******************************************************************************
// DeleteAllObjectForDesignatorInCollection:
//
// PURPOSE:		Purges a collection based on a given Designator ID. May check
//				iether owner or objectInQuestion
//
// RECEIVE:		pCollection:		the collection to work with
//				id:					id to purge on
//				isByOwner:			true - by owner, false - by objectInQuestion
//******************************************************************************

bool 
ListenManager::DeleteAllObjectForDesignatorInCollection(	Container		*pCollection,
															DesignatorId	id,
															bool			isByOwner ){

	U32				index;
	Listener		*pListener;

	for( index = 0; index < pCollection->Count();  ){
		pCollection->GetAt( (CONTAINER_ELEMENT&)pListener, index );
		
		if( isByOwner ){
			if( id == pListener->GetOwner() ){
				pCollection->RemoveAt( index );
				delete pListener;
			}
			else{
				index++;
			}

		}
		else {
			if( id == pListener->GetObjectInQuestion() ){
				pCollection->RemoveAt( index );
				delete pListener;
			}
			else{
				index++;
			}
		}
	}

	return true;
}


//******************************************************************************
// DeleteAllListeners:
//
// PURPOSE:		Deletes all listeners for a given session in a given collection
//
// RECEIVE:		pCollection:	the collection to work with 	
//				session:		the session id
//
// RETURN:		true:			success
//******************************************************************************

bool 
ListenManager::DeleteAllListeners( Container *pCollection, SESSION_ID session ){

	
	U32				index;
	Listener		*pListener;

	for( index = 0; index < pCollection->Count();  ){
		pCollection->GetAt( (CONTAINER_ELEMENT&)pListener, index );
		if( session == pListener->GetSessionId() ){
			pCollection->RemoveAt( index );
			delete pListener;
		}
		else {
			index++;
		}
	}

	return true;
}


//******************************************************************************
// PropagateObjectEventToCollection:
//
// PURPOSE:		Goes thru the collection and checks who registered for this event.
//				Triggers update for all registered object listeners
// 
//******************************************************************************

bool 
ListenManager::PropagateObjectEventToCollection(	Container		*pCollection,
													DesignatorId	id,
													ManagedObject	*pManagedObject,
													SSAPIEvent		*pEvent ){

	U32				index;
	Listener		*pListener;
	ValueSet		*pVs = new ValueSet;
	
	// first pass -- only local session listener are notified
	for( index = 0; index < pCollection->Count(); index++ ){
		pCollection->GetAt( (CONTAINER_ELEMENT&)pListener, index );
		if( ( id == pListener->GetObjectInQuestion() ) && (pListener->GetSessionId() == SSAPI_LOCAL_SESSION_ID ) ){
			// Fire off the event
			pVs->AddValue( pEvent, pListener->GetCallback().eventObjectCode );

			TRACE_EVENT(pEvent, pListener->GetCallback().pResponder, pListener );

			pListener->GetCallback().pResponder->Respond( pVs, FALSE, pListener->GetCallback().eventObjectCode );
			pVs->Clear();
		}
	}

	// second pass -- all but local session listeners are notified
	for( index = 0; index < pCollection->Count(); index++ ){
		pCollection->GetAt( (CONTAINER_ELEMENT&)pListener, index );
		if( ( id == pListener->GetObjectInQuestion() ) && (pListener->GetSessionId() != SSAPI_LOCAL_SESSION_ID ) ){
			pVs->AddValue( pEvent, pListener->GetCallback().eventObjectCode );

			TRACE_EVENT(pEvent, pListener->GetCallback().pResponder, pListener );

			pListener->GetCallback().pResponder->Respond( pVs, FALSE, pListener->GetCallback().eventObjectCode );
			pVs->Clear();
		}
	}

	delete pVs;
	return true;
}


//******************************************************************************
// PropagateManagerEventToCollection:
//
// PURPOSE:		Goes thru the collection and checks who registered for this event.
//				Triggers update for all registered manager listeners
// 
//******************************************************************************

bool 
ListenManager::PropagateManagerEventToCollection(	Container		*pCollection,
													DesignatorId	id,
													ManagedObject	*pManagedObject,
													SSAPIEvent		*pEvent){

	U32				index;
	Listener		*pListener;
	ValueSet		*pVs = new ValueSet;

	for( index = 0; index < pCollection->Count(); index++ ){
		pCollection->GetAt( (CONTAINER_ELEMENT&)pListener, index );
		if( (id == pListener->GetOwner()) || (pListener->GetOwner() == SSAPI_LISTEN_OWNER_ID_ANY ) ){
			pVs->AddValue( pEvent, pListener->GetCallback().eventObjectCode );

			TRACE_EVENT(pEvent, pListener->GetCallback().pResponder, pListener );

			pListener->GetCallback().pResponder->Respond( pVs, FALSE, pListener->GetCallback().eventObjectCode );
			pVs->Clear();
		}
	}
	delete pVs;
	return true;
}


#ifdef _DEBUG
void TraceEvent( SSAPIEvent *pEvent, SsapiResponder *pResponder, Listener *pListener ){

	if( TRACE_INDEX >= TRACE_L4 ){
		
		printf("\nNew Event: %s", pEvent->GetEventName() );
		printf( "\tSession: %d", pResponder->GetSessionID() ); 
		printf("\nOwnerID = ");
		pListener->GetOwner().PrintYourself();
		printf("\tObjID = ");
		pEvent->GetObjectId().PrintYourself();
		printf("\tParentID = ");
		pEvent->GetParentId().PrintYourself();
	}
}
#endif