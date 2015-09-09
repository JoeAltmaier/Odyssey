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
// This file is the implementation of the sgl test ddm.  It serves 
// REQ_OS_SGL_TEST to start an sgl test.  It also serves REQ_OS_SGL_XFER
// which is the dummy transfer message used in the sgl test.
// 
// Update Log: 
// 4/21/99 Joe Altmaier: Create file
/*************************************************************************/
#include "Odyssey_Trace.h"
#include "BuildSys.h"
#include "DdmSgl.h"
#include "OsStatus.h"
#include "Hw.h"

CLASSNAME(DdmSgl, SINGLE);

SERVELOCAL(DdmSgl, REQ_OS_SGL_TEST);
SERVELOCAL(DdmSgl, REQ_OS_SGL_XFER);

extern U32 gTimeStart;
extern U32 gTimeDone;

DdmSgl::DdmSgl(DID did):Ddm(did) { 
	DispatchRequest(REQ_OS_SGL_TEST, REQUESTCALLBACK( DdmSgl, Sgl_Begin ));
	DispatchRequest(REQ_OS_SGL_XFER, REQUESTCALLBACK( DdmSgl, Sgl_Echo ));

	nSglTest=0;
	pMsgQuiesce=NULL;
	}

Ddm *DdmSgl::Ctor(DID did) { return new DdmSgl(did); }

STATUS DdmSgl::Initialize(Message *pMsg) { 
	return Ddm::Initialize(pMsg);
	}
	
STATUS DdmSgl::Enable(Message *pMsg) { 
	if (pMsgQuiesce) {
		Reply(pMsgQuiesce, OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE);
		pMsgQuiesce=NULL;
		}

	pData=new char[1024];
	for (int i=0; i < 1024; i++)
		pData[i]=i;
			
	return Ddm::Enable(pMsg);
	}
	
STATUS DdmSgl::Quiesce(Message *pMsg) { 
	pMsgQuiesce=pMsg;
	
	// If tests running, run them down, return partial statistics.
	
	if (nSglTest == 0) // All done, enter quiescent state.
		return Ddm::Quiesce(pMsgQuiesce);
	
	return OK;
	}


STATUS DdmSgl::IssueRq(ContextXfer &cx, Message &msg) {

		U32 *pBuf=new U32[cx.cbTransfer >> 2];
		char ch=0;
		for (int i=0; i < (cx.cbTransfer >> 2); i++)
			pBuf[i]=(ch++) * 0x01010101;
		
		char *pBufRet=new char[cx.cbTransfer];
		
//msg.Dump("Original message");
		msg.AddSgl(0, pBuf, cx.cbTransfer, SGL_SEND);
		msg.AddSgl(1, pBuf, cx.cbTransfer, SGL_COPY);
		msg.AddSgl(2, NULL, 0, SGL_REPLY);
		msg.AddSgl(3, NULL, cx.cbTransfer, SGL_REPLY);
		msg.AddSgl(4, pBufRet, cx.cbTransfer, SGL_REPLY);
		msg.AddSgl(5, NULL, 0, SGL_DYNAMIC_REPLY);

//msg.Dump("After AddSgl");
		
		// Send transfer message to target IOP.  
		// On target IOP it will go to Sgl_Echo.
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
			Tracef("DdmSgl: Failed to reissue transfer request, status=%d\n", status);
			delete pBuf;
			delete pBufRet;
			delete &cx;
			delete &msg;
			}

		return status;
		}

// New sgl test messages arrive here.
STATUS DdmSgl::Sgl_Begin(Message *pMsg) {
	if (pMsgQuiesce)
		return Reply(pMsg, OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE);

//Tracef("DdmSgl::Sgl_Begin\n");
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
	context.nIterationTotal=payload.nOverlap * payload.nIteration;
	
	// Build transfer messages to start test.
	for (int i=0; i < payload.nOverlap; i++) {
		Message &msg=*new Message(REQ_OS_SGL_XFER);
		msg.flags |= MESSAGE_FLAGS_TIMESTAMP;

		ContextXfer &cx=*new ContextXfer;
		cx.cbTransfer=payload.cbTransfer;
		cx.pContext=&context;

		IssueRq(cx, msg);
		}

	// Did any test transfer messages start?
	if (context.nOutstanding) {
		nSglTest++;
		return OK;
		}
		
	else {
Tracef("Sgl test didn't start, replying with %x\n", OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT);
		// Test didn't start, reply now.
		delete &context;
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;
		}
	}

