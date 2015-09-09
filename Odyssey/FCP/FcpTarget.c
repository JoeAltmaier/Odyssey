/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpTarget.c
// 
// Description:
// This module implements the target mode of the FCP driver.  In target
// mode, the driver receives SCSI commands from the host.
//
// Update Log 
// 4/14/98 Jim Frandeen: Create file
// 5/5/98 Jim Frandeen: Use C++ comment style
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/17/98 Michael G. Panas: Many changes, remove FCP_Handle_CDB_Sense_Command
// and FCP_Handle_New_IOCB since this code not needed
// 9/29/98 Michael G. Panas: Added support for real I2O SGLs
// 10/05/98 Michael G. Panas: Remove test LUN conversion
// 10/30/98 Michael G. Panas: Fix bug in FCP_Send_Driver_Sense_Data
// 11/20/98 Michael G. Panas: add DEBUG info
// 11/30/98 Michael G. Panas: New memory allocation methods
// 02/17/99 Michael G. Panas: convert to new Message format and remove I2O
/*************************************************************************/
#include "FcpCommon.h"
#include "FcpBuffer.h"
#include "FcpCheck.h"
#include "FcpIOCB.h"
#include "FcpISP.h"
#include "FcpMemory.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"
#include "FcpMessage.h"
#include "FcpRequestFIFO.h"
#include "FcpTarget.h"
#include "Scsi.h"
#include "FcpString.h"
#include "FcpI2O.h"
#include "RequestCodes.h"


/*************************************************************************/
// Target globals
/*************************************************************************/
VOID		 *FCP_p_target_stack;			// Array of stacks
NU_TASK		 *FCP_p_target_task;			// Array of Target_Task
NU_QUEUE	 *FCP_p_target_wait_queue;		// Array of wait_queue
UNSIGNED	 *FCP_p_target_wait_queue_space;// Array of UNSIGNED

/*************************************************************************/
// Forward References
/*************************************************************************/
STATUS	FCP_Handle_Accept_Target_IO(FCP_EVENT_CONTEXT *p_context);
			
STATUS	FCP_Handle_CDB_Command(FCP_EVENT_CONTEXT *p_context);
			
STATUS	FCP_Handle_CDB_No_Data(FCP_EVENT_CONTEXT *p_context);
			
STATUS	FCP_Handle_CDB_Read_Command(FCP_EVENT_CONTEXT *p_context,
			UNSIGNED transfer_length);
			
STATUS	FCP_Handle_CDB_Write_Command(FCP_EVENT_CONTEXT *p_context,
			UNSIGNED transfer_length);
			
STATUS	FCP_Handle_CTIO_Final(FCP_EVENT_CONTEXT *p_context);

STATUS	FCP_Handle_CTIO_Write(FCP_EVENT_CONTEXT *p_context);

STATUS	FCP_Handle_I2O_Response(FCP_EVENT_CONTEXT *p_context);

STATUS	FCP_Send_Check_Status(FCP_EVENT_CONTEXT *p_context, 
			FCP_CHECK_CONDITION check_condition);
			
STATUS	FCP_Send_CTIO(FCP_EVENT_CONTEXT *p_context, 
			FCP_EVENT_ACTION action, U8 SCSI_status, UNSIGNED flags);
			
STATUS	FCP_Send_CTIO_Check(FCP_EVENT_CONTEXT *p_context, 
			FCP_EVENT_ACTION action, U8 *SCSI_sense,
			UNSIGNED sense_length, UNSIGNED flags);
			
STATUS	FCP_Send_Driver_Sense_Data(FCP_EVENT_CONTEXT *p_context);
				
STATUS FCP_Handle_Small_Write_Command(FCP_EVENT_CONTEXT *p_context, UNSIGNED transfer_length);

STATUS FCP_Handle_Small_Read_Command(FCP_EVENT_CONTEXT *p_context, UNSIGNED transfer_length);

STATUS FCP_Process_Failed_I2O_Request (FCP_EVENT_CONTEXT *p_context, STATUS status);


extern void	FCP_Build_ReadCTIO (FCP_EVENT_CONTEXT *pContext) ;

extern STATUS	FCP_Handle_CTIO_CacheWrite (FCP_EVENT_CONTEXT *pContext);

extern STATUS	FCP_Handle_Cache_Write_Command (FCP_EVENT_CONTEXT *pContext, UNSIGNED transfer_length);

extern STATUS	FCP_Handle_Cache_Read_Command (FCP_EVENT_CONTEXT *pContext, UNSIGNED transfer_length);

extern STATUS	FCP_Cache_Send_CTIO (FCP_EVENT_CONTEXT *p_context, FCP_EVENT_ACTION next_action);

extern void		FCP_Invalidate_Cache_Buffers(FCP_EVENT_CONTEXT *p_context);

extern void		FCP_Free_Cache_Buffers(FCP_EVENT_CONTEXT *p_context);



/*************************************************************************/
// FCP_Target_Create
// Create FCP_Target object
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS FCP_Target_Create(PINSTANCE_DATA Id)
{
 	FCP_TRACE_ENTRY(FCP_Target_Create);
			
	
	return NU_SUCCESS;
	
} // FCP_Target_Create

/*************************************************************************/
// FCP_Target_Destroy
// Destroy FCP_Target object
/*************************************************************************/
void FCP_Target_Destroy()
{
	FCP_TRACE_ENTRY(FCP_Target_Destroy);
	
	// TODO	
} // FCP_Target_Destroy


