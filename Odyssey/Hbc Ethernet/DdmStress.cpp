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
// This file is the implementation of the Stress device.  It serves 
// REQ_OS_STRESS to start a stress test.  It also serves REQ_OS_STRESS_XFER
// which is the dummy transfer message used in the stress test.
// A stress test is possibly multiple 
// It creates a timer per stress test, and responds periodically
// with updated statistics.
// 
// Update Log: 
// 8/17/98 Joe Altmaier: Create file
/*************************************************************************/

#include "BuildSys.h"
#include "DdmStress.h"
#include "Odyssey_Trace.h"
#include "OsStatus.h"

CLASSNAME(DdmStress, SINGLE);
DEVICENAME(DdmStress, DdmStress::DeviceInitialize);

SERVELOCAL(DdmStress, REQ_OS_STRESS);
SERVELOCAL(DdmStress, REQ_OS_STRESS_XFER);

char aData[1024];

DdmStress::Config DdmStress::config;

DdmStress::DdmStress(DID did):Ddm(did) { 
	DispatchRequest(REQ_OS_STRESS, REQUESTCALLBACK( DdmStress, Stress_Begin ));
	DispatchRequest(REQ_OS_STRESS_XFER, REQUESTCALLBACK( DdmStress, Stress_Echo ));

	nStressTest=0;
	pMsgQuiesce=NULL;
	}

Ddm *DdmStress::Ctor(DID did) { return new DdmStress(did); }

void DdmStress::DeviceInitialize() {
	// Get config info from boot tables
	Config *pConfig
	 =(Config*)Os::GetBootData("Stress");
	config.updateInterval=pConfig->updateInterval;
	}

STATUS DdmStress::Initialize(Message *pMsg) { 
	return Ddm::Initialize(pMsg);
	}
	
STATUS DdmStress::Enable(Message *pMsg) { 
	if (pMsgQuiesce) {
		Reply(pMsgQuiesce, OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE);
		pMsgQuiesce=NULL;
		}

	for (int i=0; i < sizeof(aData); i++)
		aData[i]=i;
			
	return Ddm::Enable(pMsg);
	}
	
STATUS DdmStress::Quiesce(Message *pMsg) { 
	pMsgQuiesce=pMsg;
	
	// If tests running, run them down, return partial statistics.
	
	if (nStressTest == 0) // All done, enter quiescent state.
		return Ddm::Quiesce(pMsgQuiesce);

	return OK;
	}


STATUS DdmStress::IssueRq(ContextXfer &cx, Message &msg) {

		msg.AddSgl(0, cx.pBuf, cx.cbTransfer, cx.bSend);

		// Send transfer message to target IOP.  
		// On target IOP it will go to Stress_Echo.
		// We might even be the target.
		STATUS status=OK;
		if (cx.pContext->vdn != VDNNULL)
			status=Send(cx.pContext->vdn, &msg, &cx, REPLYCALLBACK( DdmStress, Reply_Xfer ));
		else if (cx.pContext->tySlot == Address::iSlotMe)
			status=Send(&msg, &cx, REPLYCALLBACK( DdmStress, Reply_Xfer ));
		else
			status=Send(cx.pContext->tySlot, &msg, &cx, REPLYCALLBACK( DdmStress, Reply_Xfer ));

		if (status == OK)
			cx.pContext->nOutstanding++;
		else {
			Tracef("DdmStress: Failed to reissue transfer request, status=%d\n", status);
			delete cx.pBuf;
			delete &cx;
			delete &msg;
			}

		return status;
		}

