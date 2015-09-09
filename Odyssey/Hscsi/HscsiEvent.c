/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiEvent.c
// 
// Description:
// The HSCSI driver is event driven.  Each time an event occurs that
// requires an action, an event is posted in HSCSI_event_queue.
//
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiEvent.c $
// 
// 1     9/14/99 7:23p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/
#include "HscsiCommon.h"
#include "HscsiEvent.h"
#include "HscsiInitiator.h"
#include "HscsiRequestFIFO.h"
#include "HscsiTarget.h"
#include "HscsiMemory.h"
#include "OsTypes.h"


/*************************************************************************/
// Forward References
/*************************************************************************/
void	HSCSI_Event_Task(UNSIGNED argc, VOID *argv);
			
/*************************************************************************/
// HSCSI_Event_Create
// Create HSCSI_Event object
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS HSCSI_Event_Create(PHSCSI_INSTANCE_DATA Id)
{
	VOID			*p_memory;
	VOID			*p_stack;
	STATUS			 status = NU_SUCCESS;
	UNSIGNED 		 task_stack_size;

	HSCSI_TRACE_ENTRY(HSCSI_Event_Create);
	
	if (Id->HSCSI_config.enable_initiator_mode)
	{
		HSCSI_PRINT_HEX(TRACE_L8, "\n\rHSCSI Init event_queue_size = ",
				(UNSIGNED)Id->HSCSI_config.event_queue_size);
	}
	else
	{
		HSCSI_PRINT_HEX(TRACE_L8, "\n\rHSCSI Target event_queue_size = ",
				(UNSIGNED)Id->HSCSI_config.event_queue_size);
	}

	// get memory for the event queue
	p_memory = HSCSI_Alloc(tSMALL, Id->HSCSI_config.event_queue_size *
					sizeof(UNSIGNED *));

	// Create HSCSI_event_queue
	// This Nucleus queue contains pointers to HSCSI_EVENT_CONTEXT.
	// Each HSCSI_EVENT_CONTEXT represents work to be done.
	status = NU_Create_Queue(&Id->HSCSI_event_queue, "EventQ",
		p_memory, // starting address 
		Id->HSCSI_config.event_queue_size, // # of elements in the Q
		NU_FIXED_SIZE, // each entry is a pointer to a task
		1, // # UNSIGNED elements per entry (size of a pointer)
		NU_FIFO);
		
	if (status != NU_SUCCESS)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Event_Create", 
			"NU_Create_Queue for HSCSI_event_queue failed",
			status,
			0);
		return status;
	}
	
	// Save pointer to event queue memory (in case we deallocate later).
	Id->HSCSI_p_event_queue = p_memory;
	
	// Update p_memory for memory used for queue space.
	p_memory = (VOID*)((UNSIGNED*)p_memory + Id->HSCSI_config.event_queue_size);
	
	// Allocate stack space for HSCSI_event_task.
    // Insure UNSIGNED alignment of config stack size. 
    task_stack_size = ((Id->HSCSI_config.task_stack_size 
    	+ sizeof(UNSIGNED) - 1) / sizeof(UNSIGNED)) * sizeof(UNSIGNED);
	p_stack = HSCSI_Alloc((tSMALL|tUNCACHED), task_stack_size);

	// Save pointer to stack.
	Id->HSCSI_p_event_stack = p_stack;
	
	// Create HSCSI_Event_Task
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rHSCSI_p_event_stack = ", (UNSIGNED)Id->HSCSI_p_event_stack);
	HSCSI_PRINT_NUMBER(TRACE_L8, "\n\rtask_stack_size = ", task_stack_size);
	
	if ((Id->HSCSI_config.enable_initiator_mode) &&
				(Id->HSCSI_config.enable_target_mode))
	{
		// both Initiator and Target mode
		status = NU_Create_Task(&Id->HSCSI_event_task, 
			"Event Task",			// name assigned to task
			&HSCSI_Event_Task,
			1,						// argc passed to task
			(VOID*)Id,				// argv passed to task
			Id->HSCSI_p_event_stack,	// pointer to stack area allocated above
			task_stack_size,
			Id->HSCSI_config.task_priority,
			0,						// disable time slicing
			NU_PREEMPT,				// task is preemptable
			NU_NO_START);			// leaves the task in a dormant state
	}
	else if (Id->HSCSI_config.enable_target_mode)
	{
		status = NU_Create_Task(&Id->HSCSI_event_task, 
			"Target Event Task",	// name assigned to task
			&HSCSI_Event_Task,
			1,						// argc passed to task
			(VOID*)Id,				// argv passed to task
			Id->HSCSI_p_event_stack,	// pointer to stack area allocated above
			task_stack_size,
			Id->HSCSI_config.task_priority,
			0,						// disable time slicing
			NU_PREEMPT,				// task is preemptable
			NU_NO_START);			// leaves the task in a dormant state
	}
	else if (Id->HSCSI_config.enable_initiator_mode)
	{
		status = NU_Create_Task(&Id->HSCSI_event_task, 
			"Initiator Event Task",	// name assigned to task
			&HSCSI_Event_Task,
			1,						// argc passed to task
			(VOID*)Id,				// argv passed to task
			Id->HSCSI_p_event_stack,	// pointer to stack area allocated above
			task_stack_size,
			Id->HSCSI_config.task_priority,
			0,						// disable time slicing
			NU_PREEMPT,				// task is preemptable
			NU_NO_START);			// leaves the task in a dormant state
	}
	else
	{
		// must have at least one mode
		status = HSCSI_ERROR_EVENT_SETUP_FAILED;
	}
	
	if (status != NU_SUCCESS)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Event_Create", 
			"NU_CreateTask for HSCSI_Event_Task failed",
			status,
			(UNSIGNED)Id);
		return status;
	}
	
	// get memory for the event context pool, must be uncached for DMA
	p_memory = HSCSI_Alloc((tSMALL|tUNCACHED), (Id->HSCSI_config.event_queue_size +1) *
					sizeof(HSCSI_EVENT_CONTEXT));

	// Align the start address on a 64 byte boundary so we will not have
	// problems later.
	p_memory = (VOID*)((UNSIGNED)p_memory + (64 - ((UNSIGNED)p_memory & (64-1))));
	
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rsizeof HSCSI_EVENT_CONTEXT = ", (UNSIGNED)sizeof(HSCSI_EVENT_CONTEXT));
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rHSCSI_event_context_pool start = ", (UNSIGNED)p_memory);
	
	// Allocate HSCSI_event_context_pool
	// This Nucleus partition contains contexts that can be allocated and 
	// queued onto the HSCSI_event_queue.
	status = NU_Create_Partition_Pool(&Id->HSCSI_event_context_pool, "ContPool", 
    	p_memory, // start_address
        sizeof(HSCSI_EVENT_CONTEXT) * Id->HSCSI_config.event_queue_size, // pool_size,
        sizeof(HSCSI_EVENT_CONTEXT), // partition_size
        NU_FIFO); // OPTION suspend_type
        
	if (status != NU_SUCCESS)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Event_Create", 
			"NU_Create_Partition_Pool failed",
			status,
			(UNSIGNED)Id);
		return status;
	}

	// Save pointer to event context pool memory (in case we deallocate later).
	Id->HSCSI_p_event_context_pool = p_memory;
	
	return NU_SUCCESS;
	
} // HSCSI_Event_Create

