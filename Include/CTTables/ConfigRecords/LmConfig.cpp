/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpConfig.ccp
// 
// Description:
//  
//
// Update Log:
//	$Log: /Gemini/Include/CTTables/ConfigRecords/LmConfig.cpp $
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
#include "LmConfig.h"


CHECKFIELDDEFS(LM_CONFIG);

const fieldDef LmConfig_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
		"version",					4,	U32_FT, Persistant_PT,
		"size",						4,	U32_FT, Persistant_PT,
		"num_loops",				4,	U32_FT, Persistant_PT,
		"flags",					4,	U32_FT, Persistant_PT,
		"vd",						4,	VDN_FT, Persistant_PT,
		"FC_instance[]",			12,	BINARY_FT, Persistant_PT,
	};

// defined here so other folks can get to it	
const U32 cbLmConfig_FieldDefs = sizeof(LmConfig_FieldDefs);
				

