/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpCacheSubs.cpp
//
// Description:
//
//
// Update Log: 
//	9/99	At Taylor:	initial creation
//
//
/*************************************************************************/

#include "FcpCommon.h"		// for bzero prototype
#include "Fcpdata.h"
#include "FcpEvent.h"
#include "FcpTarget.h"
#include "FcpCacheIo.h"
#include "FcpCache.h"
#include "FcpCacheSubs.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"
#include "FcpString.h"
#include "RequestCodes.h"
#include "Pci.h"
#include "FcpMessage.h"
#include "FcpI2O.h"
#include "FcpRequestFIFO.h"
#include "FcpProto.h"
#include "FcpMemory.h"
#include "FcpIOCB.h"
#include "HdmFcp.h"


/*************************************************************************/
// FCP_Build_ReadCTIO
// 
/*************************************************************************/

void	FCP_Build_ReadCTIO (FCP_EVENT_CONTEXT *pContext)
{
	// buffer pts to head of Cache chain
	// p_dsd pts to the proper data descriptor list

	U32					relative_offset, page_size ;
	U16					count ;

	relative_offset = pContext->req_adjust.transfer_offset ;
	page_size = FCP_GetCachePageSize () ;  // Both caches have same page size
	count = pContext->req_adjust.data_segment_count ;
	pContext->req_type = READ_BUFFER_XFER ;
	pContext->req_state = FCP_VALID_CACHE ;
	FCP_Build_Cache_IOCB(pContext, relative_offset, page_size, count, 0, pContext->CTIO_flags) ;
	return ;
}


/*************************************************************************/
// FCP_Build_Cache_Message
// Allocate and build a message to send down the circuit to do I/O
/*************************************************************************/

void	FCP_Build_Cache_Message (FCP_EVENT_CONTEXT *pContext, U32 relative_offset, U32 page_size, U8 NumSGLs)
{
 	INSTANCE_DATA 			*pId ;
	CDB16					*pCDB ;
	FcpCacheRequest			*pChead ; 
	U8						*buffer ;
	U32						offset, transfer_length, length ;
	U32						startLBA ;
	SCB_PAYLOAD				payload;
	U16						i;
	
	pId = pContext->Id ;
	pCDB = &pContext->iocb.CDB ;

	pChead = (FcpCacheRequest *)pContext->buffer ; // get head of cache chain from context
	 
	startLBA = CDB_Get_Logical_Block_Address(pCDB);
	transfer_length = page_size * NumSGLs ;
	if (pContext->req_type == READ_BUFFER_XFER)
	{
		// for now all reads to cache are full cache block reads
		// later make choice based on cache policy
		if (relative_offset)
		{ // not 1st block so delta will put it on cache boundary
			startLBA += (relative_offset/512);
		}
		else
		{ // 1st block - adjust start location/length for full cache block transfer
			offset = pContext->req_adjust.first_request_offset ;
			startLBA -= (offset/512);	// step back to cache boundary
		}
	}
	else
	{	// write from cache buffer to media - only write what is required
		if ( pContext->req_adjust.first_last_flags & FCP_FIRST_BLOCK_BIT)
		{ // adjust for offset of 1st block
				
				transfer_length -= pContext->req_adjust.first_request_offset ;
		}
		if ( pContext->req_adjust.first_last_flags & FCP_LAST_BLOCK_BIT)
		{ // adjust for last partial page
			if (NumSGLs == 1)
				transfer_length = pContext->req_adjust.last_request_count ;
			else
			{
				length = page_size - pContext->req_adjust.last_request_count;
				transfer_length -= length ;
			}
		}
		startLBA += (relative_offset/512);
	}
	bzero((char *)&payload,sizeof(SCB_PAYLOAD));
	payload.ByteCount = transfer_length ;
	
    // fill in the LUN and id fields
    payload.IdLun.HostId = pContext->iocb.initiator_ID;
    payload.IdLun.LUN = BYTE_SWAP16(pContext->iocb.LUN);
	//payload.IdLun.id = 0;		// later, ISP2200 Target Id
    if (pId->ISP_Type == 0x00002200)
    	// 2200 passes the ID in the IOCB
    	payload.IdLun.id = pContext->iocb.target_ID;
    else
    	// for 2100, pass the configured Hard ID
    	payload.IdLun.id = pId->FCP_config.hard_address;
    
	// Copy the CDB from the ATIO into the I2O SCSI message.
	bcopy(
		(char *)&pContext->iocb.CDB,	// src
		(char *)&payload.CDB,			// destination
		FCP_SCSI_CDB_LENGTH);
	
	payload.CDBLength = CDB_Get_CDB_Length((CDB16 *)&payload.CDB);
	
	// set the block address to the CDB
	CDB_Set_Logical_Block_Address((CDB16 *)&payload.CDB, startLBA) ;

	CDB_Set_Transfer_Length((CDB16 *)&payload.CDB, transfer_length / 512);

   	pContext->message = FCP_Allocate_Message(SCSI_SCB_EXEC);
   	if (pContext->message == NULL)
   	{
   		// If we can't allocate a message, 
   		// we can't handle this interrupt!
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Build_Cache_Message", 
			"Fcp_Allocate_Message for message failed",
			0,
			(UNSIGNED)pId);
		return;
	}
	
	FCP_I2O_Add_Payload(pContext->message, &payload, sizeof(SCB_PAYLOAD));
	if (pContext->req_type == READ_BUFFER_XFER)
	{
		for (i = 0; i < NumSGLs; i++)
		{
			length = page_size ;
//			buffer = (U8*) FCP_Get_DMA_Address((void *) pChead->CacheBlockAddr);  // make physical for DMA
			buffer = (U8*) pChead->CacheBlockAddr;

	    	FCP_I2O_Add_SGL_Entry(pContext->message, buffer, length, 
	    	
					    		i /* index */);
			pChead = pChead->next;
		}
	}
	else
	{	// write from cache buffer to media - only write what is required
		for (i = 0; i < NumSGLs; i++)
		{
			buffer = (U8*)pChead->CacheBlockAddr;  // make physical for DMA
			length = page_size ;
			if ( (i==0) && (pContext->req_adjust.first_last_flags & FCP_FIRST_BLOCK_BIT) )
			{
				buffer += pContext->req_adjust.first_request_offset ;
				length -= pContext->req_adjust.first_request_offset ;
			}
			if ( (i== (NumSGLs-1)) && (pContext->req_adjust.first_last_flags & FCP_LAST_BLOCK_BIT) )
			{ 
				length = pContext->req_adjust.last_request_count;
			}
//			buffer = (U8*) FCP_Get_DMA_Address((void *) buffer);  // make physical for DMA
	    	FCP_I2O_Add_SGL_Entry(pContext->message, buffer, length, 
					    		i /* index */);
			pChead = pChead->next;
		}
	}
	return ;
}