// Transfer message from another DdmSgl (maybe us).
STATUS DdmSgl::Sgl_Echo(Message *pMsgArg) {
//Tracef("DdmSgl::Sgl_Echo\n");
//	pMsg->Dump("Message received");
	Message *pMsg=new Message(pMsgArg);

	SGE_SIMPLE_ELEMENT sse;
	int nSgl=pMsg->GetCSgl();
	
	for (int iSgl=0; iSgl < nSgl; iSgl++) {
		pMsg->GetSgl(iSgl, 0, sse, 1024);

		if ((sse.flags & SGL_FLAGS_DIR) == SGL_FLAGS_REPLY)
			pMsg->CopyToSgl(iSgl, 0, pData, 1024);
		else
			if (((U32*)P_SGL(sse))[1] != 0x01010101)
				Tracef("DdmSgl Send data bad!  Should be 0x01010101, was %08lx\n", ((U32*)P_SGL(sse))[0]);
				
		}

//	pMsg->Dump("Message received, GetSgl called");
	Message *pReply=pMsgArg->MakeReply(pMsg);
	
	pMsg->Dump("Prototype used in MakeReply");
	pReply->Dump("Reply contructed by MakeReply");
	
	// The reply will go to Reply_Xfer on the originating IOP.
	STATUS status=Reply(pReply, OK, false);
	if (status != OK)
		return status;
	
//	pMsg->Dump("Reply non-LAST called");
	delete (void*)pMsg;
	pMsg=pMsgArg;
	
	// Do it again, with a LAST reply this time.
	for (int iSgl=0; iSgl < nSgl; iSgl++) {
		pMsg->GetSgl(iSgl, 0, sse, 1024);

		if ((sse.flags & SGL_FLAGS_DIR) == SGL_FLAGS_REPLY)
			pMsg->CopyToSgl(iSgl, 0, pData, 1024);
		}

//	pMsg->Dump("GetSgl again");
				
	return Reply(pMsg, OK, true);
	}

// Transfer message reply.
// Count it, reissue.
STATUS DdmSgl::Reply_Xfer(MessageReply *pMsg) {
	ContextXfer &cx=*(ContextXfer*)pMsg->GetContext();
	Context &context=*cx.pContext;
	context.nIterationSoFar++;

	if ((context.nIterationSoFar % 200) == 0) {
		Tracef("DdmSgl %u\n", context.nIterationSoFar);
		Reply(context.pMsg, OK, false);
		}
		
//	pMsg->Dump("Message reply received");

	// Check content
	SGE_SIMPLE_ELEMENT sse;
	int nSgl=pMsg->GetCSgl();
	
	for (int iSgl=0; iSgl < nSgl; iSgl++) {
		pMsg->GetSgl(iSgl, sse);

		if ((sse.flags & SGL_FLAGS_DIR) == SGL_FLAGS_REPLY) {
			if (sse.count && ((U32*)P_SGL(sse))[0] != 0x00010203)
				Tracef("DdmSgl Reply %08lx[%u] data bad!  Should be 0x00010203, was %08lx\n", P_SGL(sse), iSgl, ((U32*)P_SGL(sse))[0]);

			if (!(sse.flags & SGL_FLAGS_FREE_ON_DTOR) && sse.address)
				delete P_SGL(sse);
			}

		else // SEND
			if (pMsg->IsLast() && !(sse.flags & SGL_FLAGS_FREE_ON_DTOR) && sse.address)
				delete P_SGL(sse);
		}

	// Clean up if necessary.
	if (pMsg->IsLast()) {
		context.nOutstanding--;

		if (context.nIterationSoFar < context.nIterationTotal && !pMsgQuiesce)
			IssueRq(cx, *pMsg);
		else {
			delete &cx;
			delete pMsg;
			}
		}
	else
		delete pMsg;

	// Iterations complete?
	if (context.nOutstanding == 0) {

		// Return test message.
		Reply(context.pMsg, OK);

		// Discard context
		delete &context;
		
		nSglTest--;
		
		// Did we stop because quiescing, and are we quiescent?
		if (pMsgQuiesce)
			Quiesce(pMsgQuiesce);
		}

	return OK;
	}
