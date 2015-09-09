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
// This is the FCP Target Status Table
// 
// $Log: /Gemini/Include/CTTables/FCPTargetStatusTable.h $
// 
// 3     10/11/99 6:04p Vnguyen
// Add timestamp for Performance counters and # of errors received for
// Status counters.
// 
// 2     9/15/99 11:55a Vnguyen
// Change Uptime from U32 to I64.
// 
// 1     8/16/99 2:42p Vnguyen
// Initial Check-in.
// 
// Update Log: 
//
/*************************************************************************/

#ifndef __FCPTargetStatusTable_h
#define __FCPTargetStatusTable_h

#include "CtTypes.h"
#include "PTSCommon.h"
#include "Odyssey.h"


#pragma	pack(4)

// Field definitions in FCPTargetStatusTable.cpp
extern	const fieldDef	aFCPTargetStatusTable_FieldDefs[];
extern	const U32		cbFCPTargetStatusTable_FieldDefs;

#define CT_FCPTST_TABLE "FCPT_Status_Table"
#define CT_FCPTST_TABLE_VERSION	1
#define	FCPT_STATUS_TABLE CT_FCPTST_TABLE
//
// The FCPT Status Table contains one row for each FCP Target Driver
// Describes information indicating errors, events, and health conditions. 
//
typedef struct {
	rowID	rid;							// rowID of this record.
	U32 	version;						// Version of FCP Target Table record
	U32		size;							// Size of FCP Target Table record in bytes.
	U32		key;							// For listen operation
	U32		RefreshRate;					// in microseconds
	rowID	ridLoopDescriptorRecord;		// Row ID of Export (Export Table)  entry 4 this device.
	I64		UpTime;							// in microseconds.  Not being updated at this time.

	I64		NumErrorRepliesReceived;	// The number of received replies that are in error
										// It could be either read or write.
// The counters below are not being updated yet.  We need to wait for
// the loop monitor driver.									
	I64 	FCPTargetStateTable;
	I64 	DriverReadyStateFlag;		// Reset, Ready, Not Ready
	I64 	Errors;
	I64 	LoopDown;					// We can force the loop down if we want.
	I64 	TransferIncomplete;
	
}  FCPTargetStatusRecord, FCPTargetStatusTable[];


#define CT_FCPTST_RID					"rid"
#define CT_FCPTST_VERSION				"version"
#define CT_FCPTST_SIZE					"size"
#define CT_FCPTST_KEY					"key"
#define CT_FCPTST_REFRESHRATE			"RefreshRate"
#define CT_FCPTST_RIDLDRECORD			"ridLoopDescriptorRecord"
#define CT_FCPTST_UPTIME				"UpTime"

#define CT_FCPTST_NUMERRORREPLIESRECEIVED	"NumErrorRepliesReceived"

#define CT_FCPTST_FCPTARGETSTATETABLE	"FCPTargetStateTable"
#define CT_FCPTST_DRIVERREADYSTATEFLAG 	"DriverReadyStateFlag"	// Reset, Ready, Not Ready
#define CT_FCPTST_ERRORS 				"Errors"
#define CT_FCPTST_LOOPDOWN 				"LoopDown"				// We can force the loop down if we want.
#define CT_FCPTST_TRANSFERINCOMPLETE 	"TransferIncomplete"

#endif  //__FCPTargetStatusTable_h
