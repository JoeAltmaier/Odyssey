/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpMessage.cpp
// 
// Description:
// This module contain message allocation methods plus DDM message
// linkages.
// 
// Update Log 
// 5/13/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: convert to DDM use
// 11/30/98 Michael G. Panas: New memory allocation methods, move allocate/
//                            deallocated to this file
/*************************************************************************/
#include "FcpCommon.h"
#include "FcpI2O.h"
#include "FcpMessage.h"
#include "FcpMessageFormats.h"
#include "FcpString.h"
#include "Message.h"
#include "OsTypes.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"

// DDM Headers needed
#include "HdmFcp.h"
#include "HdmFcpInit.h"


/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// MSG globals
/*************************************************************************/


/*************************************************************************/
// FCP_Message_Create
// Create FCP_Message object
// On return, this pointer is updated.
/*************************************************************************/
STATUS FCP_Message_Create(PINSTANCE_DATA Id)
{
 	FCP_TRACE_ENTRY(FCP_Message_Create);
	
	return NU_SUCCESS;
	
} // FCP_Message_Create

/*************************************************************************/
// FCP_Message_Destroy
// Destroy MSG object
/*************************************************************************/
void FCP_Message_Destroy()
{
	// TODO
	
} // FCP_Message_Destroy

/*************************************************************************/
// FCP_Allocate_Message
// Allocate a message of MESSAGESIZE bytes using new
/*************************************************************************/
void *FCP_Allocate_Message(U32 type) {

	TRACE_ENTRY(FCP_Allocate_Message);

	return (void *) new Message(type, sizeof(FCP_MSG_SIZE), MESSAGE_FLAGS_TIMESTAMP);
	
}	// FCP_Allocate_Message

/*************************************************************************/
// FCP_Message_Latency
// Return the Latency in microsecond
/*************************************************************************/
I64 FCP_Message_Latency(void *p_msg) {

	TRACE_ENTRY(FCP_Message_Latency);
	
	Message *pMsg = (Message *)p_msg;

	return pMsg->Latency();
	
}	// FCP_Allocate_Message



/*************************************************************************/
// FCP_Enable_Reply
// We are done with the initialization and the loop is up, send our
// enable message back
/*************************************************************************/
void FCP_Send_Enable_Reply(PINSTANCE_DATA Id)
{
	HdmFcpInit	*pDdm = (HdmFcpInit *)Id->pFCP_DDM;

 	TRACE_ENTRY(FCP_Send_Enable_Reply);
 	
 	// check to see if we were enabled yet
 	if (pDdm->pEnableMsg == NULL)
 	{
 		// no enable yet, flag the loop up
 		pDdm->pEnableMsg = (Message *) 1;
 		return;
 	}
 	
	pDdm->Reply(pDdm->pEnableMsg);
}

/*************************************************************************/
// FCP_Free_Message
// De-Allocate a message allocated by new
/*************************************************************************/
void FCP_Free_Message(void *message) {

	TRACE_ENTRY(FCP_Free_Message);

	delete (Message *)message;
	
}	// FCP_Free_Message

/*************************************************************************/
// FCP_Message_Send_Request
// Send a request message to the next virtual device in the route. Called
// by the Target Task
/*************************************************************************/
STATUS FCP_Message_Send_Request(FCP_EVENT_CONTEXT *p_context,
	FCP_EVENT_ACTION next_action)
{
	STATUS 			 status;
	PINSTANCE_DATA	 Id = p_context->Id;
	HdmFcp			*pHdmFcp = (HdmFcp *)Id->pFCP_DDM;
	VDN				 vd;
	union {
		IDLUN		 idlun;
		long		 l;
	} sig;
	extern	unsigned char firmware_version[];

 	FCP_TRACE_ENTRY(FCP_Message_Send_Request);
	
	// Save action to perform when response is received.
    p_context->action = next_action;
    
    // fill in the LUN and id fields
    sig.idlun.HostId = p_context->iocb.initiator_ID;
    sig.idlun.LUN = BYTE_SWAP16(p_context->iocb.LUN);
    if ((Id->ISP_Type == 0x00002200) && (firmware_version[0] >= 10))
    	// 2200 passes the ID in the IOCB
    	sig.idlun.id = p_context->iocb.target_ID;
    else
    	// for 2100, pass the configured Hard ID
    	sig.idlun.id = Id->FCP_config.hard_address;
    
    FCP_DUMP_HEX(TRACE_L8, "\n\rTarget SCB EXECUTE Message",
    					(U8 *)p_context->message, 128);
    					
    // find next Virtual Device in the route
    vd = pHdmFcp->FCP_Find_Next_Virtual_Device(p_context, sig.l);
    if (vd == -1)
    	return(FCP_ERROR_DISABLED_LUN);		// return an error with no transfer
    
	// Send the message to the next route
	status = pHdmFcp->Send(vd, (Message *)p_context->message,
								(void *) p_context);
	
    if (status != NU_SUCCESS)
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Message_Send_Request", 
			"Message Send failed",
			status,
			(UNSIGNED)p_context);
			
	return status;
	
} // FCP_Message_Send_Request

/*************************************************************************/
// FCP_Message_Send_Response
// Send a reply message back to the Message Sender. This method is called
// by the Initiator Task after status is returned from the disk. The message
// response is already filled in.
/*************************************************************************/
STATUS FCP_Message_Send_Response(FCP_EVENT_CONTEXT *p_context)
{
	STATUS 			 	status;
	HdmFcpInit			*pHdmFcpInit =
							(HdmFcpInit *) p_context->Id->pFCP_DDM;

 	TRACE_ENTRY(FCP_Message_Send_Response);
	
#if defined(FCP_DEBUG)
	// DEBUG
	p_context->Id->CmdsExecuting--;
	p_context->Id->LastDone = (void *) p_context;
#endif
	
	// Send a response back to the caller
	status = pHdmFcpInit->Reply((Message *)p_context->message);
	
	delete p_context;		// done with this guy
	
	return status;
	
} // FCP_Message_Send_Response


