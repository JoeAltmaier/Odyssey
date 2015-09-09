/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: IOPSlotRecord.h
// 
// Description:
// Provides a place to sotre all virtual device and config data needed to
// startup an IOP in a non-volatile way.  This record should be used for
// NAC, and SSD type IOPs.
// 
// Update Log:
// $Log: /Gemini/Include/CTTables/IOPSlotRecord.h $
// 
// 1     1/13/00 5:17p Mpanas
// First cut of the IOP Slot Record
// - used to start all the base virtual devices
//   needed for a given slot
// - supports NAC and SSD
// 
// 
// 01/13/98 Michael G. Panas: Create file
/*************************************************************************/

#ifndef _IOPSlotRecord_h
#define _IOPSlotRecord_h

#include "CtTypes.h"
#include "Odyssey.h"
#include "PTSCommon.h"

// Field definitions in IOPSlotRecord.cpp
extern	fieldDef	IOPSlotRecordTable_FieldDefs[];
extern	U32			cbIOPSlotRecordTable_FieldDefs;

#pragma	pack(4)

// NAC FC Instance data
// One defined for each Fibre Channel Chip
struct FC_Instance_Data {
	U32				Type;					// IO_Device type
	VDN				IO_Device;				// Target, Initiator or both
	rowID			IO_Dev_Config;			// Config record for this DDM
	U32				flags;					// (filler since rowIDs are 8 bytes?)
	VDN				DriveMonitor;			// only used on Initiator loops (0 otherwise)
	rowID			Drive_Mon_Config;		// if needed
};


#define IOP_SLOTRECORD_TABLE "IOP_Slot_Record_Table"
#define	IOP_SLOTRECORD_VERSION	1

struct IOPSlotRecord {

        static const fieldDef *FieldDefs()		{ return IOPSlotRecordTable_FieldDefs;   }
        static const U32 FieldDefsSize()	{ return cbIOPSlotRecordTable_FieldDefs;  }
        static const char* const TableName()		{ return IOP_SLOTRECORD_TABLE; }

	rowID				rid;					// rowID of this table row.
	U32 				version;				// Version of this record.
	U32					size;					// Size of this record in bytes.
	
	U32					Type;					// Type of IOP
	VDN					vdnDdm;					// Virtual Device number for LoopMonitor or SSD DDM
	rowID				ridDdm;					// rowID of the coonfig record for LoopMon or SSD
	
	// the rest of the fields are used for NACs only
	FC_Instance_Data	fc0;					// three chips for now
	FC_Instance_Data	fc1;					// 
	FC_Instance_Data	fc2;					// 
	U32					CacheBlockSize;			// Size of the FCP Library Cache
	U32					nCacheBlocksPrimary;	// Number of blocks in the Primary (IO) Cache
	U32					nCacheBlocksSecodary;	// Number of blocks in the Secondary (IO) Cache
};
	
#endif