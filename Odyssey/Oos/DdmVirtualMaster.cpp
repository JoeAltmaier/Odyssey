/* DdmVirtualMaster.cpp -- CHAOS Interface to PTS
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
 * Notes:
 *
 * VirtualMaster will accept requests at anytime however, it will
 * not make any requests until DdmManager has notified it that all
 * system entries have been started.
 *
**/
 
// Revision History: 
//  7/22/99 Tom Nelson: Create file
// ** Log at end of file **

// IMPLEMENATION NOTES:
//
// When the primary DID is created the IOP must first insert the ServeRows
// to the PTS before updating the Did and nServes in the VirtualDeviceRecord
//	
// Replies to the did Listens must read the serve rows before updating
// the "Saw DID" field in the VirtualDeviceRecord
	

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

#include "DdmVirtualMaster.h"
#include "ClassTableTable.h"
#include "IopStatusTable.h"
#include "RqOsTransport.h"
#include "SystemStatusTable.h"

// BuildSys Linkage

CLASSNAME(DdmVirtualMaster,SINGLE);

SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterListenIopState::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterListenNewDevice::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterListenNewRoute::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterListenChangeRoute::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterListenDeleteDevice::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterListenRoutesComplete::RequestCode);

SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterSetNewRoute::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterRouteReady::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterRouteDeleted::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterRoutesComplete::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterFailSlot::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterGetConfig::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterLoadVirtualDevice::RequestCode);
SERVEVIRTUAL(DdmVirtualMaster,RqOsVirtualMasterDeleteVirtualDevice::RequestCode);


// .DdmVirtualMaster -- Constructor -----------------------------------------------DdmVirtualMaster-
//
DdmVirtualMaster::DdmVirtualMaster(DID did) 
	: Ddm(did), fStarted(FALSE),
	  listenersIopState(this),listenersNewDevice(this),listenersNewRoute(this),listenersChangeRoute(this),
	  listenersDeleteDevice(this),listenersRoutesComplete(this),
	  vdc(this)
	{
	TRACE_PROC(DdmVirtualMaster::DdmVirtualMaster);

	DispatchRequest(RqOsVirtualMasterListenIopState::RequestCode,    	REQUESTCALLBACK(DdmVirtualMaster, ProcessListenIopState));
	DispatchRequest(RqOsVirtualMasterListenNewDevice::RequestCode,   	REQUESTCALLBACK(DdmVirtualMaster, ProcessListenNewDevice));
	DispatchRequest(RqOsVirtualMasterListenNewRoute::RequestCode,    	REQUESTCALLBACK(DdmVirtualMaster, ProcessListenNewRoute));
	DispatchRequest(RqOsVirtualMasterListenChangeRoute::RequestCode, 	REQUESTCALLBACK(DdmVirtualMaster, ProcessListenChangeRoute));
	DispatchRequest(RqOsVirtualMasterListenDeleteDevice::RequestCode,	REQUESTCALLBACK(DdmVirtualMaster, ProcessListenDeleteDevice));
	DispatchRequest(RqOsVirtualMasterListenRoutesComplete::RequestCode,	REQUESTCALLBACK(DdmVirtualMaster, ProcessListenRoutesComplete));
	
	DispatchRequest(RqOsVirtualMasterSetNewRoute::RequestCode, 			REQUESTCALLBACK(DdmVirtualMaster, ProcessSetNewRoute));
	DispatchRequest(RqOsVirtualMasterRouteReady::RequestCode,  			REQUESTCALLBACK(DdmVirtualMaster, ProcessRouteReady));
	DispatchRequest(RqOsVirtualMasterRouteDeleted::RequestCode,  		REQUESTCALLBACK(DdmVirtualMaster, ProcessRouteDeleted));
	DispatchRequest(RqOsVirtualMasterRoutesComplete::RequestCode, 		REQUESTCALLBACK(DdmVirtualMaster, ProcessRoutesComplete));
	DispatchRequest(RqOsVirtualMasterFailSlot::RequestCode,    			REQUESTCALLBACK(DdmVirtualMaster, ProcessFailSlot));
	DispatchRequest(RqOsVirtualMasterGetConfig::RequestCode,   			REQUESTCALLBACK(DdmVirtualMaster, ProcessGetConfig));
	DispatchRequest(RqOsVirtualMasterLoadVirtualDevice::RequestCode,   	REQUESTCALLBACK(DdmVirtualMaster, ProcessLoadVirtualDevice));
	DispatchRequest(RqOsVirtualMasterDeleteVirtualDevice::RequestCode, 	REQUESTCALLBACK(DdmVirtualMaster, ProcessDeleteVirtualDevice));
}


// .Enable -- Process Enable ------------------------------------------------------DdmVirtualMaster-
//
STATUS DdmVirtualMaster::Enable(Message *_pMsgEnable) {
	TRACE_PROC(DdmVirtualMaster::Enable);

	ERC erc;
	Message *pMsg;
	
	TRACEF(TRACE_L2,("@ Listening to IOPStatusTable (DdmVirtualMaster::Enable)\n"));
	
	// Listen to PTS IOPStateTable - Only want CurrentState changes
	pMsg = new IOPStatusRecord::RqListen(
					ListenOnInsertRow | ListenOnDeleteAnyRow | ListenOnModifyAnyRowOneField,
					ReplyContinuous | ReplyFirstWithTable | ReplyWithRow,
					CT_IOPST_IOPCURRENTSTATE);
					
	erc = Send(pMsg, REPLYCALLBACK(DdmVirtualMaster, ProcessIOPStatusTableListenReply));
		
	Reply(_pMsgEnable,OK);	// Now Enabled

	return OK;
}

//***								  ***
//*** Process Listen Replies from PTS ***
//***								  ***

// .ProcessIOPStatusTableListenReply -- Process Replies ---------------------------DdmVirtualMaster-
//
// Listen to PTS IOPStatusTable
//
ERC DdmVirtualMaster::ProcessIOPStatusTableListenReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualMaster::ProcessIOPStatusTableListenReply);

	TRACEF(TRACE_L3,("IOPStateTableListen Reply (DdmVirtualMaster::ProcessIOPStatusTableListenReply)\n") );
	if (_pReply->Status() != OK) {
		Tracef("\n*** Replied with erc=%x (DdmVirtualMaster::ProcessIOPStatusTableListenReply)\n",_pReply->Status());
		return OK;
	}

	IOPStatusRecord::RqListen *pListen = (IOPStatusRecord::RqListen*) _pReply;
	IOPStatusRecord *pIsr;
	U32 nTd;

	if (pListen->IsInitialReply()) {
		TRACEF(TRACE_L2,("@ Sending VirtualRouteTable & VirtualDeviceTable Listens (DdmVirtualMaster::ProcessIOPStatusTableListenReply)\n"));

		pIsr = pListen->GetTablePtr(&nTd);
		TRACEF(TRACE_L3,("InitialReply; cRows=%u\n",nTd) );
		// Build VirtualMaster internal IOP state table
		// There should be an entry for every slot
		for (U32 ii=0; ii < nTd; ii++) {
			iop.SetStateIop(pIsr[ii].Slot,pIsr[ii].eIOPCurrentState);
		}
		NotifyIopStates(&iop);

		// Listen to PTS VirtualStateTable
		Message *pMsg;
		ERC erc;

		// Listen to PTS VirtualRouteTable.
		pMsg = new VirtualRouteRecord::RqListen;
		erc = Send(pMsg, REPLYCALLBACK(DdmVirtualMaster,ProcessVirtualRouteTableListenReply));
	
		// Listen to Flags in VirtualDeviceTable. Do not want initial table!
		pMsg = new VirtualDeviceRecord::RqListen(ListenOnModifyAnyRowOneField,ReplyContinuous | ReplyWithRow,VDT_FLAGS_FIELD);
		erc = Send(pMsg, REPLYCALLBACK(DdmVirtualMaster,ProcessVdtFlagsListenReply));
	
		// Listen to fIOPHasDID field in VirtualDeviceTable. Do not want initial table!
		pMsg = new VirtualDeviceRecord::RqListen(ListenOnModifyAnyRowOneField,ReplyContinuous | ReplyWithRow,VDT_FIOPHASDID_FIELD);
		erc = Send(pMsg, REPLYCALLBACK(DdmVirtualMaster,ProcessVdtSawDidListenReply));

		// Listen to entire VirtualDeviceTable.
		pMsg = new VirtualDeviceRecord::RqListen;	// Listen for entire table,Insert,Delete
		erc = Send(pMsg, REPLYCALLBACK(DdmVirtualMaster,ProcessVirtualDeviceTableListenReply));
	}
	else {
		pIsr = pListen->GetModifiedPtr(&nTd);
		TRACEF(TRACE_L3,("ListenReply; cRows=%u\n",nTd) );
		switch (pListen->GetListenTypeRet()) {
		// No Inserts or Deletes expected!!!
		default:
			TRACEF(TRACE_L3,("Unexpected opCode=%u; (DdmVirtualMaster::ProcessIstListenReply)\n",pListen->GetListenTypeRet() ));	

		case ListenOnModifyAnyRowAnyField:
			iop.SetStateIop(pIsr->Slot,pIsr->eIOPCurrentState);
			NotifyIopStates(&iop);
			break;
		}
	}
	DumpStates(iop);

	delete _pReply;
	
	return OK;
}

