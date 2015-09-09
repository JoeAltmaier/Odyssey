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

extern "C" U32 gSize_available_memory = 0x800000;
extern "C" U32 gSize_small_heap = 0x200000;

//*
//* DDM Classes
//*

// New - 3rd parm is message Queue Size
// Parms - "ClassName", StackSize, QueueSize, LinkName

// 1st four entries are always there
CLASSENTRY("DDM_MANAGER",  10240,1024,DdmManager);
//CLASSENTRY("HDM_TRANSPORT",10240,1024,Transport);
CLASSENTRY("DDM_TIMER",    10240,1024,DdmTimer);
CLASSENTRY("DDM_SYSINFO",  10240,1024,DdmSysInfo);
CLASSENTRY("HDM_PTS",     10240,1024,PersistentData);

// Customize your test image here
//CLASSENTRY("HDM_TEST1",   10240,1024,DdmSccTest1);
//CLASSENTRY("HDM_TEST2",   10240,1024,DdmSccTest2);
CLASSENTRY("DDMTIMERTEST",10240,1024,DdmTimerTest);
CLASSENTRY("DDMSITEST",   10240,1024,DdmSiTest);
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
//SYSTEMENTRY("HDM_TRANSPORT" );
SYSTEMENTRY("DDM_TIMER");
SYSTEMENTRY("DDM_SYSINFO");

//*
//* Virtual Device Data
//*

enum {
	vdn_PTS	= 1,	// Must not be zero!
	vdn_SCC,
	vdn_TEST1,
	vdn_TEST1B,
	vdn_TEST2,
	vdn_TT,
	vdn_SI,
	vdn_SCC2
};

// Temporary PTS data

struct {} pddNone;
struct {VDN vdn;} pddScc = {vdn_SCC};
struct {VDN vdn; U16 id; } pddTest1 = {vdn_SCC,1};
struct {VDN vdn; U16 id; } pddTest1B= {vdn_SCC,2};
struct {VDN vdn; U16 id; } pddTest2 = {vdn_SCC,3};

#if 0
// Name/Location of PTS
PTSNAME("HDM_PTS",	vdn_PTS, IOP_HBC0, IOP_HBC1);

// PTS Entries (on PTS IOP only)
PTSENTRY("HDM_PTS",  	TRUE,  vdn_PTS,	&pddNone,	IOP_HBC0, IOP_HBC1);
PTSENTRY("DDMSITEST",	TRUE,  vdn_SI,	&pddNone,	IOP_HBC0, IOP_HBC1);
PTSENTRY("DDMTIMERTEST",TRUE,  vdn_TT,	&pddNone,	IOP_HBC0, IOP_HBC1);

#else
//*
//* Virtual Device Ddms created at system startup.
//*
VIRTUALENTRY("HDM_PTS",  TRUE, vdn_PTS,  &pddNone,	IOP_HBC0,IOP_HBC1);
// Temporary entries until real PTS is operational
//VIRTUALENTRY("HDM_SCC",  FALSE, vdn_SCC,  &pddScc,	IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("DDMSITEST",TRUE,  vdn_SI,	 &pddNone,  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("DDMTIMERTEST",TRUE,  vdn_TT,   &pddNone,	IOP_HBC0,IOP_HBC1);
//VIRTUALENTRY("HDM_TEST1",TRUE, vdn_TEST1,&pddTest1,	IOP_HBC0,IOP_HBC1);
//VIRTUALENTRY("HDM_TEST1",TRUE, vdn_TEST1B,&pddTest1B,IOP_HBC0,IOP_HBC1);
//VIRTUALENTRY("HDM_TEST2",TRUE, vdn_TEST2,&pddTest2, IOP_HBC0,IOP_HBC1);
#endif

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

BOOTENTRY("DdmManager",	&bddDdmManager );
BOOTENTRY("Transport",	&bddMessenger );
BOOTENTRY("Timer",		&bddTimer );


