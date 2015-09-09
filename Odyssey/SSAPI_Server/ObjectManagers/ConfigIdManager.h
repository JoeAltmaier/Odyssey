//************************************************************************
// FILE:		ConfigIdManager.h
//
// PURPOSE:		Defines the object that will manage the system-wide
//				configuration stamp. 
//				The object will give other managers the ability to bump
//				up the config id and then broadcast it to clients. Also,
//				the object will make it possbile for other managers to 
//				check if a given request has a valid config stamp.
//************************************************************************

#ifndef __CONFIG_ID_MANAGER_H__
#define __CONFIG_ID_MANAGER_H__

#include "ObjectManager.h"
class ConfigId;

#ifdef WIN32
#pragma pack(4)
#endif

#define	CONFIG_ID_MANAGER_NAME			"ConfigIdManager"

class ConfigIdManager : public ObjectManager{
	
	ConfigId					*m_pConfigId;
	static ConfigIdManager		*m_pThis;


//************************************************************************
// ConfigIdManager:
//
// PURPOSE:		Default constructor
//************************************************************************

ConfigIdManager( ListenManager *pListenManager,  DdmServices *pParent );


public:


//************************************************************************
// ~ConfigIdManager():
//
// PURPOSE:		The destructor
//************************************************************************

~ConfigIdManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return StringClass(CONFIG_ID_MANAGER_NAME); }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new ConfigIdManager( pLManager, pParent );
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
//************************************************************************

bool VerifyConfigIdForRequest( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// BumpUpConfigId:
//
// PURPOSE:		Bumps up the config id, posts OBJECT_MODIFIED event
//************************************************************************

void BumpUpConfigId();


//************************************************************************
//************************************************************************

protected:

//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent*, bool ){}

};

#endif	// __CONFIG_ID_MANAGER_H__