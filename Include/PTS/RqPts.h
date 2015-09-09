/* RqPts.h -- Request Interface to Pts Ddm.
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Conventions:
 * 		To use a request's payload definition: RqPtsDefineTable::Payload
 *		To reference a request code: RqPtsDefineTable::RequestCode
 *
 * Caveat:
 *		Note that using a pointer to the SGL may cause problems since
 *		the SGL data maybe fragmented if it is larger than a transport
 *		buffer.
 *
**/

// Revision History:
//  8/08/99 Tom Nelson: Create file
//  * History at end of file *


// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890


#ifndef __RqPts_h
#define __RqPts_h

#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"

#include "PtsCommon.h"
#include "TableMsgs.h"

// RqPtsMessage -- Base Class for all derived PTS messages -----------------------------------------
class RqPtsMessage : public Message {
public:
	RqPtsMessage(REQUESTCODE rqCode) : Message(rqCode) {}
	
	U32 strsize(const char *pStr)	{ return pStr == NULL ? 0 : strlen(pStr)+1; }
};

// RqPtsDefineTable --------------------------------------------------------------------------------
//
// Define Table:
//		RqPtsDefineTable(U32 cEntriesRsv,U32 persistFlags)
//		RqPtsDefineTable(U32 cEntriesRsv,U32 persistFlags,char *psTablename,fieldDef *prgFieldDefs,U32 cbrgFieldDefs)
//
class RqPtsDefineTable : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_DEFINE_TABLE };

	struct Payload  {
		U32			cEntriesRsv;	// Number of table entries to create initially.
		U32			persistFlags;	// Should this table be persistant?	 flags?????
		rowID		tableIdRet;		// Returned Id of table defined in rowID format
	
		Payload(U32 _persistFlags=0,U32 _cEntriesRsv=0) : cEntriesRsv(_cEntriesRsv), persistFlags(_persistFlags) {}
	};
		
	Payload payload;
	
	// Construct with all fields
	RqPtsDefineTable(const char *_psTableName,const U32 _persistFlags,U32 _cEntriesRsv,
					 const fieldDef *_prgFieldDefs, U32 _cbrgFieldDefs)
	: RqPtsMessage(RequestCode), payload(_persistFlags,_cEntriesRsv) {
		SetTableName(_psTableName);
		SetTableDefs(_prgFieldDefs,_cbrgFieldDefs);
	}
	// Construct with payload only
	RqPtsDefineTable(U32 _persistFlags, U32 _cEntriesRsv) 
	: RqPtsMessage(RequestCode), payload(_persistFlags,_cEntriesRsv) {
	}

	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	// Add the table name to the message as an SGL item.
	void SetTableName(const char *_psTableName) {
		AddSgl(DEFINE_TABLE_MSG_TABLENAME_SGI, (void*)_psTableName, strsize(_psTableName), SGL_COPY);
	}

	// Add the return buffer to the message as an SGL item.
	void SetTableDefs(const fieldDef* _prgFieldDefs, U32 _cbrgFieldDefs) {
		AddSgl(DEFINE_TABLE_MSG_FIELDDEFS_SGI, (void*)_prgFieldDefs, _cbrgFieldDefs, SGL_COPY );
	}
	
	rowID GetTableId()	{ return payload.tableIdRet; }
};
	
// RqPtsDeleteTable --------------------------------------------------------------------------------
//
// Delete Table:
//		RqPtsDeleteTable(tableId)
//		RsPtsDeleteTable(char *psTableName)
//
class RqPtsDeleteTable : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_DELETE_TABLE };

	struct Payload {
		rowID		tableId;	// Number of table entries to create initially.
		
		Payload() {}
		Payload(rowID _tableId) : tableId(_tableId) {}
	};
		
	Payload payload;
	
	// Constructor - Delete by tableId
	RqPtsDeleteTable(rowID _tableId) : RqPtsMessage(RequestCode), payload(_tableId) {
		// Add zero length table name to the message as an SGL item.
		AddSgl(DELETE_TABLE_MSG_TABLENAME_SGI, NULL, 0, SGL_COPY );
	}
	
	// Constructor - Delete by Table name
	RqPtsDeleteTable(const char *_psTableName) : RqPtsMessage(RequestCode) {
		// Add the table name to the message as an SGL item.
		AddSgl(DELETE_TABLE_MSG_TABLENAME_SGI,(void*) _psTableName, strsize(_psTableName), SGL_COPY );
	}

	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
};
	
// RqPtsQuerySetRID --------------------------------------------------------------------------------
//
// Query/Set Row Id:
//		RqPtsQuerySetRID(char *psTableName,fieldOpType opRid)
//
class RqPtsQuerySetRID : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_QUERY_SET_RID };

	struct Payload {
		fieldOpType	opRID;		// OpQueryRID or OpSetRID
		RowId		rowId;		// rowID returned
		
		Payload(fieldOpType _opRID) : opRID(_opRID) {}
		Payload(fieldOpType _opRID,rowID _rowId) : opRID(_opRID),rowId(_rowId) {}
		Payload(fieldOpType _opRID, U32 id) : opRID(_opRID), rowId(id) { }
	};
		
	Payload payload;
	
	// Constructor - Set RowID via RowId
	RqPtsQuerySetRID(const char *_psTableName, rowID rid) : RqPtsMessage(RequestCode), payload(OpSetRID,rid) {
		
		// Add the Table Name as SGL of the message
		AddSgl(QUERY_SET_RID_MSG_TABLENAME_SGI, (void*)_psTableName, strsize(_psTableName), SGL_COPY);
	}
	// Constructor -- Set RowID via U32
	RqPtsQuerySetRID(const char *_psTableName,U32 id) : RqPtsMessage(RequestCode), payload(OpSetRID,id) {
		
		// Add the Table Name as SGL of the message
		AddSgl(QUERY_SET_RID_MSG_TABLENAME_SGI, (void*)_psTableName, strsize(_psTableName), SGL_COPY);
	}
	
	// Constructor - Query RowID
	RqPtsQuerySetRID(const char *_psTableName) : RqPtsMessage(RequestCode), payload(OpQueryRID) {
		
		// Add the Table Name as SGL of the message
		AddSgl(QUERY_SET_RID_MSG_TABLENAME_SGI, (void*)_psTableName, strsize(_psTableName), SGL_COPY);
	}

	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	rowID GetRowId() 	{ return payload.rowId; }
};

