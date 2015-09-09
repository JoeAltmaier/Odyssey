/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SRCandExportClass.cpp
// 
// Description:
// This file implements the StorageRollCall and Export Table Reporter class. 
// 
// Update Log 
//
// $Log: /Gemini/Odyssey/DdmReporter/SRCandExportClass.cpp $
// 
// 6     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 5     8/17/99 8:31a Vnguyen
// Update TSModifyRow call to accept more parameters.
// 
// 4     8/16/99 1:05p Vnguyen
// Fix verious compiler errors.  Mostly typo and mis-match parameters.
// 
// 3     8/06/99 9:46a Vnguyen
//
// 3     8/06/99 9:43a Vnguyen
// Change PHS table to persistent to make sure the PHS row won't go away.  Otherwise
// the reference from the SRC/Export table would be pointing at the wrong place.
// 
// 2     8/05/99 10:55a Vnguyen -- Add comments
// 
// 1     8/04/99 5:26p Vnguyen -- Fix up parameters between objects.  Make
// sure they match.
//
// 08/03/99	Vnguyen. Created.
//
/*************************************************************************/

#include "PtsCommon.h"
#include "SRCandExportClass.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "Rows.h"
#include "Fields.h"
#include "Table.h"


/**********************************************************************/
// StatusReporter - This is the contstructor for the  StatusReporter
// Object.  Derived classes must call it in their constructors.
/**********************************************************************/
StatusSRCandExportReporter::StatusSRCandExportReporter() // : Reporter()
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
PerformanceSRCandExportReporter::PerformanceSRCandExportReporter() // : Reporter()
{
	// Initialize our object's message code used to query and reset ddm
	// performance data.
	m_ResetDataMsg = PHS_RESET_PERFORMANCE;
	m_ReturnDataMsg = PHS_RETURN_RESET_PERFORMANCE;
}



/**********************************************************************/
// Initialize - This is the initializor for the SRCandExport Object.
// We've been called because the PHSReporter Ddm wants to instantiate 
// (and initialize) a new reporter
// object for the SRC/Export entry.  Our job is to make sure that our
// performance table and record are created and if not, create them.
//
// A. If the PHS record is already created:  Retrieve previous PHS 
//    Configuration data (e.g. Refresh and Sample Rates) from the existing
//    PHS record, start the Reporter with those data.
//
// B. If the PHS record is not present (because the PHS row ID is not
//    initialized in the SRC/Export table) then create the PHS record row
//    with a set of defaults (e.g. Refresh and Sample Rates).  After the
//    PHS row record is inserted in the PHS table, update the SRC/Export
//    row record with the PHS row ID so the next time we know that it
//    is already created.
// 
/**********************************************************************/
STATUS SRCandExport::InitSRCandExport(
	SRCandExportData 	*pData,
	DID					didDdm, 
	VDN 				vdnDdm,
	REPORTERCODE	ReporterCode
)
{
STATUS	status;

	// Initialize member variables
	m_pData = pData;
	m_didDdm = didDdm;
	m_vdnDdm = vdnDdm;	
	m_ReporterCode = ReporterCode;	
	
	// Read the SRC/Export table for the row containing the vdnDdm of the calling Ddm
	// We can inspect the PHS RowID in the record to see if this is a new instantiation
	// or a fail over event.  If this is a fail over event, we need to get the
	// current configuration data from the PHS Table and continue.
	
	TSReadRow *pReadRow;
	pReadRow = new TSReadRow;
	
	m_cSRCandExportRecord = 0;			// TSReadRow would init this, but just in case...
	status = pReadRow->Initialize
	(
		this,							// DdmServices *pDdmServices
		m_pData->SRCandExportTableName,	// String64	rgbTableName
		m_pData->SRCandExportvdnName,	// String64	rgbKeyFieldName
		&m_vdnDdm,						// void *pKeyFieldValue
		sizeof(VDN),					// U32 cbKeyFieldValue
		m_pData->pSRCandExportRecord,	// void *prgbRowDataRet
		m_pData->cbSRCandExportRecordMax,// U32 cbRowDataRetMax
		&m_cSRCandExportRecord,			// U32 *pcRowsReadRet
		(pTSCallback_t) &CheckPHSReply,	// pTSCallback_t pCallback
		NULL							// void *pContext
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pReadRow->Send();
	else
		delete pReadRow;
	
	return status;
}

/**********************************************************************/
// CheckPHSReply - See if the SRC/Export row already has the rid for the
// PHS Reporter already initialized.
/**********************************************************************/

STATUS SRCandExport::CheckPHSReply(void* pContext, STATUS status)
{
#pragma unused(pContext)

	// If there is no row with the matching vdn, this is an error
	if (!m_cSRCandExportRecord ||
		status != OS_DETAIL_STATUS_SUCCESS)
	{ // Log error here 
		Tracef("SRCandExport::CheckPHSReply, status = %u.\n", status);	
		return status;
	}
	
	// Note:  We just read in a SRC/Export record, and the variable
	// pPHSRowID has been updated because it points to the PHSRowID inside the record.
	m_PHSRowID = *m_pData->pPHSRowID; 
	if (NULLROWID(m_PHSRowID))
	{	// This is a new instantiation.  Need to create the PHS table if
		// needed, then insert the PHS row for this Ddm.
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
			(pTSCallback_t) &CreatePHSRecord, // pTSCallback_t pCallback
			NULL							// void			*pContext
		);

		if (status == OS_DETAIL_STATUS_SUCCESS)
			pDefineTable->Send();
		else
			delete pDefineTable;
	
		return status;
	}
	else
	{	// Since the PHS record is already defined in the PHS Table, we simply
		// read the PHS record, keying off the PHS RowID, then collect the previous
		// configuration data.  At this time, that is the Refresh and Sample rates.
		TSReadRow *pReadRow;
		pReadRow = new TSReadRow;
	
		status = pReadRow->Initialize
		(
			this,						// DdmServices *pDdmServices
			m_pData->PHSTableName,		// String64	rgbTableName
			CT_PTS_RID_FIELD_NAME,		// String64	rgbKeyFieldName
			&m_PHSRowID,				// void *pKeyFieldValue
			sizeof(rowID),			 	// U32 cbKeyFieldValue
			m_pData->pPHSRecord,		// void *prgbRowDataRet
			m_pData->cbPHSRecordMax,	// U32 cbRowDataRetMax
			&m_cPHSRecord,				// U32 *pcRowsReadRet
			(pTSCallback_t) &InitBaseClass,	// pTSCallback_t pCallback
			NULL						// void *pContext
		);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pReadRow->Send();
	else
		delete pReadRow;
	}
	return status;
}
 
