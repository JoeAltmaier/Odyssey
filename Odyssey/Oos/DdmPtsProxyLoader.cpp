/* DdmPtsProxyLoader.cpp -- Start Proxy PTS and load defaults
 *
 * Copyright (C) ConvergeNet Technologies, 1999
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
 *		Load the PTS specified via the PTSPROXY macro in BuildSys.cpp
 *
**/
 
// Revision History: 
//  6/30/99 Tom Nelson: Create file
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_PTSLOADER
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>

// Private Includes
#include "DdmPtsProxyLoader.h"
#include "RqOsPtsLoader.h"
#include "RqOsDdmManager.h"
#include "BuildSys.h"
#include "PtsTable.h"

#include "DidMan.h"
#include "RqOsPtsProxy.h"
#include "VirtualTable.h"
#include "ServeTable.h"

// BuildSys Linkage

CLASSNAME(DdmPtsProxyLoader,SINGLE);
SERVELOCAL(DdmPtsProxyLoader,RqOsPtsProxyLoaderFindPts::RequestCode);


// .DdmPtsProxyLoader -- Constructor --------------------------------------------DdmPtsProxyLoader-
//
DdmPtsProxyLoader::DdmPtsProxyLoader(DID did) : Ddm(did), listenerFindPts(this) {

TRACEF(TRACE_L8, ("EXEC  DdmPtsProxyLoader::DdmPtsProxyLoader\n"));
	DispatchRequest(RqOsPtsProxyLoaderFindPts::RequestCode,REQUESTCALLBACK(DdmPtsProxyLoader, ProcessFindPts));
}

// .Initialize -- Process Initialize ---------------------------------------DdmPtsProxyLoader-
//
STATUS DdmPtsProxyLoader::Initialize(Message *pArgMsg) {

TRACEF(TRACE_L8, ("EXEC  DdmPtsProxyLoader::Initialize;\n"));

	Reply(pArgMsg);	
	return OK;
}

// .Enable -- Process Enable ---------------------------------------------DdmPtsProxyLoader-
//
STATUS DdmPtsProxyLoader::Enable(Message *pMsgEnable) {

TRACEF(TRACE_L8, ("EXEC  DdmPtsProxyLoader::Enable;\n"));

	PtsVdd *pPtsOs;		// PTS used by OS. Maybe Proxy or the real Pts.
	PtsVdd *pPtsAlt;
	
	if ((pPtsOs = PtsProxy::GetVdd()) == NULL) {
		Tracef("[ERROR] No PTSPROXY specified in Buildsys. (DdmPtsProxyLoader)\n");
		for(;;) ;
	}
	pPtsAlt = PtsName::GetVdd();
	if (pPtsAlt && pPtsAlt->slotPrimary != pPtsOs->slotPrimary) {
		Tracef("[ERROR] PtsName and PtsProxy must specify the same slots. (DdmPtsProxyLoader)\n");
		for(;;) ;
	}

	// Load PTS(s).  Send Enable Msg only on last Pts Start.
	if (pPtsOs->slotPrimary == Address::iSlotMe) {
		StartPts(pPtsOs,pMsgEnable, REPLYCALLBACK(DdmPtsProxyLoader,ProcessStartPtsReply));
	}
	else {
		FindPts(pPtsOs,pMsgEnable, REPLYCALLBACK(DdmPtsProxyLoader,ProcessFindPtsReply));
	}
	// Listen for DdmManger to finish starting SystemEntry Ddms
	RqOsDdmManagerSystemReady *pMsg = new RqOsDdmManagerSystemReady;
	Send(pMsg,REPLYCALLBACK(DdmPtsProxyLoader,ProcessSystemReadyReply));

	return OK;
}		

