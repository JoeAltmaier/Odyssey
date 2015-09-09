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
// This file contains the definition of the table messages.
// 
// Update Log: 
// 10/13/98	JFL	Created.
// 10/30/98 sg	modified GetTableDef, InsertRows, DefineTable
//				and Enumerate table to match service
// 01/10/98 sg	modify payloads so that they do not exceed the 
//				64 byte limit of header and payload
// 01/18/99	JFL	Changed cbFieldsRet to cFieldDefsRet in GetTableDefReplyPayload.
// 01/20/99	JFL	Added missing RowID field to Listen reply payload.
// 01/30/99 sg	Change cFieldDefsRet to cbFieldDefsRet in GetTableDefReplyPayload,
//				Removed 'RET' from MODIFY_ROWS_REPLY_ROWIDSRET_SGI,
//				 MODIFY_FIELDS_REPLY_ROWIDSRET_SGI, and  DELETE_ROWS_REPLY_ROWIDSRET_SGI
//				Removed READ_ROWS_REPLY_ROWIDSRET_SGI,	DELETE_ROWS_REPLY_DATA_BUFFER_SGI
// 02/01/99 sg  removed if 0 on fieldtypes, changed fieldDef.iFieldType to fieldType
// 02/01/99	JFL	Moved tableDef and fieldDef structs and fieldType enum to
//				PtsCommon.h.
// 02/02/99 sg  put if 0 back, as fieldDef now defined in ptscommon.h
// 02/27/99 JFL	Added LISTEN_REPLY_MODIFIED_RECORD_SGI.
// 02/28/99 JFL Changed ListenerID to U32 in ListenReplyPayload & StopListeningMsgPayload.
// 03/23/99 sg	Moved rowID, fieldtype to ptscommon.h
//				Change cIdsRet in reply payloads to number of things deleted or modified
//				 in Delete, Modify, Insert
//				Add stopListen; removed old commented out code
//				Modify listen reply payload;  add SG items for listen
//				Change listentype and replytype enums to flags defined in ptsprocs.h 
//				Add DefineTableReplyPayload	
// 04/26/99	sg	Remove reply payloads
// 06/07/99 sg	Remove ListenerID SGL:  it is in payload
// 07/11/99 sg	Update DeleteTable to take tableId
// 07/18/99 sg	Add QuerySetRID and TestAndSetField
// 08/08/99 sg	Add ModifyBits and change Listen payloads
/*************************************************************************/

#ifndef _TableMsgs_h
#define _TableMsgs_h

#include "PTSCommon.h"

// SEND_PTS_MSG_BY_SLOT - Controls inclusion of workaround code in Table.cpp,
// Rows.cpp, Fields.cpp and Listen.cpp that sends msgs to slot routed by 
// function code.  This avoids an early OS lack of support for route by
// function code among boards. If the OS doesn't work we can put the 
// workaround back by defining SEND_PTS_MSG_BY_SLOT as true.
#define SEND_PTS_MSG_BY_SLOT	false


/**************************************************************************/
/* Define Table Message Structures                                        */
/**************************************************************************/

//
// DefineTablePayload -
// The structure used as the payload of the message to Define tables
//
typedef struct DefineTablePayload {
	U32			cEntriesRsv;	// Number of table entries to create initially.
	U32			persistFlags;	// Should this table be persistant?	 flags?????
	rowID		tableIdRet;		// Returned Id of table defined in rowID format
} DefineTablePayload;

//
// The Define Table message uses two scatter gather list items:
// DEFINE_TABLE_MSG_TABLENAME_SGI defines buffer that contains tablename. 
// DEFINE_TABLE_MSG_FIELDDEFS_SGI buffer that contains the array of field definitions.
//
#define DEFINE_TABLE_MSG_TABLENAME_SGI			0		// tablename
#define DEFINE_TABLE_MSG_FIELDDEFS_SGI			1		// array of fieldDefs


/**************************************************************************/
/* Delete Table Message Structures                                        */
/**************************************************************************/

//
// DeleteTablePayload -
//	Delete Table takes either the table name or the ID;  
// The structure used as the payload of the message to Delete a table
//
typedef struct DeleteTablePayload {
	rowID		tableId;		// Id of table 
} DeleteTablePayload;

//
// The Delete Table message uses one scatter gather list items:
// DELETE_TABLE_MSG_TABLENAME_SGI defines buffer that contains tablename. 
//
#define DELETE_TABLE_MSG_TABLENAME_SGI			0		// tablename

