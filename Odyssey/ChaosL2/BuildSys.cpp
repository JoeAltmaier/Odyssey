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
#include "Odyssey_Trace.h"

#include "BuildSys.h" 

extern "C" U32 gSize_available_memory = 0x4000000;
extern "C" U32 gSize_small_heap = 0x900000;

//*
//* DDM Classes
//*

// New - 3rd parm is message Queue Size
// Parms - "ClassName", StackSize, QueueSize, LinkName


CLASSENTRY("DDM_MANAGER",  10240,1024,DdmManager);
CLASSENTRY("PTSPROXYLOADER",10240,1024,DdmPtsProxyLoader);      // Loads PtsProxy 
CLASSENTRY("PTSLOADER",    10240,1024, DdmPtsLoader);           // Loads real Pts
CLASSENTRY("VIRTUALPROXY", 10240,1024, DdmVirtualProxy);
CLASSENTRY("DDMTIMER", 10240, 1024, DdmTimer );
CLASSENTRY("DDMSYSINFO",   10240,1024, DdmSysInfo);

// CHAOS Ddm Classes (HBC ONLY)
CLASSENTRY("PTSPROXY",     10240,1024, DdmPtsProxy);
CLASSENTRY("HDM_TS",       10240,1024, DdmPTS );                // The real PTS

CLASSENTRY("DDM_LOGMASTER",  10240,1024,DdmLogMaster);
CLASSENTRY("DDM_IOPLOG",  10240,1024,DdmEventLogIOP);

// Customize your test image here
CLASSENTRY("DDM_TESTLM",   10240,1024,DdmTestLM);
//CLASSENTRY("HDM_TEST2",   10240,1024,DdmSccTest2);
//CLASSENTRY("DDMTIMERTEST",10240,1024,DdmTimerTest);
//CLASSENTRY("DDMSITEST",   10240,1024,DdmSiTest);
//*
//* Device Initialization
//*
//* Procedures called for initialization at system startup
//*

//DEVICEENTRY("Interrupt", 	Interrupt);
//DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DdmManager", 	DdmManager);
//DEVICEENTRY("Transport", 	Transport);
//DEVICEENTRY("FailSafe", 	FailSafe);


//*
//* System Ddms (No Virtual Device Numbers)
//*

SYSTEMENTRY("DDM_MANAGER");	// Must be first!
SYSTEMENTRY("PTSPROXYLOADER");  // Must be before VirtualProxy
SYSTEMENTRY("PTSLOADER");       
SYSTEMENTRY("VIRTUALPROXY");    // Must be after PtsProxyLoader
SYSTEMENTRY("DDM_LOGMASTER");
SYSTEMENTRY("DDM_IOPLOG");
//*
//* Virtual Device Data
//*

enum {
        vdn_PTS = 1,    // Must not be zero! 
        vdn_TS,
		vdn_LM,
		vdn_TESTLM
};


// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary


PTSPROXY("PTSPROXY",   vdn_PTS, IOP_HBC0, IOP_HBC1);
PTSNAME ("HDM_TS",     vdn_TS,  IOP_HBC0, IOP_HBC1);


// Temporary PTS data

struct {} pddNone;


VIRTUALENTRY("DDM_TESTLM",TRUE,  vdn_TESTLM,	 &pddNone,  IOP_HBC0,SLOTNULL);

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

	struct {
		U32 cPages;
		U16 bytesPerPage;
	} bddPartitionMgr={1024, 1024};

BOOTENTRY("DdmManager",	&bddDdmManager );
// BOOTENTRY("Transport",	&bddMessenger );
// BOOTENTRY("Timer",		&bddTimer );
BOOTENTRY("DdmLogMaster",	&pddNone );
BOOTENTRY("DdmEventLogIOP",	&pddNone );


