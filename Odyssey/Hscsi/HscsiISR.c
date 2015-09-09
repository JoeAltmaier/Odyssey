/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiISR.c
// 
// Description:
// This file implements interrupt handling for the ISP1040B. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiISR.c $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/
#include "HscsiCommon.h"
#include "HscsiData.h"
#include "HscsiMessage.h"
#include "HscsiISR.h"
#include "HscsiISP.h"
#include "HscsiResponseFIFO.h"
#include "HscsiRequestFIFO.h"
#include "HscsiString.h"
#include "HscsiMemory.h"
#include "OsTypes.h"
#include "RequestCodes.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
void		HSCSI_ISR_High_Target();
void		HSCSI_ISR_High_Initiator();
void		HSCSI_ISR_Low(INT vector_number);
void		HSCSI_ISR_High_Both();
void		HSCSI_ISR_Low_Both(INT vector_number);
void		HSCSI_ISR_Mailbox(PHSCSI_INSTANCE_DATA Id);
void		HSCSI_ISR_Response_Queue(PHSCSI_INSTANCE_DATA Id);

//STATUS		HSCSI_Handle_Immediate_Notify(PHSCSI_INSTANCE_DATA Id, IOCB_IMMEDIATE_NOTIFY *p_IOCB);

/*************************************************************************/
// HSCSI_ISR_Create
// Create HSCSI_ISR object.
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS HSCSI_ISR_Create(PHSCSI_INSTANCE_DATA Id)
{
	VOID			(*old_lisr)(INT); // pointer to old LISR
	UNSIGNED		 stack_size;
	STATUS			 status = HSCSI_ERROR_INTERRUPT_SETUP_FAILED;
	
 	HSCSI_TRACE_ENTRY(HSCSI_ISR_Create);
	
    // Insure UNSIGNED alignment of config stack size. 
    stack_size = ((Id->HSCSI_config.HISR_stack_size 
    	+ sizeof(UNSIGNED) - 1) / sizeof(UNSIGNED)) * sizeof(UNSIGNED);
    	
	// Allocate stack for HISR
	Id->HSCSI_p_HISR_stack = HSCSI_Alloc((tSMALL|tUNCACHED), stack_size);

	// Can be INIT, TARGET, BOTH, INIT_ONLY, or TRGT_ONLY
	// this condition should find all single interrupt handler instances and set
	// the "both" handlers
	if ((Id->HSCSI_config.config_instance == BOTH_INSTANCE) ||
				(Id->HSCSI_config.config_instance == INIT_ONLY_INSTANCE) ||
				(Id->HSCSI_config.config_instance == TRGT_ONLY_INSTANCE))
	{
		// Register low level interrupt service routine
		status = NU_Register_LISR(Id->HSCSI_interrupt,
			HSCSI_ISR_Low_Both, &old_lisr);
			
		if (status != NU_SUCCESS)
		{
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_ISR_Create", 
				"NU_Register_LISR failed",
				status,
				(UNSIGNED)Id);
			return status;
		}
		Id->HSCSI_LISR_Registered = 1;
		
		// Create high level interrupt service routine 
		status = NU_Create_HISR(&Id->HSCSI_HISR, 
			HSCSI_HISR_NAME,				// name assigned to HISR
			HSCSI_ISR_High_Both,			// function entry point for HISR
			HSCSI_HISR_PRIORITY,
			Id->HSCSI_p_HISR_stack,
			stack_size);
	}
	else if (Id->HSCSI_config.enable_target_mode)
	{
		// Register low level interrupt service routine once only
		if (Id->HSCSI_LISR_Registered == 0)
		{
			status = NU_Register_LISR(Id->HSCSI_interrupt,
				HSCSI_ISR_Low, &old_lisr);
				
			if (status != NU_SUCCESS)
			{
				HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
					"HSCSI_ISR_Create", 
					"NU_Register_LISR failed",
					status,
					(UNSIGNED)Id);
				return status;
			}
			Id->HSCSI_LISR_Registered = 1;
		}
		
		// Create high level interrupt service routine 
		status = NU_Create_HISR(&Id->HSCSI_HISR, 
			HSCSI_HISR_NAME,				// name assigned to HISR
			HSCSI_ISR_High_Target,		// function entry point for HISR
			HSCSI_HISR_PRIORITY,
			Id->HSCSI_p_HISR_stack,
			stack_size);
	}
	else if (Id->HSCSI_config.enable_initiator_mode)
	{
		// Register low level interrupt service routine once only
		if (Id->HSCSI_LISR_Registered == 0)
		{
			status = NU_Register_LISR(Id->HSCSI_interrupt,
				HSCSI_ISR_Low, &old_lisr);
				
			if (status != NU_SUCCESS)
			{
				HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
					"HSCSI_ISR_Create", 
					"NU_Register_LISR failed",
					status,
					(UNSIGNED)Id);
				return status;
			}
			Id->HSCSI_LISR_Registered = 1;
		}
		
		// Create high level interrupt service routine 
		status = NU_Create_HISR(&Id->HSCSI_HISR, 
			HSCSI_HISR_NAME,				// name assigned to HISR
			HSCSI_ISR_High_Initiator,		// function entry point for HISR
			HSCSI_HISR_PRIORITY,
			Id->HSCSI_p_HISR_stack,
			stack_size);
	}
	
	if (status != NU_SUCCESS)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_ISR_Create", 
			"NU_Create_HISR failed",
			status,
			(UNSIGNED)Id);
		return status;
	}

	return status;
	
} // HSCSI_ISR_Create

