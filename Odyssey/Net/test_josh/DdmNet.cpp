// Set below options according to hardware desired
#define DECETHER
//#define PPP
//#define INCLUDE_ODYSSEY

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif

#include "BuildSys.h"
#include "RqOsTimer.h"
#include "DdmNet.h"
#include "OsTypes.h"

// Network Includes
#if 0
#include "externs.h"
#include "nucleus.h"
#include "socketd.h"
#include "tcpdefs.h"
#endif

#include "Message.h"

#ifdef PPP
#include "ppp.h"
#endif

#ifdef DECETHER
#include "dec21143.h"
#endif

#define HANDLE_ACCEPT 10
#define HANDLE_READ 11
#define HANDLE_ERROR 12
#define HANDLE_WRITTEN 13

#define NO_FLAGS_SET 0
	
CLASSNAME (DdmNet, SINGLE);  // Class Link Name used by Buildsys.cpp --
                              // must match CLASSENTRY() name formal (4th arg)
                              // in buildsys.cpp

//  statically declare which request codes our DDM will serve:
//  (we declare ours as usable for board-local comms only)
//  *** These defs must match those in Initialize(), below ***
SERVELOCAL (DdmNet, REQ_NET_LISTEN);
SERVELOCAL (DdmNet, REQ_NET_CONNECT);
SERVELOCAL (DdmNet, REQ_NET_KILL);
SERVELOCAL (DdmNet, REQ_NET_READ);
SERVELOCAL (DdmNet, REQ_NET_WRITE);

#if 0
NU_MEMORY_POOL System_Memory;

// External init functions
extern "C" void	init_galileo();
extern "C" STATUS Init_Hardware();
#endif

DdmNet::DdmNet (DID did) : Ddm (did) {

   Tracef("DdmNet::DdmNet()\n");
   
//	rwStack	= new(tZERO) U8[8000];
// 	lStack	= new(tZERO) U8[8000];
   
   m_iConnectionSize = 0;
   m_aConnection = NULL;
   
   m_iListenMsgs = 0;
   m_apListenMsgs = NULL;

}

Ddm *DdmNet::Ctor (DID did) {
   Tracef("DdmNet::Ctor()\n");
   return (new DdmNet (did));
} 

STATUS DdmNet::Initialize (Message *pMsg) {

	Tracef("DdmNet::Initialize()\n");

	DispatchRequest(REQ_NET_LISTEN, REQUESTCALLBACK (DdmNet, ProcessListen));
	DispatchRequest(REQ_NET_CONNECT, REQUESTCALLBACK (DdmNet, ProcessConnect));
	DispatchRequest(REQ_NET_KILL, REQUESTCALLBACK (DdmNet, ProcessKill));
	DispatchRequest(REQ_NET_WRITE, REQUESTCALLBACK (DdmNet, ProcessWrite));
	DispatchRequest(REQ_NET_READ, REQUESTCALLBACK (DdmNet, ProcessRead));

	DispatchSignal(HANDLE_ACCEPT, SIGNALCALLBACK (DdmNet, HandleAccept));
	DispatchSignal(HANDLE_READ, SIGNALCALLBACK (DdmNet, HandleRead));
	DispatchSignal(HANDLE_WRITTEN, SIGNALCALLBACK (DdmNet, HandleWritten));

	Kernel::Create_Semaphore(&m_ListenSemaphore, "listen_semaphore", 1);
	Kernel::Create_Semaphore(&timerSemaphore, "timeout_semaphore", 1);

	Reply(pMsg,OK);
	return OK;
		
}

void ReadWriteThread(void* pParam) {
	Tracef("ReadWriteThread launching DoReadWrite()\n");
	((DdmNet*)pParam)->DoReadWrite();
}

STATUS DdmNet::Quiesce (Message *pMsg) { 
	Tracef("DdmNet::Quiesce()\n");
	Reply (pMsg, OK);
	return (OK); 
} 

int iTestCount = 0;

STATUS DdmNet::Enable(Message *pMsgReq) {

	Tracef("DdmNet::Enable()\n");

	Reply(pMsgReq,OK);
	return OK;
}

int iConnectionID = 0;

