/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This is the Host Connection Descriptor Table
// 
// Update Log:
// 06/09/99 Andrey Gusev: Created the file
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"



	fieldDef	HostCollectionDescriptorTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									0,	ROWID_FT,	Persistant_PT,
		"version",									0,	U32_FT,		Persistant_PT,
		"size",										0,	U32_FT,		Persistant_PT,
		"isActive",									0,	U32_FT,		Persistant_PT,
		"description",								128,BINARY_FT,	Persistant_PT,
		"collectionTableName",						0,	STRING32_FT,Persistant_PT,
		"collectionType",							0,	U32_FT,		Persistant_PT,
		"ridLUN",									0,	ROWID_FT,	Persistant_PT,
		"name",										64,	BINARY_FT,	Persistant_PT,

};

// defined here so other folks can get to it	
U32			cbHostCollectionDescriptor_FieldDefs = 
				sizeof(HostCollectionDescriptorTable_FieldDefs);