// RqPtsGetTableDef --------------------------------------------------------------------------------
//
// Get Table Definition:
//		RqPtsGetTableDef(char *psTableName, fieldDef *prgFieldDefsRet, U32 cbrgFieldDefsMax)
//
class RqPtsGetTableDef : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_GET_TABLE_DEF };

	struct Payload {
		U32 persistFlagsRet;	// Is this table persistant?
		U32 cbFieldDefsRet;		// The number of entries returned in the fielddef array.
		RowId rid;
		
		Payload(RowId _rid) : persistFlagsRet(0),cbFieldDefsRet(0),rid(_rid) {} 
	};
		
	Payload payload;
	
	void AddSgls(const char *_psTableName) {
		// Copy the table name to the message as an SGL item.
		AddSgl(GET_TABLE_DEF_MSG_TABLENAME_SGI,(void*) _psTableName, strsize(_psTableName), SGL_COPY);
	
		// Add the return buffer to the message as an SGL item.
		AddSgl(GET_TABLE_DEF_REPLY_FIELDDEFS_SGI, NULL, 0, SGL_DYNAMIC_REPLY);
	
		// Auto allocate the reply buffer for the TableDef
		AddSgl(GET_TABLE_DEF_REPLY_TABLEDEF_SGI, NULL, sizeof(tableDef), SGL_REPLY);
	}

	// Constructor - Get by table name
	RqPtsGetTableDef(const char *_psTableName) 
	: payload(0), RqPtsMessage(RequestCode) {
		AddSgls(_psTableName);
	}
	// Constructor - Get by tableId (in rowId)
	RqPtsGetTableDef(RowId rid) 
	: payload(rid), RqPtsMessage(RequestCode) {
		AddSgls(NULL);
	}
	
	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	U32 GetFieldDefsRetSize() 	{ return payload.cbFieldDefsRet; }
	
	U32 GetPersistFlags()		{ return payload.persistFlagsRet; 	 }
	
	U32 GetTableDefDataSize() {
		return GetSglDataSize(GET_TABLE_DEF_REPLY_TABLEDEF_SGI);
	}
	// Return pointer to table def
	tableDef* GetTableDefDataPtr(U32 *pcbData=NULL, U32 sType=0) { 
		return (tableDef*)GetSglDataPtr(GET_TABLE_DEF_REPLY_TABLEDEF_SGI,pcbData,sType);
	}
	// Return copy of table def
	tableDef* GetTableDataCopy(U32 *pcbData=NULL, U32 sType=0) {
		return (tableDef*)GetSglDataCopy(GET_TABLE_DEF_REPLY_TABLEDEF_SGI,pcbData,sType);
	}
	
	U32 GetFieldDefsDataSize() {
		return GetSglDataSize(GET_TABLE_DEF_REPLY_FIELDDEFS_SGI);
	}
	// Return pointer to field defs
	fieldDef* GetFieldDefsDataPtr(U32 *pcbData=NULL, U32 sType=0) { 
		return (fieldDef*)GetSglDataPtr(GET_TABLE_DEF_REPLY_FIELDDEFS_SGI,pcbData,sType);
	}
	// Return copy of field defs
	fieldDef* GetFieldsDataCopy(U32 *pcbData=NULL, U32 sType=0) {
		return (fieldDef*)GetSglDataCopy(GET_TABLE_DEF_REPLY_FIELDDEFS_SGI,pcbData,sType);
	}

	U32 GetRowDataCount() 	{ return GetTableDefDataPtr()->cRows; }
	
	U32 GetRowDataSize()  	{ return GetTableDefDataPtr()->cbRow; }
	
	U32 GetFieldDataCount() { return GetTableDefDataPtr()->cFields; }
};

