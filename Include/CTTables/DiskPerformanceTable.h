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
// This is the Disk Performance Table
//
// $Log: /Gemini/Include/CTTables/DiskPerformanceTable.h $
// 
// 7     1/04/00 4:31p Jlane
// Pad to a multiple of 8 bytes and pad so I64s are 8-byte aligned.
// 
// 6     11/22/99 4:05p Vnguyen
// Change key from I64 to U32 to be consistent with other reporters.
// 
// 5     9/08/99 10:46a Vnguyen
// Update performance and status counters.  Also, change performance
// counters from per sample to per second.
// 
// 4     8/16/99 12:54p Vnguyen
// Fix various errors during compiling.  Mostly typo and mis-match
// parameters.
// 
// 3     5/04/99 9:36a Jlane
// Miscellaneous updates to get compiuling and to bring in synch with new
// Ddm Model.
// 
// 2     4/26/99 5:38p Jlane
// Changed ridThisRow to rid.
// 
// Update Log: 
// 11/23/98	JFL	Created.
/*************************************************************************/

#ifndef _DiskPerformanceRecord_h_
#define _DiskPerformanceRecord_h_

#include "CtTypes.h"
#include "PTSCommon.h"
#include "Odyssey.h"

#pragma	pack(4)

// Field definitions in DiskPerformanceTable.cpp
extern	const fieldDef	aDiskPerformanceTable_FieldDefs[];
extern	const U32		cbDiskPerformanceTable_FieldDefs;

#define CT_DPT_TABLE_NAME 		"Disk_Performance_Table"
#define CT_DPT_TABLE_VERSION	1
#define DISK_PERFORMANCE_TABLE	CT_DPT_TABLE_NAME

//
// Disk Performance Data - (one row for each installed disk drive)
//
typedef struct DiskPerformanceRecord {
	rowID	rid;					// Row ID of this row.
	U32		version;					// Version of DiskPerformanceTable record.
	U32		size;						// # of bytes in record
	U32		key;						// For listen operation
	U32		RefreshRate;				// in microseconds.  Must be a multiple of SampleRate
	U32		SampleRate;					// in microseconds.  Must be a multiple of second.
	U32		U32Pad1;	// add 4 bytes padding to align on 8-byte boundary

	rowID	ridSRCRecord;				// Row ID of  Storage Roll Call  entry 4 this device.
	I64		UpTime;						// Number of microseconds this disk drive has been spun-up.
	
	U32		AvgReadsPerSec;				// Average number of reads per second
	U32		MaxReadsPerSec;
	U32		MinReadsPerSec;
	
	U32		AvgWritesPerSec;			// Average number of writes per second
	U32		MaxWritesPerSec;
	U32		MinWritesPerSec;
	
	U32		AvgTransferPerSec;			// Average number of transfers (read + write) per second
	U32		MaxTransferPerSec;
	U32		MinTransferPerSec;
	U32		U32Pad2;	// add 4 bytes padding to align on 8-byte boundary
	
//
	I64		AvgBytesReadPerSec;			// Total bytes read
	I64		MaxBytesReadPerSec;			// Total bytes read
	I64		MinBytesReadPerSec;			// Total bytes read

	I64		AvgBytesWrittenPerSec;		// Total bytes written
	I64		MaxBytesWrittenPerSec;		// Total bytes written
	I64		MinBytesWrittenPerSec;		// Total bytes written

	I64		AvgBytesTransferredPerSec;	// Total bytes transferred (read+write)
	I64		MaxBytesTransferredPerSec;	// Total bytes transferred (read+write)
	I64		MinBytesTransferredPerSec;	// Total bytes transferred (read+write)
//
	U32		AvgReadSize;				// # of bytes read /# of reads
	U32		MaxReadSize;				// # of bytes read /# of reads
	U32		MinReadSize;				// # of bytes read /# of reads

	U32		AvgWriteSize;
	U32		MaxWriteSize;
	U32		MinWriteSize;

	U32		AvgTransferSize;			// (#of Bytes R + # of Bytes W)/(# of Reads + # of Writes)
	U32		MaxTransferSize;			// (#of Bytes R + # of Bytes W)/(# of Reads + # of Writes)
	U32		MinTransferSize;			// (#of Bytes R + # of Bytes W)/(# of Reads + # of Writes)
	U32		U32Pad3;	// add 4 bytes padding to align on 8-byte boundary

//
	I64		AvgMicroSecPerRead;
	I64		MaxMicroSecPerRead;
	I64		MinMicroSecPerRead;

	I64		AvgMicroSecPerWrite;
	I64		MaxMicroSecPerWrite;
	I64		MinMicroSecPerWrite;

	I64		AvgMicroSecPerTransfer;
	I64		MaxMicroSecPerTransfer;
	I64		MinMicroSecPerTransfer;

} DiskPerformanceRecord, DiskPerformanceTable[];


