/* MessageBroker.h -- Message Broker Class Definition
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
 *		Allows legacy C code to Send messages.
 *
**/

// Revision History:
// 	3/15/99 Tom Nelson: Created
//

#ifndef __MessageBroker_H
#define __MessageBroker_H

#include "DdmOsServices.h"
#include "Semaphore.h"

class MessageBroker :  DdmServices {	// No access to base class!!!
// Message Broker Context
//
	class MBC {
	public:
		union {
			VDN vdn;
			DID did;
		};
		Message *pMsg;
		ERC erc;
		Semaphore semaphore;

		MBC(VDN _vdn,Message *_pMsg)  { vdn = _vdn; pMsg = _pMsg; erc=OK; }
		MBC(DID _did,Message *_pMsg)  { did = _did; pMsg = _pMsg; erc=OK; }
		MBC(Message *_pMsg) 		  { pMsg = _pMsg; erc=OK; }
	};
	
private:
	SIGNALCODE sigCodeMb;
	ERC SendWaitMbc(MBC *pMbc,ActionCallback cb);
	
public:
	MessageBroker(DdmServices *_pParentDdmSvc);
	~MessageBroker();
	
	ERC ProcessActionVdn(void *pPayload);
	ERC ProcessActionDid(void *pPayload);
	ERC ProcessActionRouted(void *pPayload);
	
	ERC ProcessSendReply(Message *pMsg);

	ERC SendWait(VDN vdnTarget, Message *pMsg);

	ERC SendWait(DID didTarget, Message *pMsg);

	ERC SendWait(Message *pMsg);
};

#endif // __MessageBroker_H