/*************************************************************************/
// FCP_Handle_Accept_Target_IO
// We are called from FCP_Event_Task when an 
// IOCB_TYPE_ACCEPT_TARGET_IO_TYPE_2 type IOCB has been received
// in the response FIFO.
/*************************************************************************/
STATUS FCP_Handle_Accept_Target_IO(FCP_EVENT_CONTEXT *p_context)
{
	STATUS		status;
	
	FCP_TRACE_ENTRY(FCP_Handle_Accept_Target_IO);
	
	switch (p_context->iocb.status)
	{
		case STATUS_CDB_RECEIVED:
		
			status = FCP_Handle_CDB_Command(p_context);
			break;
			
		case STATUS_PATH_INVALID:
		
			// ATIO for LUN that is disabled
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_Handle_Accept_Target_IO", 
				"STATUS_PATH_INVALID",
				0,
				p_context->iocb.status);
				
			// TODO
			status = FCP_ERROR_DISABLED_LUN;
			break;
			
		case STATUS_CAPABILITY_NOT_AVAILABLE:
		
			// Overflow of command resource count
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_Handle_Accept_Target_IO", 
				"STATUS_CAPABILITY_NOT_AVAILABLE",
				0,
				p_context->iocb.status);
				
			// TODO
			status = FCP_ERROR_RESOURCE_OVERFLOW;
			break;
			
		case STATUS_BUS_DEVICE_RESET:
		
			// ISP received entry while recovering from the receipt
			// of Bus Device Reset message
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_Handle_Accept_Target_IO", 
				"STATUS_BUS_DEVICE_RESET",
				0,
				p_context->iocb.status);
				
			// TODO
			status = FCP_ERROR_DEVICE_RESET;
			break;

		default:
		
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_Handle_Accept_Target_IO", 
				"Unrecognized status",
				0,
				p_context->iocb.status);
			
			// TODO
			status = FCP_ERROR_INVALID_ATIO_STATUS;
	}
	
	if (status != NU_SUCCESS)
	{
			
		STATUS		status;

		FCP_PRINT_STRING(TRACE_L1, "\n\rAccept Target IO deallocating partion");

		// Free the Status IOCB allocated for the context
		FCP_Free(p_context->pStatusIOCB);
		
    	// Deallocate the FCP_EVENT_CONTEXT 
    	// allocated by FCP_ISR_Response_Queue

    	status = NU_Deallocate_Partition(p_context);
    	if (status != NU_SUCCESS)
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_Handle_CTIO_Final", 
				"NU_Deallocate_Partition for context failed",
				status,
				0);
	}
		
	return status;
	
} // FCP_Handle_Accept_Target_IO

/*************************************************************************/
// FCP_Handle_CDB_Command
// We have received a CDB command.  Check to see what the command is
// so that we can set up buffer space if necessary.
/*************************************************************************/
STATUS FCP_Handle_CDB_Command(FCP_EVENT_CONTEXT *p_context)
{	
	STATUS							 status = NU_SUCCESS;
	UNSIGNED						 transfer_length;
	PINSTANCE_DATA					 Id;
	SCB_PAYLOAD						 payload;
	U8								 type ;
	
	FCP_TRACE_ENTRY(FCP_Handle_CDB_Command);
	
	Id = p_context->Id;			// recover our context
	
#if defined(FCP_DEBUG)    
	// DEBUG
	Id->CmdsExecuting++;
	if (Id->CmdsExecuting > Id->MaxCmdsExecuting)
		Id->MaxCmdsExecuting++;
	Id->Last = (void *) p_context;
#endif

	// Check to see if we handle this LUN
	if ((BYTE_SWAP16(p_context->iocb.LUN) < 0)
		|| (BYTE_SWAP16(p_context->iocb.LUN) >= (Id->FCP_config.num_LUNs)))
	{
		// This is not a LUN that we recognize
		if (p_context->iocb.CDB.Cmd == CMD_REQUEST_SENSE)
		
			// The host is requesting sense data, so we must have sent
			// a check status for this LUN.  Now send sense data back to
			// the host 
			status = FCP_Send_Driver_Sense_Data(p_context);
		else
		
			// Send check status for this command.  Remember that we 
			// sent check status for this LUN and this initiator.
			status = FCP_Send_Check_Status(p_context, FCP_CHECK_BAD_LUN);
				
		return status; 
	}

    FCP_DUMP_HEX(TRACE_L8, "\n\rATIO IOCB", (U8 *)&p_context->iocb, sizeof(IOCB_ATIO_TYPE_2));
    	
	// Start building the I2O message, fill in all fields
	payload.SCBFlags = FCP_SCB_FLAG_SENSE_DATA_IN_MESSAGE;
	
	// get the task queue options and pass back in the message
	switch (p_context->iocb.task_codes) {
	case	0:	// Simple Queue
		payload.SCBFlags |= FCP_SCB_FLAG_SIMPLE_QUEUE_TAG;
		break;
	case	1:	// Head of Queue
		payload.SCBFlags |= FCP_SCB_FLAG_HEAD_QUEUE_TAG;
		break;
	case	2:	// Ordered Queue
		payload.SCBFlags |= FCP_SCB_FLAG_ORDERED_QUEUE_TAG;
		break;
	case	4:	// ACA Queue
		payload.SCBFlags |= FCP_SCB_FLAG_ACA_QUEUE_TAG;
		break;
	case	5:	// UnTagged
		payload.SCBFlags |= FCP_SCB_FLAG_NO_TAG_QUEUEING;
		break;
	default:
		payload.SCBFlags |= FCP_SCB_FLAG_NO_TAG_QUEUEING;
		break;
	}
	
	payload.ByteCount = BYTE_SWAP32(p_context->iocb.transfer_length);
	
	payload.CDBLength = CDB_Get_CDB_Length(&p_context->iocb.CDB);
	
    // fill in the LUN and id fields
    payload.IdLun.HostId = p_context->iocb.initiator_ID;
    payload.IdLun.LUN = BYTE_SWAP16(p_context->iocb.LUN);
	//payload.IdLun.id = 0;		// later, ISP2200 Target Id
    if (Id->ISP_Type == 0x00002200)
    	// 2200 passes the ID in the IOCB
    	payload.IdLun.id = p_context->iocb.target_ID;
    else
    	// for 2100, pass the configured Hard ID
    	payload.IdLun.id = Id->FCP_config.hard_address;
    
	transfer_length = BYTE_SWAP32(p_context->iocb.transfer_length);
	// Copy the CDB from the ATIO into the I2O SCSI message.
	bcopy(
		(char *)&p_context->iocb.CDB,
		(char *)&payload.CDB,		// destination
		FCP_SCSI_CDB_LENGTH);
	// Switch on the type of operation specified in the CDB of
	// the Accept Target I/O IOCB.
	switch (p_context->iocb.CDB.Cmd)
	{
		
		case CMD_REQUEST_SENSE:
		
		    // Check to see if we have a driver check condition for this
		    // initiator and this LUN.
		    if (FCP_Check_Get(BYTE_SWAP16(p_context->iocb.LUN), p_context->iocb.initiator_ID))
		    {
		    	status = FCP_Send_Driver_Sense_Data(p_context);
		    	return status;
		    }
			type = SMALL_READ_XFER; 
			transfer_length = sizeof(REQUEST_SENSE);
			break ;
		    // the rest of the code is the same as a MODE_SENSE
		case CMD_READ_CAPACITY:
			type = SMALL_READ_XFER; 
			transfer_length = 8 ;
			break ;
		case CMD_MODE_SENSE6:
		case CMD_MODE_SENSE10:
			type = SMALL_READ_XFER; 
			break ;
		case CMD_SEND_DIAG:
			type = SMALL_READ_XFER; 
			break ;
		case CMD_INQUIRY:
			type = SMALL_READ_XFER; 
			break ;
		case CMD_READ6:
		case CMD_READ10:
			type = READ_BUFFER_XFER ;
			break;

		case CMD_WRITE6:
		case CMD_WRITE10:
			type = WRITE_BUFFER_XFER ;
			break;
		case CMD_MODE_SELECT6:
		case CMD_MODE_SELECT10:
			type = SMALL_WRITE_XFER; 
			break;
			
		default:
			// For all other commands, send the CDB along with no data transfer
			type = NO_DATA_XFER ;
			break ;
	}
	p_context->req_type = type ;
	switch (type)
	{
		case NO_DATA_XFER:
			// get a message frame to use for this command
			p_context->message = FCP_Allocate_Message(SCSI_SCB_EXEC);
			if (p_context->message == NULL)
			{
				// If we can't allocate a message, 
				// we can't handle this interrupt!
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_ISR_Response_Queue", 
					"Fcp_Allocate_Message for message failed",
					status,
					(UNSIGNED)Id);
				return (status);
			}
    	
			FCP_I2O_Add_Payload(p_context->message, &payload, sizeof(SCB_PAYLOAD));
			status = FCP_Handle_CDB_No_Data(p_context);
if (status != NU_SUCCESS)
{
	FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
		"FCP_Handle_CDB_Command", 
		"FCP_Handle_CDB_No_Data failed",
		status,
		0);
}

			break ;
			// Set up cache for read
		case READ_BUFFER_XFER:
			
			status = FCP_Handle_Cache_Read_Command (p_context, transfer_length) ;			
