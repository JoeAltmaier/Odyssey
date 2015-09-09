/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmScan.cpp
// 
// Description:
// This module contains the primary and secondary Scan routines.
// All the plex handlers are located in this module.
// 
//
// Update Log:
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DmScan.cpp $
// 
// 12    2/01/00 7:21p Mpanas
// Missed a piece to handle SES and Tape correctly
// 
// 11    2/01/00 2:32p Mpanas
// Fixes for Dell PV discovery
// - Don't use UNTAGGED commmands
// - Fix Plex handling for odd number of devices
// - Handle Inquiry for devices on another loop (NZ device qualifier)
// - Fix SES and Tape handling (Plex handling wrong)
// 
// 10    1/11/00 7:27p Mpanas
// New PathTable changes
// 
// 9     12/22/99 9:27p Jlane
// Print the number of devices in decimal
// 
// 8     12/22/99 7:49p Jlane
// Fix calculation error when DM_FLAGS_SPIN_ALL case
// 
// 7     11/09/99 7:36p Mpanas
// Fix several typos to allow Report LUNS and LUN scans
// to work correctly.
// 
// 6     10/12/99 8:33p Mpanas
// Add support for removed drives
// - Update DiskDescriptor with DriveRemoved
// - correct several bugs, flags not inverted correctly
// - Flag drives that have been changed (with a trace message only)
// 
// 5     10/11/99 8:36p Mpanas
// Fix bug, if device remove sometimes report as Bad.
// 
// 2     9/16/99 9:56p Mpanas
// - If drive is ready, skip the test unit wait
// - changes to support removing an existing drive to
//   update the tables correctly
// + more debug
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
#include "pci.h"
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


/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// DM_Build_Plex_Request
// Build a scan request that may span many devices. Range is specified by
// the start and end device numbers.  Each plex is a unit that is scanned
// in sequence.  Only one context is needed for a plex.  Note: start and
// end are indexes into the config.xlate[] array.  We can have a total
// of 126 devices to scan and each may have LUNs.  In the case of a
// PLEX_LUN_REQUEST, start is the index into the pluns array and end is
// the ending index. The drive_number, num_devices and pluns fields must
// be set externally after this routine is called.
/*************************************************************************/
void *DriveMonitorIsm::DM_Build_Plex_Request(Message *pMsg,
											U32 start, U32 end, U16 flags)
{
	DM_CONTEXT		*pDmc;

	DM_TRACE_ENTRY(DM_Build_Plex_Request);
	
	// allocate our context
	pDmc = new (tSMALL|tUNCACHED) DM_CONTEXT;
	memset(pDmc, 0, sizeof(DM_CONTEXT));
	
	// allocate a message to go with it
	pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE));
	
	// pass the reply pointer along (if any)
	pDmc->pReply = pMsg;	
	
	// Preset the flags
	pDmc->flags = flags;	
	
	// LUN Requests are setup different
	if (pDmc->flags & PLEX_LUN_REQUEST)
	{
		// set begin/end
		pDmc->start = start;
		pDmc->last = end * 2;
		
		// set starting index
		pDmc->index = start;

		// drive_number and num_devices set elsewhere
	}
	else
	{
		// set begin/end
		pDmc->start = start;
		pDmc->last = end;
		
		// the first drive
		pDmc->drive_number = start;
		
		// number of devices
		pDmc->num_devices = end - start + 1;
	}

	pDmc->num_good = 0;
	pDmc->num_bad = 0;

	return((void *)pDmc);
	
} // DM_Build_Plex_Request


/*************************************************************************/
// DM_Do_Plex_Request
// Start the plex statemachine
// Each plex contains a number of device IDs that are scanned
// in sequence.  A timer will be set to go off if any scan hangs up.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Plex_Request(DM_CONTEXT *pDmc)
{
	DM_TRACE_ENTRY(DM_Do_Plex_Request);
	
	// start the timer
	
	// start state machine
	DM_Do_Inquiry(pDmc);

} // DM_Do_Plex_Request


