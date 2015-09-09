/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: BsaIsm.cpp
//
// Description:
// This file is the implementation of a BSA ISM device.
//
// I2O_BSA_ messages are received by the DoWork method.  These messsages
// are then handled by translating to an I2O_SCSI_SCB_EXECUTE_MESSAGE.
// This message is then passed on to a Fibre Channel SCSI Initiator device.
//
// The config structure includes the Target Id and LUN that are passed in the
// TransactionContext field of the I2O_SCSI_SCB_EXECUTE_MESSAGE sent to
// the Initiator.
//
// $Log: /Gemini/Odyssey/IsmBsa/BsaIsm.cpp $
// 
// 26    12/02/99 3:04p Vnguyen
// Enable PHS Reporter hooks for performance and status counters.
// 
// 25    12/01/99 6:29p Mpanas
// Remove double delete of a message
// 
// 24    10/20/99 6:51p Mpanas
// Fix memory leak, reorganize to use
// RequestDefault() and ReplyDefault()
// instead of DoWork()
// 
// 23    9/05/99 2:02p Vnguyen
// Reduce the number of PHS Performance and Status counters to a minimum.
// Stop tracking max latency time.
// 
// 22    9/03/99 10:14a Vnguyen
// Fix bug in PHS_RETURN_RESET_PERFORMANCE.  Was returning m_Status
// instead of m_Perf.  Also, use DDM_REPLY_DATA_SGI instead of 0 for
// GetSgl call.
// 
// 21    8/25/99 11:44a Vnguyen
// Add timestamp and collect latency (&max) for BLOCK_READ and
// BLOCK_WRITE.
//
// Update Log: 
// 10/15/98 Michael G. Panas: Create file
// 01/26/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
// 02/12/99 Michael G. Panas: convert to new Oos model
// 02/17/99 Michael G. Panas: convert to new Message format and remove I2O
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_BSA
#include "Odyssey_Trace.h"

#include "Pci.h"
#include "Scsi.h"
#include "CDB.h"
#include "Odyssey.h"

#include "Message.h"
#include "FcpMessageFormats.h"

#include "BsaIsm.h"

#include "CTIdLun.h"
#include "BuildSys.h"

#include "RqDdmReporter.h"
#define PHS_REPORTER	// define this to activate the Reporter code

CLASSNAME(BsaIsm, MULTIPLE);

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/

/*************************************************************************/
// BsaIsm
// Constructor method for the class BsaIsm
/*************************************************************************/
BsaIsm::BsaIsm(DID did):Ddm(did) {

	TRACE_ENTRY(BsaIsm::BsaIsm);
	
	TRACE_HEX(TRACE_L6, "\n\rBsaIsm::BsaIsm this = ", (U32)this);
	TRACE_HEX(TRACE_L6, "\n\rBsaIsm::BsaIsm did = ", (U32)did);
	
	myVd = GetVdn();
	myDid = GetDid(); // stash this away for starting PHS Reporter later
	
	SetConfigAddress(&config, sizeof(config));
		
} // BsaIsm