/**********************************************************************/
// CreateRecord - Create the PHS Record
/**********************************************************************/
STATUS SRCandExport::CreatePHSRecord(void* pContext, STATUS status)
{
#pragma unused(pContext)

	// Now we insert a PHS record into the PHS table.  The derived object
	// is responsible for initializing the row record with the correct
	// values.
	
	InitializePHSRecord(m_pData->pPHSRecord, m_pData->pSRCandExportRowID);

	TSInsertRow *pInsertRow;
	pInsertRow = new TSInsertRow;
	
	status = pInsertRow->Initialize
	(	
		this,			// DdmServices 	*pDdmServices,
		m_pData->PHSTableName,	// String64 	rgbTableName,
		m_pData->pPHSRecord,	// void			*prgbRowData,
		m_pData->cbPHSRecord,	// U32			cbRowData,
		&m_PHSRowID,	// rowID		*pRowIDRet,
		(pTSCallback_t) &UpdateSRCandExportTable, // pTSCallback_t	pCallback,
		NULL			// void			*pContext 
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pInsertRow->Send();
	else
		delete pInsertRow;
	
	return status;
} 


/**********************************************************************/
// UpdateSRCandExportTable -- We just insert a row into the PHS table.
// now we need to go back to the SRC/Export Table and update the record
// there with the PHS RowID.  That way, when we have a fail over, we would
// know how to continue.
/**********************************************************************/

STATUS SRCandExport::UpdateSRCandExportTable(void* pContext, STATUS status)
{
#pragma unused(pContext)

	
	TSModifyField *pModifyField;
	pModifyField = new TSModifyField;
	
	status = pModifyField->Initialize(
		this,								// DdmServices		*pDdmServices,
		m_pData->SRCandExportTableName,		// String64			rgbTableName,
		CT_PTS_RID_FIELD_NAME,				// String64			rgbKeyFieldName,
		m_pData->pSRCandExportRowID,		// void				*pKeyFieldValue,
		sizeof(rowID),						// U32				cbKeyFieldValue,
		m_pData->SRCandExportPHSRowIDName,	// String64			rgbFieldName,
		&m_PHSRowID,						// void				*pbFieldValue,
		sizeof(rowID),						// U32				cbFieldValue,
		1,									// U32				cRowsToModify,
		NULL,								// U32				*pcRowsModifiedRet,
		NULL,								// rowID			*pRowIDRet,
 		0,									// U32				cbMaxRowID,
		(pTSCallback_t) &InitBaseClass,		// pTSCallback_t		pCallback,
		NULL								// void				*pContext 
	);
	
	

	if (status == OS_DETAIL_STATUS_SUCCESS)
		pModifyField->Send();
	else
		delete pModifyField;
	
	return status;
}


/**********************************************************************/
// InitBaseClass -  Initialize has insured that our local DiskPerformanceRecord
// is present, and matches the copy in the PTS.  Now continue with our
// iniitialization.
/**********************************************************************/
STATUS SRCandExport::InitBaseClass(void* pContext, STATUS status)
{
#pragma unused(pContext)

	if (status)
		;			// ???

	status = InitReporter( 
		m_pData->PHSTableName,	 	// The name of the table to refresh.
		m_PHSRowID,					// Row ID of table row to refresh.
		*m_pData->pRefreshRate,		// The rate at which to refresh the Ddm.
		*m_pData->pSampleRate		// The rate at which to sample the table.
	);
	
	return status;
}
