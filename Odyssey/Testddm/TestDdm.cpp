/* TestDdm.cpp -- Lee learns to write Ddms
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

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include "BuildSys.h"

#include "TestDdm.h"
#include "Critical.h"
#include "RqOsTimer.h"


// BuildSys linkage
	
CLASSNAME(TestDdm, SINGLE);


// .Enable -- Start-it-up ---------------------------------------------------DdmTimerTest-
//
STATUS TestDdm::Enable(Message *pMsgReq)
{
	
	ERC erc;
	U32 iDid;
	
		RqOsSiGetClassTable*	pMsg = new RqOsSiGetClassTable();

		iDid = GetDid();
		
		counter++;
	
		Tracef("TestDdm::Enable() did=%x\n",iDid);

		erc =  Send(pMsg, (ReplyCallback) &ShowClassTable);
		if (erc != OS_DETAIL_STATUS_SUCCESS)
			delete pMsg;
		
	Reply(pMsgReq,OK);

	return OK;
}

STATUS TestDdm::ShowClassTable(RqOsSiGetClassTable *pMsg)
{
	ERC erc;
	
	//Tracef("In ShowClassTable %d\n", counter);
	counter++;
	//Tracef("Payload.szClassName=%s\n",pMsg->payload.szClassName);
	//Tracef("Payload.cClass=%u\n",pMsg->payload.cClass);
	//Tracef("Payload.iClass=%u\n",pMsg->payload.iClass);
	//Tracef("Payload.cbStack=%u\n",pMsg->payload.cbStack);
	//Tracef("Payload.sQueue=%u\n",pMsg->payload.sQueue);
	//Tracef("Payload.flags=%u\n",pMsg->payload.flags);
	//Tracef("Payload.version=%u\n",pMsg->payload.version);
	//Tracef("Payload.nServeLocal=%u\n",pMsg->payload.nServeLocal);
	//Tracef("Payload.nServeVirtual=%u\n",pMsg->payload.nServeVirtual);
	//Tracef("Payload.nDidInstance=%u\n",pMsg->payload.nDidInstance);
	//Tracef("Payload.nDdmInstance=%u\n",pMsg->payload.nDdmInstance);

	
	if (pMsg->payload.iClass+1 == pMsg->payload.cClass) {	
		Tracef("Version 1.1 (SWQA Test Version for HW rev B");
	
		pTimerMsg = new RqOsTimerStart(1000000,1000000);
		Tracef("RefNum=%lx\n",pTimerMsg->refNum>>32);
	
		erc =  Send(pTimerMsg, (ReplyCallback) &ProcessTickReply);
	}	

	delete pMsg;
	return OK;	

}


ERC TestDdm::ExerciseCritical()
{
	U32 ret;

	ret = CriticalEnter();
	//Tracef("Critical Return = %u\n", ret);
	CriticalLeave(ret);
	
	return OK;
}

ERC TestDdm::ProcessTickReply(Message *pRqOsStartTimer) {
	
	RqOsTimerReset*pMsg;
	static U32 total = 0;

	delete pRqOsStartTimer;
	
	Tracef("Tick....");
	switch (++total) {
	case 10:
		Tracef("FAST!\n");
		pMsg = new RqOsTimerReset(pTimerMsg,250000,250000);
		Send(pMsg,&DiscardReply) ;
		break;
	case 50:
		Tracef("FASTER!;\n");
		pMsg = new RqOsTimerReset(pTimerMsg,125000,125000);
		Send(pMsg,&DiscardReply) ;
		break;
	case 100:
		Tracef("SLOWER\n");
		pMsg = new RqOsTimerReset(pTimerMsg,2000000,2000000);
		Send(pMsg,&DiscardReply) ;
		break;
	case 110:
		Tracef("\nThat's all!\n");
		RqOsTimerStop *pMsg = new RqOsTimerStop(pTimerMsg);
		Send(pMsg,&DiscardReply);
	}
	
		
//	erc = ExerciseCritical();

	return OK;
}

