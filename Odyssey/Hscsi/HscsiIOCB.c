/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiIOCB.c
// 
// Description:
// This file implements interfaces for for the QLogic I/O control block.
// Commands that send IOCBs are implemented here for organizational
// completeness.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiIOCB.c $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#include "HscsiCommon.h"
#include "HscsiIOCB.h"
#include "HscsiMailbox.h"
#include "HscsiRequestFIFO.h"
#include "HscsiResponseFIFO.h"

/*************************************************************************/
// HSCSI_IOCB_Create
// Create HSCSI_IOCB object
/*************************************************************************/
STATUS HSCSI_IOCB_Create(PHSCSI_INSTANCE_DATA Id)
{
	STATUS			 status = NU_SUCCESS;

 	HSCSI_TRACE_ENTRY(HSCSI_IOCB_Create);
			
	return status;
	
} // HSCSI_IOCB_Create

/*************************************************************************/
// HSCSI_IOCB_Destroy
// Destroy HSCSI_IOCB object
/*************************************************************************/
void HSCSI_IOCB_Destroy()
{
 	HSCSI_TRACE_ENTRY(HSCSI_IOCB_Destroy);
			
} // HSCSI_IOCB_Destroy
    	
/*************************************************************************/
// HSCSI_Send_Command_IOCB
// Send IOCB to the ISP. 
/*************************************************************************/
STATUS HSCSI_Send_Command_IOCB(
	HSCSI_EVENT_CONTEXT *p_context, 
	HSCSI_EVENT_ACTION next_action,
	HSCSI_IOCB_COMMAND *p_command_IOCB)
{
	STATUS				 status = NU_SUCCESS;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Send_Command_IOCB);

    // next_action specifies action to perform by HSCSI_Event_Task
    // when command completes.
    p_context->action = next_action;
    
	// Save pointer to context in command IOCB.
	// A message will be sent to the HSCSI_event_queue 
	// when the command completes.
	p_command_IOCB->system_defined2 = (UNSIGNED)p_context;
	
	HSCSI_DUMP_HEX(TRACE_L8, "\n\rCommand IOCB Type 2", (U8 *)p_command_IOCB, sizeof(HSCSI_IOCB_COMMAND));
	//HSCSI_PRINT_HEX(TRACE_L2, "\n\rCmd IOCB context = ", (U32)p_context);
		
	// Update Request_FIFO_Index.  This lets the ISP know that a new
	// request is ready to execute.  
	HSCSI_Request_FIFO_Update_Index(p_context->Id);
	
	return status;
				
} // HSCSI_Send_Command_IOCB

/**************************************************************************/
// HSCSI_Send_IOCB
//
// IOCB that bypasses the request queue (for testing/debugging)
/**************************************************************************/
STATUS HSCSI_Send_IOCB( // Prototype in HscsiProto.h
	HSCSI_EVENT_CONTEXT *p_context,
	HSCSI_EVENT_ACTION next_action,
	HSCSI_IOCB_COMMAND *p_command_IOCB)
{
	STATUS	status = NU_SUCCESS;
	HSCSI_TRACE_ENTRY(HSCSI_Send_IOCB);
	p_context->action = next_action;
	p_command_IOCB->system_defined2 = (UNSIGNED)p_context;
	HSCSI_DUMP_HEX(TRACE_L8, "\n\r Execute IOCB", (U8 *)p_command_IOCB,
		sizeof(HSCSI_IOCB_COMMAND));
	HSCSI_Execute_IOCB(p_context->Id);
	
	return status;
} // HSCSI_Send_IOCB
