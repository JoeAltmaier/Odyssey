/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiConfig.h
// 
// Description:
// This file defines the configuration data for the Hscsi DDM. 
//
// Update Log:
//	$Log: /Gemini/Include/CTTables/ConfigRecords/HscsiConfig.cpp $
// 
// 2     10/19/99 3:31p Cchan
// Cleaned up comments
// 
// 1     10/12/99 4:52p Cchan
// Files needed for PTS support
//
/*************************************************************************/

#include "CtTypes.h"
#include "PTSCommon.h"
#include "HscsiConfig.h"


CHECKFIELDDEFS(HSCSI_CONFIG);

const fieldDef	HscsiConfig_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
		"version",						4,	U32_FT, Persistant_PT,
		"size",							4,	U32_FT, Persistant_PT,
		"enable_initiator_mode",		4,	U32_FT, Persistant_PT,
		"enable_target_mode",			4,	U32_FT, Persistant_PT,
		"enable_fairness",				4,	U32_FT, Persistant_PT,
		"enable_fast_posting",			4,	U32_FT, Persistant_PT,
		"enable_hard_address",			4,	U32_FT, Persistant_PT,
		"event_queue_size",				4,	U32_FT, Persistant_PT,
   		"mb_queue_size",				4,	U32_FT, Persistant_PT,
		"task_stack_size",				4,	U32_FT, Persistant_PT,   
		"task_priority",				4,	U32_FT, Persistant_PT,
		"ISP_FIFO_request_size",		4,	U32_FT, Persistant_PT,
  		"ISP_FIFO_response_size",		4,	U32_FT, Persistant_PT,
  		"hard_address",					4,	U32_FT, Persistant_PT,
   		"HISR_stack_size",				4,	U32_FT, Persistant_PT,
		"num_LUNs",						4,	U32_FT, Persistant_PT,
		"num_buffers",					4,	U32_FT, Persistant_PT,
		"base_ISP_address",				4,	U32_FT,	Persistant_PT,
		"interrupt",					4,	U32_FT,	Persistant_PT,
		"config_instance",				4,	U32_FT, Persistant_PT,
		"virtual_device",				4,	VDN_FT, Persistant_PT
	};

// defined here so other folks can get to it	
const U32 cbHscsiConfig_FieldDefs = sizeof(HscsiConfig_FieldDefs);
				

