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
// This is the IOP Failover Map.  It determines which IOP slots in the 
// Odyssey will serve as redundant pairs.  
// 
// $Log: /Gemini/Include/CTTables/IOPFailoverMapTable.h $
// 
// 2     3/17/99 10:37a Jlane
// Updated with and for a corresdponding .cpp file that has the fielddefs.
//
// Update Log: 
// 02/26/99 JFL	Created.
/*************************************************************************/
#ifndef __IOPFailoverMapTable_h
#define __IOPFailoverMapTable_h

#if !defined(PtsCommon_H)
# include  "PtsCommon.h"
#endif

#if !defined(__Address_h)
# include  "Address.h"
#endif

#include "CtTypes.h"

#pragma	pack(4)


// The IOP Failover Map Table contains one row for each CMB addressable PCI sloot in the Odyssey.  The contents of each
// row is the CMB address of the slot and the CMB address of the slot's redundant counterpart slot.
//
typedef struct {
	rowID		rid;				// rowID of this record.
	U32 		version;			// Version of UserAccessTable.
	U32			size;				// Size of UserAccessTable in bytes.
	TySlot		PrimarySlotNum;		// Primary slot # (TySlot is binarily an int).		
	TySlot		FailoverSlotNum;	// Primary slot # (TySlot is binarily an int).		
} IOPFailoverMapRecord, IOPFailoverMapTable[];


//  compiler-checkable aliases for table / field names

//  table name
#define  CT_IOPFM_TABLE_NAME		"IOP_Failover_Map_Table"

//  field defs
#define  CT_IOPFM_VERSION			"version"				// Version of IOP_Status record.
#define  CT_IOPFM_SIZE				"size"					// # of bytes in record.
#define  CT_IOPFM_PRIMARYSLOTNUM	"PrimarySlotNum"		// Slot # I live in.  See DeviceId.h.
#define  CT_IOPFM_FAILOVERSLOTNUM	"FailoverSlotNum"		// Slot # of my redundant counterpart.


//  here is the standard table which defines IOP Status table fields
extern const fieldDef aIopFailoverMapTable_FieldDefs[];

//  and here is the size, in bytes, of the IOP Status field defs
extern const U32 cbIopFailoverMapTable_FieldDefs;

#endif // __IOPFailoverMapTable_h