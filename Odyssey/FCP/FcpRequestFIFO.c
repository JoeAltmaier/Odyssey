/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpRequestFIFO.c
// 
// Description:
// This file implements interfaces to the ISP request FIFO.
// 
// Update Log 
// 5/18/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/

#include "FcpCommon.h"
#include "FcpData.h"
#include "FcpRequestFIFO.h"
#include "FcpString.h"
#include "FcpMemory.h"
#include "OsTypes.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
void*	FCP_Request_FIFO_Get_Pointer();
void	FCP_Request_FIFO_Update_Index();

/*************************************************************************/
// FCP_Request_FIFO_Create
// Create FCP_Request_FIFO object.
/*************************************************************************/
STATUS FCP_Request_FIFO_Create(PINSTANCE_DATA Id)
{
	VOID			*p_memory;
	STATUS			 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Request_FIFO_Create);
			
	// Initialize index into request FIFO
	Id->FCP_request_FIFO_index = 0;
	
	// Allocate IOCB_request_FIFO
	// This FIFO of IOCBs is used to send requests to the ISP.  
	// The address of the IOCB request_queue is passed to the ISP. 
	
	// get memory for the IOCB Request FIFO
	p_memory = FCP_Alloc((tSMALL|tUNCACHED),
					(Id->FCP_config.ISP_FIFO_request_size + 1) *
					IOCB_SIZE);

    // Insure 64 byte boundary alignment of FIFO. 
    p_memory = (VOID*)((char*)p_memory + IOCB_SIZE - 1);   	
    p_memory = (VOID*)(((UNSIGNED)p_memory >> IOCB_SHIFT) << IOCB_SHIFT);
    
    // Save pointer to request_FIFO
	Id->FCP_p_IOCB_request_FIFO = (IOCB_STATUS_TYPE0*)p_memory;
	
	return status;
	
} // FCP_Request_FIFO_Create

/*************************************************************************/
// FCP_Request_FIFO_Destroy
// Initialize FCP_Request_FIFO object.
/*************************************************************************/
void FCP_Request_FIFO_Destroy()
{
 	FCP_TRACE_ENTRY(FCP_Request_FIFO_Destroy);
			
} // FCP_Request_FIFO_Destroy

//*************************************************************************/
// FCP_Request_FIFO_Get_Pointer
// Return pointer to next request FIFO entry.
/*************************************************************************/
void *FCP_Request_FIFO_Get_Pointer(PINSTANCE_DATA Id)
{
 	IOCB_STATUS_TYPE0	*p_IOCB;
 	U16		 			 request_queue_in_pointer;
	U16		 			 request_queue_out_pointer;
 	
 	FCP_TRACE_ENTRY(FCP_Request_FIFO_Get_Pointer);
	
	// Get pointer to next request FIFO entry
	p_IOCB = &Id->FCP_p_IOCB_request_FIFO[Id->FCP_request_FIFO_index];
	
	// Increment the request_queue_in_pointer
	// This is the index of the next FIFO empty request
	request_queue_in_pointer = Id->FCP_request_FIFO_index + 1;
	
	// Check for wrap around
	if (request_queue_in_pointer == Id->FCP_config.ISP_FIFO_request_size)
		request_queue_in_pointer = 0;
		
	// Read the request_queue_out_pointer.  
	// This tells us where the ISP is in the queue.
	request_queue_out_pointer = Read_ISP(Id, ISP_MAILBOX_4);
	
	// Check to see if the queue is full.  
	if (request_queue_out_pointer == request_queue_in_pointer)
	{
		// The queue is full.  This should never happen.
		// There are more FIFO entries than tasks.
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Request_FIFO_Get_Pointer", 
			"Request FIFO is full",
			0,
			(UNSIGNED)Id);
	}
		
	// Update the index of the next empty request.
	// We will send this to the ISP when we have finished building the request.
	Id->FCP_request_FIFO_index = request_queue_in_pointer;
	
	// Zero the next IOCB
	// Must be 8 byte aligned
	bzero64((void*)p_IOCB, IOCB_SIZE);
	
	// Return pointer to next request FIFO entry
	return (void*)p_IOCB;
	
} // FCP_Request_FIFO_Get_Pointer

//*************************************************************************/
// FCP_Request_FIFO_Update_Index.
// Update the Request_FIFO_Index in mailbox 4.  
// This lets the ISP know that there is a new command in the FIFO.
//*************************************************************************/
void FCP_Request_FIFO_Update_Index(PINSTANCE_DATA Id)
{
 	
 	FCP_TRACE_ENTRY(FCP_Request_FIFO_Update_Index);
			
    // update request queue pointer in mailbox 4 
    Write_ISP(Id, ISP_MAILBOX_4, Id->FCP_request_FIFO_index);
    
} // FCP_Request_FIFO_Update_Index


