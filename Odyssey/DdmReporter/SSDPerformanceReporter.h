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
// This class is the header for the SSDPerformanceReporter Reporter class. 
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/SSDPerformanceReporter.h $
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
// 
/*************************************************************************/

#ifndef __SSDPerformanceReporter_h
#define __SSDPerformanceReporter_h

#include "CTTypes.h"
#include "SSDPerformanceTable.h"
#include "Reporter.h"
#include "SampledItem.h"
#include "StorageRollCallTable.h"
#include "SRCandExportClass.h"

	
class SSDPerformanceReporter: public PerformanceSRCandExportReporter
{
public:
		SSDPerformanceReporter() {};

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
	SampledU32	m_NumPagesRead;
	SampledU32	m_NumPagesReadCacheHit;
	SampledU32	m_NumPagesReadCacheMiss;

	SampledU32	m_NumPagesWrite;
	SampledU32	m_NumPagesWriteCacheHit;
	SampledU32	m_NumPagesWriteCacheMiss;

	SampledU32	m_NumErasePagesAvailable;

	SampledI64	m_NumReadBytesTotal;
	SampledI64	m_NumWriteBytesTotal;
	
	
// Our local copy of the SSD Performance Record and Storage Roll Call Record
	SSDPerformanceRecord	m_SSDPerformanceRecord;
	StorageRollCallRecord	m_SRCRecord;
	SRCandExportData		m_Data;
};

#endif	// __SSDPerformanceReporter_h
