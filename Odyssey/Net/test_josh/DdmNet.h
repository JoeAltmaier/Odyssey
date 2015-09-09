/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: DdmNet.h
//
// Description:
//    CHAOS TCP/IP Network Services DDM Header
//
/*************************************************************************/

#ifndef __DdmNet_H
#define __DdmNet_H

#define PLUS

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "NetMsgs.h"

class Connection {

public:

	Connection() {
		Kernel::Create_Semaphore(&m_Semaphore, "connection_semaphore",1);
		m_iWriteMsgs = 0;
		m_apWriteMsgs = NULL;
		m_iConnectionID = 0;
		m_pReadRequest = NULL;
		m_bConnected = FALSE;
	}
	
	BOOL m_bConnected;
	CT_Semaphore m_Semaphore;
	
	NetMsgRead* m_pReadRequest;
	
	int m_iWriteMsgs;
	Message** m_apWriteMsgs;
	
	int m_sock;
	int m_iConnectionID;
};	

class DdmNet : public Ddm {

#if 0
protected:
	void	*rwStack;
	void	*lStack;
	void	*ctStack;
	CT_Task *ctTask;
	
	CT_Task	*rwTask;
#endif
public:
	static Ddm *Ctor(DID did);
	DdmNet(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);	
	STATUS Quiesce(Message *pMsg);


	// move to protected?
	
	//command handlers
	STATUS ProcessListen(Message *pMsg);
	STATUS ProcessConnect(Message *pMsg);
	STATUS ProcessKill(Message *pMsg);
	STATUS ProcessWrite(Message *pMsg);
	STATUS ProcessRead(Message *pMsg);

	//signal handlers 
	STATUS HandleAccept(SIGNALCODE nSignal,void* pParam);
	STATUS HandleRead(SIGNALCODE nSignal,void* pParam);
	STATUS HandleWritten(SIGNALCODE nSignal, void* pParam);

	//thread methods
	void DoListen();
	void DoReadWrite();
#if 0
	//network init
	STATUS NetInit ();
	STATUS InitDevices ();
	STATUS PPPEval ();
	STATUS EtherInit ();
	
	STATUS EchoServer(Message *pMsg);
	
#endif
	
	STATUS ListenReplyHandler(Message *pMsgReq);

private:
	Connection* GetConnection(int iConnectionID);

	int m_iConnectionSize;
	Connection* m_aConnection;

	void QueMsg(Message** & apMsgQue, int & iMsgQueSize, Message* pMsgReq);
	Message* EnqueMsg(Message** & apMsgQue, int & iMsgQueSize);
	Message* PeekMsg(Message** apMsgQue, int iMsgQueSize);

	CT_Semaphore m_ListenSemaphore;
	int m_iListenMsgs;
	Message** m_apListenMsgs;
	CT_Semaphore timerSemaphore;
	
};
#if 0
class RqNetListen : public Message {
public:
	REFNUM refNum;	// Unique listen request identifier
	char ipaddr[4];	// IP address to listen on
	int port;		// Port to listen on
	int osocketd;	// original socket descriptor
	int socketd;	// accepted socket descriptor
	int status;		// listen status
	
	RqNetListen(char _ipaddr[4], int _port) : Message(REQ_NET_LISTEN) {
		refNum = MakeRefNum();
	    memcpy(ipaddr, &_ipaddr, 4);
	    port = _port;
	 }
};
#endif
BOOL HandleError(int iRet);

#endif	// __DdmNet_H

