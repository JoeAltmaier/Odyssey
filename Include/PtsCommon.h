/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: PtsCommon.h
// 
// Description:
// This file contains the definitions used in all PTS modules.
// This file is included by every PTS module.
// 
/*************************************************************************/

#if !defined(PtsCommon_H)
#define PtsCommon_H

#include "CTTypes.h"
#include "String.h"
#include "Message.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#pragma pack(4)

//#define tZERO		0x20	// Clear memory

/*************************************************************************/
// Define the literals
//*************************************************************************/
#define CT_PTS_PART_TABLE_HDRS "PtsHdrs"
#define CT_PTS_PART_TABLE_DATA1 "PtsData1"

#define	 CB_RGB_IMAGE_BLKS	 2000000			// size of large image blob
#define	 CB_RGB_DATA_BLKS	 (1048576 + CB_RGB_IMAGE_BLKS)	// size of Small DataBlob


#define  CBMAXFIELDSPEC		64					// 63 chars + null
#define  CBMAXTABLESPEC		64					// 63 chars + null

// these are static now
#define	 CMAXENTRIES		50					// max # of fields
#define  iMaxHeaders		100					// max # of table headers
#define  iMaxHeaps			8					// max # of heap
#define  cDefaultHeapEntries 20					// default # of heap entries
#define	 cbDebugBuffer		0x8000				// size of debug buffer


#define	 ercTableNotfound	 1
#define	 ercOK				 0
#define	 ercNoFreeHeaders	 2
#define	 ercTableExists		 3
#define	 ercEOF				 4					// end of table; entry not found
#define	 ercBufTooSmall		 5
#define	 ercTableEmpty		 6
#define  ercPtsStructsTooSmall  7
#define	 ercBadParameter	 8
#define  ercKeyNotFound		 9
#define	 ercFieldNotFound	10
#define  ercNoModRowId		11
#define	 ercNoListeners		12
#define	 ercNoSuchListener	13
#define  ercNoMoreHeap		14
#define  ercNoMoreDataHeap	15
#define	 ercOkDuplicateNotFound	16
#define	 ercOkDuplicateRowFound 17
#define	 ercDuplicateFieldFound 18
#define	 ercFieldSizeTooLarge	19
#define	 ercNoDynamicReplyBuffer 20
#define  ercInconsistentRowIds	21
#define  ercBadVlfStruct		22
#define  ercVlfNotSupported		23
#define	 ercInVarLenFieldDef	24
#define	 ercIncorrectFieldSize	25
#define	 ercModifyMatchMoreThanOne	26
#define	 ercRowSizeMisMatch		27
#define	 ercInvalidCmdForVLF	28

#define CT_PTS_RID_FIELD_NAME 			"rid"
#define CT_PTS_ALL_ROWS_KEY_FIELD_NAME	"AllRows"
#define	CT_PTS_TABLE_OF_TABLES			"tableOfTables"
#define CT_PTS_TOT_TABLE_NAME			"tableName"

// defines for InitializeListenerIDs
#define INITIALIZE		0
#define AUGMENT			1
#define	CLISTENERIDS	50				// initial # of listenerIDs
		
typedef  unsigned long  U32;

typedef  unsigned char uchar;  


 //  Bit flags for the listen operations;  can be OR'd together to represent listen request 

#define	ListenOnDefineTable				0x0001	// Replies on creation of any table.  
#define	ListenOnInsertRow				0x0002	// Replies on insertion of any row.
#define	ListenOnDeleteOneRow			0x0004	// Replies on deletion of a row ?defined by a field?.
#define	ListenOnDeleteAnyRow			0x0008	// Replies on deletion of any row.
#define	ListenOnModifyOneRowOneField	0x0010	// Replies on modify of a field in a row
#define	ListenOnModifyOneRowAnyField	0x0020	// Replies on modify of any field in a row ?defined by rowID?
#define	ListenOnModifyAnyRowOneField	0x0040	// Replies on modify of a field in any row
#define	ListenOnModifyAnyRowAnyField	0x0080	// Replies on modify of any field in any row
#define	ListenReturnedOnStopListen		0x0800	// Replies when table with listeners deleted
#define	ListenReturnedOnDeleteTable		0x1000	// Replies when table with listeners deleted
#define	ListenInitialReply				0x2000	// Replies on modify of any field in any row
#define FreeListener					0x4000	// Mark free listeners
#define ListenIdHeader					0x8000	// header

