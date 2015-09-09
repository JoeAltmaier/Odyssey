/* AlarmLogTable.cpp
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
// $Log: /Gemini/Include/CTTables/AlarmLogTable.cpp $
// 
// 5     9/07/99 2:36p Joehler
// Added version and size to tables and changed userName from a
// UnicodeString16 to a UnicodeString32 in AlarmLogTable
// 
// 4     9/02/99 11:45a Joehler
// added comments
// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#include "AlarmLogTable.h"

// Alarm Log Record Table Column definition
const fieldDef AlarmLogTable_FieldDefs[] =
{
	fdALTVersion,		0,	U32_FT,			Persistant_PT,
	fdALTSize,			0,	U32_FT,			Persistant_PT,
	fdAlarmRid, 0, ROWID_FT, Persistant_PT,
	fdTimeStamp, 8, BINARY_FT, Persistant_PT,
	fdDidPerformedBy, 0, U32_FT, Persistant_PT,
	fdVdnPerformedBy, 0, U32_FT, Persistant_PT,
	fdAction, 0, U32_FT, Persistant_PT,
	fdUserName, 64, BINARY_FT, Persistant_PT,
	fdNotes, 1024, BINARY_FT, Persistant_PT
};

// size of AlarmLogRecord FieldDef structure
const U32 sizeofAlarmLogTable_FieldDefs = sizeof(AlarmLogTable_FieldDefs);

