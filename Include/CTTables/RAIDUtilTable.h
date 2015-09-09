/*************************************************************************
*
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* Description:
* This defines the entries for the running RAID Utility Table
* 
* Update Log: 
* 3/99 Ron Parks: Create file
*
*************************************************************************/

#ifndef __RaidUtilTable_h
#define __RaidUtilTable_h


#include "CtTypes.h"
#include "TableMsgs.h"

#include "RMgmtPolicies.h"
#include "RaidDefs.h"

#pragma	pack(4)


#define RAID_UTIL_DESCRIPTOR_TABLE "RAID UTILITY DESCRIPTOR TBL"
#define RAID_UTIL_DESCRIPTOR_TABLE_VERSION		1

extern const fieldDef UtilDescriptorTable_FieldDefs[];
extern const U32 sizeofUtilDescriptorTable_FieldDefs;

/********************************************************************
*
* Field Def constants
*
********************************************************************/

#define fdUDT_VERSION				"UDTVersion"
#define fdUDT_SIZE					"UDTSize"

#define fdTARGET_ARRAY				"TargetArray"
#define fdSTART_TIME				"StartTime"
#define fdUPDATE_RATE				"UpDateRate"
#define fdPROGRESS					"Progress"
#define fdUTIL_CODE					"Utility"
#define fdPRIORITY					"Priority"
#define fdUTIL_STATUS				"Status"
#define fdPERCENT_UPDATE_RATE		"PercentCompleteUpdateRate"
#define fdPERCENT_DONE				"PercentDone"
#define fdUTIL_STATUS				"Status"
#define	fdSOURCE_ROWIDS				"SourceRids"
#define fdDESTINATION_ROWIDS		"DestRids"
#define fdSOURCE_MEM_MASK			"Source"
#define fdDEST_MEM_MASK				"Dest"
#define fdCURRENT_LBA				"LBA"
#define fdENDUTIL_LBA 				"EndLBA"
#define fdPOLICY					"Policy"


/********************************************************************
*
* Information for a utility currently running on an array
*
********************************************************************/

typedef struct
{
	rowID				thisRID;		// RID in this table
	U32					version;
	U32					size;
	rowID				targetRID;		// target Array Descriptor id
	U32					startTime;		// timestamp when this util started
	U32					updateRate;		// update interval of PTS. (default = 1)
										// 1 = every %1, 5 = every %, 0 = only at completion
										// used by RAID DDM only
	RAID_UTIL_NAME		utilityCode;
	RAID_UTIL_PRIORITY	priority;		// 0 (lowest priority = more host I/Os)  (default = RAID_PRIORITY1)
										// to 10 = more utility I/Os
	RAID_UTIL_STATUS	status;			// Running, Suspended,NotRunning,Completed,Aborted, Aborted I/O err
	U32					percentCompleteUpdateRate;	// how often to report % complete
													// 1 = every %1, 5 = every %, 0 = only at completion
	U32					percentComplete;	// current percent completed
	rowID				sourceRowIds[MAX_ARRAY_MEMBERS];		// SRCT rid
	rowID				destinationRowIds[MAX_ARRAY_MEMBERS];	// SRCTRids
	I64					currentLBA;		// set to 0 by RMSTR, RAID DDM updates it
	I64					endLBA;
	RAID_UTIL_POLICIES	policy;
	STATE_IDENTIFIER	stateIdentifier;
}	RAID_ARRAY_UTILITY, RAIDArrayUtilTable[];


#endif