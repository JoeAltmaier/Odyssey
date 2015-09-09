//*************************************************************************
//
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File Name: ArrayStatusTable.cpp
// 
// Description: This file defines the array Status Table fields
// 
// $Log: /Gemini/Include/CTTables/ArrayStatusTable.cpp $
// 
// 5     11/06/99 3:03p Vnguyen
// Add four status counters to return the sum of each error counters which
// are in the array.  The benefit is that we can return the sum to the
// customer and keep the detaild array info for performance tuning.
// 
// 4     9/15/99 2:31p Vnguyen
// Update performance and status counters.  Remove variance info for the
// performance counters because nobody is using it.
// 
// 3     8/16/99 12:53p Vnguyen
// Convert to use new object: SRCandExport class.  Fix miss-match
// parameters between calls.
// 
// 2     7/12/99 10:13a Jlane
// Miscellaneous fixes.
// 
// 1     6/08/99 7:33p Jlane
// Initial creation and check-in.
//
//**************************************************************************

#include "ArrayStatusTable.h"

const fieldDef	aArrayStatusTable_FieldDefs[] = 
{
// 	Field Name						
//	fdARRAYSTATUS_ROWID,					8,						ROWID_FT,	Persistant_PT,
	fdARRAYSTATUS_VERSION,					4,						U32_FT,		Persistant_PT,
	fdARRAYSTATUS_SIZE,						4,						U32_FT,		Persistant_PT,
	fdARRAYSTATUS_KEY,						4,						U32_FT,		NotPersistant_PT,
	fdARRAYSTATUS_REFRESHRATE,				4,						U32_FT,		Persistant_PT,
	fdARRAYSTATUS_RIDSRCRECORD,				8,						ROWID_FT,	Persistant_PT,
	fdARRAYSTATUS_NUMRETRIES,				MAX_ARRAY_MEMBERS * 4,	U32_FT,		NotPersistant_PT,
	fdARRAYSTATUS_NUMRECOVEREDERRORS,		MAX_ARRAY_MEMBERS * 4,	U32_FT,		NotPersistant_PT,
	fdARRAYSTATUS_NUMREASSIGNEDSUCCESS,		MAX_ARRAY_MEMBERS * 4,	U32_FT,		NotPersistant_PT,
	fdARRAYSTATUS_NUMREASSIGNEDFAILED,		MAX_ARRAY_MEMBERS * 4,	U32_FT,		NotPersistant_PT,
	fdARRAYSTATUS_NUMRAIDREASSIGNEDSUCCESS,	4,						U32_FT,		NotPersistant_PT,
	fdARRAYSTATUS_NUMRAIDREASSIGNEDFAILED,	4,						U32_FT,		NotPersistant_PT,
	fdARRAYSTATUS_NUMRETRIESTOTAL,			4,						U32_FT,		NotPersistant_PT,
	fdARRAYSTATUS_NUMRECOVEREDERRORSTOTAL,	4,						U32_FT,		NotPersistant_PT,
	fdARRAYSTATUS_NUMREASSIGNEDSUCCESSTOTAL,4,						U32_FT,		NotPersistant_PT,
	fdARRAYSTATUS_NUMREASSIGNEDFAILEDTOTAL,	4,						U32_FT,		NotPersistant_PT

};

const U32 cbArrayStatusTable_FieldDefs = sizeof(aArrayStatusTable_FieldDefs); 
