/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdContext.h
// 
// Description:
// This file describes the context structure for the
// Solid State Drive. 
// 
// Update Log 
// 
// 02/26/99 Jim Frandeen: Create file
/*************************************************************************/

#ifndef Ssd_Context_H
#define Ssd_Context_H

#include "BuildSys.h" 
#include "Cache.h" 
#include "FlashStorage.h" 
#include "CtTypes.h" 

// MEMORY_FOR_SSD includes all memory used.
#define MEMORY_FOR_SSD						16000000				// 16 megabytes
#define MEMORY_FOR_CALLBACKS				100000					// 100K

// Page size used by flash file system and page manager
#define SSD_PAGE_SIZE 4096

// Number of page table entries, assuming 20 bits for virtual page address
#define SSD_NUM_VIRTUAL_PAGES 1048576

typedef struct _SSD_Context {
	U32	version;
	U32	size;
	U32 num_devices;
	VDN	vd;
} SSD_Context;

// Currently, no data is used in the context record.
// All configuration information comes from the SSD_DESCRIPTOR_TABLE
// defined in SsdDescriptor.h


#endif // Ssd_Context_H
