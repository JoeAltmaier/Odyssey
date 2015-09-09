/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DiskStatusReporter.cpp
// 
// Description:
// This file implements the DiskStatusReporter Reporter class. 
// 
// Update Log 
// 
// $Log: /Gemini/Odyssey/DdmReporter/DiskStatusReporter.cpp $
// 
// 13    12/15/99 2:04p Vnguyen
// Move Tracef.  Also increment key counter for each update.  This way we
// can tell when an update just happens and see if the info is propogated
// to GMI GUI or not.
// 
// 12    12/10/99 8:55a Vnguyen
// Comment out Tracef lines so the NAC display won't get cluttered up.
// 
// 11    12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 10    9/09/99 11:23a Vnguyen
// Update Disk Performance and Status Counters
// 
// 9     8/16/99 1:00p Vnguyen
// Fix various compiler errors.  Mostly typo and mis-match parameters.
// 
// 8     8/06/99 12:29p Vnguyen
// 
// 7     8/05/99 11:07a Vnguyen
// 
// 6     8/04/99 6:25p Vnguyen -- Major change to reuse the object
// SRCandExport.  Now this Status obj only initializes the Ddm specific
// data and supplies the routine to handle sample/refresh events.  
// The SRCandExport will do the rest.
// 
// 5     5/19/99 9:02a Jlane
// Miscellaneous changes and bug fixes made during bringup.
// 
// 4     5/13/99 11:38a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 3     5/05/99 10:02a Jlane
// Pass pb/cb data in Handle DdmSampleData instead of pMsg.  make DdmData
// an Sgl item instead of payload.
//
// 11/16/98	JFL	Created.  A gray Monday.
//
/*************************************************************************/

#include "DiskStatusReporter.h"
#include "BsaStatus.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "DefaultRates.h"

/**********************************************************************/
// Initialize - This is the initializor for the DiskStatusReporter Object.
// We've been called because the PHSReporter Ddm has received a reply to
// a listen operation he posted on the Storage Roll Call table and now 
// he (the PHS Ddm) wants to instantiate (and initialize) a new reporter
// object for the SRC entry.  Our Job, as the is to make sure that our
// status table and record are created and if not, create them.    
/**********************************************************************/
STATUS DiskStatusReporter::Initialize(
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

	// Initialize the DataBlock structure with SRC data before handing off
	strcpy(m_Data.SRCandExportTableName, STORAGE_ROLL_CALL_TABLE);
	m_Data.pSRCandExportRecord = &m_SRCRecord;
	m_Data.cbSRCandExportRecordMax = sizeof(StorageRollCallRecord);
	strcpy(m_Data.SRCandExportvdnName, fdSRC_VDNBSADDM); // Actual name of vdnDdm field in SRC/Export table
	strcpy(m_Data.SRCandExportPHSRowIDName, fdSRC_STATUS_RID);	// String64 rgbFieldName

	m_Data.pSRCandExportRowID = &m_SRCRecord.rid;
	m_Data.pPHSRowID = &m_SRCRecord.ridStatusRecord;

	strcpy(m_Data.PHSTableName, DISK_STATUS_TABLE);		 	// Name of table to send PHS data to
	m_Data.aPHSTableFieldDefs = (fieldDef *)aDiskStatusTable_FieldDefs;	// FieldDefs for PHS table 
	m_Data.cbPHSTableFieldDefs = cbDiskStatusTable_FieldDefs;	// size of PHS table fieldDefs, U32 cbrgFieldDefs

	m_Data.pPHSRecord = &m_DiskStatusRecord;		// A pointer to a buffer for a PHS row record
	m_Data.cbPHSRecord = sizeof(DiskStatusRecord);	// Size of a PHS row record, cbRowData
	m_Data.cbPHSRecordMax = sizeof(DiskStatusRecord); // Maximum size of buffer to receive row data, cbRowDataRetMax

	m_Data.pRefreshRate = &m_DiskStatusRecord.RefreshRate;
	m_Data.pSampleRate = &m_DiskStatusRecord.RefreshRate;  // For status reporter, use Refresh rate for Sample rate.
													
	// Up up and away...
	status = InitSRCandExport(&m_Data, didDdm, vdnDdm, ReporterCode);
	return status;
}
 

