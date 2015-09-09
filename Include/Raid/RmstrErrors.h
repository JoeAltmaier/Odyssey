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
// File: RmstrErrors.h
// 
// Description:
// Defines the Error Codes
// 
// $Log: /Gemini/Include/Raid/RmstrErrors.h $
// 
// 12    11/23/99 11:10a Dpatel
// Check for Data block sizes
// 
// 11    9/11/99 9:33p Agusev
// Update #pragma rules
// 
// 10    7/30/99 6:47p Dpatel
// Removed INSUFFICIENT CAP and INVALID STORAGE_ELEMENT
// 
// 9     7/30/99 5:37p Dpatel
// change preferred / src members..
// 
// 8     7/28/99 5:50p Dpatel
// Changed RMSTR_ERR_OS_PARTITION to RMSTR_ERR_PARTITION
// 
// 7     7/09/99 5:25p Dpatel
// 
// 6     7/07/99 5:40p Dpatel
// added invalid storage element.
// 
// 5     7/06/99 4:59p Dpatel
// Delete Spare and Utility changes.
// 
// 4     6/30/99 11:15a Dpatel
// Changes for Abort Util and Change Priority.
// 
// 3     6/28/99 5:14p Dpatel
// Changes for implementation of Utility Cmds
// 
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/
#ifndef __RmstrErrors_h
#define __RmstrErrors_h


#pragma	pack(4)

#define RMSTR_SUCCESS			0x00000000
#define RMSTR_ERR_START			0x0000

enum 
{
	RMSTR_ERR_RMSTR_NOT_INITIALIZED = (RMSTR_ERR_START) | 1,
	RMSTR_ERR_INSUFFICIENT_MEMBER_CAPACITY,
	RMSTR_ERR_INSUFFICIENT_SPARE_CAPACITY,

	RMSTR_ERR_STORAGE_ELEMENT_FAILED,
	RMSTR_ERR_STORAGE_ELEMENT_IN_USE,
	RMSTR_ERR_PARTITION_EXISTS,
	RMSTR_ERR_INVALID_COMMAND,			

	RMSTR_ERR_MEMBER_ALREADY_DOWN,
	RMSTR_ERR_UTIL_RUNNING,
	RMSTR_ERR_UTIL_NOT_SUPPORTED,	
	RMSTR_ERR_UTIL_ALREADY_RUNNING,	
	RMSTR_ERR_UTIL_ABORT_NOT_ALLOWED,
	RMSTR_ERR_UTIL_NOT_RUNNING,
	RMSTR_ERR_INVALID_PRIORITY_LEVEL,	


	RMSTR_ERR_MAX_SPARES_ALREADY_CREATED,
	RMSTR_ERR_SPARE_DOES_NOT_EXIST,

	RMSTR_ERR_ARRAY_STATE_OFFLINE,			// Verify, Init, Regenerate
	RMSTR_ERR_ARRAY_STATE_NOT_CRITICAL,	// Regenerate
	RMSTR_ERR_ARRAY_STATE_CRITICAL,		// Expand
	RMSTR_ERR_ARRAY_OFFLINE,
	RMSTR_ERR_ARRAY_CRITICAL,
	RMSTR_ERR_NAME_ALREADY_EXISTS,
	RMSTR_ERR_MAX_ARRAY_MEMBERS,

	RMSTR_ERR_INVALID_DATA_BLOCK_SIZE,
	RMSTR_ERR_INVALID_PARITY_BLOCK_SIZE,
	RMSTR_ERR_INVALID_RAID_LEVEL
};

#endif