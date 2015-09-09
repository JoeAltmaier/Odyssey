//************************************************************************
// FILE;		DDH.cpp
//
// PURPOSE:		Implements the object used to represent DDH devices inside
//				the O2K system
//************************************************************************


#include "DDH.h"
#include "IopStatusTable.h"
#include "HddDevice.h"


//************************************************************************
// DDH:
//
// PURPOSE:		Default constructor
//************************************************************************

DDH::DDH( ListenManager *pListenManager )
:Device( pListenManager, SSAPI_OBJECT_CLASS_TYPE_DDH_DEVICE ){

	m_manager	= DesignatorId( RowId(), SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );
	m_name		= CTS_SSAPI_DDH_DEVICE_NAME;
}


//************************************************************************
// ~DDH:
//
// PURPOSE:		The destructor
//************************************************************************

DDH::~DDH(){
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
DDH::BuildYourValueSet(){

	Device::BuildYourValueSet();

	AddString( &m_manufacturer, SSAPI_DDH_FID_MANUFACTURER );
	AddString( &m_hwPartNo, SSAPI_DDH_FID_HW_PART_NO );
	AddU32( m_hwRevision, SSAPI_DDH_FID_HW_REVISION );
	AddInt64( m_hwMfgDate, SSAPI_DDH_FID_HW_MFG_DATE );
	
	m_assetInfo.BuildYourValueSet( *this );
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
DDH::BuildYourselfFromYourValueSet(){

	Device::BuildYourselfFromYourValueSet();

	GetString( SSAPI_DDH_FID_MANUFACTURER, &m_manufacturer );
	GetString( SSAPI_DDH_FID_HW_PART_NO, &m_hwPartNo );
	GetU32( SSAPI_DDH_FID_HW_REVISION, &m_hwRevision );
	GetInt64( SSAPI_DDH_FID_HW_MFG_DATE, &m_hwMfgDate );

	m_assetInfo.BuildYourselfFromYourValueSet( *this );

	return true;
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool 
DDH::BuildYourselfFromPtsRow( void *pRow_ ){

	IOPStatusRecord		*pRow = (IOPStatusRecord *)pRow_;
	LocalizedDateTime	timestamp;

	m_manufacturer		= StringClass( (char *)&pRow->Manufacturer );
	m_hwPartNo			= StringClass( (char *)&pRow->strHwPartNo );
	m_hwRevision		= pRow->ulHwRevision;
	m_internalState		= pRow->eIOPCurrentState;
	m_id				= DesignatorId( pRow->rid, GetClassType() );
	
	m_assetInfo.ClearInfo();
	timestamp = pRow->ulHwMfgDate * 1000;
	m_assetInfo.SetProductionDate( timestamp );
	m_assetInfo.SetSerialNumber( UnicodeString( StringClass( (char *)&pRow->SerialNumber ) ) );
	m_assetInfo.SetVersionNumber( UnicodeString( StringClass( pRow->ulHwRevision ) ) );


	switch( pRow->Slot ){
		case CMB_DDH0:
			m_ddhNumber = 0;
			break;

		case CMB_DDH1:
			m_ddhNumber = 1;
			break;

		case CMB_DDH2:
			m_ddhNumber = 2;
			break;
		
		case CMB_DDH3:
			m_ddhNumber = 3;
			break;

		default:
			ASSERT(0);
			break;
	}
	m_location = m_ddhNumber;
	ComposeYourOverallState();
	return true;
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
DDH::HandleObjectAddedEvent( ValueSet *pObj, bool postEvent ){
	// nothing to do right now.
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
DDH::IsYourDevice( Device &device ){

	if( device.GetDesignatorId().GetClassId() == SSAPI_OBJECT_CLASS_TYPE_HDD_DEVICE ){
		if( ((HDDDevice &)device).GetTargetId() / 8 == m_ddhNumber )
			return true;
	}

	return false;
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

void 
DDH::ComposeYourOverallState(){

	if( StatusReporterInterface::CanChangeStateToGood() ){
		StatusReporterInterface::m_state	= SSAPI_OBJECT_STATE_GOOD;
		m_stateString						= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
	}
}