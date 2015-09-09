/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdDescriptor.cpp
// 
// Description:
// This module is the Table definition of the SsdDescriptor Table
//
// $Log: /Gemini/Include/CTTables/SsdDescriptor.cpp $
// 
// 8     11/22/99 10:44a Jfrandeen
// Use new FlashStorage in place of FlashFile
// 
// 7     11/09/99 5:37p Hdo
// 
// 6     10/25/99 4:22p Hdo
// Change from the IopState to rowID type for the ridIopStatus
// 
// 5     10/15/99 12:28p Agusev
// Fixed a compile error. Has anybody compiled this before?!!!!!! Christ!
// 
// 4     10/14/99 4:53p Hdo
// Add vdnBSADdm, SerialNumber, IopStatus, vdnMonitor
// 
// 07/27/99 Jim Frandeen: Create file
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"
#include "SsdDescriptor.h"


fieldDef	SSD_descriptor_table_field_defs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									8,	ROWID_FT, Persistant_PT,
		"version",									4,	U32_FT, Persistant_PT,
		"size",										4,	U32_FT, Persistant_PT,
		FIELD_CAPACITY,								8,	U64_FT, Persistant_PT,
		"vdBSADdm",									4,	U32_FT, Persistant_PT,
		"SerialNumber",								32,	STRING32_FT, Persistant_PT,
		"ridIop_Status",							8,	ROWID_FT, Persistant_PT,
		"vdMonitor",								4,	U32_FT, Persistant_PT,

		"memory_size",								4,	U32_FT, Persistant_PT,
		"callback_memory_size",						4,	U32_FT, Persistant_PT,

		// CM_CONFIG
		"cache_config_version",						4,	U32_FT, Persistant_PT,
		"cache_config_size",						4,	U32_FT, Persistant_PT,
		"page_size",								4,	U32_FT, Persistant_PT,
		"num_pages",								4,	U32_FT, Persistant_PT,
		"p_page_memory",							4,	U32_FT, Persistant_PT,
		"num_pages_secondary",						4,	U32_FT, Persistant_PT,
		"p_page_memory_secondary",					4,	U32_FT, Persistant_PT,
		"p_table_memory",							4,	U32_FT, Persistant_PT,
		"page_table_size",							4,	U32_FT, Persistant_PT,
		"hash_table_size",							4,	U32_FT, Persistant_PT,
		"num_reserve_pages",						4,	U32_FT, Persistant_PT,
		"dirty_page_writeback_threshold",			4,	U32_FT, Persistant_PT,
		"dirty_page_error_threshold",				4,	U32_FT, Persistant_PT,
		"num_prefetch_forward",						4,	U32_FT, Persistant_PT,
		"num_prefetch_backward",					4,	U32_FT, Persistant_PT,
		
		// FF_CONFIG
		"flash_config_version",						4,	U32_FT, Persistant_PT,
		"flash_config_size",						4,	U32_FT, Persistant_PT,
		"p_device",									4,	U32_FT, Persistant_PT,
		"p_memory",									4,	U32_FT, Persistant_PT,
		"memory_size",								4,	U32_FT, Persistant_PT,
		"verify_write",								4,	U32_FT, Persistant_PT,
		"verify_erase",								4,	U32_FT, Persistant_PT,
		"verify_page_erased_before_write",			4,	U32_FT, Persistant_PT,
		"wear_level_threshold",						4,	U32_FT, Persistant_PT,
		"percentage_erased_pages",					4,	U32_FT, Persistant_PT,
		"percentage_replacement_pages",				4,	U32_FT, Persistant_PT,
		"replacement_page_threshold",				4,	U32_FT, Persistant_PT,
		"erase_all_pages",							4,	U32_FT, Persistant_PT,
		"verify_structures",						4,	U32_FT, Persistant_PT,
		"test_all_random",							4,	U32_FT, Persistant_PT,
		"test_all_static",							4,	U32_FT, Persistant_PT,
		"write_error_frequency_value",				4,	U32_FT, Persistant_PT,
		"erase_error_frequency_value",				4,	U32_FT, Persistant_PT,
		"read_error_frequency_value",				4,	U32_FT, Persistant_PT
	};


// defined here so other folks can get to it	
U32			cb_SSD_descriptor_table_field_defs = 
				sizeof(SSD_descriptor_table_field_defs);
