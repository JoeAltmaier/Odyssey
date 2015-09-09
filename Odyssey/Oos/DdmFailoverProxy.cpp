/* DdmFailoverProxy.cpp -- The Master of failover
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
 *		Supports Failover when using DdmPtsProxy service.
 *
**/
 
// Revision History: 
//  6/27/99 Tom Nelson: Create file
//  7/08/99 Joe Altmaier: expunge VirtualProxy class names from macros
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_FAILOVER 
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>

// Private Includes
#include "DdmManager.h"
#include "ServeTable.h"
#include "SystemTable.h"
#include "BootTable.h"
#include "BuildSys.h"

#include "DdmFailoverProxy.h"
#include "RqOsDdmManager.h"
#include "RqOsVirtualManager.h"

// BuildSys Linkage

CLASSNAME(DdmFailoverProxy,SINGLE);

//SERVELOCAL(DdmFailoverProxy,RqOsVirtualManagerGetConfig::RequestCode);


// .DdmFailoverProxy -- Constructor ---------------------------------------DdmFailoverProxy-
//
DdmFailoverProxy::DdmFailoverProxy(DID did) : Ddm(did) {

TRACEF(TRACE_L8, ("EXEC  DdmFailoverProxy::DdmFailoverProxy\n"));

//	DispatchRequest(RqOsVirtualManagerGetConfig::RequestCode, REQUESTCALLBACK(DdmFailoverProxy, ProcessGetConfig));
}

// .Initialize -- Process Initialize -------------------------------------DdmFailoverProxy-
//
STATUS DdmFailoverProxy::Initialize(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmFailoverProxy::Initialize;\n"));
	
	Reply(pArgMsg,OK);	// Initialize Complete
			
	return OK;
}

// .Enable -- Process Enable ---------------------------------------------DdmFailoverProxy-
//
STATUS DdmFailoverProxy::Enable(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmFailoverProxy::Enable;\n"));
	
	RqOsVirtualManagerPingSlot *pMsg = new RqOsVirtualManagerPingSlot;
	
	// Check to see if VirtualManager is Enable by sending it PingSlot request.
	Send(pMsg,(void*)pArgMsg, REPLYCALLBACK(DdmFailoverProxy,ProcessGetPtsDidReply));
	
	return OK;
}

// .ProcessGetPtsDidReply -- Process Reply -------------------------------DdmFailoverProxy-
//
// DdmManager is now enabled since we received this reply.
//
ERC DdmFailoverProxy::ProcessGetPtsDidReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmFailoverProxy::ProcessGetPtsDidReply\n"));

	Message *pMsgEnable = (Message *)pArgMsg->GetContext();
	Message *pMsg;
	delete pArgMsg;
	
	// Start Heartbeat clock
	pMsgHeartbeat = new RqOsTimerStart(5000000,5000000);		// 5-second interval
	Send(pMsgHeartbeat, REPLYCALLBACK(DdmFailoverProxy,ProcessHeartbeatReply));
	
	// Listen to PtsProxy for IOP Status changes
	pMsg = new RqOsVirtualMasterListenIopState;
	Send(pMsg, REPLYCALLBACK(DdmFailoverProxy,ProcessListenIopStateReply));
#if 0	
	// Listen to PtsProxy for VDN changes
	pMsg = new RqOsPtsProxyListenVdn;
	Send(pMsg, REPLYCALLBACK(DdmFailoverProxy,ProcessListenVdnReply));
#endif		
	Reply(pMsgEnable,OK);	// Now Enabled

	return OK;
}


// .ProcessListenIopStateReply -- Process  Replies -----------------------DdmFailoverProxy-
//
// Listen to PtsProxy for changes in the IopState Table
//
ERC DdmFailoverProxy::ProcessListenIopStateReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmFailoverProxy::ProcessIopStateReply\n"));
	
	RqOsVirtualMasterListenIopState *pReply = (RqOsVirtualMasterListenIopState *) pArgMsg;
	States *pStates = pReply->GetStatesPtr();
	
	// Display IOP States