/*************************************************************************/
// FCP_Cache_Send_CTIO
// Send Continue Target I/O IOCB to the ISP.
/*************************************************************************/

STATUS		FCP_Cache_Send_CTIO (FCP_EVENT_CONTEXT *pContext, FCP_EVENT_ACTION next_action)
{
	IOCB_CTIO_TYPE_4	*pIOCB ;
	IOCB_STATUS_TYPE0 	*pStatusIOCB;
	U16					iocb_flags ;
	STATUS				status ;
    
	// IOCB ptr saved in pContext->message
	pIOCB = (IOCB_CTIO_TYPE_4 *)pContext->message ;
	iocb_flags = pContext->CTIO_flags;

	// Decrement entry count in master status IOCB
	// only used to determine when sending last IOCB
	pStatusIOCB = (IOCB_STATUS_TYPE0 *) pContext->pStatusIOCB;
	pStatusIOCB->entry_count--;
	if (pStatusIOCB->entry_count == 0)
	{	// Last IOCB being sent for this ATIO request
		// Set send status and increment resource count bits	
		iocb_flags |= ISP_CTIO_FLAGS_SEND_SCSI_STATUS;
		iocb_flags |= ISP_CTIO_FLAGS_INC_RESOURCE_COUNT;
		if (pStatusIOCB->SCSI_status != FCP_SCSI_DSC_SUCCESS)
		{	// Check for an error from a previous context 
			// for this ATIO
			pContext->CTIO_flags |= ISP_CTIO_FLAGS_SEND_SCSI_STATUS;	// so we will free pStatusIOCB
			status = FCP_Send_CTIO_Check(pContext, 
		   		next_action,
		   		(U8 *) &pStatusIOCB->sense_data[0],
		   		pStatusIOCB->sense_data_length,
				  ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
				| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
				| ISP_CTIO_FLAGS_NO_DATA_XFR);
				#if defined(FCP_DEBUG)    
				    pContext->ctio_cnt++;
				#endif
				return (status);
			
		}
	}
	// Set Fast Post bit
	iocb_flags |= ISP_CTIO_FLAGS_FAST_POST;

	// Save flags in context - Handle_CTIO_Final will check
	// for SEND_SCSI_STATUS bit to determine last IOCB and
	// release the master Status IOCB
	pContext->CTIO_flags |= iocb_flags ; 

    pIOCB->flags = BYTE_SWAP16(iocb_flags);

    // Send Continue Target I/O IOCB to the ISP.
    // next_action specifies action to perform by FCP_Event_Task
    status = FCP_Send_Command_IOCB(pContext, next_action, (IOCB_COMMAND_TYPE2*)pIOCB); 
    
#if defined(FCP_DEBUG)    
    pContext->ctio_cnt++;
#endif
    return status;    
}

/*************************************************************************/
// FCP_Build_Cache_IOCB
// Create Continue Target I/O IOCB in ISP request FIFO
/*************************************************************************/

