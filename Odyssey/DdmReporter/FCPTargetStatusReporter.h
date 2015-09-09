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
// This class is the FCP Target driver Status Reporter Reporter class. 
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/FCPTargetStatusReporter.h $
// 
// 3     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 2     9/15/99 11:59a Vnguyen
// Update Performance and Status counters to match the counters returned
// by FCP Target driver.
// 
// 1     8/16/99 2:40p Vnguyen
// New check-in.
// 
/*************************************************************************/

#ifndef __FCPTargetStatusReporter_h
#define __FCPTargetStatusReporter_h

#include "CTTypes.h"
#include "FCPTargetStatusTable.h"
#include "DdmReporter.h"
#include "Reporter.h"
#include "LoopDescriptor.h"
#include "SRCandExportClass.h"
#include "RqDdmReporter.h"

class FCPTargetStatusReporter: public StatusSRCandExportReporter
{
public:
		FCPTargetStatusReporter() {};

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
	

	// Our local copy of the FCP Target Status Record.
	FCPTargetStatusRecord	m_FCPTargetStatusRecord;	
	LoopDescriptorRecord	m_LoopDescriptorRecord; 
	SRCandExportData		m_Data;
};


#endif	// __FCPTargetStatusReporter_h
