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
// 2/18/99 Joe Altmaier: Add TyChain for chained dma.
// 02/19/99 Jim Frandeen: Add end of line
// 02/24/99 Jim Frandeen: Change STATUS to Status to avoid Nucleus.h
// 08/02/99 Joe Altmaier: TyChain takes pa args
/*************************************************************************/

#ifndef __Dma_h
#define __Dma_h

#include "Galileo.h"

typedef void (*DmaCallback)(void *pArg, Status status);

class CallbackTarget {};

typedef void (CallbackTarget::*CallbackMethod)(void *, Status);
#ifdef WIN32
#define CALLBACK(clas,method)	(CallbackMethod) method
#elif defined(__ghs__)  // Green Hills
#define CALLBACK(clas,method)	(CallbackMethod) &clas::method
#else	// MetroWerks
#define CALLBACK(clas,method)	(CallbackMethod)&method
#endif


// Dma descriptor, built by client, passed to Transfer()
struct TyDma {
	U32 cb;
	void *pSrc;
	void *pDst;
	TyDma *pNext;
	U32 ctrl_flags;
	DmaCallback pProcCallback;
	CallbackMethod pMethodCallback;
	CallbackTarget *pTarget;
	void *pArg;

	enum {flagsDefault = GT_DMA_CHAN_EN | GT_DMA_BLOCK_MODE | GT_DMA_XMIT_64 | GT_DMA_INT_MODE | GT_DMA_FET_NEXTREC};

	TyDma(TyDma *pTd, void *pSrc, void *pDst, U32 cb, U32 ctrl_flags=flagsDefault);
	TyDma(TyDma *pTd, void *pSrc, void *pDst, U32 cb, DmaCallback pProcCallback, void *pArg, U32 ctrl_flags=flagsDefault);
	TyDma(TyDma *pTd, void *pSrc, void *pDst, U32 cb, CallbackTarget *pTarget, CallbackMethod pMethodCallback, void *pArg, U32 ctrl_flags=flagsDefault);

	void SetCallback(DmaCallback pProcCallback_) { pProcCallback = pProcCallback_; }
	Status Invoke(Status status);
	BOOL HasCallback() { return (pProcCallback != NULL || pTarget != NULL); }
	};

// GT64120 dma descriptor, built and passed to chip
struct TyChain {
	U32 cb;
	U32 paSrc;
	U32 paDst;
	U32 paNext;

	TyChain(TyChain *pTc, U32 paSrc, U32 paDst, U32 cb);
	};


	// Statistics
	extern
	U32 cTransfer;


class Dma {
	// Dma transfers pending
	static
	TyDma *pDmaHead;
	static
	TyDma *pDmaTail;
	
	// Dma transfers underway
	static
	TyDma *pDmaActive;
	
	// Dma transfer last attempted
	static
	TyDma *pDmaActiveLast;
	
	// Dma descriptors currently in progress
	static
	TyChain *pDmaChain;

	static
	BOOL fSuspend;
	
	static
	BOOL fDmaWasActive;

	static
	void *pTimer;
	
public:
	static
	void Initialize();
	
	// Queue a single transfer
	static
	Status Transfer(void *pSrc, void *pDest, U32 cb, DmaCallback pProcCallback, void *pArg, U32 ctrl_flags=TyDma::flagsDefault);

	static
	Status Transfer(void *pSrc, void *pDest, U32 cb, CallbackTarget *pTarget, CallbackMethod pMethodCallback, void *pArg, U32 ctrl_flags=TyDma::flagsDefault);

	// Queue a chain of transfers
	static
	Status Transfer(TyDma *pTd);

	static
	void Suspend();

	static
	void Resume();
		
private:
	// Program the chip to start a dma chain
	static
	void DmaStart();

	// Dma complete interrupt routine
	static
	void Int_Dma(Status status);

	static
	void Disable() {
		U32 ctrl=*(U32*)(GT_DMA_CH0_CONTROL_REG);
		ctrl=GTREG(ctrl);
		ctrl &= ~(GT_DMA_CHAN_EN | GT_DMA_FET_NEXTREC);
		*(U32*)(GT_DMA_CH0_CONTROL_REG) = GTREG(ctrl);
	}

	static
	BOOL IsRunning() {
		U32 ctrl=*(U32*)(GT_DMA_CH0_CONTROL_REG);
		ctrl=GTREG(ctrl);
		return (ctrl & GT_DMA_CHAN_EN) != 0 && (ctrl & GT_DMA_ACT_STS) != 0;
	}

	static
	void Timeout(void *pContext);
};
#endif
