/* TestDdm.h -- Class declaration for TestDdm
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
//  4/15/99 Lee Egger: Created
//


#include "RqOsTimer.h"
#include "Ddm.h"
#include "RqOsSysInfo.h"

class TestDdm: public Ddm {
	RqOsTimerStart *pTimerMsg;
	
public:
	
	TestDdm(DID did) : Ddm(did) {}
	static Ddm *Ctor(DID did) { return new TestDdm(did); }

	STATUS Enable(Message *pMsg);
	
	STATUS ShowClassTable(RqOsSiGetClassTable *pMsg);
	
	ERC ExerciseCritical();
	
	STATUS ProcessTickReply(Message *pMsg);	
	
	static int counter;
	
};

int TestDdm::counter = 0;


