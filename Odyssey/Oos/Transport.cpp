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
// 10/12/98 Joe Altmaier: Created from Message.cpp
// 11/11/98 Joe Altmaier: Separate HeapPci out.  Make write sgl work.
// 08/19/99 Joe Altmaier: Separate Transport_Pci out.
// 8/24/99 Joe Altmaier: fPcimap_iSlot -> fIop_iSlot
/*************************************************************************/

#define _TRACEF
#include "Odyssey_Trace.h"
#define	TRACE_INDEX		TRACE_TRANSPORT

#include "CtTypes.h"
extern void bcopy(void *, void *, int);

#include "DdmManager.h"
#include "Transport.h"
#include "BuildSys.h"
#include "RqOsTimer.h"
#include "RqOsTransport.h"
#include "RqOsVirtualManager.h"
#include "Os.h"
#include "HeapPci.h"
#include "CtEvent.h"

#define ONESEC 1000000


	CLASSNAME(Transport,SINGLE);
	DEVICENAME(Transport, Transport::DeviceInitialize);
	SERVELOCAL(Transport,RqOsTransportVdnStop::requestcode);
	SERVELOCAL(Transport,RqOsTransportIopStop::requestcode);
	SERVELOCAL(Transport,RqOsTransportIopStart::requestcode);
	SERVELOCAL(Transport,RqOsTransportHbcMaster::requestcode);
	
	DID Transport::didTransport;

	Transport::TransportConstructor Transport::aTransportCtor[Transport::TRANSPORT_MAX]={&Transport::CtorNull, &Transport::CtorNull};
	
	// Queue of messages FROM each slot
	IcQueue *TransportBase::pQueue_iSlot[NSLOT];

	// Transport for messages TO each slot
	TransportBase *TransportBase::aPTransport[NSLOT];

	TySlot iSlotOtherHbc[]={IOP_HBC1, IOP_HBC0};
	
// Public API

	Transport::Transport(DID did) :Ddm(did) {
		SelectMasterHbc(Address::iSlotHbcMaster);
		
		DispatchRequest(RqOsTransportVdnStop::requestcode,REQUESTCALLBACK(Transport,ProcessVdnStopRequest));
		DispatchRequest(RqOsTransportIopStop::requestcode,REQUESTCALLBACK(Transport,ProcessIopStopRequest));
		DispatchRequest(RqOsTransportIopStart::requestcode,REQUESTCALLBACK(Transport,ProcessIopStartRequest));
		DispatchRequest(RqOsTransportHbcMaster::requestcode,REQUESTCALLBACK(Transport,ProcessHbcMasterRequest));
		DispatchSignal(SIG_IOP_FAIL, SIGNALCALLBACK(Transport, ProcessIopFail));
		DispatchSignal(SIG_IOP_UP, SIGNALCALLBACK(Transport, ProcessIopUp));
		}
		
	Ddm *Transport::Ctor(DID did) {
		return new Transport(did);
		}

	STATUS Transport::Initialize(Message *pMsg) {
		didTransport = GetDid();

		// Listen to VdnManager VDT (Virtual Device Table) and IST (Iop State Table)
		Send(new RqOsDdmManagerListenVdnMap(), REPLYCALLBACK(Transport,ReplyListenVdn));
		Send(new RqOsVirtualManagerListenIopState(), REPLYCALLBACK(Transport,ReplyListenIop));

		// Start heartbeat timer
		Send(new RqOsTimerStart(ONESEC, ONESEC), REPLYCALLBACK(Transport, ReplyTimer));

		for (int iSlot=0; iSlot < NSLOT; iSlot++)
			if (TransportBase::aPTransport[iSlot])
				TransportBase::aPTransport[iSlot]->SetParentDdm(this);

		return Reply(pMsg);
		}

	void Transport::DeviceInitialize() {
TRACEF(TRACE_L8, ("ENTRY Transport::DeviceInitialize\n"));
		U32 *aPciBase;

#ifdef _ODYSSEY
		aPciBase=memmaps.aPaPci;

#else	// Eval boards or Win32
		struct MP {U32 cbMsg; U32 sMsg; U32 cbBuf; U32 sBuf; U32 pciWindowLo; U32 pciWindowHi; U32 aPciWindow[NSLOT]; U32 aSWindow[NSLOT];} *pTransportParams
		 =(MP*)Oos::GetBootData("Transport");

		aPciBase=pTransportParams->aPciWindow;
#endif

		int iSlot;
		for (iSlot=0; iSlot < NSLOT; iSlot++)
			if (DeviceId::fIop_iSlot[iSlot]) {
				Address::aSlot[iSlot].syncLast=0;
				Address::aSlot[iSlot].pciBase=aPciBase[iSlot];
				}
			else
				Address::aSlot[iSlot].pciBase= (U32) -1;

		Address::pciBase = Address::aSlot[Address::iSlotMe].pciBase;

		for (iSlot=0; iSlot < NSLOT; iSlot++)
			if (DeviceId::fIop_iSlot[iSlot] && iSlot != Address::iSlotMe) {
				// Every processor has an array of TransportBase objects it uses to directly communicate with each other processor.
				// The HBCs communicate throught ethernet, everybody else uses pci.
				if ((iSlot == IOP_HBC0 || iSlot == IOP_HBC1) && (Address::iSlotMe == IOP_HBC0 || Address::iSlotMe == IOP_HBC1))
					TransportBase::aPTransport[iSlot] = aTransportCtor[TRANSPORT_NET]((TySlot)iSlot);
				else
					TransportBase::aPTransport[iSlot]=aTransportCtor[TRANSPORT_PCI]((TySlot)iSlot);

				// Create a termination queue for each slot from which a message could arrive
				TransportBase::pQueue_iSlot[iSlot] = new IcQueue(128);
				}


		// Wait a while for connection to master HBC.
		U32 nMsDelay=10000;
		TransportBase *pTransportHbc=TransportBase::aPTransport[Address::iSlotHbcMaster];
		while (pTransportHbc && pTransportHbc->GetStatus() != OK)
			if ((nMsDelay -= 500) <= 0)
				break;
			else
{ Tracef("Waiting for master hbc...\n");
				Kernel::Delay(500);
}

TRACEF(TRACE_L8, ("EXIT  Transport::DeviceInitialize\n"));
		}


