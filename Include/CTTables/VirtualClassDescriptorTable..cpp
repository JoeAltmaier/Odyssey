/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/VirtualClassDescriptorTable..cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// IOP Status table.
// 
// $Log: /Gemini/Include/CTTables/VirtualClassDescriptorTable..cpp $
// 
// 1     3/17/99 10:35p Jlane
// INitial Checkin.
// 
// 1     3/17/99 10:37a Jlane
// Initiali checkin.
// 
//
/*************************************************************************/

#include  "VirtualClassDescriptorTable.h"


const fieldDef aVirtualClassDescriptorTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName             	  Size  Type         Persist yes/no

// rowID - automatically supplied
//		"rid",						8,	ROWID_FT,		Persistant_PT,
		CT_VCDT_VERSION,			4,	U32_FT,			Persistant_PT,
		CT_VCDT_SIZE,				4,	U32_FT,			Persistant_PT,
		CT_VCDT_CLASSNAME,			32,	STRING32_FT,	Persistant_PT,
		CT_VCDT_CLASSVERSION,		4,	U32_FT,			Persistant_PT,
		CT_VCDT_CLASSREVISION,		4,	U32_FT,			Persistant_PT,
		CT_VCDT_RIDPROPERTYSHEET,	8,	ROWID_FT,		Persistant_PT,
		CT_VCDT_SSTACK,				4,	U32_FT,			Persistant_PT,
		CT_VCDT_SQUEUE,				4,	U32_FT,			Persistant_PT,
		CT_VCDT_IPRIMARYCABINET,	4,	U32_FT,			Persistant_PT,
		CT_VCDT_IPRIMARYSLOT,		4,	U32_FT,			Persistant_PT,
//		CT_VCDT_PRIMARYCTOR,		4,	U32_FT,			Persistant_PT,
		CT_VCDT_IFAILOVERCABINET,	4,	U32_FT,			Persistant_PT,
		CT_VCDT_IFAILOVERSLOT,		4,	U32_FT,			Persistant_PT
//		CT_VCDT_FAILOVERCTOR,		4,	U32_FT,			Persistant_PT
	};


//  size of field definition table, in bytes
const U32  cbVirtualClassDescriptorTable_FieldDefs =  sizeof (aVirtualClassDescriptorTable_FieldDefs);

