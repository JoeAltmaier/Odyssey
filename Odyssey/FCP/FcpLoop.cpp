/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpLoop.cpp
// 
// Description:
// This module contains all the external loop control interfaces to the
// FCP driver.  Also included are several DDM / Loop support rotines.
//
// This module depends on the QLogic ISP2100/2200 chip
// 
// Update Log:
//	$Log: /Gemini/Odyssey/FCP/FcpLoop.cpp $
// 
// 5     12/20/99 8:03p Jtaylor
// Use bcopy and bzero
// 
// 4     8/13/99 4:35p Mpanas
// Remove call pointer (defined elsewhere)
// 
// 3     8/12/99 9:19p Mpanas
// Add user context pointer for callbacks
// 
// 2     6/18/99 6:45p Mpanas
// Initial ISP2200 support
// 
// 1     6/06/99 4:14p Mpanas
// New Loop control files
//
// 06/02/99 Michael G. Panas: Create file
/*************************************************************************/

#include "FcpCommon.h"
#include "Fcp.h"

#include "FcpDebug.h"
#include "FcpEvent.h"
#include "FcpISP.h"
#include "FcpISR.h"
#include "FcpMailbox.h"
#include "FcpIOCB.h"
#include "FcpMemory.h"
#include "FcpRequestFIFO.h"
#include "FcpResponseFIFO.h"
#include "FcpMessage.h"
#include "FcpString.h"

#include "FcpLoop.h"

/*************************************************************************/
//    Forward References
/*************************************************************************/
extern "C" {
void Ext_LM_Handle_Callback(void *pCtx, U32 status);
}

/*************************************************************************/
// Global references
/*************************************************************************/
U32		FCP_Loop_ID;
U32		FCP_Loop_ALPA;
U32		FCP_Port_ID;

/*************************************************************************/
// FCP_Get_Loop_ID
// Get the Loop ID, AL_PA and Port ID (works for 2100 only)
/*************************************************************************/
STATUS FCP_Get_Loop_ID(PINSTANCE_DATA Id)
{
	UNSIGNED	mailbox_status;
	STATUS		status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Get_Loop_ID);
	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
 	// get loop ID command
    Write_ISP(Id, ISP_MAILBOX_0, MBC_GET_LOOP_ID); 
    	
    // clear the possible VP index
    Write_ISP(Id, ISP_MAILBOX_1, 0); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Get_Loop_ID", 
			"Command Failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return mailbox_status;
	}
	
	// not re-entrant
	FCP_Loop_ID = Id->FCP_mailbox_message.mailbox[1];
	FCP_Loop_ALPA = Id->FCP_mailbox_message.mailbox[2];
	FCP_Port_ID = Id->FCP_mailbox_message.mailbox[3];

    return MB_STS_GOOD;
    
} // FCP_Get_Loop_ID


/*************************************************************************/
// FCP_Get_FC_AL_Position_Map
// Get the FC_AL Position map to the address in Address.  Address must
// point to a area at least 128 bytes long to hold the map.
/*************************************************************************/
STATUS FCP_Get_FC_AL_Position_Map(PINSTANCE_DATA Id, U8 *Address, U16 *map_type)
{
	U16					 mailbox_status;
	STATUS		 		 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Get_FC_AL_Position_Map);
 	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    	
	FCP_PRINT_HEX(TRACE_L8, "\n\rAddress of buffer = ", (UNSIGNED)Address);
	 
 	// Get Position Map command
    Write_ISP(Id, ISP_MAILBOX_0, MBC_GET_FC_AL_MAP);		// 64 bit read
    	
    // Source address, bits 31-16
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)FCP_Get_DMA_Address((void *)Address) >>16)); 
    	
    // Source address, bits 15-0 
    Write_ISP(Id, ISP_MAILBOX_3, (U32)FCP_Get_DMA_Address((void *)Address) & 0xffff); 

    // Map address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Map address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Get_FC_AL_Position_Map", 
			" failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return mailbox_status;
	}

	// get the postition map type, 0 = LIRP map, 1 = FW gen map
	*map_type = Id->FCP_mailbox_message.mailbox[1];
	
    return MB_STS_GOOD;
	
} // FCP_Get_FC_AL_Position_Map


