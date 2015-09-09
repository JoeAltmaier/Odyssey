/* DdmFailoverMaster.cpp -- The Master of failover
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
**/
 
// Revision History: 
//  6/27/99 Tom Nelson: Create file
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_DDM_MGR
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>

// Private Includes
#include "DdmManager.h"
#include "ServeTable.h"
#include "SystemTable.h"
#include "BootTable.h"
#include "BuildSys.h"

#include "DdmVirtualProxy.h"

// BuildSys Linkage

CLASSNAME(DdmVirtualProxy,SINGLE);

SERVELOCAL(DdmVirtualProxy,RqOsVirtualManagerGetConfig::RequestCode);


// Statics

VddItem * VddItem::pFirst = NULL;
VddItem * VddItem::pLast = NULL;

// .DdmVirtualProxy -- Constructor ---------------------------------------DdmVirtualProxy-
//
DdmVirtualProxy::DdmVirtualProxy(DID did) : Ddm(did) {

TRACEF(TRACE_L8, ("EXEC  DdmVirtualProxy::DdmVirtualProxy\n"));

	DispatchRequest(RqOsVirtualManagerGetConfig::RequestCode, REQUESTCALLBACK(DdmVirtualProxy, ProcessGetConfig));
}

// .Initialize -- Process Initialize -------------------------------------DdmVirtualProxy-
//
STATUS DdmVirtualProxy::Initialize(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmVirtualProxy::Initialize;\n"));
	
	Reply(pArgMsg,OK);	// Initialize Complete
			
	return OK;
}

// .Enable -- Process Enable ---------------------------------------------DdmVirtualProxy-
//
STATUS DdmVirtualProxy::Enable(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmVirtualProxy::Enable;\n"));
	
	RqOsManagerGetPtsDid *pMsg = new RqOsManagerGetPtsDid;
	
	// Check to see if DdmManager is Enable by sending a GetPtsDid request.
	Send(pMsg,(void*)pArgMsg, REPLYCALLBACK(DdmVirtualProxy,ProcessGetPtsDidReply));
	
	return OK;
}

// .ProcessGetPtsDidReply -- Process Reply -------------------------------DdmVirtualProxy-
//
// DdmManager is now enabled since we received this reply.
//
ERC DdmVirtualProxy::ProcessGetPtsDidReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmVirtualProxy::ProcessGetPtsDidReply\n"));

	Message *pMsgEnable = (Message *)pArgMsg->GetContext();
	Message *pMsg;
	delete pArgMsg;
	
	// Tell PtsProxy we are booting!
	pMsg = new RqOsPtsProxySetIopState(Address::iSlotMe,proxyIOPBOOT);
	Send(pMsg, REPLYCALLBACK(DdmVirtualProxy,DiscardReply));
	
	// Listen to PtsProxy for IOP Status changes
	pMsg = new RqOsPtsProxyListenIopState;
	Send(pMsg, REPLYCALLBACK(DdmVirtualProxy,ProcessListenIopStateReply));
	
	// Make PxGetVdd request to PtsProxy
	pMsg = new RqOsPtsProxyGetVdd;
	Send(pMsg, REPLYCALLBACK(DdmVirtualProxy,ProcessGetVddReply));
	
	// Listen to PtsProxy for VDN changes
	pMsg = new RqOsPtsProxyListenVdn;
	Send(pMsg, REPLYCALLBACK(VirtualProxy,ProcessListenVdnReply));
	
	// Listen to PtsProxy for Routing changes
	pMsg = new RqOsPtsProxyListenRoute;
	Send(pMsg, REPLYCALLBACK(VirtualProxy,ProcessListenRouteReply));
	
	Reply(pMsgEnable,OK);	// Now Enabled

	return OK;
}

