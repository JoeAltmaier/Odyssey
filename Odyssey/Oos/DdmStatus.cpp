/* DdmStatus.cpp -- Display Device (DDM) Status
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
**/
 
// Revision History: 
//  12/07/99 Tom Nelson: Create file
// ** Log at end of file **

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_VIRTUAL_MGR
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>
#include "BuildSys.h"

// Private Includes
#include "DdmStatus.h"

// BuildSys Linkage

CLASSNAME(DdmStatus,SINGLE);

SERVELOCAL(DdmStatus,RqOsStatusDumpIst::RequestCode);
SERVELOCAL(DdmStatus,RqOsStatusDumpVdt::RequestCode);
SERVELOCAL(DdmStatus,RqOsStatusDumpDidActivity::RequestCode);
SERVELOCAL(DdmStatus,RqOsStatusDumpAllDidActivity::RequestCode);


Ddm * DdmStatus::pThis;

//***
//*** Static Interface Methods
//***

// .DumpIst -- Dump PTS IOPStatusTable ---------------------------------------------------DdmStatus-
//
ERC DdmStatus::DumpIst() {	// static
	TRACE_PROC(DdmStatus::DumpIst);
	
	if (pThis == NULL) 
		return ERR; 
	
	Message *pMsg = new RqOsStatusDumpIst(); 
	pThis->Send(pMsg,REPLYCALLBACK(DdmStatus,DiscardReply));
	
	return OK;
}

// .DumpVdt -- Dump PTS VirtualDeviceTable -----------------------------------------------DdmStatus-
//
ERC DdmStatus::DumpVdt() {	// static
	TRACE_PROC(DdmStatus::DumpVdt);
	
	if (pThis == NULL) 
		return ERR; 
	
	Message *pMsg = new RqOsStatusDumpVdt(); 
	pThis->Send(pMsg,REPLYCALLBACK(DdmStatus,DiscardReply));
	
	return OK;
}

// .DumpDidActivity -- Dump Ddm Activity -------------------------------------------------DdmStatus-
//
ERC DdmStatus::DumpDidActivity() {	// static
	TRACE_PROC(DdmStatus::DumpDidActivity);
	
	if (pThis == NULL) 
		return ERR; 
	
	Message *pMsg = new RqOsStatusDumpDidActivity(); 
	pThis->Send(pMsg,REPLYCALLBACK(DdmStatus,DiscardReply));

	return OK;
}

// .DumpAllDidActivity -- Dump All Did Activity -----------------------------------------DdmStatus-
//
// Dumps Did activity for all slots
//
ERC DdmStatus::DumpAllDidActivity() {	// static
	TRACE_PROC(DdmStatus::DumpAllDidActivity);
	
	if (pThis == NULL) 
		return ERR; 
	
	Message *pMsg = new RqOsStatusDumpAllDidActivity(); 
	pThis->Send(pMsg,REPLYCALLBACK(DdmStatus,DiscardReply));

	return OK;
}


//***
//*** Instance Methods
//***

// .DdmStatus -- Constructor -------------------------------------------------------------DdmStatus-
//
DdmStatus::DdmStatus(DID did) : Ddm(did) {
	TRACE_PROC(DdmStatus::DdmStatus);

	pThis = this;
	
	for (U32 ii=0; ii < DIDMAX; ii++)
		aPDidActivity[ii] = NULL;
		
	DispatchRequest(RqOsStatusDumpAllDidActivity::RequestCode, REQUESTCALLBACK(DdmStatus, ProcessDumpAllDidActivity));
	DispatchRequest(RqOsStatusDumpDidActivity::RequestCode, REQUESTCALLBACK(DdmStatus, ProcessDumpDidActivity));
	DispatchRequest(RqOsStatusDumpVdt::RequestCode, 		REQUESTCALLBACK(DdmStatus, ProcessDumpVdt));
	DispatchRequest(RqOsStatusDumpIst::RequestCode, 		REQUESTCALLBACK(DdmStatus, ProcessDumpIst));
}

// .Enable -- Process Enable -------------------------------------------------------------DdmStatus-
//
ERC DdmStatus::Enable(Message *_pRequest) {
	TRACE_PROC(DdmStatus::Enable);

	Message *pMsg;
	
	// Listen to PTS IOPStateTable
	pMsg = new IOPStatusRecord::RqListen;
	Send(pMsg, REPLYCALLBACK(DdmStatus, ProcessIOPStatusTableListenReply));

	Reply(_pRequest,OK);
	return OK;
}

// .ProcessDumpIst -- Process Dump PTS IOPStatusTable ------------------------------------DdmStatus-
//
ERC DdmStatus::ProcessDumpIst(Message */*_pRequest*/) {
	TRACE_PROC(DdmStatus::ProcessDumpIst);
	
	// Listen to entire VirtualDeviceTable.
	Message *pMsg = new IOPStatusRecord::RqEnumerateTable;
	Send(pMsg, REPLYCALLBACK(DdmStatus,ProcessIOPStatusTableEnumerateReply));
	
	return OK;
}

