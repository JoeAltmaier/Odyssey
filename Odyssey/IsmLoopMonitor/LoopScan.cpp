/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LoopScan.cpp
// 
// Description:
// This module has the methods to handle scanning in the Loop Monitor
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmLoopMonitor/LoopScan.cpp $
// 
// 9     2/09/00 6:04p Dpatel
// Fix for Jerry's bug of not being able to export LUNs to initiators
// on OUR initiator loop
// 
// 8     2/09/00 2:38p Mpanas
// Split wwName field to two fields, Port and Node
// Use Owner fieild of FCPortDatabase for flags
// 
// 7     1/14/00 5:57p Mpanas
// Fix bug: portStatus field is inccorredt for ports that belong
// to us.  Should always show FC_PORT_STATUS_ACTIVE
// when found
// 
// 6     12/09/99 9:14a Jlane
// Mike Panas added a delay so now reports the proper type for the HBA
// found on that loop.  Checked in by Carl W.
// 
// 5     11/22/99 5:46p Mpanas
// Retry if Portdatabase is not valid
// 
// 4     9/14/99 9:00p Mpanas
// Minor trace level changes
// 
// 3     9/10/99 3:15p Mpanas
// Fix problems
// - portType
// - FCDB table update
// - More debug
// 
// 2     8/14/99 9:28p Mpanas
// LoopMonitor version with most functionality
// implemented
// - Creates/Updates LoopDescriptor records
// - Creates/Updates FCPortDatabase records
// - Reads Export Table entries
// Note: This version uses the Linked List Container types
//           in the SSAPI Util code (SList.cpp)
// 
// 1     7/23/99 1:39p Mpanas
// Add latest versions of the Loop code
// 
// 
// 07/19/99 Michael G. Panas: Create file
/*************************************************************************/

#include "LmCommon.h"

#include "Pci.h"
#include "Scsi.h"
#include "CDB.h"
#include "CTIdLun.h"
#include "BuildSys.h"

#include "FcpData.h"
#include "FcpEvent.h"
#include "FcpLoop.h"
#include "FcpISP.h"
#include "FcpError.h"


/*************************************************************************/
// Forward references
/*************************************************************************/
extern "C" {
void delay_ms(signed long ms);
}
					
/*************************************************************************/
// Global references
/*************************************************************************/
static U32				LM_Drives;					// num of drives scanned
static U32				LM_Good_Drives;				// num of good drives
static U32				LM_Bad_Drives;				// num of bad drives
static U32				LM_Drive_Test[20] = {0};	// drive testing flags

// Translate Table for AL_PA to ID
// 0xff means invalid AL_PA
static	U8	AL_PA_ID[256] =
{
	0x7e,0x7d,0x7c,0xff,0x7b,0xff,0xff,0xff,	// 00 - 07
	0x7a,0xff,0xff,0xff,0xff,0xff,0xff,0x79,	// 08 - 0F
	0x78,0xff,0xff,0xff,0xff,0xff,0xff,0x77,	// 10 - 17
	0x76,0xff,0xff,0x75,0xff,0x74,0x73,0x72,	// 18 - 1F
	0xff,0xff,0xff,0x71,0xff,0x70,0x6f,0x6e,	// 20 - 27
	0xff,0x6d,0x6c,0x6b,0x6a,0x69,0x68,0xff,	// 28 - 2F
	0xff,0x67,0x66,0x65,0x64,0x63,0x62,0xff,	// 30 - 37
	0xff,0x61,0x60,0xff,0x5f,0xff,0xff,0xff,	// 38 - 3F
	0xff,0xff,0xff,0x5e,0xff,0x5d,0x5c,0x5b,	// 40 - 47
	0xff,0x5a,0x59,0x58,0x57,0x56,0x55,0xff,	// 48 - 4F
	0xff,0x54,0x53,0x52,0x51,0x50,0x4f,0xff,	// 50 - 57
	0xff,0x4e,0x4d,0xff,0x4c,0xff,0xff,0xff,	// 58 - 5F
	0xff,0xff,0xff,0x4b,0xff,0x4a,0x49,0x48,	// 60 - 67
	0xff,0x47,0x46,0x45,0x44,0x43,0x42,0xff,	// 68 - 6F
	0xff,0x41,0x40,0x3f,0x3e,0x3d,0x3c,0xff,	// 70 - 77
	0xff,0x3b,0x3a,0xff,0x39,0xff,0xff,0xff,	// 78 - 7F
	0x38,0x37,0x36,0xff,0x35,0xff,0xff,0xff,	// 80 - 87
	0x34,0xff,0xff,0xff,0xff,0xff,0xff,0x33,	// 88 - 8F
	0x32,0xff,0xff,0xff,0xff,0xff,0xff,0x31,	// 90 - 97
	0x30,0xff,0xff,0x2f,0xff,0x2e,0x2d,0x2c,	// 98 - 9F
	0xff,0xff,0xff,0x2b,0xff,0x2a,0x29,0x28,	// A0 - A7
	0xff,0x27,0x26,0x25,0x24,0x23,0x22,0xff,	// A8 - AF
	0xff,0x21,0x20,0x1f,0x1e,0x1d,0x1c,0xff,	// B0 - B7
	0xff,0x1b,0x1a,0xff,0x19,0xff,0xff,0xff,	// B8 - BF
	0xff,0xff,0xff,0x18,0xff,0x17,0x16,0x15,	// C0 - C7
	0xff,0x14,0x13,0x12,0x11,0x10,0x0f,0xff,	// C8 - CF
	0xff,0x0e,0x0d,0x0c,0x0b,0x0a,0x09,0xff,	// D0 - D7
	0xff,0x08,0x07,0xff,0x06,0xff,0xff,0xff,	// D8 - DF
	0x05,0x04,0x03,0xff,0x02,0xff,0xff,0xff,	// E0 - E7
	0x01,0xff,0xff,0xff,0xff,0xff,0xff,0x00,	// E8 - EF
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,	// F0 - F7
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff		// F8 - FF
};

