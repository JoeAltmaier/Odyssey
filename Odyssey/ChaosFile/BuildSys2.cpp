/* BuildSys.cpp -- System Tables
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
 *
**/


#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include "BuildSys.h" 
#include "DdmPartitionMgr.h"

extern "C" U32 gSize_available_memory = 0x4000000;
extern "C" U32 gSize_small_heap = 0x900000;
extern "C" U32 gSizeReserved = 0;

//*
//* DDM Classes
//*

// New - 3rd parm is message Queue Size
// Parms - "ClassName", StackSize, QueueSize, LinkName



CLASSENTRY("DDM_MANAGER",  10240,1024,DdmManager);
//CLASSENTRY("PTSLOADER",10240,1024,DdmPtsLoader);      // Loads PtsProxy 
CLASSENTRY("PTSLOADER",    10240,1024, DdmPtsLoader);           // Loads real Pts
CLASSENTRY("DDMTIMER", 10240, 1024, DdmTimer );
CLASSENTRY("DDMSYSINFO",   10240,1024, DdmSysInfo);
CLASSENTRY("VIRTUALMANAGER", 10240,1024, DdmVirtualManager);
CLASSENTRY("DDMCMBNULL", 10240,1024, DdmCmbNull);

// CHAOS Ddm Classes (HBC ONLY)
CLASSENTRY("PTS",     10240,1024, DdmPTS);
CLASSENTRY("VMS",       10240,1024, DdmVirtualMaster );                // The real PTS
//CLASSENTRY("BOOTMAN",       10240,1024, DdmBootMgr );                

CLASSENTRY("PARTITIONMGR",  10240,1024,DdmPartitionMgr);

// Customize your test image here
CLASSENTRY("DDM_TESTCF",   10240,1024,DdmTestChaosFile);
//CLASSENTRY("HDM_TEST2",   10240,1024,DdmSccTest2);
//CLASSENTRY("DDMTIMERTEST",10240,1024,DdmTimerTest);
//CLASSENTRY("DDMSITEST",   10240,1024,DdmSiTest);
//*
//* Device Initialization
//*
//* Procedures called for initialization at system startup
//*

DEVICEENTRY("Interrupt", 	Interrupt);
DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DdmManager", 	DdmManager);
//DEVICEENTRY("PIT", 			TimerPIT);
//DEVICEENTRY("Transport", 	Transport);
//DEVICEENTRY("FailSafe", 	FailSafe);


//*
//* System Ddms (No Virtual Device Numbers)
//*

SYSTEMENTRY("DDM_MANAGER");	// Must be first!
SYSTEMENTRY("PTSLOADER");       
SYSTEMENTRY("DDMCMBNULL");    
SYSTEMENTRY("DDMSYSINFO");  
SYSTEMENTRY("PARTITIONMGR");
SYSTEMENTRY("VIRTUALMANAGER");
//*
//* Virtual Device Data
//*

enum {
        vdn_PTS = 1,    // Must not be zero! 
//        vdn_TS,
        vdn_VMS,
		vdn_PMGR,
		vdn_TESTCF
};


// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary


PTSNAME ("PTS",     vdn_PTS,  IOP_HBC0, IOP_HBC1);
VMSNAME ("VMS", 	vdn_VMS,  IOP_HBC0, IOP_HBC1);


VIRTUALENTRY("DDM_TESTCF",TRUE,  vdn_TESTCF, IOP_HBC0, SLOTNULL,  NoPtsTable, NoPtsData);

//*
//* Boot Configuration (Temporary)
//*

struct {int iCabinet; TySlot iSlot;} bddDdmManager = {
	0, IOP_HBC0
};

struct {long cbMsg; long sMsg; long cbBuf; long sBuf; long pciWindow; long sWindow; } bddMessenger = {
	256000, 256, 1048536, 8192, 0x2000000, 0x1000000
};
	
struct {long divisor;}  bddTimer={10000};

FF_CONFIG flashConfig;// = { FF_CONFIG_VERSION, sizeof(FF_CONFIG), NULL, 0, 1, 1, 0, 1, 1, 25, 1, 0, 0, 0, 0, 0, 0 };
CM_CONFIG cacheConfig;// = { CM_CONFIG_VERSION, sizeof(CM_CONFIG), 1024, 256, NULL, NULL, 0, 64, 60, 95, 0, 0};
PmConfig bddPartitionMgr={512, 2048, &flashConfig, &cacheConfig, true};

BOOTENTRY("DdmManager",	&bddDdmManager );
// BOOTENTRY("Transport",	&bddMessenger );
// BOOTENTRY("Timer",		&bddTimer );
BOOTENTRY("DdmPartitionMgr",	&bddPartitionMgr );


