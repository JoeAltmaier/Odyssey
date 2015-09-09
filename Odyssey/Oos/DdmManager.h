/* DdmManager.h -- Manages VDNs and private routing.
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
//  6/12/98 Joe Altmaier: Create file
//  ** Log at end-of-file **

// 100 columns ruler
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890


#ifndef __DdmManager_h
#define __DdmManager_h

#define TRACE_ENABLE

// Public Includes
#include "OsTypes.h"
#include "CtEvent.h"
#include "DeviceId.h"
#include "Message.h"
#include "Ddm.h"
#include "RqOsDdmManager.h"

// Private Includes
#include "RoutingTables.h"
#include "VirtualTable.h"
#include "DidMan.h"
#include "Listener_T.h"

class DdmManager : public Ddm {	// Mr. DdmManager Himself!
private:

	class MasterRoute {
	private:
		static MasterRoute *pFirst;
		static U32 nRoutes;
		
	public:
		MasterRoute *pNext;
		VirtualRoute route;

		MasterRoute(const VirtualRoute &_route) : route(_route),pNext(NULL) {}
		
		VirtualRoute *GetRoute(VirtualRoute *_pRoute) {
			*_pRoute = route;
			return _pRoute;
		}
		
		static void Link(MasterRoute *pLink) {
			if (pFirst == NULL)
				pFirst = pLink;
			else {
				pLink->pNext = pFirst;
				pFirst = pLink;
			}
			++nRoutes;
		}
		static void Add(const VirtualRoute &route) {
			Link(new MasterRoute(route));
		}
		static MasterRoute *GetFirst() {
			return pFirst;
		}
		static U32 GetTotal() {
			return nRoutes;
		}
	};
	
private:
	typedef ListenerList_T<RqOsDdmManagerListenVdnMap> ListenerVdnMap;
	typedef ListenerList_T<RqOsDdmManagerSystemReady>  ListenerReady;
	typedef ListenerList_T<RqOsDdmManagerGetMasterRoutes> ListenerGetMasterRoutes;
//	static DdmManager *pDmgr;
	
public:
	static VirtualDeviceMap virtualDeviceMap;	// Did to Vdn Map - Quick fix
	static RequestRouteMap requestRouteMap;		// Request Code Routing
		
private:
	struct System {
		BOOL fReady;
		System() : fReady(FALSE) {}
	};
	
//***
//*** Ddm Manager's DDM
//***
private:
	System system;	// Ready status
	ListenerVdnMap listenerVdnMap;
	ListenerReady  listenerReady;
	ListenerGetMasterRoutes listenerGetMasterRoutes;
	
public:
	DdmManager(DID did);
	static Ddm *Ctor(DID did)	{ return new DdmManager(did); }

private:
	virtual ERC Enable(Message *_pEnableMsg);

	ERC ProcessPingSlot(Message *_pRequest);
	ERC ProcessEnableGetMasterRoutesReply(Message *_pReply);
	ERC ProcessUpdateMasterRoutes(Message *_pRequest);
	ERC ProcessUpdateGetMasterRoutesReply(Message *_pReply);
	ERC StartSystemEntries();
	ERC ProcessSystemReady(Message *_pRequest);
	ERC ProcessPingReply(Message *_pReply);
	ERC NotifySystemReady();
	ERC NotifyGetMasterRoutes();
	ERC RouteToLocalMasters();
	ERC RouteToRemoteMasters(RqOsDdmManagerGetMasterRoutes*);
	
	// Request Code Servers
	ERC ProcessGetMasterRoutes(Message *_pRequest);
	ERC ProcessSetVdnMap(Message *_pRequest);
	ERC ProcessListenVdnMap(Message *_pRequest);
	ERC ProcessSetRouteMap(Message *_pRequest);
	ERC ProcessCreateDdmInstance(Message *_pRequest);
	ERC ProcessDeleteDdmInstance(Message *_pRequest);
	ERC ProcessHaltDdmReply(Message *_pRequest);
	ERC ProcessDefineVirtualRoute(Message *_pRequest);
	ERC ProcessDeleteVirtualRoute(Message *_pRequest);
	ERC ProcessStartDid(Message *_pRequest);
	ERC ProcessStartVirtuals(Message *_pRequest);
	ERC ProcessQuiesceSystem(Message *_pRequest);
	
	ERC SetVdnMap(VDN vdn,DID did);
	ERC DefineVirtualRoute(const VirtualRoute &route);

//***
//*** Static/Service Methods
//***
public:	
	static void DeviceInitialize();
	static void Start();

	static DID Did(VDN vdn) { return virtualDeviceMap.Get(vdn); }
	static void DumpMaps();

private:
	static void StartSystem(void);
	static ERC CreateDdmInstance(char *pszClassName, VDN vdn, BOOL fStart, VirtualRoute *pRouteRet);
	static ERC ServeRequests(DidMan *pDidMan);
	static ERC Serve(REQUESTCODE reqCode, DID did);
	static ERC Serve(REQUESTCODE reqCode, VDN vdn);
};

#endif // __DdmManager_h


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmManager.h $
// 
// 26    2/09/00 3:08p Tnelson
// Removed references to unused include files.  No code changes.
// 
// 25    2/08/00 8:58p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 26    2/08/00 6:07p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 24    1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 23    12/09/99 2:06a Iowa
// 
// 22    11/04/99 1:09p Jlane
// Rolled in Tom's changes.
// 
// 21    10/14/99 5:48a Iowa
// Iowa merge
// 
//  3/23/99 Tom Nelson:   Rewrite using DidMan(s). Supports Ddm Fault-in/Deferred Requests
//  7/27/98 Joe Altmaier: Derive from class Ddm
//  7/21/98 Joe Altmaier: Rename to DdmManager

