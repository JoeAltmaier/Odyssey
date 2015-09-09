/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: DdmNet.cpp
//
// Description:
//    CHAOS TCP/IP Network Services DDM 
//
/*************************************************************************/

// Config options
#define BUFFERSIZE 5000

// Use to make virtual serves
#define VIRTUAL_NET

// Hack to set the gateway
#define GWa 208
#define GWb 25
#define GWc 238
#define GWd 1

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif

#define TRACE_INDEX		TRACE_DDM_NET

#include "DdmNet.h"
#include "BuildSys.h"
#include "RqOsTimer.h"
#include "OsTypes.h"

// Network Includes
#include "externs.h"
#include "nucleus.h"
#include "socketd.h"
#include "tcpdefs.h"
#include "sockext.h"

#include "Message.h"

#define HANDLE_ACCEPT 10
#define HANDLE_READ 11
#define HANDLE_ERROR 12
#define HANDLE_WRITTEN 13

#define NO_FLAGS_SET 0
	
CLASSNAME (DdmNet, SINGLE);  // Class Link Name used by Buildsys.cpp --
                              // must match CLASSENTRY() name formal (4th arg)
                              // in buildsys.cpp

//  statically declare which request codes our DDM will serve
//  use the VIRTUAL_NET define above to select local or virtual serves
#ifdef VIRTUAL_NET
SERVEVIRTUAL (DdmNet, REQ_NET_LISTEN);
SERVEVIRTUAL (DdmNet, REQ_NET_CONNECT);
SERVEVIRTUAL (DdmNet, REQ_NET_KILL);
SERVEVIRTUAL (DdmNet, REQ_NET_READ);
SERVEVIRTUAL (DdmNet, REQ_NET_WRITE);
SERVEVIRTUAL (DdmNet, REQ_NET_CHANGEIP);
SERVEVIRTUAL (DdmNet, REQ_NET_CHANGEGATEWAY);
#else
SERVELOCAL (DdmNet, REQ_NET_LISTEN);
SERVELOCAL (DdmNet, REQ_NET_CONNECT);
SERVELOCAL (DdmNet, REQ_NET_KILL);
SERVELOCAL (DdmNet, REQ_NET_READ);
SERVELOCAL (DdmNet, REQ_NET_WRITE);
SERVELOCAL (DdmNet, REQ_NET_CHANGEIP);
SERVELOCAL (DdmNet, REQ_NET_CHANGEGATEWAY);
#endif

//  DdmNet::DdmNet (did)
//
//  Description:
//    Our friendly constructor.
//
//  Inputs:
//    did - CHAOS "Device ID" of the instance we are constructing.
//
//  Outputs:
//    none
//

DdmNet::DdmNet (DID did) : Ddm (did)
{

	TRACEF(TRACE_L8, ("DdmNet::DdmNet()\n"));
   
	rwStack	= new(tZERO) U8[8000];
 	lStack	= new(tZERO) U8[8000];
   
	// connection info
   
   m_iConnectionSize = 0;
   m_aConnection = NULL;
   
   m_iListenMsgs = 0;
   m_apListenMsgs = NULL;
   
}
/*	end DdmNet::DdmNet  *****************************************************/

//  DdmNet::Ctor (did)
//	  (static)
//
//  Description:
//    Our static, standard system-defined helper function.
//    This routine is called by CHAOS when it wants to create
//    an instance of DdmNet.
//
//  Inputs:
//    did - CHAOS "Device ID" of the new instance we are to create.
//
//  Outputs:
//    DdmNet::Ctor - Returns a pointer to the new instance, or NULL.
//

Ddm *DdmNet::Ctor (DID did)
{

   TRACEF(TRACE_L8, ("DdmNet::Ctor()\n"));

   return (new DdmNet (did));

} 
/*	end DdmNet::Ctor  *****************************************************/

//  DdmNet::Initialize (pMsg)
//    (virtual)
//
//  Description:
//    Called for a DDM instance when the instance is being created.
//    This routine is called after the DDM's constructor, but before
//    DdmNet::Enable().
//
//    Finally, we call our base class' Initialize(), after we complete
//    our local functionality.
//
//  Inputs:
//    pMsg - Points to message which triggered our DDM's fault-in.  This is
//          always an "initialize" message.
//
//  Outputs:
//    DdmNet::Initialize - Returns OK if all is cool, else an error.
//

STATUS DdmNet::Initialize (Message *pMsg)
{

	TRACEF(TRACE_L8, ("DdmNet::Initialize()\n"));

	DispatchRequest(REQ_NET_LISTEN, REQUESTCALLBACK (DdmNet, ProcessListen));
	DispatchRequest(REQ_NET_CONNECT, REQUESTCALLBACK (DdmNet, ProcessConnect));
	DispatchRequest(REQ_NET_KILL, REQUESTCALLBACK (DdmNet, ProcessKill));
	DispatchRequest(REQ_NET_WRITE, REQUESTCALLBACK (DdmNet, ProcessWrite));
	DispatchRequest(REQ_NET_READ, REQUESTCALLBACK (DdmNet, ProcessRead));
	DispatchRequest(REQ_NET_CHANGEIP, REQUESTCALLBACK (DdmNet, ProcessChangeIP));	
	DispatchRequest(REQ_NET_CHANGEGATEWAY, REQUESTCALLBACK (DdmNet, ProcessChangeGateway));		

	DispatchSignal(HANDLE_ACCEPT, SIGNALCALLBACK (DdmNet, HandleAccept));
	DispatchSignal(HANDLE_READ, SIGNALCALLBACK (DdmNet, HandleRead));
	DispatchSignal(HANDLE_WRITTEN, SIGNALCALLBACK (DdmNet, HandleWritten));
	DispatchSignal(HANDLE_ERROR, SIGNALCALLBACK (DdmNet, HandleError));

	Kernel::Create_Semaphore(&m_ListenSemaphore, "listen_semaphore", 1);
	Kernel::Create_Semaphore(&m_readSemaphore, "read_semaphore", 1);	

	Reply(pMsg,OK);
	return OK;
		
}
/*	end DdmNet::Initialize  *****************************************************/

void ReadWriteThread(void* pParam) {
	TRACEF(TRACE_L8, ("ReadWriteThread launching DoReadWrite()\n"));
	((DdmNet*)pParam)->DoReadWrite();
}

