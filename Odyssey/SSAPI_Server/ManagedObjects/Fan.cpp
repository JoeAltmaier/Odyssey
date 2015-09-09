//************************************************************************
// FILE:		Fan.h
//
// PURPOSE:		Defines class Fan that is goin to be used to represent all
//				fan devices in the O2K box.
//************************************************************************


#include "Fan.h"
#include "ValueSet.h"
#include "SsapiLocalResponder.h"
#include "EVCStatusRecord.h"

//************************************************************************
// Fan:
//
// PURPOSE:		Default constructor
//************************************************************************

Fan::Fan( ListenManager *pListenManager, int fanNumber, int numInEvc )
	:Device( pListenManager, SSAPI_OBJECT_CLASS_TYPE_FAN ){

	m_fanNumber = fanNumber;
	m_numberInEvcRow = numInEvc;
	m_location	= m_fanNumber;
}


//************************************************************************
// ~Fan:
//
// PURPOSE:		The destructor
//************************************************************************

Fan::~Fan(){
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
Fan::BuildYourValueSet(){
	
	return Device::BuildYourValueSet();
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
Fan::BuildYourselfFromYourValueSet(){

	return Device::BuildYourselfFromYourValueSet();
}


//************************************************************************
// PowerOn:
//
// PURPOSE:		Declares an API method 
//************************************************************************

bool 
Fan::PowerOn( SsapiResponder *pResponder ){
	
	pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_NOT_SUPPORTED );

	return true;
}


//************************************************************************
// PowerOff:
//
// PURPOSE:		Declares an API method
//************************************************************************

bool 
Fan::PowerOff( SsapiResponder *pResponder ){

	pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_NOT_SUPPORTED );

	return true;
}


//************************************************************************
// SetSpeed:
//
// PURPOSE:		Sets new speed for the fan
//************************************************************************

bool 
Fan::SetSpeed( ValueSet *pRequestSet, SsapiResponder *pResponder ){

	return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_NOT_SUPPORTED );
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool
Fan::BuildYourselfFromPtsRow( void *pRow_ ){

	EVCStatusRecord		*pRow	= (EVCStatusRecord *)pRow_;

	m_id	= DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_FAN, m_fanNumber );
	m_speed	= pRow->FanSpeed[ m_fanNumber ];

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
Fan::ComposeYourOverallState(){
	
	if( m_speed ){
		if( StatusReporterInterface::CanChangeStateToGood() ){
			m_state			= SSAPI_OBJECT_STATE_GOOD; 
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
		}
	}
	else{
		m_state			= SSAPI_OBJECT_STATE_DEAD; 
		m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_DEAD;
	}
	m_name			= CTS_SSAPI_FAN_NAME;
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
Fan::IsYourDevice( Device &device ){

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
Fan::HandleObjectAddedEvent( ValueSet *pObj, bool postEvent ){

	DesignatorId			id;
	int						classType;
	
	pObj->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
	classType = id.GetClassId();

	if( id.GetRowId() == m_id.GetRowId() )
		if( (id.GetUniqueId() & 0x00FF)  == m_numberInEvcRow )
			AddPhsDataItem( id, postEvent );
}