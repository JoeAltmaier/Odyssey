/* DdmPtsProxy.cpp -- CHAOS Debugging PTS Proxy
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
 *		PTS Proxy used to debug CHAOS without the real PTS overhead.
 * 		This proxy is not compatible in any way with the real PTS and
 * 		it's only function is to work with the VirtualProxy Ddm.
 *
**/
 
// Revision History: 
//  6/12/98 Tom Nelson: Create file
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_PTSPROXY
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>
#include "CtEvent.h"

// Private Includes
#include "DdmPtsProxy.h"
#include "DdmManager.h"
#include "VirtualTable.h"
#include "BuildSys.h"

// BuildSys Linkage

CLASSNAME(DdmPtsProxy,SINGLE);

SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxySetIopState::RequestCode);
SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxySetIopStateTable::RequestCode);
SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxyListenIopState::RequestCode);

SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxyAddVdd::RequestCode);
SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxyListenVdd::RequestCode);
SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxyGetConfig::RequestCode);

SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxySetVdn::RequestCode);
SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxyListenVdn::RequestCode);
SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxySetPrimary::RequestCode);

SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxySetRoute::RequestCode);
SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxyListenRoute::RequestCode);
	
SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxySetReady::RequestCode);
SERVEVIRTUAL(DdmPtsProxy,RqOsPtsProxyListenReady::RequestCode);


//
// PtsProxy Statics
//
ProxyVirtualDeviceTable DdmPtsProxy::proxyVirtualDeviceTable;
RequestRouteTable  DdmPtsProxy::proxyRequestRouteTable;


// .PtsProxy -- Constructor -----------------------------------------------------PtsProxy-
//
DdmPtsProxy::DdmPtsProxy(DID did) : Ddm(did), listenerIopState(this),
	listenerVdd(this), listenerVdn(this), listenerRoute(this), listenerReady(this) {

TRACEF(TRACE_L8, ("EXEC  DdmPtsProxy::DdmPtsProxy\n"));
}

// .Initialize -- Process Initialize --------------------------------------------PtxProxy-
//
ERC DdmPtsProxy::Initialize(Message *pArgMsg) {

TRACEF(TRACE_L8, ("ENTER PtsProxy::Initialize;\n"));

	DispatchRequest(RqOsPtsProxySetIopState::RequestCode,     REQUESTCALLBACK(DdmPtsProxy, ProcessSetIopState));
	DispatchRequest(RqOsPtsProxySetIopStateTable::RequestCode,REQUESTCALLBACK(DdmPtsProxy, ProcessSetIopStateTable));
	DispatchRequest(RqOsPtsProxyListenIopState::RequestCode,  REQUESTCALLBACK(DdmPtsProxy, ProcessListenIopState));

	DispatchRequest(RqOsPtsProxyAddVdd::RequestCode,     REQUESTCALLBACK(DdmPtsProxy, ProcessAddVdd));
	DispatchRequest(RqOsPtsProxyListenVdd::RequestCode,  REQUESTCALLBACK(DdmPtsProxy, ProcessListenVdd));
	DispatchRequest(RqOsPtsProxyGetConfig::RequestCode,  REQUESTCALLBACK(DdmPtsProxy, ProcessGetConfig));
	
	DispatchRequest(RqOsPtsProxySetVdn::RequestCode,     REQUESTCALLBACK(DdmPtsProxy, ProcessSetVdn));
	DispatchRequest(RqOsPtsProxyListenVdn::RequestCode,  REQUESTCALLBACK(DdmPtsProxy, ProcessListenVdn));
	DispatchRequest(RqOsPtsProxySetPrimary::RequestCode, REQUESTCALLBACK(DdmPtsProxy, ProcessSetPrimary));
	
	DispatchRequest(RqOsPtsProxySetRoute::RequestCode,   REQUESTCALLBACK(DdmPtsProxy, ProcessSetRoute));
	DispatchRequest(RqOsPtsProxyListenRoute::RequestCode,REQUESTCALLBACK(DdmPtsProxy, ProcessListenRoute));
	
	DispatchRequest(RqOsPtsProxySetReady::RequestCode,   REQUESTCALLBACK(DdmPtsProxy, ProcessSetReady));
	DispatchRequest(RqOsPtsProxyListenReady::RequestCode,REQUESTCALLBACK(DdmPtsProxy, ProcessListenReady));
	
	Reply(pArgMsg,OK);	// Reply to Initialize Message
	
TRACEF(TRACE_L8, ("EXIT  PtsProxy::Initialize;\n"));
	return OK;
}


