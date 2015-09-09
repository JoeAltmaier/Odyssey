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
// This class is the DiskStatusReporter Reporter class. 
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/STSStatReporter.h $
// 
// 7     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 6     9/14/99 5:39p Vnguyen
// Update Status counters (NumTimerTimeout) to be consistent with the
// counters being returned by SCSI Target Server DDM.
// 
// 5     8/16/99 1:05p Vnguyen
// Fix verious compiler errors.  Mostly typo and mis-match parameters.
// 
// 4     8/06/99 12:29p Vnguyen
// 
// 7	8/6/99 8:12a Vnguyen -- Miscellaneous comments
// 
// 6	8/05/99 12:38p Vnguyen -- Add defines for Export Table Field names.
// 	
// 5  	8/03/99 6:25p Vnguyen -- Cleanup member variables.  Add m_Data to
//      supply Ddm data to SRCandExport object.
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

#ifndef __STSStatReporter_h
#define __STSStatReporter_h

#include "CTTypes.h"
#include "STSStatTable.h"
#include "Reporter.h"
#include "ExportTable.h"
#include "SRCandExportClass.h"

// Since the ExportTable.h file does not define these strings, we make some here.
// Ideally, these should be part of ExportTable.h file.
#define fdEXPORT_VDN "vdNext"
#define fdEXPORT_PERFORMANCE_RID "ridPerformanceRec"
#define fdEXPORT_STATUS_RID "ridStatusRec"


class STSStatusReporter: public StatusSRCandExportReporter
{
public:
		STSStatusReporter() {};

virtual	STATUS	Initialize(	DdmReporter*	pPHSReporter,	// Pointer to the PHSReporter DDM. 
							DID				didDdm,
							VDN				vdnDdm,
							REPORTERCODE	ReporterCode);

//
virtual void InitializePHSRecord(void *m_pPHSRecord, rowID *pSRCandExportRowID);
virtual STATUS	HandleDdmSampleData(void* pDdmData, U32 cbDdmData);
virtual STATUS	ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData);


//private:


	// Member variable declarations:
	

	// Our local copy of the STS Performance Record.
	STSStatRecord			m_STSStatRec;	
	ExportTableRecord		m_ExportRecord; 
	SRCandExportData		m_Data;
};


#endif	// __STSStatReporter_h
