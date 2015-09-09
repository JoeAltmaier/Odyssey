/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DriveMonitorIsm.cpp
// 
// Description:
// This module is the Drive monitor ISM definition. This module uses
// SCSI commands to discover any changes in status of the devices
// attached to the assigned Fibre Channel Loop. 
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DriveMonitorIsm.cpp $
// 
// 36    2/03/00 7:13p Jlane
// Don't redirect chip here Fcp Initiator will do it.  Set scan flags
// though.
// 
// 35    2/02/00 5:49p Jlane
// Add code to switch FCPort 2 to backplane for internal drives only if
// DDHs are present.
// 
// 34    1/11/00 7:27p Mpanas
// New PathTable changes
// 
// 33    12/22/99 9:26p Jlane
// Initialize local PTS data at Enable() time to ensure
// loop instance is valid
// 
// 32    12/22/99 8:55p Jlane
// - Move config change code to Enable() since the
// config data is read twice
// - Don't destroy existing flags, use |=
// 
// 31    12/21/99 2:00p Mpanas
// Add support for IOP Failover
// - make several modules IOP_LOCAL
// 
// 30    10/08/99 3:51p Mpanas
// Fix array initialization to use actual size
// 
// 28    9/14/99 8:41p Mpanas
// Complete re-write of DriveMonitor
// - Scan in sequence
// - Start Motors in sequence
// - LUN Scan support
// - Better table update
// - Re-organize sources
// 
// 27    8/14/99 9:37p Mpanas
// Support for new LoopMonitor
// 
// 26    7/15/99 11:53p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Message front end for the DriveMonitor
// - remove all external entry points
// 
// 25    6/18/99 2:26p Mpanas
// Zero entry before use
// 
// 24    6/08/99 6:03p Mpanas
// Turn on IMM bit for motor starts
// 
// 23    5/19/99 6:22p Mpanas
// Add Write/Read/Verify Test
// 
// 22    4/29/99 10:03p Mpanas
// Add new test code to send a READ to any Vdn
// This code is called from the "task_0()" keyboard
// handler
// 
// 21    4/26/99 12:47p Mpanas
// changes to Include IDLUN in the SCSI Payload of message
// instead of signature
// 
// 20    4/10/99 2:26a Mpanas
// Used as FLASH version 2.0 4-9-99
// 
// 19    4/09/99 3:20a Mpanas
// Add Test code to do a continuous Read when
// requested
// 
// 18    4/08/99 7:14a Mpanas
// Remove  DM_Build_Message since it screws up the SGL
// 
// 17    4/06/99 10:22p Mpanas
// new Message now takes a max payload size
// 
// 16    4/01/99 2:09p Mpanas
// Changes to match with new CHAOS
// 
// 15    3/22/99 8:22p Mpanas
// Drive Monitor PTS support code changes
// 
// 09/16/98 Michael G. Panas: Create file
// 11/23/98 Michael G. Panas: update to new DiskDescriptor header
// 11/24/98 Michael G. Panas: add support for BuildSys
// 01/24/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
// 01/27/99 Michael G. Panas: Add copy to External tables
// 02/12/99 Michael G. Panas: convert to new Oos model
// 02/19/99 Michael G. Panas: convert to new Message format and remove I2O
// 03/04/99 Michael G. Panas: Add Table Service support, move config
//                            to DmConfig.h
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"
#include "RqPts_t.h"
#include "IopStatusTable.h"

#include "DmCommon.h"
#include "DriveMonitorIsm.h"
#include "Dm_Messages.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

#include "Pci.h"
#include "Scsi.h"
#include "FC_Loop.h"
#include "CTIdLun.h"
#include "BuildSys.h"

CLASSNAME(DriveMonitorIsm, MULTIPLE);

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// DriveMonitorIsm
// Constructor method for the class DriveMonitorIsm
/*************************************************************************/
DriveMonitorIsm::DriveMonitorIsm(DID did):Ddm(did) {

	DM_TRACE_ENTRY(DriveMonitorIsm::DriveMonitorIsm);
	
	DM_PRINT_HEX(TRACE_L6, "\n\rDriveMonitorIsm::DriveMonitorIsm this = ", (U32)this);
	DM_PRINT_HEX(TRACE_L6, "\n\rDriveMonitorIsm::DriveMonitorIsm did = ", (U32)did);
	
	MyVd = GetVdn();
	MyDid = did;
	
	SetConfigAddress(&config, sizeof(config));
	
}	// DriveMonitorIsm

