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

// Size of all available memory:
extern "C" U32 gSize_available_memory=0x00800000;

extern "C" U32 gSize_small_heap = 0x900000;
// Size of memory reserved for old-fashioned Nucleus memory allocation
extern "C" U32 gSizeReserved=0;	
extern "C" U32 gSize_total_memory =0x400000;

//*
//* DDM Classes
//*

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER",   10240,1024, DdmManager);
CLASSENTRY("PTSLOADER",    10240,1024, DdmPtsLoader);           // Loads real Pts
CLASSENTRY("DDMTIMER", 10240, 1024, DdmTimer );
CLASSENTRY("DDMSYSINFO",   10240,1024, DdmSysInfo);
CLASSENTRY("VMS"          ,10240,1024, DdmVirtualMaster);
CLASSENTRY("VIRTUALMANAGER",10240,1024, DdmVirtualManager);
CLASSENTRY("DDM_LEAK",10240,1024, DdmLeak );


// CHAOS Ddm Classes (HBC ONLY)
CLASSENTRY("HDM_TS",       10240,1024, DdmPTS );                // The real PTS

// Application Ddm Classes (This Processor only)

#ifdef WIN32
CLASSENTRY("HDM_NWK_MGR",10240,1024,DdmNwkMgr);
#else
CLASSENTRY("HDM_NWK_MGR",10240,1024,DdmNet);
#endif
CLASSENTRY("HDM_SSERV",10240,1024,DdmSocketServer);
CLASSENTRY("HDM_SSAPI",20240,1024,DdmSSAPI);
//CLASSENTRY("HDM_SNMP",10240,1024,DdmSnmp);
CLASSENTRY("DDM_TEST", 10240, 1024, DdmSsapiTest );
CLASSENTRY("DDM_LOGMASTER",  10240,1024,DdmLogMaster);
CLASSENTRY("DDM_IOPLOG",  10240,1024,DdmEventLogIOP);
CLASSENTRY("DDM_RMSTR", 10240,1024,DdmRAIDMstr);
CLASSENTRY("DDM_VCM", 10240,1024,DdmVCM);
CLASSENTRY( "DDM_ALARM_MASTER", 10240, 1024, DdmAlarm );
CLASSENTRY("DDM_PARTITION_MSTR", 10240, 1024, DdmPartitionMstr );
CLASSENTRY("DDM_NULL", 10240, 1024, DdmNull );
CLASSENTRY("DDM_UPGRADE_MASTER", 10240, 1024, DdmUpgrade );
CLASSENTRY("DDM_FILE_SYSTEM", 10240, 1024, DdmFileSystem );
CLASSENTRY("DDM_PROFILE", 10240, 1024, DdmProfile );

//*
//* Device Initialization
//*
//* Procedures called for initialization at system startup
//*

// CHAOS Initialization
//DEVICEENTRY("Interrupt",        Interrupt);
//DEVICEENTRY("Dma",                      Dma);
DEVICEENTRY("DdmManager",       DdmManager);
DEVICEENTRY("DDM_PROFILE",                      Profile);
DEVICEENTRY("DDM_LEAK",                      DdmLeak);
//DEVICEENTRY("PIT",                      TimerPIT);
//DEVICEENTRY("Transport",        Transport);
//DEVICEENTRY("FailSafe",       FailSafe);
#ifndef WIN32
DEVICEENTRY("Network",          Network);
#endif

//*
//* System Ddms (No Virtual Device Numbers)
//*

// CHAOS Ddm Startups
SYSTEMENTRY("DDMMANAGER");              // Must be first!
SYSTEMENTRY("PTSLOADER");       
SYSTEMENTRY("VIRTUALMANAGER");

// CHAOS Startups (HBC ONLY)
SYSTEMENTRY("DDMTIMER");
SYSTEMENTRY("DDM_LOGMASTER");
SYSTEMENTRY("DDM_IOPLOG");
SYSTEMENTRY("DDM_TEST");
SYSTEMENTRY("HDM_NWK_MGR");
SYSTEMENTRY("HDM_SSERV");
SYSTEMENTRY("HDM_SSAPI");
SYSTEMENTRY("DDM_RMSTR" );
SYSTEMENTRY("DDM_VCM" );
SYSTEMENTRY("DDM_ALARM_MASTER" );
SYSTEMENTRY("DDM_PARTITION_MSTR");
//SYSTEMENTRY("DDM_UPGRADE_MASTER");
//SYSTEMENTRY("DDM_FILE_SYSTEM");
SYSTEMENTRY("DDM_PROFILE");
SYSTEMENTRY("DDM_LEAK");
//SYSTEMENTRY("HDM_SNMP");


//*
//* Virtual Device Data
//*

enum {
        vdn_PTS = 1,    // Must not be zero! 
		vdn_VMS,
                
        vdn_PM,                 // Pong Master
        vdn_PS                  // Pong Slaves
};

// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary

PTSNAME("HDM_TS",         vdn_PTS,      IOP_HBC0, SLOTNULL);
VMSNAME("VMS", vdn_VMS, IOP_HBC0, SLOTNULL);

// VirtualEntry config data

struct {} pddNone;

struct {VDN vdn; } pddPongMaster = { vdn_PS };

VIRTUALENTRY("DDM_NULL", TRUE, vdn_PM, IOP_HBC0,SLOTNULL, NoPtsTable, NoPtsData);

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



