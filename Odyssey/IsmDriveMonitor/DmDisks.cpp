/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmDisks.cpp
// 
// Description:
// This module has the code to handle Disk Drive Scans
// 

// Update Log:
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DmDisks.cpp $
// 
// 7     1/11/00 7:27p Mpanas
// New PathTable changes
// 
// 6     10/12/99 6:32p Mpanas
// Fix bug, make sure only chip 2 devices are flagged as Internal
// 
// 5     10/11/99 8:36p Mpanas
// Fix bug, if device remove sometimes report as Bad.
// 
// 3     9/16/99 9:53p Mpanas
// Increase Motor Start Delay to 15 * 5 seconds max to
// account for slow starting motors.
// 
// Add more debug info
// 
// 2     9/15/99 8:00p Mpanas
// make sure we do not retry a dead drive
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
#include "RqOsTimer.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

#include "DriveMonitorIsm.h"
#include "DmCommon.h"
#include "Dm_Messages.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"
#include "FC_Loop.h"


/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// DM_Check_DDH_Slot
// Check to see if we are in slot 24 or 28 (DDH slots) and chip number 2,
// return 1 if we are.
/*************************************************************************/
U32 DriveMonitorIsm::DM_Check_DDH_Slot(void)
{
	TRACE_ENTRY(DM_Check_DDH_Slot);
	
	if ((Address::iSlotMe == IOP_RAC0) || (Address::iSlotMe == IOP_RAC1))
	{
		// only chip 2 has access to the DDHs
		if (FCLOOPCHIP(config.FC_instance) == 2)
		{
			return(1);
		}
	}
	
	// not a DDH slot
	return(0);

} // DM_Check_DDH_Slot

/*************************************************************************/
// DM_Do_Disk
// Inquiry found a Direct Storage type device, start the statemachine for
// disks.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Disk(DM_CONTEXT * pDmc)
{
	DM_DEVICE_STATE	*pDMState = pDmc->pDMState;

	TRACE_ENTRY(DM_Do_Disk);

	if (pDMState->pPD == NULL)
	{
		// need to build a PathDescriptor record
		pDMState->pPD = new PathDescriptor;
		memset(pDMState->pPD, 0, sizeof(PathDescriptor));
	}
	
	if (pDMState->pDD == NULL)
	{
		// need to build a DiskDescriptor record
		pDMState->pDD = new DiskDescriptor;
		memset(pDMState->pDD, 0, sizeof(DiskDescriptor));
		
		// find DiskType by checking if DDH slot
		pDMState->pDD->DiskType = (DM_Check_DDH_Slot()) ? 
							TypeFCDisk: TypeExternalFCDisk;
	}
	else
	{
	    // use this for device found
		pDMState->pDD->CurrentStatus = DriveFound;
	}
	
	if (pDMState->pRC == NULL)
	{
		// need to build a StorageRollCall record
		pDMState->pRC = new StorageRollCallRecord;
		memset(pDMState->pRC, 0, sizeof(StorageRollCallRecord));
	}
	
	// copy the inquiry data to the DiskDescriptor Table record
	memcpy(&pDMState->pDD->InqData,
			pDmc->buffer,
			sizeof(INQUIRY));
			
	// delete the memory allocated
	delete pDmc->buffer;
	pDmc->buffer = NULL;
	
	pDmc->retries = 0;
		
	// next see if the drive is spinning
	DM_Do_Test_Unit(pDmc);
	
} // DM_Do_Disk



/*************************************************************************/
// DM_Do_Test_Unit
// Check if drive is spinning and ready
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Test_Unit(DM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;
	STATUS			 status;
	
	DM_TRACE_ENTRY(DM_Do_Test_Unit);

	// build the test unit ready message
	DM_Build_SCSI_Message( pMsg, CMD_TEST_UNIT, 0, 0,
					pDmc->drive_number, pDmc->lun_number);
	
	status = Send((VDN)config.vd, (Message *)pDmc->pMsg, (void *)pDmc,
					(ReplyCallback)&DM_Do_Test_Unit_Callback);
	
 	if (status != NU_SUCCESS)
		DM_Log_Error(DM_ERROR_TYPE_FATAL,
			"DM_Do_Test_Unit", 
			"Send failed",
			config.xlate[pDmc->drive_number],
			(UNSIGNED)status);

} // DM_Do_Test_Unit

