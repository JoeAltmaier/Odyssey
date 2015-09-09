/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdRequest.cpp
// 
// Description:
// This file implements the BSA Request methods for the
// Solid State Drive DDM. 
//
// $Log: /Gemini/Odyssey/DdmSSD/SsdRequest.cpp $
// 
// 8     1/12/00 9:57a Jfrandeen
// Use SGL interface to flash storage; add traces, show status in hex
// 
// 7     12/14/99 2:08p Jfrandeen
// Trace invalid requests
// 
// 6     12/10/99 8:11p Jfrandeen
// Use messages from message file.
// 
// 5     12/10/99 1:13p Jfrandeen
// Decrement requests outstanding in case of error.
// 
// 4     10/21/99 1:03p Hdo
// Add code to support PHS Reporter
// 
// 02/27/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD1
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include "SsdDDM.h"
#include "SsdRequest.h"
#include "ErrorLog.h"
#include "FcpMessageFormats.h"
#include "I2omsg.h"
#include <string.h>

STATUS Translate_Status_To_BSA(STATUS status);

// TEMPORARY
void FF_Break();

/*************************************************************************/
// SSD_Ddm::Decrement_Requests_Outstanding 
// Called when we resply to a request.
/*************************************************************************/
void SSD_Ddm::Decrement_Requests_Outstanding()
{
	// Decrement the number of requests outstanding.
	CT_ASSERT(m_num_requests_outstanding, Respond_To_BSA_Request);
	m_num_requests_outstanding--;
	
	TRACEF(TRACE_L5, ("\nDecrement_Requests_Outstanding: requests outstanding = %d after decrement", 
		m_num_requests_outstanding));

	// Are there any requests outstanding?
	if (m_num_requests_outstanding == 0)
	{
		// Are we waiting to process a format request?
		if (m_p_format_message)
		{
			SSD_Ddm::Process_Format_Request(m_p_format_message);
			return;
		}

		// Are we waiting to process a quiesce request?
		if (m_p_quiesce_message)
		{
			Quiesce(m_p_quiesce_message);
			return;
		}
	}

} // Decrement_Requests_Outstanding


