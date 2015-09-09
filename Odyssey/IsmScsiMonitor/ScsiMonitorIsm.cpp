/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiMonitorIsm.cpp
// 
// Description:
// This module is the Drive monitor for HSCSI (ScsiMonitor) ISM definition. This module
// uses I2O SCSI commands to discover any changes in status of the disks
// attached to the HBC. 
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmScsiMonitor/ScsiMonitorIsm.cpp $
// 
// 2     10/19/99 6:36p Cchan
// Fixed unintialized PTS variable SM_TS_Disk_Desc
// 
// 1     10/11/99 5:35p Cchan
// HSCSI (QL1040B) version of the Drive Monitor
// 
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "OsTypes.h"
#include "Odyssey.h"

// Tables referenced
#include "DiskDescriptor.h"
#include "StorageRollCallTable.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"

#include "SmCommon.h"
#include "ScsiMonitorIsm.h"

#include "Message.h"
#include "HscsiMessageFormats.h"
#include "HscsiMessageStatus.h"

#include "Pci.h"
#include "Scsi.h"
#include "CDB.h"
#include "CTIdLun.h"
#include "BuildSys.h"

CLASSNAME(ScsiMonitorIsm, SINGLE);

/*************************************************************************/
// Forward references
/*************************************************************************/
void S_Scan_For_Drives(Message *pMsg);
void S_Stop_Drive(U32 drive_no, unsigned s);
void S_Test_Unit_Ready(U32 drive_no);
void S_Read_Test(U32 drive_no);
void S_Sts_Read_Test(U32 Vdn);
void S_Sts_Write_Verify_Test(U32 Vdn, U32 logical_block_address,
	U32 block_count);
void S_Sts_Read_Verify_Test(U32 Vdn, U32 logical_block_address,
	U32 block_count);

/*************************************************************************/
// Global references
/*************************************************************************/
ScsiMonitorIsm					*pScsiMonitorIsm = NULL;

static U32						SM_Drives;					// num of drives scanned
static U32						SM_Good_Drives;				// num of good drives
static U32						SM_Bad_Drives;				// num of bad drives
static U32						SM_Drive_Test[15] = {0};	// drive testing flags

/*************************************************************************/
// ScsiMonitorIsm
// Constructor method for the class ScsiMonitorIsm
/*************************************************************************/
ScsiMonitorIsm::ScsiMonitorIsm(DID did):Ddm(did) {

	SM_TRACE_ENTRY(ScsiMonitorIsm::ScsiMonitorIsm);
	
	SM_PRINT_HEX(TRACE_L6, "\n\rScsiMonitorIsm::ScsiMonitorIsm this = ", (U32)this);
	SM_PRINT_HEX(TRACE_L6, "\n\rScsiMonitorIsm::ScsiMonitorIsm did = ", (U32)did);
	
	MyVd = GetVdn();
	
	pScsiMonitorIsm = this;	// save the first one
	
	SetConfigAddress(&config, sizeof(config));
	
}	// ScsiMonitorIsm

/*************************************************************************/
// Ctor
// Create a new instance of the Drive Monitor
/*************************************************************************/
Ddm *ScsiMonitorIsm::Ctor(DID did) {

	SM_TRACE_ENTRY(ScsiMonitorIsm::Ctor);

	if (pScsiMonitorIsm)
		return (NULL);			// Only one instance allowed 
	return new ScsiMonitorIsm(did); 
}	// Ctor

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS ScsiMonitorIsm::Initialize(Message *pMsg) {

	SM_TRACE_ENTRY(ScsiMonitorIsm::Initialize);
	
	// allocate enougn space for all the descriptors to be scanned
	SM_Disk_Desc = new DiskDescriptor[config.num_drives];
	
#if 1
	// Get our table data from the Persistent Data Store
	SmTableInitialize(pMsg);
	SM_TS_Disk_Desc = NULL;
#else
	SM_TS_Disk_Desc = new DiskDescriptor[config.num_drives];
	Reply(pMsg); 		// don't return this until the last TS message
#endif

	return OS_DETAIL_STATUS_SUCCESS;
	
}	// Initialize

