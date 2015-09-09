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
// This is the SSD Performance Table
//
// $Log: /Gemini/Include/CTTables/SSDPerformanceTable.h $
// 
// 3     10/08/99 9:17a Vnguyen
// Update tables to add additional performance and status counters.
// Fix bug with SSD Performance Table name.  Was using wrong string name.
// 
// 2     8/18/99 7:36a Vnguyen
// Fix a few typo errors.
// 
// 1     8/17/99 8:27a Vnguyen
// Initial check-in.
// 
// Update Log: 
/*************************************************************************/

#ifndef _SSDPerformanceTable_h_
#define _SSDPerformanceTable_h_

#include "CtTypes.h"
#include "PTSCommon.h"
#include "Odyssey.h"

#pragma	pack(4)

// Field definitions in SSDPerformanceTable.cpp
extern	const fieldDef	aSSDPerformanceTable_FieldDefs[];
extern	const U32		cbSSDPerformanceTable_FieldDefs;

#define CT_SSDPT_TABLE 			"SSD_Performance_Table"
#define CT_SSDPT_TABLE_VERSION	1
#define SSD_PERFORMANCE_TABLE	CT_SSDPT_TABLE

//
// SSD Performance Data - 
//
typedef struct {
	rowID	rid;						// Row ID of this row.
	U32		version;					// Version of DiskPerformanceTable record.
	U32		size;						// # of bytes in record
	U32		key;						// For listen operation
	U32		RefreshRate;				// in microseconds.  Time to update PTS record.  
										// Must be a multiple of SampleRate

	U32		SampleRate;					// in microseconds.  Time to sample DDM for data.
										// Must be a multiple of seconds.
										
	rowID	ridSRCRecord;				// Row ID of  Storage Roll Call  entry 4 this device.
	I64		UpTime;						// [For future implementation] Number of microseconds 
										// this disk drive has been spun-up.
										
//
	U32		AvgNumPagesReadPerSec;		// Average # of pages read per second
	U32		MaxNumPagesReadPerSec;
	U32		MinNumPagesReadPerSec;

	U32		AvgNumPagesReadCacheHitPerSec;	// Average # of cache hit (read) per second.
	U32		MaxNumPagesReadCacheHitPerSec;
	U32		MinNumPagesReadCacheHitPerSec;

	U32		AvgNumPagesReadCacheMissPerSec;	// Average # of cache miss (read) per second.
	U32		MaxNumPagesReadCacheMissPerSec;
	U32		MinNumPagesReadCacheMissPerSec;
	
	U32		AvgNumPagesWritePerSec;		// Average # of pages written per second
	U32		MaxNumPagesWritePerSec;
	U32		MinNumPagesWritePerSec;
	
	U32		AvgNumPagesWriteCacheHitPerSec;	// Average # of cache hit (write) per second.
	U32		MaxNumPagesWriteCacheHitPerSec;
	U32		MinNumPagesWriteCacheHitPerSec;

	U32		AvgNumPagesWriteCacheMissPerSec;	// Average # of cache miss (write) per second.
	U32		MaxNumPagesWriteCacheMissPerSec;
	U32		MinNumPagesWriteCacheMissPerSec;
	
	U32		AvgNumErasePagesAvailable;		// If this gets low [how low?] performance can suffer
	U32		MaxNumErasePagesAvailable;		
	U32		MinNumErasePagesAvailable;		

	I64		AvgNumReadBytesTotalPerSec;		//	Average # of bytes read per second
	I64		MaxNumReadBytesTotalPerSec;		
	I64		MinNumReadBytesTotalPerSec;		

	I64		AvgNumWriteBytesTotalPerSec;	// Average # of bytes written per second
	I64		MaxNumWriteBytesTotalPerSec;		
	I64		MinNumWriteBytesTotalPerSec;		

} SSDPerformanceRecord, SSDPerformanceTable[];


