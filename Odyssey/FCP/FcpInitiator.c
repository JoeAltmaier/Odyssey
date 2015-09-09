/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpInitiator.c
// 
// Description:
// In initiator mode, the FCP driver receives I2O requests and
// sends SCSI requests to the ISP.
// 
// Update Log 
// 4/14/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 6/3/98 Jim Frandeen: Change to single task model
// 10/2/98 Michael G. Panas: add support for IDLUN struct
// 11/16/98 Michael G. Panas: fix problem with response code
// 11/20/98 Michael G. Panas: add DEBUG info
// 11/30/98 Michael G. Panas: New memory allocation methods
// 02/17/99 Michael G. Panas: convert to new Message format and remove I2O
/*************************************************************************/

#include "FcpCommon.h"
#include "FcpEvent.h"
#include "FcpInitiator.h"
#include "FcpISP.h"
#include "CTIdLun.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"
#include "FcpMessage.h"
#include "FcpRequestFIFO.h"
#include "FcpString.h"
#include "Scsi.h"
#include "FcpI2O.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
STATUS	FCP_Handle_Command_Response(FCP_EVENT_CONTEXT *p_context);

STATUS	FCP_Handle_SCSI_Request(FCP_EVENT_CONTEXT *p_context);
			
STATUS	FCP_Handle_SCSI_Request_No_Data(FCP_EVENT_CONTEXT *p_context);
				
STATUS	FCP_Handle_SCSI_Request_Read(FCP_EVENT_CONTEXT *p_context);
	
STATUS	FCP_Handle_SCSI_Request_Write(FCP_EVENT_CONTEXT *p_context);
	
STATUS	FCP_Initialize_IOCB_Command(FCP_EVENT_CONTEXT *p_context,
			IOCB_COMMAND_TYPE2 **pp_command, U32 Flags);

STATUS	FCP_Initialize_IOCB4_Command(FCP_EVENT_CONTEXT *p_context,
			IOCB_COMMAND_TYPE4 **pp_command, U32 Flags);

/*************************************************************************/
// FCP_Initiator_Create
// Create FCP_Initiator object
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS FCP_Initiator_Create(PINSTANCE_DATA Id)
{
	FCP_TRACE_ENTRY(FCP_Initiator_Create);
	
	return NU_SUCCESS;
	
} // FCP_Initiator_Create
	
/*************************************************************************/
// FCP_Initiator_Destroy
// Destroy FCP_Initiator object
/*************************************************************************/
void FCP_Initiator_Destroy()
{
	FCP_TRACE_ENTRY(FCP_Initiator_Destroy);
	
} // FCP_Initiator_Destroy

