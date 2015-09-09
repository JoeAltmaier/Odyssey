//************************************************************************
// FILE;		DDH.h
//
// PURPOSE:		Defines the object used to represent DDH devices inside
//				the O2K system
//************************************************************************

#ifndef __DDH_MO_H__
#define	__DDH_MO_H__

#include "Device.h"
#include "SsapiTypes.h"
#include "IopStatusTable.h"
#include "AssetInterface.h"

#ifdef WIN32
#pragma pack(4)
#endif

class DDH : public Device {

	UnicodeString		m_manufacturer;
	UnicodeString		m_hwPartNo;
	U32					m_hwRevision;
	LocalizedDateTime	m_hwMfgDate;
	AssetInterface		m_assetInfo;
	U32					m_ddhNumber;	// 0...3

	IopState			m_internalState;	// internal, used to get the SSAPI state

public:


//************************************************************************
// DDH:
//
// PURPOSE:		Default constructor
//************************************************************************

DDH( ListenManager *pListenManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new DDH( GetListenManager() ); }


//************************************************************************
// ~DDH:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~DDH();


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
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

virtual bool BuildYourselfFromPtsRow( void *pRow );


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
// operator=
//************************************************************************


virtual const ValueSet& operator=(const ValueSet& obj ){ return *this = obj; }


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

virtual void ComposeYourOverallState();


//************************************************************************
//************************************************************************
//************************************************************************
//************************************************************************
};

#endif // __DDH_MO_H__