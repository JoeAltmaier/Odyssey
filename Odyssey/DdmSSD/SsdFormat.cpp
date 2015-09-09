/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdFormat.cpp
// 
// Description:
// This file implements the Format method for the
// Solid State Drive DDM. 
//
// Update Log 
// 
// 02/27/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD1
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include "SsdDDM.h"
#include "SsdRequest.h"
#include "ErrorLog.h"

/*************************************************************************/
// SSD_Ddm::Process_Format_Request 
// Called by Chaos we receive a BSA format message.
// Called by Respond_To_BSA_Request when number of outsanding requests
// goes to zero, and we are waiting to process a format request.
/*************************************************************************/
STATUS SSD_Ddm::Process_Format_Request(Message *p_format_message)
{
	TRACE_ENTRY(SSD_Ddm::Process_Format_Request);

	// Save pointer to format message.
	// When this pointer is non null, we are either busy formatting, or 
	// we are waiting for outstanding requests to go to zero so that we
	// can begin formatting.
	m_p_format_message = p_format_message;
	
	// Are there any requests outstanding?
	if (m_num_requests_outstanding == 0)
	
		// If so, Process_Format_Request will be called by Respond_To_BSA_Request
		// when the number of requests goes to zero.
		return OK;
		
	// Increment the number of requests outstanding.
	m_num_requests_outstanding++;
	
	// Save pointer to message in callback context.
	m_request_context.m_p_message = m_p_format_message;
	
	// Save flash handle in callback context.
	m_request_context.m_flash_handle = m_flash_handle;
	
	// Start the format operation.  Call Format_Callback when complete.
	Tracef(EOL "Starting flash format...");
	Status status = FF_Format(
		m_flash_handle, 
		Get_Config(),
		&m_request_context,
		&Format_Callback);
	if (status != OK)
		Format_Callback(
			0, // transfer_byte_count
			0, // logical_byte_address
			status,
			&m_request_context);
	
	return status;
	
} // SSD_Ddm::Process_Format_Request

/*************************************************************************/
// Format_Callback is called by the flash file system when the
// format operation has completed.
/*************************************************************************/
void SSD_Ddm::Format_Callback(

	// Number of bytes successfully transferred
	U32 transfer_byte_count,
	
	// If operation did not succeed, logical byte address of failure.
	I64 logical_byte_address,

	// result of operation
	STATUS status,

	// pointer passed in to Flash File method
	void *p_context)
{
	TRACE_ENTRY(SSD_Ddm::Format_Callback);

	SSD_Request_Context *p_request_context = (SSD_Request_Context *)p_context;

	SSD_Ddm *p_ddm = p_request_context->m_p_ddm;

	// Save status in context for reply.
	p_request_context->Set_Status(status);
	
	// Save transfer byte count in context for reply.
	p_request_context->m_transfer_byte_count = transfer_byte_count;
	
	// Save logical byte address in context for reply.
	p_request_context->m_logical_byte_address = logical_byte_address;
	
	// Was this a format request? 
	if (p_ddm->m_p_format_message)
	
		p_ddm->Action(Respond_To_Format_Request, p_context);
	else
		p_ddm->Action(Process_Reformat, p_context);

} // Format_Callback

/*************************************************************************/
// SSD_Ddm::Respond_To_Format_Request 
// The Format request has been completed.
/*************************************************************************/
STATUS SSD_Ddm::Respond_To_Format_Request(void *p_context)
{
	TRACE_ENTRY(SSD_Ddm::Respond_To_Format_Request);

	SSD_Request_Context *p_request_context = (SSD_Request_Context *)p_context;

	SSD_Ddm *p_ddm = p_request_context->m_p_ddm;

	// Zero pointer to format message.
	// When this pointer is non null, we are busy formatting.
	CT_ASSERT(p_ddm->m_p_format_message, Respond_To_Format_Request);
	p_ddm->m_p_format_message = 0;
	
	// There should be one request outstanding.
	CT_ASSERT((p_ddm->m_num_requests_outstanding == 1), Respond_To_Format_Request);

	return p_ddm->Respond_To_BSA_Request(p_request_context);
	
} // Respond_To_Format_Request

/*************************************************************************/
// SSD_Ddm::Reformat
// Called by Process_Open_Flash if we were not able to open
// the flash file system.  The file system must be re-formatted.
/*************************************************************************/
STATUS SSD_Ddm::Reformat()	
{ 
	TRACE_ENTRY(SSD_Ddm::Reformat);

	// Start the format operation.  Call Format_Callback when complete.
	// Then call Process_Reformat.
	Tracef(EOL "Reformatting flash file system...");
	Status status = FF_Format(
		m_flash_handle, 
		Get_Config(),
		&m_request_context,
		&Format_Callback);
		
	return status;
		
} // SSD_Ddm::Reformat

/*************************************************************************/
// Process_Reformat 
// This Action callback is called by Chaos when the original open failed,
// the flash was reformatted, and the FF_Format operation has completed.
/*************************************************************************/
Status SSD_Ddm::Process_Reformat(void *p_context)
{ 
	TRACE_ENTRY(SSD_Ddm::Process_Reformat);

	SSD_Request_Context *p_request_context = (SSD_Request_Context *)p_context;

	SSD_Ddm *p_ddm = p_request_context->m_p_ddm;

	// We are enabling.
	CT_ASSERT(p_ddm->m_p_enable_message, SSD_Ddm::Process_Reformat);

	Status status = p_request_context->Get_Status();
	if (status != OK)
	{
		// We were not able to re-create. the flash.
		// Respond to the enable request with an error code.
		Tracef(EOL "Reformat flash file system failed, status = 0X%X", status);
		p_ddm->Respond_To_Enable_Request(p_request_context);
		return status;
	}
	
	// We were able to re-create the flash file system.
	Tracef(EOL "Flash file system reformatted.");

	p_ddm->Respond_To_Enable_Request(p_request_context);
	
	return status;
	
} // Process_Reformat