// Transport Ddm methods

	STATUS Transport::ProcessVdnStopRequest(Message *pMsgArg) {
		RqOsTransportVdnStop *pMsg=(RqOsTransportVdnStop *)pMsgArg;

		FailSafe *pFs=FailSafe::GetFailSafe(pMsg->vdnStop);
		if (pFs)
			pFs->state=Address::IOP_FAILED;

		Reply(pMsg, OK);
		return OK;
		}
		
	STATUS Transport::ProcessIopStopRequest(Message *pMsgArg) {
		RqOsTransportIopStop *pMsg=(RqOsTransportIopStop *)pMsgArg;

		TransportBase::aPTransport[pMsg->tySlot]->SetState(Address::IOP_FAILED);

		Reply(pMsg, OK);
		return OK;
		}

	STATUS Transport::ProcessIopStartRequest(Message *pMsgArg) {
		RqOsTransportIopStart *pMsg=(RqOsTransportIopStart *)pMsgArg;

		TransportBase::aPTransport[pMsg->tySlot]->SetState(Address::IOP_ACTIVE);

		Reply(pMsg, OK);
		return OK;
		}

	STATUS Transport::ProcessHbcMasterRequest(Message *pMsgArg) {
		RqOsTransportHbcMaster *pMsg=(RqOsTransportHbcMaster *)pMsgArg;

		SelectMasterHbc(pMsg->tySlot);

		Reply(pMsg, OK);
		return OK;
		}

	void Transport::SelectMasterHbc(TySlot tySlotMaster) {
		TySlot tySlotSlave=iSlotOtherHbc[tySlotMaster];

		// Everybody route master directly
		DeviceId::iSlot_iSlot[tySlotMaster] = tySlotMaster;

		// If I am master HBC, route every slot directly
		if (Address::iSlotMe == tySlotMaster)
			for (int iSlotMap=0; iSlotMap < NSLOT; iSlotMap++)
				DeviceId::iSlot_iSlot[iSlotMap] = (TySlot)iSlotMap;

		// If I am slave HBC, route every slot through master
		else if (Address::iSlotMe == tySlotSlave)
			for (int iSlotMap=0; iSlotMap < NSLOT; iSlotMap++)
				DeviceId::iSlot_iSlot[iSlotMap] = tySlotMaster;

		// IOPs route slave through master
		else
			DeviceId::iSlot_iSlot[tySlotSlave] = tySlotMaster;
			// and leave all other slots routed directly
		}

	// Every so often, check IOP consistency.
	STATUS Transport::ReplyTimer(Message *pMsg) {
		// Check that our IOP state is still ok.
		HeapPci::CheckState();

		delete pMsg;
		return OK;
		}
		
	// When virtual devices change state, process queued messages
	STATUS Transport::ReplyListenVdn(Message *_pMsgArg) {
		RqOsDdmManagerListenVdnMap *_pMsg=(RqOsDdmManagerListenVdnMap*)_pMsgArg;
		
		// If a VDN was deleted, return queued messages with an error
		if (_pMsg->payload.did == DIDNULL)
			FailSafe::FailVdn(_pMsg->payload.vdn, CTS_CHAOS_TPT_SERVICE_TERMINATED);

		// If a VDN changed (didPrimary changed), retry queued messages
		else
			FailSafe::RetryVdn(_pMsg->payload.vdn);

		delete _pMsg;
		return OK;
		}

