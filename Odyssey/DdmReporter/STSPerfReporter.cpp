/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: STSPerfReporter.cpp
// 
// Description:
// This file implements the SCSI Target Server (STS) Performance Reporter class. 
// 
// Update Log 
//
// $Log: /Gemini/Odyssey/DdmReporter/STSPerfReporter.cpp $
// 
// 9     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 8     9/15/99 9:05a Vnguyen
// Update Performance counters to match with the counters being returned
// by the SCSI Target Server DDM.
// 
// 7     8/16/99 1:05p Vnguyen
// Fix verious compiler errors.  Mostly typo and mis-match parameters.
// 
// 6     8/06/99 12:29p Vnguyen
// 
// 5     8/06/99 10:00a Vnguyen
// 
// 5     8/06/99 9:57a Vnguyen -- Change name of rid from ridSSTPerformanceRec
// to ridPerformanceRec.
//
//	4	8/06/99 8:12a Vnguyen -- Miscellaneous comments.
// 
// 3	8/05/99 12:38p Vnguyen -- Add defines for Export Table Field names.
// 		Add method ResetCounters. 
//
// 2  	8/03/99 6:25p Vnguyen -- Cleanup member variables.  Add m_Data to
//      supply Ddm data to SRCandExport object.  Rewrite Initialize method
//		to pass Ddm data to the lower object.
// 
// 1  	8/02/99 3:35p Vnguyen -- Add variables for counters using SampleItem.h
// 
// 	0	7/29/99	3:13p	Vnguyen
//	Created.
/*************************************************************************/

#include "STSPerfReporter.h"
#include "STSPerf.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "DefaultRates.h"

/**********************************************************************/
// Initialize - This is the initializor for the STSPerfReporter Object.
// We've been called because the STS Ddm has intialized and now starts
// the Performance Reporter.  He wants to instantiate (and initialize) 
// a new reporter object for the Export (table) entry.  Our Job, is to 
// make sure that our performance table and record are created and if not, 
// create them.  Note:  All the work is done by the lower class object.
// All we need to do here is to supply Ddm specific info, e.g. table names,
// a buffer to hold the row record, etc.
/**********************************************************************/
STATUS STSPerfReporter::Initialize(
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

	// Initialize the DataBlock structure with Export (Table) data before handing off
	strcpy(m_Data.SRCandExportTableName, EXPORT_TABLE);
	
	m_Data.pSRCandExportRecord = &m_ExportRecord;
	m_Data.cbSRCandExportRecordMax = sizeof(ExportTableRecord);
	
	strcpy(m_Data.SRCandExportvdnName, fdEXPORT_VDN); // Actual name of vdnDdm field in SRC/Export table
	
	strcpy(m_Data.SRCandExportPHSRowIDName, fdEXPORT_PERFORMANCE_RID);	// String64 rgbFieldName

	m_Data.pSRCandExportRowID = &m_ExportRecord.rid;
	m_Data.pPHSRowID = &m_ExportRecord.ridPerformanceRec;

	strcpy(m_Data.PHSTableName, STS_PERFORMANCE_TABLE);		 // Name of table to send PHS data to
	m_Data.aPHSTableFieldDefs = (fieldDef *)aSTSPerfTable_FieldDefs;	// FieldDefs for PHS table 
	m_Data.cbPHSTableFieldDefs = cbSTSPerfTable_FieldDefs;	// size of PHS table fieldDefs, U32 cbrgFieldDefs

	m_Data.pPHSRecord = &m_STSPerfRec;				// A pointer to a buffer for a PHS row record
	m_Data.cbPHSRecord = sizeof(STSPerfRecord);		// Size of a PHS row record, cbRowData
	m_Data.cbPHSRecordMax = sizeof(STSPerfRecord); 	// Maximum size of buffer to receive row data, cbRowDataRetMax

	m_Data.pRefreshRate = &m_STSPerfRec.RefreshRate;
	m_Data.pSampleRate = &m_STSPerfRec.SampleRate;

	ResetCounters();	// Not really needed as the base object should call ResetCounters before
						// starting the data collection.

	// Up up and away...
	status = InitSRCandExport(&m_Data, didDdm, vdnDdm, ReporterCode);
	return status;
 }