/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
STATUS ScsiMonitorIsm::Enable(Message *pMsg) {
	
	TRACE_ENTRY(ScsiMonitorIsm::Enable);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Enable

/*************************************************************************/
// Quiesce
// Turn off the DDM belonging to this derived class
/*************************************************************************/
STATUS ScsiMonitorIsm::Quiesce(Message *pMsg) {
	
	TRACE_ENTRY(ScsiMonitorIsm::Quiesce);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Quiesce

/*************************************************************************/
// DoWork
// This derived classes method to receive messages and replies
/*************************************************************************/
STATUS ScsiMonitorIsm::DoWork(Message *pMsg) {

	SM_TRACE_ENTRY(ScsiMonitorIsm::DoWork);

	STATUS status=Ddm::DoWork(pMsg);
	
	SM_TRACE_ENTRY(ScsiMonitorIsm::DoWork Ddm::DoWork);
	
	if (status != OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION)
		return status;

	// This ISM handles Replies to messages sent from the monitor Handler
	if (pMsg->IsReply()) {
		
	    SM_DUMP_HEX(TRACE_L8, "\n\rScsiMonitorIsm::DoWork Message Reply",
	    					(U8 *)pMsg, 128);
		switch (pMsg->reqCode) {
		
		// Only one reply is valid
		case SCSI_SCB_EXEC:
			{
				SM_CONTEXT		*pDmc;
				SCB_REPLY_PAYLOAD		*p_rp = (SCB_REPLY_PAYLOAD *)pMsg->GetPPayload();
				Message 		*pRf = (Message *)pMsg;
				
				// recover the context pointer
				pDmc = (SM_CONTEXT *)pMsg->GetContext();
				
				// find out what we were up to
				switch (pDmc->action) {
				
				case SM_ACTION_INQUIRY:
					// check status, may not be a drive there
					if (pRf->DetailedStatusCode != HSCSI_SCSI_DSC_SUCCESS)
					{
						// no device home
						SM_PRINT_HEX16(TRACE_L3, "\n\rNo device, Id=", pDmc->drive_number);
						SM_PRINT_HEX16(TRACE_L3, " status=", 
											pRf->DetailedStatusCode);

						// mark table
						SM_Disk_Desc[pDmc->drive_number].CurrentStatus = DriveNotPresent;
						// and exit
						SM_Do_Update_PD(pDmc);
						break;
					}
					
					SM_PRINT_HEX16(TRACE_L8, "\n\rScsiMonitorIsm: Inquiry data, Id=", pDmc->drive_number);
				    SM_DUMP_HEX(TRACE_L8, " ",
				    					(U8 *)pDmc->buffer,
				    					sizeof(INQUIRY));
					// copy the inquiry data to the Disk Object Table
					memcpy(&SM_Disk_Desc[pDmc->drive_number].InqData,
							pDmc->buffer,
							sizeof(INQUIRY));
					
					// delete the memory allocated
					delete pDmc->buffer;
					pDmc->buffer = NULL;
					
					// next see if the drive is spinning
					SM_Do_Test_Unit(pDmc);
					break;
					
				case SM_ACTION_INQUIRY_SERIAL_NUMBER:
					// check status, may not work on some drives
					if (pRf->DetailedStatusCode != HSCSI_SCSI_DSC_SUCCESS)
					{
						// no serial number
						SM_PRINT_HEX16(TRACE_L8, "\n\rNo serial number, Id=", pDmc->drive_number);
						
						// mark table (not an error if no serial number)
						SM_Disk_Desc[pDmc->drive_number].SerialNumber[0] = 0;
					}
					else
					{
						INQUIRY_SERIAL_NUMBER	*p_is = (INQUIRY_SERIAL_NUMBER *) pDmc->buffer;
						
						SM_PRINT_HEX16(TRACE_L8, "\n\rScsiMonitorIsm: Inquiry Serial data, Id=", pDmc->drive_number);
					    SM_DUMP_HEX(TRACE_L8, " ",
					    					(U8 *)pDmc->buffer,
					    					sizeof(INQUIRY_SERIAL_NUMBER));
						// copy the serial number data to the Disk Object Table
						memcpy(&SM_Disk_Desc[pDmc->drive_number].SerialNumber[0],
								&p_is->ProductSerialNumber[0],
								p_is->PageLength);
						SM_Disk_Desc[pDmc->drive_number].SerialNumber[p_is->PageLength] = 0;
					
					}
					
					// delete the memory allocated for the serial number
					delete pDmc->buffer;
					pDmc->buffer = NULL;
					
					// update the PD
					SM_Do_Update_PD(pDmc);
					break;
					
				case SM_ACTION_TEST_UNIT:
					// find out if this drive is ready or not
					if (pRf->DetailedStatusCode != HSCSI_SCSI_DSC_SUCCESS)
					{
						// Drive is not ready
						SM_PRINT_HEX16(TRACE_L8, "\n\rDrive Not Ready, Id=",
										pDmc->drive_number);
						SM_PRINT_HEX16(TRACE_L8, "\n\rStatus =  ",
										pRf->DetailedStatusCode);
												
				    	// try again?
						if (pDmc->retries++ == 2) // retries
						{
							// Motor Start failed
							SM_Bad_Drives++;
							// mark table
							SM_Disk_Desc[pDmc->drive_number].CurrentStatus = DriveHardFailure;
							// and exit
							SM_Do_Update_PD(pDmc);
							break;
						}
						
						// drive not ready, try to spin up
						SM_Do_Start_Unit(pDmc);
					}
					else
					{
						// Drive is ready
						SM_PRINT_HEX16(TRACE_L8, "\n\rDrive Ready, Id=", pDmc->drive_number);
						
						// if drive is ready, read capacity
						SM_Do_Read_Capacity(pDmc);
						
					}
					break;
					
				case SM_ACTION_START_UNIT:
					// TODO: delay some here? 2-3 seconds to allow drive power to stabilize
					
					// Drive spinup done
					SM_PRINT_HEX16(TRACE_L8, "\n\rDrive Spinup Done, Id=",
										pDmc->drive_number);
					// drive spun up, try the test unit again
					SM_Do_Test_Unit(pDmc);
					break;
				case SM_ACTION_STOP_UNIT:
					break;	
				case SM_ACTION_GET_CAPACITY:
					if (pRf->DetailedStatusCode != HSCSI_SCSI_DSC_SUCCESS)
					{
						SM_PRINT_HEX16(TRACE_L8, "\n\rRead Cap Failed, Id=", pDmc->drive_number);
						
						// Read Capacity failed
				    	// try again
						if (pDmc->retries++ == 1) // retries
						{
							// Read Capacity failed
							SM_Disk_Desc[pDmc->drive_number].CurrentStatus = DriveHardFailure;
							SM_Bad_Drives++;
							goto RD_CAP_FAILED;
						}
						
						delete pDmc->buffer;
						SM_Do_Read_Capacity(pDmc);
						break;
					}
					else
					{
						SM_PRINT_HEX16(TRACE_L8, "\n\rScsiMonitorIsm: Capacity data, Id=", pDmc->drive_number);
					    SM_DUMP_HEX(TRACE_L8, " ",
					    					(U8 *)pDmc->buffer,
					    					sizeof(READ_CAPACITY));
					    // check for no capacity (happens on first try?)
					    if ((((READ_CAPACITY *)pDmc->buffer)->BlockAddress) == 0)
					    {
					    	// try again
							if (pDmc->retries++ == 1) // retries
							{
								// Read Capacity failed
								SM_Disk_Desc[pDmc->drive_number].CurrentStatus = DriveHardFailure;
								SM_Bad_Drives++;
								goto RD_CAP_FAILED;
							}
							
							delete pDmc->buffer;
							SM_Do_Read_Capacity(pDmc);
							break;
						
					    }
						// copy capacity data to the disk table
						// if drive is ready
						SM_Disk_Desc[pDmc->drive_number].Capacity.HighPart = 0;
						SM_Disk_Desc[pDmc->drive_number].Capacity.LowPart =
							(U32) ((READ_CAPACITY *)pDmc->buffer)->BlockAddress;
							
						SM_Disk_Desc[pDmc->drive_number].CurrentStatus = DriveReady;
					}
					
					// delete the memory allocated for the Capacity date
					delete pDmc->buffer;
					pDmc->buffer = NULL;
					
					// try to read the serial number next
					SM_Do_Inquiry_Serial_Number(pDmc);
					break;
					
					// GetCapacity failed, delete the memory allocated for the
					// Capacity data and post the error. Don't read the serial
					// number.
RD_CAP_FAILED:
					delete pDmc->buffer;
					pDmc->buffer = NULL;
					
					SM_Do_Finish(pDmc);
					break;
					
				case SM_ACTION_UPDATE_PD:
					// done with our Persistent data update
					// never gets here due to callbacks
					SM_Do_Finish(pDmc);
					break;
					
				case SM_ACTION_FINISH:
					SM_Do_Finish(pDmc);
					break;
					
				case SM_ACTION_DRIVE_READ_TEST:
					// read is done
					SM_PRINT_HEX16(TRACE_L8, "\n\rRead Done, Id=",
										pDmc->drive_number);
					
					// delete the buffer so no memory leaks
					delete pDmc->buffer;
						
					if (SM_Drive_Test[pDmc->drive_number] == 0)
					{
						// done with this test
						// delete memory allocated for message
						delete (Message *)pDmc->pMsg;
						
						// delete the context
						delete pDmc;
						
						break;
					}
					
					// do the read again
					SM_Read_Drive(pDmc);
					break;
					
				case SM_ACTION_WRITE_VERIFY_TEST:
					// write is done
					SM_PRINT_HEX16(TRACE_L8, "\n\rWrite verify Done, Id=",
										pDmc->drive_number);
					
					// delete the buffer so no memory leaks
					delete pDmc->buffer;
						
					// done with this test
					// delete memory allocated for message
					delete (Message *)pDmc->pMsg;
					
					// delete the context
					delete pDmc;
					
					break;
					
				case SM_ACTION_READ_VERIFY_TEST:
					// read is done
					Verify_Read(pDmc);
					SM_PRINT_HEX16(TRACE_L8, "\n\rRead verify Done, Id=",
										pDmc->drive_number);
					
					// delete the buffer so no memory leaks
					delete pDmc->buffer;
						
					// done with this test
					// delete memory allocated for message
					delete (Message *)pDmc->pMsg;
					
					// delete the context
					delete pDmc;
					
					break;
					
				default:
					// error - invalid action
					break;
				}
			}
			break;
						
		default:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
		}
	}
	
	// New service message has been received
	else switch(pMsg->reqCode) {

		// TODO:
		// create a message type that can scan a single drive and start
		// the motor
		
		case BSA_STATUS_CHECK:		// DO we actually get any of these?
		case BSA_POWER_MANAGEMENT:
			SM_Scan_For_Drives(pMsg);
			break;

		case SCSI_DEVICE_RESET:
			SM_Scan_For_Drives(NULL);		// RESET causes a rescan, no reply
			break;
	
		default:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
	}

	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
}	// DoWork

/*************************************************************************/
// S_Scan_For_Drives
// External entry for scan
/*************************************************************************/
void S_Scan_For_Drives(Message *pMsg)
{
	SM_TRACE_ENTRY(S_Scan_For_Drives);

	pScsiMonitorIsm->SM_Scan_For_Drives(pMsg);
}

/*************************************************************************/
// S_Stop_Drive
// External entry to issue SCSI Stop Unit to a drive
/*************************************************************************/
void S_Stop_Drive(U32 drive_no, unsigned s)
{
	SM_CONTEXT		*pDmc;
	
	SM_TRACE_ENTRY(S_Stop_Drive);
	
	pDmc = new (tSMALL|tUNCACHED) SM_CONTEXT;
	pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(HSCSI_MSG_SIZE));
	pDmc->pReply = NULL;
	pDmc->drive_number = pScsiMonitorIsm->config.xlate[drive_no];
	pDmc->index = drive_no;
	pDmc->last = 1;
	pDmc->Vd = pScsiMonitorIsm->config.vd;
	pScsiMonitorIsm->SM_Do_Stop_Unit(pDmc,s);
}

/*************************************************************************/
// S_Test_Unit_Ready
// External entry to issue SCSI Test Unit Ready to a drive
/*************************************************************************/
void S_Test_Unit_Ready(U32 drive_no)
{
	SM_CONTEXT		*pDmc;
	
	SM_TRACE_ENTRY(S_Test_Unit_Ready);
	
	pDmc = new (tSMALL|tUNCACHED) SM_CONTEXT;
	pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(HSCSI_MSG_SIZE));
	pDmc->pReply = NULL;
	pDmc->drive_number = pScsiMonitorIsm->config.xlate[drive_no];
	pDmc->index = drive_no;
	pDmc->last = 1;
	pDmc->Vd = pScsiMonitorIsm->config.vd;
	pScsiMonitorIsm->SM_Do_Test_Unit(pDmc);
}