// .ProcessSetIopState -- Process Request --------------------------------------PtsProxy-
//
ERC DdmPtsProxy::ProcessSetIopState(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  PtsProxy::ProcessSetIopState\n"));
	
	RqOsPtsProxySetIopState *pMsg = (RqOsPtsProxySetIopState*) pArgMsg;
TRACEF(TRACE_L7,("     Update IOP %u; state=%u (PtsProxy::ProcessSetIopState)\n",pMsg->payload.slot,pMsg->payload.state));
	
	if (pMsg->payload.slot < RqOsPtsProxyListenIopState::Payload::MAXSLOTS)
		iop.state[pMsg->payload.slot] = pMsg->payload.state;
	else
		Tracef("WARNING  Invalid slot=%u; state=%u  (DdmPtsProxy::ProcessSetIopState)\n",pMsg->payload.slot,pMsg->payload.state);
		
	Reply(pMsg,OK);
	
	// Return IOP state table to all listeners.
	RqOsPtsProxyListenIopState *pNotify = new RqOsPtsProxyListenIopState;
	pNotify->payload = iop;
	listenerIopState.Notify(pNotify);

	return OK;
}

// .ProcessSetIopStateTable -- Process  Request -----------------------------------PtsProxy-
//
ERC DdmPtsProxy::ProcessSetIopStateTable(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  PtsProxy::ProcessSetIopStateTable\n"));

	RqOsPtsProxySetIopStateTable *pMsg = (RqOsPtsProxySetIopStateTable*) pArgMsg;

	// Set IOP state table
	iop = pMsg->payload;	
	Reply(pMsg,OK);

	// Return IOP state table to all listeners.
	RqOsPtsProxyListenIopState *pNotify = new RqOsPtsProxyListenIopState;
	pNotify->payload = iop;
	listenerIopState.Notify(pNotify);

	return OK;
}

// .ProcessListenIopState -- Process  Request -----------------------------------PtsProxy-
//
// Add to list of IopState Listen requests.
//
ERC DdmPtsProxy::ProcessListenIopState(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  PtsProxy::ProcessListenIopState\n"));

	RqOsPtsProxyListenIopState *pMsg = (RqOsPtsProxyListenIopState*) pArgMsg;
	listenerIopState.Add(pMsg);

	// Return IOP state table
	pMsg->payload = iop;	
	Reply(pMsg,OK,FALSE);

	return OK;
}


// .ProcessAddVdd -- Process SetVdd Request--------------------------------------PtsProxy-
//
ERC DdmPtsProxy::ProcessAddVdd(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  PtsProxy::ProcessAddVdd\n"));
	RqOsPtsProxyAddVdd *pMsg = (RqOsPtsProxyAddVdd*)pArgMsg;

	if (pMsg->payload.vdn == VDNNULL)
		Reply(pMsg,UNEXPECTEDerc);
	else {
		pMsg->payload.didPrimary = DIDNULL;
		pMsg->payload.didSecondary = DIDNULL;
		proxyVirtualDeviceTable.Set(&pMsg->payload);
		Reply(pMsg,OK);
		
		RqOsPtsProxyListenVdd *pNotify = new RqOsPtsProxyListenVdd(&pMsg->payload);
		listenerVdd.Notify(pNotify);
		delete pNotify;
	}	
	return OK;
}

// .ProcessListenVdd -- Process Request -----------------------------------------PtsProxy-
//
ERC DdmPtsProxy::ProcessListenVdd(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  PtsProxy::ProcessListenVdd\n"));
	RqOsPtsProxyListenVdd *pMsg = (RqOsPtsProxyListenVdd*)pArgMsg;
	listenerVdd.Add(pMsg);
Tracef("1 ");
	VDN vdnNext;
Tracef("      First vdn=%u\n",proxyVirtualDeviceTable.GetFirst());
	// Return Virtual Device table 
	for (VDN vdn = proxyVirtualDeviceTable.GetFirst(); vdn != VDNNULL; vdn = vdnNext ) {
		ProxyVirtualDevice *pPvd = proxyVirtualDeviceTable.Get(vdn);
		vdnNext = proxyVirtualDeviceTable.GetNext(vdn);
		
		pMsg->payload.fLast = (vdnNext == VDNNULL);
		pMsg->payload = *pPvd;
		
TRACEF(TRACE_L7, ("RETURN Vdd=%u; Class=\"%s\"; fStart=%u; slotPrimary=%x; fLast=%u\n",pPvd->vdn,pPvd->szClassName,pPvd->fStart,pPvd->didPrimary,pPvd->fLast));
		Reply(pMsg,OK,FALSE);
	}
	return OK;
}

