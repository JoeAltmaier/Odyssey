/* TimerStatic.h -- Timer with static callback
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
 * Description:
 * 		This class implements Timers via TimerPIT with static callbacks
 *
**/

// Revision History: 
// 6/15/99 Tom Nelson: Created
//

#ifndef __TimerStatic_h
#define __TimerStatic_h

#include "TimerPIT.h"

//***
//*** TimerStatic
//***

typedef VOID (*TimerCallbackStatic)(void *pPayload);

class TimerStatic : public TimerPIT {
private:
	TimerCallbackStatic pTimerCallback;
	void *pContext;
	
public:
	// Timer is always created disabled.
	TimerStatic(TimerCallbackStatic _pCallback,void *_pContext) : TimerPIT() { 
		pTimerCallback = _pCallback;
		pContext = _pContext;
	}
	~TimerStatic() {}
	
	// Enable (without new callback)
	// Time is in usec.
	STATUS Enable(I64 _timeInitial,I64 _timeReschedule) { 
		Start(_timeInitial,_timeReschedule);
		return OK;
	}
	// Enable (with new callback)
	// Time is in usec.  Max interval is 123 billion seconds, or 3800 years.
	STATUS Enable(I64 _timeInitial,I64 _timeReschedule,TimerCallbackStatic _pCallback) { 
		pTimerCallback = _pCallback;
		Start(_timeInitial,_timeReschedule);
		return OK;
	}
	STATUS Disable() { 
		Stop();
		return OK;
	}
	void Expire() {
		(*pTimerCallback)(pContext);
	}
};

#endif	// __TimerStatic_h

