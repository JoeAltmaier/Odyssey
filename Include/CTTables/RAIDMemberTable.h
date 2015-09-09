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
// File: RaidMemberTable.h
// 
// Description:
// Member Descriptor Record
// 
// $Log: /Gemini/Include/CTTables/RAIDMemberTable.h $
// 
// 17    8/20/99 3:46p Dpatel
// added state identifier to keep track of state in case of failover
// 
// 15    8/11/99 3:01p Dpatel
// Added version size to tables, changed cap to I64.
// 
// 14    6/29/99 1:52p Dpatel
// Removed ShadowRID, added headers.
// 
//
/*************************************************************************/

#ifndef RAIDMemberTable_h
#define RAIDMemberTable_h

#include "CtTypes.h"
#include "TableMsgs.h"

#include "RaidDefs.h"
#include "RMgmtPolicies.h"

#pragma	pack(4)

extern const fieldDef MemberDescriptorTable_FieldDefs[];
extern const U32 sizeofMemberDescriptorTable_FieldDefs;

#define RAID_MEMBER_DESCRIPTOR_TABLE "RAID MEMBER DESCRIPTOR TBL"
#define RAID_MEMBER_DESCRIPTOR_TABLE_VERSION	1

/********************************************************************
*
* Field Def constants
*
********************************************************************/

#define fdMDT_VERSION	"MDTVersion"
#define fdMDT_SIZE		"MDTSize"

#define fdARRAY_RID		"ArrayRID"
#define fdMEMBER_RID		"MemRID"
#define fdMEMBER_VD			"MemberVD"
#define fdMEMBER_HEALTH	"Health"
#define fdINDEX			"Index"
#define fdRETRY			"RetryCnt"
#define fdQUEUEING_METHOD	"QMethod"
#define fdUNUSED_MEM_FIELD	"Unused"
#define fdSTART_LBA		"StartLBA"
#define fdEND_LBA			"LastLBA"
#define fdMAX_REQUESTS		"MaxReqs"
#define fdMEM_POLICY		"Policy"

/********************************************************************
*
* Array Member Descriptor
*
********************************************************************/

typedef struct
{
	rowID					thisRID;		// member descriptor id
	U32						version;		// MDT table version
	U32						size;			// MDT table size
	rowID					arrayRID;		// array descriptor id
	rowID					memberRID;		// rowid in SRCTbl
	VDN						memberVD;		// VD of member -- 
	RAID_MEMBER_STATUS		memberHealth;
	U32						memberIndex;	// relative to this array
	U32						maxRetryCnt;	// (default=3)set by System Master
	RAID_QUEUE_METHOD		queueMethod;	// (default=RAID_QUEUE_ELEVATOR)
	I64						startLBA;		// where host data begins
	I64						endLBA;			// where host data ends
	U32						maxOutstanding;	// (default=5)request queue depth
	RAID_MEMBER_POLICIES	policy;
	STATE_IDENTIFIER		stateIdentifier;
}	RAID_ARRAY_MEMBER, RAIDMemberDescriptorTable[];

#endif // RAIDMemberTable_h