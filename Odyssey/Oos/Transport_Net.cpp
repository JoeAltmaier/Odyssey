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
// This class implements message transport through a network.
// 
// Update Log: 
// 08/10/99 Joe Altmaier: Created
/*************************************************************************/

#define _TRACEF
#include "Odyssey_Trace.h"
#define	TRACE_INDEX		TRACE_TRANSPORT

#include "Transport_Net.h"
#include "BuildSys.h"
#include "CtEvent.h"
#include "HbcSubnet.h"

	DEVICENAME(Transport_Net, Transport_Net::DeviceInitialize);

// Public API

	Transport_Net::Transport_Net(TySlot tySlot_, U32 ipMe_, U32 ipHim_, U32 port_):TransportBase(tySlot_), transmitQueue(1024) {
		socket.Configure(ipMe_, ipHim_, port_);
		(new Task("SockRecv", 8192, SSocketReceiver, this))->Resume();
		(new Task("SockXmit", 8192, SSocketTransmitter, this))->Resume();
		}

	TransportBase* Transport_Net::Ctor(TySlot tySlot) {
		return new Transport_Net(tySlot, ip_iSlot[Address::iSlotMe], ip_iSlot[tySlot], PORT_TRANSPORT);
		}


	void Transport_Net::DeviceInitialize() {
		Transport::Register(Transport::TRANSPORT_NET, &Transport_Net::Ctor);
		}

	void Transport_Net::SetParentDdm(DdmOsServices *_pParentDdmSvc)	{ 
		DdmServices::SetParentDdm(_pParentDdmSvc);

		// Signal may have occurred before DdmTransport up.  Repeat it now.
		if (socket.IsOpen())
			Signal(Transport::SIG_IOP_UP, &tySlot);
		}

	void Transport_Net::SetState(Address::StateIop state_) {state_;}

	STATUS Transport_Net::GetStatus() { return (socket.IsOpen()? OK :CTS_CHAOS_TPT_IOP_INACCESSIBLE); }

	U32 Transport_Net::GetRemoteMemory(U32 cb_, char* &pRet_) {
		pRet_=new ((cb_ >= 8192)? tBIG : tSMALL) char[cb_];
		if (pRet_)
			return cb_;
		return 0;
		}


	STATUS Transport_Net::SendRemote(Message *pMsg, DID didTarget) {
Log('Send', (U32)pMsg);
Log(' did', (U32)didTarget);
Log('slot', (U32)tySlot);
TRACEF(TRACE_L8, ("SendRemote to %lx on slot %x.\n", didTarget, tySlot));

		// Pack didTarget into contextTransaction signature field so remote IOP can do local routing
		// If routed by board but no idLocal, reroute on destination.  Signal this by using DIDNULL.
		if (DeviceId::IdLocal(didTarget) == IDNULL)
			didTarget=DIDNULL;

Log('bind', (U32)didTarget);
		pMsg->contextTransaction.HighPart = didTarget;

		transmitQueue.Put(new LinkMessage(pMsg));
		return OK;
		}


	STATUS Transport_Net::ReplyRemote(Message *pMsg) {
		// Return pMsg to tySlot.
Log('Rply', (U32)pMsg);
TRACEF(TRACE_L8, ("ReplyRemote to slot %x.\n", tySlot));

		// Prepare msg for copy back to initiator
		InitiatorContext *pIc=pMsg->PopIc();
		if (pIc && pMsg->IsLast())
			delete pIc;

		transmitQueue.Put(new LinkMessage(pMsg));
		
		return OK;
		}

	BOOL Transport_Net::MessageTransmitter() {
		LinkMessage *pLm=transmitQueue.GetWait();

		Message *pMsg=pLm->pMsg;
		delete pLm;

		// Transmit message header and payload
		Status status = socket.Write(pMsg, pMsg->sMessage << 2);

		int sglFlags=(pMsg->IsReply()? SGL_FLAGS_REPLY :SGL_FLAGS_SEND);

		// Transmit sgl
		U32 nSgl = pMsg->GetCSgl();
		for (U32 iSgl=0; iSgl < nSgl && status == OK; iSgl++) {
			SGE_SIMPLE_ELEMENT *pSse = pMsg->GetPSgl(iSgl);
			
			// If write buffer, transmit all fragments
			if ((pSse->flags & SGL_FLAGS_DIR) == sglFlags) {
				BOOL fEob=false;
				while (!fEob && status == OK) {
					if (pSse->address != 0 && pSse->count != 0)
						status = socket.Write(P_SGL((*pSse)), pSse->count);

					fEob=pSse->IsEob();
					pSse++;
					}
				}
			}

		if (pMsg->IsReply()) {
			// Done with pMsg
			// Erase pIc because it belongs to another IOP (not pointing to local memory)
			pMsg->pInitiatorContext=NULL;
			delete (void*)pMsg;

			// Any status errors here, handle by dropping reply on floor!
			}

		else {
			// SEND
			if (status != OK)  {
Tracef("Send to %d failed, status=%x\n", tySlot, status);
				Transport::NotifyFailure(tySlot, status);
				Transport::SendFailed(pMsg, status);
				}
			}

		return true;
		}

	
		
	BOOL Transport_Net::MessageReceiver() {
Log('MsgR', (U32)0);

		// Read message header into blank message
		Message *pMsg=new Message((unsigned long)0);
		U32 cbRead;
		Status status_ = socket.Read(pMsg, sizeof(Message), &cbRead);

		if (status_ != OK || cbRead != sizeof(Message)) {
			// read timed out, or socket closed
Tracef("Socket::Read returned %08lx\n", status_);
			delete (void *)pMsg;
			Transport::NotifyFailure(tySlot, status_);
			return false;
			}

		// Read message payload
		U32 sMessage=(pMsg->sMessage << 2) - sizeof(Message);
		if (sMessage)
			status_ = socket.Read(((char*)pMsg)+sizeof(Message), sMessage, &cbRead);
		else {
			status_ = OK;
			cbRead=sMessage;
			}

		if (status_ != OK || cbRead != sMessage) {
			// read timed out, or nothing to read, or socket closed
Tracef("Socket::Read returned %08lx\n", status_);
			delete (void *)pMsg;
			Transport::NotifyFailure(tySlot, status_);
			return false;
			}

		// Read message sgl
		if (pMsg->IsReply()) {
			// ReplyRemote to us

			InitiatorContext *pIc =pMsg->pInitiatorContext;
//			pMsg->contextTransaction = pIc->contextTransaction;
Log('Repl', (U32)pIc);

			BOOL bIsLast=pMsg->IsLast();
			if (!bIsLast)
				pMsg->KeepReply(pIc->pMsg); // Restore sgl from original message

			// pMsg is a copy of a reply of our original message.
			// The sgl addresses are copied from the original message.
			// Reply data should be read into either the original buffers, or a system buffer.
			U32 nSgl = pMsg->GetCSgl();
			for (U32 iSgl=0; iSgl < nSgl; iSgl++) {
				SGE_SIMPLE_ELEMENT *pSse = pMsg->GetPSgl(iSgl);
			
				// Read any reply buffer content into a local buffer.
				if ((pSse->flags & SGL_FLAGS_DIR) == SGL_FLAGS_REPLY) {
					if (pSse->address != 0 && pSse->count != 0)
						do {
							if (pSse->flags & SGL_FLAGS_FREE_ON_DTOR) {
								pSse->address = (U32)new char[pSse->count];
								pSse->flags |= SGL_FLAGS_LOCAL_ADDRESS;
								}
							status_ = socket.Read(P_SGL((*pSse)), pSse->count, &cbRead);
						} while (!((pSse++)->flags & SGL_FLAGS_END_OF_BUFFER) && status_ == OK);
					}

				if (status_ != OK) {
					// read timed out, or socket closed
					Transport::ReplyFailed(pMsg, status_);
					Transport::NotifyFailure(tySlot, status_);
					return false;
					}
				}
				
			// pMsg is a copy of our message
			if (bIsLast) {
				// Copy message local copy reply fields back to original
				pMsg->CopyReply(pIc->pMsg);

				// Free local copy buffer
				delete (void*)pMsg;
				pMsg=pIc->pMsg;
				}

Log('IsVl', pMsg->IsValid());
			if (pMsg->IsValid())
				status_=Messenger::Reply(pMsg, (pMsg->flags & MESSAGE_FLAGS_LAST) != 0);
			else
				status_=CTS_CHAOS_TPT_MESSAGE_CORRUPT;

			if (status_ != OK) {
				Transport::NotifyFailure(tySlot, status_);
				Transport::ReplyFailed(pMsg, status_);
				}
			}

		else { // SendRemote to us
Log('Sent', (U32)pMsg->contextTransaction.HighPart);
			// Change message pci addresses into local pointers (write SGL only)
			// Send data should be read into a system buffer.
			U32 nSgl = pMsg->GetCSgl();
			for (U32 iSgl=0; iSgl < nSgl; iSgl++) {
				SGE_SIMPLE_ELEMENT *pSse = pMsg->GetPSgl(iSgl);
			
				// Read any reply buffer content into a local buffer.
				if ((pSse->flags & SGL_FLAGS_DIR) == SGL_FLAGS_SEND) {
					status_ = OK;
					if (pSse->address != 0 && pSse->count != 0)
						do {
							pSse->address = (U32)new char[pSse->count];
							pSse->flags |= SGL_FLAGS_LOCAL_ADDRESS;
							status_ = socket.Read(P_SGL((*pSse)), pSse->count, &cbRead);
						} while (!((pSse++)->flags & SGL_FLAGS_END_OF_BUFFER) && status_ == OK);
					}

				if (status_ != OK) {
					// read timed out, or nothing to read, or socket closed
					Transport::SendFailed(pMsg, status_);
					Transport::NotifyFailure(tySlot, status_);
					return false;
					}
				}

			// contextTransaction is (didTarget : 0)
			status_=DdmServices::Send((DID) pMsg->contextTransaction.HighPart, pMsg, pMsg->didInitiator, pQueue_iSlot[DeviceId::ISlot(pMsg->didInitiator)]);
			if (status_ != OK) {
				Transport::NotifyFailure(tySlot, status_);
				Transport::SendFailed(pMsg, status_);
				}
			}

		return (status_ == OK);
		}



	// These methods are run on a thread forever.
	void Transport_Net::SSocketReceiver(void *pTransport) {
		((Transport_Net*)pTransport)->SocketReceiver();
		}

	void Transport_Net::SSocketTransmitter(void *pTransport) {
		((Transport_Net*)pTransport)->SocketTransmitter();
		}

	void Transport_Net::SocketReceiver() {
		while (true) {
			while (!socket.IsOpen())
				if (socket.Connect(Address::iSlotMe == Address::iSlotHbcMaster) == OK) {
Tracef("Transport_Net::SocketReceiver CONNECT\n");
					Signal(Transport::SIG_IOP_UP, &tySlot);
					break;
					}
				else
					Kernel::Delay(1000);

			// Process anything that might be coming in, until connection fails.
			while (MessageReceiver()) ;
			
			socket.Close();
Tracef("Transport_Net::SocketReceiver DISCONNECT\n");

			Signal(Transport::SIG_IOP_FAIL, (void*)tySlot);
			}
		}

	void Transport_Net::SocketTransmitter() {
		// Send queued Send and Reply messages
		while (MessageTransmitter()) ;
		}
