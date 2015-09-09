/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpISR.c
// 
// Description:
// This file implements interrupt handling for the ISP2100. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/FCP/FcpISR.c $
// 
// 25    1/17/00 12:34p Jtaylor
// Don't change status in FCP_Handle_Immediate_Notify
// 
// 24    1/09/00 1:59p Mpanas
// Fix compiler warnings
//
//
// 5/7/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/3/98 Michael G. Panas: Add code to manage a pointer to a message
//							instead of an actual message in FCP_EVENT_CONTEXT
// 10/1/98 Michael G. Panas: add better Immediate Notify handling
// 10/1/98 Michael G. Panas: changes to support init and target in same instance
// 10/6/98 Michael G. Panas: changes to support interrupt driven mailbox
// and iocb commands
// 10/21/98 Michael G. Panas: Fix bugs with Immediate Notify
// 11/20/98 Michael G. Panas: Add DEBUG code for intr handling
// 11/30/98 Michael G. Panas: New memory allocation methods
// 11/30/98 Michael G. Panas: new async event handler
/*************************************************************************/
#include "FcpCommon.h"
#include "FcpData.h"
#include "FcpMessage.h"
#include "FcpISR.h"
#include "FcpISP.h"
#include "FcpResponseFIFO.h"
#include "FcpRequestFIFO.h"
#include "FcpString.h"
#include "FcpMemory.h"
#include "OsTypes.h"
#include "RequestCodes.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
#if defined(_NAC)
void		FCP_ISR_Low_0(INT vector_number);
void		FCP_ISR_Low_1(INT vector_number);
void		FCP_ISR_Low_2(INT vector_number);
void		FCP_ISR_High_0();
void		FCP_ISR_High_1();
void		FCP_ISR_High_2();
#else
void		FCP_ISR_High_Target();
void		FCP_ISR_High_Initiator();
void		FCP_ISR_Low(INT vector_number);
void		FCP_ISR_High_Both();
void		FCP_ISR_Low_Both(INT vector_number);
#endif
void		FCP_ISR_Mailbox(PINSTANCE_DATA Id);
void		FCP_ISR_Response_Queue(PINSTANCE_DATA Id);


/*************************************************************************/
// FCP_ISR_Create
// Create FCP_ISR object.
/*************************************************************************/
STATUS FCP_ISR_Create(PINSTANCE_DATA Id)
{
	VOID			(*old_lisr)(INT); // pointer to old LISR
	UNSIGNED		 stack_size;
	STATUS			 status = FCP_ERROR_INTERRUPT_SETUP_FAILED;
	
 	FCP_TRACE_ENTRY(FCP_ISR_Create);
	
    // Insure UNSIGNED alignment of config stack size. 
    stack_size = ((Id->FCP_config.HISR_stack_size 
    	+ sizeof(UNSIGNED) - 1) / sizeof(UNSIGNED)) * sizeof(UNSIGNED);
    	
	// Allocate stack for HISR
//	Id->FCP_p_HISR_stack = FCP_Alloc((tSMALL|tUNCACHED), stack_size);
	Id->FCP_p_HISR_stack = FCP_Alloc((tSMALL), stack_size);


#if defined(_NAC)
	// the NAC has three chips each with their own interrupt
	// we need an HISR for each
	
	switch(Id->FCP_instance)
	{
	case 0:
		// Register low level interrupt service routine
		status = NU_Register_LISR(Id->FCP_interrupt,
			FCP_ISR_Low_0, &old_lisr);
			
		if (status != NU_SUCCESS)
		{
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Create", 
				"NU_Register_LISR failed",
				status,
				(UNSIGNED)Id);
			return status;
		}
		Id->FCP_LISR_Registered = 1;
		
		// Create high level interrupt service routine 
		status = NU_Create_HISR(&Id->FCP_HISR, 
			FCP_HISR_NAME0,				// name assigned to HISR
			FCP_ISR_High_0,				// function entry point for HISR
			FCP_HISR_PRIORITY,
			Id->FCP_p_HISR_stack,
			stack_size);
		break;

	case 1:
		// Register low level interrupt service routine
		status = NU_Register_LISR(Id->FCP_interrupt,
			FCP_ISR_Low_1, &old_lisr);
			
		if (status != NU_SUCCESS)
		{
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Create", 
				"NU_Register_LISR failed",
				status,
				(UNSIGNED)Id);
			return status;
		}
		Id->FCP_LISR_Registered = 1;
		
		// Create high level interrupt service routine 
		status = NU_Create_HISR(&Id->FCP_HISR, 
			FCP_HISR_NAME1,				// name assigned to HISR
			FCP_ISR_High_1,				// function entry point for HISR
			FCP_HISR_PRIORITY,
			Id->FCP_p_HISR_stack,
			stack_size);
		break;

	case 2:
		// Register low level interrupt service routine
		status = NU_Register_LISR(Id->FCP_interrupt,
			FCP_ISR_Low_2, &old_lisr);
			
		if (status != NU_SUCCESS)
		{
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Create", 
				"NU_Register_LISR failed",
				status,
				(UNSIGNED)Id);
			return status;
		}
		Id->FCP_LISR_Registered = 1;
		
		// Create high level interrupt service routine 
		status = NU_Create_HISR(&Id->FCP_HISR, 
			FCP_HISR_NAME2,				// name assigned to HISR
			FCP_ISR_High_2,				// function entry point for HISR
			FCP_HISR_PRIORITY,
			Id->FCP_p_HISR_stack,
			stack_size);
		break;
	
	default:
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_ISR_Create", 
			"Invalid Instance number",
			Id->FCP_instance,
			(UNSIGNED)Id);
		return status;
	}
	
#else
	// Can be INIT, TARGET, BOTH, INIT_ONLY, or TRGT_ONLY
	// this condition should find all single interrupt handler instances and set
	// the "both" handlers
	if ((Id->FCP_config.config_instance == BOTH_INSTANCE) ||
				(Id->FCP_config.config_instance == INIT_ONLY_INSTANCE) ||
				(Id->FCP_config.config_instance == TRGT_ONLY_INSTANCE))
	{
		// Register low level interrupt service routine
		status = NU_Register_LISR(Id->FCP_interrupt,
			FCP_ISR_Low_Both, &old_lisr);
			
		if (status != NU_SUCCESS)
		{
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Create", 
				"NU_Register_LISR failed",
				status,
				(UNSIGNED)Id);
			return status;
		}
		Id->FCP_LISR_Registered = 1;
		
		// Create high level interrupt service routine 
		status = NU_Create_HISR(&Id->FCP_HISR, 
			FCP_HISR_NAME,				// name assigned to HISR
			FCP_ISR_High_Both,			// function entry point for HISR
			FCP_HISR_PRIORITY,
			Id->FCP_p_HISR_stack,
			stack_size);
	}
	else if (Id->FCP_config.enable_target_mode)
	{
		// Register low level interrupt service routine once only
		if (Id->FCP_LISR_Registered == 0)
		{
			status = NU_Register_LISR(Id->FCP_interrupt,
				FCP_ISR_Low, &old_lisr);
				
			if (status != NU_SUCCESS)
			{
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_ISR_Create", 
					"NU_Register_LISR failed",
					status,
					(UNSIGNED)Id);
				return status;
			}
			Id->FCP_LISR_Registered = 1;
		}
		
		// Create high level interrupt service routine 
		status = NU_Create_HISR(&Id->FCP_HISR, 
			FCP_HISR_NAME,				// name assigned to HISR
			FCP_ISR_High_Target,		// function entry point for HISR
			FCP_HISR_PRIORITY,
			Id->FCP_p_HISR_stack,
			stack_size);
	}
	else if (Id->FCP_config.enable_initiator_mode)
	{
		// Register low level interrupt service routine once only
		if (Id->FCP_LISR_Registered == 0)
		{
			status = NU_Register_LISR(Id->FCP_interrupt,
				FCP_ISR_Low, &old_lisr);
				
			if (status != NU_SUCCESS)
			{
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_ISR_Create", 
					"NU_Register_LISR failed",
					status,
					(UNSIGNED)Id);
				return status;
			}
			Id->FCP_LISR_Registered = 1;
		}
		
		// Create high level interrupt service routine 
		status = NU_Create_HISR(&Id->FCP_HISR, 
			FCP_HISR_NAME,				// name assigned to HISR
			FCP_ISR_High_Initiator,		// function entry point for HISR
			FCP_HISR_PRIORITY,
			Id->FCP_p_HISR_stack,
			stack_size);
	}

