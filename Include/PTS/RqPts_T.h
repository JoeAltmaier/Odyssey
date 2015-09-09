/* RqPts_T.h -- Template Request Interface to Pts.
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
 *		These templates require Pts records to be based on class CPtrRecordBase.
 *		All records must have the following static methods:
 *			TableName(),	FieldDefs(),	FieldDefsSize().
 *
**/

// Revision History:
//  8/22/99 Tom Nelson: Create file
//  * History at end of file *


// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890


#ifndef __RqPts_T_h
#define __RqPts_T_h

#include "RqPts.h"

// RqPtsDefineTable -- Define PTS Table -----------------------------------------------------------
//
template <class REC>
class RqPtsDefineTable_T : public RqPtsDefineTable {
public:
	RqPtsDefineTable_T<REC>(U32 _persistFlags=Persistant_PT,U32 _cEntriesRsv=20) 
	: RqPtsDefineTable(REC::TableName(),_persistFlags,_cEntriesRsv,REC::FieldDefs(),REC::FieldDefsSize()) {}
};


// RqPtsDeleteTable_T -- Delete PTS Tabe -----------------------------------------------------------
//
template <class REC>
class RqPtsDeleteTable_T : public RqPtsDeleteTable {
public:
	RqPtsDeleteTable_T<REC>() : RqPtsDeleteTable(REC::TableName() ) {}
};


// RqPtsQuerySetRID_T -- Query or Set next RowId ---------------------------------------------------
//
template <class REC>
class RqPtsQuerySetRID_T : public RqPtsQuerySetRID {
public:
	// Set
	RqPtsQuerySetRID_T<REC>(const RowId &_rid) 
	: RqPtsQuerySetRID(REC::TableName(),_rid) {}
	
	RqPtsQuerySetRID_T<REC>(U32 _id)
	: RqPtsQuerySetRID(REC::TableName(),_id) {}
	
	// Query
	RqPtsQuerySetRID_T<REC>() 
	: RqPtsQuerySetRID(REC::TableName()) {}
};

// RqPtsGetTableDef_T -- Get Table Definition ------------------------------------------------------
//
template <class REC>
class RqPtsGetTableDef_T : public RqPtsGetTableDef {
	// Constructor
	RqPtsGetTableDef_T<REC>()
	: RqPtsGetTableDef(REC::TableName()) {}
};

// RqPtsEnumerateTable_T -- Enumerate Table --------------------------------------------------------
//
template <class REC>
class RqPtsEnumerateTable_T : public RqPtsEnumerateTable {
public:
	// Constructor
	RqPtsEnumerateTable_T<REC>(U32 _uStartingRow,U32 _nRows)
	: RqPtsEnumerateTable(REC::TableName(),_uStartingRow,sizeof(REC) * _nRows) {}
	
	// Constructor - Entire Table
	RqPtsEnumerateTable_T<REC>()
	: RqPtsEnumerateTable(REC::TableName(),0,0) {}

	U32 GetRowCount() {
		return GetRowDataSize() / sizeof(REC);
	}

	REC *GetRowPtr(U32 *pnEntry=NULL) {
		return (REC*) GetRowDataPtr(pnEntry,sizeof(REC));
	}
	REC *GetRowCopy(U32 *pnEntry=NULL) {
		return (REC*) GetRowDataCopy(pnEntry,sizeof(REC));
	}
};

// RqPtsInsertRow_T -- Insert Row in PTS Table -----------------------------------------------------
//
template <class REC>
class RqPtsInsertRow_T : public RqPtsInsertRow {
public:
	// Constructor - Multiple records
	RqPtsInsertRow_T<REC>(const REC *_precData,U32 _nRows) 
	: RqPtsInsertRow(REC::TableName(),_precData,sizeof(REC) * _nRows) {}
	
	// Constructor - Single record
	RqPtsInsertRow_T<REC>(const REC &_recData) 
	: RqPtsInsertRow(REC::TableName(),&_recData,sizeof(REC)) {}
	
	U32 GetRowCount() {
		return GetRowDataSize() / sizeof(rowID);
	}
	RowId *GetRowIdPtr(U32 *pnEntry=NULL) {
		return (RowId*) GetRowIdDataPtr(pnEntry,sizeof(RowId));
	}
	RowId *GetRowIdCopy(U32 *pnEntry=NULL) {
		return (RowId*) GetRowIdDataCopy(pnEntry,sizeof(RowId));
	}
};