/**************************************************************************/
/* QuerySetRID Message Structures                                        */
/**************************************************************************/

//
// QuerySetRIDPayload -
// The structure used as the payload of the message to QuerySetRID
//
typedef struct QuerySetRIDPayload {
	fieldOpType	opRID;				// query or set rowID
	rowID		rowId;				// rowID 
} QuerySetRIDPayload;

//
// The QuerySetRID message uses two scatter gather list items:
// QUERY_SET_RID_MSG_TABLENAME_SGI defines buffer that contains tablename. 
//
#define QUERY_SET_RID_MSG_TABLENAME_SGI			0		// tablename



/**************************************************************************/
/* Get Table Definition Message Structures                                */
/**************************************************************************/

//
// GetTableDefPayload -
// The structure used as the reply payload of the message to get table definitions.
//
typedef struct GetTableDefPayload {
	U32			persistFlagsRet;	// Is this table persistant?
	U32			cbFieldDefsRet;		// The number of entries returned in the fielddef array.
	rowID		tableId;			// tableId 
} GetTableDefPayload;


//
// GET_TABLE_DEF_REPLY_FIELDDEFS_SGI - 
//
#define GET_TABLE_DEF_MSG_TABLENAME_SGI			0			// tablename
#define GET_TABLE_DEF_REPLY_FIELDDEFS_SGI		1			// buffer where fieldDefs are returned							
#define GET_TABLE_DEF_REPLY_TABLEDEF_SGI		2			// buffer where tableDef is returned


/**************************************************************************/
/* Enumerate Table Message Structures                                     */
/**************************************************************************/

//
// EnumerateTablePayload -
// This structure is used as the payload of the message to enumerate table rows.
//
typedef struct EnumerateTablePayload {
	U32			uStartingRow;	// Offset of first row to return (0 based).
	U32			cbDataRet;		// The number of bytes of returned data.
} EnumerateTablePayload;

//
// Enumerate Table Scatter Gather list index defines:   
// ENUMERATE_TABLE_MSG_TABLENAME_SGI is the buffer that contains the tablename.
// ENUMERATE_TABLE_REPLY_SGI defines the buffer where row data returned.
//
#define ENUMERATE_TABLE_MSG_TABLENAME_SGI		0		
#define ENUMERATE_TABLE_REPLY_SGI				1		



/**************************************************************************/
/* Insert Rows Message Structures                                         */
/**************************************************************************/
//
// InsertRowsPayload -
// The structure used as the reply payload of the message to insert rows.
//

typedef struct InsertRowsPayload {
	U32			cIDsRet;		// Number of rows actually inserted.
} InsertRowsPayload;

//
// Insert Rows Scatter Gather List Index defines - 
// The insert row message uses two scatter gather list items.
// INSERT_ROWS_MSG_TABLENAME_SGI is a buffer that contains the tablename.
// INSERT_ROWS_MSG_DATA_BUFFER_SGI is a buffer with the row data to insert.
//

#define INSERT_ROWS_MSG_TABLENAME_SGI						0
#define INSERT_ROWS_MSG_DATA_BUFFER_SGI						1

//
// The Insert row reply uses one scatter gather list item:
// INSERT_ROWS_REPLY_ROWIDS_SGI is the buffer where the RowIDs for the
//  newly inserted items can optionally be returned with the reply.

#define INSERT_ROWS_REPLY_ROWIDS_SGI						2


/**************************************************************************/
/* Insert Variable Length Row Message Structures                                         */
/**************************************************************************/
//
// InsertVLRowsPayload -
// The structure used as the reply payload of the message to insert rows.
//

typedef struct InsertVarLenRowPayload {
	rowID		rowId;		// rowId of the row inserted.
} InsertVarLenRowPayload;

//
// Insert Variable Length Row Scatter Gather List Index defines - 
// The insert row message uses 4 scatter gather list items.
//  INSERT_VLROW_MSG_TABLENAME_SGI is a buffer that contains the tablename.
//  INSERT_VLROW_MSG_DATA_SGI is a buffer with the fixed length row data to insert.

#define INSERT_VLROW_MSG_TABLENAME_SGI				0
#define INSERT_VLROW_MSG_DATA_SGI					1


/**************************************************************************/
/* Modify Rows Message Structures                                         */
/**************************************************************************/


