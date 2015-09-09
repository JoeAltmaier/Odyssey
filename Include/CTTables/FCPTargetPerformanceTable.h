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
// This is the Fibre Channel Target Performance Table
//
// $Log: /Gemini/Include/CTTables/FCPTargetPerformanceTable.h $
// 
// 3     10/11/99 6:04p Vnguyen
// Add timestamp for Performance counters and # of errors received for
// Status counters.
// 
// 2     9/15/99 11:56a Vnguyen
// Update Performance Counters to match the counters being returned by the
// FCP Target driver.
// 
// 1     8/16/99 2:42p Vnguyen
// Initial Check-in.
// 
// 
// Update Log: 
//
/*************************************************************************/

#ifndef _FCPTargetPerformanceTable_h_
#define _FCPTargetPerformancetable_h_

#include "CtTypes.h"
#include "PTSCommon.h"
#include "Odyssey.h"

#pragma	pack(4)

// Field definitions in FCPTargetPerformanceTable.cpp
extern	const fieldDef	aFCPTargetPerformanceTable_FieldDefs[];
extern	const U32		cbFCPTargetPerformanceTable_FieldDefs;

#define CT_FCPTPT_TABLE "FCPT_Performance_Table"
#define CT_FCPTPT_TABLE_VERSION	1
#define FCPT_PERFORMANCE_TABLE	CT_FCPTPT_TABLE

//
// Fibre Channel Target Performance Data - (one row for each FCP Target Ddm)
//
typedef struct {
	rowID	rid;						// Row ID of this row.
	U32		version;					// Version of FCPTargetPerformanceTable record.
	U32		size;						// # of bytes in record
	U32		key;						// For listen operation
	U32		RefreshRate;				// in microseconds.  Time to update PTS record.  
										// Must be a multiple of SampleRate
										
	U32		SampleRate;					// in microseconds.  Time to sample DDM for data.
										// Must be a multiple of seconds.
										// 
	rowID	ridLoopDescriptorRecord;	// Row ID of LoopDescriptor table entry 4 this device.
	I64		UpTime;						// in microseconds.  Not being updated at this time.
	
	U32		AvgNumReadPacketsPerSec;	// Average # of read per second
	U32		MaxNumReadPacketsPerSec;
	U32		MinNumReadPacketsPerSec;
	
	U32		AvgNumWritePacketsPerSec;	// Average # of write per second
	U32		MaxNumWritePacketsPerSec;
	U32		MinNumWritePacketsPerSec;
	
	U32		AvgNumRWPacketsPerSec;	// Average # of Read + Write packets
	U32		MaxNumRWPacketsPerSec;
	U32		MinNumRWPacketsPerSec;
	
	U32		AvgNumTotalPacketsPerSec; // Average # of Read + Write + Other packets.
	U32		MaxNumTotalPacketsPerSec;
	U32		MinNumTotalPacketsPerSec;
	
	I64		AvgNumBytesReadPerSec;		// Average # of bytes read per second
	I64		MaxNumBytesReadPerSec;
	I64		MinNumBytesReadPerSec;
	
	I64		AvgNumBytesWrittenPerSec;	// Average # of bytes written per second
	I64		MaxNumBytesWrittenPerSec;
	I64		MinNumBytesWrittenPerSec;
	
	I64		AvgNumBytesTranferredPerSec; // Average # of Read + Write bytes
	I64		MaxNumBytesTranferredPerSec; 
	I64		MinNumBytesTranferredPerSec; 
	
	I64		AvgMicroSecPerRead;		// Average number of microseconds per read
	I64		MaxMicroSecPerRead;
	I64		MinMicroSecPerRead;

	I64		AvgMicroSecPerWrite;	// Average number of microseconds per write
	I64		MaxMicroSecPerWrite;
	I64		MinMicroSecPerWrite;

} FCPTargetPerformanceRecord, FCPTargetPerformanceTable[];


