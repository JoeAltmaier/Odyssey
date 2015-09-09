/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LoopMonitorIsm.cpp
// 
// Description:
// This module is the FC Loop monitor ISM definition. This module uses
// FcpLoop commands to discover any changes in status of the loops
// attached. 
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmLoopMonitor/LoopMonitorIsm.cpp $
// 
// 11    2/07/00 9:24p Cchan
// Added a reply handler for reset message replies coming from
// TargetServer VDNs. (also fixed a potential memory leak of pMsg in
// ::DoWork, IsReply() handler)
// 
// 10    1/09/00 6:53p Mpanas
// Clear id/lun arrays before use
// 
// 9     1/09/00 5:00p Mpanas
// Fix LoopDescriptor ghosting problem
// 
// 8     12/21/99 1:48p Mpanas
// Add support for IOP Failover
// - make several modules IOP_LOCAL
// 
// 7     9/15/99 6:43p Mpanas
// Fix race condition, AE comes in before Initialize() is complete
// 
// 6     9/14/99 9:00p Mpanas
// Minor trace level changes
// 
// 5     8/14/99 9:28p Mpanas
// LoopMonitor version with most functionality
// implemented
// - Creates/Updates LoopDescriptor records
// - Creates/Updates FCPortDatabase records
// - Reads Export Table entries
// Note: This version uses the Linked List Container types
//           in the SSAPI Util code (SList.cpp)
// 
// 4     7/23/99 1:40p Mpanas
// Add latest versions of the Loop code
// so we can use the fix in FCP Lib
// for Immediate Notify handling
// 
// 1     6/18/99 6:46p Mpanas
// Initial version of the LoopMonitor
// for ISP2200 support
// 
// 
// 06/11/99 Michael G. Panas: Create file
/*************************************************************************/

#include "LmCommon.h"
#include "LoopMessages.h"

#include "Pci.h"
#include "Scsi.h"
#include "CDB.h"
#include "CTIdLun.h"
#include "BuildSys.h"
#include "Address.h"

#include "FcpData.h"
#include "FcpEvent.h"
#include "FcpLoop.h"
#include "FcpISP.h"
#include "FcpError.h"


CLASSNAME(LoopMonitorIsm, SINGLE);

/*************************************************************************/
// Forward references
/*************************************************************************/
extern "C" {
STATUS	LM_Handle_FC_AE(FCP_EVENT_CONTEXT *p_context);
}

/*************************************************************************/
// Global references
/*************************************************************************/
LoopMonitorIsm			*pLoopMonitorIsm = NULL;



/*************************************************************************/
// LoopMonitorIsm
// Constructor method for the class LoopMonitorIsm
/*************************************************************************/
LoopMonitorIsm::LoopMonitorIsm(DID did):Ddm(did) {

	TRACE_ENTRY(LoopMonitorIsm::LoopMonitorIsm);
	
	TRACE_HEX(TRACE_L6, "\n\rLoopMonitorIsm::LoopMonitorIsm this = ", (U32)this);
	TRACE_HEX(TRACE_L6, "\n\rLoopMonitorIsm::LoopMonitorIsm did = ", (U32)did);
	
	MyVd = GetVdn();
	
	m_config_done = 1;		// config not done
	
	pLoopMonitorIsm = this;	// save the first one
	
	SetConfigAddress(&config, sizeof(config));
	
} // LoopMonitorIsm