// .ProcessStartPtsReply -- Process Reply -----------------------------------DdmPtsProxyLoader-
//
ERC DdmPtsProxyLoader::ProcessStartPtsReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("ENTER DdmPtsProxyLoader::ProcessStartPtsReply;\n"));
	RqOsDdmManagerCreateInstance *pReply = (RqOsDdmManagerCreateInstance*) pArgMsg;
	Message *pMsgEnable = (Message *) pReply->GetContext();

	if (pReply->DetailedStatusCode != OK) {
		Tracef("[ERROR] DdmManager was unable to start Pts (DdmPtsProxyLoader)\n");
		for(;; ) ;
	}		
	Tracef("**** Started PTS; Vdn=%u; Did=%x (DdmPtsProxyLoader::ProcessStartPtsReply)\n",pReply->payload.vdn,pReply->payload.did);

	// Tell DdmManager about PTS Vdn/Did/Routing.
	RqOsDdmManagerRouteInstance *pMsg = new RqOsDdmManagerRouteInstance(&pReply->payload);
	Send(pMsg,pMsgEnable,REPLYCALLBACK(DdmPtsProxyLoader,ProcessRouteInstanceReply));

	ptsOs = pReply->payload;
	
	delete pReply;	
TRACEF(TRACE_L8, ("EXIT  DdmPtsProxyLoader::ProcessStartPtsReply;\n"));
	return OK;
}
	
// .ProcessRouteInstanceReply -- Process Reply ----------------------------DdmPtsProxyLoaderr-
//
ERC DdmPtsProxyLoader::ProcessRouteInstanceReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("ENTER DdmPtsProxyLoader::ProcessRouteInstanceReply;\n"));
	RqOsDdmManagerRouteInstance *pReply = (RqOsDdmManagerRouteInstance*) pArgMsg;
	Message *pMsgEnable = (Message *) pReply->GetContext();
	delete pReply;

	// Check if there are any VIRTUALENTRY items in BuildSys.cpp
	if (VirtualTable::GetFirst() == NULL) {
		// Tell PTS Proxy the default IOP state table
		RqOsPtsProxySetIopStateTable *pMsg = new RqOsPtsProxySetIopStateTable(&iop);
		Send(pMsg,pMsgEnable,REPLYCALLBACK(DdmPtsProxyLoader,ProcessSetStateTableReply));
	}
	else {	
		// Loading of default PTS VirtualDeviceTable
		// Send all VIRTUALENTRY items from BuildSys.
		ERC erc;
		U32 iEntry;
		VirtualEntry *pEntry;
		for (iEntry=0,pEntry = VirtualTable::GetFirst();  pEntry != NULL; pEntry = pEntry->pNextEntry,iEntry++) {
			RqOsPtsProxyAddVdd *pMsg = new RqOsPtsProxyAddVdd;
			pMsg->payload.cEntry = VirtualTable::GetCount();
			pMsg->payload.iEntry = iEntry;
			pMsg->payload.fStart = pEntry->fStart;
			pMsg->payload.vdn = pEntry->vdn;
			strcpy(pMsg->payload.szClassName,pEntry->pszClassName);
			pMsg->payload.fPrimary = TRUE;
			pMsg->payload.slotPrimary = pEntry->slotPrimary;
			pMsg->payload.slotSecondary = pEntry->slotSecondary;
			pMsg->payload.didPrimary = DIDNULL;
			pMsg->payload.didSecondary = DIDNULL;

			if (!iop.SetState(pMsg->payload.slotPrimary,proxyIOPDOWN))
				Tracef("WARNING Invalid slot primary for VIRTUALENTRY \"%s\"; Slot=%x\n",pEntry->pszClassName,pEntry->slotPrimary);
		
			if (!iop.SetState(pMsg->payload.slotSecondary,proxyIOPDOWN))
				Tracef("WARNING Invalid slot secondary for VIRTUALENTRY \"%s\"; Slot=%x\n",pEntry->pszClassName,pEntry->slotSecondary);
		
			// Include Enable pMessage only on last so we know when we are done.
			erc = Send(pMsg, (void*)(pEntry->pNextEntry == NULL) ? pMsgEnable : NULL, REPLYCALLBACK(DdmPtsProxyLoader,ProcessAddVddReply));
TRACEF(TRACE_L7, ("ADDING Vdd=%u; Class=\"%s\"; fStart=%u; slotPrimary=%x; slotSecondary=%x; fLast=%u; erc=%u\n",pEntry->vdn,pEntry->pszClassName,pEntry->fStart,pEntry->slotPrimary,pEntry->slotSecondary,pEntry->pNextEntry==NULL,erc));
		}
	}
	// No Enable yet!
