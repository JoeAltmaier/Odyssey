/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This file is the implementation of the QLogic 1040B SCSI Initiator device.
// I2O_SCSI messages are handled by passing the command on to a SCSI device.
//
// This file is linked to the Hscsi.lib to build the final version. This is
// where most of the code to handle Initiator actions is located.
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HdmHscsi/HdmHscsiEval.cpp $
// 
// 1     10/20/99 6:31p Cchan
// Eval version of HdmHscsi.cpp
// 
// 1     9/17/99 11:57a Cchan
// Files for HSCSI DDM to support the HSCSI library (QL1040B)
// 
/*************************************************************************/

#include <stdio.h>
#include "OsTypes.h"

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_DAISY_MONITOR
#include "Odyssey_Trace.h"

// Debugging is turned on
#define HSCSI_DEBUG 

#include "HscsiData.h"
#include "HscsiError.h"
#include "HscsiTrace.h"
#include "HscsiEvent.h"
#include "Pci.h"			// need the BYTE_SWAP() macros for everybody
#include "HscsiProto.h"
#include "Odyssey.h"		// Systemwide parameters

#include "Hscsi.h"

#include "HscsiInitiator.h"
#include "HscsiString.h"

#include "HdmHscsi.h"
#include "BuildSys.h"


/*************************************************************************/
// Forward references
/*************************************************************************/
extern "C" {
STATUS	HSCSI_Message_Send_Response(HSCSI_EVENT_CONTEXT *p_context);
}

CLASSNAME(HdmHscsi, SINGLE);

/*************************************************************************/
// Global references
/*************************************************************************/
HdmHscsi	*pHdmHscsi = NULL;		// really just a flag


/*************************************************************************/
// HdmHscsi
// Constructor method for the class HdmHscsi
/*************************************************************************/
HdmHscsi::HdmHscsi(DID did):Ddm(did) {

	TRACE_ENTRY(HdmHscsi::HdmHscsi);
	
	pHdmHscsi = this;			// save the first one
	
	SetConfigAddress(&config, sizeof(config));
	
} // HdmHscsi

/*************************************************************************/
// Ctor
// 
/*************************************************************************/
Ddm *HdmHscsi::Ctor(DID did) {

	TRACE_ENTRY(HdmHscsi::Ctor);
	
	if (pHdmHscsi)
		return(NULL);		// sorry only one instance allowed per board

	return new HdmHscsi(did); // create a new instance of this HDM
}

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS HdmHscsi::Initialize(Message *pMsg) {

	STATUS			 status;
	HSCSI_INSTANCE_DATA	*Id;
	extern void		*dma_base_addr;
	
	TRACE_ENTRY(HdmHscsi::Initialize);
	
	// check the instance, if BOTH, skip initialization since the
	// other guy has already done it.
	if (config.config_instance == BOTH_INSTANCE)
	{
		return (OS_DETAIL_STATUS_SUCCESS);
	}
	
	if (config.config_instance == INIT_ONLY_INSTANCE)
	{
		Id = &H_Instance_Data; 
		Id->HSCSI_instance = 0;
		instance = 0;
	}
	else
	{
		Id = &H_Instance_Data; 
		Id->HSCSI_instance = config.config_instance;
		instance = config.config_instance;
	}
	
	Id->HSCSI_state = HSCSI_STATE_RESET;
	
	// Get the config data from the config structure after it is loaded 
	// from the Persistent Data Store.
	// Use this data to Initialize config record to pass to driver.
	Id->HSCSI_config = config;
	
	// Initialize ISP parameters for Initiator
	// base address & interrupt are determined elsewhere, at Hbcmenu.cpp for example
	
	Id->ISP_Regs = (VOID *)Id->HSCSI_config.base_ISP_address; 
	
	Id->Regs = (VOID *)((UNSIGNED)Id->ISP_Regs | 0xa0000000); // conversion for HBC
	// HBC - 0x80000000
	// Eval | 0xa0000000

	Id->HSCSI_interrupt = Id->HSCSI_config.interrupt;
	
	
#if defined(HSCSI_DEBUG)
	// DEBUG setup
	Id->HSCSI_flags = 0;
	Id->CmdsExecuting = 0;
	Id->Last = (void *) 0;
	Id->LastDone = (void *) 0;
	Id->Num_low_isr = 0;
	Id->Num_high_isr = 0;
	Id->Num_high_isr_entry = 0;
#endif
	Id->HSCSI_if_print_ISR = 9; // for test only

	// Start the HSCSI Initiator driver
	status = HSCSI_Start(Id);
	
	Reply(pMsg);
	return status;
	
} // Initialize