// RqPtsEnumerateTable -----------------------------------------------------------------------------
//
// Enumerate Table:
//
class RqPtsEnumerateTable : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_ENUMERATE_TABLE };

	struct Payload {
		U32	uStartingRow;	// Offset of first row to return (0 based).
		U32	cbDataRet;		// The number of bytes of returned data.

		Payload(U32 _uStartRow) : uStartingRow(_uStartRow), cbDataRet(0) {}
	};
		
	Payload payload;
	
	// Constructor
	RqPtsEnumerateTable(const char *_psTableName, U32 _uStartingRow, U32 _cbRowDataRet) 
	: RqPtsMessage(RequestCode), payload(_uStartingRow) {
	
		// Copy the table name to the message as an SGL item.
		AddSgl(ENUMERATE_TABLE_MSG_TABLENAME_SGI,(void*) _psTableName, strsize(_psTableName), SGL_COPY);
	
		// Add the return buffer to the message as a scatter gather list item.
		AddSgl(ENUMERATE_TABLE_REPLY_SGI, NULL,  _cbRowDataRet, SGL_DYNAMIC_REPLY);
	}
	
	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	// Returns number of bytes returned from PTS
	U32 GetRowDataSize() const 	{ return payload.cbDataRet; }

	// Get ptr to Row Data sent.
	void *GetRowDataPtr(U32 *pcbData=NULL, U32 sType=0) {
		return GetSglDataPtr(ENUMERATE_TABLE_REPLY_SGI,pcbData,sType);
	}
	// Return copy of Row Data
	void* GetRowDataCopy(U32 *pcbData=NULL, U32 sType=0) {
		return GetSglDataCopy(ENUMERATE_TABLE_REPLY_SGI,pcbData,sType);
	}
};

// RqPtsInsertRow ----------------------------------------------------------------------------------
//
// Insert Row
//
class RqPtsInsertRow : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_INSERT_ROW };

	struct Payload {
		U32	cIDsRet;		// Number of rows actually inserted.
		
		Payload() : cIDsRet(0) {}
	};
		
	Payload payload;
	
	// Constructor
	RqPtsInsertRow(const char *_psTableName, const void *_prgbRowData, U32 _cbRowData) 
	: RqPtsMessage(RequestCode) {
		// Build the SGL, first copy TableName to SGL
		AddSgl(INSERT_ROWS_MSG_TABLENAME_SGI,(void*) _psTableName, strsize(_psTableName), SGL_COPY);

		// Add the row data
		AddSgl(INSERT_ROWS_MSG_DATA_BUFFER_SGI, (void*)_prgbRowData, _cbRowData, SGL_COPY);

		// Auto allocate the return buffer for the rowId(s) returned
		// ..this needs to support SGL_DYNAMIC_REPLY so we can eliminate the _nRows parameter
		AddSgl(INSERT_ROWS_REPLY_ROWIDS_SGI, NULL, 0, SGL_DYNAMIC_REPLY);
	}
	
	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	U32 GetRowDataSize() {
		return GetSglDataSize(INSERT_ROWS_MSG_DATA_BUFFER_SGI);
	}
	// Get ptr to Row Data sent.
	void *GetRowDataPtr(U32 *pcbData=NULL, U32 sType=0) {
		return GetSglDataPtr(INSERT_ROWS_MSG_DATA_BUFFER_SGI,pcbData,sType);
	}
	
	// Get size of rowID(s) of Row(s) inserted
	U32 GetRowIdDataSize() {
		return GetSglDataSize(INSERT_ROWS_REPLY_ROWIDS_SGI);
	}
	
	// Returns Pointer to rowID(s) of Row(s) inserted
	RowId *GetRowIdDataPtr(U32 *pcbData=NULL, U32 sType=0) {
		return (RowId*) GetSglDataPtr(INSERT_ROWS_REPLY_ROWIDS_SGI,pcbData,sType);
	}
	// Return copy of rowID(s) of Row(s) inserted
	RowId* GetRowIdDataCopy(U32 *pcbData=NULL, U32 sType=0) {
		return (RowId*)GetSglDataCopy(INSERT_ROWS_REPLY_ROWIDS_SGI,pcbData,sType);
	}
	
	// Returns Number of entries actually inserted
	U32 GetRowIdDataCount() 	{ return payload.cIDsRet; }
};

// RqPtsModifyRow ----------------------------------------------------------------------------------
//
// Modify Row
//
class RqPtsModifyRow : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_MODIFY_ROW};

	struct Payload {
		U32	cRowsToModify;		// number of rows to modify;  0 means ALL that match
		U32	cRowsModifiedRet;	// Number of entries actually modified.
		
		Payload(U32 _cRowsToModify) : cRowsToModify(_cRowsToModify),cRowsModifiedRet(0) {}
	};
		
	Payload payload;
	
	void AddSgls(const char *_psTableName,
				 const char *_psKeyFieldName, const void *_pKeyFieldValue, U32 _cbKeyFieldValue,
				 const void *_prgbRowData, U32 _cbRowData) {
		// Copy the table name to the message as an SGL item.
		AddSgl(GET_TABLE_DEF_MSG_TABLENAME_SGI,(void*) _psTableName, strsize(_psTableName), SGL_COPY);

		// Copy the key field name to SGL
		AddSgl(MODIFY_ROWS_MSG_KEY_NAMES_SGI, (void*) _psKeyFieldName, strsize(_psKeyFieldName), SGL_COPY);

		// Add the key field value
		AddSgl(MODIFY_ROWS_MSG_KEY_VALUES_SGI,(void*) _pKeyFieldValue, _cbKeyFieldValue, SGL_COPY);

		// Add the data
		AddSgl(MODIFY_ROWS_MSG_DATA_BUFFER_SGI,(void*) _prgbRowData, _cbRowData, SGL_COPY);

		// Add the return buffer to the message as SGL
		AddSgl(MODIFY_ROWS_REPLY_ROWIDS_SGI, NULL, 0, SGL_DYNAMIC_REPLY);
	}
	
	// Constructor - generic
	RqPtsModifyRow(const char *_psTableName,
				   const char *_psKeyFieldName, const void *_pKeyFieldValue, U32 _cbKeyFieldValue,
				   const void *_prgbRowData, U32 _cbRowData,
				   U32	 _cRowsToModify=0 ) 
	: RqPtsMessage(RequestCode), payload(_cRowsToModify) {
		
		AddSgls(_psTableName,_psKeyFieldName,_pKeyFieldValue,_cbKeyFieldValue,_prgbRowData,_cbRowData);
	}
	
	// Constructor with rowID KeyField
	RqPtsModifyRow(const char *_psTableName,
				   const rowID rid,
				   const void *_prgbRowData, U32 _cbRowData,
				   U32	 _cRowsToModify=0 ) 
	: RqPtsMessage(RequestCode), payload(_cRowsToModify) {
		
		AddSgls(_psTableName, CT_PTS_RID_FIELD_NAME, &rid, sizeof(rid), _prgbRowData, _cbRowData);
	}
	
	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	// Get size of rowID(s) of Row(s) modified
	U32 GetRowIdDataSize() {
		return GetSglDataSize(MODIFY_ROWS_REPLY_ROWIDS_SGI);
	}
	
	// Returns Pointer to rowID(s) of Row(s) modified
	RowId *GetRowIdDataPtr(U32 *pcbData=NULL, U32 sType=0) {
		return (RowId*)GetSglDataPtr(MODIFY_ROWS_REPLY_ROWIDS_SGI,pcbData,sType);
	}
	// Return copy of rowID(s) of Row(s) modified
	RowId* GetRowIdDataCopy(U32 *pcbData=NULL, U32 sType=0) {
		return (RowId*)GetSglDataCopy(MODIFY_ROWS_REPLY_ROWIDS_SGI,pcbData,sType);
	}

	// Returns Number of entries actually modified
	U32 GetRowDataCount() 	{ return payload.cRowsModifiedRet; }
};