if (status != NU_SUCCESS)
{
	FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
		"FCP_Handle_CDB_Command", 
		"FCP_Handle_Cache_Read_Command failed",
		status,
		0);
}
			break;
		case SMALL_READ_XFER:
			// get a message frame to use for this command
			p_context->message = FCP_Allocate_Message(SCSI_SCB_EXEC);
			if (p_context->message == NULL)
			{
				// If we can't allocate a message, 
				// we can't handle this interrupt!
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_ISR_Response_Queue", 
					"Fcp_Allocate_Message for message failed",
					status,
					(UNSIGNED)Id);
				return (status);
			}
			// get PCI data block from heap and use for transfer
			FCP_I2O_Add_Payload(p_context->message, &payload, sizeof(SCB_PAYLOAD));
			status = FCP_Handle_Small_Read_Command (p_context, transfer_length) ;			
if (status != NU_SUCCESS)
{
	FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
		"FCP_Handle_CDB_Command", 
		"FCP_Handle_Small_Read_Command failed",
		status,
		0);
}

			break ;
		case WRITE_BUFFER_XFER:
			status = FCP_Handle_Cache_Write_Command (p_context, transfer_length) ;			
if (status != NU_SUCCESS)
{
	FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
		"FCP_Handle_CDB_Command", 
		"FCP_Handle_Cache_Write_Command failed",
		status,
		0);
}

			break;
		case SMALL_WRITE_XFER:
			// get a message frame to use for this command
			p_context->message = FCP_Allocate_Message(SCSI_SCB_EXEC);
			if (p_context->message == NULL)
			{
				// If we can't allocate a message, 
				// we can't handle this interrupt!
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_ISR_Response_Queue", 
					"Fcp_Allocate_Message for message failed",
					status,
					(UNSIGNED)Id);
				return (status);
			}
			// get PCI data block from heap and use for transfer
			FCP_I2O_Add_Payload(p_context->message, &payload, sizeof(SCB_PAYLOAD));
			status = FCP_Handle_Small_Write_Command (p_context, transfer_length) ;			
if (status != NU_SUCCESS)
{
	FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
		"FCP_Handle_CDB_Command", 
		"FCP_Handle_Small_Write_Command failed",
		status,
		0);
}

			break;
	}
	return status;
	
} // FCP_Handle_CDB_Command

/*************************************************************************/
// FCP_Handle_CDB_No_Data
// This CDB has no data to send or receive.
// Turn the CDB into a I2O_SCSI_SCB_EXECUTE_MESSAGE and send it off to be
// executed by another driver.
/*************************************************************************/
STATUS FCP_Handle_CDB_No_Data(FCP_EVENT_CONTEXT *p_context)
{
	STATUS							 status;
	
	FCP_TRACE_ENTRY(FCP_Handle_CDB_No_Data);
	
    // Set flags to send with CTIO if request is successful.
    // Set action to perform when Message response is received
    p_context->CTIO_flags = ISP_CTIO_FLAGS_NO_DATA_XFR;
    
    status = FCP_Message_Send_Request(p_context, 
    	TARGET_ACTION_HANDLE_I2O_RESPONSE);

#if defined(FCP_DEBUG)    
    p_context->flags |= EV_SENT_MESSAGE;
#endif
    
    if (status != NU_SUCCESS)
	{
		// We were not able to send the message.
		FCP_PRINT_STRING(TRACE_L2, "\n\rFCP_Handle_CDB_No_Data: message not sent");
		status = NU_SUCCESS;
	}
    	
    return status;
    
	// When I2O response is received, FCP_Handle_I2O_Response
	// will be called by FCP_Event_Task.

} // FCP_Handle_CDB_No_Data

