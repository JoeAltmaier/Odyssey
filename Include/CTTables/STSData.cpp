/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: STSData.cpp
// 
// Description:
// This module is the Table definition of the SCSI Target Server Data Table
//
//	Note:  There should only be one of these for each
//         loaded image!
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/STSData.cpp $
// 
// 1     11/05/99 3:16p Mpanas
// New Table to make the Inquiry and Mode pages 
// persistent.  This table is used by the SCSI Target
// Server
// 
// 
// 11/03/99 Michael G. Panas: Create file
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"


	fieldDef	STSDataTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rid",										8,	ROWID_FT, Persistant_PT,
		"version",									4,	U32_FT, Persistant_PT,
		"size",										4,	U32_FT, Persistant_PT,
		"vdSTS",									4,	U32_FT, Persistant_PT,
		"InqData",					  				56,	BINARY_FT, Persistant_PT,
		"ModePages",								256,USTRING256_FT, Persistant_PT
	};

// defined here so other folks can get to it	
U32			cbSTSDataTable_FieldDefs = 
				sizeof(STSDataTable_FieldDefs);