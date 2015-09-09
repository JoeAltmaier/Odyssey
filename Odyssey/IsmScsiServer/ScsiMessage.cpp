/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiMessage.c
// 
// Description:
// This is the message interface for the SCSI target Server
// 
// Update Log 
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiMessage.cpp $
// 
// 8     11/15/99 4:08p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
// - Remove DoWork() and replace with RequestDefault() and ReplyDefault()
//
// 
// 12/10/98 Michael G. Panas: Create file
// 02/19/99 Michael G. Panas: convert to new Message format
/*************************************************************************/

#include "SCSIServ.h"
#include "ScsiSense.h"
#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

#include <string.h>

/*************************************************************************/
// Forward References
/*************************************************************************/

//************************************************************************
// ScsiReply(pMsg, Length)
// Reply to a SCSI Execute message with no errors
//************************************************************************
void ScsiServerIsm::ScsiReply(SCSI_CONTEXT *p_context, U32 Length)
{
	Message			*pReply = (Message *) p_context->pMsg;
	
	TRACE_ENTRY(ScsiReply);
					
	// add the payload to the reply message
	((Message *)p_context->pMsg)->AddReplyPayload(&Length, sizeof(U32));
	
	// set good status
	pReply->DetailedStatusCode =
					(FCP_SCSI_DSC_SUCCESS | FCP_SCSI_HBA_DSC_SUCCESS);
	
	// reply to original message
	Reply(p_context->pMsg);
}

//************************************************************************
// ScsiErrReply
// Reply to a SCSI Execute message with an error.  Build a request
// sense data structure to return in the reply.
// As a side effect, clear the SCSI status if AUTOREQUESTSENSE
// was set on the command.
//************************************************************************
void ScsiServerIsm::ScsiErrReply(SCSI_CONTEXT *p_context)
{
	Message					*pReply = (Message *) p_context->pMsg;
	SCB_REPLY_PAYLOAD		 payload;
	
	TRACE_ENTRY(ScsiErrReply);
	
	// zero the sense data
	memset(&payload.SenseData[0], 0, sizeof(REQUEST_SENSE));
				
	// build the sense data for this Virtual Circuit
	ScsiBuildSense((PREQUEST_SENSE)&payload.SenseData[0]);
	payload.AutoSenseTransferCount = sizeof(REQUEST_SENSE);
	payload.TransferCount = 0;			// no partial transfers on error
	
	// add the payload to the reply message
	((Message *)p_context->pMsg)->AddReplyPayload(&payload, sizeof(SCB_REPLY_PAYLOAD));
	
	// set check status
	pReply->DetailedStatusCode =
					(FCP_SCSI_DSC_CHECK_CONDITION | FCP_SCSI_HBA_DSC_SUCCESS);
	
	// reply to original message
	Reply(p_context->pMsg);
	
	// clear the sense codes
	SetStatus(SENSE_NO_SENSE, ASC_NO_ADDITIONAL_SENSE);
}


/*************************************************************************/
// ScsiSendMessage
// Send a message to the the next virtual device. No callback is used,
// handle reply in ReplyDefault()
/*************************************************************************/
STATUS ScsiServerIsm::ScsiSendMessage(SCSI_CONTEXT *p_context)
{
	STATUS			status;
	
	TRACE_ENTRY(ScsiSendMessage);

	status = Send((VDN)config.vdNext,
			 (Message *)p_context->p_bsaMsg,
			 (void *) p_context);

#if 0
 	if (status != NU_SUCCESS)
		XX_Log_Error(XX_ERROR_TYPE_FATAL,
			"SCSI_Send_Message", 
			"Send failed",
			myVd,
			(UNSIGNED)status);
#endif

return (status);
	
} // ScsiSendMessage