/*************************************************************************/
// S_Sts_Read_Test
// External entry for the Read through the Scsi Target Server Test code
/*************************************************************************/
void S_Sts_Read_Test(U32 Vdn)
{
	SM_CONTEXT		*pDmc;
	U32				drive_no = 0;	// always use drive 0

	SM_TRACE_ENTRY(S_Read_Test);

	// see if test is running
	if (SM_Drive_Test[drive_no] == 1)
	{
		SM_PRINT_HEX16(TRACE_L2, "\n\rStopping Sts Read Test, Vdn=", Vdn);
		// stop the test
		SM_Drive_Test[drive_no] = 0;
		return;
	}
	
	// allocate our context
	pDmc = new (tSMALL|tUNCACHED) SM_CONTEXT;
	// allocate a message to go with it
	pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(HSCSI_MSG_SIZE));
	
	// pass a NULL reply pointer along
	pDmc->pReply = NULL;	
	
	// drive number index
	pDmc->drive_number = pScsiMonitorIsm->config.xlate[drive_no];
	pDmc->index = drive_no;
	
	pDmc->last = 1;
	
	// get the VirtualDevice number for the Scsi Target Server
	pDmc->Vd = Vdn;
	
	// show test is running
	SM_Drive_Test[drive_no] = 1;
	
	SM_PRINT_HEX16(TRACE_L2, "\n\rStarting Sts Read Test, Vdn=", Vdn);
	// start the read state machine
	pScsiMonitorIsm->SM_Read_Drive(pDmc);
}