#if 0
	Tracef("[FailoverProxy]\n");
	Tracef("IOP:");
	for (U32 ii=0; ii < States::MAXSLOTS; ii++)
		if (pStates->stateOs[ii] != proxyIOPNONE)
			Tracef(" %4u",ii);

	Tracef("\n    ");
	for (U32 ii=0; ii < States::MAXSLOTS; ii++)
		if (pStates->stateOs[ii] !=proxyIOPNONE)
			Tracef(" %4.4s",States::StateName(pStates->stateOs[ii]));
	
	Tracef("\n");
#endif

	// Save IOP states
	stateIop = *pStates;
	
	delete pReply;
	
	return OK;
}
#if 0
// .ProcessListenVdnReply -- Process ListenVdn Replies -------------------DdmFailoverProxy-
//
// Listen to PtsProxy for changes in the VirtualDeviceTable
//
// Tell DdmManager of new Vdn/Did mapping.
// DdmManager will ignore if matches current mapping.
//
ERC DdmFailoverProxy::ProcessListenVdnReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmFailoverProxy::ProcessListenVdnReply\n"));

	RqOsPtsProxyListenVdn *pReply = (RqOsPtsProxyListenVdn *) pArgMsg;
TRACEF(TRACE_L7, ("LISTEN vdn=%u; fPrimary=%u; didPrimary=%x; didSecondary=%x (DdmFailoverProxy::ProcessListenVdnReply)\n",
				   pReply->payload.vdn,pReply->payload.fPrimary,pReply->payload.didPrimary,pReply->payload.didSecondary));
				   
	if (pReply->payload.fNewPrimary)
		vdnMap.Set(pReply->payload.vdn,TRUE,pReply->payload.didPrimary);
			
	if (pReply->payload.fNewSecondary)
		vdnMap.Set(pReply->payload.vdn,FALSE,pReply->payload.didSecondary);

	delete pArgMsg;
	
	return OK;
}
#endif
// .ProcessHeartbeatReply -- Process ListenVdn Replies -------------------DdmFailoverProxy-
//
// Listen to PtsProxy for changes in the VirtualDeviceTable
//
// Tell DdmManager of new Vdn/Did mapping.
// DdmManager will ignore if matches current mapping.
//
ERC DdmFailoverProxy::ProcessHeartbeatReply(Message *pArgMsg) {

//TRACEF(TRACE_L8, ("EXEC  DdmFailoverProxy::ProcessHeartbeatReply\n"));
	Message *pMsg;
	
	for (U32 ii=0; ii < States::MAXSLOTS; ii++) {
		if (pingIop.fPending[ii]) {	// FAILED!
			pingIop.fPending[ii] = FALSE;
			
			// Change OS state
//			UpdateVsrState((TySlot) ii,proxyIOPDOWN);
			
			// Tell VirtualMaster to fail IOP
			pMsg = new RqOsVirtualMasterFailSlot((TySlot) ii);
			Send(pMsg, REPLYCALLBACK(DdmFailoverProxy,DiscardOkReply));
			Tracef("**\n** Failing slot %u\n",ii);
		}
#if 0	// OOPS, Needs to be included and now use iopStatus[]
		else if (stateIop.stateOs[ii] == proxyIOPBOOT || stateIop.stateOs[ii] == proxyIOPUP) {
			// Ping the slot
			pingIop.fPending[ii] = TRUE;
			pMsg = new RqOsDdmManagerPingSlot;
			Send((TySlot) ii, pMsg, REPLYCALLBACK(DdmFailoverProxy,ProcessPingReply));
		}
#endif
	}
		
	delete pArgMsg;
	
	return OK;
}


// .ProcessPingReply -- Process  Replies ----------------------------------DdmFailoverProxy-
//
// Note which ping returned.
//
ERC DdmFailoverProxy::ProcessPingReply(Message *pArgMsg) {

//TRACEF(TRACE_L8, ("EXEC  DdmFailoverProxy::ProcessPingReply\n"));

	RqOsDdmManagerPingSlot *pReply = (RqOsDdmManagerPingSlot *) pArgMsg;
	
	TySlot iSlot = DeviceId::ISlot(pReply->payload.did);

	pingIop.fPending[iSlot] = FALSE;
			
	delete pReply;
	
	return OK;
}

