/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FCPTargetPerformanceReporter.cpp
// 
// Description:
// This file implements the FCP Target Driver Performance Reporter class. 
// 
// Update Log 
//
// $Log: /Gemini/Odyssey/DdmReporter/FCPTargetPerformanceReporter.cpp $
// 
// 4     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 3     10/11/99 6:02p Vnguyen
// Add timestamp for Performance counter and # of error received for
// Status counters
// 
// 2     9/15/99 11:59a Vnguyen
// Update Performance and Status counters to match the counters returned
// by FCP Target driver.
// 
// 1     8/16/99 2:39p Vnguyen
// New check-in.
// 
/*************************************************************************/

#include "FCPTargetPerformanceReporter.h"
#include "FCPTargetPerformance.h"
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
STATUS FCPTargetPerformanceReporter::Initialize(
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

	// Initialize the DataBlock structure with Loop Descriptor data before handing off
	strcpy(m_Data.SRCandExportTableName, LOOP_DESCRIPTOR_TABLE);
	
	m_Data.pSRCandExportRecord = &m_LoopDescriptorRecord;
	m_Data.cbSRCandExportRecordMax = sizeof(LoopDescriptorRecord);
	
	strcpy(m_Data.SRCandExportvdnName, fdLD_LM_VDN); // Actual name of vdnDdm field in SRC/Export/Loop Descriptor table
	
	strcpy(m_Data.SRCandExportPHSRowIDName, fdLD_PERFORMANCE_RID);	// String64 rgbFieldName

	m_Data.pSRCandExportRowID = &m_LoopDescriptorRecord.rid;
	m_Data.pPHSRowID = &m_LoopDescriptorRecord.ridPerformanceRec;

	strcpy(m_Data.PHSTableName, FCPT_PERFORMANCE_TABLE); 	// Name of table to send PHS data to
	m_Data.aPHSTableFieldDefs = (fieldDef *)aFCPTargetPerformanceTable_FieldDefs;	// FieldDefs for PHS table 
	m_Data.cbPHSTableFieldDefs = cbFCPTargetPerformanceTable_FieldDefs;	// size of PHS table fieldDefs, U32 cbrgFieldDefs

	m_Data.pPHSRecord = &m_FCPTargetPerformanceRecord;			// A pointer to a buffer for a PHS row record
	m_Data.cbPHSRecord = sizeof(FCPTargetPerformanceRecord); 	// Size of a PHS row record, cbRowData
	m_Data.cbPHSRecordMax = sizeof(FCPTargetPerformanceRecord); // Maximum size of buffer to receive row data, cbRowDataRetMax

	m_Data.pRefreshRate = &m_FCPTargetPerformanceRecord.RefreshRate;
	m_Data.pSampleRate = &m_FCPTargetPerformanceRecord.SampleRate;

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
void FCPTargetPerformanceReporter::InitializePHSRecord(
 		void *pPHSRecord, 
 		rowID *pSRCandExportRowID
 )
 {
 	FCPTargetPerformanceRecord	*pFCPTargetPerformanceRecord;
 	pFCPTargetPerformanceRecord = (FCPTargetPerformanceRecord *) pPHSRecord;

	// Declare and initialize a STS Performance Record with default values. 
	FCPTargetPerformanceRecord	newFCPTargetPerformanceRecord = {
	// 	Default Value						FieldName									  Size   Type
		0,0,0,							// "rid"										8,	I64_FT,
		CT_FCPTPT_TABLE_VERSION, // version: Version of FCPTargetPerformanceTable record.
		sizeof(FCPTargetPerformanceRecord), // size: # of bytes in record
		0,								// key: Currently unused.
		FCPT_PERFORMANCE_REFRESH_RATE,	// RefreshRate 
		FCPT_PERFORMANCE_SAMPLE_RATE,	// SampleRate: rate at which we sample DDMs data.
		0,0,0,							// Row ID of LoopDescriptor entry 4 this device.
		0,								// UpTime in microsecond
	
		0,								// AvgNumReadPacketsPerSec;
		0,								// MaxNumReadPacketsPerSec;
		0,								// MinNumReadPacketsPerSec;
	
		0,								// AvgNumWritePacketsPerSec;
		0,								// MaxNumWritePacketsPerSec;
		0,								// MinNumWritePacketsPerSec;
	
		0,								// AvgNumRWPacketsPerSec;
		0,								// MaxNumRWPacketsPerSec;
		0,								// MinNumRWPacketsPerSec;
	
		0,								// AvgNumTotalPacketsPerSec;
		0,								// MaxNumTotalPacketsPerSec;
		0,								// MinNumTotalPacketsPerSec;
	
		0,								// AvgNumBytesReadPerSec;
		0,								// MaxNumBytesReadPerSec;
		0,								// MinNumBytesReadPerSec;
	
		0,								// AvgNumBytesWrittenPerSec;
		0,								// MaxNumBytesWrittenPerSec;
		0,								// MinNumBytesWrittenPerSec;
	
		0,								// AvgNumBytesTranferredPerSec;
		0,								// MaxNumBytesTranferredPerSec;
		0								// MinNumBytesTranferredPerSec;
	
	};	// end of default FCPT performance record initializor.
		
	// We make a copy to intialize and return a PHS row record with default values.
	*pFCPTargetPerformanceRecord = newFCPTargetPerformanceRecord;

 	pFCPTargetPerformanceRecord->ridLoopDescriptorRecord = *pSRCandExportRowID;
 	return;	
 }
 

/**********************************************************************/
// HandleDdmSampleData -  Handle the sample perofrmance data returned
// from the Ddm by performing 
/**********************************************************************/
STATUS FCPTargetPerformanceReporter::HandleDdmSampleData( void* pDdmData, U32 cbDdmData )
{
#pragma unused(cbDdmData)
FCPT_Performance		*pFCPTPerformance;
STATUS					status = OK;

	I64 reads, writes;

	Tracef("FCPTargetPerformanceReporter::HandleDdmSampleData()\n");
	// Get a pointer to the reply payload and cast it to our reply structure.
	pFCPTPerformance = (FCPT_Performance*)pDdmData;

	m_NumReadPackets.Sample(pFCPTPerformance->NumReadPackets);
	m_NumWritePackets.Sample(pFCPTPerformance->NumWritePackets);
	m_NumTotalPackets.Sample(pFCPTPerformance->NumTotalPackets);
	
	m_NumBytesRead.Sample(pFCPTPerformance->NumBytesRead);
	m_NumBytesWritten.Sample(pFCPTPerformance->NumBytesWritten);
	
	// check for zero before doing division.  If zero, then use 1.
	reads = pFCPTPerformance->NumReadPackets ? pFCPTPerformance->NumReadPackets : 1;
	writes = pFCPTPerformance->NumWritePackets ? pFCPTPerformance->NumWritePackets : 1;

	m_ReadLatency.Sample(pFCPTPerformance->TotalReadLatency/reads);
	m_WriteLatency.Sample(pFCPTPerformance->TotalWriteLatency/writes);
	
	return OK;
}


	
/**********************************************************************
// ReturnTableRefreshData -  Returns the data to be refreshed to the 
// DiskPerformancetable
**********************************************************************/
STATUS FCPTargetPerformanceReporter::ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData)
{
	Tracef("FCPTargetPerformanceReporter::ReturnTableRefreshData()\n");

	// Note that the numbers represent data over the sample period.  Before
	// returning the results, we need to change the unit to second.
	
	U32 scale32;
	I64	scale64;
	
	// The Listen on PHS Table needs a separate record to avoid interference.
	
	// Note:  If sample rate is not a multiple of seconds, we won't be able to return meaningful
	// numbers.  In normal operation, we should be okay as sample rate is going to be a
	// multiple of seconds.  If we want to do subsecond reporting of performance data,
	// we can change the scale to represent milisecond and recompile.
	scale32 = m_FCPTargetPerformanceRecord.SampleRate/1000000; // length of sample rate in seconds.
	scale32 = scale32 ? scale32 : 1; // Protect agaisnt divided by zero
	scale64 = scale32;
	
	m_FCPTargetPerformanceRecord.AvgNumReadPacketsPerSec = m_NumReadPackets.Average()/scale32;
	m_FCPTargetPerformanceRecord.MaxNumReadPacketsPerSec = m_NumReadPackets.Max()/scale32;
	m_FCPTargetPerformanceRecord.MinNumReadPacketsPerSec = m_NumReadPackets.Min()/scale32;
		
	m_FCPTargetPerformanceRecord.AvgNumWritePacketsPerSec = m_NumWritePackets.Average()/scale32;
	m_FCPTargetPerformanceRecord.MaxNumWritePacketsPerSec = m_NumWritePackets.Max()/scale32;
	m_FCPTargetPerformanceRecord.MinNumWritePacketsPerSec = m_NumWritePackets.Min()/scale32;
	
	m_FCPTargetPerformanceRecord.AvgNumRWPacketsPerSec = ( (m_NumReadPackets.Total() + m_NumWritePackets.Total())/
									      				   (m_NumReadPackets.Count() + m_NumWritePackets.Count())  )/
									   					 scale32;
									   					 
	m_FCPTargetPerformanceRecord.MaxNumRWPacketsPerSec = (m_FCPTargetPerformanceRecord.MaxNumReadPacketsPerSec > 
										 				  m_FCPTargetPerformanceRecord.MaxNumWritePacketsPerSec) ?
														  m_FCPTargetPerformanceRecord.MaxNumReadPacketsPerSec :
														  m_FCPTargetPerformanceRecord.MaxNumWritePacketsPerSec;
										
	m_FCPTargetPerformanceRecord.MinNumRWPacketsPerSec = (m_FCPTargetPerformanceRecord.MinNumReadPacketsPerSec < 
										 				  m_FCPTargetPerformanceRecord.MinNumWritePacketsPerSec) ?
														  m_FCPTargetPerformanceRecord.MinNumReadPacketsPerSec :
														  m_FCPTargetPerformanceRecord.MinNumWritePacketsPerSec;
	
	m_FCPTargetPerformanceRecord.AvgNumTotalPacketsPerSec = m_NumTotalPackets.Average()/scale32;
	m_FCPTargetPerformanceRecord.MaxNumTotalPacketsPerSec = m_NumTotalPackets.Max()/scale32;
	m_FCPTargetPerformanceRecord.MinNumTotalPacketsPerSec = m_NumTotalPackets.Min()/scale32;
	
	m_FCPTargetPerformanceRecord.AvgNumBytesReadPerSec = m_NumBytesRead.Average()/scale64;
	m_FCPTargetPerformanceRecord.MaxNumBytesReadPerSec = m_NumBytesRead.Max()/scale64;
	m_FCPTargetPerformanceRecord.MinNumBytesReadPerSec = m_NumBytesRead.Min()/scale64;
	
	m_FCPTargetPerformanceRecord.AvgNumBytesWrittenPerSec = m_NumBytesWritten.Average()/scale64;
	m_FCPTargetPerformanceRecord.MaxNumBytesWrittenPerSec = m_NumBytesWritten.Max()/scale64;
	m_FCPTargetPerformanceRecord.MinNumBytesWrittenPerSec = m_NumBytesWritten.Min()/scale64;
	
	m_FCPTargetPerformanceRecord.AvgNumBytesTranferredPerSec = ( (m_NumBytesRead.Total() + m_NumBytesWritten.Total())/
									      				         (m_NumBytesRead.Count() + m_NumBytesWritten.Count()) )/
									   					       scale64;
	
	m_FCPTargetPerformanceRecord.MaxNumBytesTranferredPerSec = (m_FCPTargetPerformanceRecord.MaxNumBytesReadPerSec > 
										 				   	 	m_FCPTargetPerformanceRecord.MaxNumBytesWrittenPerSec) ?
														  		m_FCPTargetPerformanceRecord.MaxNumBytesReadPerSec :
														  		m_FCPTargetPerformanceRecord.MaxNumBytesWrittenPerSec;
	
	m_FCPTargetPerformanceRecord.MinNumBytesTranferredPerSec = (m_FCPTargetPerformanceRecord.MinNumBytesReadPerSec < 
										 				  		m_FCPTargetPerformanceRecord.MinNumBytesWrittenPerSec) ?
														  		m_FCPTargetPerformanceRecord.MinNumBytesReadPerSec :
														  		m_FCPTargetPerformanceRecord.MinNumBytesWrittenPerSec;
	
	
	
	m_FCPTargetPerformanceRecord.AvgMicroSecPerRead = m_ReadLatency.Average();
	m_FCPTargetPerformanceRecord.MaxMicroSecPerRead = m_ReadLatency.Max();
	m_FCPTargetPerformanceRecord.MinMicroSecPerRead = m_ReadLatency.Min();

	m_FCPTargetPerformanceRecord.AvgMicroSecPerWrite = m_WriteLatency.Average();
	m_FCPTargetPerformanceRecord.MaxMicroSecPerWrite = m_WriteLatency.Max();
	m_FCPTargetPerformanceRecord.MinMicroSecPerWrite = m_WriteLatency.Min();
	
	ResetCounters();
	rpTableData = &m_FCPTargetPerformanceRecord;
	rcbTableData = sizeof(m_FCPTargetPerformanceRecord);
	
	return OK;

}



/**********************************************************************
// ResetCounters -  Clear all counters and other variable to start the
// next Refresh period.  Note:  Base object may also call this method
// before starting the Reporter.  Basically, this method should be safe
// to call anytime before starting a data collection run over a Refresh
// preriod.
/**********************************************************************/
void FCPTargetPerformanceReporter::ResetCounters()
{
	m_NumReadPackets.Reset();
	m_NumWritePackets.Reset();
	m_NumTotalPackets.Reset();
	
	m_NumBytesRead.Reset();
	m_NumBytesWritten.Reset();

	m_ReadLatency.Reset();
	m_WriteLatency.Reset();


	
	return;
}
