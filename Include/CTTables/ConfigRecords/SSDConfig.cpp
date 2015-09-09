/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SSDConfig.cpp.
// 
// Description:
// This file defines the configuration data for the SSD DDM.
//
// Update Log:
//	$Log: /Gemini/Include/CTTables/ConfigRecords/SSDConfig.cpp $
// 
// 3     12/10/99 1:29p Hdo
// Add verify_structures
// 
// 2     10/27/99 10:35a Hdo
// Add more fields: SerialNumber, vdnBSA, vdMonitor
// 
// 1     10/01/99 5:51p Hdo
// Initial check-in
// 
/*************************************************************************/

#include "CtTypes.h"
#include "PTSCommon.h"
#include "SSDConfig.h"


CHECKFIELDDEFS(SSD_CONFIG);

const fieldDef	SSD_Config_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName				  Size   Type
		"version",						4,	U32_FT, Persistant_PT,
		"size",							4,	U32_FT, Persistant_PT,
		"capacity",						8,	U64_FT, Persistant_PT,
		"vdBSADdm",						4,	U32_FT, Persistant_PT,
		"SerialNumber",					32,	STRING32_FT, Persistant_PT,
		"vdMonitor",					4,	U32_FT, Persistant_PT,

		// CM_CONFIG
//		"cache_config_version",			4,	U32_FT, Persistant_PT,
//		"cache_config_size",			4,	U32_FT, Persistant_PT,
		"page_size",					4,	U32_FT, Persistant_PT,
		"page_table_size",				4,	U32_FT, Persistant_PT,
		"hash_table_size",				4,	U32_FT, Persistant_PT,
		"num_reserve_pages",			4,	U32_FT, Persistant_PT,
		"dirty_page_writeback_threshold",4,	U32_FT, Persistant_PT,
		"dirty_page_error_threshold",	4,	U32_FT, Persistant_PT,
		"num_prefetch_forward",			4,	U32_FT, Persistant_PT,
		"num_prefetch_backward",		4,	U32_FT, Persistant_PT,
		
		// FF_CONFIG
//		"flash_config_version",			4,	U32_FT, Persistant_PT,
//		"flash_config_size",			4,	U32_FT, Persistant_PT,
//		"memory_size",					4,	U32_FT, Persistant_PT,
//		"callback_memory_size",			4,	U32_FT, Persistant_PT,
		"verify_write_level",			4,	U32_FT, Persistant_PT,
		"verify_erase_level",			4,	U32_FT, Persistant_PT,
		"percentage_erased_pages",		4,	U32_FT, Persistant_PT,
		"percentage_replacement_pages",	4,	U32_FT, Persistant_PT,
		"replacement_page_threshold",	4,	U32_FT, Persistant_PT,
		"erase_all_pages",				4,	U32_FT, Persistant_PT,
		"verify_format_level",			4,	U32_FT, Persistant_PT,
		"verify_structures",			4,	U32_FT, Persistant_PT
	};

// defined here so other folks can get to it
const U32 cbSSD_Config_FieldDefs = sizeof(SSD_Config_FieldDefs);
