/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/IOPImageTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// IOP Image Table.
// 
// $Log: /Gemini/Include/CTTables/IOPImageTable.cpp $
// 
// 5     1/26/00 2:27p Joehler
// Added imageState to IOP Image Table.
// 
// 4     11/17/99 3:21p Joehler
// Add handle for failsafeness
// 
// 3     10/28/99 9:25a Sgavarre
// Add padding so that timestamp is eight byte aligned.
// 
// 2     10/26/99 12:08p Joehler
// Removed trial image from table
// 
// 1     9/30/99 7:42a Joehler
// First cut of File System Master and Upgrade Master
// 
//
/*************************************************************************/

#ifndef _DEBUG
#define _DEBUG    /* needed by CHECKFIELDDEFS(), otherwise harmless */
#endif

#include  "IOPImageTable.h"

//  verify that field defs agree with record def
CHECKFIELDDEFS (IOPImageRecord);


const fieldDef aIOPImageTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName               Size  Type         Persist yes/no

   CPTS_RECORD_BASE_FIELDS (Persistant_PT),
   CT_IOP_IMAGE_TABLE_HANDLE,        0, ROWID_FT,    Persistant_PT,
   CT_IOP_IMAGE_TABLE_SLOT,          0, U32_FT,      Persistant_PT,
   CT_IOP_IMAGE_TABLE_CURRENT,       0, ROWID_FT,    Persistant_PT,
   CT_IOP_IMAGE_TABLE_PRIMARY,       0, ROWID_FT,    Persistant_PT,
   CT_IOP_IMAGE_TABLE_IMAGEONE,      0, ROWID_FT,    Persistant_PT,
   CT_IOP_IMAGE_TABLE_IMAGETWO,      0, ROWID_FT,    Persistant_PT,
   CT_IOP_IMAGE_TABLE_IMAGEONEACC,   0, BOOL_FT,	 Persistant_PT,
   CT_IOP_IMAGE_TABLE_IMAGETWOACC,   0, BOOL_FT,	 Persistant_PT,
   CT_IOP_IMAGE_TABLE_IMAGESTATE,    0, U32_FT,      Persistant_PT,
   CT_IOP_IMAGE_TABLE_PAD,    		 0, U32_FT,      Persistant_PT,
   CT_IOP_IMAGE_TABLE_TIMEBOOTED,	 0, S64_FT,		 Persistant_PT
};


//  size of field definition table, in bytes
const U32 cbIOPImageTable_FieldDefs  =  sizeof (aIOPImageTable_FieldDefs);


IOPImageRecord::IOPImageRecord() 
: CPtsRecordBase (sizeof (IOPImageRecord), CT_IOP_IMAGE_TABLE_VER)
{
	Clear();
}  /* end of IOPImageRecord::IOPImageRecord */

void  IOPImageRecord::Clear()
{
	//slot = 0;
	//currentImage = 0;
	//primaryImage = 0;
	//imageOne = 0;
	//imageTwo = 0;
	handle = 0;
	imageOneAccepted = FALSE;
	imageTwoAccepted = FALSE;
	timeBooted = 0;
	imageState = eImageState_Uninitialized;
}  /* end of IOPImageRecord::Clear */

//  here are the standard field defs for our row/table
/* static */
const fieldDef *IOPImageRecord::FieldDefs (void)
{
   return (aIOPImageTable_FieldDefs);
}

//  and here is the size, in bytes, of our field defs
/* static */
const U32 IOPImageRecord::FieldDefsSize (void)
{
   return (cbIOPImageTable_FieldDefs);
}

//  here is the name of the PTS table whose rows we define
/* static */
const char *IOPImageRecord::TableName (void)
{
   return (CT_IOP_IMAGE_TABLE_NAME);
}