#endif
	
	if (status != NU_SUCCESS)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_ISR_Create", 
			"NU_Create_HISR failed",
			status,
			(UNSIGNED)Id);
		return status;
	}

	// Create Semaphore to signal Loop Up
	status = NU_Create_Semaphore(&Id->FCP_Loop_Sema,
		"Loop Up",
		0,		// initial count
		NU_FIFO);

	if (status != NU_SUCCESS)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Semaphore_Create", 
			"NU_Create_Semaphore failed",
			status,
			(UNSIGNED)Id);
		return status;
	}

	return status;
	
} // FCP_ISR_Create

/*************************************************************************/
// FCP_ISR_Destroy
// Destroy FCP_ISR object.
/*************************************************************************/
void FCP_ISR_Destroy()
{
	// TODO
 	FCP_TRACE_ENTRY(FCP_ISR_Destroy);
	
} // FCP_ISR_Destroy


#if defined(_NAC)

//========================================================================
// The NAC has three identical interrupt channels
// We need a seperate High and Low interrupt handler for each.
//========================================================================

/*************************************************************************/
// FCP_ISR_High_0
// High Level Interrupt Service Routine for both initiator and target
// gets activated by FCP_ISR_Low_0.
/*************************************************************************/
void FCP_ISR_High_0(VOID)
{
	
	U16				semaphore;
	U16				MB5;
	PINSTANCE_DATA	Id;
	//U8				update;
	
 	FCP_TRACE_ENTRY(FCP_ISR_High_0);
	
	Id = &Instance_Data[0];		// use the zeroth instance
	
#if defined(FCP_DEBUG)	
	Id->Num_high_isr_entry++;
#endif
	
	FCP_ASSERT(Id->FCP_instance == 0, "FCP_ISR_High_0");

	// Test the semaphore lock bit.
	semaphore = Read_ISP(Id, ISP_SEMAPHORE);
	if(semaphore & 0x0001)

		// The semaphore lock bit is set.
		// The interrupt is the result of a mailbox command completion 
		// or an asynchronous event.
		// Activate the mailbox interrupt service routine.
		FCP_ISR_Mailbox(Id);
	
//	update = FALSE;
//	while ( (MB5 = Read_ISP(Id, ISP_MAILBOX_5)) != Id->FCP_response_FIFO_index)
//	{
//		FCP_ISR_Response_Queue(Id);
//	    Id->FCP_response_FIFO_index++;
//	    Id->FCP_response_FIFO_index %= Id->FCP_config.ISP_FIFO_response_size;
//		update = TRUE;
//	}
//	if (update)
//	    // Update response queue in pointer in mailbox 5.
//  	Write_ISP(Id, ISP_MAILBOX_5, Id->FCP_response_FIFO_index);

	// Check to see if mailbox 5 has changed.
    MB5 = Read_ISP(Id, ISP_MAILBOX_5);
    if(Id->FCP_response_FIFO_index != MB5)
    	
    	// Mailbox 5 has changed.
    	// A new IOCB has been placed in the response FIFO.
		FCP_ISR_Response_Queue(Id);
    
//    semaphore = Read_ISP(Id, ISP_SEMAPHORE);
//	if(semaphore & 0x0001)

		// The semaphore lock bit is set.
		// The interrupt is the result of a mailbox command completion 
		// or an asynchronous event.
		// Activate the mailbox interrupt service routine.
//		FCP_ISR_Mailbox(Id);
		
#if defined(FCP_DEBUG)	
	Id->FCP_flags &= ~1;	// clear in int handler flag
	Id->Num_high_isr++;
#endif
	
} // FCP_ISR_High_0

/*************************************************************************/
// Low Level Interrupt Service Routine for the Target and Initiator chip 0
// Come here when a mailbox command is completed,
// the request queue has been updated, or
// an asynchronous event has occurred.
/*************************************************************************/
void FCP_ISR_Low_0(INT vector_number)
{
	STATUS			status;
	PINSTANCE_DATA	Id;
#if defined(FCP_DEBUG) && defined(_DEBUG)
	U32				trace_level_save;
	
	trace_level_save = TraceLevel[TRACE_INDEX];		// can't use semaphores here
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;
#endif

	Id = &Instance_Data[0];

	// Check to see if we actually have an interrupt from the ISP.
	if(Read_ISP(Id, ISP_TO_PCI_INT_STATUS) & RISC_INT_PENDING)
	{
#if defined(FCP_DEBUG)	
		Id->FCP_flags |= 1;
		Id->Num_low_isr++;
#endif
	
    	// Clear RISC interrupt to host .
		Write_ISP(Id, ISP_HCCR, HCTLCLRR2HINTR);
    
		status = NU_Activate_HISR(&Id->FCP_HISR);
		if (status != NU_SUCCESS)
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Low_0", 
				"NU_Activate_HISR failed",
				status,
				(UNSIGNED)Id);
	}
	
#if defined(FCP_DEBUG) && defined(_DEBUG)
	TraceLevel[TRACE_INDEX] = trace_level_save;
#endif
} // FCP_ISR_Low_0

/*************************************************************************/
// FCP_ISR_High_1
// High Level Interrupt Service Routine for both initiator and target
// gets activated by FCP_ISR_Low_1.
/*************************************************************************/
void FCP_ISR_High_1(VOID)
{
	
	U16				semaphore;
	U16				MB5;
	PINSTANCE_DATA	Id;
	//U8				update;
	
 	FCP_TRACE_ENTRY(FCP_ISR_High_1);
	
	Id = &Instance_Data[1];		// use the second instance
	
#if defined(FCP_DEBUG)	
	Id->Num_high_isr_entry++;
#endif
	
	FCP_ASSERT(Id->FCP_instance == 1, "FCP_ISR_High_1");

	// Test the semaphore lock bit.
	semaphore = Read_ISP(Id, ISP_SEMAPHORE);
	if(semaphore & 0x0001)

		// The semaphore lock bit is set.
		// The interrupt is the result of a mailbox command completion 
		// or an asynchronous event.
		// Activate the mailbox interrupt service routine.
		FCP_ISR_Mailbox(Id);
	
//	update = FALSE;
//	while ( (MB5 = Read_ISP(Id, ISP_MAILBOX_5)) != Id->FCP_response_FIFO_index)
//	{
//		FCP_ISR_Response_Queue(Id);
//	    Id->FCP_response_FIFO_index++;
//	    Id->FCP_response_FIFO_index %= Id->FCP_config.ISP_FIFO_response_size;
//		update = TRUE;
//	}
//	if (update)
//	    // Update response queue in pointer in mailbox 5.
//  	Write_ISP(Id, ISP_MAILBOX_5, Id->FCP_response_FIFO_index);

	// Check to see if mailbox 5 has changed.
    MB5 = Read_ISP(Id, ISP_MAILBOX_5);
    if(Id->FCP_response_FIFO_index != MB5)
    	
    	// Mailbox 5 has changed.
    	// A new IOCB has been placed in the response FIFO.
		FCP_ISR_Response_Queue(Id);

   
#if defined(FCP_DEBUG)	
	Id->FCP_flags &= ~1;	// clear in int handler flag
	Id->Num_high_isr++;
#endif
	
} // FCP_ISR_High_1

