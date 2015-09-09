/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: StorageRollCallTable.cpp
// 
// Description:
// This module is the Table definition of the Storage Roll Call Table
// and the Storage Partition table
//
//	Note:  There should only be one of these for each
//         loaded image!
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/StorageRollCallTable.cpp $
// 
// 6     7/31/99 4:59p Agusev
// 
// 5     5/27/99 6:44p Jlane
// Updated SRC to match Updated Management Table Spec.  Mostly got rid of
// unesed leftovers.
// 
// 4     3/24/99 7:41p Jtaylor
// Fixed Ron's F#$%-up.  #include StorageRollCallTable.h and added missing
// comma after Scan State.
// 
// 3     3/24/99 6:10p Rparks
// Moved string defs to header file where they belong.
// 
// 2     3/17/99 6:03p Rparks
// Defined constants for strings.
// 
// 1     3/09/99 4:34p Mpanas
// Initial Checkin
// This module has the Table definition of the Storage Roll Call Table
// and the Storage Partition table
// 
// 
// 03/09/99 Michael G. Panas: Create file
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"
#include "StorageRollCallTable.h"



	fieldDef	StorageRollCallTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
		fdSRC_VERSION,					4,	U32_FT, Persistant_PT,
		fdSRC_SIZE,						4,	U32_FT, Persistant_PT,
		fdSRC_CAPACITY,					8,	U64_FT, Persistant_PT,
		fdSRC_FUSED,					4,	U32_FT, Persistant_PT,
		fdSRC_STORAGE_CLASS,		 	4,	U32_FT, Persistant_PT,
		fdSRC_VDNBSADDM,		 		4,	U32_FT, Persistant_PT,
		fdSRC_DESC_RID,		 			8,	ROWID_FT, Persistant_PT,
		fdSRC_STATUS_RID,	 	 		8,	ROWID_FT, Persistant_PT,
		fdSRC_PERFORMANCE_RID,			8,	ROWID_FT, Persistant_PT,
		fdSRC_MONITOR_VDN,				4,	U32_FT, Persistant_PT,
		fdSRC_NAME_RID,					8,	ROWID_FT, Persistant_PT
	};

// defined here so other folks can get to it	
U32 cbStorageRollCallTable_FieldDefs = sizeof(StorageRollCallTable_FieldDefs);
				

#define fdSRC_OFFSET			"Offset"
#define fdSRC_PRODUCT_OWNER	"ProductOwner"
#define fdSRC_CIRCUIT_OWNER	"CircuitOwner"



	fieldDef	StoragePartitionTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
		fdSRC_VERSION,					4,	U32_FT, Persistant_PT,
		fdSRC_SIZE,						4,	U32_FT, Persistant_PT,
		fdSRC_OFFSET,	   				8,	U64_FT, Persistant_PT,
		fdSRC_CAPACITY,	   				8,	U64_FT, Persistant_PT,
		fdSRC_PRODUCT_OWNER,  			8,	ROWID_FT, Persistant_PT,
		fdSRC_CIRCUIT_OWNER,			8,	ROWID_FT, Persistant_PT
	};


// defined here so other folks can get to it	
U32 cbStoragePartitionTable_FieldDefs = sizeof(StoragePartitionTable_FieldDefs);
