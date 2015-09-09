//DdmNwkMgr.cpp

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <stdio.h>
#include <String.h>
#include "OsTypes.h"
#include "DdmNwkMgr.h"
#include "BuildSys.h"

#define HANDLE_ACCEPT 10
#define HANDLE_READ 11
#define HANDLE_ERROR 12
#define HANDLE_WRITTEN 13

#define NO_FLAGS_SET 0

CLASSNAME(DdmNwkMgr,SINGLE);	// Class Link Name used by Buildsys.cpp

SERVELOCAL(DdmNwkMgr, REQ_NET_LISTEN);
SERVELOCAL(DdmNwkMgr, REQ_NET_CONNECT);
SERVELOCAL(DdmNwkMgr, REQ_NET_KILL);
SERVELOCAL(DdmNwkMgr, REQ_NET_READ);
SERVELOCAL(DdmNwkMgr, REQ_NET_WRITE);

DdmNwkMgr::DdmNwkMgr(DID did): Ddm(did) {
	WSADATA WSAData;
	WSAStartup(MAKEWORD(1,1), &WSAData);

	m_iConnectionSize = 0;
	m_apConnection = NULL;
}

Ddm *DdmNwkMgr::Ctor(DID did) {
	return new DdmNwkMgr(did);
}

STATUS DdmNwkMgr::Initialize(Message *pMsg) {// virutal 

	DispatchRequest(REQ_NET_LISTEN, (RequestCallback) ProcessListen);
	DispatchRequest(REQ_NET_CONNECT, (RequestCallback) ProcessConnect);
	DispatchRequest(REQ_NET_KILL, (RequestCallback) ProcessKill);
	DispatchRequest(REQ_NET_WRITE, (RequestCallback) ProcessWrite);
	DispatchRequest(REQ_NET_READ, (RequestCallback) ProcessRead);

	DispatchSignal(HANDLE_ACCEPT, (SignalCallback) HandleAccept);
	DispatchSignal(HANDLE_READ, (SignalCallback) HandleRead);
	DispatchSignal(HANDLE_WRITTEN, (SignalCallback) HandleWritten);
	DispatchSignal(HANDLE_ERROR, (SignalCallback) HandleErrors);

	Kernel::Create_Semaphore(&m_Semaphore, "listen_semaphore", 1);

	Reply(pMsg,OK);

	return OK;
}

void ReadWriteThread(void* pParam) {
	((DdmNwkMgr*)pParam)->DoReadWrite();
}

//Enable -- Start-it-up
STATUS DdmNwkMgr::Enable(Message *pMsgReq) {
	//spawn the read_write thread
	CT_Task ctTask;
	Kernel::Create_Thread(ctTask, "read_write_thread", ReadWriteThread, this, NULL, 0);

	Reply(pMsgReq,OK);
	return OK;
}

int iConnectionID = 0;

Connection* DdmNwkMgr::GetConnection(int iConnectionID) {
	for(int i=0;i<m_iConnectionSize; i++) {
		if(m_apConnection[i]->m_iConnectionID == iConnectionID)
			return m_apConnection[i];
	}
	return NULL;
}

void DdmNwkMgr::RemoveConnection(int iConnectionID) {
	BOOL bFound = FALSE;
	for(int i=0;i<m_iConnectionSize - 1;i++) {
		if(m_apConnection[i]->m_iConnectionID == iConnectionID)
			bFound = TRUE;
		if(bFound)
			m_apConnection[i] = m_apConnection[i + 1];
	}
	if(!bFound && m_apConnection[i]->m_iConnectionID != iConnectionID)
		printf("DdmNwkMgr internal error\n");
	else
		m_iConnectionSize--;
}