/*************************************************************************/
// FCP_Handle_Command_Response
// Come here from FCP_Event_Task when the command IOCB has completed.
// Our context contains the status IOCB from the ISP.
// Build an I2O reply to send back to the requestor.  The flags and status
// in the STATUS IOCB must be translated to an I2O status in the reply.
/*************************************************************************/
STATUS FCP_Handle_Command_Response(FCP_EVENT_CONTEXT *p_context)
{	
	STATUS				 			 	 DetailedStatusCode, status = NU_SUCCESS;
	U16									 RespSts, StateFlags;
	U16									 Length, Sts;
	U32									 XferLen, Resid;
	PINSTANCE_DATA						 Id;
	SCB_REPLY_PAYLOAD					 payload;
	SCB_PAYLOAD							*pP;
	
	FCP_TRACE_ENTRY(FCP_Handle_Command_Response);
	
    FCP_DUMP_HEX(TRACE_L8, "\n\rStatus IOCB", (U8 *)&p_context->status_iocb,
    							sizeof(IOCB_STATUS_TYPE0));

#if defined(FCP_DEBUG)    
    p_context->flags |= EV_CMD_RESP_IN;
#endif
    
    Id = p_context->Id;					 // get our instance data
    
    // Get the original transfer length from the message before
    // we turn it into a reply
    pP = (SCB_PAYLOAD *)FCP_I2O_Get_Payload(p_context->message);
	XferLen = pP->ByteCount;
	
    // Get the status from our last response
    RespSts = BYTE_SWAP16(p_context->status_iocb.SCSI_status);
    StateFlags = BYTE_SWAP16(p_context->status_iocb.state_flags);
    //StsFlags = BYTE_SWAP16(p_context->status_iocb.status_flags);  //not used
    
    // The 16-bit Detailed Status Code field for SCSI operations is 
    // divided into two separate 8-bit fields.  The lower 8 bits are 
    // used to report Device Status information.  The upper 8 bits are
    // used to report Adapter Status information.
    
    // Get the Device Status part, the Adapter status is set later as needed
	DetailedStatusCode = (RespSts & IOCB_SCSI_FLAGS_SCSI_STATUS_MASK);
 
	// Was there a check condition?
    if (((RespSts & IOCB_SCSI_FLAGS_SCSI_STATUS_MASK) == FCP_SCSI_DSC_CHECK_CONDITION) &&
    			(RespSts & IOCB_SCSI_FLAGS_SENSE_VALID )) //  and sense data valid?
    {
    	// We must copy the Sense data to the I2O Message
    	Length = BYTE_SWAP16(p_context->status_iocb.sense_data_length);
    	
    	bcopy(
    		(char *) &p_context->status_iocb.sense_data[0],	// src
    		(char *) &payload.SenseData[0],					// dest
    		(U32)Length);
    	payload.AutoSenseTransferCount = (U32) Length;
	}
	else
	{
    	payload.AutoSenseTransferCount = 0;	// no sense data
	}
	
	Resid = BYTE_SWAP32(p_context->status_iocb.residual_transfer_length);
	
	// DEBUG
	FCP_PRINT_HEX(TRACE_L8, "\n\rOrig length = ", XferLen);
	FCP_PRINT_HEX(TRACE_L8, "\n\rResid length = ", Resid);
	
	// Was any data transfered?
	if (StateFlags & IOCB_STATE_FLAGS_XFER_COMPLETE)
	{
		// data was transfered OK, get the count from the original request
		// pass this back in the TransferCount field
		payload.TransferCount = XferLen;
	}
	else if ((StateFlags & IOCB_STATE_FLAGS_XFERED_DATA) &&
				((RespSts & (IOCB_SCSI_FLAGS_RESIDUAL_UNDER | IOCB_SCSI_FLAGS_RESIDUAL_OVER))
				|| (BYTE_SWAP16(p_context->status_iocb.status) == 0x15)) )
	{
		// Some data transfered, but not all
		payload.TransferCount = XferLen - Resid;
				
    	// DEBUG - dump the DMA Chan regs
		FCP_PRINT_STRING(TRACE_L8, "\n\rRead DMA Regs:");
		{
			U32 x;
			
    		Write_ISP(Id, ISP_HCCR, HCTLPAUSERISC);	// Pause RISC
    		Write_ISP(Id, 0x20, 0x24);	// DMA ctl Reset FIFO
    		Write_ISP(Id, 0x20, 0x40);	// DMA ctl Set Direction
			
			for (x = 0x60; x <= 0x00000080; x += 2)
				(void)Read_ISP(Id, x);
				
    		Write_ISP(Id, ISP_HCCR, HCTLRLSRISC);	// Release RISC
		}
	}
	else
	{
		// Nothing transfered
		payload.TransferCount = 0;
	}
	
	// Check the response for any adapter problems
	switch (BYTE_SWAP16(p_context->status_iocb.status))
	{
		case	0x15:		// Data underrun - OK
		case	0x00:		// Complete
			Sts = FCP_SCSI_HBA_DSC_SUCCESS;
			break;
		case	0x04:		// Reset
			Sts = FCP_SCSI_HBA_DSC_SCSI_BUS_RESET;
			break;
		case	0x05:		// Aborted
			Sts = FCP_SCSI_HBA_DSC_REQUEST_ABORTED;
			break;
		case	0x06:		// Timeout
			Sts = FCP_SCSI_HBA_DSC_COMMAND_TIMEOUT;
			break;
		case	0x07:		// Data Overrun
			Sts = FCP_SCSI_HBA_DSC_DATA_OVERRUN;
			break;
		case	0x1C:		// Queue Full
			Sts = FCP_SCSI_HBA_DSC_ADAPTER_BUSY;
			break;
		case	0x01:		// Transport Incomplete
		case	0x28:		// Port Unavailable
		case	0x29:		// Port Logged out
			Sts = FCP_SCSI_HBA_DSC_SELECTION_TIMEOUT;
			break;
		default:
			Sts = FCP_SCSI_HBA_DSC_REQUEST_TERMINATED;
			break;
	}
    
	DetailedStatusCode |= Sts;
	
	// add the reply payload and the status to the message
	FCP_I2O_Add_Reply_Payload(p_context->message,
			&payload, sizeof(SCB_REPLY_PAYLOAD),
			DetailedStatusCode);
	    
    FCP_DUMP_HEX(TRACE_L8, "\n\rMessage Response", (U8 *)p_context->message,
    					128);
    					
	FCP_Delete_SGL(p_context);

    // Send the I2O response back.
    FCP_Message_Send_Response(p_context);
	    
#if defined(FCP_DEBUG)    
    p_context->flags |= EV_REPLY_SENT;
#endif

	return status;
	
} // FCP_Handle_Command_Response

