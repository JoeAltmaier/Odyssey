/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Callback.cpp
// 
// Description:
// 
//
// Update Log 
// 8/27/98 Jim Frandeen: Create file
// 9/15/98 Jim Frandeen: New base types
// 10/26/98 Jim Frandeen: Merge Deallocate_Parent_And_Children into
//		Terminate.
// 11/19/98 Jim Frandeen: In Terminage_Context, only check for children
//		to deallocate if m_num_child_contexts not zero.
// 12/08/98 Jim Frandeen: Implement Callback_Context::Initialize()
/*************************************************************************/
#define	TRACE_INDEX		TRACE_CALLBACK

#include "Callback.h"
#include "ErrorLog.h"
#include "TraceMon.h"
#include "PciDev.h"
#include <String.h>

#ifdef OOS
// If running under Chaos
#include "ddm.h"
		//typedef mySignalCallback DdmOsServices::SignalCallback;
		typedef mySignalCallback (DdmOsServices::SignalCallback);
#endif

/*************************************************************************/
// Callback_Context static globals
/*************************************************************************/
#ifdef THREADX
TX_SEMAPHORE	 	Callback_Context::m_ready_semaphore;
#else
NU_SEMAPHORE		Callback_Context::m_ready_semaphore;
#endif

LIST				Callback_Context::m_list_ready;
LIST				Callback_Context::m_list_avail;
int					Callback_Context::m_stop;
U32					Callback_Context::m_max_context_size;
Callback_Statistics	Callback_Context::m_stats;

Ddm					*Callback_Context::m_p_ddm = 0;

#ifdef _DEBUG
void				*Callback_Context::m_break_callback_context;
Callback_Context	**Callback_Context::m_pp_context_table;
#endif

#ifdef _WINDOWS
CRITICAL_SECTION	Callback_Context::m_critical_section;
DWORD __stdcall Thread_Start_Routine(LPVOID lpParameter);

#else
#ifdef THREADX
U32	Callback_Context::m_previous_interrupt_posture;
TX_THREAD	Callback_Context::m_thread;
void   Thread_Start_Routine(U32 argc);
#else
U32	Callback_Context::m_protect_struct;
NU_TASK	Callback_Context::m_thread;
void   Thread_Start_Routine(U32 argc, VOID *argv);
#endif
#endif

// Amount of memory to allocate for scheduler stack.
#define SCHEDULER_STACK_SIZE 2048

/*************************************************************************/
// Callback_Context::Allocate
// Allocate a context from the available list.
/*************************************************************************/
Callback_Context *Callback_Context::Allocate(U32 size)
{
	// Make sure the size specified is not greater than the size
	// of the contexts allocated.
	if (size > m_max_context_size)
		return 0;

	ENTER_CRITICAL_SECTION;

	if (LIST_IS_EMPTY(&m_list_avail))
	{
		LEAVE_CRITICAL_SECTION;
		TRACE_STRING(TRACE_L5, "List avail is empty\n\r");
		return 0;
	}

	// Remove Callback_Context from the available list
	Callback_Context *p_callback_context = 
        (Callback_Context *)LIST_REMOVE_TAIL(&m_list_avail);

	// Increment number of contexts allocated.
	m_stats.num_contexts_allocated++;

	// Decrement number of contexts available.
	CT_ASSERT((m_stats.num_contexts_avail), Callback_Context::Allocate);
	m_stats.num_contexts_avail--;

	LEAVE_CRITICAL_SECTION;

	// Set state of this context to allocated.
	CT_ASSERT((p_callback_context->m_state == STATE_AVAILABLE), Callback_Context::Allocate);
	p_callback_context->m_state = STATE_ALLOCATED;

	// Initialize number of child contexts.
	p_callback_context->m_num_child_contexts = 0;

	// Initialize lists
	LIST_INITIALIZE(&p_callback_context->m_list);
	LIST_INITIALIZE(&p_callback_context->m_child_list);

	// Initialize pointer to context
	p_callback_context->m_p_context = p_callback_context;

	// This context has no parent.
	p_callback_context->m_p_parent = 0;

	// Initialize status
	p_callback_context->m_status = OK;

	// This is not a static context.
	p_callback_context->m_is_static = 0;

	// Return pointer to object.
	return p_callback_context;

} // Allocate

