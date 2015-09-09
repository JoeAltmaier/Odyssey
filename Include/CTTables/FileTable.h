/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This is the definition and declaration of the Card Status Record.
// 
// $Log: /Gemini/Include/CTTables/FileTable.h $
// 
// 1     9/30/99 7:42a Joehler
// First cut of File System Master and Upgrade Master
// 
// 
/*************************************************************************/

#ifndef _FileTable_h
#define _FileTable_h


#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif

#ifndef __RqPts_T_h
# include  "RqPts_T.h"
#endif


#pragma pack(4)      // standard packing for PTS things

//  table name
#define  CT_FILE_TABLE_NAME         "FileTable"

#define  CT_FILE_TABLE_VER          (1)     /* current struct version */


//
// FileRecord
//


// One entry for each File Entry in File System

class FileRecord : public CPtsRecordBase
{
public:
	CPtsVarField<U8> vfFile;

	FileRecord(U8* file, U32 cbFile);

	//  here are the standard field defs for our row/table
    static const fieldDef *FieldDefs (void);

    //  and here is the size, in bytes, of our field defs
    static const U32 FieldDefsSize (void);

    //  here is the name of the PTS table whose rows we define
    static const char *TableName (void);

    //  some PTS interface message typedefs
    typedef RqPtsDefineTable_T <FileRecord>   RqDefineTable;
    typedef RqPtsInsertRow_T   <FileRecord>   RqInsertRow;
    typedef RqPtsReadRow_T     <FileRecord>   RqReadRow;
	typedef RqPtsDeleteRow_T   <FileRecord>   RqDeleteRow;

};  /* end of class FileRecord */

//  compiler-checkable aliases for table / field names

//  field defs
#define  CT_FILE_TABLE_REC_VERSION        CT_PTS_VER_FIELD_NAME // Version of File record.
#define  CT_FILE_TABLE_SIZE               CT_PTS_SIZE_FIELD_NAME// # of bytes in record.
#define  CT_FILE_TABLE_ENTRY		 "Entry"           

//  here is the standard table which defines File table fields
extern const fieldDef aFileTable_FieldDefs[];

//  and here is the size, in bytes, of the File table field defs
extern const U32 cbFileTable_FieldDefs;


#endif  /* #ifndef _FileTable_h */

