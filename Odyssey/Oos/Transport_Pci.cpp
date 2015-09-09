

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
// This class implements message transport.
// 
// Update Log: 
// 08/14/99 Joe Altmaier: Created from Transport.cpp
/*************************************************************************/

#define _TRACEF
#include "Odyssey_Trace.h"
#define	TRACE_INDEX		TRACE_TRANSPORT

#include "Transport_Pci.h"

#include "Interrupt.h"
#include "DdmManager.h"
#include "OsTypes.h"
#include "Galileo.h"
#include "BuildSys.h"
#include "CtEvent.h"

#define ONESEC 1000000
	
	DEVICENAME(Transport_Pci, Transport_Pci::DeviceInitialize);
	
	char *Transport_Pci::pMsgs;
	U32 Transport_Pci::sMsgs;
	char *Transport_Pci::pBufs;
	U32 Transport_Pci::sBufs;
	
	HeapPci Transport_Pci::heapPci;

// Public API

	Transport_Pci::Transport_Pci(TySlot tySlot_, PciSlot &ps_):TransportBase(tySlot_), pciSlot(ps_) {
		}

	TransportBase* Transport_Pci::Ctor(TySlot tySlot) {
		return new Transport_Pci(tySlot, *new PciSlot(tySlot, Address::aSlot[tySlot].pciBase, Address::IOP_BOOTING));
		}

	void Transport_Pci::DeviceInitialize() {
TRACEF(TRACE_L8, ("ENTRY Transport_Pci::DeviceInitialize\n"));
		Transport::Register(Transport::TRANSPORT_PCI, Transport_Pci::Ctor);

		struct MP {U32 cbMsg; U32 sMsg; U32 cbBuf; U32 sBuf; U32 pciWindowLo; U32 pciWindowHi; U32 aPciWindow[NSLOT]; U32 aSWindow[NSLOT];} *pTransportParams
		 =(MP*)Oos::GetBootData("Transport");

// Use block pool
		char *pPool = new (tBIG|tPCI) char[TRANSPORT_QUEUE_SIZE * 5];

		// If crosses a megabyte boundary, allocate again.
		if ((((U32)pPool) & 0xFFF00000) != (((U32)pPool + TRANSPORT_QUEUE_SIZE * 5 - 1) & 0xFFF00000))
			pPool = new (tBIG|tPCI) char[TRANSPORT_QUEUE_SIZE * 5];

		// Align
		U32 cbOver  = ((U32)pPool) % TRANSPORT_QUEUE_SIZE;
		U32 cbWaste = (cbOver? TRANSPORT_QUEUE_SIZE - cbOver :0);
		pPool += cbWaste;
		heapPci.I2oLayout((U32)pPool, TRANSPORT_QUEUE_SIZE);

		pMsgs = new (tBIG|tPCI) char[pTransportParams->cbMsg];
		sMsgs = pTransportParams->cbMsg;
		heapPci.I2oFreeInit(pMsgs, sMsgs, pTransportParams->sMsg);

		pBufs = new (tBIG|tPCI) char[pTransportParams->cbBuf];
		sBufs = pTransportParams->cbBuf;
		heapPci.I2oBufInit(pBufs, sBufs, pTransportParams->sBuf);

		// Publish buffer sizes
		heapPci.PublishBufferSizes(pTransportParams->sMsg, pTransportParams->sBuf);

		// Initialize inbound mechanism
		Interrupt::SetHandler(Interrupt::tyI2oInPost, Int_Inbound, 0);
		Interrupt::SetHandler(Interrupt::tyI2oInDoor, Int_Inbound, 0);

TRACEF(TRACE_L8, ("EXIT  Transport_Pci::DeviceInitialize\n"));
		}

