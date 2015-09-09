//************************************************************************
// FILE:		BusSegment.cpp
//
// PURPOSE:		Implements class BusSegment that will be used to represent
//				bus segments in the O2K system
//************************************************************************


#include "BusSegment.h"
#include "EVCStatusRecord.h"
#include "SlotMap.h"
#include "Board.h"


//************************************************************************
// BusSegmentLogicalDeviceHierachy:
//
// PURPOSE:		Contains class type ids of the devices that this device
//				owns.
//************************************************************************

static const U32 BusSegmentLogicalDeviceHierachy[] = {
		SSAPI_OBJECT_CLASS_TYPE_SSD_BOARD,
		SSAPI_OBJECT_CLASS_TYPE_NAC_BOARD,
		SSAPI_OBJECT_CLASS_TYPE_FILLER_BOARD,
		SSAPI_OBJECT_CLASS_TYPE_SNAC_BOARD
};


//************************************************************************
// BusSegment:
//
// PURPOSE:		Default constructor
//************************************************************************

BusSegment::BusSegment( ListenManager *pListenManager, int number )
		:Device( pListenManager, SSAPI_OBJECT_CLASS_TYPE_BUS_SEGMENT ){
	
	m_number	= number;
	m_location	= m_number;
}


//************************************************************************
// ~BusSegment
//
// PURPOSE:		The destructor
//************************************************************************

BusSegment::~BusSegment(){
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
BusSegment::BuildYourValueSet(){
	
	Device::BuildYourValueSet();

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
BusSegment::BuildYourselfFromYourValueSet(){
	
	Device::BuildYourselfFromYourValueSet();

	return true;
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

void 
BusSegment::ComposeYourOverallState(){

	if( StatusReporterInterface::CanChangeStateToGood() ){
		m_state			= SSAPI_OBJECT_STATE_GOOD; // TBDGAI ->> how to determine the state?
		m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
	}
	
	// here comes a hack...well...sort of...'cause this is self-contained withing this object...so it's more or less ok
	switch( m_number ){

		case 0:
			m_name = CTS_SSAPI_BUS_SEGMENT_0_NAME;
			break;

		case 1:
			m_name = CTS_SSAPI_BUS_SEGMENT_1_NAME;
			break;

		case 2:
			m_name = CTS_SSAPI_BUS_SEGMENT_2_NAME;
			break;

		case 3:
			m_name = CTS_SSAPI_BUS_SEGMENT_3_NAME;
			break;
	}
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool 
BusSegment::BuildYourselfFromPtsRow( void *pRow_ ){

	EVCStatusRecord		*pRow	= (EVCStatusRecord *)pRow_;

	m_id		= DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_BUS_SEGMENT, m_number );

	return true;
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
BusSegment::IsYourDevice( Device &device ){

	bool				rc;
	SlotMap				map;

	rc = IsThisClassTypeInThisArray(	device.GetClassType(), 
										BusSegmentLogicalDeviceHierachy,  
										sizeof( BusSegmentLogicalDeviceHierachy ) / sizeof( BusSegmentLogicalDeviceHierachy[0] ));

	if( !rc )
		return false;

	if( map.GetSegmentNumberBySlotNumber( ((Board &)device).GetSlotNumber() ) == (U32)m_number )
		return true;
	
	return false;
}