/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
STATUS HdmHscsi::Enable(Message *pMsg) {

	TRACE_ENTRY(HdmHscsi::Enable);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Enable

/*************************************************************************/
// Quiesce
// Turn off the DDM belonging to this derived class
/*************************************************************************/
STATUS HdmHscsi::Quiesce(Message *pMsg) {

	TRACE_ENTRY(HdmHscsi::Quiesce);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Quiesce

/*************************************************************************/
// DoWork
// This derived classes method to receive messages and replies
/*************************************************************************/
STATUS HdmHscsi::DoWork(Message *pMsg) {

	TRACE_ENTRY(HdmHscsi::DoWork);
	
	STATUS status = Ddm::DoWork(pMsg);
	
	TRACE_ENTRY(HdmHscsi::DoWork Ddm::DoWork);
	
	if (status != OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION)
		return status;
		
	    HSCSI_DUMP_HEX(TRACE_L8, "\n\rHdmHscsi::DoWork Message",
	    					(U8 *)pMsg,	128);
	if (pMsg->IsReply()) {
		HSCSI_EVENT_CONTEXT		*p_context;
		STATUS					 status;
		
	    HSCSI_DUMP_HEX(TRACE_L8, "\n\rHdmHscsi::DoWork Message Reply",
	    					(U8 *)pMsg,	128);
		switch (pMsg->reqCode) {
		
		case SCSI_DEVICE_RESET:
		case SCSI_SCB_ABORT:
			break;
		}
	}
	
	// New service message
	else switch(pMsg->reqCode) {

	case SCSI_SCB_EXEC:
	{
		// Handle the I2O_SCSI_SCB_EXECUTE_MESSAGE
		HSCSI_EVENT_CONTEXT	*p_context;
		STATUS				status;
		
		HSCSI_PRINT_STRING(TRACE_L8, "\n\rHdmHscsi::DoWork I2O_SCSI_SCB_EXECUTE_MESSAGE");
		
		// get a new context for this command
		p_context = (HSCSI_EVENT_CONTEXT *)new HSCSI_EVENT_CONTEXT;
		p_context->Id = &H_Instance_Data;
		
		// copy a pointer to the original message into the context
		p_context->message = (void *)pMsg;
		
		// save our "this" pointer for later
		p_context->p_this_hdm = (void *)this;
		
		HSCSI_PRINT_HEX(TRACE_L8, "\n\rHdmHscsi::DoWork: p_context = ", (U32)p_context);
		HSCSI_PRINT_HEX(TRACE_L8, "\n\rHdmHscsi::DoWork: Id = ", (U32)p_context->Id);
	
#if defined(HSCSI_DEBUG)
		// DEBUG
		p_context->Id->CmdsExecuting++;
		p_context->Id->Last = (void *) p_context;
#endif
		
		// send the command to the drive via the HSCSI driver code
		status = HSCSI_Handle_SCSI_Request(p_context);
	}
		break;

	case SCSI_DEVICE_RESET:
	case SCSI_SCB_ABORT:
		// TODO:
		// Handle this correctly
		HSCSI_PRINT_STRING(TRACE_L8, "\n\rHdmHscsi::DoWork Other Message");
		
		Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		break;

	default:
		return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
		}

	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
} // DoWork


// end of C++ derived class


/*************************************************************************/
// HSCSI_Message_Send_Response
// Send a reply message back to the Message Sender. This method is called
// by the Initiator Task after status is returned from the disk. The message
// response is already filled in.
/*************************************************************************/
STATUS HSCSI_Message_Send_Response(HSCSI_EVENT_CONTEXT *p_context)
{
	STATUS 			 	status;
	HdmHscsi			*pHdmHscsi;

 	TRACE_ENTRY(HSCSI_Message_Send_Response);
	
#if defined(HSCSI_DEBUG)
	// DEBUG
	p_context->Id->CmdsExecuting--;
	p_context->Id->LastDone = (void *) p_context;
#endif
	
	// recover this pointer to our HDM
	pHdmHscsi = (HdmHscsi *) p_context->p_this_hdm;
	
	// Send a response back to the caller
	status = pHdmHscsi->Reply((Message *)p_context->message);
	
	delete p_context;		// done with this guy
	
	return status;
	
} // HSCSI_Message_Send_Response

/*************************************************************************/
// HSCSI_Handle_AE_Initiator
// Called to handle Asynchronous Events.  These may or may not Throw an
// event depending on who is listening.
/*************************************************************************/
STATUS HSCSI_Handle_AE_Initiator(HSCSI_EVENT_CONTEXT *p_context)
{
	STATUS 			 status = NU_SUCCESS;
	Message			*pMsg;
	VDN				vd = pHdmHscsi->config.virtual_device;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Handle_AE_Initiator);
 	
	// TODO:
	// Switch depending on the state of the context.
	switch (p_context->action)
	{

		// TODO:
		// handle these AEs
	
		case HSCSI_ACTION_HANDLE_OTHER_AE:
		HSCSI_PRINT_STRING(TRACE_L2, "\n\rAE Init Other AE");
		break;
		case HSCSI_ACTION_HANDLE_THROW_EVENT:
		HSCSI_PRINT_STRING(TRACE_L2, "\n\rAE Init Throw Event");
		break;
	}
	
	// in all cases we are done with this context
    // Deallocate the HSCSI_EVENT_CONTEXT 
    // allocated by HSCSI_Handle_Async_Event
    status = NU_Deallocate_Partition(p_context);
    if (status != NU_SUCCESS)
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Handle_AE_Initiator", 
			"NU_Deallocate_Partition for context failed",
			status,
			(UNSIGNED)p_context);
		
				
	return status;
	
} // HSCSI_Handle_AE_Initiator
