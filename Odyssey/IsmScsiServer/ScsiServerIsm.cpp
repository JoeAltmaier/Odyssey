/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiServerIsm.cpp
// 
// Description:
// This is the SCSI Target Server (Sts) ISM definition.  The Target Server
// emulates a SCSI disk for each LUN/Initiator ID pair (Virtual Circuit).
// Only Read/Write commands are not handle here, these are passed on to
// the next Virtual device to be handled "by someone else".  A translation
// from SCB to BSA is done in this case.  The config structure includes a
// pointer to the next virtual device.
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiServerIsm.cpp $
// 
// 22    2/09/00 7:43p Dpatel
// moved tableInitialize from initialize to enable to avoid race
// condition. MP
// 
// 21    2/07/00 8:48p Cchan
// Added a message reply to reset messages sent from LoopMonitor.
// 
// 20    1/09/00 2:05p Mpanas
// Fix compiler warnings
// 
// 19    11/15/99 4:07p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
// - Remove DoWork() and replace with RequestDefault() and ReplyDefault()
// 
// 18    10/07/99 12:24p Vnguyen
// Add Status counters to keep track of # of error received and # of error
// generated internally within the SCSI Target Server.
// 
// 17    9/09/99 4:27p Vnguyen
// Add PHS code to increment performance and status counters.
// 
// 16    9/03/99 10:31a Vnguyen
// Add Support for PHS Reporter.  Start/Stop Reporter in Enable() and
// Quiesce().  Add support for PHS Request.
// 
// 15    8/08/99 12:49p Jlane
// changed vd to vdNext.  Hi MIke!
// 
// 14    7/21/99 6:04p Mpanas
// turn off Virtual Circuit check
// 
// 13    6/06/99 4:36p Mpanas
// Implement a method to check if a Virtual Circuit is Online.
// Send a BSA_STATUS_CHECK on the first
// SCSI_SCB_EXEC message, then set a 4 second timer.
// If the status comes back with an error or the timer expires, the VC
// will be macked OFFLINE otherwise the VC is ONLINE
// 
// This version does not update the Export Table (DEFERED)
// The changes to sequence the FCP Initiator are needed for this to work.
// 
// 12    4/22/99 4:53p Mpanas
// compare for the correct status field
// 
// 11    4/05/99 10:19p Mpanas
// Add Table support, read our Export table entry
// at init time
// 
// 06/05/98 Michael G. Panas: Create file
// 09/16/98 Michael G. Panas: Convert to new DDM model
// 12/10/98 Michael G. Panas: Convert to new BuildSys model
// 01/26/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
// 02/12/99 Michael G. Panas: convert to new Oos model
// 02/19/99 Michael G. Panas: convert to new Message format
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"

#include "SCSIServ.h"
#include "ScsiContext.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

#include "Pci.h"
#include "CTIdLun.h"

// Tables referenced
#include "ExportTable.h"

#include "BuildSys.h"

#include "RqDdmReporter.h"
//#define PHS_REPORTER	// define this to activate the Reporter code

CLASSNAME(ScsiServerIsm, MULTIPLE);

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// ScsiServerIsm
// Constructor method for the class ScsiServerIsm
/*************************************************************************/
ScsiServerIsm::ScsiServerIsm(DID did):Ddm(did) {

	TRACE_ENTRY(ScsiServerIsm::ScsiServerIsm);
	
	TRACE_HEX(TRACE_L6, "\n\rScsiServerIsm::ScsiServerIsm this = ", (U32)this);
	TRACE_HEX(TRACE_L6, "\n\rScsiServerIsm::ScsiServerIsm did = ", (U32)did);
	
	MyVdn = GetVdn();
	MyDid = GetDid(); // stash this away for starting PHS Reporter later
	
	SetConfigAddress(&config, sizeof(config));
	
}	// ScsiServerIsm