/*************************************************************************/
// DM_Do_Plex_Request_2
// Start the second part of the plex statemachine.  This part handles:
// checking for readyness, reading the capacity and the serial number.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Plex_Request_2(DM_CONTEXT *pDmc)
{
	DM_DEVICE_STATE	*pDMState;
	union {
		IDLUN		 idlun;
		long		 l;
	} sig;

	DM_TRACE_ENTRY(DM_Do_Plex_Request_2);
	
	// LUN Requests are done different
	if (pDmc->flags & PLEX_LUN_REQUEST)
	{
		// start back at the first LUN index
		pDmc->index = pDmc->start;
		pDmc->lun_number = (U16)(pDmc->pluns[pDmc->index] >> 16);
		
		DM_PRINT_STRING(TRACE_L4, " - PLEX_LUN_REQUEST");
	}
	else
	{
		// start back at the first drive
		pDmc->drive_number = pDmc->start;
	}
	
    // build the device key
    sig.idlun.id = (U8)config.xlate[pDmc->drive_number];
    sig.idlun.LUN = (U16)pDmc->lun_number;
  	sig.idlun.HostId = 0;

	// check for an existing device
	if ((pDD->Get((CONTAINER_ELEMENT &)pDMState, (CONTAINER_KEY)sig.l)) == FALSE)
	{
		// no container, skip because no device there
		DM_Complete_Plex_Request_2(pDmc, 0);
		return;
	}
	
	// make sure we are a disk drive
	if ((pDMState->type != SCSI_DEVICE_TYPE_DIRECT) &&
					(pDMState->type != SCSI_DEVICE_TYPE_ARRAY_CONT))
	{
		// not a disk, do second part of SES or Tape
		switch(pDMState->type)
		{
			case SCSI_DEVICE_TYPE_SEQUENTIAL:
			// handle a tape device
			DM_Do_Update_Tape_PTS(pDmc);
			return;
			
			case SCSI_DEVICE_TYPE_ENCLOSURE_SERVICES:
			// handle an SES device
			DM_Do_Update_SES_PTS(pDmc);
			return;
			
			default:
			break;
		}
		
		// just ignore this device
		DM_Complete_Plex_Request_2(pDmc, 0);
		return;
	}
	
	// pass on the DEVICE_STATE for the rest of this statemachine
	pDmc->pDMState = pDMState;
	
	// wait for last disk type device to spin up (if any)
	// then start Read Cap state machine
	DM_Do_Test_Unit_Wait(pDmc);

} // DM_Do_Plex_Request_2


/*************************************************************************/
// DM_Complete_Plex_Request
// Come here afer each scan to see if we are done yet.
/*************************************************************************/
void DriveMonitorIsm::DM_Complete_Plex_Request(void *p, U32 status)
{
	DM_CONTEXT		*pDmc = (DM_CONTEXT *) p;

	DM_TRACE_ENTRY(DM_Complete_Plex_Request);
	
	// LUN Requests are done different
	if (pDmc->flags & PLEX_LUN_REQUEST)
	{
		DM_PRINT_STRING(TRACE_L4, " - PLEX_LUN_REQUEST");

		// check if this LUN plex is done
		if (pDmc->index == pDmc->last)
		{
			// all LUNs scanned, wait for each disk type
			// device to spin up.
			DM_Do_Plex_Request_2(pDmc);
			return;
		}

		// not yet, do next LUN
		pDmc->index++;
		pDmc->index++;	// skip 8 bytes each
		pDmc->lun_number = (U16)(pDmc->pluns[pDmc->index] >> 16);
		
	}
	else
	{
		// are we done?
		if (pDmc->drive_number == pDmc->last)
		{
			// all devices scanned, wait for each disk type
			// device to spin up.
			DM_Do_Plex_Request_2(pDmc);
			return;
		}
		
		// not yet, do next ID
		pDmc->drive_number++;
		
	}

	pDmc->retries = 0;
		
	// do next device scan
	DM_Do_Inquiry(pDmc);

} // DM_Complete_Plex_Request


