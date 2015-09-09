/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: RaidSpareDescriptor.cpp
// 
// Description:
// Raid Spare Descriptor fielddefs
// 
// $Log: /Gemini/Include/CTTables/RaidSpareDescriptor.cpp $
// 
// 9     10/15/99 3:57p Dpatel
// cmd_index not in field_defs, caused different sizeof struct and
// ptsfielddefs
// 
// 8     8/20/99 3:46p Dpatel
// added state identifier to keep track of state in case of failover
// 
// 6     8/11/99 3:01p Dpatel
// Added version size to tables, changed cap to I64.
// 
// 5     6/25/99 4:33p Dpatel
// Added Capacity and Vdn in Spare Descriptor. Fixed the VSS header.
// 
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/

#include "RaidSpareDescriptor.h"

/********************************************************************
*
* Array Spare Descriptor FieldDefs
*
********************************************************************/

const fieldDef SpareDescriptorTable_FieldDefs[] = 
{
	fdSDT_VERSION,			4,	U32_FT,			Persistant_PT,
	fdSDT_SIZE,				4,	U32_FT,			Persistant_PT,
	fdSPARE_TYPE,			4,	U32_FT,			Persistant_PT,
	fdSPARE_RID,			8,	ROWID_FT,		Persistant_PT,
	fdARRAY_RID,			8,	ROWID_FT,		Persistant_PT,
	fdHOST_RID,				8,	ROWID_FT,		Persistant_PT,
	fdCAPACITY,				8,	U64_FT,			Persistant_PT,
	fdVDN,					4,	S32_FT,			Persistant_PT,
	fdSPARE_POLICY,			4,	U32_FT,			Persistant_PT,
	// STATE IDENTIFIER, for RMSTR use only
	fdCMD_ROWID,			8,	ROWID_FT,		Persistant_PT,
	fdCMD_OPCODE,			4,	U32_FT,			Persistant_PT,
	fdCMD_STATE,			4,	U32_FT,			Persistant_PT,
	fdCMD_INDEX,			4,	U32_FT,			Persistant_PT		
	// STATE IDENTIFIER end
};
//  size of field definition table, in bytes
const U32 sizeofSpareDescriptorTable_FieldDefs  =  sizeof(SpareDescriptorTable_FieldDefs);