Connection* DdmNet::GetConnection(int iConnectionID) {
	for(int i=0;i<m_iConnectionSize; i++) {
		if(m_aConnection[i].m_iConnectionID == iConnectionID)
			return &(m_aConnection[i]);
	}
	return NULL;
	
}

STATUS DdmNet::HandleAccept(SIGNALCODE nSignal,void* pParam) {

	Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);
	
	//grab the accepted socket from the que
	NetMsgListen* pMsg = (NetMsgListen*)pParam;

	Kernel::Release_Semaphore(&m_ListenSemaphore);

	Tracef("New Connection...");

	//respond to the listen message
	Reply(pMsg, OK, FALSE);

	return OK;

}

void ListenThread(void* pParam) {
	Tracef("Listenthread()\n");
	((DdmNet *)pParam)->DoListen();
}

STATUS DdmNet::ProcessListen(Message *pMsgReq) {

	Tracef("ProcessListen()\n");
	
	NetMsgListen* pMsg = (NetMsgListen*)pMsgReq;
	
	//form new connection
	pMsg->SetConnectionID(++iConnectionID);

	//associate the socket with the connection ID
	Connection* aConn = new Connection[m_iConnectionSize + 1];
	if(m_aConnection) {
		memcpy(aConn, m_aConnection, m_iConnectionSize);
		delete[] m_aConnection;
	}
	m_aConnection = aConn;
	m_aConnection[m_iConnectionSize].m_iConnectionID = iConnectionID;
	m_aConnection[m_iConnectionSize++].m_sock = 0;
	
	//accept it
	HandleAccept(0, pMsg);
	
	return OK;
}

STATUS DdmNet::HandleWritten(SIGNALCODE nSignal, void* pParam) {
	//grab the message
	NetMsgWrite* pMsg = (NetMsgWrite*)pParam;
	Tracef("DdmNet::HandleWritten()\n");

	//tell client that message was written successfully
	Reply(pMsg,OK);
	return OK;
}

STATUS DdmNet::ProcessWrite(Message *pMsgReq) {

	NetMsgWrite* pMsgWrite = (NetMsgWrite*)pMsgReq;

	//examine contents
	

	//call HandleWritten()

	return OK;


}

STATUS DdmNet::HandleRead(SIGNALCODE nSignal,void* pParam) {
	//grab the accepted socket from the que
	NetMsgRead* pMsg = (NetMsgRead*)pParam;

	Tracef("DdmNet::HandleRead()\n");

	//respond
	Reply(pMsg, OK, false);

	return OK;
}

//ProcessRead
STATUS DdmNet::ProcessRead(Message *pMsgReq) {

	NetMsgRead* pReadMsg = (NetMsgRead*)pMsgReq;
	
	Tracef("DdmNet::ProcessRead\n");

	//pretend like we read some stuff

	





}





























