/* DdmSiTest.cpp -- Test SysInfo DDM
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
//  3/27/99 Tom Nelson: Created

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <stdio.h>
#include <String.h>
#include "OsTypes.h"

#include "RqOsSysInfo.h"
#include "DdmSiTest.h"

#include "BuildSys.h"

// BuildSys linkage
	
CLASSNAME(DdmSiTest,MULTIPLE);


// DdmSiTest -- Constructor ------------------------------------------------DdmSiTest-
//
DdmSiTest::DdmSiTest(DID did): Ddm(did)
{
//	Tracef("DdmSiTest::DdmSiTest()\n");
//	SetConfigAddress(&config,sizeof(config));	// tell Ddm:: where my config area is
}
	
// .Enable -- Start-it-up ---------------------------------------------------DdmSiTest-
//
STATUS DdmSiTest::Enable(Message *pMsgReq)
{
	ERC erc;
	RqOsSiGetClassTable *pMsg;
	
	Tracef("DdmSiTest::Enable() did=%x\n",GetDid());

	pMsg = new RqOsSiGetClassTable();

	erc =  Send(pMsg, (ReplyCallback) &ProcessReply);
	Tracef("DdmSiTest::Enable; Send()=%u\n",erc);

	Reply(pMsgReq,OK);

	return OK;
}

// .ProcessReply -- Handle Replies  -----------------------------------------DdmSiTest-
//
ERC DdmSiTest::ProcessReply(RqOsSiGetClassTable *pMsg) {

	if (pMsg->DetailedStatusCode != OK) {
		Tracef("Replied with erc = %u\n",pMsg->DetailedStatusCode);
		return OK;
	}
	if (pMsg->payload.iClass == 0) {
		Tracef("SysInfo GetClass Table Details:\n");
		Tracef("     #/# cbStack sQueue flags    nLocal nVirtual nDid nDdm Version ClassName\n");
	}
	Tracef("    %2u/%-2u  %4u  %4u  %08x  %4u    %4u   %4u %4u  ..%2lu.. \"%s\"\n",
			pMsg->payload.iClass,pMsg->payload.cClass,
			pMsg->payload.cbStack,pMsg->payload.sQueue,pMsg->payload.flags,pMsg->payload.nServeLocal,pMsg->payload.nServeVirtual,
			pMsg->payload.nDidInstance,pMsg->payload.nDdmInstance,pMsg->payload.version,pMsg->payload.szClassName);
	
	return OK;
}

