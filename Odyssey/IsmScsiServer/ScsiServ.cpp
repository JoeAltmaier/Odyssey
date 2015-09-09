/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiServ.c
// 
// Description:
// This is the main ScsiServer decode file 
// 
// Update Log 
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiServ.cpp $
// 
// 15    11/15/99 4:08p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
// - Remove DoWork() and replace with RequestDefault() and ReplyDefault()
//
// 
// 05/18/98 Michael G. Panas: Create file
// 02/19/99 Michael G. Panas: convert to new Message format
/*************************************************************************/

#include "SCSIServ.h"
#include "ScsiSense.h"
#include "FcpMessageFormats.h"
#include "ScsiModes.h"
#include "Scsi.h"

#include "CDB.h"
#include "CTIdLun.h"

#include <string.h>

/*************************************************************************/
// Forward References
/*************************************************************************/

/*************************************************************************/
// ScsiInitialize()
// Initialize this instance of the SCSI Target Server
//
/*************************************************************************/

void ScsiServerIsm::ScsiInitialize()
{
	TRACE_ENTRY(ScsiInitialize);
	
	// The initial state of any SCSI device is this until a RequestSense
	// clears it
	SetStatus(SENSE_NOT_READY, ASC_POWER_ON_OR_DEVICE_RESET_OCCURED);
	
} // ScsiInitialize


/*************************************************************************/
// ScsiDecode(context)
// Main SCSI Decode routine
//	This function takes a pointer to a SCSI message and trys to
//	decode the SCSI Function code in the first byte of the CDB.  
//	An action will be performed based on the SCSI command.
//
//	The default action is to cause an error.
/*************************************************************************/