//  DdmNet::Enable (pMsgReq)
//
//  Description:
//   Per the DDM model, quiesce this DDM

STATUS DdmNet::Quiesce (Message *pMsg)
{ 


   TRACEF(TRACE_L8, ("DdmNet::Quiesce()\n"));
   
   //  * * *  do local Quiesce stuff here.  * * *

   //  signal CHAOS that our DDM instance is finished quiescing.
   Reply (pMsg, OK);

   return (OK);      // (must *always* return success)

}  /* end of CDdmCMB::Quiesce */


//  DdmNet::Enable (pMsgReq)
//
//  Description:
//   Lets go!

STATUS DdmNet::Enable(Message *pMsgReq)
{

	TRACEF(TRACE_L8, ("DdmNet::Enable()\n"));
	
	//spawn the read_write thread
	rwTask = new(tZERO) CT_Task;		
	rwStack = new(tZERO) U8[8000];
	Kernel::Create_Thread(*rwTask, "read_write_thread", ReadWriteThread, this, rwStack, 8000);
	Kernel::Schedule_Thread(*rwTask);

	Reply(pMsgReq,OK);
	return OK;
		
}
/* end DdmNet::Enable  *****************************************************/

int iConnectionID = 0;

//	DdmNet::QueMsg(**&apMsgQue, &iMsgQueSize, *pMsgReq)
//
//	Description:
//	  Queue a message for reception by thread
//
void DdmNet::QueMsg(Message** & apMsgQue, int *iMsgQueSize, Message* pMsgReq) {
	Message** apMsgs = new Message*[*iMsgQueSize + 1];
	if(apMsgQue) {
		memcpy(apMsgs, apMsgQue, (*iMsgQueSize)*sizeof(Message*));		
		delete[] apMsgQue;
	}
	apMsgQue = apMsgs;
	apMsgQue[(*iMsgQueSize)++] = pMsgReq;
	
}
/*	end DdmNet::QueMsg  *****************************************************/

//	DdmNet::PeekMsg(** apMsgQue, iMsgQueSize)
//
//	Description:
//	  Checks for a message for reception by a thread
//
Message* DdmNet::PeekMsg(Message** apMsgQue, int iMsgQueSize) {
	Message* pMsg = NULL;
	if(iMsgQueSize > 0)
		pMsg = apMsgQue[0];
	
	return pMsg;

}
/*	end DdmNet::PeekMsg  *****************************************************/

//	DdmNet::EnqueMsg(**&apMsgQue, &iMsgQueSize)
//
//	Description:
//	  Takes a message off the queue
//
Message* DdmNet::EnqueMsg(Message** & apMsgQue, int *iMsgQueSize) {
	Message* pMsg = NULL;
	if(apMsgQue) {
		pMsg = apMsgQue[0];
		if(*iMsgQueSize > 1) {
			for(int i=0;i<*iMsgQueSize - 1;i++) {
				apMsgQue[i] = apMsgQue[i+1];
			}
		}
		else {
			delete[] apMsgQue;
			apMsgQue = NULL;
		}
		(*iMsgQueSize)--;
	}
	return pMsg;
	
}
/*	end DdmNet::EnqueMsg  *****************************************************/

//	DdmNet::GetConnection(iConnectionID)
//
//	Description:
//	  Returns the connection
//		
Connection* DdmNet::GetConnection(int iConnectionID) {
	for(int i=0;i<m_iConnectionSize; i++) {
		if(m_aConnection[i].m_iConnectionID == iConnectionID)
			return &(m_aConnection[i]);
	}
	return NULL;
	
}
/*	end DdmNet::GetConnection  *****************************************************/

//	DdmNet::RemoveConnection(iConnectionID)
//
//	Description:
//	  Removes a connection
//		
void DdmNet::RemoveConnection(int iConnectionID) {
	int i;
	BOOL bFound = FALSE;
	TRACEF(TRACE_L5, ("Removing connection (conn %d)\n", iConnectionID));	
	for(i=0;i<m_iConnectionSize - 1;i++) {
		if(m_aConnection[i].m_iConnectionID == iConnectionID)
			bFound = TRUE;
		if(bFound)
			m_aConnection[i] = m_aConnection[i + 1];
	}
	if(!bFound && m_aConnection[i].m_iConnectionID != iConnectionID)
	{
		TRACEF(TRACE_L4, ("DdmNet internal error\n"));
	}
	else
	{
		m_iConnectionSize--;
	}
}
/*	end DdmNet::RemoveConnection  *****************************************************/

