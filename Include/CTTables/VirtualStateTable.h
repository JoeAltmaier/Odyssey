/* VirtualStateTable.h -- Table of ConfigData Tables
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

#ifndef __VirtualStateTable_h
#define __VirtualStateTable_h

#include "RqPts_T.h"
#include "RqOsVirtualMaster.h"

#define PTS_VIRTUAL_STATE_TABLE				"VirtualStateTable"
#define PTS_VIRTUAL_STATE_TABLE_VERSION 	1

#if 0		// In RqOsVirtualMaster.h
enum VirtualIopState {
	vstIOPNONE, 
	vstIOPDOWN, 
	vstIOPBOOT, 
	vstIOPFAIL, 
	vstIOPUP 
};
#endif

class VirtualStateRecord {
private:
	static fieldDef rgFieldDefs[];
	static U32      cbFieldDefs;
public:
	static fieldDef *FieldDefs()		{ return rgFieldDefs;}
	static U32 const FieldDefsSize() 	{ return cbFieldDefs;  }
	static U32 const FieldDefsTotal()	{ return FieldDefsSize() / FieldDefsSize(); }
	static char *TableName()			{ return PTS_VIRTUAL_STATE_TABLE; }
	
	// Header
	const rowID		rid;				// rowID of this record.
	const U32		size;				// Size of VirtualIopState in bytes.
	const U32		version;
	// Record
	TySlot			slot;				// Slot Number
	VirtualIopState	state;				// VirtualState of this slot


	VirtualStateRecord() : size(sizeof(VirtualStateRecord)), version(PTS_VIRTUAL_STATE_TABLE_VERSION),rid() {}
		
	// PTS Interface Classes
	typedef RqPtsDefineTable_T<VirtualStateRecord> RqDefineTable;
	typedef RqPtsDeleteTable_T<VirtualStateRecord> RqDeleteTable;
	typedef RqPtsQuerySetRID_T<VirtualStateRecord> RqQuerySetRID;
	typedef RqPtsEnumerateTable_T<VirtualStateRecord> RqEnumerateTable;
	typedef RqPtsInsertRow_T<VirtualStateRecord> RqInsertRow;
	typedef RqPtsModifyRow_T<VirtualStateRecord> RqModifyRow;
	typedef RqPtsModifyField_T<VirtualStateRecord> RqModifyField;
	typedef RqPtsModifyBits_T<VirtualStateRecord> RqModifyBits;
	typedef RqPtsTestAndSetField_T<VirtualStateRecord> RqTestAndSetField;
	typedef RqPtsReadRow_T<VirtualStateRecord> RqReadRow;
	typedef RqPtsDeleteRow_T<VirtualStateRecord> RqDeleteRow;
	typedef RqPtsListen_T<VirtualStateRecord> RqListen;
};

typedef VirtualStateRecord VirtualStateTable[];

// Field names....so people could use Field methods

#define VST_SLOT		"IopSlot"
#define	VST_STATE		"IopState"

#endif // __VirtualStateTable_h

//*************************************************************************
// Update Log:
//	$Log: /Gemini/Include/CTTables/VirtualStateTable.h $
// 
// 6     9/16/99 4:00p Tnelson
// 
// 5     9/03/99 3:12p Tnelson
// 
// 4     8/27/99 10:47a Tnelson
// 
// 3     8/27/99 10:44a Tnelson
// 
// 2     8/26/99 3:47p Tnelson
// Latest and Greatest!
// 
// 1     8/25/99 5:19p Tnelson

