/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/DefaultImageTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// Default Image Table.
// 
// $Log: /Gemini/Include/CTTables/DefaultImageTable.cpp $
// 
// 4     11/21/99 4:19p Jlane
// Added image hdr fields from the IMageDescRecord.
// 
// 3     10/26/99 12:03p Joehler
// Removed multiple defines of DEBUG
// 
// 2     10/12/99 11:10a Joehler
// Modified to use ImageType enum
// 
// 1     9/30/99 7:42a Joehler
// First cut of File System Master and Upgrade Master
// 
//
/*************************************************************************/

#ifndef _DEBUG
#define _DEBUG    /* needed by CHECKFIELDDEFS(), otherwise harmless */
#endif

#include  "DefaultImageTable.h"

//  verify that field defs agree with record def
CHECKFIELDDEFS (DefaultImageRecord);


const fieldDef aDefImageTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName               Size  Type         Persist yes/no

   CPTS_RECORD_BASE_FIELDS (Persistant_PT),
   CT_DEF_IMAGE_TABLE_MAJORVERSION, 0, U32_FT,      Persistant_PT,
   CT_DEF_IMAGE_TABLE_MINORVERSION,	0, U32_FT,		Persistant_PT,
   CT_DEF_IMAGE_TABLE_DAY,          0, U32_FT,      Persistant_PT,
   CT_DEF_IMAGE_TABLE_MONTH,		0, U32_FT,		Persistant_PT,
   CT_DEF_IMAGE_TABLE_YEAR,         0, U32_FT,      Persistant_PT,
   CT_DEF_IMAGE_TABLE_HOUR,			0, U32_FT,		Persistant_PT,
   CT_DEF_IMAGE_TABLE_MINUTE,       0, U32_FT,      Persistant_PT,
   CT_DEF_IMAGE_TABLE_SECOND,		0, U32_FT,		Persistant_PT,   
   CT_DEF_IMAGE_TABLE_TYPE,         0, U32_FT,      Persistant_PT,
   CT_DEF_IMAGE_TABLE_IMAGEKEY,		0, ROWID_FT,	Persistant_PT
 };


//  size of field definition table, in bytes
const U32 cbDefImageTable_FieldDefs  =  sizeof (aDefImageTable_FieldDefs);

DefaultImageRecord::DefaultImageRecord(
		U32 majorVersion_,
		U32 minorVersion_,
		U32 day_,
		U32 month_,
		U32 year_,
		U32 hour_,
		U32 minute_,
		U32 second_,
		ImageType type_,
		RowId imageKey_) 
: CPtsRecordBase (sizeof (DefaultImageRecord), CT_DEF_IMAGE_TABLE_VER)
{
	majorVersion = majorVersion_,
	minorVersion = minorVersion_,
	day = day_,
	month = month_,
	year = year_,
	hour = hour_,
	minute = minute_,
	second = second_,
	type = type_;
	imageKey = imageKey_;
}  /* end of DefaultImageRecord::DefaultImageRecord()*/


//  here are the standard field defs for our row/table
/* static */
const fieldDef *DefaultImageRecord::FieldDefs (void)
{
   return (aDefImageTable_FieldDefs);
}

//  and here is the size, in bytes, of our field defs
/* static */
const U32 DefaultImageRecord::FieldDefsSize (void)
{
   return (cbDefImageTable_FieldDefs);
}

//  here is the name of the PTS table whose rows we define
/* static */
const char *DefaultImageRecord::TableName (void)
{
   return (CT_DEF_IMAGE_TABLE_NAME);
}


