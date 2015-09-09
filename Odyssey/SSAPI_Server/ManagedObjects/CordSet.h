//************************************************************************
// FILE:		CordSet.h
//
// PURPOSE:		Defines the object used to represent battery cord sets 
//				inside the Gemini
//************************************************************************

#ifndef __CORD_SET_H__
#define	__CORD_SET_H__

#include "Device.h"


class CordSet : public Device {
	
	U32			m_cordSetNumber;	// internal 
	bool		m_isStatusGood;		// internal

public:

//************************************************************************
// CordSet:
//
// PURPOSE:		Default constructor
//************************************************************************

CordSet( ListenManager *pListenManager, U32 cordSetNumber );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new CordSet( GetListenManager(), m_cordSetNumber ); }


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

virtual void HandleObjectAddedEvent( ValueSet *, bool postEvent = true ){}


//************************************************************************
// IsYourDevice:
//
// PURPOSE:		Determines if a gven device belongs to this device
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

virtual bool IsYourDevice( Device &device ){ return false; }


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before an object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

virtual void ComposeYourOverallState();


//************************************************************************
// operator=:
//
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet *)this) = obj; }


};

#endif // __CORD_SET_H__
