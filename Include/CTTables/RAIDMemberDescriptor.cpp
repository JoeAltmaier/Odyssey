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
// File: RaidMemberDescriptor.cpp
// 
// Description:
// Member Descriptor Table Feild Defs
// 
// $Log: /Gemini/Include/CTTables/RAIDMemberDescriptor.cpp $
// 
// 13    10/15/99 3:57p Dpatel
// cmd_index not in field_defs, caused different sizeof struct and
// ptsfielddefs
// 
// 12    8/20/99 3:46p Dpatel
// added state identifier to keep track of state in case of failover
// 
// 10    8/11/99 3:01p Dpatel
// Added version size to tables, changed cap to I64.
// 
// 9     6/29/99 1:52p Dpatel
// Removed ShadowRID, added headers.
// 
//
/*************************************************************************/

#include "RaidMemberTable.h"

/********************************************************************
*
* Array Member Descriptor
*
********************************************************************/

const fieldDef MemberDescriptorTable_FieldDefs[] = 
{
	fdMDT_VERSION,			4,	U32_FT,			Persistant_PT,
	fdMDT_SIZE,				4,	U32_FT,			Persistant_PT,
	fdARRAY_RID,			8,	ROWID_FT,		Persistant_PT,
	fdMEMBER_RID,			8,	ROWID_FT,		Persistant_PT,
	fdMEMBER_VD,			4,	S32_FT,			Persistant_PT,
	fdMEMBER_HEALTH,		4,	U32_FT,			Persistant_PT,
	fdINDEX,				4,	U32_FT,			Persistant_PT,
	fdRETRY,				4,	U32_FT,			Persistant_PT,
	fdQUEUEING_METHOD,		4,	U32_FT,			Persistant_PT,
	fdSTART_LBA,			8,	U64_FT,			Persistant_PT,
	fdEND_LBA,				8,	U64_FT,			Persistant_PT,
	fdMAX_REQUESTS,			4,	U32_FT,			Persistant_PT,
	fdMEM_POLICY,			4,	U32_FT,			Persistant_PT,
	// STATE IDENTIFIER, for RMSTR use only
	fdCMD_ROWID,			8,	ROWID_FT,		Persistant_PT,
	fdCMD_OPCODE,			4,	U32_FT,			Persistant_PT,
	fdCMD_STATE,			4,	U32_FT,			Persistant_PT,
	fdCMD_INDEX,			4,	U32_FT,			Persistant_PT		
	// STATE IDENTIFIER end

};

//  size of field definition table, in bytes
const U32 sizeofMemberDescriptorTable_FieldDefs  =  sizeof(MemberDescriptorTable_FieldDefs);

