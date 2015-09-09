/* DdmVirtualManager.h -- Virtual Manager
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
//  7/22/98 Tom Nelson: Create file
//  ** Log at end-of-file **

// 100 columns ruler
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#ifndef __DdmVirtualManager_h
#define __DdmVirtualManager_h

#include "Ddm.h"
#include "RoutingTables.h"
#include "VirtualTable.h"
#include "RqOsVirtualManager.h"
#include "Listener_T.h"

// DdmSvcDeviceLoader class
//
class DdmSvcDeviceLoader : public DdmServices {
	typedef RqOsVirtualMasterListenNewDevice::Device Device;
	typedef RqOsVirtualMasterListenNewRoute::Route Route;
	
	STATUS status;
	ActionCallback callback;

public:
	Device device;
	
	DdmSvcDeviceLoader(DdmServices *_pDdmParent) :  DdmServices(_pDdmParent),status(OK) {}

	ERC Status()	{ return status; }
	
	// ActionCallback receives pointer to this DdmServices object.	
	ERC Execute(const Device &_device,ActionCallback _callback);
	ERC LoadInstance(Device *_pDevice);
	
	ERC ProcessCreateInstanceReply(Message *_pReply);
	ERC ProcessSetNewRouteReply(Message *_pReply);
};


//
// DdmVirtualManager Class
//
class DdmVirtualManager : public Ddm {
	typedef VirtualRoute Route;
	typedef RqOsVirtualMasterListenNewDevice::Device Device;
	typedef RqOsVirtualMasterListenIopState::States States;
	
	ListenerList_T<RqOsVirtualManagerListenIopState> listenerIopState;
	States iop;
	BOOL fAllRoutesComplete;
	
public:
	static Ddm *Ctor(DID did) 	{ return new DdmVirtualManager(did); }
	
	DdmVirtualManager(DID did);

	ERC Enable(Message *_pMsgEnable);
	ERC ProcessSystemReadyReply(Message *_pReply);
	
	ERC ProcessListenIopStateReply(Message *_pReply);
	
	ERC ProcessListenNewDeviceReply(Message *_pReply);
	ERC ProcessLoadDeviceAction(void *_pLoader);

	ERC ProcessListenNewRouteReply(Message *_pReply);
	ERC ProcessListenChangeRouteReply(Message *_pReply);
	ERC ProcessListenDeleteRouteReply(Message *_pReply);
	ERC ProcessDeleteVirtualRouteReply(Message *_pReply);
	ERC ProcessDeleteDdmInstanceReply(Message *_pReply);
	ERC ProcessListenRoutesCompleteReply(Message *_pReply);

	void AutoStart();

	// Request Handlers
	ERC ProcessPingSlot(Message *pArgMsg);
	ERC ProcessFindPts(Message *pArgMsg);

	// Request Handlers (Translate PTS Messages)
	ERC ProcessGetConfig(Message *pArgMsg);
	ERC ProcessGetConfigReply(Message *pArgMsg);
	ERC ProcessSetIopState(Message *pArgMsg);
	ERC ProcessListenIopState(Message *pArgMsg);

	ERC SetLocalRouting(const Route &route);
};

#endif // __DdmVirtualManager_h

//*************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmVirtualManager.h $
// 
// 4     2/08/00 8:55p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 5     2/08/00 6:07p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 3     1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 2     9/17/99 11:12p Tnelson
// 
// 1     9/16/99 3:17p Tnelson
// Support for PTS

