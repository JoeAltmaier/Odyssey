/* TickTimer.h -- Encapsulates Kernel Timer
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
 * 		This class encapsulates the underlying Kernel timer.
 *
**/

// Revision History: 
// 3/18/99 Tom Nelson: Created
// 8/16/99 Tom Nelson: Rename TickTimer.h to stop conflicts with NucleusNet

#ifndef __TickTimer_h
#define __TickTimer_h

#include "Kernel.h"

//***
//*** Timer
//***

typedef VOID (*TimerCallbackStatic)(void *pPayload);

class TickTimer {
private:
	CT_Timer ctTimer;
	TimerCallbackStatic pTimerCallback;
	
public:
	// Timer is always created disabled.
	TickTimer(TimerCallbackStatic _pCallback,void *_pCallbackContext) { 
		pTimerCallback = _pCallback;
		// HACK: Nucleus Timer callback accepts an UNSIGNED payload we are using to (void *).
		// This will cause problems if using 64 bit pointers!!
		Kernel::Create_Timer(&ctTimer,"CtTimer",(CT_Timer_Callback)pTimerCallback,(UNSIGNED) _pCallbackContext,0,0,CT_DISABLE_TIMER);
	}
	~TickTimer() {
		Kernel::Control_Timer(&ctTimer,CT_DISABLE_TIMER);
		Kernel::Delete_Timer(&ctTimer);
	}
	// Enable (without new callback)
	// Time is in usec.
	STATUS Enable(UNSIGNED _timeInitial,UNSIGNED _timeReschedule) { 
		return Kernel::Reset_Timer(&ctTimer,(CT_Timer_Callback)pTimerCallback,_timeInitial/CT_USEC_MULT,_timeReschedule/CT_USEC_MULT,CT_ENABLE_TIMER);
	}
	// Enable (with new callback)
	// Time is in usec.  Max time is 4,294,967,294 usec. (
	STATUS Enable(UNSIGNED _timeInitial,UNSIGNED _timeReschedule,TimerCallbackStatic _pCallback) { 
		pTimerCallback = _pCallback;
		return Kernel::Reset_Timer(&ctTimer,(CT_Timer_Callback)pTimerCallback,_timeInitial/CT_USEC_MULT,_timeReschedule/CT_USEC_MULT,CT_ENABLE_TIMER);
	}
	STATUS Disable() { 
		return Kernel::Control_Timer(&ctTimer,CT_DISABLE_TIMER);
	}	
};

#endif	// __Timer_h