/*************************************************************************/
// Low Level Interrupt Service Routine for the Target and Initiator chip 1
// Come here when a mailbox command is completed,
// the request queue has been updated, or
// an asynchronous event has occurred.
/*************************************************************************/
void FCP_ISR_Low_1(INT vector_number)
{
	STATUS			status;
	PINSTANCE_DATA	Id;
#if defined(FCP_DEBUG) && defined(_DEBUG)
	U32				trace_level_save;
	
	trace_level_save = TraceLevel[TRACE_INDEX];		// can't use semaphores here
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;
#endif

	Id = &Instance_Data[1];

	// Check to see if we actually have an interrupt from the ISP.
	if(Read_ISP(Id, ISP_TO_PCI_INT_STATUS) & RISC_INT_PENDING)
	{
#if defined(FCP_DEBUG)	
		Id->FCP_flags |= 1;
		Id->Num_low_isr++;
#endif
	
    	// Clear RISC interrupt to host .
    	Write_ISP(Id, ISP_HCCR, HCTLCLRR2HINTR);
    
		status = NU_Activate_HISR(&Id->FCP_HISR);
		if (status != NU_SUCCESS)
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Low_1", 
				"NU_Activate_HISR failed",
				status,
				(UNSIGNED)Id);
	}
	
#if defined(FCP_DEBUG) && defined(_DEBUG)
	TraceLevel[TRACE_INDEX] = trace_level_save;
#endif
} // FCP_ISR_Low_1

/*************************************************************************/
// FCP_ISR_High_2
// High Level Interrupt Service Routine for both initiator and target
// gets activated by FCP_ISR_Low_2.
/*************************************************************************/
void FCP_ISR_High_2(VOID)
{
	
	U16				semaphore;
	U16				MB5;
	PINSTANCE_DATA	Id;
	//U8				update;
	
 	FCP_TRACE_ENTRY(FCP_ISR_High_2);
	
	Id = &Instance_Data[2];		// use the third instance
	
#if defined(FCP_DEBUG)	
	Id->Num_high_isr_entry++;
#endif
	
	FCP_ASSERT(Id->FCP_instance == 2, "FCP_ISR_High_2");

	// Test the semaphore lock bit.
	semaphore = Read_ISP(Id, ISP_SEMAPHORE);
	if(semaphore & 0x0001)

		// The semaphore lock bit is set.
		// The interrupt is the result of a mailbox command completion 
		// or an asynchronous event.
		// Activate the mailbox interrupt service routine.
		FCP_ISR_Mailbox(Id);
	
//	update = FALSE;
//	while ( (MB5 = Read_ISP(Id, ISP_MAILBOX_5)) != Id->FCP_response_FIFO_index)
//	{
//		FCP_ISR_Response_Queue(Id);
//	    Id->FCP_response_FIFO_index++;
//	    Id->FCP_response_FIFO_index %= Id->FCP_config.ISP_FIFO_response_size;
//		update = TRUE;
//	}
//	if (update)
//	    // Update response queue in pointer in mailbox 5.
//  	Write_ISP(Id, ISP_MAILBOX_5, Id->FCP_response_FIFO_index);

	// Check to see if mailbox 5 has changed.
    MB5 = Read_ISP(Id, ISP_MAILBOX_5);
    if(Id->FCP_response_FIFO_index != MB5)
    	
    	// Mailbox 5 has changed.
    	// A new IOCB has been placed in the response FIFO.
		FCP_ISR_Response_Queue(Id);

#if defined(FCP_DEBUG)	
	Id->FCP_flags &= ~1;	// clear in int handler flag
	Id->Num_high_isr++;
#endif
	
} // FCP_ISR_High_2

/*************************************************************************/
// Low Level Interrupt Service Routine for the Target and Initiator chip 2
// Come here when a mailbox command is completed,
// the request queue has been updated, or
// an asynchronous event has occurred.
/*************************************************************************/
void FCP_ISR_Low_2(INT vector_number)
{
	STATUS			status;
	PINSTANCE_DATA	Id;
#if defined(FCP_DEBUG) && defined(_DEBUG)
	U32				trace_level_save;
	
	trace_level_save = TraceLevel[TRACE_INDEX];		// can't use semaphores here
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;
#endif

	Id = &Instance_Data[2];

	// Check to see if we actually have an interrupt from the ISP.
	if(Read_ISP(Id, ISP_TO_PCI_INT_STATUS) & RISC_INT_PENDING)
	{
#if defined(FCP_DEBUG)	
		Id->FCP_flags |= 1;
		Id->Num_low_isr++;
#endif
	
    	// Clear RISC interrupt to host .
    	Write_ISP(Id, ISP_HCCR, HCTLCLRR2HINTR);
    
		status = NU_Activate_HISR(&Id->FCP_HISR);
		if (status != NU_SUCCESS)
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Low_2", 
				"NU_Activate_HISR failed",
				status,
				(UNSIGNED)Id);
	}
	
#if defined(FCP_DEBUG) && defined(_DEBUG)
	TraceLevel[TRACE_INDEX] = trace_level_save;
#endif
} // FCP_ISR_Low_2

#else	// _NAC

//=========================================================================
// The Eval systems can have two PCI boards, both on interrupt 0.  This
// causes a problem when we need to identify who caused the interrupt.  It
// also means we must have several interrupt handler versions to be able to
// handle the differences in instance number.  If we could pass a parameter
// to the ISR_High handler, things would be much simpler.
//=========================================================================

/*************************************************************************/
// FCP_ISR_High_Both
// High Level Interrupt Service Routine for both initiator and target
// gets activated by FCP_ISR_Low_Both.
/*************************************************************************/
void FCP_ISR_High_Both(VOID)
{
	
	U16				semaphore;
	U16				MB5;
	PINSTANCE_DATA	Id;
	U8				update;
	
 	FCP_TRACE_ENTRY(FCP_ISR_High_Both);
	
	Id = &Instance_Data[0];		// use the zeroth instance
	
#if defined(FCP_DEBUG)	
	Id->Num_high_isr_entry++;
#endif
	
	FCP_ASSERT(Id->FCP_instance == 0, "FCP_ISR_High_Both");

	// Test the semaphore lock bit.
	semaphore = Read_ISP(Id, ISP_SEMAPHORE);
	if(semaphore & 0x0001)

		// The semaphore lock bit is set.
		// The interrupt is the result of a mailbox command completion 
		// or an asynchronous event.
		// Activate the mailbox interrupt service routine.
		FCP_ISR_Mailbox(Id);
	
	update = FALSE;
	while ( (MB5 = Read_ISP(Id, ISP_MAILBOX_5)) != Id->FCP_response_FIFO_index)
	{
		FCP_ISR_Response_Queue(Id);
	    Id->FCP_response_FIFO_index++;
	    Id->FCP_response_FIFO_index %= Id->FCP_config.ISP_FIFO_response_size;
		update = TRUE;
	}
	if (update)
	    // Update response queue in pointer in mailbox 5.
    	Write_ISP(Id, ISP_MAILBOX_5, Id->FCP_response_FIFO_index);

	
	// Check to see if mailbox 5 has changed.
//    MB5 = Read_ISP(Id, ISP_MAILBOX_5);
//    if(Id->FCP_response_FIFO_index != MB5)
    	
    	// Mailbox 5 has changed.
    	// A new IOCB has been placed in the response FIFO.
//		FCP_ISR_Response_Queue(Id);
	
#if defined(FCP_DEBUG)	
	Id->FCP_flags &= ~1;	// clear in int handler flag
	Id->Num_high_isr++;
#endif
	
} // FCP_ISR_High_Both

