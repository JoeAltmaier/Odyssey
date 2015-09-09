/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpEvent.c
// 
// Description:
// The FCP driver is event driven.  Each time an event occurs that
// requires an action, an event is posted in FCP_event_queue.
//
// Update Log 
// 4/14/98 Jim Frandeen: Create file
// 5/5/98 Jim Frandeen: Use C++ comment style
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 10/1/98 Michael G. Panas: changes to support init and target in same instance
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/
#include "FcpCommon.h"
#include "FcpEvent.h"
#include "FcpInitiator.h"
#include "FcpRequestFIFO.h"
#include "FcpTarget.h"
#include "FcpISR.h"
#include "FcpMemory.h"
#include "FcpLoop.h"
#include "OsTypes.h"

extern STATUS	FCP_Handle_Awaken_Cache_Context(FCP_EVENT_CONTEXT *pContext);

/*************************************************************************/
// Forward References
/*************************************************************************/
void	FCP_Event_Task(UNSIGNED argc, VOID *argv);
			
/*************************************************************************/
// FCP_Event_Create
// Create FCP_Event object
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS FCP_Event_Create(PINSTANCE_DATA Id)
{
	VOID			*p_memory;
	VOID			*p_stack;
	STATUS			 status = NU_SUCCESS;
	UNSIGNED 		 task_stack_size;

	FCP_TRACE_ENTRY(FCP_Event_Create);
	
	if (Id->FCP_config.enable_initiator_mode)
	{
		FCP_PRINT_HEX(TRACE_L8, "\n\rFCP Init event_queue_size = ",
				(UNSIGNED)Id->FCP_config.event_queue_size);
	}
	else
	{
		FCP_PRINT_HEX(TRACE_L8, "\n\rFCP Target event_queue_size = ",
				(UNSIGNED)Id->FCP_config.event_queue_size);
	}

	// get memory for the event queue
	p_memory = FCP_Alloc(tSMALL, Id->FCP_config.event_queue_size *
					sizeof(UNSIGNED *));
	
	// Create FCP_event_queue
	// This Nucleus queue contains pointers to FCP_EVENT_CONTEXT.
	// Each FCP_EVENT_CONTEXT represents work to be done.
	status = NU_Create_Queue(&Id->FCP_event_queue, "EventQ",
		p_memory, // starting address 
		Id->FCP_config.event_queue_size, // # of elements in the Q
		NU_FIXED_SIZE, // each entry is a pointer to a task
		1, // # UNSIGNED elements per entry (size of a pointer)
		NU_FIFO);
		
	if (status != NU_SUCCESS)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Event_Create", 
			"NU_Create_Queue for FCP_event_queue failed",
			status,
			0);
		return status;
	}
	
	// Save pointer to event queue memory (in case we deallocate later).
	Id->FCP_p_event_queue = p_memory;
	
	// Update p_memory for memory used for queue space.
	p_memory = (VOID*)((UNSIGNED*)p_memory + Id->FCP_config.event_queue_size);
	
	// Allocate stack space for FCP_event_task.
    // Insure UNSIGNED alignment of config stack size. 
    task_stack_size = ((Id->FCP_config.task_stack_size 
    	+ sizeof(UNSIGNED) - 1) / sizeof(UNSIGNED)) * sizeof(UNSIGNED);
//	p_stack = FCP_Alloc((tSMALL|tUNCACHED), task_stack_size);
	p_stack = FCP_Alloc((tSMALL), task_stack_size);


	// Save pointer to stack.
	Id->FCP_p_event_stack = p_stack;
	
	// Create FCP_Event_Task
	FCP_PRINT_HEX(TRACE_L8, "\n\rFCP_p_event_stack = ", (UNSIGNED)Id->FCP_p_event_stack);
	FCP_PRINT_NUMBER(TRACE_L8, "\n\rtask_stack_size = ", task_stack_size);
	
	if ((Id->FCP_config.enable_initiator_mode) &&
				(Id->FCP_config.enable_target_mode))
	{
		// both Initiator and Target mode
		status = NU_Create_Task(&Id->FCP_event_task, 
			"Event Task",			// name assigned to task
			&FCP_Event_Task,
			1,						// argc passed to task
			(VOID*)Id,				// argv passed to task
			Id->FCP_p_event_stack,	// pointer to stack area allocated above
			task_stack_size,
			Id->FCP_config.task_priority,
			0,						// disable time slicing
			NU_PREEMPT,				// task is preemptable
			NU_NO_START);			// leaves the task in a dormant state
	}
	else if (Id->FCP_config.enable_target_mode)
	{
		status = NU_Create_Task(&Id->FCP_event_task, 
			"Target Event Task",	// name assigned to task
			&FCP_Event_Task,
			1,						// argc passed to task
			(VOID*)Id,				// argv passed to task
			Id->FCP_p_event_stack,	// pointer to stack area allocated above
			task_stack_size,
			Id->FCP_config.task_priority,
			0,						// disable time slicing
			NU_PREEMPT,				// task is preemptable
			NU_NO_START);			// leaves the task in a dormant state
	}
	else if (Id->FCP_config.enable_initiator_mode)
	{
		status = NU_Create_Task(&Id->FCP_event_task, 
			"Initiator Event Task",	// name assigned to task
			&FCP_Event_Task,
			1,						// argc passed to task
			(VOID*)Id,				// argv passed to task
			Id->FCP_p_event_stack,	// pointer to stack area allocated above
			task_stack_size,
			Id->FCP_config.task_priority,
			0,						// disable time slicing
			NU_PREEMPT,				// task is preemptable
			NU_NO_START);			// leaves the task in a dormant state
	}
	else
	{
		// must have at least one mode
		status = FCP_ERROR_EVENT_SETUP_FAILED;
	}
	
	if (status != NU_SUCCESS)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Event_Create", 
			"NU_CreateTask for FCP_Event_Task failed",
			status,
			(UNSIGNED)Id);
		return status;
	}
	
	// get memory for the event context pool, must be uncached for DMA
