/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DiskPerformanceReporter.cpp
// 
// Description:
// This file implements the DiskPerformanceReporter Reporter class. 
// 
// Update Log 
//
// $Log: /Gemini/Odyssey/DdmReporter/DiskPerformanceReporter.cpp $
// 
// 14    1/04/00 4:31p Jlane
// Initialize new pad fields just to be well-behaved.
// 
// 13    12/15/99 2:04p Vnguyen
// Move Tracef.  Also increment key counter for each update.  This way we
// can tell when an update just happens and see if the info is propogated
// to GMI GUI or not.
// 
// 12    12/14/99 10:24a Vnguyen
// Recalculate transfer (read + write) statistics.  The new method takes
// more time but is more accurate.
// 
// 11    12/10/99 8:55a Vnguyen
// Comment out Tracef lines so the NAC display won't get cluttered up.
// 
// 10    12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 9     9/09/99 11:23a Vnguyen
// Update Disk Performance and Status Counters
// 
// 8     8/16/99 12:59p Vnguyen
// Fix various compiler errors.  Mostly typo and mis-match parameters.
// 
// 7     8/05/99 11:04a Vnguyen
// 
// 6     8/04/99 6:25p Vnguyen -- Major change to reuse the object
// SRCandExport.  Now this DiskPerf obj only initializes the Ddm specific
// data and supplies the routine to handle sample/refresh events.  
// The SRCandExport will do the rest.
//
// 5     5/19/99 9:02a Jlane
// Miscellaneous changes and bug fixes made during bringup.
// 
// 4     5/13/99 11:38a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 3     5/05/99 10:02a Jlane
// Pass pb/cb data in Handle DdmSampleData instead of pMsg.  make DdmData
// an Sgl item instead of payload.
//
// 11/16/98	JFL	Created.  A gray Monday.
//
/*************************************************************************/

#include "DiskPerformanceReporter.h"
#include "BsaPerformance.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "RqDdmReporter.h"
#include "DefaultRates.h"

/**********************************************************************/
// Initialize - This is the initializor for the DiskPerformanceReporter Object.
// We've been called because the PHSReporter Ddm has received a reply to
// a listen operation he posted on the Storage Roll Call table and now 
// he (the PHS Ddm) wants to instantiate (and initialize) a new reporter
// object for the SRC entry.  Our Job, as the is to make sure that our
// performance table and record are created and if not, create them.    
/**********************************************************************/
STATUS DiskPerformanceReporter::Initialize(
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
	
	// Initialize the DataBlock structure with SRC data before handing off
	strcpy(m_Data.SRCandExportTableName, STORAGE_ROLL_CALL_TABLE);
	m_Data.pSRCandExportRecord = &m_SRCRecord;
	m_Data.cbSRCandExportRecordMax = sizeof(StorageRollCallRecord);
	strcpy(m_Data.SRCandExportvdnName, fdSRC_VDNBSADDM); // Actual name of vdnDdm field in SRC/Export table
	strcpy(m_Data.SRCandExportPHSRowIDName, fdSRC_PERFORMANCE_RID);	// String64 rgbFieldName

	m_Data.pSRCandExportRowID = &m_SRCRecord.rid;
	m_Data.pPHSRowID = &m_SRCRecord.ridPerformanceRecord;

	strcpy(m_Data.PHSTableName, DISK_PERFORMANCE_TABLE);		 	// Name of table to send PHS data to
	m_Data.aPHSTableFieldDefs = (fieldDef *)aDiskPerformanceTable_FieldDefs;	// FieldDefs for PHS table 
	m_Data.cbPHSTableFieldDefs = cbDiskPerformanceTable_FieldDefs;	// size of PHS table fieldDefs, U32 cbrgFieldDefs

	m_Data.pPHSRecord = &m_DiskPerfRec;					// A pointer to a buffer for a PHS row record
	m_Data.cbPHSRecord = sizeof(DiskPerformanceRecord);	// Size of a PHS row record, cbRowData
	m_Data.cbPHSRecordMax = sizeof(DiskPerformanceRecord); // Maximum size of buffer to receive row data, cbRowDataRetMax

	m_Data.pRefreshRate = &m_DiskPerfRec.RefreshRate;
	m_Data.pSampleRate = &m_DiskPerfRec.SampleRate;

	ResetCounters();  // Not really needed to call here as the base object will issue a call to 
					  // ResetCounter just before the data collection begins.

	// Up up and away...
	status = InitSRCandExport(&m_Data, didDdm, vdnDdm, ReporterCode);
	return status;
 }
 
 
 