//	Address::StateIop StateIop_ProxyIopState[]={ Address::IOP_RESET, Address::IOP_RESET, Address::IOP_STANDBY, Address::IOP_BOOTING, Address::IOP_FAILED, Address::IOP_ACTIVE};
Address::StateIop StateIop_ProxyIopState[]={
Address::IOP_RESET,		// IOPS_UNKNOWN = 0,       // default when we know nothing
Address::IOP_RESET,		// IOPS_EMPTY,             // no card in slot
Address::IOP_RESET,		// IOPS_BLANK,             // blank card in slot
Address::IOP_STANDBY,   // IOPS_POWERED_ON,        // fresh state when IOP is just powered on
Address::IOP_STANDBY,   // IOPS_AWAITING_BOOT,     // IOP awaiting PCI window / boot instructions
Address::IOP_RESET,		// IOPS_DIAG_MODE,         // running diagnostic "image"
Address::IOP_BOOTING,   // IOPS_BOOTING,           // IOP boot ROM handing off control to boot image
Address::IOP_ACTIVE,	// IOPS_LOADING,           // IOP image running, loading "system entries"
Address::IOP_ACTIVE,	// IOPS_OPERATING,         // normally operating (OS / app-level code)
Address::IOP_ACTIVE,	// IOPS_SUSPENDED,         // IOP's MIPS is suspended (no PCI accesses)
Address::IOP_FAILED,	// IOPS_FAILING,           // IOP failure detected, IOP is being failed away
Address::IOP_FAILED,	// IOPS_FAILED,            // IOP failed away from, may now shut down or reboot
Address::IOP_ACTIVE,	// IOPS_QUIESCING,         // IOP is gbeing Quiesced.
Address::IOP_FAILED,	// IOPS_QUIESCENT,         // in quiesced state
Address::IOP_RESET,		// IOPS_POWERED_DOWN,      // MIPS cluster is powered down
Address::IOP_RESET,		// IOPS_UNLOCKED,          // IOP's card locking solenoid has been released,
                     						       //  ready for card extraction (only possible when
                       							   //  IOP's MIPS is also powered down)
Address::IOP_RESET,		// IOPS_IMAGE_CORRUPT,     // IOP was told to boot an image, & found it corrupt
Address::IOP_RESET,		// IOPS_INSUFFICIENT_RAM   // IOP has insufficient RAM to perform requested boot
Address::IOP_RESET		// IOPS_UPDATING_BOOT_ROM  // IOP is presently reprogramming its BOOT ROM
};


