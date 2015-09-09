//DdmSocketServer.cpp

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <stdio.h>
#include <String.h>
#include "OsTypes.h"
#include "DdmSocketServer.h"
#include "BuildSys.h"

#include "..\msl\osheap.h"

CLASSNAME(DdmSocketServer,SINGLE);	// Class Link Name used by Buildsys.cpp

SERVELOCAL(DdmSocketServer, REQ_SSAPI_RESPONSE);

#ifdef WIN32
char GetNextPart(FILE* fp) {
	char sTemp[4];
	int i=0;
	char ch = getc(fp);
	while(i<3 && ch != '.' && !feof(fp)) {
		sTemp[i++] = ch;
		ch = getc(fp);
	}
	sTemp[i] = 0;

	return (char) atoi(sTemp);
}
#else
unsigned char box_IP_address[4];
#endif

DdmSocketServer::DdmSocketServer(DID did): Ddm(did) {
//	SetConfigAddress(&config,sizeof(config));	// tell Ddm:: where my config area is


#ifndef WIN32
		config.cIPa = box_IP_address[0];
		config.cIPb = box_IP_address[1];
		config.cIPc = box_IP_address[2];
		config.cIPd = box_IP_address[3];
#else

	FILE * fp = fopen("ssapi_server.config","r");
	if(fp) {
		config.cIPa = GetNextPart(fp);
		config.cIPb = GetNextPart(fp);
		config.cIPc = GetNextPart(fp);
		config.cIPd = GetNextPart(fp);
		fclose(fp);
	}
	else {
		printf("\nError reading from ssapi_server.config... IP address set to josh's ;)\n");
		config.cIPa = (char)10;
		config.cIPb = (char)1;
		config.cIPc = (char)1;
		config.cIPd = (char)100;
	}



#endif
	config.iPort = PORT_SSERVE;
	
	printf("\n#######################################################################");
	printf("\n      A friendly SSAPI interpreter is at IP: %d.%d.%d.%d", config.cIPa,config.cIPb,config.cIPc,config.cIPd );
	printf("\n#######################################################################");

	m_pMultiPacketMessageOrganizer = new MultiPacketMessageOrganizer();
}

Ddm *DdmSocketServer::Ctor(DID did) {
	return new DdmSocketServer(did);
}

STATUS DdmSocketServer::Initialize(Message *pMsg) {// virutal 
	DispatchRequest(REQ_SSAPI_RESPONSE, (ReplyCallback) &DdmSocketServer::ProcessSsapiResponse);

	Reply(pMsg,OK);
	return OK;
}

//Enable -- Start-it-up
STATUS DdmSocketServer::Enable(Message *pMsgReq) {
	NetMsgListen* pListenMsg = new NetMsgListen(config.cIPa,config.cIPb,config.cIPc,config.cIPd,config.iPort);

	Send(pListenMsg, NULL, (ReplyCallback) &DdmSocketServer::ListenReplyHandler);
	Reply(pMsgReq,OK);
	return OK;
}

void DdmSocketServer::FromSsapi(SsapiRequestMessage* pSsapiMsg) {

	int iSessionID = pSsapiMsg->m_iSessionID;
	Session* pSession = m_oSessionManager.GetSession(iSessionID);

	if(!pSession)
		return;

	NetMsgWrite* pWriteMsg = new NetMsgWrite(pSession->m_iConnectionID,iSessionID);

	//write packet packet
	pSession->Package(pWriteMsg, pSsapiMsg);

	pSession->Add(pWriteMsg,0);

	//if no write in progress, call write
	if(!pSession->WriteInProgress()) {
		pWriteMsg = pSession->GetWriteMsg();
		Send(pWriteMsg, NULL, (ReplyCallback) &DdmSocketServer::WriteReplyHandler);
	}

}

STATUS DdmSocketServer::ProcessSsapiResponse(Message* pMsgReq) {

	FromSsapi((SsapiRequestMessage*)pMsgReq);

	Reply(pMsgReq, OK);

	return OK;
}

STATUS DdmSocketServer::SsapiReplyHandler(Message* pMsgReq) {

	SsapiRequestMessage* pMsg = (SsapiRequestMessage*)pMsgReq;

	if(pMsg->m_iObjectCode != SSAPI_SESSION_TERMINATED)
		FromSsapi(pMsg);

	delete pMsg;
	return OK;
}

//ReplyHandler for calls to network listen
STATUS DdmSocketServer::ListenReplyHandler(Message *pMsgReq) {

	NetMsgListen* pListenMsg = (NetMsgListen*) pMsgReq;

	int iSessionID = m_oSessionManager.AddSession(pListenMsg->m_iConnectionID);
	Session* pSession = m_oSessionManager.GetSession(iSessionID);

	printf("\nNew Connection %d\n",iSessionID);

	//send the session information to the client
	NetMsgWrite* pWriteMsg = new NetMsgWrite(pListenMsg->m_iConnectionID,iSessionID);

	SsapiRequestMessage* pSsapiMsg = new SsapiRequestMessage();
	memset(pSsapiMsg->m_acSenderID, 0, 4);
	ValueSet * pValSet = new ValueSet();
	pValSet->AddValue(SSAPI_TYPE_INT,(char*) &iSessionID, SSAPI_TRANSPORT_SESSION_ID);
	pSsapiMsg->SetResponse(0,pValSet);

	//write packet
	pSession->Package(pWriteMsg, pSsapiMsg);
	pSession->Add(pWriteMsg,0);

	delete pSsapiMsg;

	//if no write in progress, call write
	if(!pSession->WriteInProgress()) {
		pWriteMsg = pSession->GetWriteMsg();
		Send(pWriteMsg, NULL, (ReplyCallback) &DdmSocketServer::WriteReplyHandler);
	}

	//we now call read for that connection
	NetMsgRead* pReadMsg = new NetMsgRead(pListenMsg->m_iConnectionID,iSessionID);
	Send(pReadMsg, NULL, (ReplyCallback) &DdmSocketServer::ReadReplyHandler);
	
	delete pListenMsg;
	return OK;
}

