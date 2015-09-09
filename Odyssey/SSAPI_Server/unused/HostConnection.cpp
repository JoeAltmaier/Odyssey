//************************************************************************
// FILE:		HostConnection.cpp
//
// PURPOSE:		Implements class used to represent connections between
//				a host's HBA and NAC's port.
//************************************************************************


#include "DdmSSAPI.h"
#include "HostManager.h"
#include "HostConnection.h"
#include "ExternalInitiatorPort.h"
		
//************************************************************************
// HostConnection:
//
// PURPOSE:		The default constructor
//************************************************************************

HostConnection::HostConnection( ListenManager *pListenManager )
:HostConnectionElement( pListenManager, SSAPI_OBJECT_CLASS_TYPE_HOST_CONNECTION ){
}


//************************************************************************
// ~HostConnection:
//
// PURPOSE:		The destructor
//************************************************************************

HostConnection::~HostConnection(){
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
HostConnection::BuildYourValueSet(){

	HostConnectionElement::BuildYourValueSet();

	AddU32( m_mode, SSAPI_HC_FID_MODE );
	AddGenericValue( (char *)&m_hostId, sizeof(m_hostId), SSAPI_HC_FID_HOST_ID );

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
HostConnection::BuildYourselfFromYourValueSet(){
	
	ValueSet	*pChildren;
	U32			eipCount;
	
	HostConnectionElement::BuildYourselfFromYourValueSet();

	GetU32( SSAPI_HC_FID_MODE, &m_mode );
	GetGenericValue( (char *)&m_hostId, sizeof(m_hostId), SSAPI_HC_FID_HOST_ID );

	pChildren = (ValueSet *)GetValue(SSAPI_OBJECT_FID_CHILDREN_ID_VECTOR);
	eipCount = pChildren? pChildren->GetCount() : 0;

	return true;
}


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

bool 
HostConnection::ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	
	return ((HostManager *)GetManager())->ModifyHostConnection( objectValues, pResponder );
}


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

bool 
HostConnection::DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	DesignatorId			id;

	objectValues.GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
	
	return ((HostManager *)GetManager())->DeleteHostConnection( id, pResponder );
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates its data members based on the data in the PTS row
//************************************************************************

bool 
HostConnection::BuildYourselfFromPtsRow( HostConnectionDescriptorRecord *pRow ){
	
	m_id			= DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_HOST_CONNECTION );
	m_ridName		= RowId( pRow->ridName );
	m_ridDescription= RowId( pRow->ridDescription );
	m_correctEipCount = pRow->ridEIPCount;
	m_hostId		= DesignatorId( pRow->ridHost, SSAPI_OBJECT_CLASS_TYPE_HOST );

	return true;
}


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates the PTS row based on its data members
//************************************************************************

bool 
HostConnection::WriteYourselfIntoPtsRow( HostConnectionDescriptorRecord *pRow ){

	DesignatorId			id;

	pRow->rid					= m_id.GetRowId().GetRowID();
	pRow->version				= HOST_CONNECTION_DESCRIPTOR_TABLE_VERSION;
	pRow->size					= sizeof( HostConnectionDescriptorRecord );
	pRow->ridEIPCount			= GetChildCount();
	pRow->ridDescription		= m_ridDescription;	
	pRow->ridName				= m_ridName;
	pRow->ridHost				= m_hostId.GetRowId().GetRowID();
	pRow->eHostConnectionMode	= (HostConnectionModeEnum)m_mode;

	for( U32 i = 0; i < pRow->ridEIPCount; i++ ){
		id = GetChildIdAt( i );
		pRow->ridEIPs[i]	= id.GetRowId().GetRowID();
	}

	return true;
}


//************************************************************************
// SetPortState:
//
// PURPOSE:		Informs the object of the state of the port it's connected
//				to. Currently, the state of the port is the state of the
//				connection.
//************************************************************************

void 
HostConnection::SetPortState( int portState, U32 portStateString ){
	
	if( StatusReporterInterface::CanChangeStateToGood() ){
		if( ( portState != m_state ) || ( m_stateString != portStateString ) ){
			m_state			= portState;
			m_stateString	= portStateString;
			FireEventObjectModifed();
		}
	}
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Responsible for state reconcilation
//
// NOTES:		The state is determined with regards to the states of the 
//				EIP objects stored as children.
//************************************************************************


//
// TBDGAI: the correct eip count must be persistant, or we'll not see
//			a missing EIP. Do it LATER! 
//
void 
HostConnection::ComposeYourOverallState(){
	
	ExternalPort			*pEPort;
	U32						i, stateString = CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
	int						state = SSAPI_OBJECT_STATE_GOOD;
	DesignatorId			id;

	// first of all, check of all EIPs are present
#if 0 // TBDGAI
	if( m_correctEipCount != m_children.Count() ){
		m_state = SSAPI_OBJECT_STATE_WARNING;
		m_stateString = CTS_SSAPI_HOST_STATE_EIPS_MESSING;
		FireEventObjectModifed();
		return;
	}
#endif

	for( i = 0; i < GetChildCount(); i++ ){
		id = GetChildIdAt( i );
		pEPort = (ExternalPort *)((HostManager *)GetManager())->GetManagedObject( &id );
		if( pEPort->GetState() > state ){
			state = SSAPI_OBJECT_STATE_WARNING;
			stateString = CTS_SSAPI_OBJECT_STATE_NAME_WARNING;
			break;
		}	
	}

	if( m_state!= state ){
		m_state = state;
		m_stateString = stateString;
		FireEventObjectModifed();
	}

}

//************************************************************************
// Set() Methods:
//
// PURPOSE:		Generate events
//************************************************************************

void 
HostConnection::SetName( UnicodeString name, bool shouldPostEvent ){

	if( m_name != name ){
		m_name	= name;
		if( shouldPostEvent )
			FireEventObjectModifed();
	}
}


void 
HostConnection::SetDescription( UnicodeString description, bool shouldPostEvent ){

	if( m_description != description ){
		m_description = description;
		if( shouldPostEvent )
			FireEventObjectModifed();
	}
}


//************************************************************************
// IsYourExternalPort:
//
// PURPOSE:		Checks if a given external port object is hoocked to the host
//************************************************************************

bool 
HostConnection::IsYourExternalPort( ExternalPort &port ){

	U32					i;

	for( i = 0; i < GetChildCount(); i++ ){
		if( GetChildIdAt( i ) == port.GetDesignatorId() )
			return true;
	}
	return false;
}