/*************************************************************************/
// FCP_ISR_High_Target
// High Level Interrupt Service Routine
// gets activated by FCP_ISR_Low.
/*************************************************************************/
void FCP_ISR_High_Target(VOID)
{
	
	U16				semaphore;
	U16				MB5;
	PINSTANCE_DATA	Id;
	U8				update;
	
 	FCP_TRACE_ENTRY(FCP_ISR_High_Target);
	
	Id = &Instance_Data[TARGET_INSTANCE];
	
#if defined(FCP_DEBUG)	
	Id->Num_high_isr_entry++;
#endif
	
	FCP_ASSERT(Id->FCP_instance == TARGET_INSTANCE, "FCP_ISR_High_Target");

	// Test the semaphore lock bit.
	semaphore = Read_ISP(Id, ISP_SEMAPHORE);
	if(semaphore & 0x0001)

		// The semaphore lock bit is set.
		// The interrupt is the result of a mailbox command completion 
		// or an asynchronous event.
		// Activate the mailbox interrupt service routine.
		FCP_ISR_Mailbox(Id);
	
	update = FALSE;
	while ( (MB5 = Read_ISP(Id, ISP_MAILBOX_5)) != Id->FCP_response_FIFO_index)
	{
		FCP_ISR_Response_Queue(Id);
	    Id->FCP_response_FIFO_index++;
	    Id->FCP_response_FIFO_index %= Id->FCP_config.ISP_FIFO_response_size;
		update = TRUE;
	}
	if (update)
	    // Update response queue in pointer in mailbox 5.
    	Write_ISP(Id, ISP_MAILBOX_5, Id->FCP_response_FIFO_index);

	// Check to see if mailbox 5 has changed.
//    MB5 = Read_ISP(Id, ISP_MAILBOX_5);
//    if(Id->FCP_response_FIFO_index != MB5)
    	
    	// Mailbox 5 has changed.
    	// A new IOCB has been placed in the response FIFO.
//		FCP_ISR_Response_Queue(Id);
		
#if defined(FCP_DEBUG)	
	Id->FCP_flags &= ~1;	// clear in int handler flag
	Id->Num_high_isr++;
#endif
	
} // FCP_ISR_High_Target

/*************************************************************************/
// FCP_ISR_High_Initiator
// High Level Interrupt Service Routine
// gets activated by FCP_ISR_Low.
/*************************************************************************/
void FCP_ISR_High_Initiator(VOID)
{
	
	U16				semaphore;
	U16				MB5;
	PINSTANCE_DATA	Id;
	U8				update;
	
 	FCP_TRACE_ENTRY(FCP_ISR_High_Initiator);
	
	Id = &Instance_Data[INITIATOR_INSTANCE];
	
#if defined(FCP_DEBUG)	
	Id->Num_high_isr_entry++;
#endif
	
	FCP_ASSERT(Id->FCP_instance == INITIATOR_INSTANCE, "FCP_ISR_High_Initiator");

	// Test the semaphore lock bit.
	semaphore = Read_ISP(Id, ISP_SEMAPHORE);
	if(semaphore & 0x0001)

		// The semaphore lock bit is set.
		// The interrupt is the result of a mailbox command completion 
		// or an asynchronous event.
		// Activate the mailbox interrupt service routine.
		FCP_ISR_Mailbox(Id);
	
	update = FALSE;
	while ( (MB5 = Read_ISP(Id, ISP_MAILBOX_5)) != Id->FCP_response_FIFO_index)
	{
		FCP_ISR_Response_Queue(Id);
	    Id->FCP_response_FIFO_index++;
	    Id->FCP_response_FIFO_index %= Id->FCP_config.ISP_FIFO_response_size;
		update = TRUE;
	}
	if (update)
	    // Update response queue in pointer in mailbox 5.
    	Write_ISP(Id, ISP_MAILBOX_5, Id->FCP_response_FIFO_index);

	
	// Check to see if mailbox 5 has changed.
//    MB5 = Read_ISP(Id, ISP_MAILBOX_5);
//    if(Id->FCP_response_FIFO_index != MB5)
    	
    	// Mailbox 5 has changed.
    	// A new IOCB has been placed in the response FIFO.
//		FCP_ISR_Response_Queue(Id);
	
#if defined(FCP_DEBUG)	
	Id->FCP_flags &= ~1;	// clear in int handler flag
	Id->Num_high_isr++;
#endif
	
} // FCP_ISR_High_Initiator

/*************************************************************************/
// Low Level Interrupt Service Routine for the Target and Initiator
// Come here when a mailbox command is completed,
// the request queue has been updated, or
// an asynchronous event has occurred.
/*************************************************************************/
void FCP_ISR_Low(INT vector_number)
{
	STATUS			status;
	PINSTANCE_DATA	Id;
#if defined(FCP_DEBUG) && defined(_DEBUG)
	U32				trace_level_save;
	
	trace_level_save = TraceLevel[TRACE_INDEX];		// can't use semaphores here
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;
#endif

	Id = &Instance_Data[INITIATOR_INSTANCE];

	// Check to see if we actually have an interrupt from the target ISP.
	if(Read_ISP(Id, ISP_TO_PCI_INT_STATUS) & RISC_INT_PENDING)
	{
#if defined(FCP_DEBUG)	
		Id->FCP_flags |= 1;
		Id->Num_low_isr++;
#endif
	
    	// Clear RISC interrupt to host .
    	Write_ISP(Id, ISP_HCCR, HCTLCLRR2HINTR);
    
		status = NU_Activate_HISR(&Id->FCP_HISR);
		if (status != NU_SUCCESS)
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Low", 
				"NU_Activate_HISR failed",
				status,
				(UNSIGNED)Id);
	}

	Id = &Instance_Data[TARGET_INSTANCE];
	
	// Check to see if we actually have an interrupt from the Initiator ISP.
	if(Read_ISP(Id, ISP_TO_PCI_INT_STATUS) & RISC_INT_PENDING)
	{
#if defined(FCP_DEBUG)	
		Id->FCP_flags |= 1;
		Id->Num_low_isr++;
#endif
	
    	// Clear RISC interrupt to host .
    	Write_ISP(Id, ISP_HCCR, HCTLCLRR2HINTR);
    
		status = NU_Activate_HISR(&Id->FCP_HISR);
		if (status != NU_SUCCESS)
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Low", 
				"NU_Activate_HISR failed",
				status,
				(UNSIGNED)Id);
	}
	
#if defined(FCP_DEBUG) && defined(_DEBUG)
	TraceLevel[TRACE_INDEX] = trace_level_save;
#endif
} // FCP_ISR_Low