#define ListenOnTableChange				ListenOnInsertRow | ListenOnDeleteAnyRow | ListenOnModifyAnyRowAnyField

// Listen flags that apply to specific operations

#define lFlagDefineTable				ListenOnDefineTable
#define	lFlagInsertRow					ListenOnInsertRow
#define	lFlagDeleteRow					ListenOnDeleteOneRow | ListenOnDeleteAnyRow
#define	lFlagModifyRow					ListenOnModifyOneRowAnyField | ListenOnModifyOneRowOneField | ListenOnModifyAnyRowOneField | ListenOnModifyAnyRowAnyField
#define	lFlagModifyField				ListenOnModifyOneRowAnyField | ListenOnModifyOneRowOneField | ListenOnModifyAnyRowOneField | ListenOnModifyAnyRowAnyField
#define lFlagDeleteTable				lFlagModifyField | lFlagDeleteRow | ListenOnInsertRow

// flags to define the listen reply

#define	ReplyContinuous					0x0001	// Replies until Stop Listen  
#define	ReplyOnceOnly					0x0002	// Replies only once.
#define	ReplyWithRow					0x0004	// Reply includes pointer to row
#define	ReplyWithRowID					0x0008	// Reply with rowID
#define ReplyFirstWithTable				0x0010	// First reply includes table, returned in rowbuffer sgi
#define ReplyFirstWithMatchingRows		0x0020	// First reply includes key matching rows, returned in rowbuffer sgi

// persist flags:  include field and table flags; need more definition

#define	NotPersistant_PT				0x0000	// does not persist 
#define	Persistant_PT					0x0001	// persist.
#define NonDuplicate_PT					0x2000	// this field must have unique value within table
#define VarLength_PT					0x4000	// variable length field
#define	TableOfTables_PT				0x8000	// internal flag for table of tables

#define iTOTHeader						0		// index of TableOfTables

/*************************************************************************/
//    Structures to define table headers and data blocks
/*************************************************************************/


// rowID is defined in CtTypes.h
/*
typedef struct {
	U16	 Table;
	U16  HiPart;
	U32  LoPart;
} rowID;
*/


// *** If you change this enum, make sure that table rgOpToListenFlags[]  ***
// *** in Odyssey\DdmPTS\PtsProcs.cpp is kept in sync.  It is indexed     ***
// *** by this enum value.                                                ***

// Table operation definitions for listen

	typedef enum { 
		NoOp,				
		OpDefineTable,				
		OpInsertRow,				
		OpDeleteRow,				
		OpModifyRow,				
		OpModifyField,
		OpDeleteTable
	} tableOpType;


	typedef enum { 
		OpQueryRID,				
		OpSetRID,
		OpTestSetField,
		OpClearField,
		OpAndBits,
		OpOrBits,
		OpXorBits
	} fieldOpType;
//
// Values for the iFieldType Field.
//

// *** If you change this enum, make sure that table acbDataTypeSize[]  ***
// *** in Odyssey\DdmPTS\PtsProcs.cpp is kept in sync.  It is indexed   ***
// *** by this enum value.                                              ***
	typedef enum {
		BINARY_FT,	// Miscellaneous binary data of an unspecified type.
		S32_FT,
		U32_FT,
		S64_FT,
		U64_FT,
		STRING16_FT,
		STRING32_FT,
		STRING64_FT,
		ROWID_FT,
		BOOL_FT,
		VDN_FT,
		USTRING16_FT,
		USTRING32_FT,
		USTRING64_FT,
		USTRING128_FT,
		USTRING256_FT,
		DID_FT
	} fieldType;
	
