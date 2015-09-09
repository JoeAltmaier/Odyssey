//
// CommandManager.cpp
//

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif

#include "CommandManager.h"
#include "DdmTestConsole.h"
#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include <String.h>

CT_Semaphore DdmTestConsole::testSemaphore;

	int ConsoleCommand::RunCommand(int _ConnectionID, char *_pArgs) {
//		Tracef("Running command [%s] [%s] -> %d\n", cmdName, _pArgs, _ConnectionID);
		return (pClass->*pMethod)(_ConnectionID, _pArgs);
	}

	int ConsoleCommandManager::AddCommand(char *_cmdName, char *_cmdDesc, MethodClass *_pClass, TestMethod _pMethod) {
		int index;

		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	

		for (index=0; index<numCommands; index++) {
			if (pCommand[index] == NULL)
				break;
		}
		
		if (index == numCommands) {
			// grow the command array
			ConsoleCommand** pTemp = new ConsoleCommand*[numCommands + 1];
			if (pCommand) {
				memcpy(pTemp, pCommand, sizeof(ConsoleCommand*) * numCommands);
				delete[] pCommand;
			}
			numCommands++;
//			Tracef("Added command (%d : %s)\n", numCommands, _cmdName);
			pCommand = pTemp;
		}
		
		pCommand[index] = new ConsoleCommand(_cmdName, _cmdDesc, _pClass, _pMethod);

		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);
		return index;
	}
	
	// scan through the commands looking for a match
	// and execute it if found
	int ConsoleCommandManager::RunCommand(int _ConnectionID, char *_pCmd, char *_pArgs) {	

		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	

		for (int i=0; i < numCommands; i++) {
			char *pTemp = pCommand[i]->cmdName;
			if (!strcmp(pTemp, _pCmd))
			{
//				Tracef("Running  (%d/%d).\n", i, numCommands);
				Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);		
				int status = pCommand[i]->RunCommand(_ConnectionID, _pArgs);
				return status;				
			}
			else
			{
//				Tracef("No match (%d/%d).\n", i, numCommands);
			}
		}
		
		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);				
		return -2;
	
	}
