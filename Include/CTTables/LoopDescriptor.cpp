/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LoopDescriptor.cpp
// 
// Description:
// This module is the Table definition of the Loop Descriptor Table
//
//	Note:  There should only be one of these for each
//         loaded image!
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/LoopDescriptor.cpp $
// 
// 4     8/06/99 9:31a Vnguyen
//
// 4     8/06/99  9:19a Vnguyen
// Added row IDs for Status and Performance records.
// 
// 3     7/14/99 8:25p Mpanas
// wrong field type
// 
// 2     7/13/99 9:42p Mpanas
// Change name of table to LoopDescriptorEntry
// add fields to specify which target IDs are used
// 
// 1     5/28/99 4:50p Mpanas
// First cut of the Loop Descriptor Table Definition
// 
// 05/28/99 Michael G. Panas: Create file
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"
#include "LoopDescriptor.h"



	fieldDef	Loop_Descriptor_Table_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName						Size   Type
		fdLD_VERSION,					4,	U32_FT, Persistant_PT,
		fdLD_SIZE,						4,	U32_FT, Persistant_PT,
		fdLD_LOOP_NUM,					4,	U32_FT, Persistant_PT,
		fdLD_SLOT,						4,	U32_FT, Persistant_PT,
		fdLD_CHIP_NUM,				 	4,	U32_FT, Persistant_PT,
		fdLD_DM_VDN,		 			4,	U32_FT, Persistant_PT,
		fdLD_LM_VDN,		 			4,	U32_FT, Persistant_PT,
		fdLD_FLAGS,			 	 		4,	U32_FT, Persistant_PT,
		fdLD_BOARD_TYPE,	 	 		4,	U32_FT, Persistant_PT,
		fdLD_CHIP_TYPE,					4,	U32_FT, Persistant_PT,
		fdLD_DESIRED_LOOP_STATE,		4,	U32_FT, Persistant_PT,
		fdLD_ACTUAL_LOOP_STATE,			4,	U32_FT, Persistant_PT,
		fdLD_IDS_INUSE,					4,	U32_FT, Persistant_PT,
		fdLD_TARGET_IDS,				32,	BINARY_FT, Persistant_PT,
		fdLD_STATUS_RID,	 	 		8,	ROWID_FT, Persistant_PT,
		fdLD_PERFORMANCE_RID,			8,	ROWID_FT, Persistant_PT
	};

// defined here so other folks can get to it	
U32 cbLoop_Descriptor_Table_FieldDefs = 
				sizeof(Loop_Descriptor_Table_FieldDefs);
				

