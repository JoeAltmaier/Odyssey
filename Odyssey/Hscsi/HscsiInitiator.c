/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiInitiator.c
// 
// Description:
// In initiator mode, the HSCSI driver receives I2O requests and
// sends SCSI requests to the ISP1040.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiInitiator.c $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#include "HscsiCommon.h"
#include "HscsiEvent.h"
#include "HscsiInitiator.h"
#include "HscsiISP.h"
#include "CTIdLun.h"
#include "HscsiMessageFormats.h"
#include "HscsiMessageStatus.h"
#include "HscsiMessage.h"
#include "HscsiRequestFIFO.h"
#include "HscsiString.h"
#include "Scsi.h"
#include "HscsiI2O.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
STATUS	HSCSI_Handle_Command_Response(HSCSI_EVENT_CONTEXT *p_context);

STATUS	HSCSI_Handle_SCSI_Request(HSCSI_EVENT_CONTEXT *p_context);
			
STATUS	HSCSI_Handle_SCSI_Request_No_Data(HSCSI_EVENT_CONTEXT *p_context);
				
STATUS	HSCSI_Handle_SCSI_Request_Read(HSCSI_EVENT_CONTEXT *p_context);
	
STATUS	HSCSI_Handle_SCSI_Request_Write(HSCSI_EVENT_CONTEXT *p_context);
	
STATUS	HSCSI_Initialize_IOCB_Command(HSCSI_EVENT_CONTEXT *p_context,
			HSCSI_IOCB_COMMAND **pp_command, U32 Flags);

/*************************************************************************/
// HSCSI_Initiator_Create
// Create HSCSI_Initiator object
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS HSCSI_Initiator_Create(PHSCSI_INSTANCE_DATA Id)
{
	HSCSI_TRACE_ENTRY(HSCSI_Initiator_Create);
	
	return NU_SUCCESS;
	
} // HSCSI_Initiator_Create
	
/*************************************************************************/
// HSCSI_Initiator_Destroy
// Destroy HSCSI_Initiator object
/*************************************************************************/
void HSCSI_Initiator_Destroy()
{
	HSCSI_TRACE_ENTRY(HSCSI_Initiator_Destroy);
	
} // HSCSI_Initiator_Destroy