//
// The field definition structure.
//
	typedef struct {
   		String64		name;				// Null terminated string.
		U32				cbField;			// # of bytes in field.
		fieldType		iFieldType;			// type of data
		U32				persistFlags;		// flags that describe persist state
	} fieldDef, *pFieldDef;

//
//	variable length field structure:  defines the variable length fields 
//	for input and output of rows

  	typedef struct {
		U32				cbVLField;			// # of bytes in field.
		U32				oVLField;			// offset of the vlf from start of rowID 
	} varLenField;

//
// The TableOfTables row definition structure.
//
	typedef struct {
		rowID			rowId;				// rowId given by PTS
   		String64		tableName;			// table name.
		rowID			tableId;			// tableId 	
	} rowDef;


// The definition of the structure returned by ModifyBitsInField that
// identifies the fields modified

	typedef struct {
		rowID	rowModified;
		U32		fieldRet;
	} modField;
	
	
// Table definition structure returned by GetTableDef
//
	typedef struct {
		U32		cFields;					// count of fields per row
		U32		cbRow;						// count of bytes per row													
		U32		cRows;						// count of rows in table
	} tableDef;	


//	A chunk of memory, rgDataBlocks, is allocated for the data entries of the tables.
//	The chunk has a dataHeapHdr at the beginning.  It is then divided into data blocks
//	for the tables being allocated. Each data block corresponds to a table;  a table
//  may be expanded, where it would have another data block allocated.  Each data block
//  contains a dataBlkHdr structure, that defines the next data block and the size of 
//	the current.  The data block is then divided into table entries, defined by entryDef.

 	typedef struct {
		U32			oPrevEntry;				// pointer to the previous entry
		U32			oNextEntry;			    // pointer to next entry
		U32			cbRow;					// count of bytes of row data (inc rowID)
											// and including user's varLenField
	} entryHdr;


 	typedef struct {						// keep this in sync with entryHdr
		U32			oPrevEntry;				// pointer to the previous entry
		U32			oNextEntry;			    // pointer to next entry
		U32			cbRow;					// count of bytes of row data
											//(incl. user struct vlfields)
		rowID		rowId;					// unique row ID
		unsigned char	entrydata[8];		// structure for accessing beginning
											// of entry and pointer to next												
 	} entryDef, *pEntryDef;	

// this structure is in the fixed portion of the row, and points to variable length field
	typedef struct {
		U32			iHeapHdr;				// index of heap header in array: need check?
		U32			oHeapEntry;			    // offset to heap entry from prgdatablocks
		U32			cbField;				// size of the field; does not include rowID or user struct
 	} varLenFieldDef;	


//  Variable length fields are kept in 6 fixed length field arrays
//
 	typedef struct {						// offsets only used in free list:
		U32			oPrevEntry;				// pointer to the previous free entry
		U32			oNextEntry;			    // pointer to next free entry
		rowID		rowId;					// store rowID here too, just in case
		varLenField vlfOfUser;				// varLenField structure that user sends and expects
 	} heapEntryDef;	


	typedef struct {
		U32			oNext;					// pointer to next data block for this table
											// if on freelist, points to next free data
		U32			cbBlock;				// # of bytes in data block
		entryDef	data;					// start of entries:  actual size, struct ??
	} dataBlkHdr, *pdataBlkHdr;	


	//  Header of the rgDataBlocks blob

 	typedef struct {
		U32			oFreeBlk;				// pointer to next data block
		dataBlkHdr	firstBlk;				// first data block 
	} dataHeapHdr, *pDataHeapHdr;	


//
// structure used for linked list of replies to be sent after a table modification
//

	typedef struct _listenReply{
		U32			lastReply;				// last reply to listen
		void		*pListenMsg;			// listen message for reply
		_listenReply *pNext;				// next listenreply:  
	} listenReply;	