// .ProcessDumpVdt -- Process Dump PTS VirtualDeviceTable -------------------------------DdmStatus-
//
ERC DdmStatus::ProcessDumpVdt(Message */*_pRequest*/) {
	TRACE_PROC(DdmStatus::ProcessDumpVdt);
	
	// Listen to entire VirtualDeviceTable.
	Message *pMsg = new VirtualDeviceRecord::RqEnumerateTable;
	Send(pMsg, REPLYCALLBACK(DdmStatus,ProcessVirtualDeviceTableEnumerateReply));
	
	return OK;
}

// .ProcessDumpDdmActivity -- Process Dump Ddm Activity ---------------------------------DdmStatus-
//
ERC DdmStatus::ProcessDumpDidActivity(Message */*_pRequest*/) {
	TRACE_PROC(DdmStatus::ProcessDumpDidActivity);
	
	Message *pMsg = new RqOsSysInfoGetDidActivity();
	Send(pMsg, REPLYCALLBACK(DdmStatus,ProcessGetDidActivityReply));

	return OK;
}

// .ProcessDumpAllDdmActivity -- Process Request ----------------------------------------DdmStatus-
//
// Dump Ddm Activity on all slots
//
ERC DdmStatus::ProcessDumpAllDidActivity(Message */*_pRequest*/) {
	TRACE_PROC(DdmStatus::ProcessDumpAllDidActivity);
	
	for (U32 ii=0; ii < States::MAXSLOTS; ii++) {
		if (iop.stateIop[ii] == IOPS_LOADING || 	// IOP image running, loading "system entries"
   			iop.stateIop[ii] == IOPS_OPERATING) {   // normally operating (OS / app-level code)
			
			Message *pMsg = new RqOsSysInfoGetDidActivity();
			Send(ii,pMsg, REPLYCALLBACK(DdmStatus,ProcessGetDidActivityReply));
		}
	}
	return OK;
}

// .ProcessGetDidActivityReply -- Process Reply -----------------------------------------DdmStatus-
//
ERC DdmStatus::ProcessGetDidActivityReply(Message *_pReply) {
	TRACE_PROC(DdmStatus::ProcessGetDidActivityReply);
	
	if (_pReply->Status() != OK) {
		Tracef("\n*** Replied with erc=%u (DdmStatus::ProcessGetDidActivityReply)\n",_pReply->Status());
		delete _pReply;
		return OK;
	}
	RqOsSysInfoGetDidActivity *pReply = (RqOsSysInfoGetDidActivity*) _pReply;
	
	printf("\nSlot=%u (DidActivity)\n",pReply->payload.slot);
	printf("                                REQUESTS REPLIES\n");
	printf("DID      VDN      DDM               SENT    SENT SGNLs ACTNs DEFER STATE\n");

	U32 nDid = pReply->GetDidCount();
	
	for (U32 iDid = 0; iDid < nDid; iDid++) {
		DidActivity da;
		pReply->GetDidActivity(iDid,&da);
		
		printf("%08lx %8ld %-16s %5ld   %5ld %5ld %5ld %5ld  %s\n",
			da.did, da.vdn, da.szClassName,
			da.cSend, da.cReply, da.cSignal, da.cAction, 
			da.cDeferred,
			DidMan::GetStateName(da.ddmState)
		);
	}
	delete _pReply;
	return OK;
}

// .ProcessIOPStatusTableEnumerateReply -- Process Replies ------------------------------DdmStatus-
//
// Listen to PTS IOPStatusTable
//
ERC DdmStatus::ProcessIOPStatusTableEnumerateReply(Message *_pReply) {
	TRACE_PROC(DdmStatus::ProcessIOPStatusTableEnumerateReply);

	if (_pReply->Status() != OK) {
		Tracef("\n*** Replied with erc=%u (DdmStatus::ProcessIOPStatusTableEnumerateReply)\n",_pReply->Status());
		delete _pReply;
		return OK;
	}

	IOPStatusRecord::RqEnumerateTable *pReply = (IOPStatusRecord::RqEnumerateTable*) _pReply;
	
	U32 nIsr;
	IOPStatusRecord *pIsr = pReply->GetRowPtr(&nIsr);
	States iop;
	
	TRACEF(TRACE_L3,("InitialReply; cRows=%u\n",nIsr) );
	// Build VirtualMaster internal IOP state table
	// There should be an entry for every slot
	for (U32 ii=0; ii < nIsr; ii++) {
		iop.SetStateIop(pIsr[ii].Slot,pIsr[ii].eIOPCurrentState);
	}
	DumpStates(iop);

	delete _pReply;
	
	return OK;
}

