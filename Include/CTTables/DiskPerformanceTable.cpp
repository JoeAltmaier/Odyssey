/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/DiskPerformanceTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// Disk Performance table.
// 
// $Log: /Gemini/Include/CTTables/DiskPerformanceTable.cpp $
// 
// 5     1/04/00 4:31p Jlane
// Pad to a multiple of 8 bytes and pad so I64s are 8-byte aligned.
// 
// 4     11/22/99 4:05p Vnguyen
// Change key from I64 to U32 to be consistent with other reporters.
// 
// 3     9/08/99 10:46a Vnguyen
// Update performance and status counters.  Also, change performance
// counters from per sample to per second.
// 
// 2     8/16/99 12:54p Vnguyen
// Fix various errors during compiling.  Mostly typo and mis-match
// parameters.
// 
// 1     5/04/99 9:36a Jlane
// Initial Checkin.
// 
/*************************************************************************/
#include  "DiskPerformanceTable.h"


const fieldDef aDiskPerformanceTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName             				  Size  Type        Persistance Type
//		CT_DPT_RID,								8,	ROWID_FT,	Persistant_PT,		rowID - automatically supplied
		CT_DPT_VERSION,							4,	U32_FT,		Persistant_PT,
		CT_DPT_SIZE,							4,	U32_FT,		Persistant_PT,
		CT_DPT_KEY,								4,	U32_FT,		Persistant_PT,
		CT_DPT_REFRESHRATE,						4,	U32_FT,		Persistant_PT,
		CT_DPT_SAMPLERATE,						4,	U32_FT,		Persistant_PT,
		CT_DPT_U32PAD1,							4,	U32_FT,		Persistant_PT,
		CT_DPT_RIDSRCRECORD,					8,	ROWID_FT,	Persistant_PT,
		CT_DPT_UPTIME,							8,	S64_FT,		Persistant_PT,
		
		
		CT_DPT_AVGREADSPERSEC,					4,	U32_FT,		Persistant_PT,
		CT_DPT_MAXREADSPERSEC,					4,	U32_FT,		Persistant_PT,
		CT_DPT_MINREADSPERSEC,					4,	U32_FT,		Persistant_PT,
	
		CT_DPT_AVGWRITESPERSEC,					4,	U32_FT,		Persistant_PT,
		CT_DPT_MAXWRITESPERSEC,					4,	U32_FT,		Persistant_PT,
		CT_DPT_MINWRITESPERSEC,					4,	U32_FT,		Persistant_PT,
	
		CT_DPT_AVGTRANSFERPERSEC,				4,	U32_FT,		Persistant_PT,
		CT_DPT_MAXTRANSFERPERSEC,				4,	U32_FT,		Persistant_PT,
		CT_DPT_MINTRANSFERPERSEC,				4,	U32_FT,		Persistant_PT,
		CT_DPT_U32PAD2,							4,	U32_FT,		Persistant_PT,
//
		CT_DPT_AVGBYTESREADPERSEC,				8,	S64_FT,		Persistant_PT,
		CT_DPT_MAXBYTESREADPERSEC,				8,	S64_FT,		Persistant_PT,
		CT_DPT_MINBYTESREADPERSEC,				8,	S64_FT,		Persistant_PT,

		CT_DPT_AVGBYTESWRITTENPERSEC,			8,	S64_FT,		Persistant_PT,
		CT_DPT_MAXBYTESWRITTENPERSEC,			8,	S64_FT,		Persistant_PT,
		CT_DPT_MINBYTESWRITTENPERSEC,			8,	S64_FT,		Persistant_PT,

		CT_DPT_AVGBYTESTRANSFERREDPERSEC,		8,	S64_FT,		Persistant_PT,
		CT_DPT_MAXBYTESTRANSFERREDPERSEC,		8,	S64_FT,		Persistant_PT,
		CT_DPT_MINBYTESTRANSFERREDPERSEC,		8,	S64_FT,		Persistant_PT,
//
		CT_DPT_AVGREADSIZE,						4,	U32_FT,		Persistant_PT,
		CT_DPT_MAXREADSIZE,						4,	U32_FT,		Persistant_PT,
		CT_DPT_MINREADSIZE,						4,	U32_FT,		Persistant_PT,

		CT_DPT_AVGWRITESIZE,					4,	U32_FT,		Persistant_PT,
		CT_DPT_MAXWRITESIZE,					4,	U32_FT,		Persistant_PT,
		CT_DPT_MINWRITESIZE,					4,	U32_FT,		Persistant_PT,

		CT_DPT_AVGTRANSFERSIZE,					4,	U32_FT,		Persistant_PT,
		CT_DPT_MAXTRANSFERSIZE,					4,	U32_FT,		Persistant_PT,
		CT_DPT_MINTRANSFERSIZE,					4,	U32_FT,		Persistant_PT,
		CT_DPT_U32PAD3,							4,	U32_FT,		Persistant_PT,
//
		CT_DPT_AVGMICROSECPERREAD,				8,	S64_FT,		Persistant_PT,
		CT_DPT_MAXMICROSECPERREAD,				8,	S64_FT,		Persistant_PT,
		CT_DPT_MINMICROSECPERREAD,				8,	S64_FT,		Persistant_PT,

		CT_DPT_AVGMICROSECPERWRITE,				8,	S64_FT,		Persistant_PT,
		CT_DPT_MAXMICROSECPERWRITE,				8,	S64_FT,		Persistant_PT,
		CT_DPT_MINMICROSECPERWRITE,				8,	S64_FT,		Persistant_PT,

		CT_DPT_AVGMICROSECPERTRANSFER,			8,	S64_FT,		Persistant_PT,
		CT_DPT_MAXMICROSECPERTRANSFER,			8,	S64_FT,		Persistant_PT,
		CT_DPT_MINMICROSECPERTRANSFER,			8,	S64_FT,		Persistant_PT
	};
		
//  size of field definition table, in bytes
const U32  cbDiskPerformanceTable_FieldDefs =  sizeof (aDiskPerformanceTable_FieldDefs);