#define CT_FCPTPT_RID							"rid"
#define CT_FCPTPT_VERSION						"version"
#define CT_FCPTPT_SIZE							"size"
#define CT_FCPTPT_KEY							"key"
#define CT_FCPTPT_REFRESHRATE					"RefreshRate"
#define CT_FCPTPT_SAMPLERATE					"SampleRate"
#define CT_FCPTPT_RIDLDRECORD					"ridLoopDescriptorRecord"
#define CT_FCPTPT_UPTIME						"UpTime"

#define CT_FCPTPT_AVGNUMREADPACKETSPERSEC		"AvgNumReadPacketsPerSec"
#define CT_FCPTPT_MAXNUMREADPACKETSPERSEC		"MaxNumReadPacketsPerSec"
#define CT_FCPTPT_MINNUMREADPACKETSPERSEC		"MinNumReadPacketsPerSec"
	
#define CT_FCPTPT_AVGNUMWRITEPACKETSPERSEC		"AvgNumWritePacketsPerSec"
#define CT_FCPTPT_MAXNUMWRITEPACKETSPERSEC		"MaxNumWritePacketsPerSec"
#define CT_FCPTPT_MINNUMWRITEPACKETSPERSEC		"MinNumWritePacketsPerSec"
	
#define CT_FCPTPT_AVGNUMRWPACKETSPERSEC			"AvgNumRWPacketsPerSec"
#define CT_FCPTPT_MAXNUMRWPACKETSPERSEC			"MaxNumRWPacketsPerSec"
#define CT_FCPTPT_MINNUMRWPACKETSPERSEC			"MinNumRWPacketsPerSec"
	
#define CT_FCPTPT_AVGNUMTOTALPACKETSPERSEC		"AvgNumTotalPacketsPerSec"
#define CT_FCPTPT_MAXNUMTOTALPACKETSPERSEC		"MaxNumTotalPacketsPerSec"
#define CT_FCPTPT_MINNUMTOTALPACKETSPERSEC		"MinNumTotalPacketsPerSec"
	
#define CT_FCPTPT_AVGNUMBYTESREADPERSEC			"AvgNumBytesReadPerSec"
#define CT_FCPTPT_MAXNUMBYTESREADPERSEC			"MaxNumBytesReadPerSec"
#define CT_FCPTPT_MINNUMBYTESREADPERSEC			"MinNumBytesReadPerSec"
	
#define CT_FCPTPT_AVGNUMBYTESWRITTENPERSEC		"AvgNumBytesWrittenPerSec"
#define CT_FCPTPT_MAXNUMBYTESWRITTENPERSEC		"MaxNumBytesWrittenPerSec"
#define CT_FCPTPT_MINNUMBYTESWRITTENPERSEC		"MinNumBytesWrittenPerSec"
	
#define CT_FCPTPT_AVGNUMBYTESTRANSFERREDPERSEC	"AvgNumBytesTranferredPerSec"
#define CT_FCPTPT_MAXNUMBYTESTRANSFERREDPERSEC	"MaxNumBytesTranferredPerSec"
#define CT_FCPTPT_MINNUMBYTESTRANSFERREDPERSEC	"MinNumBytesTranferredPerSec"

#define CT_FCPTPT_AVGMICROSECPERREAD			"AvgMicroSecPerRead"
#define CT_FCPTPT_MAXMICROSECPERREAD			"MaxMicroSecPerRead"
#define CT_FCPTPT_MINMICROSECPERREAD			"MinMicroSecPerRead"

#define CT_FCPTPT_AVGMICROSECPERWRITE			"AvgMicroSecPerWrite"
#define CT_FCPTPT_MAXMICROSECPERWRITE			"MaxMicroSecPerWrite"
#define CT_FCPTPT_MINMICROSECPERWRITE			"MinMicroSecPerWrite"

#endif // _FCPTargetPerformanceTable_h_