/*************************************************************************/
// HSCSI_ISR_Destroy
// Destroy HSCSI_ISR object.
/*************************************************************************/
void HSCSI_ISR_Destroy()
{
	// TODO
 	HSCSI_TRACE_ENTRY(HSCSI_ISR_Destroy);
	
} // HSCSI_ISR_Destroy

/*************************************************************************/
// HSCSI_ISR_High_Both
// High Level Interrupt Service Routine for both initiator and target
// gets activated by HSCSI_ISR_Low_Both.
/*************************************************************************/
void HSCSI_ISR_High_Both(VOID)
{
	
	U16				semaphore;
	U16				MB5;
	PHSCSI_INSTANCE_DATA	Id;
	
 	HSCSI_TRACE_ENTRY(HSCSI_ISR_High_Both);
	
	Id = &H_Instance_Data;		// only one instance
	
#if defined(HSCSI_DEBUG)	
	Id->Num_high_isr_entry++;
#endif
	
	HSCSI_ASSERT(Id->HSCSI_instance == 0, "HSCSI_ISR_High_Both");

	// Test the semaphore lock bit.
	semaphore = Read_ISP1040(Id, HSCSI_SEMAPHORE);
	if(semaphore & 0x0001)

		// The semaphore lock bit is set.
		// The interrupt is the result of a mailbox command completion 
		// or an asynchronous event.
		// Activate the mailbox interrupt service routine.
		HSCSI_ISR_Mailbox(Id);
	
	// Check to see if mailbox 5 has changed.
    MB5 = Read_ISP1040(Id, HSCSI_MAILBOX_5);
    if(Id->HSCSI_response_FIFO_index != MB5)
    	
    	// Mailbox 5 has changed.
    	// A new IOCB has been placed in the response FIFO.
		HSCSI_ISR_Response_Queue(Id);
	
#if defined(HSCSI_DEBUG)	
	Id->HSCSI_flags &= ~1;	// clear in int handler flag
	Id->Num_high_isr++;
#endif
	
} // HSCSI_ISR_High_Both