/*************************************************************************/
// SSD_Ddm::Process_BSA_Request 
// Called by Chaos we receive a BSA read or write message
/*************************************************************************/
Message *SSD_Ddm_p_message;	// Global for debugging
STATUS SSD_Ddm::Process_BSA_Request(Message *p_message)
{
	// Save p_message for debugging.
	SSD_Ddm_p_message = p_message;
	
	// Increment the number of requests outstanding.
	m_num_requests_outstanding++;

	TRACEF(TRACE_L5, ("\nProcess_BSA_Request: requests outstanding = %d, req_code = %X", 
		m_num_requests_outstanding, p_message->reqCode));

	// Check to be sure the flash file system is open.
	if (m_flash_file_system_open == 0)
	{
		Reply_With_Status(p_message, I2O_DETAIL_STATUS_DEVICE_NOT_AVAILABLE);
		return OK;
	}

	// Check to be sure we are not formatting.
	if (m_p_format_message)
	{
		Reply_With_Status(p_message, I2O_DEATIL_STATUS_DEVICE_BUSY);
		return OK;
	}

	// Is this a BSA_STATUS_CHECK?
	if (p_message->reqCode == BSA_STATUS_CHECK)
	{
		// BSA_STATUS_CHECK is a noop.
		Reply_With_Status(p_message, OK);
		return OK;
	}

	// Is this a BSA_DEVICE_RESET?
	if (p_message->reqCode == BSA_DEVICE_RESET)
	{
		// BSA_DEVICE_RESET is a noop.
		Reply_With_Status(p_message, OK);
		return OK;
	}

	// Is this a BSA_POWER_MANAGEMENT?
	if (p_message->reqCode == BSA_POWER_MANAGEMENT)
	{
		// BSA_POWER_MANAGEMENT is a noop.
		Reply_With_Status(p_message, OK);
		return OK;
	}

	// Create a callback context.
	SSD_Request_Context *p_request_context = 
		(SSD_Request_Context *)Callback_Context::Allocate(sizeof(SSD_Request_Context));
	if (p_request_context == 0)
	{
		// We could not allocate a callback context.
		Reply_With_Status(p_message, I2O_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT);
		return OK;
	}

	// Save flash handle in callback context.
	p_request_context->m_flash_handle = m_flash_handle;

	// Save pointer to message in callback context.
	p_request_context->m_p_message = p_message;

	// Save pointer to this DDM so that context can signal us when the
	// request terminates.
	p_request_context->m_p_ddm = this;

	// Get pointer to block storage payload
	BSA_RW_PAYLOAD *p_BSA = (BSA_RW_PAYLOAD *)p_message->GetPPayload();

	// Get an SGL from the original message.
	SGE_SIMPLE_ELEMENT* p_element = p_message->GetPSgl(0);
	
	// Get byte address from logical block address.
	I64 logical_byte_address = p_BSA->LogicalBlockAddress * 512;
	
	// Get count of SGL elements.
	U32 SGL_element_count = p_message->GetCSgl();
	
	STATUS status;
	
	// TEMPORARY for breaking
	if (SGL_element_count > 1)
	{
		FF_Break();
		status = OK;
	}

	switch (p_message->reqCode)
	{
	case BSA_BLOCK_READ:

		TRACEF(TRACE_L5, ("\nBSA_BLOCK_READ: TransferByteCount = 0X%X, LogicalBlockAddress = 0X%X, element count = %d", 
			p_BSA->TransferByteCount, p_BSA->LogicalBlockAddress, SGL_element_count));

		// Start the read from the flash file system.
		// Call Read_Write_Callback when the read has completed.
		status = FF_Read(
			m_flash_handle,
			p_element,
			p_BSA->TransferByteCount,
			logical_byte_address,
			p_request_context,
			&Read_Write_Callback);
		break;

	case BSA_BLOCK_WRITE:

		TRACEF(TRACE_L5, ("\nBSA_BLOCK_WRITE: TransferByteCount = 0X%X, LogicalBlockAddress = 0X%X, element count = %d", 
			p_BSA->TransferByteCount, p_BSA->LogicalBlockAddress, SGL_element_count));

		// Start the write to the flash file system.
		// Call Read_Write_Callback when the read has completed.
		status = FF_Write(
			m_flash_handle,
			p_element,
			p_BSA->TransferByteCount,
			logical_byte_address,
			p_request_context,
			&Read_Write_Callback);
		break;

	default:

		// We should not get any other request codes.
		CT_ASSERT(p_message->reqCode, SSD_Ddm::Process_BSA_Request);
		Tracef("\nInvalid request code = %X", p_message->reqCode);
		status = OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;

	} // switch (p_message->reqCode)

	if (status != OK)

		// We were not able to start the flash operation.
		Reply_With_Status(p_message, Translate_Status_To_BSA(status));

	return OK;

} // SSD_Ddm::Process_BSA_Request

/*************************************************************************/
// Read_Write_Callback is called by the flash file system when the
// read or write has completed.
/*************************************************************************/
void SSD_Ddm::Read_Write_Callback(

	// Number of bytes successfully transferred
	U32 transfer_byte_count,

	// If operation did not succeed, logical byte address of failure.
	I64 logical_byte_address,

	// result of operation
	STATUS status,

	// pointer passed in to Flash File method
	void *p_context)
{
	TRACEF(TRACE_L5, ("\nSSD_Ddm::Read_Write_Callback: transfer_byte_count = %d, status = %X", 
		transfer_byte_count, status));

	SSD_Request_Context *p_request_context = (SSD_Request_Context *)p_context;

	// Save status in context for reply.
	p_request_context->Set_Status(status);

	// Save transfer byte count in context for reply.
	p_request_context->m_transfer_byte_count = transfer_byte_count;

	// Save logical byte address in context for reply.
	p_request_context->m_logical_byte_address = logical_byte_address;

	// Respond to the request.
	p_request_context->m_p_ddm->Respond_To_BSA_Request(p_request_context);

} // SSD_Ddm::Read_Write_Callback

