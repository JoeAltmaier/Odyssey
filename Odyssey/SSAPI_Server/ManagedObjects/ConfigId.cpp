//************************************************************************
// FILE:		ConfigId.cpp
//
// PURPOSE:		Implements the object that will enforce the notion of 
//				system-wide configuration stamp
//
// NOTE:		This object is a Singleton and is coded to enforce this
//************************************************************************


#include "ConfigId.h"

const ConfigId* ConfigId::m_pThis = NULL;

//************************************************************************
// ConfigId:
//
// PURPOSE:		Default constructor
//************************************************************************

ConfigId::ConfigId( ListenManager *pListenManager )
:ManagedObject( pListenManager, SSAPI_OBJECT_CLASS_TYPE_CONFIG_ID ){

	m_id		= 0;
}


//************************************************************************
// Ctor:
//
// PURPOSE:		Makes sure the object is a singleton
//************************************************************************

ConfigId* 
ConfigId::Ctor( ListenManager *pListenManager ){

	if( m_pThis ){
		ASSERT(0);
		return (ConfigId*)m_pThis;
	}
	 
	return (ConfigId*)(m_pThis = new ConfigId( pListenManager ));
}



//************************************************************************
// ~ConfigId:
//
// PURPOSE:		The destructor
//************************************************************************

ConfigId::~ConfigId(){

	m_pThis = NULL;
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
ConfigId::BuildYourValueSet(){

	ManagedObject::BuildYourValueSet();

	AddU32( m_id, SSAPI_CONFIG_ID_FID_CONFIG_STAMP );

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
ConfigId::BuildYourselfFromYourValueSet(){

	ManagedObject::BuildYourselfFromYourValueSet();

	GetU32( SSAPI_CONFIG_ID_FID_CONFIG_STAMP, &m_id );

	return true;
}

//************************************************************************
// BumpUp:
//
// PURPOSE:		Called when the system config stamp changes.
//				Bumps up the id and fires the OBJECT_MODIFIED event
//************************************************************************

void 
ConfigId::BumpUp(){

	m_id++;
	FireEventObjectModifed();
}
