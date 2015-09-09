/* AlarmCmdQueue.h
 *
 * Copyright (C) ConvergeNet Technologies, 1999 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/

// Revision History:
// $Log: /Gemini/Include/AlarmManager/AlarmCmdQueue.h $
// 
// 13    9/13/99 3:27p Joehler
// Added remit from user functionality to the CmdServer SSAPI interface
// 
// 12    9/07/99 2:33p Joehler
// Changed userName from a UnicodeString16 to a UnicodeString32
// 
// 11    9/02/99 11:44a Joehler
// added comments
//


#ifndef __DdmAlarmCmdQueue_H
#define __DdmAlarmCmdQueue_H

/********************
*
* Set up for the Alarm Command Server
*
********************/

// name of AlarmManager Command Queue Table
#define AMSTR_CMD_QUEUE_TABLE "AlarmQueueTable\0"

typedef enum
{
	AMSTR_CMND_ACKNOWLEDGE_ALARM = 1,
	AMSTR_CMND_UNACKNOWLEDGE_ALARM,
	AMSTR_CMND_NOTIFY_ALARM,
	AMSTR_CMND_KILL_ALARM,
	AMSTR_CMND_REMIT_ALARM_FROM_USER
}	AMSTR_CMND; // Alarm Master Commands

typedef struct {
	rowID rid;
	UnicodeString32 userName;
	UnicodeString512 notes;
} AMSTR_CMND_PARAMETERS;

typedef struct {
	U32 opcode;
	AMSTR_CMND_PARAMETERS cmdParams;
} AMSTR_CMND_INFO;

#endif
