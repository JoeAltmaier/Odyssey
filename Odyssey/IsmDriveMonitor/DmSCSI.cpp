/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmScsi.cpp
// 
// Description:
// This module has the SCSI support code needed by the DriveMonitor
// 
//
// Update Log:
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DmSCSI.cpp $
// 
// 5     2/01/00 2:32p Mpanas
// Fixes for Dell PV discovery
// - Don't use UNTAGGED commmands
// - Fix Plex handling for odd number of devices
// - Handle Inquiry for devices on another loop (NZ device qualifier)
// - Fix SES and Tape handling (Plex handling wrong)
// 
// 4     1/11/00 7:27p Mpanas
// New PathTable changes
// 
// 2     9/16/99 9:54p Mpanas
// Add more debug info
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

#include "Pci.h"
#include "Scsi.h"
#include "CTIdLun.h"

#include "CDB.h"


/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// DM_Build_SCSI_Message
// Build up the SCSI CDB, SGL and ByteCount in an SCB EXEC message.
// Drive_number and lun_number specify who to talk to.  Drive_number is
// an index into the xlate[] array in the config record.  Lun_number
// is passed unchanged.
/*************************************************************************/
void DriveMonitorIsm::DM_Build_SCSI_Message(Message *pMsg, SCSI_COMMANDS Cmd,
							void *pData, long length,
							U32 drive_number, U32 lun_number)
{
	SCB_PAYLOAD						payload;
	
	DM_TRACE_ENTRY(DM_Build_SCSI_Message);
	
	memset(&payload, 0, sizeof(SCB_PAYLOAD));
	
	// set simple tag mode
	payload.SCBFlags = FCP_SCB_FLAG_SIMPLE_QUEUE_TAG;

    // fill in the LUN and id fields
    payload.IdLun.id = (U8)config.xlate[drive_number];
    payload.IdLun.LUN = (U16)lun_number;
    
	payload.CDB[0] = Cmd;
	payload.CDBLength = CDB_Get_CDB_Length((CDB16 *)&payload.CDB);
	
	if (length)
	{
		payload.ByteCount = length;
		CDB_Set_Transfer_Length((CDB16 *)&payload.CDB, length);
	}
	else
	{
		payload.ByteCount = 0;
	}

	pMsg->AddPayload(&payload, sizeof(SCB_PAYLOAD));

	if (length)
	{
		pMsg->AddSgl(0, (void *)pData, length,
						((Cmd==CMD_WRITE10)? SGL_SEND:SGL_REPLY));
	}

} // DM_Build_SCSI_Message



/*************************************************************************/
// DM_Display_Sense
// Given a pointer to a message, try to print SCSI Sense Data if the
// message has a Check Status
/*************************************************************************/
void DriveMonitorIsm::DM_Display_Sense(Message *pMsg)
{
	SCB_REPLY_PAYLOAD		*payload;
	
	DM_TRACE_ENTRY(DM_Display_Sense);
	
	if ((pMsg->DetailedStatusCode & FCP_SCSI_DEVICE_DSC_MASK) !=
		FCP_SCSI_DSC_CHECK_CONDITION)
	{
		// no check condition
		return;
	}
	
	payload = (SCB_REPLY_PAYLOAD *) pMsg->GetPPayload();
	
    DM_DUMP_HEX(TRACE_L3, "\n\rSense Data: ",
    					(U8 *)&payload->SenseData[0],
    					payload->AutoSenseTransferCount);
    					
} // DM_Display_Sense