void	FCP_Build_Cache_IOCB(FCP_EVENT_CONTEXT *pContext, U32 relative_offset,
							U32 page_size, U16 Count, U8 SCSI_status, UNSIGNED flags)
{
	FcpCacheRequest 		*pChead;
	DATA_SEGMENT_DESCRIPTOR	*pDSD;
	IOCB_CTIO_TYPE_4 		*pIOCB ;
	U32						list_address;
	U8						*buffer;
	U32						length, transfer_length=0;		 
	U32						offset;		 
	U16						i ;


    // Get ptr to next IOCB in request FIFO.
	pIOCB = (IOCB_CTIO_TYPE_4*)FCP_Request_FIFO_Get_Pointer(pContext->Id);

	// Save ptr for send	
	pContext->message = (void *)pIOCB;

	// Initialize IOCB fields

    pIOCB->entry_type = IOCB_TYPE_CONTINUE_TARGET_IO_TYPE_4;
    pIOCB->entry_count = 1;
    //p_CTIO->LUN = p_context->iocb.LUN;
    pIOCB->initiator_ID = pContext->iocb.initiator_ID;
    pIOCB->RX_ID = pContext->iocb.RX_ID;  
    pIOCB->timeout = BYTE_SWAP16(30);
    pIOCB->transfer_length = 0;							// filled in later
    pIOCB->SCSI_status = BYTE_SWAP16((U16)SCSI_status);	// Set SCSI status in reply.
	pIOCB->system_defined2 = (U32) pContext;			// save context for processing on IO complete
    pIOCB->flags = BYTE_SWAP16(flags);
	pIOCB->residual_transfer_length = 0;

	// Allocate S/G List
	pDSD = (DATA_SEGMENT_DESCRIPTOR *) FCP_Alloc(tPCI | tUNCACHED, 8 * Count);

	pChead = (FcpCacheRequest *)pContext->buffer;		// head of cache chain

	pIOCB->relative_offset = BYTE_SWAP32 (relative_offset) ;
	if (relative_offset)
	{ // not 1st block of transfer - offset = 0
		offset = 0 ;
	}
	else
		offset = pContext->req_adjust.first_request_offset ;

	// save data segment descriptor list ptr in context to delete later
	pContext->p_dsd = pDSD;
	pIOCB->dsd_list_type = 0;
	pIOCB->dsd_list_base_address = 0;
	list_address = (U32) FCP_Get_DMA_Address((void *) pDSD);
	pIOCB->dsd_list_address = BYTE_SWAP32(list_address);
	pIOCB->dsd_list_address_lo = 0;
	for (i=0; i<Count; i++)
	{
		if (i==0) 
		{	// 1st block of orig request - may be offset
			buffer = (U8*) FCP_Get_DMA_Address((void *) (pChead->CacheBlockAddr + offset));  // make physical for DMA
			length = page_size - offset;
		}
		else
		{
			length = page_size ;
			buffer = (U8*) FCP_Get_DMA_Address((void *) pChead->CacheBlockAddr);  // make physical for DMA
		}
		if ( (i == (Count - 1)) && (pContext->req_adjust.first_last_flags & FCP_LAST_BLOCK_BIT) )
			length = pContext->req_adjust.last_request_count;

		transfer_length += length ;
		pDSD->Length = BYTE_SWAP32(length);
		pDSD->Address = BYTE_SWAP32((UNSIGNED) buffer);
		pDSD++;
		pChead = pChead->next;
	}
    pIOCB->data_segment_count = BYTE_SWAP16(Count);
	pIOCB->transfer_length = BYTE_SWAP32(transfer_length);
}


void	MakeWaitCacheContext(FcpCacheRequest *pCacheReq, FCP_EVENT_CONTEXT *pContext,
									STATUS status, U32 transfer_count) 
{ // pCacheReq has been queued on the waiting for block list
  // set context state based on status returned
		if (status == CM_ERROR_PAGE_LOCKED)
				pContext->req_state = FCP_LOCKED_CACHE ;
		else
				pContext->req_state = FCP_NO_CACHE ;
		pContext->buffer = pCacheReq ; // save req block for locked/unavail cache
		pContext->req_adjust.data_segment_count = 1 ;
		if (pContext->req_adjust.first_last_flags & FCP_LAST_BLOCK_BIT)
			pContext->req_adjust.last_request_count = transfer_count ;
			
		FcpQueueToWaitList(pCacheReq) ;

}

STATUS	FCP_Handle_Awaken_Cache_Context(FCP_EVENT_CONTEXT *pContext)
{
	UNSIGNED			iocb_flags=0 ;
	U32					relative_offset, page_size;
	U16				 	SCSI_status = 0;
	FcpCacheRequest		*pCacheReq;
	STATUS				status;

	pCacheReq = (FcpCacheRequest *)pContext->buffer ;
	page_size = FCP_GetCachePageSize () ;  // Both caches have same page size
	relative_offset = pContext->req_adjust.transfer_offset ;

	if (pContext->req_type == READ_BUFFER_XFER)
	{
		pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_IN;
		if (pContext->req_state ==	FCP_VALID_CACHE)
		{// Valid cache - send to host
			FCP_Build_ReadCTIO (pContext) ;
			status = FCP_Cache_Send_CTIO (pContext, TARGET_ACTION_HANDLE_CTIO_FINAL) ;
		}
		else
		{
			pContext->req_state = FCP_FILLING_CACHE ;
			FCP_Build_Cache_Message (pContext, relative_offset, page_size, 1) ;	
		    status = FCP_Message_Send_Request(pContext,	TARGET_ACTION_HANDLE_I2O_RESPONSE);
		}
	}
	else if (pContext->req_type == WRITE_BUFFER_XFER)
	{ // got cache - build CTIO to read data to cache for write to media
		pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_OUT | ISP_CTIO_FLAGS_FAST_POST;

		FCP_Build_Cache_IOCB (pContext, relative_offset, page_size, (U16)1, SCSI_status, pContext->CTIO_flags) ;	
		status = FCP_Cache_Send_CTIO (pContext, TARGET_ACTION_HANDLE_CTIO_WRITE) ;
	}
	else
	{ // error must be read or write buffer to be waiting for a cache block
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_Handle_Awaken_Cache_Context", 
					"Unknown req_type",
					pContext->req_type,
					0);
	}
	return (status);
}					 

STATUS	FCP_Handle_CTIO_CacheWrite (FCP_EVENT_CONTEXT *pContext)
{
 	INSTANCE_DATA 			*pId ;
	IOCB_CTIO_TYPE_4 		*pCTIO ;
	STATUS					status;
	U32						relative_offset, page_size ;
	U16						NumSGLs ;
	
	pId = pContext->Id ;
	pCTIO = (IOCB_CTIO_TYPE_4 *)pContext->message ;
	NumSGLs = BYTE_SWAP16(pCTIO->data_segment_count) ;

	relative_offset = BYTE_SWAP32(pCTIO->relative_offset);
	
	page_size = FCP_GetCachePageSize () ;  // Both caches have same page size
	FCP_Free (pContext->p_dsd) ;
	pContext->p_dsd = NULL ;
	FCP_Build_Cache_Message (pContext, relative_offset, page_size, NumSGLs) ;
    status = FCP_Message_Send_Request(pContext,	TARGET_ACTION_HANDLE_I2O_RESPONSE);
    if (status != NU_SUCCESS)
	{
		// We were not able to send the message.
//		FCP_PRINT_STRING(TRACE_L2, "\n\rFCP_Handle_CTIO_CacheWrite: message not sent");
		status = NU_SUCCESS;
	}
	return (status) ;
}

