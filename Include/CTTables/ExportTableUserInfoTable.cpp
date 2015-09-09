/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"
#include "ExportTableUserInfoTable.h"


	fieldDef	ExportTableUserInfoTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									8,	ROWID_FT,	Persistant_PT,
		"version",									4,	U32_FT,		Persistant_PT,
		"size",										4,	U32_FT,		Persistant_PT,
		TFN_ETUI_RIDNAME,							8,	ROWID_FT,	Persistant_PT,
		TFN_ETUI_RIDDESCRIPTION,					8,	ROWID_FT,	Persistant_PT,
	};

// defined here so other folks can get to it	
U32			cbExportTableUserInfoTable_FieldDefs = 
				sizeof(ExportTableUserInfoTable_FieldDefs);