/*************************************************************************/
// FCP_Handle_SCSI_Request
// Come here from FCP RAC DDM message handler when we receive a new I2O message.
// Handle the request and when done, return a pointer to the response message to 
// send back.
/*************************************************************************/
STATUS FCP_Handle_SCSI_Request(FCP_EVENT_CONTEXT *p_context)
{	
	STATUS				 	 status = NU_SUCCESS;
	SCB_PAYLOAD				*pP = 
						(SCB_PAYLOAD *)FCP_I2O_Get_Payload(p_context->message);

	FCP_TRACE_ENTRY(FCP_Handle_SCSI_Request);
	
#if defined(FCP_DEBUG)    
    p_context->flags |= EV_MESSAGE_IN;
#endif
    
	p_context->p_dsd = NULL;

	FCP_PRINT_HEX16(TRACE_L3, "\n\rInit Id = ", (U16)pP->IdLun.id);

	// Switch on the operation_code in the CDB and call the appropriate
	// handler.  In each case, we pass in a pointer to the request and
	// get back a pointer to an IOCB_STATUS_TYPE0.
	switch (((CDB16*)(&pP->CDB))->Cmd)
	{
		// The following commands transfer block data from the device
		// back to the host.
		case CMD_READ6:
		case CMD_READ10:
				
		// The following commands transfer data from the device
		// back to the host.
		case CMD_REQUEST_SENSE:
		case CMD_INQUIRY:
		case CMD_READ_CAPACITY:
		case CMD_MODE_SENSE6:
		case CMD_MODE_SENSE10:
		case CMD_RECEIVE_DIAG:
		case CMD_REPORT_LUNS:
			status = FCP_Handle_SCSI_Request_Read(p_context);
				
			break;
		
		// The following commands transfer block data from the host
		// to the device.
		case CMD_WRITE6:
		case CMD_WRITE10:
				
		// The following commands transfer data from the host
		// to the device.
		case CMD_MODE_SELECT6:
		case CMD_MODE_SELECT10:
		case CMD_SEND_DIAG:
		case CMD_RESERVE6:
		case CMD_RESERVE10:
			status = FCP_Handle_SCSI_Request_Write(p_context);
				
			break;
			
		default:
		
			// Other requests do not have any data to transfer
			status = FCP_Handle_SCSI_Request_No_Data(p_context);
	}
	
    if (status != NU_SUCCESS)
    {
    
    	// Status was not success.
    	// We were not able to handle sending the SCSI command to the ISP.
		// add an error reply payload and the status to the message
		
		FCP_I2O_Add_Reply_Payload(p_context->message,
				NULL, 0,			// no payload
				FCP_SCSI_DSC_COMMAND_TERMINATED);
		    
     	// Send the error response back.
    	FCP_Message_Send_Response(p_context);
   
	}    
	return status;
	
} // FCP_Handle_SCSI_Request

