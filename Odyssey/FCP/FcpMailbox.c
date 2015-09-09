/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpMailbox.c
// 
// Description:
// This file implements interfaces to the ISP mailbox.
// 
// Update Log 
// 5/18/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 10/6/98 Michael G. Panas: changes to support interrupt driven mailbox commands
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/

#include "FcpCommon.h"
#include "FcpMailbox.h"
#include "FcpMemory.h"
#include "OsTypes.h"

/*************************************************************************/
// Forward References
/*************************************************************************/

/*************************************************************************/
// FCP_Mailbox_Create
// Create FCP_Mailbox object. 
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS FCP_Mailbox_Create(PINSTANCE_DATA Id)
{
	VOID			*p_memory;
	STATUS			 status = NU_SUCCESS;

 	FCP_TRACE_ENTRY(FCP_Mailbox_Create);
			
	// get memory for the mailbox queue
//	p_memory = FCP_Alloc((tSMALL|tUNCACHED), Id->FCP_config.mb_queue_size *
//					sizeof(FCP_MAILBOX_MESSAGE));
	p_memory = FCP_Alloc((tSMALL), Id->FCP_config.mb_queue_size *
					sizeof(FCP_MAILBOX_MESSAGE));

	if ((Id->FCP_config.enable_initiator_mode) &&
				(Id->FCP_config.enable_target_mode))
	{
		// both Initiator and Target mode (create only one)
		// Create FCP_mailbox_queue
		// This Nucleus queue contains FCP_MAILBOX_MESSAGEs.
		// Each FCP_EVENT_CONTEXT represents work to be done.
		status = NU_Create_Queue(&Id->FCP_mailbox_queue, "MBQ",
			p_memory,			// starting address 
			Id->FCP_config.mb_queue_size, // # of elements in the Q
			NU_FIXED_SIZE,		// each entry is a pointer to a task
			sizeof(FCP_MAILBOX_MESSAGE) / sizeof(UNSIGNED), // # UNSIGNED elements per entry (size of a MB)
			NU_FIFO);
		
	}
	else if (Id->FCP_config.enable_target_mode)
	{
		// Create FCP_mailbox_queue
		// This Nucleus queue contains FCP_MAILBOX_MESSAGEs.
		// Each FCP_EVENT_CONTEXT represents work to be done.
		status = NU_Create_Queue(&Id->FCP_mailbox_queue, "MBQ_TGT",
			p_memory,			// starting address 
			Id->FCP_config.mb_queue_size, // # of elements in the Q
			NU_FIXED_SIZE,		// each entry is a pointer to a task
			sizeof(FCP_MAILBOX_MESSAGE) / sizeof(UNSIGNED), // # UNSIGNED elements per entry (size of a MB)
			NU_FIFO);
	}
	else if (Id->FCP_config.enable_initiator_mode)
	{
		// Create FCP_mailbox_queue
		// This Nucleus queue contains FCP_MAILBOX_MESSAGEs.
		// Each FCP_EVENT_CONTEXT represents work to be done.
		status = NU_Create_Queue(&Id->FCP_mailbox_queue, "MBQ_INIT",
			p_memory,			// starting address 
			Id->FCP_config.mb_queue_size, // # of elements in the Q
			NU_FIXED_SIZE,		// each entry is a pointer to a task
			sizeof(FCP_MAILBOX_MESSAGE) / sizeof(UNSIGNED), // # UNSIGNED elements per entry (size of a MB)
			NU_FIFO);
	}
	else
	{
		// must have at least one mode
		FCP_Free(p_memory);
		
		status = FCP_ERROR_MAILBOX_SETUP_FAILED;
	}
	
	if (status != NU_SUCCESS)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Mailbox_Create", 
			"NU_Create_Queue for FCP_event_queue failed",
			status,
			0);
		return status;
	}
		
	// Save pointer to mailbox queue memory (in case we deallocate later).
	Id->FCP_p_mb_queue = p_memory;
	
	return status;
		
} // FCP_Mailbox_Create

/*************************************************************************/
// FCP_Mailbox_Destroy
// Initialize FCP_Mailbox object. 
/*************************************************************************/
void FCP_Mailbox_Destroy()
{
	// TODO
	
 	FCP_TRACE_ENTRY(FCP_Mailbox_Destroy);
			
} // FCP_Mailbox_Destroy


/*************************************************************************/
// FCP_Mailbox_Command
// Execute a mailbox command, wait for interrupt to complete command
/*************************************************************************/
U16 FCP_Mailbox_Command(PINSTANCE_DATA Id)
{
	FCP_MAILBOX_MESSAGE	mb;
	STATUS				status;
	UNSIGNED			actual_size;
	
 	FCP_TRACE_ENTRY(FCP_Mailbox_Command);
		
	// Interrupt RISC to let it know we have a new mailbox command
	Write_ISP(Id, ISP_HCCR, HCTLSETH2RINTR);
	
	// Wait for the mailbox command to complete.
	// nothing in the queue until the interrupt handler puts it there
	status = NU_Receive_From_Queue(&Id->FCP_mailbox_queue,
		&mb,		// Copy message to this pointer.
		sizeof(FCP_MAILBOX_MESSAGE) / sizeof(UNSIGNED),	// one mailbox_message data element in the message
		&actual_size, // receives actual size of message 
		NU_SUSPEND);

 	FCP_DUMP_HEX(TRACE_L8, "\n\rmb = ", (U8 *)&mb, sizeof(FCP_MAILBOX_MESSAGE));
		
	if (status != NU_SUCCESS)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Mailbox_Command", 
			"NU_Receive_From_Queue failed",
			status,
			(UNSIGNED)Id);
			
		return 0;
	}

	return mb.mailbox[0];
	
} // FCP_Mailbox_Command


/*************************************************************************/
// FCP_Mailbox_Wait_Ready_Intr
// Used when interrupts are enabled to insure that the ISP firmware is not
// executing a mailbox command
// by checking that the Host Interrupt bit of the Host Command and control 
// register is 0.  This insures that the host never overwrites the mailbox 
// registers containing an executing mailbox command.
/*************************************************************************/
STATUS FCP_Mailbox_Wait_Ready_Intr(PINSTANCE_DATA Id)
{
	UNSIGNED	wait_count;
	
 	FCP_TRACE_ENTRY(FCP_Mailbox_Wait_Ready_Intr);
			
	// Wait for ISP to clear host interrupt to RISC.  
	// Don't wait forever.
	wait_count = 10000;
	while(Read_ISP(Id, ISP_HCCR) & HCTLH2RINTR)
	{
		NU_Sleep(1);
		
		// wait for ISP to clear host interrupt to RISC
		if (wait_count-- == 0)
		{
			// We cannot send a mailbox command to the ISP.
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_Mailbox_Wait_Ready_Intr", 
				"Cannot send mailbox command to ISP",
				0,
				(UNSIGNED)Id);
			return FCP_ERROR_MAILBOX_TIMEOUT;
		}
	}
	
	return NU_SUCCESS;
	
} // FCP_Mailbox_Wait_Ready_Intr