// Translate Table for ID to AL_PA
static	U8	ID_AL_PA[128] =
{
	0xef,0xe8,0xe4,0xe2,0xe1,0xe0,0xdc,0xda,	// 00 - 07
	0xd9,0xd6,0xd5,0xd4,0xd3,0xd2,0xd1,0xce,	// 08 - 0F
	0xcd,0xcc,0xcb,0xca,0xc9,0xc7,0xc6,0xc5,	// 10 - 17
	0xc3,0xbc,0xba,0xb9,0xb6,0xb5,0xb4,0xb3,	// 18 - 1F
	0xb2,0xb1,0xae,0xad,0xac,0xab,0xaa,0xa9,	// 20 - 27
	0xa7,0xa6,0xa5,0xa3,0x9f,0x9e,0x9d,0x9b,	// 28 - 2F
	0x98,0x97,0x90,0x8f,0x88,0x84,0x82,0x81,	// 30 - 37
	0x80,0x7c,0x7a,0x79,0x76,0x75,0x74,0x73,	// 38 - 3F
	0x72,0x71,0x6e,0x6d,0x6c,0x6b,0x6a,0x69,	// 40 - 47
	0x67,0x66,0x65,0x63,0x5c,0x5a,0x59,0x56,	// 48 - 4F
	0x55,0x54,0x53,0x52,0x51,0x4e,0x4d,0x4c,	// 50 - 57
	0x4b,0x4a,0x49,0x47,0x46,0x45,0x43,0x3c,	// 58 - 5F
	0x3a,0x39,0x36,0x35,0x34,0x33,0x32,0x31,	// 60 - 67
	0x2e,0x2d,0x2c,0x2b,0x2a,0x29,0x27,0x26,	// 68 - 6F
	0x25,0x23,0x1f,0x1e,0x1d,0x1b,0x18,0x17,	// 70 - 77
	0x10,0x0f,0x08,0x04,0x02,0x01,0x00,0xff		// 78 - 7F
	
};


#pragma	pack(4) // why do we need this?

/*************************************************************************/
// LM_Scan_Loops
// Scan for info on the devices attached to the FC Loops
// If pMsg is non-zero, this means we were called from an incoming message
// then we must send a reply back to the caller when we are done
/*************************************************************************/
void LoopMonitorIsm::LM_Scan_Loops(Message *pMsg)
{
	U32				 loop;
	STATUS			 status;
	U16				 state, type;
	U8				 num_IDs;
	
	TRACE_ENTRY(LM_Scan_Loops);

	TRACE_STRING(TRACE_L2, "\n\rLM: Start Scan");

	// start with no good or bad loops
	
	for (loop = 0; loop < config.num_loops; loop++)
	{
	
		U32		FCinstance = config.FC_instance[loop];
		
		LM_Scan_A_Loop(FCinstance);
		
	}
	
} // LM_Scan_Loops


