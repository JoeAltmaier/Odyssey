/**********************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ArrayPerformanceReporter.cpp
// 
// Description:
// This file implements the ArrayPerformanceReporter Reporter class. 
// 
// Update Log 
//
// $Log: /Gemini/Odyssey/DdmReporter/ArrayPerformanceReporter.cpp $
// 
// 6     12/15/99 2:04p Vnguyen
// Move Tracef.  Also increment key counter for each update.  This way we
// can tell when an update just happens and see if the info is propogated
// to GMI GUI or not.
// 
// 5     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 4     11/02/99 8:53a Vnguyen
// Add two performance counters for read and writes.  Also, scale the
// performance counters to per second.
// 
// 3     9/15/99 2:24p Vnguyen
// Fix up the init routine to set performance and status counters to zero
// during init time.  Also, fix bug with memcpy, the source and dest
// pointers were swapped.
// 
// 2     8/16/99 12:59p Vnguyen
// Fix various compiler errors.  Mostly typo and mis-match parameters.
// 
// 1     7/12/99 10:13a Jlane
// Initial Checkin.  
// 
/**********************************************************************/
#include "ArrayPerformanceReporter.h"
#include "RaidPerf.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "DefaultRates.h"


/**********************************************************************/
// Initialize - This is the initializor for the FCP Target Reporter Object.
// We've been called because the FCP Ddm has intialized and now starts
// the Performance Reporter.  He wants to instantiate (and initialize) 
// a new reporter object for the Loop Descriptor entry.  Our Job, is to 
// make sure that our performance table and record are created and if not, 
// create them.  Note:  All the work is done by the lower class object.
// All we need to do here is to supply Ddm specific info, e.g. table names,
// a buffer to hold the row record, etc.
/**********************************************************************/
STATUS ArrayPerformanceReporter::Initialize(
	DdmReporter		*pPHSReporter,	// Pointer to PHSReporter DDM.
	DID				didDdm,
	VDN				vdnDdm,
	REPORTERCODE	ReporterCode
)
{
STATUS	status;

	//
	// Initialize our parent Ddm.  Need to do this to send etc.
	SetParentDdm(m_pPHSReporter = pPHSReporter);

	// Initialize the DataBlock structure with Storage Roll Call data before handing off
	strcpy(m_Data.SRCandExportTableName, STORAGE_ROLL_CALL_TABLE);
	
	m_Data.pSRCandExportRecord = &m_SRCRecord;
	m_Data.cbSRCandExportRecordMax = sizeof(m_SRCRecord);
	
	strcpy(m_Data.SRCandExportvdnName, fdSRC_VDNBSADDM); // Actual name of vdnDdm field in SRC/Export/Loop Descriptor table
	
	strcpy(m_Data.SRCandExportPHSRowIDName, fdSRC_PERFORMANCE_RID);	// String64 rgbFieldName

	m_Data.pSRCandExportRowID = &m_SRCRecord.rid;
	m_Data.pPHSRowID = &m_SRCRecord.ridPerformanceRecord;

	// Also init PHS record for Array Performance record
	strcpy(m_Data.PHSTableName, ARRAY_PERFORMANCE_TABLE); 	// Name of table to send PHS data to
	m_Data.aPHSTableFieldDefs = (fieldDef *)aArrayPerformanceTable_FieldDefs;	// FieldDefs for PHS table 
	m_Data.cbPHSTableFieldDefs = cbArrayPerformanceTable_FieldDefs;	// size of PHS table fieldDefs, U32 cbrgFieldDefs

	m_Data.pPHSRecord = &m_ArrayPerformanceRecord;			// A pointer to a buffer for a PHS row record
	m_Data.cbPHSRecord = sizeof(m_ArrayPerformanceRecord); 	// Size of a PHS row record, cbRowData
	m_Data.cbPHSRecordMax = sizeof(m_ArrayPerformanceRecord); // Maximum size of buffer to receive row data, cbRowDataRetMax

	m_Data.pRefreshRate = &m_ArrayPerformanceRecord.RefreshRate;
	m_Data.pSampleRate = &m_ArrayPerformanceRecord.SampleRate;

	ResetCounters();	// Not really needed as the base object should call ResetCounters before
						// starting the data collection.

	// Up up and away...
	status = InitSRCandExport(&m_Data, didDdm, vdnDdm, ReporterCode);
	return status;
 }