// .ProcessListenIopStateReply -- Process  Replies -----------------------DdmVirtualProxy-
//
// Listen to PtsProxy for changes in the IopState Table
//
ERC DdmVirtualProxy::ProcessListenIopStateReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmVirtualProxy::ProcessListenVdnReply\n"));
	
	RqOsPtsProxyListenIopState *pReply = (RqOsPtsProxyListenIopState *) pArgMsg;
	
	// Display IOP States
	Tracef("IOP:");
	for (U32 ii=0; ii < RqOsPtsProxyListenIopState::Payload::MAXSLOTS; ii++)
		if (pReply->payload.state[ii] != proxyIOPNONE)
			Tracef(" %4u",ii);

	Tracef("\n    ");
	for (U32 ii=0; ii < RqOsPtsProxyListenIopState::Payload::MAXSLOTS; ii++)
		if (pReply->payload.state[ii] !=proxyIOPNONE)
			Tracef(" %4.4s",RqOsPtsProxyListenIopState::Payload::StateName(pReply->payload.state[ii]));
	
	Tracef("\n");

	// When all IOPs are ready then start Ddms.
	for (U32 ii=0; ii < RqOsPtsProxyListenIopState::Payload::MAXSLOTS; ii++) {
		if (pReply->payload.state[ii] != proxyIOPNONE) {
			if (pReply->payload.state[ii] != proxyIOPUP) {
				delete pReply;
				return OK;
			}
		}
	}
	// All slots are ready, now start!
	for (VddItem *pVdd=VddItem::GetFirst(); pVdd != NULL; pVdd = pVdd->GetNext() ) {
		if (pVdd->fLoaded && pVdd->payload.fStart) {
TRACEF(TRACE_L7, ("START #%u; iSlot=%lx; didPrimary=%06x; didSecondary=%06x \"%s\" \n",
			  pVdd->payload.iEntry,Address::iSlotMe,pVdd->payload.didPrimary,pVdd->payload.didSecondary,pVdd->payload.szClassName));
			RqOsManagerStartDid *pMsgStart = new RqOsManagerStartDid;
		
			pMsgStart->payload.did = pVdd->did;
			Send(pMsgStart,REPLYCALLBACK(DdmVirtualProxy,DiscardReply));
		}
		VddItem::Free();
	}	

	return OK;
}

// .ProcessGetVddReply -- Process GetVdd Replies -------------------------DdmVirtualProxy-
//
// Discard Vdd entries not for our slot.
// Tell DdmManager to load our entries
//
ERC DdmVirtualProxy::ProcessGetVddReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("ENTER DdmVirtualProxy::ProcessGetVddReply\n"));

	RqOsPtsProxyGetVdd *pReply = (RqOsPtsProxyGetVdd*) pArgMsg;
	// Valid Entry and it's our slot!
	RqOsPtsProxyGetVdd::Payload *pEntry = &pReply->payload;
		
TRACEF(TRACE_L7, ("MAYBE  #%u; iSlot=%lx; didPrimary=%06x; didSecondary=%06x \"%s\" fLast=%u; cEntry=%u\n",
				  pEntry->iEntry,Address::iSlotMe,pEntry->didPrimary,pEntry->didSecondary,pEntry->szClassName,pReply->IsLast(),pEntry->cEntry));
	// Save onlyu our own!
	if (pEntry->cEntry > 0 && pEntry->didPrimary == DeviceId::Did(Address::iCabinet,Address::iSlotMe,IDNULL)) { 

TRACEF(TRACE_L7, ("SAVE  #%u; iSlot=%lx; didPrimary=%06x; didSecondary=%06x \"%s\" fLast=%u\n",
				  pEntry->iEntry,Address::iSlotMe,pEntry->didPrimary,pEntry->didSecondary,pEntry->szClassName,pReply->IsLast()));

		VddItem::Add(pEntry);	// Save Vdd until we have all of them.
TRACEF(TRACE_L7, ("OK\n"));
	}
	if (pReply->IsLast()) {
		// We have them all, now load them.
		if (VddItem::GetFirst() == NULL) {	// We don't have any!
				// Tell PtsProxy we are ready to run!
			RqOsPtsProxySetIopState *pMsg = new RqOsPtsProxySetIopState(Address::iSlotMe, proxyIOPUP);
			Send(pMsg,REPLYCALLBACK(DdmVirtualProxy,DiscardReply));
		}
		for (VddItem *pVdd=VddItem::GetFirst(); pVdd != NULL; pVdd = pVdd->GetNext() ) {
			RqOsManagerCreateInstance *pMsg = new RqOsManagerCreateInstance;
TRACEF(TRACE_L7, ("LOAD  #%u; iSlot=%lx; didPrimary=%06x; didSecondary=%06x \"%s\" IsLast()=%u\n",
				  pVdd->payload.iEntry,Address::iSlotMe,pVdd->payload.didPrimary,pVdd->payload.didSecondary,pVdd->payload.szClassName,pVdd->IsLast()));
			pMsg->payload.vdn = pVdd->payload.vdn;
			pMsg->payload.fStart = FALSE;	// Don't start any yet!
			strcpy(pMsg->payload.szClassName,pVdd->payload.szClassName);
			
			Send(pMsg, (void*) pVdd, REPLYCALLBACK(DdmVirtualProxy,ProcessCreateInstanceReply));
		}
	}
	delete pReply;