// RqPtsModifyRow_T -- Modify Row in PTS Table -----------------------------------------------------
//
// Modify Row
//
template <class REC>
class RqPtsModifyRow_T : public RqPtsModifyRow {
public:
	// Constructor - generic
	RqPtsModifyRow_T<REC>(const char *_psKeyFieldName, 
				          const void *_pKeyFieldValue, U32 _cbKeyFieldValue,
				          const REC  &_recData )
  	: RqPtsModifyRow(REC::TableName(),_psKeyFieldName,
				     _pKeyFieldValue,_cbKeyFieldValue,
				     &_recData, sizeof(REC)) {
	}

	// Constructor with RowId KeyField
	RqPtsModifyRow_T<REC>(const RowId &rid, const REC &_recData)
  	: RqPtsModifyRow(REC::TableName(), rid, &_recData, sizeof(REC)) {
  	}

	// Constructor with another <REC> using RowId in that <REC>
	RqPtsModifyRow_T<REC>(const REC &_recData)
  	: RqPtsModifyRow(REC::TableName(), _recData.rid, &_recData, sizeof(REC)) {
  	}

	U32 GetRowCount() {
		return GetRowDataSize() / sizeof(RowId);
	}
	rowID *GetRowIdPtr(U32 *pnEntry=NULL) {
		return GetRowIdDataPtr(pnEntry,sizeof(RowId));
	}
	rowID *GetRowIdCopy(U32 *pnEntry=NULL) {
		return GetRowIdDataCopy(pnEntry,sizeof(RowId));
	}
};

// RqPtsModifyField_T -- Modify Field in PTS Table -------------------------------------------------
//
template <class REC>
class RqPtsModifyField_T : public RqPtsModifyField  {
public:
	// Constructor - generic
	RqPtsModifyField_T<REC>(const char *_psKeyFieldName,
					        const void *_pKeyFieldValue, U32 _cbKeyFieldValue,
					        const char *_psFieldName,    const void *_pbFieldValue, U32 _cbFieldValue,
							U32 _cRowsToModify=0)
	: RqPtsModifyField(REC::TableName(), _psKeyFieldName,
					 _pKeyFieldValue, _cbKeyFieldValue,
					 _psFieldName,
					 _pbFieldValue, _cbFieldValue,
					 _cRowsToModify ) {
	}

	// Constructor with RowId KeyField
	RqPtsModifyField_T<REC>(const RowId &rid,
					        const char *_psFieldName, const void *_pbFieldValue, U32 _cbFieldValue)
	: RqPtsModifyField(REC::TableName(),
					   rid,
				       _psFieldName, _pbFieldValue, _cbFieldValue) {
	}

	U32 GetRowCount() {
		return GetRowDataSize() / sizeof(RowId);
	}
	RowId *GetRowIdPtr(U32 *pnEntry=NULL) {
		return GetRowIdDataPtr(pnEntry,sizeof(RowId));
	}
	RowId *GetRowIdCopy(U32 *pnEntry=NULL) {
		return GetRowIdDataCopy(pnEntry,sizeof(RowId));
	}
};

#if 0	// You should be using RqPtsModifyField_T with RowId KeyField constructor

// RqPtsModifyFieldByRow_T -- Modify Field in PTS Table by RowId -----------------------------------
//
// Should this just be a different constructor in ModifyField_T????
//
template <class REC>
class RqPtsModifyFieldByRow_T : public RqPtsModifyFieldByRow  {
public:
	RqPtsModifyFieldByRow_T<REC>(const RowId &rid,
								 const char *_psFieldName, const void *_pbFieldValue, U32 _cbFieldValue)
	: RqPtsModifyField<REC>(CT_PTS_RID_FIELD_NAME, &rid, sizeof(rid),
					 _psFieldName, _pbFieldValue, _cbFieldValue) {}
};
#endif

// RqPtsModifyBits -- Modify Bits in Field ---------------------------------------------------------
//
// OpAndBits, OpOrBits, OpXorBits (defined in ptscommon.h)
//
template <class REC>
class RqPtsModifyBits_T : public RqPtsModifyBits {
public:
	// Constructor - generic
	RqPtsModifyBits_T<REC>(fieldOpType _opFlag,
					 const char *_psKeyFieldName, const void *_pKeyFieldValue, U32 _cbKeyFieldValue,
					 const char *_psFieldName,    const void *_pbFieldMask,
					 U32   _cRowsToModify=0 )
	: RqPtsModifyBits(REC::TableName(), _opFlag,
					  _psKeyFieldName,_pKeyFieldValue,_cbKeyFieldValue,
					  _psFieldName, _pbFieldMask, 
					  cRowsToModify) {}
	