/*************************************************************************/
// DM_Complete_Plex_Request_2
// Come here afer each piece of the second part of the scan to see if we
// are done yet.
/*************************************************************************/
void DriveMonitorIsm::DM_Complete_Plex_Request_2(void *p, U32 status)
{
	DM_CONTEXT		*pDmc1, *pDmc = (DM_CONTEXT *) p;
	DM_DEVICE_STATE	*pDMState;
	union {
		IDLUN		 idlun;
		long		 l;
	} sig;

	DM_TRACE_ENTRY(DM_Complete_Plex_Request_2);
	
trynextdevice:

	// LUN Requests are done different
	if (pDmc->flags & PLEX_LUN_REQUEST)
	{
		DM_PRINT_STRING(TRACE_L4, " - PLEX_LUN_REQUEST");

		// check if this LUN plex is done
		if (pDmc->index == pDmc->last)
		{
			// all LUNs in LUN plex scanned, restore original
			// context & delete LUN plex context
			TRACEF(TRACE_L3, ("\n\rNum Good LUN Drives = %d",
					pDmc->num_good));
			TRACEF(TRACE_L3, ("\n\rNum Bad LUN Drives = %d",
					pDmc->num_bad));
	
			pDmc1 = pDmc->pDmc;
			
			// we are done with the plex context
			// delete the buffer (if used )
			if (pDmc->buffer)
			{
				delete pDmc->buffer;
			}
			
			// delete memory allocated for message
			delete (Message *)pDmc->pMsg;
			
			// delete the LUN context
			delete pDmc;
			
			// do next ID, if any
			DM_Complete_Plex_Request(pDmc1, 0);
			return;
		}

		// not yet, do next LUN
		pDmc->index++;
		pDmc->index++;	// skip 8 bytes each
		pDmc->lun_number = (U16)(pDmc->pluns[pDmc->index] >> 16);
		
	}
	else
	{
		// are we there yet?
		if (pDmc->drive_number == pDmc->last)
		{
			DM_Do_Finish(pDmc);
			return;
		}
		
		// not yet, do next device
		pDmc->drive_number++;
	}

    // build the device key
    sig.idlun.id = (U8)config.xlate[pDmc->drive_number];
    sig.idlun.LUN = (U16)pDmc->lun_number;
  	sig.idlun.HostId = 0;

	// better be an existing device
	if ((pDD->Get((CONTAINER_ELEMENT &)pDMState, (CONTAINER_KEY)sig.l)) == FALSE)
	{
		// no container, barf here
		goto trynextdevice;
	}
	
	// pass on the DEVICE_STATE for the rest of this statemachine
	pDmc->pDMState = pDMState;

	// make sure we are a disk drive
	if ((pDMState->type != SCSI_DEVICE_TYPE_DIRECT) &&
					(pDMState->type != SCSI_DEVICE_TYPE_ARRAY_CONT))
	{
		// not a disk, see if we are a Tape or SES
		switch(pDMState->type)
		{
			case SCSI_DEVICE_TYPE_SEQUENTIAL:
			// handle a tape device
			DM_Do_Update_Tape_PTS(pDmc);
			return;
			
			case SCSI_DEVICE_TYPE_ENCLOSURE_SERVICES:
			// handle an SES device
			DM_Do_Update_SES_PTS(pDmc);
			return;
			
			default:
			break;
		}
		
		// not a disk, tape or SES, skip because no more work for this device
		goto trynextdevice;
	}
	
	DM_PRINT_HEX(TRACE_L3, "\n\rDM_Complete_Plex_Request_2: CurrentStatus ",
					pDMState->pDD->CurrentStatus);
	
	pDmc->retries = 0;
		
	// for disks, check exiting status, must be READY or
	// SPINNINGUP to continue
	switch (pDMState->pDD->CurrentStatus)
	{
		case DriveReady:
			// drive already proven to be ready
			// start Read Cap state machine directly
			DM_Do_Read_Capacity(pDmc);
		
			break;
			
		case DriveSpinningUp:
			// this state is OK to continue,
			// wait for device to be ready then start Read Cap state machine
			DM_Do_Test_Unit_Wait(pDmc);
		
			break;

		case DriveInvalid:
			// invalid device, remove STATE and continue
			pDD->Remove((CONTAINER_KEY)sig.l);
			
			goto trynextdevice;
			break;

		default:
			// existing device changed to a bad state, update descriptor
			// and rollcall
			// update the PTS entries for the DiskDescriptor and (maybe)
			// the StorageRollCall
			DM_Do_Update_PTS(pDmc);
			break;

	}
	
} // DM_Complete_Plex_Request_2


/*************************************************************************/
// DM_Handle_Plex_Timeout
// We have timed out on this plex, clean up the current ID or LUN and start
// the next, if there is one.
/*************************************************************************/
void DriveMonitorIsm::DM_Handle_Plex_Timeout(void *p, U32 status)
{
	DM_CONTEXT		*pDmc = (DM_CONTEXT *) p;

	DM_TRACE_ENTRY(DM_Handle_Plex_Timeout);

} // DM_Handle_Plex_Timeout