void DdmNet::DoReadWrite() {

	STATUS status;

	while(1) {
		BOOL bInactive = TRUE;
		Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);
		//do reads
		int i;
//		Tracef("rw\n");
		for(i=0;i<m_iConnectionSize;i++) {
			if(m_aConnection[i].m_pReadRequest != NULL) {
				int iRet;
				char pBufRead[100];
				U32 lAvailBytes;

				// turn off blocking for read
				iRet = NU_Fcntl(m_aConnection[i].m_sock, NU_SETFLAG, NU_FALSE);
//				iRet = ioctlsocket(m_aConnection[i].m_sock,FIONREAD,&lAvailBytes);
				if(iRet < 0) {
					m_aConnection[i].m_bConnected = FALSE;
					HandleError(iRet);
					continue;
				}

       			lAvailBytes = NU_Recv(m_aConnection[i].m_sock, pBufRead, 100, 0);
		        if (lAvailBytes == NU_NOT_CONNECTED)
		        {
					m_aConnection[i].m_bConnected = FALSE;
		            continue;
       		    }
	            else if (lAvailBytes < 0)
		        {
        			HandleError(lAvailBytes);
       	   		}

			    else if (lAvailBytes > 0) {
		
					// we have some new data in pBufRead, of size lAvailBytes
					bInactive = FALSE;
					NetMsgRead* pMsg = new NetMsgRead(m_aConnection[i].m_pReadRequest);
					pMsg->SetBuffer(pBufRead,lAvailBytes);
					Tracef("buffered read (%d bytes)\n", lAvailBytes);
					Signal(HANDLE_READ,pMsg);							
				}
			}
		}

		Kernel::Release_Semaphore(&m_ListenSemaphore);

		Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);

		//do writes
		for(i=0;i<m_iConnectionSize;i++) {
			NetMsgWrite* pMsgWrite = (NetMsgWrite*)PeekMsg(m_aConnection[i].m_apWriteMsgs, m_aConnection[i].m_iWriteMsgs);
			if(pMsgWrite) {

				int iSize,iRet;
				char* pBuf;
				pMsgWrite->GetBuffer(pBuf,iSize);
				
				Tracef("  Got %d bytes\n", iSize);
				Tracef("  Buffer contains [");
				for (int i=0;i<iSize;i++)
				  Tracef("%c", pBuf[i]);
				Tracef("]\n");
				
				BOOL bDone = FALSE;
				
				while(pMsgWrite->m_iProcessPos < iSize && !bDone) {
				
					iRet = NU_Send(m_aConnection[i].m_sock, pBuf + pMsgWrite->m_iProcessPos, iSize - pMsgWrite->m_iProcessPos, 0);
					if (iRet < 0) {
						if( !HandleError(iRet)) {
							m_aConnection[i].m_bConnected = FALSE;
						}
						bDone = TRUE;
						break;//would block
					}
					else {
						if(iRet) {
							pMsgWrite->m_iProcessPos += iRet;
							if(pMsgWrite->m_iProcessPos == iSize) {
								EnqueMsg(m_aConnection[i].m_apWriteMsgs,m_aConnection[i].m_iWriteMsgs);
								Signal(HANDLE_WRITTEN,pMsgWrite);	
							}
						}
						else { // Clean socket exit
							m_aConnection[i].m_bConnected = FALSE;
							break;
						}
					}				
				}  //end while
			}  //end if
		}  //end for

		Kernel::Release_Semaphore(&m_ListenSemaphore);

		if(bInactive) {
			status = NU_Obtain_Semaphore(&timerSemaphore, 10);		
//			NU_Sleep(10);
		}
	}
}


void DdmNet::DoListen() {

	Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);
	NetMsgListen* pMsg = (NetMsgListen*)(EnqueMsg(m_apListenMsgs, m_iListenMsgs));
	Kernel::Release_Semaphore(&m_ListenSemaphore);

	struct 			addr_struct	serverAddr;
	int				osocketd, nsocketd;
	struct 			addr_struct	clientAddr;	
	STATUS			status;

	Tracef("DoListen()\n");
	
	// open a connection via the socket interface
	if ((osocketd = NU_Socket(NU_FAMILY_IP, NU_TYPE_STREAM, 0)) >= 0)
	{
	
	// fill in the address structure
	serverAddr.family = NU_FAMILY_IP;
	serverAddr.port = htons(pMsg->m_iPort);
	memcpy(serverAddr.id.is_ip_addrs, &(pMsg->m_acIPAddress), 4);	
	
	// make an NU_Bind() call to bind the server's address
	if ((NU_Bind((int16)osocketd, &serverAddr, 0))>=0)
	{
	    // be ready to accept connection requests
	    status = NU_Listen((int16)osocketd, 10);

	    if (status == NU_SUCCESS)
	    {
			// successful listen request, check for connections

			// set to non-blocking
			NU_Fcntl(osocketd, NU_SETFLAG, NU_FALSE);

			// keep trying for a connection
			while (1) {
	    		nsocketd = NU_Accept((int16)osocketd, &clientAddr, 0);
//	    		Tracef("l\n");
			    if (nsocketd >= 0)
			    {
    			
    				Tracef("Connection accepted\n");
    			
    				Kernel::Obtain_Semaphore(&m_ListenSemaphore, CT_SUSPEND);

					//place in the accept que
					pMsg->SetConnectionID(++iConnectionID);

					//associate the socket with the connection ID
					Connection* aConn = new Connection[m_iConnectionSize + 1];
					if(m_aConnection) {
						memcpy(aConn, m_aConnection, m_iConnectionSize);
						delete[] m_aConnection;
					}
					m_aConnection = aConn;
					m_aConnection[m_iConnectionSize].m_iConnectionID = iConnectionID;
					m_aConnection[m_iConnectionSize++].m_sock = nsocketd;

					Kernel::Release_Semaphore(&m_ListenSemaphore);
		
					Signal(HANDLE_ACCEPT,pMsg);

				}									// end of successful NU_Accept
				// no connection ready at this time
				else if (nsocketd == NU_NO_TASK_MATCH)
				{
					// Nap time, try again later
//					NU_Sleep(50);
					status = NU_Obtain_Semaphore(&timerSemaphore, 50);
					continue;
				}
				// error code
				else
				{
					Tracef("listen error - should break out\n");
					HandleError(nsocketd);
					continue;
				}
				
			}									// end while (1) Accept loop

	    }                                       // end successful NU_Listen
	}                                           // end successful NU_Bind

    }                                               // end successful NU_Socket
	
}


















