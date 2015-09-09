//
// DdmTestSample.cpp
//
// Description: This is a very simple example of a Ddm which utilizes the services
//   of the Neptune test facilities
//
//   This will accept a single integer through the command line arguments and echo
//   back a timer tick that many times, one each second
//
//   Please note that this has a very simplistic approach which should NOT be used
//   in real test Ddm's.  Instead of keeping track of each test request that comes
//   in and using the CHAOS timers, it puts its current thread to sleep for the
//   second.  This essentially makes this Ddm NON-multithreaded.  Only a single test
//   can be run through this Ddm at a time.
//

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif
#define PLUS
#include <stdio.h>
#include <String.h>
#include "DdmTestSample.h"
#include "BuildSys.h"
#include "Critical.h"

#include "osheap.h"

CLASSNAME(DdmTestSample,SINGLE);	// Class Link Name used by Buildsys.cpp

DdmTestSample::DdmTestSample(DID did) : Ddm(did) {

}

Ddm *DdmTestSample::Ctor(DID did) {

	return new DdmTestSample(did);

}

STATUS DdmTestSample::Initialize(Message *pMsg) {

	Tracef("DdmTestSample::Initialize()\n");

	Tracef("Registering as DID %d\n", GetDid());
	TestMsgRegister *pTestMsg = new TestMsgRegister("TestSample\0", "0d\0", GetDid());
	Send(pTestMsg, NULL, REPLYCALLBACK(DdmTestSample, Registered));
	
	DispatchRequest(REQ_TEST_RUN, REQUESTCALLBACK(DdmTestSample, TestProc));

	Reply(pMsg, OK);
	return OK;
	
}

STATUS DdmTestSample::Enable(Message *pMsg) {
	
	Tracef("DdmTestSample::Enable()\n");
	
	Reply(pMsg, OK);
	return OK;
	
}

STATUS DdmTestSample::Registered(Message *pMsgReq) {

	Tracef("DdmTestSample::Registered()\n");
	
	TestMsgRegister* pMsg = (TestMsgRegister*) pMsgReq;
	
	delete pMsg;	

	return OK;

}

STATUS DdmTestSample::TestProc(Message *pMsgReq) {

	Tracef("DdmTestSample::TestProc()\n");
	
	TestMsgRun* pMsg = (TestMsgRun*) pMsgReq;
	
	char *pArgs;
	int iBufSize, numTicks;
	TestMsgRun* pMsgReply;
	pMsg->GetStruct(pArgs, iBufSize);
	args = (args_t *)pArgs;
	numTicks = args->numTicks;
	
	Tracef("Number of ticks requested: %d\n", numTicks);
	
	char *pBuf = new char[80];
	sprintf(pBuf, "This is a test routine.\n\rWatch me tick!!\n\r");
	pMsgReply = new TestMsgRun(pMsg);
	pMsgReply->SetWriteBuffer(pBuf, strlen(pBuf) + 1);
	Reply(pMsgReply, OK, false);

	for (int tick=0; tick < numTicks; tick++)
	{		
		sprintf(pBuf, "Tick %d\n\r", tick);
		pMsgReply = new TestMsgRun(pMsg);
		pMsgReply->SetWriteBuffer(pBuf, strlen(pBuf) + 1);
		Reply(pMsgReply, OK, false);
		NU_Sleep(100);
	}
	
	delete[] pBuf;		

	Reply(pMsg, OK);
	return OK;

}