STATUS DdmSocketServer::ConnectReplyHandler(Message * pMsgReq) {
	NetMsgConnect* pConnectMsg = (NetMsgConnect*) pMsgReq;
	return OK;
}

STATUS DdmSocketServer::KillReplyHandler(Message * pMsgReq) {
	NetMsgKill* pKillMsg = (NetMsgKill*) pMsgReq;
	return OK;
}

STATUS DdmSocketServer::WriteReplyHandler(Message * pMsgReq) {
	NetMsgWrite* pWriteMsg = (NetMsgWrite*) pMsgReq;
	
	int iSessionID = pWriteMsg->m_iSessionID;

	if(pMsgReq->DetailedStatusCode != OK) {
		printf("Connection %d has been terminated\n",pWriteMsg->m_iConnectionID);
		m_oSessionManager.RemoveSession(iSessionID);
		delete pWriteMsg;

		//call cleanup session
		CleanupSession(iSessionID);
	}
	else {
		Session* pSession = m_oSessionManager.GetSession(iSessionID);

		//find this message and take it off the list of things to write	
		if(pSession) {

			pSession->MsgWritten(pWriteMsg);

			pWriteMsg = pSession->GetWriteMsg();
			if(pWriteMsg) {
				Send(pWriteMsg, NULL, (ReplyCallback) &DdmSocketServer::WriteReplyHandler);
			}
		}
		else {
			printf("got a write reply for a non-existent session\n");
		}
	}

	return OK;
}

void DdmSocketServer::CleanupSession(int iSessionID) {

	m_pMultiPacketMessageOrganizer->Clean(iSessionID);

	SsapiRequestMessage* pSsapiMsg = new SsapiRequestMessage();
	pSsapiMsg->m_iSessionID = iSessionID;
	pSsapiMsg->m_iObjectCode = SSAPI_SESSION_TERMINATED;
	Send(pSsapiMsg, NULL, REPLYCALLBACK(DdmSocketServer,SsapiReplyHandler));
}

STATUS DdmSocketServer::ReadReplyHandler(Message * pMsgReq) {
	NetMsgRead* pReadMsg = (NetMsgRead*) pMsgReq;
	
	if(pReadMsg->DetailedStatusCode == OK) {
		//pass the message to the correct session
		Session* pSession = m_oSessionManager.GetSession(pReadMsg->m_iSessionID);
		if(pSession) {
			char* pBuf;
			int iBufSize;
			pReadMsg->GetBuffer(pBuf,iBufSize);
			pSession->Read(pBuf, iBufSize);

			SsapiRequestMessage* pSsapiMsg = NULL;
			BOOL bFinished = FALSE;
			do {

				pSsapiMsg = pSession->CompleteMessage();

				if(pSsapiMsg) {

					pSsapiMsg->m_iSessionID = pReadMsg->m_iSessionID;

					//see if this message is part of a multi-packet message... if it is,
					//add it to a multi-packet receiver, then check for message complete (last packet)
					//if its cooked, then slice it up and serve it to the guests (DdmSsapi)

					if(pSsapiMsg->m_cPriority == SSAPI_PRIORITY_FILE_UPLOAD) {

						SsapiRequestMessage* pOriginalRequest = pSsapiMsg;

						pSsapiMsg = m_pMultiPacketMessageOrganizer->Place(pSsapiMsg);

						if(! pSsapiMsg) {
							//send an immediate response to signal successfull reciept.
							ValueSet* pReturnSet = new ValueSet();
							ValueSet* pRcSet = new ValueSet();

							pRcSet->AddInt(SSAPI_RC_SUCCESS, SSAPI_RETURN_STATUS);
							pReturnSet->AddValue( pRcSet, SSAPI_RETURN_STATUS_SET );
							delete pRcSet;
	
							pOriginalRequest->SetResponse(0, (ValueSet*) pReturnSet);

							FromSsapi(pOriginalRequest);

							delete pOriginalRequest;
						}
					}

					if(pSsapiMsg)
						Send(pSsapiMsg, NULL, REPLYCALLBACK(DdmSocketServer, SsapiReplyHandler));

				}
				else {
					bFinished = TRUE;
				}
			} while(!bFinished);
		}
		else {
			printf("recieved msg with invalid session.\n");
		}
	}
	else {
		printf("Connection %d has been terminated\n",pReadMsg->m_iConnectionID);
		m_oSessionManager.RemoveSession(pReadMsg->m_iSessionID);
		CleanupSession(pReadMsg->m_iSessionID);
	}
	delete pReadMsg;
	
	return OK;
}