/**********************************************************************/
// InitializePHSRecord - This method fills in the PHS table record with 
// default values.  The RowID for the Loop Descriptor Table entry is also
// put in the PHS record.  
/**********************************************************************/
void ArrayPerformanceReporter::InitializePHSRecord(
 		void *pPHSRecord, 
 		rowID *pSRCandExportRowID
 )
 {
 	ArrayPerformanceRecord	*pArrayPerformanceRecord;
 	pArrayPerformanceRecord = (ArrayPerformanceRecord *) pPHSRecord;

	// Declare and initialize a Array Performance Record with default values. 
	ArrayPerformanceRecord	newArrayPerformanceRecord = {
		// 	Default Value						FieldName									  Size   Type
			0, 0, 0,						// "rid"										8,	I64_FT,
			CT_ARRAYPT_TABLE_VERSION,		//	version: Version of ArrayPerformanceTable record.
			sizeof(ArrayPerformanceRecord),	//	size: # of bytes in record
			0,								//	key: Currently unused.
			ARRAY_PERFORMANCE_REFRESH_RATE,	//	RefreshRate 0=refresh once > 0 implies refresh every x microseconds    
			ARRAY_PERFORMANCE_SAMPLE_RATE,	//	SampleRate: rate at which we sample DDMs data.
			0, 0, 0,						//	ridSRCRecord: Row ID of  Storage Roll Call  entry 4 this device.

			0,0,0,0, 0,0,0,0, 0,0,0,0, 		// NumRdsAverage[NUM_MEASURE_SIZES];
			0,0,0,0, 0,0,0,0, 0,0,0,0, 		// NumRdsMinimum[NUM_MEASURE_SIZES];
			0,0,0,0, 0,0,0,0, 0,0,0,0, 		// NumRdsMaximum[NUM_MEASURE_SIZES];
	
			0,0,0,0, 0,0,0,0, 0,0,0,0, 		// NumWrtsAverage[NUM_MEASURE_SIZES];
			0,0,0,0, 0,0,0,0, 0,0,0,0, 		// NumWrtsMinimum[NUM_MEASURE_SIZES];
			0,0,0,0, 0,0,0,0, 0,0,0,0, 		// NumWrtsMaximum[NUM_MEASURE_SIZES];
			
			0,								// NumReadsAverage;
			0,								// NumReadsMinimum;
			0,								// NumReadsMaximum;

			0,								// NumWritesAverage;
			0,								// NumWritesMinimum;
			0,								// NumWritesMaximum;

			0,								// NumBlocksReadAverage;
			0,								// NumBlocksReadMinimum;
			0,								// NumBlocksReadMaximum;

			0,								// NumBlocksWrittenAverage;
			0,								// NumBlocksWrittenMinimum;
			0,								// NumBlocksWrittenMaximum;

			0,								// NumSGCombinedReadsAverage;
			0,								// NumSGCombinedReadsMinimum;
			0,								// NumSGCombinedReadsMaximum;

			0,								// NumSGCombinedWritesAverage;
			0,								// NumSGCombinedWritesMinimum;
			0,								// NumSGCombinedWritesMaximum;

			0,								// NumOverwritesAverage;
			0,								// NumOverwritesMinimum;
			0								// NumOverwritesMaximum;
		};	// end of default array performance record initializor.
		
	// We make a copy to intialize and return a PHS row record with default values.
	*pArrayPerformanceRecord = newArrayPerformanceRecord;

 	pArrayPerformanceRecord->ridSRCRecord = *pSRCandExportRowID;
 	return;	
 }


