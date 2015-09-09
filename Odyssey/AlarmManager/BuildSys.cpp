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
 *
**/

// $Log: /Gemini/Odyssey/AlarmManager/BuildSys.cpp $
// 
// 6     10/12/99 5:58p Joehler
// Added Alarm Master event reporting and Quiesce method
// 
// 5     9/29/99 6:46p Joehler
// Modified buildsys for modified pts table
// 
// 4     9/02/99 11:46a Joehler
// added comments

#define _TRACEF
#include "Odyssey_Trace.h"

#include "BuildSys.h" 

extern "C" U32 gSizeReserved = 0x400;
extern "C" U32 gSize_available_memory = 0x800000;
extern "C" U32 gSize_small_heap = 0x200000;

//*
//* DDM Classes
//*
//*
//* DDM Classes
//*

// New - 3rd parm is message Queue Size
// Parms - "ClassName", StackSize, QueueSize, LinkName

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER"    ,10240,1024, DdmManager);
CLASSENTRY("PTSLOADER"     ,10240,1024, DdmPtsLoader);
//CLASSENTRY("VIRTUALMANAGER",10240,1024, DdmVirtualManager);
//CLASSENTRY("DDMTRANSPORT"  ,10240,1024, Transport);
CLASSENTRY("DDMTIMER"      ,10240,1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO"    ,10240,1024, DdmSysInfo);
//CLASSENTRY("DDMCMB"                ,10240,1024, CDdmCMB);       // Use DdmCmbNull on Eval

// CHAOS Ddm Classes (HBC ONLY)
CLASSENTRY("PTS"                  ,10240,1024, DdmPTS);
//CLASSENTRY("VMS"                  ,10240,1024, DdmVirtualMaster);
//CLASSENTRY("BOOTMAN"      ,10240,1024, DdmBootMgr);

// Application Ddm Classes (This Processor only)
//CLASSENTRY("PONGMASTER",   10240,1024, DdmPongMaster);
//CLASSENTRY("PONGSLAVE",    10240,1024, DdmPongSlave);

// Log
CLASSENTRY("DDM_LOGMASTER",  10240,1024,DdmLogMaster);
CLASSENTRY("DDM_IOPLOG",  10240,1024,DdmEventLogIOP);

// Customize your test image here
CLASSENTRY("DDMALARM",   10240,1024,DdmAlarm);
CLASSENTRY("DDMMASTERDRIVER",10240,1024,DdmMasterDriver);
CLASSENTRY("DDMSSAPIDRIVER", 10240, 1024, DdmSSAPIDriver);

//*
//* Device Initialization
//*
//* Procedures called for initialization at system startup
//*

// CHAOS Initialization
//DEVICEENTRY("Interrupt",        Interrupt);
//DEVICEENTRY("Dma",                      Dma);
DEVICEENTRY("DdmManager",       DdmManager);
//DEVICEENTRY("PIT",                      TimerPIT);
//DEVICEENTRY("Transport",        Transport);
//DEVICEENTRY("FailSafe",         FailSafe);

//*
//* System Ddms (No Virtual Device Numbers)
//*

// CHAOS Ddm Startups
SYSTEMENTRY("DDMMANAGER");              // Must be first!
SYSTEMENTRY("PTSLOADER");
SYSTEMENTRY("DDM_LOGMASTER");
SYSTEMENTRY("DDM_IOPLOG");
//SYSTEMENTRY("DDMTRANSPORT" );
//SYSTEMENTRY("DDMTIMER");
//SYSTEMENTRY("DDMSYSINFO");
//SYSTEMENTRY("VIRTUALMANAGER");

// customize your test image here
SYSTEMENTRY("DDMALARM");
SYSTEMENTRY("DDMSSAPIDRIVER");
SYSTEMENTRY("DDMMASTERDRIVER");

//*
//* Virtual Device Data
//*

enum {
        vdn_PTS = 1,    // Must not be zero! 
        vdn_VMS,
                
        //vdn_PM,                 // Pong Master
        //vdn_PS                  // Pong Slaves
		vdn_DDMMASTERDRIVER,
		vdn_DDMALARM
};

PTSNAME("PTS", vdn_PTS, IOP_HBC0, IOP_HBC1);
VMSNAME("VMS", vdn_VMS, IOP_HBC0, IOP_HBC1);

//#include "PongTable.h"  // See example: include\cttables\VirtualDeviceTable.h

//PongRecord pddPongMaster(vdn_PS);

// VirtualEntry config data

struct {} pddNone;

//struct {VDN vdn; } pddPongMaster = { vdn_PS };


//*
//* VirtualEntry Ddms created at system startup.
//*
//   *** ALL VIRTUALENTRY ITEMS FOR ALL PROCESSORS ***
//       *** ARE SPECIFIED IN THE HBC BUILDSYS         ***
//
//       *** CLASSENTRY ITEMS NEED ONLY BE SPECIFIED   ***
//       *** IN THE BUILDSYS ON THE PROCESSOR THEY ARE ***
//   *** TO RUN ON                                 ***
//
//   *** IF YOU DO NOT WANT FAILOVER SPECIFY SECONDARY SLOT AS SLOTNULL ***
//   *** IF YOU WANT IOP_LOCAL SPECIFY BOTH SLOTS AS IOP_LOCAL          ***

//VIRTUALENTRY("PONGMASTER", TRUE, vdn_PM, IOP_HBC0,SLOTNULL, PongRecord &pddPongMaster);
//VIRTUALENTRY("PONGSLAVE",  TRUE, vdn_PS, IOP_RAC0,SLOTNULL, NoPtsTable, NoPtsData);

VIRTUALENTRY("DDMMASTERDRIVER", TRUE, vdn_DDMMASTERDRIVER, IOP_HBC0, SLOTNULL, NoPtsTable, NoPtsData);
//VIRTUALENTRY("DDMALARM", TRUE, vdn_DDMALARM, IOP_HBC0, SLOTNULL, NoPtsTable, NoPtsData);

//*
//* Boot Configuration (Temporary)
//*

struct {int iCabinet; TySlot iSlot;} bddDdmManager = {
        0, IOP_HBC0     // This is used only on the Eval Systems
};

struct {long cbMsg; long sMsg; long cbBuf; long sBuf; long pciWindow; long sWindow; } bddMessenger = {
        256000, 256, 1048536, 8192, 0x2000000, 0x1000000
};

BOOTENTRY("DdmManager", &bddDdmManager );
BOOTENTRY("Transport",  &bddMessenger );

