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
CLASSENTRY("PTSLOADER"     ,10240,1024, DdmPtsLoader);
CLASSENTRY("VIRTUALMANAGER",10240,1024, DdmVirtualManager);
CLASSENTRY("DDMTRANSPORT"  ,10240,1024, Transport);
CLASSENTRY("DDMTIMER"      ,10240,1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO"    ,10240,1024, DdmSysInfo);

// CHAOS Ddm Classes (HBC ONLY)
CLASSENTRY("PTS"		  ,20240,1024, DdmPTS);
CLASSENTRY("VMS"		  ,20240,1024, DdmVirtualMaster);
CLASSENTRY("BOOTMAN",	   20240,1024, DdmBootMgr);
CLASSENTRY("FAILOVER"	  ,10240,1024, DdmFailoverProxy);

// Application Ddm Classes (This Processor only)
CLASSENTRY("PONGMASTER",   10240,1024, DdmPongMaster);
CLASSENTRY("PONGSLAVE",    10240,1024, DdmPongSlave);

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
DEVICEENTRY("Transport_Pci", Transport_Pci);
DEVICEENTRY("Transport", 	Transport);
//DEVICEENTRY("Network",		Network);
DEVICEENTRY("FailSafe", 	FailSafe);


//*
//* System Ddms (No Virtual Device Numbers)
//*

// CHAOS Ddm Startups
SYSTEMENTRY("DDMMANAGER");		// Must be first!
SYSTEMENTRY("DDMTIMER");
SYSTEMENTRY("DDMSYSINFO");

SYSTEMENTRY("PTSLOADER");
SYSTEMENTRY("DDMCMB");
SYSTEMENTRY("BOOTMAN");

SYSTEMENTRY("VIRTUALMANAGER");
SYSTEMENTRY("DDMTRANSPORT" );
SYSTEMENTRY("FAILOVER");

// CHAOS Startups (HBC ONLY)
//SYSTEMENTRY("FAILOVERPROXY");	// Only if you want to attempt failover!!!

//*
//* Virtual Device Data
//*

enum {
	vdn_PTS	= 1,	// Must not be zero! 
	vdn_VMS,
		
	vdn_PM,			// Pong Master
	vdn_PS 			// Pong Slaves
};

// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary

VMSNAME("VMS", vdn_VMS,      IOP_HBC0, IOP_HBC1);
PTSNAME("PTS", vdn_PTS,      IOP_HBC0, IOP_HBC1);

#include "DdmPongMaster.h"
CHECKFIELDDEFS(PongRecord);

const fieldDef PongRecord::rgFieldDefs[] = {
	// FieldName								  Size 	  Type
//		"rowID",									8,	ROWID_FT, Persistant_PT,
		"size",										4,	U32_FT, Persistant_PT,		
		"version",									4,	U32_FT, Persistant_PT,
	
		// Set by creator
		"slaveVdn",									4,	U32_FT, Persistant_PT	// VDN of slave DDM.
};

const U32 PongRecord::cbFieldDefs = sizeof(PongRecord::rgFieldDefs);

PongRecord pddPongMaster(vdn_PS);

//*
//* VirtualEntry Ddms created at system startup.
//*
//   *** ALL VIRTUALENTRY ITEMS FOR ALL PROCESSORS ***
//	 *** ARE SPECIFIED IN THE HBC BUILDSYS         ***
//
//	 *** CLASSENTRY ITEMS NEED ONLY BE SPECIFIED   ***
//	 *** IN THE BUILDSYS ON THE PROCESSOR THEY ARE ***
//   *** TO RUN ON                                 ***
//
//   *** IF YOU DO NOT WANT FAILOVER SPECIFY SECONDARY SLOT AS SLOTNULL ***

VIRTUALENTRY("PONGMASTER",TRUE, vdn_PM,  IOP_HBC0,SLOTNULL, PongRecord, &pddPongMaster);
//VIRTUALENTRY("PONGSLAVE", TRUE, vdn_PS,  IOP_HBC0,SLOTNULL, NoPtsTable, NoPtsData);
//VIRTUALENTRY("PONGSLAVE", FALSE, vdn_PS,  IOP_RAC0,IOP_RAC1, NoPtsTable, NoPtsData);

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


