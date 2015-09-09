/* DdmTimerTest.h -- Test Timer DDM
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
//  3/27/99 Tom Nelson: Created
//


#ifndef __DdmTestTimer_H
#define __DdmTestTimer_H

#include "RqOsTimer.h"
#include "Ddm.h"

class DdmTimerTest: public Ddm {
	RqOsTimerStart *pTimerMsg;
	
public:
	DdmTimerTest(DID did) : Ddm(did) {}
	static Ddm *Ctor(DID did) { return new DdmTimerTest(did); }

	STATUS Enable(Message *pMsg);
	
	STATUS ProcessTickReply(Message *pMsg);
};
#endif	// __DdmTestTimer_H

