/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpCacheIo.cpp
// 
// Description:
// This module implements the cache request class
//
// Update Log 
//	aTaylor	Initial creation
/*************************************************************************/
#include "FcpCommon.h"
#include "FcpTarget.h"
#include "FcpCache.h"
#include "FcpCacheIo.h"
#include "Cache.h"
#include "FcpString.h"


STATUS	FcpGetCacheRequest (I64 reqkey, U8 reqop, void **pThis)
{
	FcpCacheRequest *pReq ;
	STATUS			status ;

	pReq = new FcpCacheRequest(reqkey, reqop) ;
	*pThis =  pReq ;
	if (pReq)
	{
		status = pReq->FcpGetCacheBlock (reqkey, reqop) ;
		return (status) ;
	}
	else
		return (CM_ERROR_NO_MEMORY) ;
}

void	FcpTargetSetCacheInvalid(FcpCacheRequest *pReq)
{
	pReq->FcpSetCacheInvalid() ;
}

void	FcpSetCacheContext (FcpCacheRequest *pReq, void *pContext)
{
	pReq->FcpSetCacheContext(pContext);
}

void	FcpTargetReleaseCache (FcpCacheRequest *pReq)
{
	pReq->FcpReleaseCacheBlock() ;
}

void	FcpCacheRequest::FcpSetCacheInvalid ()
{
	this->flags |= FCP_INVALIDATE_CACHE_BIT;
}

FcpCacheRequest::FcpCacheRequest (I64 key, U8 op)
{
	this->key = key;
	this->op = op;
	this->flags = 0;
	this->pContext = NULL;
	this->next = NULL ;
	this->prev = NULL ;
}

void	FcpCacheRequest::FcpSetCacheContext (void *pContext)
{
	this->pContext = pContext;
}

STATUS	FcpCacheRequest::FcpGetCacheBlock (I64 reqkey, U8 op)
{// key is unique and represents a virtual block on the virtual media
	STATUS	status ;
//	U32		cached = flags & FCP_SECONDARY_CACHE_FLAG;

	U32		oflags, cached = 0;

	if (op & FCP_WRITE_BIT)
		oflags = CM_OPEN_MODE_WRITE ;
	else
		oflags = CM_OPEN_MODE_READ ;
	if (cached)
		oflags |= CM_SECONDARY ;
	key = reqkey ;
	status = FCP_Open_Page(reqkey, oflags, &CacheBlockAddr, &CacheBlockHandle) ;

	switch (status)
	{
		case NU_SUCCESS: /* cache hit - use this block */
		/* this block is valid for partial writes */
				state = FCP_VALID_CACHE ;
			    break ;

		case CM_ERROR_CACHE_MISS:
				state = FCP_FILLING_CACHE ;
				break ;
		case CM_ERROR_PAGE_LOCKED:
		/* requested page is locked by another request */
		/* put request on waiting queue - will be dequeued when page unlocked */
			state = FCP_LOCKED_CACHE ; // is handle valid ? - should queue on this so close can check
	//		FcpQueueToWaitList(this) ;
			break ;
		case CM_ERROR_NO_PAGE_FRAMES:
			state = FCP_NO_CACHE ;
		//	FcpQueueToWaitList(this) ;
			break ;
		case CM_ERROR_MAX_DIRTY_PAGES:
state = FCP_NO_CACHE ;

			break ;
		default: //sw error - enter the debugger
state = FCP_NO_CACHE ;
			break ;
	}
	return (status) ;
}

void FcpCacheRequest::FcpReleaseCacheBlock ()
{
	STATUS	status ;
	
	if (flags & FCP_INVALIDATE_CACHE_BIT)
		status = FCP_Abort_Page (CacheBlockHandle) ;
	else
		status = FCP_Close_Page (CacheBlockHandle) ;
	// check for locked/unavailable chain
	if (status == NU_SUCCESS)
	{
		FCP_Check_Queue (CacheBlockHandle) ;
	}
	else
		state = FCP_NO_CACHE ;
	delete this ;
	return ;
}
