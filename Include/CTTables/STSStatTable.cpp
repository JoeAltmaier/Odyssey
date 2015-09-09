/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/STSStatTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// SCSI Target Server Status table.
// 
// $Log: /Gemini/Include/CTTables/STSStatTable.cpp $
// 
// 5     10/07/99 11:28a Vnguyen
// Add Status counters for number of error replies received and also # of
// errors that are generated internally in SCSI Target Server.
// 
// 4     9/14/99 5:36p Vnguyen
// Update performance and status counters to match the counters returned
// by the SCSI Target Server DDM.
// 
// 3     8/16/99 8:41a Vnguyen
// Fix up parameter declaration to pass in DID for the PHS Reporter class
// object to use.
// 
// 2     8/02/99 8:58a Vnguyen
// 
// 1     7/28/99 1:08p Vnguyen
// Initial check-in
// 
// 1     7/28/99 1:01p Vnguyen
// Initial Checkin.
// 
/*************************************************************************/
#include  "STSStatTable.h"


const fieldDef aSTSStatTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName             				Size  Type        Persistance Type
//		CT_STSST_RID,						8,	ROWID_FT,	Persistant_PT,		rowID - automatically supplied
		CT_STSST_VERSION,					4,	U32_FT,		Persistant_PT,
		CT_STSST_SIZE,						4,	U32_FT,		Persistant_PT,
		CT_STSST_KEY,						4,	U32_FT,		NotPersistant_PT,
		CT_STSST_REFRESHRATE,				4,	U32_FT,		Persistant_PT,
		CT_STSST_RIDEXPORTRECORD,			8,	ROWID_FT,	Persistant_PT,
		
		CT_STSST_NUMTIMERTIMEOUT,			8,	S64_FT,		NotPersistant_PT,
		CT_STSST_NUMERRORREPLIESRECEIVED,	8,	S64_FT,		NotPersistant_PT,
		CT_STSST_NUMERRORREPLIESSENT,		8,	S64_FT,		NotPersistant_PT,
		CT_STSST_SCSILOGPAGES,			SCSI_PAGE_SIZE * 4,			 U32_FT, NotPersistant_PT,
		CT_STSST_NUMDIFFERENTIALERROR,	DIFFERENTIAL_ERROR_SIZE * 4, U32_FT, NotPersistant_PT

	};
		
//  size of field definition table, in bytes
const U32  cbSTSStatTable_FieldDefs =  sizeof (aSTSStatTable_FieldDefs);