/*************************************************************************/
// FCP_Get_Port_Data_Base
// Get the port database for the ID passed in Loop_ID to the address in
// Address.  Address must point to a area at least 128 bytes long to hold
// the data.  VP_Index must be 0 for 2100 requests
/*************************************************************************/
STATUS FCP_Get_Port_Data_Base(PINSTANCE_DATA Id, U8 *Address,
					U16 Loop_ID, U8 VP_Index)
{
	U16					 mailbox_status;
	STATUS		 		 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Get_Port_Data_Base);
 	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    	
	FCP_PRINT_HEX(TRACE_L8, "\n\rAddress of buffer = ", (UNSIGNED)Address);
	 
 	// Get the Port DataBase
    Write_ISP(Id, ISP_MAILBOX_0, MBC_GET_PORT_DB);		// 64 bit read
    	
    // Loop ID & VP_Index
    Write_ISP(Id, ISP_MAILBOX_1, ((Loop_ID & 0xff) << 8) | (VP_Index & 0xff)); 
    	
    // Source address, bits 31-16
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)FCP_Get_DMA_Address((void *)Address) >>16)); 
    	
    // Source address, bits 15-0 
    Write_ISP(Id, ISP_MAILBOX_3, (U32)FCP_Get_DMA_Address((void *)Address) & 0xffff); 

    // Map address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Map address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Get_Port_Data_Base", 
			" failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return mailbox_status;
	}

    return MB_STS_GOOD;
	
} // FCP_Get_Port_Data_Base


/*************************************************************************/
// FCP_Get_Firmware_State
// Get the ISP firmware state.  The state is a 16 bit value.
/*************************************************************************/
STATUS FCP_Get_Firmware_State(PINSTANCE_DATA Id, U16 *state)
{
	UNSIGNED	mailbox_status;
	STATUS		status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Get_Firmware_State);
	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
 	// get Firmware State command
    Write_ISP(Id, ISP_MAILBOX_0, MBC_GET_FW_STATE); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Get_Firmware_State", 
			"Command Failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return mailbox_status;
	}
	
	*state = Id->FCP_mailbox_message.mailbox[1];

    return MB_STS_GOOD;
    
} // FCP_Get_Firmware_State

/*************************************************************************/
// FCP_Get_Port_Name
// Get the Port Name to the address in Address.  Address must
// point to a area at least 10 bytes long.
/*************************************************************************/
STATUS FCP_Get_Port_Name(PINSTANCE_DATA Id, U8 *Address,
						U8 Loop_ID, U8 VP_Index, U8 Options)
{
	U16					 mailbox_status;
	U16					 reg;
	U16					 flags;
	STATUS		 		 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Get_Port_Name);
 	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    	
	FCP_PRINT_HEX(TRACE_L8, "\n\rAddress of buffer = ", (UNSIGNED)Address);
	 
 	// Get Port Name command
    Write_ISP(Id, ISP_MAILBOX_0, MBC_GET_PORT_NAME);		// 64 bit read
    	
    // Loop ID
    Write_ISP(Id, ISP_MAILBOX_1, ((Loop_ID & 0xff) << 8) | Options); 
    	
    // VP index
    Write_ISP(Id, ISP_MAILBOX_2, (U16)VP_Index); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Get_Port_Name", 
			" failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return mailbox_status;
	}

	// get the port database and option flags
	flags = Id->FCP_mailbox_message.mailbox[1];
	Address[8] = reg & 0xff;
	Address[9] = ((reg & 0xff) >> 8);

	reg = Id->FCP_mailbox_message.mailbox[2];
	Address[0] = reg & 0xff;
	Address[1] = ((reg & 0xff) >> 8);
	reg = Id->FCP_mailbox_message.mailbox[3];
	Address[2] = reg & 0xff;
	Address[3] = ((reg & 0xff) >> 8);
	
	// not read
	reg = Read_ISP(Id, ISP_MAILBOX_6);
	Address[4] = reg & 0xff;
	Address[5] = ((reg & 0xff) >> 8);
	reg = Read_ISP(Id, ISP_MAILBOX_7);
	Address[6] = reg & 0xff;
	Address[7] = ((reg & 0xff) >> 8);
	
    return MB_STS_GOOD;
    
} // FCP_Get_Port_Name

/*************************************************************************/
// FCP_Initiate_LIP
// Startup the Fibre Channel by sending an LIP
/*************************************************************************/
STATUS FCP_Initiate_LIP(PINSTANCE_DATA Id)
{
	STATUS				 status = NU_SUCCESS;
	U16					 mailbox_status;
	
 	FCP_TRACE_ENTRY(FCP_Initiate_LIP);
	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
    Write_ISP(Id, ISP_MAILBOX_0, MBC_INITIATE_LIP);
    
	mailbox_status = FCP_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Initiate_LIP", 
			"initiate LIP failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return mailbox_status;
	}
	
	return MB_STS_GOOD;
	
} // FCP_Initiate_LIP

