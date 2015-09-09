//************************************************************************
// FILE:		PrfileManager.h
//
// PURPOSE:		Defines the object to be the interface to the OS' DDM
//				Profiler via SSAPI infrostructure
//************************************************************************

#ifndef __PROFILE_MANAGER_H__
#define	__PROFILE_MANAGER_H__

#include "ObjectManager.h"


#define	SSAPI_PROF_MGR_NAME			"Mr. Profiler"

class ProfileManager : public ObjectManager{

	static	ProfileManager		*m_pThis;

protected:

//************************************************************************
// ProfileManager:
//
// PURPOSE:		Default constructor
//************************************************************************

ProfileManager( ListenManager *pLM, DdmServices *pParent );


//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent * , bool  ){}


public:


//************************************************************************
// ~ProfileManager:
//
// PURPOSE:		The destructor
//************************************************************************

~ProfileManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return StringClass(SSAPI_PROF_MGR_NAME); }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new ProfileManager( pLManager, pParent );
}


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


private:


//************************************************************************
//
//************************************************************************

void ServiceStart( ValueSet *pParms, SsapiResponder *pResponder );
STATUS ServiceStartCallback( Message *pReply );

//************************************************************************
//
//************************************************************************

void ServiceStop( ValueSet *pParms, SsapiResponder *pResponder );
STATUS ServiceStopCallback( Message *pReply );

//************************************************************************
//
//************************************************************************

void ServiceClear( ValueSet *pParms, SsapiResponder *pResponder );
STATUS ServiceClearCallback( Message *pReply );

//************************************************************************
//
//************************************************************************

void ServiceDeliver( ValueSet *pParms, SsapiResponder *pResponder );
STATUS ServiceDeliverCallback( Message *pReply );


//************************************************************************
//
//************************************************************************

void ServiceHeapClear( ValueSet *pParms, SsapiResponder *pResponder );
STATUS ServiceHeapClearCallback( Message *pReply );


//************************************************************************
//
//************************************************************************

void ServiceHeapDeliver( ValueSet *pParms, SsapiResponder *pResponder );
STATUS ServiceHeapDeliverCallback( Message *pReply );


};

#endif // __PROFILE_MANAGER_H__