/*************************************************************************/
// HSCSI_ISR_High_Target
// High Level Interrupt Service Routine
// gets activated by HSCSI_ISR_Low. (not used in initiator-only version)
/*************************************************************************/
void HSCSI_ISR_High_Target(VOID)
{
	
	U16				semaphore;
	U16				MB5;
	PHSCSI_INSTANCE_DATA	Id;
	
 	HSCSI_TRACE_ENTRY(HSCSI_ISR_High_Target);
	
	Id = &H_Instance_Data;
	
#if defined(HSCSI_DEBUG)	
	Id->Num_high_isr_entry++;
#endif
	
	HSCSI_ASSERT(Id->HSCSI_instance == TARGET_INSTANCE, "HSCSI_ISR_High_Target");

	// Test the semaphore lock bit.
	semaphore = Read_ISP1040(Id, HSCSI_SEMAPHORE);
	if(semaphore & 0x0001)

		// The semaphore lock bit is set.
		// The interrupt is the result of a mailbox command completion 
		// or an asynchronous event.
		// Activate the mailbox interrupt service routine.
		HSCSI_ISR_Mailbox(Id);
	
	// Check to see if mailbox 5 has changed.
    MB5 = Read_ISP1040(Id, HSCSI_MAILBOX_5);
    if(Id->HSCSI_response_FIFO_index != MB5)
    	
    	// Mailbox 5 has changed.
    	// A new IOCB has been placed in the response FIFO.
		HSCSI_ISR_Response_Queue(Id);
		
#if defined(HSCSI_DEBUG)	
	Id->HSCSI_flags &= ~1;	// clear in int handler flag
	Id->Num_high_isr++;
#endif
	
} // HSCSI_ISR_High_Target

/*************************************************************************/
// HSCSI_ISR_High_Initiator
// High Level Interrupt Service Routine
// gets activated by HSCSI_ISR_Low.
/*************************************************************************/
void HSCSI_ISR_High_Initiator(VOID)
{
	
	U16				semaphore;
	U16				MB5;
	PHSCSI_INSTANCE_DATA	Id;
	
 	HSCSI_TRACE_ENTRY(HSCSI_ISR_High_Initiator);
	
	Id = &H_Instance_Data; // only one instance
	
#if defined(HSCSI_DEBUG)	
	Id->Num_high_isr_entry++;
#endif
	
	HSCSI_ASSERT(Id->HSCSI_instance == INITIATOR_INSTANCE, "HSCSI_ISR_High_Initiator");

	// Test the semaphore lock bit.
	semaphore = Read_ISP1040(Id, HSCSI_SEMAPHORE);
	if(semaphore & 0x0001) 

		// The semaphore lock bit is set.
		// The interrupt is the result of a mailbox command completion 
		// or an asynchronous event.
		// Activate the mailbox interrupt service routine.
		HSCSI_ISR_Mailbox(Id);
		
	// Check to see if mailbox 5 has changed.
    MB5 = Read_ISP1040(Id, HSCSI_MAILBOX_5);
    if(Id->HSCSI_response_FIFO_index != MB5) {
    	
    	// Mailbox 5 has changed.
    	// A new IOCB has been placed in the response FIFO.
		// However, ignore MB5 during initialization as it will change
		// during Firmware Checksum. 
		if (Id->HSCSI_state == HSCSI_STATE_ACTIVE)
						
			HSCSI_ISR_Response_Queue(Id);
	
	}
	
#if defined(HSCSI_DEBUG)	
	Id->HSCSI_flags &= ~1;	// clear in int handler flag
	Id->Num_high_isr++;
#endif
	
} // HSCSI_ISR_High_Initiator

/*************************************************************************/
// Low Level Interrupt Service Routine for the Target and Initiator
// Come here when a mailbox command is completed,
// the request queue has been updated, or
// an asynchronous event has occurred.
/*************************************************************************/
void HSCSI_ISR_Low(INT vector_number)
{
	STATUS			status;
	PHSCSI_INSTANCE_DATA	Id;
#if defined(HSCSI_DEBUG) && defined(_DEBUG)
	U32				trace_level_save;
	
	trace_level_save = TraceLevel[TRACE_INDEX];		// can't use semaphores here
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;
#endif

	Id = &H_Instance_Data;

	// Check to see if we actually have an interrupt from the target ISP.
	if(Read_ISP1040(Id, HSCSI_PCI_INT_STATUS) & HSCSI_RISC_INT_PENDING)
	{
#if defined(HSCSI_DEBUG)	
		Id->HSCSI_flags |= 1;
		Id->Num_low_isr++;
#endif
			
	    Write_ISP1040(Id, HSCSI_HCCR, HCCRCLRR2HINTR); // clear interrupt
    
		status = NU_Activate_HISR(&Id->HSCSI_HISR);
		if (status != NU_SUCCESS)
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_ISR_Low", 
				"NU_Activate_HISR failed",
				status,
				(UNSIGNED)Id);
	}