//ProcessConnect
STATUS DdmNet::ProcessConnect(Message *pMsgReq) {

	//not needed, just let it chill for now
	Tracef("DdmNet::ProcessConnect\n");

	NetMsgConnect* pMsg = (NetMsgConnect*)pMsgReq;

	pMsg->SetConnectionID(++iConnectionID);

	Reply(pMsg, OK);
	return OK;
}

//ProcessKill
STATUS DdmNet::ProcessKill(Message *pMsgReq) {
	NetMsgKill* pMsg = (NetMsgKill*)pMsgReq;

	Tracef("DdmNet::ProcessKill\n");

	// I'm not sure how we will use this, just let it be for now

	Reply(pMsg, OK);
	return OK;
}

BOOL HandleError(int iRet) {
	BOOL bSaveSock = FALSE;
	return FALSE;
#if 0
	
	switch (iRet) {
case NU_ARP_FAILED:
	/*  ARP failed to resolve addr. */
	Tracef("ARP resolution failed.\n");
	break;
case NU_INVALID_PROTOCOL:
	/*  Invalid network protocol */
	Tracef("Invalid network protocol.\n");
	break;
case NU_NO_DATA_TRANSFER:
     /*  Data was not written/read during send/receive function */
     Tracef("Data not written during transfer.\n");
     break;
case NU_NO_PORT_NUMBER:
     /*  No local port number was stored in the socket descriptor */
     Tracef("No local port number stored in socket descriptor.\n");
     break;
case NU_NO_TASK_MATCH:
     /*  No task/port number combination existed in the task table */
     Tracef("No task/port number combo exists in task table.\n");
     break;
case NU_NO_SOCKET_SPACE:
     /*  The socket structure list was full when a new socket descriptor was requested */
     Tracef("No Socket Space - Socket structure list full on request.\n");
     break;
case NU_NO_ACTION:
     /*  No action was processed by the function */
     Tracef("No action processed.\n");
     break;
case NU_NOT_CONNECTED:
     /*  A connection has been closed by the network.  */
     Tracef("Not connected - connection has been closed by network.\n");
     break;
case NU_INVALID_SOCKET:
     /*  The socket ID passed in was not in a valid range.  */
     Tracef("Invalid socket ID.\n");
     break;
case NU_NO_SOCK_MEMORY:
     /*  Memory allocation failed for internal sockets structure.  */
     Tracef("Socket structure allocation failed.\n");
     break;
case NU_NOT_A_TASK:
     /*  Attempt was made to make a sockets call from an interrupt without doing context save.  */
     Tracef("Not a task - sockets call without context save.\n");
     break;
case NU_INVALID_ADDRESS:
     /*  An incomplete address was sent */
     Tracef("Invalid address sent.\n");
     break;
case NU_NO_HOST_NAME:
     /*  No host name specified in a  */
     Tracef("No host name specified.\n");
     break;
case NU_RARP_INIT_FAILED:
     /*  During initialization RARP failed. */
     Tracef("RARP Init failed.\n");
     break;
case NU_BOOTP_INIT_FAILED:
     /*  During initialization BOOTP failed. */
     Tracef("BOOTP init failed.\n");
     break;
case NU_INVALID_PORT:
     /*  The port number passed in was not in a valid range. */
     Tracef("Invalid port number.\n");
     break;
case NU_NO_BUFFERS:
     /*  There were no buffers to place the outgoing packet in. */
     Tracef("No buffers available.\n");
     break;
case NU_NOT_ESTAB:
     /*  A connection is open but not in an established state. */
     Tracef("Connection open but not established.\n");
     break;
case NU_INVALID_BUF_PTR:
     /*  The buffer pointer is invalid */
     Tracef("Invalid buffer pointer\n");
     break;
case NU_WINDOW_FULL:
     /*  The foreign host's in window is full. */
     Tracef("Foreign host window full.\n");
     break;
case NU_NO_SOCKETS:
     /*  No sockets were specified. */
     Tracef("No sockets specified.\n");
     break;
case NU_NO_DATA:
     /*  None of the specified sockets were data ready.  NU_Select. */
     Tracef("None of the specified sockets were ready.\n");
     break;

/* The following errors are reported by the NU_Setsockopt and NU_Getsockopt
   service calls. */
case NU_INVALID_LEVEL:
     /*  The specified level is invalid. */
     Tracef("Socket option: invalid level.\n");
     break;
case NU_INVALID_OPTION:
     /*  The specified option is invalid. */
     Tracef("Socket option: invalid option.\n");
     break;
case NU_INVAL:
     /*  General purpose error condition. */
     Tracef("Socket option: error.\n");
     break;
case NU_ACCESS:
     /*  The attempted operation is not allowed on the  socket */
     Tracef("Socket option: operation not allowed.\n");
     break;
case NU_ADDRINUSE:
	 Tracef("Socket option: address in use.\n");
	 break;
case NU_HOST_UNREACHABLE:
     /*  Host unreachable */
     Tracef("Socket option: host unreachable.\n");
     break;
case NU_MSGSIZE:
     /*  Packet is too large for interface. */
     Tracef("Socket option: packet too large\n");
     break;
case NU_NOBUFS:
     /*  Could not allocate a memory buffer. */
     Tracef("Socket option: could not allocate buffer.\n");
     break;
case NU_UNRESOLVED_ADDR:
     /*  The MAC address was not resolved.*/
     Tracef("Socket option: unresolved MAC address.\n");
     break;
case NU_CLOSING:
     /*  The other side in a TCP connection has sent a FIN */
     Tracef("Socket option: foreign connection sent FIN.\n");
     break;
case NU_MEM_ALLOC:
     /* Failed to allocate memory. */
     Tracef("Socket option: memory allocation failure.\n");
     break;
case NU_RESET:
	 Tracef("Socket option: reset.\n");
	 break;


/* These error codes are returned by DNS. */
case NU_INVALID_LABEL:
     /* Indicates a domain name with an invalid label. */
     Tracef("DNS: Invalid domain label.\n");
     break;
case NU_FAILED_QUERY:
     /* No response received for a DNS Query. */
     Tracef("DNS: No response to query.\n");
     break;
case NU_DNS_ERROR:
     /* A general DNS error status. */
     Tracef("DNS: Error.\n");
     break;
case NU_NOT_A_HOST:
     /* The host name was not found. */
     Tracef("DNS: Host not found.\n");
     break;
case NU_INVALID_PARM:
     /*  A parameter has an invalid value. */
     Tracef("DNS: Invalid parameter.\n");
     break;
case NU_NO_IP:
     /*  An IP address was not specified for the device. */
     Tracef("DNS: No IP specified.\n");
     break;
case NU_DHCP_INIT_FAILED:
     /*  During initialization BOOTP failed. */
     Tracef("DHCP Init failure.\n");
     break;
    
    } 
    
	return bSaveSock;
#endif
}