/*************************************************************************/
// FCP_Handle_SCSI_Request_No_Data
// Handle any command that does not transfer data.
/*************************************************************************/
STATUS FCP_Handle_SCSI_Request_No_Data(FCP_EVENT_CONTEXT *p_context)
{
	IOCB_COMMAND_TYPE4			*p_command;
	STATUS						 status;
	
	FCP_TRACE_ENTRY(FCP_Handle_SCSI_Request_No_Data);
	
	// Create an IOCB_COMMAND_TYPE2 in the ISP request FIFO.
	status = FCP_Initialize_IOCB4_Command(p_context, &p_command, 0);
	if (status != NU_SUCCESS)
		return status;
	
    // No data to send.
    p_command->data_segment_count = 0;

    // Send Command IOCB request to the ISP.
    // next_action specifies action to perform by FCP_Event_Task
    // when command completes.
    status = FCP_Send_Command_IOCB(p_context, 
    	INITIATOR_ACTION_HANDLE_COMMAND_RESPONSE, // next_action,
    	(IOCB_COMMAND_TYPE2*)p_command); 
       	
#if defined(FCP_DEBUG)    
    p_context->flags |= EV_CMD_SENT;
#endif
    
	return status;
	
} // FCP_Handle_SCSI_Request_No_Data

/*************************************************************************/
// FCP_Handle_SCSI_Request_Read
// Handle any command that transfers data from the device.
/*************************************************************************/
STATUS FCP_Handle_SCSI_Request_Read(FCP_EVENT_CONTEXT *p_context)
{
	IOCB_COMMAND_TYPE4			*p_command;
	STATUS						 status;
	
	FCP_TRACE_ENTRY(FCP_Handle_SCSI_Request_Read);
	
	// Create an IOCB_COMMAND_TYPE4 in the ISP request FIFO.
	status = FCP_Initialize_IOCB4_Command(p_context, &p_command,
				ISP_REQ_FLAG_DATA_READ);
	if (status != NU_SUCCESS)
		return status;
	
    // Create a DATA_SEGMENT_DESCRIPTOR from the SGL in the
    // I2O request so that we can use it in the CTIO. 
    FCP_I2O_Get_SGL_Entries(p_context, (IOCB_COMMAND_TYPE4*)p_command);

    // Send Command IOCB read request to the ISP.
    // next_action specifies action to perform by FCP_Event_Task
    // when command completes.
    status = FCP_Send_Command_IOCB(p_context, 
    	INITIATOR_ACTION_HANDLE_COMMAND_RESPONSE, // next_action,
    	(IOCB_COMMAND_TYPE2*)p_command); 
       
#if defined(FCP_DEBUG)    
    p_context->flags |= EV_CMD_SENT;
#endif
    
	return status;
	
} // FCP_Handle_SCSI_Request_Read

