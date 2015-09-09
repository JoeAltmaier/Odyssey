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
// $Log: /Gemini/Odyssey/FCP/FcpCache.h $
// 
// 2     12/20/99 7:51p Jtaylor
// Make 1st level cache size 32 mb
// 
// 1     12/03/99 7:44p Jtaylor
// New file for caching
// 
// 8/18/99 aTaylor: Create file from Jim's FF interface
/*************************************************************************/

#if !defined(FcpCache_H)
#define FcpCache_H

#include "Cache.h"
#include "Callback.h"
#include "List.h"

//#define EVAL_BOARD

//#define	PAGE_SIZE			4096
#define	PAGE_SIZE			65536

#ifdef	EVAL_BOARD

#define num_pages_level1	128
#define num_pages_level2	512
#else
// 8MB at 64k page size
#define	num_pages_level1	512
#define num_pages_level2	128
#endif

#define FCP_WRITE_BIT		4

class FcpCacheRequest;

typedef struct	_CM_WAIT_QUEUE
{
	FcpCacheRequest	*head ;
	FcpCacheRequest	*tail ;
} CM_WAIT_QUEUE, *PCM_WAIT_QUEUE ;


typedef struct	_FCP_CACHE
{
	CM_CACHE_HANDLE CacheHandle ;
	CM_CONFIG		CacheConfigRecord ;
	CM_WAIT_QUEUE	WaitingForCache ;
} FCP_CACHE;


#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS	InitializeCache () ;

U32		FCP_GetCachePageSize ()	;

void	FCP_Check_Queue (CM_PAGE_HANDLE CacheBlockHandle) ;

void	FcpQueueToWaitList(FcpCacheRequest *pReq) ; 

// for debug
U8		FcpCheckContextWaiting(FCP_EVENT_CONTEXT *pContext);
U8		FcpCheckIfOnWaitList(FcpCacheRequest *pReq);


STATUS	FCP_Close_Page (CM_PAGE_HANDLE Handle);

STATUS	FCP_Open_Page (I64 key, U32 flags, U32 *pAddr, CM_PAGE_HANDLE *pHandle);

STATUS	FCP_Abort_Page (CM_PAGE_HANDLE Handle);


#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif //   FcpCache_H
