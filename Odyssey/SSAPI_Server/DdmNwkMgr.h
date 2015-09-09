//DdmNwkMgr.h

#ifndef __DdmNwkMgr_H
#define __DdmNwkMgr_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "NetMsgs.h"

#include "SimpleQue.h"

class Connection {
public:
	Connection() {
		m_iConnectionID = 0;
		m_pReadRequest = NULL;
		m_bConnected = FALSE;
	}

	NetMsgWrite* PeekWriteMsg() {
		return (NetMsgWrite*)m_WriteQue.Peek();
	}

	NetMsgWrite* RemoveWriteMsg() {
		return (NetMsgWrite*)m_WriteQue.Remove();
	}

	void PutWriteMsg(Message* pMsg) {
		m_WriteQue.Put(pMsg);
	}

	BOOL m_bConnected;

	NetMsgRead* m_pReadRequest;

	SimpleQue m_WriteQue;

	SOCKET m_sock;
	int m_iConnectionID;
};

class DdmNwkMgr: public Ddm {
public:
	static Ddm *Ctor(DID did);
	DdmNwkMgr(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	
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
	STATUS HandleErrors(SIGNALCODE nSignal, void* pParam);

	//thread methods
	void DoListen();
	void DoReadWrite();

private:
	Connection* GetConnection(int iConnectionID);
	void RemoveConnection(int iConnectionID);

	int m_iConnectionSize;
	Connection** m_apConnection;

	void QueMsg(Message** & apMsgQue, int & iMsgQueSize, Message* pMsgReq);
	Message* EnqueMsg(Message** & apMsgQue, int & iMsgQueSize);
	Message* PeekMsg(Message** apMsgQue, int iMsgQueSize);

	SimpleQue m_ListenQue;

	CT_Semaphore m_Semaphore;
};

BOOL HandleError(int iRet);

#endif	// __DdmNwkMgr_H

