//
// DdmEchoServer.cpp
//  Test CHAOS network routines
//

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif
#define PLUS
#include <stdio.h>
#include <String.h>
#include "OsTypes.h"
#include "DdmEchoServer.h"
#include "BuildSys.h"
#include "Critical.h"

#include "osheap.h"


CLASSNAME(DdmEchoServer,SINGLE);	// Class Link Name used by Buildsys.cpp

SERVELOCAL(DdmEchoServer, REQ_SSAPI_RESPONSE);

DdmEchoServer::DdmEchoServer(DID did): Ddm(did) {

}

Ddm *DdmEchoServer::Ctor(DID did) {
	return new DdmEchoServer(did);
}

STATUS DdmEchoServer::Initialize(Message *pMsg) {// virutal 

	Tracef("DdmEchoServer::Initialize()\n");

	Reply(pMsg,OK);
	return OK;
	
}

//Enable -- Start-it-up
STATUS DdmEchoServer::Enable(Message *pMsgReq) {
	
	Tracef("DdmEchoServer::Enable()\n");
	
	echomode = 0;
	iSessionID = 0;
	
	NetMsgListen* pListenMsg = new NetMsgListen(10, 1, 1, 201, 23);

	Send(pListenMsg, NULL, (ReplyCallback) ListenReplyHandler);
	Reply(pMsgReq,OK);
	return OK;
}

//ReplyHandler for calls to network listen
STATUS DdmEchoServer::ListenReplyHandler(Message *pMsgReq) {

	NetMsgListen* pListenMsg = (NetMsgListen*) pMsgReq;

	printf("\nNew Connection (%d)\n",pListenMsg->m_iConnectionID);
	
	//we now call read for that connection
	NetMsgRead* pReadMsg = new NetMsgRead(pListenMsg->m_iConnectionID,pListenMsg->m_iConnectionID);
	Send(pReadMsg, NULL, (ReplyCallback) ReadReplyHandler);
	
	delete pListenMsg;
	return OK;
}

STATUS DdmEchoServer::ReadReplyHandler(Message * pMsgReq) {

//	Tracef("DdmEchoServer::ReadReplyHandler()\n");

	NetMsgRead* pReadMsg = (NetMsgRead*) pMsgReq;
	
	// a happy read buffer was received
	if(pReadMsg->DetailedStatusCode == OK) {

		char* pBuf;
		int iBufSize;
		int iSize;
		pReadMsg->GetBuffer(pBuf,iBufSize);

		if (echomode)
		{

				// echo input bytes		
				NetMsgWrite* pWriteMsg = new NetMsgWrite(pReadMsg->m_iConnectionID,pReadMsg->m_iConnectionID);
				pWriteMsg->SetBuffer(pBuf, iBufSize);
				Send(pWriteMsg, NULL, (ReplyCallback) WriteReplyHandler);
				
				// if 'op' breakout sequence signaled, return to command mode
				if ((pBuf[0] == 'o') && (pBuf[1] == 'p'))
				{
					Tracef("Leaving echo mode..\n");
					echomode = 0;
				}
		
		}
		else
		{		
			// test case
			if (pBuf[0] == 't')
			{
				NetMsgWrite* pWriteMsg = new NetMsgWrite(1,1);
				char* pBufsend;
				pBufsend = new(tZERO) char[80];
				strcpy(pBufsend,"testing 123.  this is a test.  do not be alarmed.\r\n");
				pWriteMsg->SetBuffer(pBufsend, strlen(pBufsend)+1);
				delete[] pBufsend;
				Send(pWriteMsg, NULL, (ReplyCallback) WriteReplyHandler);
			}			

			// test case
			if (pBuf[0] == 'r')
			{
				NetMsgWrite* pWriteMsg = new NetMsgWrite(1,1);
				char* pBufsend;
				pBufsend = new(tZERO) char[200];
				strcpy(pBufsend,"This particular item is an even better test of the delivery mechanisms afforded by\r\nthe DdmNet, stack, and ethernet driver.\r\n");
				pWriteMsg->SetBuffer(pBufsend, strlen(pBufsend)+1);
				delete[] pBufsend;
				Send(pWriteMsg, NULL, (ReplyCallback) WriteReplyHandler);
			}
			
			// report memory delta
			if (pBuf[0] == 'm')
			{
				OsHeap::heapSmall.ReportDeltaMemoryUsage("mem report");
			}
			
			// change to echo mode
			if (pBuf[0] == '`')
			{
				Tracef("Entering echo mode..\n");
				echomode = 1;
			}
			
			// breakpoint dummy
			if (pBuf[0] == '\\')
			{
				Tracef("Breakpoint..\n");
			}
			
		}		
			
	}
	else {
		Tracef("Connection %d has been terminated\n",pReadMsg->m_iConnectionID);
	}
	delete pReadMsg;
	
	return OK;
}

// handles replies to write requests
//  just deletes the message since nothing else is required in this case
//  note that the messages is cast before deleting to invoke the non-virtual
//  destructor, deleting the buffer
//  THIS IS NECESSARY TO PREVENT MEMORY LEAKS!
STATUS DdmEchoServer::WriteReplyHandler(Message * pMsgReq) {

	NetMsgWrite* pMsg = (NetMsgWrite*)pMsgReq;

	delete pMsg;
	return OK;
	
}

