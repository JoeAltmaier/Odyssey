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
// This file is the implementation of the Fibre Channel Target device.
// We can have many instances that can run on any board that has a FCP
// Library.  This HDM is the head end of the whole virtual device chain.
// It mainly generates messages and handles replies.
//
// We must look up the VirtualDevice to send to based on the LUN.
// 
// This file is linked to the Fcp.lib to build the final version.  The bulk
// of the code needed to handle Fibre Channel Target operations is located
// in the library.
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HdmFcpTargetInit/HdmFcpTargetInit.cpp $
// 
// 9     2/03/00 7:20p Jlane
// Add ContinueInitialize() to redirect port 2 in slots 24 & 28 to DDHs
// only if DDHs exist.
// 
// 8     1/24/00 3:14p Jtaylor
// Add debug MaxCmdsExecuting
// 
// 7     1/09/00 8:20p Mpanas
// IOP failover missed changes
// 
// 6     12/21/99 2:03p Mpanas
// Add support for IOP Failover
// - make several modules IOP_LOCAL
// 
// 5     8/23/99 2:20p Mpanas
// Changes to support Export Table States
// 
// 4     7/24/99 12:57p Mpanas
// Queue the incoming reply instead of
// executing straight through to the FCP
// library
// 
// 3     7/21/99 8:50p Mpanas
// use new state flags (temporary)
// 
// 2     7/21/99 7:50p Mpanas
// Merge changes made in FcpTarget and FcpInit
// 
// 1     7/15/99 11:48p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Target and Initiator DDM project
// 
// 07/02/99 Michael G. Panas: Create file from HdmFcpNic
/*************************************************************************/

#include <stdio.h>
#include "OsTypes.h"
#include "Odyssey.h"		// Systemwide parameters

#define FCP_DEBUG 
#include "Odyssey_Trace.h"
#define	TRACE_INDEX		TRACE_FCP_TARGET

#include "FcpError.h"
#include "FcpTrace.h"
#include "FcpEvent.h"
#include "Pci.h"			// need the BYTE_SWAP() macros for everybody
#include "FcpProto.h"

#include "Fcp.h"
#include "FC_Loop.h"
#include "FcpData.h"
#include "FcpIsr.h"
#include "Pci.h"
#include "CTIdLun.h"

#include "FcpEvent.h"
#include "FcpMessage.h"
#include "FcpTarget.h"
#include "FcpInitiator.h"
#include "FcpMessageStatus.h"

// Tables referenced
#include "ExportTable.h"
#include "DiskDescriptor.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"
#include "RqPts_t.h"
#include "IopStatusTable.h"

#include "HdmFcpTargetInit.h"
#include "Message.h"
#include "BuildSys.h"

extern void set_RAC_mode();				// nac.cpp
extern U32 Check_DDH_Slot(void);		// nac.cpp

CLASSNAME(HdmFcpTargetInit, MULTIPLE);

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/

/*************************************************************************/
// HdmFcpTargetInit
// Constructor method for the class HdmFcpRac
/*************************************************************************/
HdmFcpTargetInit::HdmFcpTargetInit(DID did):HdmFcp(did) {

	TRACE_ENTRY(HdmFcpTargetInit::HdmFcpTargetInit);
	
	FCP_PRINT_HEX(TRACE_L6, "\n\rHdmFcpTargetInit::HdmFcpTargetInit this = ", (U32)this);
	FCP_PRINT_HEX(TRACE_L6, "\n\rHdmFcpTargetInit::HdmFcpTargetInit did = ", (U32)did);
	
	MyVd = GetVdn();
	
	SetConfigAddress(&config, sizeof(config));
	
}	// HdmFcpTargetInit

