//************************************************************************
// FILE:		Host.cpp
//
// PURPOSE:		Implements the class Host used to represent host data and
//				connections in the O2K product.
//************************************************************************


#include "Host.h"
#include "ShadowTable.h"
#include "SList.h"
#include "SsapiResponder.h"
#include "DdmSSAPI.h"
#include "UnicodeString.h"
#include "HostManager.h"
#include "UpstreamConnection.h"
#include "ConnectionManager.h"

//************************************************************************
// Host:
//
// PURPOSE:		The default constructor
//************************************************************************

Host::Host( ListenManager *pListenManager, ObjectManager *pManager )
	:ManagedObject( pListenManager, SSAPI_OBJECT_CLASS_TYPE_HOST, pManager ){

	m_state			= SSAPI_OBJECT_STATE_GOOD;			
	m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;	
}


//************************************************************************
// ~Host:
//
// PURPOSE:		The destructor
//************************************************************************

Host::~Host(){

	while( GetConnectionIdCount() )
		RemoveConnectionIdAt( 0 );
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
Host::BuildYourValueSet(){

	static bool		isRecursing = false;

	ValueSet		*pVs = new ValueSet();

	ManagedObject::BuildYourValueSet();

	AddString( &m_name, SSAPI_HOST_FID_NAME );
	AddString( &m_description, SSAPI_HOST_FID_DESCRIPTION );
	AddInt( m_os, SSAPI_HOST_FID_OS );
	AddInt( m_ipAddress, SSAPI_HOST_FID_IP_ADDRESS );

	DumpIdVectorIntoValueSet( pVs, &m_connectionIds );
	AddValue( pVs, SSAPI_HOST_FID_CONNECTION_ID_VECTOR );
	delete pVs;

	if ( !isRecursing ){
		isRecursing = true;
		ComposeYourOverallState();
		isRecursing = false;
	}
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
Host::BuildYourselfFromYourValueSet(){

	ValueSet		*pVs;

	ManagedObject::BuildYourselfFromYourValueSet();

	GetString( SSAPI_HOST_FID_NAME, &m_name );
	GetString( SSAPI_HOST_FID_DESCRIPTION, &m_description );
	GetInt( SSAPI_HOST_FID_OS, &m_os );
	GetInt( SSAPI_HOST_FID_IP_ADDRESS, &m_ipAddress );

	// clean the vector
	while( GetConnectionIdCount() )
		RemoveConnectionIdAt( 0 );

	// populate with new values
	pVs = (ValueSet *)GetValue( SSAPI_HOST_FID_CONNECTION_ID_VECTOR );
	if( pVs )
		DumpValueSetIntoIdVector( pVs, &m_connectionIds );


	return StatusReporterInterface::BuildYourselfFromYourValueSet( *this );
}


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

bool 
Host::ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	return ((HostManager *)GetManager())->ModifyHost( objectValues, pResponder );
}


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

bool 
Host::DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	DesignatorId			id;

	objectValues.GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );

	return ((HostManager *)GetManager())->DeleteHost( id, pResponder );
}


//************************************************************************
// ClearIdVector:
//
// PURPOSE:		Deallocates memory taken by DesignatorId objects in the
//				vector specified.
//************************************************************************

void 
Host::ClearIdVector( Container *pIdVector ){

	DesignatorId		*pId;

	while( pIdVector->Count() ){
		pIdVector->GetAt( (CONTAINER_ELEMENT &)pId, 0  );
		pIdVector->RemoveAt( 0 );
		delete pId;
	}
}


//************************************************************************
// WriteYourSelfIntoHostDescriptorRow:
//
// PURPOSE:		Writes out data from data members into the PTS record
//************************************************************************

bool 
Host::WriteYourSelfIntoHostDescriptorRow( HostDescriptorRecord *pRow ){

	DesignatorId			*pId;

	memset( pRow, 0, sizeof(HostDescriptorRecord) );

	pRow->size = sizeof(HostDescriptorRecord);
	pRow->version = HOST_DESCRIPTOR_TABLE_VERSION;
	pRow->rid		= m_id.GetRowId().GetRowID();
	pRow->ipAddress = m_ipAddress;
	pRow->eipCount	= m_connectionIds.Count();
	pRow->hostOs	= m_os;

	m_name.CString( &pRow->name, sizeof( pRow->name ) );
	m_description.CString( &pRow->description, sizeof( pRow->description ) );

	for( U32 i = 0; i < m_connectionIds.Count(); i++ ){
		m_connectionIds.GetAt( (CONTAINER_ELEMENT &)pId, i );
		pRow->eip[i] = pId->GetRowId().GetRowID();
	}

	return true;
}


//************************************************************************
// BuildYourSelfFromHostDescriptorRow:
//
// PURPOSE:		Populates data members based on the data from the PTS
//				row
//************************************************************************

