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
// This class is the SSDStatusReporter Reporter class. 
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/SSDStatusReporter.h $
// 
// 2     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 1     8/20/99 10:39a Vnguyen
// Initial check-in.
// 
//
/*************************************************************************/

#ifndef __SSDStatusReporter_h
#define __SSDStatusReporter_h

#include "CTTypes.h"
#include "SSDStatusTable.h"
#include "Reporter.h"
#include "StorageRollCallTable.h"
#include "SRCandExportClass.h"


class SSDStatusReporter: public StatusSRCandExportReporter
{

public:
				SSDStatusReporter() {};

virtual	STATUS	Initialize(	DdmReporter*	pPHSReporter,	// Pointer to the PHSReporter DDM. 
							DID				didDdm,
							VDN				vdnDdm,
							REPORTERCODE	ReporterCode);

//
virtual void InitializePHSRecord(void *m_pPHSRecord, rowID *pSRCandExportRowID);
virtual STATUS	HandleDdmSampleData(void* pDdmData, U32 cbDdmData);
virtual STATUS	ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData);

// Instance Data:

private:
// Our local copy of the Array Status Record and Storage Roll Call Record
	SSDStatusRecord			m_SSDStatusRecord;
	StorageRollCallRecord	m_SRCRecord;
	SRCandExportData		m_Data;
};

#endif	// __SSDStatusReporter_h