/**********************************************************************/
// HandleDdmSampleData -  Handle the sample perofrmance data returned
// from the Ddm nby performing 
/**********************************************************************/
STATUS ArrayPerformanceReporter::HandleDdmSampleData( void* pDdmData, U32 cbDdmData )
{
#pragma unused(cbDdmData)
RAID_PERFORMANCE		*pRAID_Performance;
STATUS					status = OK;

U32	NumReads;
U32 NumWrites;

	// Tracef("ArrayPerformanceReporter::HandleDdmSampleData()\n");
	// Get a pointer to the reply payload and cast it to our reply structure.
	pRAID_Performance = (RAID_PERFORMANCE*)pDdmData;
	
/* The data returned by the Ddm:
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

typedef struct
{
	U32		NumReads[NUM_MEASURE_SIZES];
	U32		NumWrites[NUM_MEASURE_SIZES];
	U32		NumBlocksRead;
	U32		NumBlocksWritten;
	U32		NumSGCombinedReads;
	U32		NumSGCombinedWrites;
	U32		NumOverwrites;
} RAID_PERFORMANCE;

*/

	// Calculate our own running statistics.
	NumReads = 0;
	NumWrites = 0;
	for (int n = 0; n < NUM_MEASURE_SIZES; n++)
	{
		m_NumRds[n].Sample(pRAID_Performance->NumReads[n]);
		NumReads += pRAID_Performance->NumReads[n];
		m_NumWrts[n].Sample(pRAID_Performance->NumWrites[n]);
		NumWrites += pRAID_Performance->NumWrites[n];
	}
	
		m_NumReads.Sample(NumReads);
		m_NumWrites.Sample(NumWrites);
		
		m_NumBlocksRead.Sample(pRAID_Performance->NumBlocksRead);
		m_NumBlocksWritten.Sample(pRAID_Performance->NumBlocksWritten);
		m_NumSGCombinedReads.Sample(pRAID_Performance->NumSGCombinedReads);
		m_NumSGCombinedWrites.Sample(pRAID_Performance->NumSGCombinedWrites);
		m_NumOverwrites.Sample(pRAID_Performance->NumOverwrites);
	
	return OK;
}


	
/**********************************************************************
// ReturnTableRefreshData -  Returns the data to be refreshed to the 
// ArrayPerformancetable
**********************************************************************/
STATUS ArrayPerformanceReporter::ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData)
{
	// Tracef("ArrayPerformanceReporter::ReturnTableRefreshData()\n");

	U32 scale32;
//	I64	scale64;
		
	// The Listen on PHS Table needs a separate record to avoid interference.
	
	// Note:  If sample rate is less than 1 second, we won't be able to return meaningful
	// numbers.  In normal operation, we should be okay as sample rate is going to be a
	// multiple of seconds.  If we want to do subsecond reporting of performance data,
	// we can change the scale to represent milisecond and recompile.
	scale32 = m_ArrayPerformanceRecord.SampleRate/1000000; // length of sample rate in seconds.
	scale32 = scale32 ? scale32 : 1; // Protect agaisnt divided by zero
//	scale64 = scale32;


	// Calculate statistics and populate our local ArrayPerformanceRecord.
	for (int n = 0; n < NUM_MEASURE_SIZES; n++)
	{
		m_ArrayPerformanceRecord.NumRdsAveragePerSec[n] = m_NumRds[n].Average()/scale32;
		m_ArrayPerformanceRecord.NumRdsMinimumPerSec[n] = m_NumRds[n].Min()/scale32;
		m_ArrayPerformanceRecord.NumRdsMaximumPerSec[n] = m_NumRds[n].Max()/scale32;

		m_ArrayPerformanceRecord.NumWrtsAveragePerSec[n] = m_NumWrts[n].Average()/scale32;
		m_ArrayPerformanceRecord.NumWrtsMinimumPerSec[n] = m_NumWrts[n].Min()/scale32;
		m_ArrayPerformanceRecord.NumWrtsMaximumPerSec[n] = m_NumWrts[n].Max()/scale32;
	}  // end of for loop

	m_ArrayPerformanceRecord.NumReadsAveragePerSec = m_NumReads.Average()/scale32;
	m_ArrayPerformanceRecord.NumReadsMinimumPerSec = m_NumReads.Min()/scale32;
	m_ArrayPerformanceRecord.NumReadsMaximumPerSec = m_NumReads.Max()/scale32;

	m_ArrayPerformanceRecord.NumWritesAveragePerSec = m_NumWrites.Average()/scale32;
	m_ArrayPerformanceRecord.NumWritesMinimumPerSec = m_NumWrites.Min()/scale32;
	m_ArrayPerformanceRecord.NumWritesMaximumPerSec = m_NumWrites.Max()/scale32;

	m_ArrayPerformanceRecord.NumBlocksReadAveragePerSec = m_NumBlocksRead.Average()/scale32;
	m_ArrayPerformanceRecord.NumBlocksReadMinimumPerSec = m_NumBlocksRead.Min()/scale32;
	m_ArrayPerformanceRecord.NumBlocksReadMaximumPerSec = m_NumBlocksRead.Max()/scale32;
	
	m_ArrayPerformanceRecord.NumBlocksWrittenAveragePerSec = m_NumBlocksWritten.Average()/scale32;
	m_ArrayPerformanceRecord.NumBlocksWrittenMinimumPerSec = m_NumBlocksWritten.Min()/scale32;
	m_ArrayPerformanceRecord.NumBlocksWrittenMaximumPerSec = m_NumBlocksWritten.Max()/scale32;
	
	m_ArrayPerformanceRecord.NumSGCombinedReadsAveragePerSec = m_NumSGCombinedReads.Average()/scale32;
	m_ArrayPerformanceRecord.NumSGCombinedReadsMinimumPerSec = m_NumSGCombinedReads.Min()/scale32;
	m_ArrayPerformanceRecord.NumSGCombinedReadsMaximumPerSec = m_NumSGCombinedReads.Max()/scale32;
	
	m_ArrayPerformanceRecord.NumSGCombinedWritesAveragePerSec = m_NumSGCombinedWrites.Average()/scale32;
	m_ArrayPerformanceRecord.NumSGCombinedWritesMinimumPerSec = m_NumSGCombinedWrites.Min()/scale32;
	m_ArrayPerformanceRecord.NumSGCombinedWritesMaximumPerSec = m_NumSGCombinedWrites.Max()/scale32;
	
	m_ArrayPerformanceRecord.NumOverwritesAveragePerSec = m_NumOverwrites.Average()/scale32;
	m_ArrayPerformanceRecord.NumOverwritesMinimumPerSec = m_NumOverwrites.Min()/scale32;
	m_ArrayPerformanceRecord.NumOverwritesMaximumPerSec = m_NumOverwrites.Max()/scale32;

	ResetCounters();
	// Return the data to the base class.
	m_ArrayPerformanceRecord.key++;
	rpTableData = &m_ArrayPerformanceRecord;
	rcbTableData = sizeof(m_ArrayPerformanceRecord);
	
	return OK;

}



/**********************************************************************
// ResetCounters -  Clear all counters and other variable to start the
// next Refresh period.  Note:  Base object may also call this method
// before starting the Reporter.  Basically, this method should be safe
// to call anytime before starting a data collection run over a Refresh
// preriod.
/**********************************************************************/
void ArrayPerformanceReporter::ResetCounters()
{
	
	// Initialize
	for (int n = 0; n < NUM_MEASURE_SIZES; n++)
	{
		m_NumRds[n].Reset();
		m_NumWrts[n].Reset();
	}
	

		m_NumReads.Reset();
		m_NumWrites.Reset();
		
		m_NumBlocksRead.Reset();
		m_NumBlocksWritten.Reset();
		m_NumSGCombinedReads.Reset();
		m_NumSGCombinedWrites.Reset();
		m_NumOverwrites.Reset();
	
	return;
}