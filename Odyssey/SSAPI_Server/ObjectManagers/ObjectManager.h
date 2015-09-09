//************************************************************************
// FILE:		ObjectManager.h
//
// PURPOSE:		Defines base class ObjectManager. The class defines an 
//				interface and provides default implementation for some
//				methods. This class is an abstract class
//************************************************************************

#ifndef __OBJECT_MANAGER_H__
#define __OBJECT_MANAGER_H__


#include "..\utils\DesignatorId.h"
#include "..\utils\DesignatorIdVector.h"
#include "..\ValueSet.h"
#include "DdmOsServices.h"




class ListenManager;
class ManagedObject;
class SsapiResponder;
class Container;
class SsapiLocalResponder;
class SSAPIEvent;
class SList;
class ConfigIdManager;
class StringResourceManager;
class SsapiAlarmContext;

class ObjectManager : public DdmServices{

	ListenManager				*m_pListenManager;
	Container					*m_pRequestQueue;
	bool						m_isReadyToServiceRequests;
	DesignatorId				m_id;
	SsapiResponder				*m_pLocalResponder;
	DesignatorIdVector			m_managedObjects;
	ConfigIdManager				*m_pConfigIdManager;
	U32							m_lastObjectCount;

	friend class ManagedObject;
	friend class DdmSSAPI;

protected:

	U32							m_configId;			// TBDGAI
	
	

	struct QueuedRequest{
		ValueSet		*pRequestSet;
		SsapiResponder	*pResponder;
		U32				requestCode;
		QueuedRequest( ValueSet *pRS, SsapiResponder *pR, U32 rC ){
			pRequestSet		= pRS;
			pResponder		= pR;
			requestCode		= rC;
		}
	};

public:

//************************************************************************
// ObjectManager:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~ObjectManager();


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

virtual bool Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder);


//************************************************************************
// GetDesignatorId:
//
// PURPOSE:		An accessor
//
// RETURN:		Returns DesigntorId of the manager
//************************************************************************

DesignatorId GetDesignatorId() const { return m_id; }


//************************************************************************
// GetDesignatorIdByRowId:
//
// PURPOSE:		Finds the first managed object whose designator Id
//				contains the row id specified
//
// RETURN:		true:		object found
//				false:		no such object exists
//************************************************************************

bool GetDesignatorIdByRowId( RowId rid, DesignatorId &designatorId );


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName()= 0;


protected:


//************************************************************************
// ObjectManager:
//
// PURPOSE:		Default constructor
//************************************************************************

	ObjectManager( ListenManager *pListenManager, DesignatorId id, DdmServices *pParent );


//************************************************************************
// ListObjects:
// 
// PURPOSE:		Services a LIST OBJECTS request. 
//
// RECEIVE:		filterSet:		set of filters for pipelining
//				pResponder:		ptr to a responder object
//************************************************************************

virtual bool ListObjects( ValueSet *pFilterSet, SsapiResponder *pResponder );


//************************************************************************
// ListObjects:
// 
// PURPOSE:		Services a PAGED LIST OBJECTS request. 
//
// RECEIVE:		pParms:			set of parms 
//				pResponder:		ptr to a responder object
//************************************************************************

virtual bool ListObjectsPaged( ValueSet *pParms, SsapiResponder *pResponder );


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

bool AddListener( ValueSet *pListener, SsapiResponder *pResponder );


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

bool DeleteListener( ValueSet *Listener, SsapiResponder *pResponder );


//************************************************************************
// ObjectManager:
//
// PURPOSE:		Copy constructor, must not be used
//************************************************************************

ObjectManager( const ObjectManager& obj ){}


//************************************************************************
// HandleAlarmRecovered:
//
// PURPOSE:		Called by the SSAPI Ddm to inform the manager that a 
//				previously submitted alarm has been recovered. The default
//				implementation does nothing.
//************************************************************************

virtual void HandleAlarmRecovered( SsapiAlarmContext *pAlarmContext ) {}


//************************************************************************
// GetManagedObjectCount:
//
// PURPOSE:		Returns managed object count
//************************************************************************

U32 GetManagedObjectCount() const { return m_managedObjects.GetCount(); }


//************************************************************************
// GetManagedObject:
//
// PURPOSE:		Performs a lookup of a managed object the 'position' 
//				specified
//************************************************************************

ManagedObject* GetManagedObject( U32 position );


//************************************************************************
// GetManagedObject:
//
// PURPOSE:		Performs a lookup on the collection of all managed objects
//				and returns ptr to the object, NULL if object no found
//************************************************************************

ManagedObject* GetManagedObject( DesignatorId	*pId );


//************************************************************************
// SetIsReadyToServiceRequests:
//
// PURPOSE:		Used by derived classes to say when they are
//				ready for service. This will start a retroactive requst
//				execution of those requests that got queued up.
//************************************************************************

void SetIsReadyToServiceRequests( bool isReady );


//************************************************************************
// IsReadyToServiceRequests:
//
// PURPOSE:		Checks if the manager object is ready to service external
//				requests. 
//************************************************************************

bool IsReadyToServiceRequests() const { return m_isReadyToServiceRequests; }


//************************************************************************
// GetListenManager:
//
// PURPOSE:		An accessor
//
// RETURN:		ptr to the ListenManager
//************************************************************************

ListenManager* GetListenManager() const { return m_pListenManager; }


