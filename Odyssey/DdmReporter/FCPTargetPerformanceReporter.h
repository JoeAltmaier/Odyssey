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
// This class is the FCP Target Performance Reporter. 
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/FCPTargetPerformanceReporter.h $
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
// 1     8/16/99 2:40p Vnguyen
// New check-in.
// 
//
/*************************************************************************/

#ifndef __FCPTargetPerformanceReporter_h
#define __FCPTargetPerformanceReporter_h

#include "CTTypes.h"
#include "FCPTargetPerformanceTable.h"
#include "Reporter.h"
#include "SampledItem.h"
#include "LoopDescriptor.h"
#include "SRCandExportClass.h"


// Note:  The FCP Target driver record is in the Loop Descriptor table.  However,
// it can still reuse the class SRCandExportReporter.  At this time, I don't want to
// rename the SRCandExportReporter class to encompass the LoopDescriptor addition.
// Conceptually, the SRCandExport class can handle anything that is similar in terms
// of things to be done with SRC, Export, and LoopDescriptor tables, etc.

class FCPTargetPerformanceReporter: public PerformanceSRCandExportReporter
{
public:
		FCPTargetPerformanceReporter() {};

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
	SampledU32		m_NumReadPackets;
	SampledU32		m_NumWritePackets;
	SampledU32		m_NumTotalPackets; // Read + Write + Other packets.
	
	SampledI64		m_NumBytesRead;
	SampledI64		m_NumBytesWritten;
	
	SampledI64		m_ReadLatency;		// microseconds
	SampledI64		m_WriteLatency;

	// Our local copy of the FCPT Performance Record.
	FCPTargetPerformanceRecord	m_FCPTargetPerformanceRecord;	
	LoopDescriptorRecord		m_LoopDescriptorRecord; 
	SRCandExportData			m_Data;
};

#endif	// __FCPTargetPerformanceReporter_h
