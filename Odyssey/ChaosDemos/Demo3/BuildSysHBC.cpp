/* BuildSys.cpp -- Linkage for DdmDemo3
 *
 * Copyright (C) Dell Computer, 2000
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Revision History:
 *      2/13/00 Tom Nelson: Created
 *
**/


#define _TRACEF
#include "Odyssey_Trace.h"

#include "BuildSys.h" 

extern "C" U32 gSizeReserved = 0x1000;
extern "C" U32 gSize_available_memory = 0x800000;
extern "C" U32 gSize_small_heap = 0x400000;

//*
//* DDM Classes
//*

// Classes to be linked into this image
//
CLASSENTRY("DDMMANAGER"	   ,20480,1024, DdmManager);
CLASSENTRY("DDMTIMER"      ,20480,1024, DdmTimer); 

// Required for multislot boot
CLASSENTRY("DDMTRANSPORT"  ,10240,1024, Transport);
CLASSENTRY("DDMCMB" 	   ,20480,1024, CDdmCMB);

// Required for multislot HBC boot
CLASSENTRY("PTS"		   ,20480,1024, DdmPTS);
CLASSENTRY("BOOTMASTER"    ,20480,1024, DdmBootMasterProxy);

// Demo Class
CLASSENTRY("DDMPONGMASTER" ,10240,1024, DdmPongMaster);

//*
//* Device Initialization
//*

// Procedures called for initialization at system startup
//
DEVICEENTRY("DdmManager", 	DdmManager);
DEVICEENTRY("FailSafe", 	FailSafe);
DEVICEENTRY("PIT",			TimerPIT);
DEVICEENTRY("Interrupt", 	Interrupt);	

// Required for Transport
DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("Transport_Pci", Transport_Pci);
DEVICEENTRY("Transport", 	Transport);

//*
//* Virtual Device Data
//*

enum {
	vdn_PTS	= 1,	// Must not be zero! 
	vdn_BOOT,
		
	vdn_PONG 		// Pong Master
};

//*
//* Device Startup
//*

// Ddms to be started
//
SYSTEMENTRY("DDMMANAGER");		// Must be first!
SYSTEMENTRY("DDMTIMER");

SYSTEMMASTER("PTS",vdn_PTS);
SYSTEMENTRY("DDMCMB");
SYSTEMMASTER("BOOTMASTER",vdn_BOOT);
SYSTEMENTRY("DDMTRANSPORT" );

SYSTEMMASTER("DDMPONGMASTER", vdn_PONG);

//*
//* Configuration Data
//*

// This is used only under Windows and the Eval Systems
//
struct {int iCabinet; TySlot iSlot;} bddDdmManager = {
	0, IOP_HBC0	
};

// Transport configuration required even if transport is not loaded
//
struct {long cbMsg; long sMsg; long cbBuf; long sBuf; long pciWindow; long sWindow; } bddMessenger = {
	256000, 256, 1048536, 8192, 0x2000000, 0x1000000
};

BOOTENTRY("DdmManager",	&bddDdmManager );
BOOTENTRY("Transport",	&bddMessenger );


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosDemos/Demo3/BuildSysHBC.cpp $
// 
// 1     2/13/00 7:05p Tnelson
//  Demo shows communication between a SYSTEMENTRY Ddm on an IOP and a
// SYSTEMMASTER Ddm on an HBC.
// 
// 
// 1     2/12/00 9:15p Tnelson
// Demo shows communication between two Ddms using a derived request
// message delivered via SERVELOCAL.  Shows minimum build to run a Ddm on
// a single HBC.
// 
// 1     2/12/00 6:15p Tnelson
// Demo minimum build to run a Ddm on a single HBC
// 