void DdmNwkMgr::DoListen() {//this method runs on its own thread

	NetMsgListen* pMsg = (NetMsgListen*)m_ListenQue.Remove();

	//set up the socket
	SOCKET sock;
	SOCKADDR_IN dest_sin;
	dest_sin.sin_family = AF_INET;
	
	dest_sin.sin_addr.s_net = (int)(pMsg->m_acIPAddress[0]);
	dest_sin.sin_addr.s_host = (int)(pMsg->m_acIPAddress[1]);
	dest_sin.sin_addr.s_lh = (int)(pMsg->m_acIPAddress[2]);
	dest_sin.sin_addr.s_impno = (int)(pMsg->m_acIPAddress[3]);

	dest_sin.sin_port = htons(pMsg->m_iPort);
	
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if(bind(sock,(SOCKADDR*)(&dest_sin),sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		HandleError(WSAGetLastError());
	
	if (sock == INVALID_SOCKET)
		return;//should we do something a little more elegant??

	ULONG lBlock = 1;
	int iRet;
	ioctlsocket(sock,FIONBIO,&lBlock);

	printf("\nListening...\n");

	//do the listening
	while(1) {
		iRet = listen(sock,SOMAXCONN);

		if(iRet != SOCKET_ERROR) {
			SOCKET pNewSock = accept(sock,NULL,NULL);

			if(pNewSock != INVALID_SOCKET) {
				Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

				//place in the accept que
				pMsg->SetConnectionID(++iConnectionID);

				//associate the socket with the connection ID
				Connection** apConn = new Connection*[m_iConnectionSize + 1];
				if(m_apConnection) {
					memcpy(apConn, m_apConnection, m_iConnectionSize * sizeof(Connection*));
					delete[] m_apConnection;
				}
				m_apConnection = apConn;
				m_apConnection[m_iConnectionSize] = new Connection();
				m_apConnection[m_iConnectionSize]->m_iConnectionID = iConnectionID;
				m_apConnection[m_iConnectionSize]->m_sock = pNewSock;
				m_apConnection[m_iConnectionSize++]->m_bConnected = TRUE;

				Kernel::Release_Semaphore(&m_Semaphore);
		
				Signal(HANDLE_ACCEPT,pMsg);
			}
			else {
				Kernel::Sleep(500);
				continue;
			}
		}
		else {
			//socket error, must recover
			HandleError(WSAGetLastError());
			Kernel::Sleep(500);
		}
	}
}

STATUS DdmNwkMgr::HandleAccept(SIGNALCODE nSignal,void* pParam) {

	//grab the accepted socket from the que
	NetMsgListen* pMsg = (NetMsgListen*)pParam;

	//respond to the listen message
	Reply(pMsg, OK, FALSE);

	return OK;
}

void ListenThread(void* pParam) {
	((DdmNwkMgr*)pParam)->DoListen();
}

//ProcessListen
STATUS DdmNwkMgr::ProcessListen(Message *pMsgReq) {

	//put this message in the listen que to be taken out by the listen thread
	m_ListenQue.Put(pMsgReq);

	//spawn a thread to acomplish the listen
	CT_Task ctTask;
	Kernel::Create_Thread(ctTask, "listen_thread", ListenThread,this, NULL, 0);

	return OK;
}

void DdmNwkMgr::DoReadWrite() {
	while(1) {
		BOOL bInactive = TRUE;
		Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);
		//do reads
		int i;
		for(i=0;i<m_iConnectionSize;i++) {
			if(m_apConnection[i]->m_pReadRequest != NULL && m_apConnection[i]->m_bConnected) {
				int iRet;
				char pBufRead[100];
				ULONG lAvailBytes;

				iRet = ioctlsocket(m_apConnection[i]->m_sock,FIONREAD,&lAvailBytes);
				if(iRet == SOCKET_ERROR) {
					m_apConnection[i]->m_bConnected = FALSE;
					Signal(HANDLE_ERROR,m_apConnection[i]);
					continue;
				}

				BOOL bTryRead = FALSE;
				if(lAvailBytes) {
					bTryRead = TRUE;
				}
				else {
					//check for socket dead
					fd_set error_set;
					FD_ZERO(&error_set);
					TIMEVAL tvZero = {0,0};
					FD_SET(m_apConnection[i]->m_sock, &error_set);

					iRet = select(0, &error_set, NULL , NULL, &tvZero);
					if(iRet == SOCKET_ERROR) {
						m_apConnection[i]->m_bConnected = FALSE;
						Signal(HANDLE_ERROR, m_apConnection[i]);
						continue;
					}
					else {
						if(error_set.fd_count > 0)
							bTryRead = TRUE;
					}
				}

				if(bTryRead) {
					iRet = recv(m_apConnection[i]->m_sock, pBufRead, min(100,(long)lAvailBytes),0);
		
					if(iRet == SOCKET_ERROR) {
						iRet = WSAGetLastError();
						if(! HandleError(iRet)) {
							m_apConnection[i]->m_bConnected = FALSE;
							Signal(HANDLE_ERROR,m_apConnection[i]);
						}
						continue;
					}
					else { 
						if(iRet) {
							//we have some new data in pBuf, of size iRet!!
							bInactive = FALSE;
							NetMsgRead* pMsg = new NetMsgRead(m_apConnection[i]->m_pReadRequest);
							pMsg->SetBuffer(pBufRead,iRet);
							Signal(HANDLE_READ,pMsg);
						}
						else { // clean socket exit
							m_apConnection[i]->m_bConnected = FALSE;
							Signal(HANDLE_ERROR,m_apConnection[i]);
							continue;
						}
					}
				}
			}
		}

		Kernel::Release_Semaphore(&m_Semaphore);

		Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

		//do writes
		for(i=0;i<m_iConnectionSize;i++) {
			NetMsgWrite* pMsgWrite = m_apConnection[i]->PeekWriteMsg();
			if(pMsgWrite && m_apConnection[i]->m_bConnected) {
				int iSize,iRet;
				char* pBuf;
				pMsgWrite->GetBuffer(pBuf,iSize);
				BOOL bDone = FALSE;
				while(pMsgWrite->m_iProcessPos < iSize && !bDone) {	
					
					iRet = send(m_apConnection[i]->m_sock,pBuf + pMsgWrite->m_iProcessPos,iSize - pMsgWrite->m_iProcessPos,NO_FLAGS_SET);
					
					if(iRet == SOCKET_ERROR) {
						iRet = WSAGetLastError();
						if( !HandleError(iRet)) {
							m_apConnection[i]->m_bConnected = FALSE;
							Signal(HANDLE_ERROR,m_apConnection[i]);
						}
						bDone = TRUE;
						break;//would block
					}
					else {
						if(iRet) {
							pMsgWrite->m_iProcessPos += iRet;
							if(pMsgWrite->m_iProcessPos == iSize) {
								m_apConnection[i]->RemoveWriteMsg();
								
								Signal(HANDLE_WRITTEN,pMsgWrite);	
							}
						}
						else { // Clean socket exit
							m_apConnection[i]->m_bConnected = FALSE;
							Signal(HANDLE_ERROR,m_apConnection[i]);
							break;
						}
					}
				}
			}
		}

		Kernel::Release_Semaphore(&m_Semaphore);

		if(bInactive) {
			Kernel::Sleep(10);
		}
	}
}

STATUS DdmNwkMgr::HandleWritten(SIGNALCODE nSignal, void* pParam) {
	//grab the message
	NetMsgWrite* pMsg = (NetMsgWrite*)pParam;

	//tell client that message was written successfully
	Reply(pMsg,OK);
	return OK;
}

STATUS DdmNwkMgr::HandleErrors(SIGNALCODE nSignal, void* pParam) {

	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);
	Connection* pConnection = (Connection*)pParam;
	RemoveConnection(pConnection->m_iConnectionID);
	Kernel::Release_Semaphore(&m_Semaphore);

	if(pConnection->m_pReadRequest) {
		Reply(pConnection->m_pReadRequest, !OK);
	}
	NetMsgWrite* pMsg = pConnection->RemoveWriteMsg();
	while(pMsg) {
		Reply(pMsg, !OK);
		pMsg = pConnection->RemoveWriteMsg();
	}

	closesocket(pConnection->m_sock);

	delete pConnection;

	return OK;
}

