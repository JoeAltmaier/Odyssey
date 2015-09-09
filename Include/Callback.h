/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CallbackContext.h
// 
// Description:
// The CallbackContext 
// 
// 
// Update Log 
// 8/2/99 Jim Frandeen: Make m_return_value a 64-bit value
// 02/24/99 Jim Frandeen: Replace STATUS with Status to avoid Nucleus.h
// 10/26/98 Jim Frandeen: Merge Deallocate_Parent_And_Children into
//		Terminate.
// 9/14/98 Jim Frandeen: New base types
// 9/5/98  Jim Frandeen: Change CALLBACK to Callback so it won't
// conflict with Windows CALLBACK.
// 8/27/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(Callback_H)
#define Callback_H

class Ddm;
class DdmContext;

#include "List.h"
#include "Simple.h"
#include "Critical.h"

#ifdef OOS
#include "Ddm.h"
#endif

// If compiling under Microsoft Development Environment
#ifdef _WINDOWS
#define _X86_
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#endif

#ifdef THREADX
#include "tx_api.h"
#else
#include "Nucleus.h"
#endif

// If debugging, save the name of the callback context.
#ifdef _DEBUG
#define Set_Callback(callback)	Set_Callback_Debug(callback, #callback)
#else
#define Set_Callback(callback)	Set_Callback_Release(callback)
#endif

/*************************************************************************/
// CALLBACK_ERROR codes begin with CALLBACK_ERROR_BASE.
/*************************************************************************/
#define CALLBACK_ERROR_BASE 4100
typedef enum
{
	CALLBACK_ERROR_ALREADY_READY 			= CALLBACK_ERROR_BASE + 1,
	CALLBACK_ERROR_NO_MEMORY				= -32 // same as NU_NO_MEMORY
} CALLBACK_ERROR;

// Declare Callback as a pointer to a method.
// This method takes a Status and pointer to void and returns void.
// This method gets called by Callback_Context each time it takes a
// context off the ready list.
typedef void (*Callback)(void *p_context, Status status);

/*************************************************************************/
// Error Codes returned
/*************************************************************************/
#define         CB_NO_MEMORY                    -32 // same as NU_NO_MEMORY

/*************************************************************************/
// Callback_Statistics
/*************************************************************************/
#define CALLBACK_STATISTICS_VERSION 2

typedef struct {

	// Version of Callback_Statistics record.
	U32	version;
	
	// Number of bytes in record
	U32	size;
	
	// Number of contexts in state allocated.
	U32	num_contexts_allocated;
	
	// Number of static contexts.  These contexts are allocated by the
	// user and initialized by the Initialize method.
	U32	num_contexts_static;
	
	// Number of contexts available to allocate.
	U32	num_contexts_avail;
	
	// Number of contexts initially allocated
	U32	num_contexts_avail_max;
	
	// Number of contexts in state ready to run.
	U32	num_contexts_ready;
	
	// Maximum number of contexts ready to run ever encountered.
	U32	num_contexts_ready_max;
	
	// Number of contexts in state active.
	U32	num_contexts_active;
	
	// Maximum number of contexts active ever encountered.
	U32	num_contexts_active_max;
	
} Callback_Statistics;

/*************************************************************************/
// Callback_Context
// This is the object that gets linked to the ready list.
// Simply a method to call and a context pointer to pass when
// the method is called.
/*************************************************************************/
class Callback_Context
{
public:
	// Initialize Callback_Context service.
	// p_memory points to memory that can be used to create Callback_Context objects.
	// size_memory is the amount of memory that can be used.
	// size_callback_context is the size of Callback_Context to allocate.
	static Status Initialize(void *p_memory, U32 size_memory,
		U32 size_callback_context, Ddm *p_ddm = 0);

	// Initialize Callback_Context object.
	void Initialize();

	// Allocate a Callback_Context from the available list.
	// If none are available, 
	// or if the size is greater than the size availble,
	// return 0.
	static Callback_Context *Allocate(U32 size);

	// Allocate a child context.
	// When all of the child contexts have terminated, the parent
	// context will be scheduled to run.
	Callback_Context *Allocate_Child(U32 size);

