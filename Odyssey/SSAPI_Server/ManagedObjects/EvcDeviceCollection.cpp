//************************************************************************
// FILE:		EvcDeviceCollection.cpp
//
// PURPOSE:		Implements class EvcDeviceCollection that will be used
//				to group EVC resource devices in the O2K product
//************************************************************************


#include "EvcDeviceCollection.h"


//************************************************************************
// EvcLogicalDeviceHierachy:
//
// PURPOSE:		Contains class type ids of the devices that this collection
//				owns.
//************************************************************************

static const U32 EvcLogicalDeviceHierachy[] = {
		SSAPI_OBJECT_CLASS_TYPE_FAN,
		SSAPI_OBJECT_CLASS_TYPE_CHASSIS_POWER_SUPPLY,
		SSAPI_OBJECT_CLASS_TYPE_DISK_POWER_SUPPLY,
		SSAPI_OBJECT_CLASS_TYPE_BATTERY,
};


//************************************************************************
// EvcDeviceCollection:
//
// PURPOSE:		Default constructor
//************************************************************************

EvcDeviceCollection::EvcDeviceCollection( ListenManager *pListenManager )
					:DeviceCollection( pListenManager, SSAPI_OBJECT_CLASS_TYPE_EVC_COLLECTION ){

	m_name	= CTS_SSAPI_EVC_DEVICE_COLLECTION_NAME;
	m_id	= DesignatorId( RowId(), SSAPI_OBJECT_CLASS_TYPE_EVC_COLLECTION, SSAPI_OBJECT_CLASS_TYPE_EVC_COLLECTION );
}


//************************************************************************
// ~EvcDeviceCollection:
//
// PURPOSE:		The destructor
//************************************************************************

EvcDeviceCollection::~EvcDeviceCollection(){
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
EvcDeviceCollection::BuildYourValueSet(){

	return DeviceCollection::BuildYourValueSet();
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
EvcDeviceCollection::BuildYourselfFromYourValueSet(){

	return DeviceCollection::BuildYourselfFromYourValueSet();
}


//************************************************************************
// HandleObjectAddedEvent:
//
// PURPOSE:		Called by the manager to inform the object that a new 
//				object has been added to the system. The object may be
//				interested to know if this new object is its child/parent
//				and update its vectors as needed.
//************************************************************************

void 
EvcDeviceCollection::HandleObjectAddedEvent( ValueSet *pObj, bool postEvent ){
}



//************************************************************************
// IsYourDevice:
//
// PURPOSE:		Determines if a gven device belongs to this device
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

bool 
EvcDeviceCollection::IsYourDevice( Device &device ){

	return IsThisClassTypeInThisArray(	device.GetClassType(), 
										EvcLogicalDeviceHierachy, 
										sizeof( EvcLogicalDeviceHierachy ) / sizeof( EvcLogicalDeviceHierachy[0] ));
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

void 
EvcDeviceCollection::ComposeYourOverallState(){
	
	DeviceCollection::ComposeYourOverallState();
}