TRACEF(TRACE_L8, ("EXIT  DdmPtsProxyLoader::ProcessRouteInstanceReply;\n"));
		
	return OK;
}

// .ProcessAddVddReply -- Process Reply ----------------------------------DdmPtsProxyLoader-
//
ERC DdmPtsProxyLoader::ProcessAddVddReply(Message *pArgMsg) {
TRACEF(TRACE_L8, ("ENTER DdmPtsProxyLoader::ProcessAddVddReply;\n"));
	Message *pMsgEnable = (Message *)pArgMsg->GetContext();
	delete pArgMsg;
	
	if (pMsgEnable != NULL)	{ //Last reply
		// Tell PTS Proxy the default IOP state table
		RqOsPtsProxySetIopStateTable *pMsg = new RqOsPtsProxySetIopStateTable(&iop);
		Send(pMsg,pMsgEnable,REPLYCALLBACK(DdmPtsProxyLoader,ProcessSetStateTableReply));
	}
	return OK;
}
	
// .ProcessSetStateTableReply -- Process Reply ---------------------------DdmPtsProxyLoader-
//
ERC DdmPtsProxyLoader::ProcessSetStateTableReply(Message *pArgMsg) {
TRACEF(TRACE_L8, ("ENTER DdmPtsProxyLoader::ProcessSetStateTableReply;\n"));

	Message *pMsgEnable = (Message *)pArgMsg->GetContext();
	delete pArgMsg;
	Reply(pMsgEnable);	// Enable
		
	// Notify PTS that VirtualDevice configuration is complete
	RqOsPtsProxySetReady *pMsg = new RqOsPtsProxySetReady(TRUE);
	Send(pMsg,REPLYCALLBACK(DdmPtsProxyLoader, DiscardReply));

TRACEF(TRACE_L8, ("EXIT  DdmPtsProxyLoader::ProcessSetStateTableReply;\n"));
	
	return OK;
}

// .ProcessFindPtsReply -- Process Reply ----------------------------------DdmPtsProxyLoader-
//
ERC DdmPtsProxyLoader::ProcessFindPtsReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("ENTER DdmPtsProxyLoader::ProcessFindPtsReply;\n"));
	RqOsPtsProxyLoaderFindPts *pReply = (RqOsPtsProxyLoaderFindPts*) pArgMsg;
	Message *pMsgEnable = (Message*) pReply->GetContext();
	
	Tracef("**** Connected with PTS in slot %x; Vdn=%u; Did=%x\n",DeviceId::ISlot(pReply->payload.did),pReply->payload.vdn,pReply->payload.did);
	
	// Tell DdmManager about PTS Vdn/Did/Routing
	RqOsDdmManagerRouteInstance *pMsg = new RqOsDdmManagerRouteInstance(&pReply->payload);
	Send(pMsg, pMsgEnable,REPLYCALLBACK(DdmPtsProxyLoader,ProcessRoutePtsInstanceReply));

	ptsOs = pReply->payload;
	delete pReply;
	
	return OK;
}

// .ProcessRoutePtsInstanceReply -- Process Reply ------------------------DdmPtsProxyLoader-
//
ERC DdmPtsProxyLoader::ProcessRoutePtsInstanceReply(Message *pArgMsg) {

TRACEF(TRACE_L8, ("ENTER DdmPtsProxyLoader::ProcessRoutePtsInstanceReply;\n"));
	Message *pMsgEnable = (Message *) pArgMsg->GetContext();
	delete pArgMsg;

	Reply(pMsgEnable,OK);	// Enable

	return OK;
}

