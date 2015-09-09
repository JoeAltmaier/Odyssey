/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpCache.cpp
// 
// Description:
// This module implements the Cache for target FCP driver.  In target
// mode, the driver receives SCSI commands from the host.
// Commands that utilize secondary storage pass through the target Cache
// 8/23/99 aTaylor Initial creation
//
// Update Log 
/*************************************************************************/
#include "FcpCommon.h"
#include "FcpBuffer.h"
#include "FcpCheck.h"
#include "FcpIOCB.h"
#include "FcpISP.h"
#include "FcpMemory.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"
#include "FcpMessage.h"
#include "FcpRequestFIFO.h"
#include "FcpTarget.h"
#include "Scsi.h"
#include "FcpString.h"
#include "FcpI2O.h"
#include "FcpCache.h"
#include "FcpCacheIo.h"
#include "Cache.h"


FCP_CACHE	FcpCache = {0};

//	initialize routine for board
//

//	This is where 1st level and 2nd level cache are assigned
//	But assigned only once - all ports on the board share the same cache
STATUS	InitializeCache ()
{
	STATUS	status=OK ;
	U32		size_memory;
	void	*p_memory, *p_memory1, *p_memory2;

	if (FcpCache.CacheHandle == NULL)
	{/* 1st instantiation for this board - set up cache */
		FcpCache.WaitingForCache.head = NULL ;
		FcpCache.WaitingForCache.tail = NULL ;
		p_memory1 = FCP_Alloc((tPCI|tUNCACHED), num_pages_level1*PAGE_SIZE);
		if (p_memory1)
		{ /* allocated 1st level cache */
			p_memory2 = FCP_Alloc((tBIG|tUNCACHED), num_pages_level2*PAGE_SIZE);
			if (p_memory2)
			{ /* have 1st and second */
				FcpCache.CacheConfigRecord.version = CM_CONFIG_VERSION ;
				FcpCache.CacheConfigRecord.page_size = PAGE_SIZE ; // 4K bytes/page
				FcpCache.CacheConfigRecord.num_pages = num_pages_level1 ;
				FcpCache.CacheConfigRecord.p_page_memory = p_memory1 ;
				FcpCache.CacheConfigRecord.num_pages_secondary = num_pages_level2 ;
				FcpCache.CacheConfigRecord.p_page_memory_secondary = p_memory2 ;
				FcpCache.CacheConfigRecord.page_table_size = 0 ;	// Not a linear map
				FcpCache.CacheConfigRecord.hash_table_size = num_pages_level1+num_pages_level2 ; // using 4MB for eval test 
				FcpCache.CacheConfigRecord.num_reserve_pages = 4 ;
				FcpCache.CacheConfigRecord.dirty_page_writeback_threshold = 20 ;
				FcpCache.CacheConfigRecord.dirty_page_error_threshold = 100 ;
				FcpCache.CacheConfigRecord.num_prefetch_forward = 0 ;
				FcpCache.CacheConfigRecord.num_prefetch_backward = 0 ;

				status = CM_Get_Memory_Size_Required(&FcpCache.CacheConfigRecord, &size_memory);
				if (status == OK)
				{
					p_memory = FCP_Alloc(tSMALL,size_memory);
					FcpCache.CacheConfigRecord.p_table_memory = p_memory ;

					status = CM_Initialize((CM_CONFIG *)&FcpCache.CacheConfigRecord, NULL, NULL,
						&FcpCache.CacheHandle, NULL) ;
					if (status == OK)
	 				{
						return status;
					}
					FCP_Free (p_memory) ;
				}
				FCP_Free (p_memory2) ;
			}
			FCP_Free (p_memory1) ;
		}
		status = -1 ;
	}
	return status;
}