// .ProcessGetConfig -- Process GetConfig Request--------------------------------PtsProxy-
//
// Returns config data
//
ERC DdmPtsProxy::ProcessGetConfig(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  PtsProxy::ProcessGetConfig\n"));

	RqOsPtsProxyGetConfig *pMsg = (RqOsPtsProxyGetConfig*) pArgMsg;
	
	VirtualEntry *pEntry;
		
	// Proxy implementation: Look in BuildSys for config data.
	
	if ((pEntry = VirtualTable::Find(pMsg->payload.vdn)) == NULL)
		Reply(pMsg,CTS_CHAOS_BAD_KEY);
	else {
		if (pEntry->pData != NULL) {
			U32 sData = pEntry->sData;
			if (sData > pMsg->payload.sData )
				Tracef("WARNING: VirtualEntry config data in Buildsys.cpp has exceeded %u bytes!\n",pMsg->payload.sData);

			pMsg->payload.CopyData(pEntry->pData,sData);
		}
		Reply(pMsg, OK);
	}
	return OK;
}

// .ProcessSetVdn -- Process SetVdn Request --------------------------------------PtsProxy-
//
// Set Vdn/Did mapping in ProxyVirtualDeviceTable.
// Reply with new mapping to all Vdn Listeners.
//
ERC DdmPtsProxy::ProcessSetVdn(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  PtsProxy::ProcessSetVdn\n"));
	RqOsPtsProxySetVdn *pMsg = (RqOsPtsProxySetVdn*) pArgMsg;
	
	proxyVirtualDeviceTable.Set(pMsg->payload.vdn,pMsg->payload.fPrimary,pMsg->payload.did);
	
	Reply(pMsg,OK);

	// Notify listeners 
	ProxyVirtualDevice *pPvd = proxyVirtualDeviceTable.Get(pMsg->payload.vdn);
	RqOsPtsProxyListenVdn *pNotify = new RqOsPtsProxyListenVdn;
	pNotify->payload.vdn = pMsg->payload.vdn;
	pNotify->payload.fPrimary = pPvd->fPrimary;
	pNotify->payload.didPrimary = pPvd->didPrimary;
	pNotify->payload.fNewPrimary = pMsg->payload.fPrimary;
	pNotify->payload.didSecondary = pPvd->didSecondary;
	pNotify->payload.fNewSecondary = !pMsg->payload.fPrimary;
	listenerVdn.Notify(pNotify);	

	return OK;
}

// .ProcessSetPrimary -- Process SRequest --------------------------------------PtsProxy-
//
ERC DdmPtsProxy::ProcessSetPrimary(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  PtsProxy::ProcessSetPrimary\n"));
	RqOsPtsProxySetPrimary *pMsg = (RqOsPtsProxySetPrimary*) pArgMsg;
	
	ProxyVirtualDevice *pPvd = proxyVirtualDeviceTable.Get(pMsg->payload.vdn);
	
	pPvd->fPrimary = pMsg->payload.fPrimary;
	
	Reply(pMsg,OK);
	
	// Notify listeners 
	RqOsPtsProxyListenVdn *pNotify = new RqOsPtsProxyListenVdn;
			
	pNotify->payload.vdn = pMsg->payload.vdn;
	pNotify->payload.fPrimary = pPvd->fPrimary;
	pNotify->payload.didPrimary = pPvd->didPrimary;
	pNotify->payload.fNewPrimary = pMsg->payload.fPrimary;
	pNotify->payload.didSecondary = pPvd->didSecondary;
	pNotify->payload.fNewSecondary = !pMsg->payload.fPrimary;
	listenerVdn.Notify(pNotify);
	delete pNotify;		

	return OK;
}

// .ProcessListenVdn -- Process ListenVdn Request --------------------------------PtsProxy-
//
// Add to list of Vdn Listen requests.
//
ERC DdmPtsProxy::ProcessListenVdn(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  PtsProxy::ProcessListenVdn\n"));

	RqOsPtsProxyListenVdn *pMsg = (RqOsPtsProxyListenVdn*) pArgMsg;

	listenerVdn.Add(pMsg);
				
	VDN vdnNext;
	
	// Return Virtual Device table 
	for (VDN vdn = proxyVirtualDeviceTable.GetFirst(); vdn != VDNNULL; vdn = vdnNext ) {
		ProxyVirtualDevice *pPvd = proxyVirtualDeviceTable.Get(vdn);
		vdnNext = proxyVirtualDeviceTable.GetNext(vdn);

		if (pPvd->didPrimary != DIDNULL || pPvd->didSecondary != DIDNULL) {
			pMsg->payload.vdn = vdn;
			pMsg->payload.fPrimary = pPvd->fPrimary; 
			pMsg->payload.didPrimary = pPvd->didPrimary;
			pMsg->payload.fNewPrimary = pMsg->payload.didPrimary != DIDNULL;
			pMsg->payload.didSecondary = pPvd->didSecondary;
			pMsg->payload.fNewSecondary = pMsg->payload.didSecondary != DIDNULL;
			pMsg->fLast = (vdnNext == VDNNULL);
			Reply(pMsg,OK,FALSE);
		}
	} 
	return OK;
}