//ProcessWrite
STATUS DdmNwkMgr::ProcessWrite(Message *pMsgReq) {

	NetMsgWrite* pMsg = (NetMsgWrite*)pMsgReq;

	//place in the write que
	Kernel::Obtain_Semaphore(&m_Semaphore,CT_SUSPEND);
	Connection* pConnection = GetConnection(pMsg->m_iConnectionID);
	if(pConnection) {
		pConnection->PutWriteMsg(pMsg);
	}
	else {
		printf("Couldn't find connection with id = %d\n",pMsg->m_iConnectionID);
	}
	Kernel::Release_Semaphore(&m_Semaphore);

	return OK;
}

//Handle ReadSignal
STATUS DdmNwkMgr::HandleRead(SIGNALCODE nSignal,void* pParam) {
	//grab the accepted socket from the que
	NetMsgRead* pMsg = (NetMsgRead*)pParam;

	//respond to the read message
	Reply(pMsg, OK, false);

	return OK;
}

//ProcessRead
STATUS DdmNwkMgr::ProcessRead(Message *pMsgReq) {

	NetMsgRead* pReadMsg = (NetMsgRead*)pMsgReq;

	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);
	Connection* pConnection = GetConnection(pReadMsg->m_iConnectionID);

	if(pConnection) {
		pConnection->m_pReadRequest = pReadMsg;
	}
	else {
		Tracef("Read request for non-existent socket\n");
	}

	Kernel::Release_Semaphore(&m_Semaphore);
	
	return OK;
}

