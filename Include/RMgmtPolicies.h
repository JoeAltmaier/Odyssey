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
* File Name:
* RmstrPolicies.h
*
* Description:
* This file defines 	- many of the commnands sent to the RAID DDM
* Update Log: 
* 2/16/99 Ron Parks: Create file
*
*************************************************************************/

#ifndef __RMgmtPolicies_h
#define __RMgmtPolicies_h

#include "CtTypes.h"

#pragma	pack(4)




/********************************************************************
*
*
*
********************************************************************/


/********************************************************************
*
* ARRAY_MEMBER Policy Mask Bits
*
********************************************************************/

typedef struct
{
	U32 HasRsrvdSecs			:1;	// 0 = no reserved secs
	U32 ResSecsAtTop			:1;	// reserved sectors at begining of drive
	U32 ResSecsAtEnd			:1;	// reserved sectors at end of drive
	U32 SourcePrimary			:1;	// (MIRRORED only) use the data from this
									// drive if a Verify inconsistency
									// is found. Also this member is the source
									// for a Hot Copy
	U32 ReadPreference			:3;	// try to read from this drive first (default = 001)
	U32 HotCopyExportMember		:1;	// is this member to be exported when hot copy mirror is broken?
	U32 Undefined				:24;
}	RAID_MEMBER_POLICIES;

/********************************************************************
*
* ARRAY_UTILTIY Policy Mask Bits
*
********************************************************************/

typedef struct
{
	U32 CantAbort				:1;		// user may not abort
	U32 FixedPriority			:1;		// user may not change initial priority
	U32 SilentMode				:1;		// fix errors silently, only report
										// total errors on completion (Verify) (default= 1)
	U32 DontFixErrors			:1;		// just count errors (Verify) (default = 0)
										// otherwise fix inconsistencies when found 
	U32 RunThruErrors			:1;		// keep running HotCopy if at least (default = 1)
										// one destination drive still UP after
										// a drive failure.
	U32 SendChkCondition		:1;		// CHECK CONDITION after Expand/HotCopy (default = 0)
										// completes
	U32	SpecifyMembersToRunOn	:1;		// If this bit is set then utility will be run 
										// on the source/dest members specified by user.
										// It is mandatory to specify source/dest members in this case
										// By default the utility will be run on all members
										// with the source (1st member) and dest=all remaining members
	U32 Undefined				:25;
}	RAID_UTIL_POLICIES;
														

typedef struct
{
	U32 startNextRegenerate		:1;		// Only for Regeneate abort,
										// start regenerate to new spare (if avail).
										// completes
	U32 Undefined				:31;
}	RAID_UTIL_ABORT_POLICIES;

/********************************************************************
*
* ARRAY_DESCRIPTOR Policy Mask Bits
*
********************************************************************/

typedef struct
{
	U32 ResSecsAtEnd		:1;
	U32 WriteResSecs		:1;
	U32 HasRsrvdSecs	   	:1;	// array has reserved sectors, (default = 0)
								// otherwise array only exists in PTC
	U32 ArrayIsMember  		:1;	// array is a member of another array
	U32 Undefined			:28;
}	RAID_ARRAY_POLICIES;


/********************************************************************
*
* Cache Policy
*
********************************************************************/

typedef struct
{
	U32 WriteThru				:1;		// I/O compelete with writ to media
	U32 WriteBack				:1;		// I/O complete with write to cache
	U32 LRU						:1;		// flush policy
	U32 ReadAhead				:1;
	U32 Undefined				:28;
}	RAID_CACHE_POLICIES;															


/********************************************************************
*
*
*
********************************************************************/

typedef struct
{
	U32 StartHotCopyWithManualBreak		:1;	// Start hot copy, user needs to manually break
	U32	StartHotCopyWithAutoBreak		:1;	// RMSTR breaks mirror on hc complete.
											// need to specify members to be exported on break up
	U32 AbortOnMemberError				:1; // all res sec writes must be successful
											// if 0 then allow the creation of critical
											// array.(default = 1) - Not used for Release 1
	U32 Undefined						:29;
}	RAID_CREATION_POLICIES;


/********************************************************************
*
*
*
********************************************************************/

typedef struct
{
	U32 BreakHotCopyMirror				:1;
	U32 Undefined						:31;
}	RAID_DELETE_POLICIES;


/********************************************************************
*
*
*
********************************************************************/

typedef struct
{
	U32 MarkAsFlaky		:1;	// (default = 1)	
	U32 MarkAsSpare		:1;	// (default = 0)
	U32 Undefined		:30;
}	RAID_DOWN_DRIVE_POLICIES;


/********************************************************************
*
*
*
********************************************************************/

typedef struct
{
	U32 Undefined		:32;
}	RAID_CONFIG_POLICIES;


/********************************************************************
*
*
*
********************************************************************/

typedef struct
{
	U32 Undefined		:32;
}	RAID_VALIDATE_POLICIES;


/********************************************************************
*
*
*
********************************************************************/

typedef struct
{
	U32 Undefined		:32;
}	RAID_EXPANSION_POLICIES;



/********************************************************************
*
*
*
********************************************************************/

typedef struct
{
	U32 Undefined		:32;
}	RAID_STATUS_CHG_POLICIES;



/********************************************************************
*
*
*
********************************************************************/

typedef struct
{
	U32 Undefined		:32;
}	RAID_CREATE_SPARE_POLICIES;



/********************************************************************
*
*
*
********************************************************************/

typedef struct
{
	U32 Undefined		:32;
}	RAID_DEL_SPARE_POLICIES;


/********************************************************************
*
*
*
********************************************************************/

typedef struct
{
	U32 Undefined		:32;
}	RAID_MEMBER_HC_POLICIES;



/********************************************************************
*
*
*
********************************************************************/

typedef struct
{
	U32 Undefined		:32;
}	RAID_FLAKY_DRIVE_POLICIES;




/***********************
Following stuff not used

************************/


typedef struct 
{
	U32		DedicatedToOwner	:1;	// spare can only be used for 'Owner'spcified in spare desciptor
	U32		Undefined			:31;
}	RAID_SPARE_POLICIES;

#endif