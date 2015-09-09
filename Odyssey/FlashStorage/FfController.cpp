/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfController.cpp
// 
// Description:
// This file implements the Controller methods of the Flash File
// Controller. 
// 
// 8/18/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfController.h"
#include "FfInterface.h"
#include "FfStats.h"

/*************************************************************************/
// FF_Controller::Initialize
// Initialize a FF_Controller object
/*************************************************************************/
Status	FF_Controller::Initialize(FF_Interface *p_flash)
{
 	TRACE_ENTRY(FF_Controller::Initialize);
 	
	LIST_INITIALIZE(&m_context_list_wait_buss);

	m_buss_context = 0;
	m_p_flash = p_flash;
	
 	// Initialize lists.
	for (U32 unit_index = 0; unit_index < Flash_Address::Num_Units(); unit_index++)
 	{
		LIST_INITIALIZE(&m_context_list_wait_unit[unit_index]);
		m_p_controller_context[unit_index] = 0;
 	}
 	
	// Can units overlap?
	// With the SSD, units can overlap operations.  It is possible to
	// erase unit 0 while unit 1 is performing another operation.
	// With the HBC, only one unit can be active at any one time.
	if (Flash_Address::Banks_Per_Array() == 1)
		m_can_units_overlap = 0;
	else
		m_can_units_overlap = 1;

	return OK;

} // FF_Controller::Initialize
	
	
/*************************************************************************/
// Is_Buss_Busy
// Check to see if controller is busy.
// If the controller is busy, queue the context on a wait list and 
// return true.  When the controller is no longer busy, the context will
// be put back on the ready list, and Is_Buss_Busy can be called
// again.
/*************************************************************************/
int FF_Controller::Is_Buss_Busy(
	FF_Controller_Context *p_controller_context)
{	
 	// Check to see if the controller is busy.
 	if (m_buss_context)
 	{
	 	TRACEF(TRACE_L5, 
			(EOL "Buss is busy, unit_index = %d", p_controller_context->m_flash_address.Unit_Index()));
 	
		m_p_flash->m_stats.Inc_Num_Waits_Buss();

 		// It is busy, so the context must wait.
 		// Put this context on the wait list for the controller.
	    LIST_INSERT_TAIL(&m_context_list_wait_buss, 
	    	&p_controller_context->m_list);
		
		return 1;
	
 	} // controller is busy
 	
	// Save pointer to context that owns the buss.
	m_buss_context = p_controller_context;
	return 0;

} // Is_Buss_Busy

/*************************************************************************/
// Is_Unit_Busy
// Check to see if logical unit_index is busy.
// If the unit_index is busy, queue the context on a wait list for this unit_index and 
// return true.  When the unit_index is no longer busy, the context will
// be put back on the ready list, and Is_Buss_Busy can be called
// again.
/*************************************************************************/
int FF_Controller::Is_Unit_Busy(
	FF_Controller_Context *p_controller_context)
{	
 	// Check to see if the unit_index is busy.
	// For the HBC, only one device can be accessed at one time.
	U32 unit_index = p_controller_context->m_flash_address.Unit_Index();
	if (m_can_units_overlap)
	{
		// Check this unit to see if it's busy.
 		if (m_p_controller_context[unit_index] != 0)
 		{
	 		TRACEF(TRACE_L5, 
				(EOL "Unit is busy, unit_index = %d", p_controller_context->m_flash_address.Unit_Index()));
 		
 			// It is busy, so the context must wait.
			m_p_flash->m_stats.Inc_Num_Waits_Unit(unit_index);

 			// Put this context on the wait list for the unit_index.
			LIST_INSERT_TAIL(&m_context_list_wait_unit[unit_index], 
	    		&p_controller_context->m_list);
						
			return 1;
		
 		} // unit_index is busy
 		
	}
	else
	{
		// Check to see if any unit is busy.
		U32 num_units = Flash_Address::Num_Units();
		for (U32 any_unit_index = 0; any_unit_index < num_units; any_unit_index++)
		{
 			if (m_p_controller_context[any_unit_index] != 0)
 			{
	 			TRACEF(TRACE_L5, 
					(EOL "Unit is busy, any_unit_index = %d", p_controller_context->m_flash_address.Unit_Index()));
 			
 				// A unit is busy, so the context must wait.
				m_p_flash->m_stats.Inc_Num_Waits_Unit(unit_index);

 				// Put this context on the wait list for the unit_index.
				LIST_INSERT_TAIL(&m_context_list_wait_unit[unit_index], 
	    			&p_controller_context->m_list);
							
				return 1;
			
 			} // unit_index is busy
		}
		// Continue if no unit is busy.
	}

	// Unit is not busy.
	// Save the pointer to the callback context that will be run
	// when the command finished.
	// The caller will start the command.

	m_p_controller_context[unit_index] = p_controller_context;
	return 0;

} // Is_Unit_Busy

