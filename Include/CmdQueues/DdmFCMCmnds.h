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
// File: DdmFCMCmnds.h
// 
// Description:
// Defines the FCM interface for Commands/Status and Events
// 
// $Log: /Gemini/Include/CmdQueues/DdmFCMCmnds.h $
// 
// 1     1/05/00 5:15p Dpatel
// Initial creation.
// 
//
/*************************************************************************/

#ifndef __DdmFCMCmnds_h
#define __DdmFCMCmnds_h

#include "CtTypes.h"
#include "TableMsgs.h"



#pragma	pack(4)

#define FCM_CMD_QUEUE_TABLE "FCM_CMD_QUEUE_TABLE"

#define FCM_CMND_START		0x0

// Error codes
#define FCM_SUCCESS			0x0

// Resolve: to add to message compiler
/*
#define FCM_ERR_INVALID_COMMAND		0x00000001
#define FCM_ERR_LOOP_ALREADY_UP		0x00000002
#define FCM_ERR_LOOP_ALREADY_DOWN	0x00000003
*/
/********************************************************************
*
* Commands sent by the upper layer of the System Master to the RAID
* agent (DDM) of the System Master
*
********************************************************************/

typedef enum
{
	FCM_CMND_LOOP_CONTROL = (FCM_CMND_START | 1),	// LIP, loop up/down
	FCM_CMND_NAC_SHUTDOWN,							// nac shutdown
	FCM_CMND_POLICY,								// read/change policies
	FCM_CMND_GET_NEXT_WWN,
	FCM_CMND_GET_CHASSIS_WWN,
	FCM_CMND_LAST_VALID								// insert all valid cmnds above this one
}	FCM_CMND; 

// Loop cmnds
#define	LOOP_UP		LM_LOOP_UP
#define LOOP_DOWN	LM_LOOP_DOWN
#define LOOP_LIP	LM_LOOP_LIP
typedef struct
{
	U32			cmnd;				// LOOP UP / DOWN/ LIP
	U32			loopInstanceNumber;	// from the Loop Descriptor record
	U32			flags;				// scan fabric? etc
}	FCM_LOOP_CONTROL_INFO;




typedef struct
{
	U32				slotNumber;
}	FCM_NAC_SHUTDOWN_INFO;


typedef struct
{
	// TBD
}	FCM_POLICY_INFO;




typedef enum {
	NAA1 = 1,
	NAA2 = 2,
	NAA5 = 5,
	NAA6 = 6
} NAA_TYPE;

typedef struct
{
	String16			wwn;		// ret value (16bytes for the wwn)
}	FCM_GET_NEXT_WWN;

typedef struct
{
	NAA_TYPE			naaType;		// refer to WWN spec
	unsigned char		chassisWWN;		// ret. value (8 bytes)
}	FCM_GET_CHASSIS_WWN;


typedef union {
	FCM_LOOP_CONTROL_INFO		loopControlInfo;
	FCM_NAC_SHUTDOWN_INFO		nacShutdownInfo;
	FCM_GET_NEXT_WWN			getNextWWN;
	FCM_GET_CHASSIS_WWN			getChassisWWN;
	FCM_POLICY_INFO				policyInfo;
} FCM_CMND_PARAMETERS;


typedef struct {
	U32						opcode;
	FCM_CMND_PARAMETERS		cmdParams;
	U32						isMessage;	// for internal use only
} FCM_CMND_INFO;



/***************************************************************
*
*
*
****************************************************************/
#define FCM_EVT_START			0x000
typedef enum 
{
	FCM_EVT_LOOP_UP = (FCM_EVT_START | 1),
	FCM_EVT_LOOP_DOWN,
	FCM_EVT_LIP,
	FCM_EVT_NAC_SHUTDOWN
} FCM_EVENT;



typedef struct {
	LoopDescriptorRecord	loopData;
} FCM_LOOP_STATUS;

typedef struct {
	U32						slotNumber;
} FCM_NAC_SHUTDOWN_STATUS;

typedef union {
	FCM_LOOP_STATUS				loopStatus;
	FCM_NAC_SHUTDOWN_STATUS		nacShutdownStatus;
} FCM_EVENT_INFO;








#endif