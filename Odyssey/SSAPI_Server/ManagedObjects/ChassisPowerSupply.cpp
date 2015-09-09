//************************************************************************
// FILE:		ChassisPowerSupply.cpp
//
// PURPOSE:		Defines class that will be used to represent chassis 
//				power supplies in the O2K
//************************************************************************


#include "ChassisPowerSupply.h"
#include "EVCStatusRecord.h"



//************************************************************************
// ChassisPowerSupply:
//
// PURPOSE:		Default constructor
//************************************************************************

ChassisPowerSupply::ChassisPowerSupply( ListenManager *pListenManager, int number, int numInEvc )
					:PowerSupply( pListenManager, SSAPI_OBJECT_CLASS_TYPE_CHASSIS_POWER_SUPPLY, number, numInEvc ){
}


//************************************************************************
// ~ChassisPowerSupply:
//
// PURPOSE:		The destructor
//************************************************************************

ChassisPowerSupply::~ChassisPowerSupply(){
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
ChassisPowerSupply::BuildYourValueSet(){

	int			temp;

	PowerSupply::BuildYourValueSet();

	temp = m_isInputOk? 1 : 0;
	AddInt( temp, SSAPI_CHASSIS_PS_FID_INPUT_OK );

	temp = m_isOutputOk? 1 : 0;
	AddInt( temp, SSAPI_CHASSIS_PS_FID_OUTPUT_OK );

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
ChassisPowerSupply::BuildYourselfFromYourValueSet(){

	int	temp;
	
	PowerSupply::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_CHASSIS_PS_FID_INPUT_OK, &temp );
	m_isInputOk = temp? true : false;
	
	GetInt( SSAPI_CHASSIS_PS_FID_OUTPUT_OK, &temp );
	m_isOutputOk = temp? true : false;

	return true;
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool 
ChassisPowerSupply::BuildYourselfFromPtsRow( void *pRow_ ){
	
	EVCStatusRecord		*pRow = (EVCStatusRecord *)pRow_;

	m_id			= DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_CHASSIS_POWER_SUPPLY, m_number);
	m_isInputOk		= pRow->fInputOK[ m_number ]? true : false;
	m_isOutputOk	= pRow->fOutputOK[ m_number ]? true : false;
	m_fanOrOverTempAlert = pRow->fFanFailOrOverTemp[ m_number ]? true : false;

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
ChassisPowerSupply::ComposeYourOverallState(){
	
	if( m_isInputOk && m_isOutputOk ){ 
		if( StatusReporterInterface::CanChangeStateToGood() ){
			m_state			= SSAPI_OBJECT_STATE_GOOD;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
		}
	}
	else if( !m_isInputOk ){
		m_state			= SSAPI_OBJECT_STATE_DEAD;
		m_stateString	= CTS_SSAPI_PS_STATE_NAME_INPUT_BAD;
	}
	else if( m_fanOrOverTempAlert ){
		m_state			= SSAPI_OBJECT_STATE_WARNING;
		m_stateString	= CTS_SSAPI_PS_STATE_NAME_OVERTEMP_ALERT;
	}
	else if( !m_isOutputOk ){
		m_state			= SSAPI_OBJECT_STATE_DEAD;
		m_stateString	= CTS_SSAPI_PS_STATE_NAME_OUTPUT_BAD;
	}
	else{
		m_state			= SSAPI_OBJECT_STATE_DEAD;
		m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_DEAD;
	}

	m_name = CTS_SSAPI_CHASSIS_PS_NAME;
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
ChassisPowerSupply::IsYourDevice( Device &device ){

	if( device.GetClassType() == SSAPI_OBJECT_CLASS_TYPE_CORD_SET ){
		switch( device.GetDesignatorId().GetUniqueId() ){
			case 1:
				return m_number == 0;
			case 2:
			case 3:
				return m_number == 1;
			case 4:
				return m_number == 2;
		}
	}

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
ChassisPowerSupply::HandleObjectAddedEvent( ValueSet *pObj, bool postEvent ){

	DesignatorId			id;
	int						classType;
	
	pObj->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
	classType = id.GetClassId();

	if( id.GetRowId() == m_id.GetRowId() )
		if( (id.GetUniqueId() & 0x00FF)  == m_numberInEvcRow )
			AddPhsDataItem( id, postEvent );
}