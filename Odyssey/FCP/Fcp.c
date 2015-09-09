/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Fcp.c
// 
// Description:
// This module contains all the external interfaces to the FCP driver.
// This module depends on the QLogic ISP2100
// 
// Update Log:
// 5/5/98 Jim Frandeen: Use C++ comment style
// 4/14/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 8/25/98 Michael G. Panas: Implement multiple QL2100 support
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/

#include "FcpCommon.h"
#include "Fcp.h"
#include "FcpBuffer.h"
#include "FcpCheck.h"
#include "FcpDebug.h"
#include "FcpEvent.h"
#include "FcpI2O.h"
#include "FcpInitiator.h"
#include "FcpISP.h"
#include "FcpISR.h"
#include "FcpMailbox.h"
#include "FcpMemory.h"
#include "FcpMessage.h"
#include "FcpRequestFIFO.h"
#include "FcpResponseFIFO.h"
#include "FcpTarget.h"
#include "FcpString.h"

/*************************************************************************/
//    Forward References
/*************************************************************************/
STATUS	FCP_Create(PINSTANCE_DATA Id); 
void	FCP_Destroy();
STATUS	FCP_Resume();

extern	U32 IsNacE1();
/*************************************************************************/
//    FCP_Create
//    Create all the FCP objects
/*************************************************************************/
STATUS FCP_Create(PINSTANCE_DATA Id)
{
	STATUS			 status;
	
	// Create objects.  
	// Pass each object the pointer to it's instance data
	// Each object initializes itself.
	
	// Starting State
	Id->FCP_state = FCP_STATE_RESET;
	
	// Trace must come first because every method depends on it.
	status = FCP_Trace_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Check_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Debug_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Error_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Event_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_I2O_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Initiator_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_IOCB_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_ISR_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Mailbox_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Memory_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Message_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Request_FIFO_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Response_FIFO_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Target_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_Buffer_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = FCP_ISP_Create(Id);	// must be last
	if (status != NU_SUCCESS)
		return status;
	
	Id->FCP_state = FCP_STATE_INITIALIZED;
	
	return status;
		
} // FCP_Create

/*************************************************************************/
//    FCP_Destroy
//    Destroy all the FCP objects
/*************************************************************************/
void FCP_Destroy(PINSTANCE_DATA Id)
{
	// TODO:
	// pass Id to each destroy method
	FCP_Check_Destroy();
	
	FCP_Debug_Destroy();
	
	FCP_Error_Destroy();
	
	FCP_Event_Destroy();
	
	FCP_I2O_Destroy();
	
	FCP_Initiator_Destroy();
	
	FCP_IOCB_Destroy();
	
	FCP_ISR_Destroy();
	
	FCP_Mailbox_Destroy();
	
	FCP_Memory_Destroy();
	
	FCP_Message_Destroy();
	
	FCP_Request_FIFO_Destroy();
	
	FCP_Response_FIFO_Destroy();
	
	FCP_Target_Destroy();
	
	FCP_Trace_Destroy();
	
	FCP_ISP_Destroy();
	
	FCP_Buffer_Destroy();
	
} // FCP_Destroy

/*************************************************************************/
//    FCP_Quiet
//    Quiesce all FCP activity, change state to QUIET
/*************************************************************************/
STATUS FCP_Quiet(PINSTANCE_DATA Id)
{
	STATUS			 status = NU_SUCCESS;
	
	// Quiesced State
	Id->FCP_state = FCP_STATE_QUIET;
	
	return(status);
} // FCP_Quiet

/*************************************************************************/
//    FCP_Restart
//    Restart all FCP activity, change state to ACTIVE
/*************************************************************************/
STATUS FCP_Restart(PINSTANCE_DATA Id)
{
	STATUS			 status = NU_SUCCESS;
	
	// Active State
	Id->FCP_state = FCP_STATE_ACTIVE;
	
	return(status);
} // FCP_Restart

/*************************************************************************/
//    FCP_Reset
//    Reset FCP Library, change state to RESET
/*************************************************************************/
STATUS FCP_Reset(PINSTANCE_DATA Id)
{
	STATUS			 status = NU_SUCCESS;
	
	// Back to Starting State
	Id->FCP_state = FCP_STATE_RESET;
	
	// wait for all I/O to drain
	// TODO:
	
	// Clear all resources
	FCP_Destroy(Id);
	
	return(status);
} // FCP_Reset

/*************************************************************************/
//    FCP_Resume
//    Resume the FCP_Event_Task
/*************************************************************************/
STATUS FCP_Resume(PINSTANCE_DATA Id)
{
	STATUS			status;
	
	// Start the Event task for this instance
	status = NU_Resume_Task(&Id->FCP_event_task);
	
	if (status != NU_SUCCESS)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Initiator_Task_Resume", 
			"NU_Resume_Task for Event task failed",
			status,
			0);
		return status;
	}

	return status;
		
} // FCP_Resume

/*************************************************************************/
//	FCP_Start
//	This entry point is called to initialize and start the FCP driver.
/*************************************************************************/
STATUS FCP_Start(PINSTANCE_DATA Id)
{	
	STATUS			status;
	
	FCP_PRINT_STRING(TRACE_L8, "\n\rFCP_Start entered.");
	
	// Make sure our structure sizes are correct.
	FCP_ASSERT(sizeof(IOCB_COMMAND_TYPE2) == IOCB_SIZE, "FCP_Start");
	FCP_ASSERT(sizeof(IOCB_ENABLE_LUN) == IOCB_SIZE, "FCP_Start");
	FCP_ASSERT(sizeof(IOCB_MODIFY_LUN) == IOCB_SIZE, "FCP_Start");
	FCP_ASSERT(sizeof(IOCB_IMMEDIATE_NOTIFY) == IOCB_SIZE, "FCP_Start");
	FCP_ASSERT(sizeof(IOCB_ATIO_TYPE_2) == IOCB_SIZE, "FCP_Start");
	FCP_ASSERT(sizeof(IOCB_CTIO_TYPE_2) == IOCB_SIZE, "FCP_Start");
	FCP_ASSERT(sizeof(IOCB_CTIO_MODE_0) == IOCB_SIZE, "FCP_Start");
	FCP_ASSERT(sizeof(IOCB_CTIO_MODE_1) == IOCB_SIZE, "FCP_Start");
	FCP_ASSERT(sizeof(IOCB_CTIO_MODE_2) == IOCB_SIZE, "FCP_Start");
	FCP_ASSERT(sizeof(IOCB_STATUS_TYPE0) == IOCB_SIZE, "FCP_Start");

	FCP_ASSERT(sizeof(FCP_MAILBOX_MESSAGE) == sizeof(UNSIGNED) * 4, "FCP_Start");
	
	FCP_PRINT_STRING(TRACE_L8, "\n\rInitial Asserts OK.");

	if ( !IsNacE1() && Id->FCP_instance == 1) {
		printf("MAC 1 Does not work in this Board\n\r");
		return NU_SUCCESS;
	}

	// Create all the FCP objects.
	status = FCP_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	// Start the FCP_Event_Task.
	status = FCP_Resume(Id);
	if (status != NU_SUCCESS)
		return status;
	
	return status;
	
} // FCP_Start

