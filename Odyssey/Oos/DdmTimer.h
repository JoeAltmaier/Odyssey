/* DdmTimer.h -- Implementation of the Timer DDM.
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
 * Copyright (C) Dell Computer, 2000
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
//  8/17/98 Joe Altmaier: Create file
//  ** Log at end-of-file **

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#ifndef __DdmTimer_h
#define __DdmTimer_h

#include "RqOsTimer.h"
#if defined(_WIN32) or not defined(_ODYSSEY) or 1
#include "TickTimer.h"
#else
#include "TimerStatic.h"
#endif

class DdmTimer: public Ddm {
	typedef struct TimerContext {
		static TimerContext *pHead;
		TimerContext *pNext;
		RqOsTimerStart *pMsg;
		DdmTimer *pDdm;
		BOOL fDone;
#if defined(_WIN32) or not defined(_ODYSSEY) or 1
		TickTimer timer;
#else
		TimerStatic timer;
#endif
		TimerContext(TimerCallbackStatic _pCallback, DdmTimer *_pDdm, RqOsTimerStart *_pMsg) 
		: timer(_pCallback,(void*) this), pDdm(_pDdm), pMsg(_pMsg), fDone(false) {
			pNext=pHead;
			pHead=this;
		}
		~TimerContext();
	} TimerContext;
		
public:
	DdmTimer(DID did);
	static Ddm *Ctor(DID did)	{ return new DdmTimer(did); }

protected:
	ERC virtual Terminate(RqOsDdmTerminate *_pRequest);
	
	ERC ProcessTimerStart(Message* _pRequest);
	ERC ProcessTimerReset(Message* _pRequest);
	ERC ProcessTimerStop(Message* _pRequest);
	ERC DeleteTimerContext(void *_pContext);

	static void HandleTimerExpire(void *_pContext);
};

#endif	// __DdmTimer_h

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmTimer.h $
// 
// 10    2/08/00 8:56p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 11    2/08/00 6:07p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 9     12/09/99 2:07a Iowa
// 
// Revision History: 
//  6/27/99 Tom Nelson:	  Use RequestCode enum from Timer Message classes
//						  Use TimerPIT only when on Odyssey platform
//  3/27/99 Tom Nelson:   Support derived timer request classes
//  3/18/99 Tom Nelson:   Changed to use Timer.h and SERVE macros
//  3/04/99 Tom Nelson:	  Change NU_Create_Timer to Kernel::Create_Timer
//  8/17/98 Joe Altmaier: Create file
