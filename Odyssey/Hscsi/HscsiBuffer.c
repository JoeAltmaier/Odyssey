/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiBuffer.c
// 
// Description:
// This file implements HSCSI buffer operations such as allocating
// and deallocating. 
// 
// Update Log:
//	$Log: /Gemini/Odyssey/Hscsi/HscsiBuffer.c $ 
// 
// 1     9/14/99 7:23p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/
#include "HscsiCommon.h"
#include "HscsiData.h"
#include "HscsiBuffer.h"
#include "HscsiError.h"
#include "HscsiMemory.h"
#include "OsTypes.h"

//*************************************************************************/
// HSCSI_Buffer_Create
// Create HSCSI_Buffer object
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS HSCSI_Buffer_Create(PHSCSI_INSTANCE_DATA Id)
{
	int				 index;
	UNSIGNED 		 number_of_buffers;
	VOID			*p_memory;
	STATUS			 status = NU_SUCCESS;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Buffer_Create);
	
	Id->HSCSI_p_buffer_memory = 0;
	
	// The INITIATOR by itself does not use buffers
	if ((Id->HSCSI_config.enable_initiator_mode) &&
				!(Id->HSCSI_config.enable_target_mode))
	{
		return status;
	}
	
	// start with the configured number of buffers
	number_of_buffers = (UNSIGNED)Id->HSCSI_config.num_buffers;
	
	// get memory for the buffer queue
	p_memory = HSCSI_Alloc((tSMALL|tUNCACHED), number_of_buffers * sizeof(UNSIGNED));
	
	// Create a BufferQueue.  The BufferQueue is a list of pointers to
	// buffers.  When we need a buffer, we use NU_Receive_From_Queue to
	// get a buffer from the queue, and when we are done, we use
	// NU_Send_To_Queue to release it.  The advantage of this method over
	// using a partition is that we get our buffers aligned on buffer
	// boundaries.  A partition would add overhead to each buffer
	// and ruin the alignment.
	HSCSI_PRINT_STRING(TRACE_L5, "\n\rCreating buffer queue");
	HSCSI_PRINT_NUMBER(TRACE_L5, "\n\rnumber_of_buffers = ", number_of_buffers);
	HSCSI_PRINT_HEX(TRACE_L5, "\n\rp_memory = ", (UNSIGNED)p_memory);

	status = NU_Create_Queue(&Id->HSCSI_buffer_queue, "FcBuffer",
		p_memory,
		number_of_buffers, // number of messages to allocate
		NU_FIXED_SIZE,
		1,	// message_size: each message is a pointer 
		NU_FIFO); // how tasks suspend on the queue.	
		
	if (status != NU_SUCCESS)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Buffer_Create", 
			"NU_Create_Queue for buffer queue failed",
			status,
			(UNSIGNED)Id);
		return status;
	}
	
	// Save pointer to buffer queue.
	Id->HSCSI_p_buffer_queue = p_memory;

	// get enough memory for n+1 buffers so we can do our alignment
	// without overlap.
retry:
	p_memory = HSCSI_Alloc((tPCI|tUNCACHED), (number_of_buffers+1) * HSCSI_BUFFER_SIZE);
	
	if (p_memory == NULL)
	{
		// decrement the number of buffers until we get a pointer that is not NULL
		// or die trying
		if (--number_of_buffers == 0)
		{
			// no more - we are dead here
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_Buffer_Create", 
				"No more memory",
				0,
				(UNSIGNED)Id);
			return -1;
		}
		HSCSI_PRINT_HEX(TRACE_L2, "\n\rHSCSI Buffer reduction retry ", (UNSIGNED)number_of_buffers);
		goto retry;
	}

	// Align buffer on buffer boundary.
	p_memory = (VOID*)((char*)p_memory + HSCSI_BUFFER_SIZE);
	p_memory = (VOID*)((UNSIGNED)p_memory & HSCSI_BUFFER_MASK);
	
	// Save pointer to first buffer.
	Id->HSCSI_p_buffer_memory = p_memory;

	// Allocate buffers and put them in the BufferQueue.
	for (index = 0; index < number_of_buffers; index++)
	{
		status = NU_Send_To_Queue(&Id->HSCSI_buffer_queue,
			&p_memory, // message
			1, // size of message (one UNSIGNED)
			NU_NO_SUSPEND);
		
		if (status != NU_SUCCESS)
		{
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_Buffer_Create", 
				"NU_Send_To_Queue for buffer queue failed",
				status,
				(UNSIGNED)Id);
			return status;
		}
		
		// Step p_memory to point to next buffer.
		p_memory = (char*)p_memory + HSCSI_BUFFER_SIZE;
	}

	return status;
	
} // HSCSI_Buffer_Create

//*************************************************************************/
// HSCSI_Buffer_Destroy
// Destroy HSCSI_Buffer object
/*************************************************************************/
void HSCSI_Buffer_Destroy()
{
	// TODO
	
 	HSCSI_TRACE_ENTRY(HSCSI_Buffer_Destroy);
		
} // HSCSI_Buffer_Destroy

/*************************************************************************/
// HSCSI_Buffer_Allocate
/*************************************************************************/
STATUS HSCSI_Buffer_Allocate(PHSCSI_INSTANCE_DATA Id,
						UNSIGNED buffer_size, void **pp_buffer)
{
	UNSIGNED		 actual_size;
	VOID			*p_buffer;
	STATUS			 status;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Buffer_Allocate);
		
	// TODO allow multiple buffers when the size exceeds the maximum.
	if (buffer_size > HSCSI_BUFFER_SIZE)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Buffer_Allocate", 
			"buffer size > maximum size allocated",
			buffer_size,
			0);
		
		return HSCSI_ERROR_BUFFER_SIZE_TOO_BIG;
	}
	
	status = NU_Receive_From_Queue(&Id->HSCSI_buffer_queue,
		&p_buffer,
		1, // size of message is one UNSIGNED, or one pointer
		&actual_size,
		NU_NO_SUSPEND);
		
	if (status != NU_SUCCESS)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Buffer_Allocate", 
			"NU_Receive_From_Queue for buffer allocate failed",
			status,
			0);
		return status;
	}
		
	HSCSI_ASSERT(((UNSIGNED)p_buffer & HSCSI_BUFFER_MASK_LOW) == 0, 
		"HSCSI_Buffer_Allocate");
	
	// Return pointer to buffer allocated.	
	*pp_buffer = p_buffer;
	
	return status;
	
} // HSCSI_Buffer_Allocate

/*************************************************************************/
// HSCSI_Buffer_Deallocate
/*************************************************************************/
STATUS  HSCSI_Buffer_Deallocate(PHSCSI_INSTANCE_DATA Id, void* p_buffer)
{
	STATUS			 status;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Buffer_Deallocate);
		
	HSCSI_ASSERT(((UNSIGNED)p_buffer & HSCSI_BUFFER_MASK_LOW) == 0, 
		"HSCSI_Buffer_Allocate");
	
	status = NU_Send_To_Queue(&Id->HSCSI_buffer_queue,
		&p_buffer, // message
		1, // size of message (one UNSIGNED)
		NU_NO_SUSPEND);
		
	if (status != NU_SUCCESS)
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Buffer_Deallocate", 
			"NU_Send_To_Queue for buffer deallocate failed",
			status,
			0);
			
	return status;
	
} // HSCSI_Buffer_Deallocate


