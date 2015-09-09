/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FCPTargetStatusReporter.cpp
// 
// Description:
// This file implements the FCP Target driver Status Reporter class. 
// 
// Update Log 
// 
// $Log: /Gemini/Odyssey/DdmReporter/FCPTargetStatusReporter.cpp $
// 
// 4     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 3     10/11/99 6:02p Vnguyen
// Add timestamp for Performance counter and # of error received for
// Status counters
// 
// 2     9/15/99 11:59a Vnguyen
// Update Performance and Status counters to match the counters returned
// by FCP Target driver.
// 
// 1     8/16/99 2:40p Vnguyen
// New check-in.
// 
/*************************************************************************/


#include "FCPTargetStatusReporter.h"
#include "FCPTargetStatus.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "DefaultRates.h"


/**********************************************************************/
// Initialize - This is the initializor for the FCPT StatusReporter Object.
// We've been called because the PHSReporter Ddm has received a message
// with a request code to start a PHS Reporter.  Our Job is to make sure 
// that our status table and record are created and if not, create them.
// Note:  All the tasks are carried out by the lower level object.  All we
// have to do here is to supply the correct table names and buffer
// space to hold the data.
/**********************************************************************/
STATUS FCPTargetStatusReporter::Initialize(
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

	// Initialize the DataBlock structure with Loop Descriptor data before handing off
	strcpy(m_Data.SRCandExportTableName, LOOP_DESCRIPTOR_TABLE);
	
	m_Data.pSRCandExportRecord = &m_LoopDescriptorRecord;
	m_Data.cbSRCandExportRecordMax = sizeof(LoopDescriptorRecord);
	
	strcpy(m_Data.SRCandExportvdnName, fdLD_LM_VDN); // Actual name of vdnDdm field in SRC/Export/Loop Descriptor table
	
	strcpy(m_Data.SRCandExportPHSRowIDName, fdLD_STATUS_RID);	// String64 rgbFieldName

	m_Data.pSRCandExportRowID = &m_LoopDescriptorRecord.rid;
	m_Data.pPHSRowID = &m_LoopDescriptorRecord.ridStatusRec;

	strcpy(m_Data.PHSTableName, FCPT_STATUS_TABLE); 	// Name of table to send PHS data to
	m_Data.aPHSTableFieldDefs = (fieldDef *)aFCPTargetStatusTable_FieldDefs;	// FieldDefs for PHS table 
	m_Data.cbPHSTableFieldDefs = cbFCPTargetStatusTable_FieldDefs;	// size of PHS table fieldDefs, U32 cbrgFieldDefs

	m_Data.pPHSRecord = &m_FCPTargetStatusRecord;			// A pointer to a buffer for a PHS row record
	m_Data.cbPHSRecord = sizeof(FCPTargetStatusRecord); 	// Size of a PHS row record, cbRowData
	m_Data.cbPHSRecordMax = sizeof(FCPTargetStatusRecord); // Maximum size of buffer to receive row data, cbRowDataRetMax

	m_Data.pRefreshRate = &m_FCPTargetStatusRecord.RefreshRate;
	m_Data.pSampleRate = &m_FCPTargetStatusRecord.RefreshRate;

	// Up up and away...
	status = InitSRCandExport(&m_Data, didDdm, vdnDdm, ReporterCode);
	return status;
}
 

/**********************************************************************/
// InitializePHSRecord - This method fills in the PHS record with default
// values.  The RowID for the Storage Roll Call Table entry is also
// put in here.  
/**********************************************************************/
void FCPTargetStatusReporter::InitializePHSRecord(
 		void *pPHSRecord, 
 		rowID *pSRCandExportRowID
 )
 {
 	FCPTargetStatusRecord	*pFCPTargetStatusRecord;
 	pFCPTargetStatusRecord = (FCPTargetStatusRecord *) pPHSRecord;

		// Declare and initialize a FCP Target Status Record with default values. 
		FCPTargetStatusRecord	newFCPTargetStatusRecord = {
		// 	Default Value				FieldName									  Size   Type
			0,0,0,						// "rid"										8,	I64_FT,
			CT_FCPTST_TABLE_VERSION,	// "Version",									4,	U32_FT,
			sizeof(FCPTargetStatusRecord), // "Size",									4,	U32_FT,
			0,							// "Key",										4,	U32_FT,
			FCPT_STATUS_REFRESH_RATE,	// "RefreshRate",								4,	U32_FT,
			0,0,0,						// Row ID of LoopDescriptor entry 4 this device.
			0,							// UpTime in microseconds
			0,							// NumErrorRepliesReceived;
			0,							// FCPTargetStateTable.
			0,							// DriverReadyStateFlag. // Reset, Ready, Not Ready
			0,							// Errors.
			0,							// LoopDown.			// We can force the loop down if we want.
			0							// TransferIncomplete.
	
		};

	*pFCPTargetStatusRecord = newFCPTargetStatusRecord;
 	pFCPTargetStatusRecord->ridLoopDescriptorRecord = *pSRCandExportRowID;
 	return;	
 }


/**********************************************************************/
// HandleDdmSampleDataReply -  Handle the Status data returned from the Ddm.
/**********************************************************************/
STATUS FCPTargetStatusReporter::HandleDdmSampleData( void* pDdmData, U32 cbDdmData )
{
#pragma unused(cbDdmData)

FCPT_Status	*pFCPTStatus;
STATUS		status = OK;

	Tracef("FCPTargetStatusReporter::HandleDdmSampleData()\n");

	// Get a pointer to the Ddm's reply and cast it to our structure.
	pFCPTStatus = (FCPT_Status*)pDdmData;

	m_FCPTargetStatusRecord.NumErrorRepliesReceived = pFCPTStatus->NumErrorRepliesReceived;
	
	
	m_FCPTargetStatusRecord.FCPTargetStateTable = pFCPTStatus->FCPTargetStateTable;
	m_FCPTargetStatusRecord.DriverReadyStateFlag = pFCPTStatus->DriverReadyStateFlag;
	m_FCPTargetStatusRecord.Errors = pFCPTStatus->Errors;
	m_FCPTargetStatusRecord.LoopDown = pFCPTStatus->LoopDown;
	m_FCPTargetStatusRecord.TransferIncomplete = pFCPTStatus->TransferIncomplete;
	
	return OK;
}
	

/**********************************************************************
// ReturnTableRefreshData -  Returns the data to be refreshed to the 
// Status Table
**********************************************************************/
STATUS FCPTargetStatusReporter::ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData)
{
	Tracef("FCPTargetStatusReporter::ReturnTableRefreshData()\n");
	rpTableData = &m_FCPTargetStatusRecord;
	rcbTableData = sizeof(m_FCPTargetStatusRecord);

	// Note:  We do not clear the status counters for each Refresh.

	return	OK;
}

