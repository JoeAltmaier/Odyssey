/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfWatchdog.cpp
// 
// Description:
// This file implements the watchdog timer methods of the Flash Storage
// Controller. 
// 
// 4/1/98 Jim Frandeen: Create file
// 6/30/99 Jim Frandeen: Implement interleaving
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfInterface.h"
#include "FfWatchdog.h"
#include "FfStats.h"

extern U32					  FF_bit_mask[32];

/*************************************************************************/
// FF_Watchdog::Open
// Open the watchdog timer.
/*************************************************************************/
Status	FF_Watchdog::Open(FF_Mem *p_mem, U32 num_units)
{
	Status		status;
	
	// Save number of units in object.
	m_num_units = num_units;

	// Initialize critical section
#ifdef THREADX
	m_previous_interrupt_posture = 0;
	status = tx_timer_create(&m_watchdog_timer,
		"FbTimer", // name of timer
		&Watchdog_Expiration_Routine,
		
		// The ID passed to Watchdog_Expiration_Routine
		// will have the unit_index number.
		unit_index,
		FF_WATCHDOG_TIMER_TICKS,	// zero initial ticks
		FF_WATCHDOG_TIMER_TICKS, // reschedule time
		TX_ACTIVATE);
#else

	// Nucleus or windows
	status = NU_Create_Timer(&m_watchdog_timer,
		"FfTimer", // name of timer
		&Watchdog_Expiration_Routine,
		
		// The ID passed to Watchdog_Expiration_Routine
		// is a pointer to the watchdog object.
		(U32 )this,
		FF_WATCHDOG_TIMER_TICKS,	// initial ticks
		FF_WATCHDOG_TIMER_TICKS, // reschedule time
		NU_ENABLE_TIMER);

#ifdef _WINDOWS

	// Initialize Windows critical section.
	InitializeCriticalSection(&m_critical_section);
#else

	// Initialize Nucleus critical section.
	ZERO(&m_protect_struct, sizeof(m_protect_struct));

#endif // Nucleus
#endif // _WINDOWS

 		if (status != OK)
 		{	
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Watchdog::Initialize", 
				"NU_Create_Timer failed",
				status,
				0);
			return status;
 		}

 	// Initialize watchdog counters and contexts.
 	for (U32 unit_index = 0; unit_index < m_num_units; unit_index++)
 	{
		m_p_context[unit_index] = 0;
		m_p_counter[unit_index] = 0;
 	}
 	
 	return OK;
	
} // FF_Watchdog::Open
	
/*************************************************************************/
// FF_Watchdog::Close
// Close the watchdog timer.
/*************************************************************************/
Status	FF_Watchdog::Close()
{
	Status		status;
	
	// Initialize critical section
#ifdef THREADX
#else

	// Nucleus or windows
	status = NU_Control_Timer(&m_watchdog_timer, NU_DISABLE_TIMER);
	if (status != OK)
	{	
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Watchdog::Close", 
			"NU_Disable_Timer failed",
			status,
			0);
		return status;
	}
	
	status = NU_Delete_Timer(&m_watchdog_timer);
	if (status != OK)
	{	
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Watchdog::Close", 
			"NU_Delete_Timer failed",
			status,
			0);
		return status;
	}
#endif

 	return OK;
	
} // FF_Watchdog::Open
	
	
/*************************************************************************/
// Memory_Size
// Return memory size required by object.
/*************************************************************************/
U32 FF_Watchdog::Memory_Size()
{
	return 0;
}

/*************************************************************************/
// Start_Watchdog
/*************************************************************************/
void FF_Watchdog::Start_Watchdog(
	Callback_Context *p_callback_context, 
	U32 unit_index,
	U32 timer_ticks)
{	
	CT_ASSERT(unit_index < m_num_units, FF_Watchdog::Start_Watchdog);

	ENTER_CRITICAL_SECTION;
	
 	// Save context for Watchdog_Expiration_Routine
 	CT_ASSERT((m_p_context[unit_index] == 0), Start_Watchdog);
 	m_p_context[unit_index] = p_callback_context;
 	
 	// Set the counter to the number of ticks.
 	// The counter will be decremented each time the watchdog timer goes off.
 	CT_ASSERT((m_p_counter[unit_index] == 0), Start_Watchdog);
 	m_p_counter[unit_index] = timer_ticks;

	LEAVE_CRITICAL_SECTION;

} // Start_Watchdog
	
