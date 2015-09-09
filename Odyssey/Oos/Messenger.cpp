/* Messenger.cpp -- Implements message primitives.
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
//  7/13/98 Joe Altmaier: Add pci, i2o initialization
//  7/17/98 Joe Altmaier: Remove Dma to separate module
//  7/21/98 Joe Altmaier: Get iSlotMe from IopMaster
//  2/07/99 Tom Nelson:   Begin abandoning I2O
//  4/21/99 Joe Altmaier: WIN32


// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890


#define _TRACEF
#include "Odyssey_Trace.h"

#include "Messenger.h"
#include "OsTypes.h"
#include "DdmManager.h"
#ifndef WIN32
#include "Transport.h"
#endif

// Public API

// .Send -- Send message to target DID -------------------------------Messenger-
//
// NOTICE: If didTarget == IDNULL and it is this IOP the Send will fail!
//
STATUS Messenger::Send(Message *pMsg, DID didTarget) {
//Tracef("Messenger::Send(%lx)\n", didTarget);

	STATUS erc;
	
	if ((erc = pMsg->BindTarget(didTarget)) != OK)
		return erc;

//	int iCabinet=didTarget.ICabinet();
	TySlot iSlot= DeviceId::ISlot(didTarget);
	U32 idLocal = DeviceId::IdLocal(didTarget);

#ifndef WIN32
	if (iSlot != Address::iSlotMe) // Send to another IOP
		return TransportBase::aPTransport[iSlot]->SendRemote(pMsg, didTarget);
#endif

	return DidMan::PutMessage(idLocal, pMsg);
}
		
// .Reply -- Send Reply Message --------------------------------------Messenger-
//
STATUS Messenger::Reply(Message *pMsg, BOOL fLast) { // Return message to Initiator

	STATUS erc;
	
	if ((erc = pMsg->BindResponse(fLast,&pMsg)) != OK)
		return erc;
	
	TySlot iSlot = DeviceId::ISlot(pMsg->didInitiator);
	U32 idLocal = DeviceId::IdLocal(pMsg->didInitiator);

#ifndef WIN32
	if (iSlot != Address::iSlotMe) // Reply to remote Iop.
		return TransportBase::aPTransport[iSlot]->ReplyRemote(pMsg);
#endif

	return DidMan::PutMessage(idLocal, pMsg);
}
	
// .Signal -- Signal target DID --------------------------------------Messenger-
//
STATUS Messenger::Signal(SIGNALCODE nSignal,void *pPayload, DID didTarget) {

//	int iCabinet=didTarget.ICabinet();
//	TySlot iSlot=DeviceId::ISlot(didTarget);
	U32 idLocal=DeviceId::IdLocal(didTarget);
			
	return DidMan::PutSignal(idLocal, nSignal,pPayload);
}

