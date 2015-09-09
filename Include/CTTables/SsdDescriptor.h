/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdDescriptor.h
// 
// Description:
// This is the SSD Object Descriptor Table
// 
// $Log: /Gemini/Include/CTTables/SsdDescriptor.h $
// 
// 7     11/22/99 10:44a Jfrandeen
// Use new FlashStorage in place of FlashFile
// 
// 6     10/25/99 4:22p Hdo
// Change from the IopState to the rowID type for the ridIopStatus
// 
// 5     10/15/99 12:22p Agusev
// 
// 4     10/14/99 4:53p Hdo
// Add vdnBSADdm, SerialNumber, IopStatus, vdnMonitor
//
// 07/27/99 Jim Frandeen: Create file
/*************************************************************************/

#ifndef _SsdDescriptor_h
#define _SsdDescriptor_h

//#include "IopStatusTable.h"
#include "Cache.h" 
#include "FlashStorage.h" 
#include "PTSCommon.h"

// Field definitions in DiskDescriptor.cpp
extern	fieldDef	SSD_descriptor_table_field_defs[];
extern	U32			cb_SSD_descriptor_table_field_defs;

#pragma	pack(4)

#define SSD_DESCRIPTOR_TABLE "SSD_Descriptor_Table"
#define	SSD_DESC_VERSION	2

#define FIELD_CAPACITY "Capacity"

// Number of page table entries, assuming 20 bits for virtual page address
#define SSD_NUM_VIRTUAL_PAGES 1048576

typedef struct _SSD_Descriptor {
	rowID			rid;					// rowID of this record.
	U32 			version;				// Version of Storage Roll Call record.
	U32				size;					// Size of SSD Descriptor record in bytes.
	I64				Capacity;				// Capacity in blocks (=512 bytes)
	VDN				vdnBSADdm;				// BSA Virtual Device number for this ID
	String32		SerialNumber;			// Device serial number	
	rowID			ridIopStatus;			// rowID of the card in IopStatusTable
	VDN				vdnMonitor;				// Virtual Device number of the SSD monitor
	U32				memory_size;
	U32				callback_memory_size;
	CM_CONFIG		cache_config;
	FF_CONFIG		flash_config;
} SSD_Descriptor;
	
#endif // _SsdDescriptor_h
