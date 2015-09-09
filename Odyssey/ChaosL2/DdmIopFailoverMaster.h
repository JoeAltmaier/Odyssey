/* DdmIopFailoverMaster.h -- CHAOS IOP Failover Master 
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
// 10/25/99 Bob Butler: Used DdmFailoverProxy.h as the basis for DdmIopFailoverMaster.h


#ifndef __DdmIopFailoverMaster_h
#define __DdmIopFailoverMaster_h

#include "Ddm.h"

#include "RqOsTimer.h"
#include "RqOsVirtualMaster.h"

//
// DdmIopFailoverMaster Ddm Class
//
class DdmIopFailoverMaster : public Ddm 
{
	typedef RqOsVirtualMasterListenIopState::States States;
	enum {MAXTIMEOUTS = 2};
	
	struct PingTimeouts {
		U16 cTimeouts[States::MAXSLOTS];
		BOOL fFailPending[States::MAXSLOTS];	
		BOOL fReplyPending[States::MAXSLOTS];		
		PingTimeouts() { 
			for (U32 ii=0; ii < States::MAXSLOTS; ii++) {
				cTimeouts[ii]=0; 
				fFailPending[ii]=FALSE;
				}
			}
	};
	
	PingTimeouts pingIop;
	
	States stateIop;
	RqOsTimerStart *pMsgHeartbeat;
	
public:
	static Ddm *Ctor(DID did) 	{ return new DdmIopFailoverMaster(did); }

	DdmIopFailoverMaster(DID did);

	STATUS Initialize(Message *pArgMsg);
	
	STATUS Enable(Message *pArgMsg);

	STATUS ProcessListenIopStateReply(Message *pArgMsg);
	STATUS ProcessHeartbeatReply(Message *pArgMsg);
	STATUS ProcessPingReply(Message *pArgMsg);
	STATUS ProcessIopOutOfServiceReply(Message *pArgMsg);
};

#endif // __DdmIopFailoverMaster_h