//  DdmNet::DoListen ()
//
//  Description:
//		Called to perform the actual listen on a socket.
//		Runs (happily) in its own thread
//		One thread per listen request
//
void DdmNet::DoListen() {

	Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);
	NetMsgListen* pMsg = (NetMsgListen*)(EnqueMsg(m_apListenMsgs, &m_iListenMsgs));
	Kernel::Release_Semaphore(&m_ListenSemaphore);

	struct 			addr_struct	serverAddr;
	int				osocketd, nsocketd;
	struct 			addr_struct	clientAddr;	
	STATUS			status;

	TRACEF(TRACE_L8, ("DoListen()\n"));
	
	// open a connection via the socket interface
	if ((osocketd = NU_Socket(NU_FAMILY_IP, NU_TYPE_STREAM, 0)) >= 0)
	{
	
	// fill in the address structure
	serverAddr.family = NU_FAMILY_IP;
	serverAddr.port = htons(pMsg->m_iPort);
	memcpy(&(pMsg->m_acIPAddress), ip_ethernet, 4);
	memcpy(serverAddr.id.is_ip_addrs, &(pMsg->m_acIPAddress), 4);	
	
	// make an NU_Bind() call to bind the server's address
	status = NU_Bind((int16)osocketd, &serverAddr, 0);

	if (status >= 0)
	{
	    // be ready to accept connection requests
	    status = NU_Listen((int16)osocketd, 10);

	    if (status == NU_SUCCESS)
	    {
			// successful listen request, check for connections

			// set socket to non-blocking operations
			NU_Fcntl(osocketd, NU_SETFLAG, NU_FALSE);

			// keep trying for a connection
			while (1) {
				
				// If the device address has changed, start listening on the correct address
				if (memcmp(pMsg->m_acIPAddress, ip_ethernet, 4))
				{
					TRACEF(TRACE_L7, ("Listen saw IP change from %08lx to %08lx\n", *(U32*)pMsg->m_acIPAddress, *(U32*)ip_ethernet));
					NU_Close_Socket(osocketd);
					memcpy(&(pMsg->m_acIPAddress), ip_ethernet, 4);
					memcpy(serverAddr.id.is_ip_addrs, &(pMsg->m_acIPAddress), 4);
					if ((osocketd = NU_Socket(NU_FAMILY_IP, NU_TYPE_STREAM, 0)) < 0)
					{
						ReportError(status, __FILE__, __LINE__);
						break;				
					}
					status = NU_Bind((int16)osocketd, &serverAddr, 0);
					if (status < 0)
					{
						ReportError(status, __FILE__, __LINE__);
						break;
					}
					status = NU_Listen((int16)osocketd, 10);
					if (status != NU_SUCCESS)
					{
						ReportError(status, __FILE__, __LINE__);
						break;
					}
					NU_Fcntl(osocketd, NU_SETFLAG, NU_FALSE);				
				}
			
	    		nsocketd = NU_Accept((int16)osocketd, &clientAddr, 0);
			    if (nsocketd >= 0)
			    {
    			
    				TRACEF(TRACE_L5, ("Connection accepted\n"));
    				
    				Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);

					//set the connection ID
					pMsg->SetConnectionID(++iConnectionID);

					//create the connection object and fill in connection info
					Connection* aConn = new Connection[m_iConnectionSize + 1];
					if(m_aConnection) {
						memcpy(aConn, m_aConnection, sizeof(Connection) * m_iConnectionSize);
						delete[] m_aConnection;
					}
					m_aConnection = aConn;
					m_aConnection[m_iConnectionSize].m_iConnectionID = iConnectionID;
					m_aConnection[m_iConnectionSize].m_sock = nsocketd;
					m_aConnection[m_iConnectionSize++].m_bConnected = TRUE;

					// Set TCP state to push so transfers occur immediately
					NU_Push(nsocketd);	

					Kernel::Release_Semaphore(&m_ListenSemaphore);
		
					Signal(HANDLE_ACCEPT,pMsg);

				}									// end of successful NU_Accept
				// no connection ready at this time
				else if (nsocketd == NU_NO_TASK_MATCH)
				{
					// Nap time, try again later
					NU_Sleep(10);
					continue;
				}
				// error code
				else
				{
					ReportError(nsocketd, __FILE__, __LINE__);
					NU_Sleep(10);
				}
				
			}									// end while (1) Accept loop

	    }                                       // end successful NU_Listen
	    else
	    {
	    	ReportError(status, __FILE__, __LINE__);
	    	Reply(pMsg, !OK);
	    }
	}                                           // end successful NU_Bind
	else
	{
		ReportError(status, __FILE__, __LINE__);
		Reply(pMsg, !OK);
	}

    }                                           // end successful NU_Socket
    else
    {
	    // unable to obtain socket
	    ReportError(osocketd, __FILE__, __LINE__);
	    Reply(pMsg, !OK);
	    return;
    }
	
}
/*	end DdmNet::DoListen  *****************************************************/

//	DdmNet::HandleAccept(nSignal, *pParam)
//
//	Description:
//		A connection has been accepted by the listen thread - send a reply to the
//		callback.  Signaled by listen thread (DoListen).

STATUS DdmNet::HandleAccept(SIGNALCODE nSignal,void* pParam) {

	// remove compilation warnings
	nSignal;

	Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);
	
	//grab the accepted socket from the que
	NetMsgListen* pMsg = (NetMsgListen*)pParam;

	Kernel::Release_Semaphore(&m_ListenSemaphore);

	TRACEF(TRACE_L5, ("New Connection...\n"));

	//respond to the listen message
	Reply(pMsg, OK, FALSE);

	return OK;

}
/*	end DdmNet::HandleAccept  *****************************************************/


void ListenThread(void* pParam) {
	TRACEF(TRACE_L8, ("Listenthread()\n"));
	((DdmNet *)pParam)->DoListen();
}
/*	end DdmNet::ListenThread  *****************************************************/


//	DdmNet::ProcessListen(*pMsgReq)
//
//	Description:
//		Process a listen request from the client
//		by queing and creating listen thread
//		Note--needs update to provide better support for multiple listens
//			--cannot remove listen threads on kills/cancels if task info cannot be
//				retrieved

STATUS DdmNet::ProcessListen(Message *pMsgReq) {

	TRACEF(TRACE_L8, ("ProcessListen()\n"));

	Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);

	//put this message in the listen que to be taken out by the listen thread
	QueMsg(m_apListenMsgs, &m_iListenMsgs, pMsgReq);

	Kernel::Release_Semaphore(&m_ListenSemaphore);

	//spawn a thread to acomplish the listen
    	
	lTask = new(tZERO) CT_Task;	  	
	lStack = new(tZERO) U8[8000];
	Kernel::Create_Thread(*lTask, "listen_thread", ListenThread, this, lStack, 8000);
	Kernel::Schedule_Thread(*lTask);

	return OK;
}
/*	end DdmNet::ProcessListen  *****************************************************/

//	DdmNet::ResetSignalSocket()
//
//	Description:
//		Resets the given signal socket to connected and NO data waiting.
//		This will stay this way until SetSignalSocket is called to fake
//		out the NU_Select routine

void DdmNet::ResetSignalSocket(int signalSocket) {

    struct sock_struct *sockptr; 

	Kernel::Obtain_Semaphore(&m_readSemaphore, CT_SUSPEND);

    if ((sockptr = socket_list[signalSocket]) == NU_NULL)
    {
		TRACEF(TRACE_L4, ("Error - invalid signal socket (DdmNet::ResetSignalSocket(%d))\n", signalSocket));
	}
	else
	{
	 	sockptr->s_state = SS_ISCONNECTED;
		sockptr->s_recvbytes = 0;
	}

	Kernel::Release_Semaphore(&m_readSemaphore);	
	
}
/*	end DdmNet::ResetSignalSocket  *****************************************************/

