/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpBuffer.c
// 
// Description:
// This file implements FCP buffer operations such as allocating
// and deallocating. 
// 
// Update Log: 
// 5/15/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 10/1/98 Michael G. Panas: changes to support init and target in same instance
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/
#include "FcpCommon.h"
#include "FcpData.h"
#include "FcpBuffer.h"
#include "FcpError.h"
#include "FcpMemory.h"
#include "OsTypes.h"

STATUS	InitializeCache (void) ;

//*************************************************************************/
// FCP_Buffer_Create
// Create FCP_Buffer object
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS FCP_Buffer_Create(PINSTANCE_DATA Id)
{
	STATUS			 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Buffer_Create);
	
	Id->FCP_p_buffer_memory = 0;
	
	// The INITIATOR by itself does not use buffers
	if ((Id->FCP_config.enable_initiator_mode) &&
				!(Id->FCP_config.enable_target_mode))
	{
		return status;
	}
	
	status = InitializeCache() ;		

	return status;
	
} // FCP_Buffer_Create

//*************************************************************************/
// FCP_Buffer_Destroy
// Destroy FCP_Buffer object
/*************************************************************************/
void FCP_Buffer_Destroy()
{
	// TODO
	
 	FCP_TRACE_ENTRY(FCP_Buffer_Destroy);
		
} // FCP_Buffer_Destroy

/*************************************************************************/
// FCP_Buffer_Allocate
/*************************************************************************/
STATUS FCP_Buffer_Allocate(PINSTANCE_DATA Id,
						UNSIGNED buffer_size, void **pp_buffer)
{
	UNSIGNED		 actual_size;
	VOID			*p_buffer;
	STATUS			 status;
	
 	FCP_TRACE_ENTRY(FCP_Buffer_Allocate);
		
	// TODO allow multiple buffers when the size exceeds the maximum.
	if (buffer_size > FCP_BUFFER_SIZE)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Buffer_Allocate", 
			"buffer size > maximum size allocated",
			buffer_size,
			0);
		
		return FCP_ERROR_BUFFER_SIZE_TOO_BIG;
	}
	
	status = NU_Receive_From_Queue(&Id->FCP_buffer_queue,
		&p_buffer,
		1, // size of message is one UNSIGNED, or one pointer
		&actual_size,
		NU_NO_SUSPEND);
		
	if (status != NU_SUCCESS)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Buffer_Allocate", 
			"NU_Receive_From_Queue for buffer allocate failed",
			status,
			0);
		return status;
	}
		
	FCP_ASSERT(((UNSIGNED)p_buffer & FCP_BUFFER_MASK_LOW) == 0, 
		"FCP_Buffer_Allocate");
	
	// Return pointer to buffer allocated.	
	*pp_buffer = p_buffer;
	
	return status;
	
} // FCP_Buffer_Allocate

/*************************************************************************/
// FCP_Buffer_Deallocate
/*************************************************************************/
STATUS  FCP_Buffer_Deallocate(PINSTANCE_DATA Id, void* p_buffer)
{
	STATUS			 status;
	
 	FCP_TRACE_ENTRY(FCP_Buffer_Deallocate);
		
	FCP_ASSERT(((UNSIGNED)p_buffer & FCP_BUFFER_MASK_LOW) == 0, 
		"FCP_Buffer_Allocate");
	
	status = NU_Send_To_Queue(&Id->FCP_buffer_queue,
		&p_buffer, // message
		1, // size of message (one UNSIGNED)
		NU_NO_SUSPEND);
		
	if (status != NU_SUCCESS)
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Buffer_Deallocate", 
			"NU_Send_To_Queue for buffer deallocate failed",
			status,
			0);
			
	return status;
	
} // FCP_Buffer_Deallocate