/*************************************************************************/
// S_Read_Test
// External entry for the Read Test code
/*************************************************************************/
void S_Read_Test(U32 drive_no)
{
	SM_CONTEXT		*pDmc;

	SM_TRACE_ENTRY(S_Read_Test);

	// see if test is running
	if (SM_Drive_Test[drive_no] == 1)
	{
		SM_PRINT_HEX16(TRACE_L2, "\n\rStopping Read Test, Id=",
										pScsiMonitorIsm->config.xlate[drive_no]);
		// stop the test
		SM_Drive_Test[drive_no] = 0;
		return;
	}
	
	// allocate our context
	pDmc = new (tSMALL|tUNCACHED) SM_CONTEXT;
	// allocate a message to go with it
	pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(HSCSI_MSG_SIZE));
	
	// pass a NULL reply pointer along
	pDmc->pReply = NULL;	
	
	// drive number index
	pDmc->drive_number = pScsiMonitorIsm->config.xlate[drive_no];
	pDmc->index = drive_no;
	
	pDmc->last = 1;
	
	// get the VirtualDevice number for the RAC Initiator
	pDmc->Vd = pScsiMonitorIsm->config.vd;
	
	// show test is running
	SM_Drive_Test[drive_no] = 1;
	
	SM_PRINT_HEX16(TRACE_L2, "\n\rStarting Read Test, Id=",
										pDmc->drive_number);
	// start the read state machine
	pScsiMonitorIsm->SM_Read_Drive(pDmc);
}

