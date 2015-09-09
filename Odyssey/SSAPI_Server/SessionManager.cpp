//SessionManager.cpp

#include "OsTypes.h"
#include "string.h"
#include "SessionManager.h"

//--------------------------------------------
//--------------Session-----------------------

double rWeight = 100.0;

Session::Session() : m_oWriteQue(1,&rWeight) {
	m_iConnectionID = 0;
	m_iLastPeekedIndex = -1;
}

Session::~Session() {
	//the session must clean up write msgs

	BOOL bStrand = FALSE;
	if(m_iLastPeekedIndex != -1) 
		bStrand = TRUE;

	NetMsgWrite* pMsg;

	//we strand one if it is currently outstanding
	//... so we don't delete it twice
	if(bStrand) {
		pMsg = GetWriteMsg(TRUE);
		pMsg = GetWriteMsg();
	}
	else 
		pMsg = GetWriteMsg();
	
	while(pMsg) {
		MsgWritten(pMsg);
		pMsg = GetWriteMsg();
	}
}

void Session::Read(char* pBuf, int iSize) {
	m_oPacketParser.AddData(pBuf, iSize);
}

void Session::Package(NetMsgWrite* pWriteMsg, SsapiRequestMessage* pSsapiMsg) {
	m_oPacketParser.Package(pWriteMsg, pSsapiMsg);
}

void Session::MsgWritten(NetMsgWrite* pMsg) {
	QueNode* pNode;
	if(m_oWriteQue.GrabFromQue(pNode,m_iLastPeekedIndex)) {
		delete pNode;
	}
	m_iLastPeekedIndex = -1;
	delete pMsg;
}

NetMsgWrite* Session::GetWriteMsg(BOOL bRemove) {
	QueNode* pNode;
	if(! WriteInProgress()) {
		if(m_oWriteQue.CheckQue(pNode,m_iLastPeekedIndex,!bRemove)) {
			//convert this into a NwkMsgWrite and return it
			NetMsgWrite* pMsgWrite = pNode->m_pMsg;
			return pMsgWrite;
		}
	}
	
	return NULL;
}

bool Session::WriteInProgress() {
	if(m_iLastPeekedIndex == -1)
		return false;
	return true;
}

void Session::Add(NetMsgWrite* pMsg, int iPriority) {
	m_oWriteQue.AddToQue(iPriority,pMsg);
}

SsapiRequestMessage* Session::CompleteMessage() {
	SsapiRequestMessage* pMsg = m_oPacketParser.GetCompleteMessage();
	return pMsg;
}

//--------------------------------------------
//---------------Session Manager--------------

SessionManager::SessionManager() {
	m_apSession = NULL;
	m_iSessionSize = 0;
}

SessionManager::~SessionManager() {
	for(int i=0;i<m_iSessionSize;i++) {
		if(m_apSession[i])
			delete m_apSession[i];
	}
	delete[] m_apSession;
}

int SessionManager::GetSessionIndex(int iSessionID) {
	for(int i=0;i<m_iSessionSize;i++) {
		if(m_apSession[i]) {
			if(m_apSession[i]->m_iConnectionID == iSessionID)
				return i;
		}
	}
	return -1;
}

Session* SessionManager::GetSession(int iSessionID) {
	
	int iLocation =  GetSessionIndex(iSessionID);

	if(iLocation == -1)
		return NULL;

	return m_apSession[iLocation];
}

void SessionManager::RemoveSession(int iSessionID) {

	int iLocation = GetSessionIndex(iSessionID);

	if(iLocation == -1)
		return;

	if(m_apSession[iLocation]) 
		delete m_apSession[iLocation];
	m_apSession[iLocation] = NULL;
}

int SessionManager::AddSession(int iConnectionID) {
	int i;
	for(i=0;i<m_iSessionSize;i++) {
		if(m_apSession[i] == NULL)
			break;
	}
	
	if (i == m_iSessionSize) {
		//then we need to grow the session array
		Session** apTemp = new Session*[m_iSessionSize + 1];
		if(m_apSession) {
			memcpy(apTemp, m_apSession, sizeof(Session*) * m_iSessionSize);
			delete[] m_apSession;
		}
		m_iSessionSize++;
		m_apSession = apTemp;
	}
	
	m_apSession[i] = new Session();
	m_apSession[i]->m_iConnectionID = iConnectionID;
	return iConnectionID;
}