TRACEF(TRACE_L8, ("EXIT  DdmVirtualProxy::ProcessGetVddReply\n"));
		
	return OK;
}

// .ProcessCreateInstanceReply -- Process Reply --------------------------DdmVirtualProxy-
//
ERC DdmVirtualProxy::ProcessCreateInstanceReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmVirtualProxy::ProcessCreateInstanceReply\n"));
	RqOsManagerCreateInstance *pReply = (RqOsManagerCreateInstance*) pArgMsg;
	VddItem *pVdd = (VddItem*) pReply->GetContext();
	
	if (pReply->DetailedStatusCode != OK) {
		Tracef("LOAD  DdmManager failed to CreateInstance for vdn=%u; \"%s\"\n",pReply->payload.vdn,pReply->payload.szClassName);
		// Tell PtsProxy we failed!
		RqOsPtsProxySetIopState *pMsg = new RqOsPtsProxySetIopState(Address::iSlotMe, proxyIOPFAIL);
		Send(pMsg,REPLYCALLBACK(DdmVirtualProxy,DiscardReply));
		for(;;) ;
	}
	else {
		RqOsPtsProxySetVdn *pMsgSetVdn = new RqOsPtsProxySetVdn;
		// Notify PtsProxy of the new Vdn/Did and Virtual Routing
		pMsgSetVdn->payload.vdn = pReply->payload.vdn;
		pMsgSetVdn->payload.fPrimary = TRUE;
		pMsgSetVdn->payload.didPrimary = pReply->payload.did;
		pMsgSetVdn->payload.didSecondary = DIDNULL;
		Send(pMsgSetVdn,REPLYCALLBACK(DdmVirtualProxy,DiscardReply));
		
		if (pReply->payload.cServes > 0) {
TRACEF(TRACE_L7,("      Sending %u VirtualServes to PtsProxy (DdmVirtualProxy::ProcessCreateInstanceReply)\n",pReply->payload.cServes));
			RqOsPtsProxySetRoute *pMsgSetRoute = new RqOsPtsProxySetRoute;
			
			pMsgSetRoute->payload.vdn = pReply->payload.vdn;
			pMsgSetRoute->payload.cServes = pReply->payload.cServes;
			for (U32 ii=0; ii<pReply->payload.cServes; ii++) {
				pMsgSetRoute->payload.reqCode[ii] = pReply->payload.reqCode[ii];
			}			
			Send(pMsgSetRoute, REPLYCALLBACK(DdmVirtualProxy,DiscardReply));
		}
		pVdd->fLoaded = TRUE;
		pVdd->did = pReply->payload.did;	// Save for starting
	}
	if (pVdd->IsLast()) {
TRACEF(TRACE_L7,("      Finished loaded virtual devices.  Updating PtsProxy IOP state.\n"));
		// Tell PtsProxy we are ready to run!
		RqOsPtsProxySetIopState *pMsg = new RqOsPtsProxySetIopState(Address::iSlotMe, proxyIOPUP);
		Send(pMsg,REPLYCALLBACK(DdmVirtualProxy,DiscardReply));
	}	
	delete pReply;

	return OK;
}

