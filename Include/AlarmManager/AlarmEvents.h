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
// File: AlarmEvents.h
// 
// Description:
// Defines the Alarm interface for Commands/Status.
//
// $Log: /Gemini/Include/AlarmManager/AlarmEvents.h $
// 
// 4     9/02/99 11:45a Joehler
// added comments
//
/*************************************************************************/

#ifndef __AlarmEvents_h
#define __AlarmEvents_h

typedef enum 
{
	AMSTR_EVT_ALARM_SUBMITTED = 1,
	AMSTR_EVT_ALARM_REMITTED,
	AMSTR_EVT_ALARM_ACKNOWLEDGED,
	AMSTR_EVT_ALARM_UNACKNOWLEDGED,
	AMSTR_EVT_ALARM_NOTIFIED,
	AMSTR_EVT_ALARM_KILLED
} AMSTR_EVENT;


typedef struct {
	DID did;
	VDN vdn;
	rowID rid;
} AMSTR_EVT_ALARM_VERBOSE_STATUS;

typedef struct {
	rowID rid;
} AMSTR_EVT_ALARM_STATUS;

typedef union {
	AMSTR_EVT_ALARM_VERBOSE_STATUS		alarmSubmitted;
	AMSTR_EVT_ALARM_VERBOSE_STATUS		alarmRemitted;
	AMSTR_EVT_ALARM_STATUS				alarmAcknowledged;
	AMSTR_EVT_ALARM_STATUS				alarmUnacknowledged;
	AMSTR_EVT_ALARM_STATUS				alarmNotified;
	AMSTR_EVT_ALARM_STATUS				alarmKilled;
} AMSTR_EVENT_INFO;

#endif