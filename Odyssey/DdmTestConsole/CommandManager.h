//
// CommandManager.h
//

#ifndef __CommandManager_H
#define __CommandManager_H

#include <String.h>
//#include "DdmTestConsole.h"

class MethodClass {};

typedef int (MethodClass::*TestMethod)(int, char *);
#ifdef WIN32
#define TESTCALL(clas,method)	(TestMethod) method
#elif defined(__ghs__)  // Green Hills
#define TESTCALL(clas,method)	(TestMethod) &clas::method
#else	// MetroWerks
#define TESTCALL(clas,method)	(TestMethod)&method
#endif

class ConsoleCommand {

public:
	char *cmdName;
	char *cmdDesc;
	MethodClass *pClass;
	TestMethod pMethod;	
	
	ConsoleCommand(char *_cmdName, char *_cmdDesc, MethodClass *_pClass, TestMethod _pMethod) {
		cmdName = new char[strlen(_cmdName) + 1];
		memcpy(cmdName, _cmdName, strlen(_cmdName) + 1);
		cmdDesc = new char[strlen(_cmdDesc) + 1];
		memcpy(cmdDesc, _cmdDesc, strlen(_cmdDesc) + 1);		
		pClass = _pClass;
		pMethod = _pMethod;
	}
	
	~ConsoleCommand() {}	
	
	int RunCommand(int _ConnectionID, char *_pArgs);

};

class ConsoleCommandManager {

public:
	ConsoleCommand** pCommand;
	int numCommands;
	
	ConsoleCommandManager() {
		pCommand = NULL;
		numCommands = 0;
	}
	
	~ConsoleCommandManager() {
		for (int i=0; i<numCommands; i++) {
			if (pCommand[i])
				delete pCommand[i];
		}
		delete[] pCommand;
	}

	int AddCommand(char *_cmdName, char *_cmdDesc, MethodClass *_pClass, TestMethod _pMethod);
	int RunCommand(int _ConnectionID, char *_pCmd, char *_pArgs);

};	

#endif