//
// ModifyRowsPayload -
// The structure used as the reply payload of the message to modify rows.
//
typedef struct ModifyRowsPayload {
	U32			cRowsToModify;		// number of rows to modify;  0 means ALL that match
	U32			cRowsModifiedRet;	// Number of entries actually modified.
} ModifyRowsPayload;

//
// Modify Rows Scatter Gather List Index defines - 
// The modify rows message uses four scatter gather list items.
// The first is the tablename:  MODIFY_ROWS_MSG_TABLENAME_SGI.
// Note: The next three correspond.  There's a Data Row for each Key Name and Value.
// MODIFY_ROWS_MSG_KEY_NAMES_SGI is an array of key field names.  
// MODIFY_ROWS_MSG_KEY_VALUES_SGI is an array of key field values. 
// MODIFY_ROWS_MSG_DATA_BUFFER_SGI is a buffer with the "modified" row data.
// 
#define MODIFY_ROWS_MSG_TABLENAME_SGI						0
#define MODIFY_ROWS_MSG_KEY_NAMES_SGI						1
#define MODIFY_ROWS_MSG_KEY_VALUES_SGI						2
#define MODIFY_ROWS_MSG_DATA_BUFFER_SGI						3

// The modify rows reply uses one scatter gather list item.
// MODIFY_ROWS_REPLY_ROWIDS_RET_SGI is a list or returned RowIDs.
#define MODIFY_ROWS_REPLY_ROWIDS_SGI					4



/**************************************************************************/
/* Modify Fields Message Structures                                       */
/**************************************************************************/

//
// ModifyFieldsPayload -
// The structure used as the reply payload of the message to modify rows.
//
typedef struct ModifyFieldsPayload {
	U32			cRowsToModify;
	U32			cRowsModifiedRet;	// The number of fields actually modified.
} ModifyFieldsPayload;

//
// Modify Rows Scatter Gather List Index defines - 
// MODIFY_FIELDS_MSG_TABLENAME_SGI contains the tablename.
// The following two arrays of Key Fieldnames and Values identify the rows.
// MODIFY_FIELDS_MSG_KEY_NAMES_SGI is an array of key field names.  
// MODIFY_FIELDS_MSG_KEY_VALUES_SGI is an array of key field values. 
//
// The following two arrays of Fieldname and Values identify the modified fields.
// MODIFY_FIELDS_MSG_FIELD_NAMES_SGI is an array of field names to be modified.  
// MODIFY_FIELDS_MSG_FIELD_VALUES_SGI is an array of field values to be written. 
//
// MODIFY_FIELDS_REPLY_ROWIDSRET_SGI is an array where the RowIDs for the
// modified items can optionally be returned with the reply.

#define MODIFY_FIELDS_MSG_TABLENAME_SGI						0
#define MODIFY_FIELDS_MSG_KEY_NAMES_SGI						1
#define MODIFY_FIELDS_MSG_KEY_VALUES_SGI					2
#define MODIFY_FIELDS_MSG_FIELD_NAMES_SGI					3
#define MODIFY_FIELDS_MSG_FIELD_VALUES_SGI					4

#define MODIFY_FIELDS_REPLY_ROWIDS_SGI						5



/**************************************************************************/
/* Modify Bits Message Structures                                       */
/**************************************************************************/
//
// ModifyBitsPayload -
// The structure used as the reply payload of the message to modify rows.
//
typedef struct ModifyBitsPayload {
	fieldOpType opFlag;	
	U32			cRowsToModify;
	U32			cRowsModifiedRet;	// The number of fields actually modified.
} ModifyBitsPayload;

//
// MODIFY_BITS_FIELD_MSG_TABLENAME_SGI contains the tablename.
// The following two arrays of Key Fieldnames and Values identify the rows.
// MODIFY_BITS_MSG_KEY_NAME_SGI is an array of key field names.  
// MODIFY_BITS_MSG_KEY_VALUE_SGI is an array of key field values. 
//
// The following two arrays of Fieldname and Values identify the modified fields.
// MODIFY_BITS_MSG_FIELD_NAME_SGI is an array of field names to be modified.  
// MODIFY_BITS_MSG_FIELD_MASK_SGI is an array of field values to be written. 
//
// MODIFY_BITS_REPLY_ROWIDS_SGI is an array where the RowIDs for the
// modified items can optionally be returned with the reply.

