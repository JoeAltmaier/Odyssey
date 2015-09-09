/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiTarget.c
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
// 6/8/99 Charles Chan: Ported from FCP to support ISP1040B
/*************************************************************************/
#include "HscsiCommon.h"
#include "HscsiBuffer.h"
#include "HscsiCheck.h"
#include "HscsiIOCB.h"
#include "HscsiISP.h"
#include "HscsiMemory.h"
#include "HscsiMessageFormats.h"
#include "HscsiMessageStatus.h"
#include "HscsiMessage.h"
#include "HscsiRequestFIFO.h"
#include "HscsiTarget.h"
#include "Scsi.h"
#include "HscsiString.h"
#include "HscsiI2O.h"


/*************************************************************************/
// Target globals
/*************************************************************************/
VOID		 *HSCSI_p_target_stack;			// Array of stacks
NU_TASK		 *HSCSI_p_target_task;			// Array of Target_Task
NU_QUEUE	 *HSCSI_p_target_wait_queue;		// Array of wait_queue
UNSIGNED	 *HSCSI_p_target_wait_queue_space;// Array of UNSIGNED

/*************************************************************************/
// Forward References
/*************************************************************************/
STATUS	HSCSI_Handle_Accept_Target_IO(HSCSI_EVENT_CONTEXT *p_context);
			
STATUS	HSCSI_Handle_CDB_Command(HSCSI_EVENT_CONTEXT *p_context);
			
STATUS	HSCSI_Handle_CDB_No_Data(HSCSI_EVENT_CONTEXT *p_context);
			
STATUS	HSCSI_Handle_CDB_Read_Command(HSCSI_EVENT_CONTEXT *p_context,
			UNSIGNED transfer_length);
			
STATUS	HSCSI_Handle_CDB_Write_Command(HSCSI_EVENT_CONTEXT *p_context,
			UNSIGNED transfer_length);
			
STATUS	HSCSI_Handle_CTIO_Final(HSCSI_EVENT_CONTEXT *p_context);

STATUS	HSCSI_Handle_CTIO_Write(HSCSI_EVENT_CONTEXT *p_context);

STATUS	HSCSI_Handle_I2O_Response(HSCSI_EVENT_CONTEXT *p_context);

STATUS	HSCSI_Send_Check_Status(HSCSI_EVENT_CONTEXT *p_context, 
			HSCSI_CHECK_CONDITION check_condition);
			
STATUS	HSCSI_Send_CTIO(HSCSI_EVENT_CONTEXT *p_context, 
			HSCSI_EVENT_ACTION action, U8 SCSI_status, UNSIGNED flags);
			
STATUS	HSCSI_Send_CTIO_Check(HSCSI_EVENT_CONTEXT *p_context, 
			HSCSI_EVENT_ACTION action, U8 *SCSI_sense,
			UNSIGNED sense_length, UNSIGNED flags);
			
STATUS	HSCSI_Send_Driver_Sense_Data(HSCSI_EVENT_CONTEXT *p_context);
				
/*************************************************************************/
// HSCSI_Target_Create
// Create HSCSI_Target object
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS HSCSI_Target_Create(PINSTANCE_DATA Id)
{
 	HSCSI_TRACE_ENTRY(HSCSI_Target_Create);
			
	
	return NU_SUCCESS;
	
} // HSCSI_Target_Create

/*************************************************************************/
// HSCSI_Target_Destroy
// Destroy HSCSI_Target object
/*************************************************************************/
void HSCSI_Target_Destroy()
{
	HSCSI_TRACE_ENTRY(HSCSI_Target_Destroy);
	
	// TODO	
} // HSCSI_Target_Destroy


