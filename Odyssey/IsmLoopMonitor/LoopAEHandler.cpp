/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LoopAEHandler.cpp
// 
// Description:
// This module handles Asyncrhonous Events for the Loop Monitor.
//
// These calls come from the FCP Library when an async event is flagged
// by an interrupt coming in.
//		STATUS LM_Handle_FC_AE(FCP_EVENT_CONTEXT *p_context)
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmLoopMonitor/LoopAEHandler.cpp $
// 
// 4     2/07/00 9:20p Cchan
// Added code to send reset messages to TargetServer VDNs when LIP reset
// occurs on the loop. 
// 
// 3     9/15/99 6:43p Mpanas
// Fix race condition, AE comes in before Initialize() is complete
// 
// 2     8/14/99 9:28p Mpanas
// LoopMonitor version with most functionality
// implemented
// - Creates/Updates LoopDescriptor records
// - Creates/Updates FCPortDatabase records
// - Reads Export Table entries
// Note: This version uses the Linked List Container types
//           in the SSAPI Util code (SList.cpp)
// 
// 1     7/23/99 1:39p Mpanas
// Add latest versions of the Loop code
// 
// 
// 07/17/99 Michael G. Panas: Create file
/*************************************************************************/

#include "LmCommon.h"
#include "LoopMessages.h"

#include "Pci.h"
#include "Scsi.h"
#include "CDB.h"
#include "CTIdLun.h"
#include "BuildSys.h"

#include "FcpData.h"
#include "FcpEvent.h"
#include "FcpLoop.h"
#include "FcpISP.h"
#include "FcpError.h"
#include "FcpProto.h"


/*************************************************************************/
// Forward references
/*************************************************************************/


/*************************************************************************/
// Global references
/*************************************************************************/
extern LoopMonitorIsm			*pLoopMonitorIsm;
U32		FCDbState = FC_PORT_STATUS_LOOP_DOWN;

//=========================================================================
// LoopMonitor
// Asyncronous Event Handling
//
//=========================================================================


/*************************************************************************/
// LM_Handle_Loop_Down
// Called to handle the Loop Up Asynchronous Event.
/*************************************************************************/
void LoopMonitorIsm::LM_Handle_Loop_Down(PINSTANCE_DATA	 Id)
{
	STATUS 			 		 status = NU_SUCCESS;
	U32				 		 chip = Id->FCP_chip;
	FCPortDatabaseRecord	*pDb;
	LM_FCDB_STATE			*pFCDBState;

 	TRACE_ENTRY(LM_Handle_Loop_Down);
 	
  	// check if we are configured yet
 	if (m_config_done)
 	{
 		// not configured, send ourselves a message for later
 		LmLoopDown *pMsg = new LmLoopDown;
		pMsg->payload.instance = Id->FCP_loop;
	
		Send((VDN)MyVd, pMsg);
		
		return;
	}
 	
 	// Flag the loop down
 	LoopFlags[chip] = LM_STS_LOOP_DOWN;
	LM_Loop_Desc[chip]->ActualLoopState = LoopDown;
 	
 	// TODO:
 	// turn off all the export entries on this loop
 	
 	// mark all FCPortDatabase entries as FC_PORT_STATUS_LOOP_DOWN
	for (int x = pPDB[chip]->Count(); x--;)
	{
		// get each container in sequence
		pPDB[chip]->GetAt((CONTAINER_ELEMENT &)pFCDBState, x);
		
		// mark as loop down
		pFCDBState->pDBR->portStatus = FC_PORT_STATUS_LOOP_DOWN;
		
		// update an entry in the FCPortDatabaseRecord Table
		// no Callback, no state, just do it.
		LmDBTableUpdate(pFCDBState, NULL, (pLMCallback_t)NULL);
	}
 	
 	// Update LoopDescriptor
	LmLpTableUpdate(NULL, NULL, Id->FCP_loop, (pLMCallback_t) NULL);
 	
} // LM_Handle_Loop_Down

/*************************************************************************/
// LM_Handle_Loop_LIP
// Called to handle the Loop LIP Asynchronous Event.
/*************************************************************************/
void LoopMonitorIsm::LM_Handle_Loop_LIP(PINSTANCE_DATA	 Id)
{
	STATUS 			 status = NU_SUCCESS;
	U32				 chip = Id->FCP_chip;

 	TRACE_ENTRY(LM_Handle_Loop_LIP);
 	
 	// check if we are configured yet
 	if (m_config_done)
 	{
 		// not configured, send ourselves a message for later
 		LmLoopLIP *pMsg = new LmLoopLIP;
		pMsg->payload.instance = Id->FCP_loop;
	
		Send((VDN)MyVd, pMsg);
		
		return;
	}
 	
 	// bump the number of LIPs
 	// TODO: check the number for too many
	num_LIPs[chip]++;

 	// check if the loop is up
 	if (LoopFlags[chip] == LM_STS_LOOP_UP)
 	{
 		// scan the FC Loop if we are up
 		LM_Scan_A_Loop(Id->FCP_loop);
 	}
 	else
 	{
	 	// Flag an LIP came when the loop was not up
	 	LoopFlags[chip] = LM_STS_LOOP_LIP;
 	}
 	
 	
} // LM_Handle_Loop_LIP

