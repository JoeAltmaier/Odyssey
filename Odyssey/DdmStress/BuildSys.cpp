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
extern "C" U32 gSize_available_memory=0x00400000; 	// 4MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap=0x00080000;			// .5MB

//*
//* DDM Classes
//*

// 1st four entries are always there
CLASSENTRY("HDM_DMGR", 10240, 1024, DdmManager);
//CLASSENTRY("HDM_TPT",  10240, 1024, Transport);
CLASSENTRY("HDM_TIMR", 10240, 1024, DdmTimer);
CLASSENTRY("HDM_PTS",  10240, 1024, PersistentData);
// Customize your test image here
CLASSENTRY("HDM_STRESS", 10240, 1024, DdmStress);
CLASSENTRY("HDM_TEST", 	10240, 1024, TestDdm);

//*
//* Device Initialization
//*
//* Procedures called for initialization at system startup
//*

//DEVICEENTRY("Interrupt", 	Interrupt);
//DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DdmManager", 	DdmManager);
//DEVICEENTRY("Transport", 	Transport);
DEVICEENTRY("FailSafe", 	FailSafe);
DEVICEENTRY("DdmStress", 	DdmStress);


//*
//* System Ddms (No Virtual Device Numbers)
//*

SYSTEMENTRY("HDM_DMGR" );	// Must be first!
//SYSTEMENTRY("HDM_TPT"  );
SYSTEMENTRY("HDM_TIMR" );
SYSTEMENTRY("HDM_STRESS");
SYSTEMENTRY("HDM_TEST");

//*
//* Virtual Device Data
//*

enum {
	vdn_PTS	= 1	// Must be one (1)
};

// Temporary PTS data

struct {} pddNone;

//*
//* Virtual Device Ddms created at system startup.
//*
VIRTUALENTRY("HDM_PTS",  	TRUE, vdn_PTS,  	&pddNone,	IOP_HBC0,IOP_HBC1);
// Temporary entries until real PTS is operational


//*
//* Boot Configuration (Temporary)
//*

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
//BOOTENTRY("Transport",	&bddMessenger );
BOOTENTRY("Timer",		&bddTimer );
BOOTENTRY("Stress",		&bddStress );