/*************************************************************************/
// HSCSI_Event_Destroy
// Destroy HSCSI_Target object
/*************************************************************************/
void HSCSI_Event_Destroy()
{
	HSCSI_TRACE_ENTRY(HSCSI_Event_Destroy);
	
	// TODO	
} // HSCSI_Event_Destroy


/*************************************************************************/
// HSCSI_Event_Task
// This Nucleus task waits for HSCSI_EVENT_CONTEXT to be queued on 
// HSCSI_event_queue.  Each HSCSI_EVENT_CONTEXT represents work to do.  
// Each HSCSI_EVENT_CONTEXT is
// handled, and the HSCSI_EVENT_CONTEXT is returned to the free pool.
/*************************************************************************/
void HSCSI_Event_Task(UNSIGNED argc, VOID *argv)
{
	UNSIGNED			 actual_size;
	HSCSI_EVENT_CONTEXT	*p_context;
	STATUS				 status;
	PHSCSI_INSTANCE_DATA		 Id;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Event_Task);
	
	// recover our instance pointer
	Id = (PHSCSI_INSTANCE_DATA)argv;
	
#if 1
	// Initialize the ISP.
	status = HSCSI_ISP_Init(Id);
	if (status != NU_SUCCESS)
	{				
		// TODO 
		return;
	}
