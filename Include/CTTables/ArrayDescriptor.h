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
// File: ArrayDescriptor.h
// 
// Description:
// Array Descriptor Record 
// 
// $Log: /Gemini/Include/CTTables/ArrayDescriptor.h $
// 
// 33    8/20/99 3:46p Dpatel
// added state identifier to keep track of state in case of failover
// 
// 31    8/11/99 3:01p Dpatel
// Added version size to tables, changed cap to I64.
// 
// 30    8/04/99 3:32p Dpatel
// Added hotcopy export member index...
// 
// 29    7/29/99 6:57p Dpatel
// Added creation policy to array descriptor...
// 
// 28    7/28/99 12:41p Dpatel
// modified the pecking order to enum
// 
// 27    7/27/99 6:06p Dpatel
// Added Source Member index and primary member index.
// 
// 26    7/20/99 6:50p Dpatel
// Changed arrayName to rowiD
// 
// 25    6/30/99 11:18a Dpatel
// Changed array name to Unicode string.
// 
// 
//
/*************************************************************************/

#ifndef ArrayDescriptor_h
#define ArrayDescriptor_h

#include "CtTypes.h"
#include "TableMsgs.h"

#include "RMgmtPolicies.h"
#include "RaidDefs.h"


#pragma	pack(4)

extern const fieldDef ArrayDescriptorTable_FieldDefs[];
extern const U32 sizeofArrayDescriptorTable_FieldDefs;

#define RAID_ARRAY_DESCRIPTOR_TABLE "RAID ARRAY DESCRIPTOR TBL"
#define	RAID_ARRAY_DESCRIPTOR_TABLE_VERSION		1

/********************************************************************
*
* Field Def constants
*
********************************************************************/

#define fdADT_SIZE			"ADTSize"
#define fdADT_VERSION		"ADTVersion"
#define fdARRAY_VDN			"ArrayVDN"			
#define fdARRAY_SRCTRID		"SRCTRID"
#define fdARRAY_ROWID		"RowID"			
#define fdSTATUS_RID		"StatusRID"
#define fdPERFORMANCE_RID	"PerformanceRID"
#define fdMONITOR_RID		"MonitorRID"
#define fdARRAY_CAPACITY	"Capacity"
#define fdAMEM_CAPACITY		"MemCapacity"
#define fdDBLK_SIZE			"DataBlkSize"		
#define fdPBLK_SIZE			"ECCBlkSize"
#define fdRAID_LEVEL		"RAIDLevel"
#define fdARRAY_TYPE		"Type"
#define fdARRAY_STATUS		"Health"
#define fdINIT_STATUS		"InitStatus"
#define fdPECKING			"PeckingOrder"
#define fdNUM_MEMBERS		"MemCnt"
#define fdNUM_UTILS			"UtilCnt"
#define	fdNUM_SPARES		"SpareCnt"
#define fdAUNUSED			"Filler Field"
#define fdARRAY_SN			"SerialNum"
#define fdCREATION			"Creation Date"
#define fdCONFIG_TS			"ConfigTimestamp"
#define fdCREATION_POLICY	"Creation Policy Mask"
#define fdARRAY_POLICY		"Array Policy Mask"
#define fdHOST_FOR_SPARE_POOL	"HostSparePool"
#define fdSOURCE_MEMBER		"SourceMemberIndex"
#define fdPREFERRED_MEMBER	"PreferredMemberIndex"
#define fdHOTCOPY_EXPORT_MEMBER	"HotCopyExportMemberIndex"
#define fdARRAY_CACHE		"Cache"
#define fdMEMBERS			"Members"
#define fdUTILS				"Utils"
#define fdSPARES			"Spares"

/********************************************************************
*
* ARRAY_DESCRIPTOR
*
********************************************************************/

typedef struct
{
	rowID				thisRID;			// rid in descriptor table
	U32					version;			// version of ADT			
	U32					size;				// size of ADT
	VDN					arrayVDN;			// VDN corresponding to array
	rowID				SRCTRID;			// Storage Roll Call Tbl rowID
	I64					totalCapacity;		// in 512 byte blks - depends on RAID Level
	I64					memberCapacity;		// in 512 byte blks - smallest member cap.
	U32					dataBlockSize;		// in 512 byte blks (default = 64)
	U32					parityBlockSize;	// in 512 byte blks	(default = DataBlockSize)
											// if parity blk size is different from data blk
											// size, parity size must be a multiple of the 
											// data blk size (matters in RAIDS 3,4,5)
	RAID_LEVEL			raidLevel;			// RAID0, RAID1, RAID5
	RAID_ARRAY_STATUS	health;				// OFFLINE,
	RAID_INIT_STATUS	initStatus;			// Initialization Done,
	RAID_PECKING_ORDER	peckingOrder;		// PECK_FIRST, PECK LAST, NEVER PECK
	U32					numberMembers;
	U32					numberUtilities;	// currently running on array
	U32					numberSpares;		// dedicated to this array/*
	U32					serialNumber;		// timestamp
	U32					creationDate;		// timestamp
	U32					timeStamp;			// configuration timestamp
	RAID_CREATION_POLICIES	createPolicy;	// created for hot copy etc.
	RAID_ARRAY_POLICIES		arrayPolicy;	// do not write res secs,
	rowID				hostForSparePool;	// the rowId in the host table
	U32					sourceMemberIndex;			// valid for RAID 1 only
	U32					preferredMemberIndex;		// valid for RAID 1 only
	U32					hotCopyExportMemberIndex;	// valid for RAID 1 only
	rowID				utilities[MAX_UTILS];		// rowid info for each running util
	rowID		 		members[MAX_ARRAY_MEMBERS]; // rowid's in Member descriptor table
	rowID		 		spares[MAX_ARRAY_SPARES];	// rowid's in spare descriptor table
	STATE_IDENTIFIER	stateIdentifier;			// identify our state machine step for failover
}	RAID_ARRAY_DESCRIPTOR, RaidArrayDescriptorTable[];

#endif