/*************************************************************************/
// Ctor
// 
/*************************************************************************/
Ddm *BsaIsm::Ctor(DID did) {

	TRACE_ENTRY(BsaIsm::Ctor);
	
	return new BsaIsm(did); // create a new instance of this HDM
	
} // Ctor

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS BsaIsm::Initialize(Message *pMsg) {

	STATUS			 status;
	
	TRACE_ENTRY(BsaIsm::Initialize);
	
	TRACE_DUMP_HEX(TRACE_L6, "\n\rBsaIsm::Initialize config data  = ",
								(U8 *)&config, (U16)sizeof(config));
	
	// initialize our reporter members
	memset(&m_Status, 0, sizeof(BSA_ISMStatus));
	memset(&m_Perf, 0, sizeof(BSA_ISMPerformance));
	
	fStatusReporterActive = false;
	fPerformanceReporterActive = false;

	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Initialize

/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
STATUS BsaIsm::Enable(Message *pMsg) {

	TRACE_ENTRY(BsaIsm::Enable);

#ifdef PHS_REPORTER
// PHS Reporter ~~
	// We start the PHS Status and Performance Reporters here.
	// Start the Status Reporter
	RqDdmReporter *pReporterMsg;
	if (!fStatusReporterActive)
	{
 		pReporterMsg = new RqDdmReporter(PHS_START, PHS_DISK_STATUS, myDid, myVd);
		Send(pReporterMsg, (ReplyCallback) &DiscardReply);
		fStatusReporterActive = true;
		// There is no need to clear the counters as the Reporter will issue
		// a PHS_RESET code when it is ready to begin collecting data.
	} /* if */

	// Start the Performance Reporter
	if (!fPerformanceReporterActive)
	{
	 	pReporterMsg = new RqDdmReporter(PHS_START, PHS_DISK_PERFORMANCE, myDid, myVd);
		Send(pReporterMsg, (ReplyCallback) &DiscardReply);
		fPerformanceReporterActive = true;
		// There is no need to clear the counters as the Reporter will issue
		// a PHS_RESET code when it is ready to begin collecting data.
	} /* if */
	
#endif
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Enable

/*************************************************************************/
// Quiesce
// Turn off the DDM belonging to this derived class
/*************************************************************************/
STATUS BsaIsm::Quiesce(Message *pMsg) {

	TRACE_ENTRY(BsaIsm::Quiesce);
	
#ifdef PHS_REPORTER
// PHS Reporter ~~
	// We stop the PHS Status and Performance Reporters here.
	// Stop the Status Reporter
	RqDdmReporter *pReporterMsg;
	if (fStatusReporterActive)
	{
 		pReporterMsg = new RqDdmReporter(PHS_STOP, PHS_DISK_STATUS, myDid, myVd);
		Send(pReporterMsg, (ReplyCallback) &DiscardReply);
		fStatusReporterActive = false;
	} /* if */

	// Stop the Performance Reporter
	if (fPerformanceReporterActive)
	{
	 	pReporterMsg = new RqDdmReporter(PHS_STOP, PHS_DISK_PERFORMANCE, myDid, myVd);
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
STATUS BsaIsm::ReplyDefault(Message *pMsg) {
	STATUS status;
	
	TRACE_ENTRY(BsaIsm::ReplyDefault);
	
    TRACE_DUMP_HEX(TRACE_L8, "\n\rBsaIsm::ReplyDefault Message",
    					(U8 *)pMsg, 128);

	switch (pMsg->reqCode) {
	
	case SCSI_SCB_EXEC:
	{
		BSA_CONTEXT				*p_context;
		BSA_REPLY_PAYLOAD		 payload;
		SCB_REPLY_PAYLOAD		*p_rp = (SCB_REPLY_PAYLOAD *)pMsg->GetPPayload();
		
		TRACE_STRING(TRACE_L6, "\n\rBsaIsm::ReplyDefault SCSI_SCB_EXEC");
		
		// clear the payload 
		memset(&payload, 0, sizeof(BSA_REPLY_PAYLOAD));
		
		// recover the context pointer
		p_context = (BSA_CONTEXT *)pMsg->GetContext();
		
		// find out what we were up to
		switch (p_context->action) {
		
		case BSA_ACTION_FINISH:
		case BSA_ACTION_TEST_UNIT:
			Message		*pBsaReply = (Message *) p_context->pMsg;
			STATUS		status = pMsg->DetailedStatusCode;

			// translate status from SCB back to BSA
			// translate SCB Status to BSA status
			// No Xlate - just pass the Scsi error code back
			pBsaReply->DetailedStatusCode = status;
			
			if (status)
				m_Status.num_error_replies_received++;
			else
			{
				if (pBsaReply->reqCode == BSA_BLOCK_READ)
				{
					m_Perf.total_read_latency += pBsaReply->Latency();
					m_Perf.num_bytes_read += p_rp->TransferCount;
				}
				
				if (pBsaReply->reqCode == BSA_BLOCK_WRITE)
				{
					m_Perf.total_write_latency += pBsaReply->Latency();
					m_Perf.num_bytes_written += p_rp->TransferCount;
				}
			}
			
			// copy the transfer count
			payload.TransferCount = p_rp->TransferCount;

			if ((status & 0xff) == SCSI_STATUS_CHECK)
			{
				// for check, we will copy the sense data to the end of
				// the BSA ERROR frame so others can use it
				payload.AutoSenseTransferCount = p_rp->AutoSenseTransferCount;
				memcpy(&payload.SenseData,
							&p_rp->SenseData,
							payload.AutoSenseTransferCount);
			}
			else
				payload.AutoSenseTransferCount = 0;

			// add the payload to the reply message
			((Message *)p_context->pMsg)->
							AddReplyPayload(&payload, sizeof(BSA_REPLY_PAYLOAD));

			// reply to caller
			Reply(p_context->pMsg);
			
			// m_Status.num_replies_sent++;

			// de-allocate resources
			// delete the context
			delete p_context;

			break;

		}
	}
		break;
	
	case SCSI_DEVICE_RESET:
		// TODO:
		break;
		
 	} /* switch */

	// in any case, delete the reply message
	delete pMsg;
	
	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
} // ReplyDefault


/*************************************************************************/
// RequestDefault
// This derived classes method to receive messages
/*************************************************************************/
STATUS BsaIsm::RequestDefault(Message *pMsg) {
	STATUS status;
	
	TRACE_ENTRY(BsaIsm::RequestDefault);
	
    TRACE_DUMP_HEX(TRACE_L8, "\n\rBsaIsm::RequestDefault Message",
    					(U8 *)pMsg, 128);

	// New service message
	// Handle the BSA_xxxx messages
	switch(pMsg->reqCode) {

	case BSA_BLOCK_READ:
	{
		BSA_CONTEXT			*p_context;
		
		TRACE_STRING(TRACE_L6, "\n\rBsaIsm::RequestDefault BSA_BLOCK_READ");
		
		// m_Status.num_messages_received++;
		m_Perf.num_reads++;
		
		p_context = new BSA_CONTEXT;
		
		p_context->pMsg = pMsg;		// save original message
		p_context->action = BSA_ACTION_FINISH;
		p_context->p_sMsg =  new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE), MESSAGE_FLAGS_TIMESTAMP);
		
		// build the Read message
		BSA_Build_SCSI_RW_Message(p_context->p_sMsg, CMD_READ10, pMsg);
		
		// send to the Initiator driver
		status = BSA_Send_Message(p_context);
		return status;
		
	}
		break;

	case BSA_BLOCK_WRITE_VERIFY:		// TODO: 
	case BSA_BLOCK_WRITE:
	{
		BSA_CONTEXT			*p_context;
		
		TRACE_STRING(TRACE_L6, "\n\rBsaIsm::RequestDefault I2O_BSA_BLOCK_WRITE");
		
		// m_Status.num_messages_received++;
		m_Perf.num_writes++;
	
		p_context = new BSA_CONTEXT;
		
		p_context->pMsg = pMsg;		// save original message
		p_context->action = BSA_ACTION_FINISH;
		p_context->p_sMsg =  new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE), MESSAGE_FLAGS_TIMESTAMP);
		
		// build the Write message
		BSA_Build_SCSI_RW_Message(p_context->p_sMsg, CMD_WRITE10, pMsg);
		
		// send to the Initiator driver
		status = BSA_Send_Message(p_context);
		return status;
		
	}
		break;

	case BSA_POWER_MANAGEMENT:
	{
		BSA_CONTEXT			*p_context;
		CDB6				*pStUn;
		
		TRACE_STRING(TRACE_L6, "\n\rBsaIsm::RequestDefault BSA_POWER_MANAGEMENT");
		
		// m_Status.num_messages_received++;
		m_Perf.num_other_requests++;
	
		p_context = new BSA_CONTEXT;
		
		p_context->pMsg = pMsg;		// save original message
		p_context->action = BSA_ACTION_FINISH;
		p_context->p_sMsg =  new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE));
		
		// build a start unit message
		BSA_Build_SCSI_Message(p_context->p_sMsg, CMD_START_UNIT, 0, 0);
				
		pStUn = (CDB6 *) &((SCB_PAYLOAD*)((Message *)p_context->p_sMsg->GetPPayload()))->CDB;
		pStUn->Length = 1; // set Start bit
		//pStUn->MSB = 1;  // set IMMED bit
		
		// send to the Initiator driver
		status = BSA_Send_Message(p_context);
		return status;
		
	}
	
	case BSA_STATUS_CHECK:
	{
		BSA_CONTEXT			*p_context;
		
		TRACE_STRING(TRACE_L6, "\n\rBsaIsm::RequestDefault BSA_STATUS_CHECK");
		
		// m_Status.num_messages_received++;
		m_Perf.num_other_requests++;
	
		p_context = new BSA_CONTEXT;
		
		p_context->pMsg = pMsg;		// save original message
		p_context->action = BSA_ACTION_TEST_UNIT;
		p_context->p_sMsg =  new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE));
		
		// build a test unit message
		BSA_Build_SCSI_Message(p_context->p_sMsg, CMD_TEST_UNIT, 0, 0);
				
		// send to the Initiator driver
		status = BSA_Send_Message(p_context);
		return status;
		
	}
	
	case BSA_DEVICE_RESET:
	{
		BSA_CONTEXT			*p_context;
		
		TRACE_STRING(TRACE_L6, "\n\rBsaIsm::RequestDefault BSA_DEVICE_RESET");
		
		// m_Status.num_messages_received++;
		m_Perf.num_other_requests++;
	
		p_context = new BSA_CONTEXT;
		
		p_context->pMsg = pMsg;		// save original message
		p_context->action = BSA_ACTION_FINISH;
		
		// build a reset message
		p_context->p_sMsg =  new Message(SCSI_DEVICE_RESET, sizeof(FCP_MSG_SIZE));
		
		// send to the Initiator driver
		status = BSA_Send_Message(p_context);
		return status;
		
	}
	
	case PHS_RESET_STATUS:
	{
		TRACE_STRING(TRACE_L6, "\n\rBsaIsm::RequestDefault PHS_RESET_STATUS");
		
		// m_Perf.num_other_requests++;

		memset(&m_Status, 0, sizeof(BSA_ISMStatus));
		
		// reply to the caller
		status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		return status;
		
	}
	
	case PHS_RETURN_STATUS:
	{
		U8		*p;
		U32		cnt = sizeof(BSA_ISMStatus);
		
		TRACE_STRING(TRACE_L6, "\n\rBsaIsm::RequestDefault PHS_RETURN_STATUS");
		
		// m_Status.num_messages_received++;
		// m_Perf.num_other_requests++;
	
		// return the Status Structure using a dynamic reply structure
		pMsg->GetSgl(DDM_REPLY_DATA_SGI, &p, &cnt);
		
		memcpy(p, &m_Status, sizeof(BSA_ISMStatus));
		
		// reply to the caller
		status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		return status;
		
	}
	
	case PHS_RESET_PERFORMANCE:
	{
		TRACE_STRING(TRACE_L6, "\n\rBsaIsm::RequestDefault PHS_RESET_PERFORMANCE");
		
		// m_Status.num_messages_received++;
		// m_Perf.num_other_requests++;  We don't need this, do we??? [Vuong]
	
		memset(&m_Perf, 0, sizeof(BSA_ISMPerformance));
		
		// reply to the caller
		status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		return status;
		
	}
	
	case PHS_RETURN_PERFORMANCE:
	{
		U8		*p;
		U32		cnt = sizeof(BSA_ISMPerformance);
		
		TRACE_STRING(TRACE_L6, "\n\rBsaIsm::RequestDefault PHS_RETURN_PERFORMANCE");
		
		// m_Status.num_messages_received++;
		// m_Perf.num_other_requests++;
	
		// return the Performance Structure using a dynamic reply structure
		pMsg->GetSgl(DDM_REPLY_DATA_SGI, &p, &cnt);
		
		memcpy(p, &m_Perf, sizeof(BSA_ISMPerformance));
		
		// reply to the caller
		status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		return status;
		
	}
	
	case PHS_RETURN_RESET_PERFORMANCE:
	{
		U8		*p;
		U32		cnt = sizeof(BSA_ISMPerformance);
		
		TRACE_STRING(TRACE_L6, "\n\rBsaIsm::RequestDefault PHS_RETURN_RESET_PERFORMANCE");
		
		// m_Status.num_messages_received++;
		// m_Perf.num_other_requests++;
	
		// return the Performance Structure using a dynamic reply structure
		pMsg->GetSgl(DDM_REPLY_DATA_SGI, &p, &cnt);
		
		memcpy(p, &m_Perf, sizeof(BSA_ISMPerformance));
		
		// reply to the caller
		status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		
		// Now we do the reset portion of the command.
		memset(&m_Perf, 0, sizeof(BSA_ISMPerformance));
		
		return status;
		
	}
	
	// these messages are not supported
	case BSA_MEDIA_VERIFY:
	case BSA_MEDIA_UNLOCK:
	case BSA_MEDIA_MOUNT:
	case BSA_MEDIA_LOCK:
	case BSA_MEDIA_FORMAT:
	case BSA_MEDIA_EJECT:
	case BSA_BLOCK_REASSIGN:
	default:
		return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
		}

	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
} // RequestDefault



