//************************************************************************
// FILE:		SSAPIEvents.cpp
//
// PURPOSE:		Contains class definitions for all possible events 
//				generated in the SSAPI
//************************************************************************

#include "SSAPIEvents.h"
#include "ManagedObject.h"

#ifndef WIN32
#include "time.h"
#endif

//************************************************************************
// SSAPIEvent:
//
// PURPOSE:		Default constructor
//
// RECEIVE:		pManagedObject:		ptr to the object to be delivered
//				eventType:			type of the event
//************************************************************************

SSAPIEvent::SSAPIEvent( ManagedObject *pManagedObject, U8 eventType, DesignatorId parentId ){

	time_t				timeTemp;

	m_eventType			= eventType;
	m_pManagedObject	= pManagedObject;
	m_parentId			= parentId;

	time( &timeTemp );
	m_timeStamp = timeTemp;
	m_timeStamp *= 1000;		

	BuildYourValueSet();
}	


//************************************************************************
// ~SSAPIEvent:
//
// PURPOSE:		The destructor
//************************************************************************

SSAPIEvent::~SSAPIEvent(){
}


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Builds up a value set of its data members
//************************************************************************

void 
SSAPIEvent::BuildYourValueSet(){

	AddInt(m_eventType, SSAPI_EVENT_FID_EVENT_TYPE );
	AddInt64(m_timeStamp, SSAPI_EVENT_FID_TIMESTAMP );	
	AddGenericValue( (char *)&m_parentId, sizeof(m_parentId), SSAPI_EVENT_FID_PARENT_ID );
	
	if( m_pManagedObject ){
		m_pManagedObject->BuildYourValueSet();
		AddValue( m_pManagedObject, SSAPI_EVENT_FID_MANAGED_OBJECT );
		m_pManagedObject->Clear();
	}
}



