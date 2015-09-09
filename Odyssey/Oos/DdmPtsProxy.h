/* DdmPtsProxy.h -- CHAOS Debugging PTS Proxy
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
//  6/22/98 Tom Nelson: Create file


#ifndef __DdmPtsProxy_h
#define __DdmPtsProxy_h

#include "Ddm.h"
#include "RoutingTables.h"
#include "VirtualTable.h"
#include "DdmVirtualProxy.h"
#include "RqOsPtsProxy.h"

#if 0
//
// Listener Classes served by VirtualProxy
//
class ListenVddPts : public Listener {
public:
	static ListenVddPts *pFirst;
	static ListenVddPts *GetFirst() { return pFirst; }

	ListenVddPts *GetNext()	 { return (ListenVddPts*) Listener::GetNext(); }

	ListenVddPts(Message *_pMsg) : Listener(&(Listener*)pFirst,_pMsg) {}
};

class ListenVdnPts : public Listener {
public:
	static ListenVdnPts *pFirst;
	static ListenVdnPts *GetFirst() { return pFirst; }

	ListenVdnPts *GetNext()	 { return (ListenVdnPts*) Listener::GetNext(); }
	
	ListenVdnPts(Message *_pMsg) : Listener(&(Listener*)pFirst,_pMsg) {}
};

class ListenRoutePts : public Listener {
public:
	static ListenRoutePts *pFirst;
	static ListenRoutePts *GetFirst() { return pFirst; }

	ListenRoutePts *GetNext()	 { return (ListenRoutePts*) Listener::GetNext(); }

	ListenRoutePts(Message *_pMsg) : Listener(&(Listener*)pFirst,_pMsg) {}
};

class ListenReadyPts : public Listener {
public:
	static ListenReadyPts *pFirst;
	static ListenReadyPts *GetFirst() { return pFirst; }

	ListenReadyPts *GetNext()	 { return (ListenReadyPts*) Listener::GetNext(); }

	ListenReadyPts(Message *_pMsg) : Listener(&(Listener*)pFirst,_pMsg) {}
};
#endif


// ProxyVirtualDeviceTable

typedef RqOsPtsProxyAddVdd::Payload ProxyVirtualDevice;

class ProxyVirtualDeviceTable {
	Array_T<ProxyVirtualDevice> aTable;	// Range Lookup
public:	
	ERC Set(ProxyVirtualDevice *pPvd) {
		aTable.Set(pPvd->vdn,*pPvd);
		
		return OK;
	}

	ERC Set(VDN vdn,BOOL fPrimary,DID did) {
		ProxyVirtualDevice *pPvd;
		
		if (vdn < (signed)aTable.Size()) {
			pPvd = Get(vdn);
			pPvd->vdn = vdn;
			if (fPrimary) {
				pPvd->didPrimary = did;
			}
			else {
				pPvd->didSecondary = did;
			}
		}
		else {
			ProxyVirtualDevice pvd;
			pvd.vdn = vdn;
			if (fPrimary) {
				pvd.didPrimary = did;
			}
			else {
				pvd.didSecondary = did;
			}
			aTable.Set(vdn,pvd);
		}
		return OK;
	}

	ProxyVirtualDevice *Get(VDN vdn) {
		static ProxyVirtualDevice pvdNull;

		if (vdn < (signed)aTable.Size())
			return &aTable[vdn];

		return &pvdNull;
	}
	// Return VDN of first table entry
	VDN GetFirst() {
		return GetNext(0);
	}
	// Return VDN of next table entry
	VDN GetNext(REQUESTCODE vdnPrev) {
		VDN vdn = vdnPrev + 1;
		
		while (vdn < (signed)aTable.Size()) {
			if (aTable[vdn].vdn != VDNNULL)
				return vdn;
			else
				++vdn;	// Try next VDN
		}
		return VDNNULL;
	}
};

//
// Pts Proxy Ddm Class
//
class DdmPtsProxy : public Ddm {
	typedef ListenerList_T<RqOsPtsProxyListenIopState> ListenerIopState;
	typedef ListenerList_T<RqOsPtsProxyListenVdd> ListenerVdd;
	typedef ListenerList_T<RqOsPtsProxyListenVdn> ListenerVdn;
	typedef ListenerList_T<RqOsPtsProxyListenRoute> ListenerRoute;
	typedef ListenerList_T<RqOsPtsProxyListenReady> ListenerReady;

	ListenerIopState listenerIopState;
	ListenerVdd listenerVdd;
	ListenerVdn listenerVdn;
	ListenerRoute listenerRoute;
	ListenerReady listenerReady;
	
	static ProxyVirtualDeviceTable proxyVirtualDeviceTable;	
	static RequestRouteTable  proxyRequestRouteTable;	// Request Code Routing

public:
	static Ddm *Ctor(DID did) 	{ return new DdmPtsProxy(did); }

	RqOsPtsProxyListenReady::Payload  ready;	// Configuration Ready Flag
	RqOsPtsProxyListenIopState::Payload iop;
	
	DdmPtsProxy(DID did);

	ERC Initialize(Message *pArgMsg);

	// Request handlers
	ERC ProcessSetIopState(Message *pArgMsg);
	ERC ProcessSetIopStateTable(Message *pArgMsg);
	ERC ProcessListenIopState(Message *pArgMsg);
	ERC ProcessAddVdd(Message *pArgMsg);
	ERC ProcessListenVdd(Message *pArgMsg);
	ERC ProcessGetConfig(Message *pArgMsg);
	ERC ProcessSetVdn(Message *pArgMsg);
	ERC ProcessListenVdn(Message *pMsg);
	ERC ProcessSetPrimary(Message *pArgMsg);
	ERC ProcessSetRoute(Message *pMsg);
	ERC ProcessListenRoute(Message *pMsg);
	ERC ProcessSetReady(Message *pArgMsg);
	ERC ProcessListenReady(Message *pMsg);
};

#endif // __DdmPtsProxy_h