// .ProcessVirtualRouteTableListenReply -- Process Replies ------------------------DdmVirtualMaster-
//
// Listen to VirtualRouteTable
//
ERC DdmVirtualMaster::ProcessVirtualRouteTableListenReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualMaster::ProcessVirtualRouteTableListenReply);
	
	if (_pReply->Status() != OK) {
		Tracef("\n*** Replied with erc=%u (DdmVirtualMaster::ProcessVirtualRouteTableListenReply)\n",_pReply->DetailedStatusCode);
		return OK;
	}

	VirtualRouteRecord::RqListen *pListen = (VirtualRouteRecord::RqListen *) _pReply;
	
//	ERC erc;
	TRACEF(TRACE_L3,("** Listen to PTS VirtualRouteTable\n"));
	VirtualRouteRecord *pVrr;
	U32 nVrr;

	if (pListen->IsInitialReply()) {
		TRACEF(TRACE_L2,("@ Process initial VirtualRouteTable Listen (DdmVirtualMaster::ProcessVirtualRouteTableListenReply)\n"));
		pVrr = pListen->GetTablePtr(&nVrr);	
		TRACEF(TRACE_L3,("InitialReply; cRows=%u\n",nVrr));
//		for (U32 ii=0; ii < nVdt; ii++)
//			DumpVrrRecord(pVrr+ii);		//***DEBUG***

		vdc.Build(pVrr,nVrr);
		NotifyNewRoutesAll();
	}
	else {
		pVrr = pListen->GetModifiedPtr(&nVrr);
		TRACEF(TRACE_L3,("UpdateReply; cRows=%u\n",nVrr));
//		DumpVdrRecord(pVrr);		//***DEBUG***

		if (pListen->GetListenTypeRet() == ListenOnInsertRow) {
			// VirtualRoute Added
			TRACEF(TRACE_L3,("** Insert VRT ROW\n"));
			
			Entry *pEntry = vdc.Add(pVrr);
			NotifyNewRoutes(pEntry);
		}
	}

	delete _pReply;
	return OK;
}


// .ProcessVirtualDeviceTableListenReply -- Process Replies -----------------------DdmVirtualMaster-
//
// Listen to VirtualDeviceTable
//
ERC DdmVirtualMaster::ProcessVirtualDeviceTableListenReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualMaster::ProcessVirtualDeviceTableListenReply);
	
	TRACEF(TRACE_L3, ("RqPtsVdtListen Reply.(DdmVirtualMaster::ProcessVirtualDeviceTableListenReply)\n"));
	
	if (_pReply->Status() != OK) {
		Tracef("\n*** Replied with erc=%u (DdmVirtualMaster::ProcessVirtualDeviceTableListenReply)\n",_pReply->Status());
		return OK;
	}
	VirtualDeviceRecord::RqListen *pListen = (VirtualDeviceRecord::RqListen *) _pReply;
	if (_pReply->DetailedStatusCode != OK) {
		Tracef("\n*** Replied erc=%u (DdmVirtualMaster::ProcessVirtualDeviceTableListenReply)\n",_pReply->DetailedStatusCode);
		return OK;
	}
//	ERC erc;
	TRACEF(TRACE_L3,("** Listen to PTS VirtualDeviceTable\n"));
	VirtualDeviceRecord *pVdt;
	U32 nVdt;
	
	if (pListen->IsInitialReply()) {
		TRACEF(TRACE_L2,("@ Process initial VirtualDeviceTable Listen (DdmVirtualMaster::ProcessVirtualDeviceTableListenReply)\n"));

		ridVdt = pListen->payload.tableId;	// Save the tableId of the VirtualDeviceTable
		
		pVdt = pListen->GetTablePtr(&nVdt);	
		TRACEF(TRACE_L3,("InitialReply; cRows=%u\n",nVdt));
		idVdt = pVdt->rid.Table;
		
		for (U32 ii=0; ii < nVdt; ii++)
			DumpVdtRecord(pVdt+ii);		//***DEBUG***
		
		vdc.Build(pVdt,nVdt);
		fStarted = TRUE;
		
		NotifyNewDevicesAll();
		NotifyNewRoutesAll();
	}
	else {
		pVdt = pListen->GetModifiedPtr(&nVdt);
		TRACEF(TRACE_L3,("UpdateReply; cRows=%u\n",nVdt));
		DumpVdtRecord(pVdt);		//***DEBUG***

		Entry *pEntry;
		switch (pListen->GetListenTypeRet()) {
		default:
			TRACEF(TRACE_L3,("Unexpected Listen opCode=%u; (DdmVirtualMaster::ProcessVirtualDeviceTableListenReply)\n",pListen->GetListenTypeRet() ));	

		case ListenOnModifyAnyRowAnyField:
			TRACEF(TRACE_L3,("VirtualDeviceRecord Changed (DdmVirtualMaster::ProcessVirtualDeviceTableListenReply)\n"));
			break;
			
		// VirtualDevice Added
		case ListenOnInsertRow:
			TRACEF(TRACE_L3,("** Insert VDT ROW\n"));	
			
			// Add new VD to the Inactive Device list
			// Devices with no slots specified are discarded
			pEntry = vdc.Add(pVdt);
			// Notify Listeners of NewDevices
			NotifyNewDevices(pEntry);

			break;

		// VirtualDevice Deleted
		case ListenOnDeleteAnyRow:	
			TRACEF(TRACE_L3,("** Delete VDT ROW %u^%u\n",pVdt->rid.GetTable(),pVdt->rid.GetRow()) );

			// What happens if the IOP is creating the device
			// when this comes in?!?
			VDN vdn = DeleteDevice(pVdt);	// Delete from local cache
			//**Need to delete routing***
			break;
			
		}
	}

	delete _pReply;
	return OK;
}