/*************************************************************************/
// HSCSI_Handle_Accept_Target_IO
// We are called from HSCSI_Event_Task when an 
// IOCB_TYPE_ACCEPT_TARGET_IO_TYPE_2 type IOCB has been received
// in the response FIFO.
/*************************************************************************
STATUS HSCSI_Handle_Accept_Target_IO(HSCSI_EVENT_CONTEXT *p_context)
{
	STATUS		status;
	
	HSCSI_TRACE_ENTRY(HSCSI_Handle_Accept_Target_IO);
	
	switch (p_context->iocb.status)
	{
		case STATUS_CDB_RECEIVED:
		
			status = HSCSI_Handle_CDB_Command(p_context);
			break;
			
		case STATUS_PATH_INVALID:
		
			// ATIO for LUN that is disabled
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_Handle_Accept_Target_IO", 
				"STATUS_PATH_INVALID",
				0,
				p_context->iocb.status);
				
			// TODO
			status = HSCSI_ERROR_DISABLED_LUN;
			break;
			
		case STATUS_CAPABILITY_NOT_AVAILABLE:
		
			// Overflow of command resource count
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_Handle_Accept_Target_IO", 
				"STATUS_CAPABILITY_NOT_AVAILABLE",
				0,
				p_context->iocb.status);
				
			// TODO
			status = HSCSI_ERROR_RESOURCE_OVERFLOW;
			break;
			
		case STATUS_RESET:
		
			// ISP received entry while recovering from the receipt
			// of Bus Device Reset message
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_Handle_Accept_Target_IO", 
				"STATUS_RESET",
				0,
				p_context->iocb.status);
				
			// TODO
			status = HSCSI_ERROR_DEVICE_RESET;
			break;

		default:
		
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_Handle_Accept_Target_IO", 
				"Unrecognized status",
				0,
				p_context->iocb.status);
			
			// TODO
			status = HSCSI_ERROR_INVALID_ATIO_STATUS;
	}
	
	if (status != NU_SUCCESS)
	{
			
		STATUS		status;
		
    	// Deallocate the HSCSI_EVENT_CONTEXT 
    	// allocated by HSCSI_ISR_Response_Queue
    	status = NU_Deallocate_Partition(p_context);
    	if (status != NU_SUCCESS)
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_Handle_CTIO_Final", 
				"NU_Deallocate_Partition for context failed",
				status,
				0);
	}
		
	return status;
	
} // HSCSI_Handle_Accept_Target_IO
*/
/*************************************************************************/
// HSCSI_Handle_CDB_Command
// We have received a CDB command.  Check to see what the command is
// so that we can set up buffer space if necessary.
/*************************************************************************/
STATUS HSCSI_Handle_CDB_Command(HSCSI_EVENT_CONTEXT *p_context)
{	
	STATUS							 status = NU_SUCCESS;
	UNSIGNED						 transfer_length;
	PINSTANCE_DATA					 Id;
	SCB_PAYLOAD						 payload;
	
	HSCSI_TRACE_ENTRY(HSCSI_Handle_CDB_Command);
	
	Id = p_context->Id;			// recover our context
	
#if defined(HSCSI_DEBUG)    
	// DEBUG
	Id->CmdsExecuting++;
	Id->Last = (void *) p_context;
#endif
	
	// Check to see if we handle this LUN
	if ((BYTE_SWAP16(p_context->iocb.LUN) < 0)
		|| (BYTE_SWAP16(p_context->iocb.LUN) >= (Id->HSCSI_config.num_LUNs)))
	{
		// This is not a LUN that we recognize
		if (p_context->iocb.CDB.Cmd == CMD_REQUEST_SENSE)
		
			// The host is requesting sense data, so we must have sent
			// a check status for this LUN.  Now send sense data back to
			// the host 
			status = HSCSI_Send_Driver_Sense_Data(p_context);
		else
		
			// Send check status for this command.  Remember that we 
			// sent check status for this LUN and this initiator.
			status = HSCSI_Send_Check_Status(p_context, HSCSI_CHECK_BAD_LUN);
				
		return status; 
	}

    HSCSI_DUMP_HEX(TRACE_L8, "\n\rATIO IOCB", (U8 *)&p_context->iocb, sizeof(IOCB_ATIO_TYPE_2));
    	
	// Start building the I2O message, fill in all fields
	payload.SCBFlags = HSCSI_SCB_FLAG_SENSE_DATA_IN_MESSAGE;
	
	// get the task queue options and pass back in the message
	switch (p_context->iocb.task_codes) {
	case	0:	// Simple Queue
		payload.SCBFlags |= HSCSI_SCB_FLAG_SIMPLE_QUEUE_TAG;
		break;
	case	1:	// Head of Queue
		payload.SCBFlags |= HSCSI_SCB_FLAG_HEAD_QUEUE_TAG;
		break;
	case	2:	// Ordered Queue
		payload.SCBFlags |= HSCSI_SCB_FLAG_ORDERED_QUEUE_TAG;
		break;
	case	4:	// ACA Queue
		payload.SCBFlags |= HSCSI_SCB_FLAG_ACA_QUEUE_TAG;
		break;
	case	5:	// UnTagged
		payload.SCBFlags |= HSCSI_SCB_FLAG_NO_TAG_QUEUEING;
		break;
	default:
		payload.SCBFlags |= HSCSI_SCB_FLAG_NO_TAG_QUEUEING;
		break;
	}
	
	payload.ByteCount = BYTE_SWAP32(p_context->iocb.transfer_length);
	
	payload.CDBLength = CDB_Get_CDB_Length(&p_context->iocb.CDB);
	
    // fill in the LUN and id fields
    payload.IdLun.HostId = p_context->iocb.initiator_ID;
    payload.IdLun.id = 0;		// later, ISP2200 Target Id
    payload.IdLun.LUN = BYTE_SWAP16(p_context->iocb.LUN);
    
	// Copy the CDB from the ATIO into the I2O SCSI message.
	Mem_Copy(&payload.CDB,		// destination
		&p_context->iocb.CDB, HSCSI_SCSI_CDB_LENGTH);
	
	// add the payload to the message
	HSCSI_I2O_Add_Payload(p_context->message, &payload, sizeof(SCB_PAYLOAD));
	
	transfer_length = BYTE_SWAP32(p_context->iocb.transfer_length);
	
	// DEBUG mgp
	HSCSI_PRINT_HEX16(TRACE_L8, "\n\rLUN = ", payload.IdLun.LUN);
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rTransfer Length = ", transfer_length);
	HSCSI_DUMP_HEX(TRACE_L8, "\n\rCDB =", (U8 *)&p_context->iocb.CDB, 16);
		
#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_CDB_IN;
#endif
    
	// Switch on the type of operation specified in the CDB of
	// the Accept Target I/O IOCB.
	switch (p_context->iocb.CDB.Cmd)
	{
		
		case CMD_REQUEST_SENSE:
		
		    // Check to see if we have a driver check condition for this
		    // initiator and this LUN.
		    if (HSCSI_Check_Get(BYTE_SWAP16(p_context->iocb.LUN), p_context->iocb.initiator_ID))
		    {
		    	status = HSCSI_Send_Driver_Sense_Data(p_context);
		    	return status;
		    }
		    // the rest of the code is the same as a MODE_SENSE
		       
		case CMD_MODE_SENSE6:
		case CMD_MODE_SENSE10:
		case CMD_SEND_DIAG:
		case CMD_INQUIRY:
		case CMD_READ6:
		case CMD_READ10:
		
			status = HSCSI_Handle_CDB_Read_Command(p_context, transfer_length);
		
			break;
			
		case CMD_READ_CAPACITY:
    	
    		// Just like a read command, except that 
    		// we transfer 8 bytes back to the host. 
			status = HSCSI_Handle_CDB_Read_Command(p_context, 
				8 /* transfer_length is 8 bytes */ );
		
			break;
			
		case CMD_WRITE6:
		case CMD_WRITE10:
		case CMD_MODE_SELECT6:
		case CMD_MODE_SELECT10:
		
			status = HSCSI_Handle_CDB_Write_Command(p_context, 
				transfer_length);
			break;
			
		default:
		
			// For all other commands, send the CDB along with no data transfer
			status = HSCSI_Handle_CDB_No_Data(p_context);
	}
	
	return status;
	
} // HSCSI_Handle_CDB_Command