//	DdmNet::SetSignalSocket()
//
//	Description:
//		Sets the given signal socket to connected and data waiting.
//		The read-write thread is then resumed.  NU_Select will then
//		See the data waiting and get out of the NU_Select routine,
//		enabling writes or reloads to be processed.

void DdmNet::SetSignalSocket(int signalSocket) {

    struct sock_struct *sockptr; 
    
	Kernel::Obtain_Semaphore(&m_readSemaphore, CT_SUSPEND);

    if ((sockptr = socket_list[signalSocket]) == NU_NULL)
    {
		TRACEF(TRACE_L4, ("Error - invalid signal socket (DdmNet::ResetSignalSocket(%d))\n", signalSocket));
	}
	else
	{
	 	sockptr->s_state = SS_ISCONNECTED;	
		sockptr->s_recvbytes = 1;
		NU_Resume_Task(rwTask);
	}
	
	Kernel::Release_Semaphore(&m_readSemaphore);	
	
}
/*	end DdmNet::SetSignalSocket  *****************************************************/

//	DdmNet::FillReadFS()
//
//	Description:
//		Reloads the readfs bitmap with the reloadSignalSocket,
//		writeSignalSocket, and all sockets with outstanding reads
//		The two signal sockets are used to break out of the
//		suspend and either perform a write or force a reload (when
//		a new read request comes in)

void DdmNet::FillReadFS() {
	
	TRACEF(TRACE_L8, ("Repacking bitmap (%d entries)\n", m_iConnectionSize));
	Kernel::Obtain_Semaphore(&m_readSemaphore, CT_SUSPEND);
	// Clear the bitmap
	NU_FD_Init(&readfs);
	Kernel::Release_Semaphore(&m_readSemaphore);	
	// Reset the data ready bits
	ResetSignalSocket(reloadSignalSocket);
	ResetSignalSocket(writeSignalSocket);
	// Set watch bits on the signal sockets
	NU_FD_Set(reloadSignalSocket, &readfs);
	NU_FD_Set(writeSignalSocket, &readfs);	
	Kernel::Obtain_Semaphore(&m_readSemaphore, CT_SUSPEND);	
	// Fill in all the sockets we want to check for reads on
	for (int i=0; i < m_iConnectionSize; i++)
	{
		if(m_aConnection[i].m_pReadRequest != NULL)
			NU_FD_Set(m_aConnection[i].m_sock, &readfs);
	}
	Kernel::Release_Semaphore(&m_readSemaphore);	
	
}
/*	end DdmNet::FillReadFS  *****************************************************/

//	DdmNet::DoReadWrite()
//
//	Description:
//		Process outstanding reads and writes
//		Runs in its own thread
//		One thread for all outstanding reads and writes

