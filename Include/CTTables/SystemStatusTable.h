/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// Description:
// This is the declaration of the System Status table.
// 
// $Log: /Gemini/Include/CTTables/SystemStatusTable.h $ 
// 
// 3     10/28/99 9:25a Sgavarre
// Added IOPsOnPCIMask field for split boot support.
// 
// 2     8/23/99 1:47p Jlane
// Got to compile.
// 
// 1     8/20/99 2:48p Jlane
// Initial Checkin.
// 
/*************************************************************************/

#ifndef _SystemStatusTable_h
#define _SystemStatusTable_h


#if !defined(PtsCommon_H)
# include  "PtsCommon.h"
#endif


#pragma  pack(4)

//  here is the standard table which defines EVC Status table PTS fields
extern const fieldDef aSystemStatusTable_FieldDefs[];

//  and here is the size, in bytes, of the EVC Status table field defs
extern const U32 cbSystemStatusTable_FieldDefs;


#define SYSTEM_STATUS_TABLE "System_Status_Table"

#define SYSTEM_STATUS_TABLE_VER  (1)      /* current struct version */

//
// System Status Table - Data detailing status of system characteristics
//                    such as IOPs present, active, etc.
// 
typedef struct  { 

	static const fieldDef *FieldDefs()		{ return aSystemStatusTable_FieldDefs; }
	static U32 const FieldDefsSize() 	{ return cbSystemStatusTable_FieldDefs; }
	static char *TableName()			{ return SYSTEM_STATUS_TABLE; }

	rowID    rid;                    // rowID of this record.
	U32      version;                // == SYSTEM_STATUS_TABLE_VER
	U32      size;                   // # of bytes in record.

	// masks of IOPs present and active.
	U32      PresentIOPsMask;       // Mask of Active IOP Slots.  Bits indexed
                                    //  by IOP addresses in TySlot type.
	U32      IOPsOnPCIMask;         // Mask of IOP Slots connected to PCI.  Bits indexed
                                    //  by IOP addresses in TySlot type.
	U32      ActiveIOPsMask;        // Mask of Active IOP Slots.  Bits indexed
                                    //  by IOP addresses in TySlot type.
} SystemStatusRecord;


//  here are compiler-checkable aliases for PTS field definition names

#define  CT_SYSST_REC_VERSION    "version"      // == EVC_STATUS_TABLE_VER
#define  CT_SYSST_SIZE           "size"            // # of bytes in record.

   //  mask of IOPs present and active.
#define  CT_SYSST_PRESENTIOPSMASK         "PresentIOPsMask"
#define  CT_SYSST_IOPSONPCIMASK           "IOPsOnPCIMask"
#define  CT_SYSST_ACTIVEIOPSMASK          "ActiveIOPsMask"


#endif  // #ifndef _SystemStatusTable_h

