/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: BsaConfig.
// 
// Description:
// This file defines the configuration data for the BsaIsm DDM. 
//
// Update Log:
//	$Log: /Gemini/Include/CTTables/ConfigRecords/BsaConfig.cpp $
// 
// 2     9/16/99 4:24p Jlane
// Add CHECKFIELDDEFS macro.
// 
// 1     9/15/99 4:56p Egroff
// Initial checkin.
// 
//
/*************************************************************************/

#include "CtTypes.h"
#include "PTSCommon.h"
#include "BsaConfig.h"


CHECKFIELDDEFS(BSA_CONFIG);

const fieldDef	BSAConfig_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
		"version",						4,	U32_FT, Persistant_PT,
		"size",							4,	U32_FT, Persistant_PT,
		"LUN",							4,	U32_FT,	Persistant_PT,
		"ID",							4,	U32_FT, Persistant_PT,
		"initVD",						4,	VDN_FT,	Persistant_PT,
		"EnableSmart",					4,	U32_FT,	Persistant_PT,
	};

// defined here so other folks can get to it	
const U32 cbBSAConfig_FieldDefs = sizeof(BSAConfig_FieldDefs);
				

