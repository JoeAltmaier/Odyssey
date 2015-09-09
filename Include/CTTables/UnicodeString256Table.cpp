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


	fieldDef	UnicodeString256Table_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									8,	ROWID_FT,	Persistant_PT,
		"version",									4,	U32_FT,		Persistant_PT,
		"size",										4,	U32_FT,		Persistant_PT,
		"string",									512,USTRING256_FT,	Persistant_PT, 
	};

// defined here so other folks can get to it	
U32			cbUnicodeString256Table_FieldDefs = 
				sizeof(UnicodeString256Table_FieldDefs);