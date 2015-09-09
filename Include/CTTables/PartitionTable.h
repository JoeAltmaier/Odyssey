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
// File: PartitionTable.h
// 
// Description:
// Partition table
// 
// $Log: /Gemini/Include/CTTables/PartitionTable.h $
// 
// 1     9/15/99 10:49a Dpatel
// Initial creation
// 
//
/*************************************************************************/

#ifndef PartitionTable_h
#define PartitionTable_h

#include "CtTypes.h"
#include "TableMsgs.h"


#pragma	pack(4)

extern const fieldDef PartitionDescriptorTable_FieldDefs[];
extern const U32 sizeofPartitionDescriptorTable_FieldDefs;

#define PARTITION_DESCRIPTOR_TABLE "PARTITION DESCRIPTOR TBL"
#define PARTITION_DESCRIPTOR_TABLE_VERSION	1

/********************************************************************
*
* Field Def constants
*
********************************************************************/

#define fdPARTITION_TABLE_VERSION	"PartitionVersion"
#define fdPARTITION_DESCRIPTOR_SIZE	"PartitionSize"

#define fdSRCTRID				"SRCTRID"
#define fdPARENT_RID			"ParentRID"
#define fdPARENT_VDN			"ParentVDN"
#define fdSTART_LBA				"StartLBA"
#define fdPARTITION_SIZE		"PartitionSize"
#define fdVIRTUAL_DEVICE		"VirtualDeviceNun"
#define	fdNEXT_RID				"NextRowId"
#define	fdPREV_RID				"PrevRowId"
#define	fdPARTITION_STATUS		"PartitionStatus"
#define	fdOPCODE				"Opcode"
#define	fdCMD_RID				"CmdRowId"
#define	fdSTATE					"State"
#define	fdINDEX					"Index"

struct PARTITION_STATE_IDENTIFIER {
	U32			opcode;
	rowID		cmdRowId;
	U32			state;
	U32			index;
};

typedef enum {
	PARTITION_DOWN = 1,
	PARTITION_UP
}PARTITION_STATUS;

/********************************************************************
*
* Partition Descriptor
*
********************************************************************/

typedef struct
{
	rowID					rid;		// partition table descriptor row id
	U32						version;		// table version
	U32						size;			// table size
	rowID					SRCTRID;		// SRC entry for this partition	
	VDN						partitionVD;	// VD of partition (NULL means unpartitioned)
	rowID					parentSRCTRID;	// the SRC rowid of the partition's parent
	VDN						parentVDN;		// the VDN of the partition's parent
	I64						startLBA;		// where partition starts
	I64						partitionSize;	// size of partition in blocks of 512
	rowID					nextRowId;		// SRC row id of next contiguous part (can be NULL)
	rowID					previousRowId;	// SRC row id of previous contiguous part (can be NULL)
	PARTITION_STATUS		status;			// state of the partition
	PARTITION_STATE_IDENTIFIER		stateIdentifier;// state of our state m/c (for owner-PMSTR use only)
}	PARTITION_DESCRIPTOR, PartitionDescriptorTable[];

#endif 