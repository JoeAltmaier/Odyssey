//
// DdmTestConsole.h
//

#ifndef __DdmTestConsole_H
#define __DdmTestConsole_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "NetMsgs.h"
#include <String.h>

#define PLUS

#include "HandleManager.h"
#include "CommandManager.h"
#include "SessionManager.h"

class DdmTestConsole : public Ddm, public MethodClass {
	
public:

	static Ddm *Ctor(DID did);
	DdmTestConsole(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);

	STATUS ListenReplyHandler(Message *pMsgReq);
	STATUS WriteReplyHandler(Message *pMsgReq);
	STATUS ReadReplyHandler(Message *pMsgReq);
	
	STATUS InputStream(Message *pMsgReq);

	static CT_Semaphore testSemaphore;		
	
private:
	TestSessionManager* SessionMan;
	ConsoleCommandManager* CommandMan;	
	HandleManager* HandleMan;
	
	void SplitCommand(char *pCommandLine, char* &pCommand, char* &pArgs);

	void WriteString(int _ConnectionID, char *inStr);	
	void WriteChar(int _ConnectionID, char ch);
	
	// Command handlers
	int DisplayCommandLine(int _ConnectionID, char *pCommand = NULL);
	int DisplayHelp(int _ConnectionID, char *pCommand = NULL);
	int DisplayList(int _ConnectionID, char *pCommand = NULL);
	int RunTest(int _ConnectionID, char *pCommandLine);
	int DisplayHandles(int _ConnectionID, char *pCommand = NULL);
	int DisplayHandle(int _ConnectionID, int handleNum, int displayMask = 0xFFFFFFFF);
	int SetHandle(int _ConnectionID, char *pCommand = NULL);
	int CloseSession(int _ConnectionID, char *pCommand);
	
};

int atoi(char *s);

#endif