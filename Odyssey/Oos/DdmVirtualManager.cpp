/* DdmVirtualManager.cpp -- Virtual Manager
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 * Copyright (C) Dell Computer, 2000
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
//  7/22/99 Tom Nelson: Create file
//  ** Log at end-of-file **

// 100 columns ruler
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_VIRTUAL_MGR
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>

// Private Includes
#include "DdmManager.h"
#include "ServeTable.h"
#include "SystemTable.h"
#include "BootTable.h"
#include "BuildSys.h"
#include "DdmCmbMsgs.h"
#include "DdmVirtualManager.h"


// BuildSys Linkage

CLASSNAME(DdmVirtualManager,SINGLE);

SERVELOCAL(DdmVirtualManager,RqOsVirtualManagerPingSlot::RequestCode);
SERVELOCAL(DdmVirtualManager,RqOsVirtualManagerGetConfig::RequestCode);
SERVELOCAL(DdmVirtualManager,RqOsVirtualManagerListenIopState::RequestCode);

// .DdmVirtualManager -- Constructor ---------------------------------------------DdmVirtualManager-
//
DdmVirtualManager::DdmVirtualManager(DID _did) : Ddm(_did),fAllRoutesComplete(FALSE),listenerIopState(this)  {
	TRACE_PROC(DdmVirtualManager::DdmVirtualManager);

	DispatchRequest(RqOsVirtualManagerPingSlot::RequestCode,      REQUESTCALLBACK(DdmVirtualManager, ProcessPingSlot));
	DispatchRequest(RqOsVirtualManagerGetConfig::RequestCode,     REQUESTCALLBACK(DdmVirtualManager, ProcessGetConfig));
	DispatchRequest(RqOsVirtualManagerListenIopState::RequestCode,REQUESTCALLBACK(DdmVirtualManager, ProcessListenIopState));
}

// .Enable -- Process Enable -----------------------------------------------------DdmVirtualManager-
//
// The VirtualMaster is a special VirtualDevice case since the VirtualManager 
// cannot load VirtualDevices until the VirtualMaster is loaded.  To solve this
// problem the VirtualManager on the HBC will load the VirtualMaster directly.
//
STATUS DdmVirtualManager::Enable(Message *_pMsgEnable) {
	TRACE_PROC(DdmVirtualManager::Enable);
	
	TRACEF(TRACE_L2,("# Enabling (DdmVirtualManager::Enable)\n"));	

	Message *pMsg;
	
	// ListenIopState - VirtualMaster returns State changes
	pMsg = new RqOsVirtualMasterListenIopState;
	Send(pMsg, REPLYCALLBACK(DdmVirtualManager,ProcessListenIopStateReply));

	// Wait for DdmManager to start all system entries
	pMsg = new RqOsDdmManagerSystemReady;
	Send(pMsg, REPLYCALLBACK(DdmVirtualManager,ProcessSystemReadyReply));
	
	Reply(_pMsgEnable,OK);	// Now Enabled

	return OK;
}

// .ProcessSystemReadyReply -- Wait for DdmManager to start SystemEntries --------DdmVirtualManager-
//
// In any other PTS filter you must wait for local caches to be initialized by the
// first listen replies before enabling.  Because the alternate VirtualMaster must
// enable for loading alternate virtual devices, this is not the case.
//
ERC DdmVirtualManager::ProcessSystemReadyReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualManager::ProcessSystemReadyReply);

	TRACEF(TRACE_L2,("# Staring VirtualMaster Listens (DdmVirtualManager::ProcessSystemReadyReply)\n"));	

	Message *pMsg;
	
	TRACEF(TRACE_L2,("\n1 "));	
	// ListenNewDevice -VirtualMaster returns VirtualDevices to create
	pMsg = new RqOsVirtualMasterListenNewDevice;
	Send(pMsg, REPLYCALLBACK(DdmVirtualManager,ProcessListenNewDeviceReply));
	
	TRACEF(TRACE_L2,("2 "));	
	// ListenNewRoute - VirtualMaster returns new Routes
	pMsg = new RqOsVirtualMasterListenNewRoute;
	Send(pMsg, REPLYCALLBACK(DdmVirtualManager,ProcessListenNewRouteReply));
	
	TRACEF(TRACE_L2,("3 "));	
	// ListenChangeRoute - VirtualMaster returns changes to a Route
	pMsg = new RqOsVirtualMasterListenChangeRoute;
	Send(pMsg, REPLYCALLBACK(DdmVirtualManager,ProcessListenChangeRouteReply));

	TRACEF(TRACE_L2,("4 "));	
	// ListenDeleteRoute - VirtualMaster returns deleted Routes
	pMsg = new RqOsVirtualMasterListenDeleteDevice;
	Send(pMsg, REPLYCALLBACK(DdmVirtualManager,ProcessListenDeleteRouteReply));

	TRACEF(TRACE_L2,("5\n"));	
	// ListenRoutesCompleted - VirtualMaster returns Route Counts
	pMsg = new RqOsVirtualMasterListenRoutesComplete;
	Send(pMsg, REPLYCALLBACK(DdmVirtualManager,ProcessListenRoutesCompleteReply));

	delete _pReply;
	return OK;
}

// .ProcessListenIopStateReply -- Process Replies --------------------------------DdmVirtualManager-
//
// ListenIopState - VirtualMaster returns State changes
// 		All IOPs are returned on first reply
// 		Single IOPs on subsequent replies
//		All IOP states/fAllUp returned on first reply
//		Single IOP state/fAllUp on subsequent replies
//
ERC DdmVirtualManager::ProcessListenIopStateReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualManager::ProcessListenIopStateReply);
	
	RqOsVirtualMasterListenIopState *pReply = (RqOsVirtualMasterListenIopState *) _pReply;
	
	// Save state changes in local state table
	pReply->GetStates(&iop);
	
	// Return IOP state change to all of our listeners.
	RqOsVirtualManagerListenIopState *pNotify = new RqOsVirtualManagerListenIopState;
	pNotify->AddStatesReply(&iop);
	listenerIopState.Notify(pNotify,OK);
	delete pNotify;
	
	AutoStart();

	delete _pReply;
	return OK;
}

// .ProcessListenNewDeviceReply -- Process Replies -------------------------------DdmVirtualManager-
//
// ListenNewDevice -VirtualMaster returns VirtualDevices to create
// 		This happens when a new VD needs to be created
//
// ** Should change this to only return to appropriate slot **
//
ERC DdmVirtualManager::ProcessListenNewDeviceReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualManager::ProcessListenNewDeviceReply);
	
	RqOsVirtualMasterListenNewDevice *pReply = (RqOsVirtualMasterListenNewDevice*) _pReply;

	Device device;
	pReply->GetDevice(&device);
	
	DdmSvcDeviceLoader *pLoader = new DdmSvcDeviceLoader(this);

	pLoader->Execute(device, ACTIONCALLBACK(DdmVirtualManager,ProcessLoadDeviceAction));
	
	delete _pReply;
	return OK;
}

// .ProcessLoadDeviceAction -- Load initial device instances ---------------------DdmVirtualManager-
//
ERC DdmVirtualManager::ProcessLoadDeviceAction(void *_pLoader) {
	TRACE_PROC(DdmVirtualManager::ProcessLoadDeviceAction);

	DdmSvcDeviceLoader *pLoader = (DdmSvcDeviceLoader*) _pLoader;

	delete _pLoader;
	
	return OK;
}


// .ProcessListenNewRouteReply -- Process Replies --------------------------------DdmVirtualManager-
//
// ListenNewRoute - VirtualMaster returns new Routes
//		This happens when new Route has been reported to VirtualMaster
//		Only primary routes are returned.
//		Returns VDN/DID/ROUTING
//
ERC DdmVirtualManager::ProcessListenNewRouteReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualManager::ProcessNewRouteReply);

	RqOsVirtualMasterListenNewRoute *pReply = (RqOsVirtualMasterListenNewRoute *) _pReply;
	
	VirtualRoute route;
	pReply->GetRoute(&route);
	
	TRACEF(TRACE_L2,("# New Route VDN=%u to DID=%x (DdmVirtualManager::ProcessNewRouteReply)\n",route.vdn,route.did));
	
	SetLocalRouting(route);
	
	delete pReply;
	
	return OK;
}


// .ProcessListenChangeRouteReply -- Process Replies -----------------------------DdmVirtualManager-
//
// ListenChangeRoute - VirtualMaster returns changes to a Route
//		This happens when a VD fails over to alternate
//		Returns VDN/DID
//
ERC DdmVirtualManager::ProcessListenChangeRouteReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualManager::ProcessListenChangeRouteReply);

	RqOsVirtualMasterListenChangeRoute *pReply = (RqOsVirtualMasterListenChangeRoute *) _pReply;
	
	TRACEF(TRACE_L2,("# Change Route VDN=%u to DID=%x (DdmVirtualManager::ProcessListenChangeRouteReply)\n",pReply->payload.vdn,pReply->payload.did));

	// Tell DdmManager about it
	RqOsDdmManagerSetVdnMap *pMsg = new RqOsDdmManagerSetVdnMap(pReply->payload.vdn,pReply->payload.did);
	ERC erc = Send(pMsg, REPLYCALLBACK(DdmVirtualManager,DiscardOkReply));

	delete _pReply;
	return OK;
}

// .ProcessListenDeleteRouteReply -- Process Replies -----------------------------DdmVirtualManager-
//
// ListenDeleteRoute - VirtualMaster returns deleted Routes
//		This happens when a the VirtualMaster sees the eVDFlagMask_Delete bit set
//		Returns VDN/slotPrimary/slotSecondary
//		When this processor has deleted the device/removed the routing it 
//			 will clear it's SawDID bit in the VirtualDeviceRecord.
//
ERC DdmVirtualManager::ProcessListenDeleteRouteReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualManager::ProcessListenDeleteRouteReply);

	RqOsVirtualMasterListenDeleteDevice *pReply = (RqOsVirtualMasterListenDeleteDevice *) _pReply;

	VirtualRoute route;
	pReply->GetRoute(&route);
	DID did;
	
//**Hey!  What if they are on the same slot???

	if (!DeviceId::IsLocal(did = route.did)) {
		if (!DeviceId::IsLocal(did = route.didAlternate)) {
			if (!DeviceId::IsLocal(did = DdmManager::Did(route.vdn)) ) {	// This works for slot local
				TRACEF(TRACE_L2,("# Deleting Route VDN=%u (DdmVirtualManager::ProcessListenDeleteRouteReply)\n",route.vdn));

				// Remove routing to remote device
				Message *pDelete = new RqOsDdmManagerDeleteVirtualRoute(route);
				Send(pDelete, pReply, REPLYCALLBACK(DdmVirtualManager,ProcessDeleteVirtualRouteReply));
				return OK;
			}	
		}
	}
	route.did = did;
	TRACEF(TRACE_L2,("# Deleting Instance VDN=%u; DID=%x (DdmVirtualManager::ProcessDeleteVirtualRouteReply)\n",route.vdn,route.did));
	Send(new RqOsDdmManagerDeleteInstance(route.did), pReply, REPLYCALLBACK(DdmVirtualManager,ProcessDeleteVirtualRouteReply));
	// Note: DeleteInstance will delete all local routing to this Did

	// Do not delete _pReply
	return OK;
}

// .ProcessDeleteVirtualRouteReply -- Process Replies ----------------------------DdmVirtualManager-
//
// Notice: Will not delete both devices if they are on the same processor!
//
ERC DdmVirtualManager::ProcessDeleteVirtualRouteReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualManager::ProcessDeleteVirtualRouteReply);

	RqOsVirtualMasterListenDeleteDevice *pListenReply = (RqOsVirtualMasterListenDeleteDevice *) _pReply->GetContext();
	
	VirtualRoute route;
	pListenReply->GetRoute(&route);
	
	TRACEF(TRACE_L2,("# Reporting deletion VDN=%u (DdmVirtualManager::ProcessDeleteVirtualRouteReply)\n",route.vdn));

	// Report to VirtualMaster we have deleted the Device/Route
	RqOsVirtualMasterRouteDeleted *pDeleted = new RqOsVirtualMasterRouteDeleted(route.vdn);
	Send(pDeleted, REPLYCALLBACK(DdmSvcDeviceLoader,DiscardOkReply));
	
	delete pListenReply;
	
	delete _pReply;
	return OK;	
}

// .ProcessListenRoutesCompleteReply -- Process Replies --------------------------DdmVirtualManager-
//
//
ERC DdmVirtualManager::ProcessListenRoutesCompleteReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualManager::ProcessListenRoutesCompleteReply);

	RqOsVirtualMasterListenRoutesComplete *pReply = (RqOsVirtualMasterListenRoutesComplete *) _pReply;
	TRACEF(TRACE_L3,("nDevices=%u; nRoutesComplete=%u (DdmVirtualManager)\n",
					pReply->payload.nDevices,pReply->payload.nRoutesComplete));
					
	fAllRoutesComplete = (pReply->payload.nDevices == pReply->payload.nRoutesComplete);

	AutoStart();
		
	delete _pReply;
	return OK;
}

// .AutoStart -- Start all initial devices marked autostart -----------------------DdmVirtualManger-
//
// Only AutoStart is all IOPs including us have reported UP!
//
// ** Actually this should be the response to a listen.
// ** This should happen when all devices have been instanciated
// ** and all IOPs have seen all the routes.
//
void DdmVirtualManager::AutoStart() {
	TRACE_PROC(DdmVirtualManager::AutoStart);

	static BOOL fStarted = FALSE;
	TRACEF(TRACE_L3,("# fStarted=%u; fAllRoutesComplete=%u (DdmVirtualManager::AutoStart)\n",fStarted,fAllRoutesComplete) );
	
	if (fAllRoutesComplete) {
		TRACEF(TRACE_L2,("# All Routes Complete - AutoStarting (DdmVirtualManager::AutoStart)\n") );
		
		// Set our state in the IOPStatusTable to operating.
		MsgCmbSetMipsState *pSet = new MsgCmbSetMipsState(MsgCmbSetMipsState::SetOperating);
		Send(pSet, REPLYCALLBACK(DdmVirtualManager,DiscardOkReply));

		RqOsVirtualMasterRoutesComplete *pReady = new RqOsVirtualMasterRoutesComplete;
		Send(pReady, REPLYCALLBACK(DdmVirtualManager,DiscardOkReply));
		
		RqOsDdmManagerStartVirtuals *pStart = new RqOsDdmManagerStartVirtuals();
		Send(pStart,REPLYCALLBACK(DdmVirtualManager,DiscardOkReply));

		fStarted = TRUE;	// Only happens once!
	}
}
		

//***					  ***
//*** Dispatched Requests ***
//***					  ***


// .ProcessPingSlot -- Process Request -------------------------------------------DdmVirtualManager-
//
// Returns VirtualManager Did.
//
ERC DdmVirtualManager::ProcessPingSlot(Message *pArgMsg) {
	TRACE_PROC(DdmVirtualManager::ProcessPingSlot);
	
	RqOsVirtualManagerPingSlot *pMsg = (RqOsVirtualManagerPingSlot*) pArgMsg;

	pMsg->payload.did = GetDid();
		
	Reply(pMsg,OK);
	
	return OK;
}

// .ProcessListenIopState -- Process  Request ------------------------------------DdmVirtualManager-
//
// Add to list of IopState Listen requests which are translates from PtsProxy Listens.
//
ERC DdmVirtualManager::ProcessListenIopState(Message *_pRequest) {
	TRACE_PROC(DdmVirtualManager::ProcessListenIopState);

	RqOsVirtualManagerListenIopState*pMsg = (RqOsVirtualManagerListenIopState*)_pRequest;
	listenerIopState.Add(pMsg);
	
	// Return current IOP state
	pMsg->AddStatesReply(&iop);
	Reply(pMsg,OK,FALSE);
						
	return OK;
}

// .ProcessGetConfig -- Process Request ------------------------------------------DdmVirtualManager-
//
// Translate VirtualManager GetConfig request to VirtualMaster GetConfig Request
//
ERC DdmVirtualManager::ProcessGetConfig(Message *_pRequest) {
	TRACE_PROC(DdmVirtualManager::ProcessGetConfig);
	
	RqOsVirtualManagerGetConfig *pRequest = (RqOsVirtualManagerGetConfig*) _pRequest;
	
	// Get the config data from the VirtualMaster
	RqOsVirtualMasterGetConfig *pGet = new RqOsVirtualMasterGetConfig(pRequest->payload.vdn);
	Send(pGet,(void*) pRequest, REPLYCALLBACK(DdmVirtualManager,ProcessGetConfigReply));
	
	return OK;
}

// .ProcessGetConfigReply -- Process GetConfigReply ------------------------------DdmVirtualManager-
//
// Translate VirtualMaster GetConfig Reply to VirtualManager GetConfig Reply
//
ERC DdmVirtualManager::ProcessGetConfigReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualManager::ProcessGetConfigReply);

	RqOsVirtualMasterGetConfig *pReply = (RqOsVirtualMasterGetConfig*) _pReply;
	RqOsVirtualManagerGetConfig *pRequest = (RqOsVirtualManagerGetConfig*) pReply->GetContext();
	
	pRequest->AddConfigDataReply(pReply->GetConfigDataPtr(),pReply->GetConfigDataSize());
	Reply(pRequest,OK);
	
	delete pReply;
		
	return OK;
}

//*********************
//** Support Methods **
//*********************

// .SetLocalRouting -- Set local routing maps ------------------------------------DdmVirtualManager-
//
ERC DdmVirtualManager::SetLocalRouting(const Route &route) {	// static
	TRACE_PROC(DdmVirtualManager::SetLocalRouting);
	
	Message *pMsg;

	TRACEF(TRACE_L2,("# Setting local route map VDN=%u; DID=%x (DdmVirtualManager::SetLocalRouting)\n",route.vdn,route.did));
	
	// Tell DdmManager about the Virtual Routes
	pMsg = new RqOsDdmManagerDefineVirtualRoute(route.vdn,route.did,route.nServes,route.aServes);
	Send(pMsg, REPLYCALLBACK(DdmVirtualManager,DiscardOkReply));

	// Report to VirtualMaster we have seen the Route
	RqOsVirtualMasterRouteReady *pReady = new RqOsVirtualMasterRouteReady(route.vdn);
	Send(pReady, REPLYCALLBACK(DdmSvcDeviceLoader,DiscardOkReply));
	
	return OK;
}


//***					 ***
//*** DdmSvcDeviceLoader ***
//***					 ***

// .Execute -- Load Ddm Instance ------------------------------------------------DdmSvcDeviceLoader-
//
ERC DdmSvcDeviceLoader::Execute(const Device &_device, ActionCallback _callback) {
	TRACE_PROC(DdmSvcDeviceLoader::Execute);

	callback = _callback;	// save callback
	device = _device;		// save copy
	
	LoadInstance(&device);
	return OK;
}

// .LoadInstance -- Load a single instance --------------------------------------DdmSvcDeviceLoader-
ERC DdmSvcDeviceLoader::LoadInstance(Device *pDevice) {
	TRACE_PROC(DdmSvcDeviceLoader::LoadInstance);
	
	TRACEF(TRACE_L3, ("MAYBE iSlot=%lx?; slotPrimary=%06x; slotSecondary=%06x \"%s\" \n",
		   Address::GetSlot(), pDevice->slotPrimary,pDevice->slotSecondary,pDevice->szClassName));
	
	BOOL fMyLocal = pDevice->slotPrimary == IOP_LOCAL && ClassTable::Find(pDevice->szClassName) != NULL;
	BOOL fMySlot = pDevice->slotPrimary == Address::GetSlot() || pDevice->slotSecondary == Address::GetSlot();
	
	// ***HACK***
	if (pDevice->slotPrimary == IOP_LOCAL && !fMyLocal) {
		TRACEF(TRACE_L2, ("# LOAD [LOCAL] iSlot=%lx; slotPrimary=%06x; slotSecondary=%06x \"%s\" (DdmSvcDeviceLoader)\n",
		   Address::GetSlot(), pDevice->slotPrimary, pDevice->slotSecondary, pDevice->szClassName));
		
		// Report to VirtualMaster we have seen the Route
		RqOsVirtualMasterRouteReady *pReady = new RqOsVirtualMasterRouteReady(pDevice->vdn);
		Send(pReady, REPLYCALLBACK(DdmSvcDeviceLoader,DiscardOkReply));
		return OK;
	}
	
	// Only load our own!
	if (fMySlot || fMyLocal) {
		TRACEF(TRACE_L2, ("# LOAD iSlot=%lx; slotPrimary=%06x; slotSecondary=%06x \"%s\" (DdmSvcDeviceLoader)\n",
		   Address::GetSlot(), pDevice->slotPrimary, pDevice->slotSecondary, pDevice->szClassName));

		RqOsDdmManagerCreateInstance *pMsg = new RqOsDdmManagerCreateInstance(pDevice->vdn,pDevice->szClassName,pDevice->fStart);	
		Send(pMsg, pDevice, REPLYCALLBACK(DdmSvcDeviceLoader,ProcessCreateInstanceReply));
	}
	return OK;
}


// .ProcessCreateInstanceReply -- Process Reply ---------------------------------DdmSvcDeviceLoader-
//
ERC DdmSvcDeviceLoader::ProcessCreateInstanceReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualManager::ProcessCreateInstanceReply);

	ERC erc;
	RqOsDdmManagerCreateInstance *pReply = (RqOsDdmManagerCreateInstance*) _pReply;
	Device *pDevice = (Device*) pReply->GetContext();
	
	if (pReply->Status() != OK) {
 		if (pDevice->slotPrimary != IOP_LOCAL) {
 			Tracef("# LOAD DdmManager failed to CreateInstance VDN=%u; \"%s\" (DdmSvcDeviceLoader::ProcessCreateInstanceReply)\n",pDevice->vdn,pDevice->szClassName);
			for(;;) ;	// Fail this IOP!! HA HA HAHA!!!!!
		}
		// else
		// Not our slot local
	}
	Route route;
	pReply->GetVirtualRoute(&route);
	
	if (pDevice->slotPrimary == IOP_LOCAL) {
		TRACEF(TRACE_L2,("# Setting slot local routing VDN=%u (DdmSvcDeviceLoader::ProcessCreateInstanceReply)\n",route.vdn));
		// **HACK**
		((DdmVirtualManager*)pParentDdmSvs)->SetLocalRouting(route);
	}
	else {
		if (route.nServes > 0)
			TRACEF(TRACE_L2,("# Sending %u VirtualServes to VirtualMaster (DdmSvcDeviceLoader::ProcessCreateInstanceReply)\n",route.nServes));
	
		// Send new Route to VirtualMaster
		RqOsVirtualMasterSetNewRoute *pRqSetRoute = new RqOsVirtualMasterSetNewRoute(route);
		erc = Send(pRqSetRoute, REPLYCALLBACK(DdmSvcDeviceLoader,ProcessSetNewRouteReply));
	}
	delete pReply;

	return OK;
}

// .ProcessSetNewRouteReply -- Process Reply ------------------------------------DdmSvcDeviceLoader-
//
ERC DdmSvcDeviceLoader::ProcessSetNewRouteReply(Message *_pReply) {
	TRACE_PROC(DdmSvcDeviceLoader::ProcessSetNewRouteReply);
	
	TRACEF(TRACE_L3,("# SetNewRoute Reply; Erc=%u (DdmSvcDeviceLoader::ProcessSetNewRouteReply)\n",_pReply->DetailedStatusCode));
	
	delete _pReply;
	
#if 0
	// Load next device
	if (++iDevice < nDevice) {
		LoadInstance(pDevice + iDevice);
		return OK;
	}
#endif
	Action(pParentDdmSvs,callback,this);
//	(pParentDdmSvs->*callback)(this);
	
	return OK;
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmVirtualManager.cpp $
// 
// 14    2/15/00 6:07p Tnelson
// Fixes for VirtualDevice delete
// 
// 13    2/08/00 8:55p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 14    2/08/00 6:07p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 11    1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 9     11/04/99 6:30p Jlane
// remove references to stateOs
// 
// 8     10/14/99 2:23p Jlane
// Fix code to set IOPState and fIOPsActive bit.
// 
// 7     10/14/99 5:51a Iowa
// Iowa merge
// 
// 6     10/12/99 4:55p Jlane
// Added code to update the IOPStatus Tabel and SystemStatusTable with the
// operating and active status (respectively) of the IOP.
// 
// 5     10/12/99 11:51a Jlane
// Add code to update IOP state.
// 
// 4     10/08/99 12:38p Tnelson
// 
// 2     9/17/99 11:12p Tnelson
// 
// 1     9/16/99 3:17p Tnelson
// Support for PTS