/*************************************************************************/
// Ctor
// Create a new instance of the ScsiServerIsm (SCSI Target Server)
/*************************************************************************/
Ddm *ScsiServerIsm::Ctor(DID did) {

	TRACE_ENTRY(ScsiServerIsm::Ctor);

	return new ScsiServerIsm(did);
	
}	// Ctor

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS ScsiServerIsm::Initialize(Message *pMsg) {

	TRACE_ENTRY(ScsiServerIsm::Initialize);

	TRACE_HEX(TRACE_L6, "\n\rScsiServerIsm::Initialize config.vd = ",
								(U32)config.vdNext);

	// initialize data structures
	pData = new StsData;
	
	// state is no status received yet
	m_VcStatus = VC_NO_STATUS;
	
	ScsiInitialize();
	
	// initialize our reporter members
	memset(&m_Status, 0, sizeof(STS_Status));
	memset(&m_Performance, 0, sizeof(STS_Performance));
	
	fStatusReporterActive = false;
	fPerformanceReporterActive = false;

	Reply(pMsg);
		
	return OS_DETAIL_STATUS_SUCCESS;
	
}	// Initialize

/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
STATUS ScsiServerIsm::Enable(Message *pMsg) {

	TRACE_ENTRY(ScsiServerIsm::Enable);

#ifdef PHS_REPORTER
// PHS Reporter ~~
	// We start the PHS Status and Performance Reporters here.
	// Start the Status Reporter
	RqDdmReporter *pReporterMsg;
	if (!fStatusReporterActive)
	{
 		pReporterMsg = new RqDdmReporter(PHS_START, PHS_SCSI_TARGET_SERVER_STATUS, MyDid, MyVdn);
		Send(pReporterMsg, (ReplyCallback) &DiscardReply);
		fStatusReporterActive = true;
		// There is no need to clear the counters as the Reporter will issue
		// a PHS_RESET code when it is ready to begin collecting data.
	} /* if */

	// Start the Performance Reporter
	if (!fPerformanceReporterActive)
	{
	 	pReporterMsg = new RqDdmReporter(PHS_START, PHS_SCSI_TARGET_SERVER_PERFORMANCE, MyDid, MyVdn);
		Send(pReporterMsg, (ReplyCallback) &DiscardReply);
		fPerformanceReporterActive = true;
		// There is no need to clear the counters as the Reporter will issue
		// a PHS_RESET code when it is ready to begin collecting data.
	} /* if */
	
#endif
	
	// Initialiaze our tables
	STSTableInitialize(pMsg);

#if !defined(NEED_TIMER)
	// always Online when no timer
	m_VcStatus = VC_ONLINE;
#endif // NEED_TIMER

	
	//Reply(pMsg);
	
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Enable

/*************************************************************************/
// Quiesce
// Turn off the DDM belonging to this derived class
/*************************************************************************/
STATUS ScsiServerIsm::Quiesce(Message *pMsg) {

	TRACE_ENTRY(ScsiServerIsm::Quiesce);

#ifdef PHS_REPORTER
// PHS Reporter ~~
	// We stop the PHS Status and Performance Reporters here.
	// Stop the Status Reporter
	RqDdmReporter *pReporterMsg;
	if (fStatusReporterActive)
	{
 		pReporterMsg = new RqDdmReporter(PHS_STOP, PHS_SCSI_TARGET_SERVER_STATUS, MyDid, MyVdn);
		Send(pReporterMsg, (ReplyCallback) &DiscardReply);
		fStatusReporterActive = false;
	} /* if */

	// Stop the Performance Reporter
	if (fPerformanceReporterActive)
	{
	 	pReporterMsg = new RqDdmReporter(PHS_STOP, PHS_SCSI_TARGET_SERVER_PERFORMANCE, MyDid, MyVdn);
		Send(pReporterMsg, (ReplyCallback) &DiscardReply);
		fPerformanceReporterActive = false;
	} /* if */
	
#endif

	
	Reply(pMsg);
	
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Quiesce

/*************************************************************************/
// ReplyDefault
// This derived classes method to receive replies
/*************************************************************************/
STATUS ScsiServerIsm::ReplyDefault(Message *pMsg) {

	SCSI_CONTEXT		*p_context;
	
	TRACE_ENTRY(ScsiServerIsm::ReplyDefault);

    TRACE_DUMP_HEX(TRACE_L8, "\n\ScsiServerIsm::ReplyDefault Message Reply",
    					(U8 *)pMsg, 128);

    if (pMsg->DetailedStatusCode)
    	m_Status.NumErrorRepliesReceived++;
			    					
	// recover the context pointer
	p_context = (SCSI_CONTEXT *)pMsg->GetContext();
			
	// handle reply from a message we sent
	switch (pMsg->reqCode) {
	
	case BSA_STATUS_CHECK:
		// got an answer from the Virtual Circuit
		// either good or bad
		
		TRACE_HEX(TRACE_L7, "\n\rScsiServerIsm::BSA_STATUS_CHECK reply status = ",
							(U32)pMsg->DetailedStatusCode);

		if (pMsg->DetailedStatusCode == OS_DETAIL_STATUS_SUCCESS)
		{
			// bring the circuit online
			m_VcStatus = VC_ONLINE;
		}
		else
		{
			// otherwise the circuit is offline
			m_VcStatus = VC_OFFLINE;
		}
		
		// kill the timer we started, if it is not 
		// already dead
		if (m_pStartTimerMsg)
		{
			Message *pKillTmr = new RqOsTimerStop(m_pStartTimerMsg);
			Send(pKillTmr);
			m_pStartTimerMsg = NULL;
		}
		
		// reply to the original message
		if (m_pMsg)
		{
			SCSI_CONTEXT	*p_context = (SCSI_CONTEXT *) new SCSI_CONTEXT;
			
			p_context->pMsg = m_pMsg;
			
			ScsiDecode(p_context);
			
			m_pMsg = NULL;
		}
		
		break;
		
	case REQ_OS_TIMER_START:
		// we have timed out on the virtual circuit, m_VcStatus
		// is already set to 0
		// it's dead jim...
		TRACE_HEX(TRACE_L7, "\n\rScsiServerIsm::REQ_OS_TIMER_START reply status = ",
							(U32)pMsg->DetailedStatusCode);

		m_Status.NumTimerTimeout++;

		// check if this is the kill reply...
		if (m_pStartTimerMsg)
		{
			// not kill reply
			m_pStartTimerMsg = NULL;
			
			// show timer timed out and
			// bring the circuit offline
			m_VcStatus = VC_TIMED_OUT;
		}
		
		// in any case, reply to the original message
		if (m_pMsg)
		{
			SCSI_CONTEXT	*p_context = (SCSI_CONTEXT *) new SCSI_CONTEXT;
			
			p_context->pMsg = m_pMsg;
			
			ScsiDecode(p_context);
			
			m_pMsg = NULL;
		}
		
		break;
		
	case REQ_OS_TIMER_STOP:
		// the timer is now stopped...
		TRACE_HEX(TRACE_L7, "\n\rScsiServerIsm::REQ_OS_TIMER_STOP reply status = ",
							(U32)pMsg->DetailedStatusCode);
		break;
		
	case BSA_POWER_MANAGEMENT:
	case BSA_DEVICE_RESET:
		break;
	}
	
	// in any case, delete the reply message
	delete pMsg;
	
	// Return success, we have already handled the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
} // ReplyDefault


	
/*************************************************************************/
// RequestDefault
// This derived classes method to receive messages
/*************************************************************************/
STATUS ScsiServerIsm::RequestDefault(Message *pMsg) {
	STATUS status;
	
	TRACE_ENTRY(ScsiServerIsm::RequestDefault);
	
    TRACE_DUMP_HEX(TRACE_L8, "\n\ScsiServerIsm::RequestDefault Message",
    					(U8 *)pMsg, 128);

	// New service message has been received
	switch(pMsg->reqCode) {

	case SCSI_SCB_EXEC:
	{
		SCSI_CONTEXT	*p_context;
		
#if defined(NEED_TIMER)
		if(m_VcStatus == VC_NO_STATUS)
		{
			Message		*pBsaMsg;
			U32			time = 4000000;	// default for no debug

		m_Performance.NumSCSICmds++;

			// save the SCB_EXEC message until we are
			// done with status state machine
			m_pMsg = pMsg;
			
			// Start a 4 second timer.  If this timer times out, the virtual
			// circuit is dead.
#if defined(_DEBUG)
			if (TraceLevel[TRACE_INDEX] >= 4)
				time = 10000000;	// 10 seconds when debug
#endif
			m_pStartTimerMsg = new RqOsTimerStart(time, 0);
			Send(m_pStartTimerMsg, this);
			
			// send a BSA_STATUS_CHECK message down the whole
			// virtual circuit to check if everyone is ready
			pBsaMsg = new Message(BSA_STATUS_CHECK, sizeof(FCP_MSG_SIZE));
			Send(config.vd, pBsaMsg, this);
			
			m_VcStatus = VC_STATUS_SENT;
			break;
		}
#endif // NEED_TIMER

		// allocate a context to work in
		p_context = (SCSI_CONTEXT *) new SCSI_CONTEXT;
		p_context->pMsg = pMsg;
		
		ScsiDecode(p_context);
	}
		break;

	case PHS_RESET_STATUS:
		TRACE_STRING(TRACE_L8, "\n\rScsiServerIsm::DoWork PHS_RESET_STATUS");
		memset(&m_Status, 0, sizeof(STS_Status));
	
		// reply to the caller
		status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		return status;

	case PHS_RETURN_STATUS:
	{
		U8		*p;
		U32		cnt = sizeof(STS_Status);
	
		TRACE_STRING(TRACE_L8, "\n\rScsiServerIsm::DoWork PHS_RETURN_STATUS");
	
		// return the Status Structure using a dynamic reply structure
		pMsg->GetSgl(DDM_REPLY_DATA_SGI, &p, &cnt);
	
		memcpy(p, &m_Status, sizeof(STS_Status));
	
		// reply to the caller
		status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		return status;
	}	
	case PHS_RESET_PERFORMANCE:
		TRACE_STRING(TRACE_L8, "\n\rScsiServerIsm::DoWork PHS_RESET_PERFORMANCE");
	
		memset(&m_Performance, 0, sizeof(STS_Performance));
	
		// reply to the caller
		status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		return status;
		
	case PHS_RETURN_PERFORMANCE:
	{	
		U8		*p;
		U32		cnt = sizeof(STS_Performance);
	
		TRACE_STRING(TRACE_L8, "\n\rScsiServerIsm::DoWork PHS_RETURN_PERFORMANCE");
	
		// return the Performance Structure using a dynamic reply structure
		pMsg->GetSgl(DDM_REPLY_DATA_SGI, &p, &cnt);
	
		memcpy(p, &m_Performance, sizeof(STS_Performance));
	
		// reply to the caller
		status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		return status;
	}	
	case PHS_RETURN_RESET_PERFORMANCE:
	{
		U8		*p;
		U32		cnt = sizeof(STS_Performance);
	
		TRACE_STRING(TRACE_L8, "\n\rScsiServerIsm::DoWork PHS_RETURN_RESET_PERFORMANCE");
	
		// return the Performance Structure using a dynamic reply structure
		pMsg->GetSgl(DDM_REPLY_DATA_SGI, &p, &cnt);
	
		memcpy(p, &m_Performance, sizeof(STS_Performance));
	
		// reply to the caller
		status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
	
		// Now we do the reset portion of the command.
		memset(&m_Performance, 0, sizeof(STS_Performance));
	
		return status;
	}			
	case SCSI_DEVICE_RESET:
	
		TRACE_STRING(TRACE_L8, "\n\rScsiServerIsm::DoWork SCSI_DEVICE_RESET");
		
		// for now, just reply to the reset message sent from LM
		status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		
		break;
		
	case SCSI_SCB_ABORT:
		break;
			
	default:
		return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
	}

	// Return success, we have already handled the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
}	// RequestDefault