// .ProcessVdtFlagsListenReply -- Process Replies ---------------------------------DdmVirtualMaster-
//
// Listen to Flags field of VirtualDeviceTable
//
ERC DdmVirtualMaster::ProcessVdtFlagsListenReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualMaster::ProcessVdtFlagsListenReply);

	if (_pReply->Status() != OK) {
		Tracef("\n*** Replied with erc=%u (DdmVirtualMaster::ProcessVdtFlagsListenReply)\n",_pReply->Status());
		return OK;
	}
	VirtualDeviceRecord::RqListen *pReply = (VirtualDeviceRecord::RqListen *) _pReply;

	if (pReply->IsInitialReply()) {
		TRACEF(TRACE_L2,("Process initial VirtualDeviceTable Listen on flags (DdmVirtualMaster::Enable)\n"));
		delete _pReply;
		return OK;
	}
	U32 nVdt;
	VirtualDeviceRecord *pVdt = pReply->GetModifiedPtr(&nVdt);
	
	TRACEF(TRACE_L3,("ModifyVdtFlags; cRows=%u\n",nVdt));
	DumpVdtRecord(pVdt);		//***DEBUG***
	
	VirtualDeviceRecord::RqModifyField *pModify;
	VDN vdn = pVdt->rid.LoPart;
	Entry *pEntry = vdc.Find(vdn);

	// If not yet active..
	if ( !(pVdt->eVDFlags & eVdFlagMask_Instanciated) && pVdt->slotPrimary != IOP_LOCAL) {
		// Save Dids for failover
		pEntry->didPrimary = pVdt->didPrimary;
		pEntry->didSecondary = pVdt->didSecondary;
	
		// If both dids have been set...
		if ((pVdt->didPrimary   != DIDNULL || pVdt->slotPrimary   == IOP_NONE || pVdt->slotPrimary   == SLOTNULL) &&
			(pVdt->didSecondary != DIDNULL || pVdt->slotSecondary == IOP_NONE || pVdt->slotSecondary == SLOTNULL)) {

			TRACEF(TRACE_L2,("@ Setting VDN=%u record to active/instanciated (DdmVirtualMaster::ProcessVdtFlagsListenReply)\n",vdn));

			// Set didActive
			pModify = new VirtualDeviceRecord::RqModifyField(pVdt->rid, VDT_DID_ACTIVE_FIELD,&pVdt->didPrimary,sizeof(DID));
			Send(pModify,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));

			// Set VDT Instanciated flag
			EnumVDFlags flags = (EnumVDFlags) eVdFlagMask_Instanciated;	
			VirtualDeviceRecord::RqModifyBits *pModBits = new VirtualDeviceRecord::RqModifyBits(OpOrBits, pVdt->rid, VDT_FLAGS_FIELD, &flags, sizeof(EnumVDFlags));
			Send(pModBits,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));
		}
	}
	else {	// Now active
		if (pVdt->slotPrimary != IOP_LOCAL) {
			if (!(pEntry->eVDFlags & eVdFlagMask_Instanciated) ) {	// Active in cache?
				// Notify Listeners of NewRoute
				TRACEF(TRACE_L2,("@ Notifying new route VDN=%u (DdmVirtualMaster::ProcessVdtFlagsListenReply)\n",vdn));
				NotifyNewRoutes(vdc.Add(pVdt));
			}
			if ( (pEntry->eVDFlags & eVDFlagMask_Alternate) != (pVdt->eVDFlags & eVDFlagMask_Alternate)) {
				TRACEF(TRACE_L2,("@ Notifying change route VDN=%u (DdmVirtualMaster::ProcessVdtFlagsListenReply)\n",vdn));
				NotifyChangeRoutes(vdc.Add(pVdt));
			}
		}	
		if ( (pEntry->eVDFlags & eVDFlagMask_Delete) != (pVdt->eVDFlags & eVDFlagMask_Delete)) {
			TRACEF(TRACE_L2,("@ Notifying delete route VDN=%u (DdmVirtualMaster::ProcessVdtFlagsListenReply)\n",vdn));
			NotifyDeleteDevices(vdn);
		}
	}
	pEntry->eVDFlags = pVdt->eVDFlags;
	
	delete _pReply;
	
	return OK;	
}

// .ProcessVdtSawDidListenReply -- Process Replies --------------------------------DdmVirtualMaster-
//
// Listen to PTS VirtualDeviceTable .fSawDid field
//
ERC DdmVirtualMaster::ProcessVdtSawDidListenReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualMaster::ProcessVdtSawDidListenReply);

	TRACEF(TRACE_L3,("VdtSawDidListen Reply (DdmVirtualMaster::ProcessVdtSawDidListenReply)\n") );

	if (_pReply->Status() != OK) {
		Tracef("\n*** Replied with erc=%u (DdmVirtualMaster::ProcessVdtSawDidListenReply)\n",_pReply->Status());
		return OK;
	}
	VirtualDeviceRecord::RqListen *pReply = (VirtualDeviceRecord::RqListen*) _pReply;
	
	U32 nVdt;
	VirtualDeviceRecord *pVdt = pReply->GetModifiedPtr(&nVdt);
	if (nVdt == 0) {
		TRACEF(TRACE_L2,("@ Process initial VirtualDeviceTable Listen on SawDid (DdmVirtualMaster::ProcessVdtSawDidListenReply)\n"));
		delete _pReply;
		return OK;
	}
	VDN vdn = pVdt->rid.LoPart;
	
	// This may not work if a record is inserted then flagged for delete before any IOP can update it.
	// Need to add a fIOPSawDevice and not deleted a record until all IOPs have seen the insert.
	
	if (pVdt->fIOPHasDID == 0 && (pVdt->eVDFlags & eVDFlagMask_Delete)) {
		TRACEF(TRACE_L2,("@ Deleting VirtualDeviceTable record Vdn=%u (DdmVirtualMaster::ProcessVdtSawDidListenReply)\n",vdn));
		Send(new VirtualDeviceRecord::RqDeleteRow(pVdt->rid), REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));
	}
	else if (vdc.SetRouteCompleteFlags(vdn,pVdt->fIOPHasDID,iop.GetIopMask()) ) {
		// Notify Listeners of Completed Routes
		TRACEF(TRACE_L2,("@ Notifying routes complete nDevices=%u; nRoutesComplete=%u (DdmVirtualMaster::ProcessVdtSawDidListenReply)\n",vdc.nDevices,vdc.nRoutesComplete));
		RqOsVirtualMasterListenRoutesComplete *pNotify = new RqOsVirtualMasterListenRoutesComplete(vdc.nDevices,vdc.nRoutesComplete);
		listenersRoutesComplete.Notify(pNotify);
		delete pNotify;
	}
	
	delete _pReply;
	
	return OK;
}


//***						 ***
//*** Served Listen Requests ***
//***						 ***

// .ProcessListenIopState -- Process Listen Request  ------------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::ProcessListenIopState(Message *_pListen) {
	TRACE_PROC(DdmVirtualMaster::ProcessListenIopState);
	
	RqOsVirtualMasterListenIopState *pListen = (RqOsVirtualMasterListenIopState*) _pListen;
	
	listenersIopState.Add(pListen);
	
	// Return current inactive device list
	pListen->AddStatesReply(&iop);
	Reply(pListen,OK,FALSE);

	return OK;
}

// .ProcessListenNewDevice -- Process Listen Request ------------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::ProcessListenNewDevice(Message *_pListen) {
	TRACE_PROC(DdmVirtualMaster::ProcessListenNewDevice);
	
	RqOsVirtualMasterListenNewDevice *pListen = (RqOsVirtualMasterListenNewDevice*) _pListen;
	
	listenersNewDevice.Add(pListen);

	// Return current device list
	for (Entry *pEntry = vdc.First(); pEntry != NULL; pEntry=pEntry->Next()) {
		// Only send devices not yet instanciated
		if (pEntry->pRoute == NULL) {
			pListen->AddDeviceReply(pEntry->pDevice,1);
			Reply(pListen,OK,FALSE);
		}
	}
	return OK;
}

// .ProcessListenNewRoute -- Process Listen Request -------------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::ProcessListenNewRoute(Message *_pListen) {
	TRACE_PROC(DdmVirtualMaster::ProcessListenNewRoute);
	
	RqOsVirtualMasterListenNewRoute *pListen = (RqOsVirtualMasterListenNewRoute*) _pListen;
	listenersNewRoute.Add(pListen);

	// Return current route list
	for (Entry *pEntry = vdc.First(); pEntry != NULL; pEntry=pEntry->Next()) {
		// Only send instanciated routes
		if (pEntry->pRoute) {
			pListen->AddRouteReply(pEntry->pRoute,1);
			Reply(pListen,OK,FALSE);
		}
	}
	return OK;
}

// .ProcessListenChangeRoute -- Process Listen Request ----------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::ProcessListenChangeRoute(Message *_pListen) {
	TRACE_PROC(DdmVirtualMaster::ProcessListenChangeRoute);
	
	RqOsVirtualMasterListenChangeRoute *pListen = (RqOsVirtualMasterListenChangeRoute*) _pListen;
	
	listenersChangeRoute.Add(pListen);

	// No reply. Wait for device changes

	return OK;
}

// .ProcessListenDeleteDevice -- Process Listen Request ---------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::ProcessListenDeleteDevice(Message *_pListen) {
	TRACE_PROC(DdmVirtualMaster::ProcessListenDeleteDevice);
	
	RqOsVirtualMasterListenDeleteDevice *pListen = (RqOsVirtualMasterListenDeleteDevice*) _pListen;
	
	listenersDeleteDevice.Add(pListen);

	// No reply. Wait for device deletions

	return OK;
}

// .ProcessListenRoutesComplete -- Process Listen Request -------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::ProcessListenRoutesComplete(Message *_pListen) {
	TRACE_PROC(DdmVirtualMaster::ProcessListenRoutesComplete);
	
	RqOsVirtualMasterListenRoutesComplete *pListen = (RqOsVirtualMasterListenRoutesComplete *) _pListen;
	
	listenersRoutesComplete.Add(pListen);

	if (fStarted) {
		pListen->SetReply(vdc.nDevices,vdc.nRoutesComplete);
		Reply(pListen,OK,FALSE);
	}
	return OK;
}