//	p_memory = FCP_Alloc((tSMALL|tUNCACHED|tZERO), (Id->FCP_config.event_queue_size +1) *
//					sizeof(FCP_EVENT_CONTEXT));
	p_memory = FCP_Alloc((tSMALL|tZERO), (Id->FCP_config.event_queue_size +1) *
					sizeof(FCP_EVENT_CONTEXT));

	// Align the start address on a 64 byte boundary so we will not have
	// problems later.
	p_memory = (VOID*)((UNSIGNED)p_memory + (64 - ((UNSIGNED)p_memory & (64-1))));
	
	FCP_PRINT_HEX(TRACE_L8, "\n\rsizeof FCP_EVENT_CONTEXT = ", (UNSIGNED)sizeof(FCP_EVENT_CONTEXT));
	FCP_PRINT_HEX(TRACE_L8, "\n\rFCP_event_context_pool start = ", (UNSIGNED)p_memory);
	
	// Allocate FCP_event_context_pool
	// This Nucleus partition contains contexts that can be allocated and 
	// queued onto the FCP_event_queue.
	status = NU_Create_Partition_Pool(&Id->FCP_event_context_pool, "ContPool", 
    	p_memory, // start_address
        sizeof(FCP_EVENT_CONTEXT) * Id->FCP_config.event_queue_size, // pool_size,
        sizeof(FCP_EVENT_CONTEXT), // partition_size
        NU_FIFO); // OPTION suspend_type
        
	if (status != NU_SUCCESS)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Event_Create", 
			"NU_Create_Partition_Pool failed",
			status,
			(UNSIGNED)Id);
		return status;
	}

	// Save pointer to event context pool memory (in case we deallocate later).
	Id->FCP_p_event_context_pool = p_memory;
	
	return NU_SUCCESS;
	
} // FCP_Event_Create

/*************************************************************************/
// FCP_Event_Destroy
// Destroy FCP_Target object
/*************************************************************************/
void FCP_Event_Destroy()
{
	FCP_TRACE_ENTRY(FCP_Event_Destroy);
	
	// TODO	
} // FCP_Event_Destroy


