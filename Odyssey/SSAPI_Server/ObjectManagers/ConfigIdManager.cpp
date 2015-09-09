//************************************************************************
// FILE:		ConfigIdManager.cpp
//
// PURPOSE:		Implements the object that will manage the system-wide
//				configuration stamp. 
//				The object will give other managers the ability to bump
//				up the config id and then broadcast it to clients. Also,
//				the object will make it possbile for other managers to 
//				check if a given request has a valid config stamp.
//************************************************************************

#include "ConfigIdManager.h"
#include "ConfigId.h"
#include "SsapiResponder.h"


ConfigIdManager* ConfigIdManager::m_pThis	= NULL;


//************************************************************************
// ConfigIdManager:
//
// PURPOSE:		Default constructor
//************************************************************************

ConfigIdManager::ConfigIdManager( ListenManager *pListenManager, DdmServices *pParent )
:ObjectManager( pListenManager, DesignatorId( RowId(), SSAPI_MANAGER_CLASS_TYPE_CONFIG_ID_MANAGER ), pParent ){

	m_pConfigId = ConfigId::Ctor( pListenManager );
	SetIsReadyToServiceRequests( true );
}


//************************************************************************
// ~ConfigIdManager():
//
// PURPOSE:		The destructor
//************************************************************************

ConfigIdManager::~ConfigIdManager(){

	delete m_pConfigId;
}


//************************************************************************
// VerifyConfigIdForRequest:
//
// PURPOSE:		Verifies config id supplied in the request. If config id
//				is not valid or no config id is supplied, the method will
//				reply with an exception.
//
// RETURN:		true:		config id is valid, may continue
//				false:		there was a problem and an exception was raised
//
// NOTE:		We assume that all SSAPI requests that have config id in 
//				them use position 0 as the place for the config id
//************************************************************************

bool 
ConfigIdManager::VerifyConfigIdForRequest( ValueSet &objectValues, SsapiResponder *pResponder ){

	U32		configId;
	bool	passedTheTest = false;

	if( objectValues.GetU32( 0, &configId ) ){
		if( *m_pConfigId == configId )
			passedTheTest = true;
	}

	if( !passedTheTest )
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_CONFIG_ID, CTS_SSAPI_CONFIG_ID_INVALID );


	return passedTheTest? true : false;
}


//************************************************************************
// BumpUpConfigId:
//
// PURPOSE:		Bumps up the config id, posts OBJECT_MODIFIED event
//************************************************************************

void 
ConfigIdManager::BumpUpConfigId(){

	m_pConfigId->BumpUp();
}


