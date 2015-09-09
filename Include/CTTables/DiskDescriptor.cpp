/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DiskDescriptor.cpp
// 
// Description:
// This module is the Table definition of the DiskDescriptor Table
//
//	Note:  There should only be one of these for each
//         loaded image!
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/DiskDescriptor.cpp $
// 
// 8     1/11/00 7:21p Mpanas
// Support for new PathDescriptor table
// 
// 7     12/16/99 1:06a Ewedel
// Added RqPts_T support, and CHECKFIELDDEFS().
// 
// 6     9/13/99 1:32p Mpanas
// Add CTDiskType field to specify int, ext, FC and SCSI
// Hey, some folks may need to know these things...
// 
// 5     9/07/99 9:20a Cwohlforth
// Removed constant max index from fieldDef array.
// 
// 4     9/05/99 6:45p Mpanas
// Add LUN and LockState fields
// 
// 3     6/07/99 10:37p Mpanas
// Add fields to the Export and DiskDescriptor
// tables to support the future ISP2200 and
// Multiple FCInstances
// 
// 2     3/16/99 8:50p Mpanas
// Use the REAL size of Inquiry
// 
// 1     3/09/99 3:10p Mpanas
// Field defs for the DiskDescriptor table are
// defined here.  Include this once per Image!
// 
// 03/05/99 Michael G. Panas: Create file
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"
#include "DiskDescriptor.h"


//  verify that FieldDefs array-driven size computation agrees
//  with sizeof(DiskDescriptor)
CHECKFIELDDEFS (DiskDescriptor);



	fieldDef	DiskDescriptorTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									8,	ROWID_FT, Persistant_PT,
		"version",									4,	U32_FT, Persistant_PT,
		"size",										4,	U32_FT, Persistant_PT,
		"SlotID",									4,	U32_FT, Persistant_PT,
		"fSNValid",									4,	U32_FT, Persistant_PT,
		"SerialNumber",								32,	STRING32_FT, Persistant_PT,
		"WWNName",					  				16,	STRING16_FT, Persistant_PT,
		"CurrentStatus",							4,	U32_FT, Persistant_PT,
		"DiskType",									4,	U32_FT, Persistant_PT,
		"LockState",								4,	U32_FT, Persistant_PT,
		"Capacity",									8,	U64_FT, Persistant_PT,
		"InqData",					  				56,	BINARY_FT, Persistant_PT,
		"ridVendor",								8,	ROWID_FT, Persistant_PT
	};

// defined here so other folks can get to it	
U32			cbDiskDescriptorTable_FieldDefs = 
				sizeof(DiskDescriptorTable_FieldDefs);
				
