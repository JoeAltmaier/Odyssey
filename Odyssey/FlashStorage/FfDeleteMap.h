/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfDeleteMap.h
// 
// Description:
// This file defines the interfaces to page delete operations 
// of the Flash Storage system. 
// The deleted page table is used to keep track of deleted pages by
// block. This table is indexed by the virtual block number.
// Each entry contains 32 bits to indicate which pages in the block
// can be erased.  Each time a page is written, the old page can be
// deleted.   However, pages can only be erased a block at a time. 
//
// 8/17/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(FbDeleteMap_H)
#define FbDeleteMap_H

#include "FfCommon.h"
#include "List.h"

/*************************************************************************/
// Deleted Address Map
// The deleted page map is a bit map that is used to keep track of 
// deleted pages by block. This table is indexed by the virtual
// block number.  Each entry contains 32 bits to indicate which
// pages in the block have been deleted.  Each time a page is written,
// the old page can be erased.   However, pages can only be erased a page
// block at a time. 
// There is one FF_DELETED_MAP_ENTRY entry per block in the system.
// There are currently 1024 blocks per device for both the 8 megabyte device
// and the 16 megabyte device. With a maximum of 64 devices, that gives us
// 64K entries.  
/*************************************************************************/

typedef struct {

	// All FF_DELETED_MAP_ENTRY with the same number of deleted 
	// entries are on the same list.
	LIST	list;

	// bit_map indicates which sectors in the block have been deleted
	// N.B. We are depending on max 32 sectors per page here.
	U32		 bit_map;
	
	// deleted_count indicates how many sectors in the block 
	// have been deleted
	U32		deleted_count;
	
} FF_DELETED_MAP_ENTRY;


const unsigned NUM_DELETED_LISTS = 33;



#endif /* FbDeleteMap_H  */
