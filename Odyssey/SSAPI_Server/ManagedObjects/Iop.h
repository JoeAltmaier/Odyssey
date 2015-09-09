//************************************************************************
// FILE:		Iop.h
//
// PURPOSE:		Defines class IOP that serves as an abstract base for
//				all IOP-type devices in the O2K.
//************************************************************************

#ifndef __IOP_MO_H__
#define	__IOP_MO_H__

#include "Board.h"
#include "ServiceableInterface.h"
#include "PowerableInterface.h"
#include "UnicodeString.h"
#include "AssetInterface.h"

#ifdef WIN32
#pragma pack(4)
#endif

class Iop : public Board, public ServiceableInterface, public PowerableInterface{

protected:

	int				m_redundantSlotNumber;			// slot # of the counterpart 
	int				m_internalState;				// enum from IopState
	UnicodeString	m_manufacturer;
	UnicodeString	m_hardwareVersion;
	UnicodeString	m_serialNumber;
	AssetInterface	m_assetInfo;


//************************************************************************
// Iop
//
// PURPOSE:		Default constructor
//************************************************************************

Iop( ListenManager *pListenManager, U32 objectClassType );


public:


//************************************************************************
// ~Iop:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~Iop();


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
// SetServiceState:
//
// PURPOSE:		Declares an API method 
//************************************************************************

virtual bool SetServiceState( SsapiResponder *pResponder, U32 newState );


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

virtual bool BuildYourselfFromPtsRow( void *pRow );


//************************************************************************
// IsPowered:
//
// PURPOSE:		An accessor
//************************************************************************

virtual bool IsPowered(){ return m_isPowered; }


//************************************************************************
// Lock:
//
// PURPOSE:		Declares an API method 
//************************************************************************

virtual bool Lock( SsapiResponder *pResponder );


//************************************************************************
// UnLock:
//
// PURPOSE:		Declares an API method
//************************************************************************

virtual bool UnLock( SsapiResponder *pResponder );


//************************************************************************
// Public Accessors:
//************************************************************************

int GetRedundantSlotNumber() const { return m_redundantSlotNumber; }

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



};

#endif	// __IOP_MO_H__