//  DdmNet::NetInit (pMsg)
//
//  Description:
//    Called to initialze the network stack memory area and start necessary devices
//    Currently this device is the DEC/Intel 21143 Ethernet driver.
//
#if 0
STATUS DdmNet::NetInit ()
{

	// move out??
    void		*first_available_memory;
    STATUS		status;

	// want to check for current nnet tasks and memory pool
	// could put in pts / global var but should actively check

	Tracef("DdmNet::Net_Init()\n");

	// Ensure memory pool doesn't already exist
	// add this!
	

	first_available_memory = (void *) new(tBIG) U8[400000];
	
	if (first_available_memory == 0)
	{
	  Tracef("Unable to allocate memory for SYSMEM\n");
	  return OK;
	}
	else
	  Tracef("first_available_memory = %8x\n", first_available_memory);

    status = NU_Create_Memory_Pool(&System_Memory, "SYSMEM", first_available_memory, 400000, 56, NU_FIFO);
   	if (status == NU_SUCCESS)
      Tracef("SYSMEM creation successful\n");
    else
      Tracef("Unable to create SYSMEM memory pool!\n");
		
   	status = NU_Init_Net();

    if (status == NU_SUCCESS)
      Tracef("Net startup successful\n");
    else
   	  Tracef("NET STARTUP FAIL!\n");

	status = InitDevices();

	return (OK);

}