// Listen message holder	
	
	typedef struct _listenMsgHdr {
		U32			listenMode;					// listen on what?
		U32			replyMode;					// reply once, continuous, with row
		U32			listenerID;					// listener
		char		rowKeyName[CBMAXFIELDSPEC];	// field name to id row
		uchar		rowKeyValue[CBMAXFIELDSPEC];// row key value
		char		fieldName[CBMAXFIELDSPEC];	// field name of listen
		uchar		fieldValue[CBMAXFIELDSPEC];	// field value if applicable
		U32			cbFieldValue;				// size of field value;  needed for no field value
		Message		*pListenMsg;				// store pointer to message
		_listenMsgHdr	*pPrev;					// pointer to previous header in list
		_listenMsgHdr	*pNext;					// pointer to next headers 

	} listenMsgHdr, *pListenMsgHdr;


 // First header in this array is the header of the block of listenerIds;  if another block has
 //	been allocated, then the header will point to the next block of listenerId header.  

	typedef struct _listenerIDmap {
		U32			listenerID;					// listener	 ID = index:  redundant 
		rowID		tableId;					// null terminated table name
		U32			listenMode;					// listen mode 
		_listenerIDmap	*pNext;					// Next ID in linked free list
	} listenerIdMap, *pListenerIdMap;


//  The table headers are allocated as a chunk of memory.  


	typedef struct _tableHeader {
		char		name[CBMAXTABLESPEC];		// null terminated table name
		U32			version;
		U32			persistFlags;				// does this table persist, have variable len fields
		U32			cFields;					// # of fields per entry
		U32			cFixedLenFields;			// # of fixed length fields
		U32			cbEntry;					// # of bytes per entry including link and rounded up to dword
		U32			cbEntryData;				// # of bytes of data per entry including rowID	& PTS vlf structs
		U32			cbFixedLenData;				// # of bytes of fixed length data per entry including rowID
		U32			cCurrentEntries;			// # of entries in table
		U32			cbCurrentTable;				// # of bytes in table (not including entry headers) w/ user vlf structs
		U32			cbrgEntryDef;				// actual # of bytes in array of field desc
		U32			cEntriesAlloc;				// count of entries to allocate if overrun
		rowID		rowId;						// rowId counter
		fieldDef	rgEntryDef[CMAXENTRIES];	// array of field descriptions
		dataBlkHdr	*pTableData;				// pointer to table for debug
												// need to reinitialize after a reload 
												// next four are offsets from prgDataBlks:	
		U32			oTableData;					// offset to table block (dataBlkHdr)
		U32			oFirstEntry;				// offset to first entry (entryDef)
		U32			oLastEntry;					// offset to last entry	 (entryDef)
		U32			oFreeEntry;					// offset to free entry	 (entryDef)
		U32			cListens;					// # of listens on queue
		listenMsgHdr	*pListenQ;				// pointer to queue of listen headers
 		U32			oNext;						// offset from pHeaders

	}  tableHeader, *pTableHeader;
	
	//  each of the heaps for the variable length fields has the following header
  	typedef struct _heapHeader {
		U32			cbEntry;					// # of bytes per entry including links
		U32			cbEntryData;				// # of bytes of data per entry
		U32			cCurrentEntries;			// # of entries in table
		U32			cEntriesAlloc;				// count of entries to allocate if overrun
		dataBlkHdr	*pTableData;				// pointer to heap for debug
												//   need to reinitialize after a reload 
												// next four are offsets from prgDataBlks:	
		U32			oTableData;					// offset to table block (dataBlkHdr)
		U32			oFreeEntry;					// offset to free entry	 (entryDef)
 	}  heapHeader;

 
  	typedef struct {
		U32			signature;
		U32			checksum;					// unknown still...
		U32			oFreeHdr;					// offset of free header
		U32			oListHdr;					// offset of first header; should be same as pListHeaders (offset)
		U32			tableCount;					// current count of tables that have been defined 
												//  this count is continuously updated for unique ID
   		tableHeader	rgHdr[iMaxHeaders];			// array of table headers
		heapHeader 	rgHeapHdr[iMaxHeaps];		// array of heap headers
	} tableHdrDef;	

  
