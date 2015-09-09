/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiMailbox.c
// 
// Description:
// This file implements interfaces to the ISP mailbox.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiMailbox.c $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#include "HscsiCommon.h"
#include "HscsiMailbox.h"
#include "HscsiMemory.h"
#include "OsTypes.h"

// #include "pcidev.h" // debug

/*************************************************************************/
// Forward References
/*************************************************************************/

/*************************************************************************/
// HSCSI_Mailbox_Create
// Create HSCSI_Mailbox object. 
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS HSCSI_Mailbox_Create(PHSCSI_INSTANCE_DATA Id)
{
	VOID			*p_memory;
	STATUS			 status = NU_SUCCESS;

 	HSCSI_TRACE_ENTRY(HSCSI_Mailbox_Create);
			
	// get memory for the mailbox queue
	p_memory = HSCSI_Alloc((tSMALL|tUNCACHED), Id->HSCSI_config.mb_queue_size *
					sizeof(HSCSI_MAILBOX_MESSAGE));

	if ((Id->HSCSI_config.enable_initiator_mode) &&
				(Id->HSCSI_config.enable_target_mode))
	{
		// both Initiator and Target mode (create only one)
		// Create HSCSI_mailbox_queue
		// This Nucleus queue contains HSCSI_MAILBOX_MESSAGEs.
		// Each HSCSI_EVENT_CONTEXT represents work to be done.
		status = NU_Create_Queue(&Id->HSCSI_mailbox_queue, "MBQ",
			p_memory,			// starting address 
			Id->HSCSI_config.mb_queue_size, // # of elements in the Q
			NU_FIXED_SIZE,		// each entry is a pointer to a task
			sizeof(HSCSI_MAILBOX_MESSAGE) / sizeof(UNSIGNED), // # UNSIGNED elements per entry (size of a MB)
			NU_FIFO);
		
	}
	else if (Id->HSCSI_config.enable_target_mode)
	{
		// Create HSCSI_mailbox_queue
		// This Nucleus queue contains HSCSI_MAILBOX_MESSAGEs.
		// Each HSCSI_EVENT_CONTEXT represents work to be done.
		status = NU_Create_Queue(&Id->HSCSI_mailbox_queue, "MBQ_TGT",
			p_memory,			// starting address 
			Id->HSCSI_config.mb_queue_size, // # of elements in the Q
			NU_FIXED_SIZE,		// each entry is a pointer to a task
			sizeof(HSCSI_MAILBOX_MESSAGE) / sizeof(UNSIGNED), // # UNSIGNED elements per entry (size of a MB)
			NU_FIFO);
	}
	else if (Id->HSCSI_config.enable_initiator_mode)
	{
		// Create HSCSI_mailbox_queue
		// This Nucleus queue contains HSCSI_MAILBOX_MESSAGEs.
		// Each HSCSI_EVENT_CONTEXT represents work to be done.
		status = NU_Create_Queue(&Id->HSCSI_mailbox_queue, "MBQ_INIT",
			p_memory,			// starting address 
			Id->HSCSI_config.mb_queue_size, // # of elements in the Q
			NU_FIXED_SIZE,		// each entry is a pointer to a task
			sizeof(HSCSI_MAILBOX_MESSAGE) / sizeof(UNSIGNED), // # UNSIGNED elements per entry (size of a MB)
			NU_FIFO);
	}
	else
	{
		// must have at least one mode
		HSCSI_Free(p_memory);
		
		status = HSCSI_ERROR_MAILBOX_SETUP_FAILED;
	}
	
	if (status != NU_SUCCESS)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Mailbox_Create", 
			"NU_Create_Queue for HSCSI_event_queue failed",
			status,
			0);
		return status;
	}
		
	// Save pointer to mailbox queue memory (in case we deallocate later).
	Id->HSCSI_p_mb_queue = p_memory;
	
	return status;
		
} // HSCSI_Mailbox_Create

/*************************************************************************/
// HSCSI_Mailbox_Destroy
// Initialize HSCSI_Mailbox object. 
/*************************************************************************/
void HSCSI_Mailbox_Destroy()
{
	// TODO
	
 	HSCSI_TRACE_ENTRY(HSCSI_Mailbox_Destroy);
			
} // HSCSI_Mailbox_Destroy


/*************************************************************************/
// HSCSI_Mailbox_Command
// Execute a mailbox command, wait for interrupt to complete command
/*************************************************************************/
U16 HSCSI_Mailbox_Command(PHSCSI_INSTANCE_DATA Id)
{
	HSCSI_MAILBOX_MESSAGE	mb;
	STATUS				status;
	UNSIGNED			actual_size;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Mailbox_Command);

	// Interrupt RISC to let it know we have a new mailbox command
	Write_ISP1040(Id, HSCSI_HCCR, HCCRSETH2RINTR); // 0xc0: 0x5000
	
	// Wait for the mailbox command to complete.
	// nothing in the queue until the interrupt handler puts it there
	status = NU_Receive_From_Queue(&Id->HSCSI_mailbox_queue,
		&mb,		// Copy message to this pointer.
		sizeof(HSCSI_MAILBOX_MESSAGE) / sizeof(UNSIGNED),	// one mailbox_message data element in the message
		&actual_size, // receives actual size of message 
		NU_SUSPEND);

 	HSCSI_DUMP_HEX(TRACE_L8, "\n\rmb = ", (U8 *)&mb, sizeof(HSCSI_MAILBOX_MESSAGE));
		
	if (status != NU_SUCCESS)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Mailbox_Command", 
			"NU_Receive_From_Queue failed",
			status,
			(UNSIGNED)Id);
			
		return 0;
	}

	return mb.mailbox[0];
	
} // HSCSI_Mailbox_Command


/*************************************************************************/
// HSCSI_Mailbox_Wait_Ready_Intr
// Used when interrupts are enabled to insure that the ISP firmware is not
// executing a mailbox command
// by checking that the Host Interrupt bit of the Host Command and control 
// register is 0.  This insures that the host never overwrites the mailbox 
// registers containing an executing mailbox command.
/*************************************************************************/
STATUS HSCSI_Mailbox_Wait_Ready_Intr(PHSCSI_INSTANCE_DATA Id)
{
	UNSIGNED	wait_count;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Mailbox_Wait_Ready_Intr);
			
	// Wait for ISP to clear host interrupt to RISC.  
	// Don't wait forever.
	wait_count = 10000;
	while(Read_ISP1040(Id, HSCSI_HCCR) & HCCRH2RINTR)
	{
		NU_Sleep(1);
		
		// wait for ISP to clear host interrupt to RISC
		if (wait_count-- == 0)
		{
			// We cannot send a mailbox command to the ISP.
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_Mailbox_Wait_Ready_Intr", 
				"Cannot send mailbox command to ISP",
				0,
				(UNSIGNED)Id);
			return HSCSI_ERROR_MAILBOX_TIMEOUT;
		}
	}
	
	return NU_SUCCESS;
	
} // HSCSI_Mailbox_Wait_Ready_Intr
