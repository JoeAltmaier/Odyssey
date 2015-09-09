//************************************************************************
// FILE:		Chassis.cpp
//
// PURPOSE:		Implements class Chassis that is used to represent O2K
//				chassis device. 
//************************************************************************

#include "EVCStatusRecord.h"
#include "Chassis.h"
#include "SSAPIServerVersion.h"
#include "ClassTypeMap.h"
#include "SsapiResponder.h"
#include "DeviceManager.h"
#include "DdmSSAPI.h"

#ifndef WIN32
#include "time.h"
#include "network.h"
extern unsigned char box_IP_address[4];
#endif


//************************************************************************
// Chassis:
//
// PURPOSE:		
//************************************************************************

Chassis::Chassis( ListenManager *pListenManager, int num )
		:Device( pListenManager, SSAPI_OBJECT_CLASS_TYPE_CHASSIS ),
		 PowerableInterface(){

	m_numberInEvcRow		= num;
	m_isPowered				= true;

#ifndef WIN32
#if 0
	Network::FillIP(&m_ipAddress, box_IP_address[0], box_IP_address[1], box_IP_address[2], box_IP_address[3]);
	Network::FillIP(&m_subnetMask, 255, 255, 255, 0);
	Network::FillIP(&m_gateway, box_IP_address[0], box_IP_address[1], box_IP_address[2], 1);
#endif
#endif
}


//************************************************************************
// ~Chassis:
//
// PURPOSE:		The destructor
//************************************************************************