void DdmNet::DoReadWrite() {

	int status;
	char pBufRead[BUFFERSIZE];
	
	// Create a signalling socket which will enable NU_Select to see that it needs to
	// reload the bitmap from outstanding read messages
	// Create one to signal a write has come in as well
	if (((reloadSignalSocket = NU_Socket(NU_FAMILY_IP, NU_TYPE_STREAM, 0)) >= 0) && ((writeSignalSocket = NU_Socket(NU_FAMILY_IP, NU_TYPE_STREAM, 0)) >= 0))
	{
		
		while(1) 
		{

			// start off by filling of the bitmap with the socket information
			FillReadFS();

			// check for data to be read or written, as well as signals
			// there is a timeout set because for some reason, if NU_SUSPEND
			// is set, it will not exit from (resume) NU_Select on a disconnect
			status = NU_Select(NSOCKETS, &readfs, &readfs, &readfs, 6000);
			if ( status == NU_SUCCESS)
			{
				Kernel::Obtain_Semaphore(&m_readSemaphore, CT_SUSPEND);
			
				// check for reads available
				for(int i=0;i<m_iConnectionSize;i++) {
					if((m_aConnection[i].m_pReadRequest != NULL) && (NU_FD_Check(m_aConnection[i].m_sock, &readfs) == NU_TRUE) && m_aConnection[i].m_bConnected) {
						TRACEF(TRACE_L7, ("Found read request (%d)\n", m_aConnection[i].m_sock));
						int iRet;
						U32 lAvailBytes;

						// turn off blocking for read
						iRet = NU_Fcntl(m_aConnection[i].m_sock, NU_SETFLAG, NU_FALSE);
				
						if(iRet < 0) {
							m_aConnection[i].m_bConnected = FALSE;
							ReportError(iRet, m_aConnection[i].m_sock, __FILE__, __LINE__);
							Signal(HANDLE_ERROR, &m_aConnection[i]);										
							continue;
						}

						lAvailBytes = BUFFERSIZE;
						// keep getting data from socket while it is feeding
						// this could cause a problem if data is coming in constantly
						// maybe a top limit should be set on how many iterations this loop can go!
						while (lAvailBytes == BUFFERSIZE)
						{
							// get available bytes
	    	   				lAvailBytes = NU_Recv(m_aConnection[i].m_sock, pBufRead, BUFFERSIZE, 0);
	       					// if not connected, disconnect
			    	    	if (lAvailBytes == NU_NOT_CONNECTED)
				    	    {
								m_aConnection[i].m_bConnected = FALSE;
								ReportError(iRet, m_aConnection[i].m_sock, __FILE__, __LINE__);								
								Signal(HANDLE_ERROR,&m_aConnection[i]);																		
					            continue;
	    	   			    }
		    	   		    // else handle error which occured
			    	        else if (lAvailBytes < 0)
				    	    {
								m_aConnection[i].m_bConnected = FALSE;				    	    
	        					ReportError(lAvailBytes, __FILE__, __LINE__);
								Signal(HANDLE_ERROR,&m_aConnection[i]);																		
			       	   		}	
							// else, data received
						    else if (lAvailBytes > 0) {
	
								// we have some new data in pBufRead, of size lAvailBytes
								NetMsgRead* pMsg = new NetMsgRead(m_aConnection[i].m_pReadRequest);
								pMsg->SetBuffer(pBufRead,lAvailBytes);
								TRACEF(TRACE_L7, ("buffered read (%d bytes) (conn %d)\n", lAvailBytes, i));
								Signal(HANDLE_READ,pMsg);						
							} // end else if
						} // end while
					} // end if
				} // end for
				
				// if the writeSignalSocket has been set, write out the data
				if (NU_FD_Check(writeSignalSocket, &readfs) == NU_TRUE)	
				{
					TRACEF(TRACE_L7, ("Incoming write request..\n"));
					for(int i=0;i<m_iConnectionSize;i++) {
						// check for queued write messages
						NetMsgWrite* pMsgWrite = (NetMsgWrite*)PeekMsg(m_aConnection[i].m_apWriteMsgs, m_aConnection[i].m_iWriteMsgs);
						if(pMsgWrite && m_aConnection[i].m_bConnected) 
						{
							// get buffer to write
							int iSize,iRet;
							char* pBuf;
							pMsgWrite->GetBuffer(pBuf,iSize);
							TRACEF(TRACE_L7, ("  Writing %d bytes from conn %d\n", iSize, i));
							TRACEF(TRACE_L7, ("  Buffer contains ["));
							for (int i=0;i<iSize;i++)
				  				TRACEF(TRACE_L7, ("%c", pBuf[i]));
							TRACEF(TRACE_L7, ("]\n"));
				
							BOOL bDone = FALSE;
	
							// keep sending while buffer not completely sent
							while(pMsgWrite->m_iProcessPos < iSize && !bDone) {
								NU_Fcntl(m_aConnection[i].m_sock, NU_SETFLAG, NU_BLOCK);
								iRet = NU_Send(m_aConnection[i].m_sock, pBuf + pMsgWrite->m_iProcessPos, iSize - pMsgWrite->m_iProcessPos, 0);
								if (iRet < 0) {
									ReportError(iRet, __FILE__, __LINE__);
									m_aConnection[i].m_bConnected = FALSE;
									Signal(HANDLE_ERROR,&m_aConnection[i]);										
									bDone = TRUE;
									break;
								}
								else {
									if(iRet) {
										pMsgWrite->m_iProcessPos += iRet;
										if(pMsgWrite->m_iProcessPos == iSize) {
											EnqueMsg(m_aConnection[i].m_apWriteMsgs,&m_aConnection[i].m_iWriteMsgs);
											Signal(HANDLE_WRITTEN,pMsgWrite);
										}
									}
								}	
							} // end while (sending)
						} // end if(pMsgWrite)
					} // end for			
				}  // end if writeSignal
				
				// if the reloadSignalSocket has been set, there were not necessarily reads or writes to
				// process, but the bitmap has to be repacked - this will be done above
				
				// check to make sure all the TCP connections we think are open
				// actually still are
				for (int i=0; i<m_iConnectionSize;i++)
				{
					if (m_aConnection[i].m_bConnected == TRUE)
					{
						if (NU_Is_Connected(m_aConnection[i].m_sock) == NU_FALSE)
						{
							TRACEF(TRACE_L5, ("Disconnect detected (%d)\n", m_aConnection[i].m_iConnectionID));
							m_aConnection[i].m_bConnected = FALSE;
							Signal(HANDLE_ERROR,&m_aConnection[i]);										
						}
					}
				}
			
				Kernel::Release_Semaphore(&m_readSemaphore);

			} // end if NU_Select
			// we must have just come here because of a disconnect
			else if (status == NU_NO_DATA)
			{

				// check to make sure all the TCP connections we think are open
				// actually still are
				for (int i=0; i<m_iConnectionSize;i++)
				{
					if (m_aConnection[i].m_bConnected == TRUE)
					{
						if (NU_Is_Connected(m_aConnection[i].m_sock) == NU_FALSE)
						{
							TRACEF(TRACE_L5, ("Disconnect detected (%d)\n", m_aConnection[i].m_iConnectionID));
							m_aConnection[i].m_bConnected = FALSE;
							Signal(HANDLE_ERROR,&m_aConnection[i]);										
						}
					}
				}
			
			}
			else
			{
				TRACEF(TRACE_L4, ("DdmNet internal error! (%s line %d)\n", __FILE__, __LINE__));
				NU_Suspend_Task(NU_Current_Task_Pointer());
			}
		} // end while(1)
	} // end if NU_Socket
	// could not create signal socket(s) - must be out of free sockets?
	else
	{
		TRACEF(TRACE_L4, ("Could not create signal sockets\n"));
	}
	
	
}
/*	end DdmNet::DoReadWrite  *****************************************************/

//	DdmNet::HandleWritten(nSignal, *pParam)
//
//	Description:
//		Tell the client that the write was a success

STATUS DdmNet::HandleWritten(SIGNALCODE nSignal, void* pParam) {

	// remove compilation warnings
	nSignal;

	// cast the message to reply
	NetMsgWrite* pMsg = (NetMsgWrite*)pParam;

	//tell client that message was written successfully
	Reply(pMsg,OK);
	return OK;

}
/*	end DdmNet::HandleWritten  *****************************************************/

//	DdmNet::ProcessWrite(*pMsgReq)
//
//	Description:
//		Processes a request for a write by placing it on the write queue

STATUS DdmNet::ProcessWrite(Message *pMsgReq) {

	TRACEF(TRACE_L8, ("DdmNet::ProcessWrite()\n"));

	NetMsgWrite* pMsg = (NetMsgWrite*)pMsgReq;

#ifdef ULTRATRACE
	char* pBuf;
	int iSize;
	pMsg->GetBuffer(pBuf,iSize);
	TRACEF(TRACE_L8, ("  Got %d bytes\n", iSize));
	TRACEF(TRACE_L8, ("  Buffer contains ["));
	for (int i=0;i<iSize;i++)
	  TRACEF(TRACE_L8, ("%c", pBuf[i]));
	TRACEF(TRACE_L8, ("]\n"));
#endif ULTRATRACE

	//place in the write que
	Kernel::Obtain_Semaphore(&m_readSemaphore,CT_SUSPEND);
	Connection* pConnection = GetConnection(pMsg->m_iConnectionID);
	if(pConnection) {
		QueMsg(pConnection->m_apWriteMsgs,&pConnection->m_iWriteMsgs,pMsg);
		// Signal NU_Select to perform writes		
		Kernel::Release_Semaphore(&m_readSemaphore);		
		SetSignalSocket(writeSignalSocket);
		Kernel::Obtain_Semaphore(&m_readSemaphore,CT_SUSPEND);		
	}
	else {
		TRACEF(TRACE_L4, ("Couldn't find connection with id = %d\n",pMsg->m_iConnectionID));
		Reply(pMsg, !OK);					
	}
	Kernel::Release_Semaphore(&m_readSemaphore);

	return OK;

}
/*	end DdmNet::ProcessWrite  *****************************************************/

