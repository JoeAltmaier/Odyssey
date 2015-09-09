/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FMConfig.cpp.
// 
// Description:
// This file defines the configuration data for the Flash Monitor DDM. 
//
// Update Log:
//	$Log: /Gemini/Include/CTTables/ConfigRecords/FMConfig.cpp $
// 
// 1     10/22/99 11:28a Hdo
// Initial check in
// 
/*************************************************************************/

#include "CtTypes.h"
#include "FMConfig.h"


CHECKFIELDDEFS(FM_CONFIG);

const fieldDef	FMConfig_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName					Size   Type
		"version",						4,		U32_FT, Persistant_PT,
		"size",							4,		U32_FT, Persistant_PT,
		"SerialNumber",					32,		STRING32_FT, Persistant_PT,
		"vdSSD",						4,		U32_FT, Persistant_PT,
	};

// defined here so other folks can get to it
const U32 cbFMConfig_FieldDefs = sizeof(FMConfig_FieldDefs);
