//************************************************************************
// FILE:		DiskPowerSupply.h
//
// PURPOSE:		Defines class that will be used to represent disk 
//				power supplies in the O2K
//************************************************************************

#ifndef __DISK_POWER_SUPPLY_H__
#define	__DISK_POWER_SUPPLY_H__

#include "PowerSupply.h"

#ifdef WIN32
#pragma pack(4)
#endif

class DiskPowerSupply : public PowerSupply{


public:

//************************************************************************
// DiskPowerSupply:
//
// PURPOSE:		Default constructor
//************************************************************************

DiskPowerSupply( ListenManager *pListenManager, int number, int numInEvc );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new DiskPowerSupply( GetListenManager(), m_number, m_numberInEvcRow ); }


//************************************************************************
// ~DiskPowerSupply:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~DiskPowerSupply();


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

virtual bool BuildYourselfFromPtsRow( void *pRow_ );


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


private:

//************************************************************************
//************************************************************************


};

#endif // __DISK_POWER_SUPPLY_H__