// RqPtsModifyField --------------------------------------------------------------------------------
//
// Modify Field
//
class RqPtsModifyField : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_MODIFY_FIELD };

	struct Payload {
		U32	cRowsToModify;		// Number of row to modify. 0=ALL that match
		U32	cRowsModifiedRet;	// The number of rows actually modified.
		
		Payload(U32 _cRowsToModify) : cRowsToModify(_cRowsToModify), cRowsModifiedRet(0) {}
	};
		
	Payload payload;
	
	void AddSgls(const char *_psTableName,
				 const char *_psKeyFieldName, const void *_pbKeyFieldValue, U32 _cbKeyFieldValue,
				 const char *_psFieldName,	  const void *_pbFieldValue,	U32 _cbFieldValue) {
				 
		// Copy the Table Name to an SGL
		AddSgl(MODIFY_FIELDS_MSG_TABLENAME_SGI,(void*) _psTableName, strsize(_psTableName), SGL_COPY);

		// Copy the Key Field Name to an SGL
		AddSgl(MODIFY_FIELDS_MSG_KEY_NAMES_SGI,	(void*)_psKeyFieldName, strsize(_psKeyFieldName), SGL_COPY);
	
		// Add the data of Key Field to the SGL
		AddSgl(MODIFY_FIELDS_MSG_KEY_VALUES_SGI, (void*)_pbKeyFieldValue, _cbKeyFieldValue,	SGL_COPY);

		// Copy the modified field value to an SGL
		AddSgl(MODIFY_FIELDS_MSG_FIELD_NAMES_SGI, (void*)_psFieldName, strsize(_psFieldName), SGL_COPY);

		// Add the data of Key Field to the SGL
		AddSgl(MODIFY_FIELDS_MSG_FIELD_VALUES_SGI, (void*)_pbFieldValue, _cbFieldValue, SGL_COPY);

		// Add the return buffer to the message as SGL
		AddSgl(MODIFY_FIELDS_REPLY_ROWIDS_SGI, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// Constructor - generic
	RqPtsModifyField(const char *_psTableName, 
					 const char *_psKeyFieldName, const void *_pbKeyFieldValue, U32 _cbKeyFieldValue,
					 const char *_psFieldName,    const void *_pbFieldValue,    U32 _cbFieldValue,
					 U32   _cRowsToModify=0 ) 
	: RqPtsMessage(RequestCode), payload(_cRowsToModify) {

		AddSgls(_psTableName,
				_psKeyFieldName, _pbKeyFieldValue, _cbKeyFieldValue,
				_psFieldName,	 _pbFieldValue,	   _cbFieldValue);
	}

	// Constructor with RowId KeyField
	RqPtsModifyField(const char *_psTableName, 
					 const rowID rid,
					 const char *_psFieldName,    const void *_pbFieldValue,   U32 _cbFieldValue,
					 U32   _cRowsToModify=0 ) 
	: RqPtsMessage(RequestCode), payload(_cRowsToModify) {
		
		AddSgls(_psTableName, 
				CT_PTS_RID_FIELD_NAME, &rid, sizeof(rid), 
				_psFieldName, _pbFieldValue, _cbFieldValue);
	}
	
	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	// Get size of rowID(s) of Row(s) modified
	U32 GetRowIdDataSize() {
		return GetSglDataSize(MODIFY_FIELDS_REPLY_ROWIDS_SGI);
	}
	
	// Returns Pointer to rowID(s) of Row(s) modified
	RowId *GetRowIdDataPtr(U32 *pcbData=NULL, U32 sType=0) {
		return (RowId*)GetSglDataPtr(MODIFY_FIELDS_REPLY_ROWIDS_SGI,pcbData,sType);
	}
	// Return copy of rowID(s) of Row(s) modified
	RowId* GetRowIdDataCopy(U32 *pcbData=NULL, U32 sType=0) {
		return (RowId*)GetSglDataCopy(MODIFY_ROWS_REPLY_ROWIDS_SGI,pcbData,sType);
	}

	// Returns Number of entries actually modified
	U32 GetRowDataCount() 	{ return payload.cRowsModifiedRet; }
};

#if 0	// Use RqPtsModifyField using constructor with RowId KeyField

// RqPtsModifyFieldByRow ---------------------------------------------------------------------------
//
// Modify field in specified row (rowId)
//
class RqPtsModifyFieldByRow : public RqPtsModifyField {
	
	RqPtsModifyFieldByRow(const char *_psTableName,
						  rowID rid,
						  const char *_psFieldName, const void *_pbFieldValue, U32 _cbFieldValue)
	: RqPtsModifyField(_psTableName, CT_PTS_RID_FIELD_NAME, &rid, sizeof(rid), _psFieldName, _pbFieldValue, _cbFieldValue, 1) {}
};
#endif
				
// RqPtsModifyBits ---------------------------------------------------------------------------------
//
// Modify Bits
//
class RqPtsModifyBits : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_MODIFY_BITS };

	struct Payload {
		fieldOpType opFlag;	
		U32	cRowsToModify;		// Number of row to modify. 0=ALL that match
		U32	cRowsModifiedRet;	// The number of rows actually modified.
		
		Payload(fieldOpType _opFlag,U32 _cRowsToModify) : opFlag(_opFlag),cRowsToModify(_cRowsToModify), cRowsModifiedRet(0) {}
	};
		
	Payload payload;

	void AddSgls(const char *_psTableName,
				 const char *_psKeyFieldName, const void *_pbKeyFieldValue, U32 _cbKeyFieldValue,
				 const char *_psFieldName,    const void *_pbFieldMask,     U32 _cbFieldMask) {
		// Copy the Table Name to an SGL
		AddSgl(MODIFY_BITS_MSG_TABLENAME_SGI,(void*) _psTableName, strsize(_psTableName), SGL_COPY);

		// Copy the Key Field Name to an SGL
		AddSgl(MODIFY_BITS_MSG_KEY_NAME_SGI, (void*)_psKeyFieldName, strsize(_psKeyFieldName), SGL_COPY);
	
		// Add the data of Key Field to the SGL
		AddSgl(MODIFY_BITS_MSG_KEY_VALUE_SGI, (void*)_pbKeyFieldValue, _cbKeyFieldValue,	SGL_COPY);

		// Copy the modified field value to an SGL
		AddSgl(MODIFY_BITS_MSG_FIELD_NAME_SGI, (void*)_psFieldName, strsize(_psFieldName), SGL_COPY);

		// Add the data of Key Field Mask to the SGL
		AddSgl(MODIFY_BITS_MSG_FIELD_MASK_SGI, (void*)_pbFieldMask, _cbFieldMask, SGL_COPY);

		// Add the return buffer to the message as SGL
		AddSgl(MODIFY_BITS_REPLY_ROWIDS_SGI, NULL, 0, SGL_DYNAMIC_REPLY);
	}
	
	// Constructor - generic
	RqPtsModifyBits(const char *_psTableName, 	 fieldOpType _opFlag,
					const char *_psKeyFieldName, const void *_pbKeyFieldValue, U32 _cbKeyFieldValue,
					const char *_psFieldName,    const void *_pbFieldMask,     U32 _cbFieldMask,
					U32   _cRowsToModify=0 ) 
	: RqPtsMessage(RequestCode), payload(_opFlag,_cRowsToModify) {

		AddSgls(_psTableName,
				_psKeyFieldName, _pbKeyFieldValue, _cbKeyFieldValue,
				_psFieldName,	 _pbFieldMask,	   _cbFieldMask);
	}

	// Constructor with RowId KeyField
	RqPtsModifyBits(const char *_psTableName, 	 fieldOpType _opFlag,
					const RowId rid,
					const char *_psFieldName,    const void *_pbFieldMask,     U32 _cbFieldMask,
					U32   _cRowsToModify=0 ) 
	: RqPtsMessage(RequestCode), payload(_opFlag,_cRowsToModify) {

		AddSgls(_psTableName,
				CT_PTS_RID_FIELD_NAME, &rid, sizeof(rid), 
				_psFieldName,	 _pbFieldMask,	   _cbFieldMask);
	}


	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	// Get size of rowID(s) of Row(s) modified
	U32 GetRowIdDataSize() {
		return GetSglDataSize(MODIFY_BITS_REPLY_ROWIDS_SGI);
	}
	
	// Returns Pointer to rowID(s) of Row(s) modified
	RowId *GetRowIdDataPtr(U32 *pcbData=NULL, U32 sType=0) {
		return (RowId*)GetSglDataPtr(MODIFY_BITS_REPLY_ROWIDS_SGI,pcbData,sType);
	}
	// Return copy of rowID(s) of Row(s) modified
	RowId* GetRowIdDataCopy(U32 *pcbData=NULL, U32 sType=0) {
		return (RowId*)GetSglDataCopy(MODIFY_BITS_REPLY_ROWIDS_SGI,pcbData,sType);
	}

	// Returns Number of entries actually modified
	U32 GetRowDataCount() 	{ return payload.cRowsModifiedRet; }
};