//***				  ***
//*** Served Requests ***
//***				  ***

// .ProcessSetNewRoute -- Process Request -----------------------------------------DdmVirtualMaster-
//
// This request tells us a new device has been instanciated
// with the attached routing information.
//
ERC DdmVirtualMaster::ProcessSetNewRoute(Message *_pRequest) {
	TRACE_PROC(DdmVirtualMaster::ProcessSetNewRoute);
	
	RqOsVirtualMasterSetNewRoute *pRequest = (RqOsVirtualMasterSetNewRoute*) _pRequest;
		
	VirtualRoute route;
	pRequest->GetRoute(&route);
	
	TRACEF(TRACE_L2,("@ Received SetNewRoute VDN=%u; DID=%x (DdmVirtualMaster::ProcessSetNewRoute)\n",route.vdn,route.did));

	ERC erc = UpdateDevice(route);
	
	Reply(pRequest,erc);
	
	return OK;
}


// .ProcessRouteReady -- Process Request ------------------------------------------DdmVirtualMaster-
//
// This request notifies us when an IOP has seen a route
//
ERC DdmVirtualMaster::ProcessRouteReady(Message *_pRequest) {
	TRACE_PROC(DdmVirtualMaster::ProcessRouteReady);

	RqOsVirtualMasterRouteReady	*pRequest = (RqOsVirtualMasterRouteReady*) _pRequest;
	
	RowId rid(idVdt,pRequest->payload.vdn);

	TRACEF(TRACE_L2,("@ Received RouteReady VDN=%u; slot=%x (DdmVirtualMaster::ProcessRouteReady)\n",rid.LoPart,pRequest->payload.slot));

	// Update "SawDid" flag
	U32 fIOPHasDID = (1 << pRequest->payload.slot);
	
	VirtualDeviceRecord::RqModifyBits *pModBits = new VirtualDeviceRecord::RqModifyBits(OpOrBits, rid, VDT_FIOPHASDID_FIELD, &fIOPHasDID, sizeof(fIOPHasDID));
	Send(pModBits,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));
	
	Reply(pRequest,OK);
	
	return OK;
}

// .ProcessRouteDeleted -- Process Request ----------------------------------------DdmVirtualMaster-
//
// This request notifies us when an IOP has deleted a route/device
//
ERC DdmVirtualMaster::ProcessRouteDeleted(Message *_pRequest) {
	TRACE_PROC(DdmVirtualMaster::ProcessRouteDeleted);

	RqOsVirtualMasterRouteDeleted *pRequest = (RqOsVirtualMasterRouteDeleted*) _pRequest;
	
	RowId rid(idVdt,pRequest->payload.vdn);
	
	TRACEF(TRACE_L2,("@ Received RouteDeleted VDN=%u; slot=%x (DdmVirtualMaster::ProcessRouteDeleted)\n",rid.LoPart,pRequest->payload.slot));

	// Clear "SawDid" flag
	U32 fIOPHasDID = ~(1 << pRequest->payload.slot);
	
	VirtualDeviceRecord::RqModifyBits *pModBits = new VirtualDeviceRecord::RqModifyBits(OpAndBits, rid, VDT_FIOPHASDID_FIELD, &fIOPHasDID, sizeof(fIOPHasDID));
	Send(pModBits,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));
	
	Reply(pRequest,OK);
	
	return OK;
}

// .ProcessRoutesComplete -- Process Request --------------------------------------DdmVirtualMaster-
//
// Informs VirtualMaster that IOP has seen fAllRoutesComplete
//
ERC DdmVirtualMaster::ProcessRoutesComplete(Message *_pRequest) {
	TRACE_PROC(DdmVirtualMaster::ProcessRoutesComplete);

	RqOsVirtualMasterRoutesComplete *pRequest = (RqOsVirtualMasterRoutesComplete*) _pRequest;
	
	TRACEF(TRACE_L2,("@ Received RoutesComplete Slot=%x. (DdmDdmVirtualMaster::ProcessRoutesComplete)\n",pRequest->payload.slot));

	// Set the state in the IOPStatusTable to operating.
//	MsgCmbSetMipsState *pSet = new MsgCmbSetMipsState(MsgCmbSetMipsState::SetOperating);
//	Send(pSet, REPLYCALLBACK(DdmVirtualManager,DiscardOkReply));

	// Also set our active bit in the system status table.
	// First compute our active mask bit.
	U32 myfIOPActiveMaskBit = (1 << 	pRequest->payload.slot);

	RqPtsModifyBits	*pSetMyBit = new RqPtsModifyBits(
		SystemStatusRecord::TableName(),	// const char *_psTableName,
		OpOrBits,							// fieldOpType _opFlag,
		CT_PTS_ALL_ROWS_KEY_FIELD_NAME,		// const char *_psKeyFieldName,
		NULL,								// const void *_pKeyFieldValue,
		0,									// U32 _cbKeyFieldValue,
		CT_SYSST_ACTIVEIOPSMASK,			// const char *_psFieldName,
		&myfIOPActiveMaskBit,				// const void *_pbFieldMask,
		sizeof(U32),						// U32 _cbFieldMask,
		(U32)0								// U32 _cRowsToModify=0
	);
	Send(pSetMyBit, REPLYCALLBACK(DdmVirtualManager,DiscardOkReply));

	Reply(_pRequest,OK);

	return OK;
}

// .ProcessFailSlot -- Process Request --------------------------------------------DdmVirtualMaster-
//
// ** NEED TO CLEAR fSawDid of failing slot for ALL VirtualDeviceRecord(s) **
//
ERC DdmVirtualMaster::ProcessFailSlot(Message *_pRequest) {
	TRACE_PROC(DdmVirtualMaster::ProcessFailSlot);
	
	RqOsVirtualMasterFailSlot *pRequest = (RqOsVirtualMasterFailSlot*) _pRequest;
	TySlot slot = pRequest->payload.slot;
	DID did;
	
	TRACEF(TRACE_L2,("@ Received FailSlot Slot=%x. (DdmDdmVirtualMaster::ProcessRoutesComplete)\n",slot));

	for (Entry *pEntry=vdc.First(); pEntry != NULL; pEntry = pEntry->Next()) {
		if (pEntry->pRoute && DeviceId::ISlot(pEntry->pRoute->did) == slot) {
			did = (pEntry->didPrimary != pEntry->pRoute->did) ? pEntry->didPrimary : pEntry->didSecondary;
			if (did == DIDNULL || did == pEntry->pRoute->did)
				Tracef("WARNING: Unable to failover device! VDN=%u DID=%x (DdmVirtualMaster::ProcessFailSlot)\n",pEntry->vdn,did);
			else {
				Tracef("FAILOVER: Class \"%s\" Vdn=%u; was did=%x; new did=%x (DdmVirtualMaster::ProcessFailSlot)\n",
						pEntry->pDevice->szClassName,pEntry->vdn,pEntry->pRoute->did,
						(pEntry->didPrimary != pEntry->pRoute->did) ? pEntry->didPrimary : pEntry->didSecondary);
				
				RowId rid(idVdt,pEntry->vdn);
				EnumVDFlags flags;
				VirtualDeviceRecord::RqModifyField *pModFld;
				VirtualDeviceRecord::RqModifyBits *pModBits;
				
				RqOsTransportVdnStop *pStop = new RqOsTransportVdnStop(pEntry->vdn);
				Send(pStop,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));
				
				// Set didActive
				pModFld = new VirtualDeviceRecord::RqModifyField(rid, VDT_DID_ACTIVE_FIELD,&did,sizeof(DID) );
				Send(pModFld,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));

				// Clear "HadDID" flags
				U32 bits = 0;
				pModFld = new VirtualDeviceRecord::RqModifyField(rid, VDT_FIOPHASDID_FIELD, &bits,sizeof(U32));
				Send(pModFld,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));

				// Set/Clear Primary flag
				if (did != pEntry->didPrimary) {
					flags = (EnumVDFlags) eVDFlagMask_Alternate;	
					pModBits = new VirtualDeviceRecord::RqModifyBits(OpAndBits, rid, VDT_FLAGS_FIELD, &flags, sizeof(EnumVDFlags));
					Send(pModBits,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));
				}
				else {
					flags = (EnumVDFlags) ~eVDFlagMask_Alternate;
					pModBits = new VirtualDeviceRecord::RqModifyBits(OpOrBits, rid, VDT_FLAGS_FIELD, &flags, sizeof(EnumVDFlags));
					Send(pModBits,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));
				}
			}
		}
	}
	Reply(_pRequest,OK);

	return OK;
}

