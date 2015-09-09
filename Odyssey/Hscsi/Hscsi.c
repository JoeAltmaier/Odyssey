/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Hscsi.c
// 
// Description:
// This module contains all the external interfaces to the HSCSI driver.
// This module depends on the HBC-embedded QLogic ISP1040B.
// 
// Update Log:
//	$Log: /Gemini/Odyssey/Hscsi/Hscsi.c $
// 
// 1     9/14/99 7:23p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
// 
/*************************************************************************/

#include "HscsiCommon.h"
#include "Hscsi.h"
#include "HscsiBuffer.h"
#include "HscsiCheck.h"
#include "HscsiDebug.h"
#include "HscsiEvent.h"
#include "HscsiI2O.h"
#include "HscsiInitiator.h"
#include "HscsiISP.h"
#include "HscsiISR.h"
#include "HscsiMailbox.h"
#include "HscsiMemory.h"
#include "HscsiMessage.h"
#include "HscsiRequestFIFO.h"
#include "HscsiResponseFIFO.h"
#include "HscsiTarget.h"
#include "HscsiString.h"

/*************************************************************************/
//    Forward References
/*************************************************************************/
STATUS	HSCSI_Create(PHSCSI_INSTANCE_DATA Id); 
void	HSCSI_Destroy();
STATUS	HSCSI_Resume();

/*************************************************************************/
//    HSCSI_Create
//    Create all the HSCSI objects
/*************************************************************************/
STATUS HSCSI_Create(PHSCSI_INSTANCE_DATA Id)
{
	STATUS			 status;
	
	// Create objects.  
	// Pass each object the pointer to it's instance data
	// Each object initializes itself.
	
	// Starting State
	Id->HSCSI_state = HSCSI_STATE_RESET;
	
	// Trace must come first because every method depends on it.
	status = HSCSI_Trace_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_Check_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_Debug_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_Error_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_Event_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_I2O_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_Initiator_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_IOCB_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_ISR_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_Mailbox_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_Memory_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_Message_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_Request_FIFO_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_Response_FIFO_Create(Id);
	if (status != NU_SUCCESS)
		return status;

/*	For now, no targets, initiator only	
	status = HSCSI_Target_Create(Id);
	if (status != NU_SUCCESS)
		return status;
*/	
	status = HSCSI_Buffer_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	status = HSCSI_ISP_Create(Id);	// must be last
	if (status != NU_SUCCESS)
		return status;
	
	Id->HSCSI_state = HSCSI_STATE_ACTIVE;
	
	return status;
		
} // HSCSI_Create

/*************************************************************************/
//    HSCSI_Destroy
//    Destroy all the HSCSI objects
/*************************************************************************/
void HSCSI_Destroy(PHSCSI_INSTANCE_DATA Id)
{
	// TODO:
	// pass Id to each destroy method
	HSCSI_Check_Destroy();
	
	HSCSI_Debug_Destroy();
	
	HSCSI_Error_Destroy();
	
	HSCSI_Event_Destroy();
	
	HSCSI_I2O_Destroy();
	
	HSCSI_Initiator_Destroy();
	
	HSCSI_IOCB_Destroy();
	
	HSCSI_ISR_Destroy();
	
	HSCSI_Mailbox_Destroy();
	
	HSCSI_Memory_Destroy();
	
	HSCSI_Message_Destroy();
	
	HSCSI_Request_FIFO_Destroy();
	
	HSCSI_Response_FIFO_Destroy();
	
//	HSCSI_Target_Destroy();
	
	HSCSI_Trace_Destroy();
	
	HSCSI_ISP_Destroy();
	
	HSCSI_Buffer_Destroy();
	
} // HSCSI_Destroy

/*************************************************************************/
//    HSCSI_Quiet
//    Quiesce all HSCSI activity, change state to QUIET
/*************************************************************************/
STATUS HSCSI_Quiet(PHSCSI_INSTANCE_DATA Id)
{
	STATUS			 status = NU_SUCCESS;
	
	// Quiesced State
	Id->HSCSI_state = HSCSI_STATE_QUIET;
	
	return(status);
} // HSCSI_Quiet

/*************************************************************************/
//    HSCSI_Restart
//    Restart all HSCSI activity, change state to ACTIVE
/*************************************************************************/
STATUS HSCSI_Restart(PHSCSI_INSTANCE_DATA Id)
{
	STATUS			 status = NU_SUCCESS;
	
	// Active State
	Id->HSCSI_state = HSCSI_STATE_ACTIVE;
	
	return(status);
} // HSCSI_Restart

/*************************************************************************/
//    HSCSI_Reset
//    Reset HSCSI Library, change state to RESET
/*************************************************************************/
STATUS HSCSI_Reset(PHSCSI_INSTANCE_DATA Id)
{
	STATUS			 status = NU_SUCCESS;
	
	// Back to Starting State
	Id->HSCSI_state = HSCSI_STATE_RESET;
	
	// wait for all I/O to drain
	// TODO:
	
	// Clear all resources
	HSCSI_Destroy(Id);
	
	return(status);
} // HSCSI_Reset

/*************************************************************************/
//    HSCSI_Resume
//    Resume the HSCSI_Event_Task
/*************************************************************************/
STATUS HSCSI_Resume(PHSCSI_INSTANCE_DATA Id)
{
	STATUS			status;
	
	// Start the Event task for this instance
	status = NU_Resume_Task(&Id->HSCSI_event_task);
	
	if (status != NU_SUCCESS)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Initiator_Task_Resume", 
			"NU_Resume_Task for Event task failed",
			status,
			0);
		return status;
	}

	return status;
		
} // HSCSI_Resume

/*************************************************************************/
//	HSCSI_Start
//	This entry point is called to initialize and start the HSCSI driver.
/*************************************************************************/
STATUS HSCSI_Start(PHSCSI_INSTANCE_DATA Id)
{	
	STATUS			status;
	
	HSCSI_PRINT_STRING(TRACE_L8, "\n\rHSCSI_Start entered.");
	
	// Make sure our structure sizes are correct.
	HSCSI_ASSERT(sizeof(HSCSI_IOCB_COMMAND) == IOCB_SIZE, "HSCSI_Start");
	HSCSI_ASSERT(sizeof(HSCSI_IOCB_CONTINUATION) == IOCB_SIZE, "HSCSI_Start");
	HSCSI_ASSERT(sizeof(IOCB_STATUS_ENTRY) == IOCB_SIZE, "HSCSI_Start");
	
	HSCSI_ASSERT(sizeof(HSCSI_MAILBOX_MESSAGE) == sizeof(UNSIGNED) * 4, "HSCSI_Start");
	
	HSCSI_PRINT_STRING(TRACE_L8, "\n\rInitial Asserts OK.");

	// Create all the HSCSI objects.
	status = HSCSI_Create(Id);
	if (status != NU_SUCCESS)
		return status;
	
	// Start the HSCSI_Event_Task.
	status = HSCSI_Resume(Id);
	if (status != NU_SUCCESS)
		return status;
		
	return status;
	
} // HSCSI_Start

