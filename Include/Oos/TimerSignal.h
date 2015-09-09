/* TimerSignal.h -- Posts Action upon timer expiration
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
 * 		This class weds Signal to TimerPit.
 *
**/

// Revision History: 
// 5/18/99 Joe Altmaier: Created
//

#ifndef __TimerSignal_h
#define __TimerSignal_h

#include "Kernel.h"
#include "TimerPIT.h"

//***
//*** TimerSignal
//***



class TimerSignal: public TimerPIT {
private:
	SIGNALCODE nSignal;
	DdmOsServices *pInst;
	void *pPayload;
	
public:
	TimerSignal(I64 _interval, I64 _repeat, DdmOsServices *_pInst, SIGNALCODE _nSignal, void *_pPayload = NULL)
	:TimerPIT(_interval, _repeat) {
		pInst=_pInst;
		nSignal=_nSignal;
		pPayload=_pPayload;
		Relink();
	}

	void Expire() {
		pInst->Signal(nSignal, pPayload);
	}
};

#endif	// __TimerSignal_h

