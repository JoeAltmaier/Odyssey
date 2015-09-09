/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/ImageDescriptorTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// Image Descriptor Table.
// 
// $Log: /Gemini/Include/CTTables/ImageDescriptorTable.cpp $
// 
// 5     1/26/00 2:26p Joehler
// Added a pad to the Image Descriptor table to solve alignment problem.
// 
// 4     11/17/99 3:20p Joehler
// Added handle for failsafeness
// 
// 3     10/26/99 12:07p Joehler
// removed redefinition of DEBUG
// 
// 2     10/12/99 11:11a Joehler
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

#include  "ImageDescriptorTable.h"

//  verify that field defs agree with record def
CHECKFIELDDEFS (ImageDescRecord);


const fieldDef aImageDescTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName               Size  Type         Persist yes/no

   CPTS_RECORD_BASE_FIELDS (Persistant_PT),
   CT_IDT_HANDLE,	 0, ROWID_FT,		 Persistant_PT,
   CT_IDT_MAJORVERSION,          0, U32_FT,      Persistant_PT,
   CT_IDT_MINORVERSION,		 0, U32_FT,		 Persistant_PT,
   CT_IDT_DAY,          0, U32_FT,      Persistant_PT,
   CT_IDT_MONTH,		0, U32_FT,		 Persistant_PT,
   CT_IDT_YEAR,          0, U32_FT,      Persistant_PT,
   CT_IDT_HOUR,		 0, U32_FT,		 Persistant_PT,
   CT_IDT_MINUTE,          0, U32_FT,      Persistant_PT,
   CT_IDT_SECOND,		 0, U32_FT,		 Persistant_PT,
   CT_IDT_TYPE,          0, U32_FT,      Persistant_PT,
   CT_IDT_IOPCOUNT,		 0, U32_FT,		 Persistant_PT,
   CT_IDT_FILEDESCKEY,   0, ROWID_FT,	 Persistant_PT,
   CT_IDT_PAD, 			 0, U32_FT, 	 Persistant_PT
 };


//  size of field definition table, in bytes
const U32 cbImageDescTable_FieldDefs  =  sizeof (aImageDescTable_FieldDefs);

ImageDescRecord::ImageDescRecord(U32 majorVersion_,
		U32 minorVersion_,
		U32 day_,
		U32 month_,
		U32 year_,
		U32 hour_,
		U32 minute_,
		U32 second_,
		ImageType type_,
		RowId fileDescKey_) : CPtsRecordBase (sizeof (ImageDescRecord),
									 CT_IMAGE_DESC_TABLE_VER)
{
	handle = 0;
	minorVersion = minorVersion_;
	majorVersion = majorVersion_;
	day = day_;
	month = month_;
	year = year_;
	hour = hour_;
	minute = minute_;
	second = second_;
	type = type_;
	fileDescKey = fileDescKey_;
	iopCount = 0;
	pad = 0;
}


//  here are the standard field defs for our row/table
/* static */
const fieldDef *ImageDescRecord::FieldDefs (void)
{
   return (aImageDescTable_FieldDefs);
}

//  and here is the size, in bytes, of our field defs
/* static */
const U32 ImageDescRecord::FieldDefsSize (void)
{
   return (cbImageDescTable_FieldDefs);
}

//  here is the name of the PTS table whose rows we define
/* static */
const char *ImageDescRecord::TableName (void)
{
   return (CT_IMAGE_DESC_TABLE_NAME);
}