// .ProcessGetConfig -- Process Request -------------------------------------------DdmVirtualMaster-
//
// Lookup className of VDN in ClassTableTable and get config TableName
// Read RowId of config data for class
// Return class to requestor. 
//
ERC DdmVirtualMaster::ProcessGetConfig(Message *_pRequest) {
	TRACE_PROC(DdmVirtualMaster:ProcessGetConfig);
	
	RqOsVirtualMasterGetConfig *pRequest = (RqOsVirtualMasterGetConfig*) _pRequest;
	
	Entry *pEntry = vdc.Find(pRequest->payload.vdn);
	pRequest->payload.ridCfg = pEntry->pDevice->ridCfg;

	TRACEF(TRACE_L2,("@ Received GetConfig VDN=%u. (DdmDdmVirtualMaster::ProcessGetConfig)\n",pRequest->payload.vdn));
	
	if (pEntry==NULL || pEntry->pDevice->ridCfg.IsClear()) {
		// Reply with config data
		pRequest->AddConfigDataReply(NULL, 0);
		Reply(pRequest,OK);

		return OK;
	}
	Device *pDevice = pEntry->pDevice;
	
	// Generic Read of config data
	RqPtsReadRow *pRead = new RqPtsReadRow(NULL,CT_PTS_RID_FIELD_NAME, &pDevice->ridCfg, sizeof(RowId));
	Send(pRead, pRequest, REPLYCALLBACK(DdmVirtualMaster,ProcessReadConfigTableReply));
	
	return OK;
}

// .ProcessReadConfigTableReply -- Process Reply -----------------------------------DdmVirtualMaster-
//
// Lookup className of VDN in ClassTableTable and get config TableName
// Read RowId of config data for class
// Return class to requestor. 
//
ERC DdmVirtualMaster::ProcessReadConfigTableReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualMaster:ProcessReadConfigTableReply);
	
	if (_pReply->Status() != OK) {
		Tracef("ERROR: Unexpected reply erc=%u (DdmVirtualMaster:ProcessReadConfigTableReply)\n",_pReply->Status());
	}
	RqPtsReadRow *pReply = (RqPtsReadRow*) _pReply;
	RqOsVirtualMasterGetConfig *pRequest = (RqOsVirtualMasterGetConfig*) pReply->GetContext();

	// Reply with config data
	pRequest->AddConfigDataReply(pReply->GetRowDataPtr(), pReply->GetRowDataSize() );
	
	if (!pRequest->payload.fGetTableDef) {
		Reply(pRequest,OK);
	}
	else {
		RqPtsGetTableDef *pGetDefMsg = new RqPtsGetTableDef(pRequest->payload.ridCfg);
		Send(pGetDefMsg, pRequest, REPLYCALLBACK(DdmVirtualMaster,ProcessGetTableDefReply));
	}
	delete pReply;
	
	return OK;	
}

// .ProcessGetTableDefReply -- Process Reply ---------------------------------------DdmVirtualMaster-
//
// Lookup className of VDN in ClassTableTable and get config TableName
// Read RowId of config data for class
// Return class to requestor. 
//
ERC DdmVirtualMaster::ProcessGetTableDefReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualMaster:ProcessGetTableDefReply);

	if (_pReply->Status() != OK) {
		Tracef("ERROR: Unexpected reply erc=%u (DdmVirtualMaster:ProcessGetTableDefReply)\n",_pReply->Status());
	}
	
	RqPtsGetTableDef *pReply = (RqPtsGetTableDef*) _pReply;
	RqOsVirtualMasterGetConfig *pRequest = (RqOsVirtualMasterGetConfig*) pReply->GetContext();

	pRequest->AddTableDefReply(pReply->GetTableDefDataPtr(),pReply->GetTableDefDataSize());
	pRequest->AddFieldDefsReply(pReply->GetFieldDefsDataPtr(),pReply->GetFieldDefsDataSize());
		
	Reply(pRequest,OK);
	
	delete _pReply;

	return OK;
}

// .ProcessLoadVirtualDevice -- Process Request -------------------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::ProcessLoadVirtualDevice(Message *_pRequest) {
	TRACE_PROC(DdmVirtualMaster::ProcessLoadVirtualDevice);

	RqOsVirtualMasterLoadVirtualDevice *pRequest = (RqOsVirtualMasterLoadVirtualDevice*) _pRequest;
	
	TRACEF(TRACE_L2,("@ Received LoadVirtualDevice \"%s\" (DdmDdmVirtualMaster::ProcessLoadVirtualDevice)\n",pRequest->payload.szClassName));

	DdmSvcVirtualDeviceLoader *pLoader = new DdmSvcVirtualDeviceLoader(this);
	
	pLoader->Execute(&pRequest->payload, &iop, pRequest,  ACTIONCALLBACK(DdmVirtualMaster,ProcessLoadVirtualDeviceReply));

	return OK;
}

// .ProcessLoadVirtualDeviceReply -- Process Replies --------------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::ProcessLoadVirtualDeviceReply(void *_pLoader) {
	TRACE_PROC(DdmVirtualMaster::ProcessLoadVirtualDeviceReply);

	DdmSvcVirtualDeviceLoader *pLoader = (DdmSvcVirtualDeviceLoader*) _pLoader;

	RqOsVirtualMasterLoadVirtualDevice *pRequest = (RqOsVirtualMasterLoadVirtualDevice *) pLoader->GetContext();
	RqOsVirtualMasterLoadVirtualDevice::Payload *pLoadDevice = pLoader->GetLoadDevicePtr();
	
	TRACEF(TRACE_L2,("@ Completed LoadVirtualDevice \"%s\"; VDN=%u; ERC=%x (DdmDdmVirtualMaster::ProcessDeleteVirtualDevice)\n",pRequest->payload.szClassName,pRequest->payload.vdnRet,pLoader->Status()) );

	pRequest->SetReply(pLoadDevice);
	Reply(pRequest,pLoader->Status());

	delete _pLoader;

	return OK;
}

// .ProcessDeleteVirtualDevice -- Process Request -----------------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::ProcessDeleteVirtualDevice(Message *_pRequest) {
	TRACE_PROC(DdmVirtualMaster::ProcessDeleteVirtualDevice);

	RqOsVirtualMasterDeleteVirtualDevice *pRequest = (RqOsVirtualMasterDeleteVirtualDevice*) _pRequest;
	
	TRACEF(TRACE_L2,("@ Received DeleteVirtualDevice VDN=%u (DdmDdmVirtualMaster::ProcessDeleteVirtualDevice)\n",pRequest->payload.vdn));

	DdmSvcVirtualDeviceUnloader *pUnloader = new DdmSvcVirtualDeviceUnloader(this);
	
	RowId rid(ridVdt.GetTable(),pRequest->payload.vdn);
	
	pUnloader->Execute(rid, pRequest,  ACTIONCALLBACK(DdmVirtualMaster,ProcessDeleteVirtualDeviceReply));

	// No reply to _pRequest
	return OK;
}

// .ProcessDeleteVirtualDeviceReply -- Process Replies ------------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::ProcessDeleteVirtualDeviceReply(void *_pLoader) {
	TRACE_PROC(DdmVirtualMaster::ProcessDeleteVirtualDeviceReply);

	DdmSvcVirtualDeviceUnloader *pLoader = (DdmSvcVirtualDeviceUnloader*) _pLoader;

	RqOsVirtualMasterDeleteVirtualDevice *pRequest = (RqOsVirtualMasterDeleteVirtualDevice*) pLoader->GetContext();

	TRACEF(TRACE_L2,("@ Completed DeleteVirtualDevice VDN=%u; ERC=%x (DdmDdmVirtualMaster::ProcessDeleteVirtualDevice)\n",pRequest->payload.vdn,pLoader->Status()) );
	
	Reply(pRequest, pLoader->Status());

	delete _pLoader;

	return OK;
}

//***							***
//*** DdmSvcVirtualDeviceLoader ***
//***							***


