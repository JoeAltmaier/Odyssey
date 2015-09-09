//
// HandleManager.cpp
//

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif

#include "Kernel.h"
#include "HandleManager.h"
#include "DdmTestConsole.h"

CT_Semaphore DdmTestConsole::testSemaphore;

	void TestHandle::SetOutput(int _ConnectionID, int _output) {
		
		int index;
		int found = 0;

		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	
		
		// check to see if a status already exists for this connection
		// if not, add it to the array, else change it
		for (index=0; index<outputSize; index++) {
			if (pOutput[index]->ConnectionID == _ConnectionID) {
				found = 1;
				break;
			}
		}
		
		if (!found)	{		
			for (index=0; index<outputSize; index++) {
				if (pOutput[index] == NULL)
					break;
			}
		
			if (index == outputSize) {
				// grow the status array
				HandleOutput** pTemp = new HandleOutput*[outputSize + 1];
				if (outputSize > 0) {
					memcpy(pTemp, pOutput, sizeof(HandleOutput*) * outputSize);
					// hmmm
					delete[] pOutput;
				}
				outputSize++;
//				Tracef("Added handle output (now %d)\n", outputSize);
				pOutput = pTemp;
			}

			pOutput[index] = new HandleOutput(_ConnectionID, _output);

		}
		else {
			pOutput[index]->status = _output;
		}

		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);

	}
	
	void TestHandle::SetState(int _state) {

		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	
	
		state = _state;
		
		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);
		

	}
		
	
	int HandleManager::AddHandle(char *_cmdLine, int _status) {

		int index;
		
		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	
		
		for (index=0; index<numHandles; index++) {
			if (pHandle[index] == NULL)
				break;
		}
		
		if (index == numHandles) {
			// grow the handle array
			TestHandle** pTemp = new TestHandle*[numHandles + 1];
			if (pHandle) {
				memcpy(pTemp, pHandle, sizeof(TestHandle*) * numHandles);
				delete[] pHandle;
			}
			numHandles++;
//			Tracef("Added handle (now %d)\n", numHandles);
			pHandle = pTemp;
		}
		
		pHandle[index] = new TestHandle(_cmdLine, index);
		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);
		return index;
			
	}
	
	void HandleManager::RemoveHandle(int _HandleIndex) {

		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	
		
		if (pHandle[_HandleIndex])
			delete pHandle[_HandleIndex];
		pHandle[_HandleIndex] = NULL;
		
		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);
		
	}
	
	int HandleManager::GetOutput(int _ConnectionID, int _handle) {

		Kernel::Obtain_Semaphore(&DdmTestConsole::testSemaphore, CT_SUSPEND);	
	
		for (int i=0; i < pHandle[_handle]->outputSize; i++) {
			if (pHandle[_handle]->pOutput[i]->ConnectionID == _ConnectionID) {
				int status =  pHandle[_handle]->pOutput[i]->status;
				Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);
				return status;				
			}
		}

		Kernel::Release_Semaphore(&DdmTestConsole::testSemaphore);		
		return OutputOff;		
			
	}

	int HandleManager::GetState(int _handle) {
	
		return pHandle[_handle]->state;		
			
	}
	