/*************************************************************************/
// Low Level Interrupt Service Routine for the Target and Initiator
// Come here when a mailbox command is completed,
// the request queue has been updated, or
// an asynchronous event has occurred.
/*************************************************************************/
void FCP_ISR_Low_Both(INT vector_number)
{
	STATUS			status;
	PINSTANCE_DATA	Id;
#if defined(FCP_DEBUG) && defined(_DEBUG)
	U32				trace_level_save;
	
	trace_level_save = TraceLevel[TRACE_INDEX];		// can't use semaphores here
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;
#endif

	Id = &Instance_Data[0];

	// Check to see if we actually have an interrupt from the ISP.
	if(Read_ISP(Id, ISP_TO_PCI_INT_STATUS) & RISC_INT_PENDING)
	{
#if defined(FCP_DEBUG)	
		Id->FCP_flags |= 1;
		Id->Num_low_isr++;
#endif
	
    	// Clear RISC interrupt to host .
    	Write_ISP(Id, ISP_HCCR, HCTLCLRR2HINTR);
    
		status = NU_Activate_HISR(&Id->FCP_HISR);
		if (status != NU_SUCCESS)
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Low_Both", 
				"NU_Activate_HISR failed",
				status,
				(UNSIGNED)Id);
	}
	
#if defined(FCP_DEBUG) && defined(_DEBUG)
	TraceLevel[TRACE_INDEX] = trace_level_save;
#endif
} // FCP_ISR_Low_Both

#endif	// _NAC



/*************************************************************************/
// FCP_ISR_Mailbox
// Come here when a mailbox command has completed or an asynchronous
// mailbox message has been received.
/*************************************************************************/
void FCP_ISR_Mailbox(PINSTANCE_DATA Id)
{
	FCP_EVENT_CONTEXT	*p_context;
	STATUS 			 	 status;

 	FCP_TRACE_ENTRY(FCP_ISR_Mailbox);
	
	// Save the contents of outgoing mailbox register 0.
	Id->FCP_mailbox_message.mailbox[0] = Read_ISP(Id, ISP_MAILBOX_0);
		
	// Check for an asynchronous event.
    switch(Id->FCP_mailbox_message.mailbox[0])
    {
        case 0x8001:
        
        	// Asynchronous event Reset detected
	        FCP_Handle_Async_Event(Id, FCP_ACTION_HANDLE_OTHER_AE,
	        			Id->FCP_mailbox_message.mailbox[0]);
	        
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Mailbox", 
				"Asynchronous event Reset detected",
				0,
				Id->FCP_mailbox_message.mailbox[0]);
	        goto Clear_Semaphore_Lock;
	        
        case 0x8002:
        
        	// Asynchronous event System Error detected
        	// usually get this error when an address error occurs,
        	// not a physical address, not mapped memory
			// Save the contents of the outgoing mailbox registers for Debug
			Id->FCP_mailbox_message.mailbox[1] = Read_ISP(Id, ISP_MAILBOX_1);
			Id->FCP_mailbox_message.mailbox[2] = Read_ISP(Id, ISP_MAILBOX_2);
			Id->FCP_mailbox_message.mailbox[3] = Read_ISP(Id, ISP_MAILBOX_3);
			Id->FCP_mailbox_message.mailbox[4] = Read_ISP(Id, ISP_MAILBOX_4);
			Id->FCP_mailbox_message.mailbox[5] = Read_ISP(Id, ISP_MAILBOX_5);
			Id->FCP_mailbox_message.mailbox[6] = Read_ISP(Id, ISP_MAILBOX_6);
			Id->FCP_mailbox_message.mailbox[7] = Read_ISP(Id, ISP_MAILBOX_7);
			
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Mailbox", 
				"Asynchronous event System Error detected",
				0,
				Id->FCP_mailbox_message.mailbox[0]);
	        goto Clear_Semaphore_Lock;
	        
        case 0x8003:
        
        	// Asynchronous event Request Transfer error
	        FCP_Handle_Async_Event(Id, FCP_ACTION_HANDLE_OTHER_AE,
	        			Id->FCP_mailbox_message.mailbox[0]);
	        
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Mailbox", 
				"Asynchronous event Request Transfer error detected",
				0,
				Id->FCP_mailbox_message.mailbox[0]);
	        goto Clear_Semaphore_Lock;
	        
        case 0x8004:
        
        	// Asynchronous event Response Transfer error
	        FCP_Handle_Async_Event(Id, FCP_ACTION_HANDLE_OTHER_AE,
	        			Id->FCP_mailbox_message.mailbox[0]);
	        
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Mailbox", 
				"Asynchronous event Response Transfer error detected",
				0,
				Id->FCP_mailbox_message.mailbox[0]);
	        goto Clear_Semaphore_Lock;
	        
        case 0x8005:
        
        	// Asynchronous event Request queue wakeup
        	// Not currently using this feature
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Mailbox", 
				"Asynchronous event Request queue wakeup detected",
				0,
				Id->FCP_mailbox_message.mailbox[0]);
	        goto Clear_Semaphore_Lock;
	        
        case 0x8010:
        
        	// Asynchronous event LIP occurred
        	// Firmware has completed the Loop Initialization Procedure
	        FCP_Handle_Async_Event(Id, FCP_ACTION_HANDLE_LIP,
	        			Id->FCP_mailbox_message.mailbox[0]);

	        goto Clear_Semaphore_Lock;
	        
        case 0x8011:
        
        	// Asynchronous event FC Loop Up
        	// The firmware has completed the process login.
	        status = NU_Release_Semaphore(&Id->FCP_Loop_Sema);
	        
	        FCP_Handle_Async_Event(Id, FCP_ACTION_HANDLE_LOOP_UP,
	        			Id->FCP_mailbox_message.mailbox[0]);

	        goto Clear_Semaphore_Lock;
	        
        case 0x8012:
        
        	// Asynchronous event FC Loop Down
	        
	        // handle Loop Down
	        FCP_Handle_Async_Event(Id, FCP_ACTION_HANDLE_LOOP_DOWN,
	        			Id->FCP_mailbox_message.mailbox[0]);
	        
	        goto Clear_Semaphore_Lock;
	        
        case 0x8013:
        
        	// Asynchronous event LIP Reset occurred
        	// LIP Reset Primitive received
	        FCP_Handle_Async_Event(Id, FCP_ACTION_HANDLE_LIP_RESET,
	        			Id->FCP_mailbox_message.mailbox[0]);
	        
	        goto Clear_Semaphore_Lock;
	        
        case 0x8014:
        
        	// Asynchronous event Port Database Changed occurred
	        FCP_Handle_Async_Event(Id, FCP_ACTION_HANDLE_OTHER_AE,
	        			Id->FCP_mailbox_message.mailbox[0]);
	        
	        goto Clear_Semaphore_Lock;
	        
        case 0x8015:
        
        	// Asynchronous event Change Notification occurred
        	// Change in the Nameserver Database
	        FCP_Handle_Async_Event(Id, FCP_ACTION_HANDLE_OTHER_AE,
	        			Id->FCP_mailbox_message.mailbox[0]);
	        
	        goto Clear_Semaphore_Lock;
	        
       case 0x8020:
       
         	// Asynchronous event SCSI command Complete
         	// Send a message to the task that is waiting
         	
       case 0x8021:
       
         	// Asynchronous event CTIO Complete.
         	// Send a message to the task that is waiting

#if defined(FCP_DEBUG)	
			// save these values for DEBUG later
			Id->FCP_mailbox_message.mailbox[1] = Read_ISP(Id, ISP_MAILBOX_1);
			Id->FCP_mailbox_message.mailbox[2] = Read_ISP(Id, ISP_MAILBOX_2);
#endif

         	// Get pointer to context for waiting task.
         	
#if 0		// NOTE: This code worked prior to the V3 IDE...
         	p_context = (FCP_EVENT_CONTEXT*)(UNSIGNED)
         				(BYTE_SWAP16(Read_ISP(Id, ISP_MAILBOX_2)) |
         				BYTE_SWAP16(Read_ISP(Id, ISP_MAILBOX_1)) << 16);