//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ) = 0;


//************************************************************************
// PurgeAllIdsFromChildrenVector:
//
// PURPOSE:		Walks thru a designator id vector object and deletes 
//				all ids equal to the one specified. Posts CHILD_DELETED
//				event for all deleted ids.
//************************************************************************

void PurgeAllIdsFromChildrenVector( DesignatorId id, Container *pVector, 
									DesignatorId objectInQuestion );


//************************************************************************
// PurgeAllIdsFromIdVector:
//
// PURPOSE:		Walks thru a vector with ids object and deletes 
//				all ids equal to the one specified. Posts OBJECT_MODIFIED
//				event for all deleted ids.
//************************************************************************

void PurgeAllIdsFromIdVector(	DesignatorId id, Container &vector, 
								DesignatorId objectInQuestion,
								bool shouldDeletePurgedIds = true); 


//************************************************************************
// AddObjectsIntoManagedObjectsVector:
//
// PURPOSE:		Adds managed objects in the container to the main Vector
//				w/all managed objects. The method checks if such object
//				already exists. If so, it will change it and post 
//				OBJECT_MODIFIED event;
//************************************************************************

void AddObjectsIntoManagedObjectsVector( Container &container, bool shouldDeleteIfExists = true );


//************************************************************************
// GetObjectManager:
//
// PURPOSE:		Performs a look-up of an object manager by class type
//				Spits out a NULL if no such manager is around
//************************************************************************

ObjectManager* GetObjectManager( U32 managerClassType );


//************************************************************************
// DeleteObjectsFromTheSystem:
//
// PURPOSE:		Deletes objects from the managed objects vector and frees
//				mempory they occupied. Posts OBJECT_DELETED event for each
//				object deleted.
//************************************************************************

void DeleteObjectsFromTheSystem( Container &objects );


//************************************************************************
// AddObject:
//
// PURPOSE:		Adds an object to the system
//
// NOTE:		Must be overridden by object managers that can add objects
//************************************************************************

virtual bool AddObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// FreeMemoryForTheContainerWithIds:
//
// PURPOSE:		Deallocates memory taken by elements in the container
//				specified. Elements must be pointers to DesignatorId
//				class.Deletes all elements from the container.
//************************************************************************

void FreeMemoryForTheContainerWithIds( Container &container );


//************************************************************************
// BumpUpConfigId:
//
// PURPOSE:		Bumps up system confgi id and broadcasts OBJECT_MODIFIED
//				event on the config id object
//************************************************************************

void BumpUpConfigId();


//************************************************************************
// RecoverParentIds
//
// PURPOSE:		Recovers parent ids of an object by checking if the object
//				with the same id existed before and copying its parents 
//************************************************************************

void RecoverParentIds( ManagedObject *pNewObj );


//************************************************************************
// RecoverChildIds
//
// PURPOSE:		Recovers child ids of an object by checking if the object
//				with the same id existed before and copying its children 
//************************************************************************

void RecoverChildIds( ManagedObject *pNewObj );


//************************************************************************
// RecoverPhsIds
//
// PURPOSE:		Recovers PHS Data ids of an object by checking if the object
//				with the same id existed before and copying its phs ids 
//************************************************************************

void RecoverPhsIds( ManagedObject *pNewObj );


//************************************************************************
// SubmitAlarm:
//
// PURPOSE:		Submits an alarm on behalf of the DdmSSAPI
//************************************************************************

void SubmitAlarm( const Event *pEvent, SsapiAlarmContext *pAlarmContext, bool isUserRemittable );


//************************************************************************
// GetObjectWithPhsId:
//
// PURPOSE:		Searches for the object that has the PHS id specified.
//
// RETURN:		success:	ptr to the object
//				failure:	NULL
//************************************************************************

ManagedObject*  GetObjectWithPhsId( DesignatorId &id );

private:

//************************************************************************
// FireObjectAddedEvent:
//
// PURPOSE:		Informs ListenManager about OBJECT_ADDED event
//************************************************************************

bool FireObjectAddedEvent( ManagedObject *pAddedObject );


//************************************************************************
// FireObjectDeletedEvent:
//
// PURPOSE:		Informs ListenManager of OBJECT_DELETED event
//************************************************************************

bool FireObjectDeletedEvent( ManagedObject *pDeletedObject );

//************************************************************************
// InqueueRequest:
//
// PURPOSE:		Adds a request into request q
//************************************************************************

bool InqueueRequest( ValueSet *pRequestSet, SsapiResponder *pResponder, U32 requestCode );


//************************************************************************
// DequeueRequest:
//
// PURPOSE:		Removes request from request q
//************************************************************************

bool DequeueRequest( ValueSet* &pRequestSet, SsapiResponder* &pResponder, U32 &requestCode );



//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		Called by the ListenManager to inform about an event.
//				For object deleted event, the method will purge all 
//				references in the designator id vectors. 
//				When done, ObjectDeletedCallbackHandler() is called to
//				other managers ability to handle OBJECT_DELETED event
//************************************************************************

void ObjectDeletedCallbackHandler( ValueSet *pVs, bool isLast, int eventObjectId );


//************************************************************************
// PrintCurrentObjectCount:
//
// PURPOSE:		Prints out the current object count after every change
//************************************************************************

void PrintCurrentObjectCount();




};

#endif	// __OBJECT_MANAGER_H__



