/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpCache.h
// 
// Description:
// This file implements the  FCP Target cache methods that interface
// to the cache manager. 
// 
// $Log: /Gemini/Odyssey/FCP/FcpCacheIo.h $
// 
// 1     12/03/99 7:44p Jtaylor
// New file for caching
// 
// 8/18/99 aTaylor: Create file from Jim's FF interface
/*************************************************************************/

#if !defined(FcpCacheIo_H)
#define FcpCacheIo_H
#include "OsTypes.h"
#include "Cache.h"
#include "FcpEvent.h"
// cache request may or may not use 2ndary cache
// we need to let this be known via flags
// flags are as follows
//	bit 0 is FCP_FIRST_BLOCK_BIT
//  bit 1 is FCP_LAST_BLOCK_BIT	
//  bit 2 - 6 are reserved
//	bit 7 is FCP_CACHE2_BIT (on => request is on caching virtual circuit)

#define	FCP_SECONDARY_CACHE_FLAG	1
#define	FCP_INVALIDATE_CACHE_BIT	2
class	FcpCacheRequest
{
public:
	I64					key ;
	U32					flags ;

	CM_PAGE_HANDLE		CacheBlockHandle ;
	U32					CacheBlockAddr ;

	void				*pContext ;
	FcpCacheRequest		*prev ;
	FcpCacheRequest		*next ;

	FcpCacheRequest (I64 key, U8 op);
	STATUS	FcpGetCacheBlock (I64 reqkey, U8 op) ;

	void FcpReleaseCacheBlock () ;
	void FcpSetCacheInvalid ();
	void FcpSetCacheContext (void *pContext);

	U8					op ;
	U8					state ;

};


STATUS	FcpGetCacheRequest (I64 reqkey, U8 reqop, void **pThis);
void	FcpTargetReleaseCache (FcpCacheRequest *pReq);
void	FcpTargetSetCacheInvalid(FcpCacheRequest *pReq);
void	FcpSetCacheContext (FcpCacheRequest *pReq, void *pContext);


#endif