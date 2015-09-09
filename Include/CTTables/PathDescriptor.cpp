/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: PathDescriptor.cpp
// 
// Description:
// This module is the Table definition of the PathDescriptor Table
//
//	Note:  There should only be one of these for each
//         loaded image!
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/PathDescriptor.cpp $
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
// 12/07/99 Michael G. Panas: Create file
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"


	fieldDef	PathDescriptorTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									8,	ROWID_FT, Persistant_PT,
		"version",									4,	U32_FT, Persistant_PT,
		"size",										4,	U32_FT, Persistant_PT,
		"FCInstance",								4,	U32_FT, Persistant_PT,
		"FCTargetID",								4,	U32_FT, Persistant_PT,
		"FCTargetLUN",								4,	U32_FT, Persistant_PT,
		"vdnDdm",									4,	U32_FT, Persistant_PT,
		"CurrentStatus",							4,	U32_FT, Persistant_PT,
		"InqType",									4,	U32_FT, Persistant_PT,
		"vdnMonitor",								4,	U32_FT, Persistant_PT,
		"ridDescriptor",							8,	ROWID_FT, Persistant_PT,
		"ridActiveDesc",							8,	ROWID_FT, Persistant_PT,
		"ridVendor",								8,	ROWID_FT, Persistant_PT
	};

// defined here so other folks can get to it	
U32			cbPathDescriptorTable_FieldDefs = 
				sizeof(PathDescriptorTable_FieldDefs);