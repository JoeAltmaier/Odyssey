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
// File: RmstrCapabilityTable.cpp
// 
// Description:
// Rmstr Capabilities
// 
// $Log: /Gemini/Include/CTTables/RmstrCapabilityTable.cpp $
// 
// 2     8/11/99 3:01p Dpatel
// Added version size to tables, changed cap to I64.
// 
// 1     8/02/99 2:48p Dpatel
// Initial creation
// 
//
/*************************************************************************/

#include "RmstrCapabilityTable.h"

/********************************************************************
*
* Rmstr Capability Descriptor
*
********************************************************************/

const fieldDef RmstrCapabilityTable_FieldDefs[] = 
{
	fdCAP_VERSION,			4,	U32_FT,			Persistant_PT,
	fdCAP_SIZE,				4,	U32_FT,			Persistant_PT,
	fdCODE,					4,	U32_FT,			Persistant_PT,
	fdDESCRIPTION,			64, STRING64_FT,	Persistant_PT,
	fdPURCHASED	,			4,	U32_FT,			Persistant_PT,
	fdENABLED,				4,	U32_FT,			Persistant_PT,
	fdCAPABILITIES,			64,	BINARY_FT,		Persistant_PT
};

//  size of field definition table, in bytes
const U32 sizeofRmstrCapabilityTable_FieldDefs  =  sizeof(RmstrCapabilityTable_FieldDefs);