/**********************************************************************/
// InitializePHSRecord - This method fills in the PHS record with default
// values.  The RowID for the Storage Roll Call Table entry is also
// put in here.  
/**********************************************************************/
void DiskPerformanceReporter::InitializePHSRecord(
 		void *pPHSRecord, 
 		rowID *pSRCandExportRowID
 )
 {
 	DiskPerformanceRecord	*pDiskPerfRec;
 	pDiskPerfRec = (DiskPerformanceRecord *) pPHSRecord;

	// Declare and initialize a Disk Status Record with default values. 
	DiskPerformanceRecord	newDiskPerformanceRecord = {
	// 	Default Value						FieldName									  Size   Type
		0,0,0,							// "rid"										8,	I64_FT,
		CT_DPT_TABLE_VERSION,			//	version: Version of DiskPerformanceTable record.
		sizeof(DiskPerformanceRecord),	//	size: # of bytes in record
		0,								//	key: Currently unused.
		DISK_PERFORMANCE_REFRESH_RATE,	//	RefreshRate 
		DISK_PERFORMANCE_SAMPLE_RATE,	//	SampleRate: rate at which we sample DDMs data.
		0,			//U32Pad1
		0,0,0,							//	ridSRCRecord: Row ID of  Storage Roll Call  entry 4 this device.
		0,								// 	UpTime: Number of milliseconds this disk drive has been spun-up.
		0,								//  AvgReadsPerSec;
		0,								//  MaxReadsPerSec;
		0,								//  MinReadsPerSec;
	
		0,								//  AvgWritesPerSec;
		0,								//  MaxWritesPerSec;
		0,								//  MinWritesPerSec;
	
		0,								//  AvgTransferPerSec;
		0,								//  MaxTransferPerSec;
		0,								//  MinTransferPerSec;
		0,			//U32Pad2
//
		0,								//  AvgBytesReadPerSec;
		0,								//  MaxBytesReadPerSec;	
		0,								//  MinBytesReadPerSec;

		0,								//  AvgBytesWrittenPerSec;
		0,								//  MaxBytesWrittenPerSec;
		0,								//  MinBytesWrittenPerSec;

		0,								//  AvgBytesTransferredPerSec;
		0,								//  MaxBytesTransferredPerSec;
		0,								//  MinBytesTransferredPerSec;
//
		0,								//  AvgReadSize;
		0,								//  MaxReadSize;
		0,								//  MinReadSize;

		0,								//  AvgWriteSize;
		0,								//  MaxWriteSize;
		0,								//  MinWriteSize;

		0,								//  AvgTransferSize;
		0,								//  MaxTransferSize;
		0,								//  MinTransferSize;
		0,			//U32Pad3
//
		0,								//  AvgMicroSecPerRead;
		0,								//  MaxMicroSecPerRead;
		0,								//  MinMicroSecPerRead;

		0,								//  AvgMicroSecPerWrite;
		0,								//  MaxMicroSecPerWrite;
		0,								//  MinMicroSecPerWrite;

		0,								//  AvgMicroSecPerTransfer;
		0,								//  MaxMicroSecPerTransfer;
		0								//  MinMicroSecPerTransfer;

	};	// end of default disk performance record initializor.
		

	*pDiskPerfRec = newDiskPerformanceRecord;
 	pDiskPerfRec->ridSRCRecord = *pSRCandExportRowID;
 	return;	
 }
 