/*************************************************************************/
// BSA_Build_SCSI_RW_Message
// Build up the SCSI CDB, SGL and ByteCount in an SCB EXEC message
// used to  build read/write messages
/*************************************************************************/
void BsaIsm::BSA_Build_SCSI_RW_Message(Message *p_sMsg, SCSI_COMMANDS Cmd,
							Message *pMsg)
{
	BSA_RW_PAYLOAD					*pBSA = (BSA_RW_PAYLOAD *)pMsg->GetPPayload();
	U8								*pData;
	long							length;
	SCB_PAYLOAD						payload;
	
	TRACE_ENTRY(BSA_Build_SCSI_RW_Message);
	
	memset(&payload, 0, sizeof(SCB_PAYLOAD));
	
    // fill in the LUN and id fields
    payload.IdLun.id = config.ID;
    payload.IdLun.LUN = config.LUN;
    
	payload.SCBFlags = FCP_SCB_FLAG_SIMPLE_QUEUE_TAG;
	payload.ByteCount = pBSA->TransferByteCount;

	payload.CDB[0] = Cmd;
	payload.CDBLength = CDB_Get_CDB_Length((CDB16 *)&payload.CDB);
	CDB_Set_Transfer_Length((CDB16 *)&payload.CDB, payload.ByteCount / 512);
	
	// set the block address to the CDB
	CDB_Set_Logical_Block_Address((CDB16 *)&payload.CDB, pBSA->LogicalBlockAddress);
	
	p_sMsg->AddPayload(&payload, sizeof(SCB_PAYLOAD));

	U8 NumSGLs = pMsg->GetCSgl();
	for (U8 i = 0; i < NumSGLs; i++)
	{
		// get an SGL from the original message
		pMsg->GetSgl(i, (void **)&pData, (unsigned long *) &length);
		// add an SGL to the SCB message
		p_sMsg->AddSgl(i, (void *)pData, length);
	}
	
} // BSA_Build_SCSI_RW_Message

