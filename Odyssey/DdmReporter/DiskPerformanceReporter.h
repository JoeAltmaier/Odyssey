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
// This class is the DiskPerformanceReporter Reporter class. 
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/DiskPerformanceReporter.h $
// 
// 10    12/14/99 10:24a Vnguyen
// Recalculate transfer (read + write) statistics.  The new method takes
// more time but is more accurate.
// 
// 9     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 8     9/09/99 11:23a Vnguyen
// Update Disk Performance and Status Counters
// 
// 7     8/16/99 1:00p Vnguyen
// Fix various compiler errors.  Mostly typo and mis-match parameters.
// 
// 6     8/05/99 11:04a Vnguyen
// 
// 5     8/04/99 6:25p Vnguyen -- Major change to reuse the object
// SRCandExport.  Now this DiskPerf obj only initializes the Ddm specific
// data and supplies the routine to handle sample/refresh events.  
// The SRCandExport will do the rest.
// 
// 4     5/05/99 4:08p Jlane
// Miscellaneous integration changes.
// 
// 3     5/05/99 10:02a Jlane
// Pass pb/cb data in Handle DdmSampleData instead of pMsg.  make DdmData
// an Sgl item instead of payload.
//
// 11/16/98	JFL	Created.
/*************************************************************************/

#ifndef __DiskPerformanceReporter_h
#define __DiskPerformanceReporter_h

#include "CTTypes.h"
#include "DiskPerformanceTable.h"
#include "Reporter.h"
#include "SampledItem.h"
#include "StorageRollCallTable.h"
#include "SRCandExportClass.h"
	
class DiskPerformanceReporter: public PerformanceSRCandExportReporter
{
public:
				DiskPerformanceReporter() {};

virtual	STATUS	Initialize(	DdmReporter*	pPHSReporter,	// Pointer to the PHSReporter DDM. 
							DID				didDdm,
							VDN				vdnDdm,
							REPORTERCODE	ReporterCode);

//
virtual void InitializePHSRecord(void *m_pPHSRecord, rowID *pSRCandExportRowID);
virtual STATUS	HandleDdmSampleData(void* pDdmData, U32 cbDdmData);
virtual STATUS	ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData);
virtual void ResetCounters();

//private:

	// Member variable declarations:
	SampledU32	m_NumReads;
	SampledU32	m_NumWrites;
	SampledU32  m_NumTransfers;
	SampledU32	m_NumOtherRequests;

	SampledI64	m_NumBytesRead;
	SampledI64	m_NumBytesWritten;
	SampledI64  m_NumBytesTransferred;
	
	SampledI64	m_ReadLatency;
	SampledI64	m_WriteLatency;
	SampledI64  m_TransferLatency;
	
	SampledU32	m_ReadSize;
	SampledU32	m_WriteSize;
	SampledU32  m_TransferSize;

	// Our local copy of the Disk Performance Record and Storage Roll Call Record
	DiskPerformanceRecord	m_DiskPerfRec;
	StorageRollCallRecord	m_SRCRecord;
	SRCandExportData		m_Data;
};

#endif	// __DiskPerformanceReporter_h