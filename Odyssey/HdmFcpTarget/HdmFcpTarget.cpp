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
//	$Log: /Gemini/Odyssey/HdmFcpTarget/HdmFcpTarget.cpp $
// 
// 12    1/24/00 3:13p Jtaylor
// Add debug MaxCmdsExecuting
// 
// 11    1/09/00 8:20p Mpanas
// IOP Failover missed Enable() code
// 
// 10    12/21/99 2:02p Mpanas
// Add support for IOP Failover
// - make several modules IOP_LOCAL
// 
// 9     10/12/99 11:35a Jlane
// Change union to structure for PHS Reporter because it is possible for
// the same Instance Data to be shared between FCP Target and FCP
// Initiator drivers.
// 
// 8     9/13/99 5:34p Vnguyen
// Add PHS Reporter performance and status counters.
// 
// 7     8/20/99 7:50p Mpanas
// Changes to support Export Table states
// Re-organize sources
// 
// 6     8/10/99 4:55p Mpanas
// _DEBUG cleanup
// 
// 5     7/24/99 12:54p Mpanas
// Queue the incoming reply instead of
// executing straight through to the FCP
// library
// 
// 4     7/21/99 8:49p Mpanas
// use new state flags (temporary)
// 
// 3     7/21/99 7:45p Mpanas
// merge changes to FcpTarget and FcpInit
// 
// 2     7/19/99 9:03p Mpanas
// add missing table pointer Increment
// 
// 1     7/15/99 11:48p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Target DDM project
// 
// 07/02/99 Michael G. Panas: Create file from HdmFcpNic
/*************************************************************************/

#include <stdio.h>
#include "OsTypes.h"
#include "Odyssey.h"		// Systemwide parameters

#ifdef _DEBUG
#define FCP_DEBUG
#endif 
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

// Tables referenced
#include "ExportTable.h"
#include "DiskDescriptor.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

#include "HdmFcpTarget.h"
#include "Message.h"
#include "BuildSys.h"

#include "RqDdmReporter.h"
//#define PHS_REPORTER	// define this to activate the Reporter code

CLASSNAME(HdmFcpTarget, MULTIPLE);

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/

/*************************************************************************/
// HdmFcpTarget
// Constructor method for the class HdmFcpRac
/*************************************************************************/
HdmFcpTarget::HdmFcpTarget(DID did):HdmFcp(did) {

	TRACE_ENTRY(HdmFcpTarget::HdmFcpTarget);
	
	FCP_PRINT_HEX(TRACE_L6, "\n\rHdmFcpTarget::HdmFcpTarget this = ", (U32)this);
	FCP_PRINT_HEX(TRACE_L6, "\n\rHdmFcpTarget::HdmFcpTarget did = ", (U32)did);
	
	MyVd = GetVdn();
	MyDid = GetDid(); // stash this away for starting PHS Reporter later

	SetConfigAddress(&config, sizeof(config));
	
}	// HdmFcpTarget

/*************************************************************************/
// Ctor
// Create a new instance of the Fcp Nic Hdm
/*************************************************************************/
Ddm *HdmFcpTarget::Ctor(DID did) {

	TRACE_ENTRY(HdmFcpTarget::Ctor);

	return new HdmFcpTarget(did); 
}	// Ctor

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS HdmFcpTarget::Initialize(Message *pMsg) {

	STATUS			 status;
	U32				 chip;
	INSTANCE_DATA	*Id;
	
	TRACE_ENTRY(HdmFcpTarget::Initialize);
	
	// initialize member data
	Tar_TS_Export = NULL;
	Tar_Export = NULL;
	
	num_xport_entrys = 0;
	num_valid_exports = 0;
	
	// Allocate the hash table
	pXlt = new (_LunXlate * [HASH_TABLE_LEN]);
	
	for (int loop = 0; loop < HASH_TABLE_LEN; loop++)
	{
		// fill each entry with an invalid entry
		pXlt[loop] = (_LunXlate *)NULL;
	}

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

	// Start the FCP Target driver
	status = FCP_Start(Id);
	
	// Get our table data from the Persistent Data Store
	TarTableInitialize(pMsg);

// initialize our PHS reporter members
	m_pStatus = &Id->FCPTarget.PHS_Status;
	m_pPerformance = &Id->FCPTarget.PHS_Performance;

	memset(m_pStatus, 0, sizeof(FCPT_Status));
	memset(m_pPerformance, 0, sizeof(FCPT_Performance));
	
	fStatusReporterActive = false;
	fPerformanceReporterActive = false;



//	Ddm::Initialize(pMsg);
	return status;
	
}	// Initialize

