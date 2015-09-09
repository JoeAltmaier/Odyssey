/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidStatus.h
//
// Description:
//
// Raid Array Status PTS Structure used with the Performance / Health / Status
// system
//
// Update Log: 
//	5/20/99	Jerry Lane:	initial creation
//
//
/*************************************************************************/

#ifndef __ArrayStatusTable_h
#define __ArrayStatusTable_h


#include "CTTypes.h"
#include "RaidDefs.h"
#include "PtsCommon.h"

#pragma	pack(4)

// Field definitions in ArrayStatusTable.cpp
extern	const fieldDef	aArrayStatusTable_FieldDefs[];
extern	const U32		cbArrayStatusTable_FieldDefs;

#define CT_ARRAYST_TABLE 			"Array_Status_Table"
#define ARRAY_STATUS_TABLE 			CT_ARRAYST_TABLE
#define CT_ARRAYST_TABLE_VERSION	1


/********************************************************************
*
* Field Def constants
*
********************************************************************/

#define fdARRAYSTATUS_ROWID						"rid"			
#define fdARRAYSTATUS_VERSION					"version"			
#define fdARRAYSTATUS_SIZE						"size"		
#define fdARRAYSTATUS_KEY						"key"		
#define fdARRAYSTATUS_REFRESHRATE				"RefreshRate"		
#define fdARRAYSTATUS_RIDSRCRECORD				"ridSRCRecord"		
#define fdARRAYSTATUS_NUMRETRIES				"NumRetries"		
#define fdARRAYSTATUS_NUMRECOVEREDERRORS		"NumRecoveredErrors"		
#define fdARRAYSTATUS_NUMREASSIGNEDSUCCESS		"NumReassignedSuccess"		
#define fdARRAYSTATUS_NUMREASSIGNEDFAILED		"NumReassignedFailed"
#define fdARRAYSTATUS_NUMRAIDREASSIGNEDSUCCESS	"NumRaidReassignedSuccess"		
#define fdARRAYSTATUS_NUMRAIDREASSIGNEDFAILED	"NumRaidReassignedFailed"		
#define fdARRAYSTATUS_NUMRETRIESTOTAL			"NumRetriesTotal"
#define fdARRAYSTATUS_NUMRECOVEREDERRORSTOTAL	"NumRecoveredErrorsTotal"
#define fdARRAYSTATUS_NUMREASSIGNEDSUCCESSTOTAL	"NumReassignedSuccessTotal"
#define fdARRAYSTATUS_NUMREASSIGNEDFAILEDTOTAL	"NumReassignedFailedTotal"


typedef struct
{
	rowID	rid;									// rowID of this record.
	U32 	version;								// Version of ARRAY_STATUS record
	U32		size;									// Size of ARRAY_STATUS record in bytes.
	U32		key;									// For listen operation
	U32		RefreshRate;							// in microseconds
	rowID	ridSRCRecord;							// Row ID of Storage Roll Call  entry 4 this device.
	U32		NumRetries[MAX_ARRAY_MEMBERS];
	U32		NumRecoveredErrors[MAX_ARRAY_MEMBERS];
	U32		NumReassignedSuccess[MAX_ARRAY_MEMBERS];
	U32		NumReassignedFailed[MAX_ARRAY_MEMBERS];

	U32		NumRaidReassignedSuccess;
	U32		NumRaidReassignedFailed;
	
	U32		NumRetriesTotal;			// The sum of NumRetries
	U32		NumRecoveredErrorsTotal;	// The sum of NumRecoveredErrors 
	U32		NumReassignedSuccessTotal;	// The sum of NumReassignedSuccess
	U32		NumReassignedFailedTotal;	// The sum of NumReassignedFailed
} ArrayStatusRecord, ArrayStatusTable[];

#endif
