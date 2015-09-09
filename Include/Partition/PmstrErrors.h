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
// File: PmstrErrors.h
// 
// Description:
// Defines the Error Codes
// 
// $Log: /Gemini/Include/Partition/PmstrErrors.h $
// 
// 2     9/15/99 10:44a Dpatel
// 
// 1     9/10/99 9:42a Dpatel
// Initial creation
// 
//
/*************************************************************************/
#ifndef __PmstrErrors_h
#define __PmstrErrors_h


#pragma	pack(4)

#define PMSTR_SUCCESS			0x00000000
#define PMSTR_ERR_START			0x0000

enum 
{
	PMSTR_ERR_PMSTR_NOT_INITIALIZED = (PMSTR_ERR_START) | 1,
	PMSTR_ERR_STORAGE_ELEMENT_USED,
	PMSTR_ERR_INVALID_PARTITION_SIZE,
	PMSTR_ERR_INVALID_COMMAND
};

#endif