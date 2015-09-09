/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ArrayStatusReporter.cpp
// 
// Description:
// This file implements the ArrayStatusReporter Reporter class. 
// 
// Update Log 
// 
// $Log: /Gemini/Odyssey/DdmReporter/ArrayStatusReporter.cpp $
// 
// 6     12/15/99 2:04p Vnguyen
// Move Tracef.  Also increment key counter for each update.  This way we
// can tell when an update just happens and see if the info is propogated
// to GMI GUI or not.
// 
// 5     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 4     11/06/99 3:05p Vnguyen
// Add four status counters for errors.  These new counters are the sum of
// the error arrays.  The benefit is that we can return the sum to the
// customers and keep the detailed error counters for performance tuning.
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
/*************************************************************************/

#include "ArrayStatusReporter.h"
#include "RaidStat.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "DefaultRates.h"

/**********************************************************************/
// Initialize - This is the initializor for the ArrayStatusReporter Object.
// We've been called because the PHSReporter Ddm has received a reply to
// a listen operation he posted on the Storage Roll Call table and now 
// he (the PHS Ddm) wants to instantiate (and initialize) a new reporter
// object for the SRC entry.  Our Job, as the is to make sure that our
// status table and record are created and if not, create them.    
/**********************************************************************/
STATUS ArrayStatusReporter::Initialize(
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

	strcpy(m_Data.PHSTableName, ARRAY_STATUS_TABLE);		 	// Name of table to send PHS data to
	m_Data.aPHSTableFieldDefs = (fieldDef *)aArrayStatusTable_FieldDefs;	// FieldDefs for PHS table 
	m_Data.cbPHSTableFieldDefs = cbArrayStatusTable_FieldDefs;	// size of PHS table fieldDefs, U32 cbrgFieldDefs

	m_Data.pPHSRecord = &m_ArrayStatusRecord;		// A pointer to a buffer for a PHS row record
	m_Data.cbPHSRecord = sizeof(m_ArrayStatusRecord);	// Size of a PHS row record, cbRowData
	m_Data.cbPHSRecordMax = sizeof(m_ArrayStatusRecord); // Maximum size of buffer to receive row data, cbRowDataRetMax

	m_Data.pRefreshRate = &m_ArrayStatusRecord.RefreshRate;
	m_Data.pSampleRate = &m_ArrayStatusRecord.RefreshRate;  // For status reporter, use Refresh rate for Sample rate.
													
	// Up up and away...
	status = InitSRCandExport(&m_Data, didDdm, vdnDdm, ReporterCode);
	return status;
}
 

/**********************************************************************/
// InitializePHSRecord - This method fills in the PHS record with default
// values.  The RowID for the Storage Roll Call Table entry is also
// put in here.  
/**********************************************************************/
void ArrayStatusReporter::InitializePHSRecord(
 		void *pPHSRecord, 
 		rowID *pSRCandExportRowID
 )
 {
 	ArrayStatusRecord	*pArrayStatusRecord;
 	pArrayStatusRecord = (ArrayStatusRecord *) pPHSRecord;
 	
 	// Declare and initialize a Disk Status Record with default values. 
			ArrayStatusRecord	newArrayStatusRecord = {
		// 	Default Value				FieldName									  Size   Type
			0,0,0,						// "rid"										8,	I64_FT,
			CT_ARRAYST_TABLE_VERSION,	// "Version",									4,	U32_FT,
			sizeof(ArrayStatusRecord),	// "Size",										4,	U32_FT,
			0,							// "Key",										4,	U32_FT,
			ARRAY_STATUS_REFRESH_RATE,	// "RefreshRate",								4,	U32_FT,
			0,0,0,						// rowID ridSRCRecord;

			0,0,0,0,0,0,0,0,			// NumRetries[MAX_ARRAY_MEMBERS]; 32 entries
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
									
			0,0,0,0,0,0,0,0,			// NumRecoveredErrors[MAX_ARRAY_MEMBERS];
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,

			0,0,0,0,0,0,0,0,			// NumReassignedSuccess[MAX_ARRAY_MEMBERS];
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,

			0,0,0,0,0,0,0,0,			// NumReassignedFailed[MAX_ARRAY_MEMBERS];
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			
			0,							// NumRaidReassignedSuccess;
			0,							// NumRaidReassignedFailed;	

			0,							// NumRetriesTotal;
			0,							// NumRecoveredErrorsTotal;
			0,							// NumReassignedSuccessTotal;
			0							// NumReassignedFailedTotal;

			
		};
		
	*pArrayStatusRecord = newArrayStatusRecord;
 	pArrayStatusRecord->ridSRCRecord = *pSRCandExportRowID;
 	return;	
 }
 




