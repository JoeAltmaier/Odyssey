/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FtTestDMA.cpp
// 
// Description:
// This file simulates the DMA for the Flash Block. 
// 
// $Log: /Gemini/Odyssey/FlashStorage/FlashStorageTest/FtTestDMA.cpp $
// 
// 2     1/12/00 10:33a Jfrandeen
// Use SGL interface
// 
// 1     8/03/99 11:34a Jfrandeen
// 
// 1     4/01/99 7:36p Jfrandeen
// Files common to all flash file system test drivers
// 
// 9/3/98 Jim Frandeen: Create file
/*************************************************************************/

#include "Callback.h"
#include "FlashStorage.h"
#include "Dma.h"
#include "FfCommon.h"
#include <String.h>
 
class FT_DMA_Context : public Callback_Context
{
public:
	void	*m_pSrc;
	void	*m_pDest;
	int		 m_cb;

}; // FT_DMA_Context

/*************************************************************************/
// DMA_Routine
/*************************************************************************/
void DMA_Routine(void *p_context, Status status)
{
	FT_DMA_Context *p_DMA_context = (FT_DMA_Context *)p_context;

	// Do the copy
	memcpy(p_DMA_context->m_pDest, p_DMA_context->m_pSrc, p_DMA_context->m_cb);

	// Schedule this context.  This will cause the callback to be called,
	// and the context to be deallocated.
	p_DMA_context->Make_Ready();

}  // DMA_Routine

/*************************************************************************/
// Transfer
/*************************************************************************/
Status Dma::Transfer(void *pSrc, void *pDest, U32 cb, void (*pProcCallback)(void *pArg, 
				Status status), void *pArg, U32 ctrl_flags)
{

	// Create a FT_DMA_Context
	FT_DMA_Context *p_DMA_context = (FT_DMA_Context *)Callback_Context::Allocate(
		sizeof(FT_DMA_Context),
		pProcCallback, pArg);
	if (p_DMA_context == 0)
		return FF_ERROR(NO_MEMORY);

	// Save parameters
	p_DMA_context->m_pSrc = pSrc;
	p_DMA_context->m_pDest = pDest;
	p_DMA_context->m_cb = cb;

	DMA_Routine(p_DMA_context, OK);

	return OK;

} // Dma::Transfer

/*************************************************************************/
// Transfer
/*************************************************************************/
Status Dma::Transfer(TyDma *p_tydma)
{
	TyDma *p_tydma_last = p_tydma;
	while (p_tydma->pNext)
	{
		// Do the copy
		memcpy(p_tydma->pDst, p_tydma->pSrc, p_tydma->cb);

		if (p_tydma->pNext)
		{
			p_tydma = p_tydma->pNext;
			delete p_tydma_last;
		}
		p_tydma_last = p_tydma;
	}

	// Create a FT_DMA_Context to transfer the last element.
	FT_DMA_Context *p_DMA_context = (FT_DMA_Context *)Callback_Context::Allocate(
		sizeof(FT_DMA_Context),
		p_tydma_last->pProcCallback, p_tydma_last->pArg);
	if (p_DMA_context == 0)
		return FF_ERROR(NO_MEMORY);

	// Save parameters
	p_DMA_context->m_pSrc = p_tydma_last->pSrc;
	p_DMA_context->m_pDest = p_tydma_last->pDst;
	p_DMA_context->m_cb = p_tydma_last->cb;

	delete p_tydma_last;

	DMA_Routine(p_DMA_context, OK);

	return OK;

} // Dma::Transfer

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