/*************************************************************************/
// SM_Scan_For_Drives
// Start the scan State Machine for all drives on the HBC SCSI port
// When the reply is returned, we will mark the drive as present or not
// and start the motor if needed
// If pMsg is non-zero, this means we were called from an incoming message
// then we must send a reply back to the caller on the last drive
/*************************************************************************/
void ScsiMonitorIsm::SM_Scan_For_Drives(Message *pMsg)
{
	SM_CONTEXT		*pDmc;
	U32				loop;
	
	SM_TRACE_ENTRY(SM_Scan_For_Drives);

	SM_PRINT_STRING(TRACE_L2, "\n\rSM: Start Scan");

	// start with no good or bad drives
	SM_Drives= 0;
	SM_Good_Drives= 0;
	SM_Bad_Drives = 0;
	
	printf("config.num_drives: %d\n",config.num_drives);
	
	for (loop = 0; loop < config.num_drives; loop++) {
	
		// allocate our context
		pDmc = new (tSMALL|tUNCACHED) SM_CONTEXT;
		// allocate a message to go with it
		pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(HSCSI_MSG_SIZE));
		
		// pass the reply pointer along (if any)
		pDmc->pReply = pMsg;	
		
		// drive number index
		pDmc->drive_number = config.xlate[loop];
		pDmc->index = loop;
		
		if (loop == (config.num_drives-1))
			pDmc->last = 1;
		else
			pDmc->last = 0;
		
		// get the VirtualDevice number for the HSCSI Initiator
		pDmc->Vd = config.vd;
		
		// preset table entry
		
		// copy global Desc to local to get the rid
		SM_Disk_Desc[loop] = SM_TS_Disk_Desc[loop];
		
		SM_Disk_Desc[loop].vdBSADdm = -1;  // no BSA yet
		SM_Disk_Desc[loop].CurrentStatus = DriveInvalid;
		SM_Disk_Desc[loop].SlotID = loop;
		SM_Disk_Desc[loop].FCTargetID =	config.xlate[loop];
		
		// start the state machine for this drive
		SM_Do_Inquiry(pDmc);
		SM_Drives++;
	}
} // SM_Scan_For_Drives

/*************************************************************************/
// SM_Do_Inquiry
// Check for a drive present
/*************************************************************************/
void ScsiMonitorIsm::SM_Do_Inquiry(SM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;	// faster access
	
	SM_TRACE_ENTRY(SM_Do_Inquiry);

	// allocate a buffer for the inquiry data
	pDmc->buffer = new (tSMALL|tUNCACHED) INQUIRY;
	
	// build the inquiry message
	SM_Build_SCSI_Message(pMsg, CMD_INQUIRY, pDmc->buffer,
					sizeof(INQUIRY), pDmc->drive_number);
	
	// handle the inquiry data when the reply comes back
	pDmc->action = SM_ACTION_INQUIRY;
	
	SM_Send_Message(pDmc);
	
} // SM_Do_Inquiry

/*************************************************************************/
// SM_Do_Inquiry_Serial_Number
// Read the serial number of a drive using an extended inquiry SCSI command
/*************************************************************************/
void ScsiMonitorIsm::SM_Do_Inquiry_Serial_Number(SM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;
	INQUIRY_CMD		*pInq;
	SCB_PAYLOAD		*pP;
	
	SM_TRACE_ENTRY(SM_Do_Inquiry_Serial_Number);

	// allocate a buffer for the serial number data
	pDmc->buffer = new (tSMALL|tUNCACHED) INQUIRY_SERIAL_NUMBER;
	
	// build the inquiry message
	SM_Build_SCSI_Message(pMsg, CMD_INQUIRY, pDmc->buffer,
					sizeof(INQUIRY_SERIAL_NUMBER), pDmc->drive_number);
	
	pP = (SCB_PAYLOAD *) pMsg->GetPPayload();
	pInq = (INQUIRY_CMD *) &pP->CDB;
	pInq->EVPD = 1;
	pInq->PageCode = INQ_PAGE_CODE_SERIAL_NUMBER;	// get serial number page
	
	// handle the inquiry serial number data when the reply comes back
	pDmc->action = SM_ACTION_INQUIRY_SERIAL_NUMBER;
	
	SM_Send_Message(pDmc);
	
} // SM_Do_Inquiry_Serial_Number

