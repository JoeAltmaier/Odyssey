/* EisCodes.h -- Defines Odyssey Error Injection ID Codes
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
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

//
// $Log: /Gemini/Include/EISTriggers.h $
// 
// 3     5/26/99 1:25p Bbutler
// 
// 2     4/20/99 9:46p Bbutler
// 
 
#ifdef _BuildTriggerIDTable
#undef EISTRIGGERS_H
#endif

#if !defined(EISTRIGGERS_H) // || defined(_BuildTriggerIDTable)
#define EISTRIGGERS_H

#include "BuildTriggerID.h"

// Define all classes of injected errors
// (defines upper 16 bites of 32 bit ID code)

DEFINE_EIS_ERROR_CLASSES

	EIS_ERROR_CLASS(EIS_TEST)
	EIS_ERROR_CLASS(DMA)
	
END_EIS_ERROR_CLASSES

// Trigger ID
// (defines lower 16 bites of 32 bit ID code)
//
DEFINE_EIS_ID_CODES(EIS_TEST)

	EIS_ID_CODE(EID_EIS_ERROR_1)
	EIS_ID_CODE(EID_EIS_ERROR_2)
	EIS_ID_CODE(EID_EIS_ERROR_3)
	
END_EIS_ID_CODES(EIS_TEST)

DEFINE_EIS_ID_CODES(DMA)

	EIS_ID_CODE(EID_DMA_TRIGGER_1)
	EIS_ID_CODE(EID_DMA_TRIGGER_2)
	EIS_ID_CODE(EID_DMA_TRIGGER_3)
	
END_EIS_ID_CODES(DMA)


#ifdef ENABLE_ERROR_INJECTION

#include "DdmTriggerMgr.h"
// Real macros will go here
#define TriggerCheck(ID, arg) DdmTriggerMgr::Trigger(ID, arg)

#define TriggerClear(ID) DdmTriggerMgr::Clear(ID)

#define TriggerClassClear(classID) DdmTriggerMgr::ClassClear(classID)

#else
// Stubbed-out macros for when Error Injection is disabled.

#define TriggerCheck(ID, parameter) 0

#define TriggerClear(ID) 

#define TriggerClassClear(classID) 

#endif

#endif	// EISTRIGGERS_H