#endif

	// Set p_context non zero to start the while loop.
	p_context = (HSCSI_EVENT_CONTEXT*)1;
	
	// Continue until we receive a zero message.  A zero message 
	// is our signal to finish when a Quiet request is received.
	while (p_context)
	{
 		HSCSI_PRINT_HEX(TRACE_L8, "\n\rWaiting for event wakeup, Id = ", Id->HSCSI_instance);
 		
		// Wait for the next event.  
		status = NU_Receive_From_Queue(&Id->HSCSI_event_queue,
			&p_context,	// Copy message to this pointer.
			1,			// one UNSIGNED data element in the message
			&actual_size, // receives actual size of message 
			NU_SUSPEND);
			
 		HSCSI_PRINT_HEX(TRACE_L8, "\n\rHSCSI event wakeup, Instance = ", Id->HSCSI_instance);
 		HSCSI_PRINT_HEX(TRACE_L8, "  p_context = ", (UNSIGNED)p_context);
 		HSCSI_PRINT_HEX(TRACE_L8, "\n\rId ptr = ", (UNSIGNED)Id);
 		HSCSI_PRINT_NUMBER(TRACE_L8, "  action = ", (UNSIGNED)p_context->action);
 		
		if (status != NU_SUCCESS)
		{
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_Event_Task", 
				"NU_Receive_From_Queue failed",
				status,
				(UNSIGNED)Id);
				
			// TODO If we can't receive a message, this task dies.
			return;
		}
		
		HSCSI_ASSERT(actual_size == 1, "HSCSI_Event_Task");
		HSCSI_ASSERT(Id == p_context->Id, "HSCSI_Event_Task");

		// Switch depending on the state of the context
		// Initiator-only does not support CTIO/ATIO
		switch (p_context->action)
		{
			case TARGET_HANDLE_NEW_IOCB:
				
				// We received a new ATIO IOCB.
				// This IOCB gets placed in the
				// queue by HSCSI_ISR_Response_Queue when it receives
				// an ATIO IOCB from the ISP response FIFO.
//				status = HSCSI_Handle_Accept_Target_IO(p_context);
				break;
				
			case TARGET_HANDLE_I2O_RESPONSE:
				
				// We received a response to our I2O request.
//				status = HSCSI_Handle_I2O_Response(p_context);
				break;
				
			case TARGET_HANDLE_CTIO_WRITE:
				
				// We received a response to the CTIO for a write.
//				status = HSCSI_Handle_CTIO_Write(p_context);
				break;
				
			case TARGET_HANDLE_CTIO_FINAL:
				
				// We received a response to the final CTIO	
				// After the final status was sent.
//				status = HSCSI_Handle_CTIO_Final(p_context);
				break;
				
			case INITIATOR_HANDLE_SCSI_REQUEST:
				
				// We received a SCSI request to be
				// handled by the initiator.
				status = HSCSI_Handle_SCSI_Request(p_context);
				break;
				
			case INITIATOR_HANDLE_COMMAND_RESPONSE:
				
				// We received a response to a command
				// IOCB that was sent to the ISP.
				status = HSCSI_Handle_Command_Response(p_context);
				break;
				
			case HSCSI_ACTION_HANDLE_OTHER_AE:
			case HSCSI_ACTION_HANDLE_THROW_EVENT:
				
				// We received an asynchronous event, pass it
				// on to the correct handler (target or initiator)
				switch (p_context->Id->HSCSI_config.config_instance)
				{
					case TARGET_INSTANCE:
					case TRGT_ONLY_INSTANCE:
						break;
					case INITIATOR_INSTANCE:
						status = HSCSI_Handle_AE_Initiator(p_context);
						break;
					case BOTH_INSTANCE:
					case INIT_ONLY_INSTANCE:
						status = HSCSI_Handle_AE_Initiator(p_context);
						break;
				}
				
				break;
				
			default:
		
				HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
					"HSCSI_Event_Task", 
					"Invalid context state",
					(UNSIGNED)p_context->action,
					(UNSIGNED)p_context);
		
		} // switch
		
	} // while
	
	// Continue here when we receive a zero message.	
	// Task passes to the finished state.  
	// It can be restarted.		
	return;
	
} // HSCSI_Event_Task

