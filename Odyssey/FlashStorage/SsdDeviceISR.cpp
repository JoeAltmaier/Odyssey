/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdDeviceISR.cpp
// 
// Description:
// This file implements interrupt handling for the Flash Storage device. 
// 
// 8/20/98 Jim Frandeen: Create file
// 6/30/99 Jim Frandeen: Implement interleaving
/*************************************************************************/
#define	TRACE_INDEX		TRACE_SSD
#include "FfPageMap.h"
#include "FfCommon.h"
#include "FfInterface.h"
#include "SsdDeviceISR.h"
#include "TraceMon.h"
#include "Callback.h"

#ifdef _WINDOWS
#include <stdio.h>
#define Tracef printf
#endif

#if 0
// Turn on TRACE_INTERRUPT
#define TRACE_INTERRUPT Tracef
#else
// Turn off TRACE_INTERRUPT -- if 1 then do nothing, else call Tracef
#define TRACE_INTERRUPT 1? 0 : Tracef  
#endif

#if 0
// Turn on TRACE_WAKEUP
#define TRACE_WAKEUP Tracef
#else
// Turn off TRACE_WAKEUP -- if 1 then do nothing, else call Tracef
#define TRACE_WAKEUP 1? 0 : Tracef  
#endif

/*************************************************************************/
// Forward References
/*************************************************************************/
#ifdef THREADX
extern "C" {
void	FF_ISR_High();
}

#else
void	FF_ISR_High();
#endif

void	FF_ISR_Low(INT vector_number);


/*************************************************************************/
// ISR globals
/*************************************************************************/
FF_INTERRUPT_CALLBACK	*FF_p_interrupt_callback;

#ifndef _WINDOWS
#ifndef THREADX
NU_HISR				  FF_HISR;					//  HISR object
VOID				 *FF_p_HISR_stack;			// points to HISR stack area
#endif
#endif

#ifdef _WINDOWS
	CRITICAL_SECTION	m_critical_section;
#else
	U32			m_protect_struct;
#endif

FF_INTERRUPT_STATUS		 FF_interrupt_status;
extern U32				 FF_disabled; // FPGA is disabled

/*************************************************************************/
// FF_ISR_Close
// Close FF_ISR object.
/*************************************************************************/
Status FF_ISR_Close()
{
 	TRACE_ENTRY(FF_ISR_Close);

	// Delete high level interrupt service routine 
	Status status;
#ifndef _WINDOWS
	status = NU_Delete_HISR(&FF_HISR);
		
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_ISR_Open", 
			"NU_Delete_HISR failed",
			status,
			0);
		return status;
	}
	
#endif

	return status;
	
} // FF_ISR_Close

/*************************************************************************/
// FF_ISR_Open
// Create FF_ISR object.
/*************************************************************************/
Status FF_ISR_Open(FF_Mem *p_mem, 
	FF_INTERRUPT_CALLBACK *p_interrupt_callback)
{
	Status			 status = OK;
#ifdef _WINDOWS
	InitializeCriticalSection(&m_critical_section);
#else
	ZERO(&m_protect_struct, sizeof(m_protect_struct));
#endif


#ifndef _WINDOWS
#ifndef THREADX
	VOID			(*old_lisr)(INT); // pointer to old LISR
	
 	TRACE_ENTRY(FF_ISR_Open);

    // Insure U32 alignment of config stack size. 
    U32 stack_size = ((FF_HISR_STACK_SIZE 
    	+ sizeof(U32) - 1) / sizeof(U32)) * sizeof(U32);
    	
	// Allocate stack for HISR
	FF_p_HISR_stack = p_mem->Allocate(stack_size);
	
	// Register low level interrupt service routine
	status = NU_Register_LISR(FF_INTERRUPT_VECTOR_NUMBER,
		FF_ISR_Low, &old_lisr);
		
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_ISR_Open", 
			"NU_Register_LISR failed",
			status,
			0);
		return status;
	}
	
	// Create high level interrupt service routine 
	status = NU_Create_HISR(&FF_HISR, 
		FF_HISR_NAME,	// name assigned to HISR
		FF_ISR_High, // function entry point for HISR
		FF_HISR_PRIORITY,
		FF_p_HISR_stack,
		stack_size);
		
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_ISR_Open", 
			"NU_Create_HISR failed",
			status,
			0);
		return status;
	}
		
