/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: IOPSlotRecord.cpp
// 
// Description:
// This module is the Table definition of the IOPSlotRecord Table
//
//	Note:  There should only be one of these for each
//         loaded image!
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/IOPSlotRecord.cpp $
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

#include "OsTypes.h"
#include "PTSCommon.h"


	fieldDef	IOPSlotRecordTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rid",										8,	ROWID_FT, Persistant_PT,
		"version",									4,	U32_FT, Persistant_PT,
		"size",										4,	U32_FT, Persistant_PT,
		"Type",										4,	U32_FT, Persistant_PT,
		"vdnDdm",									4,	U32_FT, Persistant_PT,
		"ridDdm",									8,	ROWID_FT, Persistant_PT,
		"fc0",										32,	BINARY_FT, Persistant_PT,
		"fc1",										32,	BINARY_FT, Persistant_PT,
		"fc2",										32,	BINARY_FT, Persistant_PT,
		"CacheBlockSize",							4,	U32_FT, Persistant_PT,
		"nCacheBlocksPrimary",						4,	U32_FT, Persistant_PT,
		"nCacheBlocksSecodary",						4,	U32_FT, Persistant_PT
	};

// defined here so other folks can get to it	
U32			cbIOPSlotRecordTable_FieldDefs = 
				sizeof(IOPSlotRecordTable_FieldDefs);