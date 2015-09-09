/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiRdWr.c
// 
// Description:
// Handle Reads and Writes that must be forwarded to another context 
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiRdWr.cpp $
// 
// 15    12/01/99 6:22p Mpanas
// Remove double delete of a message
// 
// 14    11/16/99 9:21p Mpanas
// Add in James Taylor's SGL changes
// 
// 13    11/15/99 6:51p Mpanas
// Add return value for Callbacks
// 
// 12    11/15/99 4:08p Mpanas
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
// 06/01/98 Michael G. Panas: Create file
// 12/10/98 Michael G. Panas: rewrite to use Message.h and new STS headers
// 02/19/99 Michael G. Panas: convert to new Message format
/*************************************************************************/

#include "SCSIServ.h"
#include "ScsiSense.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

#include "CDB.h"

/***************************************************************************/
// Forward References
/***************************************************************************/


//**************************************************************************
//
//	ScsiServerRead
//	Convert the SCSI_SCB_EXECUTE message to an I2O BSA READ message and
//	send to the next route.  The SGL points to thge buffer to transfer
//  the data to.
//**************************************************************************
U32
ScsiServerIsm::ScsiServerRead(SCSI_CONTEXT *p_context)
{
	Message							*pRdMess;
	STATUS							status;
	BSA_RW_PAYLOAD					payload;
	U8								*pData;
	U8								i, NumSGLs;
	long							length;
	SCB_PAYLOAD						*pP = 
				(SCB_PAYLOAD *)((Message *)p_context->pMsg)->GetPPayload();
	
	TRACE_ENTRY(ScsiServerRead);
	
	p_context->p_bsaMsg =  new Message(BSA_BLOCK_READ, sizeof(FCP_MSG_SIZE));
	pRdMess = (Message *) p_context->p_bsaMsg;
	
	payload.ControlFlags = FCP_SCB_FLAG_SIMPLE_QUEUE_TAG;
	payload.TimeMultiplier = 0;
	payload.FetchAhead = 0;			// TODO: how should we use this?
	payload.TransferByteCount = pP->ByteCount;
	payload.LogicalBlockAddress = CDB_Get_Logical_Block_Address((CDB16 *)&pP->CDB);

	((Message *)pRdMess)->AddPayload(&payload, sizeof(BSA_RW_PAYLOAD));

	NumSGLs = ((Message *)p_context->pMsg)->GetCSgl();
 	for (i = 0; i < NumSGLs; i++)
	{
		// Get SGL entry
		((Message *)p_context->pMsg)->GetSgl(i, (void**)&pData, (unsigned long *) &length);
		// Add SGL entry to the BSA message
		((Message *)pRdMess)->AddSgl(i, (void *)pData, length, SGL_REPLY);
	}
	
	// send to destination
	status = Send((VDN)config.vdNext,
					p_context->p_bsaMsg,
					(void *) p_context,
					(ReplyCallback)&ScsiServerReadCallback );
	
	if (status)
		TRACE_HEX(TRACE_L2, "\n\rScsiServerRead: send failed ", status);
	
	return(status);
	
} // ScsiServerRead


//**************************************************************************
//
//	ScsiServerWrite
//	Convert the SCSI_SCB_EXECUTE message to an BSA WRITE message and
//	send to the next route.  THe SGLs will point to the buffers that have
//  the data.
//**************************************************************************
U32
ScsiServerIsm::ScsiServerWrite(SCSI_CONTEXT *p_context)
{
	Message							*pWrMess;
	STATUS							status;
	BSA_RW_PAYLOAD					payload;
	U8								*pData;
	U8								i, NumSGLs;
	long							length;
	SCB_PAYLOAD						*pP = 
				(SCB_PAYLOAD *)((Message *)p_context->pMsg)->GetPPayload();
	
	TRACE_ENTRY(ScsiServerWrite);
	
	p_context->p_bsaMsg =  new Message(BSA_BLOCK_WRITE, sizeof(FCP_MSG_SIZE));
	pWrMess = (Message *) p_context->p_bsaMsg;
	
	payload.ControlFlags = FCP_SCB_FLAG_SIMPLE_QUEUE_TAG;
	payload.TimeMultiplier = 0;
	payload.FetchAhead = 0;			// unused for writes
	payload.TransferByteCount = pP->ByteCount;
	payload.LogicalBlockAddress = CDB_Get_Logical_Block_Address((CDB16 *)&pP->CDB);

	((Message *)pWrMess)->AddPayload(&payload, sizeof(BSA_RW_PAYLOAD));

	NumSGLs = ((Message *)p_context->pMsg)->GetCSgl();
 	for (i = 0; i < NumSGLs; i++)
	{
		// Get SGL entry
		((Message *)p_context->pMsg)->GetSgl(i, (void**)&pData, (unsigned long *) &length);
		// Add SGL entry to the BSA message
		((Message *)pWrMess)->AddSgl(i, (void *)pData, length, SGL_REPLY);
	}

	// send to destination
	status = Send((VDN)config.vdNext,
					p_context->p_bsaMsg,
					(void *) p_context,
					(ReplyCallback)&ScsiServerWriteCallback );
	
	if (status)
		TRACE_HEX(TRACE_L2, "\n\rScsiServerWrite: send failed ", status);
	
	return(status);
	
} // ScsiServerWrite



