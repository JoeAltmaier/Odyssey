//************************************************************************
// FILE:		ManagedObject.cpp
//
// PURPOSE:		Implements class ManagedObject which is used as a base class
//				for every managed object in SSAPI layer
//************************************************************************

#include "ManagedObject.h"
#include "ListenManager.h"
#include "SSAPIAssert.h"
#include "..\ValueSet.h"
#include "..\SSAPI_Codes.h"

// TRACE Facility hook-up
#include "Odyssey_trace.h"

//************************************************************************
// ManagedObject:
//
// PURPOSE:		Default constructor
//************************************************************************

ManagedObject::ManagedObject( ListenManager *pListenManager, U32 objectClassType, ObjectManager *pManager ) 
				: ValueSet( NULL ){

	m_pListenManager	= pListenManager;
	m_objectClassType	= objectClassType;
	m_pManager			= pManager;

}


//************************************************************************
// ~ManagedObject:
//
// PURPOSE:		The destructor
//************************************************************************

ManagedObject::~ManagedObject(){

	DesignatorId		*pId;

	while( m_phsDataObjects.Count() ){
		m_phsDataObjects.GetAt( (CONTAINER_ELEMENT &)pId, 0 );
		m_phsDataObjects.RemoveAt( 0 );
		delete pId;
	}
	while( m_parents.Count() ){
		m_parents.GetAt( (CONTAINER_ELEMENT &)pId, 0 );
		m_parents.RemoveAt( 0 );
		delete pId;
	}
	while( m_children.Count() ){
		m_children.GetAt( (CONTAINER_ELEMENT &)pId, 0 );
		m_children.RemoveAt( 0 );
		delete pId;
	}
}

//************************************************************************
// FireEventObjectModifed:
//
// PURPOSE:		Used by derived objects to fire OBJECT_MODIFED event
//
// RETURN:		true:	success
//				false:	failure
//************************************************************************

bool 
ManagedObject::FireEventObjectModifed(){

	return m_pListenManager->PropagateObjectModifiedEventForObjects( this );
					
}


//************************************************************************
// FireEventChildAdded:
//
// PURPOSE:		Used by derived objects to fire CHILD_ADDED event
//
// RETURN:		true:	success
//				false:	failure
//************************************************************************

bool 
ManagedObject::FireEventChildAdded( ManagedObject *pChild ){

	return m_pListenManager->PropagateChildAddedEvent( m_id, pChild );

}


//************************************************************************
// FireEventChildDeleted:
//
// PURPOSE:		Used by derived objects to fire CHILD_DELETED event
//
// RETURN:		true:	success
//				false:	failure
//************************************************************************

bool 
ManagedObject::FireEventChildDeleted( ManagedObject *pChild ){

	return m_pListenManager->PropagateChildDeletedEvent( m_id, pChild );
}


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		The method is responsible for adding all data to be 
//				transfered to a client into its value set. All derived 
//				objects must override this method if they have data members
//				they want client proxies to have. 
//
// NOTE:		If a derived object overrides this method, it MUST call
//				it in its base class BEFORE doing any work!
//
// RETURN:		true:		success
//************************************************************************

bool 
ManagedObject::BuildYourValueSet(){

	ValueSet			*pTempValueSet;
	
	Clear();

	AddInt(m_objectClassType, SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE );
	AddInt(m_manager.GetClassId(), SSAPI_OBJECT_FID_MANAGER_CLASS_TYPE );
	
	AddGenericValue((char *)&m_id, sizeof( m_id ), SSAPI_OBJECT_FID_ID );	

	pTempValueSet = new ValueSet();
	DumpIdVectorIntoValueSet( pTempValueSet, &m_children );
	AddValue( pTempValueSet, SSAPI_OBJECT_FID_CHILDREN_ID_VECTOR ); 
	
	pTempValueSet->Clear();
	DumpIdVectorIntoValueSet( pTempValueSet, &m_parents );
	AddValue( pTempValueSet, SSAPI_OBJECT_FID_PARENTS_ID_VECTOR ); 

	pTempValueSet->Clear();
	DumpIdVectorIntoValueSet( pTempValueSet, &m_phsDataObjects );
	AddValue( pTempValueSet, SSAPI_OBJECT_FID_PHSDATA_ID_VECTOR ); 
		
	delete pTempValueSet;
	return true;
}


