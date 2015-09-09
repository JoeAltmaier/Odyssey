/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ArrayPerformanceTable.h
//
// Description:
//
// Raid Array Performance PTS Structure used with the Performance / Health / Status
// system
//
// $Log: /Gemini/Include/CTTables/ArrayPerformanceTable.h $
// 
// 5     11/02/99 8:56a Vnguyen
// Add read and write performance counters.  Also scale all counters to
// per second.
// 
// 4     9/15/99 2:31p Vnguyen
// Update performance and status counters.  Remove variance info for the
// performance counters becuase nobody is using it.
// 
// 3     8/16/99 12:53p Vnguyen
// Convert to use new object: SRCandExport class.  Fix miss-match
// parameters between calls.
// 
// 2     7/12/99 10:13a Jlane
// Miscellaneous fixes.
// 
// 1     6/08/99 7:33p Jlane
// Initial creation and check-in.
//
// Update Log: 
//	5/20/99	Jerry Lane:	initial creation
//
/*************************************************************************/

#ifndef __ArrayPerformanceTable_h
#define __ArrayPerformanceTable_h


#include "CTTypes.h"
#include "RaidDefs.h"
#include "PtsCommon.h"

// Field definitions in ArrayPerformanceTable.cpp
extern	const fieldDef	aArrayPerformanceTable_FieldDefs[];
extern	const U32		cbArrayPerformanceTable_FieldDefs;

#define CT_ARRAYPT_TABLE 			"Array_Performance_Table"
#define CT_ARRAYPT_TABLE_VERSION	1
#define ARRAY_PERFORMANCE_TABLE		CT_ARRAYPT_TABLE

#define	NUM_MEASURE_SIZES	12		// Make sure to change the init routine in ArrayPerformanceReporter.cpp too

// Count reads and writes by the number of blocks requested
// [0]   = 512
// [1]   = 1k
// [2]   > 1k   <= 2k
// [3]   > 2k   <= 4k
// [4]   > 4k   <= 8k
// [5]   > 8k   <= 16k
// [6]   > 16k  <= 32k
// [7]   > 32k  <= 64k
// [8]   > 64k  <= 128k
// [9]   > 128k <= 256k
// [10]  > 256k <= 512k
// [11]  > 512k

#define fdARRAYPERFORMANCE_RID							"rid"								// rowID of this record.
#define fdARRAYPERFORMANCE_VERSION						"version"							// Version of DISK_EVENT_DATA record
#define fdARRAYPERFORMANCE_SIZE							"size"								// Size of DISK_EVENT_DATA record in bytes.
#define fdARRAYPERFORMANCE_KEY							"key"								// ala Rick Currently unused, could hold Circuit Key.
#define fdARRAYPERFORMANCE_REFRESHRATE					"RefreshRate"						// 0 => refresh once >0 implies refresh every x seconds?    
#define fdARRAYPERFORMANCE_SAMPLERATE					"SampleRate"						// rate at which PHSReporters sample DDMs data.
#define fdARRAYPERFORMANCE_RIDSRCRECORD					"ridSRCRecord"						// Row ID of  Storage Roll Call  entry 4 this device.
#define fdARRAYPERFORMANCE_NUMRDSAVERAGEPERSEC			"NumRdsAveragePerSec"
#define fdARRAYPERFORMANCE_NUMRDSMINIMUMPERSEC			"NumRdsMinimumPerSec"
#define fdARRAYPERFORMANCE_NUMRDSMAXIMUMPERSEC			"NumRdsMaximumPerSec"
#define fdARRAYPERFORMANCE_NUMWRTSAVERAGEPERSEC			"NumWrtsAveragePerSec"
#define fdARRAYPERFORMANCE_NUMWRTSMINIMUMPERSEC			"NumWrtsMinimumPerSec"
#define fdARRAYPERFORMANCE_NUMWRTSMAXIMUMPERSEC			"NumWrtsMaximumPerSec"
#define fdARRAYPERFORMANCE_NUMREADSAVERAGEPERSEC		"NumReadsAveragePerSec"
#define fdARRAYPERFORMANCE_NUMREADSMINIMUMPERSEC		"NumReadsMinimumPerSec"
#define fdARRAYPERFORMANCE_NUMREADSMAXIMUMPERSEC		"NumReadsMaximumPerSec"
#define fdARRAYPERFORMANCE_NUMWRITESAVERAGEPERSEC		"NumWritesAveragePerSec"
#define fdARRAYPERFORMANCE_NUMWRITESMINIMUMPERSEC		"NumWritesMinimumPerSec"
#define fdARRAYPERFORMANCE_NUMWRITESMAXIMUMPERSEC		"NumWritesMaximumPerSec"
#define fdARRAYPERFORMANCE_NUMBLOCKREADSAVERAGEPERSEC	"NumBlocksReadAveragePerSec"
#define fdARRAYPERFORMANCE_NUMBLOCKREADSMINIMUMPERSEC	"NumBlocksReadMinimumPerSec"
#define fdARRAYPERFORMANCE_NUMBLOCKREADSMAXIMUMPERSEC	"NumBlocksReadMaximumPerSec"
#define fdARRAYPERFORMANCE_NUMBLOCKWRITESAVERAGEPERSEC	"NumBlocksWrittenAveragePerSec"
#define fdARRAYPERFORMANCE_NUMBLOCKWRITESMINIMUMPERSEC	"NumBlocksWrittenMinimumPerSec"
#define fdARRAYPERFORMANCE_NUMBLOCKWRITESMAXIMUMPERSEC	"NumBlocksWrittenMaximumPerSec"
#define fdARRAYPERFORMANCE_NUMSGCOMBINEDREADSAVERAGEPERSEC	"NumSGCombinedReadsAveragePerSec"
#define fdARRAYPERFORMANCE_NUMSGCOMBINEDREADSMINIMUMPERSEC	"NumSGCombinedReadsMinimumPerSec"
#define fdARRAYPERFORMANCE_NUMSGCOMBINEDREADSMAXIMUMPERSEC	"NumSGCombinedReadsMaximumPerSec"
#define fdARRAYPERFORMANCE_NUMSGCOMBINEDWRITESAVERAGEPERSEC	"NumSGCombinedWritesAveragePerSec"
#define fdARRAYPERFORMANCE_NUMSGCOMBINEDWRITESMINIMUMPERSEC	"NumSGCombinedWritesMinimumPerSec"
#define fdARRAYPERFORMANCE_NUMSGCOMBINEDWRITESMAXIMUMPERSEC	"NumSGCombinedWritesMaximumPerSec"
#define fdARRAYPERFORMANCE_NUMOVERWRITESAVERAGEPERSEC	"NumOverwritesAveragePerSec"
#define fdARRAYPERFORMANCE_NUMOVERWRITESMINIMUMPERSEC	"NumOverwritesMinimumPerSec"
#define fdARRAYPERFORMANCE_NUMOVERWRITESMAXIMUMPERSEC	"NumOverwritesMaximumPerSec"

