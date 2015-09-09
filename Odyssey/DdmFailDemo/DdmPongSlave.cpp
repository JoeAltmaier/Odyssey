/* DdmPongSlave.cpp -- Test Failover DDM
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
#include "OsTypes.h"
#include "Os.h"

#include "RqOsPong.h"
#include "DdmPongSlave.h"
#include "RqOsVirtualMaster.h"
#include "BuildSys.h"

// BuildSys linkage
	
CLASSNAME(DdmPongSlave,MULTIPLE);


// DdmPongSlave -- Constructor ------------------------------------------------DdmPongSlave-
//
DdmPongSlave::DdmPongSlave(DID did): Ddm(did)
{
//	Tracef("DdmPongSlave::DdmPongSlave()\n");
	SetConfigAddress(&config,sizeof(config));	// tell Ddm:: where my config area is
	DispatchRequest(RqOsPong::RequestCode, REQUESTCALLBACK(DdmPongSlave, ProcessPong));
}
	
// .Enable -- Start-it-up ---------------------------------------------------DdmPongSlave-
//
STATUS DdmPongSlave::Enable(Message *pMsgEnable)
{
	ERC erc;
	
	Tracef("ENTER DdmPongSlave::Enable; vdn=%u; did=%x\n",GetVdn(),GetDid());

	erc = Reply(pMsgEnable,OK);
	Tracef("EXIT  DdmPongSlave::Enable;  Reply()=%u\n",erc);
	
	return OK;
}

// .ProcessPong -- Handle Pong Request -------------------------------------DdmPongSlave-
//
ERC DdmPongSlave::ProcessPong(Message *pArgMsg) {

	RqOsPong *pMsg = (RqOsPong*) pArgMsg;
	
	Tracef("%4uS...",pMsg->payload.count);

	++pMsg->payload.count;

//	if (pMsg->payload.count == 20) {
//		RqOsVirtualMasterFailSlot *pFail = new RqOsVirtualMasterFailSlot(Address::iSlotMe);
//		Send(pFail,REPLYCALLBACK(DdmPongSlave,DiscardOkReply));
//	}
	Reply(pMsg,OK);
	
	return OK;
}

