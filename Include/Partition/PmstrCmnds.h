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
// File: PartitionMstrCmnds.h
// 
// Description:
// Defines the Rmstr interface for Commands/Status.
// 
// $Log: /Gemini/Include/Partition/PmstrCmnds.h $
// 
// 3     9/15/99 6:57p Dpatel
// 
// 2     9/15/99 10:44a Dpatel
// 
// 1     9/10/99 9:42a Dpatel
// Initial creation
// 
//
/*************************************************************************/

#ifndef __ParitionMstrCmnds_h
#define __ParitionMstrCmnds_h

#include "CtTypes.h"
#include "TableMsgs.h"

#include "PartitionTable.h"


#pragma	pack(4)

#define PARTITION_MSTR_CMD_QUEUE "PARTITION_MSTR_CMD_QUEUE_TABLE"

#define PMSTR_CMND_START		0x0000

/********************************************************************
*
* Commands sent by the upper layer of the System Master to the Partition
* agent (DDM) of the System Master
*
********************************************************************/

typedef enum
{
	PMSTR_CMND_CREATE_PARTITION = (PMSTR_CMND_START | 1),
	PMSTR_CMND_MERGE_PARTITIONS,
	PMSTR_CMND_LAST_VALID				// insert all valid cmnds above this one
}	PMSTR_CMND; // Partition Master Cmnd





typedef struct
{
	rowID				srcToPartition;				// row id of SRC entry to be partitioned
	rowID				partitionNameRowId;			// row id in table containing logical name for partition
	I64					partitionSize;				// size of new partition
	rowID				remainderPartitionNameRowId;// row id in table containing logical name for partition
}	PMSTR_CREATE_PARTITION_INFO;




typedef struct
{
	rowID				srcPartitionRowId1;	// row id in src table for partition to merge with
	rowID				srcPartitionRowId2;	// partition to merge 
}	PMSTR_MERGE_PARTITION_INFO;




typedef union {
	PMSTR_CREATE_PARTITION_INFO			createPartitionInfo;
	PMSTR_MERGE_PARTITION_INFO			mergePartitionInfo;
} PMSTR_CMND_PARAMETERS;


typedef struct {
	U32						opcode;
	PMSTR_CMND_PARAMETERS	cmdParams;
} PMSTR_CMND_INFO;

#endif