/*************************************************************************/
// Callback_Context::Allocate_Child
// Allocate a child context from the available list.
/*************************************************************************/
Callback_Context *Callback_Context::Allocate_Child(U32 size)
{
	// Make sure the size specified is not greater than the size
	// of the contexts allocated.
	if (size > m_max_context_size)
		return 0;

	ENTER_CRITICAL_SECTION;

	if (LIST_IS_EMPTY(&m_list_avail))
	{
		LEAVE_CRITICAL_SECTION;
		TRACE_STRING(TRACE_L5, "List avail is empty\n\r");
		return 0;
	}

	// Remove Callback_Context from the available list
	Callback_Context *p_child_context = 
		(Callback_Context *)LIST_REMOVE_TAIL(&m_list_avail);

	// Increment number of contexts allocated.
	m_stats.num_contexts_allocated++;

	// Decrement number of contexts available.
	CT_ASSERT((m_stats.num_contexts_avail), Callback_Context::Allocate_Child);
	m_stats.num_contexts_avail--;

	// Link child context onto parent.
	// Make_Children_Ready removes child from parent list.
	LIST_INSERT_TAIL(&m_child_list, &p_child_context->m_list);

	LEAVE_CRITICAL_SECTION;

	// Set state of this context to allocated.
	CT_ASSERT((p_child_context->m_state == STATE_AVAILABLE), Callback_Context::Allocate_Child);
	p_child_context->m_state = STATE_ALLOCATED_CHILD;

	// Initialize child list of child.
	LIST_INITIALIZE(&p_child_context->m_child_list);

	// Initialize number of child contexts.
	p_child_context->m_num_child_contexts = 0;

	// Initialize pointer to context
	p_child_context->m_p_context = p_child_context;

	// Initialize pointer to parent context.
	p_child_context->m_p_parent = this;

	// Increment number of children that parent has
	Increment_Children();

	// Initialize status
	p_child_context->m_status = OK;

	// This is not a static context.
	p_child_context->m_is_static = 0;

	// Return pointer to child context.
	return p_child_context;

} // Allocate_Child

/*************************************************************************/
// Callback_Context::Allocate
// Allocate a context from the available list.
// This method is called to create a Callback_Context for a callback.
// When the callback method is called, the Callback_Context is
// deallocated.
/*************************************************************************/
Callback_Context *Callback_Context::Allocate(U32 size, Callback callback,
		void *p_context)
{
	// Make sure the size specified is not greater than the size
	// of the contexts allocated.
	if (size > m_max_context_size)
		return 0;

	ENTER_CRITICAL_SECTION;

	if (LIST_IS_EMPTY(&m_list_avail))
	{
		LEAVE_CRITICAL_SECTION;
		TRACE_STRING(TRACE_L5, "List avail is empty\n\r");
		return 0;
	}

	// Remove Callback_Context from the available list
	Callback_Context *p_callback_context = 
		(Callback_Context *)LIST_REMOVE_TAIL(&m_list_avail);

	// Increment number of contexts allocated.
	m_stats.num_contexts_allocated++;

	// Decrement number of contexts available.
	CT_ASSERT((m_stats.num_contexts_avail), Callback_Context::Allocate);
	m_stats.num_contexts_avail--;

	LEAVE_CRITICAL_SECTION;

	// Set state of this context to allocated.
	CT_ASSERT((p_callback_context->m_state == STATE_AVAILABLE), Callback_Context::Allocate);
	p_callback_context->m_state = STATE_ALLOCATED;

	// Initialize number of child contexts.
	p_callback_context->m_num_child_contexts = 0;

	// Initialize lists
	LIST_INITIALIZE(&p_callback_context->m_list);
	LIST_INITIALIZE(&p_callback_context->m_child_list);

	// Initialize pointer to context.
	// The Scheduler will know to deallocate the Callback_Context when it is
	// run because 	m_p_context != p_callback_context
	p_callback_context->m_p_context = p_context;

	// Initialize callback
	p_callback_context->m_callback = callback;

	// This context has no parent.
	p_callback_context->m_p_parent = 0;

	// Initialize status
	p_callback_context->m_status = OK;

	// This is not a static context.
	p_callback_context->m_is_static = 0;

	// Return pointer to object.
	return p_callback_context;

} // Allocate

