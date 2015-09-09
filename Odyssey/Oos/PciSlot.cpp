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
// This class implements pci buffer queues.
// 
// Update Log: 
// 08/14/99 Joe Altmaier: Created from HeapPci.cpp
/*************************************************************************/

#define _TRACEF
#include "Odyssey_Trace.h"
#define TRACE_INDEX TRACE_TRANSPORT

#include "PciSlot.h"
#include "Interrupt.h"
#include "Dma.h"

	extern "C" void bcopy(void *pFrom, void *pTo, int cb);

	BOOL PciSlot::fSuspend=false;
	Pulse PciSlot::pulsePci;

	PciSlot::PciSlot(TySlot tySlot_, U32 pciBase_, Address::StateIop state_):tySlot(tySlot_), pciBase(pciBase_), state(state_), syncLast(0l) {}

	void PciSlot::SetState(Address::StateIop state_) {
		state = state_;

		Address::StateSigCount ssc_;
		U32 *pSsc=(U32*)P_PA(pciBase + GT_I2O_OUTMSG_0_OFF);
		if (!PciRead(ssc_.l, pSsc))
			return;

		ssc_.ssc.state=state_;
		ssc_.ssc.sig=Address::StateSigCount::SIGNATURE;

		// PCI cycle
		PciWrite(pSsc, ssc_.l);
		}

	// Is IOP in slot still alive?
	Status PciSlot::CheckState() {
		Status status_=GetStatus();
		if (status_ != OK)
			return status_;
			
		// This state is updated each time the IOP processes a message (process state)
		Address::StateSigCount sscProc_;
		if (!PciRead(sscProc_.l, (U32*)P_PA(pciBase + GT_I2O_OUTMSG_0_OFF)))
			return CTS_CHAOS_TPT_IOP_INACCESSIBLE;
			
		// This state is incremented each time a message is posted to the IOP (post state)
		Address::StateSigCount sscPost_;
		if (!PciRead(sscPost_.l, (U32*)P_PA(pciBase + GT_I2O_INMSG_0_OFF)))
			return CTS_CHAOS_TPT_IOP_INACCESSIBLE;

		// This is the last post state we recorded
		Address::StateSigCount sscLast_;
		sscLast_.l=syncLast;

		if (sscProc_.ssc.sig == Address::StateSigCount::SIGNATURE
			&& sscProc_.ssc.state == Address::IOP_ACTIVE
			&& sscPost_.ssc.sig == Address::StateSigCount::SIGNATURE // something was posted at least once
			&& sscLast_.ssc.cMsg != sscPost_.ssc.cMsg // something was posted since last check
			&& sscLast_.ssc.cMsg == sscProc_.ssc.cMsg) // nothing was processed since last check
			{
			Fail();
			return CTS_CHAOS_TPT_IOP_FAILED;
			}

		// Remember last process state
		syncLast=sscProc_.l;
		return OK;
		}

	void PciSlot::Fail() {
		SetState(Address::IOP_FAILED);
		}

	U32 PciSlot::GetRemoteBuffer(U32 cb_, U32 &pciRet_) {
		// Get buffer from iSlot of size at least cb.
		// I2O Get buffer from PCI galileo queue.
Log('GetR', cb_);

Log('Slot', tySlot);
Log('base', pciBase);
		U32 cbcb_;
		if (!PciRead(cbcb_, (U32*)P_PA(pciBase + GT_I2O_OUTMSG_1_OFF)))
			return 0;
			
Log('cbcb', cbcb_);
		int cb1_=(cbcb_ >> 16)+1;
		int cb2_=(cbcb_ & 0xFFFF)+1;
//Tracef("GetRemoteBuffer cbcb=%lx  cb1=%d cb2=%d  allocating cb=%d\n", cbcb, cb1, cb2, cb);

		if (cb_ <= cb1_) {
Log('cb1=', cb1_);
			if (!PciRead(pciRet_, (U32*)P_PA(pciBase + GT_I2O_INQ_VR_OFF)))
				return 0;
Log('pci=', (U32)pciRet_);
			return cb1_;
			}

		if (cb_ <= cb2_) {
Log('cb2=', cb2_);
			// PCI cycle
			if (!PciRead(pciRet_, (U32*)P_PA(pciBase + GT_I2O_OUTQ_VR_OFF)))
				return 0;
Log('pci=', (U32)pciRet_);
			return cb2_;
			}

		// Return buffer chain?
Log('Err!', (U32)0);
		return 0;
		}


	void PciSlot::FreeRemoteBuffer(U32 pciRemote_) {
		// PCI
		*(U32*)P_PA(pciBase + GT_I2O_OUTQ_VR_OFF) = pciRemote_;
		BusError(); // test and clear
		}


	BOOL PciSlot::Post(U32 pciRemote_) {
		// Post pRemote to iSlot
		U32 pci_=pciBase + GT_I2O_INQ_VR_OFF;
//Tracef("PciSlot::Post(%lx, %lx)\n", pci, pciRemote);
		if (!PciWrite((U32*)P_PA(pci_), pciRemote_))
			return false;

		// Increment message counter.  Test periodically to be sure iSlot is processing messages.
		// Read process state
		Address::StateSigCount sscProc_;
		if (!PciRead(sscProc_.l, (U32*)P_PA(pciBase + GT_I2O_OUTMSG_0_OFF)))
			return false;

		// Read post state
		Address::StateSigCount ssc_;
		if (!PciRead(ssc_.l, (U32*)P_PA(pciBase + GT_I2O_INMSG_0_OFF)))
			return false;
		
		// Write new post state
		ssc_.ssc.cMsg++;
		ssc_.ssc.sig=Address::StateSigCount::SIGNATURE;
		ssc_.ssc.state=sscProc_.ssc.state; // Reflect IOPs own state so he sees our opinion of him
		return PciWrite((U32*)P_PA(pciBase + GT_I2O_INMSG_0_OFF), ssc_.l);
		}


	// Copy from/to PCI bus, recover from PCI timeout
	BOOL PciSlot::bcopy(void* pFrom, void* pTo, U32 cb) {
		Critical section;
		if (fSuspend)
			pulsePci.Wait();

		::bcopy(pFrom, pTo, cb);
		return !BusError();
		}

	BOOL PciSlot::BusError() {
		BOOL fRet=(Interrupt::TestInterrupt(Interrupt::tySlvWrErr0) | Interrupt::TestInterrupt(Interrupt::tySlvRdErr0) | Interrupt::TestInterrupt(Interrupt::tyRetryCtr0));
		if (fRet) {
			Interrupt::ClearInterrupt(Interrupt::tySlvWrErr0);
			Interrupt::ClearInterrupt(Interrupt::tySlvRdErr0);
			Interrupt::ClearInterrupt(Interrupt::tyRetryCtr0);
			}
		return fRet;
		}

	BOOL PciSlot::PciRead(U32 &u32Ret_, void *pPci_) {
		Critical section;
		if (fSuspend)
			pulsePci.Wait();
			
		u32Ret_=*(U32*)pPci_;
		return !BusError();
		}
	
	BOOL PciSlot::PciWrite(void *pPci_, U32 data_) {
		Critical section;
		if (fSuspend)
			pulsePci.Wait();

		*(U32*)pPci_ = data_;
		return !BusError();
		}

	void PciSlot::Suspend() {
		Critical section;
		fSuspend=true;
		Dma::Suspend();
		}

	void PciSlot::Resume() {
		Critical section;
		Dma::Resume();
		fSuspend=false;
		pulsePci.Signal();
		}
