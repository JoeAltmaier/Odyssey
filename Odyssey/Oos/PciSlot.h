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
// This class manages transfer buffers in the pci window.
// Alloc returns the next free buffer, if any.
// Freeing a block returns it to the free list.
// 
// Update Log: 
// 11/10/98 Joe Altmaier: Create file
/*************************************************************************/

#ifndef __PciSlot_h
#define __PciSlot_h

#include "Address.h"
#include "DeviceId.h"
#include "Galileo.h"
#include "Semaphore.h"
#include "CtEvent.h"

class PciSlot {
	static
	BOOL fSuspend;
	static
	Pulse pulsePci;
	
	U32 pciBase;
	U32 syncLast;
	Address::StateIop state;
	
public:
	TySlot tySlot;

public:
	PciSlot(TySlot tySlot, U32 pciBase, Address::StateIop state);

	// Buffer mechanism
	U32 GetRemoteBuffer(U32 cb, U32 &pciRet);
	void FreeRemoteBuffer(U32 pciRemote);
	TySlot GetSlot(U32 pciRemote);

	BOOL Post(U32 pciRemote);

	void Fail();
	
	static
	BOOL bcopy(void* pFrom, void* pTo, U32 cb);

	// State mechanism
	Status GetStatus() {
		// If we have seen an inactive state, return that
		// until we get told by somebody that hes ok again.
		if (state != Address::IOP_ACTIVE
			&& state != Address::IOP_BOOTING
			&& state != Address::IOP_STANDBY)
			return CTS_CHAOS_TPT_IOP_NOT_READY;

		// Sample his own idea of his state as published in his mailbox register
		Address::StateSigCount ssc_;
		if (!PciRead(ssc_.l, (U32*)P_PA(pciBase + GT_I2O_OUTMSG_0_OFF)))
			return CTS_CHAOS_TPT_IOP_INACCESSIBLE;

		if (ssc_.ssc.state == Address::IOP_ACTIVE || ssc_.ssc.sig != Address::StateSigCount::SIGNATURE)
			return OK;

		return CTS_CHAOS_TPT_IOP_NOT_READY;
		}

	void SetState(Address::StateIop state);
	Status CheckState();

	// PCI error mechanism
	static
	BOOL PciRead(U32 &ret, void *pPci);

	static
	BOOL PciWrite(void *pPci, U32 data);

	static
	BOOL BusError();

	static
	void Suspend();

	static
	void Resume();
};
#endif


