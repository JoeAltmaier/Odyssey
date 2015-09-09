/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RDConfig.cpp.
// 
// Description:
// This file defines the configuration data for the RAM Disk DDM. 
//
// Update Log:
//	$Log: /Gemini/Include/CTTables/ConfigRecords/RDConfig.cpp $
// 
// 2     9/16/99 4:24p Jlane
// Add CHECKFIELDDEFS macro.
// 
// 1     9/15/99 4:57p Jlane
// Initial Checkin.
// 
/*************************************************************************/

#include "CtTypes.h"
#include "PTSCommon.h"
#include "RDConfig.h"


CHECKFIELDDEFS(RD_CONFIG);

const fieldDef	RDConfig_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
		"version",						4,	U32_FT, Persistant_PT,
		"size",							4,	U32_FT, Persistant_PT,
		"Capacity",						4,	U32_FT,	Persistant_PT,
	};

// defined here so other folks can get to it	
const U32 cbRDConfig_FieldDefs = sizeof(RDConfig_FieldDefs);
				

