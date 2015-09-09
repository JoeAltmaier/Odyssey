/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/IopFiloverMapTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// IOP Status table.
// 
// $Log: /Gemini/Include/CTTables/IopFiloverMapTable.cpp $
// 
// 1     3/17/99 10:37a Jlane
// Initiali checkin.
// 
//
/*************************************************************************/

#include  "IOPFailoverMapTable.h"


const fieldDef aIopFailoverMapTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName             	  Size  Type         Persist yes/no

// rowID - automatically supplied
//		"rid",						8,	ROWID_FT,		Persistant_PT,
		CT_IOPFM_VERSION,			4,	U32_FT,			Persistant_PT,
		CT_IOPFM_SIZE,				4,	U32_FT,			Persistant_PT,
		CT_IOPFM_PRIMARYSLOTNUM,	4,	U32_FT,			Persistant_PT,
		CT_IOPFM_FAILOVERSLOTNUM,	4,	U32_FT,			Persistant_PT
 };


//  size of field definition table, in bytes
const U32  cbIopFailoverMapTable_FieldDefs =  sizeof (aIopFailoverMapTable_FieldDefs);