/*************************************************************************/
// FCP_Combine_Key
// 
/*************************************************************************/

I64		FCP_Combine_Key (U32 HighKey, U32 KeyBlock)
{
	I64		retv;

	retv = (HighKey << 32) | KeyBlock;
	return (retv);
}	


/*************************************************************************/
// FCP_Get_High_Order_Key
// 
/*************************************************************************/

U32		FCP_Get_High_Order_Key (FCP_EVENT_CONTEXT *pContext)
{
	PINSTANCE_DATA	 Id = pContext->Id;
	HdmFcp			*pHdmFcp = (HdmFcp *)pContext->Id->pFCP_DDM;
	VDN		  		 vd;
	union {
		IDLUN		 idlun;
		long		 l;
	} sig;
	extern	unsigned char firmware_version[];

    // fill in the LUN and id fields
    sig.idlun.HostId = pContext->iocb.initiator_ID;
    sig.idlun.LUN = BYTE_SWAP16(pContext->iocb.LUN);
    if ((Id->ISP_Type == 0x00002200) && (firmware_version[0] >= 10))
    	// 2200 passes the ID in the IOCB
    	sig.idlun.id = pContext->iocb.target_ID;
    else
    	// for 2100, pass the configured Hard ID
    	sig.idlun.id = Id->FCP_config.hard_address;
    
    // find next Virtual Device in the route
    vd = pHdmFcp->FCP_Find_Next_Virtual_Device(pContext, sig.l);
	return (vd);
}

/*************************************************************************/
// FCP_Get_Cache_Write_Action
// Return next action for cache write
/*************************************************************************/

U8		FCP_Get_Cache_Write_Action (U8 state, STATUS status) 
{
	U8		action;

	switch (status)
	{
		case NU_SUCCESS:			/* cache hit */
		case CM_ERROR_CACHE_MISS:	/* cache miss block assigned */
			switch (state)
			{
				case SIO:
					// Begin cache write request
					action = START_WRITE_FILL ;
					break ;
				case FILLING_WRITE_CACHE:
					action = CONTINUE_WRITE_FILL ;
					break ;
			}
			break;

		case CM_ERROR_PAGE_LOCKED:
		case CM_ERROR_NO_PAGE_FRAMES:
			switch (state)
			{
				case SIO:
					action = WAIT_ON_CACHE_RESOURCE ;
					break ;
				case FILLING_WRITE_CACHE:
					action = END_WRITE_FILL_WAIT ;
					break ;
			}
			break ;

		default: //sw error - enter the debugger
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_Get_Cache_Read_Action", 
					"Unknown status",
					status,
					0);
			break ;
	}
	return (action) ;
}

/*************************************************************************/
// FCP_Handle_Cache_Write_Command
// 
/*************************************************************************/

