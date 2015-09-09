//************************************************************************
// FILE:		FcPort.cpp
//
// PURPOSE:		Implements class GcPort to be used to represent all FC
//				ports in the O2K system
//
//************************************************************************


#include "FcPort.h"
#include "SsapiResponder.h"


//************************************************************************
// FcPort:
//
// PURPOSE:		Default constructor
//************************************************************************

FcPort::FcPort( ListenManager *pListenManager, U32 classType ) 
		:Device( pListenManager, classType ){

	m_name	= CTS_SSAPI_FC_PORT_DEVICE_NAME;	
}


//************************************************************************
// ~FcPort
//
// PURPOSE:		The destructor
//************************************************************************

FcPort::~FcPort(){
}



//************************************************************************
// SetServiceState:
//
// PURPOSE:		Declares an API method 
//************************************************************************

bool 
FcPort::SetServiceState( SsapiResponder *pResponder, U32 newState ){

	m_serviceState = m_serviceState? 0 : 1;
	pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	FireEventObjectModifed();
	return true;
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
FcPort::BuildYourValueSet(){

	int	i;
	
	Device::BuildYourValueSet();

	i = m_portNumber;
	AddInt( i, SSAPI_FC_PORT_FID_PORT_NUMBER );

	i = m_chipNumber;
	AddInt( i, SSAPI_FC_PORT_FID_CHIP_NUMBER );

	AddInt(m_serviceState, SSAPI_OBJECT_FID_SERVICE_STATE );
	
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
FcPort::BuildYourselfFromYourValueSet(){
	
	int i;

	Device::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_FC_PORT_FID_PORT_NUMBER, &i );
	m_portNumber = i;

	GetInt( SSAPI_FC_PORT_FID_CHIP_NUMBER, &i );
	m_chipNumber = i;

	GetInt( SSAPI_OBJECT_FID_SERVICE_STATE, &m_serviceState );

	return true;
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool
FcPort::BuildYourselfFromPtsRow( void *pRow_ ){

	LoopDescriptorEntry	*pRow	= (LoopDescriptorEntry *)pRow_;
	
	m_id			= DesignatorId( pRow->rid, (U16)GetClassType() );
	m_portNumber	= pRow->LoopNumber;
	m_slotNumber	= pRow->slot;
	m_chipNumber	= pRow->ChipNumber;
	m_flags			= pRow->flags;
	m_portState		= pRow->ActualLoopState;
	m_location		= pRow->LoopNumber;

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
FcPort::HandleObjectAddedEvent( ValueSet *pObj, bool postEvent ){

	// currently, we aren't interested in any events of the type

#if 0
	DesignatorId				id;
	
	pObj->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );

	if( id.GetClassId() == SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE ){
		if( id.GetRowId() == m_id.GetRowId() )
			AddPhsDataItem( id, postEvent );
	}
#endif
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

void 
FcPort::ComposeYourOverallState(){

	

	switch( m_portState ){
		case LoopUp:
			if( StatusReporterInterface::CanChangeStateToGood() ){
				m_state			= SSAPI_OBJECT_STATE_GOOD;
				m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
			}
			break;

		case LoopDown:
			m_state			= SSAPI_OBJECT_STATE_DEAD;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_DEAD;
			break;

		case LoopQuiesce:
			m_state			= SSAPI_OBJECT_STATE_DEAD;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_QUIESCED;
			break;

		default:
			ASSERT(0);
			m_state			= SSAPI_OBJECT_STATE_UNKNOWN;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_UNKNOWN;
	}
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
FcPort::IsYourDevice( Device &device ){

	return false;
}