	// Constructor with RowId KeyField
	RqPtsModifyBits_T<REC>(fieldOpType _opFlag,
						   const RowId &rid,
						   const char *_psFieldName, void *_pbFieldMask, U32 _cbFieldMask)
	: RqPtsModifyBits(REC::TableName(), _opFlag,
					  rid,
					  _psFieldName, _pbFieldMask, _cbFieldMask) {
	}
	
	U32 GetRowCount() {
		return GetRowDataSize() / sizeof(RowId);
	}
	RowId *GetRowIdPtr(U32 *pnEntry=NULL) {
		return GetRowIdDataPtr(pnEntry,sizeof(RowId));
	}
	RowId *GetRowIdCopy(U32 *pnEntry=NULL) {
		return GetRowIdDataCopy(pnEntry,sizeof(RowId));
	}
};

// RqPtsTestAndSetField_T -- Test and Set field ----------------------------------------------------
//
template <class REC>
class RqPtsTestAndSetField_T : public RqPtsTestAndSetField {
	// Constructor - generic
	RqPtsTestAndSetField_T<REC>(fieldOpType _opFlag,
						 const char *_psKeyFieldName,
						 const void *_psKeyFieldValue, U32 _cbKeyFieldValue,
						 const char *_psFieldName)
	: RqPtsTestAndSetField(_opFlag,
						   _psKeyFieldName,_psKeyFieldValue,_cbKeyFieldValue,
						   _psFieldName) {}

	// Constructor with RowId KeyField
	RqPtsTestAndSetField_T<REC>(fieldOpType _opFlag,
						 const RowId &rid,
						 const char *_psFieldName)
	: RqPtsTestAndSetField(_opFlag,
						   rid,
						   _psFieldName) {}
};

// RqPtsReadRow_T -- Read Row in PTS Table ---------------------------------------------------------
//
template <class REC>
class RqPtsReadRow_T : public RqPtsReadRow  {
public:
	// Constructor - generic
	RqPtsReadRow_T<REC>(const char *_psKeyFieldName, const void *_pKeyFieldValue, U32 _cbKeyFieldValue)
	: RqPtsReadRow(REC::TableName(), 
				   _psKeyFieldName, _pKeyFieldValue, _cbKeyFieldValue) {
	};

	// Constructor with RowId KeyField
	RqPtsReadRow_T<REC>(const RowId &rid)
	: RqPtsReadRow(REC::TableName(), rid ) {
	}
	
	U32 GetRowCount() {
		return GetRowDataSize() / sizeof(REC);
	}
	REC *GetRowPtr(U32 *pnEntry=NULL) {
		return (REC*) GetRowDataPtr(pnEntry,sizeof(REC));
	}
	REC *GetRowCopy(U32 *pnEntry=NULL) {
		return (REC*) GetRowDataCopy(pnEntry,sizeof(REC));
	}
};

// RqPtsDeleteRow_T -- Delete Row ------------------------------------------------------------------
//
template <class REC>
class RqPtsDeleteRow_T : public RqPtsDeleteRow {
public:
	// Constructor - generic
	RqPtsDeleteRow_T<REC>(const char *_psKeyFieldName, const void *_pKeyFieldValue,U32 _cbKeyFieldValue)
	: RqPtsDeleteRow(REC::TableName(),_psKeyFieldName,_pKeyFieldValue,_cbKeyFieldValue) {}
	
	// Constructor with RowId KeyField
	RqPtsDeleteRow_T<REC>(const RowId &rid)
	: RqPtsDeleteRow(REC::TableName(), rid ) {}
};

// RqPtsListen_T -- Listen to PTS Table ------------------------------------------------------------
//
template <class REC>
class RqPtsListen_T : public RqPtsListen  {
public:
	enum { LISTEN = ListenOnInsertRow | ListenOnDeleteAnyRow | ListenOnModifyAnyRowAnyField };
	enum { REPLY  = ReplyContinuous | ReplyFirstWithTable | ReplyWithRow };
	
	RqPtsListen_T<REC>(U32 _opListen=LISTEN,U32 _opReply=REPLY) 
	: RqPtsListen(_opListen,_opReply, REC::TableName() ) {}
	
