/* DdmSysInfo.cpp -- System Info Ddm
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
 * Description:
 *		Ddm that processes Get System Info requests
 *
**/

// Revision History: 
// 	 3/27/99 Tom Nelson: Created
//     5/07/99 Eric Wedel: Changed for classname in functor macros (GH).
// ** Log at end of file **

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890


#define _TRACEF
#define	TRACE_INDEX		TRACE_SYSINFO
#include "Odyssey_Trace.h"

// Public Includes
#include "DdmSysInfo.h"
#include "BuildSys.h"

// Private Includes
#include "ClassTable.h"
#include "ServeTable.h"
#include "DidMan.h"

// BuildSys Linkage

CLASSNAME(DdmSysInfo,SINGLE);

SERVELOCAL(DdmSysInfo, RqOsSysInfoGetClassTable::RequestCode);
SERVELOCAL(DdmSysInfo, RqOsSysInfoGetDidActivity::RequestCode);


// DdmScc -- Constructor --------------------------------------------DdmSysInfo-
//
DdmSysInfo::DdmSysInfo(DID did): Ddm(did) {
	TRACE_PROC(DdmSysInfo::DdmSysInfo);
	
	DispatchRequest(RqOsSysInfoGetClassTable::RequestCode,	REQUESTCALLBACK(DdmSysInfo, ProcessGetClassTable));
	DispatchRequest(RqOsSysInfoGetDidActivity::RequestCode,	REQUESTCALLBACK(DdmSysInfo, ProcessGetDidActivity));
}
	
// ProcessGetClass -- Process GetClass Request ----------------------DdmSysInfo-
//
STATUS DdmSysInfo::ProcessGetClassTable(Message* pArgMsg) {
	TRACE_PROC(DdmSysInfo::ProcessGetClassTable);

	RqOsSysInfoGetClassTable *pMsg = (RqOsSysInfoGetClassTable*)pArgMsg;

	ClassEntry *pEntry;
	ServeEntry *pServe;
	U16 iClass;
	
	// ClassTable cannot be empty or we wouldn't be here!
	for (iClass=0,pEntry = ClassTable::GetFirst();  pEntry != NULL; pEntry = pEntry->pNextEntry,iClass++) {	
		strcpy(pMsg->payload.szClassName,pEntry->pszName);
		pMsg->payload.cClass = (unsigned short)ClassTable::GetCount();
		pMsg->payload.iClass = iClass;
		pMsg->payload.cbStack = pEntry->cbStack;
		pMsg->payload.sQueue = pEntry->sQueue;
		pMsg->payload.version = 0;
		if ((pServe = ServeTable::Find(pEntry->ctor)) == NULL)
			pMsg->payload.nServeLocal = pMsg->payload.nServeVirtual = 0;
		else {
			pMsg->payload.nServeLocal = pServe->nServeLocal;
			pMsg->payload.nServeVirtual = pServe->nServeVirtual;
		}
		pMsg->payload.flags = pEntry->flags;
		pMsg->payload.nDidInstance = pEntry->nDidInstance; 
		pMsg->payload.nDdmInstance = pEntry->nDdmInstance;

		Reply(pMsg, OK, pEntry->pNextEntry == NULL ? TRUE : FALSE);
	}
	return OK;
}

// .ProcessGetDidActivity -- Process Request --------------------------------------------DdmStatus-
//
// Returns Ddm Activity for this slot
//
ERC DdmSysInfo::ProcessGetDidActivity(Message *_pRequest) {
	TRACE_PROC(DdmSysInfo::ProcessGetDidActivity);
	
	RqOsSysInfoGetDidActivity *pRequest = (RqOsSysInfoGetDidActivity*) _pRequest;
	
	// Count Dids
	U32 nDid = 0;
	for (U32 idLocal = 1; idLocal <= DidMan::idLocalLast; idLocal++)
		nDid += (DidMan::didTag_idLocal[idLocal] != NULL);
	
	DidActivity *aDa = new DidActivity[nDid];
	
	U32 iDid = 0;
	for (U32 idLocal = 1; idLocal <= DidMan::idLocalLast; idLocal++) {
		if (DidMan::didTag_idLocal[idLocal] != NULL)
			aDa[iDid++].Set(DidMan::didTag_idLocal[idLocal]);
	}
	pRequest->AddReply(aDa,iDid);
	delete [] aDa;

	Reply(pRequest,OK);

	return OK;
}


//**
//** Static Proceedural Interface for Applications
//**
//** Allows us to hide implementation details
//**

void RqOsSysInfoGetDidActivity::DidActivity::Set(const DidMan *pDm) {
	did = pDm->did;
	vdn = pDm->vdn;
	strcpy(szClassName,pDm->pClass->pszName);
	cSend = pDm->cSend;
	cReply = pDm->cReply;
	cSignal = pDm->cSignal;
	cAction = pDm->cAction;
#ifdef WIN32
	cDeferred = ((WaitQueue_T<DidLnk> &)(pDm->queueDeferred)).ItemCount();
#else
	cDeferred = pDm->queueDeferred.ItemCount();
#endif
	ddmState = pDm->state;
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmSysInfo.cpp $
// 
// 8     1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 7     1/07/00 5:37p Agusev
// Fixed WIN32 build. 
// Is this the last time or what?!
// 
// 6     12/16/99 3:40p Iowa
// System status support.
// 
