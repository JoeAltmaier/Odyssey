/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: OneTableClass.cpp
// 
// Description:
//
// This class is the One Table Reporter class. This class handles Reporters
// which only have to collect data from Ddm and update PTS Status/Performance
// tables.  This is different from the SRCandExport class in which the SRC
// Export, and/or LoopDescriptor tables also needs rid updating.
// 
// At this time, this class is used by the EVC (and maybe IOP) Status
// Reporter.
//
// Update Log 
//
// $Log: /Gemini/Odyssey/DdmReporter/OneTableClass.cpp $
// 
// 2     11/06/99 3:11p Vnguyen
// Add debugging statements
// 
// 1     8/24/99 8:31a Vnguyen
// Initial check-in.
// 
/*************************************************************************/

#include "PtsCommon.h"
#include "OneTableClass.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "Rows.h"
#include "Table.h"


/**********************************************************************/
// StatusReporter - This is the contstructor for the  StatusReporter
// Object.  Derived classes must call it in their constructors.
/**********************************************************************/
StatusOneTableReporter::StatusOneTableReporter() // : Reporter()
{
	// Initialize our object's message code used to query and reset ddm
	// status data.
	m_ResetDataMsg = PHS_RESET_STATUS;
	m_ReturnDataMsg = PHS_RETURN_STATUS;
}


/**********************************************************************/
// PerformanceReporter - This is the contstructor for the Performance 
// Reporter Object.  Derived classes must call it in their constructors.
/**********************************************************************/
PerformanceOneTableReporter::PerformanceOneTableReporter() // : Reporter()
{
	// Initialize our object's message code used to query and reset ddm
	// performance data.
	m_ResetDataMsg = PHS_RESET_PERFORMANCE;
	m_ReturnDataMsg = PHS_RETURN_RESET_PERFORMANCE;
}


/**********************************************************************/
// Initialize - This is the initializor for the OneTable Object.
// We've been called because the PHSReporter Ddm wants to instantiate 
// (and initialize) a new reporter object.  Our job is to make sure that our
// performance table and record are created and if not, create them.
//
// A. If the PHS record is already created:  Retrieve previous PHS 
//    Configuration data (e.g. Refresh and Sample Rates) from the existing
//    PHS record, start the Reporter with those data.
//
// B. If the PHS record is not present then create the PHS record row
//    with a set of defaults (e.g. Refresh and Sample Rates).  
// 
/**********************************************************************/
STATUS OneTable::InitOneTable(
	OneTableData 	*pData,
	DID				didDdm, 
	REPORTERCODE	ReporterCode
)
{
STATUS	status;

Tracef("OneTable::InitOneTable\n");
	// Initialize member variables
	m_pData = pData;
	m_didDdm = didDdm;
	m_ReporterCode = ReporterCode;	
	
	TSDefineTable *pDefineTable;
	pDefineTable = new TSDefineTable;
	// The call TSDefineTable creates a table.  If the table is already
	// present, no harm is done.
	status = pDefineTable->Initialize
	(
		this,							// DdmServices	*pDdmServices
		m_pData->PHSTableName,			// String64		rgbTableName,
		m_pData->aPHSTableFieldDefs,	// fieldDef		*prgFieldDefs
		m_pData->cbPHSTableFieldDefs,	// U32			cbrgFieldDefs
		TABLE_SIZE,						// U32			cEntriesRsv
		true,							// U32			PersistFlags
		(pTSCallback_t) &ReadPHSRecord, // pTSCallback_t pCallback
		NULL							// void			*pContext
	);

	if (status == OS_DETAIL_STATUS_SUCCESS)
		pDefineTable->Send();
	else
		delete pDefineTable;
	
	return status;
}


/**********************************************************************/
// ReadPHSRecord - Read the PHS Record if it is present, if not creat
// one with default parameters.
/**********************************************************************/
STATUS OneTable::ReadPHSRecord(void* pContext, STATUS status)
{
#pragma unused(pContext)

	// Now we read a PHS record from the PHS table.  If the call to TS
	// Enumerate Table returns zero bytes read, then we need to create a row
	// with default values.
Tracef("OneTable::ReadPHSRecord status = %d\n", status);
	TSEnumTable *pTSEnumTable;
	pTSEnumTable = new TSEnumTable;
	// Initialize all parameters necessary to get the Table Def'n.
	m_cPHSRecord = 0;
	status = pTSEnumTable->Initialize
	(	
		this,					// DdmServices *pDdmServices,
		m_pData->PHSTableName,	// String64 rgbTableName,
		0,						// U32 uStartRowNum,
		m_pData->pPHSRecord,	// void *pbRowDataRet,
		m_pData->cbPHSRecord,	// U32 cbDataRetMax,
		&m_cPHSRecord,			// U32 *pcbRowDataRet,
		(pTSCallback_t) &ReadPHSRecordReply, // pTSCallback_t pCallback,
		NULL					// void *pContext
	);

	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSEnumTable->Send();
	else
		delete pTSEnumTable;
	
	return status;
} 

/**********************************************************************/
// ReadPHSRecordReply - if PHS Record is not present in PHS table, creat
// one with default parameters.
/**********************************************************************/
STATUS OneTable::ReadPHSRecordReply(void* pContext, STATUS status)
{
#pragma unused(pContext)
Tracef("OneTable::ReadPHSRecordReply status = %d\n", status);

	if (status == OS_DETAIL_STATUS_SUCCESS &&
		m_cPHSRecord >= m_pData->cbPHSRecord) // This means we read at least one row
	{ // We've just retrieved the PHS record.  Now start the base class.
Tracef("OneTable::ReadPHSRecordReply Getting previous record\n");
		m_PHSRowID = GetPHSrid(m_pData->pPHSRecord);
		status = InitBaseClass(NULL, OS_DETAIL_STATUS_SUCCESS);
	}
	else
	{
	// Now we insert a PHS record into the PHS table.  The derived object
	// is responsible for initializing the row record with the correct
	// values.
	InitializePHSRecord(m_pData->pPHSRecord);

Tracef("OneTable::ReadPHSRecordReply Inserting a new record\n");
   
	TSInsertRow *pInsertRow;
	pInsertRow = new TSInsertRow;
	status = pInsertRow->Initialize
	(	
		this,					// DdmServices *pDdmServices,
		m_pData->PHSTableName,	// String64 rgbTableName,
		m_pData->pPHSRecord,	// void *prgbRowData,
		m_pData->cbPHSRecord,	// U32 cbRowData,
		&m_PHSRowID,			// rowID *pRowIDRet,
		(pTSCallback_t) &InitBaseClass, // pTSCallback_t pCallback,
		NULL					// void *pContext 
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pInsertRow->Send();
	else
		delete pInsertRow;
	
	}
	return status;	
	
} 

/**********************************************************************/
// InitBaseClass -  Initialize has insured that our local Record
// is present, and matches the copy in the PTS.  Now continue with our
// initialization.
/**********************************************************************/
STATUS OneTable::InitBaseClass(void* pContext, STATUS status)
{
#pragma unused(pContext)


	if (status)
	{
		Tracef("OneTable::InitBaseClass status is %d\n", status);
		return status;
	}
	

	status = InitReporter( 
		m_pData->PHSTableName,	 	// The name of the table to refresh.
		m_PHSRowID,					// Row ID of table row to refresh.
		*m_pData->pRefreshRate,		// The rate at which to refresh the Ddm.
		*m_pData->pSampleRate		// The rate at which to sample the table.
	);
	
	return status;
}