#define CT_DPT_RID								"rid"
#define CT_DPT_VERSION							"version"
#define CT_DPT_SIZE								"size"
#define CT_DPT_KEY								"key"
#define CT_DPT_REFRESHRATE						"RefreshRate"
#define CT_DPT_SAMPLERATE						"SampleRate"
#define CT_DPT_U32PAD1							"U32Pad1"

#define CT_DPT_RIDSRCRECORD						"ridSRCRecord"
#define CT_DPT_UPTIME							"UpTime"

#define CT_DPT_AVGREADSPERSEC				"AvgReadsPerSec"
#define CT_DPT_MAXREADSPERSEC				"MaxReadsPerSec"
#define CT_DPT_MINREADSPERSEC				"MinReadsPerSec"
	
#define CT_DPT_AVGWRITESPERSEC				"AvgWritesPerSec"
#define CT_DPT_MAXWRITESPERSEC				"MaxWritesPerSec"
#define CT_DPT_MINWRITESPERSEC				"MinWritesPerSec"
	
#define CT_DPT_AVGTRANSFERPERSEC			"AvgTransferPerSec"
#define CT_DPT_MAXTRANSFERPERSEC			"MaxTransferPerSec"
#define CT_DPT_MINTRANSFERPERSEC			"MinTransferPerSec"
#define CT_DPT_U32PAD2							"U32Pad2"

//
#define CT_DPT_AVGBYTESREADPERSEC			"AvgBytesReadPerSec"
#define CT_DPT_MAXBYTESREADPERSEC			"MaxBytesReadPerSec"
#define CT_DPT_MINBYTESREADPERSEC			"MinBytesReadPerSec"

#define CT_DPT_AVGBYTESWRITTENPERSEC		"AvgBytesWrittenPerSec"
#define CT_DPT_MAXBYTESWRITTENPERSEC		"MaxBytesWrittenPerSec"
#define CT_DPT_MINBYTESWRITTENPERSEC		"MinBytesWrittenPerSec"

#define CT_DPT_AVGBYTESTRANSFERREDPERSEC	"AvgBytesTransferredPerSec"
#define CT_DPT_MAXBYTESTRANSFERREDPERSEC	"MaxBytesTransferredPerSec"
#define CT_DPT_MINBYTESTRANSFERREDPERSEC	"MinBytesTransferredPerSec"
//
#define CT_DPT_AVGREADSIZE					"AvgReadSize"
#define CT_DPT_MAXREADSIZE					"MaxReadSize"
#define CT_DPT_MINREADSIZE					"MinReadSize"

#define CT_DPT_AVGWRITESIZE					"AvgWriteSize"
#define CT_DPT_MAXWRITESIZE					"MaxWriteSize"
#define CT_DPT_MINWRITESIZE					"MinWriteSize"

#define CT_DPT_AVGTRANSFERSIZE				"AvgTransferSize"
#define CT_DPT_MAXTRANSFERSIZE				"MaxTransferSize"
#define CT_DPT_MINTRANSFERSIZE				"MinTransferSize"
#define CT_DPT_U32PAD3							"U32Pad3"

//
#define CT_DPT_AVGMICROSECPERREAD			"AvgMicroSecPerRead"
#define CT_DPT_MAXMICROSECPERREAD			"MaxMicroSecPerRead"
#define CT_DPT_MINMICROSECPERREAD			"MinMicroSecPerRead"

#define CT_DPT_AVGMICROSECPERWRITE			"AvgMicroSecPerWrite"
#define CT_DPT_MAXMICROSECPERWRITE			"MaxMicroSecPerWrite"
#define CT_DPT_MINMICROSECPERWRITE			"MinMicroSecPerWrite"

#define CT_DPT_AVGMICROSECPERTRANSFER		"AvgMicroSecPerTransfer"
#define CT_DPT_MAXMICROSECPERTRANSFER		"MaxMicroSecPerTransfer"
#define CT_DPT_MINMICROSECPERTRANSFER		"MinMicroSecPerTransfer"

#endif // _DiskPerformanceRecord_h_
