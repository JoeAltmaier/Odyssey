/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: BootMgrCmds.h
// 
// Description:
//    Boot Manager DDM control interface definitions.
//    These are the commands submitted to the Boot Manager DDM by other
//    DDMs in the Odyssey system.
//
//    Commands are submitted via class CmdSender, see include\CmdSender.h
//    and friends for the particulars.
// 
// Change log located at end of file.
/*************************************************************************/

#ifndef _BootMgrCmds_h_
#define _BootMgrCmds_h_

#ifndef __Address_h
# include  "Address.h"        // for TySlot
#endif

#ifndef Simple_H
# include  "Simple.h"         // for BOOL, U32, etc.
#endif

#ifndef CTtypes_H
# include  "CtTypes.h"        // for rowID
#endif

//  common BMGR control queue name root
#define     BMGR_CONTROL_QUEUE    "BMGRControlQueue"

//  size of BMGR control queue's command messages
#define     BMGR_CONTROL_COMMAND_SIZE   (sizeof (BMGRRequest))

//  size of BMGR control queue's status messages (unused)
#define     BMGR_CONTROL_STATUS_SIZE    (sizeof (BMGREvent))


//  here are the supported BMGR command codes:
enum BMGRCommand {
	k_eIOPTakeOutOfService = 1,		// Params are BMGRCmdIopTakeOutOfService.
	k_eIOPPowerOn,			// Params are BMGRCmdIOPPowerOn.
	k_eIOPIntoService,		// Params are BMGRCmdIOPIntoService.
	k_eIOPLock,				// Params are BMGRCmdIopLock
	k_eIOPUnlock,			// Params are BMGRCmdIopUnlock
	K_eNextBMGRCommand		// Still unused.
};


//  Here's our master command parameter struct.  This struct is used
//  both for command and status transfers.  We have no separate
//  "results" struct, we simply loop back the full command packet.
// 	NOte that this Struct will be returned in the status structure.
struct BMGRRequest {
	BMGRCommand	eCommand;  		// what this request is for -- "tags" member (u)
	TySlot slot;	
	
	TySlot GetSlot() { return slot; }
};

struct BMGREvent {
	BMGRCommand	eCommand;  		// what this request is for -- "tags" member (u)
	STATUS 		status;
	TySlot slot;
		
	BMGREvent(BMGRCommand eCommand_, STATUS status_, TySlot slot_) :
		eCommand(eCommand_), status(status_), slot(slot_) { }
};

// $Log: /Gemini/Include/BootMgrCmds.h $
// 
// 2     1/26/00 2:21p Joehler
// Added Take IOP Out Of Service, Power On, IOP Into Service, Lock and
// Unlock commands to the boot manager.
// 
// 1     10/28/99 11:16a Sgavarre
// 
// 


#endif   // #ifndef _BootMgrCmds_h_
