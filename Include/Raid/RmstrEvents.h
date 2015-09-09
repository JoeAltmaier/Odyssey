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
// File: RmstrEvents.h
// 
// Description:
// Defines the Rmstr interface for Commands/Status.
// 
// $Log: /Gemini/Odyssey/SSAPI_Server/Utils/RmstrEvents.h $
// 
// 13    9/11/99 9:33p Agusev
// Update #pragma rules
// 
// 12    8/12/99 1:57p Dpatel
// Added array offline event processing code..
// 
// 11    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64.
// 
// 10    8/02/99 3:09p Dpatel
// 
// 9     7/30/99 5:37p Dpatel
// change preferred / src members..
// 
// 8     7/28/99 2:32p Dpatel
// Passed StorageRollCall data instead of row id in create array event.
// 
// 7     7/20/99 6:50p Dpatel
// Create Array and Add member changes..
// 
// 6     7/16/99 10:31a Dpatel
// Added Down Member, Commit Spare and RMSTR_EVENT_INFO
// 
// 5     7/09/99 5:25p Dpatel
// 
// 4     7/06/99 4:59p Dpatel
// Delete Spare and Utility changes.
// 
// 3     6/30/99 11:15a Dpatel
// Changes for Abort Util and Change Priority.
// 
// 2     6/28/99 5:14p Dpatel
// Changes for implementation of Utility Cmds
// 
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/

#ifndef __RmstrEvents_h
#define __RmstrEvents_h

#include "CtTypes.h"
#include "TableMsgs.h"

#include "RaidDefs.h"
#include "RMgmtPolicies.h"
#include "ArrayDescriptor.h"
#include "RaidMemberTable.h"
#include "RaidUtilTable.h"
#include "RaidSpareDescriptor.h"

#include "StorageRollCallTable.h"
#include "UnicodeString.h"

#pragma	pack(4)

#define RMSTR_EVT_START			0x000
typedef enum 
{
	RMSTR_EVT_ARRAY_ADDED = (RMSTR_EVT_START | 1),
	RMSTR_EVT_ARRAY_DELETED,
	RMSTR_EVT_UTIL_STARTED,
	RMSTR_EVT_UTIL_ABORTED,
	RMSTR_EVT_UTIL_STOPPED,
	RMSTR_EVT_UTIL_PRIORITY_CHANGED,
	RMSTR_EVT_UTIL_PERCENT_COMPLETE,  // [ Not reported to System Log]
	RMSTR_EVT_SPARE_ADDED,
	RMSTR_EVT_SPARE_DELETED,
	RMSTR_EVT_MEMBER_DOWN,
	RMSTR_EVT_ARRAY_CRITICAL,
	RMSTR_EVT_ARRAY_OFFLINE, 
	RMSTR_EVT_ARRAY_FAULT_TOLERANT,
	RMSTR_EVT_SPARE_ACTIVATED,
	RMSTR_EVT_FLAKY_DRIVE,	// [Who reports to System Log? How reports this event to RM?] 
	RMSTR_EVT_MEMBER_ADDED,
	RMSTR_EVT_MEMBER_REMOVED,
	RMSTR_EVT_SOURCE_MEMBER_CHANGED,
	RMSTR_EVT_PREFERRED_MEMBER_CHANGED 
} RMSTR_EVENT;



typedef struct {
	StorageRollCallRecord		SRCData;	
	RAID_ARRAY_DESCRIPTOR		arrayData;
	UnicodeString32				arrayName;
} RMSTR_EVT_ARRAY_ADDED_STATUS;

typedef struct {
	rowID						SRCTRowId;	
	RAID_ARRAY_DESCRIPTOR		arrayData;
} RMSTR_EVT_ARRAY_DELETED_STATUS;


typedef struct {
	RAID_ARRAY_DESCRIPTOR		arrayData;
} RMSTR_EVT_ARRAY_CRITICAL_STATUS;