/*************************************************************************/
// SSD_Ddm::Respond_To_BSA_Request 
// The BSA request has been completed.
/*************************************************************************/
STATUS SSD_Ddm::Respond_To_BSA_Request(SSD_Request_Context *p_request_context)
{
	TRACE_ENTRY(SSD_Ddm::Respond_To_BSA_Request);

	// Point to original message.  This will be the reply.
	Message *p_reply = p_request_context->m_p_message;

	// Set the status of the reply.
	p_reply->DetailedStatusCode = Get_BSA_Status(p_request_context);

	// Clear the reply payload 
	BSA_REPLY_PAYLOAD reply_payload;
	memset(&reply_payload, 0, sizeof(BSA_REPLY_PAYLOAD));

	// Save transfer byte count in reply.
	reply_payload.TransferCount = p_request_context->m_transfer_byte_count;

	// Save logical block address in context for reply.
	reply_payload.LogicalBlockAddress = p_request_context->m_logical_byte_address / 512;

	// Add the payload to the reply message.
	p_reply->AddReplyPayload(&reply_payload, sizeof(BSA_REPLY_PAYLOAD));

	TRACEF(TRACE_L5, ("\nRespond_To_BSA_Request: TransferCount = %X, LogicalBlockAddress = %X", 
		reply_payload.TransferCount, reply_payload.LogicalBlockAddress));

	// Reply to caller
	Reply(p_reply);

	Decrement_Requests_Outstanding();

	TRACEF(TRACE_L5, ("\nRespond_To_BSA_Request: Return from reply, requests outstanding = %d", 
		m_num_requests_outstanding));

	// Terminate the context.
	Callback_Context::Terminate(p_request_context, OK);
	
	return OK;

} // SSD_Ddm::Respond_To_BSA_Request


/*************************************************************************/
// SSD_Ddm::Get_BSA_Status 
// Translate a SSD status code to a BSA status code.
/*************************************************************************/
STATUS SSD_Ddm::Get_BSA_Status(SSD_Request_Context *p_request_context)
{
	// Get status from parent callback context.
	STATUS status = p_request_context->Get_Status();

	return Translate_Status_To_BSA(status);

} // SSD_Ddm::Get_BSA_Status

/*************************************************************************/
// SSD_Ddm::Reply_With_Status 
// Reply immediately to a message if we are unable to 
// transfer any data.
/*************************************************************************/
void SSD_Ddm::Reply_With_Status(Message *p_message, STATUS status)
{
	TRACEF(TRACE_L5, ("\nReply_With_Status: req_code = %X, status = %X", 
		p_message->reqCode, status));

	// Set the status of the reply.
	p_message->DetailedStatusCode = status;

	// Get pointer to block storage request payload.
	BSA_RW_PAYLOAD *p_BSA = (BSA_RW_PAYLOAD *)p_message->GetPPayload();

	// Clear the reply payload.
	BSA_REPLY_PAYLOAD reply_payload;
	memset(&reply_payload, 0, sizeof(BSA_REPLY_PAYLOAD));

	// Set zero transfer byte count in reply.
	reply_payload.TransferCount = 0;

	// Save logical block address in context for reply.
	reply_payload.LogicalBlockAddress = p_BSA->LogicalBlockAddress;

	// Add the payload to the reply message.
	p_message->AddReplyPayload(&reply_payload, sizeof(BSA_REPLY_PAYLOAD));

	// Reply to caller
	Reply(p_message);
	
	Decrement_Requests_Outstanding();

	TRACEF(TRACE_L5, ("\nReply_With_Status: Return from reply, requests outstanding = %d", 
		m_num_requests_outstanding));

} // SSD_Ddm::Reply_With_Status

STATUS Translate_Status_To_BSA(STATUS status)
{
	// TODO
	return status;
	
} // Translate_Status_To_BSA
	
