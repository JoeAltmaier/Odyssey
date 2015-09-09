/* AlarmRecordTable.cpp
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
// $Log: /Gemini/Include/CTTables/AlarmRecordTable.cpp $
// 
// 5     9/07/99 2:36p Joehler
// Added version and size to tables and changed userName from a
// UnicodeString16 to a UnicodeString32 in AlarmLogTable
// 
// 4     9/02/99 11:45a Joehler
// added comments
// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#include "AlarmRecordTable.h"

// Alarm Log Record Table Column definition
const fieldDef AlarmRecordTable_FieldDefs[] =
{
	fdATVersion,		4,	U32_FT,			Persistant_PT,
	fdATSize,			4,	U32_FT,			Persistant_PT,
	fdDid, 0, U32_FT, Persistant_PT,
	fdVdn, 0, U32_FT, Persistant_PT,
	fdActive, 0, BOOL_FT, Persistant_PT,
	fdAcknowledged, 0, BOOL_FT, Persistant_PT,
	fdClearable, 0, BOOL_FT, Persistant_PT,
	fdNumberOfEvents, 0, U32_FT, Persistant_PT,
	//fdSeverity, 0, U32_FT, Persistant_PT,
	fdEventData, ALARM_MAX_EVENT_DATA_SIZE, BINARY_FT, Persistant_PT,
	// this needs to be changed once dynamic pts entries come on line
	fdcbContext, 0, U32_FT, Persistant_PT,
	fdAlarmContext, ALARM_MAX_SIZE_CONTEXT, BINARY_FT, Persistant_PT
};

// size of AlarmRecord FieldDef structure
const U32 sizeofAlarmRecordTable_FieldDefs = sizeof(AlarmRecordTable_FieldDefs);