// .Execute -- Load Virtual Device --------------------------------------------DdmSvcVirtualDeviceLoader-
//
// Do not allow duplicates. Key is ridDdmCfgRec && ridVDOwnerUse && szClassName
// Insert Record into VDT
// Wait for Listen on fIOPHasDid equal to current (changing) IOP mask
// Reply with VDN/didPrimary/didSecondary
//
ERC DdmSvcVirtualDeviceLoader::Execute(LoadDevice *_pLoadDevice, States *_pIopStates,void *_pContext,ActionCallback _callback) {
	TRACE_PROC(DdmSvcVirtualDeviceLoader::ProcessLoadVirtualDevice);

	pLoadDevice = _pLoadDevice;
	pIopStates = _pIopStates;
	pContext = _pContext;
	callback = _callback;
		
	idListen = 0;
	
	VirtualDeviceRecord recVdt(_pLoadDevice->szClassName,_pLoadDevice->slotPrimary,_pLoadDevice->slotSecondary,_pLoadDevice->fAutoStart,_pLoadDevice->ridDdmCfgRec,_pLoadDevice->ridVDOwnerUse);
	
	// Insert the VirtualDeviceRecord
	VirtualDeviceRecord::RqInsertRow *pMsg = new VirtualDeviceRecord::RqInsertRow(&recVdt,1);
	ERC erc = Send(pMsg, REPLYCALLBACK(DdmVirtualMaster,ProcessLoadVirtualDeviceReply));

	TRACEF(TRACE_L3, ("@ INSERTING Class=\"%s\"; slotPrimary=%x; slotSecondary=%x; ridCfgRec=%u^%u; ridOwnerUse=%u^%u\n",
			  _pLoadDevice->szClassName,_pLoadDevice->slotPrimary,_pLoadDevice->slotSecondary,
			  _pLoadDevice->ridDdmCfgRec.GetTable(),_pLoadDevice->ridDdmCfgRec.GetRow(),
			  _pLoadDevice->ridVDOwnerUse.GetTable(),_pLoadDevice->ridVDOwnerUse.GetRow() ));
	
	return OK;
}

// .ProcessLoadVirtualDeviceReply -- Process Replies --------------------------DdmSvcVirtualDeviceLoader-
//
ERC DdmSvcVirtualDeviceLoader::ProcessLoadVirtualDeviceReply(Message *_pReply) {
	TRACE_PROC(DdmSvcVirtualDeviceLoader::ProcessLoadVirtualDeviceReply);

	if (_pReply->Status() != OK) {
		Tracef("\n[ERROR] Reply erc=%u (DdmVirtualMaster::ProcessLoadVirtualDeviceReply)\n\n",_pReply->Status());

		status = _pReply->Status();
		Action(pParentDdmSvs,callback,this);
	}
	else {
		VirtualDeviceRecord::RqInsertRow *pReply = (VirtualDeviceRecord::RqInsertRow *) _pReply;
		RowId rid = *pReply->GetRowIdPtr();

		// Listen to inserted row for fIOPHasDID field changes. Do not want initial table.
		VirtualDeviceRecord::RqListen *pListen = new VirtualDeviceRecord::RqListen(ListenOnModifyOneRowOneField, ReplyWithRow, //| ReplyOnceOnly, 
										rid,
										VDT_FIOPHASDID_FIELD); //, &iopMask, sizeof(U32));
		Send(pListen, REPLYCALLBACK(DdmSvcLoadVirtualDevice,ProcessLoadVirtualDeviceListenReply));
	}
	delete _pReply;
	return OK;
}

// .ProcessLoadVirtualDeviceListenReply -- Process Replies --------------------DdmSvcVirtualDeviceLoader-
//
// Listen to PTS VirtualDeviceTable fIOPHasDid field
//
ERC DdmSvcVirtualDeviceLoader::ProcessLoadVirtualDeviceListenReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualMaster::ProcessLoadVirtualDeviceListenReply);

	if (_pReply->Status() != OK) {
		Tracef("\n[ERROR] Reply erc=%u (DdmVirtualMaster::ProcessLoadVirtualDeviceListenReply)\n\n",_pReply->Status());

		CompleteListen(_pReply->Status());
		
		delete _pReply;
		return OK;
	}
	VirtualDeviceRecord::RqListen *pReply = (VirtualDeviceRecord::RqListen*) _pReply;
	
	// Only our listen for this rid should come back to us.
	// No data on initial reply - just ListenerId.
	if (pReply->IsInitialReply()) {
		idListen = pReply->GetListenerId();
		delete _pReply;
		return OK;
	}
	if (idListen == 0) {	// Too late! - ignore
		delete _pReply;
		return OK;
	}
	
	U32 nVdr;
	VirtualDeviceRecord *pVdr = pReply->GetModifiedPtr(&nVdr);
	VDN vdn = pVdr->rid.LoPart;
	
	if (pVdr->fIOPHasDID != pIopStates->GetIopMask()) {
		TRACEF(TRACE_L2,("@ LOADING VDN=%u...fIOPHasDid=%x; iopState=%x (DdmSvcVirtualDeviceLoader)\n",vdn,pVdr->fIOPHasDID,pIopStates->GetIopMask()) );
		delete _pReply;
		return OK;
	}

	TRACEF(TRACE_L2, ("@ LOAD COMPLETE \"%s\"; didP=%x; didS=%x; ridCfg=%u^%u; ridOwner=%u^%u (ProcessLoadVirutalDeviceListenReply)\n",
			  pVdr->szClassName,pVdr->didPrimary,pVdr->didSecondary,
			  pVdr->ridDdmCfgRec.GetTable(),pVdr->ridDdmCfgRec.GetRow(),
			  pVdr->ridVDOwnerUse.GetTable(),pVdr->ridVDOwnerUse.GetRow() ));
	
	pLoadDevice->SetReply(vdn,pVdr->didPrimary,pVdr->didSecondary);
	CompleteListen(OK);
	
	delete _pReply;
	return OK;
}

// .CompleteListen -- Begin completion activity ---------------------------------------DdmSvcVirtualDeviceLoader-
//
// Assumes Listen is active with PTS
//
ERC DdmSvcVirtualDeviceLoader::CompleteListen(ERC erc) {
	TRACE_PROC(DdmSvcVirtualDeviceLoader::CompleteListen);
	
	status = erc;
	
	if (idListen) {
		RqPtsStopListen *pStop = new RqPtsStopListen(idListen);
		Send(pStop, REPLYCALLBACK(DdmSvcLoadVirtualDevice,ProcessCompleteStopListenReply));
		idListen = 0;
	}
	else
		Action(pParentDdmSvs,callback,this);

	return OK;
}

// .ProcessCompleteStopListenReply -- Process Replies ---------------------------DdmSvcVirtualDeviceLoader-
//
// Callback parent when Listen Stops.
// Uses current .status code.
//
ERC DdmSvcVirtualDeviceLoader::ProcessCompleteStopListenReply(Message *_pReply) {
	TRACE_PROC(DdmVirtualMaster::ProcessCompleteStopListenReply);

	Action(pParentDdmSvs,callback,this);
//	(pParentDdmSvs->*callback)(this);	// This callback will delete us!

	delete _pReply;
	return OK;
}


//***							  ***
//*** DdmSvcVirtualDeviceUnloader ***
//***							  ***

// .Execute -- Load Virtual Device ------------------------------------------DdmSvcVirtualDeviceUnloader-
//
ERC DdmSvcVirtualDeviceUnloader::Execute(const RowId &_ridVdr, void *_pContext, ActionCallback _callback) {
	TRACE_PROC(DdmSvcVirtualDeviceUnloader::ProcessUnloadVirtualDevice);

	pContext = _pContext;
	callback = _callback;
	
	// Listen for Vdn Record to delete
	VirtualDeviceRecord::RqListen *pListen = new VirtualDeviceRecord::RqListen(ListenOnDeleteOneRow, ReplyWithRow | ReplyOnceOnly, _ridVdr);
	Send(pListen, REPLYCALLBACK(DdmSvcVirtualDeviceUnloader,ProcessDeleteListenReply));

	// Set Delete Flag
	EnumVDFlags flags = (EnumVDFlags) eVDFlagMask_Delete;	
	VirtualDeviceRecord::RqModifyBits *pModBits = new VirtualDeviceRecord::RqModifyBits(OpOrBits, _ridVdr, VDT_FLAGS_FIELD, &flags, sizeof(EnumVDFlags));
	Send(pModBits,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));
			
	return OK;
}

