/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/FCPTargetStatusTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// FCP Target Driver Status table.
// 
// $Log: /Gemini/Include/CTTables/FCPTargetStatusTable.cpp $
// 
// 4     10/11/99 6:04p Vnguyen
// Add timestamp for Performance counters and # of errors received for
// Status counters.
// 
// 3     9/15/99 11:55a Vnguyen
// Change Uptime from U32 to I64.
// 
// 2     8/18/99 7:36a Vnguyen
// Fix a few typo errors.
// 
// 1     8/16/99 2:42p Vnguyen
// Initial Check-in.
// 
// 
/*************************************************************************/
#include  "FCPTargetStatusTable.h"


const fieldDef aFCPTargetStatusTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName             				Size  Type        Persistance Type
//		CT_FCPTST_RID,						8,	ROWID_FT,	Persistant_PT,		rowID - automatically supplied
		CT_FCPTST_VERSION,					4,	U32_FT,		Persistant_PT,
		CT_FCPTST_SIZE,						4,	U32_FT,		Persistant_PT,
		CT_FCPTST_KEY,						4,	U32_FT,		NotPersistant_PT,
		CT_FCPTST_REFRESHRATE,				4,	U32_FT,		Persistant_PT,
		CT_FCPTST_RIDLDRECORD,				8,	ROWID_FT,	Persistant_PT,
		CT_FCPTST_UPTIME,					8,	S64_FT,		NotPersistant_PT,

		CT_FCPTST_NUMERRORREPLIESRECEIVED, 	8, 	S64_FT,		NotPersistant_PT,
		
		CT_FCPTST_FCPTARGETSTATETABLE,		8,	S64_FT,		NotPersistant_PT,
		CT_FCPTST_DRIVERREADYSTATEFLAG,		8,	S64_FT,		NotPersistant_PT,
		CT_FCPTST_ERRORS,					8,	S64_FT,		NotPersistant_PT,
		CT_FCPTST_LOOPDOWN,					8,	S64_FT,		NotPersistant_PT,
		CT_FCPTST_TRANSFERINCOMPLETE,		8,	S64_FT,		NotPersistant_PT
		
	};
		
//  size of field definition table, in bytes
const U32  cbFCPTargetStatusTable_FieldDefs =  sizeof (aFCPTargetStatusTable_FieldDefs);

