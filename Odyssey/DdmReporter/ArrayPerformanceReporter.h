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
// This class is the header for the ArrayPerformanceReporter Reporter class. 
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/ArrayPerformanceReporter.h $
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
/*************************************************************************/

#ifndef __ArrayPerformanceReporter_h
#define __ArrayPerformanceReporter_h

#include "CTTypes.h"
#include "ArrayPerformanceTable.h"
#include "Reporter.h"
#include "SampledItem.h"
#include "StorageRollCallTable.h"
#include "SRCandExportClass.h"

	
class ArrayPerformanceReporter: public PerformanceSRCandExportReporter
{
public:
		ArrayPerformanceReporter() {};

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
	SampledU32				m_NumRds[NUM_MEASURE_SIZES];
	SampledU32				m_NumWrts[NUM_MEASURE_SIZES];
	SampledU32				m_NumReads;
	SampledU32				m_NumWrites;
	SampledU32				m_NumBlocksRead;
	SampledU32				m_NumBlocksWritten;
	SampledU32				m_NumSGCombinedReads;
	SampledU32				m_NumSGCombinedWrites;
	SampledU32				m_NumOverwrites;
	

// Our local copy of the Array Performance Record and Storage Roll Call Record
	ArrayPerformanceRecord	m_ArrayPerformanceRecord;
	StorageRollCallRecord	m_SRCRecord;
	SRCandExportData		m_Data;
};

#endif	// __ArrayPerformanceReporter_h