/*************************************************************************/
// FCP_Initiate_LIP_Reset
// Reset the Fibre Channel by sending an LIP Reset
/*************************************************************************/
STATUS FCP_Initiate_LIP_Reset(PINSTANCE_DATA Id, U8 Loop_ID, U16 Delay)
{
	STATUS				 status = NU_SUCCESS;
	U16					 mailbox_status;
	
 	FCP_TRACE_ENTRY(FCP_Initiate_LIP_Reset);
	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
    Write_ISP(Id, ISP_MAILBOX_0, MBC_INITIATE_LIP_RESET);
    
    // Loop ID
    Write_ISP(Id, ISP_MAILBOX_1, ((Loop_ID & 0xff) << 8)); 
    
    // Delay
    Write_ISP(Id, ISP_MAILBOX_2, Delay); 

	mailbox_status = FCP_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Initiate_LIP", 
			"initiate LIP failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return mailbox_status;
	}
	
	return MB_STS_GOOD;
	
} // FCP_Initiate_LIP_Reset

/*************************************************************************/
// FCP_Get_Link_Status
// Get the Link Status for the ID passed in Loop_ID and the Port passed in
// Loop_Port to the address in Address.  Address must point to a area 
// at least 18 bytes long to hold the data.
// Loop_Port can be: 0 = Status of the port receiving the RLS Frame,
// 1 = Link Status of port A of the device, 2 = Link Status of port B of
// the device
/*************************************************************************/
STATUS FCP_Get_Link_Status(PINSTANCE_DATA Id, U8 *Address,
					U8 Loop_ID, U8 Loop_Port)
{
	U16					 mailbox_status;
	STATUS		 		 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Get_Link_Status);
 	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    	
	FCP_PRINT_HEX(TRACE_L8, "\n\rAddress of buffer = ", (UNSIGNED)Address);
	 
 	// Get the Port DataBase
    Write_ISP(Id, ISP_MAILBOX_0, MBC_GET_LINK_STATUS);		// 64 bit read
    	
    // Loop ID
    Write_ISP(Id, ISP_MAILBOX_1, ((Loop_ID & 0xff) << 8) | (Loop_Port & 0xff)); 
    	
    // Source address, bits 31-16
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)FCP_Get_DMA_Address((void *)Address) >>16)); 
    	
    // Source address, bits 15-0 
    Write_ISP(Id, ISP_MAILBOX_3, (U32)FCP_Get_DMA_Address((void *)Address) & 0xffff); 

    // Map address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Map address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Get_Link_Status", 
			" failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return mailbox_status;
	}

    return MB_STS_GOOD;
	
} // FCP_Get_Link_Status


/*************************************************************************/
//	ISP2200 only Mailbox commands
/*************************************************************************/


/*************************************************************************/
// FCP_Get_VP_Database
// Get the entire Virtual Port Database to the address in Address.
// Address must point to a area at least 1152 bytes long to hold the data.
// All populated entries in the database will be returned.  This can be
// up to 32 etries long.
/*************************************************************************/
STATUS FCP_Get_VP_Database(PINSTANCE_DATA Id, U8 *Address, U16 *VP_Entry_Count)
{
	U16					 mailbox_status;
	STATUS		 		 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Get_VP_Database);
 	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    	
	FCP_PRINT_HEX(TRACE_L8, "\n\rAddress of buffer = ", (UNSIGNED)Address);
	 
 	// Get the VP DataBase
    Write_ISP(Id, ISP_MAILBOX_0, MBC_GET_VP_DATA);		// 64 bit read
    	
    // Source address, bits 31-16
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)FCP_Get_DMA_Address((void *)Address) >>16)); 
    	
    // Source address, bits 15-0 
    Write_ISP(Id, ISP_MAILBOX_3, (U32)FCP_Get_DMA_Address((void *)Address) & 0xffff); 

    // Map address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Map address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Get_VP_Database", 
			" failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return mailbox_status;
	}
	
	*VP_Entry_Count = Id->FCP_mailbox_message.mailbox[1];

    return MB_STS_GOOD;
	
} // FCP_Get_VP_Database


