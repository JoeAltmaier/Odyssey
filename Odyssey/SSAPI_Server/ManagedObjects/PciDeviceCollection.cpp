//************************************************************************
// FILE:		PciDeviceCollection.cpp
//
// PURPOSE:		Implements class PciDeviceCollection that will be used
//				to group PCI devices in the O2K product
//************************************************************************


#include "PciDeviceCollection.h"

//************************************************************************
// PciLogicalDeviceHierachy:
//
// PURPOSE:		Contains class type ids of the devices that this collection
//				owns.
//************************************************************************

static const U32 PciLogicalDeviceHierachy[] = {
		SSAPI_OBJECT_CLASS_TYPE_HBC_BOARD,
		SSAPI_OBJECT_CLASS_TYPE_BUS_SEGMENT,
		SSAPI_OBJECT_CLASS_TYPE_HBC_FILLER_BOARD,
};


//************************************************************************
// PciDeviceCollection:
//
// PURPOSE:		Default constructor
//************************************************************************

PciDeviceCollection::PciDeviceCollection( ListenManager *pListenManager )
					:DeviceCollection( pListenManager, SSAPI_OBJECT_CLASS_TYPE_PCI_COLLECTION ){

	m_name	= CTS_SSAPI_PCI_DEVICE_COLLECTION_NAME;
	m_id	= DesignatorId( RowId(), SSAPI_OBJECT_CLASS_TYPE_PCI_COLLECTION, SSAPI_OBJECT_CLASS_TYPE_PCI_COLLECTION );
}


//************************************************************************
// ~PciDeviceCollection:
//
// PURPOSE:		The destructor
//************************************************************************

PciDeviceCollection::~PciDeviceCollection(){
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
PciDeviceCollection::BuildYourValueSet(){

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
PciDeviceCollection::BuildYourselfFromYourValueSet(){

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
PciDeviceCollection::HandleObjectAddedEvent( ValueSet *pObj, bool postEvent ){
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
PciDeviceCollection::IsYourDevice( Device &device ){

	return IsThisClassTypeInThisArray(	device.GetClassType(), 
										PciLogicalDeviceHierachy, 
										sizeof( PciLogicalDeviceHierachy )/sizeof( PciLogicalDeviceHierachy[0] ) );
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

void 
PciDeviceCollection::ComposeYourOverallState(){
	
	DeviceCollection::ComposeYourOverallState();
}

