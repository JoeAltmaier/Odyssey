//************************************************************************
// FILE:		SlotMap.h
//
// PURPOSE:		The file defines a class that will be used to get mappings
//				between bus segments, iop slot numbers, and iop slot
//				names. This really smells like a hack...but we all want to
//				release the product, so I tried to abstract it in a separate 
//				object so that it will be easier to fix/expand it later. (AG)
//************************************************************************

#ifndef __SLOT_MAP_H__
#define	__SLOT_MAP_H__

#include "SSAPITypes.h"
#include "CtTypes.h"

#ifdef WIN32
#pragma pack(4)
#endif

//************************************************************************
// struct SlotNumber2SlotNameMapCell
//
// PURPOSE:		
//************************************************************************

struct SlotNumber2SlotNameMapCell{
	U32					slotNumber;
	LocalizedString		slotName;
	U32					segmentNumber;
};


class SlotMap {

public:

//************************************************************************
// SlotMap:		
// 
// PURPOSE:		The constructor
//************************************************************************

SlotMap(){ }


//************************************************************************
// GetSlotNumber:
//
// PURPOSE:		Performs the look up of the slot # by the slot name
//************************************************************************

U32 GetSlotNumber( LocalizedString slotName );


//************************************************************************
// GetSlotName:
//
// PURPOSE:		Performs look up of the slot name by the slot #
//************************************************************************

LocalizedString GetSlotName( U32 slotNumber );


//************************************************************************
// GetSegmentNumber
//
// PURPOSE:		Performs look up of the segment # by an iop slot name/number
//************************************************************************

U32 GetSegmentNumberBySlotName( LocalizedString slotName );
U32 GetSegmentNumberBySlotNumber( U32 slotNumber );


//************************************************************************
// GetMin / Max slotNumber:
//
// PURPOSE:		Gives info about slot numbers in general
//************************************************************************

static U32 GetMinSlotNumber() ;
static U32 GetMaxSlotNumber() ;
static U32 GetNumberOfSlots();


};

#endif // __SLOT_MAP_H__