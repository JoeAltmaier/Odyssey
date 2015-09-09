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
// File: PmstrEvents.h
// 
// Description:
// Defines the Pmstr interface for Commands/Status.
// 
// $Log: /Gemini/Include/Partition/PmstrEvents.h $
// 
// 4     9/15/99 7:00p Dpatel
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

#ifndef __PmstrEvents_h
#define __PmstrEvents_h

#include "CtTypes.h"
#include "TableMsgs.h"

#include "PartitionTable.h"

#include "StorageRollCallTable.h"
#include "UnicodeString.h"

#pragma	pack(4)

#define PMSTR_EVT_START			0x000
typedef enum 
{
	PMSTR_EVT_PARTITION_CREATED = (PMSTR_EVT_START | 1),
	PMSTR_EVT_PARTITION_DELETED,
	PMSTR_EVT_PARTITION_MODIFIED
} PMSTR_EVENT;



typedef struct {
	StorageRollCallRecord		SRCData;		// src record data of new partition
	PARTITION_DESCRIPTOR		partitionData;	// new partition record data
	UnicodeString32				partitionName;	// name of new partition
	rowID						partitionNameRowId;
} PMSTR_EVT_PARTITION_CREATED_STATUS;


typedef struct {
	StorageRollCallRecord		SRCData;		// src data of deleted/modified partition
	PARTITION_DESCRIPTOR		partitionData;	// partition data for modified partition
} PMSTR_EVT_PARTITION_DELETED_STATUS, PMSTR_EVT_PARTITION_MODIFIED_STATUS;



typedef union {
	PMSTR_EVT_PARTITION_CREATED_STATUS		partitionCreatedStatus;
	PMSTR_EVT_PARTITION_DELETED_STATUS		partitionDeletedStatus;
	PMSTR_EVT_PARTITION_MODIFIED_STATUS		partitionModifiedStatus;
} PMSTR_EVENT_INFO;

#endif