/*************************************************************************/
// FCP_Handle_CDB_Read_Command
// Turn the CDB into a I2O_SCSI_SCB_EXECUTE_MESSAGE and send it off to be
// executed by another driver.
/*************************************************************************/
STATUS FCP_Handle_Small_Read_Command(FCP_EVENT_CONTEXT *p_context, UNSIGNED transfer_length)
{
	STATUS							 status;
	
	FCP_TRACE_ENTRY(FCP_Handle_CDB_Read_Command);

	p_context->buffer = FCP_Alloc((tPCI|tUNCACHED), transfer_length);
    
    // Add an entry to the I2O SGL for this buffer
    FCP_I2O_Add_SGL_Entry(p_context->message, p_context->buffer, transfer_length, 
    	0 /* index */);
        
    // Create a DATA_SEGMENT_DESCRIPTOR from the SGL in the
    // I2O request so that we will have it later for the CTIO. 
    FCP_I2O_Get_SGL_Data_Segment(p_context->message, 
    	&p_context->data_segment, 
    	0 /* index */);
    // Set flags to send with CTIO if I2O request is successful.
    p_context->CTIO_flags = ISP_CTIO_FLAGS_DATA_IN;
    
    // Send the read request to be served.
    // Set action to perform when I2O response is received
    status = FCP_Message_Send_Request(p_context,
    	TARGET_ACTION_HANDLE_I2O_RESPONSE);
    if (status != NU_SUCCESS)
	{
		// We were not able to send the message.
		FCP_PRINT_STRING(TRACE_L2, "\n\rFCP_Handle_CDB_Read_Command: message not sent");
		status = NU_SUCCESS;
	}
	
#if defined(FCP_DEBUG)    
    p_context->flags |= EV_SENT_MESSAGE;
#endif

	return status;

	// When I2O response is received, FCP_Handle_I2O_Response
	// will be called by FCP_Event_Task.
	
} // FCP_Handle_CDB_Read_Command


/*************************************************************************/
// FCP_Handle_Small_Write_Command
/*************************************************************************/
STATUS FCP_Handle_Small_Write_Command(FCP_EVENT_CONTEXT *p_context, 
	UNSIGNED transfer_length)
{
	STATUS							 status;
	
 	FCP_TRACE_ENTRY(FCP_Handle_CDB_Write_Command);
 	
	p_context->buffer = FCP_Alloc((tPCI|tUNCACHED), transfer_length);

    // Add an entry to the SGL of the I2O request for this buffer
    FCP_I2O_Add_SGL_Entry(p_context->message, p_context->buffer, transfer_length, 
    	0 /* index */);
        
    // Create a DATA_SEGMENT_DESCRIPTOR from the SGL in the
    // I2O request so that we can use it later in the CTIO. 
    FCP_I2O_Get_SGL_Data_Segment(p_context->message, &p_context->data_segment, 
    	0 /* index */);
    
    // Send CTIO to the firmware to transfer the data to be written 
    // from the initiator into our buffer.
	status = FCP_Send_CTIO(p_context,
    		// action to perform when response to CTIO is received
    		TARGET_ACTION_HANDLE_CTIO_WRITE,
    		SCSI_STATUS_GOOD,
			ISP_CTIO_FLAGS_FAST_POST	// flags
			| ISP_CTIO_FLAGS_DATA_OUT); 
		
#if defined(FCP_DEBUG)    
    p_context->flags |= EV_CTIO_WRITE_SENT;
#endif
    
	return status;
	
	// When CTIO response is received, FCP_Handle_CTIO_Write
	// will be called by FCP_Event_Task.
	
} // FCP_Handle_CDB_Write_Command
	
/*************************************************************************/
// FCP_Handle_CTIO_Final
// Called by FCP_Event_Task when CTIO response is received from the host.
// This was the final CTIO received after the status was sent to the host.
/*************************************************************************/
STATUS	FCP_Handle_CTIO_Final(FCP_EVENT_CONTEXT *p_context)
{
	STATUS							 status;
	PIOCB_CTIO_TYPE_2				 p_ctio;

 	FCP_TRACE_ENTRY(FCP_Handle_CTIO_Final);

#if defined(FCP_DEBUG)    
    p_context->flags |= EV_DONE;  // may never see this one
#endif
    
	p_ctio = (PIOCB_CTIO_TYPE_2) &p_context->status_iocb;
	
    FCP_DUMP_HEX(TRACE_L8, "\n\rCTIO response IOCB", (U8 *)p_ctio, sizeof(IOCB_CTIO_TYPE_2));
    
	// Check the status of the command
	switch(p_ctio->status & STATUS_MASK)
	{
		case STATUS_REQUEST_COMPLETE:
			break;
		case STATUS_COMP_WITH_ERROR:
			FCP_PRINT_STRING(TRACE_L1, "\n\rCTIO Final Complete with error");
			break;
		
		case STATUS_REQUEST_ABORTED:
			// Host aborted the command
			// TODO:
			// Handle Aborts? In this case just throw away buffers...
			FCP_PRINT_STRING(TRACE_L1, "\n\rCTIO Final Host Abort");
			break;
			
		case STATUS_LIP_RESET:
			// Host sent LIP Reset
			// TODO:
			// Handle LIP Reset? In this case just throw away buffers...
			FCP_PRINT_STRING(TRACE_L1, "\n\rCTIO Final LIP Reset");
			break;
			
		case STATUS_PORT_LOGGED_OUT:
			// Host Loggeed out before we were done
			// TODO:
			// In this case just throw away buffers...
			FCP_PRINT_STRING(TRACE_L1, "\n\rCTIO Final Port Logged out");
			break;
			
		default:
			// TODO:
			// Need to recover from errors here
			FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
				"FCP_Handle_CTIO_Final", 
				"bad return status",
				(UNSIGNED)&p_context,
				p_ctio->status);
				
				FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
				"FCP_Handle_CTIO_Final", 
				"RX_ID =",
				p_context->iocb.RX_ID,
				0);
				
			break;
	}

	switch (p_context->req_type)
	{
		case READ_BUFFER_XFER:
/* read has either not used a message buffer or has already released it */
/* p_context->message pts to IOCB - so NULL it out */
				p_context->message = NULL ;			
				if (p_context->p_dsd)
					FCP_Free (p_context->p_dsd) ;

		case WRITE_BUFFER_XFER:
				// cache buffers were released prior to sending IO complete status
				break ;
		case SMALL_READ_XFER:
		case SMALL_WRITE_XFER:
				if (p_context->buffer)
					FCP_Free(p_context->buffer) ;
		case NO_DATA_XFER:
				p_context->buffer = NULL ;
				break;
	}
	// Deallocate the message
	if (p_context->message)
		FCP_Free_Message(p_context->message);
	if (p_context->buffer)
		FCP_Free_Cache_Buffers(p_context);
	
	// If last CTIO, free the Status IOCB
	if (p_context->CTIO_flags & ISP_CTIO_FLAGS_SEND_SCSI_STATUS)
	{
		FCP_Free(p_context->pStatusIOCB);
#if defined(FCP_DEBUG)    
	// DEBUG
	p_context->Id->CmdsExecuting--;
	p_context->Id->LastDone = (void *) p_context;
#endif

	}

	// Free the context
    status = NU_Deallocate_Partition(p_context);
    if (status != NU_SUCCESS)
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Handle_CTIO_Final", 
			"NU_Deallocate_Partition for context failed",
			status,
			(UNSIGNED)p_context);
		
	return status;
		
} // FCP_Handle_CTIO_Final