//************************************************************************
// DumpIdVectorIntoValueSet:
//
// PURPOSE:		Populates the valueset specified with the contents of the
//				vector with ids. 
//
// RECEIVE:		pValueSet:		ptr to the valueset to populate
//				pVector:		ptr to the vector to use
//
// RETURN:		true:			success
//************************************************************************

bool 
ManagedObject::DumpIdVectorIntoValueSet( ValueSet *pValueSet, Container *pVector ){

	DesignatorId		*pId;

	pValueSet->Clear();
	
	for( U32 index = 0; index < pVector->Count(); index++ ){
		pVector->GetAt( (CONTAINER_ELEMENT &)pId, index );
		pValueSet->AddGenericValue( (char *)pId, sizeof(DesignatorId), index );
	}

	return true;
}


//************************************************************************
// DumpValueSetIntoIdVector:
//
// PURPOSE:		Populates the vector specified with the contents of the
//				value set. the value set must contain vars of DesignatorIdType
//				and they have to be contigues. 
//
// RECEIVE:		pValueSet:		ptr to the valueset to use
//				pVector:		ptr to the vector to populate
//
// RETURN:		true:			success
//************************************************************************

bool 
ManagedObject::DumpValueSetIntoIdVector( ValueSet *pValueSet, Container *pVector ){

	DesignatorId		*pId, id;

	for( U32 i = 0; pValueSet ; i++ ){
		if( pValueSet->GetGenericValue( (char *)&id, sizeof(id), i ) != TRUE )
			break;

		pId = new DesignatorId; 
		*pId = id;
		pVector->Add( (CONTAINER_ELEMENT) pId );
	}

	return true;
}


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates data members based on the underlying value set
//
// NOTE:		All subclasses that override this method must call to it
//				somewhere in the overriding method
//
// RETURN:		true:		success
//************************************************************************

bool 
ManagedObject::BuildYourselfFromYourValueSet(){

	int				temp,i;
	ValueSet		*pVs;
	DesignatorId	id;

	GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &m_iType );
	GetGenericValue( (char *)&m_id, sizeof( m_id ), SSAPI_OBJECT_FID_ID );
	GetInt(SSAPI_OBJECT_FID_MANAGER_CLASS_TYPE, &temp );

	m_manager = DesignatorId( RowId(), temp );
	
	// children
	pVs = (ValueSet *)GetValue( SSAPI_OBJECT_FID_CHILDREN_ID_VECTOR );
	for( i = 0; pVs && (i < pVs->GetCount()); i++ ){
		pVs->GetGenericValue( (char *)&id, sizeof(id), i );
		if( !IsYourChild( id ) )
			m_children.AddAt( (CONTAINER_ELEMENT)new DesignatorId( id ), m_children.Count() );
	}

	// parents
	pVs = (ValueSet *)GetValue( SSAPI_OBJECT_FID_PARENTS_ID_VECTOR );
	for( i = 0; pVs && (i < m_parents.Count()); i++ ){
		pVs->GetGenericValue( (char *)&id, sizeof(id), i );
		if( !IsYourParent( id ) )
			m_parents.AddAt( (CONTAINER_ELEMENT) new DesignatorId( id ), m_parents.Count() );
	}
		
	return true;
}


//************************************************************************
// DumpObjectVectorIntoValueSet:
//
// PURPOSE:		Populates the valueset specified with the contents of the
//				vector with objects. 
//
// RECEIVE:		pValueSet:		ptr to the valueset to use
//				pVector:		ptr to the vector to populate
//
// RETURN:		true:			success
//************************************************************************

bool 
ManagedObject::DumpObjectVectorIntoValueSet( ValueSet *pValueSet, DesignatorIdVector *pVector ){

	ASSERT( pValueSet );
	ASSERT( pVector );

	ManagedObject		*pManagedObject;
	DesignatorId		id;

	for( U32 i = 0; i < pVector->GetCount(); i++ ){
		pVector->GetAt( pManagedObject, i );
		id = pManagedObject->GetDesignatorId();
		pValueSet->AddGenericValue((char *)&id, sizeof(id), i );
	}

	return true;
}


//************************************************************************
// AddListener:
//
// PURPOSE:		Adds a listener for this object
//
// RECEIVE:		pListner:		value set with params
//
// RETURN:		true:			success
//************************************************************************

