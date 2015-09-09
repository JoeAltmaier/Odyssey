/* DdmFailoverProxy.h -- CHAOS Debugging Failover Master Proxy
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
//  6/27/99 Tom Nelson: Create file


#ifndef __DdmFailoverProxy_h
#define __DdmFailoverProxy_h

#include "Ddm.h"
#include "RoutingTables.h"
#include "VirtualTable.h"

#include "RqOsTimer.h"
#include "RqOsVirtualMaster.h"

//
// FailoverProxy Ddm Class
//
class DdmFailoverProxy : public Ddm {
	typedef RqOsVirtualMasterListenIopState::States States;
	
	typedef struct _struct {
		enum { MAXSLOTS = States::MAXSLOTS };
	
		BOOL fPending[MAXSLOTS];
	
		_struct() { for (U32 ii=0; ii < MAXSLOTS; ii++) fPending[ii]=FALSE; }
	} PingFlags;
	
//	ProxyVirtualDeviceTable vdnMap;
	States stateIop;
	RqOsTimerStart *pMsgHeartbeat;
	PingFlags pingIop;
	
public:
	static Ddm *Ctor(DID did) 	{ return new DdmFailoverProxy(did); }

	DdmFailoverProxy(DID did);

	ERC Initialize(Message *pArgMsg);
	
	ERC Enable(Message *pArgMsg);

	ERC ProcessListenIopStateReply(Message *pArgMsg);
	ERC ProcessGetPtsDidReply(Message *pArgMsg);
	ERC ProcessListenVdnReply(Message *pArgMsg);
	ERC ProcessHeartbeatReply(Message *pArgMsg);
	ERC ProcessPingReply(Message *pArgMsg);
};

#endif // __DdmFailoverProxy_h