// RqPtsTestAndSetField ----------------------------------------------------------------------------
//
// Test And Set Field
//
class RqPtsTestAndSetField : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_TEST_SET_FIELD };

	struct Payload {
		fieldOpType	opSetOrClear;
		BOOL		fTestRet;		// result of the test operation.
		
		Payload(fieldOpType _opSetOrClear) : opSetOrClear(_opSetOrClear), fTestRet(FALSE) {}
	};
		
	Payload payload;

	void AddSgls(const char *_psTableName,
				const char *_psKeyFieldName, const void *_pbKeyFieldValue, U32 _cbKeyFieldValue,
				const char *_psFieldName) {
	
		// Copy the Table Name to an SGL
		AddSgl(TEST_SET_FIELD_MSG_TABLENAME_SGI,(void*) _psTableName, strsize(_psTableName), SGL_COPY);

		// Copy the Key Field Name to an SGL
		AddSgl(TEST_SET_FIELD_MSG_KEY_NAME_SGI,(void*) _psKeyFieldName, strsize(_psKeyFieldName), SGL_COPY);
	
		// Add the data of Key Field to the SGL
		AddSgl(TEST_SET_FIELD_MSG_KEY_VALUE_SGI,(void*) _pbKeyFieldValue, _cbKeyFieldValue,	SGL_COPY);

		// Add the modified field value as SGL of the message
		AddSgl(TEST_SET_FIELD_MSG_FIELD_NAME_SGI,(void*)_psFieldName, strsize(_psFieldName), SGL_COPY);
	}
	
	// Constructor - Generic
	RqPtsTestAndSetField(const char *_psTableName,    fieldOpType opFlag,
						 const char *_psKeyFieldName, const void *_pbKeyFieldValue, U32 _cbKeyFieldValue,
						 const char *_psFieldName) 
	: RqPtsMessage(RequestCode), payload(opFlag) {
				
		AddSgls(_psTableName,
				_psKeyFieldName, _pbKeyFieldValue, _cbKeyFieldValue,
				_psFieldName);		 
	}
	
	// Constructor with RowId KeyField
	RqPtsTestAndSetField(const char *_psTableName,    fieldOpType opFlag,
						 const RowId rid,
						 const char *_psFieldName) 
	: RqPtsMessage(RequestCode), payload(opFlag) {
				
		AddSgls(_psTableName,
				CT_PTS_RID_FIELD_NAME, &rid, sizeof(rid), 
				_psFieldName); 
	}
	
	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	// Returns Number of entries actually modified
	U32 GetTestFlag() 	{ return payload.fTestRet; }
};

