//************************************************************************
// FILE:		Fan.h
//
// PURPOSE:		Defines class Fan that is goin to be used to represent all
//				fan devices in the O2K box.
//************************************************************************

#ifndef __FAN_MO_H__
#define	__FAN_MO_H__


#include "Device.h"
#include "PowerableInterface.h"

#ifdef WIN32
#pragma pack(4)
#endif

class Fan : public Device, public PowerableInterface {
	
	int				m_speed;
	int				m_fanNumber;
	int				m_numberInEvcRow;

public:


//************************************************************************
// Fan:
//
// PURPOSE:		Default constructor
//************************************************************************

Fan( ListenManager *pListenManager, int fanNumber, int numInEvc );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new Fan( GetListenManager(), m_fanNumber, m_numberInEvcRow ); }


//************************************************************************
// ~Fan:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~Fan();


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
// SetSpeed:
//
// PURPOSE:		Sets new speed for the fan
//************************************************************************

bool SetSpeed( ValueSet *pRequestSet, SsapiResponder *pResponder );


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

virtual bool BuildYourselfFromPtsRow( void *pRow );


//************************************************************************
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


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
//************************************************************************



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

#endif	// __FAN_MO_H__