	// Allocate a Callback_Context from the available list.
	// Initialize callback method and pointer to context.
	// When this method is called, the context is deallocated.
	static Callback_Context *Allocate(U32 size, Callback callback,
		void *p_context);

	// Set the callback
	void Set_Callback_Debug(Callback callback, char *callback_name);
	void Set_Callback_Release(Callback callback);

	// Get pointer to parent callback, if any
	Callback_Context *Get_Parent();
	
	// Get/Set status
	void Set_Status(Status status);
	Status Get_Status();

	// Get/Set return value
	void Set_Return_Value(UI64 return_value, U32 which_value = 0);
	UI64 Get_Return_Value(U32 which_value = 0);

	// Terminate
	// Deallocate and then run parent if last child has finished.
	// Deallocate any child contexts.
	// Note that Terminate can be set as a callback.
	static void Terminate(void *p_context, Status status);

	// Stop causes the scheduler to stop putting new events in the queue.
	// When the queue is empty, it simply returns.
	static void Stop();

	// Make_Ready puts a Callback_Context at the end of the ready list. 
	Status Make_Ready();
	void Make_Children_Ready();

	// Returns the maximum context size.  This is handy for debugging to assert
	// that a given context is not bigger than the max.
	static U32 Get_Max_Context_Size();

	// Schedule is called once, and it never returns until Stop is called.
	static Status Schedule_Contexts();
	
	// For debugging, validate internal structures.
	static void Validate();

private:  // helper methods

	void Activate();
	int Increment_Children();
	int Decrement_Children();

	// Deallocate the Callback_Context and return it to the available list.
	void Deallocate();
	
#ifdef OOS
	static STATUS Call_Callback(void *p_context);
#endif

public: // member data

	// List must be the first data member in order to use the 
	// link list methods defined in List.h.
	LIST			 m_list;

protected: // member data

	friend DdmContext;

	// Return value is used by child to pass a return value
	// back to the parent.
	UI64			 m_return_value;
	UI64			 m_return_value_1;

	// Children are linked on this list.  The purpose of this
	// list is so that children can be allocated and then made
	// ready as a group.  They can also be deallocated when
	// the parent is deallocated.
	LIST			 m_child_list;

	// Method to call when Callback_Context is ready to run
	Callback		 m_callback;

	// context passed to Callback_Context when it is called.
    // This is usually a pointer to a Callback_Context, 
    // but it does not have to be so.
	void			 *m_p_context;

	// Status passed to Callback_Context when it is called.
	Status			 m_status;

	// Number of child contexts.
	int				 m_num_child_contexts;

	// Pointer to parent context if this is a child context
	Callback_Context	*m_p_parent;

	// What is the state of this context.
	enum {
		STATE_AVAILABLE,
		STATE_ALLOCATED,
		STATE_ALLOCATED_CHILD,
		STATE_READY,
		STATE_ACTIVE
	}					m_state;

	// True if context is static.  This context was allocated
	// in the memory space of the client and initialized by
	// the Initialize method.  This context is never allocated
	// or terminated.
	int					m_is_static;

#ifdef _DEBUG
	// For debugging, save the name of the callback context
	char			*m_callback_name;
#endif

private: // static data

#ifdef THREADX
	static TX_SEMAPHORE	 m_ready_semaphore;
	static TX_THREAD	 m_thread;
#else

	// Semaphore indicates a context is ready to run.
	static NU_SEMAPHORE	 m_ready_semaphore;
	static NU_TASK		 m_thread;
#endif
	
	static LIST			 m_list_ready;
	static LIST			 m_list_avail;

	static int			 m_stop;
	static U32		 	m_max_context_size;

	static Callback_Statistics	m_stats;

#ifdef _DEBUG
	static void			*m_break_callback_context;

	// Array of pointers to Callback_Contexts.
	// This is used for debugging, so that we will be able to find
	// all the contexts and figure out what they are doing.
	static Callback_Context **m_pp_context_table;

#endif

