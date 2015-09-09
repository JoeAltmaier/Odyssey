/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfPageMapEntry.h
// 
// Description:
// This file defines the page map entry. 
// 
// 7/20/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FbPageMapEntry_H)
#define FbPageMapEntry_H

#include "Simple.h"

#pragma pack(1)

/*************************************************************************/
// Virtual to Real Map entry
// The virtual to real page map is an array with one entry per page. 
// Each entry contains the real page number that is mapped by the
// virtual page number.
// The virtual to real page map is indexed by the virtual flash address.
// The virtual to real page map is a not a persistent structure.  
// It is created from the Virtual to Real page map.
/*************************************************************************/
typedef U32 FF_VIRTUAL_TO_REAL_MAP_ENTRY;

// How many bytes in a page map entry? 4
#define FF_BYTES_PER_PAGE_MAP_ENTRY sizeof(FF_VIRTUAL_TO_REAL_MAP_ENTRY)

/*************************************************************************/
// Real to Virtual Map entry
// The real to virtual page map is used to map real addresses to virtual 
// addresses.  The real to virtual page map
// is an array with one entry per page. The real page map is 
// indexed by the real page number.   
// The real to virtual map is created from the virtual to real map.     
/*************************************************************************/

// A real to virtual map entry looks like the following:
/*
31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
 E  B  S  S  S  S  S  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  
 
*/
// V = Virtual page number
// U = Unused
// S = page State
// B = Bad Page
// E = Erased

// Values of page state
// Note that we have a page state erased and an erased flag.
// A page is in the erased state when it is assigned to a virtual
// address in the range of unassigned erased pages.
// Note that we have a bad page state and a bad page flag.
// The flag is set by the I/O routine such as erase when the
// error is detected for that sector.
// Later, the page state will be changed to bad page.
typedef enum {
	FF_PAGE_STATE_UNMAPPED,
	FF_PAGE_STATE_MAPPED,
	FF_PAGE_STATE_DELETED,
	FF_PAGE_STATE_ERASED,
	FF_PAGE_STATE_REPLACEMENT,
	FF_PAGE_STATE_PAGE_MAP,
	FF_PAGE_STATE_PAGE_MAP_TABLE,
	FF_PAGE_STATE_BAT,
	FF_PAGE_STATE_BAD_BAT,
	FF_PAGE_STATE_TOC,
	FF_PAGE_STATE_BAD_BLOCK_TABLE,
	FF_PAGE_STATE_BAD_PAGE,
	FF_PAGE_STATE_BAD_BLOCK,
	FF_PAGE_STATE_ERASING
 } FF_PAGE_STATE;
 
// We only need 5 bits, but make it 6 so that state will be in high order byte
// and address will be in low order 3 bytes when looking at memory in the debugger.
#define FF_NUM_BITS_FOR_PAGE_STATE		 6

typedef struct {
	BF	erased:					1;
	BF	bad_page:				1;
	BF	page_state:				FF_NUM_BITS_FOR_PAGE_STATE;
	BF	virtual_address:		32 - FF_NUM_BITS_FOR_PAGE_STATE - 2;
} FF_REAL_TO_VIRTUAL_MAP_ENTRY;

// Default values in case user does not specify in the format command
#define DEFAULT_PERCENTAGE_ERASED_PAGES 5
#define DEFAULT_PERCENTAGE_REPLACEMENT_PAGES 5
#define DEFAULT_REPLACEMENT_PAGE_THRESHOLD 5
#define DEFAULT_WEAR_LEVEL_THRESHOLD 25

#endif // FbPageMapEntry_H
