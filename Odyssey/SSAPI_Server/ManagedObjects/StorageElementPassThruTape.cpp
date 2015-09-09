//************************************************************************
// FILE:		StorageElementPassThruTape.h
//
// PURPOSE:		Defines the object to reprsent externally attached tape
//				devices and be able to export them.
//************************************************************************

#include "StorageElementPassThruTape.h"
#include "DeviceDescriptor.h"


//************************************************************************
// StorageElementPassThruTape:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementPassThruTape::StorageElementPassThruTape( ListenManager *pListenManager, ObjectManager *pManager )
:StorageElementPassThru( pListenManager, SSAPI_OBJECT_CLASS_TYPE_TAPE, pManager ){
}


//************************************************************************
// ~StorageElementPassThruTape:
//
// PURPOSE:		The destructor
//************************************************************************

StorageElementPassThruTape::~StorageElementPassThruTape(){
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
StorageElementPassThruTape::BuildYourValueSet(){

	StorageElementPassThru::BuildYourValueSet();

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
StorageElementPassThruTape::BuildYourselfFromYourValueSet(){

	StorageElementPassThru::BuildYourselfFromYourValueSet();

	return true;
}


//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that super classes set their state and state string
//******************************************************************************

void 
StorageElementPassThruTape::SetYourState(){

	switch( m_internalState ){

		case DriveReady:
			m_state			= SSAPI_OBJECT_STATE_GOOD;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
			break;

		case DriveInvalid:
		case DriveNotSpinning:
		case DriveSpinningUp:
			ASSERT(0);
		
		case DriveRemoved:
		case DriveNotPresent:
		case DriveHardFailure:

			m_state			= SSAPI_OBJECT_STATE_DEAD;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_DEAD;
			break;

		default:
			ASSERT(0);
			m_state			= SSAPI_OBJECT_STATE_UNKNOWN;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_UNKNOWN;
			break;
	}
}



