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

// Size of all available memory
extern "C" U32 gSize_available_memory=0x00D00000; 	// 13MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap=0x00400000;			// 4MB
// Size of memory reserved for old-fashioned Nucleus memory allocation
extern "C" U32 gSizeReserved=0;

//*
//* DDM Classes 
//*

CLASSENTRY("DDMMANAGER",   10240,1024, DdmManager);
CLASSENTRY("PTSLOADER",	   10240,1024, DdmPtsLoader);
CLASSENTRY("VIRTUALPROXY", 10240,1024, DdmVirtualProxy);
CLASSENTRY("DDMTRANSPORT", 10240,1024, Transport);
CLASSENTRY("DDMTIMER",     10240,1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO",   10240,1024, DdmSysInfo);
CLASSENTRY("PTSPROXY",     10240,1024, DdmPtsProxy);

// Application Ddm Classes for HBC only
CLASSENTRY("STRESS", 10240, 1024, DdmStress);
//CLASSENTRY("SGL", 10240, 1024, DdmSgl);
CLASSENTRY("TEST", 	10240, 1024, TestDdm);

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
DEVICEENTRY("Transport", 	Transport);
DEVICEENTRY("FailSafe", 	FailSafe);
DEVICEENTRY("DdmStress", 	DdmStress);


//*
//* System Ddms (No Virtual Device Numbers)
//*

// CHAOS Ddm Startups
SYSTEMENTRY("DDMMANAGER" );	// Must be first!
SYSTEMENTRY("PTSLOADER");		// Must be before VirtualProxy
SYSTEMENTRY("VIRTUALPROXY");
SYSTEMENTRY("DDMTRANSPORT"  );
SYSTEMENTRY("DDMSYSINFO");
SYSTEMENTRY("DDMTIMER" );
//SYSTEMENTRY("STRESS");
//SYSTEMENTRY("SGL");
SYSTEMENTRY("TEST");

//*
//* Virtual Device Data
//*

enum {
	vdn_PTS	= 1,	// Must not be zero! 
	
	vdn_STRESS,
	vdn_STRESS1
};

// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary

PTSNAME("PTSPROXY",	vdn_PTS, IOP_HBC0, IOP_HBC1);


// VirtualEntry config data

struct {} pddNone;

//*
//* Virtual Device Ddms created at system startup.
//*

VIRTUALENTRY("STRESS", 	TRUE, vdn_STRESS, 	&pddNone,	IOP_HBC0,SLOTNULL);
//VIRTUALENTRY("STRESS", 	TRUE, vdn_STRESS1, 	&pddNone,	IOP_NIC1,SLOTNULL);


//*
//* Boot Configuration (Temporary)
//*
// ...Needed for the Eval System

struct {U32 iCabinet; TySlot iSlot;} bddDdmManager = {
	0, IOP_HBC0
};

// This structure defines the pci address space for the messaging.
// cbMsg		total memory for messages
// sMsg			size of one message in bytes
// cbBuf		total memory for data buffers
// sBuf			size of one data buffer
// pciWindowLo	beginning of pci window in our physical address space (multiple of MB)
// pciWindowHi	end of pci window in our physical address space (multiple of MB)
// aPciWindow	address of each slots' pci window
// aSWindow		size of each slots' pci window

// This example for eval board defines a pci window at physical address 0x02000000
//  which is 40MB long.
//  HBC0 pci window appears at address 0x02000000 and is 16MB
//  HBC1 pci window appears at address 0x03000000 and is 16MB
struct {U32 cbMsg; U32 sMsg; U32 cbBuf; U32 sBuf; U32 pciWindowLo; U32 pciWindowHi; U32 aPciWindow[NSLOT]; U32 aSWindow[NSLOT];} bddMessenger = {
	128000, 256, 256000, 8192, 
	0x02000000, 0x04000000,
	
	// HBC0		HBC1	
	0x02000000,	0x03000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
	// hubs
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
	// SSDU0	SSDU1		SSDU2		SSDU3		SSDL0		SSDL1		SSDL2		SSDL3	
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
	// RAC0		APP0		APP1		NIC0		RAC1		APP2		APP3		NIC1
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 

	// HBC0		HBC1	
	0x01000000,	0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
	// hubs
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
	// SSDU0	SSDU1		SSDU2		SSDU3		SSDL0		SSDL1		SSDL2		SSDL3	
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
	// RAC0		APP0		APP1		NIC0		RAC1		APP2		APP3		NIC1
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
};
	
struct {U32 divisor;}  bddTimer={10000};
struct {U32 interval;}  bddStress={1000000};

BOOTENTRY("DdmManager",	&bddDdmManager );
BOOTENTRY("Transport",	&bddMessenger );
BOOTENTRY("Timer",		&bddTimer );
BOOTENTRY("Stress",		&bddStress );