/*************************************************************************/
// SSD_Ddm::Process_PHS_Request 
// Handle PHS request
/*************************************************************************/
STATUS SSD_Ddm::Process_PHS_Request(Message *pMsg)
{
	TRACE_ENTRY(SSD_Ddm::Process_PHS_Request);

	STATUS status;
	U32 cbData;
	U8	*pData;

	FF_STATISTICS flash_statistics;
	status = FF_Get_Statistics(m_flash_handle, &flash_statistics, sizeof(flash_statistics));

	// Check to be sure the flash file system is open.
	if (m_flash_file_system_open == 0)
	{
		Reply(pMsg, I2O_DETAIL_STATUS_DEVICE_NOT_AVAILABLE);
		return OK;
	}

	switch (pMsg->reqCode)
	{
	case PHS_RESET_STATUS:
		TRACE_STRING(TRACE_L8, "\n\rSSD::Process PHS_RESET_STATUS");
		// TODO: send a FF_Stats::Reset_Event_Data to FF_Stats class
		memset(&m_Status, 0, sizeof(SSD_STATUS));

		status = Reply(pMsg, OK);
		break;

	case PHS_RETURN_STATUS:
		TRACE_STRING(TRACE_L8, "\n\rSSD::Process PHS_RETURN_STATUS");

		// Update the Status record
		m_Status.NumReplacementPagesAvailable = flash_statistics.num_replacement_pages_available;
		//m_Status.PercentDirtyPage = CM_STATISTICS::num_pages_dirty;

		pMsg->GetSgl(DDM_REPLY_DATA_SGI, &pData, &cbData);

		memcpy(pData, &m_Status, sizeof(SSD_STATUS));

		status = Reply(pMsg, OK);
		break;

	case PHS_RESET_PERFORMANCE:
		TRACE_STRING(TRACE_L8, "\n\rSSD::Process PHS_RESET_PERFORMANCE");
		// TODO: send a FF_Stats::Reset_Event_Data to FF_Stats class
		memset(&m_Performance, 0, sizeof(SSD_PERFORMANCE));
		
		status = Reply(pMsg, OK);
		break;

	case PHS_RETURN_PERFORMANCE:
		TRACE_STRING(TRACE_L8, "\n\rSSD::Process PHS_RETURN_PERFORMANCE");

		// Update the performance record
		m_Performance.NumPagesRead = flash_statistics.num_page_reads;
		m_Performance.NumPagesReadCacheHit = flash_statistics.num_page_reads_cache_hit;
		m_Performance.NumPagesReadCacheMiss = flash_statistics.num_page_reads_cache_miss;

		m_Performance.NumPagesWrite = flash_statistics.num_page_writes;
		m_Performance.NumPagesWriteCacheHit = flash_statistics.num_page_writes_cache_hit;
		m_Performance.NumPagesWriteCacheMiss = flash_statistics.num_page_writes_cache_miss;

		m_Performance.NumReadBytesTotal = flash_statistics.num_read_bytes_total;
		m_Performance.NumWriteBytesTotal = flash_statistics.num_write_bytes_total;

		pMsg->GetSgl(DDM_REPLY_DATA_SGI, &pData, &cbData);

		memcpy(pData, &m_Status, sizeof(SSD_STATUS));

		status = Reply(pMsg, OK);
		break;

	case PHS_RETURN_RESET_PERFORMANCE:
		TRACE_STRING(TRACE_L8, "\n\rSSD::Process PHS_RETURN_RESET_PERFORMANCE");

		pMsg->GetSgl(DDM_REPLY_DATA_SGI, &pData, &cbData);

		memcpy(pData, &m_Status, sizeof(SSD_STATUS));

		status = Reply(pMsg, OK);

		// TODO: send a FF_Stats::Reset_Event_Data to FF_Stats class
		memset(&m_Performance, 0, sizeof(SSD_PERFORMANCE));
		break;
	} // switch (pMsg->reqCode)

	return status;

} // SSD_Ddm::Process_PHS_Request
