/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/FileTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// File Table.
// 
// $Log: /Gemini/Include/CTTables/FileTable.cpp $
// 
// 3     10/26/99 12:05p Joehler
// Correct size of variable entry
// 
// 2     10/12/99 11:11a Joehler
// Modifications for variable PTS entries
// 
// 1     9/30/99 7:42a Joehler
// First cut of File System Master and Upgrade Master
// 
//
/*************************************************************************/

#ifndef _DEBUG
#define _DEBUG    /* needed by CHECKFIELDDEFS(), otherwise harmless */
#endif

#include  "FileTable.h"

//  verify that field defs agree with record def
CHECKFIELDDEFS (FileRecord);


const fieldDef aFileTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName               Size  Type         Persist yes/no
	CPTS_RECORD_BASE_FIELDS(Persistant_PT),
	CT_FILE_TABLE_ENTRY,          8, BINARY_FT,    VarLength_PT | Persistant_PT
 };


//  size of field definition table, in bytes
const U32 cbFileTable_FieldDefs  =  sizeof (aFileTable_FieldDefs);

FileRecord::FileRecord(U8* file,
					   U32 cbFile) : CPtsRecordBase (sizeof (FileRecord),
											CT_FILE_TABLE_VER, 1)
{
	vfFile.Set(file, cbFile);
}  /* end of FileRecord::FileRecord */

//  here are the standard field defs for our row/table
/* static */
const fieldDef *FileRecord::FieldDefs (void)
{
   return (aFileTable_FieldDefs);
}

//  and here is the size, in bytes, of our field defs
/* static */
const U32 FileRecord::FieldDefsSize (void)
{
   return (cbFileTable_FieldDefs);
}

//  here is the name of the PTS table whose rows we define
/* static */
const char *FileRecord::TableName (void)
{
   return (CT_FILE_TABLE_NAME);
}