//	entry of debug buffer:  circular buffer with history 

  	typedef struct {
		char			ptsOp[4];				 // operation
		U32				filler;					 // save for whatever
		rowID			rowOp;					 // row modified
	} dbgBufEntry;


/*************************************************************************/
//    Procedural interfaces
//*************************************************************************/

 
dataBlkHdr* AllocFromTableHeap (U32 cbAlloc, U32 cEntries);

dataBlkHdr* ExtendTable(tableHeader *pCurrentHdr);

dataBlkHdr* ExtendHeap(heapHeader *pCurrentHdr);
 
int GetTableHeader (char *ptName, rowID *pTableId, pTableHeader *pCurrentHdrRet);

int GetListenHeader (pListenMsgHdr *pCurrentHdrRet, rowID *pTableId, U32 listenFlags);

int ReturnListenHeader (tableHeader	*pTableHdr,	listenMsgHdr  *pCurrentListenHdr);

int SearchForListenerId (listenerIdMap **pListenerIdRet, U32 listenerId);

int IsThisDuplicateEntry ( tableHeader	*pCurrentHdr, uchar	*pRow);		

BOOL FieldMatch	(tableHeader *pCurrentTableHeader, uchar *pModifiedRow, char *pKeyName,	
				 uchar *pKeyValue);	

int ChkFieldDefs(const char *pszTableName,const fieldDef *pFieldDefs,U32 cbFieldDefs,U32 cbClass);

int FindField (tableHeader *pCurrentHdr, char *pKeyName, U32 *poFieldRet,
			   U32 *piFieldRet);

int CompareField (tableHeader *pCurrentHdr, char *pKeyName, uchar *pKeyValue,
				  U32 cbKeyValue, U32 *ppRowRet, U32 cbMax, U32 *pcRowsRet);

int CompareRows	( tableHeader *pCurrentHdr, uchar *pCurrentRow,	uchar *pNewRow,
				  char	*prgFieldSpec, U32	*pcModifiedFields);
				  
int GetVarLenRow (tableHeader *pCurrentHdr, entryDef	*pCurrentEntry,
		     	  uchar *pRowRet, U32 cbRow, BOOL fVarLen, U32 *pcbRowRet);
			   	
int AddEntryToAHeap (entryDef *pCurrentEntry, varLenFieldDef *pVLFinRow,
					 varLenField *pVarLenFields, uchar *pHeapData);

int DeleteEntryFromHeap(entryDef *pCurrentEntry, varLenFieldDef *pVLFinRow);

int AddRowToTable(tableHeader *pCurrentHdr, entryDef *pCurrentEntry);

int ModifyVarLenField(entryDef *pCurrentEntry, varLenFieldDef *pVLFinRow,
					 varLenField *pVarLenFields, uchar *pNewRow);

void RemoveRowFromTable(tableHeader *pCurrentHdr, entryDef *pCurrentEntry,
						BOOL fDecRid);
											 
int QueryOrSetRID (char *ptName, fieldOpType opRID, rowID	*pRowID );

int CreateTableOfTables (char *ptName, U32 cEntries);
  
int SearchListenQueue (tableHeader *pCurrentTableHdr, uchar	*pModifiedRow,
						char *pKeyField, U32 cKeyField,tableOpType tableOp);	

int DefineTable (char *ptName, fieldDef *prgFields, U32 cbrgFields,
				 U32 cEntriesRsv, U32 persistflags, rowID *pTableIdRet);
int DeleteTable (char* ptName, U32 tableId);

int GetTableDef (char *ptName, rowID *pTableID, Message *pMsg, U32 *pcbrgFields, tableDef *pTableDefRet, U32 cbTableDef);

