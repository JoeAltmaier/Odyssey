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
// $Log: /Gemini/Odyssey/DdmReporter/DiskStatusReporter.h $
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
// 6     8/05/99 11:07a Vnguyen
// 
// 5     8/04/99 5:25p Vnguyen
// Miscellaneous changes in declaration and variables.
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

#ifndef __DiskStatusReporter_h
#define __DiskStatusReporter_h

#include "CTTypes.h"
#include "DiskStatusTable.h"
#include "Reporter.h"
#include "StorageRollCallTable.h"
#include "SRCandExportClass.h"
	
class DiskStatusReporter: public StatusSRCandExportReporter
{

public:
				DiskStatusReporter() {};

virtual	STATUS	Initialize(	DdmReporter*	pPHSReporter,	// Pointer to the PHSReporter DDM. 
							DID				didDdm,
							VDN				vdnDdm,
							REPORTERCODE	ReporterCode);

virtual void InitializePHSRecord(void *m_pPHSRecord, rowID *pSRCandExportRowID);
virtual STATUS	HandleDdmSampleData( void* pDdmData, U32 cbDdmData );
virtual STATUS	ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData);

// Instance Data:

// private:
	DiskStatusRecord		m_DiskStatusRecord;
	StorageRollCallRecord	m_SRCRecord;
	SRCandExportData		m_Data;
};

#endif	// __DiskStatusReporter_h