/*************************************************************************/
// FCP_Handle_CTIO_Write
// Called by FCP_Event_Task when CTIO response is received.
/*************************************************************************/
STATUS	FCP_Handle_CTIO_Write(FCP_EVENT_CONTEXT *p_context)
{
	STATUS							 status;
	PIOCB_CTIO_TYPE_4				 p_ctio;
    
 	FCP_TRACE_ENTRY(FCP_Handle_CTIO_Write);

#if defined(FCP_DEBUG)    
    p_context->flags |= EV_CTIO_WRITE_RESP_IN;
#endif

	p_ctio = (PIOCB_CTIO_TYPE_4) &p_context->status_iocb;
	if (p_context->req_type == WRITE_BUFFER_XFER )
	{
		status = FCP_Handle_CTIO_CacheWrite (p_context) ;
	    // Set flags to send with CTIO if I2O request is successful.
	    p_context->CTIO_flags = ISP_CTIO_FLAGS_NO_DATA_XFR;  // just return status
    
		return (status) ;
	}
	
    FCP_DUMP_HEX(TRACE_L8, "\n\rCTIO response IOCB (WRITE)",
    				(U8 *)p_ctio, sizeof(IOCB_CTIO_TYPE_2));
    
	// Check the status of the CTIO command
	switch(p_ctio->status & STATUS_MASK)
	{
		case STATUS_REQUEST_COMPLETE:
		case STATUS_COMP_WITH_ERROR:
			break;						// these status values are OK
			
		case STATUS_REQUEST_ABORTED:
		{
			REQUEST_SENSE	abort_sense = {RESPONSE_CODE, 0, SENSE_ABORTED_COMMAND, 0,0,0,0,
									ADDITIONAL_LENGTH, 0,0,0,0, ASC_LOGICAL_UNIT_TIMEOUT,
									0,0,0,0,0 };
			// Host aborted the command
			// TODO:
			// Handle Aborts? In this case just throw away buffers...
			FCP_PRINT_STRING(TRACE_L1, "\n\rCTIO Write Host Abort");
			// SCSI request was not successful.
			status = FCP_Send_CTIO_Check(p_context, 
	    		TARGET_ACTION_HANDLE_CTIO_FINAL,
	    		(U8 *) &abort_sense,
	    		sizeof(REQUEST_SENSE),
				  ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
				| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
				| ISP_CTIO_FLAGS_NO_DATA_XFR);
		}
			return status;
			
		case STATUS_LIP_RESET:
		{
			REQUEST_SENSE	lip_sense = {RESPONSE_CODE, 0, SENSE_ABORTED_COMMAND, 0,0,0,0,
									ADDITIONAL_LENGTH, 0,0,0,0, ASC_SCSI_BUS_RESET_OCCURED,
									0,0,0,0,0 };
			// Host sent LIP Reset
			// TODO:
			// Handle LIP Reset? In this case just throw away buffers...
			FCP_PRINT_STRING(TRACE_L1, "\n\rCTIO Write LIP Reset");
			// SCSI request was not successful.
			status = FCP_Send_CTIO_Check(p_context, 
	    		TARGET_ACTION_HANDLE_CTIO_FINAL,
	    		(U8 *) &lip_sense,
	    		sizeof(REQUEST_SENSE),
				  ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
				| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
				| ISP_CTIO_FLAGS_NO_DATA_XFR);
		}
			return status;
			
		default:
			// TODO:
			// Need to recover from errors here
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_Handle_CTIO_Write", 
				"bad return status",
				0,
				p_ctio->status);
			break;
	}
	
    // Set flags to send with CTIO if I2O request is successful.
    p_context->CTIO_flags = ISP_CTIO_FLAGS_NO_DATA_XFR;  // just return status
    
	// Send the request to be served
    // Set action to perform when Msg response is received
    status = FCP_Message_Send_Request(p_context, 
    	TARGET_ACTION_HANDLE_I2O_RESPONSE);
    if (status != NU_SUCCESS)
	{
		// We were not able to send the message.
		FCP_PRINT_STRING(TRACE_L2, "\n\rFCP_Handle_CTIO_Write: message not sent");
		status = NU_SUCCESS;
	}
	
#if defined(FCP_DEBUG)    
    p_context->flags |= EV_SENT_MESSAGE;
#endif
    
	return status;
    	
	// When Msg response is received, FCP_Handle_I2O_Response
	// will be called by FCP_Event_Task.
	
} // FCP_Handle_CTIO_Write