#define MODIFY_BITS_MSG_TABLENAME_SGI						0
#define MODIFY_BITS_MSG_KEY_NAME_SGI						1
#define MODIFY_BITS_MSG_KEY_VALUE_SGI						2
#define MODIFY_BITS_MSG_FIELD_NAME_SGI						3
#define MODIFY_BITS_MSG_FIELD_MASK_SGI						4

#define MODIFY_BITS_REPLY_ROWIDS_SGI						5


/**************************************************************************/
/* TestAndSet Fields Message Structures                                       */
/**************************************************************************/

//
// TestAndSetFieldsPayload -
// The structure used as the reply payload of the message to TestAndSet rows.
//
typedef struct TestAndSetFieldPayload {
	fieldOpType	opSetOrClear;
	BOOL		fTestRet;		// result of the test operation.
} TestAndSetFieldPayload;

//
// TestAndSetField Scatter Gather List Index defines - 
// TEST_SET_FIELD_MSG_TABLENAME_SGI contains the tablename.
// TEST_SET_FIELD_MSG_KEY_NAME_SGI is key field name.  
// TEST_SET_FIELD_MSG_KEY_VALUE_SGI is key field value. 
// TEST_SET_FIELD_MSG_FIELD_NAME_SGI is field name to be modified.  


#define TEST_SET_FIELD_MSG_TABLENAME_SGI					0
#define TEST_SET_FIELD_MSG_KEY_NAME_SGI					1
#define TEST_SET_FIELD_MSG_KEY_VALUE_SGI					2
#define TEST_SET_FIELD_MSG_FIELD_NAME_SGI					3


/**************************************************************************/
/* Read Rows Message Structures                                           */
/**************************************************************************/

//
// ReadRowsPayload -
// The structure used as the reply payload of the message to Read rows.
//
typedef struct ReadRowsPayload {
	U32			cRowsReadRet;		// Number of rows sucessfully Read.
} ReadRowsPayload;


//
// Read Rows Scatter Gather List Index defines - 
//
// The following two arrays of Key Fieldnames and Values identify the rows.
// READ_ROWS_MSG_KEY_NAMES_SGI is an array of key field names.  
// READ_ROWS_MSG_KEY_VALUES_SGI is an array of key field values. 
//
// READ_ROW_REPLY_ROWIDSRET_SGI is the buffer where the RowIDs for the
// newly Read items can optionally be returned with the reply.
// READ_ROWS_REPLY_DATA_BUFFER_SGI is a buffer for the row data to be Read.


#define READ_ROWS_MSG_TABLENAME_SGI							0
#define READ_ROWS_MSG_KEY_NAMES_SGI							1
#define READ_ROWS_MSG_KEY_VALUES_SGI						2

#define READ_ROWS_REPLY_DATA_BUFFER_SGI						3


/**************************************************************************/
/* Read Var Len Rows Message Structures                                   */
/**************************************************************************/

//
// ReadVLRowsPayload -
// The structure used as the reply payload of the message to Read VL rows.
//
typedef struct ReadVarLenRowsPayload {
	U32			cRowsReadRet;		// Number of rows sucessfully Read.
} ReadVarLenRowsPayload;

//
// Read VLRows Scatter Gather List Index defines - 
//
// The following two arrays of Key Fieldnames and Values identify the rows.
// READ_VLROWS_MSG_KEY_NAMES_SGI is an array of key field names.  
// READ_VLROWS_MSG_KEY_VALUES_SGI is an array of key field values. 
//
// READ_VLROWS_REPLY_ROWIDSRET_SGI is the buffer where the RowIDs for the
// newly Read items can optionally be returned with the reply.
// READ_VLROWS_REPLY_DATA_BUFFER_SGI is a buffer for the row data to be Read.


#define READ_VLROWS_MSG_TABLENAME_SGI						0
#define READ_VLROWS_MSG_KEY_NAMES_SGI						1
#define READ_VLROWS_MSG_KEY_VALUES_SGI						2

#define READ_VLROWS_REPLY_ROWDATA_SGI						3
#define	READ_VLROWS_REPLY_VLFS_SGI							4


/**************************************************************************/
/* Delete Rows Message Structures                                         */
/**************************************************************************/

