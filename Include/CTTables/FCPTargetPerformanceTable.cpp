/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/FCPTargetPerformanceTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// Fibre Channel Target Performance table.
// 
// $Log: /Gemini/Include/CTTables/FCPTargetPerformanceTable.cpp $
// 
// 3     10/11/99 6:04p Vnguyen
// Add timestamp for Performance counters and # of errors received for
// Status counters.
// 
// 2     9/15/99 11:56a Vnguyen
// Update Performance Counters to match the counters being returned by the
// FCP Target driver.
// 
// 1     8/16/99 2:41p Vnguyen
// Initial Check-in.
// 
// 
/*************************************************************************/
#include  "FCPTargetPerformanceTable.h"

const fieldDef aFCPTargetPerformanceTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName             				  Size  Type        Persistance Type
//		CT_FCPTPT_RID,								8,	ROWID_FT,	Persistant_PT,		rowID - automatically supplied
		CT_FCPTPT_VERSION,						4,	U32_FT,		Persistant_PT,
		CT_FCPTPT_SIZE,							4,	U32_FT,		Persistant_PT,
		CT_FCPTPT_KEY,							4,	U32_FT,		NotPersistant_PT,
		CT_FCPTPT_REFRESHRATE,					4,	U32_FT,		Persistant_PT,
		CT_FCPTPT_SAMPLERATE,					4,	U32_FT,		Persistant_PT,
		CT_FCPTPT_RIDLDRECORD,					8,	ROWID_FT,	Persistant_PT,
		CT_FCPTPT_UPTIME,						8,	S64_FT,		NotPersistant_PT,
//
		CT_FCPTPT_AVGNUMREADPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
		CT_FCPTPT_MAXNUMREADPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
		CT_FCPTPT_MINNUMREADPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
	
		CT_FCPTPT_AVGNUMWRITEPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
		CT_FCPTPT_MAXNUMWRITEPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
		CT_FCPTPT_MINNUMWRITEPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
	
		CT_FCPTPT_AVGNUMRWPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
		CT_FCPTPT_MAXNUMRWPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
		CT_FCPTPT_MINNUMRWPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
	
		CT_FCPTPT_AVGNUMTOTALPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
		CT_FCPTPT_MAXNUMTOTALPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
		CT_FCPTPT_MINNUMTOTALPACKETSPERSEC,		4,	U32_FT,		NotPersistant_PT,
	
		CT_FCPTPT_AVGNUMBYTESREADPERSEC,		8,	S64_FT,		NotPersistant_PT,
		CT_FCPTPT_MAXNUMBYTESREADPERSEC,		8,	S64_FT,		NotPersistant_PT,
		CT_FCPTPT_MINNUMBYTESREADPERSEC,		8,	S64_FT,		NotPersistant_PT,
	
		CT_FCPTPT_AVGNUMBYTESWRITTENPERSEC,		8,	S64_FT,		NotPersistant_PT,
		CT_FCPTPT_MAXNUMBYTESWRITTENPERSEC,		8,	S64_FT,		NotPersistant_PT,
		CT_FCPTPT_MINNUMBYTESWRITTENPERSEC,		8,	S64_FT,		NotPersistant_PT,
	
		CT_FCPTPT_AVGNUMBYTESTRANSFERREDPERSEC,	8,	S64_FT,		NotPersistant_PT,
		CT_FCPTPT_MAXNUMBYTESTRANSFERREDPERSEC,	8,	S64_FT,		NotPersistant_PT,
		CT_FCPTPT_MINNUMBYTESTRANSFERREDPERSEC,	8,	S64_FT,		NotPersistant_PT,
		
		CT_FCPTPT_AVGMICROSECPERREAD,			8,	S64_FT,		NotPersistant_PT,
		CT_FCPTPT_MAXMICROSECPERREAD,			8,	S64_FT,		NotPersistant_PT,
		CT_FCPTPT_MINMICROSECPERREAD,			8,	S64_FT,		NotPersistant_PT,

		CT_FCPTPT_AVGMICROSECPERWRITE,			8,	S64_FT,		NotPersistant_PT,
		CT_FCPTPT_MAXMICROSECPERWRITE,			8,	S64_FT,		NotPersistant_PT,
		CT_FCPTPT_MINMICROSECPERWRITE,			8,	S64_FT,		NotPersistant_PT
	
	};
		
//  size of field definition table, in bytes
const U32  cbFCPTargetPerformanceTable_FieldDefs =  sizeof (aFCPTargetPerformanceTable_FieldDefs);