/*************************************************************************/
// DM_Scan_One_Device
// Start the scan State Machine for one device on the FC port we service.
// If pMsg is non-zero, this means we were called from an incoming message
// then we must send a reply back to the caller on the last drive
/*************************************************************************/
void DriveMonitorIsm::DM_Scan_One_Device(Message *pMsg)
{
	DM_CONTEXT		*pDmc;
	DmScanSingle	*pSS = (DmScanSingle *) pMsg;
	
	DM_TRACE_ENTRY(DM_Scan_One_Device);

	// build the plex request for this device
	pDmc = (DM_CONTEXT *)DM_Build_Plex_Request(pMsg,
							pSS->payload.drive,
							pSS->payload.drive,
							PLEX_REPLY_NEEDED);		// need a reply when done
									
	// start the plex up
	DM_Do_Plex_Request(pDmc);
	
} // DM_Scan_One_Device


/*************************************************************************/
// DM_Scan_All_Devices
// Start the scan State Machine for all devices on the FC port we service.
// Three types of scans are allowed: scan all devices in sequence (one plex),
// scan 2 plexes at a time in sequence, and scan 4 plexes in sequence.
// This allows the motors of each disk type device to spin up in sequence.
// If pMsg is non-zero, this means we were called from an incoming message
// then we must send a reply back to the caller on the last plex.  Only
// IDs are reported and counted in the update.
/*************************************************************************/
void DriveMonitorIsm::DM_Scan_All_Devices(Message *pMsg)
{
	DM_CONTEXT		*pDmc;
	U32				loop, start, end, index, divisor;
	
	DM_TRACE_ENTRY(DM_Scan_All_Devices);

	DM_PRINT_STRING(TRACE_L2, "\n\rDM: Start Scan All Drives - Please Wait...");

	// start with no good or bad drives
	DM_Drives = config.num_drives;
	DM_Good_Drives = 0;
	DM_Bad_Drives = 0;
	
	TRACEF(TRACE_L3, ("\n\rNumber of devices to scan: %d", DM_Drives));
	
	// check for the SPIN_ALL case
	if ((config.flags & DM_FLAGS_SPIN_TYPE_MASK) == DM_FLAGS_SPIN_ALL)
	{
		// build the plex request for all the devices
		pDmc = (DM_CONTEXT *)DM_Build_Plex_Request(pMsg, 0, config.num_drives - 1, PLEX_UPDATE);
										
		// start the plex up
		DM_Do_Plex_Request(pDmc);
		return;
	}

	// check for the SPIN_2X case
	if ((config.flags & DM_FLAGS_SPIN_TYPE_MASK) == DM_FLAGS_SPIN_2X)
	{
		divisor = 2;
		index = config.num_drives / 2;
	}
	else
	{
		// default is SPIN_4X
		divisor = 4;
		index = config.num_drives / 4;
	}
	
	start = 0;
	end = index - 1;
	
	TRACEF(TRACE_L3, ("\n\rNumber of plexes to scan: %d", divisor));

	for (loop = divisor; loop; loop--) {
	
		// build a request for this plex (OK to update on the full scan)
		pDmc = (DM_CONTEXT *)DM_Build_Plex_Request(pMsg, start, end, PLEX_UPDATE);

		// start the plex up
		DM_Do_Plex_Request(pDmc);
		
		start += index;
		end += index;
	}
	
	// check for an odd man config
	loop = (config.num_drives % divisor);
	if (loop)
	{
		// back up to the correct start
		end -= index;

		// build a request for this plex (OK to update on the full scan)
		pDmc = (DM_CONTEXT *)DM_Build_Plex_Request(pMsg, end + 1, end + loop, PLEX_UPDATE);

		// start the plex up
		DM_Do_Plex_Request(pDmc);
		
	}
	
} // DM_Scan_All_Devices

