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
 *     11/14/98 Tom Nelson: Created
 *
**/



#define _TRACEF
#include "Trace_Index.h"
//#include "Trace.h"

#include "BuildSys.h" 

extern "C" U32 gSize_available_memory = 0x4000000;
extern "C" U32 gSize_small_heap = 0x900000;

//*
//* DDM Classes
//*

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER",   10240,1024, DdmManager);
CLASSENTRY("PTSPROXYLOADER",10240,1024,DdmPtsProxyLoader);      // Loads PtsProxy 
CLASSENTRY("PTSLOADER",    10240,1024, DdmPtsLoader);           // Loads real Pts
CLASSENTRY("VIRTUALPROXY", 10240,1024, DdmVirtualProxy);
//CLASSENTRY("DDMTRANSPORT", 10240,1024, Transport);
CLASSENTRY("DDMTIMER", 10240, 1024, DdmTimer );
CLASSENTRY("DDMSYSINFO",   10240,1024, DdmSysInfo);


CLASSENTRY("DDM_RMSTR", 10240,1024,DdmRAIDMstr);
CLASSENTRY("DDM_RMSTR_TEST", 10240,1024,DdmRAIDMstrTest);
CLASSENTRY("DDM_LOGMASTER",  10240,1024,DdmLogMaster);
CLASSENTRY("DDM_IOPLOG",  10240,1024,DdmEventLogIOP);
CLASSENTRY("DDMALARM",   10240,1024,DdmAlarm);


// CHAOS Ddm Classes (HBC ONLY)
CLASSENTRY("PTSPROXY",     10240,1024, DdmPtsProxy);
CLASSENTRY("HDM_TS",       10240,1024, DdmPTS );                // The real PTS

// Application Ddm Classes (This Processor only)
//CLASSENTRY("PONGMASTER",  10240,1024,DdmPongMaster);
//CLASSENTRY("PONGSLAVE",   10240,1024,DdmPongSlave);


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
//DEVICEENTRY("FailSafe",       FailSafe);


//*
//* System Ddms (No Virtual Device Numbers)
//*

// CHAOS Ddm Startups
SYSTEMENTRY("DDMMANAGER");              // Must be first!
SYSTEMENTRY("PTSPROXYLOADER");  // Must be before VirtualProxy
SYSTEMENTRY("PTSLOADER");       
SYSTEMENTRY("VIRTUALPROXY");    // Must be after PtsProxyLoader
//SYSTEMENTRY("DDMTRANSPORT" );
//SYSTEMENTRY("DDMTIMER");
//SYSTEMENTRY("DDMSYSINFO");

// CHAOS Startups (HBC ONLY)
//SYSTEMENTRY("FAILOVERPROXY");   // Only if you want to attempt failover!!!
SYSTEMENTRY("DDM_LOGMASTER");
SYSTEMENTRY("DDM_IOPLOG");
SYSTEMENTRY("DDMALARM");

SYSTEMENTRY("DDM_RMSTR" );	// Must be first!
SYSTEMENTRY("DDM_RMSTR_TEST" );	// Must be first!


//*
//* Virtual Device Data
//*

enum {
        vdn_PTS = 1,    // Must not be zero! 
        vdn_PTSPROXY,
                
        vdn_PM,                 // Pong Master
        vdn_PS                  // Pong Slaves
};

// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary

PTSPROXY("PTSPROXY",  vdn_PTSPROXY, IOP_HBC0, SLOTNULL);
PTSNAME("HDM_TS",         vdn_PTS,      IOP_HBC0, SLOTNULL);

// VirtualEntry config data

struct {} pddNone;

struct {VDN vdn; } pddPongMaster = { vdn_PS };


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

//** 1 Processor - no failover
//VIRTUALENTRY("PONGMASTER", TRUE, vdn_PM, &pddPongMaster,IOP_HBC0,SLOTNULL);
//VIRTUALENTRY("PONGSLAVE",  TRUE, vdn_PS, &pddNone,      IOP_HBC0,SLOTNULL);

//** 2 Processors - failover back to HBC
//VIRTUALENTRY("PONGMASTER", TRUE, vdn_PM, &pddPongMaster,IOP_HBC0,SLOTNULL);
//VIRTUALENTRY("PONGSLAVE",  TRUE, vdn_PS, &pddNone,      IOP_NIC1,IOP_HBC0);

//** 3 Processors - failover slave between IOP
//VIRTUALENTRY("PONGSLAVE",  TRUE, vdn_PS, &pddNone,    IOP_NIC1,IOP_RAC0);

//** 4 Processors - failover slave, master on 3rd IOP
//VIRTUALENTRY("PONGMASTER", TRUE, vdn_PM, &pddPongMaster, IOP_NIC0,SLOTNULL);
//VIRTUALENTRY("PONGSLAVE",  TRUE, vdn_PS, &pddNone,       IOP_NIC1,IOP_RAC0);


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







