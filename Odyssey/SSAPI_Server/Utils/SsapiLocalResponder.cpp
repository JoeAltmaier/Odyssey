//************************************************************************
// FILE:		SsapiLocalResponder.cpp
//
// PURPOSE:		Implements SsapiLocalResponder that is used by the
//				server side object managers to register for event 
//				notifications from other server side object managers
//				via the ListenManager.
//************************************************************************


#include "SsapiLocalResponder.h"


//************************************************************************
// SsapiLocalResponder:
//
// PURPOSE:		Default constructor
//************************************************************************

SsapiLocalResponder::SsapiLocalResponder( DdmServices *pDdm, LOCAL_EVENT_CALLBACK callback )
	:SsapiResponder( pDdm, NULL ){

	m_callback	= callback;
}


//************************************************************************
// ~SsapiLocalResponder:
//
// PURPOSE:		The destructor
//************************************************************************

SsapiLocalResponder::~SsapiLocalResponder(){
}


//************************************************************************
// Respond:
//
// PURPOSE:		Calls to the registered manager.
//************************************************************************

void 
SsapiLocalResponder::Respond( ValueSet* pVS, BOOL bIsLast,  int eventObjectId , BOOL bDelete){

	if( pParentDdmSvs ){
		(pParentDdmSvs->*m_callback)( pVS, bIsLast? true : false, eventObjectId );
	}
	if(bIsLast)
		delete this;
}