//
// DeleteRowsPayload -
// This structure is used as the payload of the message to Delete rows.
//
typedef struct DeleteRowsPayload {
	U32			cRowsToDelete;		// Number of Rows to Delete.
	U32			cRowsDeletedRet;	// Number of rows sucessfully deleted.
} DeleteRowsPayload;

//
// Delete Rows Scatter Gather List Index defines - 
//
// The following two arrays of Key Fieldnames and Values identify the rows.
// DELETE_ROWS_MSG_KEY_NAMES_SGI is an array of key field names.  
// DELETE_ROWS_MSG_KEY_VALUES_SGI is an array of key field values. 
//
// DELETE_ROW_REPLY_ROWIDSRET_SGI is the buffer where the RowIDs for the
// deleted items can optionally be returned with the reply.
// DELETE_ROWS_REPLY_DATA_BUFFER_SGI is a buffer for the row data to be deleted.
//
#define DELETE_ROWS_MSG_TABLENAME_SGI							0
#define DELETE_ROWS_MSG_KEY_NAMES_SGI							1
#define DELETE_ROWS_MSG_KEY_VALUES_SGI							2



/**************************************************************************/
/* Listen on table / field message structures                             */
/**************************************************************************/

// The idea for the listen type started out mirroring the interface with the
// ability to listen on DefineTable, InsertRow, DeleteRow etc.  But then started
// to multiply by the possible permutations on whether or not the listen was
// on any specific field and/or row.  I think what's here pretty well covers
// the spectrum of possible requirements and seems pretty straight forward.
// I have implementation ideas for it too.
// NOTE: In the following comments "a" should be read as "a specific".
//
// The following enums have been changed to flags which are defined in ptscommon.h
// 
/*enum ListenTypeEnum {
	ListenOnDefineTable,			// Replies on creation of any table.  
	ListenOnInsertRow,				// Replies on insertion of any row.
	ListenOnDeleteOneRow,			// Replies on deletion of a row ?defined by a field?.
	ListenOnDeleteAnyRow,			// Replies on deletion of any row.
	ListenOnModifyOneRowOneField,	// Replies on modify of a field in a row
	ListenOnModifyOneRowAnyField,	// Replies on modify of any field in a row ?defined by rowID?
	ListenOnModifyAnyRowOneField,	// Replies on modify of a field in any row
	ListenOnModifyAnyRowAnyField	// Replies on modify of any field in any row
	};
 
enum ReplyModeEnum {
	ReplyContinuous,
	ReplyOnceOnly
	};
*/
//
// ListenPayload -
// The structure used as the payload of the message to listen on tables
typedef struct ListenPayload {
	U32			listenType;		// Type of operation for which you want to listen.
	U32			replyMode;		// reply flags:  reply once, continuous; reply with row, rowID
	U32			listenerIDRet;	// A returned handle used to stop listening.
	U32			listenTypeRet;	// type of operation that caused a listen to happen
	rowID		tableIDRet;		// table ID returned...
} ListenPayload;


//  SG items (request parameters) where data is sent into PTS
#define LISTEN_MSG_TABLENAME_SGI							0
#define LISTEN_MSG_ROWKEY_FIELDNAME_SGI						1
#define LISTEN_MSG_ROWKEY_FIELDVALUE_SGI					2
#define LISTEN_MSG_FIELDNAME_SGI							3
#define LISTEN_MSG_FIELDVALUE_SGI							4

// SG Items where data is returned from the PTS
 //	PTABLE_SGI is a dynamic reply SGI:  Null pointer and count are added to message and transport
 //	allocates the buffer and returns the pointer and the size of the buffer
 // This parameter is only used in the original request:  it returns the entire table if requested
 //	at the address of the new buffer.

#define LISTEN_REPLY_PTABLE_SGI								5	// processed by ListenTable
 // ID and OPT_ROW and LISTENTYPE:  Null pointer, count defined;  transport allocates buffer and
 // returns pointer
#define LISTEN_REPLY_ID_AND_OPT_ROW_SGI						6	 // processed by SearchListenQueue
//#define LISTEN_REPLY_LISTENTYPE_SGI							7	 // processed by SearchListenQueue &ListenTable


/**************************************************************************/
/* StopListen message structures                                       */
/**************************************************************************/

//
// StopListenPayload -
// The structure used as the payload of the message to stop listening on tables

typedef struct StopListenPayload {
	U32		listenerID;				// The handle returned from the Listen operation.
} StopListenPayload;

#endif	// _TableMsgs_h
