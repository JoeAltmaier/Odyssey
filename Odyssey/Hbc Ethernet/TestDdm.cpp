/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class implements an api for PersistentData service, and the Ddm
// that accomplishes the service.
// 
// Update Log: 
// 8/05/98 Joe Altmaier: Create file
// 10/21/98 Joe Altmaier: diddle
// 4/22/99 Joe Altmaier: add SglTest.

/*************************************************************************/
#include "Odyssey_Trace.h"
#include <String.h>
#include "TestDdm.h"
#include "Os.h"
#include "BuildSys.h"
#include "DdmStress.h"
#include "DdmSgl.h"
#include "OsHeap.h"
#include "TimerAction.h"
#define PROFILE
#ifdef PROFILE
#include "RqProfile.h"
#endif
//#include "TimerSignal.h"

	CLASSNAME(TestDdm, SINGLE);
	
	TestDdm *TestDdm::pTestDdm;

	struct Stats {
		I64 latencyTotal;
		I64 timeTotal;
		U32 nMessage;
		U32 nOutstanding;
		U32 nIterationSoFar;
		};
	
	// Ddm methods

	TestDdm::TestDdm(DID did): Ddm(did) {
		SetConfigAddress(&config, sizeof(config));
		pTestDdm=this;
		}
	
#define TRANSFERSIZE 81920
	
	STATUS TestDdm::Initialize(Message *pMsg) { 
		return Reply(pMsg);
	}

TimerAction *pTimer;

	STATUS TestDdm::Enable(Message *pEnable) { 
Tracef("TestDdm::Enable\n");
		// Start a timer
//		(new TimerAction(this, ActionMethod, (void*)0x1))->Start(1000000, 10000);
//		(new TimerAction(this, ActionMethod, (void*)0x2))->Start(2000000, 2000000);
//		pTimer=new TimerAction(this, ActionMethod, (void*)0x3);
//		pTimer->Start(333333, 333333);
		
//		Send(new RqOsTimerStart(1000000, 1000000), REPLYCALLBACK(TestDdm, ReplyTimer));

		return Reply(pEnable);
		}

	Ddm *TestDdm::Ctor(DID did) {
		return new TestDdm(did);
		}

	void TestDdm::StartStress()
		{
		// Start a stress test
		Message *pMp=new Message(REQ_OS_STRESS, sizeof(Stats)/* +  sizeof(DdmStress::Payload)*/);
		DdmStress::Payload payload;
		payload.cbTransfer=TRANSFERSIZE;
		payload.nIteration=10000;
		payload.nOverlap=1;
		payload.bRd=0;
		payload.vdn=VDNNULL;
		payload.tySlot=IOP_HBC0;
		pMp->AddPayload(&payload, sizeof(payload));
		pTestDdm->Send(pMp, NULL, REPLYCALLBACK(TestDdm, ReplyStress));
		}

	void TestDdm::StartSgl()
		{
		// Start a sgl test
		Message *pMp=new Message(REQ_OS_SGL_TEST, sizeof(DdmSgl::Payload));
		DdmSgl::Payload payload;
		payload.cbTransfer=TRANSFERSIZE;
		payload.nOverlap=1;
		payload.nIteration=100000;
		payload.vdn=VDNNULL;
		payload.tySlot=IOP_HBC0;
		pMp->AddPayload(&payload, sizeof(payload));
{STATUS status=
		pTestDdm->Send(pMp, NULL, REPLYCALLBACK(TestDdm, ReplySgl));
Tracef("Sgl test Send=%08x\n", status);
}
		}

#ifdef PROFILE
	void TestDdm::StartProfile()
		{
		Message *pMsg=new RqProfileStart(1000);
		pTestDdm->Send(pMsg, REPLYCALLBACK(TestDdm, DiscardReply));
		}
		
	void TestDdm::StopProfile()
		{
		Message *pMsg=new RqProfileStop();
		pTestDdm->Send(pMsg, REPLYCALLBACK(TestDdm, DiscardReply));
		}
		
	void TestDdm::ClearProfile()
		{
		Message *pMsg=new RqProfileClear();
		pTestDdm->Send(pMsg, REPLYCALLBACK(TestDdm, DiscardReply));
		}
#endif
		
	STATUS TestDdm::ReplyStress(MessageReply *pMsg) {
		if (pMsg->DetailedStatusCode == OK) {
			struct Stats stats, *pStats;
			pStats=(Stats*)pMsg->GetPPayload();
			stats=*pStats;

			Tracef("[%u:%u]%u ",
				(long)stats.timeTotal, (long)(stats.latencyTotal / stats.nMessage), (long)stats.nOutstanding
				);
			Tracef("%u iterations  ",
				stats.nIterationSoFar / stats.nMessage
				);
			Tracef("%u iterations/sec  %u bytes/sec\n",
				stats.timeTotal? 
					((I64)stats.nIterationSoFar * 1000000 / stats.timeTotal / stats.nMessage)
				: 0l,
				stats.timeTotal? 
					((I64)stats.nIterationSoFar * TRANSFERSIZE * 1000000 / stats.timeTotal)
				: 0l
				);
			}
else pMsg->Dump("ReplyStress\n");

		delete pMsg;
		return OK;
		}

	STATUS TestDdm::ReplySgl(MessageReply *pMsg) {
		if (pMsg->DetailedStatusCode != OK)
			pMsg->Dump("ReplySgl\n");

		delete pMsg;
		Tracef("Memory used: %u\n", OsHeap::heapSmall.cbAllocTotal);
		return OK;
		}

	STATUS TestDdm::ReplyTimer(MessageReply *pMsg) {
		// Timer response
		delete pMsg;
		
		delete pTimer;
		pTimer=new TimerAction(this, ActionMethod, (void*)0x3);
		pTimer->Start(333333, 333333);
		return OK;
		}

int aI[4];
	STATUS TestDdm::ActionMethod(void *pPayload) {
//		Tracef("[TIMERACTION] %08lx\n", pPayload);
		aI[(int)pPayload]++;
		if ((aI[1] % 10000) == 0)
			Tracef("1 - %d\n2 - %d\n3 - %d\n\n", aI[1], aI[2], aI[3]);
		return OK;
		}
