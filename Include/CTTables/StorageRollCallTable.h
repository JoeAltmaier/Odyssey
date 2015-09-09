/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This is the Storage Roll Call Table
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/StorageRollCallTable.h $
// 
// 28    12/16/99 1:06a Ewedel
// Added RqPts_T support.
// 
// 27    9/10/99 7:11p Mpanas
// More StorageTypes
// 
// 26    9/05/99 6:32p Mpanas
// Add SES SRCType
// 
// 25    9/01/99 10:38a Egroff
// Added field def and table name, accessor functions to structure
// definitions.
// 
// 24    7/31/99 4:59p Agusev
// 
// 23    7/04/99 8:37p Agusev
// Failed last time. Going twice!
// 
// 22    7/04/99 8:36p Agusev
// Added SRCTypePartition as a storage class type
// 
// 21    5/27/99 6:44p Jlane
// Updated SRC to match Updated Management Table Spec.  Mostly got rid of
// unesed leftovers.
// 
// 20    4/22/99 9:10a Jlane
// Changhed ridThisRow to rid.
// 
// 19    3/23/99 1:01p Rparks
// Moved field string defs from cpp file.
// 
// 18    3/22/99 3:36p Mpanas
// Added SRCTypeRamDisk
// 
// 17    3/17/99 4:04p Rparks
// Added SRCUnclaimed
// 
// 16    3/17/99 3:19p Rparks
// I forgot to say I added SRCRaidSpare.
// 
// 15    3/17/99 3:18p Rparks
// 
// 12    3/09/99 4:31p Mpanas
// Add refs to the field defs for this table
// 
// 9/8/98 Michael G. Panas: Create file
// 9/29/98 Jim Frandeen: Use CtTypes.h instead of stddef.h
// 1/3/98 Jerry Lane:	Rolled in latest StorageRollCall table from the Spec.
// 02/28/99 JFL Added vdnMonitor and changed allocation fields.
/*************************************************************************/

#ifndef __StorageRollCallTable_h
#define __StorageRollCallTable_h

//#include "I2ODep.h" //no I20
#include "CtTypes.h"
#include "Odyssey.h"
#include "PTSCommon.h"

#ifndef __RqPts_T_h
# include  "RqPts_T.h"
#endif

#pragma	pack(4)

// Field Def string constants

#define fdSRC_VERSION			"version"
#define fdSRC_SIZE				"size"
#define fdSRC_CAPACITY			"Capacity"
#define fdSRC_FUSED				"fUsed"
#define fdSRC_STORAGE_CLASS		"storageclass"
#define fdSRC_VDNBSADDM			"vdnBSADdm"
#define fdSRC_DESC_RID			"ridDescriptorRecord"
#define fdSRC_STATUS_RID		"ridStatusRecord"
#define fdSRC_PERFORMANCE_RID	"ridPerformanceRecord"
#define fdSRC_MONITOR_VDN		"vdnMonitor"
#define	fdSRC_NAME_RID			"ridName"



enum SRCStorageTypes {
	SRCTypeUnknown = 0,			// Storage type unknown (still storage)
	SRCTypeFCDisk,				// Internal Odyssey disk
	SRCTypeExternalFCDisk,		// External Storage device
	SRCTypeSSD,					// Odyssey Flash storage card.
	SRCTypeDaisyLUN,			// Daisy Chain discovered LUN
	SRCTypeArray,				// Odyssey managed array.
	SRCTypeRamDisk,				// Odyssey RAM Disk
	SRCTypePartition,			// Odyssey partition of some storage element
	SRCTypeTape,				// Odyssey or External attached Tape
	SRCTypeSES					// SCSI Enclosure Services Device
};


// Field definitions in StorageRollCallTable.cpp
extern	fieldDef	StorageRollCallTable_FieldDefs[];
extern	U32			cbStorageRollCallTable_FieldDefs;

#define STORAGE_ROLL_CALL_TABLE "Storage_Roll_Call_Table"
#define	STORAGE_ROLL_CALL_TABLE_VERSION	1

