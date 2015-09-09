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
CLASSENTRY("DDM_MANAGER", 10240,1024,DdmManager);
//CLASSENTRY("HDM_TPT",  10240,1024,Transport);
CLASSENTRY("DDM_TIMER", 10240,1024,DdmTimer);
CLASSENTRY("DDM_SYSINFO",  10240,1024,DdmSysInfo);
CLASSENTRY("HDM_PTS",  10240,1024,PersistentData);
// Customize your test image here
//CLASSENTRY("HDM_SCC",  10240,1024,DdmScc);
CLASSENTRY("DDM_EIS_SD", 10240, 1024,DdmScriptDirector);
CLASSENTRY("DDM_EIS_TM", 10240, 1024,DdmTriggerMgr);
CLASSENTRY("DDM_ELOG_IOP", 10240, 1024,DdmEventLogIOP);


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

SYSTEMENTRY("DDM_MANAGER" );	// Must be first!
//SYSTEMENTRY("HDM_TPT"  );
SYSTEMENTRY("DDM_TIMER" );
SYSTEMENTRY("DDM_SYSINFO");

//*
//* Virtual Device Data
//*

enum {
	vdn_PTS	= 1,	// Must be one (1)
	vdn_EIS_SD,
	vdn_EIS_TM,
	vdn_ELOG_IOP
};

// Temporary PTS data

struct {} pddNone;
//struct {VDN vdn;} pddScc = {vdn_SCC};

//*
//* Virtual Device Ddms created at system startup.
//*
VIRTUALENTRY("HDM_PTS",  TRUE, vdn_PTS,  &pddNone,	IOP_HBC0,IOP_HBC1);
// Temporary entries until real PTS is operational
//VIRTUALENTRY("HDM_SCC",  TRUE, vdn_SCC,  &pddScc,	IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("DDM_EIS_SD",TRUE, vdn_EIS_SD,&pddNone, IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("DDM_EIS_TM",TRUE, vdn_EIS_TM,&pddNone, IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("DDM_ELOG_IOP",TRUE, vdn_ELOG_IOP,&pddNone, IOP_HBC0,IOP_HBC1);


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