#else
         	p_context = (FCP_EVENT_CONTEXT*)(UNSIGNED)
         				(*(U16 *)((UNSIGNED)Id->Regs + ISP_MAILBOX_2) |
         				*(U16 *)((UNSIGNED)Id->Regs + ISP_MAILBOX_1) << 16);
#endif
			
			FCP_PRINT_HEX(TRACE_L8, "\n\rFast Post context = ", (U32)p_context);
			
			// Clear the semaphore lock.
			// This lets the RISC know that the outgoing mailbox registers
			// are now available.
			Write_ISP(Id, ISP_SEMAPHORE, 0);
			
			// The command completed successfully, so set the status
			// in the status IOCB.
			if (Id->FCP_mailbox_message.mailbox[0] == 0x8021)
				((PIOCB_CTIO_TYPE_2) &p_context->status_iocb)->status = 
									STATUS_REQUEST_COMPLETE;
			else
			{
				p_context->status_iocb.status = 0;
				p_context->status_iocb.SCSI_status = SCSI_STATUS_GOOD;
				p_context->status_iocb.state_flags = 
									BYTE_SWAP16(IOCB_STATE_FLAGS_XFER_COMPLETE);
			}
	
			FCP_ASSERT(Id == p_context->Id, "FCP_ISR_Mailbox Fast Post");
			
			// Send a message to FCP_Event_Task.
			// The action field of the context will tell FCP_Event_Task
			// what to do next. 
    		status = NU_Send_To_Queue(&Id->FCP_event_queue, 
    			&p_context, // message is pointer to context
        		1, // size is one UNSIGNED 
        		NU_NO_SUSPEND);
				
         	if (status != NU_SUCCESS)
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_ISR_Mailbox", 
					"NU_Send_To_Queue failed",
					status,
					(UNSIGNED)Id);
         	 
	        return;

       case 0x8042:
       
         	// Asynchronous event 2 SCSI Commands Complete.
         	// Send a message to the tasks that are waiting

         	// Get pointer to context for 1st command for waiting task.
         	p_context = (FCP_EVENT_CONTEXT*)(UNSIGNED)
         				(*(U16 *)((UNSIGNED)Id->Regs + ISP_MAILBOX_2) |
         				*(U16 *)((UNSIGNED)Id->Regs + ISP_MAILBOX_1) << 16);
			
			FCP_PRINT_HEX(TRACE_L8, "\n\rRIO 2-32 context = ", (U32)p_context);
			
			// The command completed successfully, so set the status
			// in the status IOCB.
			p_context->status_iocb.status = 0;
			p_context->status_iocb.SCSI_status = SCSI_STATUS_GOOD;
			p_context->status_iocb.state_flags = 
								BYTE_SWAP16(IOCB_STATE_FLAGS_XFER_COMPLETE);
			
			// Send a message to FCP_Event_Task.
			// The action field of the context will tell FCP_Event_Task
			// what to do next. 
    		status = NU_Send_To_Queue(&Id->FCP_event_queue, 
    			&p_context, // message is pointer to context
        		1, // size is one UNSIGNED 
        		NU_NO_SUSPEND);
				
         	if (status != NU_SUCCESS)
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_ISR_Mailbox", 
					"NU_Send_To_Queue failed",
					status,
					(UNSIGNED)Id);

         	// Now get pointer to context for 2nd command for waiting task.
         	p_context = (FCP_EVENT_CONTEXT*)(UNSIGNED)
         				(*(U16 *)((UNSIGNED)Id->Regs + ISP_MAILBOX_7) |
         				*(U16 *)((UNSIGNED)Id->Regs + ISP_MAILBOX_6) << 16);
			
			FCP_PRINT_HEX(TRACE_L8, "\n\rRIO 2-32 context = ", (U32)p_context);
			
			// Clear the semaphore lock.
			// This lets the RISC know that the outgoing mailbox registers
			// are now available.
			Write_ISP(Id, ISP_SEMAPHORE, 0);
			
			// The command completed successfully, so set the status
			// in the status IOCB.
			p_context->status_iocb.status = 0;
			p_context->status_iocb.SCSI_status = SCSI_STATUS_GOOD;
			p_context->status_iocb.state_flags = 
								BYTE_SWAP16(IOCB_STATE_FLAGS_XFER_COMPLETE);
			
			// Send a message to FCP_Event_Task.
			// The action field of the context will tell FCP_Event_Task
			// what to do next. 
    		status = NU_Send_To_Queue(&Id->FCP_event_queue, 
    			&p_context, // message is pointer to context
        		1, // size is one UNSIGNED 
        		NU_NO_SUSPEND);
				
         	if (status != NU_SUCCESS)
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_ISR_Mailbox", 
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
	Id->FCP_mailbox_message.mailbox[1] = Read_ISP(Id, ISP_MAILBOX_1);
	Id->FCP_mailbox_message.mailbox[2] = Read_ISP(Id, ISP_MAILBOX_2);
	Id->FCP_mailbox_message.mailbox[3] = Read_ISP(Id, ISP_MAILBOX_3);
	
	// Clear the semaphore lock.
	// This lets the RISC know that the outgoing mailbox registers
	// are now available.
	Write_ISP(Id, ISP_SEMAPHORE, 0);
	
	FCP_PRINT_STRING(TRACE_L8, "\n\rMB Command Complete");
	
	// Send contents of mailboxes to waiting task.
    status = NU_Send_To_Queue(&Id->FCP_mailbox_queue, 
    	&Id->FCP_mailbox_message, 
        sizeof(FCP_MAILBOX_MESSAGE) / sizeof(UNSIGNED), // size is an entire MB struct
        NU_NO_SUSPEND);
				
    if (status != NU_SUCCESS)
    {
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_ISR_Mailbox", 
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
	Write_ISP(Id, ISP_SEMAPHORE, 0);
	
} // FCP_ISR_Mailbox

