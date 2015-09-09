/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SmConfig.cpp
// 
// Description:
// 
//
// Update Log:
//	$Log: /Gemini/Include/CTTables/ConfigRecords/SmConfig.cpp $
// 
// 2     10/19/99 3:34p Cchan
// Cleaned up comments.
// 
// 1     10/12/99 4:52p Cchan
// Files needed for PTS support
//
/*************************************************************************/

#include "CtTypes.h"
#include "Odyssey.h"
#include "PTSCommon.h"
#include "SmConfig.h"


CHECKFIELDDEFS(SM_CONFIG);

const fieldDef	SmConfig_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
		"version",					4,	U32_FT, Persistant_PT,
		"size",						4,	U32_FT, Persistant_PT,
		"num_drives",				4,	U32_FT, Persistant_PT,
        "flags",					4,	U32_FT, Persistant_PT,
	    "vd",						4,	VDN_FT,	Persistant_PT,
	    "xlate",					16,	BINARY_FT, Persistant_PT	
	};

// defined here so other folks can get to it	
const U32 cbSmConfig_FieldDefs = sizeof(SmConfig_FieldDefs);
				

