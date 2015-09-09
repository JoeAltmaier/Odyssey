//
// SessionManager.cpp
//

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif

#include "SessionManager.h"
#include "DdmTestConsole.h"
#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include <String.h>

CT_Semaphore DdmTestConsole::testSemaphore;

	void Session::AddToBuffer(char *_pBuf, int iSize) {

		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	
	
		if ((bufPos + iSize) > bufSize) {
			// grow the buffer
//			Tracef("Growing the Buffer\n");
			char *pTemp = new char[bufPos + iSize];
			if (pBuf) {
				memcpy(pTemp, pBuf, bufPos);
				delete[] pBuf;
			}
			bufSize = bufPos + iSize;
			pBuf = pTemp;
		}
				
		memcpy(&pBuf[bufPos], _pBuf, iSize);		
		bufPos += iSize;

		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);		
		
	}
	
	void Session::ClearBuffer() {
		// Note: this does not decrease the buffer size back to the default buffer size
		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);			
		bufPos = 0;
		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);		
		
	}
	
	int Session::BufferPosition() {
		return bufPos;
	}
		
	void Session::GetBuffer(char * &_pBuf, int &_iBufSize) {
		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	
		_pBuf = pBuf;
		_iBufSize = bufPos;
		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);				
	}
	
	void Session::DumpBuffer() {
		Tracef("Buffer Dump\n-----------\n[");
		for (int i=0; i < bufPos; i++)
		  Tracef("%c", pBuf[i]);
		Tracef("]\n");
	}

	int TestSessionManager::GetSessionIndex(int _ConnectionID) {

		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	
	
		for (int i=0; i<numSessions;i++) {
			if (pSession[i])
				if (pSession[i]->ConnectionID == _ConnectionID) {
					int status = i;
					Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);									
					return status;
				}
		}

		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);				
		return -1;
	}
	
	Session* TestSessionManager::GetSession(int _ConnectionID) {

		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	

		int sessionIndex = GetSessionIndex(_ConnectionID);
		
		if (sessionIndex == -1) {
			Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);		
			return NULL;
		}
			
		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);		
		return pSession[sessionIndex];	

	}
	
	int TestSessionManager::AddSession(int _ConnectionID) {
		int index;
		
		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	
		
		for (index=0; index<numSessions; index++) {
			if (pSession[index] == NULL)
				break;
		}
		
		if (index == numSessions) {
			// grow the session array
			Session** pTemp = new Session*[numSessions + 1];
			if (pSession) {
				memcpy(pTemp, pSession, sizeof(Session*) * numSessions);
				delete[] pSession;
			}
			numSessions++;
//			Tracef("Added session (now %d)\n", numSessions);
			pSession = pTemp;
		}
		
		pSession[index] = new Session(_ConnectionID);

		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);				
		return index;
	}
	
	void TestSessionManager::RemoveSession(int _ConnectionID) {

		int sessionIndex = GetSessionIndex(_ConnectionID);
	
		if (sessionIndex == -1) {
			return;
		}

		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	
		
		if (pSession[sessionIndex])
			delete pSession[sessionIndex];
		pSession[sessionIndex] = NULL;

		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);		
		
	}
