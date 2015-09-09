//************************************************************************
// FILE:		HDDDevice.cpp
//
// PURPOSE:		Implements class HDDDevice that is used to represent all HDD
//				devices in the O2K
//************************************************************************

#include "HDDDevice.h"
#include "DiskDescriptor.h"
#include "SsapiResponder.h"
#include "DeviceManager.h"
#include "ClassTypeMap.h"
#include "PathDescriptor.h"

//************************************************************************
// HDDDevice:
//
// PURPOSE:		Default constructor
//************************************************************************

HDDDevice::HDDDevice( ListenManager *pListenManager )
		:Device( pListenManager, SSAPI_OBJECT_CLASS_TYPE_HDD_DEVICE ){
}


//************************************************************************
// ~HDDDevice
// 
// PURPOSE:		The destructor
//************************************************************************

HDDDevice::~HDDDevice(){
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
HDDDevice::BuildYourValueSet(){

	Device::BuildYourValueSet();

	AddInt( m_bayNumber, SSAPI_HDD_FID_BAY_NUMBER );
	AddInt( m_targetId, SSAPI_HDD_FID_TARGET_ID );
	AddInt( m_status, SSAPI_HDD_FID_STATUS );

	UnicodeString	temp( m_serialNumber );
	AddString( &temp, SSAPI_HDD_FID_SERIAL_NUMBER );

	AddInt64( m_capacity, SSAPI_HDD_FID_CAPACITY ); 

	int i = m_isLocked? 1 : 0;
	AddInt(m_isLocked, SSAPI_OBJECT_FID_IS_LOCKED );

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
HDDDevice::BuildYourselfFromYourValueSet(){
	

	Device::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_HDD_FID_BAY_NUMBER, &m_bayNumber );
	GetInt( SSAPI_HDD_FID_TARGET_ID, &m_targetId );
	GetInt( SSAPI_HDD_FID_STATUS, &m_status );

	UnicodeString	temp;
	GetString( SSAPI_HDD_FID_SERIAL_NUMBER, &temp );
	temp.GetAsciiString( m_serialNumber );

	GetInt64( SSAPI_HDD_FID_SERIAL_NUMBER, &m_capacity ); 
	
	int i;
	GetInt( SSAPI_OBJECT_FID_IS_LOCKED, &i );
	m_isLocked = i? true : false;

	m_assetInfo.BuildYourselfFromYourValueSet( *this );
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
HDDDevice::ComposeYourOverallState(){

	switch( m_status ){
		case DriveInvalid:
		case DriveNotSpinning:
		case DriveSpinningUp:
		case DriveReady:
			if( StatusReporterInterface::CanChangeStateToGood() ){
				m_state			= SSAPI_OBJECT_STATE_GOOD;
				m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
			}
			break;

		case DriveRemoved:
		case DriveNotPresent:
		case DriveHardFailure:
			m_state			= SSAPI_OBJECT_STATE_DEAD;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_DEAD;
			break;

		default:
			ASSERT(0);
			m_state			= SSAPI_OBJECT_STATE_UNKNOWN;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_UNKNOWN;
			break;
	}
}


//************************************************************************
// Lock:
//
// PURPOSE:		Declares an API method 
//************************************************************************

bool 
HDDDevice::Lock( SsapiResponder *pResponder ){

	((DeviceManager*)GetManager())->ChangeHDDLockState(	m_bayNumber,
														true,
														m_id,
														pResponder );
	return true;
}


//************************************************************************
// UnLock:
//
// PURPOSE:		Declares an API method
//************************************************************************

bool 
HDDDevice::UnLock( SsapiResponder *pResponder ){

	((DeviceManager*)GetManager())->ChangeHDDLockState(	m_bayNumber,
														false,
														m_id,
														pResponder );
	return true;
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool 
HDDDevice::BuildYourselfFromPtsRow( void *pRow_ ){
	
	DiskDescriptor		*pRow = (DiskDescriptor *)pRow_;
	INQUIRY				inquiry;	// for z-termination

	m_id			= DesignatorId( pRow->rid, (U16)GetClassType() );
	m_bayNumber		= pRow->SlotID;
	m_serialNumber	= StringClass( pRow->SerialNumber );
	m_status		= pRow->CurrentStatus;
	m_name			= CTS_SSAPI_HDD_DEVICE_NAME;	
	// m_capacity TBDGAI -> need to convert U64 -> I64
	m_isLocked		= (pRow->LockState == DRIVE_LOCKED)? true : false;

	// populate asset info
	memcpy( &inquiry, &pRow->InqData, sizeof(inquiry) );
	inquiry.ProductId[sizeof(inquiry.ProductId)] = 0;	// z-termination
	m_assetInfo.ClearInfo();
	m_assetInfo.SetVersionNumber( UnicodeString( StringClass( (char *)&inquiry.ProductId ) ) );
	m_assetInfo.SetSerialNumber( UnicodeString( StringClass( (char *)&pRow->SerialNumber ) ) );

	return true;
}


//************************************************************************
// HandleObjectAddedEvent:
//
//************************************************************************

void 
HDDDevice::HandleObjectAddedEvent( ValueSet *pObj, bool postEvent ){

	DesignatorId	id;
	ClassTypeMap	map;	

	pObj->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
	// we only need objects on the right rows
	if( ( id.GetRowId() == m_ridPerformanceRecord ) || ( id.GetRowId() == m_ridStatusRecord ) )
			AddPhsDataItem( id, postEvent );
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
HDDDevice::IsYourDevice( Device &device ){

	return false;
}


//************************************************************************
// SetPhsRowIds:
//
// PURPOSE:		Provided for the DeviceManager to set row ids when
//				appropriate.
//
// RETURN:		true:		at least one row id was modified
//				false:		row ids were as requested already
//************************************************************************

bool 
HDDDevice::SetPhsRowIds( RowId &ridStatus, RowId &ridPerformance, bool postEvent ){

	if( (m_ridStatusRecord != ridStatus )
		||
		(m_ridPerformanceRecord != ridPerformance ) ){

		m_ridStatusRecord		= ridStatus;
		m_ridPerformanceRecord	= ridPerformance;

		if( postEvent )
			FireEventObjectModifed();

		return true;
	}

	return false;
}


//************************************************************************
// BuildYourselfFromPathRow:
//
// PURPOSE:		Populates some data members based on the contents of the
//				row passed.
//************************************************************************

void 
HDDDevice::BuildYourselfFromPathRow( PathDescriptor *pRow ){

	// check if this is an active descriptor!
	if( GetDesignatorId().GetRowId() != pRow->ridActiveDesc )
		return;

	m_location		= pRow->FCTargetID;
	m_targetId		= pRow->FCTargetID;
}