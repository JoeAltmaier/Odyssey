/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SSDStatusReporter.cpp
// 
// Description:
// This file implements the SSDStatusReporter Reporter class. 
// 
// Update Log 
// 
// $Log: /Gemini/Odyssey/DdmReporter/SSDStatusReporter.cpp $
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
//
/*************************************************************************/

#include "SSDStatusReporter.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "SSDStatus.h"
#include "DefaultRates.h"

/**********************************************************************/
// Initialize - This is the initializor for the SSDStatusReporter Object.
// We've been called because the PHSReporter Ddm has received a call from
// the SSD DDM and now 
// he (the PHS Ddm) wants to instantiate (and initialize) a new reporter
// object for the SRC entry.  Our Job, as the is to make sure that our
// status table and record are created and if not, create them.  
// Note:  All the work is done by the lower class object.
// All we need to do here is to supply Ddm specific info, e.g. table names,
// a buffer to hold the row record, etc.  
/**********************************************************************/
STATUS SSDStatusReporter::Initialize(
	DdmReporter		*pPHSReporter,	// Pointer to PHSReporter DDM.
	DID				didDdm,
	VDN				vdnDdm,
	REPORTERCODE	ReporterCode
)
{
STATUS	status;
	//
	// Initialize our parent Ddm.  Need to do this to send etc.
	SetParentDdm(m_pPHSReporter = pPHSReporter);

	// Initialize the Data structure with SRC data before handing off
	strcpy(m_Data.SRCandExportTableName, STORAGE_ROLL_CALL_TABLE);
	m_Data.pSRCandExportRecord = &m_SRCRecord;
	m_Data.cbSRCandExportRecordMax = sizeof(StorageRollCallRecord);
	strcpy(m_Data.SRCandExportvdnName, fdSRC_VDNBSADDM); // Actual name of vdnDdm field in SRC/Export table
	strcpy(m_Data.SRCandExportPHSRowIDName, fdSRC_STATUS_RID);	// String64 rgbFieldName

	m_Data.pSRCandExportRowID = &m_SRCRecord.rid;
	m_Data.pPHSRowID = &m_SRCRecord.ridStatusRecord;

	strcpy(m_Data.PHSTableName, SSD_STATUS_TABLE);		 	// Name of table to send PHS data to
	m_Data.aPHSTableFieldDefs = (fieldDef *)aSSDStatusTable_FieldDefs;	// FieldDefs for PHS table 
	m_Data.cbPHSTableFieldDefs = cbSSDStatusTable_FieldDefs;	// size of PHS table fieldDefs, U32 cbrgFieldDefs

	m_Data.pPHSRecord = &m_SSDStatusRecord;		// A pointer to a buffer for a PHS row record
	m_Data.cbPHSRecord = sizeof(m_SSDStatusRecord);	// Size of a PHS row record, cbRowData
	m_Data.cbPHSRecordMax = sizeof(m_SSDStatusRecord); // Maximum size of buffer to receive row data, cbRowDataRetMax

	m_Data.pRefreshRate = &m_SSDStatusRecord.RefreshRate;
	m_Data.pSampleRate = &m_SSDStatusRecord.RefreshRate;  // For status reporter, use Refresh rate for Sample rate.
													
	// Up up and away...
	status = InitSRCandExport(&m_Data, didDdm, vdnDdm, ReporterCode);
	return status;
}
 

/**********************************************************************/
// InitializePHSRecord - This method fills in the PHS record with default
// values.  The RowID for the Storage Roll Call Table entry is also
// put in here.  
/**********************************************************************/
void SSDStatusReporter::InitializePHSRecord(
 		void *pPHSRecord, 
 		rowID *pSRCandExportRowID
 )
 {
 	SSDStatusRecord	*pSSDStatusRecord;
 	pSSDStatusRecord = (SSDStatusRecord *) pPHSRecord;
 	
 	// Declare and initialize a Disk Status Record with default values. 
			SSDStatusRecord	newSSDStatusRecord = {
		// 	Default Value				FieldName							Size   Type
			0,0,0,						// "rid"							8,	I64_FT,
			CT_SSDST_TABLE_VERSION,		// "Version",							4,	U32_FT,
			sizeof(SSDStatusRecord),	// "Size",							4,	U32_FT,
			0,							// "Key",							4,	U32_FT,
			SSD_STATUS_REFRESH_RATE,	// "RefreshRate",					4,	U32_FT,
			
			0,							//  NumReplacementPagesAvailable;	8, 	S64_FT
			0,							//  PageTableSize;
			0							//  PercentDirtyPages;
				
		};
		
	*pSSDStatusRecord = newSSDStatusRecord;
 	pSSDStatusRecord->ridSRCRecord = *pSRCandExportRowID;
 	return;	
 }
 




/**********************************************************************/
// HandleDdmSampleDataReply -  Handle the Status data retruned from the Ddm.
/**********************************************************************/
STATUS SSDStatusReporter::HandleDdmSampleData( void* pDdmData, U32 cbDdmData )
{
#pragma unused(cbDdmData)
STATUS	status = OK;

	Tracef("SSDStatusReporter::HandleDdmSampleData()\n");

	// Get a pointer to the Ddm's reply and cast it to our structure.
	
	SSD_STATUS		*pSSDStatus;
	pSSDStatus = (SSD_STATUS*) pDdmData;
	
	m_SSDStatusRecord.NumReplacementPagesAvailable = pSSDStatus->NumReplacementPagesAvailable;
	m_SSDStatusRecord.PageTableSize = pSSDStatus->PageTableSize;
	m_SSDStatusRecord.PercentDirtyPages = pSSDStatus->PercentDirtyPages;
	
	return OK;
}
	

/**********************************************************************
// ReturnTableRefreshData -  Returns the data to be refreshed to the 
// ArrayStatusTable
**********************************************************************/
STATUS SSDStatusReporter::ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData)
{
	Tracef("SSDStatusReporter::ReturnTableRefreshData()\n");
	rpTableData = &m_SSDStatusRecord;
	rcbTableData = sizeof(m_SSDStatusRecord);

	return	OK;
}
