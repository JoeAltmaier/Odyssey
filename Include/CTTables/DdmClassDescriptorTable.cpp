/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/DdmClassDescriptorTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// Ddm Class Descriptor table.
// 
// $Log: /Gemini/Include/CTTables/DdmClassDescriptorTable.cpp $
// 
// 1     3/19/99 9:31a Jlane
// Initial Checkin.
// 
/*************************************************************************/

#include  "DdmClassDescriptorTable.h"


const fieldDef aDdmClassDescriptorTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName             	  Size  Type         Persist yes/no

// rowID - automatically supplied
//		"rid",						8,	ROWID_FT,		Persistant_PT,
		CT_DCDT_VERSION,			4,	U32_FT,			Persistant_PT,
		CT_DCDT_SIZE,				4,	U32_FT,			Persistant_PT,
		CT_DCDT_ICABINET,			4,	U32_FT,			Persistant_PT,
		CT_DCDT_ISLOT,				4,	U32_FT,			Persistant_PT,
		CT_DCDT_CLASSNAME,			32,	STRING32_FT,	Persistant_PT,
		CT_DCDT_CLASSVERSION,		4,	U32_FT,			Persistant_PT,
		CT_DCDT_CLASSREVISION,		4,	U32_FT,			Persistant_PT,
		CT_DCDT_RIDPROPERTYSHEET,	8,	ROWID_FT,		Persistant_PT,
		CT_DCDT_SSTACK,				4,	U32_FT,			Persistant_PT,
		CT_DCDT_SQUEUE,				4,	U32_FT,			Persistant_PT,
//		CT_DCDT_CTOR,				4,	U32_FT,			Persistant_PT,
	};


//  size of field definition table, in bytes
const U32  cbDdmClassDescriptorTable_FieldDefs =  sizeof (aDdmClassDescriptorTable_FieldDefs);

