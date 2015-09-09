//************************************************************************
// FILE:		Connection.cpp
//
// PURPOSE:		Implements an abstract base class for connections to 
//				external FC ports/devices
//************************************************************************

#include "Connection.h"
#include "FCPortDatabaseTable.h"
#include "ConnectionManager.h"
#include "DeviceManager.h"
#include "DdmSSAPI.h"


//************************************************************************
// ConnectionBase:
//
// PURPOSE:		Default constructor
//************************************************************************

ConnectionBase::ConnectionBase( ListenManager *pListenManager, U32 classType, ObjectManager *pManager )
			:ManagedObject( pListenManager, classType, pManager ){
}


//************************************************************************
// ~ConnectionBase:
//
// PURPOSE:		The destructor
//************************************************************************

ConnectionBase::~ConnectionBase(){
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
ConnectionBase::BuildYourValueSet(){
	
	ManagedObject::BuildYourValueSet();

	AddU32( m_i_t_id, SSAPI_CONNECTION_FID_ID );
	AddString( &m_wwName, SSAPI_CONNECTION_FID_WWNAME );
	AddGenericValue( (char *)&m_portId, sizeof(m_portId), SSAPI_CONNECTION_FID_GEMINI_PORT_ID );
	AddString( &m_name, SSAPI_CONNECTION_FID_NAME );

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
ConnectionBase::BuildYourselfFromYourValueSet(){

	ManagedObject::BuildYourselfFromYourValueSet();

	GetU32( SSAPI_CONNECTION_FID_ID, &m_i_t_id );
	GetString( SSAPI_CONNECTION_FID_WWNAME, &m_wwName );
	GetGenericValue( (char *)&m_portId, sizeof(m_portId), SSAPI_CONNECTION_FID_GEMINI_PORT_ID );
	GetString( SSAPI_CONNECTION_FID_NAME, &m_name );

	return StatusReporterInterface::BuildYourselfFromYourValueSet( *this );
}


//************************************************************************
//
//************************************************************************

void WriteWWNToUnicodeString( char* pWWN, U32 wwnSize, UnicodeString &us ){

	U32				i;
	char			buff[4];
	UnicodeString	temp;

	us	= StringClass("");
	for( i = 0; i < wwnSize; i++ ){
		memset( buff, 0, sizeof(buff) );
		sprintf( buff, "%02X", *((unsigned char *)(pWWN + i)) );
		temp = temp + StringClass( buff );
	}
	
	us = temp;
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the contents of the row
//************************************************************************

void 
ConnectionBase::BuildYourselfFromPtsRow( FCPortDatabaseRecord *pRow, ObjectManager *pManager ){

	DeviceManager	*pDMgr = (DeviceManager *)(((ConnectionManager *)pManager)->GetDdmSSAPI())->GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER);

	m_id			= DesignatorId( RowId( pRow->rid ), (U16)GetClassType() );
	m_i_t_id		= pRow->id;
	WriteWWNToUnicodeString( (char *)pRow->wwName, sizeof(pRow->wwName), m_wwName );
	pDMgr->GetDesignatorIdByRowId( pRow->ridLoopDescriptor, m_portId );
	m_ridName		= pRow->ridName;
	m_isInvalidConfiguration = (pRow->attribs & FC_PORT_OWNER_VC_USE_OK)? false : true;

	switch( pRow->portStatus ){
		case FC_PORT_STATUS_ACTIVE:
			m_state			= SSAPI_OBJECT_STATE_GOOD;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
			break;

		case FC_PORT_STATUS_REMOVED:
			m_state			= SSAPI_OBJECT_STATE_DEAD;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_DEAD;
			break;

		case FC_PORT_STATUS_LOOP_DOWN:
			m_state			= SSAPI_OBJECT_STATE_UNKNOWN;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_UNKNOWN;
			break;

		default:
			ASSERT(0);
			break;
	}
}


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates the table row based on the data members
//************************************************************************

void 
ConnectionBase::WriteYourselfIntoPtsRow( FCPortDatabaseRecord *pRow ){
	
	pRow->size				= sizeof(FCPortDatabaseRecord);
	pRow->version			= FC_PORT_DATABASE_TABLE_VERSION;
	pRow->id				= m_i_t_id;
	pRow->ridLoopDescriptor	= m_portId.GetRowId().GetRowID();
	pRow->ridName			= m_ridName;
	
	m_wwName.CString( pRow->wwName, sizeof( pRow->wwName ) );

}


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		1. Must be overridden by objects that can be modified
//				2. Only the name may be modified
//************************************************************************

bool 
ConnectionBase::ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	UnicodeString	name;

	if( !objectValues.GetString( SSAPI_CONNECTION_FID_NAME , &name ) ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION_NO_CONNECTION_NAME );
		return true;
	}

	((ConnectionManager *)GetManager())->ModifyConnectionName( this, name, pResponder );

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
ConnectionBase::ApplyNecessaryStatusRollupRules(){

	bool			rc;

	rc = StatusReporterInterface::ApplyNecessaryStatusRollupRules( this, m_children, m_parents );

	if( rc )
		FireEventObjectModifed();

	return rc;
}


//************************************************************************
// ComposeYourOverallStatus:
//
// PURPOSE:		Runs thru its children checking if they are present and
//				what their status is. Detemines its state based ob the
//				information collected.
//
// NOTES:		1. The state of the EP object is determined by the 
//					'portState' in the FCPortDatabase record
//************************************************************************

void 
ConnectionBase::ComposeYourOverallState(){
	
}


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

bool 
ConnectionBase::DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	((ConnectionManager *)GetManager())->DeleteConnection( this, pResponder );
	return true;
}
