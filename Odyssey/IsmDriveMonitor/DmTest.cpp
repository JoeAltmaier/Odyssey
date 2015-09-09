/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmTest.cpp
// 
// Description:
// This module contains Test Routines that can be invoked externally.
//	DM_Read_Drive
//		Read a single sector from any drive continuously
//	DM_Write_Drive_Block
//		Write an incrementing pattern to a range of sectors
//	DM_Read_Drive_Block
//		Verify a previously written incrementing pattern from a
//		range of sectors
// 
//
// Update Log:
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DmTest.cpp $
// 
// 3     1/11/00 7:27p Mpanas
// New PathTable changes
// 
// 1     9/14/99 8:40p Mpanas
// Complete re-write of DriveMonitor
// - Scan in sequence
// - Start Motors in sequence
// - LUN Scan support
// - Better table update
// - Re-organize sources
// 
// 
// 08/23/99 Michael G. Panas: Create file
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"
#include "Scsi.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

#include "DriveMonitorIsm.h"
#include "DmCommon.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"


/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// DM_Read_Drive
// Read a sector from a drive
/*************************************************************************/
void DriveMonitorIsm::DM_Read_Drive(DMT_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;	// faster access
	CDB10			*pRd;
	SCB_PAYLOAD		*pP;
	STATUS			 status;

	DM_TRACE_ENTRY(DM_Read_Drive);

	// allocate a buffer for the sector data
	pDmc->buffer = new (tSMALL|tUNCACHED) char[512];
	
	// build the Read Capacity message
	DM_Build_SCSI_Message(pMsg, CMD_READ10, pDmc->buffer, 512, pDmc->drive_number, 0);

	// fix CDB
	pP = (SCB_PAYLOAD *) pMsg->GetPPayload();
	pRd = (CDB10 *) &pP->CDB;
	// misaligned field, access as bytes
	pP->CDB[7] = 0; 		// one sector to read
	pP->CDB[8] = 1;
	pRd->BlockAddress = 0;	// set sector zero
	
	// handle the sector data when the reply comes back
	status = Send((VDN)pDmc->Vd, (Message *)pDmc->pMsg, (void *)pDmc,
					(ReplyCallback)&DM_Read_Drive_Callback);
	
 	if (status != NU_SUCCESS)
		DM_Log_Error(DM_ERROR_TYPE_FATAL,
			"DM_Read_Drive", 
			"Send failed",
			config.xlate[pDmc->drive_number],
			(UNSIGNED)status);

} // DM_Read_Drive


/*************************************************************************/
// DM_Read_Drive_Callback
// Finish (or continue) the Read a sector from a drive function
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Read_Drive_Callback(Message *pMsg)
{
	DMT_CONTEXT *pDmc = (DMT_CONTEXT *)pMsg->GetContext();
	
	TRACE_ENTRY(DM_Read_Drive_Callback);
	
	// read is done
	DM_PRINT_HEX16(TRACE_L8, "\n\rRead Done, Id=",
						pDmc->drive_number);
	
	// delete the buffer so no memory leaks
	delete pDmc->buffer;
		
	if (DM_Drive_Test[pDmc->drive_number] == 0)
	{
		// done with this test
		// delete memory allocated for message
		delete (Message *)pDmc->pMsg;
		
		// delete the context
		delete pDmc;
		
		return(0);
	}
	
	// do the read again
	DM_Read_Drive(pDmc);
	
	return(0);
	
} // DM_Read_Drive_Callback
			

/*************************************************************************/
// DM_Write_Drive_Block
// Write a single transfer of block_count sectors at logical_block_address.
/*************************************************************************/
void DriveMonitorIsm::DM_Write_Drive_Block(DMT_CONTEXT *pDmc,
	U32 logical_block_address, U32 block_count)
{
	Message			*pMsg = pDmc->pMsg;	// faster access
	CDB10			*pRd;
	SCB_PAYLOAD		*pP;
	STATUS			 status;

	DM_TRACE_ENTRY(DM_Write_Drive_Block);

	// allocate a buffer for the sector data
	U32 block_size = 512 * block_count;
	pDmc->buffer = new (tSMALL|tUNCACHED) char[block_size];
	
	// Fill the buffer with data that we can verify.
	// Each word contains its byte offset.
	U32 *p_data = (U32 *)pDmc->buffer;
	U32 num_words = block_size / sizeof(U32);
	U32 word_written;
	U32 logical_byte_address = logical_block_address * 512;
	for (U32 index = 0; index < num_words; index++)
	{
		word_written = logical_byte_address + (index * sizeof(U32));
		*(p_data + index) = word_written;
	}

	// build the Write message
	DM_Build_SCSI_Message(pMsg, CMD_WRITE10, pDmc->buffer, block_size,
		pDmc->drive_number, 0);

	// fix CDB
	pP = (SCB_PAYLOAD *) pMsg->GetPPayload();
	pRd = (CDB10 *) &pP->CDB;
	// misaligned field, access as bytes
	pP->CDB[7] = 0; 		// number of sectors to read
	pP->CDB[8] = block_count;
	pRd->BlockAddress = logical_block_address;	// set sector 
	
	// handle the data when the reply comes back
	status = Send((VDN)pDmc->Vd, (Message *)pDmc->pMsg, (void *)pDmc,
					(ReplyCallback)&DM_Write_Drive_Block_Callback);
	
 	if (status != NU_SUCCESS)
		DM_Log_Error(DM_ERROR_TYPE_FATAL,
			"DM_Write_Drive_Block", 
			"Send failed",
			config.xlate[pDmc->drive_number],
			(UNSIGNED)status);
	
} // DM_Write_Drive_Block

