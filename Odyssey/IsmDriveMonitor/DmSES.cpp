/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmSES.cpp
// 
// Description:
// This module contains code to handle SES (SCSI Enclosure Services) for
// DriveMonitor
// 

// Update Log:
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DmSES.cpp $
// 
// 4     2/01/00 7:20p Mpanas
// Add more trace info
// 
// 3     2/01/00 2:32p Mpanas
// Fixes for Dell PV discovery
// - Don't use UNTAGGED commmands
// - Fix Plex handling for odd number of devices
// - Handle Inquiry for devices on another loop (NZ device qualifier)
// - Fix SES and Tape handling (Plex handling wrong)
// 
// 2     1/11/00 7:27p Mpanas
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
// DM_Do_SES
// Inquiry found an SES type device, start the statemachine
// for this SES device.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_SES(DM_CONTEXT * pDmc)
{
	DM_DEVICE_STATE	*pDMState = pDmc->pDMState;
	DeviceDescriptor	*pDevice;

	TRACE_ENTRY(DM_Do_SES);

	if (pDMState->pPD == NULL)
	{
		// need to build a PathDescriptor record
		pDMState->pPD = new PathDescriptor;
		memset(pDMState->pPD, 0, sizeof(PathDescriptor));
	}
	
	if (pDMState->pDevice == NULL)
	{
		// need to build a DeviceDescriptor record
		pDMState->pDevice = new DeviceDescriptor;
		memset(pDMState->pDevice, 0, sizeof(DeviceDescriptor));
	}
	
	if (pDMState->pRC == NULL)
	{
		// need build a StorageRollCall record
		pDMState->pRC = new StorageRollCallRecord;
		memset(pDMState->pRC, 0, sizeof(StorageRollCallRecord));
	}
	
	pDevice = pDMState->pDevice;
	
	// set our device type
	pDevice->Type = pDMState->type;
			
	// copy the inquiry data to the SES Object Table
	memcpy(&pDevice->InqData,
			pDmc->buffer,
			sizeof(INQUIRY));
			
	// set ready
	pDevice->CurrentStatus = DriveReady;
	pDMState->pDD->CurrentStatus = DriveReady;
			
	// delete the memory allocated
	delete pDmc->buffer;
	pDmc->buffer = NULL;
	
	TRACEF(TRACE_L3, ("\n\rDM: SES, Id=%d, LUN=%d", 
						config.xlate[pDmc->drive_number],
						pDmc->lun_number));

	// first pass - just know it is there
	DM_Complete_Plex_Request(pDmc, 0);
	
} // DM_Do_SES



/*************************************************************************/
// DM_Do_Update_SES_PTS
// Second pass
// Update the SESDescriptor with the device data from the last device
// scanned.  Check if an update is allowed before the update, skip if not.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Update_SES_PTS(DM_CONTEXT *pDmc)
{
	STATUS			status;
	DM_DEVICE_STATE	*pDMState = pDmc->pDMState;
	
	DM_TRACE_ENTRY(DM_Do_Update_SES_PTS);

#if defined(DM_DEBUG) && defined(_DEBUG)
		char			s[128];

		TRACEF(TRACE_L2, ("\n\rFound SES, Id=%d, LUN=%d", 
							config.xlate[pDmc->drive_number],
							pDmc->lun_number));

		// Build a readable string
		s[0] = '\n';
		s[1] = '\r';
		memcpy(&s[2],
				&pDMState->pDevice->InqData.VendorId[0],
				28 );
		s[30] = 0;
		
		DM_PRINT_STRING(TRACE_L2, s);
		DM_PRINT_STRING(TRACE_L2, "\n\r");

		DM_DUMP_HEX(TRACE_L8, " = ",
				(U8 *)pDMState->pDevice,
				sizeof(DeviceDescriptor));
#endif

	// check if we need to find our descriptor
	// must be failover and new device
	if ((config.flags & DM_FLAGS_REDUNDANT) &&
							(pDMState->state & DEVICE_STATE_NEW_DEVICE))
	{
		DmFindDescriptor(pDmc, (pDMCallback_t)&DM_Do_Update_SES_PTS_1);
		return;
	}
	
	// Are Updates allowed?
	if (DM_Check_PTS_Update_OK() == 0)
	{
		// Not allowed, Skip update
		DM_Do_Update_SES_PTS_1(pDmc);
		return;
	}
	
	// Update the row this descriptor goes to
	status = DmTableUpdateDeviceDesc(pDmc, (pDMCallback_t)&DM_Do_Update_SES_PTS_1);
	
} // DM_Do_Update_SES_PTS

/*************************************************************************/
// DM_Do_Update_SES_PTS_1
// Update the PathDescriptor with the device data from the last device
// scanned.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Update_SES_PTS_1(DM_CONTEXT *pDmc)
{
	STATUS			status;
	
	DM_TRACE_ENTRY(DM_Do_Update_SES_PTS_1);

	if(pDmc->pDMState->state & DEVICE_STATE_ADD_DD)
	{
		// still need to add the descriptor
		status = DmTableUpdateDeviceDesc(pDmc, (pDMCallback_t)&DM_Do_Update_SES_PTS_1);
		return;
	}	

	// Update the Path this descriptor goes to
	status = DmTableUpdatePD(pDmc, (pDMCallback_t)&DM_Do_Update_SES_PTS_2);
	
} // DM_Do_Update_SES_PTS_1


/*************************************************************************/
// DM_Do_Update_SES_PTS_2
// Update the StorageRollCall with the device data from the last drive
// scanned.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Update_SES_PTS_2(DM_CONTEXT *pDmc)
{
	STATUS			status;
	
	DM_TRACE_ENTRY(DM_Do_Update_SES_PTS_2);

	// Are Updates allowed?
	if (DM_Check_PTS_Update_OK() == 0)
	{
		// Not allowed, Skip update
		DM_Complete_Plex_Request_2((void *)pDmc, ercOK);
		return;
	}
	
	// Update the RollCall entry this descriptor goes to
	status = DmTableUpdateRC(pDmc, (pDMCallback_t)&DM_Complete_Plex_Request_2);
	
} // DM_Do_Update_SES_PTS_2