/*************************************************************************/
// FCP_Get_VP_Database_Entry
// Get a single Virtual Port Database entry to the address in Address.
// Address must point to a area at least 36 bytes long to hold the data.
/*************************************************************************/
STATUS FCP_Get_VP_Database_Entry(PINSTANCE_DATA Id, U8 *Address, U16 VP_Index)
{
	U16					 mailbox_status;
	STATUS		 		 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Get_VP_Database_Entry);
 	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    	
	FCP_PRINT_HEX(TRACE_L8, "\n\rAddress of buffer = ", (UNSIGNED)Address);
	 
 	// Get the VP DataBase Entry
    Write_ISP(Id, ISP_MAILBOX_0, MBC_GET_VP_DB_ENTRY);		// 64 bit read
    	
    // VP index
    Write_ISP(Id, ISP_MAILBOX_1, (U16)VP_Index); 
    	
    // Source address, bits 31-16
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)FCP_Get_DMA_Address((void *)Address) >>16)); 
    	
    // Source address, bits 15-0 
    Write_ISP(Id, ISP_MAILBOX_3, (U32)FCP_Get_DMA_Address((void *)Address) & 0xffff); 

    // Map address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Map address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Get_VP_Database_Entry", 
			" failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return mailbox_status;
	}
	
    return MB_STS_GOOD;
	
} // FCP_Get_VP_Database_Entry



/*************************************************************************/
//	ISP2200 only IOCB commands
/*************************************************************************/



/*************************************************************************/
// FCP_VP_Control
// Send IOCB to Enable or Disable Virtual Ports
// This command can be sent by Loop monitors.
// The value in *user is passed when the callback is called
/*************************************************************************/
STATUS FCP_VP_Control(PINSTANCE_DATA Id, 
			U16 VP_Count, U8 *VP_Indexes,
			U8 command, void *user)
{
 	IOCB_VP_CTL		*p_command;
	FCP_EVENT_CONTEXT *p_context;
 	STATUS			 status = NU_SUCCESS;
    
 	FCP_TRACE_ENTRY(FCP_VP_Control);
 	
	// Check to see if this driver is running in target mode
	if (Id->FCP_config.enable_target_mode == 0)
		return status;
		
    // Get pointer to next IOCB in request FIFO.
	p_command = (IOCB_VP_CTL*)FCP_Request_FIFO_Get_Pointer(Id);

    // Build VP Control queue entry 
    p_command->entry_type = IOCB_TYPE_VP_CTL;		// entry type 
    p_command->entry_count= 0x01;        			// entry count (always 1) 
    p_command->status = 0;
    
    // Fill in the VP Control fields
    p_command->command = command;
    p_command->vp_count = VP_Count;
    bcopy(
       	(char *)VP_Indexes,
    	(char *)&p_command->vp_index[0],
    	(int) VP_Count);
    
	// Allocate an FCP_EVENT_CONTEXT from the pool.
	status = NU_Allocate_Partition(&Id->FCP_event_context_pool, 
		(VOID**)&p_context, NU_NO_SUSPEND);
	if (status != NU_SUCCESS)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_VP_Control", 
			"NU_Allocate_Partition for event context failed",
			status,
			(UNSIGNED)Id);
		return status;
	}
	
	// Zero the context
	bzero((char *)p_context, sizeof(FCP_EVENT_CONTEXT));
	
	// Set the instance pointer
	p_context->Id = Id;
	
	// Save user context pointer
	p_context->message = user;
	
	// Set action to perform
	p_context->action = FCP_ACTION_HANDLE_LOOP_IOCB_COMPL;

	// Save pointer to context in command IOCB.
	// A message will be sent to the FCP_event_queue 
	// when the command completes.
	p_command->system_defined2 = (UNSIGNED)p_context;
	
    // Send Command IOCB request to the ISP.
	// Update Request_FIFO_Index.  This lets the ISP know that a new
	// request is ready to execute.  
	FCP_Request_FIFO_Update_Index(Id);
	
	return status;
	
} // FCP_VP_Control
    	

