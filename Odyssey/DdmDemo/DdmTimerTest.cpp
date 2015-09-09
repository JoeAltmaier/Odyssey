/* DdmTimerTest.cpp -- Test the Timer DDM
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

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <stdio.h>
#include <String.h>
#include "OsTypes.h"

#include "RqOsTimer.h"
#include "DdmTimerTest.h"
#include "BuildSys.h"

// BuildSys linkage
	
CLASSNAME(DdmTimerTest,MULTIPLE);


// .Enable -- Start-it-up ---------------------------------------------------DdmTimerTest-
//
STATUS DdmTimerTest::Enable(Message *pMsgReq)
{
	ERC erc;
	
	Tracef("DdmTimerTest::Enable() did=%x\n",GetDid());

	pTimerMsg = new RqOsTimerStart(1000000,1000000);
	Tracef("RefNum=%lx\n",pTimerMsg->refNum>>32);
	erc =  Send(pTimerMsg, (ReplyCallback) &ProcessTickReply);
	Tracef("DdmTimerTest::Enable; Send()=%u\n",erc);

	Reply(pMsgReq,OK);

	return OK;
}

// .ProcessTickReply -- Handle Replies to keep things going -----------------DdmTimerTest-
//
ERC DdmTimerTest::ProcessTickReply(Message *pRqOsStartTimer) {
	
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
	return OK;
}