/*************************************************************************/
// FCP_Process_Failed_I2O_Request
// Called by FCP_Handle_I2O_Response when bad status response received.
/*************************************************************************/
STATUS FCP_Process_Failed_I2O_Request (FCP_EVENT_CONTEXT *p_context, STATUS status)
{
	FCP_EVENT_CONTEXT	*pNext, *pPrev;
	SCB_REPLY_PAYLOAD	*pP  ;
	IOCB_STATUS_TYPE0	*pCommandSense ;
	IOCB_STATUS_TYPE0	*pStatusIOCB;
	U8					*pSenseData  ;
	UNSIGNED			transfer_count ;
	U8					IO_Complete = FALSE ;

	switch (p_context->req_type)    
	{
		case SMALL_READ_XFER:
		case SMALL_WRITE_XFER:
		case NO_DATA_XFER:
				IO_Complete = TRUE ;
				break ;
		case READ_BUFFER_XFER:	
				pStatusIOCB = (IOCB_STATUS_TYPE0 *) p_context->pStatusIOCB;
				// Decrement entry count in master status IOCB here
				// since we're not calling FCP_Cache_Send_CTIO to send the data
				// only used to determine when sending last IOCB
				pStatusIOCB->entry_count--;
				if (pStatusIOCB->entry_count == 0)
				{	// This is the last context for this ATIO request
					IO_Complete = TRUE;
				}
				// Invalidate all cache buffers associated with this context
				FCP_Invalidate_Cache_Buffers(p_context);
				// Free all cache buffers associated with this context
				FCP_Free_Cache_Buffers(p_context);
				break;
		case WRITE_BUFFER_XFER:
				// Invalidate all cache buffers associated with this context
				FCP_Invalidate_Cache_Buffers(p_context);
				// Free all cache buffers associated with this context
				FCP_Free_Cache_Buffers(p_context);
				pNext = p_context->req_link.pNextContext ;
				pPrev =	p_context->req_link.pPrevContext ;
				if (pNext == NULL)
				{ // we are tail - if we are head also we are done
					if (pPrev == NULL)
					{ // IO complete - send status
						IO_Complete = TRUE ;
					}
					else
					{ // previous is new chain
						pPrev->req_link.pNextContext = NULL ;
					}
				}
				else
				{ // not tail remove link from chain
					if (pPrev)
						pPrev->req_link.pNextContext = pNext ;
					pNext->req_link.pPrevContext = pPrev ;
				}
				break ;
	}
	pCommandSense = p_context->pStatusIOCB ;
	pSenseData = &pCommandSense->sense_data[0] ;
	pCommandSense->SCSI_status = status ;
	if ((status & FCP_SCSI_DEVICE_DSC_MASK) != FCP_SCSI_DSC_CHECK_CONDITION)
	{
			// failed last command, send check status back
			pSenseData[0] = RESPONSE_CODE ;
			pSenseData[2] = SENSE_HARDWARE_ERROR ;
			pSenseData[7] = ADDITIONAL_LENGTH ;
			pSenseData[12] = ASC_LOGICAL_UNIT_DOES_NOT_RESPOND_TO_SELECTION ;
	    	transfer_count = sizeof(REQUEST_SENSE) ;
			FCP_PRINT_HEX(TRACE_L1, "\n\rI2O Response Bad ",
							status);
	}
	else
	{
			pP = (SCB_REPLY_PAYLOAD *)FCP_I2O_Get_Payload(p_context->message);
	    	transfer_count = pP->AutoSenseTransferCount;
	    	// copy sense data
			bcopy(
				(char *) &pP->SenseData[0],		// src
				(char *)pSenseData,				// dest
				transfer_count) ; 
				
				FCP_PRINT_HEX(TRACE_L1, "\n\rI2O Response Bad - check cond ",
							status);
	} 
	pCommandSense->sense_data_length = transfer_count ;
	
	// free message
	if (p_context->message)
		FCP_Free_Message (p_context->message) ;
	p_context->message = NULL;
	
	if (IO_Complete)
	{	// This is the last context for this ATIO - send status
		p_context->CTIO_flags |= ISP_CTIO_FLAGS_SEND_SCSI_STATUS;	// so we will free pStatusIOCB
		status = FCP_Send_CTIO_Check(p_context, 
   			TARGET_ACTION_HANDLE_CTIO_FINAL,
			pSenseData,
   			transfer_count,
			ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
			| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
			| ISP_CTIO_FLAGS_NO_DATA_XFR);
	}
	else
	{	// done with context
	  	status = NU_Deallocate_Partition(p_context);
    	if (status != NU_SUCCESS)
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"Process Failed I2O Request", 
				"NU_Deallocate_Partition for context failed",
				status,
				0);
		status = NU_SUCCESS;
	}
	return (status);
}

