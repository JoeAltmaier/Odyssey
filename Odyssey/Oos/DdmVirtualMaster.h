/* DdmVirtualMaster.h -- CHAOS Interface to PTS
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
// ** Log at end-of-file **

#ifndef __DdmVirtualMaster_h
#define __DdmVirtualMaster_h

#include "Ddm.h"
#include "RoutingTables.h"
#include "VirtualTable.h"
#include "RqOsVirtualMaster.h"

#include "IopStatusTable.h"
#include "VirtualDeviceTable.h"
#include "VirtualRouteTable.h"

#include "Listener_T.h"

template <class REC>
class DoubleLink_T {
public:
	REC **ppFirst,**ppLast;
	REC *pPrev,*pNext;
	U32 *pnLinked;
	BOOL fLinked;

	DoubleLink_T<REC>(REC **_ppFirst,REC **_ppLast,U32 *_pnLinked) 
	: ppFirst(_ppFirst),ppLast(_ppLast),pnLinked(_pnLinked),fLinked(FALSE) {
	}
	REC *Next()		{ return pNext; }
	REC *Prev()		{ return pPrev; }
	
	// Link to head of list
	void Insert() {
		if (!fLinked) {
			if (*ppLast == NULL)
				*ppLast = (REC*) this;
					
			pNext = *ppFirst;
			pPrev = NULL;
			*ppFirst = (REC*)this;
			fLinked = TRUE;
			++(*pnLinked);
		}
	}
	// Unlink Entry
	void Unlink() {
		if (fLinked) {
			if (*ppFirst == (REC*)this)
				*ppFirst = pNext;
			else
				pPrev->pNext = pNext;
			
			if (pNext != NULL)
				pNext->pPrev = pPrev;
			else
				*ppLast = NULL;
				
			fLinked = FALSE;
			--(*pnLinked);
		}
	}
};

// Cached Virtual Devices
//
class DdmSvcDeviceCache : public DdmServices {
public:
	typedef RqOsVirtualMasterListenNewDevice::Device Device;
	typedef RqOsVirtualMasterListenNewRoute::Route Route;

public:
	// List Link Entry
	class Entry : public DoubleLink_T<Entry> {
	public:
		VDN vdn;
		U32 nServes;
		Route *pRoute;
		Device *pDevice;
		DID didPrimary;
		DID didSecondary;
		BOOL fRoute;
		BOOL fNotify;
		U32 fSawDid;
		EnumVDFlags eVDFlags;	// Saved Flags
		
		Entry(VDN _vdn,Entry **ppFirst,Entry **ppLast,U32 *pnEntry) 
		: DoubleLink_T<Entry>(ppFirst,ppLast,pnEntry),
		  vdn(_vdn),fRoute(FALSE),fNotify(FALSE),pRoute(NULL), pDevice(NULL),nServes(0),
		  fSawDid(0),didPrimary(DIDNULL),didSecondary(DIDNULL) { 
		}
		~Entry() {
			Unlink();
			delete pRoute;
			delete pDevice;
		}
	};

	Entry *pFirst, *pLast;
	U32 nEntry;
	U32 nDevices,nRoutes;
	U32 nRoutesComplete;
	
	U32 Total()		{ return nEntry; }
	Entry *First()	{ return pFirst; }

	Entry *Find(VDN vdn) {
		for (Entry *pEntry = First(); pEntry != NULL; pEntry = pEntry->Next() ) {
			if (pEntry->vdn == vdn)
				return pEntry;
		}
		return NULL;
	}
	BOOL IsPrimary(VDN vdn,DID did) {
		Entry *pEntry = Find(vdn);
		if (pEntry)
			return (pEntry->pDevice->slotPrimary == DeviceId::ISlot(did) );

		return FALSE;
	}			

	ERC Build(VirtualRouteRecord *paVrr,U32 nVrr);
	ERC Build(VirtualDeviceRecord *paVdr,U32 nVdr);

	Entry *Add(VirtualRouteRecord *paVrr);
	Entry *Add(VirtualDeviceRecord *paVdr);

	VDN Delete(VirtualDeviceRecord *paVdr);

	BOOL SetRouteCompleteFlags(VDN vdn,U32 fSawDid, U32 fIopMask);
	U32 RoutesCompleted(U32 fIopMask);
		

	// Constructor
	DdmSvcDeviceCache(DdmServices *_pDdmParent) 
	: DdmServices(_pDdmParent),pFirst(NULL),pLast(NULL),nEntry(0),
	  nDevices(0),nRoutes(0),nRoutesComplete(0) {}
};

// VirtualDeviceLoader
//
class DdmSvcVirtualDeviceLoader : public DdmServices {
	typedef RqOsVirtualMasterLoadVirtualDevice::Payload LoadDevice;
	typedef RqOsVirtualMasterListenIopState::States States;
	
	LoadDevice *pLoadDevice;
	States *pIopStates;
	void *pContext;
	U32 idListen;
	
	STATUS status;
	ActionCallback callback;

public:
	DdmSvcVirtualDeviceLoader(DdmServices *_pDdmParent) 
	:  DdmServices(_pDdmParent),status(OK) {}

	ERC Status()			 		{ return status; }
	void *GetContext() 		 		{ return pContext; }
	LoadDevice *GetLoadDevicePtr()  { return pLoadDevice; }
	
	// ActionCallback receives pointer to this DdmServices object.	
	ERC Execute(LoadDevice *_pLoadDevice,States *pIopStates, void *pContext, ActionCallback _callback);

private:
	ERC ProcessLoadVirtualDeviceReply(Message *_pReply);
	ERC ProcessLoadVirtualDeviceListenReply(Message *_pReply);
	
	ERC CompleteListen(ERC _status);
	ERC ProcessCompleteStopListenReply(Message *_pReply);
};

// VirtualDeviceUnloader
//
class DdmSvcVirtualDeviceUnloader : public DdmServices {
	void *pContext;
	ActionCallback callback;

	STATUS status;

public:
	DdmSvcVirtualDeviceUnloader(DdmServices *_pDdmParent) 
	:  DdmServices(_pDdmParent), status(OK) {}

	ERC Status()			 		{ return status; }
	void *GetContext() 		 		{ return pContext; }
	
	// ActionCallback receives pointer to this DdmServices object.	
	ERC Execute(const RowId &ridVdr, void *pContext, ActionCallback _callback);

private:
	ERC ProcessDeleteListenReply(Message *_pReply);
	
};

//
// VirtualMaster Ddm Class
//
class DdmVirtualMaster : public Ddm {
	typedef RqOsVirtualMasterListenNewDevice::Device Device;
	typedef RqOsVirtualMasterListenNewRoute::Route Route;
	typedef RqOsVirtualMasterListenIopState::States States;
	typedef DdmSvcDeviceCache::Entry Entry;
		
	ListenerList_T<RqOsVirtualMasterListenIopState>    listenersIopState;
	ListenerList_T<RqOsVirtualMasterListenNewDevice>   listenersNewDevice;
	ListenerList_T<RqOsVirtualMasterListenNewRoute>    listenersNewRoute;
	ListenerList_T<RqOsVirtualMasterListenChangeRoute> listenersChangeRoute;
	ListenerList_T<RqOsVirtualMasterListenDeleteDevice> listenersDeleteDevice;
	ListenerList_T<RqOsVirtualMasterListenRoutesComplete> listenersRoutesComplete;
	
	RowId ridVdt;	// Saved TableId of VirtualDeviceTable
	DdmSvcDeviceCache vdc;
	States iop;
	U16 idVdt;
	BOOL fStarted;
	
public:
	static Ddm *Ctor(DID did) 	{ return new DdmVirtualMaster(did); }

	DdmVirtualMaster(DID did);

	ERC Enable(Message *pArgMsg);

	ERC ProcessIOPStatusTableListenReply(Message *_pReply);
	ERC ProcessVirtualStateTableListenReply(Message *_pReply);
	ERC ProcessVirtualRouteTableListenReply(Message *_pReply);
	ERC ProcessVirtualDeviceTableListenReply(Message *_pReply);
	ERC ProcessVdtFlagsListenReply(Message *_pReply);
	ERC ProcessVdtModifyStateReply(Message *_pReply);
	ERC ProcessVdtSawDidListenReply(Message *_pReply);
	
	// Dispatched Listen Requests
	ERC ProcessListenIopState(Message *_pListen);
	ERC ProcessListenNewDevice(Message *_pListen);
	ERC ProcessListenNewRoute(Message *_pListen);
	ERC ProcessListenChangeRoute(Message *_pListen);
	ERC ProcessListenDeleteDevice(Message *_pListen);
	ERC ProcessListenRoutesComplete(Message *_pListen);

	// Dispatched Requests
	ERC ProcessSetNewRoute(Message *_pRequest);
	ERC ProcessRouteReady(Message *_pRequest);
	ERC ProcessRouteDeleted(Message *_pRequest);
	ERC ProcessRoutesComplete(Message *_pRequest);
	ERC ProcessFailSlot(Message *_pRequest);

	ERC ProcessLoadVirtualDevice(Message *_pRequest);
	ERC ProcessLoadVirtualDeviceReply(void *_pContext);
	ERC ProcessDeleteVirtualDevice(Message *_pRequest);
	ERC ProcessDeleteVirtualDeviceReply(void *_pUnloader);

	ERC ProcessGetConfig(Message *_pRequest);
	ERC ProcessReadClassTableReply(Message *_pReply);
	ERC ProcessReadConfigTableReply(Message *_pReply);
	ERC ProcessGetTableDefReply(Message *_pReply);

	// Support Methods
	ERC DumpStates(States &iop);
	ERC DumpVdtRecord(VirtualDeviceRecord *pVdt);

	// PTS Interfaces
	ERC UpdateIsrState(TySlot slot, IopState state);
	ERC UpdateDevice(const Route &route);
	VDN DeleteDevice(VirtualDeviceRecord *pVdr);

	ERC NotifyIopStates(States *pStates);
	ERC NotifyChangeRoutes(Entry *pEntry);
	ERC NotifyNewRoutes(Entry *paEntry);
	ERC NotifyNewDevices(Entry *paEntry);
	ERC NotifyDeleteDevices(VDN vdn);
	
	void NotifyNewRoutesAll() {
		for (Entry *pEntry=vdc.pFirst; pEntry != NULL; pEntry=pEntry->pNext)
			if (pEntry->fNotify) {
				NotifyNewRoutes(pEntry);
				pEntry->fNotify = FALSE;
			}
	}
	void NotifyNewDevicesAll() {
		for (Entry *pEntry=vdc.First(); pEntry != NULL; pEntry=pEntry->pNext)
			if (pEntry->pDevice) {
				pEntry->pDevice->SetLastFlag(pEntry->pNext == NULL);
				NotifyNewDevices(pEntry);
			}
	}
};


#endif // __DdmVirtualMaster_h

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmVirtualMaster.h $
// 
// 6     2/08/00 8:56p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 7     2/08/00 6:07p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 5     1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 4     12/09/99 2:07a Iowa
// 
// 3     10/14/99 5:52a Iowa
// Iowa merge
// 
// 2     9/17/99 11:12p Tnelson
// 
// 1     9/16/99 3:17p Tnelson
// Support for PTS