// RqPtsReadRow ------------------------------------------------------------------------------------
//
// Read Row
//
class RqPtsReadRow : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_READ_ROW};

	struct Payload {
		U32	cRowsReadRet;		// Number of rows sucessfully Read.
		
		Payload() : cRowsReadRet(0) {}
	};
		
	Payload payload;
	
	void AddSgls(const char *_psTableName, 
				 const char *_psKeyFieldName, const void *_pbKeyFieldValue, U32 _cbKeyFieldValue) {
				 
		// Copy the Table Name to an SGL
		AddSgl(READ_ROWS_MSG_TABLENAME_SGI,(void*) _psTableName, strsize(_psTableName), SGL_COPY);

		// Copy the key field name to an SGL
		AddSgl(READ_ROWS_MSG_KEY_NAMES_SGI, (void*) _psKeyFieldName, strsize(_psKeyFieldName), SGL_COPY);

		// Add the key field value to SGL
		AddSgl(READ_ROWS_MSG_KEY_VALUES_SGI, (void*)_pbKeyFieldValue, _cbKeyFieldValue, SGL_COPY);

		// Add the returned data buffer to the message as a SGL item.
		AddSgl(READ_ROWS_REPLY_DATA_BUFFER_SGI, NULL, 0, SGL_DYNAMIC_REPLY);
	}
	
	// Constructor - generic
	RqPtsReadRow(const char *_psTableName, 
				 const char *_psKeyFieldName, const void *_pbKeyFieldValue, U32 _cbKeyFieldValue) 
	: RqPtsMessage(RequestCode) {
		
		AddSgls(_psTableName, 
				_psKeyFieldName, _pbKeyFieldValue, _cbKeyFieldValue);
	}

	// Constructor with RowId KeyField
	RqPtsReadRow(const char *_psTableName, 
				 const RowId rid) 
	: RqPtsMessage(RequestCode) {
		
		AddSgls(_psTableName, 
				CT_PTS_RID_FIELD_NAME, &rid, sizeof(rid));
	}


	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	// Get Row Data Ptr returned
	U32 GetRowDataSize() {
		return GetSglDataSize(READ_ROWS_REPLY_DATA_BUFFER_SGI);
	}
	// Return pointer to row data
	void *GetRowDataPtr(U32 *pcbData=NULL, U32 sType=0) {
		return GetSglDataPtr(READ_ROWS_REPLY_DATA_BUFFER_SGI,pcbData,sType);
	}
	// Return copy of row data
	void* GetRowDataCopy(U32 *pcbData=NULL, U32 sType=0) {
		return GetSglDataCopy(READ_ROWS_REPLY_DATA_BUFFER_SGI,pcbData,sType);
	}
	
	// Returns Number of entries actually read
	U32 GetRowDataCount() 	{ return payload.cRowsReadRet; }
};

