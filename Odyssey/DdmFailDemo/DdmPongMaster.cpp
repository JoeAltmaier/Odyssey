/* DdmPongMaster.cpp -- Test Failover DDM
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
 *		IOP Fail-Over Demo Master to run on HBC
 *
**/

// Revision History:
//  7/02/99 Tom Nelson: Created

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <stdio.h>
#include <String.h>

#include "RqOsVirtualMaster.h"
#include "RqOsPong.h"
#include "DdmPongMaster.h"
#include "RqOsTimer.h"

#include "BuildSys.h"

// BuildSys linkage
	
CLASSNAME(DdmPongMaster,MULTIPLE);


// DdmPongMaster -- Constructor ------------------------------------------------DdmPongMaster-
//
DdmPongMaster::DdmPongMaster(DID did): Ddm(did)
{
//	Tracef("DdmPongMaster::DdmPongMaster()\n");
//	SetConfigAddress(&config,sizeof(config));	// tell Ddm:: where my config area is
}
	
// .Enable -- Start-it-up ---------------------------------------------------DdmPongMaster-
//
STATUS DdmPongMaster::Enable(Message *pMsgEnable)
{
	Message *pMsg;
	
	Tracef("ENTER DdmPongMaster::Enable; did=%x\n",GetDid());

//	Tracef(">>    Config Data: sConfig=%u\n",sizeof(config));
//	Tracef(">>    Target VDN= %u\n",config.vdnSlave);
	
	// Load Slave Ddm(s)
	pMsg = new RqOsVirtualMasterLoadVirtualDevice("PONGSLAVE", IOP_RAC0, IOP_RAC1);
	Send(pMsg, pMsgEnable, REPLYCALLBACK(DdmPongMaster,ProcessLoadVirtualDeviceReply));
	
	return OK;
}

// .ProcessLoadVirtualDeviceReply ------------------------------------------DdmPongMaster-
//
ERC DdmPongMaster::ProcessLoadVirtualDeviceReply(Message *_pReply) {
//	TRACE_PROC(DdmPongMaster::ProcessLoadVirtualDeviceReply);

	Message *pMsg;
	RqOsVirtualMasterLoadVirtualDevice *pReply = (RqOsVirtualMasterLoadVirtualDevice *) _pReply;

	vdnSlave = pReply->payload.vdnRet;
	Tracef("VdnSlave = %u\n",vdnSlave);
	
	// Start Send Clock clock
	pMsg = new RqOsTimerStart(15000,15000);		// .1 second interval
	Send(pMsg, REPLYCALLBACK(DdmFailoverProxy,ProcessPongClockReply));

	cTicks = 1;
	pMsg = new RqOsPong(1);
	Send(vdnSlave,pMsg, (ReplyCallback) &ProcessPongReply);
	Tracef(">>    Sent Pong. (DdmPongMaster::Enable)\n");

	// Enable
	pMsg = (Message*) pReply->GetContext();
	Reply(pMsg,OK);

	Tracef("DdmPongMaster Enabled\n");
	
	delete _pReply;
	return OK;
}

// .ProcessReply -- Handle Replies  -----------------------------------------DdmPongMaster-
//
ERC DdmPongMaster::ProcessPongReply(Message *pArgMsg) {

	RqOsPong *pMsg = (RqOsPong*) pArgMsg;
	
	if (pMsg->DetailedStatusCode != OK) {
		Tracef("\n***  Replied with erc=%u (DdmPongMaster::ProcessPongReply)\n",pMsg->DetailedStatusCode);
		return OK;
	}
	count = pMsg->payload.count;
	cTicks = 0;
	Tracef("%4uM...",pMsg->payload.count);
	delete pMsg;	// Delete Pong Reply
		
	return OK;
}

// .ProcessPongClockReply -- Handle Clock Replies  ----------------------------DdmPongMaster-
//
ERC DdmPongMaster::ProcessPongClockReply(Message *pArgMsg) {

	ERC erc;
	RqOsPong *pMsg = (RqOsPong*) pArgMsg;
	
	delete pArgMsg;	// Delete Clock Reply

	if (cTicks++ == 0) {	// Only send if we got back the last one
		pMsg = new RqOsPong(count);
		erc =  Send(vdnSlave, pMsg, (ReplyCallback) &ProcessPongReply);
		if (erc != OK)
			Tracef(">>    Send Pong. erc=%u (DdmPongMaster::ProcessPongClockReply)\n",erc);
	}
	else if (cTicks < 10)
		Tracef("@ ");
	else
		Tracef("* ");
		
	return OK;
}