/*************************************************************************/
// FCP_Handle_I20_Response
// Called by FCP_Event_Task when I2O response message is received.
// Check the DetailedStatusCode for errors, return check status if any.
/*************************************************************************/
STATUS FCP_Handle_I2O_Response(FCP_EVENT_CONTEXT *p_context)
{	
	FCP_EVENT_CONTEXT	*pNext, *pPrev;
	IOCB_STATUS_TYPE0	*pCommandSense ;
	STATUS				status ; 
	UNSIGNED			transfer_count ;
	SCB_REPLY_PAYLOAD	*pP;
	IOCB_STATUS_TYPE0	*pStatusIOCB;
	U8					IO_Complete = FALSE ;

 	FCP_TRACE_ENTRY(FCP_Handle_I2O_Response);

#if defined(FCP_DEBUG)    
    p_context->flags |= EV_REPLY_RECEIVED;
#endif

	pP =(SCB_REPLY_PAYLOAD *)FCP_I2O_Get_Payload(p_context->message);

	pStatusIOCB = (IOCB_STATUS_TYPE0 *) p_context->pStatusIOCB;

	// subtract what was transferred from residual length
    pStatusIOCB->residual_transfer_length -= pP->TransferCount;

	status = FCP_I2O_Get_DetailedStatusCode(p_context->message);
	if (status != FCP_SCSI_DSC_SUCCESS)
	{ // failed command - set up request sense data
		status = FCP_Process_Failed_I2O_Request (p_context, status) ;
		return (status) ; 
	} 
	// good response - continue/end IO
	if (p_context->message)
		FCP_Free_Message (p_context->message) ;
	p_context->message = NULL ;
	switch (p_context->req_type)    
	{
		case SMALL_READ_XFER:
		case SMALL_WRITE_XFER:
		case NO_DATA_XFER:
				IO_Complete = TRUE ;
				break ;
		case READ_BUFFER_XFER:	
				// cache fill completed o.k - send to host
				FCP_Build_ReadCTIO (p_context) ;
				status = FCP_Cache_Send_CTIO (p_context, TARGET_ACTION_HANDLE_CTIO_FINAL);
				break ;
		case WRITE_BUFFER_XFER:
				// write completed o.k. - release cache
				FCP_Free_Cache_Buffers(p_context);
				pNext = p_context->req_link.pNextContext ;
				pPrev =	p_context->req_link.pPrevContext ;
				if (pNext == NULL)
				{ // we are tail - if we are head also we are done
					if (pPrev == NULL)
					{ // IO complete - send status
						IO_Complete = TRUE ;
					}
					else
					{ // previous is new chain
						pPrev->req_link.pNextContext = NULL ;
					}
				}
				else
				{ // not tail remove link from chain
					if (pPrev)
						pPrev->req_link.pNextContext = pNext ;
					pNext->req_link.pPrevContext = pPrev ;
				}
				if (!IO_Complete)
				{
					status = NU_Deallocate_Partition(p_context);
					if (status != NU_SUCCESS)
						FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
						"FCP_Handle_I2O_Response", 
						"NU_Deallocate_Partition for context failed",
						status,
						(UNSIGNED)p_context);
				}
					
				break ;
	}
	// Check to see if the request was successful
	if (IO_Complete)
	{
		// This SCSI request was successful
		pCommandSense = p_context->pStatusIOCB ;
		if (pCommandSense->SCSI_status == FCP_SCSI_DSC_SUCCESS)
		{
			p_context->CTIO_flags |= ISP_CTIO_FLAGS_SEND_SCSI_STATUS;	// so we will free pStatusIOCB
			status = FCP_Send_CTIO(p_context, 
    			TARGET_ACTION_HANDLE_CTIO_FINAL,
    			SCSI_STATUS_GOOD,    
				ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
				| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
				| ISP_CTIO_FLAGS_FAST_POST
				| p_context->CTIO_flags);
		}
		else
		{
			p_context->CTIO_flags |= ISP_CTIO_FLAGS_SEND_SCSI_STATUS;	// so we will free pStatusIOCB
			transfer_count = pCommandSense->sense_data_length ;
			status = FCP_Send_CTIO_Check(p_context, 
		   		TARGET_ACTION_HANDLE_CTIO_FINAL,
		   		(U8 *) &pCommandSense->sense_data[0],
		   		transfer_count,
				  ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
				| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
				| ISP_CTIO_FLAGS_NO_DATA_XFR);
			
		}
	}
    return status;
    
} // FCP_Handle_I2O_Response

/*************************************************************************/
// FCP_Send_Check_Status
// The command failed to execute due to an error encountered in the driver.
// Return check status to host.
/*************************************************************************/
STATUS FCP_Send_Check_Status(FCP_EVENT_CONTEXT *p_context, 
	FCP_CHECK_CONDITION check_condition)
{
	STATUS							 status;

 	FCP_TRACE_ENTRY(FCP_Send_Check_Status);
	
	// Save check_condition so that we will have it when the
	// host asks for sense information.
	FCP_Check_Save(BYTE_SWAP16(p_context->iocb.LUN), p_context->iocb.initiator_ID,
		check_condition);
	
    // Send Continue Target I/O with check condition 
	status = FCP_Send_CTIO(p_context, 
    	TARGET_ACTION_HANDLE_CTIO_FINAL,
    	SCSI_STATUS_CHECK,    
		  ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
		| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
		| ISP_CTIO_FLAGS_FAST_POST
		| ISP_CTIO_FLAGS_NO_DATA_XFR); 

#if defined(FCP_DEBUG)    
    p_context->flags |= EV_CTIO_ERROR_SENT;
#endif
    
	return status;
	    
} // FCP_Send_Check_Status

/*************************************************************************/
// FCP_Send_CTIO
// Send Continue Target I/O to ISP
/*************************************************************************/
STATUS FCP_Send_CTIO(FCP_EVENT_CONTEXT *p_context, 
	FCP_EVENT_ACTION next_action, 
	U8 SCSI_status, UNSIGNED flags)
{
	IOCB_CTIO_MODE_0 			*p_CTIO;
	IOCB_STATUS_TYPE0			*pStatusIOCB;
	STATUS						 status;
    
 	FCP_TRACE_ENTRY(FCP_Send_CTIO);
	
    // Create Continue Target I/O IOCB in ISP request FIFO
    // Get pointer to next IOCB in request FIFO.
	p_CTIO = (IOCB_CTIO_MODE_0*)FCP_Request_FIFO_Get_Pointer(p_context->Id);
	
	// Initialize CTIO fields
    p_CTIO->entry_type = IOCB_TYPE_CONTINUE_TARGET_IO_TYPE_2;
    p_CTIO->entry_count = 1;
    //p_CTIO->LUN = p_context->iocb.LUN;
    p_CTIO->initiator_ID = p_context->iocb.initiator_ID;
    p_CTIO->RX_ID = p_context->iocb.RX_ID;  
    p_CTIO->timeout = BYTE_SWAP16(30);
    p_CTIO->transfer_length = p_context->iocb.transfer_length; // already swappped
    
	// Set SCSI status in reply.
    p_CTIO->SCSI_status = BYTE_SWAP16((U16)SCSI_status);

    p_CTIO->flags = BYTE_SWAP16(flags);
    	
    // Check for write data transfer (no resid on write or no data)
    if ((flags & ISP_CTIO_FLAGS_NO_DATA_XFR) == ISP_CTIO_FLAGS_DATA_IN)
    {
		pStatusIOCB = (IOCB_STATUS_TYPE0 *) p_context->pStatusIOCB;

	    // Set the residual length
		p_CTIO->residual_transfer_length = BYTE_SWAP32(pStatusIOCB->residual_transfer_length);  // fix endian
	}
	
    // Check for data transfer
    if ((flags & ISP_CTIO_FLAGS_NO_DATA_XFR) == ISP_CTIO_FLAGS_NO_DATA_XFR)
    {
    	// No data transfer
    	p_CTIO->data_segment_count = 0;
    	p_CTIO->transfer_length = 0;
    }
    else
    {
    	// Move data_segment into CTIO
    	p_CTIO->segment_0 = p_context->data_segment;	// already swapped when saved
    
    	// Only one data segment for now.
    	p_CTIO->data_segment_count = BYTE_SWAP16(1);
    	p_CTIO->transfer_length = p_CTIO->segment_0.Length;
    
    }
    
    FCP_DUMP_HEX(TRACE_L8, "\n\rCTIO MODE 0 IOCB", (U8 *)p_CTIO, sizeof(IOCB_CTIO_MODE_0));
    
    // Send Continue Target I/O IOCB to the ISP.
    // next_action specifies action to perform by FCP_Event_Task
    // when command completes.
    status = FCP_Send_Command_IOCB(p_context, next_action,
    	(IOCB_COMMAND_TYPE2*)p_CTIO); 
    
#if defined(FCP_DEBUG)    
    p_context->ctio_cnt++;
#endif
    
    return status;    
    
	// When I2O response is received, FCP_Event_Task will call
	// the next method.
	
} // FCP_Send_CTIO

