/* BuildSys.cpp -- System Tables for DdmFailDemo
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
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
 *      7/08/99 Tom Nelson: Created
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

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER"	   ,10240,1024, DdmManager);
CLASSENTRY("DDMCMB",	    10240,1024, CDdmCMB);
CLASSENTRY("VIRTUALMANAGER",10240,1024, DdmVirtualManager);
CLASSENTRY("DDMTRANSPORT"  ,10240,1024, Transport);
CLASSENTRY("DDMTIMER"      ,10240,1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO"    ,10240,1024, DdmSysInfo);
CLASSENTRY("DDMSTATUS"	   ,10240,1024, DdmStatus);

// Application Ddm Classes (This Processor only)
CLASSENTRY("PONGSLAVE",    10240,1024, DdmPongSlave);
CLASSENTRY("DEMOLOCAL",		10240,1024, DdmDemo1);
//*
//* Device Initialization
//*
//* Procedures called for initialization at system startup
//*

// CHAOS Initialization
DEVICEENTRY("Interrupt", 	Interrupt);
DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DdmManager", 	DdmManager);
DEVICEENTRY("PIT",			TimerPIT);
DEVICEENTRY("Transport_Pci",Transport_Pci);
DEVICEENTRY("Transport", 	Transport);
//DEVICEENTRY("Network",		Network);
DEVICEENTRY("FailSafe", 	FailSafe);

//*
//* System Ddms
//*

// CHAOS Ddm Startups
SYSTEMENTRY("DDMMANAGER");		// Must be first!
SYSTEMENTRY("DDMTIMER");
SYSTEMENTRY("DDMSYSINFO");

SYSTEMENTRY("DDMCMB");

SYSTEMENTRY("DDMSTATUS");
SYSTEMENTRY("VIRTUALMANAGER");
SYSTEMENTRY("DDMTRANSPORT" );

//*
//* Boot Configuration (Temporary)
//*

struct {int iCabinet; TySlot iSlot;} bddDdmManager = {
	0, IOP_HBC0	// This is used only on the Eval Systems
};

struct {long cbMsg; long sMsg; long cbBuf; long sBuf; long pciWindow; long sWindow; } bddMessenger = {
	256000, 256, 1048536, 8192, 0x2000000, 0x1000000
};

BOOTENTRY("DdmManager",	&bddDdmManager );
BOOTENTRY("Transport",	&bddMessenger );


