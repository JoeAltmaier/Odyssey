//************************************************************************
// FILE:		SsapiLocalResponder.h
//
// PURPOSE:		Declares class SsapiLocalResponder that is used by the
//				server side object managers to register for event 
//				notifications from other server side object managers
//				via the ListenManager.
//************************************************************************

#ifndef __SSAPI_LOCAL_RESPONDER__
#define	__SSAPI_LOCAL_RESPONDER__

#include "..\SsapiResponder.h"

#ifdef WIN32
#pragma pack(4)
#endif
	
#define	SSAPI_LOCAL_SESSION_ID	0

typedef void (DdmServices::*LOCAL_EVENT_CALLBACK)( ValueSet *pEvent, bool isLast, int eventObjectId );




class SsapiLocalResponder : public SsapiResponder{

	LOCAL_EVENT_CALLBACK	m_callback;
	
public:

//************************************************************************
// SsapiLocalResponder:
//
// PURPOSE:		Default constructor
//************************************************************************

SsapiLocalResponder( DdmServices *pDdm, LOCAL_EVENT_CALLBACK callback );


//************************************************************************
// ~SsapiLocalResponder:
//
// PURPOSE:		The destructor
//************************************************************************

~SsapiLocalResponder();


//************************************************************************
// GetSessionID:
//
// PURPOSE:		An accessor
//************************************************************************

virtual int GetSessionID() { return SSAPI_LOCAL_SESSION_ID; }


//************************************************************************
// Respond:
//
// PURPOSE:		Calls to the registered manager.
//************************************************************************

virtual void Respond( ValueSet* pVS, BOOL bIsLast, int eventObjectId = 0, BOOL bDelete=TRUE );

};


#endif	// __SSAPI_LOCAL_RESPONDER__