/*************************************************************************/
// HSCSI_Handle_Command_Response
// Come here from HSCSI_Event_Task when the command IOCB has completed.
// Our context contains the status IOCB from the ISP.
// Build an I2O reply to send back to the requestor.  The flags and status
// in the STATUS IOCB must be translated to an I2O status in the reply.
/*************************************************************************/
STATUS HSCSI_Handle_Command_Response(HSCSI_EVENT_CONTEXT *p_context)
{	
	STATUS				 			 	 DetailedStatusCode, status = NU_SUCCESS;
	U16									 RespSts, StateFlags;
	U16									 Length, StsFlags, Sts;
	U32									 XferLen, Resid;
	PHSCSI_INSTANCE_DATA						 Id;
	SCB_REPLY_PAYLOAD					 payload;
	SCB_PAYLOAD							*pP;
	
	HSCSI_TRACE_ENTRY(HSCSI_Handle_Command_Response);
	
    HSCSI_DUMP_HEX(TRACE_L8, "\n\rStatus IOCB", (U8 *)&p_context->status_iocb,
    							sizeof(IOCB_STATUS_ENTRY));

#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_CMD_RESP_IN;
#endif
    
    Id = p_context->Id;					 // get our instance data
    
    // Get the original transfer length from the message before
    // we turn it into a reply
    pP = (SCB_PAYLOAD *)HSCSI_I2O_Get_Payload(p_context->message);
	XferLen = pP->ByteCount;
	
    // Get the status from our last response
    RespSts = BYTE_SWAP16(p_context->status_iocb.SCSI_status);
    StateFlags = BYTE_SWAP16(p_context->status_iocb.state_flags);
    StsFlags = BYTE_SWAP16(p_context->status_iocb.status_flags);
    
    // The 16-bit Detailed Status Code field for SCSI operations is 
    // divided into two separate 8-bit fields.  The lower 8 bits are 
    // used to report Device Status information.  The upper 8 bits are
    // used to report Adapter Status information.
    
    // Get the Device Status part, the Adapter status is set later as needed
	DetailedStatusCode = (RespSts & IOCB_SCSI_FLAGS_SCSI_STATUS_MASK);
 
	// Was there a check condition?
    if (((RespSts & IOCB_SCSI_FLAGS_SCSI_STATUS_MASK) == HSCSI_SCSI_DSC_CHECK_CONDITION) &&
    			(RespSts & IOCB_SCSI_FLAGS_SENSE_VALID )) //  and sense data valid?
    {
    	// We must copy the Sense data to the I2O Message
    	Length = BYTE_SWAP16(p_context->status_iocb.sense_data_length);
    	
    	Mem_Copy( (U8 *) &payload.SenseData[0],
    				(U8 *) &p_context->status_iocb.sense_data[0],
    				(U32)Length);
    	payload.AutoSenseTransferCount = (U32) Length;
	}
	else
	{
    	payload.AutoSenseTransferCount = 0;	// no sense data
	}
	
	Resid = BYTE_SWAP32(p_context->status_iocb.residual_transfer_length);
	
	// DEBUG
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rOrig length = ", XferLen);
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rResid length = ", Resid);
	
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
    	// DMA ctl registers are significantly different from ISP2xxx!
		HSCSI_PRINT_STRING(TRACE_L8, "\n\rRead DMA Regs:");
		{
			U32 x;
			
    		Write_ISP1040(Id, HSCSI_HCCR, HCCRPAUSERISC);	// Pause RISC
    		Write_ISP1040(Id, 0x22, 0x000C);	// DMA ctl Reset FIFO
    		Write_ISP1040(Id, 0x20, 0x0001);	// DMA ctl Set Direction
			
			for (x = 0x60; x <= 0x00000080; x += 2)
				(void)Read_ISP1040(Id, x);
				
    		Write_ISP1040(Id, HSCSI_HCCR, HCCRRLSRISC);	// Release RISC
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
			Sts = HSCSI_SCSI_HBA_DSC_SUCCESS;
			break;
		case	0x04:		// Reset
			Sts = HSCSI_SCSI_HBA_DSC_SCSI_BUS_RESET;
			break;
		case	0x05:		// Aborted
			Sts = HSCSI_SCSI_HBA_DSC_REQUEST_ABORTED;
			break;
		case	0x06:		// Timeout
			Sts = HSCSI_SCSI_HBA_DSC_COMMAND_TIMEOUT;
			break;
		case	0x07:		// Data Overrun
			Sts = HSCSI_SCSI_HBA_DSC_DATA_OVERRUN;
			break;
		case	0x1C:		// Queue Full
			Sts = HSCSI_SCSI_HBA_DSC_ADAPTER_BUSY;
			break;
		case	0x01:		// Transport Incomplete
		case	0x28:		// Port Unavailable
		case	0x29:		// Port Logged out
			Sts = HSCSI_SCSI_HBA_DSC_SELECTION_TIMEOUT;
			break;
		default:
			Sts = HSCSI_SCSI_HBA_DSC_REQUEST_TERMINATED;
			break;
	}
    
	DetailedStatusCode |= Sts;
	
	// add the reply payload and the status to the message
	HSCSI_I2O_Add_Reply_Payload(p_context->message,
			&payload, sizeof(SCB_REPLY_PAYLOAD),
			DetailedStatusCode);
	    
    HSCSI_DUMP_HEX(TRACE_L8, "\n\rMessage Response", (U8 *)p_context->message,
    					128);
    					
    // Send the I2O response back.
    HSCSI_Message_Send_Response(p_context);
	    
#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_REPLY_SENT;
#endif

	return status;
	
} // HSCSI_Handle_Command_Response

