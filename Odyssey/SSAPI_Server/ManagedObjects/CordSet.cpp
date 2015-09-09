//************************************************************************
// FILE:		CordSet.cpp
//
// PURPOSE:		Implements the object used to represent battery cord sets 
//				inside the Gemini
//************************************************************************


#include "CordSet.h"
#include "SsapiAssert.h"
#include "EvcStatusRecord.h"

//************************************************************************
// CordSet:
//
// PURPOSE:		Default constructor
//************************************************************************

CordSet::CordSet( ListenManager *pListenManager, U32 cordSetNumber )
:Device( pListenManager, SSAPI_OBJECT_CLASS_TYPE_CORD_SET ) {

	m_cordSetNumber = cordSetNumber;
	m_name			= CTS_SSAPI_DEVICE_NAME_CORD_SET;
	m_location		= m_cordSetNumber;
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool 
CordSet::BuildYourselfFromPtsRow( void *pRow_ ){

	EVCStatusRecord		*pRow = (EVCStatusRecord *)pRow_;

	m_id	= DesignatorId( pRow->rid, GetClassType(), m_cordSetNumber );

	switch( m_cordSetNumber ){
		case 1:
			m_isStatusGood = pRow->fInputOK[0]? true : false;
			break;
		case 2:
		case 3:
			m_isStatusGood = pRow->fInputOK[1]? true : false;
			break;
		case 4:
			m_isStatusGood = pRow->fInputOK[2]? true : false;
			break;

		default:
			ASSERT(0);
			break;
	}
	return true;
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before an object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

void 
CordSet::ComposeYourOverallState(){

	if( m_isStatusGood && StatusReporterInterface::CanChangeStateToGood() ){
		m_state			= SSAPI_OBJECT_STATE_GOOD;
		m_stateString	= CTS_SSAPI_DEVICE_CORD_SET_STATE_POWER_ON;
	}
	else if( !m_isStatusGood ){
		m_state			= SSAPI_OBJECT_STATE_DEAD;
		m_stateString	= CTS_SSAPI_DEVICE_CORD_SET_STATE_POWER_OFF;
	}
}