/*************************************************************************/
// DM_Do_Test_Unit_Callback
// Finish the Test Unit, check to see if drive was ready.  If not, send a
// Motor Start command.
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Do_Test_Unit_Callback(Message *pMsg)
{
	DM_CONTEXT 		*pDmc = (DM_CONTEXT *)pMsg->GetContext();
	Message 		*pRf = (Message *)pMsg;
	DM_DEVICE_STATE	*pDMState = pDmc->pDMState;
	
	TRACE_ENTRY(DM_Do_Test_Unit_Callback);

	// find out if this drive is ready or not
	if (pRf->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		SCB_REPLY_PAYLOAD	*pP = (SCB_REPLY_PAYLOAD *)pMsg->GetPPayload();
		REQUEST_SENSE		*pRS = (REQUEST_SENSE *)&pP->SenseData[0];
		
		if ((++pDmc->retries < 3) &&
			(pRf->DetailedStatusCode != FCP_SCSI_HBA_DSC_SELECTION_TIMEOUT))
		{
			// try again to clear the check status
			DM_Do_Test_Unit(pDmc);
			return(0);
		}
	
		// Drive is not ready
		TRACEF(TRACE_L3, ("\n\rDrive Not Ready, Id=%d, LUN=%d, status=%X", 
							config.xlate[pDmc->drive_number],
							pDmc->lun_number,
							pRf->DetailedStatusCode));
		
		DM_Display_Sense(pMsg);

		// mark new status					
		pDMState->pDD->CurrentStatus = DriveSpinningUp;

		// drive not ready, try to spin up
		DM_Do_Start_Unit(pDmc);
	}
	else
	{
		// Drive is ready
		TRACEF(TRACE_L3, ("\n\rDrive Ready, Id=%d, LUN=%d, status=%X", 
							config.xlate[pDmc->drive_number],
							pDmc->lun_number,
							pRf->DetailedStatusCode));
		
		// mark good status to save time later
		pDMState->pDD->CurrentStatus = DriveReady;

		pDmc->retries = 0;
		
		// if drive is ready, complete this device
		DM_Complete_Plex_Request(pDmc, 0);
	}
	
	return(0);
	
} // DM_Do_Test_Unit_Callback
			
	
/*************************************************************************/
// DM_Do_Start_Unit
// Make a drive ready by spinning it up.  The spinup is done as an
// immediate command.  We delay 2 seconds before starting the next drive.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Start_Unit(DM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;
	CDB6			*pStUn;
	SCB_PAYLOAD		*pP;
	STATUS			 status;
	
	DM_TRACE_ENTRY(DM_Do_Start_Unit);

	// build the start unit message
	DM_Build_SCSI_Message( pMsg, CMD_START_UNIT, 0, 0,
					pDmc->drive_number, pDmc->lun_number);
	
	pP = (SCB_PAYLOAD *) pMsg->GetPPayload();
	pStUn = (CDB6 *) &pP->CDB;
	pStUn->Length = 1; // set Start bit
	pStUn->MSB = 1;  // set IMMED bit
	
	status = Send((VDN)config.vd, (Message *)pDmc->pMsg, (void *)pDmc,
					(ReplyCallback)&DM_Do_Start_Unit_Callback);
	
 	if (status != NU_SUCCESS)
		DM_Log_Error(DM_ERROR_TYPE_FATAL,
			"DM_Do_Start_Unit", 
			"Send failed",
			config.xlate[pDmc->drive_number],
			(UNSIGNED)status);

} // DM_Do_Start_Unit

/*************************************************************************/
// DM_Do_Start_Unit_Callback
// Finish the start unit command, do a 2 second delay to allow the drive
// stabilize before spinning up.  Later on, we will check again for readyness.
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Do_Start_Unit_Callback(Message *pMsg)
{
	DM_CONTEXT 		*pDmc = (DM_CONTEXT *)pMsg->GetContext();
	RqOsTimerStart	*pStartTimerMsg;
	
	TRACE_ENTRY(DM_Do_Start_Unit_Callback);

	// delay some here, 2-3 seconds to allow drive power to stabilize
	pStartTimerMsg = new RqOsTimerStart(2000000, 0);
	Send(pStartTimerMsg, (void *)pDmc,
						(ReplyCallback)&DM_Do_Start_Unit_Timer_Callback);
	
	// Drive spinup started
	TRACEF(TRACE_L3, ("\n\rDrive spinup started, Id=%d, LUN=%d, status=%X", 
						config.xlate[pDmc->drive_number],
						pDmc->lun_number,
						pMsg->DetailedStatusCode));
		
	return(0);
	
} // DM_Do_Start_Unit_Callback
			