/*************************************************************************/
// Ctor
// Create a new instance of the Fcp Nic Hdm
/*************************************************************************/
Ddm *HdmFcpTargetInit::Ctor(DID did) {

	TRACE_ENTRY(HdmFcpTargetInit::Ctor);

	return new HdmFcpTargetInit(did); 
}	// Ctor

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS HdmFcpTargetInit::Initialize(Message *pMsg) {

	STATUS			 status;
	U32				 chip;
	INSTANCE_DATA	*Id;
	
	TRACE_ENTRY(HdmFcpTargetInit::Initialize);
	
	TI_TS_Export = NULL;
	
	TI_Export = NULL;
	
	// Old config had system loop # but in new we calculate it from local loop (chip) #.
	if (config.version > 3)
	{
		chip = config.loop_instance;
		config.loop_instance = FCLOOPINSTANCE( Address::iSlotMe, chip );
	}
	else
		// Chip number will be used to index the Instance_Data array
		chip = FCLOOPCHIP(config.loop_instance);
	
	
	Id = &Instance_Data[chip];
	Id->FCP_instance = chip;
	instance = chip;
	
	// pass the FCInstance number and chip number to the FCP Library
	Id->FCP_loop = config.loop_instance;
	Id->FCP_chip = chip;
	
	// set our DDM pointer
	Id->pFCP_DDM = (void *) this;
	
	Id->FCP_state = FCP_STATE_RESET;

	// Get the config data from the config structure after it is loaded 
	// from the Persistent Data Store.
	// Use this data to Initialize config record to pass to driver.
	Id->FCP_config = config;
	
	// Initialize ISP parameters for Target
	Id->Regs = (VOID *)((UNSIGNED)Id->ISP_Regs | 0xA0000000);
	
#if defined(FCP_DEBUG)    
	// DEBUG
	Id->FCP_flags = 0;
	Id->CmdsExecuting = 0;
	Id->MaxCmdsExecuting = 0;
	Id->Last = (void *) 0;
	Id->LastDone = (void *) 0;
	Id->Num_low_isr = 0;
	Id->Num_high_isr = 0;
	Id->Num_high_isr_entry = 0;
#endif
	Id->FCP_if_print_ISR = 9; // for test only

	// make sure this is NULL
	pEnableMsg = NULL;

	// If we are in slot 24 or 28 AND we are being configured for Chip 2
	if (Check_DDH_Slot() && (chip == 2))
	{
		// then fire off a message to read the IopStatusTable to see if
		// we have any DDHs and if so redirect chip two out the backplane to the
		// DDHs.  Note that I pass the Iniitialize Msg as the context of
		// the EnumTable Msg for reply in FinishInitialize().
		RqPtsEnumerateTable_T<IOPStatusRecord> *pReadIopStatusTable
			= new RqPtsEnumerateTable_T<IOPStatusRecord>;
		Send( pReadIopStatusTable, pMsg, REPLYCALLBACK(HdmFcpTargetInit,ContinueInitialize));
	}
	else
	{
		// Start the FCP Target/Initiator driver
		status = FCP_Start(Id);
	
		// Get our table data from the Persistent Data Store
		TI_TableInitialize(pMsg);
	}
	
	return status;
	
}	// Initialize

/*************************************************************************/
// ContinueInitialize
// Initialize NAC Port 2 according to slot and presence or lack of DDHs.
// Note that the original Initialize Message is expected to be in the 
// Context of the message parameter.
/*************************************************************************/
STATUS HdmFcpTargetInit::ContinueInitialize(Message *pMsg) {
STATUS status = pMsg->Status();
Message* pInitializeMsg = (Message*)pMsg->GetContext();
RqPtsEnumerateTable_T<IOPStatusRecord> *pReadIopStatusTable 
						= (RqPtsEnumerateTable_T<IOPStatusRecord> *)pMsg;
U32				chip 	= FCLOOPCHIP(config.loop_instance);
INSTANCE_DATA	*Id 	= &Instance_Data[chip];

	// If we read the IOPStatus Table w/o any error.
	if (status == OK)
		for (U32 nRows = pReadIopStatusTable->GetRowCount(),
		 	 i = 0;
			 i < nRows;
			 i++)
			{
				IOPStatusRecord* pRow = &pReadIopStatusTable->GetRowPtr()[i];
				if (pRow->IOP_Type == IOPTY_DDH)
				// Double check that we still think we're in a NAC slot and chip.
					if (Check_DDH_Slot() && (chip == 2))
						{
							// Change port 2 to RAC mode
							// Talk to DDH (internal FC)
							set_RAC_mode();
							TRACEF(TRACE_L3, ("\n\rFC Port 2 Configured as Internal (DDH)\n\r"));
							break;
						}

			}  // end for...

	// Delete our table enumerate msg.
	delete pMsg;
	
	// Start the FCP Target & Initiator driver
	status = FCP_Start(Id);
	
	// Get our table data from the Persistent Data Store
	TI_TableInitialize(pMsg);
	
	return status;
}


