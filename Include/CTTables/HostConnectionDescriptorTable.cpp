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
#include "HostConnectionDescriptorTable.h"


	fieldDef	HostConnectionDescriptorTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									0,	ROWID_FT,	Persistant_PT,
		"version",									0,	U32_FT,		Persistant_PT,
		"size",										0,	U32_FT,		Persistant_PT,
		"eHostConnectionMode",						0,	U32_FT,		Persistant_PT,
		"ridEIPCount",								0,	U32_FT,		Persistant_PT,
		"ridEIPs",									sizeof(rowID)*MAX_HOST_CONNECTIONS,	ROWID_FT,	Persistant_PT,
		"InitiatorID",								sizeof(U32)*MAX_HOST_CONNECTIONS, BINARY_FT, Persistant_PT,
		"TargetID",									sizeof(U32)*MAX_HOST_CONNECTIONS, BINARY_FT, Persistant_PT,
		"LUN",										sizeof(U32)*MAX_HOST_CONNECTIONS, BINARY_FT, Persistant_PT,
		"flgEIPs",									sizeof(U32)*MAX_HOST_CONNECTIONS, BINARY_FT, Persistant_PT,
		HOST_CONNECTION_TABLE_FN_RID_NAME,			0,	ROWID_FT,	Persistant_PT,
		HOST_CONNECTION_TABLE_FN_RID_DESCRIPTION,	0,	ROWID_FT,	Persistant_PT,
		"ridHost",									0,	ROWID_FT,	Persistant_PT,
};

// defined here so other folks can get to it	
U32			cbHostConnectionDescriptor_FieldDefs = 
				sizeof(HostConnectionDescriptorTable_FieldDefs);