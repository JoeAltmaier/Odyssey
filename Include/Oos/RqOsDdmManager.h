/* RqOsDdmManager.h -- Request Interface to DdmManager Ddm.
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
//  6/22/99 Tom Nelson: Create file
//  ** Log at end-of-file **
//

// 100 columns ruler
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#ifndef __RqOsManager_h
#define __RqOsManager_h

#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"
#include <string.h>
#include "VirtualRoute.h"

// SystemReady -- Check if all SystemEntries have started -------------------------------DdmManager-
//
// Replies with OK when all SystemEntries have been enabled.
//
class RqOsDdmManagerSystemReady : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_SYSTEMREADY };
	
	RqOsDdmManagerSystemReady() : Message(RequestCode) {}
};

// QuiesceOs -- Quiesce System Entries --------------------------------------------------DdmManager-
//
class RqOsDdmManagerQuiesceOs : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_QUIESCEOS };
	
	RqOsDdmManagerQuiesceOs() : Message(RequestCode) {}
};

// PingSlot -- Ping the DdmManager -----------------------------------------------------DdmManager-
//
// Send to slot to be pinged.
//
class RqOsDdmManagerPingSlot : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_PING };
	
	struct Payload {
		DID did;	// DdmManager Did returned

		Payload() : did(DIDNULL) {}
	};
	
	Payload payload;
	
	RqOsDdmManagerPingSlot() 
	: Message(RequestCode) {}
};

// GetMasterRoutes -- Get Routing for System Masters ------------------------------------DdmManager-
//
class RqOsDdmManagerGetMasterRoutes : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_GETMASTERROUTES };
	
	enum { SGI_VIRTUALROUTE = 0 };

	RqOsDdmManagerGetMasterRoutes() 
	: Message(RequestCode) {
		AddSgl(SGI_VIRTUALROUTE, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	void AllocateRoutes(U32 nRoutes) {
		AllocateSgl(SGI_VIRTUALROUTE, nRoutes * sizeof(VirtualRoute));
	}
	// You must first allocate the size of this SGL (above) if more than one route is added.
	void AddRoute(U32 iRoute, const VirtualRoute &route) {
		CopyToSgl(SGI_VIRTUALROUTE, iRoute * sizeof(VirtualRoute), (void*) &route, sizeof(VirtualRoute));
	}

	U32 GetTotalRoutes() {
		return GetSglDataSize(SGI_VIRTUALROUTE) / sizeof(VirtualRoute);
	}

	// Return copy of requested route
	VirtualRoute *GetRoute(U32 iRoute, VirtualRoute *pRoute) {
		CopyFromSgl(SGI_VIRTUALROUTE,iRoute * sizeof(VirtualRoute), pRoute,sizeof(VirtualRoute));
		return pRoute;
	}
};

// UpdateMasterRoutes -- Force DdmManager to update Routing for System Masters ----------DdmManager-
//
class RqOsDdmManagerUpdateMasterRoutes : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_UPDATEMASTERROUTES };
	
	RqOsDdmManagerUpdateMasterRoutes() 
	: Message(RequestCode) {}
};


// CreateInstance -- Load Ddm Instance --------------------------------------------------DdmManager-
//
// Applications SHOULD NOT create virtual devices using this method.
// They should use RqOsVirtualMasterLoadVirtualDevice() processed by DdmVirtualMaster.
//
// Returns OK if instance created
//
class RqOsDdmManagerCreateInstance : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_CREATEINSTANCE };

	struct Payload {
		VDN vdn;
		char szClassName[sCLASSNAME];
		BOOL fStart;
		
		Payload()
		: vdn(VDNNULL) {}
		
		Payload(VDN _vdn, const char *_pszClassName, BOOL _fStart) 
		: vdn(_vdn), fStart(_fStart) {
			strcpy(szClassName,(char*)_pszClassName);
		}
	};
	
	Payload payload;
		
	enum { SGI_VIRTUALROUTE = 0 };

	// Create non-virtual device.  Autostart is not support by non-virtual devices.
	// To start a non-virtual devices send an RqOsPing() request to the did returned.
	RqOsDdmManagerCreateInstance(const char *_pszClassName) 
	: Message(RequestCode), payload(VDNNULL,_pszClassName,FALSE) {
		// Add the optional row data buffer as an SGL item
		AddSgl(SGI_VIRTUALROUTE, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// ** This method is to be used by DdmVirtualManager only **
	// Applications SHOULD NOT create virtual devices using this method.
	// They should use RqOsVirtualMasterLoadVirtualDevice() processed by DdmVirtualMaster.
	RqOsDdmManagerCreateInstance(VDN _vdn,const char *_pszClassName,BOOL _fStart=FALSE) 
	: Message(RequestCode), payload(_vdn,_pszClassName,_fStart) {
		// Add the optional row data buffer as an SGL item
		AddSgl(SGI_VIRTUALROUTE, NULL, 0, SGL_DYNAMIC_REPLY);
	}
	
	void AddRoute(VDN _vdn, DID _did, U32 _nServes, REQUESTCODE *_paServes) {
		VirtualRoute virtualRoute(_vdn,_did,DIDNULL,_nServes,_paServes);
		AddSgl(SGI_VIRTUALROUTE, &virtualRoute, sizeof(VirtualRoute), SGL_COPY);
	}
	void AddRoute(const VirtualRoute &virtualRoute) {
		AddSgl(SGI_VIRTUALROUTE, (void*) &virtualRoute, sizeof(VirtualRoute), SGL_COPY);
	}

	// Return pointer to route data
	VirtualRoute *GetRoutePtr(U32 *pnSize=NULL) {
		return (VirtualRoute*) GetSglDataPtr(SGI_VIRTUALROUTE, pnSize);
	}	
	// Return copy of route data
	VirtualRoute *GetVirtualRoute(VirtualRoute *pVirtualRoute) {
		CopyFromSgl(SGI_VIRTUALROUTE,0,pVirtualRoute,sizeof(VirtualRoute));
		return pVirtualRoute;
	}
};

// DeleteInstance -- Delete Ddm Instance ------------------------------------------------DdmManager-
//
// Applications SHOULD NOT delete virtual devices using this method.
// They should use RqOsVirtualMasterDeleteVirtualDevice() processed by DdmVirtualMaster.
//
// Deletes Ddm Instance.  The Ddm will be Quiesced before being deleted.
//
class RqOsDdmManagerDeleteInstance : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_DELETEINSTANCE };

	struct Payload {
		DID did;
		Payload(DID _did) : did(_did) {}
	};
	
	Payload payload;
		
	RqOsDdmManagerDeleteInstance(DID _did) : Message(RequestCode), payload(_did) {
	}
};

// ListenVdnMap -- Listen for changes in VirtualDeviceMap -------------------------------DdmManager-
//
class RqOsDdmManagerListenVdnMap : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_LISTENVDNMAP };

	struct Payload {
		VDN vdn;
		DID did;
	
		Payload() 
		: vdn(VDNNULL), did(DIDNULL) {}
	};
	
	Payload payload;
	
	RqOsDdmManagerListenVdnMap() : Message(RequestCode) {
	}
};

// SetVdnMap -- Set VirtualDeviceMap VDN/DID --------------------------------------------DdmManager-
//
// Interface for DdmVirtualManager.
//
class RqOsDdmManagerSetVdnMap : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_SETVDNMAP };
	
	struct Payload {
		VDN vdn;
		DID did;
		
		Payload(VDN _vdn=VDNNULL,DID _did=DIDNULL) 
		: vdn(_vdn),did(_did) {}
	};
	
	Payload payload;
		
	RqOsDdmManagerSetVdnMap(VDN _vdn,DID _did) 
	: payload(_vdn,_did), Message(RequestCode) {}
};
	
// DefineVirtualRoute -- Define Virtual Route in slot maps ------------------------------DdmManager-
//
// Interface for DdmVirtualManager.
// 
class RqOsDdmManagerDefineVirtualRoute : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_DEFINEVIRTUALROUTE };
	
	enum { SGI_VIRTUALROUTE = 0 };

	RqOsDdmManagerDefineVirtualRoute(VDN _vdn,DID _did,U32 _nServes,const REQUESTCODE *_paServes)
	: Message(RequestCode) {
		VirtualRoute virtualRoute(_vdn, _did, DIDNULL, _nServes, _paServes);
		AddSgl(SGI_VIRTUALROUTE, &virtualRoute, sizeof(VirtualRoute), SGL_COPY);
	}

	RqOsDdmManagerDefineVirtualRoute(const VirtualRoute &virtualRoute) 
	: Message(RequestCode) {
		AddSgl(SGI_VIRTUALROUTE, (void*) &virtualRoute, sizeof(VirtualRoute), SGL_COPY);
	}
	
	void SetRoute(VDN _vdn, DID _did, U32 _nServes, const REQUESTCODE *_paServes) {
		VirtualRoute virtualRoute(_vdn,_did, DIDNULL, _nServes,_paServes);
		AddSgl(SGI_VIRTUALROUTE, &virtualRoute, sizeof(VirtualRoute), SGL_COPY);
	}
	void SetRoute(const VirtualRoute &virtualRoute) {
		AddSgl(SGI_VIRTUALROUTE, (void*) &virtualRoute, sizeof(VirtualRoute), SGL_COPY);
	}
	
	// Return pointer to route data
	VirtualRoute *GetRoutePtr(U32 *pnSize=NULL) {
		return (VirtualRoute*) GetSglDataPtr(SGI_VIRTUALROUTE,pnSize);
	}
	// Return copy of route data
	VirtualRoute *GetVirtualRoute(VirtualRoute *pVirtualRoute) {
		*pVirtualRoute = *GetRoutePtr();
		return pVirtualRoute;
	}
};

// DeleteVirtualRoute -- Removes Virtual Route from slot maps ---------------------------DdmManager-
//
// Interface for DdmVirtualManager.
// 
class RqOsDdmManagerDeleteVirtualRoute : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_DELETEVIRTUALROUTE };
		
	enum { SGI_VIRTUALROUTE = 0 };

	RqOsDdmManagerDeleteVirtualRoute(VDN _vdn, DID _did, U32 _nServes, const REQUESTCODE *_paServes) 
	: Message(RequestCode) {
		VirtualRoute virtualRoute(_vdn, _did,DIDNULL, _nServes, _paServes);
		AddSgl(SGI_VIRTUALROUTE, &virtualRoute, sizeof(VirtualRoute), SGL_COPY);
	}
	RqOsDdmManagerDeleteVirtualRoute(VirtualRoute _virtualRoute) 
	: Message(RequestCode) {
		AddSgl(SGI_VIRTUALROUTE, &_virtualRoute, sizeof(VirtualRoute), SGL_COPY);
	}
	
	void SetRoute(VDN _vdn, DID _did, U32 _nServes, const REQUESTCODE *_paServes) {
		VirtualRoute virtualRoute(_vdn,_did,DIDNULL,_nServes,_paServes);
		AddSgl(SGI_VIRTUALROUTE, &virtualRoute, sizeof(VirtualRoute), SGL_COPY);
	}
	void SetRoute(const VirtualRoute &virtualRoute) {
		AddSgl(SGI_VIRTUALROUTE, (void*) &virtualRoute, sizeof(VirtualRoute), SGL_COPY);
	}
	
	// Return pointer to route data
	VirtualRoute *GetRoutePtr(U32 *pnSize=NULL) {
		return (VirtualRoute*) GetSglDataPtr(SGI_VIRTUALROUTE,pnSize);
	}
	// Return copy of route data
	VirtualRoute *GetVirtualRoute(VirtualRoute *pVirtualRoute) {
		*pVirtualRoute = *GetRoutePtr();
		return pVirtualRoute;
	}
};

// StartVirtuals -- Start all Ddms that have a VDN --------------------------------------DdmManager-
//
// Interface for DdmVirtualManager.
// 
class RqOsDdmManagerStartVirtuals : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_STARTVIRTUALS };
		
	RqOsDdmManagerStartVirtuals() : Message(RequestCode) {
	}
};

// StartDid -- Start Ddm by Did if not yet started --------------------------------------DdmManager-
//
// Interface for DdmVirtualManager.
//
class RqOsDdmManagerStartDid : public Message {
public:
	enum { RequestCode = REQ_OS_DDMMAN_STARTDID };

	struct Payload {
		DID did;
		Payload(DID _did=DIDNULL) : did(_did) {}
	}; 

	Payload payload;
		
	RqOsDdmManagerStartDid(DID _did=DIDNULL) : payload(_did), Message(RequestCode) {
	}
};

#endif	// __RqOsManager_h

/*************************************************************************/
// Update Log:
//	$Log: /Gemini/Include/Oos/RqOsDdmManager.h $
// 
// 10    2/08/00 8:47p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 11    2/08/00 6:12p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 9     1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 5     9/16/99 3:05p Tnelson
// Support for PTS
