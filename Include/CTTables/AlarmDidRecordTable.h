/* AlarmDidRecordTable.h
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
// $Log: /Gemini/Include/CTTables/AlarmDidRecordTable.h $
// 
// 3     9/07/99 2:36p Joehler
// Added version and size to tables and changed userName from a
// UnicodeString16 to a UnicodeString32 in AlarmLogTable
// 
// 2     9/02/99 11:45a Joehler
// added comments

#ifndef __AlarmDidRecordTable_H
#define __AlarmDidRecordTable_H

#include "Rows.h"

#define fdADTVersion "AlarmDidVersion"
#define fdADTSize "AlarmDidSize"
#define fdVdn "Vdn"
#define fdDid "Did"
#define fdNumberOfAlarms "NumberOfAlarms"

/*********************
*
* Field Def Constants 
*
*********************/

// names of AlarmRecord, DidRecord and AlarmLogRecord Tables
#define ALARM_DID_TABLE_NAME "AlarmDidTable\0"

// define initial size of tables
#define ALARM_DID_TABLE_SIZE 10

// version of did table
#define ALARM_DID_TABLE_VERSION 1

/*********************
*
* Did Record
*   holds information about all alarms associated with a device
*
*********************/

typedef struct DidRecord
{
	rowID				rid;
	U32					version;			// version of ADT row
	U32					size;				// size of ADT row
	DID					did;
	VDN					vdn;
	U16					numberOfAlarms;
} DidRecord;

extern const fieldDef AlarmDidTable_FieldDefs[];
extern const U32 sizeofAlarmDidTable_FieldDefs;
#endif