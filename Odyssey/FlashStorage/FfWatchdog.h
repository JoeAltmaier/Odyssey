/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfController.h
// 
// Description:
// This file defines the interface to the Flash File 
// Watchdog object. 
// 
// 8/12/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfWatchdog_H)
#define FfWatchdog_H

#include "Callback.h"
#include "FlashDevice.h"
#include "List.h"

#ifdef THREADX
#include "tx_api.h"
#else
#include "Nucleus.h"
#endif

#pragma pack(1)

// The Nucleus timer tick is 10 milliseconds.
// Watchdog timer goes off every second.
#define FF_WATCHDOG_TIMER_TICKS 100

#ifdef SIM
#define FF_READ_TIMER_WATCHDOG_TICKS 4
#define FF_WRITE_TIMER_WATCHDOG_TICKS 4
#define FF_ERASE_TIMER_WATCHDOG_TICKS 16
#else
#define FF_READ_TIMER_WATCHDOG_TICKS 5
#define FF_WRITE_TIMER_WATCHDOG_TICKS 6
#define FF_ERASE_TIMER_WATCHDOG_TICKS 8
#endif

class FF_Watchdog;
class FF_Interface;
class FF_Page_Map;

/*************************************************************************/
// FF_Watchdog
/*************************************************************************/
class FF_Watchdog
{
public:
	// Open the object
	Status Open(FF_Mem *p_mem, U32 num_units);

	// Close the object
	Status Close();

	void Start_Watchdog(
		Callback_Context *p_callback_context, 
		U32 device,
		U32 timer_ticks);

	void Wake_Up_Waiting_Context(U32 device, UI64 controller_status);

	// Return memory size required by object.
	static U32 Memory_Size();

private: // helper methods

	static void Watchdog_Expiration_Routine(U32 id);
	void Check_For_Timeout();

private: // member data

	// Number of units to watch.
	// Unit index is concatenation of array and column, e.g.,
	// ACCC if Array is one bit, and Column is 3 bits.
	U32						 m_num_units;

	// Each cell has a watchdog context associated with it. 
	// We start the timer when we wait for an interrupt.
	// If the timer expires, then we never got the interrupt.
	Callback_Context		*m_p_context[FF_NUM_UNITS_MAX];

	// Each cell has a watchdog counter associated with it. 
	// We set the counter when we wait for an interrupt.
	// If the counter expires, then we never got the interrupt.
	U32						 m_p_counter[FF_NUM_UNITS_MAX];

	FF_INTERRUPT_STATUS 	 m_pending_operations;

	// Watchdog timer.
	
#ifdef THREADX

	TX_TIMER			 m_watchdog_timer;

	// ThreadX disables and enables interrupts.
	U32				m_previous_interrupt_posture;
	
#define ENTER_CRITICAL_SECTION \
	m_previous_interrupt_posture = tx_interrupt_control(TX_INT_DISABLE);

#define LEAVE_CRITICAL_SECTION \
	tx_interrupt_control(m_previous_interrupt_posture);

#else

	// Nucleus or windows
	NU_TIMER			 	m_watchdog_timer;

#ifdef _WINDOWS
	CRITICAL_SECTION	m_critical_section;
#else

	// Nucleus
	U32			m_protect_struct;

#define ENTER_CRITICAL_SECTION \
	m_protect_struct = CriticalEnter();

#define LEAVE_CRITICAL_SECTION \
	CriticalLeave(m_protect_struct);
#endif // Nucleus
#endif // _WINDOWS

}; // FF_Watchdog


#endif // FfWatchdog_H