/**********************************************************************/
// InitializePHSRecord - This method fills in the PHS table record with 
// default values.  The RowID for the Export Table entry is also
// put in the PHS record.  
/**********************************************************************/
void STSPerfReporter::InitializePHSRecord(
 		void *pPHSRecord, 
 		rowID *pSRCandExportRowID
 )
 {
 	STSPerfRecord	*pSTSPerfRec;
 	pSTSPerfRec = (STSPerfRecord *) pPHSRecord;

	// Declare and initialize a STS Performance Record with default values. 
	STSPerfRecord	newSTSPerfRecord = {
	// 	Default Value						FieldName									  Size   Type
		0,0,0,							// "rid"										8,	I64_FT,
		CT_STSPT_TABLE_VERSION,			//	version: Version of DiskPerformanceTable record.
		sizeof(STSPerfRecord),			//	size: # of bytes in record
		0,								//	key: Currently unused.
		STS_PERFORMANCE_REFRESH_RATE,	//	RefreshRate 
		STS_PERFORMANCE_SAMPLE_RATE,	//	SampleRate: rate at which we sample DDMs data.
		0,0,0,							//	ridExportRecord: Row ID of Export entry 4 this device.
		0,								//  AvgNumBSAReadsPerSec
		0,								//  MaxNumBSAReadsPerSec;
		0,								//  MinNumBSAReadsPerSec;
	
		0,								//  AvgNumBSAWritesPerSec;
		0,								//  MaxNumBSAWritesPerSec;
		0,								//  MinNumBSAWritesPerSec;
	
		0,								//  AvgNumBSACmdsPerSec;
		0,								//  MaxNumBSACmdsPerSec;
		0,								//  MinNumBSACmdsPerSec;
	
		0,								//  AvgNumSCSICmdsPerSec;
		0,								//  MaxNumSCSICmdsPerSec;
		0,								//  MinNumSCSICmdsPerSec;
	
		0,								//  AvgNumBSABytesReadPerSec;
		0,								//  MaxNumBSABytesReadPerSec;
		0,								//  MinNumBSABytesReadPerSec;

		0,								//  AvgNumBSABytesWrittenPerSec;
		0,								//  MaxNumBSABytesWrittenPerSec;
		0,								//  MinNumBSABytesWrittenPerSec;

		0,								//  AvgNumBSABytesPerSec;	
		0,								//  MaxNumBSABytesPerSec;
		0								//  MinNumBSABytesPerSec;
		
	};	// end of default STS performance record initializor.
		
	// We make a copy to intialize and return a PHS row record with default values.
	*pSTSPerfRec = newSTSPerfRecord;

 	pSTSPerfRec->ridExportRecord = *pSRCandExportRowID;
 	return;	
 }
 

