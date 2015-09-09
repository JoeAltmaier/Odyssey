//************************************************************************
// FILE:		Iop.cpp
//
// PURPOSE:		Implements class IOP that serves as an abstract base for
//				all IOP-type devices in the O2K.
//************************************************************************

#include "Iop.h"
#include "IOPStatusTable.h"
#include "SsapiResponder.h"
#include "MicroController.h"
#include "DeviceManager.h"


//************************************************************************
// Iop
//
// PURPOSE:		Default constructor
//************************************************************************

Iop::Iop( ListenManager *pListenManager, U32 objectClassType )
	:Board( pListenManager, objectClassType ){
}


//************************************************************************
// ~Iop:
//
// PURPOSE:		The destructor
//************************************************************************

Iop::~Iop(){
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
Iop::BuildYourValueSet(){
	
	int		temp;

	Board::BuildYourValueSet();

	AddInt(m_redundantSlotNumber, SSAPI_IOP_FID_REDUNDANT_SLOT );
	AddString( &m_manufacturer, SSAPI_IOP_FID_MANUFACTURER );
	AddString( &m_hardwareVersion, SSAPI_IOP_FID_HW_VERSION );
	AddString( &m_serialNumber, SSAPI_IOP_FID_SERIAL_NUMBER );

	temp = m_isPowered? 1 : 0;
	AddInt(temp, SSAPI_OBJECT_FID_IS_POWERED );

	AddInt(m_serviceState, SSAPI_OBJECT_FID_SERVICE_STATE );

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
Iop::BuildYourselfFromYourValueSet(){

	int		temp;

	GetInt( SSAPI_IOP_FID_REDUNDANT_SLOT, &m_redundantSlotNumber );
	GetString( SSAPI_IOP_FID_MANUFACTURER, &m_manufacturer );
	GetString( SSAPI_IOP_FID_HW_VERSION, &m_hardwareVersion );
	GetString( SSAPI_IOP_FID_SERIAL_NUMBER, &m_serialNumber );
	GetInt( SSAPI_OBJECT_FID_IS_POWERED, &temp );
	m_isPowered = temp? true : false;
	GetInt( SSAPI_OBJECT_FID_SERVICE_STATE, &m_serviceState );

	m_assetInfo.BuildYourselfFromYourValueSet( *this );

	return true;
}


//************************************************************************
// PowerOn:
//
// PURPOSE:		Declares an API method 
//************************************************************************

bool 
Iop::PowerOn( SsapiResponder *pResponder ){

	if( !m_isLocked ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_STATE, CTS_SSAPI_EXCEPTION_DEVICE_NOT_LOCKED );
		return true;
	}

	((DeviceManager *)GetManager())->ChangeIopPowerState(	this,
															false,
															pResponder );
	return true;
}


//************************************************************************
// PowerOff:
//
// PURPOSE:		Declares an API method
//************************************************************************

bool 
Iop::PowerOff( SsapiResponder *pResponder ){

	if( m_serviceState ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_STATE, CTS_SSAPI_EXCEPTION_DEVICE_IN_SERVICE );
		return true;
	}

	((DeviceManager *)GetManager())->ChangeIopPowerState(	this,
															true,
															pResponder );
	return true;
}


//************************************************************************
// SetServiceState:
//
// PURPOSE:		Declares an API method 
//************************************************************************

bool 
Iop::SetServiceState( SsapiResponder *pResponder, U32 newState ){

	if( newState && !m_isPowered ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_STATE, CTS_SSAPI_EXCEPTION_DEVICE_POWERED_DOWN );
		return true;
	}
	else if( newState && !m_isLocked ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_STATE, CTS_SSAPI_EXCEPTION_DEVICE_NOT_LOCKED );
		return true;
	}

	((DeviceManager *)GetManager())->ChangeIopServiceState(	this,
															newState? true : false,
															pResponder );
	return true;
}


//************************************************************************
// Lock:
//
// PURPOSE:		Declares an API method 
//************************************************************************

bool 
Iop::Lock( SsapiResponder *pResponder ){

	if( m_isPowered || m_serviceState ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		return true;
	}

	return Board::Lock( pResponder );
}


//************************************************************************
// UnLock:
//
// PURPOSE:		Declares an API method
//************************************************************************

