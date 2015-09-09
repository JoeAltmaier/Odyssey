//************************************************************************
// FILE:		Chassis.h
//
// PURPOSE:		Defines class Chassis that is used to represent O2K
//				chassis device. 
//
// NOTE:		SSAPI version is located on this object
//************************************************************************

#ifndef __CHASSIS_H__
#define	__CHASSIS_H__

#include "Device.h"
#include "PowerableInterface.h"
#include "UnicodeString.h"
#include "SystemConfigSettingsTable.h"
#include "AssetInterface.h"
#include "SsapiTypes.h"


#ifdef WIN32
#pragma pack(4)
#endif

class Chassis : public Device, public PowerableInterface {
	
	LocalizedDateTime	m_gmtTimeBase;
	int					m_numberOfBays;					// for HDDs
	int					m_numberOfBusSegments;
	int					m_numberOfSlots;				// for IOPs
	int					m_ssapiMajorVersionNumber;
	int					m_ssapiMinorVersionNumber;
	int					m_keyPosition;
	StringClass			m_serialNumber;					// to be transfered as Unicode String
	int					m_ipAddress;
	int					m_subnetMask;
	int					m_gateway;
	UnicodeString		m_hostName;
	UnicodeString		m_location;
	SList				m_trapIpAddresses;
	AssetInterface		m_assetInfo;
	RowId				m_rowIdOfConfigSettings;		// internal
	int					m_numberInEvcRow;				// internal
		


public:

//************************************************************************
// Chassis:
//
// PURPOSE:		
//************************************************************************

Chassis( ListenManager *pListenManager, int numInEvc );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new Chassis( GetListenManager(), m_numberInEvcRow ); }


//************************************************************************
// 
//************************************************************************

virtual ~Chassis();




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

virtual bool BuildYourValueSet();


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

virtual bool BuildYourselfFromYourValueSet();


//************************************************************************
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


//************************************************************************
// PowerOn:
//
// PURPOSE:		Declares an API method 
//************************************************************************

virtual bool PowerOn( SsapiResponder *pResponder );


//************************************************************************
// PowerOff:
//
// PURPOSE:		Declares an API method
//************************************************************************

virtual bool PowerOff( SsapiResponder *pResponder );


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

virtual bool BuildYourselfFromPtsRow( void *pRow );


//************************************************************************
// BuildYourselfFromConfigSettingsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool BuildYourselfFromConfigSettingsRow( SystemConfigSettingsRecord *pRow );


//************************************************************************
// WriteYourselfIntoConfigSettingsRow:
//
// PURPOSE:		Writes data from data members into the appropriate PTS
//				row;
//************************************************************************

bool WriteYourselfIntoConfigSettingsRow( SystemConfigSettingsRecord *pRow );


//************************************************************************
// IsYourDevice:
//
// PURPOSE:		Determines if a gven device belongs to this device
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

virtual bool IsYourDevice( Device &device );


//************************************************************************
// IsPowered:
//
// PURPOSE:		An accessor
//************************************************************************

virtual bool IsPowered(){ return m_isPowered; }


//************************************************************************
// Accessors
//************************************************************************

RowId	GetConfigSettingsRowId() const { return m_rowIdOfConfigSettings;	}
U32		GetMaxTrapAddressCount() const { return SCS_SNMP_TRAP_ADDRESS_MAX_COUNT; }


protected:

//************************************************************************
// HandleObjectAddedEvent:
//
// PURPOSE:		Called by the manager to inform the object that a new 
//				object has been added to the system. The object may be
//				interested to know if this new object is its child/parent
//				and update its vectors as needed.
//************************************************************************

virtual void HandleObjectAddedEvent( ValueSet *pObj, bool postEvent = true );


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

virtual void ComposeYourOverallState();


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

virtual bool ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder );


};

#endif //__CHASSIS_H__