//	DdmNet::HandleRead(nSignal, *pParam)
//
//	Description:
//		Handle a signal that data has been read

STATUS DdmNet::HandleRead(SIGNALCODE nSignal,void* pParam) {

	// remove compilation warnings
	nSignal;

	NetMsgRead* pMsg = (NetMsgRead*)pParam;

	TRACEF(TRACE_L8, ("DdmNet::HandleRead()\n"));

	//respond
	Reply(pMsg, OK, false);

	return OK;
}

//	DdmNet::ProcessRead(*pMsgReq)
//
//	Description:
//		Processes a request for a read by placing the message in the read
//		request variable for the connection

STATUS DdmNet::ProcessRead(Message *pMsgReq) {

	NetMsgRead* pReadMsg = (NetMsgRead*)pMsgReq;
	
	Kernel::Obtain_Semaphore(&m_readSemaphore, CT_SUSPEND);
	Connection* pConnection = GetConnection(pReadMsg->m_iConnectionID);

	if(pConnection) {
		pConnection->m_pReadRequest = pReadMsg;
		// Repack the bitmap with the new read request
		TRACEF(TRACE_L8, ("New read request\n"));
		Kernel::Release_Semaphore(&m_readSemaphore);
		SetSignalSocket(reloadSignalSocket);
		Kernel::Obtain_Semaphore(&m_readSemaphore, CT_SUSPEND);		
	}
	else {
		TRACEF(TRACE_L4, ("Read request for non-existent socket\n"));
	}

	Kernel::Release_Semaphore(&m_readSemaphore);
	
	return OK;
}

//	DdmNet::HandleError(nSignal, *pParam)
//
//	Description:
//		Handle a signal that there was a network error

STATUS DdmNet::HandleError(SIGNALCODE nSignal, void* pParam) {

	// remove compilation warnings
	nSignal;

	Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);
	Kernel::Obtain_Semaphore(&m_readSemaphore, CT_SUSPEND);	
	Connection *pConnection = (Connection *)pParam;
	
	TRACEF(TRACE_L8, ("Handling Error (conn %d)\n", pConnection->m_iConnectionID));	

	if(pConnection->m_pReadRequest) {
		TRACEF(TRACE_L8, ("HE : Removing Read\n"));
		Reply(pConnection->m_pReadRequest, !OK);
		pConnection->m_pReadRequest = NULL;		
	}

	NetMsgWrite* pMsgWrite = (NetMsgWrite*)PeekMsg(pConnection->m_apWriteMsgs, pConnection->m_iWriteMsgs);
	while (pMsgWrite)
	{
		TRACEF(TRACE_L8, ("HE : Removing Write\n"));	
		Reply(pMsgWrite, !OK);			
		EnqueMsg(pConnection->m_apWriteMsgs,&pConnection->m_iWriteMsgs);
		pMsgWrite = (NetMsgWrite*)PeekMsg(pConnection->m_apWriteMsgs, pConnection->m_iWriteMsgs);		
	}
	
	NU_Close_Socket(pConnection->m_sock);

	RemoveConnection(pConnection->m_iConnectionID);

	Kernel::Release_Semaphore(&m_readSemaphore);
	Kernel::Release_Semaphore(&m_ListenSemaphore);

//	delete pConnection;

	return OK;

}

//ProcessConnect
STATUS DdmNet::ProcessConnect(Message *pMsgReq) {

	//not needed, just let it chill for now
	TRACEF(TRACE_L8, ("DdmNet::ProcessConnect\n"));

	NetMsgConnect* pMsg = (NetMsgConnect*)pMsgReq;

	pMsg->SetConnectionID(++iConnectionID);

	Reply(pMsg, OK);
	return OK;
}

//	DdmNet::ProcessKill(*pMsgReq)
//
//	Description:
//		Processes a request to kill a connection being managed by DdmNet by doing the following:
//			* removing outstanding read and write requests
//			* close the socket
//			* removing the connection from the management list
//		This means that the client MUST NOT issue the kill command until he is sure that all
//		of his outstanding writes have been completed and responded to

STATUS DdmNet::ProcessKill(Message *pMsgReq) {
	NetMsgKill* pMsg = (NetMsgKill*)pMsgReq;

	TRACEF(TRACE_L8, ("DdmNet::ProcessKill\n"));
	TRACEF(TRACE_L4, ("ProcessKill received kill for connection %d\n", pMsg->m_iConnectionID));

	Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);
	Kernel::Obtain_Semaphore(&m_readSemaphore, CT_SUSPEND);	

	Connection* pConnection = GetConnection(pMsg->m_iConnectionID);
	
	if(pConnection->m_pReadRequest) {
		TRACEF(TRACE_L8, ("ProcessKill : Removing Read\n"));
		Reply(pConnection->m_pReadRequest, !OK);
		pConnection->m_pReadRequest = NULL;		
	}

	NetMsgWrite* pMsgWrite = (NetMsgWrite*)PeekMsg(pConnection->m_apWriteMsgs, pConnection->m_iWriteMsgs);
	while (pMsgWrite)
	{
		TRACEF(TRACE_L8, ("ProcessKill : Removing Write\n"));	
		Reply(pMsgWrite, !OK);			
		EnqueMsg(pConnection->m_apWriteMsgs,&pConnection->m_iWriteMsgs);
		pMsgWrite = (NetMsgWrite*)PeekMsg(pConnection->m_apWriteMsgs, pConnection->m_iWriteMsgs);		
	}
	
	NU_Close_Socket(pConnection->m_sock);

	RemoveConnection(pConnection->m_iConnectionID);

	Kernel::Release_Semaphore(&m_readSemaphore);
	Kernel::Release_Semaphore(&m_ListenSemaphore);

	Reply(pMsg, OK);
	return OK;
}

//	DdmNet::ProcessChangeIP(*pMsgReq)
//
//	Description:
//		Processes a request to change the IP address by doing the following:
//			* removing outstanding read and write requests
//			* kill all currently open connections,
//			* changing outstanding listen request to listen on the new ethernet	IP
//			* changing the IP

