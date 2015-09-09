/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidCommands.h
//
// Description:	
// 		Definitions for RAID Commands and Status and Events
//
//
// Update Log: 
//	6/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __RaidCommands_h
#define __RaidCommands_h

#include "RaidDefs.h"
#include "RMgmtPolicies.h"
#include "ArrayDescriptor.h"
#include "RaidMemberTable.h"
#include "RaidUtilTable.h"


#pragma	pack(4)


#define RAID_COMMAND_QUEUE	"RaidCommandQueue"

struct RaidSetPriorityStruct
{
	rowID				UtilRowID;		// row id of utility in utility table
	RAID_UTIL_PRIORITY	Priority;		// new priority
};

struct RaidAbortStruct
{
	rowID				UtilRowID;		// row id of utility in utility table to abort
};

struct RaidDownMemberStruct
{
	rowID				MemberRowID; 	// row id of member in member table to down
};

struct RaidRemoveMemberStruct
{
	rowID				MemberRowID; 	// row id of member in member table to remove
};

struct RaidReplaceMemberStruct
{
	rowID				OldMemberRowID;	// row id of member being replaced
	RAID_ARRAY_MEMBER	Member;			// replacement member
};

struct RaidAddMemberStruct
{
	RAID_ARRAY_MEMBER	Member;			// member to add
};



//
// RAID Request received from CmdServer
//

struct RaidRequest
{
	VDN		RaidVDN;
	STATUS	Opcode;
	union
	{
		RaidSetPriorityStruct		PriorityData;
		RaidAbortStruct				AbortData;
		RaidDownMemberStruct		DownData;
		RAID_ARRAY_UTILITY			UtilityData;
		RAID_ARRAY_DESCRIPTOR		ArrayData;		// temp
		RaidReplaceMemberStruct		ReplaceData;
		RaidAddMemberStruct			AddData;
		RaidRemoveMemberStruct		RemoveData;
	} Data;
};


struct RaidStatus
{
	U32		Status;
	rowID	UtilRowID;
};

//
// Status' reported to CmdServer
//

#define	RAIDCMD_STATUS_SUCCESS					0x0000
#define	RAIDCMD_STATUS_BUSY						0x0001
#define	RAIDCMD_STATUS_INAPPROPRIATE_CMD		0x0002
#define	RAIDCMD_STATUS_INSUFFICIENT_RESOURCE  	0x0003


// Levels for Read Preference 
#define	READ_PREFERENCE_HIGH					0x0000
#define	READ_PREFERENCE_MEDIUM					0x0001
#define	READ_PREFERENCE_LOW						0x0002


//
// RAID Event structures
//

struct RaidStartUtilEvent
{
	rowID	UtilRowID;
};

struct RaidStopUtilEvent
{
	rowID				UtilRowID;
	U32					MiscompareCnt;		// for verify
	RAID_UTIL_STATUS	Reason;				// defined in RaidDefs.h
};

struct RaidPercentEvent
{
	rowID	UtilRowID;
	U32		Percent;
};

struct RaidMemberDownEvent
{
	rowID	ArrayRowID;
	rowID	MemberRowID;
	U32		Reason;
};

struct RaidOfflineEvent
{
	rowID	ArrayRowID;
	rowID	MemberRowID;
	U32		Reason;
};


//
// RAID Event reported to CmdServer
//

struct RaidEvent
{
	union
	{
		RaidStartUtilEvent	StartUtil;
		RaidStopUtilEvent	StopUtil;
		RaidPercentEvent	PercentUtil;
		RaidMemberDownEvent	MemberDown;
		RaidOfflineEvent	RaidOffline;
	} Event;
};


//
// RAID Commands sent to the RAID DDM
//

typedef enum
{
	RAID_REQUEST_START_UTIL = 1,
	RAID_REQUEST_ABORT_UTIL,
	RAID_REQUEST_DOWN_MEMBER,
	RAID_REQUEST_CHG_PRIORITY,
	RAID_REQUEST_REPLACE_MEMBER,
	RAID_REQUEST_REMOVE_MEMBER,				// for RAID 1
	RAID_REQUEST_ADD_MEMBER					// for RAID 1
} RAID_CMND;


//
// RAID Events posted by the RAID DDM
//

typedef enum
{
	RAID_EVT_UTIL_STARTED = 100,
	RAID_EVT_UTIL_STOPPED,
	RAID_EVT_UTIL_PERCENT_COMPLETE,
	RAID_EVT_MEMBER_DOWN,
	RAID_EVT_ENABLED,					// temp
	RAID_EVT_ARRAY_OFFLINE
} RAID_EVENT;


#endif
