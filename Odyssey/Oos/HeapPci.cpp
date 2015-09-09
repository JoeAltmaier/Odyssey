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
// 11/10/98 Joe Altmaier: Created from Transport.cpp
// 04/27/99 Jerry Lane: Publish buffer sizes minus 1 and add one on access.
// 07/06/99 Joe Altmaier: test/clear bus errors on pci cycles.
// 8/24/99 Joe Altmaier: fPcimap_iSlot -> fIop_iSlot
/*************************************************************************/

#define _TRACEF
#include "Odyssey_Trace.h"
#define TRACE_INDEX TRACE_TRANSPORT

#include "OsTypes.h"
#include "Galileo.h"
#include "HeapPci.h"
#include "PciSlot.h"
#include "Interrupt.h"
#include "Dma.h"
#include "BuildSys.h"
#include "Os.h"

	FAILNAME(HeapPci, HeapPci::FailPci);
	
	
	extern "C" void bcopy(void *pFrom, void *pTo, int cb);

	HeapPci::HeapPci(): HeapLink() {
		// Set state to not ready
		SetState(Address::IOP_BOOTING);
		}

	void HeapPci::SetState(Address::StateIop state) {
		Address::StateSigCount ssc_;
		ssc_.l=*(U32*)(GT_I2O_OUTMSG_0_REG);

		ssc_.ssc.state=state;
		ssc_.ssc.sig=Address::StateSigCount::SIGNATURE;
		if (state == Address::IOP_BOOTING)
			ssc_.ssc.cMsg=0;

		Address::aSlot[Address::iSlotMe].syncLast=ssc_.l;
		*(U32*)(GT_I2O_OUTMSG_0_REG) = ssc_.l;
		}

	// Is our state good?
	BOOL HeapPci::CheckState() {
		Address::StateSigCount ssc_;
		ssc_.l=*(U32*)(GT_I2O_OUTMSG_0_REG);
		
		Address::StateSigCount sscPost_;
		sscPost_.l=*(U32*)(GT_I2O_INMSG_0_REG);

		// Has someone failed us?
		if (sscPost_.ssc.sig == Address::StateSigCount::SIGNATURE
		 && sscPost_.ssc.state == Address::IOP_FAILED)
			Os::Fail();

		// Update our cMsg to show we are processing messages
		ssc_.ssc.cMsg=sscPost_.ssc.cMsg;
		Address::aSlot[Address::iSlotMe].syncLast=ssc_.l;
		*(U32*)(GT_I2O_OUTMSG_0_REG) = ssc_.l;
		
		return true;
		}


	void HeapPci::FailPci() {
		Critical section;
#ifndef WIN32
		HeapPci::SetState(Address::IOP_FAILED);
#endif
		}

// Free -- Return block to it's free bucket -----------------------HeapPci-
//
	U32 HeapPci::Free(void *pMem) {
		Header *pHdr = (((Header *) pMem)-1);
		FreeLocalBuffer((char*)pMem, pHdr->iQueue);
		return 0;
		}


	void HeapPci::I2oLayout(U32 i2oBase, int cbQueue) {
	// i2oBase is memory to hold queues
	// cbQueue is 16K, 32K, 64K, 128K or 256K
	// Must have enough room to hold four queues, not crossing megabyte boundary
		U32 qBase=i2oBase & 0x000FFFFCl;
		U32 qBar=i2oBase & 0x1FF00000l;

		// Set QBAR (queue base address register)
		*(U32*)(GT_I2O_QBAR_REG) = GTREG(qBar);

TRACEF(TRACE_L8, ("HeapPci::I2oLayout qbar=%8lx qBase=%8lx\n", qBar, qBase));
		// Set each queue address
		// Inbound Free	
		*(U32*)(GT_I2O_INFREE_HEAD_REG) = GTREG(qBase);
		*(U32*)(GT_I2O_INFREE_TAIL_REG) = GTREG(qBase);
		qBase += cbQueue;

		// Inbound Post
		*(U32*)(GT_I2O_INPOST_HEAD_REG) = GTREG(qBase);
		*(U32*)(GT_I2O_INPOST_TAIL_REG) = GTREG(qBase);
		qBase += cbQueue;

		// Outbound Free
		*(U32*)(GT_I2O_OUTFREE_HEAD_REG) = GTREG(qBase);
		*(U32*)(GT_I2O_OUTFREE_TAIL_REG) = GTREG(qBase);
		qBase += cbQueue;

		// Outbound Post
		*(U32*)(GT_I2O_OUTPOST_HEAD_REG) = GTREG(qBase);
		*(U32*)(GT_I2O_OUTPOST_TAIL_REG) = GTREG(qBase);
		qBase += cbQueue;

		// Enable queues from PCI
		*(U32*)(GT_I2O_QCTRL_REG) = GTREG(1l | (cbQueue >> 13));

		modQueue=cbQueue-1;
		}


