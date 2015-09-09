/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FCPortDatabase.cpp
// 
//  $Log: /Gemini/Include/CTTables/FCPortDatabaseTable.cpp $
// 
// 7     2/09/00 6:02p Dpatel
// Fix for Jerry's bug of not being able to export LUNs to initiators
// on OUR initiator loop
// 
// 6     2/09/00 2:32p Mpanas
// Split wwName field to Port and Node wwn
// Add Flags for Owner field
//
// Description:
// This module is the Table definition of the FC Port Database Table
//
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"
#include "FcPortDatabaseTable.h"


	fieldDef	FCPortDatabaseTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									8,	ROWID_FT,	Persistant_PT,
		"version",									0,	U32_FT,		Persistant_PT,
		"size",										0,	U32_FT,		Persistant_PT,
		"ridLoopDescriptor",						0,	ROWID_FT,	Persistant_PT,
		"id",										0,	U32_FT,		Persistant_PT,
		"wwName",									8,	BINARY_FT,	Persistant_PT,
		"wwnNodeName",								8,	BINARY_FT,	Persistant_PT,
		"portType",									0,	U32_FT,		Persistant_PT,
		"portStatus",								0,	U32_FT,		Persistant_PT,
		"attribs",									0,	U32_FT, 	Persistant_PT,
		FCP_PORT_DTB_TABLE_FN_RID_NAME,				0,	ROWID_FT,	Persistant_PT
	};

// defined here so other folks can get to it	
U32			cbFCPortDatabase_FieldDefs = 
				sizeof(FCPortDatabaseTable_FieldDefs);