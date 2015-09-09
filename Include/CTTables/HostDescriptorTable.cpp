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
// This is the Host Descriptor Table
// 
// Update Log:
// 06/09/99 Andrey Gusev: Created the file
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"
#include "HostDescriptorTable.h"



	fieldDef	HostDescriptorTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									0,	ROWID_FT,	Persistant_PT,
		"version",									0,	U32_FT,		Persistant_PT,
		"size",										0,	U32_FT,		Persistant_PT,
		HDT_FN_NAME,			  					32,	USTRING16_FT,	Persistant_PT,
		HDT_FN_DESCRIPTION,							128,USTRING64_FT,	Persistant_PT,
		HDT_FN_HOST_OS,								0,	U32_FT,		Persistant_PT,
		HDT_FN_IP_ADDRESS,							0,	U32_FT,		Persistant_PT,
		HDT_FN_EIP_COUNT,							0,	U32_FT,		Persistant_PT,
		HDT_FN_EIP,									sizeof(rowID)*HOST_DESCRIPTOR_TABLE_EIP_MAX_COUNT, BINARY_FT, Persistant_PT,
	};

// defined here so other folks can get to it	
U32			cbHostDescriptor_FieldDefs = 
				sizeof(HostDescriptorTable_FieldDefs);