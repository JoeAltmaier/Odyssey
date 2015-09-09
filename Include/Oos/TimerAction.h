/* TimerAction.h -- Posts Action upon timer expiration
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
 * 		This class weds Action to Timer.
 *
**/

// Revision History: 
// 5/18/99 Joe Altmaier: Created
//

#ifndef __TimerAction_h
#define __TimerAction_h

#include "Kernel.h"
#include "DdmOsServices.h"
#include "TimerPIT.h"

//***
//*** TimerAction
//***

class TimerAction: public TimerPIT {
private:
	DdmServices::ActionCallbackStatic acs;
	DdmServices::ActionCallback ac;
	DdmServices *pInst;
	void *pPayload;
	BOOL fACS;

	// Private because we don't want it.
	TimerAction(TimerAction&);
		
public:
	// Timer is always created disabled.
	TimerAction(DdmServices *_pInst, DdmServices::ActionCallbackStatic _acs, void *_pPayload)
	: TimerPIT() {
		pInst = _pInst;
		acs = _acs;
		pPayload = _pPayload;
		fACS = true;
	}
		
	TimerAction(DdmServices *_pInst, DdmServices::ActionCallback _ac, void *_pPayload)
	: TimerPIT() {
		pInst = _pInst;
		ac = _ac;
		pPayload = _pPayload;
		fACS = false;
	}
	// Start() in base class
	// Stop()  in base class
	void Expire() {
		if (fACS)
			pInst->Action(acs, pPayload);
		else
			pInst->Action(pInst, ac, pPayload);
	}
};

#endif	// __TimerAction_h

