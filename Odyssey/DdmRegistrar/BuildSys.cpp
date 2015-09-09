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


#include "BuildSys.h" 


//*
//* DDM Classes
//*

// New - 3rd parm is message Queue Size
// Parms - "ClassName", StackSize, QueueSize, LinkName

// 1st four entries are always there
CLASSENTRY("HDM_DMGR", 10240,1024,DdmManager);
//CLASSENTRY("HDM_TPT",  10240,1024,Transport);
//CLASSENTRY("HDM_TIMR", 10240,1024,DdmTimer);
CLASSENTRY("HDM_PTS",  10240,1024,PersistentData);
// Customize your test image here
CLASSENTRY("HDM_SCC",  10240,1024,DdmScc);
CLASSENTRY("HDM_TEST", 10240,1024,DdmSccTest);
CLASSENTRY("HDM_TEST2",10240,1024,DdmSccTest);

//*
//* Device Initialization
//*
//* Procedures called for initialization at system startup
//*

DEVICEENTRY("Interrupt", 	Interrupt);
DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DdmManager", 	DdmManager);
//DEVICEENTRY("Transport", 	Transport);
//DEVICEENTRY("FailSafe", 	FailSafe);


//*
//* System Ddms (No Virtual Device Numbers)
//*

SYSTEMENTRY("HDM_DMGR" );	// Must be first!
//SYSTEMENTRY("HDM_TPT"  );
//SYSTEMENTRY("HDM_TIMR" );


//*
//* Virtual Device Data
//*

enum {
	vdn_PTS	= 1,	// Must be one (1)
	vdn_TEST,
	vdn_SCC,
	vdn_TEST2
};

// Temporary PTS data

struct {} pddNone;
struct {VDN vd;} pddScc = {vdn_SCC};
struct {VDN vd; U16 id; } pddTest = {vdn_SCC,1};
struct {VDN vd; U16 id; } pddTest2={vdn_SCC,2};

//*
//* Virtual Device Ddms created at system startup.
//*
VIRTUALENTRY("HDM_PTS",  TRUE, vdn_PTS,  &pddNone,	IOP_HBC0,IOP_HBC1);
// Temporary entries until real PTS is operational
VIRTUALENTRY("HDM_SCC",  TRUE, vdn_SCC,  &pddScc,	IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_TEST", TRUE, vdn_TEST, &pddTest,	IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_TEST2",TRUE, vdn_TEST2,&pddTest2, IOP_HBC0,IOP_HBC1);


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