/*************************************************************************/
// SM_Do_Test_Unit
// Check if drive is spinning and ready
/*************************************************************************/
void ScsiMonitorIsm::SM_Do_Test_Unit(SM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;
	
	SM_TRACE_ENTRY(SM_Do_Test_Unit);

	// build the test unit ready message
	SM_Build_SCSI_Message( pMsg, CMD_TEST_UNIT, 0, 0, pDmc->drive_number);
	
	pDmc->action = SM_ACTION_TEST_UNIT;
	
	SM_Send_Message(pDmc);
	
} // SM_Do_Test_Unit

/*************************************************************************/
// SM_Do_Start_Unit
// Make a drive ready by spinning it up
/*************************************************************************/
void ScsiMonitorIsm::SM_Do_Start_Unit(SM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;
	CDB6			*pStUn;
	SCB_PAYLOAD		*pP;
	
	SM_TRACE_ENTRY(SM_Do_Start_Unit);

	// build the start unit message
	SM_Build_SCSI_Message( pMsg, CMD_START_UNIT, 0, 0, pDmc->drive_number);
	
	pP = (SCB_PAYLOAD *) pMsg->GetPPayload();
	pStUn = (CDB6 *) &pP->CDB;
	pStUn->Length = 1; // set Start bit
	pStUn->MSB = 1;  // set IMMED bit
	
	pDmc->action = SM_ACTION_START_UNIT;
	
	SM_Send_Message(pDmc);
	
} // SM_Do_Start_Unit

/*************************************************************************/
// SM_Do_Stop_Unit
// Make a drive spin down
/*************************************************************************/
void ScsiMonitorIsm::SM_Do_Stop_Unit(SM_CONTEXT *pDmc, unsigned s)
{
	Message			*pMsg = pDmc->pMsg;
	CDB6			*pStUn;
	SCB_PAYLOAD		*pP;
	
	SM_TRACE_ENTRY(SM_Do_Stop_Unit);
	
	SM_Build_SCSI_Message( pMsg, CMD_START_UNIT, 0, 0, pDmc->drive_number);

#if defined(SM_DEBUG) && defined(_DEBUG)
	if (s) {
		SM_PRINT_HEX16(TRACE_L2, "\n\rStarting Disk, Id = ",
				pDmc->drive_number);
	}
	else {
		SM_PRINT_HEX16(TRACE_L2, "\n\rStopping Disk, Id = ",
				pDmc->drive_number);
	}
#endif
	
	pP = (SCB_PAYLOAD *) pMsg->GetPPayload();
	pStUn = (CDB6 *) &pP->CDB;
	pStUn->Length = s; // Toggle Start bit
	pStUn->MSB = 1; // set IMMED bit
	
	pDmc->action = SM_ACTION_STOP_UNIT;
	
	SM_Send_Message(pDmc);

} // SM_Do_Stop_Unit

/*************************************************************************/
// SM_Do_Read_Capacity
// Read the actual drive capacity
/*************************************************************************/
void ScsiMonitorIsm::SM_Do_Read_Capacity(SM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;	// faster access
	SM_TRACE_ENTRY(SM_Do_Read_Capacity);

	// allocate a buffer for the read capacity data
	pDmc->buffer = new (tSMALL|tUNCACHED) READ_CAPACITY;
	
	// build the Read Capacity message
	SM_Build_SCSI_Message(pMsg, CMD_READ_CAPACITY, pDmc->buffer,
					sizeof(READ_CAPACITY), pDmc->drive_number);
	
	// handle the capacity data when the reply comes back
	pDmc->action = SM_ACTION_GET_CAPACITY;

	SM_Send_Message(pDmc);
	
} // SM_Do_Read_Capacity

/*************************************************************************/
// SM_Do_Update_PD
// Update the Persistent Data with the drive data
/*************************************************************************/
void ScsiMonitorIsm::SM_Do_Update_PD(SM_CONTEXT *pDmc)
{
	STATUS			status;
	Message			*pMsg = pDmc->pMsg;	// faster access
	
	SM_TRACE_ENTRY(SM_Do_Update_PD);

	// check for any change in drive status
	
	// something has changed, send an update PD message
	pDmc->action = SM_ACTION_UPDATE_PD;
	
	// Build the PD Message
	// Update the row this descriptor goes to
	status = SmTableUpdate(pMsg, pDmc);
	
	//DM_Do_Finish(pDmc);  // for test
	
} // DM_Do_Update_PD