/*************************************************************************/
// DM_Do_Start_Unit_Timer_Callback
// Finish the start unit command
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Do_Start_Unit_Timer_Callback(Message *pMsg)
{
	DM_CONTEXT *pDmc = (DM_CONTEXT *)pMsg->GetContext();

	TRACE_ENTRY(DM_Do_Start_Unit_Timer_Callback);

	// Drive spinup timeout done
	TRACEF(TRACE_L3, ("\n\rDrive Spinup Timeout Done, Id=%d, LUN=%d", 
						config.xlate[pDmc->drive_number],
						pDmc->lun_number));
		
	// drive spinning up, complete this plex
	DM_Complete_Plex_Request(pDmc, 0);
	
	return(0);
	
} // DM_Do_Start_Unit_Timer_Callback


	
/*************************************************************************/
// DM_Do_Test_Unit_Wait
// Wait until drive is spinning and ready or time out
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Test_Unit_Wait(DM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;
	DM_DEVICE_STATE	*pDMState = pDmc->pDMState;
	STATUS			 status;
	
	DM_TRACE_ENTRY(DM_Do_Test_Unit_Wait);

	// check to see if we already failed this disk
	if (pDMState->state & DEVICE_STATE_DEVICE_FAILURE)
	{
		// don't even try to wait for ready, drive is dead jim
		// move on to next device
		DM_Complete_Plex_Request_2(pDmc, 0);
		return;
	}

	// build the test unit ready message
	DM_Build_SCSI_Message( pMsg, CMD_TEST_UNIT, 0, 0,
					pDmc->drive_number, pDmc->lun_number);
	
	pMsg->DetailedStatusCode = 0;
	
	status = Send((VDN)config.vd, (Message *)pDmc->pMsg, (void *)pDmc,
					(ReplyCallback)&DM_Do_Test_Unit_Wait_Callback);
	
 	if (status != NU_SUCCESS)
		DM_Log_Error(DM_ERROR_TYPE_FATAL,
			"DM_Do_Test_Unit_Wait", 
			"Send failed",
			config.xlate[pDmc->drive_number],
			(UNSIGNED)status);

} // DM_Do_Test_Unit_Wait

/*************************************************************************/
// DM_Do_Test_Unit_Wait_Callback
// Finish the Test Unit Wait
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Do_Test_Unit_Wait_Callback(Message *pMsg)
{
	DM_CONTEXT 		*pDmc = (DM_CONTEXT *)pMsg->GetContext();
	Message 		*pRf = (Message *)pMsg;
	DM_DEVICE_STATE	*pDMState = pDmc->pDMState;
	RqOsTimerStart*		pStartTimerMsg;
	
	TRACE_ENTRY(DM_Do_Test_Unit_Wait_Callback);

	// find out if this drive is ready or not
	if (pRf->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		// Drive is not ready
		TRACEF(TRACE_L3, ("\n\rDrive Not Ready after wait, Id=%d, LUN=%d, status=%X", 
							config.xlate[pDmc->drive_number],
							pDmc->lun_number,
							pRf->DetailedStatusCode));
		DM_Display_Sense(pMsg);
		
    	// try again?
		if ((pDmc->retries++ >= 15) ||
			(pRf->DetailedStatusCode == FCP_SCSI_HBA_DSC_SELECTION_TIMEOUT))
		{
			// Motor Start failed
			pDmc->num_bad++;
			// mark table
			pDMState->pDD->CurrentStatus = DriveHardFailure;
			// and exit (try next device)
			DM_Complete_Plex_Request_2(pDmc, DriveHardFailure);
			return(0);
		}
		
		// drive not ready, delay 5 seconds and try again
		pStartTimerMsg = new RqOsTimerStart(5000000, 0);
		Send(pStartTimerMsg, (void *)pDmc,
						(ReplyCallback)&DM_Do_Test_Unit_Timer_Callback);
	}
	else
	{
		// show good status
		pDMState->pDD->CurrentStatus = DriveReady;

		// Drive is ready
		TRACEF(TRACE_L3, ("\n\rDrive Ready, Id=%d, LUN=%d", 
						config.xlate[pDmc->drive_number],
						pDmc->lun_number));
		
		pDmc->retries = 0;
		
		// if drive is ready, complete this device by reading the capacity
		DM_Do_Read_Capacity(pDmc);
	}
	
	return(0);
	
} // DM_Do_Test_Unit_Wait_Callback


/*************************************************************************/
// DM_Do_Test_Unit_Timer_Callback
// Finish the test unit command
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Do_Test_Unit_Timer_Callback(Message *pMsg)
{
	DM_CONTEXT *pDmc = (DM_CONTEXT *)pMsg->GetContext();

	TRACE_ENTRY(DM_Do_Test_Unit_Timer_Callback);

	// try again
	DM_Do_Test_Unit_Wait(pDmc);
	
	return(0);
	
} // DM_Do_Test_Unit_Timer_Callback