/**********************************************************************/
// HandleDdmSampleData -  Handle the sample perofrmance data returned
// from the Ddm by performing 
/**********************************************************************/
STATUS DiskPerformanceReporter::HandleDdmSampleData( void* pDdmData, U32 cbDdmData )
{
#pragma unused(cbDdmData)

	BSA_ISMPerformance		*pBSA_ISMPerformance;
	STATUS					status = OK;
	U32 reads, writes;

//	Tracef("DiskPerformanceReporter::HandleDdmSampleData()\n");
	// Get a pointer to the reply payload and cast it to our reply structure.
	pBSA_ISMPerformance = (BSA_ISMPerformance*)pDdmData;
	
	// Calculate our own running statistics.
	
	m_NumReads.Sample(pBSA_ISMPerformance->num_reads);
	m_NumWrites.Sample(pBSA_ISMPerformance->num_writes);
	m_NumTransfers.Sample(pBSA_ISMPerformance->num_reads +
	                       pBSA_ISMPerformance->num_writes);
	m_NumOtherRequests.Sample(pBSA_ISMPerformance->num_other_requests);

	m_NumBytesRead.Sample(pBSA_ISMPerformance->num_bytes_read);
	m_NumBytesWritten.Sample(pBSA_ISMPerformance->num_bytes_written);
	m_NumBytesTransferred.Sample(pBSA_ISMPerformance->num_bytes_read +
	                             pBSA_ISMPerformance->num_bytes_written);

	// check for zero before doing division.  If zero, then use 1.
	reads = pBSA_ISMPerformance->num_reads ? pBSA_ISMPerformance->num_reads : 1;
	writes = pBSA_ISMPerformance->num_writes ? pBSA_ISMPerformance->num_writes : 1;

	m_ReadLatency.Sample(pBSA_ISMPerformance->total_read_latency/reads);
	m_WriteLatency.Sample(pBSA_ISMPerformance->total_write_latency/writes);
	m_TransferLatency.Sample((pBSA_ISMPerformance->total_read_latency + 
                              pBSA_ISMPerformance->total_write_latency) /
                              (reads + writes));
	m_ReadSize.Sample(pBSA_ISMPerformance->num_bytes_read/reads);
	m_WriteSize.Sample(pBSA_ISMPerformance->num_bytes_written/writes);
	m_TransferSize.Sample((pBSA_ISMPerformance->num_bytes_read +
	                       pBSA_ISMPerformance->num_bytes_written) / 
	                       (reads + writes));
	return OK;
}


	
/**********************************************************************
// ReturnTableRefreshData -  Returns the data to be refreshed to the 
// DiskPerformancetable
**********************************************************************/
STATUS DiskPerformanceReporter::ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData)
{

	U32 scale32;
	I64	scale64;

//	Tracef("DiskPerformanceReporter::ReturnTableRefreshData()\n");
	
	scale32 = m_DiskPerfRec.SampleRate/1000000; // length of sample rate in seconds.
	scale32 = scale32 ? scale32 : 1; // Protect agaisnt divided by zero
	scale64 = scale32;
		
// Disk Performance Data - (one row for each installed disk drive)
//

	// Calculate average number of reads per sample period.
	m_DiskPerfRec.AvgReadsPerSec = m_NumReads.Average()/scale32;
	m_DiskPerfRec.MaxReadsPerSec = m_NumReads.Max()/scale32;
	m_DiskPerfRec.MinReadsPerSec = m_NumReads.Min()/scale32;
	
	m_DiskPerfRec.AvgWritesPerSec = m_NumWrites.Average()/scale32;
	m_DiskPerfRec.MaxWritesPerSec = m_NumWrites.Max()/scale32;
	m_DiskPerfRec.MinWritesPerSec = m_NumWrites.Min()/scale32;
	
	m_DiskPerfRec.AvgTransferPerSec = m_NumTransfers.Average()/scale32;
	m_DiskPerfRec.MaxTransferPerSec = m_NumTransfers.Max()/scale32;
	m_DiskPerfRec.MinTransferPerSec = m_NumTransfers.Max()/scale32;
//
	m_DiskPerfRec.AvgBytesReadPerSec = m_NumBytesRead.Average()/scale64;
	m_DiskPerfRec.MaxBytesReadPerSec = m_NumBytesRead.Max()/scale64;
	m_DiskPerfRec.MinBytesReadPerSec = m_NumBytesRead.Min()/scale64;

	m_DiskPerfRec.AvgBytesWrittenPerSec = m_NumBytesWritten.Average()/scale64;
	m_DiskPerfRec.MaxBytesWrittenPerSec = m_NumBytesWritten.Max()/scale64;
	m_DiskPerfRec.MinBytesWrittenPerSec = m_NumBytesWritten.Min()/scale64;

	m_DiskPerfRec.AvgBytesTransferredPerSec = m_NumBytesTransferred.Average()/scale64;
	m_DiskPerfRec.MaxBytesTransferredPerSec = m_NumBytesTransferred.Max()/scale64;
	m_DiskPerfRec.MinBytesTransferredPerSec = m_NumBytesTransferred.Min()/scale64;
//
	m_DiskPerfRec.AvgReadSize = m_ReadSize.Average();
	m_DiskPerfRec.MaxReadSize = m_ReadSize.Max();
	m_DiskPerfRec.MinReadSize = m_ReadSize.Min();

	m_DiskPerfRec.AvgWriteSize = m_WriteSize.Average();
	m_DiskPerfRec.MaxWriteSize = m_WriteSize.Max();
	m_DiskPerfRec.MinWriteSize = m_WriteSize.Min();

	m_DiskPerfRec.AvgTransferSize = m_TransferSize.Average();
	m_DiskPerfRec.MaxTransferSize = m_TransferSize.Max();
	m_DiskPerfRec.MinTransferSize = m_TransferSize.Min();
//
	m_DiskPerfRec.AvgMicroSecPerRead = m_ReadLatency.Average();
	m_DiskPerfRec.MaxMicroSecPerRead = m_ReadLatency.Max();
	m_DiskPerfRec.MinMicroSecPerRead = m_ReadLatency.Min();

	m_DiskPerfRec.AvgMicroSecPerWrite = m_WriteLatency.Average();
	m_DiskPerfRec.MaxMicroSecPerWrite = m_WriteLatency.Max();
	m_DiskPerfRec.MinMicroSecPerWrite = m_WriteLatency.Min();

	m_DiskPerfRec.AvgMicroSecPerTransfer = m_TransferLatency.Average();
	m_DiskPerfRec.MaxMicroSecPerTransfer = m_TransferLatency.Max();
	m_DiskPerfRec.MinMicroSecPerTransfer = m_TransferLatency.Min();



	// reset all performance counters for next run
	ResetCounters();
	
	m_DiskPerfRec.key++;
	rpTableData = &m_DiskPerfRec;
	rcbTableData = sizeof(m_DiskPerfRec);
	
	return OK;

}


/**********************************************************************/
// ResetCounters -  Reset all counters for a new data colection.  Need
// to do this once after every Refresh.
/**********************************************************************/

void DiskPerformanceReporter::ResetCounters()
{


	m_NumReads.Reset();
	m_NumWrites.Reset();
	m_NumTransfers.Reset();
	m_NumOtherRequests.Reset();

	m_NumBytesRead.Reset();
	m_NumBytesWritten.Reset();
	m_NumBytesTransferred.Reset();
	
	m_ReadLatency.Reset();
	m_WriteLatency.Reset();
	m_TransferLatency.Reset();
	
	m_ReadSize.Reset();
	m_WriteSize.Reset();
	m_TransferSize.Reset();
	
	return;
}