/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DeviceDescriptor.cpp
// 
// Description:
// This module is the Table definition of the DeviceDescriptor Table
//
//	Note:  There should only be one of these for each
//         loaded image!
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/DeviceDescriptor.cpp $
// 
// 2     1/11/00 5:26p Agusev
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


	fieldDef	DeviceDescriptorTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rid",										8,	ROWID_FT, Persistant_PT,
		"version",									4,	U32_FT, Persistant_PT,
		"size",										4,	U32_FT, Persistant_PT,
		"fUsed",									4,	U32_FT,	Persistant_PT,
		"SlotID",									4,	U32_FT, Persistant_PT,
		"fSNValid",									4,	U32_FT, Persistant_PT,
		"SerialNumber",								32,	STRING32_FT, Persistant_PT,
		"WWNName",					  				16,	STRING16_FT, Persistant_PT,
		"CurrentStatus",							4,	U32_FT, Persistant_PT,
		"Type",										4,	U32_FT, Persistant_PT,
		"InqData",					  				56,	BINARY_FT, Persistant_PT,
		"ridVendor",								8,	ROWID_FT, Persistant_PT,
		"ridName",									8,	ROWID_FT, Persistant_PT
	};

// defined here so other folks can get to it	
U32			cbDeviceDescriptorTable_FieldDefs = 
				sizeof(DeviceDescriptorTable_FieldDefs);