/*************************************************************************/
// Ctor
// Create a new instance of the Drive Monitor
/*************************************************************************/
Ddm *DriveMonitorIsm::Ctor(DID did) {

	DM_TRACE_ENTRY(DriveMonitorIsm::Ctor);

	return new DriveMonitorIsm(did); 
}	// Ctor

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS DriveMonitorIsm::Initialize(Message *pMsg) {

	DM_TRACE_ENTRY(DriveMonitorIsm::Initialize);
	
	// zero the pointers used for all the descriptors to be scanned
	DM_TS_Disk_Desc = NULL;
	DM_TS_Path_Desc = NULL;
	
	// allocate space for our LoopDescriptor
	DM_Loop_Desc = new(tPCI) LoopDescriptorEntry;
	
	// zero the test array
	memset(&DM_Drive_Test[0], 0, sizeof(U32)*20);
	
	DM_Scan_In_Progress = 0;
	
	// initialize the Container for the Device List
	pDD = new SList;
	
	// Assume we're not a RAC.
	m_fHaveDDHs = false;

	// If we are in slot 24 or 28 AND we are being configured for Chip 2
	if (DM_Check_DDH_Slot())
	{
		// then fire off a message to read the IopStatusTable to see if
		// we have any DDHs and if so redirect chip two out the backplane to the
		// DDHs.  Note that I pass the Iniitialize Msg as the context of
		// the EnumTable Msg for reply in FinishInitialize().
		RqPtsEnumerateTable_T<IOPStatusRecord> *pReadIopStatusTable
			= new RqPtsEnumerateTable_T<IOPStatusRecord>;
		Send( pReadIopStatusTable, pMsg, REPLYCALLBACK(DriveMonitorIsm,FinishInitialize));
	}
	else
		// Otherwise we're done with Initialize now go ahead and reply.
		Reply(pMsg);

	return OS_DETAIL_STATUS_SUCCESS;
	
}	// Initialize

/*************************************************************************/
// FinishInitialize
// Initialize NAC Port 2 according to slot and presence or lack of DDHs.
// Note that the original Initialize Message is expected to be in the 
// Context of the message parameter.
/*************************************************************************/
STATUS DriveMonitorIsm::FinishInitialize(Message *pMsg) {
STATUS status = pMsg->Status();
Message* pInitializeMsg = (Message*)pMsg->GetContext();
RqPtsEnumerateTable_T<IOPStatusRecord> *pReadIopStatusTable 
	= (RqPtsEnumerateTable_T<IOPStatusRecord> *)pMsg;
extern void set_RAC_mode();
	
	// If we read the IOPStatus Table w/o any error.
	if (status == OK)
		for (U32 nRows = pReadIopStatusTable->GetRowCount(),
		 	 i = 0;
			 i < nRows;
			 i++)
			{
				IOPStatusRecord* pRow = &pReadIopStatusTable->GetRowPtr()[i];
				if (pRow->IOP_Type == IOPTY_DDH)
				// Double check that we still think we're in a NAC slot and chip.
					if (DM_Check_DDH_Slot())
					{
						// Set a flag so we know to set flags for internal drives.						m_fHaveDDHs = true;
						m_fHaveDDHs = true;
					}

			}  // end for...

	// Delete our table enumerate msg and reply to our initialize msg.
	delete pMsg;
	Reply(pInitializeMsg);

	return OS_DETAIL_STATUS_SUCCESS;
}