// Derived Transport methods
	void Transport_Pci::SetState(Address::StateIop state) {
		pciSlot.SetState(state);
		}

	STATUS Transport_Pci::GetStatus() { return pciSlot.GetStatus(); }

	STATUS Transport_Pci::SendRemote(Message *pMsg, DID didTarget) {
Log('Send', (U32)pMsg);
Log(' did', (U32)didTarget);
Log('slot', (U32)tySlot);
TRACEF(TRACE_L8, ("SendRemote to %lx on slot %x.\n", didTarget, tySlot));

		Status status_=pciSlot.GetStatus();
		
		// Destination active?
		switch (status_) {
		case CTS_CHAOS_TPT_IOP_INACCESSIBLE:
			// We can't talk to the slot
			Transport::NotifyFailure(pciSlot.tySlot, status_);
			// fall through to next case

		case CTS_CHAOS_TPT_IOP_NOT_READY:
			// We've previously noticed the IOP is not active.
			// Leave msg queued, or return to sender.
			Transport::SendFailed(pMsg, status_);
			return OK;

		default:// OK, active
			break;
			}

		// Allocate buffer for message itself
		int cbHeader = pMsg->sMessage << 2;
		U32 pciAlloc;
		U32 cb = pciSlot.GetRemoteBuffer(cbHeader, pciAlloc);
TRACEF(TRACE_L8, ("GetRemoteBuffer returned %08lx\n", pciAlloc));
Log(' pci', (U32)pciAlloc);

		if (!cb || !pciAlloc || pciAlloc == (U32)-1) {
Tracef("Failure(1)! Congestion allocating %u. didTarget=%x\n", cbHeader,didTarget);
			Transport::Fail(pciSlot.tySlot, CTS_CHAOS_TPT_SEND_CONGESTION);
			Transport::SendFailed(pMsg, CTS_CHAOS_TPT_SEND_CONGESTION);
			return OK;
			}

		U32 cbUsed = ((cbHeader + 7) & 0xFFFFFFF8);

		// Record buffers in case we need to abort and free them all.
		BufferList &blPci=*new BufferList(pciSlot, pciAlloc);
//		blPci.Add(pciAlloc); - target will free msg buffer

		// Pack didTarget into contextTransaction signature field so remote IOP can do local routing
		// Put blPci into contextTransaction field so we can find it in SendFixup, Int_Inbound
		// If routed by board but no idLocal, reroute on destination.  Signal this by using DIDNULL.
		if (DeviceId::IdLocal(didTarget) == IDNULL)
			didTarget=DIDNULL;

Log('bind', (U32)didTarget);
		pMsg->contextTransaction.HighPart = didTarget;
		pMsg->contextTransaction.LowPart = (U32)&blPci;
		
		// Copy message to dma buffer
		Message *pMsgBuf=(Message*)new char[cbUsed];
		*pMsgBuf = *pMsg;
		pMsg=pMsgBuf;

		// Record 1st dma request
		TyDma *pTd=new TyDma(NULL, pMsg, P_PCI(pciAlloc), cbUsed, this, CALLBACK(Transport_Pci, SendFixup), pMsg);
		TyDma *pTdLast=NULL;

		// Assign addresses to write SGL.  Allocate buffers as needed.
		// Start with remainder of message buffer.
		pciAlloc += cbUsed;
		cb -= cbUsed;

		// Assign addresses		
		U32 nSgl = pMsg->GetCSgl();
		for (U32 iSgl=0; iSgl < nSgl; iSgl++) {
			SGE_SIMPLE_ELEMENT *pSse = pMsg->GetPSgl(iSgl);
			
			// If write buffer, copy to remote buffer
			if (pSse->flags & SGL_FLAGS_DIR) {
				BOOL fEob=false;
				while (!fEob) {
					if (pSse->count > cb) {
						// Allocate another buffer.
						cb = pciSlot.GetRemoteBuffer(pSse->count, pciAlloc);
Log(' pci', (U32)pciAlloc);
						if (!cb || !pciAlloc || pciAlloc == (U32)-1) {
							pciSlot.FreeRemoteBuffer(blPci.paPost);
							delete &blPci;
Tracef("Failure(2)! Congestion allocating %u. didTarget=%x\n", pSse->count,didTarget);
							pMsg=pMsg->POriginalMessage();
							delete (void*)pMsgBuf;
							Transport::Fail(pciSlot.tySlot, CTS_CHAOS_TPT_SEND_CONGESTION);
							Transport::SendFailed(pMsg, CTS_CHAOS_TPT_SEND_CONGESTION);
							return OK;
							}
						blPci.Add(pciAlloc);
						}
					
					cbUsed=((pSse->count + 7) & 0xFFFFFFF8);

					// Record dma request
					pTd=new TyDma(pTd, (void*)pSse->address, P_PCI(pciAlloc), cbUsed);
						
					pSse->address=pciAlloc;
					pSse->flags &= ~SGL_FLAGS_LOCAL_ADDRESS;
					// Buffer not to be freed when remote msg deleted.
					// We will free it after reply.
					pSse->flags &= ~SGL_FLAGS_FREE_ON_DTOR;

					pciAlloc += cbUsed;
					cb -= cbUsed;

					fEob=pSse->IsEob();
					pSse++;
					}
				}

			else /* Read SGL */
			 if (pSse->address != 0 && !IS_PCI(PA_SGL((*pSse)))) {
				// Read sgl, not pci memory.  Allocate pci memory.
				BOOL fEob=false;
				while (!fEob) {
					TyDma *pTdNew=new TyDma(NULL, NULL, (void*)pSse->address, pSse->count, NULL, (void*)iSgl);
					if (pTdLast)
						pTdLast->pNext=pTdNew;
					else
						blPci.pTd=pTdNew;
					pTdLast=pTdNew;

					// Client must allocate buffer
					pSse->address=0;
					pSse->flags &= ~SGL_FLAGS_LOCAL_ADDRESS;

					fEob=pSse->IsEob();
					pSse++;
					}
				}
			}

		if (pTdLast)
			pTdLast->SetCallback(InboundFixup);
			
		// Fix message sgl to point to reply buffers as pci addresses.
		pMsg->GlobalizeSgl();
Log('glob', 0);

		// Copy message to remote transfer buffer(s)
		return Dma::Transfer(pTd);
		}

	// Continue here after message copied to pRemote
	void Transport_Pci::SendFixup(void *pMsgArg, Status status) {
		Message *pMsgBuf=(Message*)pMsgArg;
		BufferList &blPci=*(BufferList*)pMsgBuf->GetContext();
		
		// Free dma buffer, use original message.
		Message *pMsg=pMsgBuf->POriginalMessage();
		delete (void*)pMsgBuf;

Log('fixu', (U32)pMsg);
		
		if (status == OK) {
			// Queue message transfer buffer to remote IOP
Log('post', (U32)blPci.paPost);
			if (!pciSlot.Post(blPci.paPost))
				status=CTS_CHAOS_TPT_POST_FAIL;
			}
		
		// Any status errors here, handle by deferring or
		// by returning a Reply with status code.
		if (status != OK) {
TRACEF(TRACE_L1, ("Send to %d failed, status=%x\n", blPci.pciSlot.tySlot, status));
			TySlot tySlot=blPci.pciSlot.tySlot;
			
			// Try to return the buffer if at all possible.  Ignore any errors.
			blPci.pciSlot.FreeRemoteBuffer(blPci.paPost);
			delete &blPci;

			Transport::SendFailed(pMsg, status);
			Transport::NotifyFailure(tySlot, status);
			}

		// Reply comes back through Int_Inbound().
		}


	STATUS Transport_Pci::ReplyRemote(Message *pMsg) {
		// Return pMsg to tySlot.
Log('Rply', (U32)pMsg);
TRACEF(TRACE_L8, ("ReplyRemote to slot %x.\n", tySlot));

		Status status_=pciSlot.GetStatus();

		switch (status_) {
		case CTS_CHAOS_TPT_IOP_INACCESSIBLE:
			// We can't talk to the slot
			Transport::NotifyFailure(pciSlot.tySlot, status_);
			// fall through to next case

		case CTS_CHAOS_TPT_IOP_NOT_READY:
			// We've previously noticed the IOP is not active.

			Transport::ReplyFailed(pMsg, status_);
			return OK;

		default:// OK, active
			break;
			}

		pMsg->GlobalizeSgl();
		
		// Allocate transfer buffer
		U32 pciRemote;
		U32 cb = pciSlot.GetRemoteBuffer((U32)(pMsg->sMessage << 2), pciRemote);
		if (!pciRemote || pciRemote == (U32)-1) {
			Transport::Fail(pciSlot.tySlot, CTS_CHAOS_TPT_REPLY_CONGESTION);
			Transport::ReplyFailed(pMsg, CTS_CHAOS_TPT_REPLY_CONGESTION);
			return OK;
			}

Log(' pci', (U32)pciRemote);
		// Prepare msg header for copy back to initiator
		pMsg->contextTransaction.LowPart=pciRemote; // So we can get it back out in ReplyFixup

		InitiatorContext *pIc=pMsg->PopIc();
		if (pIc && pMsg->IsLast())
			delete pIc;

		// Copy message to remote transfer buffer
Log(' dma', (U32)pMsg->sMessage << 2);
		return Dma::Transfer(pMsg, (char*)P_PCI(pciRemote), pMsg->sMessage << 2, this, CALLBACK(Transport_Pci, ReplyFixup), pMsg);
		}
		

	// Continue here after message copied to pciRemote
	void Transport_Pci::ReplyFixup(void *pMsgArg, Status status) {
		Message *pMsg = (Message*)pMsgArg;
		U32 pciRemote = (U32)pMsg->GetContext();
Log('RepF', (U32)pciRemote);
		TySlot iSlot = DeviceId::ISlot(pMsg->didInitiator);

		// Done with pMsg
		// Erase pTc because it belongs to another IOP (not pointing to local memory)
		pMsg->pInitiatorContext=NULL;
		delete (void*)pMsg;
				
		// Any status errors here, handle by dropping reply on floor!
		if (status != OK) 
			pciSlot.FreeRemoteBuffer(pciRemote);
		else
			pciSlot.Post(pciRemote);
Log('Post', (U32)iSlot);
		}


	void Transport_Pci::Int_Inbound(long ) {
Log('IntI', (U32)0);
		// HISR level interrupt routine for Galileo Inbound I2O POST queue
		
		// Scan freed buffer queue, copy to available queue
		U32 pci;
		while ((pci=heapPci.GetLocalReturnBuffer()) != 0xFFFFFFFF)
{ 
Log('pciF', (U32)pci);
			delete PLOCAL_PCI(pci);
}

		// Scan inbound message post queue, process messages
		while ((pci=heapPci.GetLocalMessage()) != 0xFFFFFFFF) {
Log('pciM', (U32)pci);

			// Map pci to local memory pointer.
			Message *pMsg=(Message*)PLOCAL_PCI(pci);
Log('pLcl', (U32)pMsg);

			// Carefully consistency check the pointer
			// pMsg valid pointer?
			// pMsg points to buffer?
			// pMsg points to buffer in ready state?
			// msgReply.InitiatorContext valid?
			
			if (!pci || !pMsg) // Somebody put crap into our inbound queue!
				continue;

			if (pMsg->IsReply()) {
				// ReplyRemote to us
				pMsg->LocalizeSgl();

				// pMsg is a copy of our message, in a transport buffer
				InitiatorContext *pTc =pMsg->pInitiatorContext;
//				pMsg->contextTransaction = pTc->contextTransaction;
Log('Repl', (U32)pTc);

				// Copy any allocated reply buffer content back
				// to the user's non-pci buffer.
				BufferList &blPci=*(BufferList*)pMsg->POriginalMessage()->GetContext();
				TyDma *pTd=blPci.pTd;
				if (pTd) {
					// Copy sgl address, count to pTd chain
					// Should deal with fragments
					while (pTd) {
						int iSgl=(int)pTd->pArg;
						SGE_SIMPLE_ELEMENT sse;
						pMsg->GetSgl(iSgl, sse);
						if (sse.flags & SGL_FLAGS_FREE_ON_DTOR)
							pTd->pSrc=P_SGL(sse);
						if (!pTd->pNext)
							pTd->pArg=pMsg;
						pTd=pTd->pNext;
						}
					pTd=blPci.pTd;
					blPci.pTd=NULL;
					Dma::Transfer(pTd);
					}
				else // No dma necessary, call dma completion routine
					InboundFixup(pMsg, OK);
				}

			else { // SendRemote to us
Log('Sent', (U32)pMsg->contextTransaction.HighPart);
				// Change message pci addresses into local pointers (write SGL only)
				pMsg->LocalizeSgl();
				
				// contextTransaction is (didTarget : pBufferList)
				// InitiatorContext identifies the message to the initiator
				DdmServices::Send((DID) pMsg->contextTransaction.HighPart, pMsg, pMsg->didInitiator, pQueue_iSlot[DeviceId::ISlot(pMsg->didInitiator)]);
				}
			}
		}

	void Transport_Pci::InboundFixup(void *pMsgArg, int status) {
		Message *pMsg=(Message*)pMsgArg;
		InitiatorContext *pIc =pMsg->pInitiatorContext;
		BufferList &blPci=*(BufferList*)pMsg->POriginalMessage()->GetContext();
		TySlot tySlot_=blPci.pciSlot.tySlot;

		if (status != OK) {
			Transport::NotifyFailure(tySlot_, status);
			// Return transport failure status to user?
			pMsg->DetailedStatusCode = status;
			}
				
		if (pMsg->IsLast()) {
Log('Last', (U32)pIc->pMsg);

			// Free remote buffers
//			BufferList &blPci=*(BufferList*)pMsg->POriginalMessage()->GetContext();
Log('dlBl', (U32)&blPci);
			delete &blPci;

			// Copy message local copy reply fields back to original
			pMsg->CopyReply(pIc->pMsg);

Log('Delt', (U32)pMsg);
			// Free local copy buffer
			delete (void*)pMsg;
			pMsg=pIc->pMsg;
			}

Log('IsVl', pMsg->IsValid());
		if (pMsg->IsValid())
			Messenger::Reply(pMsg, (pMsg->flags & MESSAGE_FLAGS_LAST) != 0);
		else {
			Transport::NotifyFailure(tySlot_, CTS_CHAOS_TPT_MESSAGE_CORRUPT);
			Transport::ReplyFailed(pMsg, CTS_CHAOS_TPT_MESSAGE_CORRUPT);
			}
Log('done', 0);
		}

	U32 Transport_Pci::GetRemoteBuffer(U32 cb_, U32 &pciRet_) {
		return pciSlot.GetRemoteBuffer(cb_, pciRet_);
		}

	U32 Transport_Pci::GetRemoteMemory(U32 cb_, char* &pRet_) {
		U32 pciRet_;
		int cbRet_=pciSlot.GetRemoteBuffer(cb_, pciRet_);
		pRet_=(pciRet_? (char*)P_PCI(pciRet_) :0);
		return cbRet_;
		}

	// BufferList

	BufferList::BufferList(PciSlot &pciSlot_, U32 paPost_): pciSlot(pciSlot_), paPost(paPost_), pTd(NULL) { nBuf=0; }

	BufferList::~BufferList() { Free(); }
	
	void BufferList::Add(U32 pa) { aPaBuf[nBuf++]=pa; }

	void BufferList::Free() {	
			while (nBuf--)
				if (ISLOCAL_PCI(aPaBuf[nBuf]))
					delete PLOCAL_PCI(aPaBuf[nBuf]);
				else
					pciSlot.FreeRemoteBuffer(aPaBuf[nBuf]);

			while (pTd) {
				TyDma *pTdNext=pTd->pNext;
				delete pTd;
				pTd=pTdNext;
				}
				
			pTd=NULL;
			}