/*************************************************************************/
// LM_Handle_Loop_Up
// Called to handle the Loop Up Asynchronous Event.
/*************************************************************************/
void LoopMonitorIsm::LM_Handle_Loop_Up(PINSTANCE_DATA	 Id)
{
	STATUS 			 status = NU_SUCCESS;
	U32				 chip = Id->FCP_chip;

 	TRACE_ENTRY(LM_Handle_Loop_Up);
 	
  	// check if we are configured yet
 	if (m_config_done)
 	{
 		// not configured, send ourselves a message for later
 		LmLoopUp *pMsg = new LmLoopUp;
		pMsg->payload.instance = Id->FCP_loop;
	
		Send((VDN)MyVd, pMsg);
		
		return;
	}
 	
	// check if an LIP came in before the loop
 	// was flagged as up
 	if (LoopFlags[chip] == LM_STS_LOOP_LIP)
 	{
	 	// Flag the loop up before the scan
	 	LoopFlags[chip] = LM_STS_LOOP_UP;
	 	
 		// scan the FC Loop
 		LM_Scan_A_Loop(Id->FCP_loop);
 	}
 	else
 	{
 		// Flag the loop as up now
 		LoopFlags[chip] = LM_STS_LOOP_UP;
 	}
 	
} // LM_Handle_Loop_Up


/*************************************************************************/
// LM_Handle_Loop_LIP_Reset
// Called to handle the Loop LIP Reset Asynchronous Event.
/*************************************************************************/
void LoopMonitorIsm::LM_Handle_Loop_LIP_Reset(PINSTANCE_DATA Id)
{
	STATUS 			 status = NU_SUCCESS;
	U32				 chip = Id->FCP_chip;
	LM_ET_STATE		 *pETState;

 	TRACE_ENTRY(LM_Handle_Loop_LIP_Reset);
 	
  	// check if we are configured yet
 	if (m_config_done)
 	{
 		// not configured, just return
		return;
	}
	
	if (num_resets[chip] > 0) {
	
		// warning posted on trace if there are pending reset messages
		TRACEF(TRACE_L2, ("\n\rWarning: reset counter is %x", num_resets[chip]));
		
		return;	
	}
		
	// go through the LM_ET_STATE container for each VDN on the loop
	for (int i = pETS[chip]->Count()-1; i >= 0; i--) {
		
		// allocate new reset messages
		Message *pMsg = new Message(SCSI_DEVICE_RESET);	
		pETS[chip]->GetAt((CONTAINER_ELEMENT &)pETState,i); // get VDN info
		
		// send message with the chip number
		status = Send((VDN)pETState->vdNext, pMsg, (void *)chip);
		
		if (status == OK)

			num_resets[chip]++; // increment reset counter if Send successful
	
		else 
			TRACEF(TRACE_L2, ("\n\rSend LIP Reset to VDN %x failed",pETState->vdNext));

	}

	LoopFlags[chip] = LM_STS_LOOP_ERRORS;
 	
} // LM_Handle_Loop_LIP_Reset


//=========================================================================
// External entry
//=========================================================================

/*************************************************************************/
// LM_Handle_FC_AE
// Called to handle Asynchronous Events.  These may or may not Throw an
// event depending on who is listening.
/*************************************************************************/
STATUS LM_Handle_FC_AE(FCP_EVENT_CONTEXT *p_context)
{
	STATUS 			 status = NU_SUCCESS;
	PINSTANCE_DATA	 Id = p_context->Id;
	char			*s;
	
 	TRACE_ENTRY(LM_Handle_FC_AE);
 	
 	// display which type
 	if (Id->FCP_config.enable_target_mode &&
 						Id->FCP_config.enable_initiator_mode)
 		s = "TargetInit";
 	else if (Id->FCP_config.enable_target_mode)
 		s = "Target";
 	else
 		s = "Initiator";
	TRACEF(TRACE_L2, ("\n\rAE(%d) %s ",Id->FCP_loop, s));
 	
	// Switch depending on the state of the context.
	switch (p_context->action)
	{
		// TODO:
		// handle these AEs
		case FCP_ACTION_HANDLE_LOOP_UP:
		TRACE_STRING(TRACE_L2, "Loop Up");
		pLoopMonitorIsm->LM_Handle_Loop_Up(Id);
		break;
		
		case FCP_ACTION_HANDLE_LOOP_DOWN:
		TRACE_STRING(TRACE_L2, "Loop Down");
		pLoopMonitorIsm->LM_Handle_Loop_Down(Id);
		break;
		
		case FCP_ACTION_HANDLE_LIP:
		TRACE_STRING(TRACE_L2, "LIP");
		pLoopMonitorIsm->LM_Handle_Loop_LIP(Id);
		break;

		case FCP_ACTION_HANDLE_LIP_RESET:
		TRACE_STRING(TRACE_L2, "LIP Reset");
		pLoopMonitorIsm->LM_Handle_Loop_LIP_Reset(Id);
		break;
		case FCP_ACTION_HANDLE_OTHER_AE:
		TRACE_STRING(TRACE_L2, "Other AE");
		break;
		case FCP_ACTION_HANDLE_THROW_EVENT:
		TRACE_STRING(TRACE_L2, "Throw Event");
		break;
	}
				
	// in all cases we are done with this context
    // Deallocate the FCP_EVENT_CONTEXT 
    // allocated by FCP_Handle_Async_Event
    status = NU_Deallocate_Partition(p_context);
    if (status != NU_SUCCESS)
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Handle_AE_Target", 
			"NU_Deallocate_Partition for context failed",
			status,
			(UNSIGNED)p_context);
					
	return status;
	
} // LM_Handle_FC_AE