/*************************************************************************/
// DM_Do_Read_Capacity
// Read the actual drive capacity
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Read_Capacity(DM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;	// faster access
	STATUS			 status;

	DM_TRACE_ENTRY(DM_Do_Read_Capacity);

	// allocate a buffer for the read capacity data
	pDmc->buffer = new (tSMALL|tUNCACHED) READ_CAPACITY;
	
	// build the Read Capacity message
	DM_Build_SCSI_Message(pMsg, CMD_READ_CAPACITY, pDmc->buffer,
					sizeof(READ_CAPACITY), pDmc->drive_number,
					pDmc->lun_number);
	
	// handle the capacity data when the reply comes back
	status = Send((VDN)config.vd, (Message *)pDmc->pMsg, (void *)pDmc,
					(ReplyCallback)&DM_Do_Read_Capacity_Callback);
	
 	if (status != NU_SUCCESS)
		DM_Log_Error(DM_ERROR_TYPE_FATAL,
			"DM_Do_Read_Capacity", 
			"Send failed",
			config.xlate[pDmc->drive_number],
			(UNSIGNED)status);

} // DM_Do_Read_Capacity

/*************************************************************************/
// DM_Do_Read_Capacity_Callback
// Finish (or continue) the Read a sector from a drive function
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Do_Read_Capacity_Callback(Message *pMsg)
{
	DM_CONTEXT 			*pDmc = (DM_CONTEXT *)pMsg->GetContext();
	Message 			*pRf = (Message *)pMsg;
	SCB_REPLY_PAYLOAD	*p_rp = (SCB_REPLY_PAYLOAD *)pMsg->GetPPayload();
	DM_DEVICE_STATE		*pDMState = pDmc->pDMState;
	
	TRACE_ENTRY(DM_Do_Read_Capacity_Callback);

	if (pRf->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		DM_PRINT_HEX16(TRACE_L3, "\n\rRead Cap Failed, Id=",
									config.xlate[pDmc->drive_number]);
		
		DM_Display_Sense(pMsg);

		// Read Capacity failed
    	// try again
		if (pDmc->retries++ == 3)
		{
			// Read Capacity failed
			pDMState->pDD->CurrentStatus = DriveHardFailure;
			pDmc->num_bad++;
			goto RD_CAP_FAILED;
		}
		
		delete pDmc->buffer;
		pDmc->buffer = NULL;
		DM_Do_Read_Capacity(pDmc);
		return(0);
	}
	else
	{
		TRACEF(TRACE_L8, ("\n\rDriveMonitorIsm: Capacity data, Id=%d, LUN=%d", 
							config.xlate[pDmc->drive_number],
							pDmc->lun_number));
	    DM_DUMP_HEX(TRACE_L8, " ",
	    					(U8 *)pDmc->buffer,
	    					sizeof(READ_CAPACITY));
	    
	    // check for no capacity (happens on first try?)
	    if ((((READ_CAPACITY *)pDmc->buffer)->BlockAddress) == 0)
	    {
	    	// try again
			if (pDmc->retries++ == 3)
			{
				// Read Capacity failed
				pDMState->pDD->CurrentStatus = DriveHardFailure;
				pDmc->num_bad++;
				goto RD_CAP_FAILED;
			}
			
			delete pDmc->buffer;
			pDmc->buffer = NULL;
			DM_Do_Read_Capacity(pDmc);
			return(0);
		
	    }
		// copy capacity data to the disk table
		// if drive is ready
		pDMState->pDD->Capacity.HighPart = 0;
		pDMState->pDD->Capacity.LowPart =
			(U32) ((READ_CAPACITY *)pDmc->buffer)->BlockAddress;
			
		pDMState->pDD->CurrentStatus = DriveReady;
	}
	
	// delete the memory allocated for the Capacity data
	delete pDmc->buffer;
	pDmc->buffer = NULL;
	
	// try to read the serial number next
	DM_Do_Inquiry_Serial_Number(pDmc);
	return(0);
	
	// GetCapacity failed, delete the memory allocated for the
	// Capacity data and post the error. Don't read the serial
	// number.
RD_CAP_FAILED:
	delete pDmc->buffer;
	pDmc->buffer = NULL;
	
	DM_Complete_Plex_Request_2(pDmc, DriveHardFailure);
	
	return(0);
	
} // DM_Do_Read_Capacity_Callback
			
	
/*************************************************************************/
// DM_Do_Inquiry_Serial_Number
// Read the serial number of a drive using an extended inquiry SCSI command
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Inquiry_Serial_Number(DM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;
	INQUIRY_CMD		*pInq;
	SCB_PAYLOAD		*pP;
	STATUS			 status;
	
	DM_TRACE_ENTRY(DM_Do_Inquiry_Serial_Number);

	// allocate a buffer for the serial number data
	pDmc->buffer = new (tSMALL|tUNCACHED) INQUIRY_SERIAL_NUMBER;
	
	// build the inquiry message
	DM_Build_SCSI_Message(pMsg, CMD_INQUIRY, pDmc->buffer,
					sizeof(INQUIRY_SERIAL_NUMBER), pDmc->drive_number,
					pDmc->lun_number);
	
	pP = (SCB_PAYLOAD *) pMsg->GetPPayload();
	pInq = (INQUIRY_CMD *) &pP->CDB;
	pInq->EVPD = 1;
	pInq->PageCode = INQ_PAGE_CODE_SERIAL_NUMBER;	// get serial number page
	
	// handle the inquiry serial number data when the reply comes back
	status = Send((VDN)config.vd, (Message *)pDmc->pMsg, (void *)pDmc,
					(ReplyCallback)&DM_Do_Inquiry_Serial_Number_Callback);
	
 	if (status != NU_SUCCESS)
		DM_Log_Error(DM_ERROR_TYPE_FATAL,
			"DM_Do_Inquiry_Serial_Number", 
			"Send failed",
			config.xlate[pDmc->drive_number],
			(UNSIGNED)status);

} // DM_Do_Inquiry_Serial_Number