// Buffer management


	// Put buffers onto free list
	void HeapPci::I2oFreeInit(char *pFree, U32 cb, U32 cbBuffer) {
		// Each buffer has a header before it.
		cbBuffer += sizeof(Header);
		
		while (cb > cbBuffer) {
			// Build the header.
			Header *pHdr=(Header*)pFree;
			pHdr->iQueue=0;
			pHdr->pHeap=this;
			
			// Put the buffer on the free list.
			// The header is not visible to the world through the I2O queue.
			I2oFree(pFree+sizeof(Header));
			pFree += cbBuffer;
			// round to cache line size
			int db=((int)pFree & 0x1F);
			if (db > 0) db = 32-db;
			pFree += db;
			cb -= (cbBuffer + db);
			}
		}


	void HeapPci::I2oFree(char *pFree) {
		// Do this pointer indirection in two steps to avoid compiler bug.
		// (Compiler fetched only byte, even though was U32*)
		U32 paHead=*(U32*)(GT_I2O_INFREE_HEAD_REG);
		paHead=GTREG(paHead); // Do on separate line to avoid multiple i/o accesses
		U32 *pHead=(U32*) P_PA(paHead);
		U32 qBase=paHead & (0x1FFFFFFF - modQueue);
		U32 qIndex=paHead & modQueue;

		*pHead = PCI_PALOCAL(PA_P(pFree));

		qIndex = (qIndex + 4) & modQueue;
		paHead=qBase | qIndex;
		*(U32*)(GT_I2O_INFREE_HEAD_REG) = GTREG(paHead);
		}
		

	// Put buffers onto free list
	void HeapPci::I2oBufInit(char *pFree, U32 cb, U32 cbBuffer) {
		// Each buffer as a header before it.
		cbBuffer += sizeof(Header);

		while (cb > cbBuffer) {
			Header *pHdr=(Header*)pFree;
			pHdr->iQueue=1;
			pHdr->pHeap=this;
			
			I2oBuf(pFree+sizeof(Header));
			pFree += cbBuffer;
			// round to cache line size
			int db=((int)pFree & 0x1F);
			if (db > 0) db = 32-db;
			pFree += db;
			cb -= (cbBuffer + db);
			}
		}


	void HeapPci::I2oBuf(char *pFree) {
		U32 *pPa=(U32*)(GT_I2O_OUTPOST_HEAD_REG);
		U32 paHead=*pPa;
		paHead=GTREG(paHead); // Do on a separate line to avoid multiple i/o accesses
		U32 *pHead=(U32*) P_PA(paHead);
		U32 qBase=(U32)pHead & (0x1FFFFFFF - modQueue);
		U32 qIndex=(U32)pHead & modQueue;

		*pHead = PCI_PALOCAL(PA_P(pFree));

		qIndex = (qIndex + 4) & modQueue;
		paHead=qBase | qIndex;
		*(U32*)(GT_I2O_OUTPOST_HEAD_REG) = GTREG(paHead);
		}


	void HeapPci::FreeLocalBuffer(char *pLocal_, int iQueue_) {
//Tracef("FreeLocalBuffer(%lx)\n", pLocal);
		Critical section;
		if (iQueue_)
			I2oBuf(pLocal_);
		else
			I2oFree(pLocal_);
		}
		

	U32 HeapPci::GetLocalMessage() {
		U32 paHead=*(U32*)GT_I2O_INPOST_HEAD_REG;
		U32 *pHead=(U32*) P_PA(GTREG(paHead));
		U32 paTail=*(U32*)GT_I2O_INPOST_TAIL_REG;
		U32 *pTail=(U32*) P_PA(GTREG(paTail));

		if (pHead == pTail)
			return 0xFFFFFFFF;

		// Get pci and inc tail ptr.
		U32 qBase=PA_P(pTail) & ~modQueue;
		U32 qIndex=PA_P(pTail) & modQueue;

		U32 pci=*pTail;

		qIndex = (qIndex + 4) & modQueue;
		*(U32*)GT_I2O_INPOST_TAIL_REG = GTREG(qBase | qIndex);
			
		return pci;
		}

	U32 HeapPci::GetLocalReturnBuffer() {
		U32 paHead=*(U32*)GT_I2O_OUTFREE_HEAD_REG;
		U32 *pHead=(U32*) P_PA(GTREG(paHead));
		U32 paTail=*(U32*)GT_I2O_OUTFREE_TAIL_REG;
		U32 *pTail=(U32*) P_PA(GTREG(paTail));

		if (pHead == pTail)
			return 0xFFFFFFFF;

		// Get pci and inc tail ptr.
		U32 qBase=PA_P(pTail) & ~modQueue;
		U32 qIndex=PA_P(pTail) & modQueue;

		U32 pci=*pTail;

		qIndex = (qIndex + 4) & modQueue;
		*(U32*)GT_I2O_OUTFREE_TAIL_REG = GTREG(qBase | qIndex);
			
		return pci;
		}

	void HeapPci::PublishBufferSizes(U32 sMsg, U32 sBuf) {
		U32 cbcb=((sMsg-1) << 16) | (sBuf-1);
		*(U32*)(GT_I2O_OUTMSG_1_REG) = cbcb;

		// Set state to ready
		SetState(Address::IOP_ACTIVE);
		}
