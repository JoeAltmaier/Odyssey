/* DdmManager.cpp -- Manages VDNs and private routing.
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
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
//  6/12/98 Joe Altmaier: Create file
//  ** Log at end-of-file **

// 100 columns ruler
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_DDM_MGR
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>
#ifndef WIN32
	#include "Nucleus.h"
#endif

// Private Includes
#include "DdmManager.h"
#include "ServeTable.h"
#include "SystemTable.h"
#include "BootTable.h"
#include "BuildSys.h"

// BuildSys Linkage

CLASSNAME(DdmManager,SINGLE);
DEVICENAME(DdmManager, DdmManager::DeviceInitialize);

SERVELOCAL(DdmManager,RqOsDdmManagerPingSlot::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerSystemReady::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerSetVdnMap::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerCreateInstance::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerDeleteInstance::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerDefineVirtualRoute::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerDeleteVirtualRoute::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerStartDid::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerStartVirtuals::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerListenVdnMap::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerQuiesceOs::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerGetMasterRoutes::RequestCode);
SERVELOCAL(DdmManager,RqOsDdmManagerUpdateMasterRoutes::RequestCode);

//
// DdmManager Statics
//
//DdmManager *DdmManager::pDmgr = NULL;

VirtualDeviceMap DdmManager::virtualDeviceMap;
RequestRouteMap DdmManager::requestRouteMap;

U32 DdmManager::MasterRoute::nRoutes = 0;
DdmManager::MasterRoute * DdmManager::MasterRoute::pFirst = NULL;

//***		    		***
//*** Ddm Manager's DDM ***
//***		     		***

// .DdmManager -- Constructor -----------------------------------------------------------DdmManager-
//
DdmManager::DdmManager(DID did) : Ddm(did), listenerVdnMap(this), listenerReady(this), listenerGetMasterRoutes(this) {
	TRACE_PROC(DdmManager::DdmManager);

	TRACEF(TRACE_L1,("! DdmManager: Did=%x; iCabinet=%u; iSlot=%u (DdmManager)\n",did,Address::iCabinet,Address::iSlotMe));

//	pDmgr = this;
	
	DispatchRequest(RqOsDdmManagerPingSlot::RequestCode,       		REQUESTCALLBACK(DdmManager,ProcessPingSlot));
	DispatchRequest(RqOsDdmManagerSystemReady::RequestCode,    		REQUESTCALLBACK(DdmManager,ProcessSystemReady));
	DispatchRequest(RqOsDdmManagerSetVdnMap::RequestCode,      		REQUESTCALLBACK(DdmManager,ProcessSetVdnMap));
	DispatchRequest(RqOsDdmManagerCreateInstance::RequestCode, 		REQUESTCALLBACK(DdmManager,ProcessCreateDdmInstance));
	DispatchRequest(RqOsDdmManagerDeleteInstance::RequestCode, 		REQUESTCALLBACK(DdmManager,ProcessDeleteDdmInstance));
	DispatchRequest(RqOsDdmManagerDefineVirtualRoute::RequestCode,	REQUESTCALLBACK(DdmManager,ProcessDefineVirtualRoute));
	DispatchRequest(RqOsDdmManagerDeleteVirtualRoute::RequestCode,	REQUESTCALLBACK(DdmManager,ProcessDeleteVirtualRoute));
	DispatchRequest(RqOsDdmManagerStartDid::RequestCode,	   		REQUESTCALLBACK(DdmManager,ProcessStartDid));
	DispatchRequest(RqOsDdmManagerStartVirtuals::RequestCode,  		REQUESTCALLBACK(DdmManager,ProcessStartVirtuals));
	DispatchRequest(RqOsDdmManagerListenVdnMap::RequestCode,   		REQUESTCALLBACK(DdmManager,ProcessListenVdnMap));
	DispatchRequest(RqOsDdmManagerQuiesceOs::RequestCode,      		REQUESTCALLBACK(DdmManager,ProcessQuiesceSystem));

	DispatchRequest(RqOsDdmManagerGetMasterRoutes::RequestCode,   	REQUESTCALLBACK(DdmManager,ProcessGetMasterRoutes));
	DispatchRequest(RqOsDdmManagerUpdateMasterRoutes::RequestCode,	REQUESTCALLBACK(DdmManager,ProcessUpdateMasterRoutes));
}

// .Enable -- Process Initialize --------------------------------------------------------DdmManager-
//
STATUS DdmManager::Enable(Message *_pEnableMsg) {
	TRACE_PROC(DdmManager::Enable);
	
	if (Address::GetSlot() == Address::GetHbcMasterSlot()) {
		RouteToLocalMasters();
		StartSystemEntries();
	}
	else {
		// Get Master routes from primary HBC
		RqOsDdmManagerGetMasterRoutes *pGmr = new RqOsDdmManagerGetMasterRoutes;
		Send(Address::GetHbcMasterSlot(), pGmr, REPLYCALLBACK(DdmManager, ProcessEnableGetMasterRoutesReply));
	}
	Reply(_pEnableMsg,OK);	// Enable complete
	
	return OK;
}

// .ProcessEnableGetMasterRoutesReply -- Process Reply -----------------------------------DdmManager-
//
ERC DdmManager::ProcessEnableGetMasterRoutesReply(Message *_pReply) {
	TRACE_PROC(DdmManager::ProcessEnableGetMasterRoutesReply);

	RouteToRemoteMasters((RqOsDdmManagerGetMasterRoutes*) _pReply);
	StartSystemEntries();
	
	delete _pReply;
	return OK;
}

// .StartSystemEntries -- Startup preloaded system entry Ddms ---------------------------DdmManager-
//
// Start first SystemEntry (skipping DdmManager) by sending it a Ping.  This
// allows each Ddm to wait until the previous SystemEntry has enabled.
//
ERC DdmManager::StartSystemEntries() {
	TRACE_PROC(DdmManager::StartSystemEntries);
	
	SystemEntry *pSystem;
	ERC erc;

	// Skip us (DdmManager).
	if ((pSystem = SystemTable::GetNextActive(SystemTable::GetFirst()) ) != NULL) {
		RqOsDdmPing *pMsg = new RqOsDdmPing;
		Send(pSystem->route.did, pMsg,(void*) pSystem, REPLYCALLBACK(DdmManager,ProcessPingReply));
		TRACEF(TRACE_L2,("! Starting \"%s\" did=%x erc=%u (DdmManager::StartSystemEntries)\n",pSystem->pszClassName,pSystem->route.did,erc));
	}
	else
		NotifySystemReady();

	return OK;
}

// .ProcessPingReply -- Start next System Entry -----------------------------------------DdmManager-
//
// Start first SystemEntry (skipping DdmManager) by sending it a Ping.  This
// allows each Ddm to wait until the previous SystemEntry has enabled.
//
ERC DdmManager::ProcessPingReply(Message *_pReply) {
	TRACE_PROC(DdmManager::ProcessPingReply);
	
	SystemEntry *pSystem = (SystemEntry *) _pReply->GetContext();
	ERC erc;
	
	if ((pSystem = SystemTable::GetNextActive(pSystem)) != NULL) {
		RqOsDdmPing *pMsg = new RqOsDdmPing;
		erc = Send(pSystem->route.did,pMsg,(void*) pSystem, REPLYCALLBACK(DdmManager,ProcessPingReply));
		TRACEF(TRACE_L2,("! Started \"%s\" did=%x erc=%u (DdmManager::ProcessPingReply)\n",pSystem->pszClassName,pSystem->route.did,erc));
	}
	else
		NotifySystemReady();

	delete _pReply;
	
	return OK;	
}

// .NotifySystemReady -- Reply to SystemReady requests ----------------------------------DdmManager-
//
ERC DdmManager::NotifySystemReady() {
	TRACE_PROC(DdmManager::NotifySystemReady);
	
	TRACEF(TRACE_L2,("! Completed SystemEntry Starts (DdmManager::StartSystemEntries)\n"));
	system.fReady = TRUE;

	// Notify all waiting SystemReady requests
	RqOsDdmManagerSystemReady *pNotify = new RqOsDdmManagerSystemReady;
	listenerReady.NotifyLast(pNotify);
	delete pNotify;

	// Notify all GetMasterRoutes requests
	NotifyGetMasterRoutes();
	
	return OK;
}

// .NotifyGetMasterRoutes -- Reply to GetMassterRoutes requests ----------------------DdmManager-
//
ERC DdmManager::NotifyGetMasterRoutes() {
	TRACE_PROC(DdmManager::NotifyGetMasterRoutes);
	
	TRACEF(TRACE_L2,("! Notifying all GetMasterRoutes Requests (DdmManager::NotifyGetMasterRoutes)\n"));

	RqOsDdmManagerGetMasterRoutes *pNotify = new RqOsDdmManagerGetMasterRoutes;
	
	U32 ii;
	MasterRoute *pLink;
	pNotify->AllocateRoutes(MasterRoute::GetTotal());
	for (ii=0,pLink = MasterRoute::GetFirst(); pLink != NULL; ii++,pLink=pLink->pNext)
		pNotify->AddRoute(ii,pLink->route);
	
	listenerGetMasterRoutes.NotifyLast(pNotify);
	delete pNotify;

	return OK;
}

// .RouteToLocalMasters -- Set routing to local masters and start them ------------------DdmManager
//
ERC DdmManager::RouteToLocalMasters() {
	TRACE_PROC(DdmManager::RouteToLocalMasters);
	
	U32 ii;
	MasterRoute *pLink;
	// Set Master routes to ourself
	for (ii=0,pLink = MasterRoute::GetFirst(); pLink != NULL; ii++,pLink=pLink->pNext) {
		Tracef("! System Master is Local at VDN=%u; DID=%x (DdmManager)\n",pLink->route.vdn,pLink->route.did);
		DidMan::SetMode(pLink->route.did,PRIMARY);
		DefineVirtualRoute(pLink->route);
	}
	return OK;
}

// .RouteToRemoteMasters -- Set routing to remove masters -------------------------------DdmManager
//
ERC DdmManager::RouteToRemoteMasters(RqOsDdmManagerGetMasterRoutes *pReply) {
	TRACE_PROC(DdmManager::RouteToRemoveMasters);
	
	VirtualRoute route;
	
	// Set Virtual Routes to system masters
	for (U32 ii=0; ii < pReply->GetTotalRoutes(); ii++) {
		pReply->GetRoute(ii,&route);

		Tracef("! System Master is remote at VDN=%u; DID=%x (DdmManager)\n",route.vdn,route.did);
		DefineVirtualRoute(route);
	}
	return OK;
}

//***				  ***		
//*** Served Requests ***
//***				  ***		

// .ProcessUpdateMasterRoutes -- Force updating of master routes from primary HBC -------DdmManager-
//
ERC DdmManager::ProcessUpdateMasterRoutes(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessUpdateMasterRoutes);
	
	if (Address::GetSlot() == Address::GetHbcMasterSlot()) {
		RouteToLocalMasters();
		StartSystemEntries();
		Reply(_pRequest,OK);
	}
	else {
		// Get Master routes from primary HBC.
		RqOsDdmManagerGetMasterRoutes *pGmr = new RqOsDdmManagerGetMasterRoutes;
		Send(Address::GetHbcMasterSlot(), pGmr, _pRequest, REPLYCALLBACK(DdmManager, ProcessUpdateGetMasterRoutesReply));
	}
	return OK;
}

// .ProcessUpdateGetMasterRoutesReply -- Process Reply -----------------------------------DdmManager-
//
ERC DdmManager::ProcessUpdateGetMasterRoutesReply(Message *_pReply) {
	TRACE_PROC(DdmManager::ProcessUpdateGetMasterRoutesReply);

	RouteToRemoteMasters((RqOsDdmManagerGetMasterRoutes*) _pReply);
	
	Reply((Message*) _pReply->GetContext(),OK);
	
	delete _pReply;
	return OK;
}

// .ProcessGetMasterRoutes -- Return routes of system masters ---------------------------DdmManager-
//
ERC DdmManager::ProcessGetMasterRoutes(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessGetMasterRoutes);

	RqOsDdmManagerGetMasterRoutes *pRequest = (RqOsDdmManagerGetMasterRoutes *) _pRequest;

	U32 ii;
	MasterRoute *pLink;

	if (!system.fReady)
		listenerGetMasterRoutes.Add(pRequest);
	else {
		TRACEF(TRACE_L3,("! Processing GetMasterRoutes Request (DdmManager::NotifyGetMasterRoutes)\n"));
		
		pRequest->AllocateRoutes(MasterRoute::GetTotal());
		for (ii=0,pLink = MasterRoute::GetFirst(); pLink != NULL; ii++,pLink=pLink->pNext)
			pRequest->AddRoute(ii,pLink->route);

		Reply(pRequest,OK);
	}
	return OK;
}

// .ProcessSystemReady -- Reply when all SystemEntries are started ----------------------DdmManager-
//
ERC DdmManager::ProcessSystemReady(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessSystemReady);
	
	RqOsDdmManagerSystemReady* pRequest = (RqOsDdmManagerSystemReady*) _pRequest;
	
	if (!system.fReady)
		listenerReady.Add(pRequest);
	else
		Reply(pRequest,OK);

	return OK;	
}
	
// .ProcessPingSlot -- Process Request --------------------------------------------------DdmManager-
//
// Returns DdmManager Did.
//
ERC DdmManager::ProcessPingSlot(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessGetPtsDid);
	
	RqOsDdmManagerPingSlot *pRequest = (RqOsDdmManagerPingSlot*) _pRequest;

	pRequest->payload.did = GetDid();
		
	Reply(pRequest,OK);
	
	return OK;
}

// .ProcessSetVdnMap -- Process Request -------------------------------------------------DdmManager-
//
ERC DdmManager::ProcessSetVdnMap(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessSetVdnMap);

	RqOsDdmManagerSetVdnMap *pRequest = (RqOsDdmManagerSetVdnMap*) _pRequest;
	
	SetVdnMap(pRequest->payload.vdn,pRequest->payload.did);
	
	Reply(pRequest,OK);
	
	return OK;
}

// .ProcessCreateDdmInstance -- Process Message -----------------------------------------DdmManager-
//
// Create Did for Vdn using specified Ddm Class.
// Returns Did and request codes of virtual serves.
//
ERC DdmManager::ProcessCreateDdmInstance(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessCreateDdmInstance);

	RqOsDdmManagerCreateInstance *pRequest = (RqOsDdmManagerCreateInstance*) _pRequest;
	RqOsDdmManagerCreateInstance::Payload *pEntry = &pRequest->payload;
	ERC erc;
	VirtualRoute route;
	
	erc = CreateDdmInstance(pEntry->szClassName, pEntry->vdn, pEntry->fStart, &route);
	pRequest->AddRoute(route);

	Reply(pRequest,erc);

	return OK;
}


// .ProcessDeleteDdmInstance -- Process Message -----------------------------------------DdmManager-
//
// Delete local serves
// Halts Ddm
// Delete Ddm/Did for specified Did.
//
// Notice: Does not remove Vdn/Did mappings or virtual routes
// 		   Send RqOsDdmManagerDeleteVirtualRoute first.
//
ERC DdmManager::ProcessDeleteDdmInstance(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessDeleteDdmInstance);

	RqOsDdmManagerDeleteInstance *pRequest = (RqOsDdmManagerDeleteInstance*) _pRequest;	
	DID	did = pRequest->payload.did;
	
	if (!DeviceId::IsLocal(did)) {
		Reply(pRequest,CTS_CHAOS_INVALID_DID);
		return OK;
	}
	DidMan *pDidMan = DidMan::GetDidMan(did);

	// Remove all routing to this Ddm
	
	if (pDidMan->vdn != VDNNULL)
		SetVdnMap(pDidMan->vdn, DIDNULL);

	ServeEntry *pServe = ServeTable::Find(pDidMan->pClass->ctor);

	// Remove Serve Routings
	if (pServe != NULL)  {
		for (ServeItem *pItem = pServe->pServeLocal; pItem != NULL; pItem = pItem->pNextItem)
			requestRouteMap.Clear(pItem->reqCode);
	
		for (ServeItem *pItem = pServe->pServeVirtual; pItem != NULL; pItem = pItem->pNextItem)
			requestRouteMap.Clear(pItem->reqCode);
	}
	
	// Send RqOsDdmHalt() to Ddm
	Message *pDelete = new RqOsDdmHalt();
	Send(did, pDelete, pRequest, REPLYCALLBACK(DdmManager,ProcessHaltDdmReply));
	
	return OK;
}

// .ProcessDeleteDdmReply -- Process Message --------------------------------------------DdmManager-
//
ERC DdmManager::ProcessHaltDdmReply(Message *_pReply) {
	TRACE_PROC(DdmManager::ProcessHaltDdmReply);

	RqOsDdmManagerDeleteInstance *pRequest = (RqOsDdmManagerDeleteInstance*) _pReply->GetContext();
	DidMan::DeleteDidMan(pRequest->payload.did);	

	Reply(pRequest,_pReply->Status());

	delete _pReply;
	return OK;
}

// .ProcessDefineVirtualRoute -- Process Message -----------------------------------------DdmManager-
//
// Define the mapping of VDN/DID
// Define virtual routing for VDN
//
ERC DdmManager::ProcessDefineVirtualRoute(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessDefineVirtualRoute);
	
	RqOsDdmManagerDefineVirtualRoute *pRequest = (RqOsDdmManagerDefineVirtualRoute*) _pRequest;
	
	VirtualRoute route;
	pRequest->GetVirtualRoute(&route);
	
	DefineVirtualRoute(route);
	
	Reply(pRequest,OK);
	
	return OK;
}

// .ProcessDeleteVirtualRoute -- Process Message ----------------------------------------DdmManager-
//
// Delete the mapping of VDN/DID
// Delete virtual routing for VDN
//
ERC DdmManager::ProcessDeleteVirtualRoute(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessDeleteVirtualRoute);
	
	RqOsDdmManagerDeleteVirtualRoute *pRequest = (RqOsDdmManagerDeleteVirtualRoute*) _pRequest;

	VirtualRoute route;
	pRequest->GetVirtualRoute(&route);
	
	SetVdnMap(route.vdn,DIDNULL);
	
	TRACEF(TRACE_L3, ("     Deleting mapped Vdn=%u\n",route.vdn));

	for (U32 ii=0; ii < route.nServes; ii++) {
		TRACEF(TRACE_L3, ("     Deleting Serve Rq=%x on Vdn=%u\n",route.aServes[ii], route.vdn));
		requestRouteMap.Clear(route.aServes[ii]);
	}
	Reply(pRequest,OK);
	
	return OK;
}

// .StartDid -- Process Message ---------------------------------------------------------DdmManager-
//
// Start Did for previously created instance if not yet started.
//
ERC DdmManager::ProcessStartDid(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessStartDid);

	RqOsDdmManagerStartDid *pMsg = (RqOsDdmManagerStartDid*) _pRequest;
	
	ERC erc = DidMan::StartDid(pMsg->payload.did);

	Reply(pMsg,erc);

	return OK;
}

// .StartVirtuals -- Process Message ----------------------------------------------------DdmManager-
//
// Start all unintanciated Ddms that have a VDN
//
ERC DdmManager::ProcessStartVirtuals(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessStartVirtuals);

	TRACEF(TRACE_L1,("Starting Virtual Devices flagged as AutoStart (DdmManager)\n"));
	
	DidMan::StartVirtuals();

	Reply(_pRequest,OK);

	return OK;
}

// .ProcessListenVdnMap -- Process Request ----------------------------------------------DdmManager-
//
// Add to list of Vdn Listen requests.
//
ERC DdmManager::ProcessListenVdnMap(Message *_pRequest) {
	TRACE_PROC(DdmManager::ProcessListenVdn);

	RqOsDdmManagerListenVdnMap *pMsg = (RqOsDdmManagerListenVdnMap*) _pRequest;
	listenerVdnMap.Add(pMsg);
					
	VDN vdnNext;

	// Return local VirtualDeviceMap
	for (VDN vdn = virtualDeviceMap.GetFirst(); vdn != VDNNULL; vdn = vdnNext ) {
		pMsg->payload.vdn = vdn; 
		pMsg->payload.did = virtualDeviceMap.Get(vdn);
		vdnNext = virtualDeviceMap.GetNext(vdn);
		Reply(pMsg,OK,FALSE);
	} 
	return OK;
}

// .SetVdnMap -- Set Did for specified Vdn in VirtualDeviceMap --------------------------DdmManager-
//
ERC DdmManager::SetVdnMap(VDN vdn, DID did) {
	TRACE_PROC(DdmManager::SetVdnMap);

	virtualDeviceMap.Set(vdn,did);
	
	ListenerVdnMap::ListenerMessage *pReply = new ListenerVdnMap::ListenerMessage;
	
	pReply->payload.vdn = vdn;
	pReply->payload.did = did;

	listenerVdnMap.Notify(pReply);
	delete pReply;
	
	return OK;
}

// .QuiesceSystem -- Quiesce all remaining Ddms in reverse Did order --------------------DdmManager-
//
ERC DdmManager::ProcessQuiesceSystem(Message *_pRequest) {

	delete _pRequest;
	return OK;
}

//***		   				 ***
//*** Static/Service Methods ***
//***						 ***

// .DeviceInitialize -- Initialize for Static Methods ----------------------------------DdmMananger-
//
// Many DdmManager methods are called before any instance of DdmManager
// has been created.
//
void DdmManager::DeviceInitialize() {	// static
	TRACE_PROC(DdmManager::DeviceInitialize);
	
//	DdmManager::pDmgr = NULL;
	
	//*** May want to validate Buildsys contents here ***
}

// .Start -- Startup the DdmManager -----------------------------------------------------DdmManager-
//
void DdmManager::Start() {	// static
	TRACE_PROC(DdmManager::Start);
	
	DdmManager::StartSystem();
}

// .StartSystem -- Start all system entries ---------------------------------------------DdmManager-
//
// Read SystemEntry Table: 
// 		Create DidMan for each entry.
//		All entries are autostarted.
// 		One of these entries better be the DdmManager!
//
void DdmManager::StartSystem() {	// static
	TRACE_PROC(DdmManager::StartSystem);

	SystemEntry *pSystem;
	ERC erc;
	
	if (SystemTable::GetFirst() == NULL) {
		Tracef("[ERROR] No system devices specified in BuildSys! (DdmManager::StartSystem)\n");
		for (;;) ;
	}
		
	// Start all  entries in SystemTable::
	for (pSystem=SystemTable::GetFirst(); pSystem != NULL; pSystem = pSystem->pNextEntry) {
		if (pSystem->vdn != VDNNULL) {
			// Load Device (Master) on HBCs.  Masters have a VDN.
			TRACEF(TRACE_L2, ("! Loading Master \"%s\" vdn=%u ",pSystem->pszClassName,pSystem->vdn));
			if (Address::GetSlot() == Address::GetHbcMasterSlot() || Address::GetSlotFop() == Address::GetHbcMasterSlot() ) {
				if ((erc = CreateDdmInstance(pSystem->pszClassName,pSystem->vdn,FALSE, &pSystem->route)) != OK) {
					Tracef("\n[ERROR] Unable to load \"%s\" vdn=%u! (DdmManager::StartSystem)\n",pSystem->pszClassName,pSystem->vdn);
					for (;;) ;
				}
				MasterRoute::Add(pSystem->route);	// Save routing to Master(s)

				pSystem->fActive = Address::GetSlot() == Address::GetHbcMasterSlot() || (DidMan::GetClassFlags(pSystem->route.did) & ACTIVEACTIVE);
				DidMan::SetMode(pSystem->route.did, ALTERNATE);	// Will change to Primary when defining route
			}
			else
				Tracef("\n[WARNING] Cannot load masters on IOPs. \"%s\" vdn=%u (DdmManager::StartSystem)",pSystem->pszClassName,pSystem->vdn);
		}
		else {
			// Load Device (Non-Master) all slots
			TRACEF(TRACE_L2, ("! Loading Device \"%s\" ",pSystem->pszClassName));
			if ((erc = CreateDdmInstance(pSystem->pszClassName,VDNNULL,FALSE, &pSystem->route)) != OK) {
				Tracef("\n[ERROR] Unable to load \"%s\"! (DdmManager::StartSystem)\n",pSystem->pszClassName);
				for (;;) ;
			}
			if (pSystem->route.nServes > 0) {
				Tracef("\n[ERROR] SystemEntry Class \"%s\" may not serve virtual request(s)! (DdmManager::StartSystem)\n",pSystem->pszClassName);
				for (;;) ;
			}
			pSystem->fActive = TRUE;
			DidMan::SetMode(pSystem->route.did,SYSTEM);
		}
		TRACEF(TRACE_L2, ("\"%s\" DID=%x (DdmManager::StartSystem)\n",pSystem->pszClassName, pSystem->route.did));
	}
	
	// Start DdmManager.  All other loaded SystemEntries will be started by DdmManager.
	VirtualRoute *pRoute = &SystemTable::GetFirst()->route;
	if (DidMan::GetClassCtor(pRoute->did) != Ctor) {
		Tracef("\n[ERROR] DdmManager must be first system device started! (DdmManager::StartSystem)\n");
		for (;;) ;
	}
	DidMan::StartDdm(pRoute->did);	// Kick-start DdmManager
}


// .CreateDdmInstance -- Create DDM Instance and return routing -------------------------DdmManager-
//
ERC DdmManager::CreateDdmInstance(char *pszClassName,VDN vdn,BOOL fStart, VirtualRoute *pRouteRet) {	// Static
	TRACE_PROC(DdmManager::CreateDdmInstance);

	DidMan *pDidMan;
	
	if ((pDidMan = DidMan::CreateDidMan(pszClassName,vdn,fStart)) == NULL) {
		Tracef("[ERROR] Class \"%s\" is not defined in class table! (DdmManager::CreateDdmInstance)\n",pszClassName);
		Tracef("        VirtualDevice NOT loaded. (DdmManager::CreateDdmInstance)\n");
		return CTS_CHAOS_CLASS_NOT_FOUND;
	}
	pRouteRet->SetRoute(vdn, pDidMan->did);

	ServeEntry *pServe;
	ServeItem  *pItem;

	pRouteRet->nServes = 0;
	if ((pServe = ServeTable::Find(pDidMan->pClass->ctor)) != NULL) {
		// Process SERVELOCALs   (served now)
		for (pItem = pServe->pServeLocal; pItem != NULL; pItem = pItem->pNextItem) {
			TRACEF(TRACE_L3, ("!      Setting ServeLocal RQ=%x on did=%x (DdmManager)\n",pItem->reqCode,pDidMan->did));
			Serve(pItem->reqCode,pDidMan->did);
		}
		// Process SERVEVIRTUALs (returned to sender)
		U32 ii;
		for (ii=0,pItem=pServe->pServeVirtual; pItem != NULL; pItem = pItem->pNextItem,ii++) {
			TRACEF(TRACE_L3, ("!      Returning ServeVirtual RQ=%x on vdn=%u (DdmManager)\n",pItem->reqCode,pDidMan->vdn));
			pRouteRet->aServes[ii] = pItem->reqCode;	// Return request codes of virtual serves
		}
		pRouteRet->nServes = ii;
	}
	return OK;
}

// .DefineVirtualRoute -- Set Virtual Routing -------------------------------------------DdmManager-
//
// Define the mapping of VDN/DID
// Define virtual routing for VDN
//
ERC DdmManager::DefineVirtualRoute(const VirtualRoute &route) {
	TRACE_PROC(DdmManager::DefineVirtualRoute);
	
	SetVdnMap(route.vdn,route.did);
	TRACEF(TRACE_L3, ("!     Define map Vdn=%u to Did=%x (DdmManager)\n",route.vdn,route.did));

	for (U32 ii=0; ii < route.nServes; ii++) {
		TRACEF(TRACE_L3, ("!     Serving Rq=%x on Vdn=%u (DdmManager)\n",route.aServes[ii], route.vdn));
		Serve(route.aServes[ii], route.vdn);
	}
	return OK;
}

// Serve -- Serve Request Code by DID ---------------------------------------------------DdmManager-
//
STATUS DdmManager::Serve(REQUESTCODE reqCode, DID did) {
	TRACE_PROC(DdmManager::Serve(did));
	
	RequestRouteCodes *prr = requestRouteMap.Get(reqCode);
	if (prr->IsVdn()) {
		Tracef("[WARNING] Ignoring LocalServe for Request %x with did %u\n",reqCode,did);
		Tracef("          Request already served virtually by vdn=%x\n",prr->vdn);
		return OK;
	}
	else if (prr->IsDid()) {
		if (prr->did != did) {	// allow duplicates
			Tracef("[WARNING] Ignoring LocalServe for Request %x with did %u (did=%x)\n",reqCode,did);
			Tracef("          Request already served locally by did=%x\n",prr->did);
		}
		return OK;
	}
	return requestRouteMap.Set(reqCode,did);
}

// .Serve -- Serve Request Code by VDN --------------------------------------------------DdmManager-
//
STATUS DdmManager::Serve(REQUESTCODE reqCode, VDN vdn) {
	TRACE_PROC(DdmManager::Serve(vdn));
	
	RequestRouteCodes *prr = requestRouteMap.Get(reqCode);
	if (prr->IsDid()) {
		Tracef("[WARNING] Ignoring VirtualServe for Request %x with vdn %u\n",reqCode,vdn);
		Tracef("          Request already served locally by did=%x\n",prr->did);
		return OK;
	}
	else if (prr->IsVdn()) {
		if (prr->vdn != vdn) {	// allow duplicates
			Tracef("[WARNING] Ignoring VirtualServe for Request %x with vdn %u (did=%x)\n",reqCode,vdn);
			Tracef("          Request already served virtually by vdn=%u\n",prr->vdn);
		}
		return OK;
	}
	return requestRouteMap.Set(reqCode,vdn);
}

// .DumpMaps -- Displays DdmManager Maps ------------------------------------------------DdmManager-
//
void DdmManager::DumpMaps() {	// static
	TRACE_PROC(DdmManager::DumpMaps);

	Tracef("*** VirtualDeviceMap:\n    VDN   DID\n");
	DID did;
	
	for (VDN vdn=1; (did = virtualDeviceMap.Get(vdn)) != DIDNULL; vdn++) {
		Tracef("    %5u  %x\n",vdn,did);
	} 
	RequestRouteCodes *prrCode;
	REQUESTCODE reqCode;
	
	Tracef("*** RequestRouteMap (Virtual):\n    ReqCode  VDN\n");
	reqCode = requestRouteMap.GetFirst();
	while (reqCode != 0) {
		prrCode = requestRouteMap.Get(reqCode);
		if (prrCode->IsVdn()) {
			Tracef("     %6x   %u\n",reqCode,prrCode->vdn);      
		}
		reqCode = requestRouteMap.GetNext(reqCode);
	}
	
	Tracef("*** RequestRouteMap (Local):\n    ReqCode  DID\n");
	reqCode = requestRouteMap.GetFirst();
	while (reqCode != 0) {
		prrCode = requestRouteMap.Get(reqCode);
		if (prrCode->IsDid()) {
			Tracef("     %6x   %x\n",reqCode,prrCode->did);      
		}
		reqCode = requestRouteMap.GetNext(reqCode);
	}
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmManager.cpp $
// 
// 51    2/15/00 6:05p Tnelson
// Fixes for VirtualDevice delete
// 
// 50    2/08/00 8:58p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 51    2/08/00 6:07p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 49    1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 48    11/04/99 1:09p Jlane
// Rolled in Tom's changes.
// 
// 47    10/14/99 5:48a Iowa
// Iowa merge
// 
//  6/23/99 Tom Nelson:   Added supported for VirtualProxy/VirtualManager
//  5/07/99 Eric Wedel:   Changed for classname in functor macros (GH).
//  3/23/99 Tom Nelson:   Rewrite using DidMan(s). Supports Ddm Fault-in/Deferred Requests
//  2/12/99 Joe Altmaier: TRACEF
//  2/02/99 Tom Nelson:	  Fault-in Ddm Model/Linked Msg Queues
// 12/04/98 Tom Nelson:   Added PERSISTANTENTRY/BOOTENTRY Macros
//  7/21/98 Joe Altmaier: Rename to DdmManager