/*************************************************************************/
// DM_Do_Inquiry_Serial_Number_Callback
// Finish DM_Do_Inquiry_Serial_Number
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Do_Inquiry_Serial_Number_Callback(Message *pMsg)
{
	DM_CONTEXT 			*pDmc = (DM_CONTEXT *)pMsg->GetContext();
	Message 			*pRf = (Message *)pMsg;
	DM_DEVICE_STATE		*pDMState = pDmc->pDMState;
	
	TRACE_ENTRY(DM_Do_Inquiry_Serial_Number_Callback);

	// check status, may not work on some drives
	if (pRf->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		// no serial number
		TRACEF(TRACE_L8, ("\n\rNo serial number, Id=%d, LUN=%d", 
							config.xlate[pDmc->drive_number],
							pDmc->lun_number));
		
		DM_Display_Sense(pMsg);

		// mark table (not an error if no serial number)
		pDMState->pDD->SerialNumber[0] = 0;
	}
	else
	{
		INQUIRY_SERIAL_NUMBER	*p_is = (INQUIRY_SERIAL_NUMBER *) pDmc->buffer;
		
		TRACEF(TRACE_L8, ("\n\rDriveMonitorIsm: Inquiry Serial data, Id=%d, LUN=%d", 
							config.xlate[pDmc->drive_number],
							pDmc->lun_number));
	    DM_DUMP_HEX(TRACE_L8, " ",
	    					(U8 *)pDmc->buffer,
	    					sizeof(INQUIRY_SERIAL_NUMBER));
		// copy the serial number data to the Disk Object Table
		memcpy(&pDMState->pDD->SerialNumber[0],
				&p_is->ProductSerialNumber[0],
				p_is->PageLength);
		pDMState->pDD->SerialNumber[p_is->PageLength] = 0;
		// SN now valid
		pDMState->pDD->fSNValid = 1;
	
	}
	
	// delete the memory allocated for the serial number
	delete pDmc->buffer;
	pDmc->buffer = NULL;
	
	// check to see if we want to print the info for this drive
	if ((pDMState->pDD->CurrentStatus == DriveReady) &&
					((pDmc->flags & PLEX_LUN_REQUEST) == 0))
	{
#if defined(DM_DEBUG) && defined(_DEBUG)
		char			s[128];

		TRACEF(TRACE_L2, ("\n\rFound Disk, Id=%d, LUN=%d", 
							config.xlate[pDmc->drive_number],
							pDmc->lun_number));

		// Build a readable string
		s[0] = '\n';
		s[1] = '\r';
		memcpy(&s[2],
				&pDMState->pDD->InqData.VendorId[0],
				28 );
		s[30] = 0;
		
		DM_PRINT_STRING(TRACE_L2, s);
		DM_PRINT_HEX(TRACE_L3, "\n\rCap = ",
				pDMState->pDD->Capacity.LowPart);
		DM_PRINT_STRING(TRACE_L2, "\n\r");

		DM_DUMP_HEX(TRACE_L8, " = ",
				(U8 *)pDMState->pDD,
				sizeof(DiskDescriptor));
#endif
	}
	
	if (pDMState->pDD->CurrentStatus == DriveReady)
	{
		// bump the number of good drives found in this plex
		pDmc->num_good++;
	}
	
	// update the PTS entries for the DiskDescriptor and (maybe)
	// the StorageRollCall
	DM_Do_Update_PTS(pDmc);
	
	return(0);
	
} // DM_Do_Inquiry_Serial_Number_Callback
			
	
/*************************************************************************/
// DM_Do_Update_PTS
// Update the DiskDescriptor with the device data from the last drive
// scanned.  Check if an update is allowed before the update, skip if not.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Update_PTS(DM_CONTEXT *pDmc)
{
	STATUS			status;
	
	DM_TRACE_ENTRY(DM_Do_Update_PTS);

	// check if we need to find our descriptor
	// must be failover and new device
	if ((config.flags & DM_FLAGS_REDUNDANT) &&
							(pDmc->pDMState->state & DEVICE_STATE_NEW_DEVICE))
	{
		DmFindDescriptor(pDmc, (pDMCallback_t)&DM_Do_Update_PTS_1);
		return;
	}
	
	// Are Updates allowed?
	if (DM_Check_PTS_Update_OK() == 0)
	{
		// Not allowed, Skip update
		DM_Do_Update_PTS_1(pDmc);
		return;
	}
	
	// Update the row this descriptor goes to
	status = DmTableUpdateDD(pDmc, (pDMCallback_t)&DM_Do_Update_PTS_1);
	
} // DM_Do_Update_PTS


