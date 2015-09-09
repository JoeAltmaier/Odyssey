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
// File: WWNTable.cpp
// 
// Description:
// The WWName table
// 
// $Log: /Gemini/Include/CTTables/WWNTable.cpp $
// 
// 1     1/05/00 6:58p Dpatel
// Initial creation
//
/*************************************************************************/

#include "WWNTable.h"

const fieldDef	WWNTable_FieldDefs[] = 
{
	fdVERSION,		4,	U32_FT,			Persistant_PT,
	fdSIZE,			4,	U32_FT,			Persistant_PT,
	fdCHASSIS_WWN,	8,	U64_FT,			Persistant_PT
};
				 
//  size of field definition table, in bytes
const U32 sizeofWWNTable_FieldDefs  =  sizeof(WWNTable_FieldDefs);