/*************************************************************************/
// FCP_ISR_Response_Queue
// Come here when a new IOCB has been placed in the response queue.
// This IOCB could be a response to a request IOCB, or it could be
// a new command to be completed in target mode.
/*************************************************************************/
void FCP_ISR_Response_Queue(PINSTANCE_DATA Id)
{
	FCP_EVENT_CONTEXT	*p_context;
	IOCB_ATIO_TYPE_2	*p_IOCB;
	IOCB_RIO_TYPE1		*p_RIO;
	IOCB_STATUS_TYPE0	*pStatusIOCB ;
	STATUS				 status;
	U8					i, count;
	
 	FCP_TRACE_ENTRY(FCP_ISR_Response_Queue);
	
	// Point to IOCB in response FIFO
	p_IOCB = (IOCB_ATIO_TYPE_2*)
		&(Id->FCP_p_IOCB_response_FIFO[Id->FCP_response_FIFO_index]);
	
	// decide what to do with the IOCB based on the entry_type
	switch(p_IOCB->entry_type) {
	
	case	IOCB_TYPE_ACCEPT_TARGET_IO_TYPE_2:
		// This is a new ATIO IOCB.
		FCP_PRINT_STRING(TRACE_L7, "\n\rNew ATIO IOCB");
		
		// Allocate an FCP_EVENT_CONTEXT from the pool.
		status = NU_Allocate_Partition(&Id->FCP_event_context_pool, 
    		(VOID**)&p_context, NU_NO_SUSPEND);
    	if (status != NU_SUCCESS)
    	{
    		// TODO
    		// If we can't allocate an event context, 
    		// we can't handle this interrupt!
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Response_Queue", 
				"NU_Allocate_Partition for event context failed",
				status,
				(UNSIGNED)Id);
			return;
		}

#if defined(FCP_DEBUG)	
    	// DEBUG - save the context with the ATIO so we can find it later
    	p_IOCB->system_defined2 = (UNSIGNED)p_context;
#endif

    	// Copy IOCB from response FIFO to iocb of context
//	   	Mem_Copy((char*)&p_context->iocb, // to
//    		(char*)p_IOCB, // from
//    		IOCB_SIZE); // number of bytes 
    		  
		// src and dest need to be 8 byte aligned		
    	bcopy64((char*)p_IOCB, // from
    		(char*)&p_context->iocb, // to    		
    		IOCB_SIZE); // number of bytes 

    	// Zero the rest of the context (assumes iocb first)
//		Mem_Set((void*)((UNSIGNED)p_context + IOCB_SIZE), 0,
//			sizeof(FCP_EVENT_CONTEXT) - IOCB_SIZE);
			
    	// Zero the rest of the context (assumes iocb first)
    	// must be 8 byte aligned
		bzero64((void*)((UNSIGNED)p_context + IOCB_SIZE),
			sizeof(FCP_EVENT_CONTEXT) - IOCB_SIZE);

		
		// Allocate a Master Status structure since some commands
		// will require more than one context for caching
		pStatusIOCB = (IOCB_STATUS_TYPE0 *) FCP_Alloc((tSMALL|tUNCACHED),IOCB_SIZE);

		// Zero Master Status structure
//		Mem_Set((void*)pStatusIOCB, 0,IOCB_SIZE);

		// must be 8 byte aligned
		bzero64((void*)pStatusIOCB, IOCB_SIZE);


		// Set ptr to Master Status structure
		p_context->pStatusIOCB = pStatusIOCB;

		// Set residual to transfer length - subtract from it as each context finishes
		pStatusIOCB->residual_transfer_length = BYTE_SWAP32(p_IOCB->transfer_length);

		// Set the instance pointer
		p_context->Id = Id;
		
    	// Set action to perform
    	p_context->action = TARGET_ACTION_HANDLE_NEW_IOCB;
		break;
		
	case	IOCB_TYPE_IMMEDIATE_NOTIFY:
	
		// This is a new IMMEDIATE_NOTIFY IOCB. Unsolicited Type
		FCP_PRINT_STRING(TRACE_L7, "\n\rImmediate Notify IOCB");
		
		// Allocate an FCP_EVENT_CONTEXT from the pool.
		status = NU_Allocate_Partition(&Id->FCP_event_context_pool, 
    		(VOID**)&p_context, NU_NO_SUSPEND);
    	if (status != NU_SUCCESS)
    	{
    		// TODO
    		// If we can't allocate an event context, 
    		// we can't handle this interrupt!
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Response_Queue", 
				"NU_Allocate_Partition for event context failed",
				status,
				(UNSIGNED)Id);
			return;
		}
#if defined(FCP_DEBUG)	
    	// DEBUG - save the context with the IOCB so we can find it later
    	p_IOCB->system_defined2 = (UNSIGNED)p_context;
#endif

    	// Copy IOCB from response FIFO to iocb of context
//    	Mem_Copy((char*)&p_context->iocb, // to
//    		(char*)p_IOCB, // from
//    		IOCB_SIZE); // number of bytes 
    		  
    	// must be 8 byte aligned		
	   	bcopy64((char*)p_IOCB, // from
	   		(char*)&p_context->iocb, // to
    		IOCB_SIZE); // number of bytes 
    		
    	// Zero the rest of the context (assumes iocb first)
//		Mem_Set((void*)((UNSIGNED)p_context + IOCB_SIZE), 0,
//			sizeof(FCP_EVENT_CONTEXT) - IOCB_SIZE);
			
		// must be 8 byte aligned
		bzero64((void*)((UNSIGNED)p_context + IOCB_SIZE),
			sizeof(FCP_EVENT_CONTEXT) - IOCB_SIZE);
		    	
		// Set the instance pointer
		p_context->Id = Id;
		
    	// Set action to perform
    	p_context->action = FCP_ACTION_HANDLE_IMMEDIATE_NOTIFY;
		break;
		
	case	IOCB_TYPE_MARKER:
		FCP_PRINT_STRING(TRACE_L7, "\n\rMarker IOCB");
FCP_Response_FIFO_Increment_Index(Id);
		return;
		
	case	IOCB_TYPE_NOTIFY_ACKNOWLEDGE:
		FCP_PRINT_STRING(TRACE_L7, "\n\rNotify ACK IOCB");
		
		p_context = (FCP_EVENT_CONTEXT*)p_IOCB->system_defined2;
		if (p_context == (FCP_EVENT_CONTEXT *)0) 
		{
    		// If we can't find an event context, 
    		// we can't handle this interrupt!
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Response_Queue", 
				"No event context for Response IOCB",
				0,
				(UNSIGNED)Id);
			return;
		}
		
		// need to return the context back to the pool
	    // Deallocate the FCP_EVENT_CONTEXT 
	    // allocated by FCP_ISR_Response_Queue
	    status = NU_Deallocate_Partition(p_context);

	    if (status != NU_SUCCESS)
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"IOCB_TYPE_NOTIFY_ACKNOWLEDGE", 
				"NU_Deallocate_Partition for context failed",
				status,
				(UNSIGNED)p_context);
					
FCP_Response_FIFO_Increment_Index(Id);
		return;
		
	case	IOCB_TYPE_ENABLE_LUN:
	{
 		IOCB_ENABLE_LUN		*p_response;
 		
		FCP_PRINT_STRING(TRACE_L7, "\n\rEnable Lun IOCB");
		
		p_response = (IOCB_ENABLE_LUN*)
			&(Id->FCP_p_IOCB_response_FIFO[Id->FCP_response_FIFO_index]);
					
		// check to see that the enable LUNs completed OK
		if (p_response->entry_status || p_response->status != 0x01) {
			
			// already enabled is OK
			if (p_response->status != 0x3E) {
		    	(void)Read_ISP(Id, ISP_MAILBOX_0); // DEBUG
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_Enable_LUNs", 
					"Enable LUN failed",
					(U16)p_response->status,
					(UNSIGNED)Id);
				
				return;
			}
			else
			{
				FCP_PRINT_STRING(TRACE_L7, " Already Enabled!");
			}
		}
		
		// This IOCB does not require any other processing
FCP_Response_FIFO_Increment_Index(Id);
	}
		return;
		
	case	IOCB_TYPE_REPORT_ID_ACQ:
	    FCP_DUMP_HEX(TRACE_L8, "\n\rIOCB_TYPE_REPORT_ID_ACQ IOCB ",
	    					(U8 *)p_IOCB,
	    					sizeof(IOCB_REPORT_ID_ACQ));

FCP_Response_FIFO_Increment_Index(Id);
		return;
		
	case	IOCB_TYPE_VP_CTL:
	case	IOCB_TYPE_MODIFY_PORT_CFG:
	case	IOCB_TYPE_ABORT_OPEN_XCHG:
	
		// Handle Multi-Target response IOCBs
		FCP_PRINT_STRING(TRACE_L7, "\n\rLoop Control IOCB");
		
	case	IOCB_TYPE_CONTINUE_TARGET_IO_TYPE_2:
	case	IOCB_TYPE_CONTINUE_TARGET_IO_TYPE_4:
		FCP_PRINT_STRING(TRACE_L7, "\n\rContinue Target IO Type 2 IOCB");
		
	case	IOCB_TYPE_STATUS_TYPE_0:
		FCP_PRINT_STRING(TRACE_L7, "\n\rStatus Type 0 IOCB");

	    // If this IOCB is the result of a request that completed, then
	    // system_defined2 points to a context.
		p_context = (FCP_EVENT_CONTEXT*)p_IOCB->system_defined2;
		if (p_context == (FCP_EVENT_CONTEXT *)0) 
		{
    		// If we can't find an event context, 
    		// we can't handle this interrupt!
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISR_Response_Queue", 
				"No event context for Response IOCB",
				0,
				(UNSIGNED)Id);
			return;
		}
		
    	// Copy IOCB from response FIFO to status_iocb of context
