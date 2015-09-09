// DdmEchoServer.cpp
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
	NetMsgListen* pListenMsg = new NetMsgListen(208,25,238,170,23);

	Tracef("DdmEchoServer::Enable()\n");

	//start test
	Send(pListenMsg, NULL, (ReplyCallback) ListenReplyHandler);

	Reply(pMsgReq,OK);
	return OK;
}

//ReplyHandler for calls to network listen
STATUS DdmEchoServer::ListenReplyHandler(Message *pMsgReq) {

	NetMsgListen* pListenMsg = (NetMsgListen*) pMsgReq;

	printf("\nNew Connection (%d)\n",pListenMsg->m_iConnectionID);
	int iSessionID = 1;

	//we now call read for that connection
	NetMsgRead* pReadMsg = new NetMsgRead(pListenMsg->m_iConnectionID,iSessionID);
	Send(pReadMsg, NULL, (ReplyCallback) ReadReplyHandler);
	
	delete pListenMsg;
	return OK;
}

STATUS DdmEchoServer::ReadReplyHandler(Message * pMsgReq) {

	Tracef("DdmEchoServer::ReadReplyHandler()\n");

	NetMsgRead* pReadMsg = (NetMsgRead*) pMsgReq;
	
	if(pReadMsg->DetailedStatusCode == OK) {
		char* pBuf;
		int iBufSize;
		pReadMsg->GetBuffer(pBuf,iBufSize);			
	}
	else {
		Tracef("Connection %d has been terminated\n",pReadMsg->m_iConnectionID);
	}
	
	delete pReadMsg;
	return OK;
}

STATUS DdmEchoServer::WriteReplyHandler(Message * pMsgReq) {

	Tracef("DdmEchoServer::WriteReplyHandler()\n");

	if(pMsgReq->DetailedStatusCode == OK) {

	}
	else {
		Tracef("Connection %d has been terminated\n",((NetMsgWrite*)pMsgReq)->m_iConnectionID);
	}
	
	delete (NetMsgWrite*)pMsgReq;

	return OK;
}