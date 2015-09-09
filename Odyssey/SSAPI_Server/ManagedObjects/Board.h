//************************************************************************
// FILE:		Board.h
//
// PURPOSE:		Defines an abstract class Board that will serve as a base
//				class for all board-type devices in the O2K
//************************************************************************

#ifndef __BOARD_MO_H__
#define __BOARD_MO_H__

#include "Device.h"
#include "LockableInterface.h"
#include "..\SSAPITypes.h"

#ifdef WIN32
#pragma pack(4)
#endif

class Board : public Device, public LockableInterface{

protected:

	int					m_slotNumber;
	LocalizedString		m_slotName;

	friend class DeviceManager;
//************************************************************************
// Board:
//
// PURPOSE:		The default constructor
//************************************************************************

Board( ListenManager *pListenManager, U32 objectClassType );



public:


//************************************************************************
// ~Board:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~Board();


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
// GetSlotNumber
//
// PURPOSE:		An accessor. Returns the slot number of this board
//************************************************************************

int GetSlotNumber() const { return m_slotNumber; }


protected:


//************************************************************************
// SetYourSlotName
//
// PURPOSE:		The methods finds the appropriate slotName (LocalizedString)
//				by the m_slotNumber data member and sets m_slotName
//************************************************************************

void SetYourSlotName();

};


#endif // __BOARD_MO_H__