void	FCP_Check_Queue (CM_PAGE_HANDLE CacheBlockHandle)
{
		STATUS			status;
		FCP_EVENT_CONTEXT	*pContext;
		FcpCacheRequest	*head, *prev, *next, *marker ;

		head = FcpCache.WaitingForCache.head;
		marker = NULL ;
		while (head != marker)
		{
			prev = head->prev ;
			next = head->next ;
		  // remove from wait list - if still locked it will be put on the wait chain
				if (prev)
				prev->next = next ;
			else
				FcpCache.WaitingForCache.head = next;					
			if (next)
				next->prev = prev ;
			else
				FcpCache.WaitingForCache.tail = prev ;					

			head->next = NULL;
			head->prev = NULL;
			status = head->FcpGetCacheBlock (head->key, head->op) ;
			if (status == NU_SUCCESS)
			{// We've got block we were waiting for 
				head->state = FCP_VALID_CACHE ;
			}
			else if (status == CM_ERROR_CACHE_MISS)
			{
				status = NU_SUCCESS;
				head->state = FCP_FILLING_CACHE ;
			}
			else
			{
	FcpQueueToWaitList(head);
				if (marker == NULL)
				{ // 1st time failed & put on the wait chain again
					marker = head ;
					if (next == NULL)
						next = marker;
				}
			}
			if (status == NU_SUCCESS)
			{// We've got cache block - procede with request 
				pContext = (FCP_EVENT_CONTEXT *)head->pContext ;
		if (pContext->buffer != head)
		{
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_Check_Queue", 
					"Context->buffer != head",
					0,
					0);
		}
				pContext->action = TARGET_ACTION_HANDLE_CACHE_AVAILABLE ;
				pContext->req_state = head->state ;
			// Send a message to FCP_Event_Task.
			// The action field of the context will tell FCP_Event_Task
			// what to do next. 
    			status = NU_Send_To_Queue(&pContext->Id->FCP_event_queue, 
  						&pContext, // message is pointer to context
      					1, // size is one UNSIGNED 
      					NU_NO_SUSPEND);
				return ;
			}
			head = next ;
		}
		return ;
}

U8		FcpCheckIfOnWaitList(FcpCacheRequest *pReq)
{
		FcpCacheRequest	*head, *next;

		head = FcpCache.WaitingForCache.head;
		while (head != NULL)
		{
			next = head->next ;
			if (head == pReq)
				return (TRUE);
			head = next;
		}
		return (FALSE);
}

U8		FcpCheckContextWaiting(FCP_EVENT_CONTEXT *pContext)
{
		FcpCacheRequest	*head, *next;

		head = FcpCache.WaitingForCache.head;
		while (head != NULL)
		{
			next = head->next ;
			if (head->pContext == pContext)
				return (TRUE);
			head = next;
		}
		return (FALSE);
}



void	FcpQueueToWaitList(FcpCacheRequest *pReq) 
{
	FcpCacheRequest	*tail ;
		FCP_EVENT_CONTEXT	*pContext;


		pContext = (FCP_EVENT_CONTEXT *)pReq->pContext ;
		if (pContext->buffer != pReq)
		{
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FcpQueueToWaitList", 
					"Context->buffer != head",
					0,
					0);
		}

	tail = FcpCache.WaitingForCache.tail ;
	pReq->next = NULL ;
	pReq->prev = tail ;
	if (tail)
	{
		tail->next = pReq ;
	}
	else
	{
		FcpCache.WaitingForCache.head = pReq ;
	}
	FcpCache.WaitingForCache.tail = pReq ;
}

		
U32		FCP_GetCachePageSize ()	
{
	return (FcpCache.CacheConfigRecord.page_size) ;
}

STATUS	FCP_Open_Page (I64 key, U32 flags, U32 *pAddr, CM_PAGE_HANDLE *pHandle)
{
	return (CM_Open_Page (FcpCache.CacheHandle, key, flags, (Callback_Context *)NULL, (void **)pAddr, pHandle)) ;
}

STATUS	FCP_Close_Page (CM_PAGE_HANDLE Handle)
{
	return (CM_Close_Page (FcpCache.CacheHandle, Handle)) ;
}

STATUS	FCP_Abort_Page (CM_PAGE_HANDLE Handle)
{
	return (CM_Abort_Page (FcpCache.CacheHandle, Handle)) ;
}

