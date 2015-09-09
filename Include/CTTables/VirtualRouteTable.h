/* VirtualRouteTable.h -- PTS Virtual Route Table
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
//  8/23/99 Tom Nelson: Create file
// ** Log at end-of-file **

#ifndef __VirtualRouteTable_h
#define __VirtualRouteTable_h

#include "RqPts_T.h"
#include "PtsRecordBase.h"

#define PTS_VIRTUAL_ROUTE_TABLE				"VirtualRouteTable"
#define PTS_VIRTUAL_ROUTE_TABLE_VERSION 	1


class VirtualRouteRecord : public CPtsRecordBase {
private:
	static fieldDef rgFieldDefs[];
	static U32      cbFieldDefs;

public:
	enum { MAXSERVES = 64 };
		
	static fieldDef *FieldDefs()		{ return rgFieldDefs;}
	static U32 const FieldDefsSize() 	{ return cbFieldDefs;  }
	static U32 const FieldDefsTotal()	{ return FieldDefsSize() / sizeof(fieldDef); }
	static char *TableName()			{ return PTS_VIRTUAL_ROUTE_TABLE; }
	
	// Record
	VDN				vdn;				// Device Serving Requests
	U32				nServes;			// Number of serves
	U32				maxServes;			// == MAXSERVES
	REQUESTCODE		aReqCodes[MAXSERVES];// Request Code served

	// Constructors
	VirtualRouteRecord() 
	: CPtsRecordBase(sizeof(VirtualRouteRecord),PTS_VIRTUAL_ROUTE_TABLE_VERSION),
	 vdn(VDNNULL),maxServes(MAXSERVES),nServes(0) {}
	
	VirtualRouteRecord(VDN _vdn,U32 _nServes,const REQUESTCODE *_paReqCodes)
	: CPtsRecordBase(sizeof(VirtualRouteRecord),PTS_VIRTUAL_ROUTE_TABLE_VERSION),
	  vdn(_vdn),maxServes(MAXSERVES),nServes(_nServes) {
	  
	  memcpy(aReqCodes,_paReqCodes,_nServes * sizeof(REQUESTCODE));
	}
		
	// PTS Interface Classes
	typedef RqPtsDefineTable_T<VirtualRouteRecord> RqDefineTable;
	typedef RqPtsDeleteTable_T<VirtualRouteRecord> RqDeleteTable;
	typedef RqPtsQuerySetRID_T<VirtualRouteRecord> RqQuerySetRID;
	typedef RqPtsEnumerateTable_T<VirtualRouteRecord> RqEnumerateTable;
	typedef RqPtsInsertRow_T<VirtualRouteRecord> RqInsertRow;
	typedef RqPtsModifyRow_T<VirtualRouteRecord> RqModifyRow;
	typedef RqPtsModifyField_T<VirtualRouteRecord> RqModifyField;
	typedef RqPtsModifyBits_T<VirtualRouteRecord> RqModifyBits;
	typedef RqPtsTestAndSetField_T<VirtualRouteRecord> RqTestAndSetField;
	typedef RqPtsReadRow_T<VirtualRouteRecord> RqReadRow;
	typedef RqPtsDeleteRow_T<VirtualRouteRecord> RqDeleteRow;
	typedef RqPtsListen_T<VirtualRouteRecord> RqListen;
};

typedef VirtualRouteRecord VirtualRouteTable[];

// Field names....so people could use Field methods

#define VRT_RQCODE		"RqCode"
#define VRT_NSERVES		"nServes"
#define VRT_MAXSERVES 	"maxServes"
#define	VRT_VDN			"Vdn"

#endif // __VirtualRouteTable_h

//*************************************************************************
// Update Log:
//	$Log: /Gemini/Include/CTTables/VirtualRouteTable.h $
// 
// 6     2/09/00 2:46p Tnelson
// Added const to argument
// 
// 5     10/14/99 3:49a Iowa
// Iowa merge
// 
// 4     9/16/99 4:00p Tnelson
// 
// 3     9/03/99 3:12p Tnelson
// 
// 2     8/26/99 3:46p Tnelson
// Latest and Greatest!
// 
// 1     8/25/99 5:34p Tnelson
// Created