/*************************************************************************/
// HSCSI_Handle_SCSI_Request
// Come here from HSCSI DDM message handler when we receive a new I2O message.
// Handle the request and when done, return a pointer to the response message to 
// send back.
/*************************************************************************/
STATUS HSCSI_Handle_SCSI_Request(HSCSI_EVENT_CONTEXT *p_context)
{	
	STATUS				 	 status = NU_SUCCESS;
	SCB_PAYLOAD				*pP = 
						(SCB_PAYLOAD *)HSCSI_I2O_Get_Payload(p_context->message);

	HSCSI_TRACE_ENTRY(HSCSI_Handle_SCSI_Request);
	
#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_MESSAGE_IN;
#endif
    
	HSCSI_PRINT_HEX16(TRACE_L3, "\n\rInit Id = ", (U16)pP->IdLun.id);

	// Switch on the operation_code in the CDB and call the appropriate
	// handler.  In each case, we pass in a pointer to the request and
	// get back a pointer to an IOCB_STATUS_ENTRY.
	
	p_context->CDB_length = pP->CDBLength; // Pass the CDB size to the context
	
	switch (((CDB12*)(&pP->CDB))->Cmd)
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
			status = HSCSI_Handle_SCSI_Request_Read(p_context);
				
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
			status = HSCSI_Handle_SCSI_Request_Write(p_context);
				
			break;
		case CMD_START_UNIT:
			status = HSCSI_Handle_SCSI_Request_No_Data(p_context);
			break;
			
		default:
		
			// Other requests do not have any data to transfer
			status = HSCSI_Handle_SCSI_Request_No_Data(p_context);
	}
	
    if (status != NU_SUCCESS)
    {
    
    	// Status was not success.
    	// We were not able to handle sending the SCSI command to the ISP.
		// add an error reply payload and the status to the message
		
		HSCSI_I2O_Add_Reply_Payload(p_context->message,
				NULL, 0,			// no payload
				HSCSI_SCSI_DSC_COMMAND_TERMINATED);
		    
     	// Send the error response back.
    	HSCSI_Message_Send_Response(p_context);
   
	}    
	return status;
	
} // HSCSI_Handle_SCSI_Request

/*************************************************************************/
// HSCSI_Handle_SCSI_Request_No_Data
// Handle any command that does not transfer data.
/*************************************************************************/
STATUS HSCSI_Handle_SCSI_Request_No_Data(HSCSI_EVENT_CONTEXT *p_context)
{
	HSCSI_IOCB_COMMAND			*p_command;
	STATUS						 status;
	
	HSCSI_TRACE_ENTRY(HSCSI_Handle_SCSI_Request_No_Data);
	
	// Create an IOCB_COMMAND in the ISP request FIFO.
	status = HSCSI_Initialize_IOCB_Command(p_context, &p_command, 0);
	if (status != NU_SUCCESS)
		return status;
	
    // No data to send.
    p_command->data_segment_count = 0;

    // Send Command IOCB request to the ISP.
    // next_action specifies action to perform by HSCSI_Event_Task
    // when command completes.
    status = HSCSI_Send_Command_IOCB(p_context, // HSCSI_Send_Command_IOCB
    	INITIATOR_HANDLE_COMMAND_RESPONSE, // next_action,
    	(HSCSI_IOCB_COMMAND*)p_command); 
       	
#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_CMD_SENT;
#endif
    
	return status;
	
} // HSCSI_Handle_SCSI_Request_No_Data

/*************************************************************************/
// HSCSI_Handle_SCSI_Request_Read
// Handle any command that transfers data from the device.
/*************************************************************************/
STATUS HSCSI_Handle_SCSI_Request_Read(HSCSI_EVENT_CONTEXT *p_context)
{
	HSCSI_IOCB_COMMAND			*p_command;
	STATUS						 status;
	
	HSCSI_TRACE_ENTRY(HSCSI_Handle_SCSI_Request_Read);
	
	// Create an IOCB_COMMAND in the ISP request FIFO.
	status = HSCSI_Initialize_IOCB_Command(p_context, &p_command,
				HSCSI_REQ_FLAG_DATA_READ);
	if (status != NU_SUCCESS)
		return status;
	
    // Create a DATA_SEGMENT_DESCRIPTOR from the SGL in the
    // I2O request so that we can use it in the CTIO. 
    HSCSI_I2O_Get_SGL_Data_Segment(p_context->message, 
    	&p_command->data_segment_0, 
    	0 /* index of SGL segment */ );
    
    // For now, we always send one data segment.
    p_command->data_segment_count = BYTE_SWAP16(1);

    // Send Command IOCB read request to the ISP.
    // next_action specifies action to perform by HSCSI_Event_Task
    // when command completes.
    status = HSCSI_Send_Command_IOCB(p_context, // HSCSI_Send_Command_IOCB
    	INITIATOR_HANDLE_COMMAND_RESPONSE, // next_action,
    	(HSCSI_IOCB_COMMAND*)p_command); 
       
#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_CMD_SENT;
#endif
    
	return status;
	
} // HSCSI_Handle_SCSI_Request_Read

