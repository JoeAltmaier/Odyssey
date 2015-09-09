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
// File: PartitionTable.cpp
// 
// Description:
// Partition Descriptor Table Feild Defs
// 
// $Log: /Gemini/Include/CTTables/PartitionTable.cpp $
// 
// 1     9/15/99 10:49a Dpatel
// Initial creation
// 
// 
//
/*************************************************************************/

#include "PartitionTable.h"

/********************************************************************
*
* Partition Descriptor
*
********************************************************************/

const fieldDef PartitionDescriptorTable_FieldDefs[] = 
{
	fdPARTITION_TABLE_VERSION,			4,	U32_FT,			Persistant_PT,
	fdPARTITION_DESCRIPTOR_SIZE,		4,	U32_FT,			Persistant_PT,
	fdSRCTRID,				8,	ROWID_FT,		Persistant_PT,
	fdVIRTUAL_DEVICE,		4,	U32_FT,			Persistant_PT,
	fdPARENT_RID,			8,	ROWID_FT,		Persistant_PT,
	fdPARENT_VDN,			4,	U32_FT,			Persistant_PT,
	fdSTART_LBA,			8,	U64_FT,			Persistant_PT,
	fdPARTITION_SIZE,		8,	U64_FT,			Persistant_PT,
	fdNEXT_RID,				8,	ROWID_FT,		Persistant_PT,
	fdPREV_RID,				8,	ROWID_FT,		Persistant_PT,
	fdPARTITION_STATUS,		4,	U32_FT,			Persistant_PT,
	fdOPCODE,				4,	U32_FT,			Persistant_PT,
	fdCMD_RID,				8,	ROWID_FT,		Persistant_PT,
	fdSTATE,				4,	U32_FT,			Persistant_PT,
	fdINDEX,				4,	U32_FT,			Persistant_PT
};

//  size of field definition table, in bytes
const U32 sizeofPartitionDescriptorTable_FieldDefs  =  sizeof(PartitionDescriptorTable_FieldDefs);