STATUS	FCP_Handle_Cache_Write_Command (FCP_EVENT_CONTEXT *pContext, UNSIGNED transfer_length)
{			
 	INSTANCE_DATA 			*pId ;
	FCP_EVENT_CONTEXT 		*pNextContext;
	IOCB_ATIO_TYPE_2		*pIOCB ;
	FcpCacheRequest			*pCacheReq,*pChead, *pCtail ;
	I64						key ;
	U32						page_size_in_sectors ;
	U32						page_size, start_LBA, cache_offset, relative_offset, HighKey, KeyBlock ;
	U32						i, first_block_count, num_blocks, remainder, last_block_count ;
	U32						current_offset, current_byte_count ;
	U8						cache_count, state, action ;
	STATUS					status ;

	pId = pContext->Id ;
	pIOCB = &pContext->iocb ;
	
	page_size = FCP_GetCachePageSize () ;  // Both caches have same page size

	/* start LBA is sector number which is 512 byte units */
	/* start Block number for cache check is in page_size units */

	start_LBA = CDB_Get_Logical_Block_Address(&pIOCB->CDB);
	page_size_in_sectors = page_size/512 ;

	KeyBlock = start_LBA/page_size_in_sectors ;
	cache_offset = start_LBA % page_size_in_sectors ;
	cache_offset *= 512 ;		// number of bytes into cache block for start of transfer

	first_block_count = page_size - cache_offset ;
	num_blocks = 1 ; /* always have at least one cache block */

	pContext->req_adjust.first_last_flags = FCP_FIRST_BLOCK_BIT ;
	pContext->req_adjust.first_request_offset = cache_offset ;
	if (first_block_count >= transfer_length)
	{	/* its all in the first cache block */
		remainder = 0 ;
		first_block_count = transfer_length ;
		last_block_count = transfer_length ;
		pContext->req_adjust.first_last_flags |= FCP_LAST_BLOCK_BIT ;
	}
	else
	{	/* multiple cache blocks required */
		remainder = transfer_length - first_block_count ; 
		last_block_count = remainder % page_size ;
		num_blocks += remainder/page_size ;
		if (last_block_count)
			num_blocks++ ;
		else
			last_block_count = page_size ;
	}
	relative_offset = 0 ;
	current_offset = 0 ;
	cache_count = 0 ;
	current_byte_count = first_block_count ;
    pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_OUT;
	HighKey	= FCP_Get_High_Order_Key (pContext) ;
	pNextContext = pContext ;		// save head of chain
	state = SIO ;

	for (i = 0; i < num_blocks; i++)
	{
		if (i == (num_blocks - 1))
			current_byte_count = last_block_count ;
		key	=  FCP_Combine_Key (HighKey, KeyBlock) ;	
		status = FcpGetCacheRequest (key, WRITE_BUFFER_XFER, &pCacheReq) ;
		action = FCP_Get_Cache_Write_Action (state, status) ;
		if (current_byte_count != page_size)	// partial write
			FcpTargetSetCacheInvalid(pCacheReq);
		switch (action)
		{
			case START_WRITE_FILL:
						pContext->buffer = pCacheReq ;
						pContext->req_adjust.transfer_offset = relative_offset ;
						state = FILLING_WRITE_CACHE ;
						FcpSetCacheContext (pCacheReq, pContext);
						pContext->req_state = FCP_FILLING_CACHE ;
						pContext->req_type = WRITE_BUFFER_XFER ;
						// first cache block for this context
						pChead = pCacheReq ;
						pCtail = pCacheReq ;
						cache_count = 1;
						break ;
			case CONTINUE_WRITE_FILL:
						FcpSetCacheContext (pCacheReq, pContext);
						pCtail->next = pCacheReq ;
						pCacheReq->prev = pCtail;
						pCtail = pCacheReq ;
						// Step count of cache blocks for this context
						cache_count++;
						break ;
			case END_WRITE_FILL_WAIT:	
						pContext->req_adjust.transfer_offset = relative_offset ;
						pContext->req_adjust.data_segment_count = cache_count ;
			  			FCP_Build_Cache_IOCB (pContext, relative_offset, page_size, cache_count, 0, ISP_CTIO_FLAGS_DATA_OUT) ;	

		  				relative_offset = current_offset ;
						// Get a new context
						pContext = FCP_Get_Cache_Context(pContext);
					    pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_OUT;
			case WAIT_ON_CACHE_RESOURCE:
						FcpSetCacheContext (pCacheReq, pContext);
						pContext->req_adjust.transfer_offset = relative_offset ;
						if (i == (num_blocks - 1))
							pContext->req_adjust.first_last_flags |= FCP_LAST_BLOCK_BIT;
		  				MakeWaitCacheContext(pCacheReq, pContext, status, current_byte_count) ;
						relative_offset += current_byte_count ;

						if (i != (num_blocks - 1))
						{
							pContext = FCP_Get_Cache_Context(pContext);
						    pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_OUT;
						}
						state = SIO ;
						break ;

		}
		KeyBlock++ ;
		current_offset += current_byte_count ;
		current_byte_count = page_size ;
	}
	pContext->req_adjust.first_last_flags |= FCP_LAST_BLOCK_BIT;
	pContext->req_adjust.last_request_count = last_block_count ;
	if (pContext->req_state == FCP_FILLING_CACHE)
	{ // build IOCB to write remaining blocks in cache
		pContext->req_adjust.data_segment_count = cache_count ;
		FCP_Build_Cache_IOCB (pContext, relative_offset, page_size, cache_count, 0, ISP_CTIO_FLAGS_DATA_OUT) ;	
	}
	pContext = pNextContext;		// Head of context chain
	while (pContext)
	{
		pNextContext = (FCP_EVENT_CONTEXT *) pContext->req_link.pNextContext;
		if (pContext->req_state == FCP_FILLING_CACHE)
		{// send iocb to ISP
			FCP_Cache_Send_CTIO (pContext, TARGET_ACTION_HANDLE_CTIO_WRITE) ;
		}
		pContext = pNextContext;
	}
	return (NU_SUCCESS);
}

/*************************************************************************/
// FCP_Get_Cache_Read_Action
// Return next action for cache read
/*************************************************************************/

U8		FCP_Get_Cache_Read_Action (U8 state, STATUS status) 
{
	U8		action;

	switch (status)
	{
		case NU_SUCCESS: /* cache hit - use this block */
			switch (state)
			{
				case SIO:
					// Begin cache read request
					action = START_CACHE_READ ;
					break ;
				case CACHE_READ:
					action = CONTINUE_CACHE_READ ;
					break ;
				case MEDIA_READ:
					action = END_MEDIA_START_CACHE_READ ;
					break ;
			}
			break ;

		case CM_ERROR_CACHE_MISS: /* cache miss block assigned */
			switch (state)
			{
				case SIO:
					// Begin media read request
					action = START_MEDIA_READ ;
					break ;
				case CACHE_READ:
					action = END_CACHE_START_MEDIA_READ ;
					break ;
				case MEDIA_READ:
					action = CONTINUE_MEDIA_READ ;
					break ;
			}
			break;

		case CM_ERROR_PAGE_LOCKED:
			// page locked for update - put in locked page container
			// when our callback is called we will continue
			switch (state)
			{
				case SIO:
					// Begin media read request
					action = WAIT_ON_CACHE_RESOURCE ;
					break ;
				case CACHE_READ:
					action = END_CACHE_WAIT_ON_RESOURCE ;
					break ;
				case MEDIA_READ:
					action = END_MEDIA_WAIT_ON_RESOURCE ;
					break ;
			}
			break ;

		case CM_ERROR_NO_PAGE_FRAMES:
			switch (state)
			{
				case SIO:
					// Begin media read request
					action = WAIT_ON_CACHE_RESOURCE ;
					break ;
				case CACHE_READ:
					action = END_CACHE_WAIT_ON_RESOURCE ;
					break ;
				case MEDIA_READ:
					action = END_MEDIA_WAIT_ON_RESOURCE ;
					break ;
			}
			break ;

		case CM_ERROR_MAX_DIRTY_PAGES:
			break ;

		default: //sw error - enter the debugger
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_Get_Cache_Read_Action", 
					"Unknown status",
					status,
					0);
			break ;
	}
	return (action) ;
}

