/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RdIsm.cpp
//
// Description:
// This file is the implementation of a BSA Ram Disk ISM device.
//
// I2O_BSA_ messages are received by the DoWork method. 
//
//
// Update Log: 
// 03/11/99 Michael G. Panas: Create file from BsaIsm.cpp
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "OsStatus.h"

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_RAM_DISK
#include "Odyssey_Trace.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"

#include "Pci.h"
#include "Scsi.h"
#include "Odyssey.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

#include "RdIsm.h"

#include "BuildSys.h"


CLASSNAME(RdIsm, MULTIPLE);

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/
extern 	"C" void bcopy(U8 *buffer, U8 *p, U32 length);

/*************************************************************************/
// RdIsm
// Constructor method for the class RdIsm
/*************************************************************************/
RdIsm::RdIsm(DID did):Ddm(did) {

	TRACE_ENTRY(RdIsm::RdIsm);
	
	TRACE_HEX(TRACE_L6, "\n\rRdIsm::RdIsm this = ", (U32)this);
	TRACE_HEX(TRACE_L6, "\n\rRdIsm::RdIsm did = ", (U32)did);
	
	myVd = GetVdn();
	
	SetConfigAddress(&config, sizeof(config));
		
} // RdIsm

/*************************************************************************/
// Ctor
// 
/*************************************************************************/
Ddm *RdIsm::Ctor(DID did) {

	TRACE_ENTRY(RdIsm::Ctor);
	
	return new RdIsm(did); // create a new instance of this HDM
	
} // Ctor

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS RdIsm::Initialize(Message *pMsg) {

	STATUS			 status;
	
	TRACE_ENTRY(RdIsm::Initialize);
	
	TRACE_DUMP_HEX(TRACE_L6, "\n\rRdIsm::Initialize config data  = ",
								(U8 *)&config, (U16)sizeof(config));
	
	// allocate enough memory for all the requested blocks
	pRam = new(tBIG) unsigned char [config.capacity * 512];
	if (pRam == NULL)
	{
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_HARD;
	}
	pEnd = (U8 *) pRam + (config.capacity * 512);
	
	// zero the entire data area
	memset(pRam, 0xE5, config.capacity * 512);
	
	num_sectors = config.capacity;
				
	status = RdTableInitialize(pMsg);
	
	//Reply(pMsg);
	
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Initialize

/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
STATUS RdIsm::Enable(Message *pMsg) {

	TRACE_ENTRY(RdIsm::Enable);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Enable

/*************************************************************************/
// Quiesce
// Turn off the DDM belonging to this derived class
/*************************************************************************/
STATUS RdIsm::Quiesce(Message *pMsg) {

	TRACE_ENTRY(RdIsm::Quiesce);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Quiesce

/*************************************************************************/
// DoWork
// This derived classes method to receive messages and replies
/*************************************************************************/
STATUS RdIsm::DoWork(Message *pMsg) {

	TRACE_ENTRY(RdIsm::DoWork);
	
	STATUS status = Ddm::DoWork(pMsg);
	TRACE_ENTRY(RdIsm::DoWork Ddm::DoWork);
	
	if (status != OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION)
		return status;
		
    TRACE_DUMP_HEX(TRACE_L8, "\n\rRdIsm::DoWork Message",
    					(U8 *)pMsg, 64);

	if (pMsg->IsReply()) {
		
	    TRACE_DUMP_HEX(TRACE_L8, "\n\rRdIsm::DoWork Message Reply",
	    					(U8 *)pMsg, 64);
	    // no replies
		return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
	}
	// New service message
	// Handle the BSA_xxxx messages
	else switch(pMsg->reqCode) {

	case BSA_BLOCK_READ:
	{
		BSA_RW_PAYLOAD	*pBsa = (BSA_RW_PAYLOAD *)pMsg->GetPPayload();
		U8 				*p = (pRam + (pBsa->LogicalBlockAddress * 512));
		BSA_REPLY_PAYLOAD		 payload;
		U32						 count = 0;
		
		TRACE_HEX(TRACE_L8, "\n\rRdIsm::DoWork BSA_BLOCK_READ, Block=",
						pBsa->LogicalBlockAddress);
		
	    TRACE_DUMP_HEX(TRACE_L8, "\n\rRdIsm::DoWork Read Data",
	    					(U8 *)p, 16);
	    					
		// clear the payload 
		memset(&payload, 0, sizeof(BSA_REPLY_PAYLOAD));
		
		// check for READ off the end
		if ((p > pEnd) || ((p + pBsa->TransferByteCount) > pEnd))
		{
			// bad address
			status = FCP_SCSI_HBA_DSC_DATA_OVERRUN;
			count = 0;
		}
		else 
		{
			U8			*buffer;
			U32			length;
			U32			iSgl = pMsg->GetCSgl();
			
			for (int i = 0; iSgl > 0; iSgl--, i++) {
				// Read data from the RAM Disk to the SGL Address
				pMsg->GetSgl(i, (void**) &buffer, (unsigned long *) &length);
		
				TRACE_HEX(TRACE_L8, "\n\rRD: read, buffer = ", (U32) buffer);
				TRACE_HEX(TRACE_L8, "  length = ", (U32) length);
				
#if 0
				// use a memcpy() for now
				memcpy(buffer, p, length);
#else
				bcopy(p, buffer, length);
#endif
				p += length;
				count += length;
			}
			
			// done with the read, return count and good status
			status = FCP_SCSI_HBA_DSC_SUCCESS;

		}
		// reply to the BSA message
		// pass the Scsi error code back
		pMsg->DetailedStatusCode = status;
		
		// copy the transfer count
		payload.TransferCount = count;

		// no sense data
		payload.AutoSenseTransferCount = 0;

		// add the reply payload to the original message to make a reply
		pMsg->AddReplyPayload(&payload, sizeof(BSA_REPLY_PAYLOAD));

		// reply to caller
		Reply(pMsg);
		
		return OS_DETAIL_STATUS_SUCCESS; // need a reply here
		
	}
		break;

	case BSA_BLOCK_WRITE_VERIFY:		// TODO: 
	case BSA_BLOCK_WRITE:
	{
		BSA_RW_PAYLOAD	*pBsa = (BSA_RW_PAYLOAD *)pMsg->GetPPayload();
		U8 				*p = (pRam + (pBsa->LogicalBlockAddress * 512));
		BSA_REPLY_PAYLOAD		 payload;
		U32						 count = 0;
				
		TRACE_HEX(TRACE_L8, "\n\rRdIsm::DoWork BSA_BLOCK_WRITE, Block=",
						pBsa->LogicalBlockAddress);
		
	    TRACE_DUMP_HEX(TRACE_L8, "\n\rRdIsm::DoWork Write Data - old",
	    					(U8 *)p, 16);
	    					
		// clear the payload 
		memset(&payload, 0, sizeof(BSA_REPLY_PAYLOAD));
		
		// check for a write past the end
		if ((p > pEnd) || ((p + pBsa->TransferByteCount) > pEnd))
		{
			// bad address
			status = FCP_SCSI_HBA_DSC_DATA_OVERRUN;
			count = 0;
		}
		else 
		{				
			U8			*buffer;
			U32			length;
			U32			iSgl = pMsg->GetCSgl();
			
			for (int i = 0; iSgl > 0; iSgl--, i++) {
			
				// Write data to the RAM Disk from the SGL address
				pMsg->GetSgl(i, (void**) &buffer, (unsigned long *) &length);
		
				TRACE_HEX(TRACE_L8, "\n\rRD: write, buffer = ", (U32) buffer);
				TRACE_HEX(TRACE_L8, "  length = ", (U32) length);
				
			    TRACE_DUMP_HEX(TRACE_L8, "\n\rRdIsm::DoWork write Data - incoming",
			    					(U8 *)buffer, 16);
			    					
#if 0
				// use a memcpy() for now
				memcpy(p, buffer, length);
#else
				bcopy(buffer, p, length);
#endif
				p += length;
				count += length;
			}
			
			// done with the write, return count and good status
			status = FCP_SCSI_HBA_DSC_SUCCESS;

		}
		
	    TRACE_DUMP_HEX(TRACE_L8, "\n\rRdIsm::DoWork write Data - after",
	    					(U8 *)p - count, 16);
	    					
		// reply to the BSA message
		// pass the Scsi error code back
		pMsg->DetailedStatusCode = status;
		
		// copy the transfer count
		payload.TransferCount = count;

		// no sense data
		payload.AutoSenseTransferCount = 0;

		// add the reply payload to the original message to make a reply
		pMsg->AddReplyPayload(&payload, sizeof(BSA_REPLY_PAYLOAD));

		// reply to caller
		Reply(pMsg);
		
		return OS_DETAIL_STATUS_SUCCESS;
		
	}
		break;

	case BSA_POWER_MANAGEMENT:
	{
		TRACE_STRING(TRACE_L8, "\n\rRdIsm::DoWork BSA_POWER_MANAGEMENT");
		
		// reply to caller
		Reply(pMsg, FCP_SCSI_HBA_DSC_SUCCESS);
		
		return OS_DETAIL_STATUS_SUCCESS;
		
	}
	
	case BSA_STATUS_CHECK:
	{
		TRACE_STRING(TRACE_L8, "\n\rRdIsm::DoWork BSA_STATUS_CHECK");
		
		// reply to caller
		Reply(pMsg, FCP_SCSI_HBA_DSC_SUCCESS);
		
		return OS_DETAIL_STATUS_SUCCESS;
		
	}
	
	case BSA_DEVICE_RESET:
	{
		TRACE_STRING(TRACE_L8, "\n\rRdIsm::DoWork BSA_DEVICE_RESET");
		
		// reply to caller
		Reply(pMsg, FCP_SCSI_HBA_DSC_SUCCESS);
		
		return OS_DETAIL_STATUS_SUCCESS;
		
	}
	
	// these messages are not supported
	case BSA_MEDIA_VERIFY:
	case BSA_MEDIA_UNLOCK:
	case BSA_MEDIA_MOUNT:
	case BSA_MEDIA_LOCK:
	case BSA_MEDIA_FORMAT:
	case BSA_MEDIA_EJECT:
	case BSA_BLOCK_REASSIGN:
	default:
		return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
		}

	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
} // DoWork


// end of C++ derived class


