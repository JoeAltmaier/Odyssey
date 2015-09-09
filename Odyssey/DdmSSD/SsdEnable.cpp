/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdEnable.cpp
// 
// Description:
// This file implements the Enable function for the
// Solid State Drive DDM. 

// Enable must wait for the flash file system to be opened.
// 
// Update Log 
// 
// 03/02/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD1
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include "SsdDDM.h"
#include "SsdRequest.h"
#include "ErrorLog.h"
#include "RequestCodes.h"
#include "StorageRollCallTable.h"
#include "OsTypes.h"

extern "C" U32 gSize_available_memory;
extern "C" U32 gSize_small_heap;

/*************************************************************************/
// SSD_Ddm::Enable -- gets called by Chaos
// before the first message is sent.
// Initialize the SSD object, allocate memory,
// start a context to initialize the flash file system.
/*************************************************************************/
STATUS SSD_Ddm::Enable(Message *p_enable_message)	// virtual
{
	TRACE_ENTRY(SSD_Ddm::Enable);

	// Save pointer to enable message so that
	// we can respond to it when we have finished enabling.
	m_p_enable_message = p_enable_message;
	CT_ASSERT(m_p_enable_message, SSD_Ddm::Enable);

	// Initialize all member variables.
	m_capacity = 0;
	m_p_callback_context_memory = 0;
	m_p_flash_file_system_memory = 0;
	m_p_quiesce_message = 0;
	m_p_format_message = 0;
	m_num_requests_outstanding = 0;
	m_flash_file_system_open = 0;
	m_p_HISR_stack = 0;

#ifdef PHS_REPORTER
	// Start the Status reporter
	RqDdmReporter *pRqReporter = new RqDdmReporter(PHS_START, PHS_SSD_STATUS, MyDid, MyVdn);
	Send(pRqReporter, (ReplyCallback) &DiscardReply);

	// Start the Performance reporter
	RqDdmReporter *pRqReporter = new RqDdmReporter(PHS_START, PHS_SSD_PERFORMANCE, MyDid, MyVdn);
	Send(pRqReporter, (ReplyCallback) &DiscardReply);
#endif

    // Allocate memory for callback contexts.
    STATUS status;
	m_p_callback_context_memory = new (tBIG) char[m_p_SSD_descriptor->callback_memory_size];
	if (m_p_callback_context_memory == 0)
	{
		status = CTS_FLASH_NO_MEMORY;
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"SSD_Ddm::Enable", 
			"new failed",
			status,
			0);
		return status;
	}

    // Initialize callbacks.
	status = Callback_Context::Initialize(m_p_callback_context_memory, 
		MEMORY_FOR_CALLBACKS, 
		140, // sizeof(FF_Bat_Context), 
		this);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Context_Task",
			"Callback_Context::Initialize failed",
			status,
			0);
		return status;
	}

	// Allocate memory for the flash file system as specified in the config.
	m_p_flash_file_system_memory = new (tBIG) char[m_p_SSD_descriptor->flash_config.memory_size];
	if (m_p_flash_file_system_memory == 0)
	{
		status = CTS_FLASH_NO_MEMORY;
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"SSD_Ddm::Enable",
			"new failed",
			status,
			0);
		// We are finished enabling.  Tell Chaos.
		Reply(p_enable_message);

		// Return error code.
		return status;
	}

	// Save pointer to this DDM so that scheduler context can signal us when a
	// request terminates.
	m_request_context.m_p_ddm = this;

	// When FF_Open has finished, call Process_Open_Flash.
	m_request_context.Set_Callback(&Process_Open_Flash);

	// Now it's OK to reset.
	m_OK_to_reset = 1;

	// Begin initializing the flash file system. 
	m_p_SSD_descriptor->flash_config.p_device = &m_device;
	m_p_SSD_descriptor->flash_config.p_memory = m_p_flash_file_system_memory;
    status =  FF_Open(
		&m_p_SSD_descriptor->flash_config,
	    &m_p_SSD_descriptor->cache_config,
		&m_request_context,
		&m_flash_handle);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"SSD_Ddm::Enable", 
			"FF_Initialize failed",
			status,
			0);

		// We are finished enabling.  Tell Chaos.
		m_request_context.Set_Status(status);
		Respond_To_Enable_Request(&m_request_context);

		// Return error code
		return status;
	}

	return status;

} // SSD_Ddm::Enable

/*************************************************************************/
// SSD_Ddm::Process_Open_Flash 
// Called by the flash file system when FF_Open has completed.
/*************************************************************************/
void SSD_Ddm::Process_Open_Flash(void *p_context, Status status)
{
	TRACE_ENTRY(SSD_Ddm::Process_Open_Flash);

	SSD_Request_Context *p_request_context = (SSD_Request_Context *)p_context;

	SSD_Ddm *p_ddm = p_request_context->m_p_ddm;

	// Check to see if the flash file system was opened.
	switch (status)
	{
		case OK:
			// The open has completed successfully.
			// Tell Chaos that the enable request was successful.
			Tracef(EOL "Open flash successful");
			p_ddm->Respond_To_Enable_Request(p_request_context);
			return;

		default:

			// If the table of contents is invalid, the 
			// flash file system is invalid.
			// We must re-create the flash file system.

			// TODO -- TEMPORARY
			// For now, we will create the flash file system.
			Tracef(EOL "Open flash failed, status = 0X%X", status);

			status = p_ddm->Reformat();
			if (status == OK)

				// Schedule the next context.
				return;
			break;
	}

	// We were not able to open the flash.
	// Tell Chaos that the enable request was not successful.
	p_ddm->Respond_To_Enable_Request(p_request_context);

} // SSD_Ddm::Process_Open_Flash

/*************************************************************************/
// SSD_Ddm::Respond_To_Enable_Request 
// The enable request has been completed.
/*************************************************************************/
STATUS SSD_Ddm::Respond_To_Enable_Request(SSD_Request_Context *p_request_context)
{
	TRACE_ENTRY(SSD_Ddm::Respond_To_Enable_Request);

	// Check to see if flash file system was successfully opened.
	STATUS status = p_request_context->Get_Status();
	if (status == OK)
	{
		Tracef(EOL "Flash file system opened.");
		m_flash_file_system_open = 1;

		// Find the capacity of the flash file system --
		// the number of blocks of user data available.
		FF_STATISTICS flash_statistics;
		status = FF_Get_Statistics(m_flash_handle, &flash_statistics, sizeof(flash_statistics));
		if (status == OK)
			m_capacity = (flash_statistics.page_size / 512) * flash_statistics.num_user_pages;

		// Update our config information.
		status = Ssd_Table_Modify_Capacity();
	}
	else
	{
		Tracef(EOL "Flash file system failed to open, status = 0X%X.", status);
	}
		
	// Did this quiesce message come from a reset?
	CT_ASSERT(m_p_enable_message, SSD_Ddm::Respond_To_Enable_Request);
	if (m_p_enable_message!= SSD_RESET_MESSAGE)

		// Let Chaos know we are ready for messages.
		Reply(m_p_enable_message);

	// Zero pointer to enable message.
	// This was our mark on the wall to indicate we were processing an enable.
	m_p_enable_message = 0;

	return status;

} // SSD_Ddm::Respond_To_Enable_Request
	

