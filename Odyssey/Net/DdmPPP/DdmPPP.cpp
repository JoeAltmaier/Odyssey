//
// DdmPPP.cpp
//
//  Establishes a PPP connection to a client by emulating
//   a modem and PPP server
//  Replies to message when HungupPPP method called
//   or PPP_HANGUP request made
//

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif

#include  "DdmPPP.h"
#include  "Network.h"
#include  "externs.h"
#include  "nucleus.h"
#include  "socketd.h"
#include  "tcpdefs.h"
#include  "tc_defs.h"
#include  "ppp.h"
#include  "string.h"
#include  "BuildSys.h"

#define TRACE_INDEX		TRACE_DDM_PPP

CLASSNAME (DdmPPP, SINGLE);

SERVELOCAL(DdmPPP, REQ_PPP_BEGIN);
SERVELOCAL(DdmPPP, REQ_PPP_HANGUP);
SERVELOCAL(DdmPPP, REQ_PPP_HUNGUP);

extern "C" STATUS ttyport_out(int port, int data);
extern "C" int ttyport_in(int port);

// These functions are interrupt driven
extern "C" int	ttyin(int);
extern "C" int	ttyout(int port, int data);
extern "C" int  ttyhit(int);

extern "C" void ttyioctl(int, int, int);
extern "C" void ttyinit(int, int);

Message *pPPPMsg;

// Static member
MessageBroker *DdmPPP::pMsgBroker;

void SerialPollThread(void *pParam)
{
	((DdmPPP *)pParam)->SerialPoll();
}

//  DdmPPP::DdmPPP (did)
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

DdmPPP::DdmPPP (DID did) : Ddm (did)
{

	TRACEF(TRACE_L8, ("DdmPPP::DdmPPP()\n"));
	
	pMsgBroker = new MessageBroker(this);

//	pppStack	= new(tZERO|tUNCACHED) U8[8192];
//	Kernel::Create_Thread(pppTask, "pppSerialPoll", SerialPollThread,	this, pppStack, 8192);

}
/*	end DdmPPP::DdmPPP  *****************************************************/

//  DdmPPP::Ctor (did)
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
//    DdmPPP::Ctor - Returns a pointer to the new instance, or NULL.
//

Ddm *DdmPPP::Ctor (DID did)
{

   TRACEF(TRACE_L8, ("DdmPPP::Ctor()\n"));

   return (new DdmPPP (did));

} 
/*	end DdmNet::Ctor  *****************************************************/

//  DdmPPP::Initialize (pMsg)
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
//    DdmPPP::Initialize - Returns OK if all is cool, else an error.
//

STATUS DdmPPP::Initialize (Message *pMsg)
{

	TRACEF(TRACE_L8, ("DdmPPP::Initialize()\n"));

	DispatchRequest(REQ_PPP_BEGIN, REQUESTCALLBACK (DdmPPP, BeginPPP));
	DispatchRequest(REQ_PPP_HANGUP, REQUESTCALLBACK (DdmPPP, HangupPPP));	
	DispatchRequest(REQ_PPP_HUNGUP, REQUESTCALLBACK (DdmPPP, HungupPPP));		
	
	pPPPMsg = NULL;

//	Kernel::Schedule_Thread(pppTask);	

	Reply(pMsg,OK);
	return OK;
		
}
/*	end DdmPPP::Initialize  *****************************************************/

//  DdmPPP::Enable (pMsgReq)
//
//  Description:
//   Lets go!

STATUS DdmPPP::Enable(Message *pMsgReq)
{

	TRACEF(TRACE_L8, ("DdmPPP::Enable()\n"));

	Tracef("Sending PPP request message\n");
	Message *pMsgPPP = new Message(REQ_PPP_BEGIN);
	Send(pMsgPPP, REPLYCALLBACK(DdmServices, DiscardReply));
	Tracef("PPP request message sent\n");


	Reply(pMsgReq,OK);
	return OK;
		
}
/* end DdmPPP::Enable  *****************************************************/

