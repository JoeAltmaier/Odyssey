/* DdmIopFailoverMaster.cpp 
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Description:
 *		Diagnose failover conditions and take an IOP out of service when necessary
 *
**/
 
// Revision History: 
//  6/27/99 Tom Nelson: Create file
//  7/08/99 Joe Altmaier: expunge VirtualProxy class names from macros
// 10/25/99 Bob Butler: Used DdmFailoverProxy as the basis for the IOP FO master
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_FAILOVER 
#include "Odyssey_Trace.h"


// Private Includes
#include "BuildSys.h"

#include "DdmIopFailoverMaster.h"
#include "BootMgrMsgs.h"
// #include "RqOsVirtualManager.h"

// BuildSys Linkage

CLASSNAME(DdmIopFailoverMaster,SINGLE);


// .DdmIopFailoverMaster -- Constructor ---------------------------------------DdmIopFailoverMaster-
//
DdmIopFailoverMaster::DdmIopFailoverMaster(DID did) : Ddm(did) 
{
	TRACE_ENTRY("DdmIopFailoverMaster::DdmIopFailoverMaster\n");
}

// .Initialize -- Process Initialize -------------------------------------DdmIopFailoverMaster-
//
STATUS DdmIopFailoverMaster::Initialize(Message *pArgMsg) 
{
	TRACE_ENTRY("DdmIopFailoverMaster::Initialize;\n");
	
	Reply(pArgMsg,OK);	// Initialize Complete
			
	return OK;
}

// .Enable -- Process Enable ---------------------------------------------DdmIopFailoverMaster-
//
STATUS DdmIopFailoverMaster::Enable(Message *pArgMsg) 
{
	Message *pMsgEnable = (Message *)pArgMsg->GetContext();
	
	// Start Heartbeat clock
	pMsgHeartbeat = new RqOsTimerStart(1000000,1000000);		// 1-second interval
	Send(pMsgHeartbeat, REPLYCALLBACK(DdmIopFailoverMaster,ProcessHeartbeatReply));
	
	// Listen to Iop Status Table for IOP Status changes
	Message *pMsg = new RqOsVirtualMasterListenIopState;	// Listen for entire table,Insert,Delete
	Send(pMsg, REPLYCALLBACK(VCMapper,ProcessListenIopStateReply));

	Reply(pArgMsg,OK);	// Now Enabled

	return OK;
}


// .ProcessListenIopStateReply -- Process  Replies -----------------------DdmIopFailoverMaster-
//
//
ERC DdmIopFailoverMaster::ProcessListenIopStateReply(Message *pArgMsg) 
{
	TRACE_ENTRY("DdmIopFailoverMaster::ProcessListenIopStateReply\n");
	
	RqOsVirtualMasterListenIopState *pReply = (RqOsVirtualMasterListenIopState *) pArgMsg;
	
	// Save IOP states
	stateIop = *pReply->GetStatesPtr();
	
	delete pReply;
	
	return OK;
}

// .ProcessHeartbeatReply -- Process Timer Replies -------------------DdmIopFailoverMaster-
//
ERC DdmIopFailoverMaster::ProcessHeartbeatReply(Message *pArgMsg) 
{
	Message *pMsg;
	
	if (Address::iSlotHbcMaster == Address::iSlotMe)
	{
		for (U32 ii=0; ii < States::MAXSLOTS; ii++) 
		{
			if (pingIop.cTimeouts[ii] >= MAXTIMEOUTS) 	// FAILED!
			{
				pingIop.cTimeouts[ii] = 0;
				pingIop.fFailPending[ii] = TRUE;
				
				pMsg = new MsgIopOutOfService((TySlot)ii);
				Send(pMsg, REPLYCALLBACK(DdmIopFailoverMaster,ProcessIopOutOfServiceReply));
				Tracef("**\n** Failing slot %u\n",ii);
			}
			else if (stateIop.stateIop[ii] == IOPS_OPERATING && !pingIop.fFailPending[ii]) { 
				if (!pingIop.fReplyPending[ii]) {
					// Ping the slot
					pingIop.cTimeouts[ii] = 0;
					pMsg = new RqOsDdmManagerPingSlot;
					Send((TySlot) ii, pMsg, REPLYCALLBACK(DdmIopFailoverMaster,ProcessPingReply));
				}
				else
					++pingIop.cTimeouts[ii];
			}
		}
	}
			
	delete pArgMsg;
	
	return OK;
}


// .ProcessPingReply -- Process  Replies ----------------------------------DdmIopFailoverMaster-
//
// reset ReplyPending flag and fail the IOP if send failed
//
ERC DdmIopFailoverMaster::ProcessPingReply(Message *pArgMsg) 
{
	RqOsDdmManagerPingSlot *pReply = (RqOsDdmManagerPingSlot *) pArgMsg;
	
	TySlot iSlot = DeviceId::ISlot(pReply->payload.did);
	
	
	pingIop.fReplyPending[iSlot] = FALSE;
	pingIop.cTimeouts[iSlot] = 0;

	if (pReply->DetailedStatusCode != OK && !pingIop.fFailPending)
	{
		Message *pMsg = new MsgIopOutOfService(iSlot);
		Send(pMsg, REPLYCALLBACK(DdmIopFailoverMaster,ProcessIopOutOfServiceReply));
		Tracef("**\n** Failing slot %u\n",iSlot);
		pingIop.fFailPending[iSlot] = TRUE;
	}
				
	delete pReply;
	
	return OK;
}

// .ProcessIopOutOfServiceReply -- Process  Replies ----------------------------------DdmIopFailoverMaster-
//
ERC DdmIopFailoverMaster::ProcessIopOutOfServiceReply(Message *pArgMsg) 
{
	MsgIopOutOfService *pReply = (MsgIopOutOfService *)pArgMsg;
	
	// There may be a race condition here -- what if this flag gets reset, but the
	// IOP state is still operational?  It shouldn't matter, but if we try to ping again
	// and then fail, we may try to take it out of service again.  BootManager should 
	// handle that case since it is already possible to get OutOfService Msgs from more
	// than one source.
	
	pingIop.fFailPending[pReply->GetSlot()] = false;
	delete pReply;
	
	return OK;
}