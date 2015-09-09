/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpConfig.h
// 
// Description:
// This file defines the configuration data for the BsaIsm DDM. 
//
// Update Log:
//	$Log: /Gemini/Include/CTTables/ConfigRecords/FcpConfig.cpp $
// 
// 2     9/16/99 4:24p Jlane
// Add CHECKFIELDDEFS macro.
// 
// 1     8/25/99 1:26p Egroff
// Initial checkin.
// 
//
/*************************************************************************/

#include "CtTypes.h"
#include "PTSCommon.h"
#include "FcpConfig.h"


CHECKFIELDDEFS(FCP_CONFIG);

const fieldDef	FcpConfig_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
		"version",						4,	U32_FT, Persistant_PT,
		"size",							4,	U32_FT, Persistant_PT,
		"enable_initiator_mode",		4,	U32_FT, Persistant_PT,
		"enable_target_mode",			4,	U32_FT, Persistant_PT,
		"enable_fairness",				4,	U32_FT, Persistant_PT,
		"enable_fast_posting",			4,	U32_FT, Persistant_PT,
		"enable_hard_address",			4,	U32_FT, Persistant_PT,
		"options",						4,	U32_FT, Persistant_PT,
		"event_queue_size",				4,	U32_FT, Persistant_PT,
   		"mb_queue_size",				4,	U32_FT, Persistant_PT,
		"task_stack_size",				4,	U32_FT, Persistant_PT,   
		"task_priority",				4,	U32_FT, Persistant_PT,
		"max_frame_size",				4,	U32_FT, Persistant_PT,
		"ISP_FIFO_request_size",		4,	U32_FT, Persistant_PT,
  		"ISP_FIFO_response_size",		4,	U32_FT, Persistant_PT,
  		"hard_address",					4,	U32_FT, Persistant_PT,
   		"node_name[8]",					8,	BINARY_FT, Persistant_PT,	//UNSIGNED_CHAR
   		"HISR_stack_size",				4,	U32_FT, Persistant_PT,
   		"num_IDs",						4,	U32_FT, Persistant_PT,
		"num_LUNs",						4,	U32_FT, Persistant_PT,
		"num_buffers",					4,	U32_FT, Persistant_PT,
		"config_instance",				4,	U32_FT, Persistant_PT,
		"loop_instance",				4,	U32_FT, Persistant_PT,
		"virtual_device",				4,	VDN_FT, Persistant_PT
	};

// defined here so other folks can get to it	
const U32 cbFcpConfig_FieldDefs = sizeof(FcpConfig_FieldDefs);
				