/*************************************************************************/
// DM_Do_Update_PTS_1
// Update the PathDescriptor with the device data from the last drive
// scanned.  Path is always updated.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Update_PTS_1(DM_CONTEXT *pDmc)
{
	STATUS			status;
	
	DM_TRACE_ENTRY(DM_Do_Update_PTS_1);

	if(pDmc->pDMState->state & DEVICE_STATE_ADD_DD)
	{
		// still need to add the descriptor
		status = DmTableUpdateDD(pDmc, (pDMCallback_t)&DM_Do_Update_PTS_1);
		return;
	}	

	// Update the Path this descriptor goes to
	status = DmTableUpdatePD(pDmc, (pDMCallback_t)&DM_Do_Update_PTS_2);
	
} // DM_Do_Update_PTS_1


/*************************************************************************/
// DM_Do_Update_PTS_2
// Update the StorageRollCall with the device data from the last drive
// scanned.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Update_PTS_2(DM_CONTEXT *pDmc)
{
	STATUS			status;
	
	DM_TRACE_ENTRY(DM_Do_Update_PTS_2);

	// Are Updates allowed?
	if (DM_Check_PTS_Update_OK() == 0)
	{
		// Not allowed, Skip update
		DM_Complete_Plex_Request_2((void *)pDmc, ercOK);
		return;
	}
	
	// TODO: do we really want to skip the update?  Not when RollCall
	//       has a status or state field.
	// if the CurrentStatus of this DiskDesc row is not DriveReady,
	// we are done here. If it is DriveReady, add a Storage Roll Call
	// entry for this disk.
	if (pDmc->pDMState->pDD->CurrentStatus != DriveReady)
	{
		DM_Complete_Plex_Request_2((void *)pDmc, ercOK);
		return;
	}
	
	// Update the RollCall entry this descriptor goes to
	status = DmTableUpdateRC(pDmc, (pDMCallback_t)&DM_Complete_Plex_Request_2);
	
} // DM_Do_Update_PTS_2



/*************************************************************************/
// DM_Start_One_Disk
// Start the Motor Start State Machine for one disk.
// We were called from an incoming message, perform a motor start and then
// reply to caller.
/*************************************************************************/
void DriveMonitorIsm::DM_Start_One_Disk(Message *pMsg)
{
	DM_CONTEXT		*pDmc;
	DmStart			*pDS = (DmStart *) pMsg;
		
	DM_TRACE_ENTRY(DM_Start_One_Disk);

	// allocate our context
	pDmc = new (tSMALL|tUNCACHED) DM_CONTEXT;
	// allocate a message to go with it
	pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE));
	
	// pass the reply pointer along (if any)
	pDmc->pReply = pMsg;	
	
	// setup drive number index (from message)
	pDmc->drive_number = pDS->payload.drive;
	pDmc->lun_number = 0;
	pDmc->start = 1;		// start this drive
	
	// set the callback
	pDmc->Callback = (pDMCallback_t) &DM_Do_Finish_Msg;
	
	DM_Do_Start_Stop_One_Disk(pDmc);
	
} // DM_Start_One_Disk
	

