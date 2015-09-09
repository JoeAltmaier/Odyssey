/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfDebug.cpp
// 
// Description:
// This file implements debug methods for the Flash File. 
// 
// 07/30/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfCommon.h"
#include "FfInterface.h"

// Global is set by FF_Set_Break_Address and checked by FF_Check_Break_Address.
Flash_Address FF_break_address;

/*************************************************************************/
// FF_Break
// Set a break here.
/*************************************************************************/
void FF_Break()
{
}
	
/*************************************************************************/
// FF_Log_Error
/*************************************************************************/
void FF_Log_Error(char * message, char * testfilename, int lineno)
{
	FF_Break();
}

/*************************************************************************/
// FF_Check_Break_Address
// Can be called from anywhere when we want to see what happens to a
// particular flash address.
/*************************************************************************/
void FF_Check_Break_Address(Flash_Address flash_address)
{
	FF_Set_Break_Address();
	if (FF_break_address.Is_Null())
		return;
	if (flash_address == FF_break_address)
		FF_Break();
}
	
/*************************************************************************/
// FF_Check_Break_Block
/*************************************************************************/
void FF_Check_Break_Block(Flash_Address flash_address)
{
	FF_Set_Break_Address();
	if (FF_break_address.Is_Null())
		return;

	Flash_Address break_block = FF_break_address;
	break_block.Sector(0);
	flash_address.Sector(0);
	if (flash_address == break_block)
		FF_Break();
}
	
/*************************************************************************/
// FF_Set_Break_Address
// Change this code to set the global FF_break_address to a value to look for.
/*************************************************************************/
void FF_Set_Break_Address()
{
	FF_break_address.Array(0);
	FF_break_address.Block(0);
	FF_break_address.Column(0);
	FF_break_address.Sector(0);
}
	
/*************************************************************************/
// FF_Error
// Set a break here to break whenever we return an error code.
// The macro FF_ERROR(code) calls us.
/*************************************************************************/
Status FF_Error(Status status)
{
	if (status == FF_ERROR_CODE(NO_ERASED_PAGES))
		return status;
	FF_Break();
	return status;
}
	
/*************************************************************************/
// Break_Waiting_Context
// When Get_Statistics is called, we rifle through all the structures to
// find all waiting contexts.  Set a break here.
/*************************************************************************/
void FF_Break_Waiting_Context(Callback_Context *p_callback_context)
{
	FF_Controller_Context *p_controller_context = 
		(FF_Controller_Context *)p_callback_context;
	FF_Break();
}

/*************************************************************************/
// FF_Interface::Find_Waiting_Contexts()
// Called by FF_Get_Statistics.
/*************************************************************************/
void FF_Interface::Find_Waiting_Contexts()
{
	m_controller.Find_Waiting_Contexts();
	m_page_map.Find_Waiting_Contexts();
}

/*************************************************************************/
// FF_Controller::Find_Waiting_Contexts()
/*************************************************************************/
void FF_Controller::Find_Waiting_Contexts()
{
	Callback_Context *p_callback_context;

	// Anyone waiting for buss?
	if (!LIST_IS_EMPTY(&m_context_list_wait_buss))
	{
		p_callback_context = (Callback_Context *)
			LIST_POINT_HEAD(&m_context_list_wait_buss);

		// First context in list waiting for buss
		FF_Break_Waiting_Context(p_callback_context);
		while (!LIST_ENTRY_IS_LAST(&m_context_list_wait_buss, &p_callback_context->m_list))
		{
			p_callback_context = (Callback_Context *)LIST_POINT_NEXT(&p_callback_context->m_list);
			FF_Break_Waiting_Context(p_callback_context);
		}
	}

	// Anyone waiting for unit?
	U32 index;
	for (index = 0; index < Flash_Address::Num_Units(); index++)
	{
		if (!LIST_IS_EMPTY(&m_context_list_wait_unit[index]))
		{
			p_callback_context = (Callback_Context *)
				LIST_POINT_HEAD(&m_context_list_wait_unit[index]);

			// First context in list waiting for buss
			FF_Break_Waiting_Context(p_callback_context);
			while (!LIST_ENTRY_IS_LAST(&m_context_list_wait_unit[index], &p_callback_context->m_list))
			{
				p_callback_context = (Callback_Context *)LIST_POINT_NEXT(&p_callback_context->m_list);
				FF_Break_Waiting_Context(p_callback_context);
			}
		}
	}

	// Anyone waiting for command to complete?
	for (index = 0; index < Flash_Address::Num_Units(); index++)
	{
		if (m_p_controller_context[index])
		{
			FF_Break_Waiting_Context(m_p_controller_context[index]);
		}
	}

} // FF_Controller::Find_Waiting_Contexts()

/*************************************************************************/
// FF_Page_Map::Find_Waiting_Contexts()
/*************************************************************************/
void FF_Page_Map::Find_Waiting_Contexts()
{
	Callback_Context *p_callback_context;

	// Anyone waiting for erased page?
	if (!LIST_IS_EMPTY(&m_list_wait_erased_page))
	{
		p_callback_context = (Callback_Context *)
			LIST_POINT_HEAD(&m_list_wait_erased_page);

		// First context in list waiting for buss
		FF_Break_Waiting_Context(p_callback_context);
		while (!LIST_ENTRY_IS_LAST(&m_list_wait_erased_page, &p_callback_context->m_list))
		{
			p_callback_context = (Callback_Context *)LIST_POINT_NEXT(&p_callback_context->m_list);
			FF_Break_Waiting_Context(p_callback_context);
		}
	}

} // FF_Page_Map::Find_Waiting_Contexts()



