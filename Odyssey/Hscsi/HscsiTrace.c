/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiTrace.c
// 
// Description:
// This file handles tracing for debugging.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiTrace.c $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/
#include "HscsiCommon.h"
#include "HscsiError.h"
#include "HscsiEvent.h"
#include "HscsiInitiator.h"
#include "HscsiISR.h"
#include "HscsiTarget.h"
#include "HscsiTrace.h"


/*************************************************************************/
// Trace globals
/*************************************************************************/
UNSIGNED		 HSCSI_if_trace_on = 0;
CHAR			 HSCSI_task_name[8] = HSCSI_EVENT_TASK_NAME;
CHAR			 HSCSI_HISR_name[9] = HSCSI_HISR_NAME;
UNSIGNED		 HSCSI_task_min_remaining_free_bytes; // min free bytes in stack
UNSIGNED		 HSCSI_main_min_remaining_free_bytes; // min free bytes in stack
UNSIGNED		 HSCSI_HISR_min_remaining_free_bytes; // min free bytes in stack
UNSIGNED		 HSCSI_trace_minimum_stack;
UNSIGNED		 HSCSI_trace_num_remaining_free_bytes;
NU_TASK			*HSCSI_trace_p_current_task;
UNSIGNED		 HSCSI_trace_preempt;
UNSIGNED		 HSCSI_trace_priority;
UNSIGNED		 HSCSI_trace_scheduled_count;
char			 HSCSI_trace_task_name[8]; // 8 byte name
UNSIGNED		 HSCSI_trace_task_status;
UNSIGNED		 HSCSI_trace_time_slice;
VOID			*HSCSI_trace_stack_base;
UNSIGNED		 HSCSI_trace_stack_size;

/*************************************************************************/
// HSCSI_Trace_Create
// Create HSCSI_Trace object
/*************************************************************************/
STATUS	HSCSI_Trace_Create(PHSCSI_INSTANCE_DATA Id)
{
 	HSCSI_TRACE_ENTRY(HSCSI_Trace_Create);
			
	// TODO
	
	HSCSI_task_min_remaining_free_bytes = Id->HSCSI_config.task_stack_size;
	HSCSI_main_min_remaining_free_bytes = Id->HSCSI_config.task_stack_size;
	HSCSI_HISR_min_remaining_free_bytes = Id->HSCSI_config.HISR_stack_size;
	HSCSI_trace_p_current_task = 0;
	HSCSI_if_trace_on = 1;
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rHSCSI_trace_p_current_task = ",
								(UNSIGNED)&HSCSI_trace_p_current_task);
	
	return NU_SUCCESS;
	
} // HSCSI_Trace_Create

/*************************************************************************/
// HSCSI_Trace_Destroy
/*************************************************************************/
void	HSCSI_Trace_Destroy()
{
	// TODO
	
} // HSCSI_Trace_Destroy

/*************************************************************************/
// HSCSI_Trace_Entry
// HSCSI_Trace_Entry is called at the beginning of each method
// when HSCSI_DEBUG is defined.
/*************************************************************************/
void HSCSI_Trace_Entry(char* p_method_name)
{
//	STATUS				 status;
		
//	if (HSCSI_if_trace_on == 0)
//		return;
		
	// Print method name
	printf("\n\rHSCSI_Trace_Entry ");
	printf(p_method_name);
	printf("\n\r");
#if 0
	// Get pointer to current task
	HSCSI_trace_p_current_task = 0;
	HSCSI_trace_p_current_task = NU_Current_Task_Pointer();
		
	// Get information about current task
	status = NU_Task_Information(HSCSI_trace_p_current_task,
		HSCSI_trace_task_name,		// 8 char destination of task's name
		(DATA_ELEMENT*)&HSCSI_trace_task_status, // current status of task
		&HSCSI_trace_scheduled_count, // number of times task has been scheduled
		(OPTION*)&HSCSI_trace_priority,	// task's priority
		(OPTION*)&HSCSI_trace_preempt,	// task's preempt status
		&HSCSI_trace_time_slice,	// task's time slice value
		&HSCSI_trace_stack_base,	// starting address of task's stack
		&HSCSI_trace_stack_size,	// number of bytes in task's stack
		&HSCSI_trace_minimum_stack);	// minimum number of bytes left in stack
		
	// See what task this is and keep track of the
	// minimum free bytes in the stack for this task.
	// N.B. The following is a MACHINE DEPENDENT way of comparing one 8-byte
	// field to another 8-byte field. In big endian architecture, the 
	// address is the address of the high order byte.
	// In little endian, we would have (long long)HSCSI_trace_task_name[0]
	if ((long long)HSCSI_trace_task_name[7] == (long long)HSCSI_task_name[7])
	{
		if (HSCSI_trace_minimum_stack < HSCSI_task_min_remaining_free_bytes)
			HSCSI_task_min_remaining_free_bytes = HSCSI_trace_minimum_stack;
	}
	else if ((long long)HSCSI_trace_task_name[7] == (long long)HSCSI_HISR_name[7])
	{
		if (HSCSI_trace_minimum_stack < HSCSI_HISR_min_remaining_free_bytes)
			HSCSI_HISR_min_remaining_free_bytes = HSCSI_trace_minimum_stack;
	}	
	else if ((long long)HSCSI_trace_task_name[7] 
		== (long long)HSCSI_main_task_name[7])
	{
		if (HSCSI_trace_minimum_stack < HSCSI_main_min_remaining_free_bytes)
			HSCSI_main_min_remaining_free_bytes = HSCSI_trace_minimum_stack;
	}	

	
	// Check for stack overflow	
	HSCSI_trace_num_remaining_free_bytes = NU_Check_Stack();
#endif
		
} // HSCSI_Trace_Entry


