/* AlarmLogTable.h
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
// $Log: /Gemini/Include/CTTables/AlarmLogTable.h $
// 
// 5     9/07/99 2:36p Joehler
// Added version and size to tables and changed userName from a
// UnicodeString16 to a UnicodeString32 in AlarmLogTable
// 
// 4     9/02/99 11:45a Joehler
// added comments
//				added userName field to table

#ifndef __AlarmLogTable_H
#define __AlarmLogTable_H

#include "Rows.h"

/*********************
*
* Field Def Constants 
*
*********************/

#define fdALTVersion "AlarmLogVersion"
#define fdALTSize "AlarmLogSize"
#define fdTimeStamp "TimeStamp"
#define fdAlarmRid "AlarmRid"
#define fdVdnPerformedBy "VdnPerformedBy"
#define fdDidPerformedBy "DidPerformedBy"
#define fdAction "Action"
#define fdUserName "UserName"
#define fdNotes "Notes"

// names of AlarmRecord, DidRecord and AlarmLogRecord Tables
#define ALARM_LOG_TABLE_NAME "AlarmLogTable\0"

// define initial size of tables
#define ALARM_LOG_TABLE_SIZE 100

// define the version of the alarm log table
#define ALARM_LOG_TABLE_VERSION 1

/*********************
*
* Alarm Log Record
*   hold log information for each alarm
*
*********************/

typedef struct  AlarmLogRecord 
{
	rowID rid;
	U32					version;			// version of ALT row
	U32					size;				// size of ALT row
	rowID alarmRid; // key to search by
	I64 timeStamp;
	DID didPerformedBy;
	VDN vdnPerformedBy;
	U32 action; 
	UnicodeString32 userName;
	UnicodeString512 notes;
} AlarmLogRecord;

extern const fieldDef AlarmLogTable_FieldDefs[];
extern const U32 sizeofAlarmLogTable_FieldDefs;

#endif