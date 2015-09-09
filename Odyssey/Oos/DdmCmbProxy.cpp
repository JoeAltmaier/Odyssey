/* DdmCmbProxy.cpp -- Build IOPStatusTable then do nothing.
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
//  8/17/99 Tom Nelson: Create file
// ** Log at end of file **

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_NULL
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>

#include "BuildSys.h"
#include "IOPStatusTable.h"

// Private Includes
#include "DdmCmbProxy.h"
#include "VirtualTable.h"

// BuildSys Linkage

CLASSNAME(DdmCmbProxy,MULTIPLE);


// .DdmCmbProxy -- Constructor -------------------------------------------------DdmCmbProxy-
//
DdmCmbProxy::DdmCmbProxy(DID did) : Ddm(did) {
	TRACE_PROC(DdCmbNull::DdmCmbProxy);

}

// .Initialize -- Process Initialize ------------------------------------------DdmCmbProxy-
//
ERC DdmCmbProxy::Initialize(Message *_pMsg) {
	TRACE_PROC(DdmCmbProxy::Initialize);
	
	Reply(_pMsg,OK);	// Initialize Complete
			
	return OK;
}
	
// .Enable -- Process Enable --------------------------------------------------DdmCmbProxy-
//
ERC DdmCmbProxy::Enable(Message *_pMsg) {
	TRACE_PROC(DdmCmbProxy::Enable);

	ERC erc;

	// Create IOPStatusTable (IST)
	IOPStatusRecord::RqDefineTable *pDefine = new IOPStatusRecord::RqDefineTable(NotPersistant_PT);
	erc = Send(pDefine, _pMsg, REPLYCALLBACK(DdmCmbProxy,ProcessIstDefineTableReply));

	return OK;
}

// .ProcessVstDefineTableReply -- Process Reply -------------------------------DdmCmbProxy-
//
//	Insert default IOPStatusTable into PTS
//
ERC DdmCmbProxy::ProcessIstDefineTableReply(Message *_pReply) {
	TRACE_PROC(DdmCmbProxy::ProcessIstDefineTableReply);
	
	if (_pReply->DetailedStatusCode != OK) {
		Tracef("* Unable to define IOPStatusTable. Erc=%u (DdmCmbProxy)\n",_pReply->DetailedStatusCode);
		Reply((Message*)_pReply->GetContext(),OK);	// Enable
		return OK;
	}
	// Allocate array big enough for all possible slots
	IOPStatusRecord *paIsr = new IOPStatusRecord[NSLOT];
	
	// Build default array of all valid slot numbers
	U32 nSlot = 0;
	for (U32 ii=0; ii < NSLOT; ii++) {
		if ((paIsr[nSlot].Slot = DeviceId::iSlot_iSlot[ii]) != (TySlot) -1) {
			paIsr[nSlot].eIOPCurrentState = IOPS_EMPTY;
			++nSlot;
		}
	}
	// Change any slot referenced by a VirtualEntry to IOPS_AWAITING_BOOT
	for (U32 ii=0; ii < nSlot; ii++) {
		VirtualEntry *pEntry;
		if ((pEntry = VirtualTable::Find(paIsr[ii].Slot)) != NULL)
			paIsr[ii].eIOPCurrentState = IOPS_AWAITING_BOOT;
	}
	
	
	IOPStatusRecord::RqInsertRow *pInsert = new IOPStatusRecord::RqInsertRow(paIsr,nSlot);
	ERC erc = Send(pInsert, _pReply->GetContext(), REPLYCALLBACK(DdmPtsLoader,ProcessIstInsertReply));
		
	delete [] paIsr;
	delete _pReply;
	
	// No Enable yet!									
	return OK;
}

// .ProcessIstInsertReply -- Process Reply ------------------------------------DdmCmbProxy-
//
ERC DdmCmbProxy::ProcessIstInsertReply(Message *_pReply) {
	TRACE_PROC(DdmCmbProxy::ProcessIstInsertReply);

	// Enable
	Reply((Message*)_pReply->GetContext(),OK);
	
	return OK;
}
	

//*************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmCmbProxy.cpp $
// 
// 1     2/15/00 6:14p Tnelson
// Proxy services for testing
// 
// 1     9/16/99 3:17p Tnelson
// Support for PTS
