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
// This is the SCSI Target Server (STS) Status Table
// 
// $Log: /Gemini/Include/CTTables/STSStatTable.h $
// 
// 5     10/07/99 11:28a Vnguyen
// Add Status counters for number of error replies received and also # of
// errors that are generated internally in SCSI Target Server.
// 
// 4     9/14/99 5:36p Vnguyen
// Update performance and status counters to match the counters returned
// by the SCSI Target Server DDM.
// 
// 3     8/16/99 8:41a Vnguyen
// Fix up parameter declaration to pass in DID for the PHS Reporter class
// object to use.
// 
// 2     8/02/99 8:58a Vnguyen
// 
// 1     7/28/99 1:08p Vnguyen
// Initial check-in
// 
// Update Log: 
// 7/28/99 Vnguyen: Create file
/*************************************************************************/

#ifndef _STSStatTable_h
#define _STSStatTable_h

#include "CtTypes.h"
#include "PTSCommon.h"
#include "Odyssey.h"
#include "STSStat.h"

#pragma	pack(4)

// Field definitions in STSStatTable.cpp
extern	const fieldDef	aSTSStatTable_FieldDefs[];
extern	const U32		cbSTSStatTable_FieldDefs;

#define CT_STSST_TABLE_NAME "STS_Status_Table"
#define CT_STSST_TABLE_VERSION	1
#define	STS_STATUS_TABLE CT_STSST_TABLE_NAME
//
// The STS Status Table contains one row for each SCSI Target Server
// Describes information indicating errors, events, and health conditions. 
//
typedef struct {
	rowID	rid;							// rowID of this record.
	U32 	version;						// Version of STS Status Table record
	U32		size;							// Size of STS Status Table record in bytes.
	U32		key;							// used for Listen operation
	U32		RefreshRate;					// in microseconds
	rowID	ridExportRecord;				// Row ID of Export (Export Table)  entry 4 this device.
	
	I64		NumTimerTimeout;				// Number of timer timeout due to BSA command not responding
	I64		NumErrorRepliesReceived;		// The number of received replies that have status != 0.
	I64 	NumErrorRepliesSent;			// The total of # of errors received and errors generated internally.
	
	U32		SCSILogPages[SCSI_PAGE_SIZE];
	U32		NumDifferentialError[DIFFERENTIAL_ERROR_SIZE];	// minimum req'd by SCSI
	
}  STSStatRecord, STSStatTable[];


#define	CT_STSST_RID						"rid"
#define	CT_STSST_VERSION					"version"
#define	CT_STSST_SIZE						"size"
#define	CT_STSST_KEY						"key"
#define	CT_STSST_REFRESHRATE				"RefreshRate"
#define	CT_STSST_RIDEXPORTRECORD			"ridExportRecord"

#define	CT_STSST_NUMTIMERTIMEOUT			"NumTimerTimeout"
#define	CT_STSST_NUMERRORREPLIESRECEIVED	"NumErrorRepliesReceived"
#define	CT_STSST_NUMERRORREPLIESSENT		"NumErrorRepliesSent"
#define	CT_STSST_SCSILOGPAGES				"SCSILogPages"
#define	CT_STSST_NUMDIFFERENTIALERROR		"NumDifferentialError"

#endif  //_STSStatTable_h