STATUS DdmNet::InitDevices ()
{

	STATUS status;
	
 	Tracef("DdmNet::InitDevices()\n");

// init actual devices - eventually messageize?
#ifdef PPP  
	status = PPPEval();
#endif
#ifdef DECETHER
	status = EtherInit();
#endif
	
	return (OK);

}

//  DdmNet::PPPEval (pMsg)
//
//  Description:
//    Inits the PPP device, does modem emulation for Windows null-modem PPP, and begins negotiation
//

STATUS DdmNet::PPPEval ()
{
	
	return (OK);

}

//  DdmNet::EtherInit ()
//
//  Description:
//    Inits the DEC 21143 Ethernet device
//

STATUS DdmNet::EtherInit ()
{

    STATUS              status;
    uchar               ip_addr[] = {208, 25, 238, 170};
    uchar               subnet[]  = {255, 255, 255, 0};
    NU_DEVICE           devices[1];

	Tracef("Initializing the ethernet..\n");

	/* Initialize the Hardware */
#ifdef  INCLUDE_ODYSSEY
        if ((status = Init_Hardware()) != NU_SUCCESS)
                        printf("Error in init %d\r\n", status);
        NU_Control_Interrupts(0x1C400);
#else
        init_galileo();
#endif  INCLUDE_ODYSSEY

	/* set up the ethernet */
	devices[0].dv_name = "DEC21143_0";
#ifdef INCLUDE_ODYSSEY
    devices[0].dv_hw.ether.dv_irq = 6;
#else
	devices[0].dv_hw.ether.dv_irq = 0;
#endif
    devices[0].dv_hw.ether.dv_io_addr = 0x0L;      /* unused    			*/
    devices[0].dv_hw.ether.dv_shared_addr = 0;     /* paramaters.           */

    devices[0].dv_init = DEC21143_Init;
    devices[0].dv_flags = 0;
    memcpy (devices[0].dv_ip_addr, ip_addr, 4);
    memcpy (devices[0].dv_subnet_mask, subnet, 4);
    devices[0].dv_use_rip2 = 0;
    devices[0].dv_ifmetric = 0;
    devices[0].dv_recvmode = 0;
    devices[0].dv_sendmode = 0;

    if (NU_Init_Devices (devices, 1) != NU_SUCCESS)
    {
        Tracef ("Failed to initailize the device.\n\r");
    }
    else
    	Tracef("Ethernet initialization completed.\n");

	return (OK);

}
#endif

void DdmNet::QueMsg(Message** & apMsgQue, int & iMsgQueSize, Message* pMsgReq) {
	Message** apMsgs = new Message*[iMsgQueSize + 1];
	if(apMsgQue) {
		memcpy(apMsgs, apMsgQue, iMsgQueSize);
		delete[] apMsgQue;
	}
	apMsgQue = apMsgs;
	apMsgQue[iMsgQueSize++] = pMsgReq;
	
}

Message* DdmNet::PeekMsg(Message** apMsgQue, int iMsgQueSize) {
	Message* pMsg = NULL;
	if(iMsgQueSize > 0)
		pMsg = apMsgQue[0];
	
	return pMsg;

}

Message* DdmNet::EnqueMsg(Message** & apMsgQue, int & iMsgQueSize) {
	Message* pMsg = NULL;
	if(apMsgQue) {
		pMsg = apMsgQue[0];
		if(iMsgQueSize > 1) {
			for(int i=0;i<iMsgQueSize - 1;i++) {
				apMsgQue[i] = apMsgQue[i+1];
			}
		}
		else {
			delete[] apMsgQue;
			apMsgQue = NULL;
		}
		iMsgQueSize--;
	}
	return pMsg;
	
}