#define CT_SSDPT_RID							"rid"
#define CT_SSDPT_VERSION						"version"
#define CT_SSDPT_SIZE							"size"
#define CT_SSDPT_KEY							"key"
#define CT_SSDPT_REFRESHRATE					"RefreshRate"
#define CT_SSDPT_SAMPLERATE						"SampleRate"
#define CT_SSDPT_RIDSRCRECORD					"ridSRCRecord"
#define CT_SSDPT_UPTIME							"UpTime"

#define CT_SSDPT_AVGNUMPAGESREADPERSEC			"AvgNumPagesReadPerSec"
#define CT_SSDPT_MAXNUMPAGESREADPERSEC			"MaxNumPagesReadPerSec"
#define CT_SSDPT_MINNUMPAGESREADPERSEC			"MinNumPagesReadPerSec"

#define CT_SSDPT_AVGNUMPAGESREADCACHEHITPERSEC	"AvgNumPagesReadCacheHitPerSec"
#define CT_SSDPT_MAXNUMPAGESREADCACHEHITPERSEC	"MaxNumPagesReadCacheHitPerSec"
#define CT_SSDPT_MINNUMPAGESREADCACHEHITPERSEC	"MinNumPagesReadCacheHitPerSec"

#define CT_SSDPT_AVGNUMPAGESREADCACHEMISSPERSEC	"AvgNumPagesReadCacheMissPerSec"
#define CT_SSDPT_MAXNUMPAGESREADCACHEMISSPERSEC	"MaxNumPagesReadCacheMissPerSec"
#define CT_SSDPT_MINNUMPAGESREADCACHEMISSPERSEC	"MinNumPagesReadCacheMissPerSec"
	
	
#define CT_SSDPT_AVGNUMPAGESWRITEPERSEC			"AvgNumPagesWritePerSec"
#define CT_SSDPT_MAXNUMPAGESWRITEPERSEC			"MaxNumPagesWritePerSec"
#define CT_SSDPT_MINNUMPAGESWRITEPERSEC			"MinNumPagesWritePerSec"
	
#define CT_SSDPT_AVGNUMPAGESWRITECACHEHITPERSEC	"AvgNumPagesWriteCacheHitPerSec"
#define CT_SSDPT_MAXNUMPAGESWRITECACHEHITPERSEC	"MaxNumPagesWriteCacheHitPerSec"
#define CT_SSDPT_MINNUMPAGESWRITECACHEHITPERSEC	"MinNumPagesWriteCacheHitPerSec"

#define CT_SSDPT_AVGNUMPAGESWRITECACHEMISSPERSEC	"AvgNumPagesWriteCacheMissPerSec"
#define CT_SSDPT_MAXNUMPAGESWRITECACHEMISSPERSEC	"MaxNumPagesWriteCacheMissPerSec"
#define CT_SSDPT_MINNUMPAGESWRITECACHEMISSPERSEC	"MinNumPagesWriteCacheMissPerSec"
	
#define CT_SSDPT_AVGNUMERASEPAGESAVAILABLE		"AvgNumErasePagesAvailable"
#define CT_SSDPT_MAXNUMERASEPAGESAVAILABLE		"MaxNumErasePagesAvailable"		
#define CT_SSDPT_MINNUMERASEPAGESAVAILABLE		"MinNumErasePagesAvailable"		

#define CT_SSDPT_AVGNUMREADBYTESTOTALPERSEC		"AvgNumReadBytesTotalPerSec"
#define CT_SSDPT_MAXNUMREADBYTESTOTALPERSEC		"MaxNumReadBytesTotalPerSec"
#define CT_SSDPT_MINNUMREADBYTESTOTALPERSEC		"MinNumReadBytesTotalPerSec"		

#define CT_SSDPT_AVGNUMWRITEBYTESTOTALPERSEC	"AvgNumWriteBytesTotalPerSec"
#define CT_SSDPT_MAXNUMWRITEBYTESTOTALPERSEC	"MaxNumWriteBytesTotalPerSec"
#define CT_SSDPT_MINNUMWRITEBYTESTOTALPERSEC	"MinNumWriteBytesTotalPerSec"		


#endif // _SSDPerformanceTable_h_
