//
// SessionManager.h
//

#ifndef __SessionManager_H
#define __SessionManager_H

#include "OsTypes.h"
//#include "DdmTestConsole.h"

#define DEFAULTBUFFERSIZE 80

class Session {

public:
	char* pBuf;
	int bufPos;
	int bufSize;
	int ConnectionID;

	Session(int _ConnectionID) {
		pBuf = NULL;
		bufSize = DEFAULTBUFFERSIZE;
		if (bufSize > 0) {
			pBuf = new char[bufSize];
		}
		bufPos = 0;
		ConnectionID = _ConnectionID;
	}
	
	~Session() {}

	void AddToBuffer(char *_pBuf, int iSize);
	void ClearBuffer();
	int BufferPosition();
	void GetBuffer(char * &_pBuf, int &_iBufSize);
	void DumpBuffer();

};

class TestSessionManager {

public:
	Session** pSession;
	int numSessions;

	TestSessionManager() {
		pSession = NULL;
		numSessions = 0;
	}
	
	~TestSessionManager() {
		for (int i=0; i<numSessions; i++) {
			if (pSession[i])
				delete pSession[i];
		}
		delete[] pSession;
	}
	
	int GetSessionIndex(int _ConnectionID);
	Session* GetSession(int _ConnectionID);
	int AddSession(int _ConnectionID);
	void RemoveSession(int _ConnectionID);
	
};

#endif