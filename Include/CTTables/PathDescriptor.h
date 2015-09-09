/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: PathDescriptor.h
// 
// Description:
// This is the Storage Path Descriptor Table.  May have multiple path
// entries for a unit of storage.
// 
// Update Log:
// $Log: /Gemini/Include/CTTables/PathDescriptor.h $
// 
// 2     12/15/99 2:00p Mpanas
// Add in the ridActiveDesc.  This will point to the Descriptor when
// this path is active
// 
// 1     12/10/99 9:45p Mpanas
// New files to support Failover the correct way
// - PathDescriptor.h  .cpp
// - DeviceDescriptor.h  .cpp
// 
// 
// 12/07/98 Michael G. Panas: Create file
/*************************************************************************/

#ifndef _PathDescriptor_h
#define _PathDescriptor_h

#include "Scsi.h"
#include "CtTypes.h"
#include "Odyssey.h"
#include "PTSCommon.h"

// Field definitions in PathDescriptor.cpp
extern	fieldDef	PathDescriptorTable_FieldDefs[];
extern	U32			cbPathDescriptorTable_FieldDefs;

#pragma	pack(4)

#define PATH_DESC_TABLE "Path_Descriptor_Table"
#define	PATH_DESC_VERSION	1

typedef struct PathDescriptor {

        static const fieldDef *FieldDefs()		{ return PathDescriptorTable_FieldDefs;   }
        static const U32 FieldDefsSize()	{ return cbPathDescriptorTable_FieldDefs;  }
        static const char* const TableName()		{ return PATH_DESC_TABLE; }

	rowID			rid;					// rowID of this table row.
	U32 			version;				// Version of Path descriptor record.
	U32				size;					// Size of Path descriptor record in bytes.
	U32				FCInstance;				// FC Loop number (index in LoopDescriptor table)
	U32				FCTargetID;				// actual FCP target ID.
	U32				FCTargetLUN;			// actual FCP target LUN.
	VDN				vdnDdm;					// BSA/ES Virtual Device number for this device
	U32				CurrentStatus;			// see DriveStatus enum in Odyssey.h
	U32				InqType;				// so we can find the correct descriptor
	VDN				vdnMonitor;				// Virtual Device number of the discovering monitor
	rowID			ridDescriptor;			// rowID of the descriptor for this device
	rowID			ridActiveDesc;			// rowID of the descriptor when path active
	rowID			ridVendor;				// rowID of the Vendor Enum for this device
	} PathDescriptorRecord, *pPathDescriptorRecord;
	
#endif