/*************************************************************************/
// DM_Stop_One_Disk
// Start the Motor Spin Down State Machine for one disk.
// If pMsg is non-zero, this means we were called from an incoming message
// then we must send a reply back to the caller on the last disk
/*************************************************************************/
void DriveMonitorIsm::DM_Stop_One_Disk(Message *pMsg)
{
	DM_CONTEXT		*pDmc;
	DmStop			*pDS = (DmStop *) pMsg;
	
	DM_TRACE_ENTRY(DM_Stop_One_Disk);

	// allocate our context
	pDmc = new (tSMALL|tUNCACHED) DM_CONTEXT;
	// allocate a message to go with it
	pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE));
	
	// pass the reply pointer along (if any)
	pDmc->pReply = pMsg;	
	
	// setup drive number index (from message)
	pDmc->drive_number = pDS->payload.drive;
	pDmc->lun_number = 0;
	pDmc->start = 0;		// stop this drive
	
	// set the callback
	pDmc->Callback = (pDMCallback_t) &DM_Do_Finish_Msg;
	
	DM_Do_Start_Stop_One_Disk(pDmc);
	
} // DM_Stop_One_Disk


/*************************************************************************/
// DM_Stop_All_Disks
// Start the Motor Spin Down State Machine for each disk found and
// spinning.  Reply to caller when done.
/*************************************************************************/
void DriveMonitorIsm::DM_Stop_All_Disks(Message *pMsg)
{
	DM_CONTEXT		*pDmc, *pMDmc;
	DM_DEVICE_STATE	*pDMState;
	
	DM_TRACE_ENTRY(DM_Stop_All_Disks);

	// allocate our master context
	pMDmc = new (tSMALL|tUNCACHED) DM_CONTEXT;
	// allocate a message to go with it
	pMDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE));
	
	// pass the reply pointer along
	pMDmc->pReply = pMsg;	
	
	pMDmc->num_devices = 0;
	
	// loop for all devices found
	// unroll the DM_DEVICE_STATE containers, spin down the
	// motor on all disk type devices
	for (int x = pDD->Count(); x--;)
	{
		// get each container in sequence
		pDD->GetAt((CONTAINER_ELEMENT &)pDMState, x);
		
		// check if the device is still active
		if ((pDMState->type == SCSI_DEVICE_TYPE_DIRECT) ||
				(pDMState->type == SCSI_DEVICE_TYPE_ARRAY_CONT))
		{
			// bump the count of devices
			pMDmc->num_devices++;
			
			// pass on our master pointer
			pDmc->pDmc = pMDmc;	

			// allocate a context to use for this spindown
			pDmc = new (tSMALL|tUNCACHED) DM_CONTEXT;
			// allocate a message to go with it
			pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE));
			
			// no reply
			pDmc->pReply = NULL;	
			
			// setup drive device numbers
			pDmc->drive_number = pDMState->pPD->FCTargetID;
			pDmc->lun_number = pDMState->pPD->FCTargetLUN;
			pDmc->start = 0;		// stop this drive
			
			// set the callback
			pDmc->Callback = (pDMCallback_t) &DM_Stop_All_Disks_Callback;
			
			DM_Do_Start_Stop_One_Disk(pDmc);

		}
		else
		{
			// not a disk, do next device
			continue;
		}
	}

	// check for any spin downs
	if (pMDmc->num_devices == 0)
	{
		// none, reply back
		DM_Do_Finish_Msg((void *)pMDmc, FCP_SCSI_HBA_DSC_SUCCESS);
	}

} // DM_Stop_All_Disks


/*************************************************************************/
// DM_Stop_All_Disks_Callback
// Done with processing this Message, send a reply and delete context
/*************************************************************************/
void DriveMonitorIsm::DM_Stop_All_Disks_Callback(void *p, U32 status)
{
	DM_CONTEXT		*pDmc = (DM_CONTEXT *) p;		// this request
	DM_CONTEXT		*pODmc = pDmc->pDmc;			// orig request

	DM_TRACE_ENTRY(DM_Stop_All_Disks_Callback);

	// we are done with this context
	// delete memory allocated for message
	delete (Message *)pDmc->pMsg;
	
	// delete the context
	delete pDmc;

	// check if we are done spinning down all the disks
	if (--pODmc->num_devices)
	{
		DM_Do_Finish_Msg((void *)pODmc, FCP_SCSI_HBA_DSC_SUCCESS);
	}

} // DM_Stop_All_Disks_Callback