/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
STATUS HdmFcpTargetInit::Enable(Message *pMsg) {

	TRACE_ENTRY(HdmFcpTargetInit::Enable);
	
	// must change the config data here also since it is re-read from the PTS
	if (config.version > 3)
	{
		config.loop_instance = FCLOOPINSTANCE( Address::iSlotMe, config.loop_instance );
	}
	
	// check if the loop already came up
	if (pEnableMsg)
	{
		// loop is up, so send the reply now
		Reply(pMsg);
		return OS_DETAIL_STATUS_SUCCESS;
	}
	
	// save the enable message until the loop comes up
	// since we can't receive messages until then
	pEnableMsg = pMsg;
	
	//Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Enable

/*************************************************************************/
// Quiesce
// Turn off the DDM belonging to this derived class
/*************************************************************************/
STATUS HdmFcpTargetInit::Quiesce(Message *pMsg) {

	TRACE_ENTRY(HdmFcpTargetInit::Quiesce);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Quiesce

/*************************************************************************/
// DoWork
// This derived classes method to receive messages and replies
/*************************************************************************/
STATUS HdmFcpTargetInit::DoWork(Message *pMsg) {

	TRACE_ENTRY(HdmFcpTargetInit::DoWork);

	STATUS status=Ddm::DoWork(pMsg);
	
	TRACE_ENTRY(HdmFcpTargetInit::DoWork Ddm::DoWork);
	
	if (status != OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION)
		return status;

	// This HDM mostly handles Replies to messages sent from the Interrupt Handler
	if (pMsg->IsReply()) {
		FCP_EVENT_CONTEXT		*p_context;
		STATUS					 status;
		
	    FCP_DUMP_HEX(TRACE_L8, "\n\rHdmFcpTargetInit::DoWork Message Reply",
	    					(U8 *)pMsg,	128);
		switch (pMsg->reqCode) {
		case SCSI_SCB_EXEC:
			p_context = (FCP_EVENT_CONTEXT *)pMsg->GetContext();
			// Send this message to FCP_Event_Task because we
			// don't want to handle it on this thread
			// The action field of the context will tell FCP_Event_Task
			// what to do next. 
    		status = NU_Send_To_Queue(&p_context->Id->FCP_event_queue, 
    			&p_context, // message is pointer to context
        		1, // size is one UNSIGNED 
        		NU_NO_SUSPEND);
				
         	if (status != NU_SUCCESS)
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"HdmFcpTarget::DoWork", 
					"NU_Send_To_Queue failed",
					status,
					(UNSIGNED)p_context->Id);
         	 
			break;
			
		case SCSI_DEVICE_RESET:
		case SCSI_SCB_ABORT:
			break;
		}
	}
	
	// New service message has been received
	else switch(pMsg->reqCode) {

	case SCSI_SCB_EXEC:
		{
			// Handle the I2O_SCSI_SCB_EXECUTE_MESSAGE
			FCP_EVENT_CONTEXT	*p_context;
			STATUS				status;
			INSTANCE_DATA		*Id = &Instance_Data[instance];
			
			FCP_PRINT_STRING(TRACE_L8, "\n\rHdmFcpTargetInit::DoWork I2O_SCSI_SCB_EXECUTE_MESSAGE");
			
			if (Id->FCP_state != FCP_STATE_ACTIVE)
			{
				// sorry, can't accept messages yet...
				FCP_PRINT_STRING(TRACE_L8, "\n\rHdmFcpTargetInit::DoWork Not Active!");
				
				Reply(pMsg, FCP_SCSI_HBA_DSC_FUNCTION_UNAVAILABLE);
				return OS_DETAIL_STATUS_SUCCESS;
			}
			
			// get a new context for this command
			p_context = (FCP_EVENT_CONTEXT *)new FCP_EVENT_CONTEXT;
			p_context->Id = Id;
			
			// copy a pointer to the original message into the context
			p_context->message = (void *)pMsg;
			
			// save our "this" pointer for later
			p_context->p_this_hdm = (void *)this;
			
			FCP_PRINT_HEX(TRACE_L8, "\n\rHdmFcpTargetInit::DoWork: p_context = ", (U32)p_context);
			FCP_PRINT_HEX(TRACE_L8, "\n\rHdmFcpTargetInit::DoWork: Id = ", (U32)p_context->Id);
		
	#if defined(FCP_DEBUG)
			// DEBUG
			p_context->Id->CmdsExecuting++;
			if (p_context->Id->CmdsExecuting > p_context->Id->MaxCmdsExecuting)
				p_context->Id->MaxCmdsExecuting++;
			p_context->Id->Last = (void *) p_context;
	#endif
			
			// send the command to the drive via the FCP driver code
			status = FCP_Handle_SCSI_Request(p_context);
		}
		break;
			
		default:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
	}

	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
}	// DoWork


// end of C++ derived class