#if defined(HSCSI_DEBUG) && defined(_DEBUG)
	TraceLevel[TRACE_INDEX] = trace_level_save;
#endif

} // HSCSI_ISR_Low

/*************************************************************************/
// Low Level Interrupt Service Routine for the Target and Initiator
// Come here when a mailbox command is completed,
// the request queue has been updated, or
// an asynchronous event has occurred. (not used in initiator-only version)
/*************************************************************************/
void HSCSI_ISR_Low_Both(INT vector_number)
{
	STATUS			status;
	PHSCSI_INSTANCE_DATA	Id;
#if defined(HSCSI_DEBUG) && defined(_DEBUG)
	U32				trace_level_save;
	
	trace_level_save = TraceLevel[TRACE_INDEX];		// can't use semaphores here
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;
#endif

	Id = &H_Instance_Data;

	// Check to see if we actually have an interrupt from the ISP.
	if(Read_ISP1040(Id, HSCSI_PCI_INT_STATUS) & HSCSI_RISC_INT_PENDING)
	{
#if defined(HSCSI_DEBUG)	
		Id->HSCSI_flags |= 1;
		Id->Num_low_isr++;
#endif
	
    	// Clear RISC interrupt to host .
    	Write_ISP1040(Id, HSCSI_HCCR, HCCRCLRR2HINTR);
    
		status = NU_Activate_HISR(&Id->HSCSI_HISR);
		if (status != NU_SUCCESS)
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_ISR_Low_Both", 
				"NU_Activate_HISR failed",
				status,
				(UNSIGNED)Id);
	}
	
#if defined(HSCSI_DEBUG) && defined(_DEBUG)
	TraceLevel[TRACE_INDEX] = trace_level_save;
#endif
} // HSCSI_ISR_Low_Both

/*************************************************************************/
// HSCSI_ISR_Mailbox
// Come here when a mailbox command has completed or an asynchronous
// mailbox message has been received.
/*************************************************************************/
void HSCSI_ISR_Mailbox(PHSCSI_INSTANCE_DATA Id)
{
	HSCSI_EVENT_CONTEXT	*p_context;
	STATUS 			 	 status;

 	HSCSI_TRACE_ENTRY(HSCSI_ISR_Mailbox);
	
	// Save the contents of outgoing mailbox register 0.
	Id->HSCSI_mailbox_message.mailbox[0] = Read_ISP1040(Id, HSCSI_MAILBOX_0);
		
	// Check for an asynchronous event.
    switch(Id->HSCSI_mailbox_message.mailbox[0])
    {
        case 0x8001:
        
        	// Asynchronous event Reset detected
	        HSCSI_Handle_Async_Event(Id, HSCSI_ACTION_HANDLE_OTHER_AE,
	        			Id->HSCSI_mailbox_message.mailbox[0]);
	        
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_WARNING,
				"HSCSI_ISR_Mailbox", 
				"Asynchronous event Reset detected",
				0,
				Id->HSCSI_mailbox_message.mailbox[0]);
	        goto Clear_Semaphore_Lock;
	        
        case 0x8002:
        
        	// Asynchronous event System Error detected
        	// usually get this error when an address error occurs,
        	// not a physical address, not mapped memory
			// Save the contents of the outgoing mailbox registers for Debug
			Id->HSCSI_mailbox_message.mailbox[1] = Read_ISP1040(Id, HSCSI_MAILBOX_1);
			Id->HSCSI_mailbox_message.mailbox[2] = Read_ISP1040(Id, HSCSI_MAILBOX_2);
			Id->HSCSI_mailbox_message.mailbox[3] = Read_ISP1040(Id, HSCSI_MAILBOX_3);
			Id->HSCSI_mailbox_message.mailbox[4] = Read_ISP1040(Id, HSCSI_MAILBOX_4);
			Id->HSCSI_mailbox_message.mailbox[5] = Read_ISP1040(Id, HSCSI_MAILBOX_5);
			Id->HSCSI_mailbox_message.mailbox[6] = Read_ISP1040(Id, HSCSI_MAILBOX_6);
			Id->HSCSI_mailbox_message.mailbox[7] = Read_ISP1040(Id, HSCSI_MAILBOX_7);
			
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_ISR_Mailbox", 
				"Asynchronous event System Error detected",
				0,
				Id->HSCSI_mailbox_message.mailbox[0]);
	        goto Clear_Semaphore_Lock;
	        
        case 0x8003:
        
        	// Asynchronous event Request Transfer error