bool 
ManagedObject::AddListener( ValueSet *pListener, SsapiResponder *pResponder ){

	U32				listenerType;
	DesignatorId	owner;
	bool			rc = true;

	// get parms
	rc &= pListener->GetInt( SSAPI_MANAGED_OBJECT_ADD_LISTENER_LISTENER_TYPE, (int *)&listenerType )? true : false;

	// do work
	if( listenerType & SSAPI_LISTENER_TYPE_CHILD_ADDED )
		rc &= m_pListenManager->AddListenerForChildAddedEvent( m_id, pResponder->GetSessionID(), CALLBACK_METHOD(pResponder, SSAPI_MANAGED_OBJECT_ADD_LISTENER_EVENT_OBJECT ), m_id );

	if( listenerType & SSAPI_LISTENER_TYPE_CHILD_DELETED )
		rc &= m_pListenManager->AddListenerForChildDeletedEvent( m_id, pResponder->GetSessionID(), CALLBACK_METHOD(pResponder, SSAPI_MANAGED_OBJECT_ADD_LISTENER_EVENT_OBJECT ), m_id );

	if( listenerType & SSAPI_LISTENER_TYPE_OBJECT_MODIFIED )
		rc &= m_pListenManager->AddListenerForObjectModifiedEventForObjects( m_id, pResponder->GetSessionID(), CALLBACK_METHOD(pResponder, SSAPI_MANAGED_OBJECT_ADD_LISTENER_EVENT_OBJECT), m_id, true );
	
	pResponder->RespondToRequest(	rc? SSAPI_RC_SUCCESS : SSAPI_EXCEPTION_INTERNAL,
									rc? 0 : CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_ADD_LISTENER,
									rc? FALSE : TRUE );

	return rc;
}


//************************************************************************
// DeleteListener:
//
// PURPOSE:		Deletes a listener for this object
//
// RECEIVE:		pListner:		value set with params
//				pResponder:		the object used for responding
//
// RETURN:		true:			success
//************************************************************************

bool 
ManagedObject::DeleteListener( ValueSet *pListener, SsapiResponder *pResponder ){
	
	U32				listenerType;
	DesignatorId	owner = m_id;
	bool			rc = true;

	// get parms;
	rc &= pListener->GetInt( SSAPI_MANAGED_OBJECT_DELETE_LISTENER_LISTENER_TYPE, (int *)&listenerType )? true : false;

	// do work
	if( listenerType & SSAPI_LISTENER_TYPE_CHILD_ADDED )
		rc &= m_pListenManager->DeleteListenerForChildAddedEvent( owner, pResponder->GetSessionID(), m_id );

	if( listenerType & SSAPI_LISTENER_TYPE_CHILD_DELETED )
		rc &= m_pListenManager->DeleteListenerForChildDeletedEvent( owner, pResponder->GetSessionID(), m_id );

	if( listenerType & SSAPI_LISTENER_TYPE_OBJECT_MODIFIED )
		rc &= m_pListenManager->DeleteListenerForObjectModifiedEventForObjects( owner, pResponder->GetSessionID(), m_id );
	
	// reply
	pResponder->RespondToRequest(	rc? SSAPI_RC_SUCCESS : SSAPI_EXCEPTION_INTERNAL,
									rc? 0 : CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_DELETE_LISTENER,
									rc? FALSE :	TRUE );
	return rc;
}


//************************************************************************
// AddPhsDataItem:
//
// PURPOSE:		Adds an id to the PHS Data vector if this id has not been
//				there before. When id is added,  OBJECT_MODIFIED event is 
//				posted.
//************************************************************************

void 
ManagedObject::AddPhsDataItem( DesignatorId &id, bool shouldPostEvent ){

	DesignatorId		*pId;

	// check for uniqueness
	for( U32 index = 0; index < m_phsDataObjects.Count(); index++ ){
		m_phsDataObjects.GetAt( (CONTAINER_ELEMENT&) pId, index );
		if( *pId == id )
			return;
	}

	m_phsDataObjects.Add( (CONTAINER_ELEMENT) new DesignatorId( id ) );

	if( shouldPostEvent )
		FireEventObjectModifed();
}


//************************************************************************
// Methods to work with the PHS data vector
//
//************************************************************************

void 
ManagedObject::DeletePhsDataItem( DesignatorId &id, bool shouldPostEvent ){

	DesignatorId	*pId;
	U32				i;
	bool			found = false;

	for( i  = 0; i < m_phsDataObjects.Count(); i++ ){
		m_phsDataObjects.GetAt( (CONTAINER_ELEMENT&) pId, i );
		if( *pId == id ){
			m_phsDataObjects.RemoveAt( i );
			delete pId;
			found = true;
			break;
		}
	}

	if( found ){
		if( shouldPostEvent )
			FireEventObjectModifed();
	}
	else{
		ASSERT(0);
	}
}