/*************************************************************************/
// SM_Do_Finish
// Done with the scan for this drive, deallocate the resources
/*************************************************************************/
void ScsiMonitorIsm::SM_Do_Finish(SM_CONTEXT *pDmc)
{
	STATUS			status;
	char			s[128];

	SM_TRACE_ENTRY(SM_Do_Finish);

	if (SM_Disk_Desc[pDmc->drive_number].CurrentStatus == DriveReady)
	{
#if defined(SM_DEBUG) && defined(_DEBUG)
		SM_PRINT_HEX16(TRACE_L2, "\n\rFound Disk, Id = ",
				pDmc->drive_number);

		// Build a readable string
		s[0] = '\n';
		s[1] = '\r';
		memcpy(&s[2],
				&SM_Disk_Desc[pDmc->drive_number].InqData.VendorId[0],
				28 );
		s[30] = 0;
		
		SM_PRINT_STRING(TRACE_L2, s);
		SM_PRINT_HEX(TRACE_L3, "\n\rCap = ",
				SM_Disk_Desc[pDmc->drive_number].Capacity.LowPart);
		SM_PRINT_STRING(TRACE_L2, "\n\r");

		SM_DUMP_HEX(TRACE_L8, " = ",
				(U8 *)&SM_Disk_Desc[pDmc->drive_number],
				sizeof(DiskDescriptor));
#endif
		
		// bump the number of good drives found
		SM_Good_Drives++;

	}
	
	SM_Drives--;		// good or bad
	
	if (SM_Drives <= 0)
	{
		SM_PRINT_HEX(TRACE_L2, "\n\rNum Good Drives = ",
				SM_Good_Drives);
		SM_PRINT_HEX(TRACE_L2, "\n\rNum Bad Drives = ",
				SM_Bad_Drives);
	}
	
	// check for reply needed
	if ((pDmc->pReply) && (SM_Drives <= 0))
	{
		status = Reply(pDmc->pReply);
	}
	
	// delete the buffer (if used )
	if (pDmc->buffer)
	{
		delete pDmc->buffer;
	}
	
	// delete memory allocated for message
	delete (Message *)pDmc->pMsg;
	
	// delete the context
	delete pDmc;

} // SM_Do_Finish


/*************************************************************************/
// SM_Build_SCSI_Message
// Build up the SCSI CDB, SGL and ByteCount in an SCB EXEC message
/*************************************************************************/
void ScsiMonitorIsm::SM_Build_SCSI_Message(Message *pMsg, SCSI_COMMANDS Cmd,
							void *pData, long length, U32 drive_number)
{
	SCB_PAYLOAD						payload;
	
	SM_TRACE_ENTRY(SM_Build_SCSI_Message);
	
	memset(&payload, 0, sizeof(SCB_PAYLOAD));

    // fill in the LUN and id fields
    payload.IdLun.id = (U8)drive_number;
    payload.IdLun.LUN = 0;	// LUN is always zero here
    
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

} // SM_Build_SCSI_Message

/*************************************************************************/
// SM_Send_Message
// Send an SCB_EXECUTE message to the HSCSI
/*************************************************************************/
void ScsiMonitorIsm::SM_Send_Message(SM_CONTEXT *pDmc)
{
	STATUS			status;
	
	SM_TRACE_ENTRY(SM_Send_Message);

	status = Send((VDN)pDmc->Vd, pDmc->pMsg, (void *)pDmc);
	
 	if (status != NU_SUCCESS)
		SM_Log_Error(SM_ERROR_TYPE_FATAL,
			"SM_Send_Message", 
			"Send failed",
			pDmc->drive_number,
			(UNSIGNED)status);
	
} // SM_Send_Message

/*************************************************************************/
// SM_Read_Drive
// Read a sector from a drive
/*************************************************************************/
void ScsiMonitorIsm::SM_Read_Drive(SM_CONTEXT *pDmc)
{
	Message			*pMsg = pDmc->pMsg;	// faster access
	CDB10			*pRd;
	SCB_PAYLOAD		*pP;
	U16				LBA;
	
	SM_TRACE_ENTRY(SM_Read_Drive);

	// allocate a buffer for the sector data
	pDmc->buffer = new (tSMALL|tUNCACHED) char[512];
	
	// build the Read Capacity message
	SM_Build_SCSI_Message(pMsg, CMD_READ10, pDmc->buffer, 512, pDmc->drive_number);

	// fix CDB
	pP = (SCB_PAYLOAD *) pMsg->GetPPayload();
	pRd = (CDB10 *) &pP->CDB;
	// misaligned field, access as bytes
	pP->CDB[7] = 0; 		// one sector to read
	pP->CDB[8] = 1;
	LBA = rand();
	pRd->BlockAddress = LBA;	// set sector zero
	
	// handle the sector data when the reply comes back
	pDmc->action = SM_ACTION_DRIVE_READ_TEST;

	SM_Send_Message(pDmc);
	
} // SM_Read_Drive

/*************************************************************************/
// S_Sts_Write_Verify_Test
// External entry for the Write through the Scsi Target Server Test code.
// Write a single transfer of block_count sectors at logical_block_address.
/*************************************************************************/
void S_Sts_Write_Verify_Test(U32 Vdn, U32 logical_block_address,
	U32 block_count)
{
	SM_CONTEXT		*pDmc;
	U32				drive_no = 0;	// always use drive 0

	SM_TRACE_ENTRY(S_Sts_Write_Verify_Test);

	// allocate our context
	pDmc = new (tSMALL|tUNCACHED) SM_CONTEXT;
	// allocate a message to go with it
	pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(HSCSI_MSG_SIZE));
	
	// pass a NULL reply pointer along
	pDmc->pReply = NULL;	
	
	// drive number index
	pDmc->drive_number = pScsiMonitorIsm->config.xlate[drive_no];
	pDmc->index = drive_no;
	
	pDmc->last = 1;
	
	// get the VirtualDevice number for the Scsi Target Server
	pDmc->Vd = Vdn;
	
	SM_PRINT_HEX16(TRACE_L2, "\n\rStarting Sts Write Verify Test, Vdn=", Vdn);
	// start the read state machine
	pScsiMonitorIsm->SM_Write_Drive_Block(pDmc, logical_block_address,
		block_count);
}

