/*
 * ErrorTrigger.h - Error Injection Trigger Class
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
 * Revision History:
 *     4/6/1999 Bob Butler: Moved from ErrorInjectionService.cpp
 *
*/
#ifndef ErrorTrigger_h
#define ErrorTrigger_h

#include "OsTypes.h"
#include <string.h>
#include <stdio.h>

class ErrorTrigger
{
public:
	enum TriggerState { DISARMED, ARMED, TRIGGERED, CLEARED };

	static ErrorTrigger *CreateTrigger(char *_pScriptLine);
	
	ErrorTrigger() : phase(0), slot(0), ID(0), triggerIteration(0), iteration(0), parameter(0) {}
	ErrorTrigger (const ErrorTrigger &_et);
	ErrorTrigger (U32 _phase, U32 _slot, U32 _ID, U32 _iteration, U32 _parameter)
		: phase(_phase), slot(_slot), iteration(_iteration), ID(_ID), parameter(_parameter) 
		{		}
	virtual ~ErrorTrigger();
	void PersistArgs(U32 *aArgs)
	{ aArgs[0] = phase; aArgs[1] = slot; aArgs[2] = ID; aArgs[3] = iteration; aArgs[4] = parameter; }
	
	ErrorTrigger &operator =(const ErrorTrigger &_et);
	void SetArmed() { triggerState = ARMED; }
	void SetDisarmed() { triggerState = DISARMED; }
	void SetCleared() { triggerState = CLEARED; }
	void SetTriggered() { triggerState = TRIGGERED; }
	BOOL Trigger(U32 _ID, U32 _parameter);
	BOOL IsArmed() { return triggerState == ARMED; }
	TriggerState GetState() { return triggerState; }
	U32 GetPhase() { return phase; }
	U32 GetSlot() { return slot; }
	U32 GetID() { return ID; }

	BOOL Compare (U32 _phase, U32 _slot, U32 _ID, U32 _iteration, U32 _parameter)
	{
		return ID == _ID 
		    && phase == _phase && slot == _slot 
		    && iteration == _iteration && parameter == _parameter;
	}	
	BOOL operator== (ErrorTrigger &_et) { return _et.Compare (phase, slot, ID, iteration, parameter); };

private:
	ErrorTrigger::TriggerState triggerState;
	U32 ID;
	U32 phase;
	U32 slot;
	U32 iteration; // iteration to trigger on
	U32 triggerIteration;  // current iteration 
	U32 parameter;
};

#endif