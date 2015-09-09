/* AlarmRecordTable.h
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
// $Log: /Gemini/Include/CTTables/AlarmRecordTable.h $
// 
// 6     9/07/99 2:36p Joehler
// Added version and size to tables and changed userName from a
// UnicodeString16 to a UnicodeString32 in AlarmLogTable
// 
// 5     9/02/99 11:45a Joehler
// added comments

#ifndef __AlarmRecordTable_H
#define __AlarmRecordTable_H

#include "Rows.h"

/*********************
*
* Field Def Constants 
*
*********************/

#define fdATVersion "AlarmTableVersion"
#define fdATSize "AlarmTableSize"
#define fdVdn "Vdn"
#define fdDid "Did"
#define fdActive "Active"
#define fdAcknowledged "Acknowledged"
#define fdClearable "Clearable"
#define fdNumberOfEvents "NumberOfEvents"
#define fdEventData "EventData"
#define fdcbContext "CbContext"
#define fdAlarmContext "AlarmContext"

// names of AlarmRecord, DidRecord and AlarmLogRecord Tables
#define ALARM_RECORD_TABLE_NAME "AlarmRecordTable\0"

// define initial size of tables
#define ALARM_RECORD_TABLE_SIZE 20

// define alarm table version
#define ALARM_TABLE_VERSION 1

// maximum size of event data
#define ALARM_MAX_EVENT_DATA_SIZE 512

// temporary maximum size of alarm context
#define ALARM_MAX_SIZE_CONTEXT 1024

/*********************
*
* Alarm Record
*   this is the actual record which is stored in the AlarmTable
*
*********************/

typedef struct AlarmRecord
{
	rowID rid;
	U32					version;			// version of AT row
	U32					size;				// size of AT row
	DID did;
	VDN vdn;
	BOOL active;
	BOOL acknowledged;
	BOOL clearable;
	U32 numberOfEvents;
	U8 eventData[ALARM_MAX_EVENT_DATA_SIZE];
	// this needs to be changed once dynamic pts entries come on line
	U32 cbContext;
	U8 alarmContext[ALARM_MAX_SIZE_CONTEXT];
} AlarmRecord;

extern const fieldDef AlarmRecordTable_FieldDefs[];
extern const U32 sizeofAlarmRecordTable_FieldDefs;

#endif