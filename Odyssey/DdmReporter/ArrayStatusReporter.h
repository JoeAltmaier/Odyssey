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
// This class is the ArrayStatusReporter Reporter class. 
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/ArrayStatusReporter.h $
// 
// 4     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
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
// 4     5/05/99 4:08p Jlane
// Miscellaneous integration changes.
// 
// 3     5/05/99 10:02a Jlane
// Pass pb/cb data in Handle DdmSampleData instead of pMsg.  make DdmData
// an Sgl item instead of payload.
//
// 11/16/98	JFL	Created.
/*************************************************************************/

#ifndef __ArrayStatusReporter_h
#define __ArrayStatusReporter_h

#include "CTTypes.h"
#include "ArrayStatusTable.h"
#include "Reporter.h"
#include "StorageRollCallTable.h"
#include "SRCandExportClass.h"


class ArrayStatusReporter: public StatusSRCandExportReporter
{

public:
				ArrayStatusReporter() {};

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
	ArrayStatusRecord		m_ArrayStatusRecord;
	StorageRollCallRecord	m_SRCRecord;
	SRCandExportData		m_Data;
};

#endif	// __ArrayStatusReporter_h
