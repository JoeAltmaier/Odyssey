//************************************************************************
// FILE:		StorageElementPassThruSes.h
//
// PURPOSE:		Defines the object to reprsent externally attached SES
//				devices and be able to export them.
//************************************************************************

#ifndef __STORAGE_ELEMENT_PASS_THRU_SES_H__
#define	__STORAGE_ELEMENT_PASS_THRU_SES_H__

#include "StorageElementPassThru.h"
struct DeviceDescriptor;


class StorageElementPassThruSes : public StorageElementPassThru {

	

public:

//************************************************************************
// StorageElementPassThruSes:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementPassThruSes( ListenManager *pListenManager, ObjectManager *pManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new StorageElementPassThruSes( GetListenManager(), GetManager() ); }


//************************************************************************
// ~StorageElementPassThruSes:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~StorageElementPassThruSes();


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


protected:

//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that super classes set their state and state string
//******************************************************************************

virtual void SetYourState();


};

#endif	// __STORAGE_ELEMENT_PASS_THRU_SES_H__