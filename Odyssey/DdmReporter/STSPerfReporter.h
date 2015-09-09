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
// This class is the SCSI Target Server Performance Reporter. 
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/STSPerfReporter.h $
// 
// 8     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 7     9/15/99 9:06a Vnguyen
// Update Performance counters to match with the counters being returned
// by the SCSI Target Server DDM.
// 
// 6     8/16/99 1:05p Vnguyen
// Fix verious compiler errors.  Mostly typo and mis-match parameters.
// 
// 5     8/06/99 12:29p Vnguyen
// 
// 4     8/06/99 8:24a Vnguyen
//
// 4	8/6/99 8:12a Vnguyen -- Miscellaneous comments
// 
// 3	8/05/99 12:38p Vnguyen -- Add defines for Export Table Field names.
// 		Add method ResetCounters. 
// 2  	8/03/99 6:25p Vnguyen -- Cleanup member variables.  Add m_Data to
//      supply Ddm data to SRCandExport object.
// 
// 1  	8/02/99 3:35p Vnguyen -- Add variables for counters using SampleItem.h
// 
//
// 07/28/99	3:24p Vnguyen	Created.
/*************************************************************************/

#ifndef __STSPerfReporter_h
#define __STSPerfReporter_h

#include "CTTypes.h"
#include "STSPerfTable.h"
#include "SampledItem.h"
#include "Reporter.h"
#include "ExportTable.h"
#include "SRCandExportClass.h"

// Since the ExportTable.h file does not define these strings, we make some here.
// Ideally, these should be part of ExportTable.h file.
#define fdEXPORT_VDN "vdNext"
#define fdEXPORT_PERFORMANCE_RID "ridPerformanceRec"
#define fdEXPORT_STATUS_RID "ridStatusRec"


class STSPerfReporter: public PerformanceSRCandExportReporter
{
public:
		STSPerfReporter() {};

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
	SampledU32		m_NumBSAReads;
	SampledU32		m_NumBSAWrites;
	SampledU32		m_NumSCSICmds;

	SampledI64		m_NumBSABytesRead;
	SampledI64		m_NumBSABytesWritten;
	
	// Our local copy of the STS Performance Record.
	STSPerfRecord			m_STSPerfRec;	
	ExportTableRecord		m_ExportRecord; 
	SRCandExportData		m_Data;
};

#endif	// __STSPerfReporter_h