STATUS DdmPPP::BeginPPP(Message *pMsg)
{

	TRACEF(TRACE_L8, ("DdmPPP::BeginPPP()\n"));

#ifdef PPP

	char	mstring[80];
	U32		server_ip, client_ip;	
	STATUS	status;
	uchar	subnet[] = {255,255,255,252};
	
	// Ensure that there is not already a PPP message queued.  If there is, this indicates a problem
	// since no one should be requesting a PPP session while one is open!  Should we respond to the new
	// or old message?  Let's first check to see if the link is up.  If it is up, reply to the new message
	// indicating we can't start another PPP session.  If it is down, we somehow didn't ever get notification
	// that the last session ended.  Why?
	if (pPPPMsg)
	{
		TRACEF(TRACE_L8, ("DdmPPP - PPP Connection already open?\n"));
		if (NU_PPP_Still_Connected() == NU_TRUE)
		{
			TRACEF(TRACE_L8, ("DdmPPP - PPP Connection already open!!\n"));		
			Reply(pMsg, !OK);
			return(0);
		}
		else
		{
			TRACEF(TRACE_L8, ("Discarding errant PPP connection!!\n"));
			HungupPPPc();
		}
	}
	
	TRACEF(TRACE_L8, ("Beginning PPP session\n"));
   	NU_Change_Communication_Mode(MDM_TERMINAL_COMMUNICATION);
	NU_Modem_Control_String("OK\n\r");
	
	// Place the request message in the one message queue for the hangup routine to reply to
	pPPPMsg = pMsg;

	// Get our IP from the network configuration
	server_ip = Network::getIP(PPP);
	client_ip = server_ip + 1;

	TRACEF(TRACE_L8, ("Emulating modem..\n"));   	   

	UART_Reinit_Port();
//	MDM_Purge_Input_Buffer();
	
	
    // Receive strings from the serial port and echo OK back.  This emulates
    // a modem so the client believes he/she is dialing out to a server, when
    // hes/she actually is connected to the console port via a null-modem cable
    // Finish this loop once the dial command is issued

	// Should we time this out to go back to the console if the connection isn't made
	// in n seconds??

    while(1)
    {
        /* Receive a MODEM command. */
        Get_Modem_String(mstring);

        /* Respond with OK. */
        NU_Modem_Control_String("OK\n\r");

        /* If the command received was a command to dial then Windows 95 now
           thinks that it has a modem connection to a remote HOST.  Get out of
           this loop because data exchanged beyond this point will be in the
           form of IP packets. */
        if (strncmp(mstring, "ATDT", 4) == 0)
            break;
    }
    
	TRACEF(TRACE_L8, ("Dial command received - connecting..\n"));

    // Pretend like we are a friendly modem;
    NU_Modem_Control_String("CONNECT 19200\n\r");

    // Change to SERVER mode
    NCP_Change_IP_Mode (SERVER);

    // Set the IP address to assign to the client
    NU_Set_PPP_Client_IP_Address ((uchar *)&client_ip);

    // Switch to PPP mode.
     NU_Change_Communication_Mode(MDM_NETWORK_COMMUNICATION);

	// Start PPP negotiation - only used for this null modem connection
	// Normally handled by NU_Wait_For_PPP_Client
//    NU_Set_PPP_Login ("rbraun", "xxx");
    
    status = PPP_Lower_Layer_Up((uchar *)&server_ip);


    if (status == NU_SUCCESS)
    {
    
		TRACEF(TRACE_L8, ("PPP Lower Layer up..\n"));   	   
    
        // Set our new address
        DEV_Attach_IP_To_Device ("PPP", (uchar *)&server_ip, subnet);

	}
	else
	{
		TRACEF(TRACE_L8, ("Could not bring PPP Lower Layer up.. hanging up.\n"));   	   
        MDM_Hangup();
		TRACEF(TRACE_L8, ("Replying to request message..\n"));
		Reply(pPPPMsg, !OK);
		pPPPMsg = NULL;	 	           
	}
	
	return OK;

#else
	Tracef("PPP device not available (undefined)!\n");
#endif PPP

	
} /* End BeginPPP */