/**********************************************************************/
// HandleDdmSampleDataReply -  Handle the Status data retruned from the Ddm.
/**********************************************************************/
STATUS ArrayStatusReporter::HandleDdmSampleData( void* pDdmData, U32 cbDdmData )
{
#pragma unused(cbDdmData)
RAID_STATUS		*pRAIDStatus;
STATUS			status = OK;

	// Tracef("ArrayStatusReporter::HandleDdmSampleData()\n");

	// Get a pointer to the Ddm's reply and cast it to our structure.
	pRAIDStatus = (RAID_STATUS*)pDdmData;
	
	// Here is where we map the data from DDM Fromat to PTS Format.

	/**********************************************************************
	//	DDM Format:
	typedef struct
	{
		U32		NumRetries[MAX_ARRAY_MEMBERS];
		U32		NumRecoveredErrors[MAX_ARRAY_MEMBERS];
		U32		NumReassignedSuccess[MAX_ARRAY_MEMBERS];
		U32		NumReassignedFailed[MAX_ARRAY_MEMBERS];
		U32		NumRaidReassignedSuccess;
		U32		NumRaidReassignedFailed;
	} RAID_STATUS;
	
	//	PTS Format:
	typedef struct
	{
		rowID	rid;									// rowID of this record.
		U32 	version;								// Version of DISK_EVENT_DATA record
		U32		size;									// Size of DISK_EVENT_DATA record in bytes.
		U32		key;									// ala Rick Currently unused, could hold Circuit Key.
		U32		RefreshRate;							// 0 => refresh once >0 implies refresh every x seconds?    
		rowID	ridSRCRecord;							// Row ID of  Storage Roll Call  entry 4 this device.
		U32		NumRetries[MAX_ARRAY_MEMBERS];
		U32		NumRecoveredErrors[MAX_ARRAY_MEMBERS];
		U32		NumReassignedSuccess[MAX_ARRAY_MEMBERS];
		U32		NumReassignedFailed[MAX_ARRAY_MEMBERS];
		U32		NumRaidReassignedSuccess;
		U32		NumRaidReassignedFailed;
	} Array_Status_Record, Array_Status_Table[];
	**********************************************************************/
	
	// I'm thinking in the interest of optimization just copy the data,
	//  rather than making the equivalent assignments.
	memcpy(
		&m_ArrayStatusRecord.NumRetries[0], 	// destination
		&pRAIDStatus->NumRetries[0],			// source
		sizeof(m_ArrayStatusRecord.NumRetries) +
		sizeof(m_ArrayStatusRecord.NumRecoveredErrors) +
		sizeof(m_ArrayStatusRecord.NumReassignedSuccess) +
		sizeof(m_ArrayStatusRecord.NumReassignedFailed) +
		sizeof(m_ArrayStatusRecord.NumRaidReassignedSuccess) +
		sizeof(m_ArrayStatusRecord.NumRaidReassignedFailed)			// count of bytes.
	);
	
	return OK;
}
	

/**********************************************************************
// ReturnTableRefreshData -  Returns the data to be refreshed to the 
// ArrayStatusTable
**********************************************************************/
STATUS ArrayStatusReporter::ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData)
{
	// Tracef("ArrayStatusReporter::ReturnTableRefreshData()\n");

	m_ArrayStatusRecord.NumRetriesTotal = 0;
	m_ArrayStatusRecord.NumRecoveredErrorsTotal = 0;
	m_ArrayStatusRecord.NumReassignedSuccessTotal = 0;
	m_ArrayStatusRecord.NumReassignedFailedTotal = 0;
	for (int i = 0; i <MAX_ARRAY_MEMBERS; i++)
	{
		m_ArrayStatusRecord.NumRetriesTotal += m_ArrayStatusRecord.NumRetries[i];
		m_ArrayStatusRecord.NumRecoveredErrorsTotal += m_ArrayStatusRecord.NumRecoveredErrors[i];
		m_ArrayStatusRecord.NumReassignedSuccessTotal += m_ArrayStatusRecord.NumReassignedSuccess[i];
		m_ArrayStatusRecord.NumReassignedFailedTotal += m_ArrayStatusRecord.NumReassignedFailed[i];
	}
	
	m_ArrayStatusRecord.key++;
	rpTableData = &m_ArrayStatusRecord;
	rcbTableData = sizeof(m_ArrayStatusRecord);

	return	OK;


}