/*************************************************************************/
// Callback_Context::Deallocate
// Critical section is up to caller.
/*************************************************************************/
inline void Callback_Context::Deallocate()
{
	// Increment number of contexts available.
	m_stats.num_contexts_avail++;

	// Set the state of this context.
	m_state = STATE_AVAILABLE;

	// Link Callback_Context back on avail list
	LIST_INSERT_TAIL(&m_list_avail, &m_list);

} // Deallocate

/*************************************************************************/
// Callback_Context::Initialize
// Initialize Callback_Context object.
// p_memory points to memory that can be used to create Callback_Context objects.
// size_memory is the amount of memory that can be used.
/*************************************************************************/
Status Callback_Context::Initialize(void *p_memory, U32 size_memory,
		U32 size_callback_context, Ddm *p_ddm)
{
	Callback_Context			*p_callback_context;
	Status status;

	// Save pointer to DDM.  This is used to call signal.
	m_p_ddm = p_ddm;	
	m_stop = 0;

	// Initialize all lists
	LIST_INITIALIZE(&m_list_avail);
	LIST_INITIALIZE(&m_list_ready);

	// Be sure we have a minimum amount of memory.
	if (size_memory < (SCHEDULER_STACK_SIZE + 100))
		return CB_NO_MEMORY;

	// Be sure we have alignment.
	p_memory = (void *)ALIGN(p_memory, 8);
	size_memory -= 8;

	// Allocate some of the memory for the scheduler stack.
	void *p_scheduler_stack;
	p_scheduler_stack = (char *)p_memory + SCHEDULER_STACK_SIZE;
	p_memory = p_scheduler_stack;
	size_memory -= SCHEDULER_STACK_SIZE;

	// Save max context size rounded for alignment.
	m_max_context_size = ((size_callback_context + 7) / 8) * 8;

	// Initialize statistics record.
	memset((void *)&m_stats, 0, sizeof(Callback_Statistics));
	m_stats.version = CALLBACK_STATISTICS_VERSION;
	m_stats.size = sizeof(Callback_Statistics);

	// Calculate how many Callback_Context objects we can create
	m_stats.num_contexts_avail_max = size_memory / m_max_context_size;

#ifdef _DEBUG

	// Save pointer to context table.
	m_pp_context_table = (Callback_Context **)p_memory;
#endif

	// Allocate memory for context table.
	p_memory = (char *)p_memory + m_stats.num_contexts_avail_max * sizeof(U32);
	p_memory = (void *)ALIGN(p_memory, 8);
	size_memory -= (((m_stats.num_contexts_avail_max * sizeof(Callback_Context *) + 7)
		/ 8) * 8);
	
	// Recalculate how many Callback_Context objects we can create
	m_stats.num_contexts_avail_max = size_memory / m_max_context_size;
	m_stats.num_contexts_avail = m_stats.num_contexts_avail_max;

	// Create each Callback_Context
	for (U32 index = 0; index < m_stats.num_contexts_avail_max; index++)
	{
		// Allocate memory for context
		p_callback_context = (Callback_Context *)p_memory;

#ifdef _DEBUG

		// Save pointer to this context
		*(m_pp_context_table + index) = p_callback_context;
#endif

		// Point to next callback context
		p_memory = (char *)p_memory + m_max_context_size;
			
		// Link context on available list
		LIST_INSERT_TAIL(&m_list_avail, &p_callback_context->m_list);

		// Set state of this context to available.
		p_callback_context->m_state = STATE_AVAILABLE;

	}

	// Initialize critical section
#ifdef _WINDOWS
	InitializeCriticalSection(&m_critical_section);
	status = NU_Create_Semaphore(&m_ready_semaphore, "ReadySem",
		0, // initial count
		NU_FIFO);
	if (status != OK)
		return status;

	// Create thread to run Scheduler
	DWORD thread_ID;
	HANDLE thread_handle = CreateThread(
		0, // LPSECURITY_ATTRIBUTES lpThreadAttributes,	// pointer to thread security attributes  
		SCHEDULER_STACK_SIZE, // DWORD dwStackSize,	// initial thread stack size, in bytes 
		&Thread_Start_Routine, // LPTHREAD_START_ROUTINE lpStartAddress,	// pointer to thread function 
		0, // LPVOID lpParameter,	// argument for new thread 

		// If this value is zero, the thread runs immediately after creation. 
		0, // DWORD dwCreationFlags,	// creation flags 
		&thread_ID // LPDWORD lpThreadId 	// pointer to returned thread identifier 
	   );

	if (thread_handle == NULL)
	{
		DWORD erc = GetLastError();
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Callback_Context::Initialize", 
			"CreateThread failed",
			erc,
			0);
		return erc;
	}