// .ProcessListenVdnReply -- Process ListenVdn Replies -------------------DdmVirtualProxy-
//
// Listen to PtsProxy for changes in the VirtualDeviceTable
//
// Tell DdmManager of new Vdn/Did mapping.
// DdmManager will ignore if matches current mapping.
//
ERC DdmVirtualProxy::ProcessListenVdnReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmVirtualProxy::ProcessListenVdnReply\n"));

	RqOsPtsProxyListenVdn *pReply = (RqOsPtsProxyListenVdn *) pArgMsg;
	RqOsManagerSetVdnMap *pMsg = new RqOsManagerSetVdnMap;
	
	// Tell DdmManager about it!
	pMsg->payload.vdn = pReply->payload.vdn;
	pMsg->payload.fPrimary = pReply->payload.fPrimary;
	pMsg->payload.didPrimary = pReply->payload.didPrimary;
	pMsg->payload.didSecondary = pReply->payload.didSecondary;
	delete pReply;

	Send(pMsg, REPLYCALLBACK(DdmVirtualProxy,DiscardReply));
	
	return OK;
}

// .ProcessListenRouteReply -- Process ListenVdn Replies ------------------DdmVirtualProxy-
//
// Listen to PtsProxy for changes in the VirtualRouteTable
//
// Tell DdmManager of new virtual request routing.
// DdmManager will ignore if matches current mapping.
//
ERC DdmVirtualProxy::ProcessListenRouteReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmVirtualProxy::ProcessListenRouteReply\n"));
	
	RqOsPtsProxyListenRoute *pReply = (RqOsPtsProxyListenRoute *) pArgMsg;
	RqOsManagerSetRouteMap *pMsg = new RqOsManagerSetRouteMap;
TRACEF(TRACE_L7, ("      Received RqOsPtsProxyListenRouteReply. (DdmVirtualProxy::ProcessListenRouteReply)\n"));

	// Tell DdmManager about it!
	pMsg->payload = pReply->payload;
	delete pReply;

	Send(pMsg, REPLYCALLBACK(DdmVirtualProxy,DiscardReply));
		
	return OK;
}

// .ProcessGetConfig -- Process Request ----------------------------------DdmVirtualProxy-
//
// Translate VirtualManager GetConfig request to PtsProxy GetConfig Request
//
ERC DdmVirtualProxy::ProcessGetConfig(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmVirtualProxy::ProcessGetConfig\n"));
	RqOsVirtualManagerGetConfig *pGetMsg = (RqOsVirtualManagerGetConfig*) pArgMsg;
	RqOsPtsProxyGetConfig *pMsg = new RqOsPtsProxyGetConfig;
	
	pMsg->payload = pGetMsg->payload;

	Send(pMsg,(void*) pGetMsg, REPLYCALLBACK(DdmVirtualProxy,ProcessGetConfigReply));
	
	return OK;
}

// .ProcessGetConfigReply -- Process GetConfigReply ----------------------DdmVirtualProxy-
//
// Translate PtsProxy GetConfig Reply to VirtualManager GetConfig Reply
//
ERC DdmVirtualProxy::ProcessGetConfigReply(Message *pArgMsg) {
TRACEF(TRACE_L8, ("EXEC  DdmVirtualProxy::ProcessGetConfigReply\n"));

	RqOsPtsProxyGetConfig *pReply = (RqOsPtsProxyGetConfig*) pArgMsg;
	RqOsVirtualManagerGetConfig *pGetMsg = (RqOsVirtualManagerGetConfig*) pReply->GetContext();
	
	pGetMsg->payload = pReply->payload;
	delete pReply;
		
	Reply(pGetMsg,OK);
	
	return OK;
}