STATUS DdmNet::ProcessChangeIP(Message *pMsgReq) {

	STATUS status;

	TRACEF(TRACE_L8, ("DdmNet::ProcessChangeIP()\n"));

	Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);
	Kernel::Obtain_Semaphore(&m_readSemaphore, CT_SUSPEND);

	NetMsgChangeIP* pMsg = (NetMsgChangeIP *)pMsgReq;
	
	// The socket server does NOT have to reissue the listen request, so the listen
	// threads have to listen on the new IP now!
//	NetMsgListen* pMsg = (NetMsgListen*)(PeekMsg(m_apListenMsgs, m_iListenMsgs));

	// Remove all the outstanding read and write requests since they won't be valid anymore with all
	// the connections gone
	// Hmm.. It might automagically reload without being forced when those read sockets fail
	// in the NU_Select - this would cause logged errors possibly, however.. hmmm...
	for (int i=0; i < m_iConnectionSize; i++)
	{
		if(m_aConnection[i].m_pReadRequest) {
			TRACEF(TRACE_L8, ("CIp : Bad Read\n"));
			Reply(m_aConnection[i].m_pReadRequest, !OK);
			m_aConnection[i].m_pReadRequest = NULL;		
		}
		
		NetMsgWrite* pMsgWrite = (NetMsgWrite*)PeekMsg(m_aConnection[i].m_apWriteMsgs,m_aConnection[i].m_iWriteMsgs);
		while (pMsgWrite)
		{
			TRACEF(TRACE_L8, ("CIp : Bad Read - Removing\n"));
			Reply(pMsgWrite, !OK);			
			EnqueMsg(m_aConnection[i].m_apWriteMsgs,&m_aConnection[i].m_iWriteMsgs);
			pMsgWrite = (NetMsgWrite*)PeekMsg(m_aConnection[i].m_apWriteMsgs, m_aConnection[i].m_iWriteMsgs);		
		}		
	}

	// Kill each open socket and connection
	while (m_iConnectionSize > 0)
	{
		status = NU_Abort(m_aConnection[0].m_sock);
		if (status != NU_SUCCESS)
			ReportError(status, __FILE__, __LINE__);
		RemoveConnection(m_aConnection[0].m_iConnectionID);
	}

	// Change the IP address of the ethernet interface
	// Leave the subnet if a null subnet was provided
	Network::FillIP(ip_ethernet, pMsg->m_acIPAddress[0], pMsg->m_acIPAddress[1], pMsg->m_acIPAddress[2], pMsg->m_acIPAddress[3]);
	if (pMsg->m_acSubnet[0] != 0)
		Network::FillIP(subnet_ethernet, pMsg->m_acSubnet[0], pMsg->m_acSubnet[1], pMsg->m_acSubnet[2], pMsg->m_acSubnet[3]);

	Network::IPChange(0, ip_ethernet, subnet_ethernet);

	Kernel::Release_Semaphore(&m_readSemaphore);
	Kernel::Release_Semaphore(&m_ListenSemaphore);

	Reply(pMsg, OK);
	return OK;

}
/*	end DdmNet::ProcessChangeIP  *****************************************************/

//	DdmNet::ProcessChangeGateway(*pMsgReq)
//
//	Description:
//		Processes a request to change the gateway by calling the Network class function

STATUS DdmNet::ProcessChangeGateway(Message *pMsgReq) {

	TRACEF(TRACE_L8, ("DdmNet::ProcessChangeGateway()\n"));

//	Need to semaphore protect!!
	NetMsgChangeGateway* pMsg = (NetMsgChangeGateway *)pMsgReq;

	Network::FillIP(ip_gateway, pMsg->m_acIPAddress[0], pMsg->m_acIPAddress[1], pMsg->m_acIPAddress[2], pMsg->m_acIPAddress[3]);
	Network::GatewayChange(0, ip_gateway);

	Reply(pMsg, OK);
	return OK;

}
/*	end DdmNet::ProcessChangeGateway  *****************************************************/

// ReportError
//  Codes below are correct
//  With socket/status, file, and line number
BOOL ReportError(int iRet, int socketd, char *file, int line) {

	TRACEF(TRACE_L4, ("Error [%s line %d] : Socket %d : ", file, line, socketd));
	return ReportError(iRet);

}

// ReportError
//  With file and line number
BOOL ReportError(int iRet, char *file, int line) {

	TRACEF(TRACE_L4, ("Error [%s line %d] : ", file, line));
	return ReportError(iRet);

}

