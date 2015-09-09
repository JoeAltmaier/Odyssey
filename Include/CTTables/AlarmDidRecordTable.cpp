/* AlarmDidRecordTable.cpp
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
// $Log: /Gemini/Include/CTTables/AlarmDidRecordTable.cpp $
// 
// 3     9/07/99 2:36p Joehler
// Added version and size to tables and changed userName from a
// UnicodeString16 to a UnicodeString32 in AlarmLogTable
// 
// 2     9/02/99 11:45a Joehler
// added comments
// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#include "AlarmDidRecordTable.h"

// Did Record Table Column definition
const fieldDef AlarmDidTable_FieldDefs[] =
{
	fdADTVersion,		4,	U32_FT,			Persistant_PT,
	fdADTSize,			4,	U32_FT,			Persistant_PT,
	fdDid, 0, U32_FT, Persistant_PT,
	fdVdn, 0, U32_FT, Persistant_PT,
	fdNumberOfAlarms, 0, U32_FT, Persistant_PT
};

// size of DidRecord FieldDef structure
const U32 sizeofAlarmDidTable_FieldDefs = sizeof(AlarmDidTable_FieldDefs);

