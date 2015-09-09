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
// This file is the implementation of the Fibre Channel Initiator device.
// SCSI_ messages are handled by passing the command on to a Fibre
// Channel SCSI Target device.  Currently these are Disks only.  Since we
// are the end point of the chain, we do not need to have routing
// information.
//
// This file is linked to the Fcp.lib to build the final version. This is
// where most of the code to handle Initiator actions is located.
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HdmFcpInitiator/HdmFcpInit.cpp $
// 
// 8     2/03/00 7:17p Jlane
// Redirect port 2 in slotsa 24 & 28 to DDHs only if DDHs exist. 
// 
// 7     1/24/00 3:02p Jtaylor
// Add debug MaxCmdsExecuting
// 
// 6     1/11/00 7:28p Mpanas
// IOP missed change
// 
// 5     12/21/99 2:02p Mpanas
// Add support for IOP Failover
// - make several modules IOP_LOCAL
// 
// 4     8/10/99 5:04p Mpanas
// _DEBUG cleanup
// 
// 3     7/21/99 7:47p Mpanas
// Make sure we index our Instance_Data
// with the chip number
// 
// 2     7/19/99 8:24p Mpanas
// Fix Enable race condition
// 
// 1     7/15/99 11:49p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Initiator DDM project
// 
// 07/02/99 Michael G. Panas: Create file from HdmFcpRac.cpp
/*************************************************************************/

#include <stdio.h>
#include "OsTypes.h"

#include "Odyssey_Trace.h"
#define	TRACE_INDEX		TRACE_FCP_INITIATOR

// Debugging is turned on
#ifdef _DEBUG
#define FCP_DEBUG
#endif

#include "FcpError.h"
#include "FcpTrace.h"
#include "FcpEvent.h"
#include "Pci.h"			// need the BYTE_SWAP() macros for everybody
#include "FcpProto.h"
#include "Odyssey.h"		// Systemwide parameters

#include "Fcp.h"
#include "FC_Loop.h"

#include "FcpInitiator.h"
#include "FcpString.h"
#include "FcpMessageStatus.h"
#include "RqPts_t.h"
#include "IopStatusTable.h"

#include "HdmFcpInit.h"
#include "BuildSys.h"

extern void set_RAC_mode();				// nac.cpp
extern U32 Check_DDH_Slot(void);		// nac.cpp

CLASSNAME(HdmFcpInit, MULTIPLE);

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// HdmFcpInit
// Constructor method for the class HdmFcpInit
/*************************************************************************/
HdmFcpInit::HdmFcpInit(DID did):Ddm(did) {

	TRACE_ENTRY(HdmFcpInit::HdmFcpInit);
	
	SetConfigAddress(&config, sizeof(config));
	
} // HdmFcpInit

/*************************************************************************/
// Ctor
// 
/*************************************************************************/
Ddm *HdmFcpInit::Ctor(DID did) {

	TRACE_ENTRY(HdmFcpInit::Ctor);
	
	return new HdmFcpInit(did); // create a new instance of this HDM
}

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS HdmFcpInit::Initialize(Message *pMsg) {

	STATUS			 status;
	U32				 chip;
	INSTANCE_DATA	*Id;
	
	TRACE_ENTRY(HdmFcpInit::Initialize);
	
	// Old config had system loop # but in new we calculate it from local loop (chip) #.
	if (config.version > FCP_CONFIG_VERSION)
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
	// Use this data to Initialize the config record to pass to driver.
	Id->FCP_config = config;
	
	// Initialize ISP parameters for Initiator
	Id->Regs = (VOID *)((UNSIGNED)Id->ISP_Regs | 0xA0000000);
	
#if defined(FCP_DEBUG)
	// DEBUG setup
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
		Send( pReadIopStatusTable, pMsg, REPLYCALLBACK(HdmFcpInit,FinishInitialize));
	}
	else
	{
		// Start the FCP Initiator driver
		status = FCP_Start(Id);
	
		Reply(pMsg);
	}
	return status;
	
} // Initialize


/*************************************************************************/
// FinishInitialize
// Initialize NAC Port 2 according to slot and presence or lack of DDHs.
// Note that the original Initialize Message is expected to be in the 
// Context of the message parameter.
/*************************************************************************/
STATUS HdmFcpInit::FinishInitialize(Message *pMsg) {
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
							TRACEF(TRACE_L1, ("\n\rFC Port 2 Configured as Internal (DDH)\n\r"));
							break;
						}

			}  // end for...

	// Delete our table enumerate msg.
	delete pMsg;
	
	// Start the FCP Initiator driver
	status = FCP_Start(Id);
	
	// Reply to our initialize msg.
	Reply(pInitializeMsg);
	
	return status;
}


/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
STATUS HdmFcpInit::Enable(Message *pMsg) {

	TRACE_ENTRY(HdmFcpInit::Enable);
	
	// need to change config data here too since config is re-read
	if (config.version > FCP_CONFIG_VERSION)
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
STATUS HdmFcpInit::Quiesce(Message *pMsg) {

	TRACE_ENTRY(HdmFcpInit::Quiesce);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Quiesce

/*************************************************************************/
// DoWork
// This derived classes method to receive messages and replies
/*************************************************************************/
STATUS HdmFcpInit::DoWork(Message *pMsg) {

	TRACE_ENTRY(HdmFcpInit::DoWork);
	
	STATUS status = Ddm::DoWork(pMsg);
	
	TRACE_ENTRY(HdmFcpInit::DoWork Ddm::DoWork);
	
	if (status != OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION)
		return status;
		
	    FCP_DUMP_HEX(TRACE_L8, "\n\rHdmFcpInit::DoWork Message",
	    					(U8 *)pMsg,	128);
	if (pMsg->IsReply()) {
		FCP_EVENT_CONTEXT		*p_context;
		STATUS					 status;
		
	    FCP_DUMP_HEX(TRACE_L8, "\n\rHdmFcpInit::DoWork Message Reply",
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
		FCP_EVENT_CONTEXT	*p_context;
		STATUS				status;
		INSTANCE_DATA		*Id = &Instance_Data[instance];
		
		FCP_PRINT_STRING(TRACE_L8, "\n\rHdmFcpInit::DoWork I2O_SCSI_SCB_EXECUTE_MESSAGE");
		
		if (Id->FCP_state != FCP_STATE_ACTIVE)
		{
			// sorry, can't accept messages yet...
			FCP_PRINT_STRING(TRACE_L8, "\n\rHdmFcpInit::DoWork Not Active!");
			
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
		
		FCP_PRINT_HEX(TRACE_L8, "\n\rHdmFcpInit::DoWork: p_context = ", (U32)p_context);
		FCP_PRINT_HEX(TRACE_L8, "\n\rHdmFcpInit::DoWork: Id = ", (U32)p_context->Id);
	
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

	case SCSI_DEVICE_RESET:
	case SCSI_SCB_ABORT:
		// TODO:
		// Handle this correctly
		FCP_PRINT_STRING(TRACE_L8, "\n\rHdmFcpInit::DoWork Other Message");
		
		Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		break;

	default:
		return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
		}

	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
} // DoWork


// end of C++ derived class

