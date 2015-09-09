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
// $Log: /Gemini/Include/CTTables/IOPImageTable.h $
// 
// 6     1/26/00 2:27p Joehler
// Added imageState to the IOP Image Table.
// 
// 5     11/17/99 3:21p Joehler
// Add handle for failsafeness
// 
// 4     10/28/99 9:25a Sgavarre
// Add padding so that timestamp is eight byte aligned.
// 
// 3     10/26/99 12:08p Joehler
// Removed trial image from table
// 
// 2     10/12/99 11:12a Joehler
// Added RqEnumTable
// 
// 1     9/30/99 7:42a Joehler
// First cut of File System Master and Upgrade Master
// 
// 
/*************************************************************************/

#ifndef _IOPImageTable_h
#define _IOPImageTable_h


#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif

#ifndef __RqPts_T_h
# include  "RqPts_T.h"
#endif


#pragma pack(4)      // standard packing for PTS things

//  table name
#define  CT_IOP_IMAGE_TABLE_NAME         "IOPImageTable"

#define  CT_IOP_IMAGE_TABLE_VER          (1)     /* current struct version */


//
// IOPImageRecord
//


// eImageState - This enum defines that state of the image table record itself.
enum eImageState {
	eImageState_Uninitialized	= 0,	// This iop image table record is uninitialized.
	eImageState_Initialized		= 1		// This Iop image table record is initialized.
	};
 
// One entry for each Image in the system	
class IOPImageRecord : public CPtsRecordBase
{
public:
	RowId		handle;
	TySlot		slot;
	RowId		currentImage;
	RowId		primaryImage;
	RowId		imageOne;
	RowId		imageTwo;
	BOOL		imageOneAccepted;
	BOOL		imageTwoAccepted;
	U32			imageState;
	U32			pad;
	I64			timeBooted;

	IOPImageRecord();
	void Clear();

	//  here are the standard field defs for our row/table
    static const fieldDef *FieldDefs (void);

    //  and here is the size, in bytes, of our field defs
    static const U32 FieldDefsSize (void);

    //  here is the name of the PTS table whose rows we define
    static const char *TableName (void);

    //  some PTS interface message typedefs
    typedef RqPtsDefineTable_T <IOPImageRecord>   RqDefineTable;
    typedef RqPtsInsertRow_T   <IOPImageRecord>   RqInsertRow;
    typedef RqPtsReadRow_T     <IOPImageRecord>   RqReadRow;
	typedef RqPtsDeleteRow_T   <IOPImageRecord>   RqDeleteRow;
	typedef RqPtsModifyField_T   <IOPImageRecord>   RqModifyField;
	typedef RqPtsModifyRow_T   <IOPImageRecord>   RqModifyRow;
	typedef RqPtsEnumerateTable_T <IOPImageRecord> RqEnumTable;

};  /* end of class FileRecord */

//  compiler-checkable aliases for table / field names

//  field defs
#define  CT_FILE_TABLE_REC_VERSION        CT_PTS_VER_FIELD_NAME // Version of File record.
#define  CT_FILE_TABLE_SIZE               CT_PTS_SIZE_FIELD_NAME// # of bytes in record.
#define	 CT_IOP_IMAGE_TABLE_HANDLE		"Handle"
#define	 CT_IOP_IMAGE_TABLE_SLOT		"Slot"
#define	 CT_IOP_IMAGE_TABLE_CURRENT		"CurrentImage"
#define	 CT_IOP_IMAGE_TABLE_PRIMARY	    "PrimaryImage"
#define	 CT_IOP_IMAGE_TABLE_IMAGEONE    "ImageOne"
#define	 CT_IOP_IMAGE_TABLE_IMAGETWO	"ImageTwo"
#define  CT_IOP_IMAGE_TABLE_IMAGEONEACC "ImageOneAccepted"
#define  CT_IOP_IMAGE_TABLE_IMAGETWOACC "ImageTwoAccepted"
#define  CT_IOP_IMAGE_TABLE_IMAGESTATE	"ImageState"
#define  CT_IOP_IMAGE_TABLE_PAD			"Pad"
#define  CT_IOP_IMAGE_TABLE_TIMEBOOTED	"TimeBooted"

//  here is the standard table which defines IOP Image table fields
extern const fieldDef aIOPImageTable_FieldDefs[];

//  and here is the size, in bytes, of the IOP Image table field defs
extern const U32 cbIOPImageTable_FieldDefs;


#endif  /* #ifndef _IOPImageTable_h */

