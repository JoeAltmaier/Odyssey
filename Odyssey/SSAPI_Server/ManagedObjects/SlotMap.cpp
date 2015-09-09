//************************************************************************
// FILE:		SlotMap.cpp
//
// PURPOSE:		The file implements a class that will be used to get mappings
//				between bus segments, iop slot numbers, and iop slot
//				names. 
//************************************************************************


#include "SlotMap.h"
#include "CtEvent.h"
#include "SSAPIAssert.h"

static const SlotNumber2SlotNameMapCell	SLOT_MAP[] = {
				{ 24, CTS_SSAPI_SLOT_NAME_A4, 0 },		// Bus Segment A
				{ 27, CTS_SSAPI_SLOT_NAME_A3, 0 },
				{ 26, CTS_SSAPI_SLOT_NAME_A2, 0 },
				{ 25, CTS_SSAPI_SLOT_NAME_A1, 0 },

				{ 16, CTS_SSAPI_SLOT_NAME_B1, 1 },		// Bus Segment B
				{ 17, CTS_SSAPI_SLOT_NAME_B2, 1 },
				{ 18, CTS_SSAPI_SLOT_NAME_B3, 1 },
				{ 19, CTS_SSAPI_SLOT_NAME_B4, 1 },

				{ 29, CTS_SSAPI_SLOT_NAME_C1, 2 },		// Bus Segment C
				{ 30, CTS_SSAPI_SLOT_NAME_C2, 2 },
				{ 31, CTS_SSAPI_SLOT_NAME_C3, 2 },
				{ 28, CTS_SSAPI_SLOT_NAME_C4, 2 },

				{ 20, CTS_SSAPI_SLOT_NAME_D1, 3 },		// Bus Segment D
				{ 21, CTS_SSAPI_SLOT_NAME_D2, 3 },
				{ 22, CTS_SSAPI_SLOT_NAME_D3, 3 },
				{ 23, CTS_SSAPI_SLOT_NAME_D4, 3 },

				{ 0, CTS_SSAPI_HBC_SLOT_NAME_00, -1 },	// HBC 0
				{ 1, CTS_SSAPI_HBC_SLOT_NAME_01, -1 },	// HBC 1
};

static const U16 SLOT_MAP_SIZE = sizeof(SLOT_MAP) / sizeof(SLOT_MAP[0]);


//************************************************************************
// GetSlotNumber:
//
// PURPOSE:		Performs the look up of the slot # by the slot name
//************************************************************************

U32 
SlotMap::GetSlotNumber( LocalizedString slotName ){

	U16			index;

	for( index = 0; index < SLOT_MAP_SIZE; index++ )
		if( SLOT_MAP[ index ].slotName == slotName )
			return SLOT_MAP[ index ].slotNumber;

	ASSERT(0);
	return -1;
}


//************************************************************************
// GetSlotName:
//
// PURPOSE:		Performs look up of the slot name by the slot #
//************************************************************************

LocalizedString
SlotMap::GetSlotName( U32 slotNumber ){

	U16			index;

	for( index = 0; index < SLOT_MAP_SIZE; index++ ){
		if( SLOT_MAP[ index ].slotNumber == slotNumber )
			return SLOT_MAP[ index ].slotName;
	}

	ASSERT(0);
	return -1;
}


//************************************************************************
// GetSegmentNumber
//
// PURPOSE:		Performs look up of the segment # by an iop slot name/number
//************************************************************************

U32 
SlotMap::GetSegmentNumberBySlotName( LocalizedString slotName ){
	
	U16			index;

	for( index = 0; index < SLOT_MAP_SIZE; index++ )
		if( SLOT_MAP[ index ].slotName == slotName )
			return SLOT_MAP[ index ].segmentNumber;

	ASSERT(0);
	return -1;
}


U32
SlotMap::GetSegmentNumberBySlotNumber( U32 slotNumber ){

	U16			index;

	for( index = 0; index < SLOT_MAP_SIZE; index++ )
		if( SLOT_MAP[ index ].slotNumber == slotNumber )
			return SLOT_MAP[ index ].segmentNumber;

	ASSERT(0);
	return -1;
}


//************************************************************************
// GetMin / Max slotNumber:
//
// PURPOSE:		Gives info about slot numbers in general
//************************************************************************

U32 
SlotMap::GetMinSlotNumber() {

	U16			index;
	U32			min = 0xFFFFFFFF;

	for( index = 0; index < SLOT_MAP_SIZE; index++ ){
		if( SLOT_MAP[ index ].slotNumber < min )
			min = SLOT_MAP[ index ].slotNumber;
	}

	return min;
}


U32 
SlotMap::GetMaxSlotNumber() {

	U16			index;
	U32			max = 0;

	for( index = 0; index < SLOT_MAP_SIZE; index++ ){
		if( SLOT_MAP[ index ].slotNumber > max )
			max = SLOT_MAP[ index ].slotNumber;
	}

	return max;
}


U32 
SlotMap::GetNumberOfSlots(){

	return (U32)SLOT_MAP_SIZE;
}
