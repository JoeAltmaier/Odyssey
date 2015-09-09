/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiRequestFIFO.c
// 
// Description:
// This file implements interfaces to the ISP request FIFO.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiRequestFIFO.c $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#include "HscsiCommon.h"
#include "HscsiData.h"
#include "HscsiRequestFIFO.h"
#include "HscsiString.h"
#include "HscsiMemory.h"
#include "OsTypes.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
void*	HSCSI_Request_FIFO_Get_Pointer();
void	HSCSI_Request_FIFO_Update_Index();

/*************************************************************************/
// HSCSI_Request_FIFO_Create
// Create HSCSI_Request_FIFO object.
/*************************************************************************/
STATUS HSCSI_Request_FIFO_Create(PHSCSI_INSTANCE_DATA Id)
{
	VOID			*p_memory;
	STATUS			 status = NU_SUCCESS;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Request_FIFO_Create);
			
	// Initialize index into request FIFO
	Id->HSCSI_request_FIFO_index = 0;
	
	// Allocate IOCB_request_FIFO
	// This FIFO of IOCBs is used to send requests to the ISP.  
	// The address of the IOCB request_queue is passed to the ISP. 
	
	// get memory for the IOCB Request FIFO
	p_memory = HSCSI_Alloc((tSMALL|tUNCACHED),
					(Id->HSCSI_config.ISP_FIFO_request_size + 1) *
					IOCB_SIZE);

    // Insure 64 byte boundary alignment of FIFO. 
    p_memory = (VOID*)((char*)p_memory + IOCB_SIZE - 1);   	
    p_memory = (VOID*)(((UNSIGNED)p_memory >> IOCB_SHIFT) << IOCB_SHIFT);
    
    // Save pointer to request_FIFO
	Id->HSCSI_p_IOCB_request_FIFO = (IOCB_STATUS_ENTRY*)p_memory;
	
	return status;
	
} // HSCSI_Request_FIFO_Create

/*************************************************************************/
// HSCSI_Request_FIFO_Destroy
// Initialize HSCSI_Request_FIFO object.
/*************************************************************************/
void HSCSI_Request_FIFO_Destroy()
{
 	HSCSI_TRACE_ENTRY(HSCSI_Request_FIFO_Destroy);
			
} // HSCSI_Request_FIFO_Destroy

//*************************************************************************/
// HSCSI_Request_FIFO_Get_Pointer
// Return pointer to next request FIFO entry.
/*************************************************************************/
void *HSCSI_Request_FIFO_Get_Pointer(PHSCSI_INSTANCE_DATA Id)
{
 	IOCB_STATUS_ENTRY	*p_IOCB;
 	U16		 			 request_queue_in_pointer;
	U16		 			 request_queue_out_pointer;
 	
 	HSCSI_TRACE_ENTRY(HSCSI_Request_FIFO_Get_Pointer);
	
	// Get pointer to next request FIFO entry
	p_IOCB = &Id->HSCSI_p_IOCB_request_FIFO[Id->HSCSI_request_FIFO_index];
	
	// Increment the request_queue_in_pointer
	// This is the index of the next FIFO empty request
	request_queue_in_pointer = Id->HSCSI_request_FIFO_index + 1;
	
	// Check for wrap around
	if (request_queue_in_pointer == Id->HSCSI_config.ISP_FIFO_request_size)
		request_queue_in_pointer = 0;
		
	// Read the request_queue_out_pointer.  
	// This tells us where the ISP is in the queue.
	request_queue_out_pointer = Read_ISP1040(Id, HSCSI_MAILBOX_4);
	
	// Check to see if the queue is full.  
	if (request_queue_out_pointer == request_queue_in_pointer)
	{
		// The queue is full.  This should never happen.
		// There are more FIFO entries than tasks.
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Request_FIFO_Get_Pointer", 
			"Request FIFO is full",
			0,
			(UNSIGNED)Id);
	}
		
	// Update the index of the next empty request.
	// We will send this to the ISP when we have finished building the request.
	Id->HSCSI_request_FIFO_index = request_queue_in_pointer;
	
	// Zero the next IOCB
	Mem_Set((void*)p_IOCB, 0, IOCB_SIZE);
	
	// Return pointer to next request FIFO entry
	return (void*)p_IOCB;
	
} // HSCSI_Request_FIFO_Get_Pointer

//*************************************************************************/
// HSCSI_Request_FIFO_Update_Index.
// Update the Request_FIFO_Index in mailbox 4.  
// This lets the ISP know that there is a new command in the FIFO.
//*************************************************************************/
void HSCSI_Request_FIFO_Update_Index(PHSCSI_INSTANCE_DATA Id)
{
 	
 	HSCSI_TRACE_ENTRY(HSCSI_Request_FIFO_Update_Index);
			
    // update request queue pointer in mailbox 4
    
    Write_ISP1040(Id, HSCSI_MAILBOX_4, Id->HSCSI_request_FIFO_index);
    
} // HSCSI_Request_FIFO_Update_Index