/*************************************************************************/
// HSCSI_Handle_CDB_No_Data
// This CDB has no data to send or receive.
// Turn the CDB into a I2O_SCSI_SCB_EXECUTE_MESSAGE and send it off to be
// executed by another driver.
/*************************************************************************
STATUS HSCSI_Handle_CDB_No_Data(HSCSI_EVENT_CONTEXT *p_context)
{
	STATUS							 status;
	
	HSCSI_TRACE_ENTRY(HSCSI_Handle_CDB_No_Data);
	
    // Set flags to send with CTIO if I2O request is successful.
    // Set action to perform when I2O response is received
    p_context->CTIO_flags = ISP_CTIO_FLAGS_NO_DATA_XFR;
    
    status = HSCSI_Message_Send_Request(p_context, 
    	TARGET_ACTION_HANDLE_I2O_RESPONSE);

#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_SENT_MESSAGE;
#endif
    
    if (status != NU_SUCCESS)
	{
		// We were not able to send the I2O message.  Return check condition.
		status = HSCSI_Send_Check_Status(p_context, HSCSI_CHECK_I2O);
		return status;
	}
    	
    return status;
    
	// When I2O response is received, HSCSI_Handle_I2O_Response
	// will be called by HSCSI_Event_Task.

} // HSCSI_Handle_CDB_No_Data
*/