/*************************************************************************/
// BSA_Build_SCSI_Message
// Build up the SCSI CDB, SGL and ByteCount in an SCB EXEC message
/*************************************************************************/
void BsaIsm::BSA_Build_SCSI_Message(Message *pMsg, SCSI_COMMANDS Cmd,
							void *pData, long length)
{
	SCB_PAYLOAD				payload;
		
	TRACE_ENTRY(BSA_Build_SCSI_Message);
	
	memset(&payload, 0, sizeof(SCB_PAYLOAD));

    // fill in the LUN and id fields
    payload.IdLun.id = config.ID;
    payload.IdLun.LUN = config.LUN;
    
	payload.CDB[0] = Cmd;
	payload.CDBLength = CDB_Get_CDB_Length((CDB16 *)&payload.CDB);
	
	if (length)
	{
		payload.ByteCount = length;
		CDB_Set_Transfer_Length((CDB16 *)&payload.CDB, length);
	}
	else
	{
		payload.ByteCount = 0;
	}

	pMsg->AddPayload(&payload, sizeof(SCB_PAYLOAD));

	if (length)
	{
		pMsg->AddSgl(0, (void *)pData, length);
	}

} // BSA_Build_SCSI_Message


/*************************************************************************/
// BSA_Send_Message
// Send an SCB_EXECUTE message to the initiator driver
/*************************************************************************/
STATUS BsaIsm::BSA_Send_Message(BSA_CONTEXT *p_context)
{
	STATUS			status;
	
	TRACE_ENTRY(BSA_Send_Message);

    TRACE_DUMP_HEX(TRACE_L8, "\n\rBSA_Send_Message Message",
    					(U8 *)p_context->p_sMsg, 128);

	status = Send((VDN)config.initVd, (Message *)p_context->p_sMsg,
							(void *)p_context);

	// m_Status.num_messages_sent++;
	
#if 0
	// TODO:
 	if (status != NU_SUCCESS)
		BSA_Log_Error(BSA_ERROR_TYPE_FATAL,
			"BSA_Send_Message", 
			"Send failed",
			myVd,
			(UNSIGNED)status);
#endif

return (status);
	
} // BSA_Send_Message