	// Listen on AnyRowOneField
	RqPtsListen_T<REC>(U32 _opListen,U32 _opReply,
					   const char *_pszFieldName, const void *_pFieldValue=NULL, U32 _cbFieldValue=0)
	: RqPtsListen(_opListen,_opReply, REC::TableName(),NULL,NULL,0,_pszFieldName,_pFieldValue,_cbFieldValue ) {
	}
	
	// Listen on OneRow
	RqPtsListen_T<REC>(U32 _opListen,U32 _opReply,
					   const char *_pszRowFieldName, const void *_pRowFieldValue,   U32 _cbRowFieldValue,
					   const char *_pszFieldName,    const void *_pFieldValue=NULL, U32 _cbFieldValue=0)
	: RqPtsListen(_opListen,_opReply, REC::TableName(),_pszRowFieldName,_pRowFieldValue,_cbRowFieldValue,_pszFieldName,_pFieldValue,_cbFieldValue ) {
	}
	
	// Listen on OneRow - RowId
	RqPtsListen_T<REC>(U32 _opListen,U32 _opReply,
					   const RowId &rid,
					   const char *_pszFieldName=NULL, const void *_pFieldValue=NULL, U32 _cbFieldValue=0)
	: RqPtsListen(_opListen,_opReply, REC::TableName(),rid,_pszFieldName,_pFieldValue,_cbFieldValue ) {
	}

	U32 GetTableCount() {
		return GetTableDataSize() / sizeof(REC);
	}
	REC *GetTablePtr(U32 *pnEntry=NULL) {
		return (REC*) GetTableDataPtr(pnEntry,sizeof(REC));
	}
	REC *GetTableCopy(U32 *pnEntry=NULL) {
		return (REC*) GetTableDataCopy(pnEntry,sizeof(REC));
	}
	
	U32 *GetModifiedCount() {
		return GetModifiedDataSize() / sizeof(REC);
	}
	REC *GetModifiedPtr(U32 *pnEntry=NULL) {
		return (REC*) GetModifiedDataPtr(pnEntry,sizeof(REC));
	}
	REC *GetModifiedCopy(U32 *pnEntry=NULL) {
		return (REC*) GetModifiedDataCopy(pnEntry,sizeof(REC));
	}

	// Gets Table or Modified Rows depending on IsInitialReply()
	U32 *GetRowCount() {
		return GetRowDataSize() / sizeof(REC);
	}
	REC *GetRowPtr(U32 *pnEntry=NULL) {
		return (REC*) GetRowDataPtr(pnEntry,sizeof(REC));
	}
	REC *GetRowCopy(U32 *pnEntry=NULL) {
		return (REC*) GetRowDataCopy(pnEntry,sizeof(REC));
	}
};

#endif // __RqPts_T_h

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Include/PTS/RqPts_T.h $
// 
// 27    1/28/00 1:07p Jlane
// Fixed a cast error in InsertRow_T
// 
// 26    1/26/00 2:31p Joehler
// GetTableCount returns a U32, not a U32*.
// 
// 25    12/09/99 1:40a Iowa
// 
// 20    9/16/99 4:04p Tnelson
// 
// 19    8/31/99 10:25p Tnelson
// Made everything const safe
// 
// 18    8/31/99 3:32p Tnelson
// 
// 17    8/31/99 2:36p Tnelson
// 
// 16    8/31/99 2:09p Tnelson
// 
// 15    8/31/99 1:27p Tnelson
// Fixes for Mr. Wedel
// 
// 14    8/27/99 8:34p Tnelson
// Changed GetRowPtr(U32 iRow) to GetRowPtr(U32 *pnRows)
// 
// 13    8/27/99 7:45p Tnelson
// 
// 12    8/27/99 7:42p Tnelson
// 
// 11    8/27/99 7:33p Tnelson
// All  requests now based on on RqPtsMessage
// 
// 10    8/27/99 6:23p Tnelson
// 
// 9     8/27/99 6:19p Tnelson
// 
// 8     8/27/99 6:15p Tnelson
// New GetRowPtr/GetRowDataPtr
// 
// 7     8/27/99 5:33p Tnelson
// Default reserved entries on DefineTable may not be zero!  This argument
// is really allocation size.
// 
// 6     8/27/99 3:41p Tnelson
// More "const" fixes
// 
// 5     8/27/99 12:04p Tnelson
// Added const to fieldDef parms
// 
// 4     8/26/99 3:23p Tnelson
// 
// 3     8/26/99 3:20p Tnelson
// Fixes.  Template ALL base PTS requests
// 
// 2     8/26/99 1:48p Tnelson
// 
// 1     8/22/99 6:03p Tnelson
// Created. Not Complete