#else

#ifdef THREADX
	// Create m_ready_semaphore
	// This ThreadX semaphore is used to wake up the task 
	// when there is work to be done.
	m_previous_interrupt_posture = 0;
	status = tx_semaphore_create(&m_ready_semaphore, "ReadySem",
		0); // initial count
	if (status != OK)
		return status;

	// Create ThreadX task to run scheduler
    status =  tx_thread_create(&m_thread, 
    	"Schedulr", 
    	&Thread_Start_Routine, 
    	0, // entry_input
    	p_scheduler_stack, // stack_start
        SCHEDULER_STACK_SIZE, 
        15, // priority
        15, // preempt_threshold
        TX_NO_TIME_SLICE, 
        TX_AUTO_START);

	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Callback_Context::Initialize", 
			"tx_thread_create failed",
			status,
			0);
		return status;
	}
#else
	// Initialize Nucleus critical section.
	memset(&m_protect_struct, 0, sizeof(m_protect_struct));
	{
		// We will need a semaphore
		// and a thread to run the Scheduler.
		// Create m_ready_semaphore
		// This Nucleus semaphore is used to wake up the task 
		// when there is work to be done.
		status = NU_Create_Semaphore(&m_ready_semaphore, "ReadySem",
			0, // initial count
			NU_FIFO);
		if (status != OK)
			return status;
	
	    status = NU_Create_Task(&m_thread, 
	    	"Schedulr", 
	    	Thread_Start_Routine, 
	    	0, 
			NU_NULL, 
	    	p_scheduler_stack, // stack_start
	        SCHEDULER_STACK_SIZE, 
			1, 
			20, 
			NU_PREEMPT, 
			NU_START);
		if (status != OK)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"Callback_Context::Initialize", 
				"tx_thread_create failed",
				status,
				0);
			return status;
		}
	}
#endif
#endif
	
	Validate();
	return status;
		
} // Callback_Context::Initialize

/*************************************************************************/
// Make_Children_Ready
// All child contexts are ready to run.
/*************************************************************************/
void Callback_Context::Make_Children_Ready()
{
	ENTER_CRITICAL_SECTION;

	while (!LIST_IS_EMPTY(&m_child_list))
	{
		Callback_Context *p_child = 
            (Callback_Context *)LIST_REMOVE_TAIL(&m_child_list);
		
#ifdef OOS
		if (m_p_ddm)
		{
			// We are running under Chaos, so we will put each 
			// child context on the Chaos ready list.
			m_p_ddm->Action(&Call_Callback, p_child);
		}
		else
#endif
	
		{
			// Link child context on ready list
			LIST_INSERT_TAIL(&m_list_ready, &p_child->m_list);
		}

		// Decrement number of contexts in state allocated.
		CT_ASSERT((m_stats.num_contexts_allocated), Callback_Context::Make_Children_Ready);
		m_stats.num_contexts_allocated--;

		// Increment number of contexts ready.
		if (++m_stats.num_contexts_ready > m_stats.num_contexts_ready_max)
			m_stats.num_contexts_ready_max = m_stats.num_contexts_ready;

		// Set state of this context to ready.
		CT_ASSERT((p_child->m_state == STATE_ALLOCATED_CHILD), 
			Callback_Context::Make_Children_Ready);
		p_child->m_state = STATE_READY;
	}

	LEAVE_CRITICAL_SECTION;

#ifdef OOS
		if (m_p_ddm)
		{
			// We are running under Chaos, so we will 
			// not need to use the semaphore to signal that
			// a context is ready.
		}
		else
#endif
	{
#ifdef THREADX
		// Signal that ready list has new callback waiting to run.
		tx_semaphore_put(&m_ready_semaphore);
#else
		// Signal that ready list has new callback waiting to run.
		NU_Release_Semaphore(&m_ready_semaphore);
#endif
	}
		
} // FB_Context::Make_Children_Ready