/**********************************************************************/
// InitializePHSRecord - This method fills in the PHS record with default
// values.  The RowID for the Storage Roll Call Table entry is also
// put in here.  
/**********************************************************************/
void DiskStatusReporter::InitializePHSRecord(
 		void *pPHSRecord, 
 		rowID *pSRCandExportRowID
 )
 {
 	DiskStatusRecord	*pDiskStatusRec;
 	pDiskStatusRec = (DiskStatusRecord *) pPHSRecord;

		// Declare and initialize a Disk Status Record with default values. 
		DiskStatusRecord	newDiskStatusRecord = {
		// 	Default Value				FieldName									  Size   Type
			0,0,0,						// "rid"										8,	I64_FT,
			CT_DST_TABLE_VERSION,		// "Version",									4,	U32_FT,
			sizeof(DiskStatusRecord),	// "Size",										4,	U32_FT,
			0,							// "Key",										4,	U32_FT,
			DISK_STATUS_REFRESH_RATE,	// "RefreshRate",								4,	U32_FT,
			0,0,0,						// "ridSRCRecord"								8,	I64_FT,
			0,							// "num_error_replies_received",				4,	U32_FT,
			0,							// "num_recoverable_media_errors_no_delay",		4,	U32_FT,
			0,							// "num_recoverable_media_errors_delay",		4,	U32_FT,
			0,							// "num_recoverable_media_errors_by_retry",		4,	U32_FT,
			0,							// "num_recoverable_media_errors_by_ecc",		4,	U32_FT,
			0,							// "num_recoverable_nonmedia_errors",			4,	U32_FT,
			0,							// "num_bytesprocessedtotal",					4,	U32_FT,
			0							// "num_unrecoverable_media_errors",			4,	U32_FT,
		};

	*pDiskStatusRec = newDiskStatusRecord;
 	pDiskStatusRec->ridSRCRecord = *pSRCandExportRowID;
 	return;	
 }
 



/**********************************************************************/
// HandleDdmSampleDataReply -  Handle the Status data retruned from the Ddm.
/**********************************************************************/
STATUS DiskStatusReporter::HandleDdmSampleData( void* pDdmData, U32 cbDdmData )
{
#pragma unused(cbDdmData)
BSA_ISMStatus	*pBSA_ISMStatus;
STATUS			status = OK;

//	Tracef("DiskStatusReporter::HandleDdmSampleData()\n");

	// Get a pointer to the Ddm's reply and cast it to our structure.
	pBSA_ISMStatus = (BSA_ISMStatus*)pDdmData;
	
	m_DiskStatusRecord.num_error_replies_received = pBSA_ISMStatus->num_error_replies_received;

#if 0	
	m_DiskStatusRecord.num_recoverable_media_errors_no_delay 
		= pBSA_ISMStatus->num_recoverable_media_errors_no_delay;	// The number of recoverable disk media errors w/o delay
	m_DiskStatusRecord.num_recoverable_media_errors_delay
		= pBSA_ISMStatus->num_recoverable_media_errors_delay;		// The number of recoverable disk media errors with delay
	m_DiskStatusRecord.num_recoverable_media_errors_by_retry 
		= pBSA_ISMStatus->num_recoverable_media_errors_by_retry;	// The number of disk media errors recoverable w/retry.
	m_DiskStatusRecord.num_recoverable_media_errors_by_ecc  
		= pBSA_ISMStatus->num_recoverable_media_errors_by_ecc;		// The number of disk media errors recoverable w/ecc.
	m_DiskStatusRecord.num_recoverable_nonmedia_errors 
		= pBSA_ISMStatus->num_recoverable_nonmedia_errors;			// The number of non-disk media errors recoverable..
	m_DiskStatusRecord.num_bytes_processed_total 
		= pBSA_ISMStatus->num_bytes_processed_total;				// The total number of bytes processed.
	m_DiskStatusRecord.num_unrecoverable_media_errors 
		= pBSA_ISMStatus->num_unrecoverable_media_errors;			// The number of unrecoverable disk media errors.

#endif

	return OK;
}
	

/**********************************************************************
// ReturnTableRefreshData -  Returns the data to be refreshed to the 
// DiskStatusTable
**********************************************************************/
STATUS DiskStatusReporter::ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData)
{
//	Tracef("DiskStatusReporter::ReturnTableRefreshData()\n");
	m_DiskStatusRecord.key++;
	rpTableData = &m_DiskStatusRecord;
	rcbTableData = sizeof(m_DiskStatusRecord);

	// Note:  We do not clear the status counters for each Refresh.

	return	OK;
}
