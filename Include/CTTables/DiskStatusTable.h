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
// This is the Disk Status Table
// 
// $Log: /Gemini/Include/CTTables/DiskStatusTable.h $
// 
// 6     12/14/99 3:47p Vnguyen
// Add missing table field for disk ECC error.  This sets the field data
// to zero for display.  Eventually, we can supply the correct value by
// reading the hard drive directly.
// 
// 5     9/08/99 10:46a Vnguyen
// Update performance and status counters.  Also, change performance
// counters from per sample to per second.
// 
// 4     8/16/99 12:54p Vnguyen
// Fix various errors during compiling.  Mostly typo and mis-match
// parameters.
// 
// 3     5/04/99 9:36a Jlane
// Miscellaneous updates to get compiuling and to bring in synch with new
// Ddm Model.
// 
// 2     4/26/99 5:37p Jlane
// Changed ridThisRow to rid.
//
// Update Log: 
// 12/31/98 Michael G. Panas: Create file
/*************************************************************************/

#ifndef __DiskStatusTable_h
#define __DiskStatusTable_h

#include "CtTypes.h"
#include "PTSCommon.h"
#include "Odyssey.h"

#pragma	pack(4)

// Field definitions in DiskStatusTable.cpp
extern	const fieldDef	aDiskStatusTable_FieldDefs[];
extern	const U32		cbDiskStatusTable_FieldDefs;

#define CT_DST_TABLE_NAME 		"Disk_Status_Table"
#define CT_DST_TABLE_VERSION	1
#define	DISK_STATUS_TABLE CT_DST_TABLE_NAME
//
// The Disk Status Table contains one row for each disk.
// Describes information indicating errors, events, and health conditions. 
//
typedef struct {
	rowID	rid;									// rowID of this record.
	U32 	version;								// Version of DISK_EVENT_DATA record
	U32		size;									// Size of DISK_EVENT_DATA record in bytes.
	U32		key;									// for listen operation
	U32		RefreshRate;							// in microseconds
	rowID	ridSRCRecord;							// Row ID of  Storage Roll Call  entry 4 this device.
	U32		num_error_replies_received;
	U32		num_recoverable_media_errors_no_delay;	// The number of recoverable disk media errors w/o delay
	U32		num_recoverable_media_errors_delay;		// The number of recoverable disk media errors with delay
	U32		num_recoverable_media_errors_by_retry;	// The number of disk media errors recoverable w/retry.
	U32		num_recoverable_media_errors_by_ecc;	// The number of disk media errors recoverable w/ecc.
	U32		num_recoverable_nonmedia_errors;		// The number of non-disk media errors recoverable..
	U32		num_bytes_processed_total;				// The total number of bytes processed.
	U32		num_unrecoverable_media_errors;			// The number of unrecoverable disk media errors.
}  DiskStatusRecord, DiskStatusTable[];


#define	CT_DST_RID										"rid"
#define	CT_DST_VERSION									"version"
#define	CT_DST_SIZE										"size"
#define	CT_DST_KEY										"key"
#define	CT_DST_REFRESHRATE								"RefreshRate"
#define	CT_DST_RIDSRCRECORD								"ridSRCRecord"
#define	CT_DST_NUM_ERROR_REPLIES_RECEIVED				"num_error_replies_received"
#define	CT_DST_NUM_RECOVERABLE_MEDIA_ERRORS_NO_DELAY	"num_recoverable_media_errors_no_delay"
#define	CT_DST_NUM_RECOVERABLE_MEDIA_ERRORS_DELAY		"num_recoverable_media_errors_delay"
#define	CT_DST_NUM_RECOVERABLE_MEDIA_ERRORS_BY_RETRY	"num_recoverable_media_errors_by_retry"
#define	CT_DST_NUM_RECOVERABLE_MEDIA_ERRORS_BY_ECC		"num_recoverable_media_errors_by_ecc"
#define	CT_DST_NUM_RECOVERABLE_NONMEDIA_ERRORS			"num_recoverable_nonmedia_errors"
#define	CT_DST_NUM_BYTES_PROCESSED_TOTAL				"num_bytes_processed_total"
#define	CT_DST_NUM_UNRECOVERABLE_MEDIA_ERRORS			"num_unrecoverable_media_errors"

#endif