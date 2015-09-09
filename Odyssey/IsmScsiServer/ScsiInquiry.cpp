/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiInquiry.c
// 
// Description:
// This code Inquiry for the Scsi Target Server
// - normal Inquiry
// - extended Inquiry
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiInquiry.cpp $
// 
// 1     11/15/99 4:01p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
// 
// 11/05/99 Michael G. Panas: Create file
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


//******************************************************************************
//	ScsiBuildInquiryData
//	Build a set of default Inquiry page data for this Virtual Circuit. These
//  values are built at the address pointed to by p.  This pointer must point to
//  an Inquiry structure.  The struct must be zeroed before this call.
//******************************************************************************
void ScsiServerIsm::ScsiBuildInquiryData(void *p)
{
	INQUIRY		*pInq = (INQUIRY *)p;
	
	TRACE_ENTRY(ScsiBuildInquiryData);
	
	pInq->type = SCSI_DEVICE_TYPE_DIRECT;
	pInq->Version = 2;
	pInq->RespFmt = 2;
	pInq->AdditionalLength = INQUIRY_ADDITIONAL_LENGTH;
	
	memcpy(&pInq->VendorId, INQUIRY_VENDOR_ID, 8);
	memcpy(&pInq->ProductId, INQUIRY_PRODUCT_ID, 16);
	
	// TODO:
	// Get this value from the actual build somehow
	memcpy(&pInq->ProductRevision, INQUIRY_REVISION_LEVEL, 4);
	
	memset(&pInq->VendorSpecific, 0x20, 20);
	
} // ScsiBuildInquiryData


/*************************************************************************/
// ScsiServerInquiry
// Handle SCSI Inquiry command.  Includes all extended versions.
/*************************************************************************/
void ScsiServerIsm::ScsiServerInquiry(SCSI_CONTEXT *p_context)
{
	U32				 Status = 0;
	SCB_PAYLOAD		*pP = 
				(SCB_PAYLOAD *)((Message *)p_context->pMsg)->GetPPayload();
	INQUIRY_CMD		*p_inq_cmd = (INQUIRY_CMD *)pP->CDB;
	U32				length;
	
	TRACE_ENTRY(ScsiServerInquiry);
	
	if (p_inq_cmd->EVPD == 0)
	{
		// our inquiry struct for this Virtual Circuit
		// has already been read, just return it to the caller
		
		// use the CDB length or the INQUIRY size which ever is smaller
		length = (p_inq_cmd->Length < sizeof(INQUIRY)) ?
						p_inq_cmd->Length : sizeof(INQUIRY);
		
		// copy the structure to the SGL address in the message
		ScsiSendData(p_context, (U8 *)&pData->InqData, length);
		
		// reply to the message (with no errors)
		ScsiReply(p_context, length);
	}
	else
	{
		// Handle Vital Product Data Requests
		switch (p_inq_cmd->PageCode)
		{
		case INQ_PAGE_CODE_SUPPORTED_PAGES:
			{
			struct XX {
				SUPPORTED_PAGES		header;
				U8					supported[2];
			} pages = { // header
						SCSI_DEVICE_TYPE_DIRECT, INQ_PAGE_CODE_SUPPORTED_PAGES, 0, 2,
						// supported pages
						0, INQ_PAGE_CODE_SERIAL_NUMBER};
			
			// use the CDB length or the supported pages string
			// size whichever is smaller
			length = (p_inq_cmd->Length < sizeof(pages)) ?
								p_inq_cmd->Length : sizeof(pages);
			
			// copy the structure to the SGL address in the message
			ScsiSendData(p_context, (U8 *)&pages, length);
			
			// reply to the message (with no errors)
			ScsiReply(p_context, length);
			}
			break;
			
		case INQ_PAGE_CODE_SERIAL_NUMBER:
			{
			// build an inquiry serial number struct for this Virtual Ciruit
			// and return it to the caller
			INQUIRY_SERIAL_NUMBER		InqSn;
			
			memset(&InqSn, 0, sizeof(INQUIRY_SERIAL_NUMBER));		// zero the structure
			
			InqSn.DeviceType = SCSI_DEVICE_TYPE_DIRECT;
			InqSn.PageCode = INQ_PAGE_CODE_SERIAL_NUMBER;
			InqSn.PageLength = sizeof(INQUIRY_SERIAL_NUMBER) - 2; // ???
			
			// get the serial numbers for this Virtual Circuit
			memcpy(&InqSn.ProductSerialNumber, &pStsExport->SerialNumber, 8);
			memcpy(&InqSn.BoardSerialNumber, "000001", 6);
			
			// use the CDB length or the INQUIRY SERIAL NUMBER
			// size whichever is smaller
			length = (p_inq_cmd->Length < sizeof(INQUIRY_SERIAL_NUMBER)) ?
								p_inq_cmd->Length : sizeof(INQUIRY_SERIAL_NUMBER);
			
			// copy the structure to the SGL address in the message
			ScsiSendData(p_context, (U8 *)&InqSn, length);
			
			// reply to the message (with no errors)
			ScsiReply(p_context, length);
			}
			break;
			
		case INQ_PAGE_CODE_DEVICE_IDENTIFICATION:
			// TODO:
			// for now, just an error
			SetStatus(SENSE_ILLEGAL_REQUEST, ASC_INVALID_FIELD_IN_CDB);
			ScsiErrReply(p_context);
			m_Status.NumErrorInternal++;
			break;

		default:
			// error in CDB
			SetStatus(SENSE_ILLEGAL_REQUEST, ASC_INVALID_FIELD_IN_CDB);
			ScsiErrReply(p_context);
			m_Status.NumErrorInternal++;
		}
	}
} // ScsiServerInquiry