/*************************************************************************/
// FCP_Handle_SCSI_Request_Write
// Handle any command that transfers data to the device.
// Return pointer to status_IOCB or zero if command complete without error.   	
/*************************************************************************/
STATUS FCP_Handle_SCSI_Request_Write(FCP_EVENT_CONTEXT *p_context)
{
	IOCB_COMMAND_TYPE4			*p_command;
	STATUS						 status;
    
	FCP_TRACE_ENTRY(FCP_Handle_SCSI_Request_Write);
	
	// Create an IOCB_COMMAND_TYPE4 in the ISP request FIFO.
	status = FCP_Initialize_IOCB4_Command(p_context, &p_command,
					ISP_REQ_FLAG_DATA_WRITE);
	if (status != NU_SUCCESS)
		return status;
	
    // Create a DATA_SEGMENT_DESCRIPTOR from the SGL in the
    // I2O request so that we can use it in the CTIO. 
    FCP_I2O_Get_SGL_Entries(p_context, (IOCB_COMMAND_TYPE4*)p_command);

    // Send Command IOCB write request to the ISP.
    // next_action specifies action to perform by FCP_Event_Task
    // when command completes.
    status = FCP_Send_Command_IOCB(p_context, 
    	INITIATOR_ACTION_HANDLE_COMMAND_RESPONSE, // next_action,
    	(IOCB_COMMAND_TYPE2*)p_command); 
       	
#if defined(FCP_DEBUG)    
    p_context->flags |= EV_CMD_SENT;
#endif
    
	return status;
	
} // FCP_Handle_SCSI_Request_Write

/*************************************************************************/
// FCP_Initialize_IOCB_Command
// Create a IOCB_COMMAND_TYPE2 from a CDB.
/*************************************************************************/
STATUS FCP_Initialize_IOCB_Command(
	FCP_EVENT_CONTEXT *p_context, 
	IOCB_COMMAND_TYPE2 **pp_command,
	U32		Flags)
{
	IOCB_COMMAND_TYPE2	*p_command;
	STATUS				 status = NU_SUCCESS;
	SCB_PAYLOAD			*pP = 
						(SCB_PAYLOAD *)FCP_I2O_Get_Payload(p_context->message);
	IDLUN				*p_idlun = (IDLUN *)&pP->IdLun;

	
	FCP_TRACE_ENTRY(FCP_Initialize_IOCB_Command);
	
    // Get pointer to next IOCB in ISP request FIFO.
	p_command = (IOCB_COMMAND_TYPE2*)FCP_Request_FIFO_Get_Pointer(p_context->Id);
	
	// Set up Command fields
    p_command->entry_type = IOCB_TYPE_COMMAND_TYPE_2;
    p_command->entry_count = 1;
    
    // We can't always send to Logical Unit Number 0.  For Daisy Chains need a LUN
    // Get the LUN number passed to us.
    p_command->LUN = BYTE_SWAP16(p_idlun->LUN);

    // Get the SCSI target passed to us
    p_command->target = p_idlun->id;
    	
    // Set the timeout value in seconds.
    p_command->timeout = BYTE_SWAP16(IOCB_TIMEOUT_SECONDS);
        
    // Set direction of data transfer.
    p_command->control_flags = BYTE_SWAP16((U16)Flags);
    
	// get the task queue options from the I2O message and set the IOCB
	// control flags
	switch (pP->SCBFlags & FCP_SCB_FLAG_TAG_TYPE_MASK) {
	
		case	FCP_SCB_FLAG_SIMPLE_QUEUE_TAG:	// Simple Queue
			p_command->control_flags |= BYTE_SWAP16(ISP_REQ_FLAG_SIMPLE_TAG);
			break;
		case	FCP_SCB_FLAG_HEAD_QUEUE_TAG:	// Head of Queue
			p_command->control_flags |= BYTE_SWAP16(ISP_REQ_FLAG_HEAD_TAG);
			break;
		case	FCP_SCB_FLAG_ORDERED_QUEUE_TAG:	// Ordered Queue
			p_command->control_flags |= BYTE_SWAP16(ISP_REQ_FLAG_ORDERED_TAG);
			break;
		case	FCP_SCB_FLAG_ACA_QUEUE_TAG:		// ACA Queue (no equivalent)
			break;
		case	FCP_SCB_FLAG_NO_TAG_QUEUEING:	// UnTagged
		default:
			// do nothing
			break;
	}
	    
	// Move CDB into IOCB
	bcopy(
		(char *)&pP->CDB,			// src
		(char *)&p_command->CDB,	// dest
		sizeof(CDB16));
	    
    // Get the number of bytes to be transferred
    p_command->data_byte_count = BYTE_SWAP32(pP->ByteCount);
    
    *pp_command = p_command; 		// return the new IOCB
    	
    return status;

} // FCP_Initialize_IOCB_Command