/*************************************************************************/
// FCP_Modify_VP_Config
// Send IOCB to modify Virtual Ports in the VP Database.  These VP entries
// must have already been configured by the IFWCB sent at init time.
// The value in *user is passed when the callback is called
/*************************************************************************/
STATUS FCP_Modify_VP_Config(PINSTANCE_DATA Id, 
			U16 VP_Count, U8 *VP_Indexes,
			VP_CONFIG *vp_config, U8 command,
			void *user)
{
 	IOCB_MODIFY_VP_CFG		*p_command;
	FCP_EVENT_CONTEXT		*p_context;
 	STATUS			 		 status = NU_SUCCESS;
    
 	FCP_TRACE_ENTRY(FCP_Modify_VP_Config);
 	
	// Check to see if this driver is running in target mode
	if (Id->FCP_config.enable_target_mode == 0)
		return status;
		
    // Get pointer to next IOCB in request FIFO.
	p_command = (IOCB_MODIFY_VP_CFG*)FCP_Request_FIFO_Get_Pointer(Id);

    // Build VP Control queue entry 
    p_command->entry_type = IOCB_TYPE_MODIFY_PORT_CFG;	// entry type 
    p_command->entry_count= 0x01;        			// entry count (always 1) 
    p_command->status = 0;
    
    // Fill in the VP Control fields
    p_command->command = command;
    p_command->vp_count = VP_Count;
    bcopy(
       	(char *)VP_Indexes,
    	(char *)&p_command->vp_index1,
    	(int) VP_Count);
    
    // copy the VP Config data
    bcopy(
		(char *)vp_config,
    	(char *)&p_command->vp1,
		(VP_Count * sizeof(VP_CONFIG)));
    
	// Allocate an FCP_EVENT_CONTEXT from the pool.
	status = NU_Allocate_Partition(&Id->FCP_event_context_pool, 
		(VOID**)&p_context, NU_NO_SUSPEND);
	if (status != NU_SUCCESS)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_VP_Control", 
			"NU_Allocate_Partition for event context failed",
			status,
			(UNSIGNED)Id);
		return status;
	}
	
	// Zero the context
	bzero((char *)p_context, sizeof(FCP_EVENT_CONTEXT));
	
	// Set the instance pointer
	p_context->Id = Id;
	
	// Save user context pointer
	p_context->message = user;
	
	// Set action to perform
	p_context->action = FCP_ACTION_HANDLE_LOOP_IOCB_COMPL;

	// Save pointer to context in command IOCB.
	// A message will be sent to the FCP_event_queue 
	// when the command completes.
	p_command->system_defined2 = (UNSIGNED)p_context;
	
    // Send Command IOCB request to the ISP.
	// Update Request_FIFO_Index.  This lets the ISP know that a new
	// request is ready to execute.  
	FCP_Request_FIFO_Update_Index(Id);
	
	return status;
	
} // FCP_Modify_VP_Config
    	

/*************************************************************************/
// FCP_Handle_Loop_IOCB_Completion
// Handle the completion of a Loop control IOCB
/*************************************************************************/
STATUS FCP_Handle_Loop_IOCB_Completion(FCP_EVENT_CONTEXT *p_context)
{
	STATUS				 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Handle_Loop_IOCB_Completion);
 	
 	// Call the LoopMonitor Callback handler
 	// pass the users context and the IOCB completion status
 	Ext_LM_Handle_Callback(p_context->message, p_context->status_iocb.status);
 	
    // Deallocate the FCP_EVENT_CONTEXT 
    // allocated 
    status = NU_Deallocate_Partition(p_context);
    if (status != NU_SUCCESS)
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Handle_Loop_IOCB_Completion", 
			"NU_Deallocate_Partition for context failed",
			status,
			(UNSIGNED)p_context);
		
 	return status;
	
} // FCP_Handle_Loop_IOCB_Completion



/*************************************************************************/
// FCP_Handle_Loop_Event
// TODO: what did we need this for?
/*************************************************************************/
STATUS FCP_Handle_Loop_Event(FCP_EVENT_CONTEXT *p_context)
{
	STATUS				 status = NU_SUCCESS;
	void				(*callback)(void *);
	
 	FCP_TRACE_ENTRY(FCP_Handle_Loop_Event);
 	
 	// check for a Callback address
 	callback = (void (*)(void *))p_context->p_this_hdm;
 	if (callback)
 		(*callback)(p_context->message);
 	
    // Deallocate the FCP_EVENT_CONTEXT 
    // allocated 
    status = NU_Deallocate_Partition(p_context);
    if (status != NU_SUCCESS)
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Handle_Loop_Event", 
			"NU_Deallocate_Partition for context failed",
			status,
			(UNSIGNED)p_context);
		
 	return status;
	
} // FCP_Handle_Loop_Event
