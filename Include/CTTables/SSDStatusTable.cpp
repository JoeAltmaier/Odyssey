/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/SSDStatusTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// SSD Status table.
// 
// $Log: /Gemini/Include/CTTables/SSDStatusTable.cpp $
// 
// 3     10/08/99 9:17a Vnguyen
// Update tables to add additional performance and status counters.
// Fix bug with SSD Performance Table name.  Was using wrong string name.
// 
// 2     8/18/99 7:36a Vnguyen
// Fix a few typo errors.
// 
// 1     8/17/99 8:27a Vnguyen
// Initial check-in.
// 
// Update Log:
// 
/*************************************************************************/
#include  "SSDStatusTable.h"


const fieldDef aSSDStatusTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName             						  Size  Type        Persistance Type
//		CT_SSDST_RID,									8,	ROWID_FT,	Persistant_PT,		rowID - automatically supplied
		CT_SSDST_VERSION,								4,	U32_FT,		Persistant_PT,
		CT_SSDST_SIZE,									4,	U32_FT,		Persistant_PT,
		CT_SSDST_KEY,									4,	U32_FT,		NotPersistant_PT,
		CT_SSDST_REFRESHRATE,							4,	U32_FT,		Persistant_PT,
		CT_SSDST_RIDSRCRECORD,							8,	ROWID_FT,	Persistant_PT,
		
		CT_SSDST_NUMREPLACEMENTPAGESAVAILABLE,			4,	U32_FT,		NotPersistant_PT,
		CT_SSDST_PAGETABLESIZE,							4,	U32_FT,		NotPersistant_PT,
		CT_SSDST_PERCENTDIRTYPAGES,						4,	U32_FT,		NotPersistant_PT
	};
		
//  size of field definition table, in bytes
const U32  cbSSDStatusTable_FieldDefs =  sizeof (aSSDStatusTable_FieldDefs);
