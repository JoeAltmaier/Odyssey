/*************************************************************************
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* File Name:
* RaidDefs.h
*
* Description:
* This file defines 	- many of the commnands sent to the RAID DDM
*						- component parts of the array descriptor
*
* 
* Update Log: 
* 2/99		Ron Parks: Initial creation
*
*************************************************************************/

#ifndef __RaidDefs_h
#define __RaidDefs_h

#include "CtTypes.h"
#include "RequestCodes.h"


#define MAX_ARRAY_MEMBERS		32	// max num members in a given array, if change this #, need to update ArrayStatusReporter.cpp
#define MAX_ARRAY_SPARES		MAX_ARRAY_MEMBERS/2	// max num dedicated spares 
#define MAX_UTILS				MAX_ARRAY_MEMBERS/2	// max num utils on a given array

#pragma	pack(4)


/********************************************************************
*
*	UTILITIES 
*
********************************************************************/

typedef enum
{
	RAID_UTIL_NONE = 0,
	RAID_UTIL_VERIFY = 1,
	RAID_UTIL_REGENERATE,
	RAID_UTIL_LUN_HOTCOPY,
	RAID_UTIL_MEMBER_HOTCOPY,
	RAID_UTIL_BKGD_INIT,
	RAID_UTIL_EXPAND,
	LAST_UTIL_NAME	// insert all commands above this point
}	RAID_UTIL_NAME;

typedef enum
{
	PECK_FIRST =1,
	PECK_LAST,
	NEVER_PECK
} RAID_PECKING_ORDER;

typedef enum
{
	RAID_UTIL_NOT_RUNNING = 0,
	RAID_UTIL_RUNNING,
	RAID_UTIL_SUSPENDED,	
	RAID_UTIL_ABORTED,
	RAID_UTIL_ABORTED_IOERROR,
	RAID_UTIL_COMPLETE
}	RAID_UTIL_STATUS;

typedef enum
{
	RAID_INIT_NOT_STARTED = 0,		// set before issuing cmd to RAID DDM
	RAID_INIT_RUNNING,
	RAID_INIT_COMPLETE,	
	RAID_INIT_ERROR
}	RAID_INIT_STATUS;


typedef enum
{
	PRIORITY_LOW = 3, // lowest priority.
	PRIORITY_MEDIUM = 6,
	PRIORITY_HIGH = 9
}	RAID_UTIL_PRIORITY;



/********************************************************************
*
*	ARRAY'S 
*
********************************************************************/

//ARRAY STATUS
typedef enum
{
	RAID_OFFLINE = 1,		// accepts no I/O
	RAID_CRITICAL,			// the next drive failure puts array offline
	RAID_FAULT_TOLERANT, 	// a drive may fail without going offline
	RAID_VULNERABLE,		// dependin on which drive fails, the next
							// failure could put the array offline
	RAID_OKAY,				// raid0 is functional
	LAST_RAID_STATUS		// insert all commands above this point
}	RAID_ARRAY_STATUS;



// Request Queueing Methods
typedef enum
{
	RAID_QUEUE_FIFO = 1,
	RAID_QUEUE_ELEVATOR,
	LAST_METHOD			// insert all commands above this point
}	RAID_QUEUE_METHOD;

//Array Member Status
typedef enum
{
	RAID_STATUS_UP = 1,			// fully operational
	RAID_STATUS_DOWN,			// all data is not present
	RAID_STATUS_EMPTY,			// destination in a HotCopy
	RAID_STATUS_REGENERATING,	// destination in a HotCopy
	LAST_STATUS					// insert all commands above this point
}	RAID_MEMBER_STATUS;

//ARRAY_STATS
typedef struct
{
	U64				NumReads[12]; 	// 512,1k,2k,4k,8k,16,32k,64k,128k,256k,512k, >	
	U64				NumWrites[12];	// 512,1k,2k,4k,8k,16,32k,64k,128k,256k,512k, >
	U64				BlocksRead;
	U64				BlocksWritten;
	U64				CombinedReads;
	U64				CombinedWrites;
} RAID_ARRAY_STATS;


// RAID Levels
typedef enum
{
	RAID0 = 1,
	RAID1,
	RAIDCAT,
	RAID3,
	RAID4,
	RAID5,
	INVALID_RAID_LEVEL
}	RAID_LEVEL;



/********************************************************************
*
*	SPARES
*
********************************************************************/
typedef enum
{
	RAID_DEDICATED_SPARE = 1,	// dedicated to a specific array
	RAID_HOST_POOL_SPARE,		// dedicated for arrays of a specific host
	RAID_GENERAL_POOL_SPARE		// use for any array on any host
}	RAID_SPARE_TYPE;



/********************************************************************
*
*	STATE IDENTIFIER to identify the last state during failover
*
********************************************************************/
#define fdCMD_ROWID			"CmdRowId"
#define fdCMD_OPCODE		"CmdOpcode"
#define fdCMD_STATE			"CmdState"
#define fdCMD_INDEX			"CmdIndex"

typedef struct{
	rowID			cmdRowId;
	U32				cmdOpcode;
	U32				cmdState;
	U32				index;		// for multiple entries with same state
} STATE_IDENTIFIER;


#endif