/*************************************************************************/
// HSCSI_Handle_CDB_Read_Command
// Turn the CDB into a I2O_SCSI_SCB_EXECUTE_MESSAGE and send it off to be
// executed by another driver.
/*************************************************************************/
STATUS HSCSI_Handle_CDB_Read_Command(HSCSI_EVENT_CONTEXT *p_context,
	UNSIGNED transfer_length)
{
	void							*p_buffer;
	STATUS							 status;
	
	HSCSI_TRACE_ENTRY(HSCSI_Handle_CDB_Read_Command);
	
    // Allocate buffer for the I/O operation.
    status = HSCSI_Buffer_Allocate(p_context->Id, transfer_length, &p_buffer);
	if (status != NU_SUCCESS)
	{
		// No buffer could be allocated.  Return check condition.
		// Remember check condition for sense request.
		status = HSCSI_Send_Check_Status(p_context, HSCSI_CHECK_NO_BUFFER);
		return status;
	}
    
    // Save the buffer pointer so it can be deallocated later
    p_context->buffer = p_buffer;
    
    // Add an entry to the I2O SGL for this buffer
    HSCSI_I2O_Add_SGL_Entry(p_context->message, p_buffer, transfer_length, 
    	0 /* index */);
        
    // Create a DATA_SEGMENT_DESCRIPTOR from the SGL in the
    // I2O request so that we will have it later for the CTIO. 
    HSCSI_I2O_Get_SGL_Data_Segment(p_context->message, 
    	&p_context->data_segment, 
    	0 /* index */);

    // Set flags to send with CTIO if I2O request is successful.
    p_context->CTIO_flags = ISP_CTIO_FLAGS_DATA_IN;
    
    // Send the read request to be served.
    // Set action to perform when I2O response is received
    status = HSCSI_Message_Send_Request(p_context,
    	TARGET_ACTION_HANDLE_I2O_RESPONSE);
    if (status != NU_SUCCESS)
	{
		// We were not able to send the I2O message.  Return check condition.
		status = HSCSI_Send_Check_Status(p_context, HSCSI_CHECK_I2O);
		return status;
	}
	
#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_SENT_MESSAGE;
#endif
    
	return status;
	
	// When I2O response is received, HSCSI_Handle_I2O_Response
	// will be called by HSCSI_Event_Task.
	
} // HSCSI_Handle_CDB_Read_Command


/*************************************************************************/
// HSCSI_Handle_CDB_Write_Command
/*************************************************************************/
STATUS HSCSI_Handle_CDB_Write_Command(HSCSI_EVENT_CONTEXT *p_context, 
	UNSIGNED transfer_length)
{
	void							*p_buffer;
	STATUS							 status;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Handle_CDB_Write_Command);
	
    // Allocate buffer for the I/O operation.
    status = HSCSI_Buffer_Allocate(p_context->Id, transfer_length, &p_buffer);
	if (status != NU_SUCCESS)
	{
		// No buffer could be allocated.  Return check condition.
		// Remember check condition for sense request.
		status = HSCSI_Send_Check_Status(p_context, HSCSI_CHECK_NO_BUFFER);
		return status;
	}
    
    // Save the buffer pointer so it can be deallocated later
    p_context->buffer = p_buffer;
    
    // Add an entry to the SGL of the I2O request for this buffer
    HSCSI_I2O_Add_SGL_Entry(p_context->message, p_buffer, transfer_length, 
    	0 /* index */);
        
    // Create a DATA_SEGMENT_DESCRIPTOR from the SGL in the
    // I2O request so that we can use it later in the CTIO. 
    HSCSI_I2O_Get_SGL_Data_Segment(p_context->message, &p_context->data_segment, 
    	0 /* index */);
    
    // Send CTIO to the firmware to transfer the data to be written 
    // from the initiator into our buffer.
	status = HSCSI_Send_CTIO(p_context,
    		// action to perform when response to CTIO is received
    		TARGET_ACTION_HANDLE_CTIO_WRITE,
    		SCSI_STATUS_GOOD,
			ISP_CTIO_FLAGS_FAST_POST	// flags
			| ISP_CTIO_FLAGS_DATA_OUT); 
		