Chassis::~Chassis(){
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
Chassis::BuildYourValueSet(){

	ValueSet			*pTrapAddresses = new ValueSet();
	U32					address;
	time_t				timeTemp;

	Device::BuildYourValueSet();

	AddInt( m_numberOfBays, SSAPI_CHASSIS_FID_NUM_BAYS );
	AddInt( m_numberOfBusSegments, SSAPI_CHASSIS_FID_NUM_BUS_SEGMENTS );
	AddInt( m_numberOfSlots, SSAPI_CHASSIS_FID_NUM_SLOTS );
	AddInt( m_ssapiMajorVersionNumber, SSAPI_CHASSIS_FID_SSAPI_MAJOR_VERSION );
	AddInt( m_ssapiMinorVersionNumber, SSAPI_CHASSIS_FID_SSAPI_MINOR_VERSION );
	AddInt( m_keyPosition, SSAPI_CHASSIS_FID_KEY_POSITION );
	AddInt( m_gateway, SSAPI_CHASSIS_FID_GATEWAY );

	int	i = m_isPowered? 1 : 0;
	AddInt( i, SSAPI_OBJECT_FID_IS_POWERED );

	UnicodeString temp( m_serialNumber );
	AddString( &temp, SSAPI_CHASSIS_FID_SERIAL_NUMBER );

	AddInt( m_ipAddress, SSAPI_CHASSIS_FID_IP_ADDRESS );
	AddInt( m_subnetMask, SSAPI_CHASSIS_FID_SUBNET_MASK );

	time( &timeTemp );
	m_gmtTimeBase = timeTemp;
	m_gmtTimeBase *= 1000;
	AddInt64( m_gmtTimeBase, SSAPI_CHASSIS_FID_GMT_TIME_BASE );

	AddString( &m_hostName, SSAPI_CHASSIS_FID_HOST_NAME );
	AddString( &m_location, SSAPI_CHASSIS_FID_LOCATION );

	for( i = 0; i < m_trapIpAddresses.Count(); i++ ){
		m_trapIpAddresses.GetAt( (CONTAINER_ELEMENT &)address, i );
		pTrapAddresses->AddInt( address, i );
	}
	AddValue( pTrapAddresses, SSAPI_CHASSIS_FID_TRAP_IP_ADDRESS_VECTOR );

	delete pTrapAddresses;

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
Chassis::BuildYourselfFromYourValueSet(){

	ValueSet			*pTrapAddresses;
	U32					address;

	Device::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_CHASSIS_FID_NUM_BAYS, &m_numberOfBays );
	GetInt( SSAPI_CHASSIS_FID_NUM_BUS_SEGMENTS, &m_numberOfBusSegments );
	GetInt( SSAPI_CHASSIS_FID_NUM_SLOTS, &m_numberOfSlots );
	GetInt( SSAPI_CHASSIS_FID_SSAPI_MAJOR_VERSION, &m_ssapiMajorVersionNumber );
	GetInt( SSAPI_CHASSIS_FID_SSAPI_MINOR_VERSION, &m_ssapiMinorVersionNumber );
	GetInt( SSAPI_CHASSIS_FID_KEY_POSITION, &m_keyPosition );
	GetInt( SSAPI_CHASSIS_FID_GATEWAY, &m_gateway );

	UnicodeString temp;
	GetString( SSAPI_CHASSIS_FID_SERIAL_NUMBER, &temp );
	temp.GetAsciiString( m_serialNumber );

	int	i;
	GetInt( SSAPI_OBJECT_FID_IS_POWERED, &i );
	m_isPowered	= i? true : false;
	
	GetInt( SSAPI_CHASSIS_FID_IP_ADDRESS, &m_ipAddress );
	GetInt( SSAPI_CHASSIS_FID_SUBNET_MASK, &m_subnetMask );
	GetString( SSAPI_CHASSIS_FID_HOST_NAME, &m_hostName );
	GetString( SSAPI_CHASSIS_FID_LOCATION, &m_location );

	m_trapIpAddresses.RemoveAll();
	pTrapAddresses = (ValueSet *)GetValue(SSAPI_CHASSIS_FID_TRAP_IP_ADDRESS_VECTOR);
	for( i = 0; pTrapAddresses && ( i < pTrapAddresses->GetCount()); i++ ){
		pTrapAddresses->GetInt( i, (int *)&address );
		m_trapIpAddresses.Add( (CONTAINER_ELEMENT)address );
	}

	m_assetInfo.BuildYourselfFromYourValueSet( *this );

	return true;
}


//************************************************************************
// PowerOn:
//
// PURPOSE:		Declares an API method 
//************************************************************************

bool 
Chassis::PowerOn( SsapiResponder *pResponder ){
	
	pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	return true;
}


//************************************************************************
// PowerOff:
//
// PURPOSE:		Declares an API method
//************************************************************************

bool 
Chassis::PowerOff( SsapiResponder *pResponder ){

	((DeviceManager *)GetManager())->PowerDownChassis( pResponder );	
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
Chassis::ComposeYourOverallState(){

	// TBDGAI	-> determine chassis' state! but how exactly?
	if( StatusReporterInterface::CanChangeStateToGood() ){
		StatusReporterInterface::m_state	= SSAPI_OBJECT_STATE_GOOD;
		m_stateString						= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
	}
	m_name			= CTS_SSAPI_CHASSIS_NAME;
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool 
Chassis::BuildYourselfFromPtsRow( void *pRow_ ){
	
	EVCStatusRecord	*pRow	= (EVCStatusRecord *)pRow_;

	m_id						= DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_CHASSIS );
	m_keyPosition				= pRow->KeyPosition;
	m_ssapiMajorVersionNumber	= SSAPI_MAJOR_VERSION_NUMBER;
	m_ssapiMinorVersionNumber	= SSAPI_MINOR_VERSION_NUMBER;
	m_numberOfBays				= SSAPI_NUMBER_OF_BAYS;
	m_numberOfBusSegments		= SSAPI_NUMBER_OF_BUS_SEGEMENTS;
	m_numberOfSlots				= SSAPI_NUMBER_OF_SLOTS;

	m_assetInfo.ClearInfo();
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
Chassis::HandleObjectAddedEvent( ValueSet *pObj, bool postEvent ){

	DesignatorId				id;
	
	pObj->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );

	// we only need PHSDataTemperature
	if( id.GetClassId() == SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE ){
		if( id.GetRowId() == m_id.GetRowId() )
			if( m_numberInEvcRow == (id.GetUniqueId() & 0x00FF) )
				AddPhsDataItem( id, postEvent );
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
Chassis::IsYourDevice( Device &device ){
	
	ClassTypeMap		map;
	
	return map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_DEVICE_COLLECTION,	
								device.GetClassType() ,
								true );
	return false;
}


//************************************************************************
// BuildYourselfFromConfigSettingsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool 
Chassis::BuildYourselfFromConfigSettingsRow( SystemConfigSettingsRecord *pRow ){
	
	m_rowIdOfConfigSettings	= pRow->rid;
	m_ipAddress				= pRow->ipAddress;
	m_subnetMask			= pRow->subnetMask;
	m_hostName				= UnicodeString( (void *)pRow->hostName );
	m_location				= UnicodeString( (void *)pRow->location );
	m_gateway				= pRow->gateway;

	m_trapIpAddresses.RemoveAll();
	for( U32 i = 0; i < pRow->snmpTrapAddressCount; i++ )
		m_trapIpAddresses.Add( (CONTAINER_ELEMENT)pRow->snmpTrapAddress[i] );

	return true;
}


//************************************************************************
// WriteYourselfIntoConfigSettingsRow:
//
// PURPOSE:		Writes data from data members into the appropriate PTS
//				row;
//************************************************************************

bool 
Chassis::WriteYourselfIntoConfigSettingsRow( SystemConfigSettingsRecord *pRow ){

	U32   address;

	pRow->size			= sizeof(SystemConfigSettingsRecord);
	pRow->version		= SYSTEM_CONFIG_SETTINGS_VERSION;
	pRow->ipAddress		= m_ipAddress;
	pRow->subnetMask	= m_subnetMask;
	pRow->gateway		= m_gateway;
	m_hostName.CString( pRow->hostName, sizeof( pRow->hostName ) );
	m_location.CString( pRow->location, sizeof( pRow->location ) );

	for( pRow->snmpTrapAddressCount = 0; pRow->snmpTrapAddressCount < m_trapIpAddresses.Count(); pRow->snmpTrapAddressCount++ ){
		m_trapIpAddresses.GetAt( (CONTAINER_ELEMENT &)address, pRow->snmpTrapAddressCount );
		pRow->snmpTrapAddress[pRow->snmpTrapAddressCount] = address;
	}

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
Chassis::ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	ObjectManager				*pManager = ((DdmSSAPI *)pParentDdmSvs)->GetObjectManager( SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );
	UnicodeString				us;
	SystemConfigSettingsRecord	row;

	// check string lengths
	if( objectValues.GetString( SSAPI_CHASSIS_FID_HOST_NAME, &us ) ){
		if( us.GetSize() > sizeof( row.hostName ) ){
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_HOST_NAME_TOO_LONG );
			return true;
		}
	}

	if( objectValues.GetString( SSAPI_CHASSIS_FID_LOCATION, &us ) ){
		if( us.GetSize() > sizeof( row.hostName ) ){
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_LOCATION_TOO_LONG );
			return true;
		}
	}
	
	row.size = 1;
	return ((DeviceManager *)pManager)->ModifyChassisDevice( objectValues, pResponder );
}