// ReportError
//  Codes below are correct
//  Yet to be handled
BOOL ReportError(int iRet) {
	BOOL bSaveSock = FALSE;

	switch (iRet) {
	case NU_ARP_FAILED:
		/*  ARP failed to resolve addr. */
		TRACEF(TRACE_L4, ("ARP resolution failed.\n"));
		break;
	case NU_INVALID_PROTOCOL:
		/*  Invalid network protocol */
		TRACEF(TRACE_L4, ("Invalid network protocol.\n"));
		break;
	case NU_NO_DATA_TRANSFER:
	     /*  Data was not written/read during send/receive function */
	     TRACEF(TRACE_L4, ("Data not written during transfer.\n"));
	     break;
	case NU_NO_PORT_NUMBER:
	     /*  No local port number was stored in the socket descriptor */
	     TRACEF(TRACE_L4, ("No local port number stored in socket descriptor.\n"));
	     break;
	case NU_NO_TASK_MATCH:
	     /*  No task/port number combination existed in the task table */
	     TRACEF(TRACE_L4, ("No task/port number combo exists in task table.\n"));
	     break;
	case NU_NO_SOCKET_SPACE:
	     /*  The socket structure list was full when a new socket descriptor was requested */
	     TRACEF(TRACE_L4, ("No Socket Space - Socket structure list full on request.\n"));
	     break;
	case NU_NO_ACTION:
	     /*  No action was processed by the function */
	     TRACEF(TRACE_L4, ("No action processed.\n"));
	     break;
	case NU_NOT_CONNECTED:
	     /*  A connection has been closed by the network.  */
	     TRACEF(TRACE_L4, ("Not connected - connection has been closed by network.\n"));
    	 break;
	case NU_INVALID_SOCKET:
    	 /*  The socket ID passed in was not in a valid range.  */
	     TRACEF(TRACE_L4, ("Invalid socket ID.\n"));
    	 break;
	case NU_NO_SOCK_MEMORY:
    	 /*  Memory allocation failed for internal sockets structure.  */
	     TRACEF(TRACE_L4, ("Socket structure allocation failed.\n"));
    	 break;
	case NU_NOT_A_TASK:
    	 /*  Attempt was made to make a sockets call from an interrupt without doing context save.  */
	     TRACEF(TRACE_L4, ("Not a task - sockets call without context save.\n"));
    	 break;
	case NU_INVALID_ADDRESS:
    	 /*  An incomplete address was sent */
	     TRACEF(TRACE_L4, ("Invalid address sent.\n"));
    	 break;
	case NU_NO_HOST_NAME:
    	 /*  No host name specified in a  */
	     TRACEF(TRACE_L4, ("No host name specified.\n"));
    	 break;
	case NU_RARP_INIT_FAILED:
    	 /*  During initialization RARP failed. */
	     TRACEF(TRACE_L4, ("RARP Init failed.\n"));
    	 break;
	case NU_BOOTP_INIT_FAILED:
    	 /*  During initialization BOOTP failed. */
	     TRACEF(TRACE_L4, ("BOOTP init failed.\n"));
    	 break;
	case NU_INVALID_PORT:
    	 /*  The port number passed in was not in a valid range. */
	     TRACEF(TRACE_L4, ("Invalid port number.\n"));
    	 break;
	case NU_NO_BUFFERS:
    	 /*  There were no buffers to place the outgoing packet in. */
	     TRACEF(TRACE_L4, ("No buffers available.\n"));
    	 break;
	case NU_NOT_ESTAB:
    	 /*  A connection is open but not in an established state. */
	     TRACEF(TRACE_L4, ("Connection open but not established.\n"));
    	 break;
	case NU_INVALID_BUF_PTR:
    	 /*  The buffer pointer is invalid */
	     TRACEF(TRACE_L4, ("Invalid buffer pointer"));
    	 break;
	case NU_WINDOW_FULL:
 	    /*  The foreign host's in window is full. */
		 TRACEF(TRACE_L4, ("Foreign host window full.\n"));
		 break;
	case NU_NO_SOCKETS:
	     /*  No sockets were specified. */
	     TRACEF(TRACE_L4, ("No sockets specified.\n"));
	     break;
	case NU_NO_DATA:
 	    /*  None of the specified sockets were data ready.  NU_Select. */
 	    TRACEF(TRACE_L4, ("None of the specified sockets were ready.\n"));
 	    break;

	/* The following errors are reported by the NU_Setsockopt and NU_Getsockopt
	   service calls. */
	case NU_INVALID_LEVEL:
	    /*  The specified level is invalid. */
 	    TRACEF(TRACE_L4, ("Socket option: invalid level.\n"));
 	    break;
	case NU_INVALID_OPTION:
    	 /*  The specified option is invalid. */
	     TRACEF(TRACE_L4, ("Socket option: invalid option.\n"));
    	 break;
	case NU_INVAL:
    	 /*  General purpose error condition. */
	     TRACEF(TRACE_L4, ("Socket option: error.\n"));
    	 break;
	case NU_ACCESS:
    	 /*  The attempted operation is not allowed on the socket */
	     TRACEF(TRACE_L4, ("Socket option: operation not allowed.\n"));
    	 break;
	case NU_ADDRINUSE:
		 TRACEF(TRACE_L4, ("Socket option: address in use.\n"));
		 break;
	case NU_HOST_UNREACHABLE:
	     /*  Host unreachable */
    	 TRACEF(TRACE_L4, ("Socket option: host unreachable.\n"));
	     break;
	case NU_MSGSIZE:
    	 /*  Packet is too large for interface. */
	     TRACEF(TRACE_L4, ("Socket option: packet too large"));
    	 break;
	case NU_NOBUFS:
    	 /*  Could not allocate a memory buffer. */
	     TRACEF(TRACE_L4, ("Socket option: could not allocate buffer.\n"));
    	 break;
	case NU_UNRESOLVED_ADDR:
    	 /*  The MAC address was not resolved.*/
	     TRACEF(TRACE_L4, ("Socket option: unresolved MAC address.\n"));
    	 break;
	case NU_CLOSING:
    	 /*  The other side in a TCP connection has sent a FIN */
	     TRACEF(TRACE_L4, ("Socket option: foreign connection sent FIN.\n"));
    	 break;
	case NU_MEM_ALLOC:
    	 /* Failed to allocate memory. */
	     TRACEF(TRACE_L4, ("Socket option: memory allocation failure.\n"));
    	 break;
	case NU_RESET:
		 TRACEF(TRACE_L4, ("Socket option: reset.\n"));
		 break;

	/* These error codes are returned by DNS. */
	case NU_INVALID_LABEL:
	     /* Indicates a domain name with an invalid label. */
    	 TRACEF(TRACE_L4, ("DNS: Invalid domain label.\n"));
	     break;
	case NU_FAILED_QUERY:
    	 /* No response received for a DNS Query. */
	     TRACEF(TRACE_L4, ("DNS: No response to query.\n"));
    	 break;
	case NU_DNS_ERROR:
    	 /* A general DNS error status. */
	     TRACEF(TRACE_L4, ("DNS: Error.\n"));
    	 break;
	case NU_NOT_A_HOST:
    	 /* The host name was not found. */
	     TRACEF(TRACE_L4, ("DNS: Host not found.\n"));
    	 break;
	case NU_INVALID_PARM:
    	 /*  A parameter has an invalid value. */
	     TRACEF(TRACE_L4, ("DNS: Invalid parameter.\n"));
    	 break;
	case NU_NO_IP:
    	 /*  An IP address was not specified for the device. */
	     TRACEF(TRACE_L4, ("DNS: No IP specified.\n"));
    	 break;
	case NU_DHCP_INIT_FAILED:
    	 /*  During initialization DHCP failed. */
	     TRACEF(TRACE_L4, ("DHCP Init failure.\n"));
    	 break;
    
    } 
        
	return bSaveSock;
}

