//******************************************************************************
// FILE:		ListenManager.h
//
// PURPOSE:		Defines ListenManager object that will be repsonsible 
//				for storing listen objects and propagating events
//******************************************************************************

#ifndef	__LISTEN_MANAGER_H__
#define	__LISTEN_MANAGER_H__


#include "CtTypes.h"
#include "ObjectManager.h"
#include "..\utils\DesignatorId.h"
#include "..\ManagedObjects\Listener.h"


#define	SSAPI_LISTEN_OWNER_ID_ANY		DesignatorId()	// listen on events regardles of the owner id

class Container;

class ListenManager {

	// internal hashtable
	Container			*m_pObjectAddedListeners;				// for ObjectManagers
	Container			*m_pObjectDeletedListeners;				// for ObjectManagers
	Container			*m_pObjectModifiedListenersForManagers;	// for ObjectManagers
	Container			*m_pChildAddedListenersForManagers;		// for ObjectManagers
	Container			*m_pChildDeletedListenersForManagers;	// for ObjectManagers
	Container			*m_pChildAddedListeners;				// for ManagedObjects
	Container			*m_pChildDeletedListeners;				// for ManagedObjects
	Container			*m_pObjectModifiedListenersForObjects;	// for ManagedObjects


public:

//******************************************************************************
// ListenManager:
//
// PURPOSE:		Default constructor
//******************************************************************************

ListenManager();


//******************************************************************************
// ~ObjectManager
// 
// PURPOSE:		The destructor
//******************************************************************************

~ListenManager();



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

bool AddListenerForChildAddedEventForManagers(	DesignatorId	owner, 
												SESSION_ID		session,
												CALLBACK_METHOD	callback );


//******************************************************************************
// DeleteListenerForChildAddedEventForManagers:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool DeleteListenerForChildAddedEventForManagers(DesignatorId owner, 
												 SESSION_ID session );


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

bool AddListenerForChildDeletedEventForManagers(DesignatorId	owner, 
												SESSION_ID		session,
												CALLBACK_METHOD	callback );


//******************************************************************************
// DeleteListenerForChildDeletedEventForManagers:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool DeleteListenerForChildDeletedEventForManagers(	DesignatorId owner, 
													SESSION_ID session );


//******************************************************************************
// AddListenerForObjectAddedEvent:
//
// PURPOSE:		Adds a listener object to listen for OBJECT_ADDED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to reply to clients
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool AddListenerForObjectAddedEvent(	DesignatorId	owner, 
										SESSION_ID		session,
										CALLBACK_METHOD	callback );


//******************************************************************************
// DeleteListenerForObjectAddedEvent:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool DeleteListenerForObjectAddedEvent( DesignatorId owner, SESSION_ID session );


//******************************************************************************
// AddListenerForObjectDeletedEvent:
//
// PURPOSE:		Adds a listener object to listen for OBJECT_DELTED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to reply to clients
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool AddListenerForObjectDeletedEvent(DesignatorId		owner, 
									  SESSION_ID		session,
									  CALLBACK_METHOD	callback);


//******************************************************************************
// DeleteListenerForObjectDeletedEvent:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool DeleteListenerForObjectDeletedEvent( DesignatorId owner, SESSION_ID session );


//******************************************************************************
// AddListenerForObjectModifiedEventForManagers:
//
// PURPOSE:		Adds a listener object to listen for OBJECT_MODIFIED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to reply to clients
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool AddListenerForObjectModifiedEventForManagers(DesignatorId		owner, 
												  SESSION_ID		session,
												  CALLBACK_METHOD	callback);


//******************************************************************************
// DeleteListenerForObjectModifiedEventForManagers:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool DeleteListenerForObjectModifiedEventForManagers(DesignatorId owner, SESSION_ID session );


//******************************************************************************
// AddListenerForObjectModifiedEventForObjects:
//
// PURPOSE:		Adds a listener object to listen for OBJECT_MODIFIED events
//
// RECEIVE:		owner:		id of the object who wants to listen on the event
//				session:	the session id of the owning session
//				callback:	a means to reply to clients
//
// RETURN:		true:		success
//
// NOTE:		operation fails if owner already exists
//******************************************************************************

bool AddListenerForObjectModifiedEventForObjects(	DesignatorId	owner, 
													SESSION_ID		session,
													CALLBACK_METHOD	callback,
													DesignatorId	objectInQuestion,
													bool			shouldNotifyManager);


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

bool DeleteListenerForObjectModifiedEventForObjects(	DesignatorId	owner, 
														SESSION_ID		session,
														DesignatorId	objectInQuestion );


//******************************************************************************
// AddListenerForChildAddedEvent:
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

bool AddListenerForChildAddedEvent(DesignatorId		owner, 
								   SESSION_ID		session,
								   CALLBACK_METHOD	callback,
								   DesignatorId		objectInQuestion);


//******************************************************************************
// DeleteListenerForChildAddedEvent:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool DeleteListenerForChildAddedEvent(	DesignatorId	owner,
										SESSION_ID		session,
										DesignatorId	objectInQuestion);


