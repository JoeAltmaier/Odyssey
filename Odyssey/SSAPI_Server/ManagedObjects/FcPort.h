//************************************************************************
// FILE:		FcPort.h
//
// PURPOSE:		Defines class GcPort to be used to represent all FC
//				ports in the O2K system
//************************************************************************

#ifndef __FC_PORT_H__
#define	__FC_PORT_H__


#include "Device.h"
#include "ServiceableInterface.h"
#include "LoopDescriptor.h"

#ifdef WIN32
#pragma pack(4)
#endif


class FcPort : public Device, public ServiceableInterface{

	U32					m_portNumber;
	TySlot				m_slotNumber;			// internal, to determine which NAc it belongs to
	U32					m_chipNumber;
	U32					m_flags;
	LocalizedString		m_chipName;
	LoopState			m_portState;			// internal, to determine m_state

public:

//************************************************************************
// FcPort:
//
// PURPOSE:		Default constructor
//************************************************************************

FcPort( ListenManager *pListenManager, U32 classType = SSAPI_OBJECT_CLASS_TYPE_FC_PORT );



//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new FcPort( GetListenManager() ); }


//************************************************************************
// ~FcPort
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~FcPort();


//************************************************************************
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }



//************************************************************************
// SetServiceState:
//
// PURPOSE:		Declares an API method 
//************************************************************************

virtual bool SetServiceState( SsapiResponder *pResponder, U32 newState );


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
// GetSlotNumber:
//
// PURPOSE:		An accessor. Returns the slot number of the IOP this port 
//				is on.
//************************************************************************

U32 GetSlotNumber() const { return (U32)m_slotNumber; }

U32 GetPortNumber() const { return m_portNumber; }

protected:

//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

virtual void ComposeYourOverallState();

};

#endif	// __FC_PORT_H__