//
// The Storage Roll Call Table contains one row for each array, disk, SSD or LUN off the HBCs.
//
typedef struct StorageRollCallRecord {		// contains one row for each array, disk, SSD or LUN off the HBCs.

        static const fieldDef *FieldDefs()		{ return StorageRollCallTable_FieldDefs;   }
        static const U32 FieldDefsSize()	{ return cbStorageRollCallTable_FieldDefs;  }
        static const char* const TableName()		{ return STORAGE_ROLL_CALL_TABLE; }

	rowID			rid;					// rowID of this record.
	U32 			version;				// Version of Storage Roll Call record.
	U32				size;					// Size of Storage Roll Call record in bytes.
	I64				Capacity;				// Available capacity of this storage.
	U32				fUsed;					// Is this device free or used?
	SRCStorageTypes	storageclass;			// The class of the storage.
	VDN				vdnBSADdm;				// BSA Virtual Device number for this storage.
	rowID			ridDescriptorRecord;	// rowID of  descriptor Table entry for this storage
	rowID			ridStatusRecord;		// rowID of the status record for this storage.
	rowID			ridPerformanceRecord;	// rowID of the performance record for this storage.
	VDN				vdnMonitor;				// VirtualDevice Number of discovering Monitor.
	rowID			ridName;				// rowID of the name string in of the UnicodeString tables


    //  some PTS interface message typedefs
    typedef RqPtsDefineTable_T<StorageRollCallRecord> RqDefineTable;
    typedef RqPtsDeleteTable_T<StorageRollCallRecord> RqDeleteTable;
    typedef RqPtsQuerySetRID_T<StorageRollCallRecord> RqQuerySetRID;
    typedef RqPtsEnumerateTable_T<StorageRollCallRecord> RqEnumerateTable;
    typedef RqPtsInsertRow_T<StorageRollCallRecord> RqInsertRow;
    typedef RqPtsModifyRow_T<StorageRollCallRecord> RqModifyRow;
    typedef RqPtsModifyField_T<StorageRollCallRecord> RqModifyField;
    typedef RqPtsModifyBits_T<StorageRollCallRecord> RqModifyBits;
    typedef RqPtsTestAndSetField_T<StorageRollCallRecord> RqTestAndSetField;
    typedef RqPtsReadRow_T<StorageRollCallRecord> RqReadRow;
    typedef RqPtsDeleteRow_T<StorageRollCallRecord> RqDeleteRow;
    typedef RqPtsListen_T<StorageRollCallRecord> RqListen;

} StorageRollCallRecord;

//, *pStorageRollCallRecord, StorageRollCallTable[];

// Field definitions in StorageRollCallTable.cpp
extern	fieldDef	StoragePartitionTable_FieldDefs[];
extern	U32			cbStoragePartitionTable_FieldDefs;

#define STORAGE_PARTITION_TABLE "Storage_Partition_Table"
#define	STORAGE_PARTITION_TABLE_VERSION	1


//
// The Partition Table describes the virtual circuits that exist on each storage roll call entry.
//
typedef struct StoragePartitionRecord {		// contains one row for each partition?

        static const fieldDef *FieldDefs()		{ return StoragePartitionTable_FieldDefs;   }
        static const U32 FieldDefsSize()	{ return cbStoragePartitionTable_FieldDefs;  }
        static const char* const TableName()		{ return STORAGE_PARTITION_TABLE; }

	rowID	ridThisRow;		// row ID of this row.
	U32 	version;		// Version of Partition Table record.
	U32		size;			// Size of Partition Table record in bytes.
	I64		Offset;			// Partition Offset (in bytes?)
	I64		Capacity;		// Partition Capacity (in bytes?)
	rowID	ProductOwner;	// Product using this partition. 
	rowID	CircuitOwner;	// Virtual Circuit using this partition. 
} StoragePartitionRecord, StoragePartitionTable[];

#endif