/*************************************************************************/
// FCP_Initialize_IOCB4_Command
// Create a IOCB_COMMAND_TYPE4 from a CDB.
/*************************************************************************/
STATUS FCP_Initialize_IOCB4_Command(
	FCP_EVENT_CONTEXT *p_context, 
	IOCB_COMMAND_TYPE4 **pp_command,
	U32		Flags)
{
	IOCB_COMMAND_TYPE4	*p_command;
	STATUS				 status = NU_SUCCESS;
	SCB_PAYLOAD			*pP = 
						(SCB_PAYLOAD *)FCP_I2O_Get_Payload(p_context->message);
	IDLUN				*p_idlun = (IDLUN *)&pP->IdLun;

	
	FCP_TRACE_ENTRY(FCP_Initialize_IOCB4_Command);
	
    // Get pointer to next IOCB in ISP request FIFO.
	p_command = (IOCB_COMMAND_TYPE4*)FCP_Request_FIFO_Get_Pointer(p_context->Id);
	
	// Set up Command fields
    p_command->entry_type = IOCB_TYPE_COMMAND_TYPE_4;
    p_command->entry_count = 1;
    
    // We can't always send to Logical Unit Number 0.  For Daisy Chains need a LUN
    // Get the LUN number passed to us.
    p_command->LUN = BYTE_SWAP16(p_idlun->LUN);

    // Get the SCSI target passed to us
    p_command->target = p_idlun->id;
    	
    // Set the timeout value in seconds.
    p_command->timeout = BYTE_SWAP16(IOCB_TIMEOUT_SECONDS);
        
    // Set direction of data transfer.
    p_command->control_flags = BYTE_SWAP16((U16)Flags);
    
	// get the task queue options from the I2O message and set the IOCB
	// control flags
	switch (pP->SCBFlags & FCP_SCB_FLAG_TAG_TYPE_MASK) {
	
		case	FCP_SCB_FLAG_SIMPLE_QUEUE_TAG:	// Simple Queue
			p_command->control_flags |= BYTE_SWAP16(ISP_REQ_FLAG_SIMPLE_TAG);
			break;
		case	FCP_SCB_FLAG_HEAD_QUEUE_TAG:	// Head of Queue
			p_command->control_flags |= BYTE_SWAP16(ISP_REQ_FLAG_HEAD_TAG);
			break;
		case	FCP_SCB_FLAG_ORDERED_QUEUE_TAG:	// Ordered Queue
			p_command->control_flags |= BYTE_SWAP16(ISP_REQ_FLAG_ORDERED_TAG);
			break;
		case	FCP_SCB_FLAG_ACA_QUEUE_TAG:		// ACA Queue (no equivalent)
			break;
		case	FCP_SCB_FLAG_NO_TAG_QUEUEING:	// UnTagged
		default:
			// do nothing
			break;
	}
	    
	// Move CDB into IOCB
	bcopy(
		(char *)&pP->CDB,			// src
		(char *)&p_command->CDB,	// dest
		sizeof(CDB16));
	    
    // Get the number of bytes to be transferred
    p_command->data_byte_count = BYTE_SWAP32(pP->ByteCount);
    
    *pp_command = p_command; 		// return the new IOCB
    	
    return status;

} // FCP_Initialize_IOCB4_Command