bool 
Iop::UnLock( SsapiResponder *pResponder ){

	if( m_serviceState ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_STATE, CTS_SSAPI_EXCEPTION_DEVICE_IN_SERVICE );
		return true;
	}
	else if( m_isPowered ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_STATE, CTS_SSAPI_EXCEPTION_DEVICE_POWERED_UP );
		return true;
	}


	return Board::UnLock( pResponder );
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool 
Iop::BuildYourselfFromPtsRow( void *pRow_ ){
	
	IOPStatusRecord		*pRow	= (IOPStatusRecord *)pRow_;
	UnicodeString		s;	
	MicroController		*pMicroController;
	LocalizedDateTime	timestamp;

	m_id					= DesignatorId( pRow->rid, (U16)GetClassType(), 0 );
	m_slotNumber			= pRow->Slot;
	SetYourSlotName();
	m_redundantSlotNumber	= pRow->RedundantSlot;
	m_internalState			= pRow->eIOPCurrentState;
	m_manufacturer			= UnicodeString( (StringClass)pRow->Manufacturer );
	m_hardwareVersion		= UnicodeString( (StringClass)pRow->ulHwRevision );
	m_serialNumber			= UnicodeString( (StringClass)pRow->SerialNumber );

	// populate asset info
	m_assetInfo.ClearInfo();
	timestamp = pRow->ulHwMfgDate * 1000;
	m_assetInfo.SetProductionDate( timestamp );
	m_assetInfo.SetSerialNumber( m_serialNumber );
	m_assetInfo.SetVersionNumber( m_hardwareVersion );


	RemoveMicroControllers();
	pMicroController	= new MicroController(	CTS_SSAPI_AVR_NAME,
												StringClass(pRow->ulAvrSwVersion),
												StringClass(pRow->ulAvrSwRevision) );  

	m_microControllerVector.Add( (CONTAINER_ELEMENT)pMicroController );

	return OK;
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
Iop::HandleObjectAddedEvent( ValueSet *pObj, bool postEvent ){
	
	DesignatorId				id;
	
	pObj->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );

	// we only need PHSDataTemperature
	if( id.GetClassId() == SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE ){
		if( id.GetRowId() == m_id.GetRowId() )
			AddPhsDataItem( id, postEvent );
	}
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

void 
Iop::ComposeYourOverallState(){

	m_isLocked = true;

	switch( m_internalState){

		case IOPS_POWERED_ON:
			m_state			= SSAPI_OBJECT_STATE_UNKNOWN;
			m_stateString	= CTS_SSAPI_IOP_STATE_NAME_POWERED_ON;
			m_isPowered		= true;
			m_serviceState	= 0;
			break;

		case IOPS_AWAITING_BOOT:
			m_state			= SSAPI_OBJECT_STATE_UNKNOWN;
			m_stateString	= CTS_SSAPI_IOP_STATE_NAME_AWAITING_BOOT;
			m_isPowered		= true;
			m_serviceState	= 0;
			break;

		case IOPS_LOADING:
			m_state			= SSAPI_OBJECT_STATE_UNKNOWN;
			m_stateString	= CTS_SSAPI_IOP_STATE_NAME_LOADING;
			m_isPowered		= true;
			m_serviceState	= 0;
			break;

		case IOPS_DIAG_MODE:
			m_state			= SSAPI_OBJECT_STATE_DIAG;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_BEING_DIAGNOSTED;
			m_isPowered		= true;
			m_serviceState	= 1;
			break;

		case IOPS_OPERATING:
			if( StatusReporterInterface::CanChangeStateToGood() ){
				m_state			= SSAPI_OBJECT_STATE_GOOD;
				m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
			}
			m_isPowered		= true;
			m_serviceState	= 1;
			break;

		case IOPS_BOOTING:
			m_state			= SSAPI_OBJECT_STATE_UNKNOWN;
			m_stateString	= CTS_SSAPI_IOP_STATE_NAME_BOOTING;
			m_isPowered		= true;
			m_serviceState	= 0;
			break;

		case IOPS_FAILING:
			m_state			= SSAPI_OBJECT_STATE_WARNING;
			m_stateString	= CTS_SSAPI_IOP_STATE_NAME_FAILING;
			m_serviceState	= 0;
			break;
		
		case IOPS_QUIESCENT:
			m_state			= SSAPI_OBJECT_STATE_DEAD;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_QUIESCED;
			m_isPowered		= true;
			m_serviceState	= 0;
			break;

		case IOPS_FAILED:
			m_state			= SSAPI_OBJECT_STATE_DEAD;
			m_stateString	= CTS_SSAPI_IOP_STATE_NAME_FAILED;
			m_isPowered		= false;
			m_serviceState	= 0;
			break;

		case IOPS_POWERED_DOWN:
			m_state			= SSAPI_OBJECT_STATE_DEAD;
			m_stateString	= CTS_SSAPI_IOP_STATE_NAME_POWERED_DOWN;
			m_isPowered		= false;
			m_serviceState	= 0;
			break;

		case IOPS_UNLOCKED:
			m_isLocked		= false;
			m_isPowered		= false;
			m_state			= SSAPI_OBJECT_STATE_DEAD;
			m_stateString	= CTS_SSAPI_IOP_STATE_NAME_UNLOCKED;
			m_serviceState	= 0;
			break;

		default:
			ASSERT(0);
			break;
	}
	// SSAPI_OBJECT_STATE_DEGRADED TBDGAI -> need to get the counterpart of this iop
	// thru the device manager (RedundatSlot) and see if if its' quiesced, then 
	// this iop is degraded!!!!!
}