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
// This file is the implementation of a test Echo Scsi device.
//
// I2O_SCSI_ messages are received by the DoWork method from a Target or
// other sender. These messsages are then handled by translating the
// Initiator and LUN to the correct LUN and target id and then passing
// the command on to a Fibre Channel SCSI Initiator device.
//
// One assumption is the Target Id and LUN are passed in the TransactionContext
// field of the I2O_SCSI_SCB_EXECUTE_MESSAGE received.
//
// This will be turned into the BSA ISM later.  In this case we do not need
// to translate the I2O BSA message into an SCB_EXEC message.
//
// Update Log:
//	$Log: /Gemini/Odyssey/IsmEchoScsi/EchoScsiIsm.cpp $
// 
// 12    7/28/99 7:58p Mpanas
// changes to support new EchoScsi config
// 
// 10/2/98 Michael G. Panas: Create file
// 11/24/98 Michael G. Panas: add support for BuildSys
// 01/24/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
// 02/12/99 Michael G. Panas: convert to new Oos model
// 02/17/99 Michael G. Panas: convert to new Message format
/*************************************************************************/

#include <stdio.h>
#include "OsTypes.h"
#include "RequestCodes.h"

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_ECHO_SCSI

#include "Odyssey_Trace.h"
#include "Pci.h"
#include "Odyssey.h"

#include "Message.h"
#include "FcpMessageFormats.h"

#include "EchoScsiIsm.h"
#include "CTIdLun.h"
#include "BuildSys.h"

// Tables referenced
#include "ExportTable.h"
#include "DiskDescriptor.h"


CLASSNAME(EchoScsiIsm, MULTIPLE);

/*************************************************************************/
// Forward references
/*************************************************************************/


/*************************************************************************/
// Global references
/*************************************************************************/



/*************************************************************************/
// EchoScsiIsm
// Constructor method for the class HdmFcpRac
/*************************************************************************/
EchoScsiIsm::EchoScsiIsm(DID did):Ddm(did) {

	TRACE_ENTRY(EchoScsiIsm::EchoScsiIsm);
	
	myVd = GetVdn();
	
	SetConfigAddress(&config, sizeof(config));
	
} // EchoScsiIsm

/*************************************************************************/
// Ctor
// 
/*************************************************************************/
Ddm *EchoScsiIsm::Ctor(DID did) {

	TRACE_ENTRY(EchoScsiIsm::Ctor);
	
	return new EchoScsiIsm(did); // create a new instance of this HDM
} // Ctor

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS EchoScsiIsm::Initialize(Message *pMsg) {

	STATUS			 status;
	
	TRACE_ENTRY(EchoScsiIsm::Initialize);
	
    TRACE_DUMP_HEX(TRACE_L8, "\n\rEchoScsiIsm::Initialize Config Data",
    					(U8 *)&config,
    					sizeof(ES_CONFIG));

	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Initialize

/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
STATUS EchoScsiIsm::Enable(Message *pMsg) {

	STATUS			 status;
	
	TRACE_ENTRY(EchoScsiIsm::Enable);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Enable

/*************************************************************************/
// Quiesce
// Turn off the DDM belonging to this derived class
/*************************************************************************/
STATUS EchoScsiIsm::Quiesce(Message *pMsg) {

	STATUS			 status;
	
	TRACE_ENTRY(EchoScsiIsm::Quiesce);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Quiesce

/*************************************************************************/
// DoWork
// This derived classes method to receive messages and replies
/*************************************************************************/
STATUS EchoScsiIsm::DoWork(Message *pMsg) {

	TRACE_ENTRY(EchoScsiIsm::DoWork);
	
	STATUS status = Ddm::DoWork(pMsg);
	TRACE_ENTRY(EchoScsiIsm::DoWork Ddm::DoWork);
	
	if (status != OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION)
		return status;
		
    TRACE_DUMP_HEX(TRACE_L8, "\n\rEchoScsiIsm::DoWork Message",
    					(U8 *)pMsg, 128);

	// New service message
	switch(pMsg->reqCode) {

	case SCSI_SCB_EXEC:
	{
		// Handle the SCSI_SCB_EXECUTE_MESSAGE
		IDLUN				*p_idlun;
		SCB_PAYLOAD			*pP = (SCB_PAYLOAD *)pMsg->GetPPayload();
		
		TRACE_STRING(TRACE_L8, "\n\rEchoScsiIsm::DoWork I2O_SCSI_SCB_EXECUTE_MESSAGE");
		
		TRACE_HEX16(TRACE_L6, "\n\rES MyVd: ", myVd);
		
		// point ot the old LUN and Target Id
		p_idlun = (IDLUN *) &pP->IdLun;
		
		TRACE_HEX16(TRACE_L6, "\n\rES old Id: ", p_idlun->id);
		TRACE_HEX16(TRACE_L6, "\n\rES old LUN: ", p_idlun->LUN);
		
		TRACE_HEX16(TRACE_L6, "\n\rES ID: ", config.ID);
		TRACE_HEX16(TRACE_L6, "\n\rES LUN: ", config.LUN);
		
		// pass the new LUN and Target Id on to the Initiator
		p_idlun->id = config.ID;
		p_idlun->LUN = config.LUN;		// always zero here
		
		// clear the old SCSI-1/2 LUN field since some vendors (IBM)
		// require it to be zero
		pP->CDB[1] &= 0x1f;
		
		// forward on to the SCSI FCP Initiator
		status = Forward(pMsg, config.vdnNext);
		return status;
		
	}
		break;

	case SCSI_DEVICE_RESET:
	case SCSI_SCB_ABORT:
		// no work here for these guys, just forward
		TRACE_HEX16(TRACE_L8, "\n\rEchoScsiIsm::DoWork Other Message", (U16) pMsg->reqCode);
		
		Forward(pMsg, config.vdnNext);
		break;

	default:
		return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
		}

	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
} // DoWork


// end of C++ derived class