/*************************************************************************/
// Ctor
// Create a new instance of the Loop Monitor
/*************************************************************************/
Ddm *LoopMonitorIsm::Ctor(DID did) {

	TRACE_ENTRY(LoopMonitorIsm::Ctor);

	return new LoopMonitorIsm(did); 
}	// Ctor

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS LoopMonitorIsm::Initialize(Message *pMsg) {

	TRACE_ENTRY(LoopMonitorIsm::Initialize);
	
	// Initialize member values
	for (int i = 0; i < config.num_loops; i++)
	{
		LoopDescriptorEntry *pLD;
		// Old config had system loop # but in new we calculate it from local loop (chip) #.
		if (config.version > LM_CONFIG_VERSION)
			config.FC_instance[i] = FCLOOPINSTANCE( Address::iSlotMe, config.FC_instance[i] );
		
		U32					 chip = FCLOOPCHIP(config.FC_instance[i]);
		
		Lm_TS_Export[chip] = NULL;
		
		// initialize the Container for the FCPortDatabaseRecords
		pPDB[chip] = new SList;
		
		// initialize the Container for the Export Table status
		pETS[chip] = new SList;
		
		// NULL until the table update fills these in
		LM_TS_Loop_Desc[chip] = new(tPCI) LoopDescriptorEntry;
		memset(LM_TS_Loop_Desc[chip], 0, sizeof(LoopDescriptorEntry));

		// create the local LoopDescriptorEntry
		LM_Loop_Desc[chip] = new(tPCI) LoopDescriptorEntry;
		memset(LM_Loop_Desc[chip], 0, sizeof(LoopDescriptorEntry));
		
		// fill in the entry data we know about
		pLD = LM_Loop_Desc[chip];
		pLD->version = LOOP_DESCRIPTOR_TABLE_VERSION;
		pLD->size = sizeof(LoopDescriptorEntry);
		pLD->LoopNumber = config.FC_instance[i];
		pLD->slot = (TySlot)FCLOOPSLOT(config.FC_instance[i]);
		pLD->ChipNumber = chip;
		pLD->vdnLoopMonitor = MyVd;
		pLD->ChipType = Instance_Data[chip].ISP_Type;
		pLD->DesiredLoopState = LoopUp;
		pLD->ActualLoopState = LoopInvalid;
		
		// set flags
		if (Instance_Data[chip].FCP_config.enable_initiator_mode)
			pLD->flags |= LD_FLAGS_INITIATOR;
		if (Instance_Data[chip].FCP_config.enable_target_mode)
			pLD->flags |= LD_FLAGS_TARGET;
		
		// make sure our slot matches config slot
		if (pLD->slot != Address::iSlotMe)
		{
			Tracef("\nLoopMonitorIsm::Initialize: Configured slot (%d) does not match (%d)",pLD->slot ,Address::iSlotMe);
		}
		
		LoopFlags[chip] = LM_STS_LOOP_UNDEF;
		num_IDs[chip] = 0;
		num_LIPs[chip] = 0;
		num_resets[chip] = 0;
		nExportRows[chip] = 0;
		
		m_num_IDs[chip] = 0;
		m_ids[chip] = (U8 *)new char[32];
		memset(m_ids[chip], 0, 32);
		m_num_luns[chip] = (U8 *)new char[32];
		memset(m_num_luns[chip], 0, 32);
	}
	
	// Start the table state machine
	LmTableInitialize(pMsg);
	
	//Reply(pMsg); 		// don't return this until the last TS message

	return OS_DETAIL_STATUS_SUCCESS;
	
}	// Initialize

