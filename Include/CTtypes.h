/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CtTypes.h
// 
// Description:
// This module defines universal types, macros and constants.
// 
// $Log: /Gemini/Include/CTtypes.h $
// 
// 33    9/04/99 5:19p Agusev
// Added a missing constructor for RowId
// 
// 32    9/04/99 4:37p Agusev
// Got rid of a warning message
// 
// 31    9/04/99 4:33p Agusev
// Fixed the WIN32 build
// 
// 30    9/03/99 4:37p Ewedel
// Added class RowId, a more convenient alternative to the rowID type.
// [Written by TRN.]
// 
// 29    8/23/99 2:16p Agusev
// Got rid of a warning message
// 
// 28    8/23/99 11:12a Tnelson
// Removed constuctors to try and fix build problems.
// 
// 27    8/21/99 8:46p Tnelson
// Add constructors to rowID
// 
// 26    8/08/99 4:12p Jaltmaier
// operator == only for C++
// 
// 25    8/07/99 1:22p Jlane
// Added operator== for rowID.
// 
// 24    5/13/99 2:54p Agusev
// Added UnicodeString type defs
// 
// 23    3/26/99 5:14p Ewedel
// Removed literal reference to vss Log: keyword in comment -- was causing
// doubled keyword expansion.
// 
// 22    3/23/99 8:05a Sgavarre
// Add table element to RowID type
// 
// 21    3/03/99 5:32p Jlane
// Added String16 and Vss Log: keyword to get automatic change headers.
//
// 4/14/98 Jim Frandeen: Create file
// 9/29/98 Jim Frandeen: Merge stddef.h because it conflicts with Windows.
// 9/29/98 Michael G. Panas: Move C++ things out of the extern "C" area.
// 10/08/98 Jim Frandeen: include Nucleus.h.
//		Move I64 to work for C or C++.
// 10/26/98 Tom Nelson: Moved several defs to OsTypes.h
// 11/03/98	JFL	Added rowID typedef.
// 1/13/99  Jim Frandeen: Remove #include "Trace.h" so that other
//			versions of trace.h can be used.
// 02/12/99 JFL	Included OSStatus.h and excluded i20 stuff and xFunctionCode.h
// 02/12/99 JWF Add end line to end of file (for GreenHills compiler)
/*************************************************************************/

#if !defined(CTtypes_H)
#define CTtypes_H

#include "OsTypes.h"	/* Nucleus.h/I2oTypes.h/Simple.h/OsTypes.h */
#include "OsStatus.h"

#if 0
#include "i2omsg.h"
#include "i2oddmlib.h"
#include "XFunctionCode.h"
#endif

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#define DEBUG
//#undef TRACE_ENABLE

typedef char	String16[16],
				String32[32],
				String64[64],
				String128[128],
				String256[256],
				String512[512],
				String1024[1024],
				String2048[2048];

typedef char	UnicodeString16[32],
				UnicodeString32[64],
				UnicodeString64[128],
				UnicodeString128[256],
				UnicodeString256[512],
				UnicodeString512[1024],
				UnicodeString1024[2048],
				UnicodeString2048[4096];

#define NIOP 18

// Declare the unique row identifier type used by the Table Service.
typedef struct rowID {
	U16	Table;
	U16	HiPart;
	U32	LoPart;
	
#ifdef   __cplusplus
	int operator== (const rowID& r1) { 
		return ((this->Table == r1.Table) &&
				(this->HiPart == r1.HiPart) && 
				(this->LoPart == r1.LoPart)) 
				? 1 : 0;};	
#endif

} rowID;


#ifdef __cplusplus

// RowId - rowID as a class with constructors/methods
//
// Cannot put constructors in "struct rowID" because some current code
// has "rowId = {0,0,0}"  and constructors (at least with Metrowerks)
// prevent initializers from working. (TRN)

typedef I64 RID;

class RowId : public rowID {
public:	
// Allow RowId = rowID
	RowId(rowID& r1) 	 { *(rowID *) this = r1; } 
	RowId( U16 t, U16 h, U32 l ) { Table = t; HiPart = h; LoPart = l; }
	RowId& operator= (const rowID& r1) { 
		*(rowID *) this = r1;
		return *this;
	}
	// cast to rowID
	operator rowID() {
		return *(rowID *)this;
	}
	// cast to RID - so I can say Tracef("%u\n",(RID) rowId);
	operator RID() {
		return (((I64) Table) << 48) | (((I64) HiPart) << 32) | (I64) LoPart;
	}
	// Assign RowId = RID - so I can say rowId = 0
	RowId& operator=(const RID rid) {
		Table = (U16) (rid >> 48);
		HiPart = (U16) (rid >> 32);
		LoPart = (U32) rid;
		return *this;
	}
	RowId(U32 _loPart=0) { Table=0; HiPart = 0; LoPart = _loPart; }	// default
	RowId(U16 _table,U32 _loPart) { Table = _table; HiPart = 0; LoPart = _loPart; }
				
	void Set(U16 _table,U32 _loPart) { Table = _table; LoPart = _loPart; HiPart = 0; }
	
	void SetTable(U16 _table)	 { Table = _table;   HiPart=0; }
	void SetRow(U32 _loPart)	 { LoPart = _loPart; HiPart=0; }

	U16 GetTable()		{ return Table; }
	U32 GetRow()		{ return LoPart;}
	
	void Clear()		{ Table=0; HiPart=0; LoPart=0; }
	BOOL IsClear()		{ return Table==0 && HiPart==0 && LoPart==0; }

   //  here's a little helper, for compatibility with Andrey's existing
   //  SSAPI RowId class (returns a copy
   rowID GetRowID() const { 
      rowID temp = *this;

      return temp;
   } 

	// Don't put any Tracef in here

   //  (except, we need it for Andrey's existing code - ewx 9/2/99)
   //  If you use this, you must include util.mcp in your project.
   void PrintYourself(void) const;

};  /* end of class RowId */


// Allow all combinations of RowId == rowID
inline int operator== (const rowID& rid1, const rowID& rid2) {
	return ((rowID &)rid1).operator==(rid2);
};
// Allow all combinations of RowId != rowID
inline int operator!= (const rowID& rid1, const rowID& rid2) {
	
	return !(rid1 == rid2);
};

#endif  // #ifdef __cplusplus


/*	ConvergeNet's Organization_ID field */
#define CT_ORGANIZATION_ID 0x0118

/*	Module IDs  */
#define CT_MODULE_ID_NIC 0x01

/* IOP ID -- Every IOP has a unique ID */
typedef U8 CT_IOP_ID;		/* Should use TySlot */

/*  driver ID -- Every driver in the Odyssey system as a unique driver ID. */  
typedef U8 CT_DRIVER_ID;	/* Should use DID */

/*  Logical Unit Number */  
typedef U16 CT_LUN;

/*	Define a Message Frame Address */
typedef U32 CT_MFA;

/* Handle definitions */
typedef void* CT_LUN_MAP_HANDLE;

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif // CTtypes_H
