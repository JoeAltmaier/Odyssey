//
// DdmTestConsole.cpp
//
//  Provides an interface to the test registry
//  Allows for serial or telnet connections
//

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif
#define PLUS
#include <stdio.h>
#include "OsTypes.h"
#include "DdmTestConsole.h"
#include "BuildSys.h"
#include "TestMsgs.h"
#include "TestTable.h"
#include <ctype.h>
#include "Network.h"

#include "osheap.h"

CLASSNAME(DdmTestConsole,SINGLE);	// Class Link Name used by Buildsys.cpp

// Damn we really need to get a common atoi!! This one is way too simple
int atoi (char *s)
{
	int i = 0;
	while(s && *s && isdigit(*s))
	{
		i = i * 10 + (*s - '0');
		++s;
	}
	return i;
}

static char *welcome_message = "\r\n                         ~         w   W   w\r\n             ~        ~        ~    \\  |  /   ~\r\n            o        ~   .:.:.:.     \\.|./              ~\r\n       ~                 wwWWWww      //   ~\r\n                 ((c     ))""""""((     //|        ~      ~\r\n        o       /\\/\\((  (( 6 6 ))   // |  ~                      ~\r\n               (d d  ((  )))^(((   //  |                  ~\r\n          o    /   / c((-(((')))-.//   |     ~\r\n              /===/ `) (( )))(( ,_/    |~         Welcome to Neptune\r\n       ~     /o o/  / c((( (()) |      |  ~     Odyssey Test Interface\r\n          ~  `~`^  / c (((  ))  |      |         ~\r\n                  /c  c(((  (   |  ~   |    ~     Probing the depths\r\n           ~     /  c  (((  .   |      |   ~         ~\r\n                / c   c ((^^^^^^`\\   ~ | ~        ~              ~\r\n               |c  c c  c((^^^ ^^^`\\   |                   ~\r\n       ~        \\ c   c   c(^^^^^^^^`\\ |    ~\r\n            ~    `\\ c   c  c;`\\^^^^^./ |             ~\r\n                   `\\c c  c  ;/^^^^^/  |  ~\r\n        ~        ~   `\\ c  c /^^^^/' ~ |       ~             ~\r\n              ~        `;c   |^^/'     o\r\n                  .-.  ,' c c//^\\\\         ~                        ~\r\n          ~      ( @ `.`c  -///^\\\\\\  ~             ~\r\n                  \\ -` c__/|/     \\|                      ~\r\n           ~       `---'   '   ~   '          ~\r\n\0";
static char *command_prompt = "Neptune> \0";

DdmTestConsole::DdmTestConsole(DID did) : Ddm(did) {

}

Ddm *DdmTestConsole::Ctor(DID did) {
	return new DdmTestConsole(did);
}

STATUS DdmTestConsole::Initialize(Message *pMsg) {

	Tracef("DdmTestConsole::Initialize()\n");
	
	Kernel::Create_Semaphore(&testSemaphore, "test_console_semaphore", 1);	

	Reply(pMsg, OK);
	return OK;
	
}

STATUS DdmTestConsole::Enable(Message *pMsg) {

	Tracef("DdmTestConsole::Enable()\n");
	
	NetMsgListen* pListenMsg = new NetMsgListen(10, 40, 20, 200, PORT_NEPTUNE);
	Send(pListenMsg, NULL, REPLYCALLBACK(DdmTestConsole, ListenReplyHandler));

	SessionMan = new TestSessionManager();
	CommandMan = new ConsoleCommandManager();
	HandleMan = new HandleManager();
	
	// Add the commands
	CommandMan->AddCommand("help\0", "This command\0", this, TESTCALL(this, DisplayHelp));		
	CommandMan->AddCommand("list\0", "List available tests\0", this, TESTCALL(this, DisplayList));			
	CommandMan->AddCommand("run\0", "Run the given test - run [TestName] [TestArgs]\0", this, TESTCALL(this, RunTest));
	CommandMan->AddCommand("ps\0", "Display processes (with mask if requested) - ps [running/finished/error/#]\0", this, TESTCALL(this, DisplayHandles));
	CommandMan->AddCommand("output\0", "Set output to this terminal - output [process #] [0=off, 1=on]\0", this, TESTCALL(this, SetHandle));
	CommandMan->AddCommand("exit\0", "Close this console session", this, TESTCALL(this, CloseSession));
	
	Reply(pMsg, OK);
	return OK;

}

//ReplyHandler for calls to network listen
STATUS DdmTestConsole::ListenReplyHandler(Message *pMsgReq) {

	NetMsgListen* pListenMsg = (NetMsgListen*) pMsgReq;

	int sessionNum;

//	Tracef("\nDdmTestConsole: New Connection (%d)\n",pListenMsg->m_iConnectionID);
	
	sessionNum = SessionMan->AddSession(pListenMsg->m_iConnectionID);
//	Tracef("Session %d added\n", sessionNum);

	// Welcome new client
//	WriteString(pListenMsg->m_iConnectionID, "\r\n**** Odyssey Test Interface ****\r\n\r\n\0");
	WriteString(pListenMsg->m_iConnectionID, welcome_message);
	DisplayCommandLine(pListenMsg->m_iConnectionID);
	
	//we now call read for that connection
	NetMsgRead* pReadMsg = new NetMsgRead(pListenMsg->m_iConnectionID,pListenMsg->m_iConnectionID);
	Send(pReadMsg, NULL, REPLYCALLBACK(DdmTestConsole, ReadReplyHandler));
	
	delete pListenMsg;
	return OK;
}

STATUS DdmTestConsole::ReadReplyHandler(Message * pMsgReq) {

	NetMsgRead* pReadMsg = (NetMsgRead*) pMsgReq;
	
	// a happy read buffer was received
	if(pReadMsg->DetailedStatusCode == OK) {
	
		char *pBuf;
		char *pTemp;
		char *pCommandLine;
		char *pCommand = NULL;
		char *pArgs = NULL;
		int clSize;
		int iBufSize;
		int currSession;
		int oldSize;
		int tempi;
		
		pReadMsg->GetBuffer(pBuf,iBufSize);		
		
		currSession = SessionMan->GetSessionIndex(pReadMsg->m_iConnectionID);
		
		// Get the old size of the buffer
		oldSize = SessionMan->pSession[currSession]->BufferPosition();
												
		// Scan input stream
		for (int i=0; i < iBufSize; i++) {
		
			switch (pBuf[i]) {

				// [ESC]: Clear Buffer / line on the screen
				case 0x1B:
					SessionMan->pSession[currSession]->ClearBuffer();
					WriteString(pReadMsg->m_iConnectionID, "\r\0");
					WriteString(pReadMsg->m_iConnectionID, "                                                                  \r\0");
					DisplayCommandLine(pReadMsg->m_iConnectionID);
					break;
					
				// Backspace
				case 0x08:
					if (SessionMan->pSession[currSession]->BufferPosition() > 0) {
						SessionMan->pSession[currSession]->bufPos--;
						WriteString(pReadMsg->m_iConnectionID, "\b\0");
						WriteString(pReadMsg->m_iConnectionID, " \0");
						WriteString(pReadMsg->m_iConnectionID, "\b\0");
					}
					break;

				// Disregard newline
				case 0x0A:
					break;
									
				// Enter (command in buffer)
				case 0x0D:
					SessionMan->pSession[currSession]->GetBuffer(pTemp, clSize);
					if (clSize > 0) {
						STATUS status;
						pCommandLine = new char[clSize+1];
						memcpy(pCommandLine, pTemp, clSize);
						pCommandLine[clSize] = 0;
						
						SessionMan->pSession[currSession]->ClearBuffer();
						
						// trim whitespace off front (move the data, not the pointer)
						while (pCommandLine[0] == ' ') {
							memcpy(pCommandLine, &pCommandLine[1], strlen(pCommandLine));				
						}

						// and back
						for (int iCurrent = strlen(pCommandLine); iCurrent > 0; iCurrent--) {
							if (pCommandLine[iCurrent-1] != ' ')
								break;
							else
								pCommandLine[iCurrent-1] = 0;
						}
						
						// if there actually is a command here, process the command
						if (strlen(pCommandLine) > 0) {
						
							// process the command - it must be done here since there could be
							// multiple commands per incoming buffer
							SplitCommand(pCommandLine, pCommand, pArgs);
//							Tracef("Command : [%s]\nArgs : [%s]\n", pCommand, pArgs);
						
							// Determine what the command is and execute it
							status = CommandMan->RunCommand(pReadMsg->m_iConnectionID, pCommand, pArgs);
							// icky - this really needs some spit shining
							// we didn't find a command
							if (status == -2)
							{
								WriteString(pReadMsg->m_iConnectionID, "\n\rInvalid command\n\r\0");											
							}
				
							delete[] pCommandLine;
						}
						
					}
				
					// Display the command line prompt
					WriteString(pReadMsg->m_iConnectionID, "\n\r\0");					
					DisplayCommandLine(pReadMsg->m_iConnectionID);
					break;

				// Dump test tables out serial port (Ctrl-T)
				case 0x14:
					TestTable::Dump();
					break;
					
				// Report delta memory (Ctrl-D)
				case 0x04:
					OsHeap::heapSmall.ReportDeltaMemoryUsage("Mem Report");
					break;			

				// Regular character, copy into buffer and echo to client
				default:
					SessionMan->pSession[currSession]->AddToBuffer(&pBuf[i], 1);
					WriteChar(pReadMsg->m_iConnectionID, pBuf[i]);					
					break;
					
			} // end switch
		} // end for iBufSize
	} // end if DetailedStatus OK
	else {
		// We have lost the connection with that socket
		// DdmNet should handle cleanup of outstanding messages, but we need to do some internal
		// cleanup.  Remove the session it was under.  We should also cleanup all the outputs
		// involving that ConnectionID, but that will have to wait
//		Tracef("DdmTestConsole: Connection %d has been terminated\n",pReadMsg->m_iConnectionID);
		SessionMan->RemoveSession(pReadMsg->m_iConnectionID);				
	}
	delete pReadMsg;
	
	return OK;
}

// Takes a string (null terminated), places a null at the space in between the
// command and args, and returns pointers to the command and arguments
void DdmTestConsole::SplitCommand(char *pCommandLine, char* &pCommand, char* &pArgs) {

	int tempi;
	int clSize = strlen(pCommandLine);
	
	for (tempi=0; tempi < clSize; tempi++) {
		if (pCommandLine[tempi] == ' ')
		{
			pCommandLine[tempi] = 0;
			break;
		}
	}
	
	pCommand = pCommandLine;
	if (tempi == clSize)
		// We never found a space, return no arguments
		pArgs = &pCommandLine[tempi];
	else 
		// we found a space, so point to right after that
		pArgs = &pCommandLine[tempi+1];

}

int DdmTestConsole::DisplayCommandLine(int _ConnectionID, char *pCommand) {

	WriteString(_ConnectionID, command_prompt);				
	return 0;

}

int DdmTestConsole::DisplayHelp(int _ConnectionID, char *pCommand) {

	NetMsgWrite* pWriteMsg = new NetMsgWrite(_ConnectionID,_ConnectionID);	
	char *pBufsend = new(tZERO) char[1000];
	strcpy(pBufsend, "\n\rHelp:\n\r");
	for(int i=0; i < CommandMan->numCommands; i++) {
		strcat(pBufsend, CommandMan->pCommand[i]->cmdName);
		strcat(pBufsend, " : ");
		strcat(pBufsend, CommandMan->pCommand[i]->cmdDesc);
		strcat(pBufsend, "\n\r");
	}
	pWriteMsg->SetBuffer(pBufsend, strlen(pBufsend)+1);
	delete[] pBufsend;
	Send(pWriteMsg, NULL, REPLYCALLBACK(DdmTestConsole, WriteReplyHandler));
	return 0;

}

int DdmTestConsole::DisplayList(int _ConnectionID, char *pCommand) {

	TestEntry* pFirstEntry = TestTable::GetFirst();
	
	NetMsgWrite* pWriteMsg = new NetMsgWrite(_ConnectionID,_ConnectionID);	
	char *pBufsend = new(tZERO) char[1000];
	strcpy(pBufsend, "\n\rAvailable Test Procedures:\n\r");

	if (pFirstEntry == NULL)
		strcat(pBufsend, "  <empty>\n\r");
	for (TestEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry=pEntry->pNextEntry) {
		strcat(pBufsend, pEntry->pszName);
		strcat(pBufsend, "\n\r");		
	}	

	pWriteMsg->SetBuffer(pBufsend, strlen(pBufsend)+1);
	delete[] pBufsend;
	Send(pWriteMsg, NULL, REPLYCALLBACK(DdmTestConsole, WriteReplyHandler));
	return 0;

}

int DdmTestConsole::RunTest(int _ConnectionID, char *pCommandLine) {

	char *pCommand = NULL;
	char *pArgs = NULL;	
	int handle;

	SplitCommand(pCommandLine, pCommand, pArgs);
	
	// Generate the handle
	handle = HandleMan->AddHandle(pCommandLine, OutputOn);
	// Set the status to output on by default
	HandleMan->pHandle[handle]->SetOutput(_ConnectionID, OutputOn);

	// Run the command!
	TestMsgRun* pTestMsg = new TestMsgRun(pCommand, pArgs, handle);
	Send(pTestMsg, NULL, REPLYCALLBACK(DdmTestConsole, InputStream));
	
	return 0;

}

int DdmTestConsole::DisplayHandles(int _ConnectionID, char *pCommandLine) {

	int displayMask = 0x0;
	char *pCurrent = pCommandLine;
	int iCurrent;
	char *pArg;
	char *pRest;
	int rulenum, tempi, pos;

	// trim whitespace off front
	for (pos=0; pos < (int)strlen(pCommandLine); pos++) {
		if (pCommandLine[pos] == ' ')
			pCurrent++;
		else
			break;
	}
	
	// and back
	for (iCurrent = strlen(pCurrent); iCurrent > 0; iCurrent--) {
		if (pCurrent[iCurrent-1] != ' ')
			break;
	}
	
	// If we didn't get any arguments, display all the handles
	if (iCurrent == 0)	{
		displayMask = 0xFFFFFFFF;
	}
	// If we got a digit, we must be requesting individual handles
	else if (isdigit(pCurrent[0])) {
		SplitCommand(pCurrent, pArg, pRest);
		while (pArg[0] != 0) {
			int handlenum = atoi(pArg);
			if (handlenum < HandleMan->numHandles) {
				WriteString(_ConnectionID, "\n\rHandle Information:\n\r\0");
				DisplayHandle(_ConnectionID, handlenum);		
			}
			else {
				WriteString(_ConnectionID, "\n\rInvalid handle specified\n\r\0");				
			}
			pCurrent = pRest;
			SplitCommand(pCurrent, pArg, pRest);
		}
		return 0;
	}
	// else I guess we must want to mask the entire list
	else {
	
		SplitCommand(pCurrent, pArg, pRest);
		while (pArg[0] != 0) {
			if (!strcmp(pArg, "running"))
				displayMask = displayMask |= Running;
			else if (!strcmp(pArg, "finished"))
				displayMask = displayMask |= Finished;
			else if (!strcmp(pArg, "error"))
				displayMask = displayMask |= Error;	
			pCurrent = pRest;
			SplitCommand(pCurrent, pArg, pRest);
		}
		
		// if we didn't recognize any masks, it must have been a typo - display them all
		if (displayMask ==  0x0)
			displayMask = 0xFFFFFFFF;
	}

	WriteString(_ConnectionID, "\n\rHandle List:\n\r\0");

	for(int i=0; i < HandleMan->numHandles; i++) {
		DisplayHandle(_ConnectionID, i, displayMask);
	}

	return 0;
	
}

int DdmTestConsole::DisplayHandle(int _ConnectionID, int handleNum, int displayMask) {

	NetMsgWrite* pWriteMsg = new NetMsgWrite(_ConnectionID,_ConnectionID);	
	char *pBufsend = new(tZERO) char [200];
	char *pBuftemp = new(tZERO) char[80];	

	if ((HandleMan->pHandle[handleNum]) && (HandleMan->GetState(handleNum) & displayMask)) {
		sprintf(pBuftemp, "%d  ", handleNum);
		strcat(pBufsend, pBuftemp);			
		switch (HandleMan->GetState(handleNum)) {
			case Running:
				strcat(pBufsend, "Running   ");
				break;
			case Finished:
				strcat(pBufsend, "Finished  ");
				break;
			case Error:
				strcat(pBufsend, "Error     ");
				break;
			default:
				strcat(pBufsend, "Unknown   ");
				break;
		}
		switch (HandleMan->GetOutput(_ConnectionID, handleNum)) {
			case OutputOn:
				strcat(pBufsend, "Output On  ");	
				break;			
			case OutputOff:
				strcat(pBufsend, "Output Off ");				
				break;
			case OutputLogged:
				strcat(pBufsend, "Logged     ");				
				break;
			default:
				strcat(pBufsend, "Invalid!   ");
				break;
		}
		strcat(pBufsend, HandleMan->pHandle[handleNum]->cmdLine);
		strcat(pBufsend, "\n\r");
	}
		
	pWriteMsg->SetBuffer(pBufsend, strlen(pBufsend)+1);
	delete[] pBufsend;
	delete[] pBuftemp;	
	Send(pWriteMsg, NULL, REPLYCALLBACK(DdmTestConsole, WriteReplyHandler));
	return OK;

}

int DdmTestConsole::SetHandle(int _ConnectionID, char *pCommandLine) {

	char *pHandleNum = NULL;
	char *pOption = NULL;	
	int handle;
	int option;

	SplitCommand(pCommandLine, pHandleNum, pOption);
	
	if ((pOption[0] == 0) || (pOption[0] == ' '))
	{
		WriteString(_ConnectionID, "\n\rIncorrect syntax - please see help\n\r\0");				
		return -1;
	}
	
	handle = atoi(pHandleNum);
	option = atoi(pOption);
	
	// Set the output for this terminal
	if (handle < HandleMan->numHandles) {
		HandleMan->pHandle[handle]->SetOutput(_ConnectionID, option);
	}
	else {
		WriteString(_ConnectionID, "\n\rInvalid process number\n\r\0");				
		return -1;
	}

	return 0;

}

int DdmTestConsole::CloseSession(int _ConnectionID, char *pCommand) {

	WriteString(_ConnectionID, "\n\rClosing console session..\n\r\0");

	for(int i=0; i < HandleMan->numHandles; i++) {
		HandleMan->pHandle[i]->SetOutput(_ConnectionID, OutputOff);
	}
	
	NetMsgKill* pKillMsg = new NetMsgKill(_ConnectionID);
	Send(pKillMsg, NULL, REPLYCALLBACK(DdmOsServices, DiscardOkReply));

	return 0;

}

void DdmTestConsole::WriteString(int _ConnectionID, char *inStr) {

	NetMsgWrite* pWriteMsg = new NetMsgWrite(_ConnectionID,_ConnectionID);	
	pWriteMsg->SetBuffer(inStr, strlen(inStr)+1);
	Send(pWriteMsg, NULL, REPLYCALLBACK(DdmTestConsole, WriteReplyHandler));	

}

void DdmTestConsole::WriteChar(int _ConnectionID, char ch) {

	NetMsgWrite* pWriteMsg = new NetMsgWrite(_ConnectionID,_ConnectionID);	
	pWriteMsg->SetBuffer(&ch, 1);
	Send(pWriteMsg, NULL, REPLYCALLBACK(DdmTestConsole, WriteReplyHandler));	

}
	
STATUS DdmTestConsole::WriteReplyHandler(Message * pMsgReq) {

	NetMsgWrite* pMsg = (NetMsgWrite*)pMsgReq;

	if (pMsg->DetailedStatusCode != OK) {
//		Tracef("Tried to write to invalid connection (%d)\n", pMsg->m_iConnectionID);
		// need to clean up that connections remenants, for now just turn off the output on all handles
		for(int i=0; i < HandleMan->numHandles; i++) {
			HandleMan->pHandle[i]->SetOutput(pMsg->m_iConnectionID, OutputOff);
		}
	}

	delete pMsg;
	return OK;
	
}

STATUS DdmTestConsole::InputStream(Message * pMsgReq) {

	TestMsgRun* pMsg = (TestMsgRun*)pMsgReq;
	
	if(pMsg->DetailedStatusCode == OK) {
		
		if (!pMsg->IsLast()) {

			char *pBuf;
			int iBufSize;
			pMsg->GetBuffer(pBuf,iBufSize);	
	
			// Scan through the status records and output the data to the correct locations
			for (int i=0; i < HandleMan->pHandle[pMsg->iTestHandle]->outputSize; i++) {
				int ConnectionID = HandleMan->pHandle[pMsg->iTestHandle]->pOutput[i]->ConnectionID;
				switch (HandleMan->pHandle[pMsg->iTestHandle]->pOutput[i]->status) {
					case OutputOn:
//						Tracef("Con: Sending %d bytes to %d from %08x\n", iBufSize, ConnectionID, &pBuf);	
//						Tracef("Con:  Buffer contains [");
//						for (int j=0;j<iBufSize;j++)
//			  				Tracef("%c", pBuf[j]);
//						Tracef("]\n");
						
						NetMsgWrite* pWriteMsg = new NetMsgWrite(ConnectionID,ConnectionID);
						pWriteMsg->SetBuffer(pBuf, iBufSize);
						Send(pWriteMsg, NULL, REPLYCALLBACK(DdmTestConsole, WriteReplyHandler));					
						break;			
					case OutputOff:
//						Tracef("Connection %d output off\n", ConnectionID);
						break;
					case OutputLogged:
						WriteString(ConnectionID, "\n\rLogged output not implemented\n\r\0");				
						break;
					default:
						WriteString(ConnectionID, "\n\rInvalid handle output!!\n\r\0");
						break;		
				}	
			} 
		}
		else
		{
			Tracef("Test %d completed\n", pMsg->iTestHandle);
			// This is the last message - change the handle states to finished
			HandleMan->pHandle[pMsg->iTestHandle]->SetState(Finished);	
		}
	}
	else if (pMsg->DetailedStatusCode == UNKNOWN_TEST) {
		// Tell the user that the test manager could not find a test by that name
		// Should we delete the handle or leave it on the list as an error?
		WriteString(HandleMan->pHandle[pMsg->iTestHandle]->pOutput[0]->ConnectionID, "\n\rUnknown test requested.\n\r\0");						
		HandleMan->pHandle[pMsg->iTestHandle]->SetState(Error);
	}
	else if (pMsg->DetailedStatusCode == PARSE_ERROR) {
		// Tell the user that the test manager could not parse the arguments
		// Should we delete the handle or leave it on the list as an error?
		WriteString(HandleMan->pHandle[pMsg->iTestHandle]->pOutput[0]->ConnectionID, "\n\Error parsing arguments.\n\r\0");						
		HandleMan->pHandle[pMsg->iTestHandle]->SetState(Error);	
	}
	else {
//		Tracef("Bad InputStream\n");	
	}

	delete pMsg;
	return OK;
	
}