// .ProcessSystemReadyReply -- Process Reply -------------------------------DdmPtsProxyLoader-
ERC DdmPtsProxyLoader::ProcessSystemReadyReply(Message */*pArgMsg*/) {

TRACEF(TRACE_L8, ("ENTER DdmPtsProxyLoader::ProcessSystemReadyReply;\n"));

	system.fReady = TRUE;
	
	RqOsPtsProxyLoaderFindPts *pNotify = new RqOsPtsProxyLoaderFindPts;
	pNotify->payload = ptsOs;
	listenerFindPts.NotifyLast(pNotify);
	delete pNotify;
	
	return OK;
}

// .ProcessFindPts -- Process Request --------------------------------------DdmPtsProxyLoader-
//
// Returns Vdn/Did and Routing for the PTS.
// Will not reply until DdmManager is "Ready".
//
ERC DdmPtsProxyLoader::ProcessFindPts(Message *pArgMsg) {

TRACEF(TRACE_L8, ("ENTER DdmPtsProxyLoader::ProcessFindPts;\n"));
	RqOsPtsProxyLoaderFindPts *pMsg = (RqOsPtsProxyLoaderFindPts*) pArgMsg;
	
	if (system.fReady) {
		TRACEF(TRACE_L8, ("****  Reply vdnPts=%u; didPts=%x (DdmPtsProxyLoader::ProcessFindPts)\n",ptsOs.vdn,ptsOs.did));
		
		pMsg->payload = ptsOs;
		Reply(pMsg,OK);
	}
	else
		listenerFindPts.Add(pMsg);

	return OK;
}

// .StartPts -- Start PTS ---------------------------------------------------DdmPtsProxyLoader-
//
ERC DdmPtsProxyLoader::StartPts(PtsVdd *pPtsVdd, Message *pContext,ReplyCallback pCallback) {
	
	ERC erc;
	
TRACEF(TRACE_L8, ("LOAD  PTS vdn=%u; iSlot=%lx; slotPrimary=%x; slotSecondary=%x \"%s\" \n",pPtsVdd->vdn,Address::iSlotMe,pPtsVdd->slotPrimary,pPtsVdd->slotSecondary,pPtsVdd->pszClassName));

	// Load and start PTS
	RqOsDdmManagerCreateInstance *pMsg = new RqOsDdmManagerCreateInstance(pPtsVdd->vdn,pPtsVdd->pszClassName);
	if ((erc = Send(pMsg, (void*) pContext, pCallback)) != OK) {
		Tracef("[ERROR] %u; Unable to send to start PTS \"%s\" (DdmPtsProxyLoader)\n",erc,pPtsVdd->pszClassName);
		for(;; ) ;
	}
	return OK;
}

// .FindPts -- Find PTS -----------------------------------------------------DdmPtsProxyLoader-
//
ERC DdmPtsProxyLoader::FindPts(PtsVdd *pPtsVdd, Message *pContext,ReplyCallback pCallback) {
	
	ERC erc;
	
TRACEF(TRACE_L8, ("FIND  PTS vdn=%u; iSlot=%lx; slotPrimary=%x; slotSecondary=%x \"%s\" \n",pPtsVdd->vdn,Address::iSlotMe,pPtsVdd->slotPrimary,pPtsVdd->slotSecondary,pPtsVdd->pszClassName));
	// Find and PTS
	RqOsPtsProxyLoaderFindPts *pMsg = new RqOsPtsProxyLoaderFindPts;
	if ((erc = Send(pPtsVdd->slotPrimary,pMsg, (void*) pContext, pCallback)) != OK) {
		Tracef("[ERROR] %u; Unable to Find PTS from DdmPtsProxyLoader in slot %x (DdmPtsProxyLoader)\n", erc,pPtsVdd->slotPrimary);
		for(;;) ;
	}
	return OK;
}