void ScsiServerIsm::ScsiDecode(void *p)
{
	SCSI_CONTEXT 			*p_context = (SCSI_CONTEXT *)p;
	U32						 Status = 0;
	SCB_PAYLOAD				*pP = 
				(SCB_PAYLOAD *)((Message *)p_context->pMsg)->GetPPayload();
	
	TRACE_ENTRY(ScsiDecode);
	
	// check to see if we are online
	if (m_VcStatus != VC_ONLINE)
	{
		TRACEF(TRACE_L2, ("\n\rScsiDecode: VC %d Offline: %d", MyVdn, m_VcStatus));

		// sorry, this vc is dead
		SetStatus(SENSE_NOT_READY, ASC_LOGICAL_UNIT_FAILURE);
		ScsiErrReply(p_context);
		m_Status.NumErrorInternal++;
		
		// delete the context
		delete p_context;
		return;
	}
	
	// these commands must work even if an an error condition is present
	switch (pP->CDB[0]) {
	
	case CMD_REQUEST_SENSE:
	{
		CDB6			*p_cmd = (CDB6 *)pP->CDB;
		U32				length;
		REQUEST_SENSE	sense;
		
		// return the request sense data from the last error (if any)
		
		// build the sense data for this error (if any)
		ScsiBuildSense((PREQUEST_SENSE)&sense);
		
		// use the CDB length or the INQUIRY SERIAL NUMBER
		// size whichever is smaller
		length = (p_cmd->Length < sizeof(REQUEST_SENSE)) ?
							p_cmd->Length : sizeof(REQUEST_SENSE);
		
		// copy the structure to the SGL address in the message
		ScsiSendData(p_context, (U8 *)&sense, length);
		
		// reply to the message (with no errors)
		ScsiReply(p_context, length);
		
		// clear the sense codes in case there was an error
		SetStatus(SENSE_NO_SENSE, ASC_NO_ADDITIONAL_SENSE);
	}
		delete p_context;
		return;
		
	case CMD_INQUIRY:
		ScsiServerInquiry(p_context);
		delete p_context;
		return;
	}
	
	// check status, return error status there was one
	if (GetSenseKeyStatus())
	{
		ScsiErrReply(p_context);
		m_Status.NumErrorInternal++;
		delete p_context;
		return;
	}
	
	// these commands will only work if status is good
	switch (pP->CDB[0]) {
	
	// READs and WRITEs are commands passed on to be processed
	// in some other Message context.
	case CMD_READ6:
	case CMD_READ10:
		Status = ScsiServerRead(p_context);
		if (Status)
		{
			SetStatus(SENSE_NOT_READY, ASC_LOGICAL_UNIT_COMMUNICATION_FAILURE);
			ScsiErrReply(p_context);
			m_Status.NumErrorInternal++;
			// if failure, delete the context, message not sent
			delete p_context;
		}
		break;
		
	case CMD_WRITE6:
	case CMD_WRITE10:
		Status = ScsiServerWrite(p_context);
		if (Status)
		{
			SetStatus(SENSE_NOT_READY, ASC_LOGICAL_UNIT_COMMUNICATION_FAILURE);
			ScsiErrReply(p_context);
			m_Status.NumErrorInternal++;
			// if failure, delete the context, message not sent
			delete p_context;
		}
		break;
		
	case CMD_TEST_UNIT:
		// reply with no errors
		ScsiReply(p_context, 0);
		delete p_context;
		break;
		
	case CMD_MODE_SENSE6:
	case CMD_MODE_SENSE10:
		Status = ScsiServerModeSense(p_context);
		delete p_context;
		break;
		
	case CMD_MODE_SELECT6:
	case CMD_MODE_SELECT10:
		Status = ScsiServerModeSelect(p_context);
		delete p_context;
		break;
		
	case CMD_READ_CAPACITY:
	{
		// return the capacity of this LUN
		READ_CAPACITY	Cap;
		
		// Get the capacity from the Export Table entry for this
		// Virtual Circuit
		Cap.BlockAddress = pStsExport->Capacity;
		TRACE_HEX(TRACE_L2, "\n\rScsiDecode: Read Cap = ", Cap.BlockAddress);
		
		// set the block size
		Cap.BlockLength = SYSTEM_BLOCK_SIZE;
		
		// copy the data to the SGL address
		ScsiSendData(p_context, (U8 *)&Cap, sizeof(READ_CAPACITY));
		
		// reply to the message (with no errors)
		ScsiReply(p_context, sizeof(READ_CAPACITY));
		
		// delete the context since we are done with it
		delete p_context;
		break;
	}
		
	case CMD_FORMAT_UNIT:
		// just return a good reply message for now
		ScsiReply(p_context, 0);
		delete p_context;
		break;
		
	case CMD_START_UNIT:
		// just return a good reply message
		ScsiReply(p_context, 0);
		delete p_context;
		break;
		
	case CMD_RESERVE6:
	case CMD_RESERVE10:
		goto Illegal;
		break;
		
	case CMD_RELEASE6:
	case CMD_RELEASE10:
		goto Illegal;
		break;
		
	case CMD_SEND_DIAG:
		// check for pass-thru
		if ( pStsExport->vdLegacyScsi &&
				pData->InqData.EncServ)
		{
			Status = ScsiPassThru(p_context);
			if (Status)
			{
				SetStatus(SENSE_NOT_READY, ASC_LOGICAL_UNIT_COMMUNICATION_FAILURE);
				ScsiErrReply(p_context);
				m_Status.NumErrorInternal++;
			}
			// we are done with this
			delete p_context;
			break;
		}
		goto Illegal;
		break;
		
	case CMD_RECEIVE_DIAG:
		// check for pass-thru
		if ( pStsExport->vdLegacyScsi &&
				pData->InqData.EncServ)
		{
			Status = ScsiPassThru(p_context);
			if (Status)
			{
				SetStatus(SENSE_NOT_READY, ASC_LOGICAL_UNIT_COMMUNICATION_FAILURE);
				ScsiErrReply(p_context);
				m_Status.NumErrorInternal++;
			}
			// we are done with this
			delete p_context;
			break;
		}
		goto Illegal;
		break;
		
	case CMD_REPORT_LUNS:
	{
		// TODO:
	}
		goto Illegal;
		break;
		
	case CMD_VERIFY10:
		// Need to implement a VERIFY stub so
		// NT is happy when check disk runs,
		// for now, just return a good reply message
		// TODO: actual Verify needs to work
		ScsiReply(p_context, 0);
		delete p_context;
		break;
		
	default:
Illegal:
		// To be consistant with the SCSI Spec, the default action is to cause an error.
		// A reply to the SCSI message is generated with error data flagged
		// as: CHECK_STATUS with a sense key of SENSE_ILLEGAL_REQUEST and
		// the ASC/ASCQ values set to ASC_INVALID_COMMAND_OPERATION_CODE.
		SetStatus(SENSE_ILLEGAL_REQUEST, ASC_INVALID_COMMAND_OPERATION_CODE);
		ScsiErrReply(p_context);
		m_Status.NumErrorInternal++;
		
		// delete the context
		delete p_context;
		return;
	}
}