/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
STATUS DriveMonitorIsm::Enable(Message *pMsg) {
	
	TRACE_ENTRY(DriveMonitorIsm::Enable);
	
	// Old config had system loop # but in new we calculate it from local
	// loop (chip) #.  This must be done in Enable() since the config is
	// re-read after the Initalize() is done.
	if (config.version > DM_CONFIG_VERSION)
	{
		config.FC_instance = FCLOOPINSTANCE( Address::iSlotMe, config.FC_instance );
	}
	
	// check for and handle internal ddh / external legacy scan flag
	if (m_fHaveDDHs)
	{
	
		xlate_t ddh_xlate = {	
			0, 1, 2, 3, 4,				// drive ID translation table
			8, 9, 10, 11, 12,			// for use with Disk Drive Hubs (4)
			16, 17, 18, 19, 20,
			24, 25, 26, 27, 28
		};
			
		config.num_drives = 20;			// number of drives to scan
		for (U32 i = 0; i < config.num_drives; i++ )
			config.xlate[i] = ddh_xlate[i];
		config.flags |= (				// Turn on DM Flags for int ports
							DM_FLAGS_SPIN_4X
						);
		config.flags &= ~(				// Turn off DM Flags for ext ports
							DM_FLAGS_SCAN_ALL		|
							DM_FLAGS_SCAN_8_LUNS	|
							DM_FLAGS_USE_GET_LUNS
						);

	}
	else
	{
		xlate_t ext_xlate = {	
			0, 1, 2, 3, 4,				// drive ID translation table
			5, 6, 7, 8, 9,				// for legacy systems
			10, 11, 12, 13, 14,
			15, 16, 17, 18, 19
		};
			
		config.num_drives = 126;		// number of drives to scan
		for (U32 i = 0; i < config.num_drives; i++ )
			config.xlate[i] = ext_xlate[i];
		config.flags |= (				// Turn on DM Flags for ext ports
							DM_FLAGS_SPIN_4X 		|
							DM_FLAGS_SCAN_ALL		|
							DM_FLAGS_SCAN_8_LUNS	|
							DM_FLAGS_USE_GET_LUNS
						);
	}
	 
	// Get our table data from the PTS
	DmTableInitialize(pMsg);

	//Reply(pMsg);  // don't reply until PTS table init is done

	return OS_DETAIL_STATUS_SUCCESS;
	
} // Enable

/*************************************************************************/
// Quiesce
// Turn off the DDM belonging to this derived class
/*************************************************************************/
STATUS DriveMonitorIsm::Quiesce(Message *pMsg) {
	
	TRACE_ENTRY(DriveMonitorIsm::Quiesce);
	
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
	
} // Quiesce

