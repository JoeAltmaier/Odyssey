/**********************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SSDPerformanceReporter.cpp
// 
// Description:
// This file implements the SSDPerformanceReporter Reporter class. 
// 
// Update Log 
//
// $Log: /Gemini/Odyssey/DdmReporter/SSDPerformanceReporter.cpp $
// 
// 3     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 2     10/08/99 9:16a Vnguyen
// Update Reporter to cover additional performance and status counters.
// 
// 1     8/20/99 10:39a Vnguyen
// Initial check-in.
// 
/**********************************************************************/
#include "SSDPerformanceReporter.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "SSDPerformance.h"
#include "DefaultRates.h"


/**********************************************************************/
// Initialize - This is the initializor for the SSD Reporter Object.
// We've been called because the SSD Ddm has intialized and now starts
// the Performance Reporter.  He wants to instantiate (and initialize) 
// a new reporter object for the Storage Roll Call table entry.  Our Job is to 
// make sure that our performance table and record are created and if not, 
// create them.  Note:  All the work is done by the lower class object.
// All we need to do here is to supply Ddm specific info, e.g. table names,
// a buffer to hold the row record, etc.
/**********************************************************************/
STATUS SSDPerformanceReporter::Initialize(
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
	strcpy(m_Data.PHSTableName, SSD_PERFORMANCE_TABLE); 	// Name of table to send PHS data to
	m_Data.aPHSTableFieldDefs = (fieldDef *)aSSDPerformanceTable_FieldDefs;	// FieldDefs for PHS table 
	m_Data.cbPHSTableFieldDefs = cbSSDPerformanceTable_FieldDefs;	// size of PHS table fieldDefs, U32 cbrgFieldDefs

	m_Data.pPHSRecord = &m_SSDPerformanceRecord;			// A pointer to a buffer for a PHS row record
	m_Data.cbPHSRecord = sizeof(m_SSDPerformanceRecord); 	// Size of a PHS row record, cbRowData
	m_Data.cbPHSRecordMax = sizeof(m_SSDPerformanceRecord); // Maximum size of buffer to receive row data, cbRowDataRetMax

	m_Data.pRefreshRate = &m_SSDPerformanceRecord.RefreshRate;
	m_Data.pSampleRate = &m_SSDPerformanceRecord.SampleRate;

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
void SSDPerformanceReporter::InitializePHSRecord(
 		void *pPHSRecord, 
 		rowID *pSRCandExportRowID
 )
 {
 	SSDPerformanceRecord	*pSSDPerformanceRecord;
 	pSSDPerformanceRecord = (SSDPerformanceRecord *) pPHSRecord;

	// Declare and initialize a SSD Performance Record with default values. 
	SSDPerformanceRecord	newSSDPerformanceRecord = {
		// 	Default Value						FieldName							Size   Type
			0, 0, 0,						// "rid"								8,	ROWID_FT,
			CT_SSDPT_TABLE_VERSION,			//	version: Version of SSDPerformanceTable record.
			sizeof(SSDPerformanceRecord),	//	size: # of bytes in record
			0,								//	key: Currently unused.
			SSD_PERFORMANCE_REFRESH_RATE,	//	
			SSD_PERFORMANCE_SAMPLE_RATE,	//	
			0, 0, 0,						//	ridSRCRecord: Row ID of  Storage Roll Call  entry 4 this device.
			0,								// 	UpTime: Number of microseconds this disk drive has been spun-up.

			0,								//	AvgNumPagesReadPerSec;
			0,								//	MaxNumPagesReadPerSec;
			0,								//	MinNumPagesReadPerSec;

			0,								//	AvgNumPagesReadCacheHitPerSec;
			0,								//	MaxNumPagesReadCacheHitPerSec;
			0,								//	MinNumPagesReadCacheHitPerSec;

			0,								//	AvgNumPagesReadCacheMissPerSec;
			0,								//	MaxNumPagesReadCacheMissPerSec;
			0,								//	MinNumPagesReadCacheMissPerSec;
	
			0,								//	AvgNumPagesWritePerSec;
			0,								//	MaxNumPagesWritePerSec;
			0,								//	MinNumPagesWritePerSec;
	
			0,								//	AvgNumPagesWriteCacheHitPerSec;
			0,								//	MaxNumPagesWriteCacheHitPerSec;
			0,								//	MinNumPagesWriteCacheHitPerSec;

			0,								//	AvgNumPagesWriteCacheMissPerSec;
			0,								//	MaxNumPagesWriteCacheMissPerSec;
			0,								//	MinNumPagesWriteCacheMissPerSec;
	
			0,								//	AvgNumErasePagesAvailablePerSec;	
			0,								//	MaxNumErasePagesAvailablePerSec;		
			0,								//	MinNumErasePagesAvailablePerSec;		

			0,								//	AvgNumReadBytesTotalPerSec;
			0,								//	MaxNumReadBytesTotalPerSec;		
			0,								//	MinNumReadBytesTotalPerSec;		

			0,								//	AvgNumWriteBytesTotalPerSec;
			0,								//	MaxNumWriteBytesTotalPerSec;		
			0								//	MinNumWriteBytesTotalPerSec;		

		};	// end of default SSD performance record initializor.
		
	// We make a copy to intialize and return a PHS row record with default values.
	*pSSDPerformanceRecord = newSSDPerformanceRecord;

 	pSSDPerformanceRecord->ridSRCRecord = *pSRCandExportRowID;
 	return;	
 }


/**********************************************************************/
// HandleDdmSampleData -  Handle the sample perofrmance data returned
// from the Ddm by performing 
/**********************************************************************/
STATUS SSDPerformanceReporter::HandleDdmSampleData( void* pDdmData, U32 cbDdmData )
{
#pragma unused(cbDdmData)
SSD_PERFORMANCE		*pSSDPerformance;
STATUS				status = OK;

	Tracef("SSDPerformanceReporter::HandleDdmSampleData()\n");
	// Get a pointer to the reply payload and cast it to our reply structure.
	pSSDPerformance = (SSD_PERFORMANCE*) pDdmData;
	



	// Calculate our own running statistics.

	m_NumPagesRead.Sample(pSSDPerformance->NumPagesRead);
	m_NumPagesReadCacheHit.Sample(pSSDPerformance->NumPagesReadCacheHit);
	m_NumPagesReadCacheMiss.Sample(pSSDPerformance->NumPagesReadCacheMiss);

	m_NumPagesWrite.Sample(pSSDPerformance->NumPagesWrite);
	m_NumPagesWriteCacheHit.Sample(pSSDPerformance->NumPagesWriteCacheHit);
	m_NumPagesWriteCacheMiss.Sample(pSSDPerformance->NumPagesWriteCacheMiss);

	m_NumErasePagesAvailable.Sample(pSSDPerformance->NumErasePagesAvailable);

	m_NumReadBytesTotal.Sample(pSSDPerformance->NumReadBytesTotal);
	m_NumWriteBytesTotal.Sample(pSSDPerformance->NumWriteBytesTotal);

	return OK;
}


	
/**********************************************************************
// ReturnTableRefreshData -  Returns the data to be refreshed to the 
// PHS Reporter SSDPerformancetable
**********************************************************************/
STATUS SSDPerformanceReporter::ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData)
{
	Tracef("SSDPerformanceReporter::ReturnTableRefreshData()\n");

	U32 scale32;
	I64	scale64;


	// The Listen on PHS Table needs a separate record to avoid interference.
	
	// Note:  If sample rate is less than 1 second, we won't be able to return meaningful
	// numbers.  In normal operation, we should be okay as sample rate is going to be a
	// multiple of seconds.  If we want to do subsecond reporting of performance data,
	// we can change the scale to represent milisecond and recompile.
	scale32 = m_SSDPerformanceRecord.SampleRate/1000000; // length of sample rate in seconds.
	scale32 = scale32 ? scale32 : 1; // Protect agaisnt divided by zero
	scale64 = scale32;
			
	m_SSDPerformanceRecord.AvgNumPagesReadPerSec = m_NumPagesRead.Average()/scale32;
	m_SSDPerformanceRecord.MaxNumPagesReadPerSec = m_NumPagesRead.Max()/scale32;
	m_SSDPerformanceRecord.MinNumPagesReadPerSec = m_NumPagesRead.Min()/scale32;

	m_SSDPerformanceRecord.AvgNumPagesReadCacheHitPerSec = m_NumPagesReadCacheHit.Average()/scale32;
	m_SSDPerformanceRecord.MaxNumPagesReadCacheHitPerSec = m_NumPagesReadCacheHit.Max()/scale32;
	m_SSDPerformanceRecord.MinNumPagesReadCacheHitPerSec = m_NumPagesReadCacheHit.Min()/scale32;

	m_SSDPerformanceRecord.AvgNumPagesReadCacheMissPerSec = m_NumPagesReadCacheMiss.Average()/scale32;
	m_SSDPerformanceRecord.MaxNumPagesReadCacheMissPerSec = m_NumPagesReadCacheMiss.Max()/scale32;
	m_SSDPerformanceRecord.MinNumPagesReadCacheMissPerSec = m_NumPagesReadCacheMiss.Min()/scale32;
	
	m_SSDPerformanceRecord.AvgNumPagesWritePerSec = m_NumPagesWrite.Average()/scale32;
	m_SSDPerformanceRecord.MaxNumPagesWritePerSec = m_NumPagesWrite.Max()/scale32;
	m_SSDPerformanceRecord.MinNumPagesWritePerSec = m_NumPagesWrite.Min()/scale32;
	
	m_SSDPerformanceRecord.AvgNumPagesWriteCacheHitPerSec = m_NumPagesWriteCacheHit.Average()/scale32;
	m_SSDPerformanceRecord.MaxNumPagesWriteCacheHitPerSec = m_NumPagesWriteCacheHit.Max()/scale32;
	m_SSDPerformanceRecord.MinNumPagesWriteCacheHitPerSec = m_NumPagesWriteCacheHit.Min()/scale32;

	m_SSDPerformanceRecord.AvgNumPagesWriteCacheMissPerSec = m_NumPagesWriteCacheMiss.Average()/scale32;
	m_SSDPerformanceRecord.MaxNumPagesWriteCacheMissPerSec = m_NumPagesWriteCacheMiss.Max()/scale32;
	m_SSDPerformanceRecord.MinNumPagesWriteCacheMissPerSec = m_NumPagesWriteCacheMiss.Min()/scale32;
	
	// NumErasePagesAvailable is special.  Also requires special handling from SSD Ddm.  Note:  Do not scale.
	// It is a status type counter.  But we collect it here to get more frequent data.
	m_SSDPerformanceRecord.AvgNumErasePagesAvailable = m_NumErasePagesAvailable.Average();		
	m_SSDPerformanceRecord.MaxNumErasePagesAvailable = m_NumErasePagesAvailable.Max();		
	m_SSDPerformanceRecord.MinNumErasePagesAvailable = m_NumErasePagesAvailable.Min();		

	m_SSDPerformanceRecord.AvgNumReadBytesTotalPerSec = m_NumReadBytesTotal.Average()/scale64;
	m_SSDPerformanceRecord.MaxNumReadBytesTotalPerSec = m_NumReadBytesTotal.Max()/scale64;
	m_SSDPerformanceRecord.MinNumReadBytesTotalPerSec = m_NumReadBytesTotal.Min()/scale64;
	
	m_SSDPerformanceRecord.AvgNumWriteBytesTotalPerSec = m_NumWriteBytesTotal.Average()/scale64;
	m_SSDPerformanceRecord.MaxNumWriteBytesTotalPerSec = m_NumWriteBytesTotal.Max()/scale64;
	m_SSDPerformanceRecord.MinNumWriteBytesTotalPerSec = m_NumWriteBytesTotal.Min()/scale64;

	// Return the data to the base class.
	rpTableData = &m_SSDPerformanceRecord;
	rcbTableData = sizeof(m_SSDPerformanceRecord);
	
	return OK;

}


/**********************************************************************
// ResetCounters -  Clear all counters and other variable to start the
// next Refresh period.  Note:  Base object may also call this method
// before starting the Reporter.  Basically, this method should be safe
// to call anytime before starting a data collection run over a Refresh
// preriod.
/**********************************************************************/
void SSDPerformanceReporter::ResetCounters()
{
	
	// Initialize
	m_NumPagesRead.Reset();
	m_NumPagesReadCacheHit.Reset();
	m_NumPagesReadCacheMiss.Reset();

	m_NumPagesWrite.Reset();
	m_NumPagesWriteCacheHit.Reset();
	m_NumPagesWriteCacheMiss.Reset();

	m_NumErasePagesAvailable.Reset();
	
	m_NumReadBytesTotal.Reset();
	m_NumWriteBytesTotal.Reset();
	
	return;
}