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
/*************************************************************************/
#define _DEBUG
#include <String.h>
#include "TestDdm.h"
#include "Oos.h"
#include "BuildSys.h"
#include "DdmStress.h"
#include "Odyssey_Trace.h"

	CLASSNAME(TestDdm);
	
//	static 
	BOOL TestDdm::fStress=false;
	
	// Ddm methods

	TestDdm::TestDdm(DID did): Ddm(did) {
		SetConfigAddress(&config, sizeof(config));
		}
	
#define TRANSFERSIZE 8192

	STATUS TestDdm::Initialize() {
Tracef("TestDdm::Initialize\r\n");
		return I2O_DETAIL_STATUS_SUCCESS;
		}
		
	void TestDdm::Enable(Message *pEnable) { 
		{
		// Start a timer
		MessagePrivate *pMp=new MessagePrivate(OOS_TIMER_START);
		struct {long time0; long timeN;} payload={1000000, 1000000};
		pMp->AddPayload(&payload, sizeof(payload));
//		Send(pMp, NULL, (ReplyCallback)&ReplyTimer);
		}
		if (fStress)
		{
		// Start a stress test
		MessagePrivate *pMp=new MessagePrivate(OOS_STRESS);
		DdmStress::Payload payload;
		payload.cbTransfer=TRANSFERSIZE;
		payload.nIteration=1048576;
		payload.nOverlap=1;
		payload.bRd=0;
		payload.tySlot=IOP_HBC0;
		pMp->AddPayload(&payload, sizeof(payload));
		Send(pMp, NULL, (ReplyCallback)&ReplyStress);
		}

		Ddm::Enable(pEnable);
		}

	Ddm *TestDdm::Ctor(DID did) {
		return new TestDdm(did);
		}


	STATUS TestDdm::ReplyStress(MessageReply *pMsg) {
		if (pMsg->DetailedStatusCode == I2O_DETAIL_STATUS_SUCCESS) {
			struct Stats {
				U32 nOutstanding;
				U32 nIterationSoFar;
				U32 latencyTotal;
				U32 timeTotal;
				} *pStats;
			pStats=(Stats*)pMsg->GetPPayload();

//Tracef("%lx %lx %lx %lx\n", pStats->nOutstanding, pStats->nIterationSoFar, pStats->latencyTotal, pStats->timeTotal);
			Tracef("[%u]%u ",
				pStats->timeTotal, pStats->nOutstanding
				);
			Tracef("%u ITERATIONS  ",
				pStats->nIterationSoFar
				);
			Tracef("%u PER SECOND  %u bytes/sec\n",
				pStats->timeTotal? 
					(pStats->nIterationSoFar * 1000000 / pStats->timeTotal)
				: 0,
				pStats->timeTotal? 
					(pStats->nIterationSoFar * TRANSFERSIZE * 1000000 / pStats->timeTotal)
				: 0
				);
			}
else Tracef("ReplyStress stats=%d\n", pMsg->DetailedStatusCode);

		delete pMsg;
		return I2O_DETAIL_STATUS_SUCCESS;
		}
/*
	STATUS TestDdm::ReplyBsa(MessageReply *pMsg) {
		if (!(lba % 0x10000000)) {
			int ticks1=NU_Retrieve_Clock();
			Tracef(" nTicks=%lx nMsg=512K, three queue ops per msg.\r\n", (ticks1-ticks));
			ticks=ticks1; 
			Tracef(" nTimes: %lx\r\n", nTimes++);
			}
					
		lba += BLKSIZE;
		MessageBsa &msg=*(MessageBsa*)pMsg;
		msg.LogicalByteAddress=lba % 0x100000;
		msg.TransferByteCount=BLKSIZE;
		msg.AddSgl(0, pBuf, BLKSIZE);
STATUS status=
		Send(pMsg, config.vd);
if (status != I2O_DETAIL_STATUS_SUCCESS)
Debug("Bsa Send failed: ");Debug(status);Debug("\r\n");
		return I2O_DETAIL_STATUS_SUCCESS;
		}
*/

	STATUS TestDdm::ReplyTimer(MessageReply *pMsg) {
		// Timer response
		Tracef("\r[TIMER] IsLast=%d\r", pMsg->IsLast());
		delete pMsg;
		return I2O_DETAIL_STATUS_SUCCESS;
		}
