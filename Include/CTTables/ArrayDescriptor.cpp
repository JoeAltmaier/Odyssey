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
// File: ArrayDescriptor.cpp
// 
// Description:
// Array Descriptor Table Feild Defs
// 
// $Log: /Gemini/Include/CTTables/ArrayDescriptor.cpp $
// 
// 23    10/15/99 3:57p Dpatel
// cmd_index not in field_defs, caused different sizeof struct and
// ptsfielddefs
// 
// 22    8/20/99 3:46p Dpatel
// added state identifier to keep track of state in case of failover
// 
// 20    8/11/99 3:01p Dpatel
// Added version size to tables, changed cap to I64.
// 
// 19    8/04/99 3:32p Dpatel
// Added hotcopy export member index...
// 
// 18    7/29/99 6:57p Dpatel
// Added creation policy to array descriptor...
// 
// 17    7/27/99 6:06p Dpatel
// Added Source Member index and primary member index.
// 
// 16    7/20/99 6:50p Dpatel
// Changed arrayName to rowiD
// 
// 15    6/30/99 11:18a Dpatel
// Changed array name to Unicode string.
// 
// 
//
/*************************************************************************/

#include "ArrayDescriptor.h"

const fieldDef	ArrayDescriptorTable_FieldDefs[] = 
{
	fdADT_VERSION,		4,	U32_FT,			Persistant_PT,
	fdADT_SIZE,			4,	U32_FT,			Persistant_PT,
	fdARRAY_VDN,		4,	S32_FT,			Persistant_PT,
	fdARRAY_SRCTRID,	8,	ROWID_FT,		Persistant_PT,
	fdARRAY_CAPACITY,	8,	U64_FT,			Persistant_PT,
	fdAMEM_CAPACITY,	8,	U64_FT,			Persistant_PT,
	fdDBLK_SIZE,		4,	U32_FT,			Persistant_PT,
	fdPBLK_SIZE,		4,	U32_FT,			Persistant_PT,
	fdRAID_LEVEL,		4,	U32_FT,			Persistant_PT,
	fdARRAY_STATUS,		4,	U32_FT,			Persistant_PT,	// rsmCRITICAL,
	fdINIT_STATUS,		4,	U32_FT,			Persistant_PT,	// INIT_COMPLETE, INIT_NOT_STARTED
	fdPECKING,			4,	U32_FT,			Persistant_PT,	// rsmHIGH,rsmMEDIUM,rsmLOW
	fdNUM_MEMBERS,		4,	U32_FT,			Persistant_PT,
	fdNUM_UTILS,		4,	U32_FT,			Persistant_PT,	// currently running on array
	fdNUM_SPARES,		4,	U32_FT,			Persistant_PT,	// dedicated spares on array
	fdARRAY_SN,			4,	U32_FT,			Persistant_PT,	// timestamp
	fdCREATION,			4,	U32_FT,			Persistant_PT,	// timestamp
	fdCONFIG_TS,		4,	U32_FT,			Persistant_PT,	// configuration timestamp
	fdCREATION_POLICY,	4,	U32_FT,			Persistant_PT,
	fdARRAY_POLICY,		4,	U32_FT,			Persistant_PT,
	fdHOST_FOR_SPARE_POOL,	8,	ROWID_FT,	Persistant_PT,
	fdSOURCE_MEMBER,	4,	U32_FT,			Persistant_PT,
	fdPREFERRED_MEMBER,	4,	U32_FT,			Persistant_PT,
	fdHOTCOPY_EXPORT_MEMBER,4,					U32_FT,			Persistant_PT,
	fdUTILS,				MAX_UTILS*8,		ROWID_FT,		Persistant_PT,
	fdMEMBERS,				MAX_ARRAY_MEMBERS*8,ROWID_FT,		Persistant_PT,
	fdSPARES,				MAX_ARRAY_SPARES*8,	ROWID_FT,		Persistant_PT,
	// STATE IDENTIFIER, for RMSTR use only
	fdCMD_ROWID,		8,	ROWID_FT,		Persistant_PT,
	fdCMD_OPCODE,		4,	U32_FT,			Persistant_PT,
	fdCMD_STATE,		4,	U32_FT,			Persistant_PT,
	fdCMD_INDEX,		4,	U32_FT,			Persistant_PT	
	// STATE IDENTIFIER end
};				 
//  size of field definition table, in bytes
const U32 sizeofArrayDescriptorTable_FieldDefs  =  sizeof(ArrayDescriptorTable_FieldDefs);
