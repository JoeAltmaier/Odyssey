/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdQuiesce.cpp
// 
// Description:
// This file implements the Quiesce function for the
// Solid State Drive DDM.
//
// Quiesce must wait for the flash file system to close.
// The cache used by the flash file system must also be flushed and closed.
// 
// Update Log 
// 
// 03/1/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD1
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include "SsdDDM.h"
#include "SsdRequest.h"
#include "ErrorLog.h"
//#include "OsTypes.h"


/*************************************************************************/
// SSD_Ddm::Quiesce 
// The ddm must be put into a state so that it can be deleted.
/*************************************************************************/
STATUS SSD_Ddm::Quiesce(Message *p_quiesce_message)	// virtual
{
	TRACE_ENTRY(SSD_Ddm::Quiesce);

	// Save pointer to quiesce message.
	// When this pointer is non zero, we are processing a quiesce.
	CT_ASSERT(m_p_quiesce_message == 0, SSD_Ddm::Process_Quiesce);
	m_p_quiesce_message = p_quiesce_message;

	Tracef(EOL "Beginning flash file system quiesce.");

#ifdef PHS_REPORTER
	// Start the Status reporter
	RqDdmReporter *pRqReporter = new RqDdmReporter(PHS_STOP, PHS_SSD_STATUS, MyDid, MyVdn);
	Send(pRqReporter, (ReplyCallback) &DiscardReply);

	// Start the Performance reporter
	RqDdmReporter *pRqReporter = new RqDdmReporter(PHS_STOP, PHS_SSD_PERFORMANCE, MyDid, MyVdn);
	Send(pRqReporter, (ReplyCallback) &DiscardReply);
#endif

#if 0 // TEMPORARY always allow quiesce for debugging
	// Are there any requests outstanding?
	if (m_num_requests_outstanding)
	{
		// If so, Quiesce will be called by Respond_To_BSA_Request
		// when the number of requests goes to zero.
		Tracef(EOL "Waiting for %d outstanding requests to quiesce", m_num_requests_outstanding);
		return OK;
	}
#endif

	// Set up the callback context to call Process_Quiesce
	// when the file system has been closed.
	m_request_context.Set_Callback(&Process_Quiesce);

	// Start the close operation.
	FF_Close(m_flash_handle, &m_request_context);

	return OK;

} // SSD_Ddm::Quiesce

/*************************************************************************/
// SSD_Ddm::Process_Quiesce 
// Called by the flash file system when the close has been completed.
/*************************************************************************/
void SSD_Ddm::Process_Quiesce(void *p_context, Status status)
{
	TRACE_ENTRY(SSD_Ddm::Process_Quiesce);

	SSD_Request_Context *p_request_context = (SSD_Request_Context *)p_context;

	SSD_Ddm *p_ddm = p_request_context->m_p_ddm;

	CT_ASSERT((p_ddm->m_num_requests_outstanding == 0), SSD_Ddm::Process_Quiesce);
	CT_ASSERT(p_ddm->m_p_quiesce_message, SSD_Ddm::Process_Quiesce);

	if (status == OK)
		Tracef(EOL "Flash file system quiesced.");
	else
		Tracef(EOL "Flash file system quiesce failed, status = %d.", status);

	// Free all the memory that we allocated.
	delete (p_ddm->m_p_callback_context_memory);
	p_ddm->m_p_callback_context_memory = 0;
	
	delete (p_ddm->m_p_flash_file_system_memory);
	p_ddm->m_p_flash_file_system_memory = 0;

	// Did this quiesce message come from a reset?
	if (p_ddm->m_p_quiesce_message!= p_ddm->m_p_reset_message)

		// Let Chaos know we are quiesced.
		p_ddm->Reply(p_ddm->m_p_quiesce_message);
	else
	{
		// Reset the hardware
		WARM_RESET_REGISTER = 1;

		// Never returns!
	}

	// Zero pointer to quiesce message.
	// This was our mark on the wall to indicate we were processing a quiesce.
	p_ddm->m_p_quiesce_message = 0;

} // SSD_Ddm::Process_Quiesce
	
