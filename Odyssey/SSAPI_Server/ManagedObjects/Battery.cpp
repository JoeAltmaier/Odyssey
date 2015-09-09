//************************************************************************
// FILE:		Battery.cpp
//
// PURPOSE:		Implements class Battery that will be used to repsent 
//				battery-type devices in the O2K
//************************************************************************

#include "Battery.h"
#include "EVCStatusRecord.h"


//************************************************************************
// Battery:
//
// PURPOSE:		Default constructor
//************************************************************************

Battery::Battery( ListenManager *pListenManager, int number, int i )
		:Device( pListenManager, SSAPI_OBJECT_CLASS_TYPE_BATTERY ){

	m_number = number;
	m_numberInEvcRow = i;
	m_location = number;
}


//************************************************************************
// ~Battery:
//
// PURPOSE:		The destructor
//************************************************************************

Battery::~Battery(){
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
Battery::BuildYourValueSet(){

	Device::BuildYourValueSet();

	AddInt( m_isPresent, SSAPI_BATTERY_FID_IS_PRESENT );


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
Battery::BuildYourselfFromYourValueSet(){

	Device::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_BATTERY_FID_IS_PRESENT, &m_isPresent );

	return true;
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************** **********************************************************

void 
Battery::ComposeYourOverallState(){

	 m_name			= CTS_SSAPI_BATTERY_NAME;	 

	 if( !m_isPresent || !m_current ){
		m_state			= SSAPI_OBJECT_STATE_DEAD;
		m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_DEAD;
	 }
	 else if( StatusReporterInterface::CanChangeStateToGood() ){
		m_state			= SSAPI_OBJECT_STATE_GOOD;
		m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
	 }
	
}

//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool 
Battery::BuildYourselfFromPtsRow( void *pRow_ ){

	EVCStatusRecord	*pRow	= (EVCStatusRecord *)pRow_;

	m_id			= DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_BATTERY, m_number );
	m_isPresent		= pRow->fBatteryInstalled[m_number];
	m_current		= pRow->BatteryCurrent[m_number];

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
Battery::IsYourDevice( Device &device ){

	return false;
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
Battery::HandleObjectAddedEvent( ValueSet *pObj, bool postEvent ){
	DesignatorId			id;
	
	pObj->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );

	// we only need PHSDataVoltage
	if( (id.GetClassId() == SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE) ||
		(id.GetClassId() == SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE) )
		if( id.GetRowId() == m_id.GetRowId() )
			if( (id.GetUniqueId() & 0x00FF) == m_numberInEvcRow )
				AddPhsDataItem( id, postEvent );
}