//	        HSCSI_Handle_Async_Event(Id, HSCSI_ACTION_HANDLE_OTHER_AE,
//	        			Id->HSCSI_mailbox_message.mailbox[0]);
	        
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_ISR_Mailbox", 
				"Asynchronous event Request Transfer error detected",
				0,
				Id->HSCSI_mailbox_message.mailbox[0]);
	        goto Clear_Semaphore_Lock;
	        
        case 0x8004:
        
        	// Asynchronous event Response Transfer error
//	        HSCSI_Handle_Async_Event(Id, HSCSI_ACTION_HANDLE_OTHER_AE,
//	        			Id->HSCSI_mailbox_message.mailbox[0]);
	        
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_ISR_Mailbox", 
				"Asynchronous event Response Transfer error detected",
				0,
				Id->HSCSI_mailbox_message.mailbox[0]);
	        goto Clear_Semaphore_Lock;
	        
        case 0x8005:
        
        	// Asynchronous event Request queue wakeup
        	// Not currently using this feature
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_ISR_Mailbox", 
				"Asynchronous event Request queue wakeup detected",
				0,
				Id->HSCSI_mailbox_message.mailbox[0]);
	        goto Clear_Semaphore_Lock;
	        
	    case 0x8006:
	    	// Execution timeout reset

	        HSCSI_Handle_Async_Event(Id, HSCSI_ACTION_HANDLE_OTHER_AE,
	        			Id->HSCSI_mailbox_message.mailbox[0]);

	    	HSCSI_Log_Error(HSCSI_ERROR_TYPE_WARNING, "HSCSI_ISR_Mailbox",
	    		"Execution Timeout Reset detected", 0,
	    		Id->HSCSI_mailbox_message.mailbox[0]);
	    	goto Clear_Semaphore_Lock;
	        
       case 0x8020:
       
         	// Asynchronous event SCSI command Complete (fast posting)
         	// Send a message to the task that is waiting
         	
       case 0x8021:
       
         	// Asynchronous event CTIO Complete. (fast posting)
         	// Send a message to the task that is waiting

#if defined(HSCSI_DEBUG)	
			// save these values for DEBUG later
			Id->HSCSI_mailbox_message.mailbox[1] = Read_ISP1040(Id, HSCSI_MAILBOX_1);
			Id->HSCSI_mailbox_message.mailbox[2] = Read_ISP1040(Id, HSCSI_MAILBOX_2);
#endif

         	// Get pointer to context for waiting task.
         	
#if 0		// NOTE: This code worked prior to the V3 IDE...
         	p_context = (HSCSI_EVENT_CONTEXT*)(UNSIGNED)
         				(BYTE_SWAP16(Read_ISP1040(Id, HSCSI_MAILBOX_2)) |
         				BYTE_SWAP16(Read_ISP1040(Id, HSCSI_MAILBOX_1)) << 16);
#else
         	p_context = (HSCSI_EVENT_CONTEXT*)(UNSIGNED)
         				(*(U16 *)((UNSIGNED)Id->Regs + HSCSI_MAILBOX_2) |
         				*(U16 *)((UNSIGNED)Id->Regs + HSCSI_MAILBOX_1) << 16);
#endif
			
			HSCSI_PRINT_HEX(TRACE_L8, "\n\rFast Post context = ", (U32)p_context);
			
			// Clear the semaphore lock.
			// This lets the RISC know that the outgoing mailbox registers
			// are now available.
			Write_ISP1040(Id, HSCSI_SEMAPHORE, 0);
			
			// The command completed successfully, so set the status
			// in the status IOCB.
