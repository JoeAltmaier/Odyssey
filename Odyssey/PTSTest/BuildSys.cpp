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
 *  11/14/98	Tom Nelson: Created
 *	02/01/99	JFL	Adapted for PTSTest.
 *
**/


#include "BuildSys.h"

// Size of all available memory
extern "C" U32 gSize_available_memory=0x01000000;
// Size of all available memory
extern "C" U32 gSize_total_memory=0x01000000; 	// 16MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap=0x00100000;		// 1MB
// Size of memory reserved for old-fashioned Nucleus memory allocation
extern "C" U32 gSizeReserved = 0x0;				// 0MB

///////////////////////////////////////
//*
//* DDM Classes
//*
////////////////////////////////////////

// Parms - "ClassName", StackSize, QueueSize, LinkName

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER",	10240, 1024, DdmManager);
CLASSENTRY("DDMTRANSPORT",	10240, 1024, Transport);
CLASSENTRY("DDMTIMER",		10240, 1024, DdmTimer);

CLASSENTRY("PTSLOADER", 	10240, 1024, DdmPtsLoader);
CLASSENTRY("VIRTUALMANAGER",10240, 1024, DdmVirtualManager);
//CLASSENTRY("DDMSYSINFO",	10240, 1024, DdmSysInfo);

//  DdmCmbNull establishes a "valid" IOP Status table, based on entries
//  in buildsys, which permit virtual manager to run.
//CLASSENTRY("HDM_CMB",		10240, 1024, CDdmCMB);      // real CMB DDM class
CLASSENTRY("HDM_CMBNULL",	10240, 1024, DdmCmbNull);   // dummy CMB DDM class

// CHAOS Ddm Classes (HBC ONLY)
CLASSENTRY("PTS",			10240, 1024, DdmPTS);			// The real PTS
CLASSENTRY("VMS",			10240, 1024, DdmVirtualMaster);

//  enable boot man only when running on Odyssey!
//CLASSENTRY("BOOTMAN"      ,10240,1024, DdmBootMgr);

// DDMs that run here
CLASSENTRY("HDM_TST",		10240, 1024, DdmPtsTest);


////////////////////////////////
//
// Device Table
//
/////////////////////////////////

// These functions are each called once at system startup
// before the DDMs are enabled.

DEVICEENTRY("Interrupt",	Interrupt);
DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DDMMANAGER",	DdmManager);
DEVICEENTRY("PIT",			TimerPIT);
DEVICEENTRY("Transport_Pci",	Transport_Pci);
DEVICEENTRY("Transport",	Transport);
//DEVICEENTRY("FailSafe", 	FailSafe);


//===========================================================================
// System DDMs Table
//
SYSTEMENTRY("DDMMANAGER");		// Must be first
SYSTEMENTRY("PTSLOADER");		//
SYSTEMENTRY("HDM_CMBNULL");
//SYSTEMENTRY("DDMSYSINFO");
SYSTEMENTRY("DDMTIMER");
SYSTEMENTRY("DDMTRANSPORT");
//  the CMB DDM must be "system start," since it helps facilitate boot
//  (e.g., virtual manager needs its tables)
//SYSTEMENTRY("HDM_CMB");
SYSTEMENTRY("VIRTUALMANAGER");

//  only use bootman on Odyssey hardware
//SYSTEMENTRY("BOOTMAN");


//////////////////////////////////////
//
// Virtual Device Numbers
//
//////////////////////////////////////

// use this when building seperate images
// These numbers will eventually come from the persistant
// table service.
#include "SystemVdn.h"


////////////////////////////////////////////
//
// PTS Proxy and PTS
//
////////////////////////////////////////////

PTSNAME("PTS", vdnPTS, IOP_HBC0, IOP_HBC1);
VMSNAME("VMS", vdnVMS, IOP_HBC0, IOP_HBC1);

// Temporary PTS data

//struct {} pddNone;

////////////////////////////
//
// Virtual Device Ddms
//
////////////////////////////

VIRTUALENTRY("HDM_TST", TRUE, vdnLAST, IOP_HBC0, SLOTNULL, NoPtsTable, NoPtsData);

//////////////////////////////////////
//
// Boot Data
//
////////////////////////////////////////
//
// These values will eventually come from the Boot ROM
// 
//  Boot Configuration (Temporary)
//


// This structure defines the pci address space for the messaging.
// cbMsg                total memory for messages
// sMsg                 size of one message in bytes
// cbBuf                total memory for data buffers
// sBuf                 size of one data buffer
// pciWindowLo  		beginning of pci window in our physical address space (multiple of MB)
// pciWindowHi  		end of pci window in our physical address space (multiple of MB)
// aPciWindow   		address of each slots' pci window
// aSWindow             size of each slots' pci window

// This example for eval board defines a pci window at physical address 0x02000000
//  which is 40MB long.
//  HBC0 pci window appears at address 0x02000000 and is 16MB
//  HBC1 pci window appears at address 0x03000000 and is 16MB
struct {
	U32 cbMsg;
	U32 sMsg;
	U32 cbBuf;
	U32 sBuf;
	U32 pciWindowLo;
	U32 pciWindowHi;
	U32 aPciWindow[NSLOT];
	U32 aSWindow[NSLOT];
} bddMessenger = {
        256000, 256, 1048536, 8192, 0x02000000, 0x04000000,

        // HBC0         HBC1
        0x02000000,     0x03000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        // hubs
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        // SSDU0        SSDU1      SSDU2       SDU3         SSDL0       SSDL1	   SSDL2       SSDL3
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        // RAC0         APP0       APP1        NIC0         RAC1        APP2	   APP3        NIC1
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,

        // HBC0         HBC1
        0x01000000,     0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        // hubs
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        // SSDU0        SSDU1      SSDU2       SSDU3        SSDL0       SSDL1	   SSDL2       SSDL3
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        // RAC0         APP0       APP1        NIC0         RAC1        APP2	   APP3        NIC1
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

struct {U32 divisor;}  bddTimer = {10000};

struct {int iCabinet; TySlot iSlot;} bddDdmManager = {
	0, IOP_HBC0};

BOOTENTRY ("DdmManager", &bddDdmManager);
BOOTENTRY ("Transport",  &bddMessenger );
BOOTENTRY ("Timer",      &bddTimer );
