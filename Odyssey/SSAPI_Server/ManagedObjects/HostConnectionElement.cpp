//************************************************************************
// FILE:		HostConnectionElement.cpp
//
// PURPOSE:		Implements class that will be used as the base class for
//				all HostConnection objects and all HostConnectionCollection
//				objects and thus is a virtual base class for all the 
//				host connection management related stuff.
//************************************************************************

#include "HostConnectionElement.h"


//************************************************************************
// HostConnectionElement:
//
// PURPOSE:		Default constructor
//************************************************************************

HostConnectionElement::HostConnectionElement( ListenManager *pListenManager, U32 objectClassType )
					  :ManagedObject( pListenManager, objectClassType ){

	m_state			= SSAPI_OBJECT_STATE_GOOD;			// TBDGAI
	m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
}


//************************************************************************
// ~HostConnectionElement:
//
// PURPOSE:		The desructor
//************************************************************************

HostConnectionElement::~HostConnectionElement(){
}


//************************************************************************
// ApplyNecessaryStatusRollupRules:
//
// PURPOSE:		Applies applicabale rollup rules.
//				The object that implements this method is expected to call
//				the protected method with the same name and specify 
//				necessary parms.
//
// RETURN:		true:			object's state's been changed
//				false:			object's state's remained unchanged
//************************************************************************

bool 
HostConnectionElement::ApplyNecessaryStatusRollupRules(){

	return false;
}


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

bool 
HostConnectionElement::BuildYourValueSet(){

	ManagedObject::BuildYourValueSet();

	AddString( &m_description, SSAPI_HCE_FID_DESCRIPTION );
	AddString( &m_name,	SSAPI_FCE_FID_NAME );

	ComposeYourOverallState();

	return StatusReporterInterface::BuildYourValueSet( *this );
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
HostConnectionElement::BuildYourselfFromYourValueSet(){

	ManagedObject::BuildYourselfFromYourValueSet();

	GetString( SSAPI_HCE_FID_DESCRIPTION, &m_description );
	GetString( SSAPI_FCE_FID_NAME, &m_name );

	return StatusReporterInterface::BuildYourselfFromYourValueSet( *this );
}