//			if (Id->HSCSI_mailbox_message.mailbox[0] == 0x8021)
//				((PIOCB_CTIO_TYPE_2) &p_context->status_iocb)->status = 
//									STATUS_COMPLETE;
//			else
//			{
				p_context->status_iocb.SCSI_status = SCSI_STATUS_GOOD;
				p_context->status_iocb.state_flags = 
									BYTE_SWAP16(IOCB_STATE_FLAGS_XFER_COMPLETE);
//			}
	
			HSCSI_ASSERT(Id == p_context->Id, "HSCSI_ISR_Mailbox Fast Post");
			
			// Send a message to HSCSI_Event_Task.
			// The action field of the context will tell HSCSI_Event_Task
			// what to do next. 
    		status = NU_Send_To_Queue(&Id->HSCSI_event_queue, 
    			&p_context, // message is pointer to context
        		1, // size is one UNSIGNED 
        		NU_NO_SUSPEND);
				
         	if (status != NU_SUCCESS)
				HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
					"HSCSI_ISR_Mailbox", 
					"NU_Send_To_Queue failed",
					status,
					(UNSIGNED)Id);
         	 
	        return;

        default:
        
        	// It's not an asynchronous event
        	break;
        
    } // switch			

	// Continue if it's not an asynchronous event
	// Assume that a task is waiting for a mailbox command to complete.
	// Save the contents of the outgoing mailbox registers.
	Id->HSCSI_mailbox_message.mailbox[1] = Read_ISP1040(Id, HSCSI_MAILBOX_1);
	Id->HSCSI_mailbox_message.mailbox[2] = Read_ISP1040(Id, HSCSI_MAILBOX_2);
	Id->HSCSI_mailbox_message.mailbox[3] = Read_ISP1040(Id, HSCSI_MAILBOX_3);
		
	// Clear the semaphore lock.
	// This lets the RISC know that the outgoing mailbox registers
	// are now available.
	Write_ISP1040(Id, HSCSI_SEMAPHORE, 0);
	
	HSCSI_PRINT_STRING(TRACE_L8, "\n\rMB Command Complete");
	
	// Send contents of mailboxes to waiting task.
    status = NU_Send_To_Queue(&Id->HSCSI_mailbox_queue, 
    	&Id->HSCSI_mailbox_message, 
        sizeof(HSCSI_MAILBOX_MESSAGE) / sizeof(UNSIGNED), // size is an entire MB struct
        NU_NO_SUSPEND);
				
    if (status != NU_SUCCESS)
    {
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_ISR_Mailbox", 
			"NU_Send_To_Queue failed",
			status,
			(UNSIGNED)Id);
		return;
	}
	
	return;
		
Clear_Semaphore_Lock:

	// Clear the semaphore lock.
	// This lets the RISC know that the outgoing mailbox registers
	// are now available.
	Write_ISP1040(Id, HSCSI_SEMAPHORE, 0);
	
} // HSCSI_ISR_Mailbox