/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
STATUS LoopMonitorIsm::Enable(Message *pMsg) {
	
	TRACE_ENTRY(LoopMonitorIsm::Enable);
	
	for (int i = 0; i < config.num_loops; i++)
	{
		// fix our config data at enable() time too since the config record is re-read
		if (config.version > LM_CONFIG_VERSION)
			config.FC_instance[i] = FCLOOPINSTANCE( Address::iSlotMe, config.FC_instance[i] );
	}
	
	// config is now done
	m_config_done = 0;
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Enable

/*************************************************************************/
// Quiesce
// Turn off the DDM belonging to this derived class
/*************************************************************************/
STATUS LoopMonitorIsm::Quiesce(Message *pMsg) {
	
	TRACE_ENTRY(LoopMonitorIsm::Quiesce);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Quiesce

/*************************************************************************/
// DoWork
// This derived classes method to receive messages and replies
/*************************************************************************/
STATUS LoopMonitorIsm::DoWork(Message *pMsg) {

	TRACE_ENTRY(LoopMonitorIsm::DoWork);

	STATUS status=Ddm::DoWork(pMsg);
	
	TRACE_ENTRY(LoopMonitorIsm::DoWork Ddm::DoWork);
	
	if (status != OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION)
		return status;

	// This ISM handles Replies to messages sent from the monitor Handler
	if (pMsg->IsReply()) {
		
	    TRACE_DUMP_HEX(TRACE_L8, "\n\rLoopMonitorIsm::DoWork Message Reply",
	    					(U8 *)pMsg, 128);
		switch (pMsg->reqCode) {
		
		case DM_SCAN:
			// reply from DriveMonitor
			TRACE_STRING(TRACE_L3, "\n\rLoopMonitorIsm::DoWork: DM_SCAN reply");
			
			// get our chip number
			U32 chip = (U32)pMsg->GetContext();
			
			// loop is now up completely
 			LoopFlags[chip] = LM_STS_LOOP_UP;
			break;
		
		case LM_LOOP_UP:
		case LM_LOOP_LIP:
		case LM_LOOP_DOWN:
			break;		// do nothing for these
			
		case SCSI_DEVICE_RESET:
			// reset reply
			TRACE_STRING(TRACE_L3, "\n\rLoopMonitorIsm::DoWork: SCSI_DEVICE_RESET reply");
			
			// get chip number sent by the AE handler
			chip = (U32)pMsg->GetContext(); 
						
			// decrement reset counter
			num_resets[chip]--;
			
			break;
							
		default:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
		}
		
		// deallocate message reply
		delete pMsg;
	}
	
	// New service message has been received
	else {
	    TRACE_DUMP_HEX(TRACE_L8, "\n\rLoopMonitorIsm::DoWork Message",
	    					(U8 *)pMsg, 128);
		switch(pMsg->reqCode) {

		case LM_LOOP_UP:
		
		{
			LmLoopUp		*pLpMsg = (LmLoopUp *) pMsg;
			PINSTANCE_DATA	 Id = &Instance_Data[FCLOOPCHIP(pLpMsg->payload.instance)];
			
			// Handle a defered LoopUp
			LM_Handle_Loop_Up(Id);
			
			Reply(pMsg);
		}
			break;

		case LM_LOOP_LIP:
		
		{
			LmLoopLIP		*pLpMsg = (LmLoopLIP *) pMsg;
			PINSTANCE_DATA	 Id = &Instance_Data[FCLOOPCHIP(pLpMsg->payload.instance)];
			
			// Handle a defered LoopLIP
			LM_Handle_Loop_LIP(Id);
			
			Reply(pMsg);
		}
			break;

		case LM_LOOP_DOWN:
		
		{
			LmLoopDown		*pLpMsg = (LmLoopDown *) pMsg;
			PINSTANCE_DATA	 Id = &Instance_Data[FCLOOPCHIP(pLpMsg->payload.instance)];
			
			// Handle a defered LoopDown
			LM_Handle_Loop_Down(Id);
			
			Reply(pMsg);
		}
			break;

		case LM_ENABLE_ID:		// DO we actually get any of these?
		{
			LM_CONTEXT	*pLmc = new LM_CONTEXT;
			
			// set the callback address
			pLmc->Callback = (pLMCallback_t) &LM_Do_Callback;
			
			// TODO:
			// finish this
			LM_Enable_Loop_ID(pLmc, 0, 0);
		}
			break;

		case LM_DISABLE_ID:
		{
			LM_CONTEXT	*pLmc = new LM_CONTEXT;
			
			// set the callback address
			pLmc->Callback = (pLMCallback_t) &LM_Do_Callback;
			
			// TODO:
			// finish this
			LM_Disable_Loop_ID(pLmc, 0, 0);
		}
			break;

		case LM_SCAN:
			LM_Scan_Loops(pMsg);
			break;
	
		default:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
		}
	}

	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
}	// DoWork


/*************************************************************************/
// LM_LIP
// Re-scan the specified loop by sending an LIP
/*************************************************************************/
void LoopMonitorIsm::LM_LIP(U32 chip)
{
	LM_CONTEXT		*pLmc;
	PINSTANCE_DATA	 Id = &Instance_Data[chip];
	STATUS			 status;
	
	TRACE_ENTRY(LM_LIP);

	status = FCP_Initiate_LIP(Id);
	
	if (status != MB_STS_GOOD)
	{
		TRACE_HEX(TRACE_L2, "\n\rLM_LIP: LIP Failed - ", status);
	}
	
} // LM_LIP


/*************************************************************************/
// LM_Get_Pos_Map
// Print the FC_AL Position map
/*************************************************************************/
void LoopMonitorIsm::LM_Get_Pos_Map(U32 chip)
{
	LM_CONTEXT		*pLmc;
	U8				*buffer = (U8 *)new char[128];
	PINSTANCE_DATA	 Id = &Instance_Data[chip];
	STATUS			 status;
	U16				 type = 0;
	
	TRACE_ENTRY(LM_Get_Pos_Map);

	status = FCP_Get_FC_AL_Position_Map(Id, buffer, &type);
	
	if (status != MB_STS_GOOD)
	{
		TRACE_HEX(TRACE_L2, "\n\rLM_Get_Pos_Map: Failed - ", status);
	}
	else
	{
		TRACE_HEX16(TRACE_L2, "\n\rMap Type = ", type);
		TRACE_DUMP_HEX(TRACE_L2, "\n\rFC_AL Map:", buffer, 128);
	}
	
} // LM_Get_Pos_Map


/*************************************************************************/
// LM_Get_State
// Get the state of the loop to the U16 pointed to by *state
/*************************************************************************/
U32 LoopMonitorIsm::LM_Get_State(U32 chip, U16 *state)
{
	LM_CONTEXT		*pLmc;
	PINSTANCE_DATA	 Id = &Instance_Data[chip];
	STATUS			 status;
	
	TRACE_ENTRY(LM_Get_State);

	status = FCP_Get_Firmware_State(Id, state);
	
	if (status != MB_STS_GOOD)
	{
		TRACE_HEX(TRACE_L2, "\n\rLM_Get_State: LIP Failed - ", status);
	}
	
	return(*state);
} // LM_Get_State

/*************************************************************************/
// LM_Do_Callback
// finish operation requested
/*************************************************************************/
void LoopMonitorIsm::LM_Do_Callback(void *ptr)
{
	LM_CONTEXT		*pLmc = (LM_CONTEXT *) ptr;
	
	TRACE_ENTRY(LM_Do_Callback);

	// delete context we are done with it
	delete pLmc;
	
} // LM_Do_Callback


