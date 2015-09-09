/* ClassTableTable.h -- Table of ConfigData Tables
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
**/

// Revision History: 
//  8/19/99 Tom Nelson: Create file
// ** Log at end-of-file **

#ifndef __ClassTableTable_h
#define __ClassTableTable_h

#include "RqPts_T.h"
#include "PtsRecordBase.h"

#define PTS_CLASS_TABLE_TABLE			"ClassTableTable"
#define PTS_CLASS_TABLE_TABLE_VERSION 	1

class ClassTableRecord : public CPtsRecordBase {
private:
	static fieldDef rgFieldDefs[];
	static U32      cbFieldDefs;
public:
	static fieldDef *FieldDefs()		{ return rgFieldDefs;}
	static U32 const FieldDefsSize() 	{ return cbFieldDefs;  }
	static U32 const FieldDefsTotal()	{ return FieldDefsSize() / sizeof(fieldDef); }
	static char *TableName()			{ return PTS_CLASS_TABLE_TABLE; }
	
	// Record Content	
	String32		szClassName;		// Class name
	String64		szConfigTableName;	// Table Name of config records
	
	// Constructors
	ClassTableRecord() 
	: CPtsRecordBase(sizeof(ClassTableRecord),PTS_CLASS_TABLE_TABLE_VERSION) {
		szClassName[0] = szConfigTableName[0] = EOS;
	}
	ClassTableRecord(char *_pszClassName,char *_pszConfigTableName) 
	: CPtsRecordBase(sizeof(ClassTableRecord),PTS_CLASS_TABLE_TABLE_VERSION) {
		SetClassName(_pszClassName);
		SetConfigTableName(_pszConfigTableName);
	}
	// Methods
	// Check for duplicate class names in next "nRec" records starting with "this"
	BOOL IsDuplicateClass(U32 nRec,const char *pszClassName) {
		for (U32 ii=0; ii<nRec; ii++)
			if (strcmp(this[ii].szClassName,pszClassName) == 0)
				return TRUE;
		
		return FALSE;
	}
	void SetClassName(const char *_pszClassName) {
		strncpy(szClassName,(char*)_pszClassName,sCLASSNAME);
	   	szClassName[sCLASSNAME-1] = EOS;
	}
	void SetConfigTableName(const char *_pszConfigTableName) {
		strncpy(szConfigTableName,(char*)_pszConfigTableName,CBMAXTABLESPEC);
	   	szClassName[CBMAXTABLESPEC-1] = EOS;
	}
	// PTS Interface Classes
	typedef RqPtsDefineTable_T<ClassTableRecord> RqDefineTable;
	typedef RqPtsDeleteTable_T<ClassTableRecord> RqDeleteTable;
	typedef RqPtsQuerySetRID_T<ClassTableRecord> RqQuerySetRID;
	typedef RqPtsEnumerateTable_T<ClassTableRecord> RqEnumerateTable;
	typedef RqPtsInsertRow_T<ClassTableRecord> RqInsertRow;
	typedef RqPtsModifyRow_T<ClassTableRecord> RqModifyRow;
	typedef RqPtsModifyField_T<ClassTableRecord> RqModifyField;
	typedef RqPtsModifyBits_T<ClassTableRecord> RqModifyBits;
	typedef RqPtsTestAndSetField_T<ClassTableRecord> RqTestAndSetField;
	typedef RqPtsReadRow_T<ClassTableRecord> RqReadRow;
	typedef RqPtsDeleteRow_T<ClassTableRecord> RqDeleteRow;
	typedef RqPtsListen_T<ClassTableRecord> RqListen;
 };
 
 typedef ClassTableRecord ClassTableTable[];


// Field names....so people could use Field methods

#define	CTT_CLASS_NAME		"ClassName"
#define	CTT_TABLE_NAME		"TableName"


#endif // __ClassTableTable_h

//*************************************************************************
// Update Log:
//	$Log: /Gemini/Include/CTTables/ClassTableTable.h $
// 
// 7     10/14/99 3:48a Iowa
// Iowa merge
// 
// 6     9/17/99 9:48p Tnelson
// 
// 5     9/17/99 9:46p Tnelson
// 
// 4     9/16/99 4:01p Tnelson
// 
// 1     8/19/99 6:30p Tnelson


