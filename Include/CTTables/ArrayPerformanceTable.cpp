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
// File Name: ArrayPerformanceTable.cpp
// 
// Description: This file defines the array Performance Table fields
// 
// $Log: /Gemini/Include/CTTables/ArrayPerformanceTable.cpp $
// 
// 5     11/02/99 8:56a Vnguyen
// Add read and write performance counters.  Also scale all counters to
// per second.
// 
// 4     9/15/99 2:31p Vnguyen
// Update performance and status counters.  Remove variance info for the
// performance counters becuase nobody is using it.
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

#include "ArrayPerformanceTable.h"

const fieldDef	aArrayPerformanceTable_FieldDefs[] = 
{
// 	Field Name						
//	fdARRAYPERFORMANCE_RID,								8,						ROWID_FT,	Persistant_PT,
	fdARRAYPERFORMANCE_VERSION,							4,						U32_FT,		Persistant_PT,
	fdARRAYPERFORMANCE_SIZE,							4,						U32_FT,		Persistant_PT,
	fdARRAYPERFORMANCE_KEY,								4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_REFRESHRATE,						4,						U32_FT,		Persistant_PT,
	fdARRAYPERFORMANCE_SAMPLERATE,						4,						U32_FT,		Persistant_PT,
	fdARRAYPERFORMANCE_RIDSRCRECORD,					8,						ROWID_FT,	Persistant_PT,
	
	fdARRAYPERFORMANCE_NUMRDSAVERAGEPERSEC,					NUM_MEASURE_SIZES * 4,	U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMRDSMINIMUMPERSEC,					NUM_MEASURE_SIZES * 4,	U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMRDSMAXIMUMPERSEC,					NUM_MEASURE_SIZES * 4,	U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMWRTSAVERAGEPERSEC,				NUM_MEASURE_SIZES * 4,	U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMWRTSMINIMUMPERSEC,				NUM_MEASURE_SIZES * 4,	U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMWRTSMAXIMUMPERSEC,				NUM_MEASURE_SIZES * 4,	U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMREADSAVERAGEPERSEC,				4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMREADSMINIMUMPERSEC,				4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMREADSMAXIMUMPERSEC,				4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMWRITESAVERAGEPERSEC,				4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMWRITESMINIMUMPERSEC,				4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMWRITESMAXIMUMPERSEC,				4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMBLOCKREADSAVERAGEPERSEC,			4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMBLOCKREADSMINIMUMPERSEC,			4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMBLOCKREADSMAXIMUMPERSEC,			4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMBLOCKWRITESAVERAGEPERSEC,			4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMBLOCKWRITESMINIMUMPERSEC,			4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMBLOCKWRITESMAXIMUMPERSEC,			4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMSGCOMBINEDREADSAVERAGEPERSEC,		4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMSGCOMBINEDREADSMINIMUMPERSEC,		4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMSGCOMBINEDREADSMAXIMUMPERSEC,		4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMSGCOMBINEDWRITESAVERAGEPERSEC,	4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMSGCOMBINEDWRITESMINIMUMPERSEC,	4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMSGCOMBINEDWRITESMAXIMUMPERSEC,	4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMOVERWRITESAVERAGEPERSEC,			4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMOVERWRITESMINIMUMPERSEC,			4,						U32_FT,		NotPersistant_PT,
	fdARRAYPERFORMANCE_NUMOVERWRITESMAXIMUMPERSEC,			4,						U32_FT,		NotPersistant_PT

};

const U32 cbArrayPerformanceTable_FieldDefs = sizeof(aArrayPerformanceTable_FieldDefs); 
