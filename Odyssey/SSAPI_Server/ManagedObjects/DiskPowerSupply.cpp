//************************************************************************
// FILE:		DiskPowerSupply.cpp
//
// PURPOSE:		Implements class that will be used to represent disk 
//				power supplies in the O2K
//************************************************************************


#include "ClassTypeMap.h"
#include "DiskPowerSupply.h"
#include "EVCStatusRecord.h"


//************************************************************************
// DiskPowerSupply:
//
// PURPOSE:		Default constructor
//************************************************************************

DiskPowerSupply::DiskPowerSupply( ListenManager *pListenManager, int number, int num )
				:PowerSupply( pListenManager, SSAPI_OBJECT_CLASS_TYPE_DISK_POWER_SUPPLY, number, num ){
}


//************************************************************************
// ~DiskPowerSupply:
//
// PURPOSE:		The destructor
//************************************************************************

DiskPowerSupply::~DiskPowerSupply(){
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool 
DiskPowerSupply::BuildYourselfFromPtsRow( void *pRow_ ){

	m_id			= DesignatorId( ((EVCStatusRecord *)pRow_)->rid, SSAPI_OBJECT_CLASS_TYPE_DISK_POWER_SUPPLY, m_number);
	m_state			= ((EVCStatusRecord *)pRow_)->fDCtoDCEnable[ m_number ]? 
						(StatusReporterInterface::CanChangeStateToGood()? SSAPI_OBJECT_STATE_GOOD : m_state)
						: 
						SSAPI_OBJECT_STATE_DEAD;
	m_stateString	= (m_state == SSAPI_OBJECT_STATE_GOOD)?
						(StatusReporterInterface::CanChangeStateToGood()? CTS_SSAPI_OBJECT_STATE_NAME_GOOD : m_stateString )
						:
						CTS_SSAPI_OBJECT_STATE_NAME_DEAD;

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
DiskPowerSupply::ComposeYourOverallState(){
	
	// m_state is already populated

	m_name = CTS_SSAPI_DISK_PS_NAME;
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
DiskPowerSupply::IsYourDevice( Device &device ){

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
DiskPowerSupply::HandleObjectAddedEvent( ValueSet *pObj, bool postEvent ){

	DesignatorId			id;
	int						classType;
	ClassTypeMap			map;
	
	pObj->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
	classType = id.GetClassId();


	if( id.GetRowId() == m_id.GetRowId() )
		if( (id.GetUniqueId() & 0x00FF) == m_numberInEvcRow )
			AddPhsDataItem( id, postEvent );
}
