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
// This is the Loop Descriptor Table used to manage FC Loops
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/LoopDescriptor.h $
// 
// 6     8/06/99 9:31a Vnguyen
//
// 6     8/06/99  9:19a Vnguyen
// Added row IDs for Status and Performance records.
// 
// 5     7/17/99 3:21p Mpanas
// Add flags defines
// 
// 4     7/14/99 3:24p Jlane
// Added LoopDescriptorRecord declaration and added typedef before struct.
// 
// 3     7/13/99 9:42p Mpanas
// Change name of table to LoopDescriptorEntry
// add fields to specify which target IDs are used
// 
// 2     5/28/99 4:51p Mpanas
// Compiles now
// 
// 1     5/28/99 12:49p Mpanas
// First pass at the Loop Descriptor Table
// Not compiled...
// 
// 05/27/99 Michael G. Panas: Create file
/*************************************************************************/

#ifndef __LoopDescriptor_h
#define __LoopDescriptor_h

#include "CtTypes.h"
#include "Odyssey.h"
#include "PTSCommon.h"
#include "IopTypes.h"

// Field definitions in LoopDescriptor.cpp
extern	fieldDef	Loop_Descriptor_Table_FieldDefs[];
extern	U32			cbLoop_Descriptor_Table_FieldDefs;

#pragma	pack(4)

#define 		fdLD_VERSION				"version"
#define 		fdLD_SIZE					"size"
#define 		fdLD_LOOP_NUM				"LoopNumber"
#define 		fdLD_SLOT					"slot"
#define 		fdLD_CHIP_NUM				"ChipNumber"
#define 		fdLD_DM_VDN					"vdnDriveMonitor"
#define 		fdLD_LM_VDN					"vdnLoopMonitor"
#define 		fdLD_FLAGS					"flags"
#define 		fdLD_BOARD_TYPE				"btBoardType"
#define 		fdLD_CHIP_TYPE				"ChipType"
#define 		fdLD_DESIRED_LOOP_STATE		"DesiredLoopState"
#define 		fdLD_ACTUAL_LOOP_STATE		"ActualLoopState"
#define 		fdLD_IDS_INUSE				"IDsInUse"
#define 		fdLD_TARGET_IDS				"TargetIDs"
#define 		fdLD_STATUS_RID				"ridStatusRecord"
#define 		fdLD_PERFORMANCE_RID		"ridPerformanceRecord"

#define LOOP_DESCRIPTOR_TABLE "LoopDescriptorTable"
#define	LOOP_DESCRIPTOR_TABLE_VERSION	1


typedef struct LoopDescriptorEntry 				// one entry for each FC Port.
{
	rowID  			rid;   				// rowID of this record.
	U32   			version;   			// Version of Loop_Descriptor_Record.
	U32  			size;   			// Size of Loop_Descriptor_Record in bytes.
	U32  			LoopNumber;  		// The FC Loop instance.
	TySlot  		slot;  				// The actual slot on which the loop exists.
	U32  			ChipNumber;  		// The chip number that handles the loop.
	VDN  			vdnDriveMonitor; 	// 0 => no drive monitor.
	VDN  			vdnLoopMonitor;  	// We'll always have one of these per board.
	U32  			flags;  			// Target, initiator, IP-able etc.
	IopType 		btBoardType;  		// NIC, RAC, NAC etc.
	U32  			ChipType;  			// Qlogic 2100, 2200, etc.
	LoopState 		DesiredLoopState; 	// What state should we be in.
	LoopState 		ActualLoopState;  	// What state are we in now.
	U32				IDsInUse;			// number of Target IDs in use
	U8				TargetIDs[32];		// array of used target IDs (if target)
	rowID			ridStatusRec;		// rowID of Status Reporter for this record
	rowID			ridPerformanceRec;	// rowID of Performance Reporter for this record
} LoopDescriptorRecord;

// define the bit values for the flags field
#define		LD_FLAGS_INITIATOR		1
#define		LD_FLAGS_TARGET			2
#define		LD_FLAGS_IP				4

#endif