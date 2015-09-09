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
// $Log: /Gemini/Include/CTTables/FileDescriptorTable.h $
// 
// 2     1/26/00 2:26p Joehler
// Added a pad to the File Descriptor table to solve alignment problem.
// 
// 1     9/30/99 7:42a Joehler
// First cut of File System Master and Upgrade Master
// 
// 
/*************************************************************************/

#ifndef _FileDescriptorTable_h
#define _FileDescriptorTable_h


#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif

#ifndef __RqPts_T_h
# include  "RqPts_T.h"
#endif


#pragma pack(4)      // standard packing for PTS things

//  table name
#define  CT_FILE_DESC_TABLE_NAME         "FileDescriptorTable"

#define  CT_FILE_DESC_TABLE_VER          (1)     /* current struct version */


//
// FileRecord
//


// One entry for each File Entry in File System

class FileDescRecord : public CPtsRecordBase
{
public:
	U32	type;
	U32 cbFile;
	UnicodeString16	fileName;
	U32 pad;
	I64 creationDate;
	RowId fileKey;

	FileDescRecord(U32 type_, 
		U32 cbFile_, 
		UnicodeString16 fileName_,
		RowId fileKey_);

	FileDescRecord();
	void Clear();

	//  here are the standard field defs for our row/table
    static const fieldDef *FieldDefs (void);

    //  and here is the size, in bytes, of our field defs
    static const U32 FieldDefsSize (void);

    //  here is the name of the PTS table whose rows we define
    static const char *TableName (void);

    //  some PTS interface message typedefs
    typedef RqPtsDefineTable_T <FileDescRecord>   RqDefineTable;
    typedef RqPtsInsertRow_T   <FileDescRecord>   RqInsertRow;
    typedef RqPtsReadRow_T     <FileDescRecord>   RqReadRow;
	typedef RqPtsDeleteRow_T   <FileDescRecord>   RqDeleteRow;
	typedef RqPtsEnumerateTable_T <FileDescRecord> RqEnumTable;

};  /* end of class FileDescRecord */

//  compiler-checkable aliases for table / field names

//  field defs
#define CT_FDT_REC_VERSION		CT_PTS_VER_FIELD_NAME // Version of File Descriptor record.
#define CT_FDT_SIZE				CT_PTS_SIZE_FIELD_NAME// # of bytes in record.
#define CT_FDT_TYPE				"Type"        
#define	CT_FDT_CBFILE			"CbFile"
#define CT_FDT_FILENAME			"FileName"
#define CT_FDT_PAD				"Pad"
#define CT_FDT_CREATION_DATE	"CreationDate"
#define	CT_FDT_FILEKEY			"FileKey"

//  here is the standard table which defines File Descriptor table fields
extern const fieldDef aFileDescTable_FieldDefs[];

//  and here is the size, in bytes, of the File Descriptor table field defs
extern const U32 cbFileDescTable_FieldDefs;


#endif  /* #ifndef _FileTable_h */

