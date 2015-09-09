/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdCreate.cpp
// 
// Description:
// This file creates the flash file system for the
// Solid State Drive DDM. 
//
// Update Log 
// 
// 04/26/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD1
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include "SsdDDM.h"
#include "SsdRequest.h"
#include "ErrorLog.h"

/*************************************************************************/
// SSD_Ddm::Create
// called by Process_Open_Flash if we were not able to open
// the flash file system.  The file system must be re-created.
/*************************************************************************/
STATUS SSD_Ddm::Create(SSD_Request_Context *p_request_context)	
{ 
	TRACE_ENTRY(SSD_Ddm::Create);

	// When FF_Open has finished, call Process_Open_Flash.
	m_request_context.Set_Callback(&Process_Create_Flash);
	
	// Begin creating the flash file system. 
	Tracef(EOL "Recreating flash file system...");
    Status status =  FF_Create(
		&m_config.flash_config,
	    &m_config.cache_config,
	    &m_device,
		m_p_flash_file_system_memory,
		p_request_context);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"SSD_Ddm::Enable", 
			"FF_Create failed",
			status,
			0);
	}
	
	return status;
		
} // SSD_Ddm::Create

/*************************************************************************/
// Process_Create_Flash 
// This callback is called by the flash file system when the
// FF_Create operation has completed.
/*************************************************************************/
void SSD_Ddm::Process_Create_Flash(void *p_context, Status status)
{ 
	TRACE_ENTRY(SSD_Ddm::Process_Create_Flash);

	SSD_Request_Context *p_request_context = (SSD_Request_Context *)p_context;

	SSD_Ddm *p_ddm = p_request_context->m_p_ddm;

	if (status != OK)
	{
		// We were not able to re-create. the flash.
		// Respond to the enable request with an error code.
		Tracef(EOL "Create flash file system failed, status = %d", status);
		p_ddm->Respond_To_Enable_Request(p_request_context);
		return;
	}
	
	// We were able to re-create the flash file system.
	Tracef(EOL "Flash file system recreated.");

	// Now try to open it again.
	// When FF_Open has finished, call Process_Open_Flash.
	p_request_context->Set_Callback(&Process_Open_Flash);
	
	// Begin initializing the flash file system. 
    status =  FF_Open(
		&p_ddm->m_config.flash_config,
	    &p_ddm->m_config.cache_config,
	    &p_ddm->m_device,
		p_ddm->m_p_flash_file_system_memory,
		p_request_context,
		&p_ddm->m_flash_handle);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"SSD_Ddm::Enable", 
			"FF_Initialize failed",
			status,
			0);
			
		// We were not able to open the flash.
		// Respond to the enable request with an error.
		p_ddm->Respond_To_Enable_Request(p_request_context);
	}
	
	
} // Process_Create_Flash