/*************************************************************************/
// Callback_Context::Make_Ready
// Put context on ready list for execution..
/*************************************************************************/
Status Callback_Context::Make_Ready()
{
 	ENTER_CRITICAL_SECTION;

	if (m_state == STATE_READY)
	{
		LEAVE_CRITICAL_SECTION;
		return CALLBACK_ERROR_ALREADY_READY;
	}

	CT_ASSERT(((m_state == STATE_ALLOCATED) || (m_state == STATE_ACTIVE)
		|| (m_state == STATE_ALLOCATED_CHILD) ), 
		Callback_Context::Make_Ready);


	// Is this a static context?
	if (m_is_static == 0)
	{
		if (m_state == STATE_ALLOCATED)
		{
			// Decrement number of contexts in state allocated.
			CT_ASSERT((m_stats.num_contexts_allocated), Callback_Context::Make_Ready);
			m_stats.num_contexts_allocated--;
		}
		else if (m_state == STATE_ALLOCATED_CHILD)
		{
			// Remove this child from its parent.
			CT_ASSERT((!LIST_IS_EMPTY(&m_list)), Callback_Context::Make_Ready);
			LIST_REMOVE(&m_list);

			// Decrement number of contexts in state allocated.
			CT_ASSERT((m_stats.num_contexts_allocated), Callback_Context::Make_Ready);
			m_stats.num_contexts_allocated--;
		}
		else
		{
			// Decrement number of contexts in state active.
			CT_ASSERT((m_stats.num_contexts_active), Callback_Context::Make_Ready);
			m_stats.num_contexts_active--;
		}
	}

#ifdef OOS
	if (m_p_ddm)
	{
		// We are running under Chaos, so we will put  
		// the context on the Chaos ready list.
		m_p_ddm->Action(&Call_Callback, this);
	}
	else
#endif
	
	{
		LIST_INSERT_TAIL(&m_list_ready, &m_list);
	}

	// Set state of this context to ready.
	m_state = STATE_READY;

	// Increment number of contexts ready.
	if (++m_stats.num_contexts_ready > m_stats.num_contexts_ready_max)
		m_stats.num_contexts_ready_max = m_stats.num_contexts_ready;

	LEAVE_CRITICAL_SECTION;
	
#ifdef OOS
	if (m_p_ddm)
	{
		// We are running under Chaos, so we will 
		// not need to use the semaphore to signal that
		// a context is ready.
	}
	else
#endif
	{
#ifdef THREADX
		// Signal that ready list has new callback waiting to run.
		tx_semaphore_put(&m_ready_semaphore);
#else
		// Signal that ready list has new callback waiting to run.
		NU_Release_Semaphore(&m_ready_semaphore);
#endif
	}
	return OK;
		
} // Make_Ready
	
#ifdef _DEBUG	
U32 trace_callback = 0;
Callback_Context *p_trace_context;
#endif

/*************************************************************************/
// Schedule_Contexts
// Schedule_Contexts is called once, and it never returns until Stop is called.
// This is a static method.
/*************************************************************************/
Status Callback_Context::Schedule_Contexts()
{
	// Continue until Stop is called.
	while (1)
	{
		ENTER_CRITICAL_SECTION;

		while (LIST_IS_EMPTY(&m_list_ready))
		{
			LEAVE_CRITICAL_SECTION;

			// See if we should stop
			if (m_stop)
				return OK;

#ifdef THREADX
			// Wait for the next event.  
			Status status = tx_semaphore_get(
				&m_ready_semaphore,
				TX_WAIT_FOREVER);
#else
			// Wait for the next event.  
			Status status = NU_Obtain_Semaphore(
				&m_ready_semaphore,
				NU_SUSPEND);
#endif
				
			if (status != OK)
			{
				CT_Log_Error(CT_ERROR_TYPE_FATAL,
					"Callback_Context::Schedule_Contexts", 
					"NU_Obtain_Semaphore failed",
					status,
					0);
				return status;
			}

			// We have a new context on the ready list.
			ENTER_CRITICAL_SECTION;
		} // list is empty
		
		// The list is not empty.  Remove the first callback from the queue.
		Callback_Context *p_callback_context = 
			(Callback_Context *)LIST_REMOVE_HEAD(&m_list_ready);

		p_callback_context->Activate();
		LEAVE_CRITICAL_SECTION;

		// Call next Callback method
		// passing a Status and a pointer to its context.

#ifdef _DEBUG	

		// This code is here so we can set the next instruction pointer here
		// and step through the callback_contexts using the debugger.
		if (trace_callback == 0xCAFE)
		{
			for (U32 index = 0; index < m_stats.num_contexts_avail_max; index++)
				p_trace_context = *(m_pp_context_table + index);
		}

		if (p_callback_context == m_break_callback_context)
		{

			// Set a breakpoint here
			trace_callback = 0xCAFE;
		}

#endif
		// This is where we call the callback method,
		// passing a pointer to the context and the status.
		(p_callback_context->m_callback)
			(p_callback_context->m_p_context, p_callback_context->m_status);
		
	} // while
	
	return OK;
	
} // Callback_Context::Schedule_Contexts

