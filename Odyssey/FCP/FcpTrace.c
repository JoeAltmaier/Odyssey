/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpTrace.c
// 
// Description:
// This file handles tracing for debugging.
// 
// Update Log 
// 5/27/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/
#include "FcpCommon.h"
#include "FcpError.h"
#include "FcpEvent.h"
#include "FcpInitiator.h"
#include "FcpISR.h"
#include "FcpTarget.h"
#include "FcpTrace.h"


/*************************************************************************/
// Trace globals
/*************************************************************************/
UNSIGNED		 FCP_if_trace_on = 0;
CHAR			 FCP_task_name[8] = FCP_EVENT_TASK_NAME;
CHAR			 FCP_HISR_name[8] = FCP_HISR_NAME;
UNSIGNED		 FCP_task_min_remaining_free_bytes; // min free bytes in stack
UNSIGNED		 FCP_main_min_remaining_free_bytes; // min free bytes in stack
UNSIGNED		 FCP_HISR_min_remaining_free_bytes; // min free bytes in stack
UNSIGNED		 FCP_trace_minimum_stack;
UNSIGNED		 FCP_trace_num_remaining_free_bytes;
NU_TASK			*FCP_trace_p_current_task;
UNSIGNED		 FCP_trace_preempt;
UNSIGNED		 FCP_trace_priority;
UNSIGNED		 FCP_trace_scheduled_count;
char			 FCP_trace_task_name[8]; // 8 byte name
UNSIGNED		 FCP_trace_task_status;
UNSIGNED		 FCP_trace_time_slice;
VOID			*FCP_trace_stack_base;
UNSIGNED		 FCP_trace_stack_size;

/*************************************************************************/
// FCP_Trace_Create
// Create FCP_Trace object
/*************************************************************************/
STATUS	FCP_Trace_Create(PINSTANCE_DATA Id)
{
 	FCP_TRACE_ENTRY(FCP_Trace_Create);
			
	// TODO
	
	FCP_task_min_remaining_free_bytes = Id->FCP_config.task_stack_size;
	FCP_main_min_remaining_free_bytes = Id->FCP_config.task_stack_size;
	FCP_HISR_min_remaining_free_bytes = Id->FCP_config.HISR_stack_size;
	FCP_trace_p_current_task = 0;
	FCP_if_trace_on = 1;
	FCP_PRINT_HEX(TRACE_L8, "\n\rFCP_trace_p_current_task = ",
								(UNSIGNED)&FCP_trace_p_current_task);
	
	return NU_SUCCESS;
	
} // FCP_Trace_Create

/*************************************************************************/
// FCP_Trace_Destroy
/*************************************************************************/
void	FCP_Trace_Destroy()
{
	// TODO
	
} // FCP_Trace_Destroy

/*************************************************************************/
// FCP_Trace_Entry
// FCP_Trace_Entry is called at the beginning of each method
// when FCP_DEBUG is defined.
/*************************************************************************/
void FCP_Trace_Entry(char* p_method_name)
{
//	STATUS				 status;
		
//	if (FCP_if_trace_on == 0)
//		return;
		
	// Print method name
	printf("\n\rFCP_Trace_Entry ");
	printf(p_method_name);
	printf("\n\r");
#if 0
	// Get pointer to current task
	FCP_trace_p_current_task = 0;
	FCP_trace_p_current_task = NU_Current_Task_Pointer();
		
	// Get information about current task
	status = NU_Task_Information(FCP_trace_p_current_task,
		FCP_trace_task_name,		// 8 char destination of task's name
		(DATA_ELEMENT*)&FCP_trace_task_status, // current status of task
		&FCP_trace_scheduled_count, // number of times task has been scheduled
		(OPTION*)&FCP_trace_priority,	// task's priority
		(OPTION*)&FCP_trace_preempt,	// task's preempt status
		&FCP_trace_time_slice,	// task's time slice value
		&FCP_trace_stack_base,	// starting address of task's stack
		&FCP_trace_stack_size,	// number of bytes in task's stack
		&FCP_trace_minimum_stack);	// minimum number of bytes left in stack
		
	// See what task this is and keep track of the
	// minimum free bytes in the stack for this task.
	// N.B. The following is a MACHINE DEPENDENT way of comparing one 8-byte
	// field to another 8-byte field. In big endian architecture, the 
	// address is the address of the high order byte.
	// In little endian, we would have (long long)FCP_trace_task_name[0]
	if ((long long)FCP_trace_task_name[7] == (long long)FCP_task_name[7])
	{
		if (FCP_trace_minimum_stack < FCP_task_min_remaining_free_bytes)
			FCP_task_min_remaining_free_bytes = FCP_trace_minimum_stack;
	}
	else if ((long long)FCP_trace_task_name[7] == (long long)FCP_HISR_name[7])
	{
		if (FCP_trace_minimum_stack < FCP_HISR_min_remaining_free_bytes)
			FCP_HISR_min_remaining_free_bytes = FCP_trace_minimum_stack;
	}	
	else if ((long long)FCP_trace_task_name[7] 
		== (long long)FCP_main_task_name[7])
	{
		if (FCP_trace_minimum_stack < FCP_main_min_remaining_free_bytes)
			FCP_main_min_remaining_free_bytes = FCP_trace_minimum_stack;
	}	

	
	// Check for stack overflow	
	FCP_trace_num_remaining_free_bytes = NU_Check_Stack();
#endif
		
} // FCP_Trace_Entry