/*************************************************************************/
// SM_Write_Drive_Block
// Write a single transfer of block_count sectors at logical_block_address.
/*************************************************************************/
void ScsiMonitorIsm::SM_Write_Drive_Block(SM_CONTEXT *pDmc,
	U32 logical_block_address, U32 block_count)
{
	Message			*pMsg = pDmc->pMsg;	// faster access
	CDB10			*pRd;
	SCB_PAYLOAD		*pP;

	SM_TRACE_ENTRY(SM_Write_Drive_Block);

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
	SM_Build_SCSI_Message(pMsg, CMD_WRITE10, pDmc->buffer, block_size,
		pDmc->drive_number);

	// fix CDB
	pP = (SCB_PAYLOAD *) pMsg->GetPPayload();
	pRd = (CDB10 *) &pP->CDB;
	// misaligned field, access as bytes
	pP->CDB[7] = 0; 		// number of sectors to read
	pP->CDB[8] = block_count;
	pRd->BlockAddress = logical_block_address;	// set sector 
	
	// handle the data when the reply comes back
	pDmc->action = SM_ACTION_WRITE_VERIFY_TEST;

	SM_Send_Message(pDmc);
	
} // SM_Read_Drive

/*************************************************************************/
// S_Sts_Read_Verify_Test
// External entry for the Read through the Scsi Target Server Test code.
// Read a single transfer of block_count sectors at logical_block_address.
/*************************************************************************/
void S_Sts_Read_Verify_Test(U32 Vdn, U32 logical_block_address,
	U32 block_count)
{
	SM_CONTEXT		*pDmc;
	U32				drive_no = 0;	// always use drive 0

	SM_TRACE_ENTRY(S_Sts_Write_Verify_Test);

	// allocate our context
	pDmc = new (tSMALL|tUNCACHED) SM_CONTEXT;
	// allocate a message to go with it
	pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(HSCSI_MSG_SIZE));
	
	// pass a NULL reply pointer along
	pDmc->pReply = NULL;	
	
	// drive number index
	pDmc->drive_number = pScsiMonitorIsm->config.xlate[drive_no];
	pDmc->index = drive_no;
	
	pDmc->last = 1;
	
	// get the VirtualDevice number for the Scsi Target Server
	pDmc->Vd = Vdn;
	
	SM_PRINT_HEX16(TRACE_L2, "\n\rStarting Sts Read Verify Test, Vdn=", Vdn);
	// start the read state machine
	pScsiMonitorIsm->SM_Read_Drive_Block(pDmc, logical_block_address,
		block_count);
}

/*************************************************************************/
// SM_Read_Drive_Block
// Write a single transfer of block_count sectors at logical_block_address.
/*************************************************************************/
void ScsiMonitorIsm::SM_Read_Drive_Block(SM_CONTEXT *pDmc,
	U32 logical_block_address, U32 block_count)
{
	Message			*pMsg = pDmc->pMsg;	// faster access
	CDB10			*pRd;
	SCB_PAYLOAD		*pP;

	SM_TRACE_ENTRY(SM_Write_Drive_Block);

	// allocate a buffer for the sector data
	U32 block_size = 512 * block_count;
	pDmc->buffer = new (tSMALL|tUNCACHED) char[block_size];
	
	// build the Read message
	SM_Build_SCSI_Message(pMsg, CMD_READ10, pDmc->buffer, block_size,
		pDmc->drive_number);

	// fix CDB
	pP = (SCB_PAYLOAD *) pMsg->GetPPayload();
	pRd = (CDB10 *) &pP->CDB;
	// misaligned field, access as bytes
	pP->CDB[7] = 0; 		// number of sectors to read
	pP->CDB[8] = block_count;
	pRd->BlockAddress = logical_block_address;	// set sector 
	
	// handle the data when the reply comes back
	pDmc->action = SM_ACTION_READ_VERIFY_TEST;
	
	// Save logical_block_address and block_count in context
	// so that we can verify when we get the response.
	pDmc->logical_block_address = logical_block_address;
	pDmc->block_count = block_count;

	SM_Send_Message(pDmc);
	
} // SM_Read_Drive_Block

/*************************************************************************/
// Verify_Read
// Come here when read has finished to verify data.
/*************************************************************************/
void ScsiMonitorIsm::Verify_Read(SM_CONTEXT *pDmc)
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
			SM_PRINT_STRING(TRACE_L3, "Read verify error");
			return;
		}
	}
	
	SM_PRINT_STRING(TRACE_L3, "Verify successful");
}

