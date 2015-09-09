/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: RmstrCmnds.h
// 
// Description:
// Defines the Rmstr interface for Commands/Status.
// 
// $Log: /Gemini/Odyssey/SSAPI_Server/Utils/RmstrCmnds.h $
// 
// 22    9/11/99 9:33p Agusev
// Update #pragma rules
// 
// 21    8/24/99 3:57p Dpatel
// added the array name row id, removed Unicode string.
// 
// 20    8/12/99 1:57p Dpatel
// Added array offline event processing code..
// 
// 19    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64.
// 
// 18    8/04/99 6:19p Dpatel
// Used Delete Array Struct for Break Hot copy..
// 
// 17    8/04/99 3:32p Dpatel
// Added break hot copy mirror..., removed delete policy.
// 
// 16    8/02/99 3:09p Dpatel
// 
// 15    7/30/99 5:37p Dpatel
// change preferred / src members..
// 
// 14    7/28/99 5:49p Dpatel
// Changed add member, remove member info..
// 
// 13    7/28/99 12:41p Dpatel
// modified the pecking order to enum
// 
// 12    7/28/99 12:35p Dpatel
// removed preferred member from Create array def 
// 
// 11    7/27/99 6:06p Dpatel
// Added primary.source member indexes for create array
// 
// 10    7/23/99 5:46p Dpatel
// Added internal cmds, added hot copy, changed commit spare etc.
// 
// 9     7/22/99 6:41p Dpatel
// Changed Create Array Definition
// 
// 8     7/20/99 6:50p Dpatel
// Create Array and Add member changes..
// 
// 7     7/16/99 10:31a Dpatel
// Added Down Member, Commit Spare and RMSTR_EVENT_INFO
// 
// 6     7/09/99 5:25p Dpatel
// 
// 5     7/06/99 4:59p Dpatel
// Delete Spare and Utility changes.
// 
// 4     6/30/99 11:15a Dpatel
// Changes for Abort Util and Change Priority.
// 
// 3     6/28/99 5:14p Dpatel
// Changes for implementation of Utility Cmds
// 
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/

#ifndef __RaidCmnds_h
#define __RaidCmnds_h

#include "CtTypes.h"
#include "TableMsgs.h"

#include "RaidDefs.h"
#include "RMgmtPolicies.h"
#include "ArrayDescriptor.h"
#include "RaidMemberTable.h"
#include "RaidUtilTable.h"


#pragma	pack(4)

#define RMSTR_CMD_QUEUE_TABLE "RMSTR_CMD_QUEUE_TABLE"

#define RMSTR_CMND_START		0x0000

/********************************************************************
*
* Commands sent by the upper layer of the System Master to the RAID
* agent (DDM) of the System Master
*
********************************************************************/

typedef enum
{
	RMSTR_CMND_CREATE_ARRAY = (RMSTR_CMND_START | 1),
	RMSTR_CMND_DELETE_ARRAY,
	RMSTR_CMND_CREATE_SPARE,
	RMSTR_CMND_DELETE_SPARE,
	RMSTR_CMND_START_UTIL,
	RMSTR_CMND_ABORT_UTIL,
	RMSTR_CMND_CHANGE_UTIL_PRIORITY,
	RMSTR_CMND_DOWN_A_MEMBER,
	RMSTR_CMND_ADD_MEMBER,
	RMSTR_CMND_REMOVE_MEMBER,
	RMSTR_CMND_CHANGE_SOURCE_MEMBER,
	RMSTR_CMND_CHANGE_PREFERRED_MEMBER,
	RMSTR_CMND_LAST_VALID				// insert all valid cmnds above this one
}	RMSTR_CMND; // RAID Master Cmnd


typedef struct
{
	rowID				targetRowId;	// target Array Descriptor id (may be null for Hotcopy)
	RAID_UTIL_NAME		utilityName;	// Init, Expand, Verify, Regenerate
	RAID_UTIL_PRIORITY	priority;		// 0 (lowest priority = more host I/Os)  (default = RAID_PRIORITY1)
										// to 10 = more utility I/Os
	U32					updateRate;		// update interval of PTS. (default = 1)
										// 1 = every %1, 5 = every %, 0 = only at completion
	RAID_UTIL_POLICIES	policy;
	// The following fields are to be used if you want to specify
	// exactly which members you want to use for the utility.
	// You will have to specify the "SpecifyMembersToRunOn" policy bit
	// By default,Raid Master will automatically select the source and destination members
	// Note: BKGD_INIT source destination members are always selected
	// automatically.
	U32					numberSourceMembers;
	U32					numberDestinationMembers;
	rowID				sourceMembers[MAX_ARRAY_MEMBERS];	
	rowID				destinationMembers[MAX_ARRAY_MEMBERS];
}	RMSTR_START_UTIL_INFO;