#endif
#endif

	// Save pointer to interrupt callback.
	FF_p_interrupt_callback = p_interrupt_callback;
	
	return status;
	
} // FF_ISR_Open

/*************************************************************************/
// FF_ISR_Memory_Required
// Return memory required by device ISR.
/*************************************************************************/
U32 FF_ISR_Memory_Required()
{
    U32 stack_size = ((FF_HISR_STACK_SIZE 
    	+ sizeof(U32) - 1) / sizeof(U32)) * sizeof(U32);

	return stack_size;
}

/*************************************************************************/
// FF_ISR_High
// High Level Interrupt Service Routine
// gets activated by FF_ISR_Low.
// An interrupt occurs when a cell goes from busy to ready.
/*************************************************************************/
void FF_ISR_High(VOID)
{
	ENTER_CRITICAL_SECTION;
	
	// Get the interrupt status read by the low-level interrupt handler.
	FF_INTERRUPT_STATUS interrupt_status = FF_interrupt_status;
		
	// Zero the interrupt status.  
	// Now the low-level interrupt handler can set it again.
	FF_interrupt_status.source = 0;
	FF_interrupt_status.controller_status = 0;
	
	LEAVE_CRITICAL_SECTION;
	
	//TRACE(TRACE_SSD, TRACE_L5, EOL "FF_ISR_High, interrupt status = %I64X", interrupt_register); 

	// It is possible that there is no interrupt to be handled.  
	// It is possible that two interrupts could occur, the ISR would be 
	// scheduled twice, and the first call would handle both interrupts.

	// Find out which unit caused the interrupt.
	// Flash processor interrupt registers allow the flash controllers to 
	// specify which unit needs to be serviced by a flash processor 
	// interrupt. 
	// Interrupt status has one bit for each unit in each flash array.
	// A "1" bit indicates that an interrupt is pending.
	// Array 1, interleaved columns [7:0] interrupt pending is reg bits [15:08]
	// Array 0, interleaved columns [7:0] interrupt pending is reg bits [07:00]
	U32 unit_index = 0;
	while (interrupt_status.source)
	{
		if (interrupt_status.source & 1)
		
			// Call interrupt callback specified by FF_ISR_Open.
			// The interrupt callback wakes up the waiting context specified by unit_index
			// and passes in the controller_status.
			FF_p_interrupt_callback(unit_index, interrupt_status.controller_status);
		interrupt_status.source = (interrupt_status.source >> 1);
		unit_index++;
	}
								
} // FF_ISR_High

#ifndef _WINDOWS
#ifndef THREADX
/*************************************************************************/
// Low Level Interrupt Service Routine
/*************************************************************************/
void FF_ISR_Low(INT vector_number)
{
	CT_ASSERT((FF_disabled == 0), FF_ISR_Low);
	
	// Get interrupt status of page pusher. 
	FF_INTERRUPT_STATUS interrupt_status;
	FF_Get_Interrupt_Status(&interrupt_status);

	// A bit for each cell indicates whether the cell caused an interrupt
	// by going from busy to ready.
	// OR in the bit in case we get more than one interrupt per ISR.
	FF_interrupt_status.source |= interrupt_status.source;

	// OR status bits in case we get more than one interrupt per ISR.
	// We could only overlap a read or write with an erase completion,
	// and erase completion does not have any status bits.  This assures
	// that the correct status bits will go with the correct source.
	// Status for a read or write completion may be passed to an erase
	// completion, but the data will not be inspected or used.
	FF_interrupt_status.controller_status |= interrupt_status.controller_status;

	NU_Activate_HISR(&FF_HISR);
			
} // FF_ISR_Low
#endif
#endif


