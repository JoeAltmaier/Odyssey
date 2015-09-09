//************************************************************************
// FILE:		SSAPIEvents.h
//
// PURPOSE:		Contains class definitions for all possible events 
//				generated in the SSAPI
//
// CLASSES:		SSAPIEvent					base 
//				SSAPIEventObjectAdded
//				SSAPIEventObjectDeleted
//				SSAPIEventObjectModified
//				SSAPIEventChildAdded
//				SSAPIEventChildDeleted
//************************************************************************

#ifndef	__SSAPI_EVENTS_H__
#define	__SSAPI_EVENTS_H__

#include "..\ValueSet.h"
#include "..\SSAPI_Codes.h"
#include "DesignatorId.h"
#include "ManagedObject.h"
#include "SsapiTypes.h"

#ifdef WIN32
#pragma pack(4)
#endif



class SSAPIEvent : public ValueSet{

	U32						m_eventType;
	ManagedObject			*m_pManagedObject;
	LocalizedDateTime		m_timeStamp;
	DesignatorId			m_parentId;			// for child-related events only

protected:

//************************************************************************
// SSAPIEvent:
//
// PURPOSE:		Default constructor
//
// RECEIVE:		pManagedObject:		ptr to the object to be delivered
//				eventType:			type of the event
//************************************************************************

SSAPIEvent( ManagedObject *pManagedObject, U8 eventType, DesignatorId parentId = DesignatorId() );


public:


//************************************************************************
// ~SSAPIEvent:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~SSAPIEvent();


//************************************************************************
// SetTimeStamp:
//
// PURPOSE:		Timestampt is set automatically when an object is created.
//				However, it maybe reset as needed by using this method
//************************************************************************

void SetTimeStamp( LocalizedDateTime &newTimeStamp ) { m_timeStamp = newTimeStamp; }


//************************************************************************
// operator=:
//
//
//************************************************************************

const ValueSet& operator= ( const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


//************************************************************************
// Accessors:
//************************************************************************

U32 GetEventType() const { return m_eventType;}
DesignatorId GetParentId() const { return m_parentId; }
DesignatorId GetObjectId() const { return m_pManagedObject->GetDesignatorId(); }
virtual char* GetEventName() = 0;

private:

//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Builds up a value set of its data members
//************************************************************************

void BuildYourValueSet();

};






//************************************************************************
//					CLASS SSAPIEventObjectAdded
//************************************************************************

class SSAPIEventObjectAdded : public SSAPIEvent {

public:

//************************************************************************
//************************************************************************

SSAPIEventObjectAdded( ManagedObject *pManagedObject ) 
	:SSAPIEvent( pManagedObject, SSAPI_EVENT_OBJECT_ADDED ) {}

virtual char* GetEventName() { return "OBJECT_ADDED"; }
};




//************************************************************************
//					CLASS SSAPIEventObjectDeleted
//************************************************************************

class SSAPIEventObjectDeleted : public SSAPIEvent {

public:

//************************************************************************
//************************************************************************

SSAPIEventObjectDeleted( ManagedObject *pManagedObject ) 
	:SSAPIEvent( pManagedObject, SSAPI_EVENT_OBJECT_DELETED ) {}

virtual char* GetEventName() { return "OBJECT_DELETED"; }

};



//************************************************************************
//					CLASS SSAPIEventObjectModified
//************************************************************************

class SSAPIEventObjectModified : public SSAPIEvent {

public:

//************************************************************************
//************************************************************************

SSAPIEventObjectModified( ManagedObject *pManagedObject ) 
	:SSAPIEvent( pManagedObject, SSAPI_EVENT_OBJECT_MODIFIED ) {}


virtual char* GetEventName() { return "OBJECT_MODIFIED"; }

};


//************************************************************************
//					CLASS SSAPIEventChildAdded
//************************************************************************

class SSAPIEventChildAdded : public SSAPIEvent {

public:

//************************************************************************
//************************************************************************

SSAPIEventChildAdded( ManagedObject *pManagedObject, DesignatorId parentId ) 
	:SSAPIEvent( pManagedObject, SSAPI_EVENT_CHILD_ADDED, parentId ) {}

virtual char* GetEventName() { return "CHILD_ADDED"; }

};



//************************************************************************
//					CLASS SSAPIEventChildDeleted
//************************************************************************

class SSAPIEventChildDeleted : public SSAPIEvent {

public:

//************************************************************************
//************************************************************************

SSAPIEventChildDeleted( ManagedObject *pManagedObject, DesignatorId parentId ) 
	:SSAPIEvent( pManagedObject, SSAPI_EVENT_CHILD_DELETED, parentId ) {}

virtual char* GetEventName() { return "CHILD_DELETED"; }

};

#endif	// __SSAPI_EVENTS_H__