/*************************************************************************/
// Callback_Context::Activate
// Activate a context and prepare it to run.
// ENABLE/DISABLE around this.
/*************************************************************************/
void Callback_Context::Activate()
{
	// Decrement number of contexts ready.
	CT_ASSERT((m_state == STATE_READY), Callback_Context::Schedule_Contexts);
	CT_ASSERT((m_stats.num_contexts_ready), Callback_Context::Schedule_Contexts);
	m_stats.num_contexts_ready--;

	// Check to see if this Callback_Context should be deallocated.
	// If the Callback_Context was created for a callback, then
	// p_callback_context != m_p_context.
	if (m_p_context != this)
		Deallocate();
	else
	{
		// Set state of this context to active.
		m_state = STATE_ACTIVE;

		// Increment number of contexts active.
		if (++m_stats.num_contexts_active > m_stats.num_contexts_active_max)
			m_stats.num_contexts_active_max = m_stats.num_contexts_active;
		CT_ASSERT((m_stats.num_contexts_active <= m_stats.num_contexts_active_max), 
			Callback_Context::Activate);
	}
	
} // Activate

#ifdef OOS
/*************************************************************************/
// Callback_Context::Call_Callback
// We get called by Chaos when a context is ready to run.
/*************************************************************************/
STATUS Callback_Context::Call_Callback(void *p_context)
{
	Callback_Context *p_callback_context = (Callback_Context *) p_context;

	ENTER_CRITICAL_SECTION;
	
	p_callback_context->Activate();
	
	LEAVE_CRITICAL_SECTION;
	
    (p_callback_context->m_callback)
                        (p_callback_context->m_p_context,p_callback_context->m_status);
     return OK;
                        
} // Call_Callback
#endif // OOS

/*************************************************************************/
// Callback_Context::Terminate
// Deallocate and then run parent if last child has finished.
// Note that Terminate can be set as a callback.
/*************************************************************************/
void Callback_Context::Terminate(void *p_context, Status status)
{
	Callback_Context *p_callback_context = (Callback_Context *)p_context;

	ENTER_CRITICAL_SECTION;

	// Static contexts are not terminated.
	CT_ASSERT((p_callback_context->m_is_static == 0), Terminate);

	if (p_callback_context->m_num_child_contexts)
	{
		// Deallocate child contexts.
		while (!LIST_IS_EMPTY(&p_callback_context->m_child_list))
		{
			Callback_Context *p_child = (Callback_Context *)LIST_REMOVE_TAIL(
				&p_callback_context->m_child_list);
			
			// Decrement number of contexts in state allocated.
			CT_ASSERT((p_child->m_state == STATE_ALLOCATED), 
				Callback_Context::Deallocate_Parent_And_Children);
			CT_ASSERT((m_stats.num_contexts_allocated), 
				Callback_Context::Deallocate_Parent_And_Children);
			m_stats.num_contexts_allocated--;

			// Link child context on available list
			p_child->Deallocate();
		}
	}

	CT_ASSERT(((p_callback_context->m_state == STATE_ALLOCATED) 
		|| (p_callback_context->m_state == STATE_ACTIVE)
		|| (p_callback_context->m_state == STATE_ALLOCATED_CHILD) ), 
		Callback_Context::Terminate);
	if (p_callback_context->m_state == STATE_ALLOCATED) 
	{
		// Decrement number of contexts in state allocated.
		CT_ASSERT((m_stats.num_contexts_allocated), Callback_Context::Terminate);
		m_stats.num_contexts_allocated--;
	}
	else if (p_callback_context->m_state == STATE_ALLOCATED_CHILD)
	{
		// Remove this child from its parent.
		CT_ASSERT((!LIST_IS_EMPTY(&p_callback_context->m_list)), Callback_Context::Make_Ready);
		LIST_REMOVE(&p_callback_context->m_list);

		// Decrement number of contexts in state allocated.
		CT_ASSERT((m_stats.num_contexts_allocated), Callback_Context::Make_Ready);
		m_stats.num_contexts_allocated--;
	}
	else
	{
		// Decrement number of contexts in state active.
		CT_ASSERT((m_stats.num_contexts_active), Callback_Context::Terminate);
		m_stats.num_contexts_active--;
	}

	// Link Callback_Context back on avail list
	p_callback_context->Deallocate();

	// See if this is a child context
	Callback_Context *p_parent = p_callback_context->m_p_parent;
	if (p_parent)
	{
		// Save the child's status in the parent status
		if (p_parent->Get_Status() == OK)
			p_parent->Set_Status(status);

		// Decrement number of children that parent has
		if (p_parent->Decrement_Children() == 0)
		{
			LEAVE_CRITICAL_SECTION;

			// Parent has no more children, so schedule parent to run.
			p_parent->Make_Ready();
			return;
		}
	}

	LEAVE_CRITICAL_SECTION;

} // Terminate

