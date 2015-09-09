//************************************************************************
// FILE:		BoardDevices.h
//
// PURPOSE:		Defines final board-type classes in the O2K system
//
// CLASSES:		FillerBoard
//				NacBoard
//				HbcBoard
//				SsdBoard
//************************************************************************

#ifndef __BOARD_DEVICES_MOS_H__
#define __BOARD_DEVICES_MOS_H__

#include "Iop.h"
#include "Nac.h"
#include "IOPStatusTable.h"

#ifdef WIN32
#pragma pack(4)
#endif

class FillerBoard : public Board {

public:

//************************************************************************
// FillerBoard:
// 
// PURPOSE:		Default constructor
//************************************************************************

FillerBoard( ListenManager *pListenManager, U32 classType = SSAPI_OBJECT_CLASS_TYPE_FILLER_BOARD )
	:Board( pListenManager,  classType ) {

	m_state			= SSAPI_OBJECT_STATE_GOOD;
	m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new FillerBoard( GetListenManager() ); }


//************************************************************************
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool BuildYourselfFromPtsRow( void *pRow_ ){

	IOPStatusRecord	 *pRow = (IOPStatusRecord *)pRow_;

	m_id			= DesignatorId( pRow->rid, (U16)GetClassType() );
	m_slotNumber	= pRow->Slot;
	SetYourSlotName();

	return true;
}


protected:

//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

virtual void ComposeYourOverallState(){ 
	m_state = SSAPI_OBJECT_STATE_GOOD;
	m_name	= CTS_SSAPI_FILLER_BOARD_NAME;
}


//************************************************************************
// HandleObjectAddedEvent:
//
// PURPOSE:		Called by the manager to inform the object that a new 
//				object has been added to the system. The object may be
//				interested to know if this new object is its child/parent
//				and update its vectors as needed.
//************************************************************************

virtual void HandleObjectAddedEvent( ValueSet* /* pObj  */, bool postEvent = true){}

};




class HbcFillerBoard : public FillerBoard {

public:

//************************************************************************
// FillerBoard:
// 
// PURPOSE:		Default constructor
//************************************************************************

HbcFillerBoard( ListenManager *pListenManager )
	:FillerBoard( pListenManager, SSAPI_OBJECT_CLASS_TYPE_HBC_FILLER_BOARD ) {

}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new HbcFillerBoard( GetListenManager() ); }


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

bool BuildYourselfFromPtsRow( void *pRow_ ){

	return FillerBoard::BuildYourselfFromPtsRow(pRow_);
}


};




//************************************************************************
//
//						C L A S S	S S D
//
//************************************************************************


class Ssd : public Iop {

public:

//************************************************************************
// Ssd:
//************************************************************************

Ssd( ListenManager *pListenManager ) : Iop( pListenManager, SSAPI_OBJECT_CLASS_TYPE_SSD_BOARD ){
}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new Ssd( GetListenManager() ); }


//************************************************************************
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


protected:


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		CAssigns the name and state (via Iop class)
//************************************************************************

virtual void ComposeYourOverallState(){
	Iop::ComposeYourOverallState();
	m_name	= CTS_SSAPI_SSD_NAME;
}

};



//************************************************************************
//
//						C L A S S	H B C
//
//************************************************************************


class Hbc : public Iop {

public:

//************************************************************************
// Hbc:
//************************************************************************

Hbc( ListenManager *pListenManager ) : Iop( pListenManager, SSAPI_OBJECT_CLASS_TYPE_HBC_BOARD ){
}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new Hbc( GetListenManager() ); }


//************************************************************************
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


protected:


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		CAssigns the name and state (via Iop class)
//************************************************************************

virtual void ComposeYourOverallState(){
	Iop::ComposeYourOverallState();
	m_name	= CTS_SSAPI_HBC_NAME;
}


};

#endif	// __BOARD_DEVICES_MOS_H__