/*************************************************************************/
// FCP_Send_CTIO_Check
// Send Continue Target I/O with sense data to ISP
// Data can not be sent with this mode
/*************************************************************************/
STATUS FCP_Send_CTIO_Check(FCP_EVENT_CONTEXT *p_context, 
	FCP_EVENT_ACTION next_action, U8 *SCSI_sense,
	UNSIGNED sense_length, UNSIGNED flags)
{
	IOCB_CTIO_MODE_1 			*p_CTIO;
	IOCB_STATUS_TYPE0			*pStatusIOCB;
	STATUS						 status;
    
 	FCP_TRACE_ENTRY(FCP_Send_CTIO_Check);
	
    // Create Continue Target I/O IOCB in ISP request FIFO
    // Get pointer to next IOCB in request FIFO.
	p_CTIO = (IOCB_CTIO_MODE_1*)FCP_Request_FIFO_Get_Pointer(p_context->Id);
	
	// Initialize CTIO fields
    p_CTIO->entry_type = IOCB_TYPE_CONTINUE_TARGET_IO_TYPE_2;
    p_CTIO->entry_count = 1;
    //p_CTIO->LUN = p_context->iocb.LUN;
    p_CTIO->initiator_ID = p_context->iocb.initiator_ID;
    p_CTIO->RX_ID = p_context->iocb.RX_ID;  
    p_CTIO->timeout = BYTE_SWAP16(30);
    
	// Set the residual length
	pStatusIOCB = (IOCB_STATUS_TYPE0 *) p_context->pStatusIOCB;
	p_CTIO->residual_transfer_length = BYTE_SWAP32(pStatusIOCB->residual_transfer_length);  // fix endian

	// Set SCSI Check status in reply.
    p_CTIO->SCSI_status = BYTE_SWAP16((U16)
    		(SCSI_STATUS_CHECK | IOCB_SCSI_FLAGS_SENSE_VALID));

    p_CTIO->flags = BYTE_SWAP16(flags + 1);		// CTIO Mode 1
    	
    // Copy the sense data into the CTIO
	bcopy(
		(char *) SCSI_sense,
		(char *) &p_CTIO->response_information[0],
		(U32)sense_length);
				
	p_CTIO->sense_length = BYTE_SWAP16(sense_length);

    FCP_DUMP_HEX(TRACE_L8, "\n\rCTIO MODE 1 IOCB", (U8 *)p_CTIO, sizeof(IOCB_CTIO_MODE_1));
    
    // Send Continue Target I/O IOCB to the ISP.
    // next_action specifies action to perform by FCP_Event_Task
    // when command completes.
    status = FCP_Send_Command_IOCB(p_context, next_action,
    	(IOCB_COMMAND_TYPE2*)p_CTIO); 
       
#if defined(FCP_DEBUG)    
    p_context->flags |= EV_CTIO_CHECK_SENT;
#endif
    
    return status;    
    
	// When I2O response is received, FCP_Event_Task will call
	// the next method.
	
} // FCP_Send_CTIO_Check

/*************************************************************************/
// FCP_Send_Driver_Sense_Data
// We have sense data for this initiator and this LUN, so send it.
/*************************************************************************/
STATUS FCP_Send_Driver_Sense_Data(FCP_EVENT_CONTEXT *p_context)
{
	STATUS							 status;
	U32								 transfer_length;

 	FCP_TRACE_ENTRY(FCP_Send_Driver_Sense_Data);
	
	// Create sense data record to send
	FCP_Check_Create_Sense(BYTE_SWAP16(p_context->iocb.LUN), 
		p_context->iocb.initiator_ID, &p_context->sense_data);
	
	// Set up data segment descriptor to point to sense_data
	//p_context->data_segment.Address = ((UNSIGNED)&p_context->sense_data  & ~KSEG1);
	p_context->data_segment.Address = (UNSIGNED)FCP_Get_DMA_Address(&p_context->sense_data);
	p_context->data_segment.Address = BYTE_SWAP32(p_context->data_segment.Address);
	
	// Get transfer_length from CDB
	transfer_length = CDB_Get_Transfer_Length(&p_context->iocb.CDB);
	if (transfer_length > sizeof(REQUEST_SENSE))
		transfer_length = sizeof(REQUEST_SENSE);
	p_context->data_segment.Length = BYTE_SWAP32(transfer_length);
	
	// Send sense data
	status = FCP_Send_CTIO(p_context, 
    	TARGET_ACTION_HANDLE_CTIO_FINAL,    
    	SCSI_STATUS_GOOD,
		ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
		| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
		| ISP_CTIO_FLAGS_FAST_POST
		| ISP_CTIO_FLAGS_DATA_IN); 
	
#if defined(FCP_DEBUG)    
    p_context->flags |= EV_CTIO_SENSE_SENT;
#endif
    
	return status;
	
} // FCP_Send_Driver_Sense_Data