int InsertRow(char *ptName, uchar *prgRows, U32 cbrgRows, Message *pMsg, U32 *pcRowsInRet);

int InsertVarLenRow (char *ptName, uchar *pRowData, U32 cbRow, rowID *pRowIdRet);	


int EnumerateTable(char* ptName, U32 offset, uchar* pBufRet, U32 cbMax, Message *pMsg, U32 *pcbRet);


int ReadRow(char *ptName, char *pKeyName, uchar *pKeyValue, U32 cbKeyValue, Message *pMsg, U32 *pcRowsRead);

int ReadVLRow(char *ptName, char  *pKeyName, uchar *pKeyValue, U32  cbKeyValue,	Message *pMsg, U32 *pcRowsRead);		// count of rows read and returned if space
//int ReadRowMultiKey(char *ptName, char *prgKeyNames, U32 cbrgKeyNames, uchar *prgKeyValues, 
//					/*U32 cbrgKeyValues,*/ uchar *prgRowsRet, U32 cbMax, U32 *pcbRet);

int DeleteRow(char *ptName, char* pKeyName, uchar *pKeyValue, U32 cbKeyValue, U32 cRowsDelMax, U32 *pcRowsDel);

/*int DeleteRowMultiKey(char *ptName, char *prgKeyNames, U32 cbrgKeyNames, uchar *prgKeyValues,
					   U32 cRowsDelMax, U32 *pcRowsDelRet); */

int ListenTable(char *ptName, char *pRowKeyName, uchar *pRowKeyValue, U32 cbRowKeyValue,
				char *pFieldKeyName, uchar *pFieldKeyValue, U32	cbFieldKeyValue, U32 listenFlags,
				U32 replyMode, Message *pListenMsg, U32 *pListenerIdRet);
				
int StopListen( U32	listenerId);

int ModifyRow(char *ptName, char *prgKeyNames, U32 cbrgKeyNames, uchar *prgKeyValues, U32 cbrgKeyValue,
			  uchar *prgRowBuf, U32 cbRowBuf, Message *pMsg, U32 *pcRowsModRet);

int ModifyField( char *ptName, char *prgKeyNames, U32 cbKeyNames, uchar *prgKeyValues, U32 cbrgKeyValue,
		  char *prgFieldNames, uchar *prgFieldValues, U32 cbrgFieldValues, Message  *pMsg, U32 *pcRowsModRet);
	
int ModifyBitsInField (char  *ptName, char  *pKeyName,	uchar *pKeyValue, U32 cbKeyValue, char  *pFieldName,	
  				U32 *pBitMaskValue, fieldOpType  OpBitField, Message  *pMsg, U32 *pcRowsModRet);				


int TestAndSetOrClearField (char  *ptName,	fieldOpType opField, char *pKeyName, uchar *pKeyValue,
							U32 cbKeyValue,	char  *pFieldName, BOOL  *pTestRet);

int InitializeHeaders (tableHdrDef *pHeaderMem, U32 cbHeaderMem);

int InitializeDataBlock (dataHeapHdr *pHeaderMem, U32 cbHeaderMem);

int InitializeHeaps ();

int InitializeListenerIDs (U32 cListenIds, U32 tableState);

void ClearNonPersistFields (tableHeader *pCurrentHdr);	


// ChkFieldDefs is undefined in Nac build.
// So for now...
#ifdef _DEBUG
#define CHECKFIELDDEFS(class)  static int class##FieldDefs=\
		ChkFieldDefs(class::TableName(),class::FieldDefs(),class::FieldDefsSize(),sizeof(class))
#else
#define CHECKFIELDDEFS(class)	static int class##FieldDefs = 0
#endif


#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* PtsCommon_H  */