// RqPtsDeleteRow ----------------------------------------------------------------------------------
//
// Delete Row
//
class RqPtsDeleteRow : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_DELETE_ROW };

	struct Payload {
		U32	cRowsToDelete;		// Number of Rows to Delete.
		U32	cRowsDeletedRet;	// Number of rows sucessfully deleted.
		
		Payload(U32 _cRowsToDelete=1) : cRowsToDelete(_cRowsToDelete),cRowsDeletedRet(0) {}
	};
		
	Payload payload;
	
	void AddSgls(const char *_psTableName,
				const char *_psKeyFieldName,const void *_pbKeyFieldValue,U32 _cbKeyFieldValue) {
				
		// Copy the Table Name to an SGL
		AddSgl(DELETE_ROWS_MSG_TABLENAME_SGI,(void*) _psTableName, strsize(_psTableName), SGL_COPY);
		
		// Copy the key field name to an SGL
		AddSgl(DELETE_ROWS_MSG_KEY_NAMES_SGI, (void*)_psKeyFieldName, strsize(_psKeyFieldName), SGL_COPY);

		// Add the key field value
		AddSgl(DELETE_ROWS_MSG_KEY_VALUES_SGI, (void*)_pbKeyFieldValue, _cbKeyFieldValue, SGL_COPY);
	}
	
	// Constructor - generic
	RqPtsDeleteRow(const char *_psTableName,
				   const char *_psKeyFieldName,const void *_pbKeyFieldValue,U32 _cbKeyFieldValue) 
	: RqPtsMessage(RequestCode) {

		AddSgls(_psTableName, 
				_psKeyFieldName, _pbKeyFieldValue, _cbKeyFieldValue);
	}
	
	// Constructor with RowId KeyField
	RqPtsDeleteRow(const char *_psTableName,
				   const RowId rid) 
	: RqPtsMessage(RequestCode) {

		AddSgls(_psTableName, 
				CT_PTS_RID_FIELD_NAME, &rid, sizeof(rid));
	}
	
	
	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	// Returns Number of rows deleted
	U32 GetRowDataCount() 	{ return payload.cRowsDeletedRet; }
};