#ifdef _WINDOWS
/*************************************************************************/
// Thread_Start_Routine
// This thread runs Schedule_Contexts for Windows.
/*************************************************************************/
DWORD __stdcall Thread_Start_Routine(LPVOID lpParameter)
{
	Status status = Callback_Context::Schedule_Contexts();
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Thread_Start_Routine", 
			"Callback_Context::Schedule failed",
			status,
			0);
	}

	return status;
} // Thread_Start_Routine

#else
#ifdef THREADX
/*************************************************************************/
// Thread_Start_Routine
// This thread runs Schedule_Contexts for ThreadX.
/*************************************************************************/
void   Thread_Start_Routine(U32 argc)
{
	Status status = Callback_Context::Schedule_Contexts();
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Thread_Start_Routine", 
			"Callback_Context::Schedule failed",
			status,
			0);
	}

} // Thread_Start_Routine

#else
/*************************************************************************/
// Thread_Start_Routine
// This thread runs Schedule_Contexts for Nucleus
/*************************************************************************/
void   Thread_Start_Routine(U32 argc, VOID *argv)
{
	Status status = Callback_Context::Schedule_Contexts();
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Thread_Start_Routine", 
			"Callback_Context::Schedule failed",
			status,
			0);
	}

} // Thread_Start_Routine
#endif //  Nucleus
#endif //  _WINDOWS

/*************************************************************************/
// Callback_Context::Validate
// For debugging, validate internal structures.
/*************************************************************************/
void Callback_Context::Validate()
{
#ifdef _DEBUG

	ENTER_CRITICAL_SECTION;

	if (LIST_IS_EMPTY(&m_list_avail))
	{
		LEAVE_CRITICAL_SECTION;
		return;
	}

	Callback_Context *p_callback_context = 
		(Callback_Context *)m_list_avail.forward_link;
		
	CT_ASSERT((p_callback_context->m_state == STATE_AVAILABLE), Callback_Context::Validate);
	
	// Make sure links point somewhere -- cause fault if not.
	Callback_Context *p_test_context = (Callback_Context *)p_callback_context->m_list.forward_link;
	p_test_context = (Callback_Context *)p_callback_context->m_list.backward_link;

	// Check the last one on this list.
	p_callback_context = (Callback_Context *)m_list_avail.backward_link;
		
	CT_ASSERT((p_callback_context->m_state == STATE_AVAILABLE), Callback_Context::Validate);
	
	// Make sure links point somewhere -- cause fault if not.
	p_test_context = (Callback_Context *)p_callback_context->m_list.forward_link;
	p_test_context = (Callback_Context *)p_callback_context->m_list.backward_link;

	LEAVE_CRITICAL_SECTION;
	
#endif
	
} // Validate