/*************************************************************************/
// HSCSI_ISR_Response_Queue
// Come here when a new IOCB has been placed in the response queue.
// This IOCB could be a response to a request IOCB, or it could be
// a new command to be completed in target mode.
/*************************************************************************/
void HSCSI_ISR_Response_Queue(PHSCSI_INSTANCE_DATA Id)
{
	HSCSI_EVENT_CONTEXT	*p_context;
	IOCB_STATUS_ENTRY	*p_IOCB;
	STATUS				 status;
	
 	HSCSI_TRACE_ENTRY(HSCSI_ISR_Response_Queue);
	
	// Point to IOCB in response FIFO
	p_IOCB = (IOCB_STATUS_ENTRY*)
		&(Id->HSCSI_p_IOCB_response_FIFO[Id->HSCSI_response_FIFO_index]);
	
	// decide what to do with the IOCB based on the entry_type
	switch(p_IOCB->entry_type) {

	case	HSCSI_IOCB_TYPE_MARKER:
		HSCSI_PRINT_STRING(TRACE_L7, "\n\rMarker IOCB");
		HSCSI_Response_FIFO_Increment_Index(Id);
		return;

	case	HSCSI_IOCB_TYPE_STATUS:
		HSCSI_PRINT_STRING(TRACE_L7, "\n\rStatus IOCB");

	    // If this IOCB is the result of a request that completed, then
	    // system_defined2 points to a context.
		p_context = (HSCSI_EVENT_CONTEXT*)p_IOCB->system_defined2;
		if (p_context == (HSCSI_EVENT_CONTEXT *)0) 
		{
    		// If we can't find an event context, 
    		// we can't handle this interrupt!
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_ISR_Response_Queue", 
				"No event context for Response IOCB",
				0,
				(UNSIGNED)Id);
			return;
		}
		
    	// Copy IOCB from response FIFO to status_iocb of context
    	Mem_Copy((char*)&p_context->status_iocb, // to
    		(char*)p_IOCB, // from
    		IOCB_SIZE); // number of bytes   		
    
		break;

	default:
		// If we don't know what the type is,
		// we can't handle this interrupt!
	    HSCSI_DUMP_HEX(TRACE_L8, "\n\rEntry Data ",
	    					(U8 *)p_IOCB,
	    					sizeof(IOCB_STATUS_ENTRY));

		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_ISR_Response_Queue", 
			"Unknown entry type",
			p_IOCB->entry_type,
			(UNSIGNED)Id);
		return;
	}
	
	// Increment the index when we have processed this IOCB.
    HSCSI_Response_FIFO_Increment_Index(Id);

	// Send a message to HSCSI_Event_Task.
	// The action field of the context will tell HSCSI_Event_Task
	// what to do next. 
    status = NU_Send_To_Queue(&Id->HSCSI_event_queue, 
    	&p_context, // message is pointer to context
        1, // size is one UNSIGNED 
        NU_NO_SUSPEND);
        	
    if (status != NU_SUCCESS)
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_ISR_Response_Queue", 
			"NU_Send_To_Queue for request completion failed",
			status,
			(UNSIGNED)Id);

} // HSCSI_ISR_Response_Queue

/*************************************************************************/
// HSCSI_Handle_Async_Event
// Send a message to the event queue, this event in turn will call the
// AE Handler method defined for the DDM handling this library.
// A new context is allocated to service the AE, which is deallocated
// in the AE Handler for  the DDM.
/*************************************************************************/

void HSCSI_Handle_Async_Event (PHSCSI_INSTANCE_DATA Id, HSCSI_EVENT_ACTION action, U16 event_code)
{
	HSCSI_EVENT_CONTEXT	*p_context;
	STATUS				 status = NU_SUCCESS;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Handle_Async_Event);
 	
	// Allocate an HSCSI_EVENT_CONTEXT from the pool.
	status = NU_Allocate_Partition(&Id->HSCSI_event_context_pool, 
		(VOID**)&p_context, NU_NO_SUSPEND);

	if (status != NU_SUCCESS)
	{
		// TODO
		// If we can't allocate an event context, 
		// we can't handle this action!
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Handle_Async_Event", 
			"NU_Allocate_Partition for event context failed",
			status,
			(UNSIGNED)Id);
		return;
	}
    
    p_context->action = action;
    p_context->Id = Id;
    
    // CTIO_flags overloaded to save the event code
//	p_context->CTIO_flags = event_code;  
    
	// Send a message to HSCSI_Event_Task.
	// The action field of the context will tell HSCSI_Event_Task
	// what to do next. 
    status = NU_Send_To_Queue(&Id->HSCSI_event_queue, 
    	&p_context, // message is pointer to context
        1, // size is one UNSIGNED 
        NU_NO_SUSPEND);
        	
    if (status != NU_SUCCESS)
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Handle_Async_Event", 
			"NU_Send_To_Queue for request completion failed",
			status,
			(UNSIGNED)Id);

	status = HSCSI_Marker_IOCB(Id); // send Marker IOCB to clear the reset
	
	if (status != NU_SUCCESS)
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL, "HSCSI_Handle_Async_Event",
		"HSCSI_Marker_IOCB failed", status, (UNSIGNED)Id);
		
	return;
}