/*************************************************************************/
// DM_Do_Start_Stop_One_Disk
// Start or Stop a single disk, perform Callback when done. This a small
// statemachine that calls a timer when spinning disks up.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Start_Stop_One_Disk(DM_CONTEXT *pDmc)
{
	STATUS			status;
	CDB6			*pStUn;
	SCB_PAYLOAD		*pP;

	DM_TRACE_ENTRY(DM_Do_Start_Stop_One_Disk);

	// build the start unit message
	DM_Build_SCSI_Message( pDmc->pMsg, CMD_START_UNIT, 0, 0,
					pDmc->drive_number, pDmc->lun_number);
	
	pP = (SCB_PAYLOAD *) pDmc->pMsg->GetPPayload();
	pStUn = (CDB6 *) &pP->CDB;
	pStUn->Length = pDmc->start;	// set Start bit
	pStUn->MSB = 1;  				// set IMMED bit
	
	// start just this unit
	status = Send((VDN)config.vd, (Message *)pDmc->pMsg, (void *)pDmc,
					(ReplyCallback)&DM_Do_Start_Stop_One_Unit_Callback);
	
 	if (status != NU_SUCCESS)
		DM_Log_Error(DM_ERROR_TYPE_FATAL,
			"DM_Do_Inquiry_Serial_Number", 
			"Send failed",
			config.xlate[pDmc->drive_number],
			(UNSIGNED)status);

} // DM_Do_Start_Stop_One_Disk


/*************************************************************************/
// DM_Do_Start_Stop_One_Unit_Callback
// Finish the start unit command, do a 2 second delay to allow the drive
// stabilize before spinning up.  Later on, we will check again for readyness.
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Do_Start_Stop_One_Unit_Callback(Message *pMsg)
{
	DM_CONTEXT 			*pDmc = (DM_CONTEXT *)pMsg->GetContext();
	RqOsTimerStart		*pStartTimerMsg;
	
	TRACE_ENTRY(DM_Do_Start_Stop_One_Unit_Callback);

	if (pDmc->start)
	{
		// delay some here, 2-3 seconds to allow drive power to stabilize
		pStartTimerMsg = new RqOsTimerStart(3000000, 0);
		Send(pStartTimerMsg, (void *)pDmc,
							(ReplyCallback)&DM_Do_Start_One_Unit_Timer_Callback);
		
		// Drive spinup started
		DM_PRINT_HEX16(TRACE_L3, "\n\rDrive Spinup started, Id=",
							config.xlate[pDmc->drive_number]);
	}
	else
	{
		// must be a Stop, no delay needed,
		// just call the callback if there is one
		if (pDmc->Callback)
			(this->*pDmc->Callback)((void *)pDmc, 0);
	}
	
	return(0);
	
} // DM_Do_Start_Stop_One_Unit_Callback


/*************************************************************************/
// DM_Do_Start_One_Unit_Timer_Callback
// Finish the start unit command
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Do_Start_One_Unit_Timer_Callback(Message *pMsg)
{
	DM_CONTEXT *pDmc = (DM_CONTEXT *)pMsg->GetContext();

	TRACE_ENTRY(DM_Do_Start_One_Unit_Timer_Callback);

	// Drive spinup timeout done
	DM_PRINT_HEX16(TRACE_L3, "\n\rDrive Spinup Timeout Done, Id=",
						config.xlate[pDmc->drive_number]);
	
	// drive spinning up, complete the processing of the original Caller
	// Do a callback if there is one
	if (pDmc->Callback)
		(this->*pDmc->Callback)((void *)pDmc, 0);
	
	return(0);
	
} // DM_Do_Start_One_Unit_Timer_Callback


	

/*************************************************************************/
// DM_Do_Finish_Msg
// Done with processing this Message, send a reply and delete context
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Finish_Msg(void *p, U32 status)
{
	DM_CONTEXT		*pDmc = (DM_CONTEXT *) p;

	DM_TRACE_ENTRY(DM_Do_Finish_Msg);

	// check for reply needed
	if (pDmc->pReply)
	{
		status = Reply(pDmc->pReply, FCP_SCSI_HBA_DSC_SUCCESS);
	}

	// we are done with our context
	// delete the buffer (if used )
	if (pDmc->buffer)
	{
		delete pDmc->buffer;
	}
	
	// delete memory allocated for message
	delete (Message *)pDmc->pMsg;
	
	// delete the context
	delete pDmc;

} // DM_Do_Finish_Msg


