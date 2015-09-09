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
// This is the SSD Status Table
// 
// $Log: /Gemini/Include/CTTables/SSDStatusTable.h $
// 
// 3     10/08/99 9:17a Vnguyen
// Update tables to add additional performance and status counters.
// Fix bug with SSD Performance Table name.  Was using wrong string name.
// 
// 2     8/18/99 7:36a Vnguyen
// Fix a few typo errors.
// 
// 1     8/17/99 8:27a Vnguyen
// Initial check-in.
// 
// Update Log: 
//
/*************************************************************************/

#ifndef __SSDStatusTable_h
#define __SSDStatusTable_h

#include "CtTypes.h"
#include "PTSCommon.h"
#include "Odyssey.h"

#pragma	pack(4)

// Field definitions in DiskStatusTable.cpp
extern	const fieldDef	aSSDStatusTable_FieldDefs[];
extern	const U32		cbSSDStatusTable_FieldDefs;

#define CT_SSDST_TABLE 			"SSD_Status_Table"
#define CT_SSDST_TABLE_VERSION	1
#define	SSD_STATUS_TABLE 		CT_SSDST_TABLE
//
// The SSD Status Table contains one row for each disk.
// Describes information indicating errors, events, and health conditions. 
//
typedef struct {
	rowID	rid;									// rowID of this record.
	U32 	version;								// Version of DISK_EVENT_DATA record
	U32		size;									// Size of SSD_EVENT_DATA record in bytes.
	U32		key;									// For listen operation
	U32		RefreshRate;							// in microseconds.  Time to poll for data and update PTS
	rowID	ridSRCRecord;							// Row ID of  Storage Roll Call  entry 4 this device.
	
	U32		NumReplacementPagesAvailable;	// # of replacement pages available.  If this goes to 0: bad news.
	U32		PageTableSize;
	U32		PercentDirtyPages;
	
	
}  SSDStatusRecord, SSDStatusTable[];


#define	CT_SSDST_RID							"rid"
#define	CT_SSDST_VERSION						"version"
#define	CT_SSDST_SIZE							"size"
#define	CT_SSDST_KEY							"key"
#define	CT_SSDST_REFRESHRATE					"RefreshRate"
#define	CT_SSDST_RIDSRCRECORD					"ridSRCRecord"

#define	CT_SSDST_NUMREPLACEMENTPAGESAVAILABLE	"NumReplacementPagesAvailable"
#define	CT_SSDST_PAGETABLESIZE					"PageTableSize"
#define	CT_SSDST_PERCENTDIRTYPAGES				"PercentDirtyPages"

#endif  // __SSDStatusTable_h
