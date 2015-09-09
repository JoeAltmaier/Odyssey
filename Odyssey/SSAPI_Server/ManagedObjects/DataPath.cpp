//************************************************************************
// FILE:		DataPath.cpp
//
// PURPOSE:		Implements DataPath's default functionality.
//************************************************************************

#include "DataPath.h"
#include "HostConnectionDescriptorTable.h"
#include "ConnectionManager.h"
#include "Connection.h"

//************************************************************************
// DataPath:
//
// PURPOSE:		Default constructor
//************************************************************************

DataPath::DataPath( ListenManager *pListenManager, U32 objectClassType, ObjectManager *pManager )
		 :ManagedObject( pListenManager, objectClassType, pManager ){

	m_state			= SSAPI_OBJECT_STATE_GOOD;			// TBDGAI
	m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
}


//************************************************************************
// ~DataPath:
//
// PURPOSE:		The desructor
//************************************************************************

DataPath::~DataPath(){
}


//************************************************************************
// ApplyNecessaryStatusRollupRules:
//
// PURPOSE:		Applies applicabale rollup rules.
//				The object that implements this method is expected to call
//				the protected method with the same name and specify 
//				necessary parms.
//
// RETURN:		true:			object's state's been changed
//				false:			object's state's remained unchanged
//************************************************************************

bool 
DataPath::ApplyNecessaryStatusRollupRules(){

	return false;
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
DataPath::BuildYourValueSet(){

	ManagedObject::BuildYourValueSet();

	AddString( &m_description, SSAPI_DATA_PATH_FID_DESCRIPTION );
	AddString( &m_name,	SSAPI_DATA_PATH_FID_NAME );

	ComposeYourOverallState();

	return StatusReporterInterface::BuildYourValueSet( *this );
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
DataPath::BuildYourselfFromYourValueSet(){

	ManagedObject::BuildYourselfFromYourValueSet();

	GetString( SSAPI_DATA_PATH_FID_DESCRIPTION, &m_description );
	GetString( SSAPI_DATA_PATH_FID_NAME, &m_name );

	return StatusReporterInterface::BuildYourselfFromYourValueSet( *this );
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Responsible for state reconcilation
//
// NOTES:		The state is determined with regards to the states of the 
//				EIP objects stored as children.
//************************************************************************

void 
DataPath::ComposeYourOverallState(){
	
	ConnectionBase			*pConn;
	U32						i, stateString = CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
	int						state = SSAPI_OBJECT_STATE_GOOD;
	DesignatorId			id;

	for( i = 0; i < GetChildCount(); i++ ){
		id = GetChildIdAt( i );
		pConn = (ConnectionBase *)((ConnectionManager *)GetManager())->GetManagedObject( &id );
		if( pConn->GetState() > state ){
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
DataPath::SetName( UnicodeString name, bool shouldPostEvent ){

	if( m_name != name ){
		m_name	= name;
		if( shouldPostEvent )
			FireEventObjectModifed();
	}
}


void 
DataPath::SetDescription( UnicodeString description, bool shouldPostEvent ){

	if( m_description != description ){
		m_description = description;
		if( shouldPostEvent )
			FireEventObjectModifed();
	}
}


//************************************************************************
// IsYourConnection:
//
// PURPOSE:		Checks if a given connection object is a part of this data 
//				path
//************************************************************************

bool 
DataPath::IsYourConnection( ConnectionBase &conn ){

	return IsYourChild( conn.GetDesignatorId() );
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates its data members based on the data in the PTS row
//************************************************************************

bool 
DataPath::BuildYourselfFromPtsRow( HostConnectionDescriptorRecord *pRow ){
	
	m_id			= DesignatorId( pRow->rid, GetClassType() );
	m_ridName		= RowId( pRow->ridName );
	m_ridDescription= RowId( pRow->ridDescription );
	m_correctConnectionCount = pRow->ridEIPCount;

	return true;
}


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates the PTS row based on its data members
//************************************************************************

bool 
DataPath::WriteYourselfIntoPtsRow( HostConnectionDescriptorRecord *pRow ){

	DesignatorId				id;

	pRow->rid					= m_id.GetRowId().GetRowID();
	pRow->version				= HOST_CONNECTION_DESCRIPTOR_TABLE_VERSION;
	pRow->size					= sizeof( HostConnectionDescriptorRecord );
	pRow->ridEIPCount			= GetChildCount();
	pRow->ridDescription		= m_ridDescription;	
	pRow->ridName				= m_ridName;
	pRow->eHostConnectionMode	= (HostConnectionModeEnum)GetPathType();

	for( U32 i = 0; i < pRow->ridEIPCount; i++ ){
		id = GetChildIdAt( i );
		pRow->ridEIPs[i]	= id.GetRowId().GetRowID();
	}

	return true;
}


//************************************************************************
// SetConnectionState:
//
// PURPOSE:		Informs the object of the state of the connection it's connected
//				to. Currently, the state of the connection is the state of the
//				data path.
//************************************************************************

void 
DataPath::SetConnectionState( int connState, U32 connStateString ){
	
	if( StatusReporterInterface::CanChangeStateToGood() ){
		if( ( connState != m_state ) || ( m_stateString != connStateString ) ){
			m_state			= connState;
			m_stateString	= connStateString;
			FireEventObjectModifed();
		}
	}
}


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

bool 
DataPath::DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	DesignatorId			id;

	objectValues.GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
	
	return ((ConnectionManager *)GetManager())->DeleteDataPath( id, pResponder );
}