/*************************************************************************/
// Watchdog_Expiration_Routine
// Come here when the watchdog timer goes off.
// Check each unit_index to see if it has timed out.
/*************************************************************************/
void FF_Watchdog::Watchdog_Expiration_Routine(U32 id)
{
	FF_Watchdog *p_watchdog = (FF_Watchdog *)id;
	p_watchdog->Check_For_Timeout();

} // Watchdog_Expiration_Routine

/*************************************************************************/
// Check_For_Timeout
// Come here when the watchdog timer goes off.
// Check each unit_index to see if it has timed out.
/*************************************************************************/
void FF_Watchdog::Check_For_Timeout()
{
	for (U32 unit_index = 0; unit_index < m_num_units; unit_index++)
	{
		// Is this unit_index waiting for a timeout or interrupt?
		if (m_p_counter[unit_index])
		{
			ENTER_CRITICAL_SECTION;
			
			// Is the unit_index still waiting?
			if (m_p_counter[unit_index] == 0)
			{
				LEAVE_CRITICAL_SECTION;
				break;
			}
			
			// The unit_index is waiting.  Decrement the wait count.
			if (--m_p_counter[unit_index] == 0)
			{
				// Timeout has occurred.
				Callback_Context *p_callback_context = m_p_context[unit_index];
				m_p_context[unit_index] = 0;

				LEAVE_CRITICAL_SECTION;				
				
				CT_ASSERT(p_callback_context, 
					FF_Watchdog::Watchdog_Expiration_Routine);
				
				// Set the error code for this context.
				p_callback_context->Set_Status(FF_ERROR_CODE(TIMEOUT));
			
				// Queue context back on ready list for execution..
				Status status = p_callback_context->Make_Ready();
			
#if 0
				if (status == CALLBACK_ERROR_ALREADY_READY)
				{
					// If this context is already ready, then the interrupt must have gone off.
					// The watchdog timer did go off.  
					// Since the context has not yet run, change the status to OK
					p_callback_context->Set_Status(OK);
				}
#else
				CT_ASSERT((status == OK), FF_Watchdog::Check_For_Timeout);
#endif
			} // timeout
			else
			{
				LEAVE_CRITICAL_SECTION;
			}
		} // m_p_counter[unit_index]
	} // for
 	
} // Check_For_Timeout

/*************************************************************************/
// Wake_Up_Waiting_Context
// Called from the interrupt service routine.
// unit_index index is a concatenation of array and column, e.g.,
// ACCC if Array is one bit, and Column is 3 bits.
/*************************************************************************/
void FF_Watchdog::Wake_Up_Waiting_Context(U32 unit_index, UI64 controller_status)
{
	CT_ASSERT(unit_index < m_num_units, FF_Watchdog::Wake_Up_Waiting_Context);
	
	ENTER_CRITICAL_SECTION;

	Callback_Context *p_callback_context = m_p_context[unit_index];
	
 	// Stop the watchdog timer.
 	m_p_counter[unit_index] = 0;
 	m_p_context[unit_index] = 0;
 	
	LEAVE_CRITICAL_SECTION;

	// If no context is waiting, then we got a spurious interrupt.
	if (p_callback_context == 0)
		return;

	// Save the controller_status in the context to be scheduled.  This contains
	// device-dependent information such as ECC error detected.
	p_callback_context->Set_Return_Value(controller_status);

	// Queue context back on ready list for execution..
	Status status = p_callback_context->Make_Ready();

#if 0
	if (status == CALLBACK_ERROR_ALREADY_READY)
	{
		// If this context is already ready, then the watchdog may have gone off.
		if (p_callback_context->Get_Status() == FF_ERROR_TIMEOUT)
		{
			// The watchdog timer did go off.  
			// Since the context has not yet run, change the status to OK
			p_callback_context->Set_Status(OK);
		}
	}
#else
	CT_ASSERT((status == OK), FF_Watchdog::Wake_Up_Waiting_Context);
#endif

} // FF_Watchdog::Wake_Up_Waiting_Context


