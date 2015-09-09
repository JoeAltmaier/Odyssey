/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/DiskStatusTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// Disk Status table.
// 
// $Log: /Gemini/Include/CTTables/DiskStatusTable.cpp $
// 
// 5     12/14/99 3:47p Vnguyen
// Add missing table field for disk ECC error.  This sets the field data
// to zero for display.  Eventually, we can supply the correct value by
// reading the hard drive directly.
// 
// 4     11/22/99 4:08p Vnguyen
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
#include  "DiskStatusTable.h"


const fieldDef aDiskStatusTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName             						  Size  Type        Persistance Type
//		CT_DST_RID,										8,	ROWID_FT,	Persistant_PT,		rowID - automatically supplied
		CT_DST_VERSION,									4,	U32_FT,		Persistant_PT,
		CT_DST_SIZE,									4,	U32_FT,		Persistant_PT,
		CT_DST_KEY,										4,	U32_FT,		Persistant_PT,
		CT_DST_REFRESHRATE,								4,	U32_FT,		Persistant_PT,
		CT_DST_RIDSRCRECORD,							8,	ROWID_FT,	Persistant_PT,
		CT_DST_NUM_ERROR_REPLIES_RECEIVED,			 	4,	U32_FT,		Persistant_PT,
		CT_DST_NUM_RECOVERABLE_MEDIA_ERRORS_NO_DELAY,	4,	U32_FT,		Persistant_PT,
		CT_DST_NUM_RECOVERABLE_MEDIA_ERRORS_DELAY,		4,	U32_FT,		Persistant_PT,
		CT_DST_NUM_RECOVERABLE_MEDIA_ERRORS_BY_RETRY,	4,	U32_FT,		Persistant_PT,
		CT_DST_NUM_RECOVERABLE_MEDIA_ERRORS_BY_ECC,		4,	U32_FT,		Persistant_PT,
		CT_DST_NUM_RECOVERABLE_NONMEDIA_ERRORS,			4,	U32_FT,		Persistant_PT,
		CT_DST_NUM_BYTES_PROCESSED_TOTAL,				4,	U32_FT,		Persistant_PT,
		CT_DST_NUM_UNRECOVERABLE_MEDIA_ERRORS,			4,	U32_FT,		Persistant_PT
	};
		
//  size of field definition table, in bytes
const U32  cbDiskStatusTable_FieldDefs =  sizeof (aDiskStatusTable_FieldDefs);