/*************************************************************************/
// FCP_Handle_Cache_Read_Command
// 
/*************************************************************************/

STATUS	FCP_Handle_Cache_Read_Command (FCP_EVENT_CONTEXT *pContext, UNSIGNED transfer_length)
{			
 	INSTANCE_DATA 			*pId ;
	FCP_EVENT_CONTEXT		*pNextContext ;
	IOCB_STATUS_TYPE0 		*pStatusIOCB;
	IOCB_ATIO_TYPE_2		*pIOCB ;
	FcpCacheRequest			*pCacheReq, *pChead, *pCtail ;
	I64						key ;
	U32						page_size, start_LBA, cache_offset, current_offset, relative_offset;
	U32						page_size_in_sectors ;
	U32						HighKey, KeyBlock, i ;
	U32						current_byte_count, first_block_count, num_blocks, remainder, last_block_count ;
	U8						state, cache_count;
	U8						action;
	STATUS					status, cache_status ;

	pId = pContext->Id ;
	pIOCB = &pContext->iocb ;
	
	page_size = FCP_GetCachePageSize () ;  // Both caches have same page size

	/* start LBA is sector number which is 512 byte units */
	/* start Block number for cache check is in page_size units */

	start_LBA = CDB_Get_Logical_Block_Address(&pIOCB->CDB);
	page_size_in_sectors = page_size/512 ;
	
	KeyBlock = start_LBA/page_size_in_sectors ;
	cache_offset = start_LBA % page_size_in_sectors ;	
	cache_offset *= 512 ;		// number of bytes into cache block for start of transfer
	first_block_count = page_size - cache_offset ;
	current_byte_count = first_block_count ;
	num_blocks = 1 ; /* always have at least one cache block */

    pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_IN;
	pContext->req_adjust.first_last_flags = FCP_FIRST_BLOCK_BIT ;
	pContext->req_adjust.first_request_offset = cache_offset ;
	if (first_block_count >= transfer_length)
	{	/* its all in the first cache block */
		remainder = 0 ;
		first_block_count = transfer_length ;
		last_block_count = transfer_length ;
		pContext->req_adjust.first_last_flags |= FCP_LAST_BLOCK_BIT ;
	}
	else
	{	/* multiple cache blocks required */
		remainder = transfer_length - first_block_count ; 
		last_block_count = remainder % page_size ;
		num_blocks += remainder/page_size ;
		if (last_block_count)
			num_blocks++ ;
		else
			last_block_count = page_size ;
	}
	pContext->req_adjust.last_request_count = 0 ;
	pContext->req_link.pNextContext = NULL ;
	pContext->req_link.pPrevContext = NULL ;
	relative_offset = 0 ;
	pContext->req_adjust.transfer_offset = 0 ;
	current_offset = 0 ;
	cache_count = 0 ;
	pContext->req_state = FCP_FILLING_CACHE ; // assume we will get cache
// data segment count is the number of elements in the data segment descriptor list
// entry type is CTIO type 4 (0x1E) - uses ptr to SGList for transfer 
// entry count is the total # of IOCBs to complete this transaction	 (s/b 1 for writes if can get cache)
// number of pMsgs sent	s/b num_blocks (need to invalidate partial writes that are not cache hits)
	pContext->buffer = NULL ; // head of cache chain
	// Use LBA and transfer length to build SGL for write - only write what is necessary to cache
	HighKey	= FCP_Get_High_Order_Key (pContext) ;
	pChead = pCtail = NULL ;		// head and tail of cache chain for this request (either IOCB or message)
	pNextContext = pContext ;		// save head of chain

	// count # contexts for this ATIO to tell
	// when last IOCB is being sent
	pStatusIOCB = (IOCB_STATUS_TYPE0 *) pContext->pStatusIOCB;
	pStatusIOCB->entry_count = 1;

	state = SIO ;
	for (i= 0; i < num_blocks; i++)
	{
		key	=  FCP_Combine_Key (HighKey, KeyBlock) ;	
		// TODO
		// need to get flags from pContext based upon VD table - whether participates in 2ndary
		cache_status = FcpGetCacheRequest (key, READ_BUFFER_XFER, &pCacheReq) ;
		action = FCP_Get_Cache_Read_Action (state, cache_status) ;
		switch (action)
		{
			case CONTINUE_MEDIA_READ: 
			case CONTINUE_CACHE_READ:
				// Continue with same context
				// Add this cache block to chain for this context
				FcpSetCacheContext (pCacheReq, pContext);
				pCtail->next = pCacheReq ;
				pCacheReq->prev = pCtail;
				pCtail = pCacheReq ;
				// Step count of cache blocks for this context
				cache_count++;
				break ;

			case END_CACHE_START_MEDIA_READ:
				// Build IOCB for the cache read
				pContext->req_adjust.data_segment_count = cache_count ;

				FCP_Build_Cache_IOCB (pContext, relative_offset,
										page_size, cache_count, 0, ISP_CTIO_FLAGS_DATA_IN) ;	
				pContext->req_state = FCP_VALID_CACHE ;

				// Get a new context for the media read
				pContext = FCP_Get_Cache_Context(pContext);

				pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_IN;

				// used in Send_CTIO to tell when last IOCB
				// is being sent
				pStatusIOCB->entry_count++;

				relative_offset = current_offset ;
				pContext->req_adjust.transfer_offset = relative_offset ;

				// Fall thru to start media read
			case START_MEDIA_READ:
				// Starting context for media read
				FcpSetCacheContext (pCacheReq, pContext);
				state = MEDIA_READ ;
				pContext->req_state = FCP_FILLING_CACHE ;
				pContext->req_type = READ_BUFFER_XFER ;
				// first cache block for this context
				pChead = pCacheReq ;
				pCtail = pCacheReq ;
				pContext->buffer = pChead ;
				cache_count = 1;
				break ;

			case END_MEDIA_START_CACHE_READ:
				// Build message for the media read
				pContext->req_adjust.data_segment_count = cache_count ;

				FCP_Build_Cache_Message (pContext, relative_offset, page_size, cache_count) ;	

				// Get a new context for the cache read
				pContext = FCP_Get_Cache_Context(pContext);

				pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_IN;

				// used in Send_CTIO to tell when last IOCB
				// is being sent
				pStatusIOCB->entry_count++;

				relative_offset = current_offset ;
				pContext->req_adjust.transfer_offset = relative_offset ;

				// Fall thru to cache read
			case START_CACHE_READ:
				// Starting context for cache read
				FcpSetCacheContext (pCacheReq, pContext);
				state = CACHE_READ ;
				pContext->req_type = READ_BUFFER_XFER ;
				pContext->req_state = FCP_VALID_CACHE ;
				// first cache block for this context
				pChead = pCacheReq ;
				pCtail = pCacheReq ;
				pContext->buffer = pChead ;
				cache_count = 1 ;
				break;

			case WAIT_ON_CACHE_RESOURCE:
				// This Context is waiting for cache
				FcpSetCacheContext (pCacheReq, pContext);

				if (i == (num_blocks - 1))
					// last block - set flag before call to MakeWaitCacheContext
					pContext->req_adjust.first_last_flags |= FCP_LAST_BLOCK_BIT;

				MakeWaitCacheContext(pCacheReq, pContext, cache_status, last_block_count) ;
				if (!(pContext->req_adjust.first_last_flags & FCP_LAST_BLOCK_BIT))
				{	// not last block - get another context for remainder

					// Get a new context for next state
					pContext = FCP_Get_Cache_Context(pContext);

					pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_IN;

					// used in Send_CTIO to tell when last IOCB
					// is being sent
					pStatusIOCB->entry_count++;

					relative_offset = current_offset ;
					pContext->req_adjust.transfer_offset = relative_offset ;
				}
				state = SIO;
				break ;

			case END_CACHE_WAIT_ON_RESOURCE:
				// Build IOCB for the cache read
				pContext->req_adjust.data_segment_count = cache_count ;

				FCP_Build_Cache_IOCB (pContext, relative_offset,
								page_size, cache_count, 0, ISP_CTIO_FLAGS_DATA_IN) ;	
				pContext->req_state = FCP_VALID_CACHE ;

				// Get a new context for waiting on resource
				pContext = FCP_Get_Cache_Context(pContext);

				pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_IN;

				FcpSetCacheContext (pCacheReq, pContext);

				// used in Send_CTIO to tell when last IOCB
				// is being sent
				pStatusIOCB->entry_count++;

				relative_offset = current_offset ;
				pContext->req_adjust.transfer_offset = relative_offset ;

				if (i == (num_blocks - 1))
					// last block - set flag before call to MakeWaitCacheContext
					pContext->req_adjust.first_last_flags |= FCP_LAST_BLOCK_BIT;

				MakeWaitCacheContext(pCacheReq, pContext, cache_status, last_block_count) ;
				if (!(pContext->req_adjust.first_last_flags & FCP_LAST_BLOCK_BIT))
				{	// not last block - get another context for remainder

					// Get a new context for next state
					pContext = FCP_Get_Cache_Context(pContext);

					pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_IN;

					// used in Send_CTIO to tell when last IOCB
					// is being sent
					pStatusIOCB->entry_count++;

					current_offset += page_size ;	// wait context is for cache size
					relative_offset = current_offset ;
					pContext->req_adjust.transfer_offset = relative_offset ;
				}
				state = SIO;
				break ;

			case END_MEDIA_WAIT_ON_RESOURCE:
				// Build message for the media read
				pContext->req_adjust.data_segment_count = cache_count ;

				pContext->req_state = FCP_FILLING_CACHE ;
				FCP_Build_Cache_Message (pContext, relative_offset, page_size, cache_count) ;	

				// Get a new context for waiting on resource
				pContext = FCP_Get_Cache_Context(pContext);

				pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_IN;

				FcpSetCacheContext (pCacheReq, pContext);

				// used in Send_CTIO to tell when last IOCB
				// is being sent
				pStatusIOCB->entry_count++;

				relative_offset = current_offset ;
				pContext->req_adjust.transfer_offset = relative_offset ;

				if (i == (num_blocks - 1))
					// last block - set flag before call to MakeWaitCacheContext
					pContext->req_adjust.first_last_flags |= FCP_LAST_BLOCK_BIT;

				MakeWaitCacheContext(pCacheReq, pContext, cache_status, last_block_count) ;
				if (!(pContext->req_adjust.first_last_flags & FCP_LAST_BLOCK_BIT))
				{	// not last block - get another context for remainder

					// Get a new context for next state
					pContext = FCP_Get_Cache_Context(pContext);

					pContext->CTIO_flags = ISP_CTIO_FLAGS_DATA_IN;

					// used in Send_CTIO to tell when last IOCB
					// is being sent
					pStatusIOCB->entry_count++;

					current_offset += page_size ;	// wait context is for cache size
					relative_offset = current_offset ;
					pContext->req_adjust.transfer_offset = relative_offset ;
				}
				state = SIO;
				break ;
		}
		KeyBlock++ ;
		current_offset += current_byte_count ;
		current_byte_count = page_size ;
	}
	pContext->req_adjust.first_last_flags |= FCP_LAST_BLOCK_BIT ;
	pContext->req_adjust.last_request_count = last_block_count ;	

	// Build message or IOCB for last context, if necessary
	switch (action)
	{
	 	case START_CACHE_READ:
	 	case CONTINUE_CACHE_READ:
		case END_MEDIA_START_CACHE_READ:
			pContext->req_adjust.data_segment_count = cache_count ;
			FCP_Build_Cache_IOCB (pContext, relative_offset, page_size, cache_count, 0, ISP_CTIO_FLAGS_DATA_IN) ;	
  	 		pContext->req_state = FCP_VALID_CACHE ;
  	    	break ;
	 	case START_MEDIA_READ:
		case CONTINUE_MEDIA_READ: 
		case END_CACHE_START_MEDIA_READ:
			pContext->req_state = FCP_FILLING_CACHE ;
			pContext->req_adjust.data_segment_count = cache_count ;
			FCP_Build_Cache_Message (pContext, relative_offset, page_size, cache_count) ;	
			break ;

	}
	// Send all IOCBs and Messages
	pContext = pNextContext;		// Head of context chain
	while (pContext)
	{
		pNextContext = (FCP_EVENT_CONTEXT *) pContext->req_link.pNextContext;
		if (pContext->req_state == FCP_FILLING_CACHE)
		{	// Send message to next VD
		    status = FCP_Message_Send_Request(pContext,	TARGET_ACTION_HANDLE_I2O_RESPONSE);
		    if (status != NU_SUCCESS)
			{
				// We were not able to send the message.
				status = NU_SUCCESS;
			}
		}
		else if	(pContext->req_state == FCP_VALID_CACHE)
		{
			status = FCP_Cache_Send_CTIO (pContext, TARGET_ACTION_HANDLE_CTIO_FINAL);
		}
		pContext = pNextContext ;
	}
	return (NU_SUCCESS);
}