//******************************************************************************
// AddListenerForChildDeletedEvent:
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

bool AddListenerForChildDeletedEvent(DesignatorId		owner, 
									 SESSION_ID			session,
									 CALLBACK_METHOD	callback,
									 DesignatorId		objectInQuestion);


//******************************************************************************
// DeleteListenerForChildDeletedEvent:
//
// PURPOSE:		Deletes a listener object for the given owner
//
// RECEIVE:		owner:		id of the object who was listening
//
// RETURN:		true:		success
//******************************************************************************

bool DeleteListenerForChildDeletedEvent(DesignatorId	owner, 
										SESSION_ID		session,
										DesignatorId	objectInQuestion );


//******************************************************************************
// DeleteAllListeners:
//
// PURPOSE:		Deletes all listeners for a given session
//
// RECEIVE:		session:	the session id
//
// RETURN:		true:		success
//******************************************************************************

bool DeleteAllListeners( SESSION_ID session );


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

bool PropagateObjectAddedEvent( DesignatorId managerId, 
								ManagedObject *pAddedObject );


//******************************************************************************
// PropagateObjectDeletedEvent:
//
// PURPOSE:		Propagates OBJECT_DELETED event to all listeners
//
// RECEIVE:		managerId:			object  manager on which the event ocurred
//				pDeletedObject:		ptr to the deleted object
//
// RETURN:		true:				success
//******************************************************************************

bool PropagateObjectDeletedEvent(DesignatorId managerId, 
								 ManagedObject *pDeletedObject );


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

bool PropagateObjectModifiedEventForManagers(DesignatorId managerId, 
											 ManagedObject *pModifiedObject );


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

bool PropagateChildAddedEvent(	DesignatorId	objectInQuestion, 
								ManagedObject	*pChild );


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

bool PropagateChildDeletedEvent(DesignatorId	objectInQuestion, 
								ManagedObject	*pChild );


//******************************************************************************
// PropagateObjectModifiedEventForObjects:
//
// PURPOSE:		Propagates OBJECT_MODIFIED event to all listeners
//
// RECEIVE:		pModifiedObject:	ptr to the modified object
//
// RETURN:		true:				success
//******************************************************************************

bool PropagateObjectModifiedEventForObjects( ManagedObject *pModifiedObject );


//******************************************************************************
//******************************************************************************



private:


//******************************************************************************
// DeleteAllObjectsForOwner:
//
// PURPOSE:		Deletes all listeners for a given owner
//
// RECEIVE:		owner:		the owner id
//
// RETURN:		true:		success
//******************************************************************************

bool DeleteAllObjectsForOwner( DesignatorId owner );


//******************************************************************************
// DeleteAllObjectsForObjectInQuestion:
//
// PURPOSE:		Deletes all listeners for a given object in question
//
// RECEIVE:		objectInQuestion:		the id of the object in question
//
// RETURN:		true:		success
//******************************************************************************

bool DeleteAllObjectsForObjectInQuestion( DesignatorId objectInQuestion );



//******************************************************************************
// FreeCollection:
// 
// PURPOSE:		Frees up the colection specified. Deallocates all memory taken
//******************************************************************************

void FreeCollection( Container *pCollection );


//******************************************************************************
// AddListenerToCollection:
// 
// PURPOSE:		Creates a listener and adds it into the collection. Fails if
//				a listener with this owner already exists.
//******************************************************************************

bool AddListenerToCollection(	Container	*pCollection,	DesignatorId	owner,
								SESSION_ID	session,		CALLBACK_METHOD callback,
								DesignatorId objectInQuestion = DesignatorId(),
								bool		 shouldNotifyParent = false );								


//******************************************************************************
// DeleteListenerFromCollection:
//
// PURPOSE:		Deletes the listener from the collection. Fails if a listener
//				with this owner does not exist.
//******************************************************************************

bool DeleteListenerFromCollection(	Container		*pCollection, 
									DesignatorId	owner, 
									SESSION_ID		session,
									DesignatorId	objectInQuestion = DesignatorId() );


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

bool DeleteAllObjectForDesignatorInCollection(	Container		*pCollection,
												DesignatorId	id,
												bool			isByOwner );


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

bool DeleteAllListeners( Container *pCollection, SESSION_ID session );



//******************************************************************************
// PropagateObjectEventToCollection:
//
// PURPOSE:		Goes thru the collection and checks who registered for this event.
//				Triggers update for all registered object listeners
// 
//******************************************************************************

bool PropagateObjectEventToCollection(	Container		*pCollection,
										DesignatorId	id,
										ManagedObject	*pManagedObject,
										SSAPIEvent		*pEvent);


//******************************************************************************
// PropagateManagerEventToCollection:
//
// PURPOSE:		Goes thru the collection and checks who registered for this event.
//				Triggers update for all registered manager listeners
// 
//******************************************************************************

bool PropagateManagerEventToCollection(	Container		*pCollection,
										DesignatorId	id,
										ManagedObject	*pManagedObject,
										SSAPIEvent		*pEvent);

};

#endif	// __LISTEN_MANAGER_H__