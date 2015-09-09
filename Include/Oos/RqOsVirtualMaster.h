/* RqOsVirtualMaster.h -- Request Interface to VirtualMaster Ddm.
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
//  8/17/99 Tom Nelson: Create file
// ** Log at end-of-file **

// 100 columns ruler
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#ifndef __RqOsVirtualMaster_h
#define __RqOsVirtualMaster_h

#include <string.h>

#include "CtTypes.h"
#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"

#include "RqOsDdmManager.h"
#include "IOPStatusTable.h"


// LoadVirtualDdm -- Load virtual device Ddm(s) -----------------------------------DdmVirtualMaster-
//
// Does not reply until both specified slots have loaded the Ddm.
//
// Returns Vdn/didPrimary/didSecondary.  
//
class RqOsVirtualMasterLoadVirtualDevice : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_LOADVD };
	
	struct Payload {
		char szClassName[sCLASSNAME];
		TySlot slotPrimary, slotSecondary;
		RowId ridDdmCfgRec;
		RowId ridVDOwnerUse;
		BOOL fAutoStart;
		
		VDN vdnRet;
		DID didPrimaryRet;
		DID didSecondaryRet;

		Payload() 
		: slotPrimary(IOP_NONE), slotSecondary(IOP_NONE), ridDdmCfgRec(0), ridVDOwnerUse(0), fAutoStart(FALSE),
		  vdnRet(VDNNULL), didPrimaryRet(DIDNULL), didSecondaryRet(DIDNULL) {
		  szClassName[0] = EOS;
		}
		Payload(const char *_pszClassName,TySlot _slotPrimary,TySlot _slotSecondary,BOOL _fAutoStart,const RowId _ridDdmCfgRec,const RowId _ridVDOwnerUse) 
		: slotPrimary(_slotPrimary), slotSecondary(_slotSecondary), ridDdmCfgRec(_ridDdmCfgRec), ridVDOwnerUse(_ridVDOwnerUse),fAutoStart(_fAutoStart),
		  vdnRet(VDNNULL),didPrimaryRet(DIDNULL),didSecondaryRet(DIDNULL) {
			strcpy(szClassName,(char*)_pszClassName);
		}
		void SetReply(VDN _vdn,DID _didPrimary, DID _didSecondary) {
			vdnRet = _vdn;
			didPrimaryRet = _didPrimary;
			didSecondaryRet = _didSecondary;
		}
		void SetReply(Payload *_pPayload) {
			vdnRet = _pPayload->vdnRet;
			didPrimaryRet = _pPayload->didPrimaryRet;
			didSecondaryRet = _pPayload->didSecondaryRet;
		}
	};
	
	Payload payload;
	
	RqOsVirtualMasterLoadVirtualDevice(const char *_pszClassName,TySlot _slotPrimary, TySlot _slotSecondary, BOOL _fAutoStart=FALSE, const RowId _ridDdmCfgRec=0, const RowId _ridVDOwnerUse=0) 
	: payload(_pszClassName,_slotPrimary,_slotSecondary,_fAutoStart,_ridDdmCfgRec,_ridVDOwnerUse), Message(RequestCode) {}
	
	VDN GetVdn()			{ return payload.vdnRet; }
	DID GetDidPrimary()		{ return payload.didPrimaryRet;   }
	DID GetDidSecondary()	{ return payload.didSecondaryRet; }
	
	void SetReply(VDN _vdn, DID _didPrimary,DID _didSecondary) { payload.SetReply(_vdn,_didPrimary,_didSecondary); }
	void SetReply(Payload *_pPayload)	{ payload.SetReply(_pPayload); }
};

// Delete Virtual Ddm -- Will quiesce Ddm if needed -------------------------------DdmVirtualMaster-
//
class RqOsVirtualMasterDeleteVirtualDevice : public Message {
public:
	enum { RequestCode  = REQ_OS_VIRTUALMASTER_DELETEVD };
	
	struct Payload {
		VDN vdn;
		
		Payload(VDN _vdn) : vdn(_vdn) {}
	};
	
	Payload payload;
	
	RqOsVirtualMasterDeleteVirtualDevice(VDN _vdn) 
	: payload(_vdn), Message(RequestCode) {
	}
};

// GetConfig Data -- Return device configuration data -----------------------------DdmVirtualMaster-
//
// May optionaly request the config table definition.
//
class RqOsVirtualMasterGetConfig : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_GETCONFIG };
	
	struct Payload {
		VDN vdn;
		U32 cbData;	// Size of data returned
		BOOL fGetTableDef;

		RowId ridCfg;	// Rid of config record returned
		
		Payload(VDN _vdn=VDNNULL,BOOL _fGetTableDef=FALSE) : vdn(_vdn),fGetTableDef(_fGetTableDef),cbData(0) {}
	};
	
	Payload payload;
	
	enum { 
		SGI_CONFIG_RECORD_RETURNED = 0,
		SGI_TABLEDEF_RETURNED,
		SGI_FIELDDEFS_RETURNED
	};
#if 0		
	RqOsVirtualMasterGetConfig() : Message(RequestCode) {
		AddSgl(SGI_CONFIG_RECORD_RETURNED, NULL, 0, SGL_DYNAMIC_REPLY);
	}
#endif
	RqOsVirtualMasterGetConfig(VDN _vdn,BOOL _fGetTableDef=FALSE) : Message(RequestCode), payload(_vdn,_fGetTableDef) {
		AddSgl(SGI_CONFIG_RECORD_RETURNED, NULL, 0, SGL_DYNAMIC_REPLY);
		AddSgl(SGI_TABLEDEF_RETURNED,  NULL, 0, SGL_DYNAMIC_REPLY);
		AddSgl(SGI_FIELDDEFS_RETURNED, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// Add Reply config data
	void AddConfigDataReply(void *pData,U32 cbData) {
		CopyToSgl(SGI_CONFIG_RECORD_RETURNED,0, pData,cbData);
		payload.cbData = cbData;
	}
	// Add Reply TableDef
	void AddTableDefReply(tableDef *pTableDef,U32 cbTableDef) {
		CopyToSgl(SGI_TABLEDEF_RETURNED,0,pTableDef,cbTableDef);
	}
	// Add Reply FieldDefs
	void AddFieldDefsReply(fieldDef *pFieldDefs,U32 cbFieldDefs) {
		CopyToSgl(SGI_FIELDDEFS_RETURNED,0,pFieldDefs,cbFieldDefs);
	}
	
	// Return pointer to config data
	void *GetConfigDataPtr(U32 *pnSize=NULL) {
		return GetSglDataPtr(SGI_CONFIG_RECORD_RETURNED,pnSize);
	}
	// Return size of config data
	U32 GetConfigDataSize() {
		return GetSglDataSize(SGI_CONFIG_RECORD_RETURNED);
	}

	// Get TableDef
	U32 GetTableDefDataSize() {
		return GetSglDataSize(SGI_TABLEDEF_RETURNED);
	}
	tableDef* GetTableDefPtr(U32 *pcbData=NULL) { 
		return (tableDef*)GetSglDataPtr(SGI_TABLEDEF_RETURNED,pcbData,sizeof(tableDef));
	}
	tableDef* GetTableCopy(U32 *pcbData=NULL) {
		return (tableDef*)GetSglDataCopy(SGI_TABLEDEF_RETURNED,pcbData,sizeof(tableDef));
	}
	
	// Get FieldDefs
	U32 GetFieldDefsDataSize() {
		return GetSglDataSize(SGI_FIELDDEFS_RETURNED);
	}
	fieldDef* GetFieldDefsPtr(U32 *pcbData=NULL) { 
		return (fieldDef*)GetSglDataPtr(SGI_FIELDDEFS_RETURNED,pcbData,sizeof(fieldDef));
	}
	fieldDef* GetFieldDefsCopy(U32 *pcbData=NULL) {
		return (fieldDef*)GetSglDataCopy(SGI_FIELDDEFS_RETURNED,pcbData,sizeof(fieldDef));
	}
};

// FailSlot -- Tell Master to switch devices on slot to alternate -----------------DdmVirtualMaster-
//
// Interface for DdmVirtualManager.
//
class RqOsVirtualMasterFailSlot : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_FAILSLOT };
	
	struct Payload {
		TySlot slot;
		
		Payload(TySlot _slot) : slot(_slot) {}
	};
	
	Payload payload;

	// Constructor
	RqOsVirtualMasterFailSlot(TySlot _slot) 
	: payload(_slot), Message(RequestCode) {
	}
};

// ListenIopState - VirtualStateTable summary -------------------------------------DdmVirtualMaster-
//
// Returns the state of all IOPs on every listen
//
// Interface for DdmVirtualManager.
//
class RqOsVirtualMasterListenIopState : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_LISTENIOPSTATE };
	
	enum { SGI_STATES_RETURNED = 0 };
	
	struct States {
		enum { MAXSLOTS = NSLOT };
		
		U32 nSlots;		// Number of slots
		IopState stateIop[MAXSLOTS];
		
		States() 
		: nSlots(MAXSLOTS) {
			for (U32 ii=0; ii < MAXSLOTS; ii++) {
				stateIop[ii] = IOPS_UNKNOWN;
			}
		}
		BOOL SetStateIop(TySlot slot,IopState state) {
			if (slot >= MAXSLOTS)
				return FALSE;

			stateIop[slot] = state;
			return TRUE;
		}
		// Returns bit mask of IOPs that are valid
		U32 GetIopMask() {
			U32 fIopBit = 1;
			U32 fIopMask = 0;
			for (U32 ii=0; ii < MAXSLOTS; fIopBit<<=1,ii++) {
				if (stateIop[ii] != IOPS_UNKNOWN && stateIop[ii] != IOPS_EMPTY && stateIop[ii] != IOPS_BLANK)
					fIopMask |= fIopBit;
			}
			return fIopMask;
		}
	};
	
	RqOsVirtualMasterListenIopState() : Message(RequestCode) {
		// Returned States SGL
		AddSgl(SGI_STATES_RETURNED, NULL, 0, SGL_DYNAMIC_REPLY);
	}
	
	void AddStatesReply(States *_pStates) {
		CopyToSgl(SGI_STATES_RETURNED,0,_pStates,sizeof(States));
	}

	States *GetStates(States *_pStates) {
		CopyFromSgl(SGI_STATES_RETURNED,0,_pStates,sizeof(States));
		return _pStates;
	}

	// Return pointer to States - DEPRICATED: Use GetStates() above!
	States *GetStatesPtr(U32 *pnSize=NULL) {
		return (States*) GetSglDataPtr(SGI_STATES_RETURNED,pnSize);
	}
};

// ListenNewDevice -- Notify for virtual device creation --------------------------DdmVirtualMaster-
//
// Interface for DdmVirtualManager.
//
class RqOsVirtualMasterListenNewDevice : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_LISTENNEWDEVICE };
	
	struct Payload {
		U32 nRec;	// Number of records returned
		
		Payload() : nRec(0) {}
	};
	
	Payload payload;

	enum { SGI_DEVICES_RETURNED= 0 };
	
	struct Device {
		VDN vdn;
		char szClassName[sCLASSNAME];
		TySlot slotPrimary;
		TySlot slotSecondary;
		RowId ridCfg;
		BOOL fStart;
		BOOL fLast;		// Flagged "last device" returned from initial listen
		
		Device(VDN _vdn=VDNNULL) : vdn(_vdn) {}
		
		Device(VDN _vdn, const char *_pszClass, TySlot _slotP, TySlot _slotS, const RowId _ridCfg, BOOL _fStart)
		: vdn(_vdn),slotPrimary(_slotP),slotSecondary(_slotS),ridCfg(_ridCfg),fStart(_fStart),fLast(FALSE) {
			//**BUG** does not check max length
			strcpy(szClassName,(char*)_pszClass);
		}
		void SetLastFlag(BOOL _fLast) 	{ fLast = _fLast; }
	};
	
	// Constructor
	RqOsVirtualMasterListenNewDevice() : Message(RequestCode) {
		// Returned Device SGL
		AddSgl(SGI_DEVICES_RETURNED, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// Add devices in array to reply data
	void AddDeviceReply(Device *paDevice,U32 nDevice=1) {
		CopyToSgl(SGI_DEVICES_RETURNED,0,paDevice,nDevice * sizeof(Device));
		payload.nRec = nDevice;
	}
	
	// Return number of devices
	U32 GetDeviceCount() {
		return payload.nRec;
	}
	// Return copy of device data
	Device *GetDevice(Device *pDevice) {
		CopyFromSgl(SGI_DEVICES_RETURNED, 0, pDevice, sizeof(Device));
		return pDevice;
	}
};

// ListenNewRoute -- Notify for new Virtual Device routing ------------------------DdmVirtualMaster-
//
// Interface for DdmVirtualManager.
//
class RqOsVirtualMasterListenNewRoute : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_LISTENNEWROUTE };
	
	struct Payload {
		U32 nRoutes;	// # of routes returned

		Payload(U32 _nRoutes=0) : nRoutes(_nRoutes) {}
	};
	
	Payload payload;
		
	enum { SGI_ROUTES_RETURNED = 0 };
	typedef VirtualRoute Route;

	// Constructor
	RqOsVirtualMasterListenNewRoute() : Message(RequestCode) {
		// Returned Device SGL
		AddSgl(SGI_ROUTES_RETURNED, NULL, 0, SGL_DYNAMIC_REPLY);
	}
	// Add reply data
	void AddRouteReply(Route *paRoute,U32 nRoutes=1) {
		CopyToSgl(SGI_ROUTES_RETURNED,0,paRoute, nRoutes * sizeof(Route));
		payload.nRoutes = nRoutes;
	}

	// Return number of devices
	U32 GetRouteCount() {
		return payload.nRoutes;
	}
	// Return copy of route data
	VirtualRoute *GetRoute(VirtualRoute *pVirtualRoute) {
		CopyFromSgl(SGI_ROUTES_RETURNED, 0, pVirtualRoute, sizeof(VirtualRoute));
		return pVirtualRoute;
	}
};


// ListenChangeRoute -- Notify for virtual device route changes -------------------DdmVirtualMaster-
//
// Interface for DdmVirtualManager.
// 
class RqOsVirtualMasterListenChangeRoute : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_LISTENCHANGEROUTE };
	
	struct Payload {
		VDN vdn;
		DID did;

		Payload(VDN _vdn,DID _did) : vdn(_vdn), did(_did) {}
	};
	
	Payload payload;
		
	RqOsVirtualMasterListenChangeRoute(VDN _vdn=VDNNULL,DID _did=DIDNULL) 
	: payload(_vdn,_did), Message(RequestCode) {
	}
};


// ListenDeleteDevice -- Nofity for virtual device deletes ------------------------DdmVirtualMaster-
//
// Vdn must be specified.  Specify DIDNULL if for non-existant device.
//
// Interface for DdmVirtualManager.
//
class RqOsVirtualMasterListenDeleteDevice : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_LISTENDELETEROUTE };
	
	enum { SGI_VIRTUALROUTE = 0 };

	// Construct Reply for .Notify
	RqOsVirtualMasterListenDeleteDevice(VDN _vdn, DID _didPrimary, DID _didSecondary, U32 _nServes, const REQUESTCODE *_paServes) 
	: Message(RequestCode) {
		AddSgl(SGI_VIRTUALROUTE, NULL, 0, SGL_DYNAMIC_REPLY);
		VirtualRoute virtualRoute(_vdn, _didPrimary, _didSecondary, _nServes, _paServes);
		CopyToSgl(SGI_VIRTUALROUTE, 0, &virtualRoute, sizeof(VirtualRoute));
	}

	// Construct for Listen
	RqOsVirtualMasterListenDeleteDevice()
	: Message(RequestCode) {
		// Returned Routes SGL
		AddSgl(SGI_VIRTUALROUTE, NULL, 0, SGL_DYNAMIC_REPLY);
	}
	void SetRoute(VDN _vdn, DID _did, DID _didAlternate, U32 _nServes, const REQUESTCODE *_paServes) {
		VirtualRoute virtualRoute(_vdn,_did, _didAlternate, _nServes, _paServes);
		CopyToSgl(SGI_VIRTUALROUTE, 0, &virtualRoute, sizeof(VirtualRoute));
	}
	void SetRoute(const VirtualRoute &virtualRoute) {
		CopyToSgl(SGI_VIRTUALROUTE, 0, (void*) &virtualRoute, sizeof(VirtualRoute));
	}

	// Return copy of route data
	VirtualRoute *GetRoute(VirtualRoute *pVirtualRoute) {
		CopyFromSgl(SGI_VIRTUALROUTE, 0, pVirtualRoute, sizeof(VirtualRoute));
		return pVirtualRoute;
	}
};

// ListenRoutesComplete -- Notify for number of virtual routes complete -----------DdmVirtualMaster-
//
// Interface for DdmVirtualManager.
//
class RqOsVirtualMasterListenRoutesComplete : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_LISTENROUTESCOMPLETE };
	
	struct Payload {
		U32 nDevices;
		U32 nRoutesComplete;

		Payload(U32 _nDevices=0,U32 _nRoutesComplete=0) 
		: nDevices(_nDevices),nRoutesComplete(_nRoutesComplete) {}
	};
	
	Payload payload;
		
	RqOsVirtualMasterListenRoutesComplete() 
	: Message(RequestCode) {}
	
	RqOsVirtualMasterListenRoutesComplete(U32 _nDevices, U32 _nRoutesComplete) 
	: payload(_nDevices,_nRoutesComplete), Message(RequestCode) {}
	
	void SetReply(U32 _nDevices,U32 _nRoutesComplete) {
		payload.nDevices = _nDevices;
		payload.nRoutesComplete = _nRoutesComplete;
	}
};

// Set New Route -- Tell master virtual route to newly loaded Ddm -----------------DdmVirtualMaster-
//
// Interface for DdmVirtualManager.
//
class RqOsVirtualMasterSetNewRoute : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_SETNEWROUTE };
	
	typedef RqOsVirtualMasterListenNewRoute::Route Route;
	
	enum { SGI_VIRTUALROUTE = 0 };
		
	RqOsVirtualMasterSetNewRoute(const Route &route) 
	: Message(RequestCode) {
		AddSgl(SGI_VIRTUALROUTE, (void*) &route, sizeof(Route), SGL_COPY);
	}

	// Return copy of route data
	VirtualRoute *GetRoute(VirtualRoute *pVirtualRoute) {
		CopyFromSgl(SGI_VIRTUALROUTE, 0, pVirtualRoute, sizeof(VirtualRoute));
		return pVirtualRoute;
	}
};


// RouteReady -- Tell master that route has been set on slot ----------------------DdmVirtualMaster-
//
// Interface for DdmVirtualManager.
//
class RqOsVirtualMasterRouteReady : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_ROUTEREADY };
	
	struct Payload {
		TySlot slot;
		VDN vdn;
		
		Payload(VDN _vdn=VDNNULL) : vdn(_vdn), slot(Address::iSlotMe) {}
	};
	
	Payload payload;

	// Constructor
	RqOsVirtualMasterRouteReady(VDN vdn) 
	: payload(vdn), Message(RequestCode) {}
};

// RouteDeleted -- Tell Master that IOPs VDN route/device has been deleted --------DdmVirtualMaster-
//
// Interface for DdmVirtualManager.
//
class RqOsVirtualMasterRouteDeleted : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_ROUTEDELETED };
	
	struct Payload {
		TySlot slot;
		VDN vdn;
		
		Payload(VDN _vdn=VDNNULL) : vdn(_vdn), slot(Address::GetSlot() ) {}
	};
	
	Payload payload;

	// Constructor
	RqOsVirtualMasterRouteDeleted(VDN vdn) 
	: payload(vdn), Message(RequestCode) {}
};

// RoutesComplete -- Tell Master that IOP knows all the routes --------------------DdmVirtualMaster-
//
// Interface for DdmVirtualManager.
//
class RqOsVirtualMasterRoutesComplete : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMASTER_ROUTESCOMPLETE };
	
	struct Payload {
		TySlot slot;
		
		Payload(TySlot _slot=Address::GetSlot() ) : slot(_slot) {}
	};
	
	Payload payload;

public:
	// Constructor
	RqOsVirtualMasterRoutesComplete() 
	: payload(), Message(RequestCode) {}
};

#endif	// __RqOsVirtualMaster_h

//*************************************************************************
// Update Log:
//	$Log: /Gemini/Include/Oos/RqOsVirtualMaster.h $
// 
// 10    2/08/00 8:47p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 11    2/08/00 6:54p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 9     1/26/00 9:46a Szhang
// Add public for delete message constructor.
// 
// 8     1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 7     12/09/99 1:39a Iowa
// 
// 6     11/04/99 6:30p Jlane
// remove stateOS from listenIOP payload.
// 
// 5     11/04/99 1:21p Jlane
// Roll in Tom's changes.
// 
// 4     10/14/99 4:19a Iowa
// Iowa merge
// 
// 3     10/07/99 9:54a Agusev
// Put in changes as per Tom's request
// 
// 2     9/17/99 11:15p Tnelson
// 
// 1     9/16/99 3:06p Tnelson
