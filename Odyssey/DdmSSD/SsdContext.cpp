/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdContext.cpp
// 
// Description:
// This file implements context creation for the
// Solid State Drive DDM. 
// 
// Update Log 
// 
// 02/25/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD1
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include "SsdContext.h"
#include "SsdDDM.h"
#include "Callback.h"
#include "ErrorLog.h"
#include "FlashFile.h"
#include "Nucleus.h"


/*************************************************************************/
// SSD_Ddm::Initialize_Scheduler -- gets called only once.
// Create a thread to initialize contexts.
/*************************************************************************/
STATUS SSD_Ddm::Initialize_Scheduler()
{ 
	// Allocate stack space for context scheduler task.
	STATUS status;
	void *m_p_scheduler_stack = new (tBIG) char[MEMORY_FOR_SCHEDULER_STACK];
	if (m_p_scheduler_stack == 0)
	{
		status = FF_ERROR_NO_MEMORY;
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Initialize_Scheduler", 
			"new failed",
			status,
			0);
		return status;
	}
	
    // Allocate memory for callback contexts.
	void *m_p_callback_contexts = new (tBIG) char[MEMORY_FOR_CALLBACKS];
	if (m_p_callback_contexts == 0)
	{
		status = FF_ERROR_NO_MEMORY;
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Context_Task", 
			"new failed",
			status,
			0);
		return status;
	}
    
    // Initialize callbacks.
	status = Callback_Context::Initialize(m_p_callback_contexts, MEMORY_FOR_CALLBACKS, 92);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Context_Task", 
			"Callback_Context::Initialize failed",
			status,
			0);
		return status;
	}

    status = NU_Create_Task(&m_scheduler_task, "CtxTask", Context_Task, 
    	0, NU_NULL, m_p_scheduler_stack,
                   MEMORY_FOR_SCHEDULER_STACK, 1, 20, NU_PREEMPT, NU_START);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Initialize_Scheduler", 
			"NU_Create_Task",
			status,
			0);
	}
	
	return status;
	
} // Initialize_Scheduler

/*************************************************************************/
// SSD_Ddm::Scheduler_Task -- gets called only once.
// This is the task that runs the context scheduler.
/*************************************************************************/
void SSD_Ddm::Scheduler_Task(UNSIGNED argc, VOID *argv)
{ 
	// Start the scheduler
	STATUS status = Callback_Context::Schedule();
	
	// Should never return.
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Context_Task", 
			"Callback_Context::Schedule failed",
			status,
			0);
	}

} // Scheduler_Task