// .ProcessSetRoute -- Process SetRouting Request ------------------------------PtsProxy-
//
// Set routing in ProxyRequestRoutingTable.
// Reply with new routing to all Routing Listeners.
//
ERC DdmPtsProxy::ProcessSetRoute(Message *pArgMsg) {

	U32		ii;
TRACEF(TRACE_L8, ("ENTER PtsProxy::ProcessSetRoute\n"));

	RqOsPtsProxySetRoute *pMsg = (RqOsPtsProxySetRoute*) pArgMsg;
TRACEF(TRACE_L7, ("     Received %u SetRoute Serves from vdn %u (DdmPtsProxy::ProcessSetRoute)\n",pMsg->payload.cServes,pMsg->payload.vdn));	
	for ( ii=0; ii < pMsg->payload.cServes; ii++)
		proxyRequestRouteTable.Set(pMsg->payload.reqCode[ii],pMsg->payload.vdn);
		
	Reply(pMsg,OK);
	
	RqOsPtsProxyListenRoute *pNotify = new RqOsPtsProxyListenRoute;
	for ( ii=0; ii < pMsg->payload.cServes; ) {
		U32 jj;
		for (jj=0; ii < pMsg->payload.cServes && jj < RqOsPtsProxyListenRoute::Payload::MAXSERVES; jj++) {
			pNotify->payload.reqCode[jj] = pMsg->payload.reqCode[ii++];
			pNotify->payload.vdn[jj] = pMsg->payload.vdn;
		}
		pNotify->payload.cServes = jj;
		listenerRoute.Notify(pNotify);
	}
	delete pNotify;
	
TRACEF(TRACE_L8, ("EXIT  PtsProxy::ProcessSetRoute\n"));
	return OK;
}

// .ProcessListenRoute -- Process ListenRouting Request -----------------------PtsProxy-
//
// Add to list of Routing Listen requests.
//
ERC DdmPtsProxy::ProcessListenRoute(Message *pArgMsg) {

TRACEF(TRACE_L8, ("ENTER PtsProxy::ProcessListenRoute\n"));
	
	RqOsPtsProxyListenRoute *pMsg = (RqOsPtsProxyListenRoute*) pArgMsg;
	listenerRoute.Add(pMsg);

	RequestRouteCodes *prrCode;

	// Return current route table 
	REQUESTCODE reqCode = proxyRequestRouteTable.GetFirst();
	U32 ii;
	if (reqCode != 0) {
		do {
			for (ii=0; ii < RqOsPtsProxyListenRoute::Payload::MAXSERVES && reqCode != 0; ii++ ) {
				prrCode = proxyRequestRouteTable.Get(reqCode);
				if (prrCode->IsVdn()) {
					pMsg->payload.reqCode[ii] = reqCode;
					pMsg->payload.vdn[ii] = prrCode->vdn;
				}
				reqCode = proxyRequestRouteTable.GetNext(reqCode);
			}	
			pMsg->payload.cServes = ii;
			Reply(pMsg,OK,FALSE);
		
		} while (reqCode != 0);
	}
TRACEF(TRACE_L8, ("EXIT  PtsProxy::ProcessListenRoute\n"));
	return OK;
}

// .ProcessSetReady -- Process Request ------------------------------------------PtsProxy-
//
ERC DdmPtsProxy::ProcessSetReady(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  PtsProxy::ProcessSetReady\n"));
	RqOsPtsProxySetReady *pMsg = (RqOsPtsProxySetReady*) pArgMsg;
	
	ready = pMsg->payload;
	
	// Notify listeners
	RqOsPtsProxyListenReady *pNotify = new RqOsPtsProxyListenReady(&ready);
	listenerReady.Notify(pNotify);
	delete pNotify;
	
	return OK;
}

// .ProcessListenReady -- Process Request --------------------------------------PtsProxy-
//
// Add to list of Ready Listen requests.
//
ERC DdmPtsProxy::ProcessListenReady(Message *pArgMsg) {

TRACEF(TRACE_L8, ("ENTER PtsProxy::ProcessListenReady\n"));
	
	RqOsPtsProxyListenReady *pMsg = (RqOsPtsProxyListenReady*) pArgMsg;
	listenerReady.Add(pMsg);
	
	pMsg->payload = ready;
	Reply(pMsg,OK,FALSE);
	
TRACEF(TRACE_L8, ("EXIT  PtsProxy::ProcessListenReady\n"));
	return OK;
}