/*************************************************************************/
// ScsiServerReadCallback
//  Wait for the reply Callback.  Then translate the BSA status
//	to a SCSI SCB EXECUTE reply status and answer the original message.
/*************************************************************************/
STATUS ScsiServerIsm::ScsiServerReadCallback(Message *pMsg)
{
	SCSI_CONTEXT 		*p_context = (SCSI_CONTEXT *)pMsg->GetContext();
	SCB_REPLY_PAYLOAD	 payload;
	Message				*pRf = (Message *) p_context->pMsg;
	BSA_REPLY_PAYLOAD 	*pBsaPayload = (BSA_REPLY_PAYLOAD *) pMsg->GetPPayload();;
	
	TRACE_ENTRY(ScsiServerReadCallback);

#if 1
	// This section used to debug data transfer problems
{
	U8		*pData;
	long	length;

	// get first SGL from the original message
	((Message *)p_context->pMsg)->GetSgl(0, (void**)&pData, (unsigned long *) &length);
    TRACE_DUMP_HEX(TRACE_L4, "\n\rScsiServerIsm::DoWork read Data - received",
    					(U8 *)pData, 16);
}
#endif				
	// translate BSA status to SCSI status
	pRf->DetailedStatusCode = pMsg->DetailedStatusCode;
	
	// translate BSA reply to a SCSI reply
	// first, clear the payload 
	memset(&payload, 0, sizeof(SCB_REPLY_PAYLOAD));
	
	// copy the transfer count
	payload.TransferCount = pBsaPayload->TransferCount;
	
	// For PHS Reporter stuffs.
	m_Performance.NumBSAReads++;
	m_Performance.NumBSABytesRead += payload.TransferCount;
				
	if ((pRf->DetailedStatusCode & FCP_SCSI_DEVICE_DSC_MASK) == FCP_SCSI_DSC_CHECK_CONDITION)
	{
		// copy the sense data to the SCB message (via the payload)
		payload.AutoSenseTransferCount = pBsaPayload->AutoSenseTransferCount;
		memcpy(&payload.SenseData,
					&pBsaPayload->SenseData,
					payload.AutoSenseTransferCount);
	}
	
	// add the payload to the reply message
	((Message *)p_context->pMsg)->
					AddReplyPayload(&payload, sizeof(SCB_REPLY_PAYLOAD));

	// answer original message
	Reply(p_context->pMsg);
			
	// de-allocate resources
	// delete the context
	delete p_context;
	
	// finally delete the reply message
	delete pMsg;

	return(0);
	
} // ScsiServerReadCallback



/*************************************************************************/
// ScsiServerWriteCallback
//  Wait for a reply Callback.  Then translate the BSA status
//	to a SCSI SCB EXECUTE reply status and answer the original message.
/*************************************************************************/
STATUS ScsiServerIsm::ScsiServerWriteCallback(Message *pMsg)
{
	SCSI_CONTEXT 		*p_context = (SCSI_CONTEXT *)pMsg->GetContext();
	SCB_REPLY_PAYLOAD	 payload;
	Message				*pRf = (Message *) p_context->pMsg;
	BSA_REPLY_PAYLOAD 	*pBsaPayload = (BSA_REPLY_PAYLOAD *) pMsg->GetPPayload();;

	TRACE_ENTRY(ScsiServerWriteCallback);

	// translate BSA status to SCSI status
	pRf->DetailedStatusCode = pMsg->DetailedStatusCode;
	
	// translate BSA reply to an I2O SCSI reply
	// first, clear the payload 
	memset(&payload, 0, sizeof(SCB_REPLY_PAYLOAD));
	
	// copy the transfer count
	payload.TransferCount = pBsaPayload->TransferCount;
	
	// For PHS Reporter stuffs.  
	m_Performance.NumBSAWrites++;
	m_Performance.NumBSABytesWritten += payload.TransferCount;
				
	if ((pRf->DetailedStatusCode & FCP_SCSI_DEVICE_DSC_MASK) == FCP_SCSI_DSC_CHECK_CONDITION)
	{
		// copy the sense data to the SCB message (via the payload)
		payload.AutoSenseTransferCount = pBsaPayload->AutoSenseTransferCount;
		memcpy(&payload.SenseData,
					&pBsaPayload->SenseData,
					payload.AutoSenseTransferCount);
	}
	
	// add the payload to the reply message
	((Message *)p_context->pMsg)->
					AddReplyPayload(&payload, sizeof(SCB_REPLY_PAYLOAD));

	// answer original message
	Reply(p_context->pMsg);
			
	// de-allocate resources
	// delete the context
	delete p_context;

	// finally delete the reply message
	delete pMsg;

	return(0);
	
} // ScsiServerWriteCallback