/*************************************************************************/
// DoWork
// This derived classes method to receive messages and replies
/*************************************************************************/
STATUS DriveMonitorIsm::DoWork(Message *pMsg) {
	
	DM_TRACE_ENTRY(DriveMonitorIsm::DoWork);

	// This ISM handles Replies to messages sent from the monitor Handler
	if (pMsg->IsReply()) {
		
		// no reply should ever come here, since we use callbacks
	    DM_DUMP_HEX(TRACE_L8, "\n\rDriveMonitorIsm::DoWork Message Reply",
	    					(U8 *)pMsg, 128);
		return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
	}
	
	// New service message has been received
	else switch(pMsg->reqCode) {

		case DM_SCAN:
			// scan all devices configured
			DM_Scan_All_Devices(pMsg);
			break;

		case DM_SCAN_SINGLE:
			// scan a single device (in DmScan.cpp)
			DM_Scan_One_Device(pMsg);
			break;
	
		case DM_START:
			// start a single disk (in DmDisk.cpp)
			DM_Start_One_Disk(pMsg);
			break;

		case DM_STOP:
			// stop a single disk (in DmDisk.cpp)
			DM_Stop_One_Disk(pMsg);
			break;

		case DM_STOP_ALL:
			// stop all the disks found (in DmDisk.cpp)
			DM_Stop_All_Disks(pMsg);
			break;

		case DM_READ_VERIFY:
			// External entry for the Read through the Scsi Target Server Test code.
			// Read a single transfer of block_count sectors at logical_block_address.
			{
				DMT_CONTEXT			*pDmc;
				U32					drive_no = 0;	// always use drive 0
				DmReadVerifyTest	*pRvt = (DmReadVerifyTest *) pMsg;
			
				DM_TRACE_ENTRY(DM_READ_VERIFY);
			
				// allocate our context
				pDmc = new  DMT_CONTEXT;
				// allocate a message to go with it
				pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE));
				
				// save the reply pointer for later
				pDmc->pReply = pMsg;	
				
				// drive number index
				pDmc->drive_number = drive_no;
				pDmc->index = drive_no;
				
				pDmc->last = 1;
				
				// get the VirtualDevice number for the Scsi Target Server
				pDmc->Vd = pRvt->payload.vdnSTS;
				
				DM_PRINT_HEX16(TRACE_L2, "\n\rStarting Sts Read Verify Test, Vdn=", pDmc->Vd);
				
				// start the read state machine
				DM_Read_Drive_Block(pDmc, pRvt->payload.block,
											pRvt->payload.count);
			}
			
			break;

		case DM_WRITE_VERIFY:
			// External entry for the Write through the Scsi Target Server Test code.
			// Write a single transfer of block_count sectors at logical_block_address.
			{
				DMT_CONTEXT			*pDmc;
				U32					drive_no = 0;	// always use drive 0
				DmWriteVerifyTest	*pWvt = (DmWriteVerifyTest *) pMsg;
			
				DM_TRACE_ENTRY(DM_WRITE_VERIFY);
			
				// allocate our context
				pDmc = new (tSMALL|tUNCACHED) DMT_CONTEXT;
				// allocate a message to go with it
				pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE));
				
				// pass a NULL reply pointer along
				pDmc->pReply = NULL;	
				
				// drive number index
				pDmc->drive_number = drive_no;
				pDmc->index = drive_no;
				
				pDmc->last = 1;
				
				// get the VirtualDevice number for the Scsi Target Server
				pDmc->Vd = pWvt->payload.vdnSTS;
				
				DM_PRINT_HEX16(TRACE_L2, "\n\rStarting Sts Write Verify Test, Vdn=", pDmc->Vd);
				// start the read state machine
				DM_Write_Drive_Block(pDmc, pWvt->payload.block,
												pWvt->payload.count);
			}

			break;

		case DM_READ_TEST:
			// External entry for a Read through the Scsi Target Server Test code
			// or direct to a drive
			{
				DMT_CONTEXT		*pDmc;
				U32				drive_no;
				VDN				vdn;
				DmReadTest		*pRt = (DmReadTest *) pMsg;
			
				DM_TRACE_ENTRY(DM_READ_TEST);
			
				// get the test drive number
				drive_no = pRt->payload.drive;
				
				if (pRt->payload.type == DM_TYP_DRIVE)
				{
					// get the VirtualDevice number for the FCP Initiator
					vdn = config.vd;
	
				}
				else
				{
					// get the VirtualDevice number for the Scsi Target Server
					vdn = pRt->payload.vdnSTS;
				}
				
				// see if test is running
				if (DM_Drive_Test[drive_no] == 1)
				{
					DM_PRINT_HEX16(TRACE_L2, "\n\rStopping Read Test, Vdn=", vdn);
					DM_PRINT_HEX16(TRACE_L2, ", Id=", config.xlate[drive_no]);
					// stop the test
					DM_Drive_Test[drive_no] = 0;
					
					// answer back
					Reply(pMsg, 0);
					break;
				}
				
				// allocate our context
				pDmc = new (tSMALL|tUNCACHED) DMT_CONTEXT;
				// allocate a message to go with it
				pDmc->pMsg = new Message(SCSI_SCB_EXEC, sizeof(FCP_MSG_SIZE));
				
				// get the VirtualDevice number to send to
				pDmc->Vd = vdn;

				// pass a NULL reply pointer along
				pDmc->pReply = NULL;	
				
				// drive number index
				pDmc->drive_number = drive_no;
				pDmc->index = drive_no;
				
				pDmc->last = 1;
				
				// show test is running
				DM_Drive_Test[drive_no] = 1;
				
				DM_PRINT_HEX16(TRACE_L2, "\n\rStarting Read Test, Vdn=", pDmc->Vd);
				DM_PRINT_HEX16(TRACE_L2, ", Id=", pDmc->drive_number);
				// start the read state machine
				DM_Read_Drive(pDmc);
				
				Reply(pMsg, 0);
				
			}

			break;

		default:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
	}

	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
}	// DoWork