/*************************************************************************/
// DM_Do_Inquiry
// Check for a drive present
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Inquiry(DM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;	// faster access
	STATUS			 status;
	
	DM_TRACE_ENTRY(DM_Do_Inquiry);

	// allocate a buffer for the inquiry data
	pDmc->buffer = new (tSMALL|tUNCACHED) INQUIRY;
	
	// build the inquiry message
	DM_Build_SCSI_Message(pMsg, CMD_INQUIRY, pDmc->buffer,
					sizeof(INQUIRY), pDmc->drive_number, pDmc->lun_number);
	
	status = Send((VDN)config.vd, (Message *)pDmc->pMsg, (void *)pDmc,
					(ReplyCallback)&DM_Do_Inquiry_Callback);
	
 	if (status != NU_SUCCESS)
		DM_Log_Error(DM_ERROR_TYPE_FATAL,
			"DM_Do_Inquiry", 
			"Send failed",
			config.xlate[pDmc->drive_number],
			(UNSIGNED)status);

} // DM_Do_Inquiry

/*************************************************************************/
// DM_Do_Inquiry_Callback
// Finish Inquiry, check what type of device we found (if any)
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Do_Inquiry_Callback(Message *pMsg)
{
	DM_CONTEXT 		*pDmc = (DM_CONTEXT *)pMsg->GetContext();
	Message 		*pRf = (Message *)pMsg;
	INQUIRY			*pInq;
	DM_DEVICE_STATE	*pDMState;
	union {
		IDLUN		 idlun;
		long		 l;
	} sig;
	
	TRACE_ENTRY(DM_Do_Inquiry_Callback);

	// point to Inquiry data
	pInq = (INQUIRY *)pDmc->buffer;
	
	// check for devices on other loops that show up here
	// the perpheral qualifier field will be non-zero for these
	// Clariion in particular lists all devices on each loop
	if ((pRf->DetailedStatusCode == FCP_SCSI_DSC_SUCCESS) && 
				(pInq->qual) && (pInq->type != SCSI_DEVICE_TYPE_UNKNOWN))
	{
		// no device accessable from our side, don't even consider this
		TRACEF(TRACE_L3, ("\n\rDevice is on another loop, Id=%d, LUN=%d", 
							config.xlate[pDmc->drive_number],
							pDmc->lun_number));

		// check for end of plex
		DM_Complete_Plex_Request((void *)pDmc, 0);
		return(0);
	}
	
    // build the device key
    sig.idlun.id = (U8)config.xlate[pDmc->drive_number];
    sig.idlun.LUN = (U16)pDmc->lun_number;
  	sig.idlun.HostId = 0;

	// check for an existing device
	if ((pDD->Get((CONTAINER_ELEMENT &)pDMState, (CONTAINER_KEY)sig.l)) == FALSE)
	{
		if (pRf->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
		{
			// no device home, don't create the container
			TRACEF(TRACE_L3, ("\n\rNo device, Id=%d, LUN=%d, status=%X", 
								config.xlate[pDmc->drive_number],
								pDmc->lun_number,
								pRf->DetailedStatusCode));
	
			// check for end of plex
			DM_Complete_Plex_Request((void *)pDmc, 0);
			return(0);
		}
		else
		{
			// did not exist and good device status
			pDMState = new DM_DEVICE_STATE;
			memset(pDMState, 0, sizeof(DM_DEVICE_STATE));
			
			// set initial states
			pDMState->state = (DEVICE_STATE_ADD_DD|
								DEVICE_STATE_ADD_RC|
								DEVICE_STATE_ADD_PATH|
								DEVICE_STATE_NEW_DEVICE);
			
			// add to the list
			pDD->Add((CONTAINER_ELEMENT)pDMState, (CONTAINER_KEY)sig.l);
			
			TRACEF(TRACE_L3, ("\n\rDM: New Inq device type %d, Id=%d, LUN=%d",
									*(U8 *)pDmc->buffer, 
									config.xlate[pDmc->drive_number],
									pDmc->lun_number));
		}
	}
	
	// save the pointer for later
	pDmc->pDMState = pDMState;
	
	// check status, may not be a device there
	if (pRf->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		// no device home
		TRACEF(TRACE_L3, ("\n\rExisting device failed, Id=%d, LUN=%d, status=%X", 
							config.xlate[pDmc->drive_number],
							pDmc->lun_number,
							pRf->DetailedStatusCode));

		// set new states
		pDMState->state |= DEVICE_STATE_DEVICE_FAILURE;

		if (((pDMState->state & DEVICE_STATE_ADD_DD) == 0) && pDMState->pDD)
		{
			// set a bad status for disks
			if (pRf->DetailedStatusCode == FCP_SCSI_HBA_DSC_SELECTION_TIMEOUT)
				pDMState->pDD->CurrentStatus = DriveRemoved;
			else
				pDMState->pDD->CurrentStatus = DriveHardFailure;
		}
		
		// check for end of plex
		DM_Complete_Plex_Request((void *)pDmc, 0);
		return(0);
	}
	
	// At this point we have a valid device,
	// reset error state and flag device found
	pDMState->state &= ~DEVICE_STATE_DEVICE_FAILURE;
	pDMState->state |= DEVICE_STATE_DEVICE_FOUND;

	DM_PRINT_HEX16(TRACE_L8, "\n\rDriveMonitorIsm: Inquiry data, Id=",
						config.xlate[pDmc->drive_number]);
    DM_DUMP_HEX(TRACE_L8, " ",
    					(U8 *)pDmc->buffer,
    					sizeof(INQUIRY));
    
	// save the type for later
	pDMState->type = pInq->type;
						
	TRACEF(TRACE_L3, ("\n\rDM: Inq device type %d, Id=%d, LUN=%d",
							pInq->type, 
							config.xlate[pDmc->drive_number],
							pDmc->lun_number));

	// check which device type we have
	switch(pInq->type)
	{
		case SCSI_DEVICE_TYPE_DIRECT:
		case SCSI_DEVICE_TYPE_ARRAY_CONT:
		// check for LUN request on disk types only
		if ((config.flags & DM_FLAGS_SCAN_MASK) && 
					((pDmc->flags & PLEX_LUN_REQUEST) != PLEX_LUN_REQUEST))
		{
			if (config.flags & DM_FLAGS_USE_GET_LUNS)
			{
				// first report LUNs
				DM_Do_Report_Luns(pDmc);
				break;
			}
			
			// otherwise must be first 8 LUNs only
			DM_Do_Scan_8_Luns(pDmc);
			break;
		}
		
		// handle a disk or array type device
		DM_Do_Disk(pDmc);
		break;
		
		case SCSI_DEVICE_TYPE_SEQUENTIAL:
		// handle a tape device
		DM_Do_Tape(pDmc);
		break;
		
		case SCSI_DEVICE_TYPE_ENCLOSURE_SERVICES:
		// handle an SES device
		DM_Do_SES(pDmc);
		break;
		
		default:
		// need to remove the container already allocated
		pDD->Remove((CONTAINER_KEY)sig.l);
		
		// unsupported device type
		DM_Do_Other(pDmc);
		break;
		
	}
	
	return(0);
	
} // DM_Do_Inquiry_Callback
			

#define	REPORT_LUNS_BUFFER_SIZE		4096
/*************************************************************************/
// DM_Do_Report_Luns
// Check for LUNs under this ID
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Report_Luns(DM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;	// faster access
	STATUS			 status;
	
	DM_TRACE_ENTRY(DM_Do_Report_Luns);

	// allocate a buffer for the max number of luns
	pDmc->pluns = new U32[REPORT_LUNS_BUFFER_SIZE];
	pDmc->pluns[0] = 0; // first entry zero
	
	// build the report LUNs message
	DM_Build_SCSI_Message(pMsg, CMD_REPORT_LUNS,
					pDmc->pluns,
					REPORT_LUNS_BUFFER_SIZE * sizeof(U32),
					pDmc->drive_number,
					pDmc->lun_number);
	
	status = Send((VDN)config.vd, (Message *)pDmc->pMsg, (void *)pDmc,
					(ReplyCallback)&DM_Do_Report_Luns_Callback);
	
 	if (status != NU_SUCCESS)
		DM_Log_Error(DM_ERROR_TYPE_FATAL,
			"DM_Do_Report_Luns", 
			"Send failed",
			pDmc->drive_number,
			(UNSIGNED)status);

} // DM_Do_Report_Luns

/*************************************************************************/
// DM_Do_Report_Luns_Callback
// Finish Report LUNS command, check how many LUNs we found (if any)
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Do_Report_Luns_Callback(Message *pMsg)
{
	DM_CONTEXT 		*pDmc1, *pDmc = (DM_CONTEXT *)pMsg->GetContext();
	Message 		*pRf = (Message *)pMsg;

	TRACE_ENTRY(DM_Do_Report_Luns_Callback);

	// check status, report LUNs may not work on all devices
	if (pRf->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		SCB_REPLY_PAYLOAD		*payload = (SCB_REPLY_PAYLOAD *) pMsg->GetPPayload();
		REQUEST_SENSE			*pRq = (REQUEST_SENSE *)&payload->SenseData[0];
		
		// maybe no device home
		DM_PRINT_HEX16(TRACE_L3, "\n\rReport LUNs failed, Id=",
									config.xlate[pDmc->drive_number]);
		DM_PRINT_HEX16(TRACE_L3, " status=", pRf->DetailedStatusCode);
		
		// check for a retry (only one case)
		if ((pRf->DetailedStatusCode == FCP_SCSI_DSC_CHECK_CONDITION) && 
				(pRq->ASC_ASCQ == ASC_POWER_ON_OR_DEVICE_RESET_OCCURED))
		{
			delete pDmc->pluns;
			
			// try again
			DM_Do_Report_Luns(pDmc);
			return(0);
		}

		// check if scan 8 LUNs is set
		if (config.flags & DM_FLAGS_SCAN_8_LUNS)
		{
			// Scan the first 8 LUNs
			DM_Do_Scan_8_Luns(pDmc);
			return(0);
		}
		
		// done with scan, check for end of plex
		DM_Complete_Plex_Request((void *)pDmc, 0);
		return(0);
	}
	
	// worked OK
	// check how many LUNs, must be a least one
	if (pDmc->pluns[0] == 0)
	{
		// no LUNs?
		DM_Complete_Plex_Request((void *)pDmc, 0);
		return(0);
	}
	
	// build a LUN plex request to do all the LUNs found
	pDmc1 = (DM_CONTEXT *)DM_Build_Plex_Request(NULL,
						2,								// starting index
						(pDmc->pluns[0] / 8),			// ending index
						PLEX_LUN_REQUEST);
						
	// save our original request
	pDmc1->pDmc = pDmc;
	
	// drive_number never changes
	pDmc1->drive_number = pDmc->drive_number;
	
	pDmc1->num_devices = (pDmc->pluns[0] / 8);
	
	// move the LUN list to the new request
	pDmc1->pluns = pDmc->pluns;
	pDmc->pluns = NULL;
						
	// start the plex up
	DM_Do_Plex_Request(pDmc1);
	
	return(0);
	
} // DM_Do_Report_Luns_Callback

/*************************************************************************/
// DM_Do_Scan_8_Luns
// Build a plex request to scan the first 8 LUNs on this ID.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Scan_8_Luns(DM_CONTEXT *pDmc)
{
	DM_CONTEXT			*pDmc1;
	U32					 lun, index;

	TRACE_ENTRY(DM_Do_Scan_8_Luns);

	// build a LUN plex request to do the first 8 LUNs
	pDmc1 = (DM_CONTEXT *)DM_Build_Plex_Request(NULL,
						0,			// start at index 0
						7,			// end at 7
						PLEX_LUN_REQUEST);

	// save original request
	pDmc1->pDmc = pDmc;
	
	// drive_number never changes
	pDmc1->drive_number = pDmc->drive_number;
	
	pDmc1->num_devices = 8;
	
	// allocate a buffer for the max number of luns
	pDmc1->pluns = new U32[8 * 2];
	
	// build the LUN list (0 - 7)
	for (lun = 0, index = 0; lun < 8; lun++)
	{
		pDmc1->pluns[index] = lun << 16;
		index++;
		index++;
	}
	
	// start the plex up
	DM_Do_Plex_Request(pDmc1);
	
} // DM_Do_Scan_8_Luns


			
/*************************************************************************/
// DM_Do_Finish
// Done with the scan for this plex, add in the totals (if allowed),
// check for all plexes done, then check for removed devices and
// print totals.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Finish(DM_CONTEXT *pDmc)
{
	DM_DEVICE_STATE	*pDMState;
	STATUS			 status;

	DM_TRACE_ENTRY(DM_Do_Finish);

	// can we update totals?
	if (pDmc->flags & PLEX_UPDATE)
	{
		// totals only updated when full scan
		DM_Drives -= pDmc->num_devices;		// good or bad
		DM_Good_Drives += pDmc->num_good;
		DM_Bad_Drives += pDmc->num_bad;
	}
	
	if ((DM_Drives <= 0) && (pDmc->flags & PLEX_UPDATE))
	{
		// done with all plexes, see if any removed drives
		// overload this as the removed drive update counter
		DM_Drives = 0;
		DM_Removed_Drives = 0;
		
		// unroll the Device containers, check for removed devices
		for (int x = pDD->Count() - 1; x >= 0; x--)
		{
			// get each Device container in sequence
			pDD->GetAt((CONTAINER_ELEMENT &)pDMState, x);
			
			// check if the device was found on this last scan
			if (pDMState->state & DEVICE_MASK)
			{
				// reset the found bits
				pDMState->state &= ~DEVICE_MASK;
			}
			else
			{	
				// drive not found this scan, update the disk descriptor
				// with removed status
				if (pDMState->pDD == NULL)
				{
					// no descriptor, don't update anything and don't
					// count it as a real device.  May have been a BSA
					// or ES vdn configured without any device.  We will
					// just save the STATE until the device comes online.
					// Note: device may never come online...
					continue;
				}
				// need a context per request to pass State info
				DM_CONTEXT *pDmc1 = new DM_CONTEXT;
				memset(pDmc1, 0, sizeof(DM_CONTEXT));
				
				DM_Removed_Drives++;
				
				pDMState->pDD->CurrentStatus = DriveRemoved;
				pDmc1->pDMState = pDMState;
				pDmc1->pReply = pDmc->pReply;
				pDmc1->index = x;			// for removal
				
				// no device anymore
				TRACEF(TRACE_L3, ("\n\rDevice Removed, Slot=%d, Id=%d, LUN=%d", 
									pDMState->pDD->SlotID,
									pDMState->pPD->FCTargetID,
									pDMState->pPD->FCTargetLUN));
		
				// Update the row this descriptor goes to
				status = DmTableUpdateDD(pDmc1, (pDMCallback_t)&DM_Removed_Drive_Callback);
			}		
		}

		TRACEF(TRACE_L2, ("\n\rNum Good Drives = %d", DM_Good_Drives));
		TRACEF(TRACE_L2, ("\n\rNum Bad Drives = %d", DM_Bad_Drives));
		TRACEF(TRACE_L2, ("\n\rNum Removed Drives = %d", DM_Removed_Drives));

		// Scan now complete
		DM_Scan_In_Progress = 0;
		
		// check for reply needed
		if ((pDmc->pReply) && (DM_Removed_Drives == 0))
		{
			status = Reply(pDmc->pReply, FCP_SCSI_HBA_DSC_SUCCESS);
			pDmc->pReply = NULL;
		}
	
	}
	
	// check for manditory reply needed (for single requests)
	if ((pDmc->flags & PLEX_REPLY_NEEDED) && (pDmc->pReply))
	{
		// Scan now complete
		DM_Scan_In_Progress = 0;
		
		status = Reply(pDmc->pReply, FCP_SCSI_HBA_DSC_SUCCESS);
	}

	// we are done with the plex context
	// delete the buffer (if used )
	if (pDmc->buffer)
	{
		delete pDmc->buffer;
	}
	
	// delete memory allocated for message
	delete (Message *)pDmc->pMsg;
	
	// delete the context
	delete pDmc;

} // DM_Do_Finish


/*************************************************************************/
// DM_Removed_Drive_Callback
/*************************************************************************/
void DriveMonitorIsm::DM_Removed_Drive_Callback(void *p, U32 status)
{
	DM_CONTEXT		*pDmc = (DM_CONTEXT *)p;
	DM_DEVICE_STATE	*pDMState = pDmc->pDMState;

	DM_TRACE_ENTRY(DM_Removed_Drive_Callback);
	
	// check for the last request
	if (DM_Removed_Drives == ++DM_Drives)
	{
		// make sure we reply if needed
		if (pDmc->pReply)
		{
			status = Reply(pDmc->pReply, FCP_SCSI_HBA_DSC_SUCCESS);
		}
	}

#if 0	
	// delete the Device Container?
	// so the error is reported only once
	// Note: this will destroy the BSA VDN for this device, so
	//       for now we will not delete
	if (pDMState->pDD)
		delete pDMState->pDD;
	if (pDMState->pRC)
		delete pDMState->pRC;
	if (pDMState->pOther)
		delete pDMState->pOther;
	
	pDD->RemoveAt(pDmc->index);
#endif
	// delete the context
	delete pDmc;

} // DM_Removed_Drive_Callback


