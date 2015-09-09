/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/SSDPerformanceTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// SSD Performance table.
// 
// $Log: /Gemini/Include/CTTables/SSDPerformanceTable.cpp $
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
#include  "SSDPerformanceTable.h"


const fieldDef aSSDPerformanceTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName             				  Size  Type        Persistance Type
//		CT_SSDPT_RID,							8,	ROWID_FT,	Persistant_PT,		rowID - automatically supplied
		CT_SSDPT_VERSION,						4,	U32_FT,		Persistant_PT,
		CT_SSDPT_SIZE,							4,	U32_FT,		Persistant_PT,
		CT_SSDPT_KEY,							4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_REFRESHRATE,					4,	U32_FT,		Persistant_PT,
		CT_SSDPT_SAMPLERATE,					4,	U32_FT,		Persistant_PT,
		CT_SSDPT_RIDSRCRECORD,					8,	ROWID_FT,	Persistant_PT,
		CT_SSDPT_UPTIME,						8,	S64_FT,		NotPersistant_PT,

		CT_SSDPT_AVGNUMPAGESREADPERSEC,			4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MAXNUMPAGESREADPERSEC,			4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MINNUMPAGESREADPERSEC,			4,	U32_FT,		NotPersistant_PT,

		CT_SSDPT_AVGNUMPAGESREADCACHEHITPERSEC,	4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MAXNUMPAGESREADCACHEHITPERSEC,	4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MINNUMPAGESREADCACHEHITPERSEC,	4,	U32_FT,		NotPersistant_PT,

		CT_SSDPT_AVGNUMPAGESREADCACHEMISSPERSEC,4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MAXNUMPAGESREADCACHEMISSPERSEC,4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MINNUMPAGESREADCACHEMISSPERSEC,4,	U32_FT,		NotPersistant_PT,
	
	
		CT_SSDPT_AVGNUMPAGESWRITEPERSEC,		4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MAXNUMPAGESWRITEPERSEC,		4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MINNUMPAGESWRITEPERSEC,		4,	U32_FT,		NotPersistant_PT,
	
		CT_SSDPT_AVGNUMPAGESWRITECACHEHITPERSEC,4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MAXNUMPAGESWRITECACHEHITPERSEC,4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MINNUMPAGESWRITECACHEHITPERSEC,4,	U32_FT,		NotPersistant_PT,

		CT_SSDPT_AVGNUMPAGESWRITECACHEMISSPERSEC,4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MAXNUMPAGESWRITECACHEMISSPERSEC,4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MINNUMPAGESWRITECACHEMISSPERSEC,4,	U32_FT,		NotPersistant_PT,
	
		CT_SSDPT_AVGNUMERASEPAGESAVAILABLE, 	4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MAXNUMERASEPAGESAVAILABLE, 	4,	U32_FT,		NotPersistant_PT,
		CT_SSDPT_MINNUMERASEPAGESAVAILABLE, 	4,	U32_FT,		NotPersistant_PT,

		CT_SSDPT_AVGNUMREADBYTESTOTALPERSEC,	8,	S64_FT,		NotPersistant_PT,
		CT_SSDPT_MAXNUMREADBYTESTOTALPERSEC,	8,	S64_FT,		NotPersistant_PT,
		CT_SSDPT_MINNUMREADBYTESTOTALPERSEC,	8,	S64_FT,		NotPersistant_PT,	

		CT_SSDPT_AVGNUMWRITEBYTESTOTALPERSEC,	8,	S64_FT,		NotPersistant_PT,
		CT_SSDPT_MAXNUMWRITEBYTESTOTALPERSEC,	8,	S64_FT,		NotPersistant_PT,
		CT_SSDPT_MINNUMWRITEBYTESTOTALPERSEC,	8,	S64_FT,		NotPersistant_PT	

	};
		
//  size of field definition table, in bytes
const U32  cbSSDPerformanceTable_FieldDefs =  sizeof (aSSDPerformanceTable_FieldDefs);

