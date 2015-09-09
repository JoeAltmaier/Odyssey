/* MessageBroker.cpp -- MessageBroker Class
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
 *		Allows legacy C code to Send messages via it's associated Ddm.
 *
**/

// Revision History:
// 	3/15/99 Tom Nelson: Created
//  5/07/99 Eric Wedel: Changed for classname in functor macros (GH).
//  5/11/99 Eric Wedel: Also changed ACTIONCALLBACK() functor macros (GH).
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890


#include "MessageBroker.h"

// .MessageBroker -- Constructor -------------------------------------------MessageBroker-
//
MessageBroker::MessageBroker(DdmServices *_pParentDdmSrvs) : DdmServices(_pParentDdmSrvs) {

}
	
// .~MessageBroker -- Constructor ------------------------------------------MessageBroker-
//
MessageBroker::~MessageBroker() {

}


// .ProcessBrokerSignal -- Process Signal to Brokered Message --------------MessageBroker-
//
// Entry: DDM Thread.
//
ERC MessageBroker::ProcessActionVdn(void *pPayload) {
	
	MBC *pMbc = (MBC *) pPayload;

	pMbc->erc = Send(pMbc->vdn,pMbc->pMsg,(void *)pMbc,REPLYCALLBACK(MessageBroker, ProcessSendReply));
	
	if (pMbc->erc != OK)
		pMbc->semaphore.Signal();
	
	return OK;
}

// .ProcessBrokerDid -- Process Signal to Brokered Message -----------------MessageBroker-
//
// Entry: DDM Thread.
//
ERC MessageBroker::ProcessActionDid(void *pPayload) {
	
	MBC *pMbc = (MBC *) pPayload;

	pMbc->erc = Send(pMbc->did,pMbc->pMsg,(void *)pMbc,REPLYCALLBACK(MessageBroker, ProcessSendReply));
	
	if (pMbc->erc != OK)
		pMbc->semaphore.Signal();
	
	return OK;
}

// .ProcessBrokerRouted -- Process Signal to Brokered Message --------------MessageBroker-
//
// Entry: DDM Thread.
//
ERC MessageBroker::ProcessActionRouted(void *pPayload) {
	
	MBC *pMbc = (MBC *) pPayload;

	pMbc->erc = Send(pMbc->pMsg,(void *)pMbc,REPLYCALLBACK(MessageBroker, ProcessSendReply));
	
	if (pMbc->erc != OK)
		pMbc->semaphore.Signal();
	
	return OK;
}
// .ProcessSendReply -- Process Brokered Message Callback ------------------MessageBroker-
//
// Entry: DDM Thread.
//
ERC MessageBroker::ProcessSendReply(Message *pMsg) {
	
	MSGREPLYKEEP(pMsg);

	MBC *pMbc = (MBC *) pMsg->GetContext();

	pMbc->semaphore.Signal();
	
	return OK;
}

// .SendWait -- Signal Broker with Context containing message --------------MessageBroker-
//
// Uses the DdmServices Signal Method to put the message on to the Ddm's
// message queue.  Signal is the only DdmServices method guarenteed to be
// re-entrant.
//
// Entry: Any Thread.
//
ERC MessageBroker::SendWaitMbc(MBC *pMbc,ActionCallback ecb) {	// private

	ERC erc;
	
	if ((erc = Action(ecb,(void*) pMbc)) != OK)
		pMbc->erc = erc;
	else
		pMbc->semaphore.Wait(); // Suspend Thread
	
	return pMbc->erc;
}

// .SendWait -- Build Context for brokered message -------------------------MessageBroker-
//
// Uses the DdmServices Signal Method to put the message on to the Ddm's
// message queue.  Signal is the only DdmServices method guarenteed to be
// thread-safe.
//
// Entry: Any Thread.
//
ERC MessageBroker::SendWait(VDN vdnTarget,Message *pMsg) {

	MBC mbc(vdnTarget,pMsg);

	return SendWaitMbc(&mbc,ACTIONCALLBACK(MessageBroker, ProcessActionVdn));
}

// .SendWait -- Build Context for brokered message -------------------------MessageBroker-
//
// Entry: Any Thread.
//
ERC MessageBroker::SendWait(DID didTarget,Message *pMsg) {

	MBC mbc(didTarget,pMsg);

	return SendWaitMbc(&mbc,ACTIONCALLBACK(MessageBroker, ProcessActionDid));
}

// .SendWait -- Build Context for brokered message -------------------------MessageBroker-
//
// Entry: Any Thread.
//
ERC MessageBroker::SendWait(Message *pMsg) {

	MBC mbc(pMsg);

	return SendWaitMbc(&mbc,ACTIONCALLBACK(MessageBroker, ProcessActionRouted));
}


