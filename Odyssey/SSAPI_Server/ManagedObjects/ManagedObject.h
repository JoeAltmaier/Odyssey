//************************************************************************
// FILE:		ManagedObject.h
//
// PURPOSE:		Defines class ManagedObject which is used as a base class
//				for every managed object in SSAPI layer
//************************************************************************

#ifndef __ManagedObject_H
#define __ManagedObject_H

#include "..\valueset.h"
#include "..\utils\DesignatorIdVector.h"
#include "..\ObjectManagers\ObjectManager.h"
#include "CoolVector.h"
#include "DdmOsServices.h"
#include "CtEvent.h"
#include "SSAPIAssert.h"
#include "SSAPITypes.h"
#ifdef WIN32
#pragma pack(4)
#endif


class ListenManager;
class ValueSet;
class SsapiResponder;

class ManagedObject : public ValueSet, public DdmServices {

	friend class ObjectManager;
	friend class StatusReporterInterface;

	ListenManager			*m_pListenManager;	// not owned here. DO NOT delete()!
	U32						m_objectClassType;
	ObjectManager			*m_pManager;

protected:
	
	DesignatorId			m_manager;
	DesignatorId			m_id;
	CoolVector				m_parents;			
	CoolVector				m_children;			
	CoolVector				m_phsDataObjects;	// holds pointers to object ids


	


public:


//************************************************************************
// ManagedObject:
//
// PURPOSE:		Default constructor
//************************************************************************

ManagedObject( ListenManager *pListenManager, U32 objectClassType, ObjectManager *pManager = NULL );


//************************************************************************
//	ManagedObject:
//
//	PURPOSE:		Constructor that can read an object from a binary buffer
//************************************************************************

ManagedObject(char* pBuf) : ValueSet(pBuf) {}


//************************************************************************
// ~ManagedObject:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~ManagedObject();

//************************************************************************
// PURPOSE:		Data member accessors
//************************************************************************

DesignatorId	GetDesignatorId() const { return m_id; }
DesignatorId	GetManagerId()	const	{ return m_manager; }



//************************************************************************
// CheckIfValid:
//
// PURPOSE;		Provided for debug purposes. Asserts if there is a problem
//************************************************************************
#ifdef _DEBUG

virtual void CheckIfValid();
#endif


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		The method is responsible for adding all data to be 
//				transfered to a client into its value set. All derived 
//				objects must override this method if they have data members
//				they want client proxies to have. 
//
// NOTE:		If a derived object overrides this this method, it MUST call
//				it in its base class BEFORE doing any work!.
//
// RETURN:		true:		success
//************************************************************************

virtual bool BuildYourValueSet();


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

virtual bool BuildYourselfFromYourValueSet();


//************************************************************************
// AddListener:
//
// PURPOSE:		Adds a listener for this object
//
// RECEIVE:		pListner:		value set with params
//				pResponder:		the object used for responding
//
// RETURN:		true:			success
//************************************************************************

bool AddListener( ValueSet *pListener, SsapiResponder *pResponder );


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

bool DeleteListener( ValueSet *pListener, SsapiResponder *pResponder );


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

virtual bool ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

virtual bool DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// AddChildId
//
// PURPOSE:		Adds a child id into the m_children vector. Posts CHILD_ADDED
//				event. 
//
// NOTE:		Only adds ids that were not in the vector prior to the call
//************************************************************************

void AddChildId( ManagedObject *pObj, bool shouldFireEvent = true );


//************************************************************************
// DeleteChildId:
//
// PURPOSE:		Deletes a child id and posts CHILD_DELETED event
//************************************************************************

void DeleteChildId( ManagedObject *pObj, bool shouldFireEvent = true );



//************************************************************************
// AddParentId:
//
// PURPOSE:		Adds a parent id into the m_children vector. Posts 
//				OBJECT_MODIFIED event. 
//
// NOTE:		Only adds ids that were not in the vector prior to the call
//************************************************************************

void AddParentId( ManagedObject *pObj, bool shouldFireEvent = true );
void DeleteParentId( DesignatorId id, bool shouldFireEvent = true );


//************************************************************************
// Methods to work with the PHS data vector
//
//************************************************************************

void AddPhsDataItem( DesignatorId&, bool shouldPostEvent = true );
void DeletePhsDataItem( DesignatorId& id, bool shouldPostEvent = true );
DesignatorId GetPhsDataItemAt( U32 position );
U32 GetPhsDataItemCount(){ return m_phsDataObjects.Count(); }
bool IsYourPhsData( const DesignatorId &id );
bool IsYourParent( const DesignatorId &id );
void PurgeAllPhsIds();

//************************************************************************
// OverrideExistingValues
//
// Purpose:		Overrides values in the value set with values present
//				in the value set specified. (useful for ModifyObject() )
//************************************************************************

void OverrideExistingValues( ValueSet *vs );


//************************************************************************
// IsYourChild:
//
// PURPOSE:		Checks if the object has a child with id provided
//************************************************************************

bool IsYourChild( const DesignatorId &id );


virtual const ValueSet& operator=(const ValueSet& obj ) = 0;


//************************************************************************
// Accessors:
//************************************************************************

U32 GetClassType() const { return m_objectClassType; }
U32 GetChildCount() const { return m_children.Count(); }
U32	GetParentCount() const { return m_parents.Count(); }
DesignatorId GetChildIdAt( U32 position );
DesignatorId GetParentIdAt( U32 position );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance() = 0;

protected:


//************************************************************************
// GetChild:
//
// PURPOSE:		returns ptr to child object which is located at the
//				'position' in the 'm_children' vector
//************************************************************************

ManagedObject* GetChild( U32 position );


//************************************************************************
// GetParent:
//
// PURPOSE:		returns ptr to parent object which is located at the
//				'position' in the 'm_parents' vector
//************************************************************************

ManagedObject* GetParent( U32 position );


//************************************************************************
// DumpObjectVectorIntoValueSet:
//
// PURPOSE:		Populates the valueset specified with the contents of the
//				vector with objects. 
//
// RECEIVE:		pValueSet:		ptr to the valueset to populate
//				pVector:		ptr to the vector to use
//
// RETURN:		true:			success
//************************************************************************

bool DumpObjectVectorIntoValueSet( ValueSet *pValueSet, DesignatorIdVector *pVector );


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

bool DumpIdVectorIntoValueSet( ValueSet *pValueSet, Container *pVector );


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

bool DumpValueSetIntoIdVector( ValueSet *pValueSet, Container *pVector );


//******************************************************************************
// CopyIdVector:
//
// PURPOSE:		Copies ids from 'source' into the 'destanation'
//
// NOTE:		The caller is responsible for freeing up the memory allocated
//				for ids in the 'destanation'
//******************************************************************************

void CopyIdVector( Container &source, Container &destanation );


//******************************************************************************
// GetObjectManager:
//
// PURPOSE:		Gets a pointer to an arbitrary manager object
//******************************************************************************

ObjectManager* GetObjectManager( ObjectManager *pParent, U32 managerClassType );


//************************************************************************
// FireEventObjectModifed:
//
// PURPOSE:		Used by derived objects to fire OBJECT_MODIFED event
//
// RETURN:		true:	success
//				false:	failure
//************************************************************************

bool FireEventObjectModifed();


//************************************************************************
// FireEventChildAdded:
//
// PURPOSE:		Used by derived objects to fire CHILD_ADDED event
//
// RETURN:		true:	success
//				false:	failure
//************************************************************************

bool FireEventChildAdded( ManagedObject *pChild );


//************************************************************************
// FireEventChildDeleted:
//
// PURPOSE:		Used by derived objects to fire CHILD_DELETED event
//
// RETURN:		true:	success
//				false:	failure
//************************************************************************

bool FireEventChildDeleted( ManagedObject *pChild );


//************************************************************************
// FreeMemoryForTheContainerWithIds:
//
// PURPOSE:		Deallocates memory taken by elements in the container
//				specified. Elements must be pointers to DesignatorId
//				class. Deletes all elements from the container.
//************************************************************************

void FreeMemoryForTheContainerWithIds( Container &container );


//************************************************************************
// Protected accessors
//************************************************************************

ListenManager* GetListenManager() const { return m_pListenManager; }
ObjectManager* GetManager() const		{ return m_pManager; }	



};






















#endif