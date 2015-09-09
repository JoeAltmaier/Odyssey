//************************************************************************
// FILE:		Board.cpp
//
// PURPOSE:		Implements an abstract class Board that will serve as a base
//				class for all board-type devices in the O2K
//************************************************************************


#include "Board.h"
#include "SlotMap.h"
#include "SsapiResponder.h"
#include "DeviceManager.h"

//************************************************************************
// Board:
//
// PURPOSE:		The default constructor
//************************************************************************

Board::Board( ListenManager *pListenManager, U32 objectClassType )
:Device( pListenManager, objectClassType ){

}


//************************************************************************
// ~Board:
//
// PURPOSE:		The destructor
//************************************************************************

Board::~Board(){
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
Board::BuildYourValueSet(){

	Device::BuildYourValueSet();

	AddInt(m_slotNumber, SSAPI_BOARD_FID_SLOT );
	AddInt(m_slotName, SSAPI_BOARD_FID_SLOT_NAME );

	int i = m_isLocked? 1 : 0;
	AddInt(m_isLocked, SSAPI_OBJECT_FID_IS_LOCKED );

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
Board::BuildYourselfFromYourValueSet(){

	Device::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_BOARD_FID_SLOT, &m_slotNumber );
	GetInt( SSAPI_BOARD_FID_SLOT_NAME, (int *)&m_slotName );

	int i;
	GetInt( SSAPI_OBJECT_FID_IS_LOCKED, &i );
	m_isLocked = i? true : false;

	return true;
}


//************************************************************************
// Lock:
//
// PURPOSE:		Declares an API method 
//************************************************************************

bool 
Board::Lock( SsapiResponder *pResponder ){

	((DeviceManager *)GetManager())->ChangeBoardLockState( (TySlot)m_slotNumber, true, m_id, pResponder );
	return true;
}


//************************************************************************
// UnLock:
//
// PURPOSE:		Declares an API method
//************************************************************************

bool 
Board::UnLock( SsapiResponder *pResponder ){

	((DeviceManager *)GetManager())->ChangeBoardLockState( (TySlot)m_slotNumber, false, m_id, pResponder );
	return true;
}


//************************************************************************
// SetYourSlotName
//
// PURPOSE:		The methods finds the appropriate slotName (LocalizedString)
//				by the m_slotNumber data member and sets m_slotName
//************************************************************************

void 
Board::SetYourSlotName(){
	SlotMap		map;

	m_slotName = map.GetSlotName( m_slotNumber );
	m_location = m_slotNumber;
}