typedef struct
{
	RAID_LEVEL			raidLevel;			// rsmRAID0, rsmMIRRORED, rsmRAID5, nWAY_MIRROR
	rowID				arrayNameRowId;		// row id in table containing logical name for array
	RAID_CREATION_POLICIES	createPolicy;	// hot copy etc..
	U32					totalCapacity;		// in 512 byte blks (based on RAID level)
	U32					memberCapacity;		// in 512 byte blks (smallest member capacity)
	U32					dataBlockSize;		// in 512 byte blks (default = 64)
	U32					parityBlockSize;	// in 512 byte blks (default = dataBlockSize)
	RAID_ARRAY_POLICIES	arrayPolicy;		// do not write res secs etc.
	RAID_PECKING_ORDER	peckingOrder;		// rmstrHIGH_PRIORITY, rmstrMEDIUM_PRIORITY,
											// rmstrLOW_PRIORITY - Priority is pecking order
											// for cannibalization: lowest priority FT
											// arrays are targeted first	
	rowID				hostForSparePool;	// rowID of the host whose spare pool is to be 
											// used, if not specified general pool used
	U32					numberMembers;		// total no.of members upto MAX_ARRAY_MEMBERS
	U32					numberSpares;		// total no.of spares upto MAX_ARRAY_SPARES
	U32					preferredMemberIndex;	// index into the arrayMembers[] for the preferred
												// member to read from. eg SSD for preferred read
	U32					sourceMemberIndex;		// index into the arrayMembers[] for the primary member
												// to be specified as hot copy source for Hot copy mirror
												// or as primary for a regular mirror
	U32					hotCopyExportMemberIndex;	// index into array members for member to be exported
													// when hot copy completes (valid for auto break)
	RAID_UTIL_PRIORITY	hotCopyPriority;			// priority for hot copy
	rowID				arrayMembers[MAX_ARRAY_MEMBERS + MAX_ARRAY_SPARES];		// SRC row id	
}	RMSTR_CREATE_ARRAY_DEFINITION;




typedef struct
{
	RAID_DELETE_POLICIES	policy;
	rowID					arrayRowId;					// rowID in ADT
	rowID					hotCopyExportMemberRowId;	// in MDT
}	RMSTR_DELETE_ARRAY_INFO;


typedef struct
{
	RAID_SPARE_TYPE		spareType;	// DEDICATED, POOL, HOST_POOL
	rowID				spareId;	// rowID in the SRCT to use for spare
	rowID				arrayRowId;	// rowID in Array Desc.Table -for DEDICATED ONLY
	rowID				hostId;		// rowID in Host Table - for HOST POOL SPARE
}	RMSTR_CREATE_SPARE_INFO;

typedef struct
{
	rowID				spareId;	// rowID in the Spare Desc Table
}	RMSTR_DELETE_SPARE_INFO;



typedef struct {
	RAID_UTIL_ABORT_POLICIES	policy;		// start next regen. on abort
	rowID						utilRowId;	// util to abort
} RMSTR_ABORT_UTIL_INFO;


typedef struct
{
	RAID_UTIL_PRIORITY	newPriority;
	rowID				utilRowId;			// in UDT
}	RMSTR_CHANGE_PRIORITY_INFO;

typedef struct
{
		rowID		arrayRowId;				// in ADT
		rowID		memberRowId;			// in MDT
}	RMSTR_DOWN_A_MEMBER_INFO;


typedef struct
{
		rowID		arrayRowId;				// in ADT
		rowID		newMemberRowId;			// in SRCT
}	RMSTR_ADD_MEMBER_INFO;


typedef struct
{
		rowID		arrayRowId;				// in ADT
		rowID		memberRowId;			// in MDT	
} RMSTR_REMOVE_MEMBER_INFO;



typedef struct
{
		rowID		arrayRowId;				// in ADT
		rowID		newMemberRowId;			// in MDT	
} RMSTR_CHANGE_SOURCE_MEMBER_INFO, RMSTR_CHANGE_PREFERRED_MEMBER_INFO;


// This is for RMSTR internal use only
// There is no opcode for this command so it cannot be issued
typedef struct
{
		rowID		arrayRowId;
		rowID		spareRowId;
		rowID		memberRowId;
}	RMSTR_COMMIT_SPARE_INFO;

// This is for RMSTR internal use only
typedef struct
{
		rowID		arrayRowId;
		rowID		memberRowId;
		U32			reason;
}	RMSTR_PROCESS_MEMBER_DOWN_INFO, RMSTR_PROCESS_ARRAY_OFFLINE_INFO;

// This is for RMSTR internal use only
typedef struct
{
		rowID				utilRowId;
		U32					miscompareCount;
		RAID_UTIL_STATUS	reason;
}	RMSTR_PROCESS_STOP_UTIL_EVENT_INFO;


typedef union {
		RMSTR_CREATE_ARRAY_DEFINITION	createArrayDefinition;
		RMSTR_DELETE_ARRAY_INFO			deleteArrayInfo;
		RMSTR_CREATE_SPARE_INFO			createSpareInfo;
		RMSTR_DELETE_SPARE_INFO			deleteSpareInfo;
		RMSTR_START_UTIL_INFO			startUtilInfo;
		RMSTR_ABORT_UTIL_INFO			abortUtilInfo;
		RMSTR_CHANGE_PRIORITY_INFO		changePriorityInfo;
		RMSTR_DOWN_A_MEMBER_INFO		downAMemberInfo;
		RMSTR_ADD_MEMBER_INFO			addMemberInfo;
		RMSTR_REMOVE_MEMBER_INFO		removeMemberInfo;
		RMSTR_CHANGE_SOURCE_MEMBER_INFO		changeSourceMemberInfo;
		RMSTR_CHANGE_PREFERRED_MEMBER_INFO	changePreferredMemberInfo;
		RMSTR_COMMIT_SPARE_INFO				commitSpareInfo;			// internal use only
		RMSTR_PROCESS_MEMBER_DOWN_INFO		processMemberDownInfo;		// internal use only
		RMSTR_PROCESS_ARRAY_OFFLINE_INFO	processArrayOfflineInfo;	// internal use only
		RMSTR_PROCESS_STOP_UTIL_EVENT_INFO	processStopUtilEventInfo;	//internal use only
} RMSTR_CMND_PARAMETERS;


typedef struct {
	U32						opcode;
	RMSTR_CMND_PARAMETERS	cmdParams;
} RMSTR_CMND_INFO;

#endif