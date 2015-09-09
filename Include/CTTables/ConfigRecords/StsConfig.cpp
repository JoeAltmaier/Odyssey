/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: StsConfig.cpp
// 
// Description:
// This module is the Table definition of the SCSI Target Server Config
// Table
//
//	Note:  There should only be one of these for each
//         loaded image!
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/ConfigRecords/StsConfig.cpp $
// 
// 6     9/16/99 4:24p Jlane
// Add CHECKFIELDDEFS macro.
// 
// 5     9/01/99 12:08p Jlane
// Renamed ridVCMCommand to ridVcId.
// 
// 4     8/25/99 1:26p Egroff
// Updated to have rid and fielddef accessor methods.
// 
// 3     8/07/99 1:30p Jlane
// Changed vd to vdNext to match .cpp.
// 
// 2     8/07/99 1:25p Jlane
// Added ridVCMCommand.
// 
// 1     7/15/99 4:19p Jlane
// Initial Check-in.
// 
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"
#include "StsConfig.h"
#include "CtTypes.h"


CHECKFIELDDEFS(STS_CONFIG);

const fieldDef	StsConfigTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									8,	ROWID_FT, Persistant_PT,
		"version",									4,	U32_FT, Persistant_PT,
		"size",										4,	U32_FT, Persistant_PT,
		"ridVcId",									8,	ROWID_FT, Persistant_PT,
		"vdNext",									4,	U32_FT, Persistant_PT,
	};

// defined here so other folks can get to it	
const U32			cbStsConfigTable_FieldDefs = 
				sizeof(StsConfigTable_FieldDefs);