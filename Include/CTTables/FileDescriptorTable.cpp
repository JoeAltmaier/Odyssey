/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/FileDescriptorTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// File Table.
// 
// $Log: /Gemini/Include/CTTables/FileDescriptorTable.cpp $
// 
// 3     1/26/00 2:26p Joehler
// Added a pad to the File Descriptor table to solve alignment problem and
// set creationDate.
// 
// 2     10/26/99 12:04p Joehler
// Removed multiple defines of DEBUG
// 
// 1     9/30/99 7:42a Joehler
// First cut of File System Master and Upgrade Master
// 
//
/*************************************************************************/

#ifndef _DEBUG
#define _DEBUG    /* needed by CHECKFIELDDEFS(), otherwise harmless */
#endif

#include  "FileDescriptorTable.h"


//  verify that field defs agree with record def
CHECKFIELDDEFS (FileDescRecord);


const fieldDef aFileDescTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName               Size  Type         Persist yes/no

   CPTS_RECORD_BASE_FIELDS (Persistant_PT),
   CT_FDT_TYPE,          0, U32_FT,      Persistant_PT,
   CT_FDT_CBFILE,		 0, U32_FT,		 Persistant_PT,
   CT_FDT_FILENAME,		32, BINARY_FT,	 Persistant_PT,
   CT_FDT_PAD, 			0, U32_FT,		 Persistant_PT,
   CT_FDT_CREATION_DATE, 0, S64_FT,		 Persistant_PT,
   CT_FDT_FILEKEY,		 0, ROWID_FT,		 Persistant_PT
 };


//  size of field definition table, in bytes
const U32 cbFileDescTable_FieldDefs  =  sizeof (aFileDescTable_FieldDefs);

FileDescRecord::FileDescRecord(U32 type_,
							   U32 cbFile_,
							   UnicodeString16 fileName_,
							   RowId fileKey_) 
							   : CPtsRecordBase (sizeof (FileDescRecord),
												  CT_FILE_DESC_TABLE_VER),
								type(type_), cbFile(cbFile_), fileKey(fileKey_)
{
	creationDate = Kernel::Time_Stamp();
	memcpy(fileName, fileName_, sizeof(fileName));
}  /* end of FileDescRecord::FileDescRecord */

FileDescRecord::FileDescRecord() : CPtsRecordBase (sizeof (FileDescRecord),
												  CT_FILE_DESC_TABLE_VER)
{
   Clear ();
}  /* end of FileDescRecord::FileDescRecord */

void  FileDescRecord::Clear()
{
	type = 0;
	cbFile = 0;
	memset(fileName, 0, sizeof(UnicodeString16));
	creationDate = Kernel::Time_Stamp();
	fileKey = 0;
}  /* end of FileRecord::Clear */


//  here are the standard field defs for our row/table
/* static */
const fieldDef *FileDescRecord::FieldDefs (void)
{
   return (aFileDescTable_FieldDefs);
}

//  and here is the size, in bytes, of our field defs
/* static */
const U32 FileDescRecord::FieldDefsSize (void)
{
   return (cbFileDescTable_FieldDefs);
}

//  here is the name of the PTS table whose rows we define
/* static */
const char *FileDescRecord::TableName (void)
{
   return (CT_FILE_DESC_TABLE_NAME);
}