typedef struct {
	RAID_ARRAY_UTILITY		utilityData;
} RMSTR_EVT_UTIL_STARTED_STATUS;


typedef struct {
	RAID_ARRAY_UTILITY		utilityData;
	RAID_UTIL_STATUS		reason;		// reason for stopping
	U32						miscompareCount;
} RMSTR_EVT_UTIL_STOPPED_STATUS;


typedef struct {
	RAID_ARRAY_UTILITY	utilityData;
	U32					percentComplete;
} RMSTR_PERCENT_COMPLETE_STATUS;


typedef struct {
	RAID_ARRAY_UTILITY	utilityData;
	RAID_UTIL_PRIORITY	oldPriority;
} RMSTR_PRIORITY_CHANGED_STATUS;


typedef struct {
	RAID_SPARE_DESCRIPTOR		spareData;
} RMSTR_EVT_SPARE_ADDED_STATUS, RMSTR_EVT_SPARE_DELETED_STATUS, RMSTR_EVT_SPARE_ACTIVATED_STATUS;


typedef struct {
	RAID_ARRAY_MEMBER		memberData;
} RMSTR_EVT_MEMBER_DOWN_STATUS, RMSTR_EVT_MEMBER_ADDED_STATUS, RMSTR_EVT_MEMBER_REMOVED_STATUS;

typedef struct {
	RAID_ARRAY_DESCRIPTOR	arrayData;		// array that is offline
	RAID_ARRAY_MEMBER		memberData;		// down member, which caused the array to go offline
} RMSTR_EVT_ARRAY_OFFLINE_STATUS;


typedef struct {
	RAID_ARRAY_DESCRIPTOR		arrayData;
} RMSTR_EVT_SOURCE_MEMBER_CHANGED_STATUS, RMSTR_EVT_PREFERRED_MEMBER_CHANGED_STATUS;

typedef struct {
	RAID_ARRAY_DESCRIPTOR		arrayData;
	RAID_MEMBER_STATUS			memberHealth[MAX_ARRAY_MEMBERS];
} RMSTR_EVT_ARRAY_FAULT_TOLERANT_STATUS;


typedef union {
	RMSTR_EVT_ARRAY_ADDED_STATUS		arrayAddedStatus;
	RMSTR_EVT_ARRAY_DELETED_STATUS		arrayDeletedStatus;
	RMSTR_EVT_ARRAY_CRITICAL_STATUS		arrayCriticalStatus;
	RMSTR_EVT_ARRAY_OFFLINE_STATUS		arrayOfflineStatus;
	RMSTR_EVT_ARRAY_FAULT_TOLERANT_STATUS		arrayFaultTolerantStatus;

	RMSTR_EVT_UTIL_STARTED_STATUS		utilStartedStatus;
	RMSTR_EVT_UTIL_STOPPED_STATUS		utilStoppedStatus;
	RMSTR_PERCENT_COMPLETE_STATUS		percentCompleteStatus;
	RMSTR_PRIORITY_CHANGED_STATUS		priorityChangedStatus;

	RMSTR_EVT_SPARE_ADDED_STATUS		spareAddedStatus;
	RMSTR_EVT_SPARE_DELETED_STATUS		spareDeletedStatus;
	RMSTR_EVT_SPARE_ACTIVATED_STATUS	spareActivatedStatus;

	RMSTR_EVT_MEMBER_DOWN_STATUS		memberDownStatus;
	RMSTR_EVT_MEMBER_ADDED_STATUS		memberAddedStatus;
	RMSTR_EVT_MEMBER_REMOVED_STATUS		memberRemovedStatus;
	RMSTR_EVT_SOURCE_MEMBER_CHANGED_STATUS		sourceMemberChangedStatus;
	RMSTR_EVT_PREFERRED_MEMBER_CHANGED_STATUS	preferredMemberChangedStatus;
} RMSTR_EVENT_INFO;

#endif