#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_CTIO_WRITE_SENT;
#endif
    
	return status;
	
	// When CTIO response is received, HSCSI_Handle_CTIO_Write
	// will be called by HSCSI_Event_Task.
	
} // HSCSI_Handle_CDB_Write_Command
	
/*************************************************************************/
// HSCSI_Handle_CTIO_Final
// Called by HSCSI_Event_Task when CTIO response is received from the host.
// This was the final CTIO received after the status was sent to the host.
/*************************************************************************/
STATUS	HSCSI_Handle_CTIO_Final(HSCSI_EVENT_CONTEXT *p_context)
{
	STATUS							 status;
	PIOCB_CTIO_TYPE_2				 p_ctio;

 	HSCSI_TRACE_ENTRY(HSCSI_Handle_CTIO_Final);

#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_DONE;  // may never see this one
#endif
    
	p_ctio = (PIOCB_CTIO_TYPE_2) &p_context->status_iocb;
	
    HSCSI_DUMP_HEX(TRACE_L8, "\n\rCTIO response IOCB", (U8 *)p_ctio, sizeof(IOCB_CTIO_TYPE_2));
    
	// Check the status of the command
	switch(p_ctio->status & STATUS_MASK)
	{
		case STATUS_REQUEST_COMPLETE:
		case STATUS_COMP_WITH_ERROR:
			break;						// these status values are OK
		
		case STATUS_REQUEST_ABORTED:
			// Host aborted the command
			// TODO:
			// Handle Aborts? In this case just throw away buffers...
			HSCSI_PRINT_STRING(TRACE_L1, "\n\rCTIO Final Host Abort");
			break;
			
		case STATUS_LIP_RESET:
			// Host sent LIP Reset
			// TODO:
			// Handle LIP Reset? In this case just throw away buffers...
			HSCSI_PRINT_STRING(TRACE_L1, "\n\rCTIO Final LIP Reset");
			break;
			
		case STATUS_PORT_LOGGED_OUT:
			// Host Loggeed out before we were done
			// TODO:
			// In this case just throw away buffers...
			HSCSI_PRINT_STRING(TRACE_L1, "\n\rCTIO Final Port Logged out");
			break;
			
		default:
			// TODO:
			// Need to recover from errors here
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_WARNING,
				"HSCSI_Handle_CTIO_Final", 
				"bad return status",
				0,
				p_ctio->status);
			break;
	}
	
#if defined(HSCSI_DEBUG)    
	// DEBUG
	p_context->Id->CmdsExecuting--;
	p_context->Id->LastDone = (void *) p_context;
#endif
	
	// Deallocate the buffer if it was used
	if (p_context->buffer)
		status = HSCSI_Buffer_Deallocate(p_context->Id, p_context->buffer);
	
	// Deallocate the message
	if (p_context->message)
		HSCSI_Free_Message(p_context->message);
	
    // Deallocate the HSCSI_EVENT_CONTEXT 
    // allocated by HSCSI_ISR_Response_Queue
    status = NU_Deallocate_Partition(p_context);
    if (status != NU_SUCCESS)
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Handle_CTIO_Final", 
			"NU_Deallocate_Partition for context failed",
			status,
			(UNSIGNED)p_context);
		
	return status;
		
} // HSCSI_Handle_CTIO_Final


