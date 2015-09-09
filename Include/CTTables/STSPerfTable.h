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
// This is the SCSI Target Server (STS) Performance Table
//
// $Log: /Gemini/Include/CTTables/STSPerfTable.h $
// 
// 6     10/18/99 9:57a Vnguyen
// Change key fron I64 to U32 to be consistent with other Reporters.  Also
// add a few comments to explain the meaning of the counters.
// 
// 5     9/14/99 5:36p Vnguyen
// Update performance and status counters to match the counters returned
// by the SCSI Target Server DDM.
// 
// 4     8/16/99 8:40a Vnguyen
// Fix up parameter declaration to pass in DID for the PHS Reporter class
// object to use.
// 
// 3     8/02/99 8:58a Vnguyen
// 
// 2     7/28/99 10:16a Vnguyen
// 
// 1     7/28/99 9:44a Vnguyen
// Initial check-in
// 
// 
// Update Log: 
// 3/27/99	Vnguyen.  Created.
/*************************************************************************/

#ifndef _STSPerfRecord_h_
#define _STSPerfRecord_h_

#include "CtTypes.h"
#include "PTSCommon.h"
#include "Odyssey.h"

#pragma	pack(4)

// Field definitions in STSPerfTable.cpp
extern	const fieldDef	aSTSPerfTable_FieldDefs[];
extern	const U32		cbSTSPerfTable_FieldDefs;

#define CT_STSPT_TABLE_NAME "STS_Performance_Table"
#define CT_STSPT_TABLE_VERSION	1
#define STS_PERFORMANCE_TABLE	CT_STSPT_TABLE_NAME

//
// STS Performance Data - (one row for each installed disk drive)
//
typedef struct {
	rowID	rid;					// Row ID of this row.
	U32		version;					// Version of STSPerfTable record.
	U32		size;						// # of bytes in record
	U32		key;						// For listen operation

	U32		RefreshRate;				// in microseconds.  How often to update PTS.  Must be a multiple
										// of SampleRate.
	U32		SampleRate;					// in microseconds.  Rate at which PHSReporters sample DDMs data.
										// Must be a multiple of seconds.
										
	rowID	ridExportRecord;			// Row ID of Export (Export Table) entry 4 this device.

	U32		AvgNumBSAReadsPerSec;		// Average number of BSA reads per second
	U32		MaxNumBSAReadsPerSec;
	U32		MinNumBSAReadsPerSec;
	
	U32		AvgNumBSAWritesPerSec;		// Average number of BSA writes per second
	U32		MaxNumBSAWritesPerSec;
	U32		MinNumBSAWritesPerSec;
	
	U32		AvgNumBSACmdsPerSec;		// Average number of BSA Commands (R + W) per second
	U32		MaxNumBSACmdsPerSec;
	U32		MinNumBSACmdsPerSec;
	
	U32		AvgNumSCSICmdsPerSec;		// Average number of SCSI Commands (BSA + others) per second
	U32		MaxNumSCSICmdsPerSec;
	U32		MinNumSCSICmdsPerSec;
	
	I64		AvgNumBSABytesReadPerSec;	// Average number of bytes read per second
	I64		MaxNumBSABytesReadPerSec;
	I64		MinNumBSABytesReadPerSec;

	I64		AvgNumBSABytesWrittenPerSec; // Average number of bytes written per second
	I64		MaxNumBSABytesWrittenPerSec;
	I64		MinNumBSABytesWrittenPerSec;

	I64		AvgNumBSABytesPerSec;		// total bytes:  read + write
	I64		MaxNumBSABytesPerSec;
	I64		MinNumBSABytesPerSec;
} STSPerfRecord, STSPerfTable[];


//STSPT - SCSI Target Server Performance Record

#define CT_STSPT_RID								"rid"
#define CT_STSPT_VERSION							"version"
#define CT_STSPT_SIZE								"size"
#define CT_STSPT_KEY								"key"
#define CT_STSPT_REFRESHRATE						"RefreshRate"
#define CT_STSPT_SAMPLERATE							"SampleRate"
#define CT_STSPT_RIDEXPORTRECORD					"ridExportRecord"

#define CT_STSPT_AVGNUMBSAREADSPERSEC				"AvgNumBSAReadsPerSec"
#define CT_STSPT_MAXNUMBSAREADSPERSEC				"MaxNumBSAReadsPerSec"
#define CT_STSPT_MINNUMBSAREADSPERSEC				"MinNumBSAReadsPerSec"
	
#define CT_STSPT_AVGNUMBSAWRITESPERSEC				"AvgNumBSAWritesPerSec"
#define CT_STSPT_MAXNUMBSAWRITESPERSEC				"MaxNumBSAWritesPerSec"
#define CT_STSPT_MINNUMBSAWRITESPERSEC				"MinNumBSAWritesPerSec"
	
#define CT_STSPT_AVGNUMBSACMDSPERSEC				"AvgNumBSACmdsPerSec"
#define CT_STSPT_MAXNUMBSACMDSPERSEC				"MaxNumBSACmdsPerSec"
#define CT_STSPT_MINNUMBSACMDSPERSEC				"MinNumBSACmdsPerSec"
	
#define CT_STSPT_AVGNUMSCSICMDSPERSEC				"AvgNumSCSICmdsPerSec"
#define CT_STSPT_MAXNUMSCSICMDSPERSEC				"MaxNumSCSICmdsPerSec"
#define CT_STSPT_MINNUMSCSICMDSPERSEC				"MinNumSCSICmdsPerSec"

#define CT_STSPT_AVGNUMBSABYTESREADPERSEC			"AvgNumBSABytesReadPerSec"
#define CT_STSPT_MAXNUMBSABYTESREADPERSEC			"MaxNumBSABytesReadPerSec"
#define CT_STSPT_MINNUMBSABYTESREADPERSEC			"MinNumBSABytesReadPerSec"

#define CT_STSPT_AVGNUMBSABYTESWRITTENPERSEC		"AvgNumBSABytesWrittenPerSec"
#define CT_STSPT_MAXNUMBSABYTESWRITTENPERSEC		"MaxNumBSABytesWrittenPerSec"
#define CT_STSPT_MINNUMBSABYTESWRITTENPERSEC		"MinNumBSABytesWrittenPerSec"

#define CT_STSPT_AVGNUMBSABYTESPERSEC				"AvgNumBSABytesPerSec"
#define CT_STSPT_MAXNUMBSABYTESPERSEC				"MaxNumBSABytesPerSec"
#define CT_STSPT_MINNUMBSABYTESPERSEC				"MinNumBSABytesPerSec"

#endif // _STSPerfRecord_h_