// HungupPPP() is called when the connection is broken and control should be transfered back to
// the console menu.  Normally called from the PPP library when it has hung up
STATUS DdmPPP::HungupPPPc()
{

	TRACEF(TRACE_L8, ("DdmPPP::HungupPPP()\n"));

	if (pPPPMsg)
	{
		// Reply to the original PPP request message
		Reply(pPPPMsg, OK);
		pPPPMsg = NULL;
		return (0);
	}
	else
		return (-1);
	
}

STATUS DdmPPP::HungupPPP(Message *pMsg)
{

	TRACEF(TRACE_L8, ("DdmPPP::HungupPPP(*pMsg)\n"));

	if (pPPPMsg)
	{
		// Reply to the original PPP request message
		Reply(pPPPMsg, OK);
		pPPPMsg = NULL;
		return (0);
	}
	
	Reply(pMsg, OK);
	
	return (0);
		
}

// HangupPPP(Message *) is called when a Ddm wants to force the PPP connection to be broken
// and control returned to the console menu.  
STATUS DdmPPP::HangupPPP(Message *pMsg)
{

	TRACEF(TRACE_L8, ("DdmPPP::HangupPPP()\n"));

	if (pPPPMsg)
	{
		TRACEF(TRACE_L8, ("Forcing PPP Hangup\n"));	
		// Force PPP to disconnect (request a hangup)
		// This FORCED hangup will disconnect even if there are sockets
		// currently open
		PPP_Hangup(NU_FORCE);
		// Reply to the original PPP request message
		Reply(pPPPMsg, OK);
		pPPPMsg = NULL;
		// Reply to the HangupPPP message
		Reply(pMsg, OK);
		TRACEF(TRACE_L8, ("HangupPPP() done\n"));
		return (0);
	}
	else
	{
		Reply(pMsg, !OK);
		return (0);
	}
	
}

void DdmPPP::SerialPoll()
{

	TRACEF(TRACE_L8, ("DdmPPP::SerialPoll()\n"));

	while(1)
	{
		// If there is data waiting..
		if (ttyhit(COM1))
		{
			// Read it in, and keep reading it in
			// Could hog the CPU
			UART_PPP_ISR (COM1);
			while (ttyhit(COM1))
			{
				UART_PPP_ISR (COM1);			
			}		
		}
		else
		{
			NU_Sleep(10);		
		}	
	}


}

extern "C" void SendHangup()
{
	TRACEF(TRACE_L8, ("SendHangup() waiting for MessageBroker response\n"));
	
	Message *pMsg = new Message(REQ_PPP_HUNGUP);
	DdmPPP::pMsgBroker->SendWait(pMsg);
	delete pMsg;
	TRACEF(TRACE_L8, (" /SendHangup()\n"));

}

// "Receives" a fake modem command from the client
// Used to emulate a modem
char *DdmPPP::Get_Modem_String(char *response)
{
    char            c;
    char            *write_ptr;

    write_ptr = response;
    *write_ptr = NU_NULL;

    while (1)
    {
		/* get a character from the port if one's there */
        if (NU_Terminal_Data_Ready())
        {
            NU_Get_Terminal_Char(&c);

            switch (c)
            {
                case 0xD:                /* CR - return the result string */
                    if (*response)
                        return response;
					continue;
				default:
                    if (c != 10)
                    {      /* add char to end of string */
                        *write_ptr++ = (char)c;
                        *write_ptr = NU_NULL;
						/* ignore RINGING and the dial string */
					}
			}
		}
        else
            NU_Sleep(5);

	}

} /* End Get_Modem_String */