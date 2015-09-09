/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: STSStatReporter.cpp
// 
// Description:
// This file implements the SCSI Target Server (STS) Status Reporter class. 
// 
// Update Log 
// 
// $Log: /Gemini/Odyssey/DdmReporter/STSStatReporter.cpp $
// 
// 10    12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 9     10/07/99 12:20p Vnguyen
// Combine Status counters #of_error_received and #of_error_internal
// before sending the total number of error to PTS.  This allows the
// customer to monitor a single counter for all errors.
// 
// 8     10/07/99 11:31a Vnguyen
// Add Status counter for # of error received and # of error that are sent
// back.  Note that the # of error sent back is the sum of # of error
// received and the # of error that are generated internally.
// 
// 7     9/14/99 5:38p Vnguyen
// Update Status counters (NumTimerTimeout) to be consistent with the
// counters being returned by SCSI Target Server DDM.
// 
// 6     9/03/99 9:31a Vnguyen
// Rename STSStatus to STS_Status to be consistent.
// 
// 5     8/16/99 1:05p Vnguyen
// Fix verious compiler errors.  Mostly typo and mis-match parameters.
// 
// 4     8/06/99 12:29p Vnguyen
// 
//	8/06/99 - 8:30a Vnguyen.	Created by copying the DiskStatusReporter
//	and change variables/table names to support STS Status Reporter.
/*************************************************************************/


#include "STSStatReporter.h"
#include "STSStat.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "DefaultRates.h"

/**********************************************************************/
// Initialize - This is the initializor for the STSStatusReporter Object.
// We've been called because the PHSReporter Ddm has received a message
// with a request code to start a PHS Reporter.  Our Job is to make sure 
// that our status table and record are created and if not, create them.
// Note:  All the tasks are carried out by the lower level object.  All we
// have to do here is to supply the correct table names and buffer
// space to hold the data.
/**********************************************************************/
STATUS STSStatusReporter::Initialize(
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

	// Initialize the DataBlock structure with Export data before handing off
	strcpy(m_Data.SRCandExportTableName, EXPORT_TABLE);
	m_Data.pSRCandExportRecord = &m_ExportRecord;
	m_Data.cbSRCandExportRecordMax = sizeof(ExportTableRecord);
	
	strcpy(m_Data.SRCandExportvdnName, fdEXPORT_VDN); // Actual name of vdnDdm field in SRC/Export table
	
	strcpy(m_Data.SRCandExportPHSRowIDName, fdEXPORT_STATUS_RID);	// String64 rgbFieldName

	m_Data.pSRCandExportRowID = &m_ExportRecord.rid;
	m_Data.pPHSRowID = &m_ExportRecord.ridStatusRec;

	strcpy(m_Data.PHSTableName, STS_STATUS_TABLE);		 	// Name of table to send PHS data to
	m_Data.aPHSTableFieldDefs = (fieldDef *)aSTSStatTable_FieldDefs;	// FieldDefs for PHS table 
	m_Data.cbPHSTableFieldDefs = cbSTSStatTable_FieldDefs;	// size of PHS table fieldDefs, U32 cbrgFieldDefs

	m_Data.pPHSRecord = &m_STSStatRec;					// A pointer to a buffer for a PHS row record
	m_Data.cbPHSRecord = sizeof(STSStatRecord);	// Size of a PHS row record, cbRowData
	m_Data.cbPHSRecordMax = sizeof(STSStatRecord); // Maximum size of buffer to receive row data, cbRowDataRetMax

	m_Data.pRefreshRate = &m_STSStatRec.RefreshRate;
	m_Data.pSampleRate = &m_STSStatRec.RefreshRate;  // For status reporter, use Refresh rate for Sample rate.
													
	// Up up and away...
	status = InitSRCandExport(&m_Data, didDdm, vdnDdm, ReporterCode);
	return status;
}
 

/**********************************************************************/
// InitializePHSRecord - This method fills in the PHS record with default
// values.  The RowID for the Storage Roll Call Table entry is also
// put in here.  
/**********************************************************************/
void STSStatusReporter::InitializePHSRecord(
 		void *pPHSRecord, 
 		rowID *pSRCandExportRowID
 )
 {
 	STSStatRecord	*pSTSStatusRec;
 	pSTSStatusRec = (STSStatRecord *) pPHSRecord;

		// Declare and initialize a Disk Status Record with default values. 
		STSStatRecord	newSTSStatusRecord = {
		// 	Default Value				FieldName									  Size   Type
			0,0,0,						// "rid"										8,	ROWID_FT,
			CT_STSST_TABLE_VERSION,		// "Version",									4,	U32_FT,
			sizeof(STSStatRecord),		// "Size",										4,	U32_FT,
			0,							// "Key",										4,	U32_FT,
			STS_STATUS_REFRESH_RATE,	// "RefreshRate",								4,	U32_FT,
			0,0,0,						// "ridExportRecord"							8,	ROWID_FT,
			0,							// NumTimerTimeout;								8,	I64_FT,
			0,							// NumErrorRepliesReceived						8,	I64_FT,
			0,							// NumErrorRepliesSent							8,	I64_FT,
			
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,			// "SCSILogPages"							64 * 4, U32_FT,
			
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0				// "NumDifferentialError"					64 * 4, U32_FT,
		};

	*pSTSStatusRec = newSTSStatusRecord;
 	pSTSStatusRec->ridExportRecord = *pSRCandExportRowID;
 	return;	
 }
 


/**********************************************************************/
// HandleDdmSampleDataReply -  Handle the Status data returned from the Ddm.
/**********************************************************************/
STATUS STSStatusReporter::HandleDdmSampleData( void* pDdmData, U32 cbDdmData )
{
#pragma unused(cbDdmData)

STS_Status	*pSTSStatus;
STATUS		status = OK;

	Tracef("STSStatusReporter::HandleDdmSampleData()\n");

	// Get a pointer to the Ddm's reply and cast it to our structure.
	pSTSStatus = (STS_Status*)pDdmData;
	
	m_STSStatRec.NumTimerTimeout = pSTSStatus->NumTimerTimeout;
	m_STSStatRec.NumErrorRepliesReceived = pSTSStatus->NumErrorRepliesReceived;
	m_STSStatRec.NumErrorRepliesSent = pSTSStatus->NumErrorRepliesReceived + pSTSStatus->NumErrorInternal;
	
	// Need for loop here if array asignment does not work.
	int i;
	for (i = 0; i < SCSI_PAGE_SIZE; i++)
		m_STSStatRec.SCSILogPages[i] = pSTSStatus->SCSILogPages[i];
	
	for (i = 0; i < DIFFERENTIAL_ERROR_SIZE; i++)
		m_STSStatRec.NumDifferentialError[i] = pSTSStatus->NumDifferentialError[i];
	
	return OK;
}
	

/**********************************************************************
// ReturnTableRefreshData -  Returns the data to be refreshed to the 
// Status Table
**********************************************************************/
STATUS STSStatusReporter::ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData)
{
	Tracef("STSStatusReporter::ReturnTableRefreshData()\n");
	rpTableData = &m_STSStatRec;
	rcbTableData = sizeof(m_STSStatRec);

	// Note:  We do not clear the status counters for each Refresh.

	return	OK;
}


