DesignatorId
ManagedObject::GetPhsDataItemAt( U32 position ){

	DesignatorId	*pId = NULL;
	int				rc;

	rc = m_phsDataObjects.GetAt( (CONTAINER_ELEMENT&) pId, position );
	ASSERT(rc);

	return pId? *pId : DesignatorId();
}


bool 
ManagedObject::IsYourPhsData( const DesignatorId &id ){

	DesignatorId	*pId;
	U32				i;

	for( i  = 0; i < m_phsDataObjects.Count(); i++ ){
		m_phsDataObjects.GetAt( (CONTAINER_ELEMENT&) pId, i );
		if( *pId == id )
			return true;
	}
	return false;
}


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

bool 
ManagedObject::ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_NOT_SUPPORTED );
}


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

bool 
ManagedObject::DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_NOT_SUPPORTED );
}


//************************************************************************
// AddChildId
//
// PURPOSE:		Adds a child id into the m_children vector. Posts CHILD_ADDED
//				event. 
//
// NOTE:		1. Only adds ids that were not in the vector prior to the call
//				2. Adds a new id to the END of the list
//************************************************************************

void 
ManagedObject::AddChildId( ManagedObject *pObj, bool shouldFireEvent ){

	DesignatorId		*pId;
	U32					i;

	for( i = 0; i < m_children.Count(); i++ ){
		m_children.GetAt( (CONTAINER_ELEMENT &)pId, i );
		if( *pId == pObj->GetDesignatorId() )
			return;
	}


	m_children.AddAt( (CONTAINER_ELEMENT)new DesignatorId( pObj->GetDesignatorId() ), m_children.Count() );
	if( shouldFireEvent )
		FireEventChildAdded( pObj );
}

//************************************************************************
// DeleteChildId:
//
// PURPOSE:		Deletes a child id and posts CHILD_DELETED event
//************************************************************************

void 
ManagedObject::DeleteChildId( ManagedObject *pObj, bool shouldFireEvent ){

	DesignatorId		*pId;
	U32					i;

	for( i = 0; i < m_children.Count(); i++ ){
		m_children.GetAt( (CONTAINER_ELEMENT &)pId, i );
		if( *pId == pObj->GetDesignatorId() ){
			m_children.RemoveAt( i );
			delete pId;
			if( shouldFireEvent )
				FireEventChildDeleted( pObj );
		}
	}
}


void 
ManagedObject::DeleteParentId( DesignatorId id, bool shouldFireEvent ){

	DesignatorId		*pId;
	U32					i;

	for( i = 0; i < m_parents.Count(); i++ ){
		m_parents.GetAt( (CONTAINER_ELEMENT &)pId, i );
		if( *pId == id ){
			m_parents.RemoveAt( i );
			delete pId;
			if( shouldFireEvent )
				FireEventObjectModifed( );
		}
	}
}


//************************************************************************
// AddParentId:
//
// PURPOSE:		Adds a parent id into the m_children vector. Posts 
//				OBJECT_MODIFIED event. 
//
// NOTE:		1. Only adds ids that were not in the vector prior to the call
//				2. Adds a new id to the END of the list
//************************************************************************

void 
ManagedObject::AddParentId( ManagedObject *pObj, bool shouldFireEvent ){

	DesignatorId		*pId;
	U32					i;

	for( i = 0; i < m_parents.Count(); i++ ){
		m_parents.GetAt( (CONTAINER_ELEMENT &)pId, i );
		if( *pId == pObj->GetDesignatorId() )
			return;
	}

	m_parents.AddAt( (CONTAINER_ELEMENT)new DesignatorId( pObj->GetDesignatorId() ), m_parents.Count() );
	if( shouldFireEvent )
		FireEventObjectModifed();
}


//************************************************************************
// GetChild:
//
// PURPOSE:		returns ptr to child object which is located at the
//				'position' in the 'm_children' vector
//************************************************************************

ManagedObject* 
ManagedObject::GetChild( U32 position ){
	
	DesignatorId		*pId = NULL;
	
	m_children.GetAt( (CONTAINER_ELEMENT &)pId, position );

#ifdef _DEBUG
	if( !m_pManager || !m_pManager->GetManagedObject( pId ))
		ASSERT(0);
#endif

	return m_pManager->GetManagedObject( pId ) ;
}


//************************************************************************
// GetParent:
//
// PURPOSE:		returns ptr to parent object which is located at the
//				'position' in the 'm_parents' vector
//************************************************************************

