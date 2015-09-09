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
// File: RaidSpareDescriptor.h
// 
// Description:
// Raid Spare Descriptor 
// 
// $Log: /Gemini/Include/CTTables/RaidSpareDescriptor.h $
// 
// 7     8/20/99 3:46p Dpatel
// added state identifier to keep track of state in case of failover
// 
// 5     8/11/99 3:01p Dpatel
// Added version size to tables, changed cap to I64.
// 
// 4     6/25/99 4:34p Dpatel
// Added Capacity and Vdn in Spare Descriptor. Fixed the VSS header.
// 
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/

#ifndef RaidSpareDescriptor_h
#define RaidSpareDescriptor_h

#include "CtTypes.h"
#include "TableMsgs.h"

#include "RMgmtPolicies.h"
#include "RaidDefs.h"

#pragma	pack(4)

extern const fieldDef SpareDescriptorTable_FieldDefs[];
extern const U32 sizeofSpareDescriptorTable_FieldDefs;

#define RAID_SPARE_DESCRIPTOR_TABLE "RAID SPARE TBL"
#define RAID_SPARE_DESCRIPTOR_TABLE_VERSION		1

/********************************************************************
*
* Field Def constants
*
********************************************************************/
#define fdSDT_VERSION		"SDTVersion"
#define fdSDT_SIZE			"SDTSize"

#define fdSPARE_TYPE		"SpareType"
#define fdSPARE_RID			"SRCTRID"
#define fdARRAY_RID			"ArrayRID"
#define fdHOST_RID			"HostRID"
#define fdCAPACITY			"Capacity"
#define fdVDN				"Vdn"
#define fdSPARE_POLICY		"Policy"

/********************************************************************
*
* RAID Spare
*
********************************************************************/

typedef struct
{
	rowID					thisRID;		// this id
	U32						version;
	U32						size;
	RAID_SPARE_TYPE			spareType;		// Dedicated, host, general
	rowID					SRCTRID;		// Storage Descriptor Id
	rowID					arrayRID;		// rid in ADT if dedicated
	rowID					hostRID;		// rid in HostTable if ded.to host
	I64						capacity;		// capacity from SRC Record
	VDN						bsaVdn;			// vdn from SRC Record
	RAID_SPARE_POLICIES		policy;
	STATE_IDENTIFIER		stateIdentifier;
}	RAID_SPARE_DESCRIPTOR, RAIDSpareDescriptorTable[];

#endif // RAIDMemberTable_h