/*************************************************************************/
// FCP_Event_Task
// This Nucleus task waits for FCP_EVENT_CONTEXT to be queued on 
// FCP_event_queue.  Each FCP_EVENT_CONTEXT represents work to do.  
// Each FCP_EVENT_CONTEXT is
// handled, and the FCP_EVENT_CONTEXT is returned to the free pool.
/*************************************************************************/
void FCP_Event_Task(UNSIGNED argc, VOID *argv)
{
	UNSIGNED			 actual_size;
	FCP_EVENT_CONTEXT	*p_context;
	STATUS				 status;
	PINSTANCE_DATA		 Id;
	
 	FCP_TRACE_ENTRY(FCP_Event_Task);
	
	// recover our instance pointer
	Id = (PINSTANCE_DATA)argv;
	
#if 1
	// Initialize the ISP.
	status = FCP_ISP_Init(Id);
	if (status != NU_SUCCESS)
	{				
		// TODO 
		return;
	}
#endif

	// Set p_context non zero to start the while loop.
	p_context = (FCP_EVENT_CONTEXT*)1;
	
	// Continue until we receive a zero message.  A zero message 
	// is our signal to finish when a Quiet request is received.
	while (p_context)
	{
 		FCP_PRINT_HEX(TRACE_L8, "\n\rWaiting for event wakeup, Id = ", Id->FCP_instance);
 		
		// Wait for the next event.  
		status = NU_Receive_From_Queue(&Id->FCP_event_queue,
			&p_context,	// Copy message to this pointer.
			1,			// one UNSIGNED data element in the message
			&actual_size, // receives actual size of message 
			NU_SUSPEND);
			
 		FCP_PRINT_HEX(TRACE_L8, "\n\rFCP event wakeup, Instance = ", Id->FCP_instance);
 		FCP_PRINT_HEX(TRACE_L8, "  p_context = ", (UNSIGNED)p_context);
 		FCP_PRINT_HEX(TRACE_L8, "\n\rId ptr = ", (UNSIGNED)Id);
 		FCP_PRINT_NUMBER(TRACE_L8, "  action = ", (UNSIGNED)p_context->action);
 		
		if (status != NU_SUCCESS)
		{
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_Event_Task", 
				"NU_Receive_From_Queue failed",
				status,
				(UNSIGNED)Id);
				
			// TODO If we can't receive a message, this task dies.
			return;
		}
		
//		FCP_ASSERT(actual_size == 1, "FCP_Event_Task");
		if (actual_size != 1)
		{
			FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
				"FCP_Event_Task", 
				"NU_Receive_From_Queue actual size != 1",
				actual_size,
				(UNSIGNED)&p_context);
				
				FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
				"FCP_Event_Task", 
				"NU_Receive_From_Queue RX_ID =",
				p_context->iocb.RX_ID,
				0);
				
		}
		
		FCP_ASSERT(Id == p_context->Id, "FCP_Event_Task");

		// Switch depending on the state of the context.
		switch (p_context->action)
		{
			case TARGET_ACTION_HANDLE_NEW_IOCB:
				
				// We received a new ATIO IOCB.
				// This IOCB gets placed in the
				// queue by FCP_ISR_Response_Queue when it receives
				// an ATIO IOCB from the ISP response FIFO.
				status = FCP_Handle_Accept_Target_IO(p_context);
				break;
				
			case TARGET_ACTION_HANDLE_I2O_RESPONSE:
				
				// We received a response to our I2O request.
				status = FCP_Handle_I2O_Response(p_context);
				break;
				
			case TARGET_ACTION_HANDLE_CTIO_WRITE:
				
				// We received a response to the CTIO for a write.
				status = FCP_Handle_CTIO_Write(p_context);
				break;
				
			case TARGET_ACTION_HANDLE_CTIO_FINAL:
				
				// We received a response to the final CTIO	
				// After the final status was sent.
				status = FCP_Handle_CTIO_Final(p_context);
				break;
				
			case TARGET_ACTION_HANDLE_CACHE_AVAILABLE:
				status = FCP_Handle_Awaken_Cache_Context(p_context) ;
				break;

			case INITIATOR_ACTION_HANDLE_SCSI_REQUEST:
				
				// We received a SCSI request to be
				// handled by the initiator.
				status = FCP_Handle_SCSI_Request(p_context);
				break;
				
			case FCP_ACTION_HANDLE_LOOP_EVENT:
				
				// Handle Loop Events
				status = FCP_Handle_Loop_Event(p_context);
				break;
				
			case FCP_ACTION_HANDLE_LOOP_IOCB_COMPL:
				
				// We received a response to a command
				// IOCB that was sent to the ISP by the LoopHandler.
				status = FCP_Handle_Loop_IOCB_Completion(p_context);
				break;
				
			case INITIATOR_ACTION_HANDLE_COMMAND_RESPONSE:
				
				// We received a response to a command
				// IOCB that was sent to the ISP.
				status = FCP_Handle_Command_Response(p_context);
				break;
				
			case FCP_ACTION_HANDLE_IMMEDIATE_NOTIFY:
				
				// We received an Immediate Notify IOCB
				status = FCP_Handle_Immediate_Notify(p_context);
				break;
				
			case FCP_ACTION_HANDLE_LOOP_UP:
			case FCP_ACTION_HANDLE_LOOP_DOWN:
			case FCP_ACTION_HANDLE_LIP:
			case FCP_ACTION_HANDLE_LIP_RESET:
			case FCP_ACTION_HANDLE_OTHER_AE:
			case FCP_ACTION_HANDLE_THROW_EVENT:
				
				// We received an asynchronous event, pass it
				// on to the handler in the LoopMonitor
				status = LM_Handle_FC_AE(p_context);
				break;
				
			default:
		
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_Event_Task", 
					"Invalid context state",
					(UNSIGNED)p_context->action,
					(UNSIGNED)p_context);
		
		} // switch
		
	} // while
	
	// Continue here when we receive a zero message.	
	// Task passes to the finished state.  
	// It can be restarted.		
	return;
	
} // FCP_Event_Task