// New stress test messages arrive here.
STATUS DdmStress::Stress_Begin(Message *pMsg) {
	if (pMsgQuiesce)
		return Reply(pMsg, OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE);

	// Find test parameters in message payload.
	Payload &payload=*(Payload *)pMsg->GetPPayload();

	// Make a context structure to keep statistics.
	Context &context=*new Context();
	context.pMsg=pMsg;
	context.vdn=payload.vdn;
	context.tySlot=payload.tySlot;
	context.cbTransfer=payload.cbTransfer;
	context.nOutstanding=0;
	context.nIterationSoFar=0;
	context.latencyTotal=0;
	context.timeStart=Time();
	context.timeTotal=0;
	context.nIterationTotal=payload.nOverlap * payload.nIteration;
	
	// Build transfer messages to start test.
	for (int i=0; i < payload.nOverlap; i++) {
		Message &msg=*new Message(REQ_OS_STRESS_XFER);
		msg.flags |= MESSAGE_FLAGS_TIMESTAMP;

		ContextXfer &cx=*new ContextXfer;
		cx.cbTransfer=payload.cbTransfer;
		cx.pContext=&context;
		cx.bSend=(payload.bRd? SGL_REPLY :SGL_SEND);
		if (cx.bSend) {
			cx.pBuf=new (tPCI)char[cx.cbTransfer];
			for (unsigned int ib=0; ib < cx.cbTransfer; ib++)
				cx.pBuf[ib]=ib;
			}
		else
			cx.pBuf=NULL;
			

		IssueRq(cx, msg);
		}

	context.nMessage=context.nOutstanding;

	// Did any test transfer messages start?
	if (context.nOutstanding) {
		// Start a timer for returning periodic statistics.
		RqOsTimerStart *pMp=new RqOsTimerStart(config.updateInterval, config.updateInterval);
{STATUS status=
		Send(pMp, &context, REPLYCALLBACK( DdmStress, Reply_Timer ));
Tracef("TimerStart send status=%d\n", status);
}
		context.pTimer=pMp;
		
		nStressTest++;
		return OK;
		}
		
	else {
Tracef("Test didn't start, replying with %x\n", OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT);
		// Test didn't start, reply now.
		delete &context;
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;
		}
	}

// Transfer message from another IOP.
STATUS DdmStress::Stress_Echo(Message *pMsg) {
//Tracef("DdmStress::Stress_Echo\n");
	SGE_SIMPLE_ELEMENT sse;
	pMsg->GetSgl(0, sse);

	if ((sse.flags & SGL_FLAGS_DIR) == SGL_FLAGS_REPLY)
		pMsg->CopyToSgl(0, 0, aData, sizeof(aData));
		
	// The reply will go to Reply_Xfer on the originating IOP.
	return Reply(pMsg);
	}

// Transfer message reply.
// Count it, reissue.
STATUS DdmStress::Reply_Xfer(MessageReply *pMsg) {
	ContextXfer &cx=*(ContextXfer*)pMsg->GetContext();
	Context &context=*cx.pContext;
//Tracef("DdmStress::Reply_Xfer timeNow=%lx\n", timeNow);

	// Count statistics.
	context.nOutstanding--;
	context.nIterationSoFar++;
	context.latencyTotal += pMsg->Latency();
	context.timeTotal=Time() - context.timeStart;

	// Check content
	SGE_SIMPLE_ELEMENT sse;
	pMsg->GetSgl(0, sse);
	if (cx.bSend == SGL_REPLY)
		if (((U32*)sse.address)[0] != 0x00010203)
			Tracef("DdmStress Reply data bad!  Should be 0x00010203, was %08lx\n", ((U32*)sse.address)[0]);

	// Reissue if necessary.
	if (context.nIterationSoFar < context.nIterationTotal && !pMsgQuiesce)
		IssueRq(cx, *pMsg);
	else {
		if (cx.pBuf)
			delete cx.pBuf;
		delete &cx;
		delete pMsg;
		}

	// Iterations complete?
	if (context.nOutstanding == 0) {

		// Return statistics message for last time.
		((MessageReply*)context.pMsg)->AddPayload(&context.latencyTotal, sizeof(context) - ((char*)&context.latencyTotal - (char*)&context) );
		Reply(context.pMsg, OK);

		// Stop timer
		Message *pMp=new RqOsTimerStop(context.pTimer);
		Send(pMp, (void*)NULL);

		// Disconnect statistics from message.
		context.pMsg=NULL;
		nStressTest--;
		
		// Did we stop because quiescing, and are we quiescent?
		if (pMsgQuiesce)
			Quiesce(pMsgQuiesce);
		
		// Note: timer_stop is still outstanding.  Does that matter?
		}

	return OK;
	}

// Timer reply.
// Return statistics so far.
STATUS DdmStress::Reply_Timer(MessageReply *pMsg) {
Tracef("DdmStress::Reply_Timer.\n");
	Context &context=*(Context*)pMsg->GetContext();

	if (context.pMsg) {
		((MessageReply*)context.pMsg)->AddPayload(&context.latencyTotal, sizeof(context) - ((char*)&context.latencyTotal - (char*)&context) );

		Reply(context.pMsg, OK, false);
		}
		
	else // Free statistics block, iterations complete.
		delete &context;

	// Last reply to our timer, or non-last reply.
	// Either way, delete it.
	delete pMsg;
	
	return OK;
	}