/*************************************************************************/
// DM_Write_Drive_Block_Callback
// Finish Write Drive Block function
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Write_Drive_Block_Callback(Message *pMsg)
{
	DMT_CONTEXT		*pDmc = (DMT_CONTEXT *)pMsg->GetContext();
	Message 		*pRf = (Message *)pMsg;
	STATUS			 status;
	
	TRACE_ENTRY(DM_Write_Drive_Block_Callback);

	// write is done
	
	// check for reply needed
	if (pDmc->pReply)
	{
		status = Reply(pDmc->pReply, pRf->DetailedStatusCode);
	}
	
	DM_PRINT_HEX16(TRACE_L8, "\n\rWrite verify Done, Id=",
						config.xlate[pDmc->drive_number]);
	
	// delete the buffer so no memory leaks
	delete pDmc->buffer;
		
	// done with this test
	// delete memory allocated for message
	delete (Message *)pDmc->pMsg;
	
	// delete the context
	delete pDmc;
	
	return(0);
	
} // DM_Write_Drive_Block_Callback
			

/*************************************************************************/
// DM_Read_Drive_Block
// Write a single transfer of block_count sectors at logical_block_address.
/*************************************************************************/
void DriveMonitorIsm::DM_Read_Drive_Block(DMT_CONTEXT *pDmc,
	U32 logical_block_address, U32 block_count)
{
	Message			*pMsg = pDmc->pMsg;	// faster access
	CDB10			*pRd;
	SCB_PAYLOAD		*pP;
	STATUS			 status;

	DM_TRACE_ENTRY(DM_Write_Drive_Block);

	// allocate a buffer for the sector data
	U32 block_size = 512 * block_count;
	pDmc->buffer = new (tSMALL|tUNCACHED) char[block_size];
	
	// build the Read message
	DM_Build_SCSI_Message(pMsg, CMD_READ10, pDmc->buffer, block_size,
					pDmc->drive_number, 0);

	// fix CDB
	pP = (SCB_PAYLOAD *) pMsg->GetPPayload();
	pRd = (CDB10 *) &pP->CDB;
	// misaligned field, access as bytes
	pP->CDB[7] = 0; 		// number of sectors to read
	pP->CDB[8] = block_count;
	pRd->BlockAddress = logical_block_address;	// set sector 
	
	// Save logical_block_address and block_count in context
	// so that we can verify when we get the response.
	pDmc->logical_block_address = logical_block_address;
	pDmc->block_count = block_count;

	// handle the data when the reply comes back
	status = Send((VDN)pDmc->Vd, (Message *)pDmc->pMsg, (void *)pDmc,
					(ReplyCallback)&DM_Read_Drive_Block_Callback);
	
 	if (status != NU_SUCCESS)
		DM_Log_Error(DM_ERROR_TYPE_FATAL,
			"DM_Read_Drive_Block", 
			"Send failed",
			config.xlate[pDmc->drive_number],
			(UNSIGNED)status);

} // DM_Read_Drive_Block

/*************************************************************************/
// DM_Read_Drive_Block_Callback
// Finish Read a sector from a drive function
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Read_Drive_Block_Callback(Message *pMsg)
{
	DMT_CONTEXT		*pDmc = (DMT_CONTEXT *)pMsg->GetContext();
	Message 		*pRf = (Message *)pMsg;
	STATUS			 status;
	
	TRACE_ENTRY(DM_Read_Drive_Block_Callback);
	
	// check return status, if there was an error don't
	// bother to verify
	if (pRf->DetailedStatusCode)
	{
		status = Reply(pDmc->pReply, pRf->DetailedStatusCode);
	}
	else
	// read is done, now try to verify that it is correct
	{
		status = Verify_Read(pDmc);
		
		// check for reply needed
		if (pDmc->pReply)
		{
			status = Reply(pDmc->pReply, status);
		}
	}
	
	DM_PRINT_HEX16(TRACE_L8, "\n\rRead verify Done, Id=",
						config.xlate[pDmc->drive_number]);
	
	// delete the buffer so no memory leaks
	delete pDmc->buffer;
		
	// done with this test
	// delete memory allocated for message
	delete (Message *)pDmc->pMsg;
	
	// delete the context
	delete pDmc;

	return(0);
	
} // DM_Read_Drive_Block_Callback
			

/*************************************************************************/
// Verify_Read
// Come here when read has finished to verify data.
// Return a status value
/*************************************************************************/
STATUS DriveMonitorIsm::Verify_Read(DMT_CONTEXT *pDmc)
{
	// Verify the data.
	// Each word should contain its byte offset.
	U32 *p_data = (U32 *)pDmc->buffer;
	U32 block_size = 512 * pDmc->block_count;
	U32 num_words = block_size / sizeof(U32);
	U32 word_written;
	U32 word_read;
	U32 logical_byte_address = pDmc->logical_block_address * 512;
	for (U32 index = 0; index < num_words; index++)
	{
		word_written = logical_byte_address + (index * sizeof(U32));
		word_read = *(p_data + index);
		if (word_read != word_written)
		{
			DM_PRINT_STRING(TRACE_L3, "Read verify error");
			return (FCP_SCSI_HBA_DSC_COMPLETE_WITH_ERROR);
		}
	}
	
	DM_PRINT_STRING(TRACE_L3, "Verify successful");
	return (FCP_SCSI_HBA_DSC_SUCCESS);
	
} // Verify_Read