typedef struct
{
	rowID	rid;									// rowID of this record.
	U32 	version;								// Version
	U32		size;									// Size of record in bytes.
	U32		key;									// For listen operation
	U32		RefreshRate;							// in microseconds, must be a multiple of SampleRate
	U32		SampleRate;								// rate at which PHSReporters sample DDMs data.  In microseconds.
													// Must be a multiple of seconds.
	rowID	ridSRCRecord;							// Row ID of  Storage Roll Call  entry 4 this device.

	U32		NumRdsAveragePerSec[NUM_MEASURE_SIZES];	// This counter is for development.  Do not display to users
	U32		NumRdsMinimumPerSec[NUM_MEASURE_SIZES];
	U32		NumRdsMaximumPerSec[NUM_MEASURE_SIZES];
	
	U32		NumWrtsAveragePerSec[NUM_MEASURE_SIZES]; // This counter is for development.  Do not display to users
	U32		NumWrtsMinimumPerSec[NUM_MEASURE_SIZES];
	U32		NumWrtsMaximumPerSec[NUM_MEASURE_SIZES];
	
	U32		NumReadsAveragePerSec;		// Number of reads.  This is the sum of the above read counters
	U32		NumReadsMinimumPerSec;
	U32		NumReadsMaximumPerSec;

	U32		NumWritesAveragePerSec;		// Number of writes.  This is the sum of the above read counters
	U32		NumWritesMinimumPerSec;
	U32		NumWritesMaximumPerSec;

	U32		NumBlocksReadAveragePerSec;	// Note:  each read may have multiple of blocks
	U32		NumBlocksReadMinimumPerSec;
	U32		NumBlocksReadMaximumPerSec;

	U32		NumBlocksWrittenAveragePerSec;	// Note:  each write may have multiple of blocks
	U32		NumBlocksWrittenMinimumPerSec;
	U32		NumBlocksWrittenMaximumPerSec;

	U32		NumSGCombinedReadsAveragePerSec;  
	U32		NumSGCombinedReadsMinimumPerSec;
	U32		NumSGCombinedReadsMaximumPerSec;

	U32		NumSGCombinedWritesAveragePerSec;
	U32		NumSGCombinedWritesMinimumPerSec;
	U32		NumSGCombinedWritesMaximumPerSec;

	U32		NumOverwritesAveragePerSec;
	U32		NumOverwritesMinimumPerSec;
	U32		NumOverwritesMaximumPerSec;

} ArrayPerformanceRecord, ArrayPerformanceTable[];

#endif  // __ArrayPerformanceTable_h