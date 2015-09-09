/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpIOCB.c
// 
// Description:
// This file implements interfaces for for the QLogic I/O control block.
// Commands that send IOCBs are implemented here for organizational
// completeness.
// 
// Update Log
//	$Log: /Gemini/Odyssey/FCP/FcpIOCB.c $
// 
// 7     6/18/99 6:45p Mpanas
// Initial ISP2200 support
//
// 4/14/98 Jim Frandeen: Create file
// 5/5/98 Jim Frandeen: Use C++ comment style
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 10/6/98 Michael G. Panas: changes to support interrupt driven mailbox
// and iocb commands
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/

#include "FcpCommon.h"
#include "FcpIOCB.h"
#include "FcpMailbox.h"
#include "FcpRequestFIFO.h"
#include "FcpResponseFIFO.h"

/*************************************************************************/
// FCP_IOCB_Create
// Create FCP_IOCB object
/*************************************************************************/
STATUS FCP_IOCB_Create(PINSTANCE_DATA Id)
{
	STATUS			 status = NU_SUCCESS;

 	FCP_TRACE_ENTRY(FCP_IOCB_Create);
			
	return status;
	
} // FCP_IOCB_Create

/*************************************************************************/
// FCP_IOCB_Destroy
// Destroy FCP_IOCB object
/*************************************************************************/
void FCP_IOCB_Destroy()
{
 	FCP_TRACE_ENTRY(FCP_IOCB_Destroy);
			
} // FCP_IOCB_Destroy


/*************************************************************************/
// FCP_Enable_LUNs
// Rewritten to handle 16 bit LUNs sends only one ENABLE_LUN IOCB
/*************************************************************************/
STATUS FCP_Enable_LUNs(PINSTANCE_DATA Id)
{
 	IOCB_ENABLE_LUN		*p_command;
 	STATUS				 status = NU_SUCCESS;
    
 	FCP_TRACE_ENTRY(FCP_Enable_LUNs);
 	
	// Check to see if this driver is running in target mode
	if (Id->FCP_config.enable_target_mode == 0)
		return status;
		
    // Get pointer to next IOCB in request FIFO.
	p_command = (IOCB_ENABLE_LUN*)FCP_Request_FIFO_Get_Pointer(Id);

    // Build enable LUN queue entry 
    p_command->entry_type = IOCB_TYPE_ENABLE_LUN;	// entry type 
    p_command->entry_count= 0x01;        			//entry count (always 1) 
    p_command->command_count = COMMAND_COUNT *
    									Id->FCP_config.num_LUNs;
    p_command->immediate_notify_count = IMMEDIATE_NOTIFY_COUNT *
    									Id->FCP_config.num_LUNs;
    p_command->status = 0;
    
    // for 2200, we need to set the correct ID
    if (Id->ISP_Type == 0x00002200)
    	p_command->target_ID = 2;	// Hack for test
    
    // Send Command IOCB request to the ISP.
	// Update Request_FIFO_Index.  This lets the ISP know that a new
	// request is ready to execute.  
	FCP_Request_FIFO_Update_Index(Id);
	
	return status;
	
} // FCP_Enable_LUNs
    	

/*************************************************************************/
// FCP_Send_Command_IOCB
// Send IOCB to the ISP. 
/*************************************************************************/
STATUS FCP_Send_Command_IOCB(
	FCP_EVENT_CONTEXT *p_context, 
	FCP_EVENT_ACTION next_action,
	IOCB_COMMAND_TYPE2 *p_command_IOCB)
{
	STATUS				 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Send_Command_IOCB);

    // next_action specifies action to perform by FCP_Event_Task
    // when command completes.
    p_context->action = next_action;
    
	// Save pointer to context in command IOCB.
	// A message will be sent to the FCP_event_queue 
	// when the command completes.
	p_command_IOCB->system_defined2 = (UNSIGNED)p_context;
	
	FCP_DUMP_HEX(TRACE_L8, "\n\rCommand IOCB Type 2", (U8 *)p_command_IOCB, sizeof(IOCB_COMMAND_TYPE2));
	//FCP_PRINT_HEX(TRACE_L2, "\n\rCmd IOCB context = ", (U32)p_context);
		
	// Update Request_FIFO_Index.  This lets the ISP know that a new
	// request is ready to execute.  
	FCP_Request_FIFO_Update_Index(p_context->Id);
	
	return status;
				
} // FCP_Send_Command_IOCB