// Hack: 1st reply is bogus
BOOL fFirst=true;

	// When iops change state, update local state
	STATUS Transport::ReplyListenIop(Message *_pMsgArg) {
		RqOsVirtualManagerListenIopState::States &payload=*((RqOsVirtualManagerListenIopState*)_pMsgArg)->GetStatesPtr();
		InitiatorContext *pIc;

		if (!fFirst)				
		// For each iop state rcvd, update Address::aSlot[iSlot].state
		for (TySlot tySlot_=IOP_HBC0; tySlot_ < RqOsVirtualManagerListenIopState::States::MAXSLOTS; tySlot_=(TySlot)((int)tySlot_+1)) {
			Address::aSlot[tySlot_].state=StateIop_ProxyIopState[payload.stateIop[tySlot_]];

			if (DeviceId::fIop_iSlot[tySlot_] && tySlot_ != Address::iSlotMe)
				if (Address::aSlot[tySlot_].state == Address::IOP_RESET || Address::aSlot[tySlot_].state == Address::IOP_FAILED) {
					// For all non-alive IOPs, mark outstanding messages MESSAGE_FLAGS_DEAD
					while ((pIc=TransportBase::pQueue_iSlot[tySlot_]->Get()) != NULL)
						pIc->pMsg->flags |= MESSAGE_FLAGS_DEAD;
					// For each did-routed message that we sent to this slot, return message with error
					FailSafe::FailIop(tySlot_, CTS_CHAOS_TPT_IOP_FAILED);
					}
				// IOP alive
				else {
#ifndef WIN32
					TransportBase::aPTransport[tySlot_]->SetState(Address::IOP_ACTIVE);
#endif
					}
			}

		fFirst=false;
		
		delete _pMsgArg;
		return OK;
		}

	// When we fail to communicate with an IOP, we Signal here.
	STATUS Transport::ProcessIopFail(SIGNALCODE _sig, void *_context) {
		_sig;
		TySlot _tySlot=(TySlot)_context;

		Fail(_tySlot, CTS_CHAOS_TPT_IOP_FAILED);

		return OK;
		}

	// When communications is established, we Signal here.
	STATUS Transport::ProcessIopUp(SIGNALCODE _sig, void *_context) {
		_sig;
		TySlot tySlot_=*(TySlot*)_context;

#ifndef WIN32
		TransportBase::aPTransport[tySlot_]->SetState(Address::IOP_ACTIVE);
#endif

		// For each message that is routed to this slot, resend
		FailSafe::RetryIop(tySlot_);

		return OK;
		}

	void Transport::Fail() {
		Os::Fail();
		// doesn't return
		}
		
	void Transport::Fail(TySlot tySlot_, Status status_) {
		// See if tySlot is bad, or we are
		// Test our pci interface, their pci interface, the HBC's pci interface
		STATUS statusSlot_=TransportBase::aPTransport[tySlot_]->GetStatus();
//		STATUS statusHbc_=TransportBase::aPTransport[Address::iSlotHbcMaster]->GetStatus();
		// Could be CTS_TPT_IOP_INACCESSIBLE CTS_TPT_IOP_FAILED or CTS_SUCCESS (OK)
		// ...
		// Ok, iop failed.
#ifndef WIN32
		TransportBase::aPTransport[tySlot_]->SetState(Address::IOP_FAILED);
#endif
		// Inform VdnManager that iop has failed:  RqVdnManagerFailIop(tySlot_, status_)
		// ...
		}

	void Transport::SendFailed(Message *pMsg, STATUS status_) { // static
		// If not a VDN-routed msg, or 2nd time, reply with error
		if (!(pMsg->flags & MESSAGE_FLAGS_AUTO_RETRY) || (pMsg->flags & MESSAGE_FLAGS_FAIL)) {
			pMsg->DetailedStatusCode = status_;
			Messenger::Reply(pMsg, true);
			}
				
		else {// Already on queue, retry after VDN route changes
			pMsg->flags |= MESSAGE_FLAGS_FAIL; // If this happens again, return the message with erc.
			FailSafe *_pFs=FailSafe::GetFailSafe(pMsg->pInitiatorContext->vdn);
			if (_pFs)
				_pFs->state=Address::IOP_FAILED;

			else {// Ok, for some reason there is no queue (even though AUTO_RETRY is set)?
				pMsg->DetailedStatusCode = status_;
				Messenger::Reply(pMsg, true);
				}
			}
		}

	void Transport::ReplyFailed(Message *pMsg, STATUS status_) { // static
		status_;
		// Drop message on floor.
		// Erase pInitiatorContext because it belongs to another IOP (not pointing to local memory)
		pMsg->pInitiatorContext=NULL;
		// Free message buffer (or message copy)
		delete (void*)pMsg;
		}

	void Transport::NotifyFailure(TySlot tySlot_, Status status_) { // static
		// Stop bus traffic, in case we are at fault.
		Os::Suspend();
		// Inform VdnManager that we have a problem:  RqVdnManagerIopInaccessible(tySlot_, []status_)
		// ...
		}

	// Used by static TransportRegistrar object
	void Transport::Register(Transport::TyTransport ty, Transport::TransportConstructor tc) {
		aTransportCtor[ty]=tc;
		}

// TransportBase methods

	TransportBase::TransportBase(TySlot _tySlot) :DdmServices(), tySlot(_tySlot) {}

	void TransportBase::SetParentDdm(DdmOsServices *_pParentDdmSvc) {
		DdmServices::SetParentDdm(_pParentDdmSvc);
		}



	// Allocate memory on a certain board.
	void *operator new[](unsigned int cb_, DID did_, int *pCbActual_) {
		TySlot tySlotAlloc_=DeviceId::ISlot(did_);
		if (tySlotAlloc_ == Address::iSlotMe || did_ == DIDNULL) { 
			*pCbActual_=cb_;

			void *pRet=new (cb_ >= 8192? tBIG : tSMALL) char[cb_];
			if (pRet)
				return pRet;
				
			*pCbActual_=0;
			return NULL;
		}

		char *pRet_;
		*pCbActual_=TransportBase::aPTransport[tySlotAlloc_]->GetRemoteMemory(cb_, pRet_);
		return pRet_;
	}