	// Critical sections must work under both Windows and Nucleus
#ifdef _WINDOWS

		// Critical section to protect the list
	static CRITICAL_SECTION	m_critical_section;

#define ENTER_CRITICAL_SECTION \
	EnterCriticalSection(&m_critical_section);

#define LEAVE_CRITICAL_SECTION \
	LeaveCriticalSection(&m_critical_section);

#else
#ifdef THREADX

	// ThreadX disables and enables interrupts.
	static U32			m_previous_interrupt_posture;
	
#define ENTER_CRITICAL_SECTION \
	m_previous_interrupt_posture = tx_interrupt_control(TX_INT_DISABLE);

#define LEAVE_CRITICAL_SECTION \
	tx_interrupt_control(m_previous_interrupt_posture);

#else

	// Nucleus
	static U32			m_protect_struct;

#define ENTER_CRITICAL_SECTION \
	m_protect_struct = CriticalEnter();

#define LEAVE_CRITICAL_SECTION \
	CriticalLeave(m_protect_struct);
#endif // Nucleus
#endif // NOT _WINDOWS

	static Ddm		*m_p_ddm;
		
}; // Callback_Context


/*************************************************************************/
// Decrement_Children
/*************************************************************************/
inline int Callback_Context::Decrement_Children()
{
	return --m_num_child_contexts;
}

/*************************************************************************/
// Get_Parent
/*************************************************************************/
inline Callback_Context *Callback_Context::Get_Parent()
{
	return m_p_parent;
}

/*************************************************************************/
// Increment_Children
/*************************************************************************/
inline int Callback_Context::Increment_Children()
{
	return ++m_num_child_contexts;
}

/*************************************************************************/
// Initialize Callback_Context object.
// This can be used to create a Callback_Context that never gets allocated
// or freed.
/*************************************************************************/
inline void Callback_Context::Initialize()
{
	// Set state of this context to allocated.
	m_state = STATE_ALLOCATED;

	// Initialize number of child contexts.
	m_num_child_contexts = 0;
	
	// Initialize status
	m_status = OK;

	// Increment the number of static contexts.
	m_stats.num_contexts_static++;

	// Set flag to indicate this is a static context.
	// This context is never allocated or terminated.
	m_is_static = 1;

	// Initialize lists
	LIST_INITIALIZE(&m_list);
	LIST_INITIALIZE(&m_child_list);

	// Initialize pointer to context
	m_p_context = this;

}

/*************************************************************************/
// Increment_Children
/*************************************************************************/
inline U32 Callback_Context::Get_Max_Context_Size()
{
	return m_max_context_size;
}

#ifdef _DEBUG
/*************************************************************************/
// Set_Callback_Debug
/*************************************************************************/
inline void Callback_Context::Set_Callback_Debug(Callback callback,
												 char *callback_name)
{
	m_callback = callback;
	m_callback_name = callback_name;
}
#endif

/*************************************************************************/
// Set_Callback_Release
/*************************************************************************/
inline void Callback_Context::Set_Callback_Release(Callback callback)
{
	m_callback = callback;
}

/*************************************************************************/
// Set_Return_Value
// Sets return value
/*************************************************************************/
inline void Callback_Context::Set_Return_Value(UI64 return_value, U32 which_value)
{
	if (which_value == 0)
		m_return_value = return_value;
	else
		m_return_value_1 = return_value;
}

/*************************************************************************/
// Get_Return_Value
// Returns return value
/*************************************************************************/
inline UI64 Callback_Context::Get_Return_Value(U32 which_value)
{
	if (which_value == 0)
		return m_return_value;
	return m_return_value_1;
}

/*************************************************************************/
// Set_Status
// Sets status
/*************************************************************************/
inline void Callback_Context::Set_Status(Status status)
{
	m_status = status;
}

/*************************************************************************/
// Get_Status
// Returns status
/*************************************************************************/
inline Status Callback_Context::Get_Status()
{
	return m_status;
}

/*************************************************************************/
// Stop
/*************************************************************************/
inline void Callback_Context::Stop()
{
	m_stop = 1;
}

#endif //   Callback_H