//ProcessConnect
STATUS DdmNwkMgr::ProcessConnect(Message *pMsgReq) {

	//not needed, just let it chill for now

	NetMsgConnect* pMsg = (NetMsgConnect*)pMsgReq;

	pMsg->SetConnectionID(++iConnectionID);

	Reply(pMsg, OK);
	return OK;
}

//ProcessKill
STATUS DdmNwkMgr::ProcessKill(Message *pMsgReq) {
	NetMsgKill* pMsg = (NetMsgKill*)pMsgReq;

	//I'm not sure how we will use this, just let it be for now

	Reply(pMsg, OK);
	return OK;
}

BOOL HandleError(int iRet) {
	BOOL bSaveSock = FALSE;
	switch (iRet) {
	case WSANOTINITIALISED:
		Tracef( "Socket Not Initialized.\n");
		break;
	case WSAENETDOWN:
		Tracef( "Socket Net Down.\n");
		break;
	case WSAEFAULT:
		Tracef( "Socket Fault.\n");
		break;
	case WSAENOTCONN:
		Tracef( "Socket Not Connection.\n");
		break;
	case WSAEINTR:
		Tracef( "Socket WSAEINTR.\n");
		break;
	case WSAEINPROGRESS :
		Tracef( "Socket In Progress.\n");
		break;
	case WSAENETRESET:
		Tracef( "Socket Net Reset.\n");
		break;
	case WSAENOTSOCK :
		Tracef( "Socket Not A Socket.\n");
		break;
	case WSAEOPNOTSUPP :
		Tracef( "Socket Option Not Supported.\n");
		break;
	case WSAESHUTDOWN :
		Tracef( "Socket Shut Down.\n");
		break;
	case WSAEWOULDBLOCK:
		bSaveSock = TRUE;
		break;
	case WSAEMSGSIZE :
		Tracef( "Socket Message Size.\n");
		break;
	case WSAEINVAL:
		Tracef( "Socket WSAEINVAL.\n");
		break;
	case WSAECONNABORTED:
		Tracef( "Socket Connection Aborted.\n");
		break;
	case WSAETIMEDOUT :
		Tracef( "Socket Timeout.\n");
		break;
	case WSAECONNRESET :
		Tracef( "Socket Connection Reset.\n");
		break;
	case WSAEADDRINUSE:
		Tracef("Socket Address In Use.\n");
		break;
	case WSAENOBUFS :
		Tracef("Socket Buffers Used Up.\n");
		break;
	case WSAEADDRNOTAVAIL:
		Tracef("Socket Address Not Available.\n");
		break;
	};
	return bSaveSock;
}