/*************************************************************************/
// FCP_Invalidate_Cache_Buffers
// Invalidate all cache objects allocated for this context
/*************************************************************************/

void	FCP_Invalidate_Cache_Buffers(FCP_EVENT_CONTEXT *pContext)
{
	FcpCacheRequest		*pReq, *pNext;

	// Cache object ptrs stored in pContext->buffer
	pReq = (FcpCacheRequest *) pContext->buffer ;
	while (pReq)
	{
		pNext = pReq->next;
		FcpTargetSetCacheInvalid(pReq);	// Invalidate this cache req object
		pReq = pNext;
	}
}


/*************************************************************************/
// FCP_Free_Cache_Buffers
// Release all cache objects allocated for this context
/*************************************************************************/

void	FCP_Free_Cache_Buffers(FCP_EVENT_CONTEXT *pContext)
{
	FcpCacheRequest		*pReq, *pNext;

	pReq = (FcpCacheRequest *) pContext->buffer ;
	while (pReq)
	{
		pNext = pReq->next ;
		if (FcpCheckIfOnWaitList(pReq) == TRUE)
		{
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
   				"FCP_Free_CacheBuffers", 
				"Request on Wait List",
				0,
				0);
		}
		pReq = pNext ;
	}
	// Cache object ptrs stored in pContext->buffer
	pReq = (FcpCacheRequest *) pContext->buffer ;
	while (pReq)
	{
		pNext = pReq->next ;
		FcpTargetReleaseCache (pReq);	// Release cache req object
		pReq = pNext ;
	}
	pContext->buffer = NULL ; // no cache connected with this any longer
}

