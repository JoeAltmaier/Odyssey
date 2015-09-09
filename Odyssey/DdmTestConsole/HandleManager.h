//
// HandleManager.h
//

#ifndef __HandleManger_H
#define __HandleManger_H

#include <String.h>
//#include "DdmTestConsole.h"

enum HandleOut {OutputOff=0, OutputOn, OutputLogged};
enum HandleState {Running=0x1, Finished=0x2, Error=0x4};

class HandleOutput {

public:
	int ConnectionID;
	int status;
	
	HandleOutput(int _ConnectionID, int _status) {
		ConnectionID = _ConnectionID;
		status = _status;
	}

};

class TestHandle {

public:
	char *cmdLine;
	int handle;
	HandleOutput** pOutput;
	int outputSize;
	int state;
	
	TestHandle(char *_cmdLine, int _handle) {
		cmdLine = new char[strlen(_cmdLine) + 1];
		memcpy(cmdLine, _cmdLine, strlen(_cmdLine) + 1);
		handle = _handle;
		pOutput = NULL;
		outputSize = 0;
		state = Running;
	}
	
	~TestHandle() {
		for (int i=0; i<outputSize; i++) {
			if (pOutput[i])
				delete pOutput[i];
		}
		delete[] pOutput;	
	}
	
	void SetOutput(int _ConnectionID, int _output);
	void SetState(int _state);	
	
};

class HandleManager {

public:
	TestHandle** pHandle;
	int numHandles;
	
	HandleManager() {
		pHandle = NULL;
		numHandles = 0;
	}
	
	~HandleManager() {
		for (int i=0; i<numHandles; i++) {
			if (pHandle[i])
				delete pHandle[i];
		}
		delete[] pHandle;
	}
	
	int AddHandle(char *_cmdLine, int _output);
	void RemoveHandle(int _HandleIndex);
	int GetOutput(int _ConnectionID, int _handle);
	int GetState(int _handle);
	
};
	
#endif