//    	Mem_Copy((char*)&p_context->status_iocb, // to
//    		(char*)p_IOCB, // from
//    		IOCB_SIZE); // number of bytes   		

		// must be 8 byte aligned
		bcopy64((char*)p_IOCB, // from
			(char*)&p_context->status_iocb, // to
  		IOCB_SIZE); // number of bytes   		
    
		break;
				
	case	IOCB_TYPE_RIO_TYPE_1:
			p_RIO = (IOCB_RIO_TYPE1*)
					&(Id->FCP_p_IOCB_response_FIFO[Id->FCP_response_FIFO_index]);
			count = p_RIO->handle_count;
			for (i = 0; i < count; i++)
			{
				p_context = (FCP_EVENT_CONTEXT*)p_RIO->handle[i];
				// The command completed successfully, so set the status
				// in the status IOCB.
				p_context->status_iocb.status = 0;
				p_context->status_iocb.SCSI_status = SCSI_STATUS_GOOD;
				p_context->status_iocb.state_flags = 
									BYTE_SWAP16(IOCB_STATE_FLAGS_XFER_COMPLETE);
			
				// Send a message to FCP_Event_Task.
				// The action field of the context will tell FCP_Event_Task
				// what to do next. 
    			status = NU_Send_To_Queue(&Id->FCP_event_queue, 
    				&p_context, // message is pointer to context
        			1, // size is one UNSIGNED 
        			NU_NO_SUSPEND);
				
	         	if (status != NU_SUCCESS)
					FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
						"FCP_ISR_Mailbox", 
						"NU_Send_To_Queue failed",
						status,
						(UNSIGNED)Id);
			}
FCP_Response_FIFO_Increment_Index(Id);
			return;

	default:
		// If we don't know what the type is,
		// we can't handle this interrupt!
	    FCP_DUMP_HEX(TRACE_L8, "\n\rEntry Data ",
	    					(U8 *)p_IOCB,
	    					sizeof(IOCB_ATIO_TYPE_2));

		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_ISR_Response_Queue", 
			"Unknown entry type",
			p_IOCB->entry_type,
			(UNSIGNED)Id);
		return;
	}
	
	// Increment the index when we have processed this IOCB.
FCP_Response_FIFO_Increment_Index(Id);

	// Send a message to FCP_Event_Task.
	// The action field of the context will tell FCP_Event_Task
	// what to do next. 
    status = NU_Send_To_Queue(&Id->FCP_event_queue, 
    	&p_context, // message is pointer to context
        1, // size is one UNSIGNED 
        NU_NO_SUSPEND);
        	
    if (status != NU_SUCCESS)
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_ISR_Response_Queue", 
			"NU_Send_To_Queue for request completion failed",
			status,
			(UNSIGNED)Id);

} // FCP_ISR_Response_Queue

/*************************************************************************/
// FCP_Handle_Immediate_Notify
// Send a Notify Acknowledge IOCB back
/*************************************************************************/
STATUS FCP_Handle_Immediate_Notify(FCP_EVENT_CONTEXT *p_context)
{
	STATUS							status = NU_SUCCESS;
	IOCB_NOTIFY_ACKNOWLEDGE			*p_ack;
	IOCB_IMMEDIATE_NOTIFY			*p_IOCB = 
								(IOCB_IMMEDIATE_NOTIFY *)&p_context->iocb;
	PINSTANCE_DATA					 Id = p_context->Id;

 	FCP_TRACE_ENTRY(FCP_Handle_Immediate_Notify);
 	
    // Get pointer to next IOCB in ISP request FIFO.
	p_ack = (IOCB_NOTIFY_ACKNOWLEDGE*)FCP_Request_FIFO_Get_Pointer(Id);
	
	// Move Immediate Notify IOCB into Notify Ack IOCB
	// so the flags and sequenceIdentifier are correct
//	Mem_Copy(p_ack,			// dest
//			p_IOCB,			// src
//			IOCB_SIZE);

	// must be 8 byte aligned
	bcopy64((char *)p_IOCB,			// src
			(char *)p_ack,			// dest
			IOCB_SIZE);
	    
	// Set up Command fields
    p_ack->entry_type = IOCB_TYPE_NOTIFY_ACKNOWLEDGE;
 //   p_ack->status |= BYTE_SWAP16(ISP_ACK_FLAGS_INC_NOTIFY_RES_CNT);
    
    switch (p_IOCB->status)
    {
    	case	STATUS_LIP_RESET:
	    	// return the Clear LIP Reset Event flag (bit 5)
	    	p_ack->flags = BYTE_SWAP16(ISP_ACK_FLAGS_CLEAR_LIP_RESET);
	    	FCP_PRINT_STRING(TRACE_L7, "\n\rLIP_RESET");
    		break;
    		
    	case	STATUS_MESSAGE_RECEIVED:
    		// the task_flags are valid when this status is received
    		// TODO:
    		// Handle all the flags correctly, for now just show the
    		// TARGET_RESET flags is in.  We should send an I2O_SCB_RESET 
    		// message to all the pieces of code downstream
    		if (p_IOCB->task_flags & ISP_TASK_FLAGS_TARGET_RESET)
    		{
	    		FCP_PRINT_STRING(TRACE_L7, "\n\rTARGET_RESET");
    		}
    		break;
  
    	default:
    		// TODO:
    		// handle all cases
    		break;
    }

    // Send the IOCB on its way
    FCP_Request_FIFO_Update_Index(Id);
    
	return status;
} // FCP_Handle_Immediate_Notify

/*************************************************************************/
// FCP_Handle_Async_Event
// Send a message to the event queue, this event in turn will call the
// AE Handler method defined for the DDM handling this library.
// A new context is allocated to service the AE, which is deallocated
// in the AE Handler for  the DDM.
/*************************************************************************/
void FCP_Handle_Async_Event(PINSTANCE_DATA Id, FCP_EVENT_ACTION action, U16 event_code)
{
	FCP_EVENT_CONTEXT	*p_context;
	STATUS				 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Handle_Async_Event);
 	
	// Allocate an FCP_EVENT_CONTEXT from the pool.
	status = NU_Allocate_Partition(&Id->FCP_event_context_pool, 
		(VOID**)&p_context, NU_NO_SUSPEND);

	if (status != NU_SUCCESS)
	{
		// TODO
		// If we can't allocate an event context, 
		// we can't handle this action!
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Handle_Async_Event", 
			"NU_Allocate_Partition for event context failed",
			status,
			(UNSIGNED)Id);
		return;
	}
    p_context->action = action;
    p_context->Id = Id;
    
    // CTIO_flags overloaded to save the event code
    p_context->CTIO_flags = event_code;  
    
	// Send a message to FCP_Event_Task.
	// The action field of the context will tell FCP_Event_Task
	// what to do next. 
    status = NU_Send_To_Queue(&Id->FCP_event_queue, 
    	&p_context, // message is pointer to context
        1, // size is one UNSIGNED 
        NU_NO_SUSPEND);
        	
    if (status != NU_SUCCESS)
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Handle_Async_Event", 
			"NU_Send_To_Queue for request completion failed",
			status,
			(UNSIGNED)Id);

	return;
} // FCP_Handle_Async_Event
    