/*************************************************************************/
// FCP_Get_Cache_Context
// Allocate and initialize a new context, chain it to the last context
/*************************************************************************/

FCP_EVENT_CONTEXT	*FCP_Get_Cache_Context(FCP_EVENT_CONTEXT *pContext)
{
 	INSTANCE_DATA 		*pId;
	FCP_EVENT_CONTEXT	*pNewContext;
	STATUS				status;

	pId = pContext->Id;
	// Allocate a new context
	status = NU_Allocate_Partition(&pId->FCP_event_context_pool, 
	    		(VOID**)&pNewContext, NU_NO_SUSPEND);
   	if (status != NU_SUCCESS)
   	{
   		// TODO
   		// If we can't allocate an event context, 
  		// we can't handle this interrupt!
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
   				"FCP_Get_Cache_Context", 
				"NU_Allocate_Partition for event context failed",
				status,
				(UNSIGNED)pId);
		return (NULL);
	}
	// Copy IOCB structure
	pNewContext->iocb = pContext->iocb ;

   	// Zero the rest of the context (assumes iocb first)
   	// Must be 8 byte aligned
	bzero64((char *)((UNSIGNED)pNewContext + IOCB_SIZE),
					sizeof(FCP_EVENT_CONTEXT) - IOCB_SIZE);


	// Set ptr to master status IOCB
	pNewContext->pStatusIOCB = pContext->pStatusIOCB;

	// Set the instance pointer
	pNewContext->Id = pId;

	pNewContext->req_type = pContext->req_type;

	// Link new context to last context
	pContext->req_link.pNextContext = pNewContext;	// set forw ptr to new context
	pNewContext->req_link.pPrevContext = pContext;	// set back ptr to old context
	pNewContext->req_link.pNextContext = NULL;		//set forw ptr to NULL (end of chain)

	return (pNewContext);
}

