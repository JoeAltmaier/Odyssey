//************************************************************************
// FILE:		StorageElementSsd.cpp
//
// PURPOSE:		Implements the object whose instances are used to represent
//				SSD boards as storage elements in the O2K system
//************************************************************************

#include "StorageElementSsd.h"
#include "SSDDescriptor.h"


//************************************************************************
// StorageElementSsd:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementSsd::StorageElementSsd( ListenManager *pLM, ObjectManager *pManager ):
StorageElement( pLM, SSAPI_OBJECT_CLASS_TYPE_SSD_STORAGE_ELEMENT, pManager ){
}


//************************************************************************
// ~StorageElementSsd:
//
// PURPOSE:		The destructor
//************************************************************************

StorageElementSsd::~StorageElementSsd(){
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
StorageElementSsd::BuildYourValueSet(){

	return StorageElement::BuildYourValueSet();
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
StorageElementSsd::BuildYourselfFromYourValueSet(){

	return StorageElement::BuildYourselfFromYourValueSet();
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before an object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

void 
StorageElementSsd::ComposeYourOverallState(){

	SetYourState();
}


//******************************************************************************
// ReportYourUnderlyingDevices:
//
// PURPOSE:		Gives a chance to every Storage Element to report devices that
//				make it up.
//
// FUNTIONALITY: Derived classes must populate the specified vector with IDs of
//				devices that they contain of. Memory allocated will be freed
//				by the caller, so derived classes must not deallocate anything.
//******************************************************************************

void 
StorageElementSsd::ReportYourUnderlyingDevices( Container &devices ){

	DesignatorId		id;
	ObjectManager		*pDeviceMgr = GetObjectManager(GetManager(), SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER);

	if( !pDeviceMgr->GetDesignatorIdByRowId( m_ridDescriptor, id ) ){
		ASSERT(0);
		return;
	}

	devices.Add( (CONTAINER_ELEMENT) new DesignatorId( id ) );
}


//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that sub classes set their state and state string
//******************************************************************************

void 
StorageElementSsd::SetYourState(){

	// TBDGAI
	m_state			= SSAPI_OBJECT_STATE_GOOD;
	m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members with information in the PTS row
//************************************************************************

void 
StorageElementSsd::BuildYourselfFromPtsRow( void *pRow_ ){

	SSD_Descriptor		*pRow = (SSD_Descriptor *)pRow_;

	m_ridDescriptor = pRow->ridIopStatus;
}
