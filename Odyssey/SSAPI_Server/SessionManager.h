//SessionManager.h

#ifndef __SessionManager_H
#define __SessionManager_H

#include "SsapiRequestMessage.h"
#include "PacketParser.h"
#include "WriteQue.h"

class Session {
public:
	Session();
	~Session();
	void Read(char* pBuf, int iSize);
	SsapiRequestMessage* CompleteMessage();

	void Package(NetMsgWrite* pWriteMsg, SsapiRequestMessage* pSsapiMsg);

	void MsgWritten(NetMsgWrite* pMsg);
	NetMsgWrite* GetWriteMsg(BOOL bRemove = FALSE);
	bool WriteInProgress();
	void Add(NetMsgWrite* pMsg, int iPriority);
	
	int m_iConnectionID;
private:
	WriteQue m_oWriteQue;
	int m_iLastPeekedIndex;
	PacketParser m_oPacketParser;
};

class SessionManager {
public:
	SessionManager();
	~SessionManager();

	Session* GetSession(int iSessionID);

private:
	int GetSessionIndex(int iSessionID);

public:

	void RemoveSession(int iSessionID);

	//session id will be the index of this array at 
	//which the session lives... only expand when full
	Session** m_apSession;
	int m_iSessionSize;
	
	//returns the session id
	int AddSession(int iConnectionID);

	SsapiRequestMessage* PostMessage(int iSessionID);
	
};

#endif