#if 0
	// Persistant fields
	RowId			ridVDOwnerUse;		// For internal use by VD Owner/Creator. ** KEY FIELD **
	String32		szClassName;		// Class type name of the VD.
	TySlot			slotPrimary;		// Slot of primary device
	TySlot			slotSecondary;		// Slot of secondary device
	RowId			ridDdmCfgRec;		// Configuration data for the VD.
	EnumVDAttrs  	eVDAttrs;			// Attributes of the Virtual Device
	U32				nServes;			// Number of Virtual Serves

	// Non-Persistant fields
	EnumVDFlags  	eVDFlags;			// Attributes of the Virtual Device
	U32				fIOPHasDID;			// Flag array indicating IOPs Know of DID.
	DID				didPrimary;			// DID of Primary DDM.
	DID				didSecondary;		// DID of secondary DDM.
	DID 			didActive;			// DID of current active DDM

	// Persistant Non-Duplicate Key
	Key				key;				// Duplicate Key
#endif

// .ProcessVirtualDeviceTableEnumerateReply -- Process Replies ---------------------DdmVirtualMaster-
//
// Listen to VirtualDeviceTable
//
ERC DdmStatus::ProcessVirtualDeviceTableEnumerateReply(Message *_pReply) {
	TRACE_PROC(DdmStatus::ProcessVirtualDeviceTableEnumerateReply);
	
	if (_pReply->Status() != OK) {
		Tracef("\n*** Replied with erc=%u (DdmStatus::ProcessVirtualDeviceTableEnumerateReply)\n",_pReply->Status());
		return OK;
	}
//	ERC erc;
	VirtualDeviceRecord::RqEnumerateTable *pReply = (VirtualDeviceRecord::RqEnumerateTable *) _pReply;

	U32 nVdr;
	VirtualDeviceRecord *pVdt = pReply->GetRowCopy(&nVdr);
	
	Tracef("\nRid    eVDFlags fIOPHasDID slotP... slotS... didP.... didS.... className\n");
	for (U32 ii=0; ii < nVdr; ii++) {
		VirtualDeviceRecord *pVdr = pVdt + ii;
		Tracef("%2u^%-3u %8x %10x %8x %8x %8x %8x %s\n",
			pVdr->rid.Table,pVdr->rid.LoPart, pVdr->eVDFlags, pVdr->fIOPHasDID,
			pVdr->slotPrimary,pVdr->slotSecondary,
			pVdr->didPrimary, pVdr->didSecondary,
			pVdr->szClassName);
	}
	delete _pReply;
	return OK;
}

// .DumpStates -- Display current cached states -----------------------------------DdmVirtualMaster-
//
ERC DdmStatus::DumpStates(States &iop) {
	TRACE_PROC(DdmStatus::DumpStates);
	
	U32 ii;
	
	// Display current slot states.
	Tracef("\nSLOT:");	// Display slot state
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

// .ProcessIOPStatusTableListenReply -- Process Replies ---------------------------DdmStatus-
//
// Listen to PTS IOPStatusTable
//
ERC DdmStatus::ProcessIOPStatusTableListenReply(Message *_pReply) {
	TRACE_PROC(DdmStatus::ProcessIOPStatusTableListenReply);

	if (_pReply->Status() != OK) {
		Tracef("\n*** Replied with erc=%x (DdmStatus::ProcessIOPStatusTableListenReply)\n",_pReply->Status());
		return OK;
	}
	IOPStatusRecord::RqListen *pListen = (IOPStatusRecord::RqListen*) _pReply;
	IOPStatusRecord *pIsr;
	U32 nTd;

	if (pListen->IsInitialReply()) {
		pIsr = pListen->GetTablePtr(&nTd);
		TRACEF(TRACE_L3,("InitialReply; cRows=%u\n",nTd) );
		// Build VirtualMaster internal IOP state table
		// There should be an entry for every slot
		for (U32 ii=0; ii < nTd; ii++) {
			iop.SetStateIop(pIsr[ii].Slot,pIsr[ii].eIOPCurrentState);
		}
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
			break;
		}
	}
	TRACEF(TRACE_L3, ("*\n* IOP States (DdmStatus)\n*\n"));
	if (TraceLevel[TRACE_INDEX] >= TRACE_L3)
		DumpStates(iop);
	
	delete _pReply;
	
	return OK;
}

//**
//** Static Proceedural Interface for Applications
//**
//** Allows us to hide the DdmStatus implementation
//**

RqOsStatusDumpIst::Invoke() {	// static
	return DdmStatus::DumpIst();
}

RqOsStatusDumpVdt::Invoke() {	// static
	return DdmStatus::DumpVdt();
}

ERC RqOsStatusDumpDidActivity::Invoke() {	// static
	return DdmStatus::DumpDidActivity();
}

ERC RqOsStatusDumpAllDidActivity::Invoke() {	// static
	return DdmStatus::DumpAllDidActivity();
}

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmStatus.cpp $
// 
// 3     1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 2     12/23/99 4:01p Jlane
// Use TRACE_INDEX TRACE_VIRTUAL_MGR instead of TRACE_NULL and only dump
// IOP states on every IOPST Listen reply if TRACE level is 3 or more.
// 
// 1     12/16/99 3:41p Iowa
// System status support.
// Memory leak detection.
// 


