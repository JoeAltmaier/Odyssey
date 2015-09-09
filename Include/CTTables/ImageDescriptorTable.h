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
// $Log: /Gemini/Include/CTTables/ImageDescriptorTable.h $
// 
// 4     1/26/00 2:26p Joehler
// Added a pad to the Image Descriptor table to solve alignment problem.
// 
// 3     11/17/99 3:21p Joehler
// Add handle for failsafeness
// 
// 2     10/12/99 11:11a Joehler
// Modified to use ImageType enum
// 
// 1     9/30/99 7:42a Joehler
// First cut of File System Master and Upgrade Master
// 
// 
/*************************************************************************/

#ifndef _ImageDescriptorTable_h
#define _ImageDescriptorTable_h

#include "UpgradeImageType.h"

#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif

#ifndef __RqPts_T_h
# include  "RqPts_T.h"
#endif


#pragma pack(4)      // standard packing for PTS things

//  table name
#define  CT_IMAGE_DESC_TABLE_NAME         "ImageDescriptorTable"

#define  CT_IMAGE_DESC_TABLE_VER          (1)     /* current struct version */


//
// ImageDescRecord
//


// One entry for each Image in System

class ImageDescRecord : public CPtsRecordBase
{
public:
	RowId handle;
	U32 majorVersion;
	U32 minorVersion;
	U32 day;
	U32 month;
	U32 year;
	U32 hour;
	U32 minute;
	U32 second;
	ImageType type;
	U32 iopCount;
	RowId fileDescKey;
	U32 pad;

	ImageDescRecord(U32 majorVersion_,
		U32 minorVersion_,
		U32 day_,
		U32 month_,
		U32 year_,
		U32 hour_,
		U32 minute_,
		U32 second_,
		ImageType type_,
		RowId fileDescKey_);

	//  here are the standard field defs for our row/table
    static const fieldDef *FieldDefs (void);

    //  and here is the size, in bytes, of our field defs
    static const U32 FieldDefsSize (void);

    //  here is the name of the PTS table whose rows we define
    static const char *TableName (void);

    //  some PTS interface message typedefs
    typedef RqPtsDefineTable_T <ImageDescRecord>   RqDefineTable;
    typedef RqPtsInsertRow_T   <ImageDescRecord>   RqInsertRow;
    typedef RqPtsReadRow_T     <ImageDescRecord>   RqReadRow;
	typedef RqPtsModifyRow_T <ImageDescRecord>	   RqModifyRow;
	typedef RqPtsModifyField_T <ImageDescRecord>   RqModifyField;
	typedef RqPtsDeleteRow_T   <ImageDescRecord>   RqDeleteRow;
	typedef RqPtsEnumerateTable_T <ImageDescRecord> RqEnumTable;

};  /* end of class ImageDescRecord */

//  compiler-checkable aliases for table / field names

//  field defs
#define CT_IDT_REC_VERSION		CT_PTS_VER_FIELD_NAME // Version of File Descriptor record.
#define CT_IDT_SIZE				CT_PTS_SIZE_FIELD_NAME// # of bytes in record.
#define	CT_IDT_HANDLE	"handle"
#define CT_IDT_MAJORVERSION		"majorVersion"
#define CT_IDT_MINORVERSION		"minorVersion"
#define CT_IDT_DAY				"day"
#define CT_IDT_MONTH			"month"
#define CT_IDT_YEAR				"year"
#define CT_IDT_HOUR				"hour"
#define CT_IDT_MINUTE			"minute"
#define CT_IDT_SECOND			"second"
#define CT_IDT_TYPE				"type"
#define	CT_IDT_IOPCOUNT			"iopCount"
#define CT_IDT_FILEDESCKEY	    "fileDescKey"
#define CT_IDT_PAD				"pad"

//  here is the standard table which defines Image Descriptor table fields
extern const fieldDef aImageDescTable_FieldDefs[];

//  and here is the size, in bytes, of the Image Descriptor table field defs
extern const U32 cbImageDescTable_FieldDefs;


#endif  /* #ifndef _ImageDescriptorTable_h */