// .ProcessDeleteListenReply -- Process Replies -----------------------------DdmSvcVirtualDeviceUnloader-
//
ERC DdmSvcVirtualDeviceUnloader::ProcessDeleteListenReply(Message *_pReply) {
	TRACE_PROC(DdmSvcVirtualDeviceUnloader::ProcessDeleteListenReply);

	if ((status = _pReply->Status()) != OK) {
		Tracef("\n*** Replied with erc=%u (DdmSvcVirtualDeviceUnloader::ProcessDeleteListenReply)\n",_pReply->Status());
	}
	VirtualDeviceRecord::RqListen *pListen = (VirtualDeviceRecord::RqListen *) _pReply;
	
	if (!pListen->IsInitialReply())	// Ignore Listener ID returned
		(pParentDdmSvs->*callback)(this);	// This callback will delete us!
	
	return OK;
}

//***				  ***
//*** Support Methods ***
//***				  ***


// .UpdateIsrState -- Update PTS IOPStatusRecord -----------------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::UpdateIsrState(TySlot slot, IopState state) {
	TRACE_PROC(DdmVirtualMaster::UpdateIsrState);

	// Set state of IOP in VirtualStateTable
	Message *pMsg = new IOPStatusRecord::RqModifyField( CT_IOPST_SLOT,&slot,sizeof(TySlot),CT_IOPST_IOPCURRENTSTATE,&state,sizeof(state));
	ERC erc = Send(pMsg, REPLYCALLBACK(DdmVirtualProxy,DiscardOkReply));

	return OK;
}

// .NotifyIopStates -- Process Request --------------------------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::NotifyIopStates(States *pStates) {
	TRACE_PROC(DdmVirtualMaster::NotifyIopStates);
	
	// Notify Listeners of IopState
	RqOsVirtualMasterListenIopState*pNotify = new RqOsVirtualMasterListenIopState;
	pNotify->AddStatesReply(pStates);
	listenersIopState.Notify(pNotify);
	delete pNotify;

	return OK;
}

// .NotifyChangeRoutes -- Process Request -----------------------------------------DdmVirtualMaster-
//
// Notify listeners of a changed route
//
ERC DdmVirtualMaster::NotifyChangeRoutes(Entry *pEntry) {
	TRACE_PROC(DdmVirtualMaster::NotifyNChangeRoutes);
	
	if (pEntry != NULL) {
		// Notify Listeners of NewRoutes (Active)
		RqOsVirtualMasterListenChangeRoute*pNotifyRoute
				= new RqOsVirtualMasterListenChangeRoute(pEntry->vdn,pEntry->pRoute->did);

		listenersChangeRoute.Notify(pNotifyRoute);

		delete pNotifyRoute;
	}
	return OK;
}

// .NotifyNewRoutes -- Process Request --------------------------------------------DdmVirtualMaster-
//
// Notify listeners of a new route
//
ERC DdmVirtualMaster::NotifyNewRoutes(Entry *pEntry) {
	TRACE_PROC(DdmVirtualMaster::NotifyNewRoutes);
	
	if (pEntry != NULL) {
		// Notify Listeners of NewRoutes (Active)
		RqOsVirtualMasterListenNewRoute*pNotifyRoute = new RqOsVirtualMasterListenNewRoute;

		pNotifyRoute->AddRouteReply(pEntry->pRoute,1);
		listenersNewRoute.Notify(pNotifyRoute);

		delete pNotifyRoute;
	}
	return OK;
}

// .NotifyNewDevices -- Process Request --------------------------------------------DdmVirtualMaster-
//
// Notify listeners of a new device
//
ERC DdmVirtualMaster::NotifyNewDevices(Entry *pEntry) {
	TRACE_PROC(DdmVirtualMaster::NotifyNewDevices);
	
	// Notify listeners of NewDevices
	
	if (pEntry != NULL) {
		RqOsVirtualMasterListenNewDevice*pNotifyDevice = new RqOsVirtualMasterListenNewDevice;
	
		pNotifyDevice->AddDeviceReply(pEntry->pDevice,1);
		listenersNewDevice.Notify(pNotifyDevice);

		delete pNotifyDevice;
	}
	return OK;
}

// .NotifyDeleteDevices -- Process Request ----------------------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::NotifyDeleteDevices(VDN vdn) {
	TRACE_PROC(DdmVirtualMaster::NotifyDeleteDevices);
	
	Entry *pEntry = vdc.Find(vdn);	// Can only be one


	// Notify Listeners of DeleteDevices
	RqOsVirtualMasterListenDeleteDevice*pNotify 
		= new RqOsVirtualMasterListenDeleteDevice(vdn, pEntry->didPrimary, pEntry->didSecondary, pEntry->nServes, pEntry->pRoute->aServes);

	listenersDeleteDevice.Notify(pNotify);
	delete pNotify;

	return OK;
}

// .UpdateDevice -- Update Defined Record in PTS from new Route ----------------DdmVirtualMaster-
//
// Inserts VirtualRouting into PTS VirtualRouteTable
// Updates VDT record DID
// Updates VDT record flags
//
ERC DdmVirtualMaster::UpdateDevice(const Route &route) {
	TRACE_PROC(DdmVirtualMaster::UpdateDevice);

	Entry *pEntry = vdc.Find(route.vdn);	// Can only be one
	
	if (pEntry == NULL) {
		Tracef("@ ERROR: Vdn=%u not in local cache (VirtualMaster::UpdateDevice)\n",route.vdn);
		return OK;
	}
	RowId rid(idVdt,route.vdn);
	
	if (route.nServes > 0) {
		// Insert VirtualRouting into PTS VirtualRouteTable
		VirtualRouteRecord vrr(route.vdn,route.nServes,route.aServes);
		VirtualRouteRecord::RqInsertRow *pInsert = new VirtualRouteRecord::RqInsertRow(&vrr,1);
		Send(pInsert, REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));
	}
	VirtualDeviceRecord::RqModifyField *pModify;
	EnumVDFlags flags;
	
	// Update table DID
	if (pEntry->pDevice->slotPrimary == DeviceId::ISlot(route.did) ) {
		pModify = new VirtualDeviceRecord::RqModifyField(rid, VDT_DID_PRIMARY_FIELD,&route.did,sizeof(DID));
		flags = eVDFlagMask_HasPrimary;
	}
	else if (pEntry->pDevice->slotSecondary == DeviceId::ISlot(route.did) ) {
		pModify = new VirtualDeviceRecord::RqModifyField(rid, VDT_DID_SECONDARY_FIELD,&route.did,sizeof(DID));
		flags = eVDFlagMask_HasSecondary;
	}
	else {
		Tracef("@ ERROR: slotP=%u or slotS=%u doesn't match did=%u (DdmVirtualMaster::UpdateDevice)\n",pEntry->pDevice->slotPrimary,pEntry->pDevice->slotSecondary,route.did);
	}		
	Send(pModify,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));

	// Update table flags 
	VirtualDeviceRecord::RqModifyBits *pModBits = new VirtualDeviceRecord::RqModifyBits(OpOrBits, rid, VDT_FLAGS_FIELD, &flags, sizeof(EnumVDFlags));
	Send(pModBits,REPLYCALLBACK(DdmVirtualMaster,DiscardOkReply));

	return OK;
}

// .DeleteDevice -- Delete Device Record in PTS -----------------------DdmVirtualMaster-
//
// Inserts VirtualRouting into PTS VirtualRouteTable
// Updates VDT record DID
// Updates VDT record flags
//
VDN DdmVirtualMaster::DeleteDevice(VirtualDeviceRecord *pVdr) {
	TRACE_PROC(DdmVirtualMaster::DeleteDevice);

	// Change VDR state to Deleted
	// then really delete the record
	
	// Delete from local cache
	VDN vdn = vdc.Delete(pVdr);

	return vdn;
}

// .DumpStates -- Display current cached states -----------------------------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::DumpStates(States &iop) {
	TRACE_PROC(DdmVirtualMaster::DumpStates);
	
	U32 ii;
	
	// Display current slot states.
	Tracef("SLOT:");	// Display slot state
	for (ii=0; ii < RqOsVirtualMasterListenIopState::States::MAXSLOTS; ii++) {
		if (iop.stateIop[ii] != IOPS_EMPTY && iop.stateIop[ii] != IOPS_UNKNOWN)
			Tracef(" %2u",ii);
	}
	Tracef(" (IOP States)\n     ");	// Display slot state
	for (ii=0; ii < RqOsVirtualMasterListenIopState::States::MAXSLOTS; ii++) {
		if (iop.stateIop[ii] != IOPS_EMPTY && iop.stateIop[ii] != IOPS_UNKNOWN)
			Tracef(" %2u",iop.stateIop[ii]);
	}
	Tracef("  (IOP)\n");

	return OK;
}