/*************************************************************************/
// HSCSI_Handle_CTIO_Write
// Called by HSCSI_Event_Task when CTIO response is received.
/*************************************************************************/
STATUS	HSCSI_Handle_CTIO_Write(HSCSI_EVENT_CONTEXT *p_context)
{
	STATUS							 status;
	PIOCB_CTIO_TYPE_2				 p_ctio;
    
 	HSCSI_TRACE_ENTRY(HSCSI_Handle_CTIO_Write);

#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_CTIO_WRITE_RESP_IN;
#endif
    
	p_ctio = (PIOCB_CTIO_TYPE_2) &p_context->status_iocb;
	
    HSCSI_DUMP_HEX(TRACE_L8, "\n\rCTIO response IOCB (WRITE)",
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
			HSCSI_PRINT_STRING(TRACE_L1, "\n\rCTIO Write Host Abort");
			// SCSI request was not successful.
			status = HSCSI_Send_CTIO_Check(p_context, 
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
			HSCSI_PRINT_STRING(TRACE_L1, "\n\rCTIO Write LIP Reset");
			// SCSI request was not successful.
			status = HSCSI_Send_CTIO_Check(p_context, 
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
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_Handle_CTIO_Write", 
				"bad return status",
				0,
				p_ctio->status);
			break;
	}
	
    // Set flags to send with CTIO if I2O request is successful.
    p_context->CTIO_flags = ISP_CTIO_FLAGS_NO_DATA_XFR;  // just return status
    
	// Send the I2O request to be served
    // Set action to perform when I2O response is received
    status = HSCSI_Message_Send_Request(p_context, 
    	TARGET_ACTION_HANDLE_I2O_RESPONSE);
    if (status != NU_SUCCESS)
	{
		// We were not able to send the I2O message.  Return check condition.
		status = HSCSI_Send_Check_Status(p_context, HSCSI_CHECK_I2O);
		return status;
	}
	
#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_SENT_MESSAGE;
#endif
    
	return status;
    	
	// When I2O response is received, HSCSI_Handle_I2O_Response
	// will be called by HSCSI_Event_Task.
	
} // HSCSI_Handle_CTIO_Write

/*************************************************************************/
// HSCSI_Handle_I2O_Response
// Called by HSCSI_Event_Task when I2O response message is received.
// Check the DetailedStatusCode for errors, return check status if any.
/*************************************************************************/
STATUS HSCSI_Handle_I2O_Response(HSCSI_EVENT_CONTEXT *p_context)
{	
	STATUS				 status;
	STATUS				 DetailedStatusCode = 
					HSCSI_I2O_Get_DetailedStatusCode(p_context->message);

 	HSCSI_TRACE_ENTRY(HSCSI_Handle_I2O_Response);

#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_REPLY_RECEIVED;
#endif
    
	// Check to see if the request was successful
	if ((DetailedStatusCode &
					HSCSI_SCSI_DEVICE_DSC_MASK) != HSCSI_SCSI_DSC_CHECK_CONDITION)
	{
		if (DetailedStatusCode != HSCSI_SCSI_DSC_SUCCESS)
		{
			// failed last command, send check status back
			REQUEST_SENSE	cmd_failed_sense = {RESPONSE_CODE, 0,
									SENSE_HARDWARE_ERROR, 0,0,0,0,
									ADDITIONAL_LENGTH, 0,0,0,0,
									ASC_LOGICAL_UNIT_DOES_NOT_RESPOND_TO_SELECTION,
									0,0,0,0,0 };
									
			HSCSI_PRINT_HEX(TRACE_L1, "\n\rI2O Response Bad ",
							DetailedStatusCode);
			
			// SCSI request was not successful.
			status = HSCSI_Send_CTIO_Check(p_context, 
	    		TARGET_ACTION_HANDLE_CTIO_FINAL,
	    		(U8 *) &cmd_failed_sense,
	    		sizeof(REQUEST_SENSE),
				  ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
				| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
				| ISP_CTIO_FLAGS_NO_DATA_XFR);
			
		}
		else
			// SCSI request was successful
			status = HSCSI_Send_CTIO(p_context, 
	    		TARGET_ACTION_HANDLE_CTIO_FINAL,
	    		SCSI_STATUS_GOOD,    
				  ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
				| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
				| ISP_CTIO_FLAGS_FAST_POST
				| p_context->CTIO_flags);
	} 
	else
	{
		SCB_REPLY_PAYLOAD	*pP =
			(SCB_REPLY_PAYLOAD *)HSCSI_I2O_Get_Payload(p_context->message);
						
		// SCSI request was not successful.
		status = HSCSI_Send_CTIO_Check(p_context, 
    		TARGET_ACTION_HANDLE_CTIO_FINAL,
    		(U8 *) &pP->SenseData[0],
    		pP->AutoSenseTransferCount,
			  ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
			| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
			| ISP_CTIO_FLAGS_NO_DATA_XFR);
	}

#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_CTIO_SENT;
#endif
    
    return status;
    
} // HSCSI_Handle_I2O_Response

/*************************************************************************/
// HSCSI_Send_Check_Status
// The command failed to execute due to an error encountered in the driver.
// Return check status to host.
/*************************************************************************/
STATUS HSCSI_Send_Check_Status(HSCSI_EVENT_CONTEXT *p_context, 
	HSCSI_CHECK_CONDITION check_condition)
{
	STATUS							 status;

 	HSCSI_TRACE_ENTRY(HSCSI_Send_Check_Status);
	
	// Save check_condition so that we will have it when the
	// host asks for sense information.
	HSCSI_Check_Save(BYTE_SWAP16(p_context->iocb.LUN), p_context->iocb.initiator_ID,
		check_condition);
	
    // Send Continue Target I/O with check condition 
	status = HSCSI_Send_CTIO(p_context, 
    	TARGET_ACTION_HANDLE_CTIO_FINAL,
    	SCSI_STATUS_CHECK,    
		  ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
		| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
		| ISP_CTIO_FLAGS_FAST_POST
		| ISP_CTIO_FLAGS_NO_DATA_XFR); 

#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_CTIO_ERROR_SENT;
#endif
    
	return status;
	    
} // HSCSI_Send_Check_Status

/*************************************************************************/
// HSCSI_Send_CTIO
// Send Continue Target I/O to ISP
/*************************************************************************/
STATUS HSCSI_Send_CTIO(HSCSI_EVENT_CONTEXT *p_context, 
	HSCSI_EVENT_ACTION next_action, 
	U8 SCSI_status, UNSIGNED flags)
{
	IOCB_CTIO_MODE_0 					*p_CTIO;
	STATUS								 status;
    
 	HSCSI_TRACE_ENTRY(HSCSI_Send_CTIO);
	
    // Create Continue Target I/O IOCB in ISP request FIFO
    // Get pointer to next IOCB in request FIFO.
	p_CTIO = (IOCB_CTIO_MODE_0*)HSCSI_Request_FIFO_Get_Pointer(p_context->Id);
	
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
		SCB_REPLY_PAYLOAD	*pP =
			(SCB_REPLY_PAYLOAD *)HSCSI_I2O_Get_Payload(p_context->message);
							    
    	HSCSI_PRINT_HEX(TRACE_L8, "\n\rSend CTIO: actual Xfer Len ", pP->TransferCount);
    
	    // figure the resid length
	    p_CTIO->residual_transfer_length = BYTE_SWAP32(p_context->iocb.transfer_length) -
	    				pP->TransferCount;
		p_CTIO->residual_transfer_length = BYTE_SWAP32(p_CTIO->residual_transfer_length);  // fix endian
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
    
    HSCSI_DUMP_HEX(TRACE_L8, "\n\rCTIO MODE 0 IOCB", (U8 *)p_CTIO, sizeof(IOCB_CTIO_MODE_0));
    
    // Send Continue Target I/O IOCB to the ISP.
    // next_action specifies action to perform by HSCSI_Event_Task
    // when command completes.
    status = HSCSI_Send_Command_IOCB(p_context, next_action,
    	(IOCB_COMMAND*)p_CTIO); 
    
#if defined(HSCSI_DEBUG)    
    p_context->ctio_cnt++;
#endif
    
    return status;    
    
	// When I2O response is received, HSCSI_Event_Task will call
	// the next method.
	
} // HSCSI_Send_CTIO

/*************************************************************************/
// HSCSI_Send_CTIO_Check
// Send Continue Target I/O with sense data to ISP
// Data can not be sent with this mode
/*************************************************************************/
STATUS HSCSI_Send_CTIO_Check(HSCSI_EVENT_CONTEXT *p_context, 
	HSCSI_EVENT_ACTION next_action, U8 *SCSI_sense,
	UNSIGNED sense_length, UNSIGNED flags)
{
	IOCB_CTIO_MODE_1 			*p_CTIO;
	STATUS						 status;
	SCB_REPLY_PAYLOAD			*pP =
			(SCB_REPLY_PAYLOAD *)HSCSI_I2O_Get_Payload(p_context->message);
    
 	HSCSI_TRACE_ENTRY(HSCSI_Send_CTIO_Check);
	
    // Create Continue Target I/O IOCB in ISP request FIFO
    // Get pointer to next IOCB in request FIFO.
	p_CTIO = (IOCB_CTIO_MODE_1*)HSCSI_Request_FIFO_Get_Pointer(p_context->Id);
	
	// Initialize CTIO fields
    p_CTIO->entry_type = IOCB_TYPE_CONTINUE_TARGET_IO_TYPE_2;
    p_CTIO->entry_count = 1;
    //p_CTIO->LUN = p_context->iocb.LUN;
    p_CTIO->initiator_ID = p_context->iocb.initiator_ID;
    p_CTIO->RX_ID = p_context->iocb.RX_ID;  
    p_CTIO->timeout = BYTE_SWAP16(30);
    
    // figure the resid length
    p_CTIO->residual_transfer_length = BYTE_SWAP32(p_context->iocb.transfer_length) -
    				pP->TransferCount;
	p_CTIO->residual_transfer_length = BYTE_SWAP32(p_CTIO->residual_transfer_length);  // fix endian
	
	// Set SCSI Check status in reply.
    p_CTIO->SCSI_status = BYTE_SWAP16((U16)
    		(SCSI_STATUS_CHECK | IOCB_SCSI_FLAGS_SENSE_VALID));

    p_CTIO->flags = BYTE_SWAP16(flags + 1);		// CTIO Mode 1
    	
    // Copy the sense data into the CTIO
	Mem_Copy( (U8 *) &p_CTIO->response_information[0],
				(U8 *) SCSI_sense,
				(U32)sense_length);
				
	p_CTIO->sense_length = BYTE_SWAP16(sense_length);

    HSCSI_DUMP_HEX(TRACE_L8, "\n\rCTIO MODE 1 IOCB", (U8 *)p_CTIO, sizeof(IOCB_CTIO_MODE_1));
    
    // Send Continue Target I/O IOCB to the ISP.
    // next_action specifies action to perform by HSCSI_Event_Task
    // when command completes.
    status = HSCSI_Send_Command_IOCB(p_context, next_action,
    	(IOCB_COMMAND*)p_CTIO); 
       
#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_CTIO_CHECK_SENT;
#endif
    
    return status;    
    
	// When I2O response is received, HSCSI_Event_Task will call
	// the next method.
	
} // HSCSI_Send_CTIO_Check

/*************************************************************************/
// HSCSI_Send_Driver_Sense_Data
// We have sense data for this initiator and this LUN, so send it.
/*************************************************************************/
STATUS HSCSI_Send_Driver_Sense_Data(HSCSI_EVENT_CONTEXT *p_context)
{
	STATUS							 status;
	U32								 transfer_length;

 	HSCSI_TRACE_ENTRY(HSCSI_Send_Driver_Sense_Data);
	
	// Create sense data record to send
	HSCSI_Check_Create_Sense(BYTE_SWAP16(p_context->iocb.LUN), 
		p_context->iocb.initiator_ID, &p_context->sense_data);
	
	// Set up data segment descriptor to point to sense_data
	//p_context->data_segment.Address = ((UNSIGNED)&p_context->sense_data  & ~KSEG1);
	p_context->data_segment.Address = (UNSIGNED)HSCSI_Get_DMA_Address(&p_context->sense_data);
	p_context->data_segment.Address = BYTE_SWAP32(p_context->data_segment.Address);
	
	// Get transfer_length from CDB
	transfer_length = CDB_Get_Transfer_Length(&p_context->iocb.CDB);
	if (transfer_length > sizeof(REQUEST_SENSE))
		transfer_length = sizeof(REQUEST_SENSE);
	p_context->data_segment.Length = BYTE_SWAP32(transfer_length);
	
	// Send sense data
	status = HSCSI_Send_CTIO(p_context, 
    	TARGET_ACTION_HANDLE_CTIO_FINAL,    
    	SCSI_STATUS_GOOD,
		ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
		| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
		| ISP_CTIO_FLAGS_FAST_POST
		| ISP_CTIO_FLAGS_DATA_IN); 
	
#if defined(HSCSI_DEBUG)    
    p_context->flags |= EV_CTIO_SENSE_SENT;
#endif
    
	return status;
	
} // HSCSI_Send_Driver_Sense_Data