/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
STATUS HdmFcpTarget::Enable(Message *pMsg) {

	TRACE_ENTRY(HdmFcpTarget::Enable);

	// must change the config data here also since it is re-read from the PTS
	if (config.version > 3)
	{
		config.loop_instance = FCLOOPINSTANCE( Address::iSlotMe, config.loop_instance );
	}
	
#ifdef PHS_REPORTER
// PHS Reporter ~~
	// We start the PHS Status and Performance Reporters here.
	// Start the Status Reporter
	RqDdmReporter *pReporterMsg;
	if (!fStatusReporterActive)
	{
 		pReporterMsg = new RqDdmReporter(PHS_START, PHS_FCP_TARGET_STATUS, MyDid, MyVd);
		Send(pReporterMsg, (ReplyCallback) &DiscardReply);
		fStatusReporterActive = true;
		// There is no need to clear the counters as the Reporter will issue
		// a PHS_RESET code when it is ready to begin collecting data.
	} /* if */

	// Start the Performance Reporter
	if (!fPerformanceReporterActive)
	{
	 	pReporterMsg = new RqDdmReporter(PHS_START, PHS_FCP_TARGET_PERFORMANCE, MyDid, MyVd);
		Send(pReporterMsg, (ReplyCallback) &DiscardReply);
		fPerformanceReporterActive = true;
		// There is no need to clear the counters as the Reporter will issue
		// a PHS_RESET code when it is ready to begin collecting data.
	} /* if */
	
#endif
		
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Enable

/*************************************************************************/
// Quiesce
// Turn off the DDM belonging to this derived class
/*************************************************************************/
STATUS HdmFcpTarget::Quiesce(Message *pMsg) {

	TRACE_ENTRY(HdmFcpTarget::Quiesce);

#ifdef PHS_REPORTER
// PHS Reporter ~~
	// We stop the PHS Status and Performance Reporters here.
	// Stop the Status Reporter
	RqDdmReporter *pReporterMsg;
	if (fStatusReporterActive)
	{
 		pReporterMsg = new RqDdmReporter(PHS_STOP, PHS_FCP_TARGET_STATUS, MyDid, MyVd);
		Send(pReporterMsg, (ReplyCallback) &DiscardReply);
		fStatusReporterActive = false;
	} /* if */

	// Stop the Performance Reporter
	if (fPerformanceReporterActive)
	{
	 	pReporterMsg = new RqDdmReporter(PHS_STOP, PHS_FCP_TARGET_PERFORMANCE, MyDid, MyVd);
		Send(pReporterMsg, (ReplyCallback) &DiscardReply);
		fPerformanceReporterActive = false;
	} /* if */
	
#endif

	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Quiesce

/*************************************************************************/
// DoWork
// This derived classes method to receive messages and replies
/*************************************************************************/
STATUS HdmFcpTarget::DoWork(Message *pMsg) {

	TRACE_ENTRY(HdmFcpTarget::DoWork);

	STATUS status=Ddm::DoWork(pMsg);
	
	TRACE_ENTRY(HdmFcpTarget::DoWork Ddm::DoWork);
	
	if (status != OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION)
		return status;

	// This HDM mostly handles Replies to messages sent from the Interrupt Handler
	if (pMsg->IsReply()) {
		FCP_EVENT_CONTEXT		*p_context;
		STATUS					 status;
		
	    FCP_DUMP_HEX(TRACE_L8, "\n\rHdmFcpTarget::DoWork Message Reply",
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
		case PHS_RESET_STATUS:
			TRACE_STRING(TRACE_L8, "\n\rHdmFcpTarget::DoWork PHS_RESET_STATUS");
			memset(m_pStatus, 0, sizeof(FCPT_Status));
		
			// reply to the caller
			status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
			return status;
	
		case PHS_RETURN_STATUS:
		{
			U8		*p;
			U32		cnt = sizeof(FCPT_Status);
		
			TRACE_STRING(TRACE_L8, "\n\rHdmFcpTarget::DoWork PHS_RETURN_STATUS");
		
			// return the Status Structure using a dynamic reply structure
			pMsg->GetSgl(DDM_REPLY_DATA_SGI, &p, &cnt);
		
			memcpy(p, m_pStatus, sizeof(FCPT_Status));
		
			// reply to the caller
			status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
			return status;
		}	
		case PHS_RESET_PERFORMANCE:
			TRACE_STRING(TRACE_L8, "\n\rHdmFcpTarget::DoWork PHS_RESET_PERFORMANCE");
		
			memset(m_pPerformance, 0, sizeof(FCPT_Performance));
		
			// reply to the caller
			status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
			return status;
			
		case PHS_RETURN_PERFORMANCE:
		{	
			U8		*p;
			U32		cnt = sizeof(FCPT_Performance);
		
			TRACE_STRING(TRACE_L8, "\n\rHdmFcpTarget::DoWork PHS_RETURN_PERFORMANCE");
		
			// return the Performance Structure using a dynamic reply structure
			pMsg->GetSgl(DDM_REPLY_DATA_SGI, &p, &cnt);
		
			memcpy(p, m_pPerformance, sizeof(FCPT_Performance));
		
			// reply to the caller
			status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
			return status;
		}	
		case PHS_RETURN_RESET_PERFORMANCE:
		{
			U8		*p;
			U32		cnt = sizeof(FCPT_Performance);
		
			TRACE_STRING(TRACE_L8, "\n\rHdmFcpTarget::DoWork PHS_RETURN_RESET_PERFORMANCE");
		
			// return the Performance Structure using a dynamic reply structure
			pMsg->GetSgl(DDM_REPLY_DATA_SGI, &p, &cnt);
		
			memcpy(p, m_pPerformance, sizeof(FCPT_Performance));
		
			// reply to the caller
			status = Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
		
			// Now we do the reset portion of the command.
			memset(m_pPerformance, 0, sizeof(FCPT_Performance));
		
			return status;
		}
					
		// handle this message for testing, pretend an ATIO came in
		case SCSI_SCB_EXEC:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
			
		default:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
	}

	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
}	// DoWork


// end of C++ derived class