// RqPtsListen -------------------------------------------------------------------------------------
//
// Listen
//
class RqPtsListen : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_LISTEN };

	struct Payload {
		U32	listenType;		// Type of operation for which you want to listen.
		U32	replyMode;		// reply flags:  reply once, continuous; reply with row, rowID
		U32	listenerIDRet;	// A returned handle used to stop listening.
		U32	listenTypeRet;	// type of operation that caused a listen to happen
		RowId tableId;		// Table ID of this listen
		
		Payload(U32 _listenType,U32 _replyMode) : listenType(_listenType), replyMode(_replyMode), listenerIDRet(0),listenTypeRet(0) {}
	};
		
	Payload payload;
	
	void AddSgls(const char *_psTableName, 
				 const char *_psRowKeyFieldName=NULL, const void *_prgbRowKeyFieldValue=NULL, U32 _cbRowKeyFieldValue=0,
				 const char *_psFieldName=NULL,	   	  const void *_prgbFieldValue=NULL, 	  U32 _cbFieldValue=0) {
		
		// Copy the Table name tp an SGL item
		AddSgl(LISTEN_MSG_TABLENAME_SGI,(void*) _psTableName, strsize(_psTableName), SGL_COPY);

		// Copy the Row Key Field name to an SGL item
		AddSgl(LISTEN_MSG_ROWKEY_FIELDNAME_SGI, (void*)_psRowKeyFieldName, strsize(_psRowKeyFieldName), SGL_COPY);

		// Add the Row Key Field value as an SGL item
		AddSgl(LISTEN_MSG_ROWKEY_FIELDVALUE_SGI, (void*)_prgbRowKeyFieldValue, _cbRowKeyFieldValue, SGL_COPY);
		
		// Copy the Field name to an SGL item
		AddSgl(LISTEN_MSG_FIELDNAME_SGI, (void*)_psFieldName, strsize(_psFieldName), SGL_COPY);

		// Add the Field value as an SGL item
		AddSgl(LISTEN_MSG_FIELDVALUE_SGI, (void*)_prgbFieldValue, _cbFieldValue, SGL_COPY);

		// Only add an SGL item for the returned Table data if the user wants it.
		// Add the return Table Data SGL item
//		if (payload.replyMode & ReplyFirstWithTable)
			AddSgl(LISTEN_REPLY_PTABLE_SGI, NULL,0, SGL_DYNAMIC_REPLY);

		// Add the optional row data buffer as an SGL item
		AddSgl(LISTEN_REPLY_ID_AND_OPT_ROW_SGI,	NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// Constructor - Generic
	RqPtsListen(U32	  _listenType, U32 _replyMode,
				const char *_psTableName, 
				const char *_psRowKeyFieldName=NULL, const void *_prgbRowKeyFieldValue=NULL, U32 _cbRowKeyFieldValue=0,
				const char *_psFieldName=NULL,	   	 const void *_prgbFieldValue=NULL, 		 U32 _cbFieldValue=0)
	: RqPtsMessage(RequestCode), payload(_listenType,_replyMode) {
		
		AddSgls(_psTableName, 
				_psRowKeyFieldName, _prgbRowKeyFieldValue, _cbRowKeyFieldValue,
				_psFieldName,		_prgbFieldValue,	   _cbFieldValue);
	}

	// Constructor with RowId KeyField
	RqPtsListen(U32	  _listenType, U32 _replyMode,
				const char *_psTableName, 
				const RowId &rid,
				const char *_psFieldName=NULL, const void *_prgbFieldValue=NULL, U32 _cbFieldValue=0)
	: RqPtsMessage(RequestCode), payload(_listenType,_replyMode) {
		
		AddSgls(_psTableName, 
				CT_PTS_RID_FIELD_NAME, &rid, sizeof(rid),
				_psFieldName,		_prgbFieldValue,	   _cbFieldValue);
	}
	

	char *GetTableNamePtr(U32 *pcbTableName=NULL) {
		return (char*) GetSglDataPtr(READ_ROWS_MSG_TABLENAME_SGI,pcbTableName);
	}
	
	U32 GetListenerId() 	{ return payload.listenerIDRet; }
	
	U32 GetListenTypeRet() 	{ return payload.listenTypeRet; }
	
	BOOL IsInitialReply()	{ return GetListenTypeRet() & ListenInitialReply; }
	
	BOOL IsTableData() 		{ return (payload.replyMode & ReplyFirstWithTable) && IsInitialReply(); }
	
	// Get Table Data Size returned
	U32 GetTableDataSize() {
		return GetSglDataSize(LISTEN_REPLY_PTABLE_SGI);
	}
	// Returns pointer to Table data
	void *GetTableDataPtr(U32 *pcbData=NULL, U32 sType=0) {
		return GetSglDataPtr(LISTEN_REPLY_PTABLE_SGI,pcbData,sType);
	}
	// Return copy of Table data
	void *GetTableDataCopy(U32 *pcbData=NULL, U32 sType=0) {
		return GetSglDataCopy(LISTEN_REPLY_PTABLE_SGI,pcbData,sType);
	}

	// Get ModifiedRow Data Size returned
	U32 GetModifiedDataSize() {
		return GetSglDataSize(LISTEN_REPLY_ID_AND_OPT_ROW_SGI);
	}
	// Returns pointer to ModifiedRow data 
	void *GetModifiedDataPtr(U32 *pcbData=NULL, U32 sType=0) {
		return GetSglDataPtr(LISTEN_REPLY_ID_AND_OPT_ROW_SGI,pcbData,sType);
	}
	// Return copy of ModifiedRow data
	void *GetModifiedDataCopy(U32 *pcbData=NULL, U32 sType=0) {
		return GetSglDataCopy(LISTEN_REPLY_ID_AND_OPT_ROW_SGI,pcbData,sType);
	}
	
	// Get Row Data Size returned
	U32 GetRowDataSize() {
		return (IsInitialReply()) ? GetTableDataSize() : GetModifiedDataSize();
	}
	// Returns pointer to Row data 
	void *GetRowDataPtr(U32 *pcbData=NULL, U32 sType=0) {
		return (IsInitialReply()) ? GetTableDataPtr(pcbData,sType) : GetModifiedDataPtr(pcbData,sType);
	}
	// Return copy of Row data
	void *GetRowDataCopy(U32 *pcbData=NULL, U32 sType=0) {
		return (IsInitialReply()) ? GetTableDataCopy(pcbData,sType) : GetModifiedDataCopy(pcbData,sType);
	}
};


// RqPtsStopListen ---------------------------------------------------------------------------------
//
// Stop Listen:
//		RqPtsStopListen(U32 listenerId)
//
class RqPtsStopListen : public RqPtsMessage {
public:
	enum { RequestCode = TS_FUNCTION_STOP_LISTENING };

	struct Payload {
		U32	listenerId;				// The handle returned from the Listen operation.
		
		Payload(U32 _listenerId=0) : listenerId(_listenerId) {}
	};
		
	Payload payload;
	
	// Constructor
	RqPtsStopListen(U32 _listenerId) 
	: RqPtsMessage(RequestCode), payload(_listenerId) {}
};

#endif	// __RqPts_h

//**************************************************************************************************
// Update Log:
// $Log: /Gemini/Include/PTS/RqPts.h $
// 
// 29    1/26/00 3:33p Joehler
// functions need to return RowId instead of rowID or an illegal cast
// results. 
// 
// 28    12/09/99 1:40a Iowa
// 
// 24    9/16/99 4:04p Tnelson
// 
// 23    9/02/99 2:51p Ewedel
// Fixed various SGL bugs, and also changed weird GetDataPtrSize() to
// standard GetRowDataSize().
// 
// 22    8/31/99 10:25p Tnelson
// Made everything const safe
// 
// 21    8/31/99 2:37p Tnelson
// 
// 20    8/31/99 2:36p Tnelson
// 
// 19    8/31/99 2:09p Tnelson
// 
// 18    8/31/99 1:27p Tnelson
// Fixes for Mr. Wedel
// 
// 17    8/27/99 8:34p Tnelson
// Changed GetRowPtr(U32 iRow) to GetRowPtr(U32 *pnRows)
// 
// 16    8/27/99 7:33p Tnelson
// All  requests now based on on RqPtsMessage
// 
// 13    8/27/99 6:15p Tnelson
// New GetRowPtr/GetRowDataPtr
// 
// 12    8/27/99 5:33p Tnelson
// Default reserved entries on DefineTable may not be zero!  This argument
// is really allocation size.
// 
// 11    8/27/99 3:40p Tnelson
// More "const" fixes
// 
// 10    8/27/99 12:04p Tnelson
// Added const to fieldDef parms
// 
// 7     8/26/99 3:20p Tnelson
// Fixes.  Template ALL base PTS requests
// 
// 5     8/22/99 6:04p Tnelson
// Bug Fixes
// 
// 4     8/20/99 11:38a Tnelson
// Not quite ready for prime time
// 
// 3     8/15/99 5:48p Tnelson
// Moved update $Log to end of file
// 
// 2     8/15/99 5:47p Tnelson
// Updated to newest PTS.