bool 
Host::BuildYourSelfFromHostDescriptorRow( HostDescriptorRecord *pRow ){


	m_id					= DesignatorId( pRow->rid, (U16)GetClassType() );
	m_name					= (void *)&pRow->name;
	m_description			= (void *)&pRow->description;
	m_ipAddress				= pRow->ipAddress;
	m_os					= pRow->hostOs;
	m_correctEipCount		= pRow->eipCount;

	return true;
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
Host::ApplyNecessaryStatusRollupRules(){

	bool			rc;

	rc = StatusReporterInterface::ApplyNecessaryStatusRollupRules( this, m_children, m_parents );

	if( rc )
		FireEventObjectModifed();

	return rc;
}


//************************************************************************
// AreThereAnyTooLongStrings
//
// PURPOSE:		Checks every string's size against the maximum in the PTS 
//				row. Will raise an exception if something's wrong.
//				Checks strings in the value set, not the members. 
//
// RETURN:		true:	all strings are OK, may proceed
//				false:	an exception was rased, terminate normal execution
//************************************************************************

bool 
Host::AreThereAnyTooLongStrings( SsapiResponder *pResponder ){

	HostDescriptorRecord		row;

	BuildYourselfFromYourValueSet();

	if( m_name.GetSize() > sizeof( row.name ) ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_NAME_TOO_LONG);
		return false;
	}

	if( m_description.GetSize() > sizeof( row.description ) ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_DESCRIPTION_TOO_LONG );
		return false;
	}
	
	row.size = 1;
	return true;
}


//************************************************************************
// ComposeYourOverallStatus:
//
// PURPOSE:		Runs thru its children checking if they are present and
//				what their status is. Detemines its state based ob the
//				information collected.
//
// NOTES:		1. The state is determined as defined by the StatusReporter
//				interface through checking states of the EIP objects
//				stored as children
//************************************************************************

//
// TBDGAI: the correct eip count must be persistant, or we'll not see
//			a missing EIP. Do it LATER! 
//
void 
Host::ComposeYourOverallState(){

	U32						i, stateString = CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
	int						state = SSAPI_OBJECT_STATE_GOOD;
	DesignatorId			id;
	ConnectionBase			*pConn;
	ConnectionManager		*pCM = (ConnectionManager *) GetObjectManager( GetManager(), SSAPI_MANAGER_CLASS_TYPE_CONNECTION_MANAGER );

	// first of all, check of all EIPs are present

	if( m_correctEipCount != m_connectionIds.Count() ){
		m_state = SSAPI_OBJECT_STATE_WARNING;
		m_stateString = CTS_SSAPI_HOST_STATE_CONNECTION_MESSING;
		FireEventObjectModifed();
		return;
	}

	for( i = 0; i < m_connectionIds.Count(); i++ ){
		GetConnectionIdAt( i, id );
		pConn = (ConnectionBase *)pCM->GetManagedObject( &id );
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
// Connection Methods:
//
// PURPOSE:		The following methods do basic operations on the 
//				connection id vector data member
//************************************************************************

bool 
Host::IsYourConnection( const DesignatorId &id ){
	
	U32				i;
	DesignatorId	*pId;

	for( i = 0; i < m_connectionIds.Count(); i++ ){
		m_connectionIds.GetAt( (CONTAINER_ELEMENT &)pId, i );
		if( ((DesignatorId &)id) == *pId )
			return true;
	}
	return false;
}


void 
Host::AddConnectionId( const DesignatorId &id ){

	// remove if already there
	RemoveConnectionId( id );

	m_connectionIds.Add( (CONTAINER_ELEMENT) new DesignatorId( id ) );
}


bool 
Host::GetConnectionIdAt( U32 position, DesignatorId &id ){

	DesignatorId			*pId;

	if( !m_connectionIds.GetAt( (CONTAINER_ELEMENT &)pId, position ) )
		return false;

	id = *pId;
	return true;
}


bool 
Host::RemoveConnectionId( const DesignatorId &id ){

	DesignatorId		*pId;

	for( U32 i = 0; i < m_connectionIds.Count(); i++ ){
		m_connectionIds.GetAt( (CONTAINER_ELEMENT &)pId, i );
		if( ((DesignatorId &)id) == *pId ){
			m_connectionIds.RemoveAt( i );
			delete pId;
			return true;
		}
	}

	return false;
}


bool 
Host::RemoveConnectionIdAt( U32 position ){
	
	DesignatorId	*pId;

	if( m_connectionIds.GetAt( (CONTAINER_ELEMENT &)pId, position ) ){
		delete pId;
		return m_connectionIds.RemoveAt( position )? true : false;
	}
	
	return false;
}