ManagedObject* 
ManagedObject::GetParent( U32 position ){

	DesignatorId		*pId = NULL;
	
	m_parents.GetAt( (CONTAINER_ELEMENT &)pId, position );

#ifdef _DEBUG
	if( !m_pManager || ! m_pManager->GetManagedObject( pId ) )
		ASSERT(0);
#endif
	return m_pManager->GetManagedObject( pId ) ;
}

//************************************************************************
// CheckIfValid:
//
// PURPOSE;		Provided for debug purposes. Asserts if there is a problem
//************************************************************************
#ifdef _DEBUG

void 
ManagedObject::CheckIfValid(){

	if( !m_pListenManager && ! m_pManager )
		ASSERT(0);
}
#endif


//************************************************************************
// OverrideExistingValues
//
// Purpose:		Overrides values in the value set with values present
//				in the value set specified. (useful for ModifyObject() )
//************************************************************************

void 
ManagedObject::OverrideExistingValues( ValueSet *vs ){

	Value			*pValue;
	
	for( int valuesFound = 0, index = 0; valuesFound < vs->GetCount(); index++ ){
		if( (pValue = vs->GetValue( index )) != NULL ){
			AddValue( pValue, index );
			valuesFound++;
		}
	}

}


//************************************************************************
// IsYourChild:
//
// PURPOSE:		Checks if the object has a child with id provided
//************************************************************************

bool 
ManagedObject::IsYourChild( const DesignatorId &id ){
	
	DesignatorId			*pId;
	U32						i;

	for( i = 0; i < m_children.Count(); i++ ){
		m_children.GetAt( (CONTAINER_ELEMENT &)pId, i );
		if( *pId == id )
			return true;
	}

	return false;

}


bool
ManagedObject::IsYourParent( const DesignatorId &id ){

	DesignatorId			*pId;
	U32						i;

	for( i = 0; i < m_parents.Count(); i++ ){
		m_parents.GetAt( (CONTAINER_ELEMENT &)pId, i );
		if( *pId == id )
			return true;
	}

	return false;
}

//************************************************************************
// GetChildIdAt:
//
// PURPOSE:		An accessor
//************************************************************************

DesignatorId
ManagedObject::GetChildIdAt( U32 position ){
	
	DesignatorId	*pId;

	if( m_children.GetAt( (CONTAINER_ELEMENT &)pId, position ) ){
		return *pId;
	}

	ASSERT(0);
	return DesignatorId();
}


//************************************************************************
// GetParentIdAt:
//
// PURPOSE:		An accessor
//************************************************************************

DesignatorId 
ManagedObject::GetParentIdAt( U32 position ){
	
	DesignatorId	*pId;

	if( m_parents.GetAt( (CONTAINER_ELEMENT &)pId, position ) ){
		return *pId;
	}

	ASSERT(0);
	return DesignatorId();
}


//************************************************************************
// FreeMemoryForTheContainerWithIds:
//
// PURPOSE:		Deallocates memory taken by elements in the container
//				specified. Elements must be pointers to DesignatorId
//				class. Deletes all elements from the container.
//************************************************************************

void 
ManagedObject::FreeMemoryForTheContainerWithIds( Container &container ){

	DesignatorId			*pId;

	while( container.Count() ){
		container.GetAt( (CONTAINER_ELEMENT &)pId, 0 );
		container.RemoveAt( 0 );
		delete pId;
	}
}


//******************************************************************************
// CopyIdVector:
//
// PURPOSE:		Copies ids from 'source' into the 'destanation'
//
// NOTE:		The caller is responsible for freeing up the memory allocated
//				for ids in the 'destanation'
//******************************************************************************

void 
ManagedObject::CopyIdVector( Container &source, Container &destanation ){
	
	DesignatorId	*pId;
	U32				index;

	for( index = 0; index < source.Count(); index++ ){
		source.GetAt( (CONTAINER_ELEMENT &)pId, index );
		destanation.AddAt( (CONTAINER_ELEMENT) new DesignatorId( *pId ), index );
	}
}


//******************************************************************************
// GetObjectManager:
//
// PURPOSE:		Gets a pointer to an arbitrary manager object
//******************************************************************************

ObjectManager* 
ManagedObject::GetObjectManager( ObjectManager *pParent, U32 managerClassType ){

	return pParent->GetObjectManager( managerClassType );
}


void 
ManagedObject::PurgeAllPhsIds(){

	FreeMemoryForTheContainerWithIds( m_phsDataObjects );
}