// .DumpVdtRecord -- Display context of VirtualDeviceTable record -----------------DdmVirtualMaster-
//
ERC DdmVirtualMaster::DumpVdtRecord(VirtualDeviceRecord *pRec) {
	TRACE_PROC(DdmVirtualMaster::DumpVdtRecord);

	TRACEF(TRACE_L3,("@ class=\"%s\"; rid = %u^%u (DdmVirtualMaster::DumpVdtRecord)\n",pRec->szClassName,pRec->rid.Table,pRec->rid.LoPart));
#if 0
	rowID			rid;				// rowID of this record.
	U32 			version;			// Version of VirtualDeviceRecord.
	U32				size;				// Size of VirtualDeviceRecord in bytes.
	rowID			ridVDOwnerUse;		// For internal use by VD Owner/Creator.
	U32				fIOPHasVDR;			// Flag array indicating IOPs know of VD.
	U32				fIOPHasDID;			// Flag array indicating IOPs Know VDs DID.
	EnumVDFlagMasks	eVDFlags;			// Attributes of the Virtual Device
	EnumVDStates	eVDStateActual;		// Current state of the Virtual Device
	EnumVDStates	eVDStateDesired;	// Desired state of the Virtual Device
	String32		stClassName;		// Class type name of the VD.
	U32				sStackSize;			// Stack size 10K default.
	rowID			ridDdmCfgRec;		// Configuration data for the VD.
	TySlot			slotPrimary;		// Slot of primary device
	TySlot			slotSecondary;		// Slot of secondary device
	DID				didPrimary;			// DID of Primary DDM.
	DID				didSecondary;		// DID of secondary DDM.
#endif
	return OK;
}

//***					 ***
//*** VirtualDeviceCache ***
//***					 ***


// .Build -- Build list of cached virtual devices --------------------------------DdmSvcDeviceCache-
//
// Scan VirtualDeviceRecords and add all active devices to the Route list
//
// Returns array of pEntry pointers to just completed routes
//
ERC DdmSvcDeviceCache::Build(VirtualDeviceRecord *pVdr,U32 nVdr) {
	TRACE_PROC(DdmSvcDeviceCache::Build(VDR));
	
	for (U32 ii=0; ii < nVdr; ii++)
		Add(pVdr + ii);

	return OK;
}

// .Build -- Build list of cached virtual routes ---------------------------------DdmSvcDeviceCache-
//
// Scan VirtualDeviceRecords and add all active devices to the Route list
//
ERC DdmSvcDeviceCache::Build(VirtualRouteRecord *pVrr,U32 nVrr) {
	TRACE_PROC(DdmSvcDeviceCache::Build(VRR));
		
	for (U32 ii=0; ii < nVrr; ii++)
		Add(pVrr + ii);

	return OK;
}

// .Add -- Build list of cached virtual devices ----------------------------------DdmSvcDeviceCache-
//
// Add VirtualDeviceRecord.
//
// To add a Route to a current record simply re-add it.
//
DdmSvcDeviceCache::Entry *DdmSvcDeviceCache::Add(VirtualDeviceRecord *pVdr) {
	TRACE_PROC(DdmSvcDeviceCache::Add(VDR));
	
	Entry *pEntry;
	VDN vdn = pVdr->rid.LoPart;
	
	// Discard this device if no slots
	if (pVdr->slotPrimary == SLOTNULL && pVdr->slotSecondary == SLOTNULL)
		return NULL;

	if ((pEntry = Find(vdn)) == NULL) {	// Not yet entered
		pEntry = new Entry(vdn,&pFirst,&pLast,&nEntry);
		pEntry->pDevice = new Device(vdn,pVdr->szClassName,pVdr->slotPrimary,pVdr->slotSecondary,pVdr->ridDdmCfgRec,pVdr->eVDAttrs & eVDAttrMask_AutoStart);
		pEntry->Insert();
		++nDevices;
	}
	if (pVdr->eVDFlags & eVdFlagMask_Instanciated) {
		if (pEntry->pRoute == NULL) {
			pEntry->nServes = pVdr->nServes;	// Serves Needed
			pEntry->pRoute = new Route();
			++nRoutes;
		}
		if (pEntry->pRoute->did == DIDNULL && pEntry->nServes == pEntry->pRoute->nServes) 
			pEntry->fRoute = pEntry->fNotify = TRUE;	// Serves complete

		pEntry->pRoute->SetRoute(vdn,pVdr->didActive);	// Serves maybe needed
	
		return pEntry;
	}
	pEntry->eVDFlags = pVdr->eVDFlags;
	
	return pEntry;
}

// .Add -- Build list of cached virtual routes -----------------------------------DdmSvcDeviceCache-
//
// Add VirtualRouteRecord to Entry
//
DdmSvcDeviceCache::Entry *DdmSvcDeviceCache::Add(VirtualRouteRecord *pVrr) {
	TRACE_PROC(DdmSvcDeviceCache::Add(VRR));
	
	Entry *pEntry;
	VDN vdn = pVrr->vdn;

	if ((pEntry = Find(vdn)) == NULL) {	// Not yet entered
		pEntry = new Entry(vdn,&pFirst,&pLast,&nEntry);
		pEntry->Insert();
	}
	if (pEntry->pRoute == NULL ) {
		pEntry->nServes = pVrr->nServes;	// Serves Needed
		pEntry->pRoute = new Route();
		++nRoutes;
	}
	pEntry->pRoute->SetServes(pVrr->nServes,pVrr->aReqCodes);
	
	if (pEntry->pRoute->did != DIDNULL) 
		pEntry->fRoute = pEntry->fNotify = TRUE;	// Route Complete

	return pEntry;
}

// .Delete -- Delete Entry -------------------------------------------------------DdmSvcDeviceCache-
//
// Delete entire entry
//
VDN DdmSvcDeviceCache::Delete(VirtualDeviceRecord *pVdr) {
	TRACE_PROC(DdmSvcDeviceCache::Delete);
	
	Entry *pEntry;
	VDN vdn = pVdr->rid.LoPart;

	if ((pEntry = Find(vdn)) != NULL) {	// Not yet entered
		delete pEntry;
		return vdn;
	}
	Tracef("ERROR: No device to delete!! (DdmSvcDeviceCache::Delete)\n");
	for(;; ) ;
	
	return VDNNULL;
}

// .SetRouteCompleteFlags -- Set saved "SawDid" flags ----------------------------DdmSvcDeviceCache-
//
BOOL DdmSvcDeviceCache::SetRouteCompleteFlags(VDN vdn,U32 fSawDid, U32 fIopMask) {
	TRACE_PROC(DdmSvcDeviceCache::SetRouteCompleteFlags);
	
	Entry *pEntry;

	if ((pEntry = Find(vdn)) == NULL)
		return FALSE;
	
	pEntry->fSawDid = fSawDid;
	nRoutesComplete = RoutesCompleted(fIopMask);
	
	return TRUE;
}

// .RoutesCompleted -- Count Routes Completed ------------------------------------DdmSvcDeviceCache-
//
U32 DdmSvcDeviceCache::RoutesCompleted(U32 fIopMask) {
	TRACE_PROC(DdmSvcDeviceCache::RoutesCompleted);

	U32 nComplete = 0;
	for (Entry *pEntry = First(); pEntry != NULL; pEntry = pEntry->Next()) {
		if (pEntry->fSawDid == fIopMask)
			++nComplete;
	}
	return nComplete;
}

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmVirtualMaster.cpp $
// 
// 16    2/15/00 6:07p Tnelson
// Fixes for VirtualDevice delete
// 
// 15    2/08/00 8:55p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 16    2/08/00 6:07p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 14    1/28/00 1:44p Jlane
// Fix LoadVirtualDevice Duplicate VDN bug.
// 
// 13    1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 12    12/09/99 2:07a Iowa
// 
// 11    11/17/99 6:55p Jlane
// Fix Toms listen bug preventing >1 VD instantiation!
// 
// 10    11/04/99 6:30p Jlane
// comment out stateOS refs.
// 
// 9     11/04/99 1:14p Jlane
// Roll in Tom's Changes.
// 
// 8     10/14/99 6:09a Iowa
// Fix VSS merge muck-up.
// 
// 2     9/17/99 11:12p Tnelson
// 
// 1     9/16/99 3:17p Tnelson
// Support for PTS