/**********************************************************************/
// HandleDdmSampleData -  Handle the sample perofrmance data returned
// from the Ddm by performing 
/**********************************************************************/
STATUS STSPerfReporter::HandleDdmSampleData( void* pDdmData, U32 cbDdmData )
{
#pragma unused(cbDdmData)
STS_Performance		*pSTSPerformance;
STATUS					status = OK;

	Tracef("STSPerfReporter::HandleDdmSampleData()\n");
	// Get a pointer to the reply payload and cast it to our reply structure.
	pSTSPerformance = (STS_Performance*)pDdmData;
	
	// Calculate our own running statistics.
	m_NumBSAReads.Sample(pSTSPerformance->NumBSAReads);
	m_NumBSAWrites.Sample(pSTSPerformance->NumBSAWrites);
	m_NumSCSICmds.Sample(pSTSPerformance->NumSCSICmds);

	m_NumBSABytesRead.Sample(pSTSPerformance->NumBSABytesRead);
	m_NumBSABytesWritten.Sample(pSTSPerformance->NumBSABytesWritten);
	
	return OK;
}


	
/**********************************************************************
// ReturnTableRefreshData -  Returns the data to be refreshed to the 
// DiskPerformancetable
**********************************************************************/
STATUS STSPerfReporter::ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData)
{
	U32 scale32;
	I64	scale64;
	
	Tracef("STSPerformanceReporter::ReturnTableRefreshData()\n");

	// Note that the numbers represent data over the sample period.  Before
	// returning the results, we need to change the unit to second.
	scale32 = m_STSPerfRec.SampleRate/1000000; // length of sample rate in seconds.
	scale32 = scale32 ? scale32 : 1; // Protect agaisnt divided by zero
	scale64 = scale32;
	
	
	// The Listen on PHS Table needs a separate record to avoid interference.


	m_STSPerfRec.AvgNumBSAReadsPerSec = m_NumBSAReads.Average()/scale32;
	m_STSPerfRec.MaxNumBSAReadsPerSec = m_NumBSAReads.Max()/scale32;
	m_STSPerfRec.MinNumBSAReadsPerSec = m_NumBSAReads.Min()/scale32;
	
	m_STSPerfRec.AvgNumBSAWritesPerSec = m_NumBSAWrites.Average()/scale32;
	m_STSPerfRec.MaxNumBSAWritesPerSec = m_NumBSAWrites.Max()/scale32;
	m_STSPerfRec.MinNumBSAWritesPerSec = m_NumBSAWrites.Min()/scale32;
	
	m_STSPerfRec.AvgNumBSACmdsPerSec = ( (m_NumBSAReads.Total() + m_NumBSAWrites.Total())/
									     (m_NumBSAReads.Count() + m_NumBSAWrites.Count())  )/
									   scale32;
									   
	m_STSPerfRec.MaxNumBSACmdsPerSec = (m_STSPerfRec.MaxNumBSAReadsPerSec > m_STSPerfRec.MaxNumBSAWritesPerSec) ?
										m_STSPerfRec.MaxNumBSAReadsPerSec :
										m_STSPerfRec.MaxNumBSAWritesPerSec;
										
	m_STSPerfRec.MinNumBSACmdsPerSec = (m_STSPerfRec.MinNumBSAReadsPerSec < m_STSPerfRec.MinNumBSAWritesPerSec) ?
										m_STSPerfRec.MinNumBSAReadsPerSec :
										m_STSPerfRec.MinNumBSAWritesPerSec;
	
	m_STSPerfRec.AvgNumSCSICmdsPerSec = m_NumSCSICmds.Average()/scale32;
	m_STSPerfRec.MaxNumSCSICmdsPerSec = m_NumSCSICmds.Max()/scale32;
	m_STSPerfRec.MinNumSCSICmdsPerSec = m_NumSCSICmds.Min()/scale32;
	
	// These are I64 numbers
	m_STSPerfRec.AvgNumBSABytesReadPerSec = m_NumBSABytesRead.Average()/scale64;
	m_STSPerfRec.MaxNumBSABytesReadPerSec = m_NumBSABytesRead.Max()/scale64;
	m_STSPerfRec.MinNumBSABytesReadPerSec = m_NumBSABytesRead.Min()/scale64;

	m_STSPerfRec.AvgNumBSABytesWrittenPerSec = m_NumBSABytesWritten.Average()/scale64;
	m_STSPerfRec.MaxNumBSABytesWrittenPerSec = m_NumBSABytesWritten.Max()/scale64;
	m_STSPerfRec.MinNumBSABytesWrittenPerSec = m_NumBSABytesWritten.Min()/scale64;

	m_STSPerfRec.AvgNumBSABytesPerSec = ( (m_NumBSABytesRead.Total() + m_NumBSABytesWritten.Total())/
									      (m_NumBSABytesRead.Count() + m_NumBSABytesWritten.Count())  )/
									   	scale64;
	
	m_STSPerfRec.MaxNumBSABytesPerSec = (m_STSPerfRec.MaxNumBSABytesReadPerSec > 
										 m_STSPerfRec.MaxNumBSABytesWrittenPerSec) ?
										m_STSPerfRec.MaxNumBSABytesReadPerSec :
										m_STSPerfRec.MaxNumBSABytesWrittenPerSec;
	
	m_STSPerfRec.MinNumBSABytesPerSec = (m_STSPerfRec.MinNumBSABytesReadPerSec < 
										 m_STSPerfRec.MinNumBSABytesWrittenPerSec) ?
										m_STSPerfRec.MinNumBSABytesReadPerSec :
										m_STSPerfRec.MinNumBSABytesWrittenPerSec;

	// Don't forget to listen on the Table so the Hi and Low thresholds are updated in the
	// local PTS record.

	ResetCounters();
	rpTableData = &m_STSPerfRec;
	rcbTableData = sizeof(m_STSPerfRec);
	
	return OK;

}



/**********************************************************************
// ResetCounters -  Clear all counters and other variable to start the
// next Refresh period.  Note:  Base object may also call this method
// before starting the Reporter.  Basically, this method should be safe
// to call anytime before starting a data collection run over a Refresh
// preriod.
/**********************************************************************/
void STSPerfReporter::ResetCounters()
{

	m_NumBSAReads.Reset();
	m_NumBSAWrites.Reset();
	m_NumSCSICmds.Reset();

	m_NumBSABytesRead.Reset();
	m_NumBSABytesWritten.Reset();


	return;
}
