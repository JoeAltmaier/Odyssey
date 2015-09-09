 /*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class implements the bootblock server
// 
// Update Log: 
// 9/29/99 Ryan Braun: Create file
/*************************************************************************/

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif

#include "BootBlockServer.h"
#include "HbcSubnet.h"
#include "BuildSys.h"

DEVICENAME(BootBlockServer, BootBlockServer::Initialize);

void BootBlockServer::Initialize() {

//	Tracef("BootBlockServer::Initialize()\n");
	StartServer();
	
}

void BootBlockServer::StartServer() {

//	Tracef("BootBlockServer::StartServer()\n");

	bbsTask = new(tZERO) CT_Task;
	bbsStack = new(tZERO) U8[8000];
	Kernel::Create_Thread(*bbsTask, "BootBlockServer", ServerThread, NULL, bbsStack, 8000);
	Kernel::Schedule_Thread(*bbsTask);

}

// I'm Mr. Thread
// My job will never end!  I will be worked to death!!  Unless I screw up that is.
void BootBlockServer::ServerThread(void *pParam) {
	
	struct 			addr_struct	serverAddr;
	int				osocketd, nsocketd;
	struct 			addr_struct	clientAddr;	
	STATUS			status;
	
//	Tracef("BootBlockServer::ServerThread()\n");
	
	// remove compilation warnings
	status = (STATUS) pParam;
	
	// open a connection via the socket interface
	if ((osocketd = NU_Socket(NU_FAMILY_IP, NU_TYPE_STREAM, 0)) >= 0)
	{
	
	// fill in the address structure
	serverAddr.family = NU_FAMILY_IP;
	serverAddr.port = htons(PORT_BOOTBLOCK);
	memcpy(&serverAddr.id.is_ip_addrs, &ip_iSlot[Address::iSlotMe], 4);	
	
	// make an NU_Bind() call to bind the server's address
	status = NU_Bind((int16)osocketd, &serverAddr, 0);

	if (status >= 0)
	{
	    // be ready to accept connection requests
	    status = NU_Listen((int16)osocketd, 10);

	    if (status == NU_SUCCESS)
	    {
			// successful listen request, check for connections

			// set socket to blocking operations
			NU_Fcntl(osocketd, NU_SETFLAG, NU_BLOCK);

			// keep trying for a connection
			// this is not multithreaded for connections - only one can be made at
			// a time.  thus we will have to be careful and exit from closed conenctions
			while (1) {
//				Tracef("BootBlockServer waiting for a connection..\n");
	    		nsocketd = NU_Accept((int16)osocketd, &clientAddr, 0);
			    if (nsocketd >= 0)
			    {
    			
//    				Tracef("Bootblock Server Connection accepted (%d)\n", nsocketd);
    				
					// Set TCP state to push so transfers occur immediately
					NU_Push(nsocketd);	
	
					// Send that puppy!
					PushBootBlock(nsocketd);
					
					// Disconnect and get ready for another fun connection
//					Tracef("Closing socket (%d)\n", nsocketd);
					NU_Close_Socket(nsocketd);

				}									// end of successful NU_Accept
				// no connection ready at this time
				else if (nsocketd == NU_NO_TASK_MATCH)
				{
					// Nap time, try again later
			    	Tracef("No Task Match (%d)\n", nsocketd);
					NU_Sleep(10);
					continue;
				}
				// error code
				else
				{
			    	Tracef("Accept Error (%d)\n", nsocketd);
					NU_Sleep(10);
				}
				
			}									// end while (1) Accept loop

	    }                                       // end successful NU_Listen
	    else
	    {
	    	Tracef("Listen Error (%d)\n", status);
	    }
	}                                           // end successful NU_Bind
	else
	{
    	Tracef("BIND Error (%d)\n", status);	
	}

    }                                           // end successful NU_Socket
    else
    {
	    // unable to obtain socket
    	Tracef("Socket Error (%d)\n", status);
    }

}

void BootBlockServer::PushBootBlock(int socketd) {

	bootblock_t clientblock;

	Tracef("Sending bootblock to ethernet client\n", socketd);
	
	// copy the set size 2048 byte structure so we can manipulate the items we want on
	// the server end
	memcpy(&clientblock, &bootblock, sizeof(bootblock_t));
	
	// for now, the block will be sent raw, unprepared
	// the client will have to change what they want on their end

	// go ahead and send it to the client
	NU_Send(socketd, (char *)&clientblock, sizeof(bootblock_t), 0);

}