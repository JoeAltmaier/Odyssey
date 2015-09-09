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
// $Log: /Gemini/Include/CTTables/DefaultImageTable.h $
// 
// 4     11/21/99 4:20p Jlane
// Added image hdr fields from the IMageDescRecord.
// 
// 3     11/17/99 3:20p Joehler
// Move image types to ImageType.h
// 
// 2     10/12/99 11:10a Joehler
// Modified to use ImageType enum
// 
// 1     9/30/99 7:42a Joehler
// First cut of File System Master and Upgrade Master
// 
// 
/*************************************************************************/

#ifndef _DefaultImageTable_h
#define _DefaultImageTable_h

#include "UpgradeImageType.h"

#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif

#ifndef __RqPts_T_h
# include  "RqPts_T.h"
#endif

#pragma pack(4)      // standard packing for PTS things

//  table name
#define  CT_DEF_IMAGE_TABLE_NAME         "DefaultImageTable"

#define  CT_DEF_IMAGE_TABLE_VER          (1)     /* current struct version */


//
// DefaultImageRecord
//


// One entry for each board type in the system

class DefaultImageRecord : public CPtsRecordBase
{
public:
	U32 majorVersion;
	U32 minorVersion;
	U32 day;
	U32 month;
	U32 year;
	U32 hour;
	U32 minute;
	U32 second;
	ImageType type;
	RowId imageKey; // key into image desc table
	
	DefaultImageRecord(
		U32 majorVersion_,
		U32 minorVersion_,
		U32 day_,
		U32 month_,
		U32 year_,
		U32 hour_,
		U32 minute_,
		U32 second_,
		ImageType type,
		RowId imageKey);

	//  here are the standard field defs for our row/table
    static const fieldDef *FieldDefs (void);

    //  and here is the size, in bytes, of our field defs
    static const U32 FieldDefsSize (void);

    //  here is the name of the PTS table whose rows we define
    static const char *TableName (void);

    //  some PTS interface message typedefs
    typedef RqPtsDefineTable_T <DefaultImageRecord>   RqDefineTable;
    typedef RqPtsInsertRow_T   <DefaultImageRecord>   RqInsertRow;
    typedef RqPtsReadRow_T     <DefaultImageRecord>   RqReadRow;
	typedef RqPtsEnumerateTable_T <DefaultImageRecord> RqEnumTable;
	typedef RqPtsModifyField_T <DefaultImageRecord>   RqModifyField;
	typedef RqPtsModifyRow_T <DefaultImageRecord>   RqModifyRow;

};  /* end of class DefaultImageRecord */

//  compiler-checkable aliases for table / field names

//  field defs
#define  CT_DEF_IMAGE_TABLE_REC_VERSION     CT_PTS_VER_FIELD_NAME // Version of File record.
#define  CT_DEF_IMAGE_TABLE_SIZE            CT_PTS_SIZE_FIELD_NAME// # of bytes in record.
#define  CT_DEF_IMAGE_TABLE_MAJORVERSION	"majorVersion"
#define  CT_DEF_IMAGE_TABLE_MINORVERSION	"minorVersion"
#define  CT_DEF_IMAGE_TABLE_DAY				"day"
#define  CT_DEF_IMAGE_TABLE_MONTH			"month"
#define  CT_DEF_IMAGE_TABLE_YEAR			"year"
#define  CT_DEF_IMAGE_TABLE_HOUR			"hour"
#define  CT_DEF_IMAGE_TABLE_MINUTE			"minute"
#define  CT_DEF_IMAGE_TABLE_SECOND			"second"
#define  CT_DEF_IMAGE_TABLE_CHKSUM			"chksum"
#define  CT_DEF_IMAGE_TABLE_TYPE		 	"Type"           
#define	 CT_DEF_IMAGE_TABLE_IMAGEKEY		"ImageKey"

//  here is the standard table which defines File table fields
extern const fieldDef aDefImageTable_FieldDefs[];

//  and here is the size, in bytes, of the File table field defs
extern const U32 cbDefImageTable_FieldDefs;


#endif  /* #ifndef _DefaultImageTable_h */