/*************************************************************************/
// HSCSI_Handle_SCSI_Request_Write
// Handle any command that transfers data to the device.
// Return pointer to status_IOCB or zero if command complete without error.   	
/*************************************************************************/
STATUS HSCSI_Handle_SCSI_Request_Write(HSCSI_EVENT_CONTEXT *p_context)
{
	HSCSI_IOCB_COMMAND			*p_command;
	STATUS						 status;
    
	HSCSI_TRACE_ENTRY(HSCSI_Handle_SCSI_Request_Write);
	
	// Create an IOCB_COMMAND in the ISP request FIFO.
	status = HSCSI_Initialize_IOCB_Command(p_context, &p_command,
					HSCSI_REQ_FLAG_DATA_WRITE);
	if (status != NU_SUCCESS)
		return status;
	
    // Create a DATA_SEGMENT_DESCRIPTOR from the SGL in the
    // I2O request so that we can use it in the CTIO. 
    HSCSI_I2O_Get_SGL_Data_Segment(p_context->message, 
    	&p_command->data_segment_0, 0);
    
    // For now, we always send one data segment.
    p_command->data_segment_count = BYTE_SWAP16(1);

    // Send Command IOCB write request to the ISP.
    // next_action specifies action to perform by HSCSI_Event_Task
    // when command completes.
    status = HSCSI_Send_Command_IOCB(p_context, 
    	INITIATOR_HANDLE_COMMAND_RESPONSE, // next_action,
    	(HSCSI_IOCB_COMMAND*)p_command); 
       	
#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_CMD_SENT;
#endif
    
	return status;
	
} // HSCSI_Handle_SCSI_Request_Write

/*************************************************************************/
// HSCSI_Initialize_IOCB_Command
// Create a IOCB_COMMAND from a CDB.
/*************************************************************************/
STATUS HSCSI_Initialize_IOCB_Command(
	HSCSI_EVENT_CONTEXT *p_context, 
	HSCSI_IOCB_COMMAND **pp_command,
	U32		Flags)
{
	HSCSI_IOCB_COMMAND	*p_command;
	STATUS				 status = NU_SUCCESS;
	SCB_PAYLOAD			*pP = 
						(SCB_PAYLOAD *)HSCSI_I2O_Get_Payload(p_context->message);
	IDLUN				*p_idlun = (IDLUN *)&pP->IdLun;

	
	HSCSI_TRACE_ENTRY(HSCSI_Initialize_IOCB_Command);
	
    // Get pointer to next IOCB in ISP request FIFO.
	p_command = (HSCSI_IOCB_COMMAND*)HSCSI_Request_FIFO_Get_Pointer(p_context->Id);
	
	// Set up Command fields
    p_command->entry_type = HSCSI_IOCB_TYPE_COMMAND;
    p_command->entry_count = 1;
    
    // We can't always send to Logical Unit Number 0.  For Daisy Chains need a LUN
    // Get the LUN number passed to us.
    p_command->LUN = p_idlun->LUN;
	
    // Get the SCSI target passed to us
    p_command->target = p_idlun->id;
    	
    // Set the timeout value in seconds.
	// timeout should be zero, or the ISP will send bus reset
	p_command->timeout = 0;
	        
    // Set direction of data transfer.
    p_command->control_flags = BYTE_SWAP16((U16)Flags);
    p_command->CDB_length = BYTE_SWAP16(p_context->CDB_length);
	// get the task queue options from the I2O message and set the IOCB
	// control flags
	switch (pP->SCBFlags & HSCSI_SCB_FLAG_TAG_TYPE_MASK) {
	
		case	HSCSI_SCB_FLAG_SIMPLE_QUEUE_TAG:	// Simple Queue
			p_command->control_flags |= BYTE_SWAP16(HSCSI_REQ_FLAG_SIMPLE_TAG);
			break;
		case	HSCSI_SCB_FLAG_HEAD_QUEUE_TAG:	// Head of Queue
			p_command->control_flags |= BYTE_SWAP16(HSCSI_REQ_FLAG_HEAD_TAG);
			break;
		case	HSCSI_SCB_FLAG_ORDERED_QUEUE_TAG:	// Ordered Queue
			p_command->control_flags |= BYTE_SWAP16(HSCSI_REQ_FLAG_ORDERED_TAG);
			break;
		case	HSCSI_SCB_FLAG_ACA_QUEUE_TAG:		// ACA Queue (no equivalent)
			break;
		case	HSCSI_SCB_FLAG_NO_TAG_QUEUEING:	// UnTagged
		default:
			// do nothing
			break;
	}
	    
	// Move CDB into IOCB
	Mem_Copy(&p_command->CDB, // dest
		(CDB12*)&pP->CDB, sizeof(CDB12));
	        
    *pp_command = p_command; 		// return the new IOCB
    	
    return status;

} // HSCSI_Initialize_IOCB_Command
