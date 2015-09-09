/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/STSPerfTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// SCSI Target Server (STS) Performance table.
// 
// $Log: /Gemini/Include/CTTables/STSPerfTable.cpp $
// 
// 6     10/18/99 9:57a Vnguyen
// Change key fron I64 to U32 to be consistent with other Reporters.  Also
// add a few comments to explain the meaning of the counters.
// 
// 5     9/14/99 5:36p Vnguyen
// Update performance and status counters to match the counters returned
// by the SCSI Target Server DDM.
// 
// 4     8/18/99 7:36a Vnguyen
// Fix a few typo errors.
// 
// 3     8/16/99 8:40a Vnguyen
// Fix up parameter declaration to pass in DID for the PHS Reporter class
// object to use.
// 
// 2     8/02/99 8:58a Vnguyen
// 
// 1     7/28/99 10:15a Vnguyen
// Initial check-in
// 
// 1     7/28/99 9:36a Vnguyen
// Initial Checkin.
// 
/*************************************************************************/
#include  "STSPerfTable.h"


const fieldDef aSTSPerfTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName             				  Size  Type        Persistance Type
//		CT_STSPT_RID,							8,	ROWID_FT,	Persistant_PT,		rowID - automatically supplied
		CT_STSPT_VERSION,						4,	U32_FT,		Persistant_PT,
		CT_STSPT_SIZE,							4,	U32_FT,		Persistant_PT,
		CT_STSPT_KEY,							4,	U32_FT,		NotPersistant_PT,
		CT_STSPT_REFRESHRATE,					4,	U32_FT,		Persistant_PT,
		CT_STSPT_SAMPLERATE,					4,	U32_FT,		Persistant_PT,
		CT_STSPT_RIDEXPORTRECORD,				8,	ROWID_FT,	Persistant_PT,
			
		CT_STSPT_AVGNUMBSAREADSPERSEC,			4,	U32_FT,		NotPersistant_PT,
		CT_STSPT_MAXNUMBSAREADSPERSEC,			4,	U32_FT,		NotPersistant_PT,
		CT_STSPT_MINNUMBSAREADSPERSEC,			4,	U32_FT,		NotPersistant_PT,
	
		CT_STSPT_AVGNUMBSAWRITESPERSEC,			4,	U32_FT,		NotPersistant_PT,
		CT_STSPT_MAXNUMBSAWRITESPERSEC,			4,	U32_FT,		NotPersistant_PT,
		CT_STSPT_MINNUMBSAWRITESPERSEC,			4,	U32_FT,		NotPersistant_PT,
	
		CT_STSPT_AVGNUMBSACMDSPERSEC,			4,	U32_FT,		NotPersistant_PT,
		CT_STSPT_MAXNUMBSACMDSPERSEC,			4,	U32_FT,		NotPersistant_PT,
		CT_STSPT_MINNUMBSACMDSPERSEC,			4,	U32_FT,		NotPersistant_PT,
	
		CT_STSPT_AVGNUMSCSICMDSPERSEC,			4,	U32_FT,		NotPersistant_PT,
		CT_STSPT_MAXNUMSCSICMDSPERSEC,			4,	U32_FT,		NotPersistant_PT,
		CT_STSPT_MINNUMSCSICMDSPERSEC,			4,	U32_FT,		NotPersistant_PT,

		CT_STSPT_AVGNUMBSABYTESREADPERSEC,		8,	S64_FT,		NotPersistant_PT,
		CT_STSPT_MAXNUMBSABYTESREADPERSEC,		8,	S64_FT,		NotPersistant_PT,
		CT_STSPT_MINNUMBSABYTESREADPERSEC,		8,	S64_FT,		NotPersistant_PT,

		CT_STSPT_AVGNUMBSABYTESWRITTENPERSEC,	8,	S64_FT,		NotPersistant_PT,
		CT_STSPT_MAXNUMBSABYTESWRITTENPERSEC,	8,	S64_FT,		NotPersistant_PT,
		CT_STSPT_MINNUMBSABYTESWRITTENPERSEC,	8,	S64_FT,		NotPersistant_PT,

		CT_STSPT_AVGNUMBSABYTESPERSEC,			8,	S64_FT,		NotPersistant_PT,
		CT_STSPT_MAXNUMBSABYTESPERSEC,			8,	S64_FT,		NotPersistant_PT,
		CT_STSPT_MINNUMBSABYTESPERSEC,			8,	S64_FT,		NotPersistant_PT

	};
		
//  size of field definition table, in bytes
const U32  cbSTSPerfTable_FieldDefs =  sizeof (aSTSPerfTable_FieldDefs);