/*************************************************************************/
// LM_Scan_A_Loop
// Scan for info on the devices attached to this FC Loop
/*************************************************************************/
void LoopMonitorIsm::LM_Scan_A_Loop(U32 FCinstance)
{
	U32				 chip = FCLOOPCHIP(FCinstance);
	PINSTANCE_DATA	 Id = &Instance_Data[chip];
	LM_CONTEXT		*pLmc;
	STATUS			 status;
	U16				 state, type;
	U8				 loop_ids;
	U8				*buffer = (U8 *)new(tPCI) char[2048];
	U8				*map = (U8 *)new(tPCI) char[128];
	U32				 portType = 0;
	U16				 vp_index = 0, id;
	FCPortDatabaseRecord	*pDb;
	LM_FCDB_STATE	*pFCDBState;
	
	TRACE_ENTRY(LM_Scan_A_Loop);

	// allocate our context
	status = FCP_Get_Firmware_State(Id, &state);
	
	if (status != MB_STS_GOOD)
	{
		TRACE_HEX(TRACE_L2, "\n\rLM_Scan_Loops: FCP_Get_Firmware_State Failed - ", status);
	}
	else
	{
		TRACEF(TRACE_L2, ("\n\r\n\rLoop:%d state=%x", FCinstance, state));
	}
	
	if (state != 0x0003)
	{
		// loop is not ready
		LoopFlags[chip] = LM_STS_LOOP_DOWN;
		LM_Loop_Desc[chip]->ActualLoopState = LoopDown;
		
		// Update tables
		goto UpdateTable;
	}
	
	// get the AL_PA map for this loop
	status = FCP_Get_FC_AL_Position_Map(Id, map, &type);
	
	if (status != MB_STS_GOOD)
	{
		TRACE_HEX(TRACE_L2, "\n\rLM_Scan_Loops: LM_Get_Pos_Map Failed - ", status);
	}
	else
	{
		TRACE_HEX16(TRACE_L8, "\n\rMap Type = ", type);
		TRACE_DUMP_HEX(TRACE_L3, "\n\rFC_AL Map:", map, 128);
	}
	
	// set our portType based on the loop
	if ((Instance_Data[chip].FCP_config.enable_initiator_mode) &&
			(Instance_Data[chip].FCP_config.enable_target_mode))
		portType = FC_PORT_TYPE_TARGET_INITIATOR;
	else if (Instance_Data[chip].FCP_config.enable_initiator_mode)
		portType = FC_PORT_TYPE_INITIATOR;
	else if (Instance_Data[chip].FCP_config.enable_target_mode)
		portType = FC_PORT_TYPE_TARGET;
	
	TRACE_HEX(TRACE_L3, "\n\rLM_Scan_Loops: num containers: ", pPDB[chip]->Count());

	// get the number of IDs
	loop_ids = map[0];
	num_IDs[chip] = map[0];
	TRACE_HEX16(TRACE_L3, "\n\rNumber of IDs found = ", num_IDs[chip]);
		
	// Make an entry for us if it does not exist,
	// first try to get the record from the list
	id = AL_PA_ID[map[1]];
	if ((pPDB[chip]->Get((CONTAINER_ELEMENT &)pFCDBState, (CONTAINER_KEY)id)) == FALSE)
	{
		// did not exist
		pFCDBState = new(tPCI) LM_FCDB_STATE;
		memset(pFCDBState, 0, sizeof(LM_FCDB_STATE));
		pDb = new(tPCI) FCPortDatabaseRecord;
		pFCDBState->pDBR = pDb;
		//pFCDBState->state = FCDB_STATE_ADD;	// Add new entry
		memset(pDb, 0, sizeof(FCPortDatabaseRecord));
		
		// fill in the record
		pDb->version = FC_PORT_DATABASE_TABLE_VERSION;
		pDb->size = sizeof(FCPortDatabaseRecord);
		// assumes this is already valid
		pDb->ridLoopDescriptor = LM_Loop_Desc[chip]->rid;
		pDb->id = id;
		// TODO: wwn support
		//memcpy(&pDb->wwName[0], &buffer[20], 8);
		//memcpy(&pDb->wwnNodeName[0], &buffer[12], 8);
		// always mark active
		pDb->portStatus = FC_PORT_STATUS_ACTIVE;
		// set portType
		pDb->portType = portType;
		
		pDb->attribs = FC_PORT_OWNER_INTERNAL;		// ours
						
		// add to the list
		pPDB[chip]->Add((CONTAINER_ELEMENT)pFCDBState, (CONTAINER_KEY)id);
	}
	
	// skip self (must be at least 2 ?)
	loop_ids--;
	// read the Port Database for each loop ID
	for (int i = 2; loop_ids; i++, loop_ids--)
	{
		int trys = 200;		// set number of retries for get database
		
		id = AL_PA_ID[map[i]];
			
		if (id == 0xff)
			continue;		// invalid AL_PA, skip this one
		
		TRACE_HEX16(TRACE_L2, "\n\rID = ", id);

retry:
		status = FCP_Get_Port_Data_Base(Id, buffer, id, vp_index);
		
		if (status != MB_STS_GOOD)
		{
			TRACE_HEX(TRACE_L2, "\n\rLM_Scan_Loops: FCP_Get_Port_Data_Base Failed - ", status);
		}
		else
		{
			// check for a valid Port Database
			if ((buffer[0x4e] & 0x30) == 0)
			{
				// zero is not valid
				if (--trys)
				{
					// delay 10ms here
					delay_ms(10);
					
					TRACE_HEX16(TRACE_L3, "\n\rLM_Scan_Loops: FCP_Get_Port_Data_Base Rety = ", trys);
					goto retry;
				}
				else
				{
					// no more trys, Get Port DB is never going to work for this ID
					TRACEF(TRACE_L2, ("\n\rLM_Scan_Loops(%d): FCP_Get_Port_Data_Base, no more retries", FCinstance));
					
					status = MB_STS_INVALID_CMD;
				}
			}
			else
			{
				if (trys != 200)
				{
					TRACEF(TRACE_L2, ("\n\rLM_Scan_Loops(%d): FCP_Get_Port_Data_Base, retries=%d", 
															FCinstance, (200 - trys)));
				}
			}
			
			TRACE_DUMP_HEX(TRACE_L3, "\n\rPort Database:", buffer, 128);
			
			// try to get the record from the list
			if ((pPDB[chip]->Get((CONTAINER_ELEMENT &)pFCDBState, (CONTAINER_KEY)id)) == FALSE)
			{
				// did not exist
				pFCDBState = new(tPCI) LM_FCDB_STATE;
				memset(pFCDBState, 0, sizeof(LM_FCDB_STATE));
				pDb = new(tPCI) FCPortDatabaseRecord;
				pFCDBState->pDBR = pDb;
				//pFCDBState->state = FCDB_STATE_ADD;	// Add new entry
				memset(pDb, 0, sizeof(FCPortDatabaseRecord));
				
				// fill in the record
				pDb->version = FC_PORT_DATABASE_TABLE_VERSION;
				pDb->size = sizeof(FCPortDatabaseRecord);
				// assumes this is already valid
				pDb->ridLoopDescriptor = LM_Loop_Desc[chip]->rid;
				pDb->id = id;

				// add to the list
				pPDB[chip]->Add((CONTAINER_ELEMENT)pFCDBState, (CONTAINER_KEY)id);
			}
			else
			{
				pDb = pFCDBState->pDBR;
			}
			
			if (status != MB_STS_GOOD)
			{
				// mark a hard failure
				pDb->portStatus = 0xff;
			}
			else
			{
				// mark active
				pDb->portStatus = FC_PORT_STATUS_ACTIVE;

				// always update: WWN, type and Owner
				memcpy(&pDb->wwName[0], &buffer[20], 8);
				memcpy(&pDb->wwnNodeName[0], &buffer[12], 8);
				
				// Target bit 0, Initiator bit 1
				pDb->portType = (buffer[0x4e] >> 4) & 0x03;
				TRACE_HEX(TRACE_L3, "\n\rLM_Scan_Loops: portType: ", pDb->portType);
				
				// check if ours
				if (LM_Loop_Desc[chip]->IDsInUse && 
						(memchr(&LM_Loop_Desc[chip]->TargetIDs[0], id,
						   LM_Loop_Desc[chip]->IDsInUse)))
					pDb->attribs |= FC_PORT_OWNER_INTERNAL;		// ours
				else
					pDb->attribs &= ~FC_PORT_OWNER_INTERNAL;		// theirs
					
				// Check if this port is usable in a virtual circuit
				// must be on a target port and be an external initiator
				if (((portType == FC_PORT_TYPE_TARGET) || 
						(portType == FC_PORT_TYPE_TARGET_INITIATOR)) &&
							(pDb->portType == FC_PORT_TYPE_INITIATOR))
					pDb->attribs |= FC_PORT_OWNER_VC_USE_OK;		// OK in vc
				else
					pDb->attribs &= ~FC_PORT_OWNER_VC_USE_OK;		// not OK in vc
								
			}
		}
	}
	
	TRACE_HEX(TRACE_L3, "\n\rLM_Scan_Loops: num containers after scan: ", pPDB[chip]->Count());

	// unroll the FCPortDatabase containers, update the PTS
	// with the new port IDs that were found and/or removed
	for (int x = pPDB[chip]->Count() - 1; x >= 0; x--)
	{
		// get each container in sequence
		pPDB[chip]->GetAt((CONTAINER_ELEMENT &)pFCDBState, x);
		
		// check if the device is still active
		if ((pFCDBState->pDBR->portStatus == FC_PORT_STATUS_ACTIVE) && 
					(memchr(&map[1], ID_AL_PA[pFCDBState->pDBR->id], map[0])))
		{
			pFCDBState->pDBR->portStatus = FC_PORT_STATUS_ACTIVE;
		}
		// allow our ports to always show ACTIVE and bad ports to show REMOVED
		else if ((pFCDBState->pDBR->attribs & FC_PORT_OWNER_INTERNAL) == 0)
		{
			// not active anymore, device offline/removed
			pFCDBState->pDBR->portStatus = FC_PORT_STATUS_REMOVED;
		}
		
		// update or add an entry in the FCPortDatabaseRecord Table
		// no Callback, no state, just do it.
		LmDBTableUpdate(pFCDBState, NULL, (pLMCallback_t)NULL);
	}

	// this loop is now up
	LM_Loop_Desc[chip]->ActualLoopState = LoopUp;
	
	// do we have a DriveMonitor assigned to this loop?
	if (LM_Loop_Desc[chip]->vdnDriveMonitor)
	{
		// yes and since the loop is up, send a DM_SCAN message to it.
		LM_Send_DM_SCAN(chip);
	}

UpdateTable:

	// Update the table entry for this loop
	pLmc = new (tSMALL|tUNCACHED) LM_CONTEXT;
	memset(pLmc, 0, sizeof(LM_CONTEXT));
	pLmc->last = 1;
		
	LmLpTableUpdate(NULL, pLmc, FCinstance, (pLMCallback_t) &LM_Scan_Callback);

	// return local resources
	delete map;
	delete buffer;
	
} // LM_Scan_A_Loop


/*************************************************************************/
// LM_Send_DM_SCAN
// Send a DM_SCAN message to the DriveMonitor assigned to this loop
/*************************************************************************/
void LoopMonitorIsm::LM_Send_DM_SCAN(U32 chip )
{
	TRACE_ENTRY(LM_Send_DM_SCAN);
	
	Message	*pMsg = new Message(DM_SCAN);
	
	Send(LM_Loop_Desc[chip]->vdnDriveMonitor, pMsg, (void *) chip);

	TRACE_STRING(TRACE_L3, "\n\rLoopScan: DM_SCAN message sent");
	
	// this loop is waiting for the DM_SCAN reply now
 	LoopFlags[chip] = LM_STS_LOOP_DM_SCAN_REQ;
	
} // LM_Send_DM_SCAN


/*************************************************************************/
// LM_Scan_Callback
// Handle LoopDescriptor Update callback
/*************************************************************************/
void LoopMonitorIsm::LM_Scan_Callback(void *pClientContext, STATUS status)
{
	LM_CONTEXT		*pLmc = (LM_CONTEXT *)pClientContext;

	TRACE_ENTRY(LM_Scan_Callback);
	
	// delete the context if there was one
	if (pLmc)
		delete pLmc;
	
} // LM_Scan_Callback


