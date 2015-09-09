//************************************************************************
// FILE:		ConfigId.h
//
// PURPOSE:		Defines the object that will enforce the notion of 
//				system-wide configuration stamp
//
// NOTE:		This object is a Singleton and is coded to enforce this
//************************************************************************

#ifndef __SSAPI_CONFIG_ID_H__
#define __SSAPI_CONFIG_ID_H__

#include "ManagedObject.h"


#ifdef WIN32
#pragma pack(4)
#endif

class ConfigId : public ManagedObject {

	U32							m_id;
	static const ConfigId		*m_pThis;

//************************************************************************
// ConfigId:
//
// PURPOSE:		Default constructor
//************************************************************************

ConfigId( ListenManager *pListenManager );


public:


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new ConfigId( GetListenManager() ); }


//************************************************************************
// Ctor:
//
// PURPOSE:		Makes sure the object is a singleton
//************************************************************************

static ConfigId* Ctor( ListenManager *pListenManager );


//************************************************************************
// ~ConfigId:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~ConfigId();


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
// operator=:
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *(ValueSet *)this = obj; }


//************************************************************************
// operator==:
//************************************************************************

bool operator==( U32 id ) { return m_id == id; }


//************************************************************************
// BumpUp:
//
// PURPOSE:		Called when the system config stamp changes.
//				Bumps up the id and fires the OBJECT_MODIFIED event
//************************************************************************

void BumpUp();


};
#endif __SSAPI_CONFIG_ID_H__