/*************************************************************************/
// Mark_Buss_Not_Busy
// Called when transfer such as read or write has completed.
// Mark the buss not busy and check to see if another
// context is waiting for the buss.
/*************************************************************************/
void FF_Controller::Mark_Buss_Not_Busy(Flash_Address flash_address)
{
	// Turn off the busy flag.
	CT_ASSERT((m_buss_context), FF_Controller::Mark_Buss_Not_Busy);
	m_buss_context = 0;

	// See if another context is waiting for the buss.
	if (!LIST_IS_EMPTY(&m_context_list_wait_buss))
	{
		m_p_flash->m_stats.Dec_Num_Waits_Buss();

		FF_Controller_Context *p_controller_context;
		p_controller_context = (FF_Controller_Context *)LIST_REMOVE_HEAD(
			&m_context_list_wait_buss);
		
 		TRACEF(TRACE_L5, 
			(EOL "Mark_Buss_Not_Busy Making waiting context ready for unit_index = %d", 
			p_controller_context->m_flash_address.Unit_Index()));
 	
		// Queue context back on ready list for execution..
		p_controller_context->Make_Ready();
	}
		
} // FF_Controller::Mark_Buss_Not_Busy

/*************************************************************************/
// Mark_Unit_Not_Busy
// Called when operation such as read, write, erase has completed.
// Mark this unit_index not busy and check to see if another
// context is waiting for this unit_index.
/*************************************************************************/
void FF_Controller::Mark_Unit_Not_Busy(Flash_Address flash_address)
{
 	// Check to see if the unit_index is busy.
	// For the HBC, only one device can be accessed at one time.
	U32 unit_index = flash_address.Unit_Index();
	CT_ASSERT(unit_index < Flash_Address::Num_Units(), FF_Controller::Mark_Unit_Not_Busy);

	CT_ASSERT(FF_Controller::m_p_controller_context[unit_index], 
		FF_Controller::Mark_Unit_Not_Busy);
	
	// Set callback context for this unit_index to null because it is
	// no longer busy.
	m_p_controller_context[unit_index] = 0;
		
	if (m_can_units_overlap)
	{
		// See if another context is waiting for this unit_index.
		if (!LIST_IS_EMPTY(&m_context_list_wait_unit[unit_index]))
		{
			m_p_flash->m_stats.Dec_Num_Waits_Unit(unit_index);

			FF_Controller_Context *p_controller_context;
			p_controller_context = (FF_Controller_Context *)LIST_REMOVE_HEAD(
				&m_context_list_wait_unit[unit_index]);
			
 			TRACEF(TRACE_L5, 
				(EOL "Mark_Unit_Not_Busy Making context ready for unit_index = %d", 
				p_controller_context->m_flash_address.Unit_Index()));
 		
			// Queue context back on ready list for execution.
			p_controller_context->Make_Ready();
		}
	} // m_can_units_overlap

	else
	{
		// See if another context is waiting for any unit.
		U32 num_units = Flash_Address::Num_Units();
		for (U32 any_unit_index = 0; any_unit_index < num_units; any_unit_index++)
		{
			if (!LIST_IS_EMPTY(&m_context_list_wait_unit[any_unit_index]))
			{
				m_p_flash->m_stats.Dec_Num_Waits_Unit(any_unit_index);

				FF_Controller_Context *p_controller_context;
				p_controller_context = (FF_Controller_Context *)LIST_REMOVE_HEAD(
					&m_context_list_wait_unit[any_unit_index]);
				
 				TRACEF(TRACE_L5, 
					(EOL "Mark_Unit_Not_Busy Making context ready for unit_index = %d", 
					p_controller_context->m_flash_address.Unit_Index()));
 			
				// Queue context back on ready list for execution.
				p_controller_context->Make_Ready();
				return;
			}
		}
	} // NOT m_can_units_overlap
		
} // FF_Controller::Mark_Unit_Not_Busy

