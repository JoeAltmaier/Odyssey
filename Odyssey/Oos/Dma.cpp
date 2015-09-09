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
// This class implements efficient data copying.
// 
// Update Log: 
// 7/17/98 Joe Altmaier: Create file
/*************************************************************************/
#include <String.h>

#define _TRACEF
#include "Odyssey_Trace.h"

#include "OsTypes.h"
#include "Dma.h"
#include "Interrupt.h"
#include "Address.h"
#include "Critical.h"
#include "BuildSys.h"
#include "Kernel.h"
#include "Os.h"
#include "PciSlot.h"
#include "TimerStatic.h"

#ifdef _ODYSSEY
#define TYPES_H	// don't include Types.h
#define I32 signed long
#include "Mips_util.h"
#endif

	DEVICENAME(Dma, Dma::Initialize);
	FAILNAME(Dma, Dma::Suspend);
	SUSPENDNAME(Dma, Dma::Suspend, Dma::Resume);
	
	TyDma::TyDma(TyDma *_pNext, void *_pSrc, void *_pDst, U32 _cb, DmaCallback _pProcCallback, void *_pArg, U32 _ctrl_flags) {
		cb = _cb;
		pSrc = _pSrc;
		pDst = _pDst;
		pNext = _pNext;
		ctrl_flags = _ctrl_flags;
		pProcCallback = _pProcCallback;
		pArg = _pArg;
		pTarget = NULL;
	}

	TyDma::TyDma(TyDma *_pNext, void *_pSrc, void *_pDst, U32 _cb, CallbackTarget *_pTarget, CallbackMethod _pMethodCallback, void *_pArg, U32 _ctrl_flags) {
		cb = _cb;
		pSrc = _pSrc;
		pDst = _pDst;
		pNext = _pNext;
		ctrl_flags = _ctrl_flags;
		pTarget = _pTarget;
		pMethodCallback = _pMethodCallback;
		pArg = _pArg;
		pProcCallback = NULL;
	}

	TyDma::TyDma(TyDma *_pNext, void *_pSrc, void *_pDst, U32 _cb, U32 _ctrl_flags) {
		cb = _cb;
		pSrc = _pSrc;
		pDst = _pDst;
		pNext = _pNext;
		ctrl_flags = _ctrl_flags;
		pProcCallback = NULL;
		pTarget = NULL;
	}

	Status TyDma::Invoke(Status status) {
		if (pProcCallback)
			pProcCallback(pArg, status);

		else if (pTarget)
			(pTarget->*pMethodCallback)(pArg, status);

		return OK;
	}


	TyChain::TyChain(TyChain *_pTc, U32 _paSrc, U32 _paDst, U32 _cb) {
		cb=GTREG(_cb);
		// Make pointers into pa or pci
		paSrc=GTREG(_paSrc);
		paDst=GTREG(_paDst);
		U32 _paNext=PA_P(_pTc);
		paNext=_pTc? GTREG(_paNext) :0;
		}
		
	TyDma *Dma::pDmaHead;
	TyDma *Dma::pDmaTail;
	TyDma *Dma::pDmaActive;
	TyDma *Dma::pDmaActiveLast;
	TyChain *Dma::pDmaChain;

	BOOL Dma::fDmaWasActive;
	BOOL Dma::fSuspend;
	
	U32 cTransfer;
	void *Dma::pTimer;

	void Dma::Initialize() {
		pDmaHead = NULL;
		pDmaTail = NULL;
		pDmaActive = NULL;
		pDmaActiveLast = NULL;
		pDmaChain = NULL;

		// Timer for timing out dma
		pTimer=new TimerStatic(Dma::Timeout, NULL);
		
		// Initialize dma mechanism
		Interrupt::SetHandler(Interrupt::tyDma0Comp, (Interrupt::IntHandler)Int_Dma, OK);
		Interrupt::SetHandler(Interrupt::tyDmaOutl, (Interrupt::IntHandler)Int_Dma, CTS_CHAOS_DMA_ERROR);
	}

	STATUS Dma::Transfer(void *_pSrc, void *_pDest, U32 _cb, DmaCallback _pProcCallback, void *_pArg, U32 _ctrl_flags) {
		// Queue dma descriptor
		return Transfer(new TyDma(NULL, _pSrc, _pDest, _cb, _pProcCallback, _pArg, _ctrl_flags));
		}

	STATUS Dma::Transfer(void *_pSrc, void *_pDest, U32 _cb, CallbackTarget *_pTarget, CallbackMethod _pMethodCallback, void *_pArg, U32 _ctrl_flags) {
		// Queue dma descriptor
		return Transfer(new TyDma(NULL, _pSrc, _pDest, _cb, _pTarget, _pMethodCallback, _pArg, _ctrl_flags));
		}

	STATUS Dma::Transfer(TyDma *pTd) {
		TyDma *pTdLast=pTd;
		while (pTdLast->pNext)
			pTdLast=pTdLast->pNext;

		BOOL _fStart=false;

		{ Critical section;
		if (pDmaTail) {
			pDmaTail->pNext=pTd;
			pDmaTail=pTdLast;
		}
		else {
			pDmaHead=pTd;
			pDmaTail=pTdLast;
		}

		if (pDmaActive == NULL) {
			pDmaActive=pDmaHead;
			pDmaHead=NULL;
			pDmaTail=NULL;
			_fStart=true;
		}
		}

//Tracef("Dma::Transfer pDmaActive=%lx fStart=%d\r\n", pDmaActive, _fStart);

		if (_fStart)
			DmaStart();

//Tracef("Dma::Transfer DmaStart done.\n");
		return OK;
	}


	void Dma::DmaStart() {
		if (fSuspend)
			return;
			
Log('DmaS', (U32)pDmaActive);
		cTransfer++;
Log('cDma', (U32)cTransfer);

		STATUS status=OK;
		
		// CPU move short messages
		while (pDmaActive && pDmaActive->cb < 300) {
//		while (pDmaActive) {
			TyDma &desc=*pDmaActive;

			if (!PciSlot::bcopy(desc.pSrc, desc.pDst, desc.cb))
				status=CTS_CHAOS_DMA_ERROR;
Log('bcpy', (U32)desc.pDst);

			status=desc.Invoke(status);

			pDmaActive=desc.pNext;
			delete &desc;
		}
Log('Dma?', (U32)pDmaActive);

		if (!pDmaActive)
			return;
	
		TyDma *pTd=pDmaActive;
Log('DmaP', (U32)pTd);

		// Disable channel
		*(U32*)(GT_DMA_CH0_CONTROL_REG) = GTREG(0);

		U32 ctrl_flags=0;
		U32 cbTotal=0;

		// Build a chain of dma descriptors, until we get to a callback.
		// Dma descriptors contain physical addresses in little-endian order.
		while (pTd) {
Log('pSrc', (U32)pTd->pSrc);
Log('pDst', (U32)pTd->pDst);

			int cbLeft=pTd->cb;
			int cb=cbLeft;
			U32 paSrc=PA_P(pTd->pSrc);
			U32 paDst=PA_P(pTd->pDst);

			// DMA Controller command
			// Add user flags e.g. GT_DMA_DDIR_HOLD
			U32 CTRL = pTd->ctrl_flags;

			U32 config=*(U32*)(GT_CPU_CONFIG_REG);
			if ( GTREG(config) & GT_PCI_2GIG ) {
				/* We are Using PCI 2G Space, the regular decoder is bypassed
				 * So we need to Set the Overrride bits for the Source and
				 * Destination Addresses
				 */
				if (!IS_PALOCAL(paSrc))
					CTRL |= GT_DMA_SLP_PCI0;
#ifdef _ODYSSEY
				else if (IS_CACHED(pTd->pSrc))
					mips_sync_cache(pTd->pSrc, (cbLeft + 31) & 0xFFFFFFE0, SYNC_CACHE_FOR_DEV);
#endif
				if (!IS_PALOCAL(paDst))
					CTRL |= GT_DMA_DLP_PCI0;
			}

			// If dma mode changes, stop and do what we have so far that is all the same mode.
			if (ctrl_flags && ctrl_flags != CTRL)
				break;

			ctrl_flags = CTRL;

			cbTotal += cb;

			// Can't dma 64k.  Break it up into smaller pieces.
			while (cb) {
				if (cb >= 65536)
					cb=64000;
				pDmaChain=new TyChain(pDmaChain, paSrc, paDst, cb);
				cbLeft -= cb;
				paSrc += cb;
				paDst += cb;
				cb=cbLeft;
			}

			// If callback, stop on this transfer so call can be made immediately.
		 	if (pTd->HasCallback())
		 		break;

			pTd=pTd->pNext;
		}

 
Log('Go! ', (U32)pDmaChain);
		U32 _paDmaChain=PA_P(pDmaChain);
		*(U32*)(GT_DMA_CH0_NEXT_REG) = GTREG(_paDmaChain);

		// Remember where we left off.
		pDmaActiveLast=pTd;

		// Start timer to cancel dma if it lasts too long.
		// The PCI bus runs at 50MHz
		// We can transfer 8 bytes per cycle
		// We get at most half of the PCI bus
		// Time out if it takes longer than 4 times the max
		// Total time to transfer cb bytes in usec:
		//  (cb / 8) cycles / 50 cycles/usec / (1/2) pci bus * factor of 4
		//  comes out to cb / 50
//		((TimerStatic*)pTimer)->Enable(cbTotal / 50 , 0);

		// Start channel
		*(U32*)(GT_DMA_CH0_CONTROL_REG) = GTREG(ctrl_flags);
	}


	void Dma::Int_Dma(Status status_) {
Log('DmaI', 0);
		// If timeout expired and more dma programmed, then we get int
		// but dma is already reprogrammed and running.
		if (IsRunning())
			return;

		/* Disable the DMA Channel */
		Disable();

		/* Disable the timer */
		((TimerStatic*)pTimer)->Disable();

		// If expired and no more dma, then we get int but no dma active.
		if (!pDmaActiveLast)
			return;
			
		// HISR level interrupt routine for dma complete.
		TyDma &desc=*pDmaActiveLast;

		// Return control to caller.
Log('Call', (U32)desc.pProcCallback);
		desc.Invoke(status_);

		// Make any read data cache-consistent
		while (pDmaActive) {
Log('delt', (U32)pDmaActive);
			TyDma *pTd=pDmaActive;
			pDmaActive=pTd->pNext;
#ifdef _ODYSSEY
			if (IS_PALOCAL(PA_P(pTd->pDst)) && IS_CACHED(pTd->pDst))
				mips_sync_cache(pTd->pDst, (pTd->cb + 31) & 0xFFFFFFE0, SYNC_CACHE_FOR_CPU);
#endif
			delete pTd;
		 	if (pTd == pDmaActiveLast)
		 		break;
		}

		// Discard complete links in the chain
		while (pDmaChain) {
Log('chan', (U32)pDmaChain);
			TyChain *pTc=(pDmaChain->paNext ? (TyChain*)P_PA(GTREG(pDmaChain->paNext)) : NULL);
			delete pDmaChain;
			pDmaChain=pTc;
		}
					
		pDmaActiveLast=NULL;
		
		// Start any queued dma transfer(s)
		DmaStart();
Log('done', 0);
	}

	// A dma transfer failed to complete.
	void Dma::Timeout(void *pContext_) {
		pContext_;
		Int_Dma(CTS_CHAOS_DMA_TIMEOUT);
	}

	// Suspend dma (when PCI bus is going to be unavailable for a short while)
	void Dma::Suspend() {
		Critical section;
		
		fDmaWasActive = (!fSuspend && pDmaActive != NULL);
		if (fDmaWasActive) {
			// Suspend chip
			Disable();
		}
		
		// Disable starting more dma
		fSuspend=true;
	}
	
	// Resume dma (when PCI bus is available again)
	void Dma::Resume() {
		Critical section;

		// Enable starting more dma
		fSuspend=false;

		if (fDmaWasActive) {
			// Enable chip
			U32 ctrl=*(U32*)(GT_DMA_CH0_CONTROL_REG);
			ctrl=GTREG(ctrl);
			ctrl |= GT_DMA_CHAN_EN;
			*(U32*)(GT_DMA_CH0_CONTROL_REG) = GTREG(ctrl);
		}
		else DmaStart(); // Start anything that arrived while suspended
	}