/*************************************************************************/
// $Log: /Gemini/Include/PtsCommon.h $
// 
// 36    1/31/00 9:26a Sgavarre
// Made image tables smaller to fit into currently allocated partition
// size.
// 
// 34    11/22/99 3:40p Sgavarre
// add version number for tables
// 
// 33    11/04/99 1:14p Sgavarre
// Add persist routine for clearing fields
// 
// 32    10/28/99 9:57a Sgavarre
// support for persist
// 
// 31    10/06/99 4:47p Sgavarre
// add var len fields
// 
// 30    10/02/99 1:55p Agusev
// Added Unicode types for field defs
// 
// 29    9/16/99 11:15p Jlane
// Re-Add ChkFldDefs
// 
// 28    9/15/99 9:34p Jlane
// Comment-out ChkFldDefs to get NAC to build.
// 
// 27    9/15/99 4:28p Sgavarre
// not good at naming these fields... 
//
// 25    9/15/99 2:28p Sgavarre
// Add structure for variable length field
// 
// 24    9/09/99 2:36p Jlane
// Bumped up CMAXENTRIES to 50
// 23    9/03/99 3:06p Tnelson
// Fixed ChkFieldDefs to be 'const' safe
// Added CHKFIELDDEFS macro that will compile out if not DEBUG
// 
// 22    9/01/99 3:07p Jlane
// Added CT_PTS_ALL_ROWS_KEY_FIELD_NAME a special key field name used to
// designate all rows.
// 
// 21    8/28/99 6:02p Sgavarre
// add vdn type;  add error codes;  add checkFieldDef routine;
// 
// 20    8/19/99 4:47p Egroff
// Added VDN_FT for Virtual Device Numbers in config recs.
// 
// 19    8/13/99 5:29p Sgavarre
// Add ModifyBits and support for multirow operations and updates in
// listen
// 
// 18    8/08/99 11:31a Jlane
// Added listen reply bit: ListenReturnedOnStopListen.
// 
// 17    7/23/99 2:48p Sgavarre
// Add QuerySetRID, TestSetOrClearField and Table of tables
// 
// 16    7/12/99 1:49p Sgavarre
// Add listen replies when a table is deleted;
// Add deletetable by tableId;
// 
// 15    6/08/99 3:37p Sgavarre
// incremented # of field defs to 35;  (needs to be more dynamic)
// modified listener structures and defines
// added elements to tableheader for auto-grow feature and some stats
// 
// 14    6/05/99 12:27a Ewedel
// Boosted CMAXENTRIES from 20 to 25, to keep up with EVC record.
// [It would be nice if this were dynamic...]
// 
// 13    4/16/99 6:15p Jlane
// Modifications made debugging Listen.
// 
// 12    3/27/99 1:21p Sgavarre
// add listen code changes
// 
// 11    3/23/99 8:47a Sgavarre
// Add errors;  add listener structures;  Add listen flags;  Add reply
// flags;  change perist enum to flags to be expanded later;  Add new APIs
// for listen procs;  modify API for GetTableHeader
// 
// 10    3/12/99 5:38p Ewedel
// Changed max field def count from 10 to 20, and added error code
// ercPtsStructsTooSmall to report when a new table def exceeds this
// limit.
// 
// 9     3/11/99 7:11p Ewedel
// Removed extra VSS Log: keyword from comment text (it was expanding
// twice), and ordered all change history in consistent reverse-chrono.
// 
// 8     3/11/99 6:06p Ewedel
// Added cross-comment warning of PtsProcs.cpp table dependency on
// fieldType enum.
// 
// 7     3/03/99 5:43p Jlane
// Added S32_FT, S64_FT and String16_FT field types.  Also added VSS Log:
// keyword to get automatic change headers.
//
// 02/02/99 sg	moved tableDef and fieldDef up, before reference 
// 02/01/99	JFL	Updated tableDef and fieldDef declarations with those
//				from TableMsgs.h.
// 11/3/98 sg:		 remove some cb fields from APIs as not needed
// 10/5/98 sgavarre: Create file; add definitions from PTS program
/*************************************************************************/

