/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: BuildSys.cpp
//
// Description:
// This file contains system tables to initialize the application.
//
//
// Update Log:
// 7/8/99 Jim Frandeen: Create file from Tom Nelson's BuildSys.
/*************************************************************************/


#define _TRACEF
#include "Trace_Index.h"

#include "BuildSys.h" 

// Size of all available memory for SSD boards
extern "C" U32 gSize_available_memory = 0x0C800000;

// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap = 0x02000000;

// Size of memory reserved for old-fashioned Nucleus memory allocation
extern "C" U32 gSizeReserved=0;

//
// DDM Classes
//
// Parms - "ClassName", StackSize, QueueSize, LinkName
//

#define DEF_STK_SIZE	32768

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER",   DEF_STK_SIZE,1024, DdmManager);
CLASSENTRY("PTSLOADER",    DEF_STK_SIZE,1024,DdmPtsLoader);	// Loads Real Pts.
CLASSENTRY("VIRTUALMANAGER", DEF_STK_SIZE,1024, DdmVirtualManager);
CLASSENTRY("VMS",			DEF_STK_SIZE,1024, DdmVirtualMaster);
CLASSENTRY("DDMTRANSPORT",	DEF_STK_SIZE,1024, Transport);
CLASSENTRY("DDMTIMER",		DEF_STK_SIZE,1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO",	DEF_STK_SIZE,1024, DdmSysInfo);

// Customize the SSD image here:
//CLASSENTRY("DDM_LED", DEF_STK_SIZE, 1024, DdmLED);
//CLASSENTRY("DDM_IOPLOG",  DEF_STK_SIZE,1024,DdmEventLogIOP);

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
DEVICEENTRY("Transport_Pci",Transport_Pci);
DEVICEENTRY("Transport", 	Transport);
DEVICEENTRY("FailSafe", 	FailSafe);
DEVICEENTRY("Initialize_SSD",        Initialize_SSD);

//*
//* System Ddms (No Virtual Device Numbers)
//*

// CHAOS Ddm Startups
SYSTEMENTRY("DDMMANAGER");	// Must be first!
SYSTEMENTRY("PTSLOADER");		// 
//SYSTEMENTRY("CDdmCMB");
SYSTEMENTRY("DDMTIMER");
//SYSTEMENTRY("DDMSYSINFO");
SYSTEMENTRY("VIRTUALMANAGER");	// 
SYSTEMENTRY("DDMTRANSPORT");
//SYSTEMENTRY("DDM_LED");
//SYSTEMENTRY("DDM_IOPLOG");


// CHAOS Startups (HBC ONLY)

//==============================================================================
// SystemVdn.h - Virtual Device Numbers
//
// These numbers will eventually come from the persistant
// table service.
#include "SystemVdn.h"


// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary

PTSNAME("HDM_PTS",      vdnPTS,  IOP_HBC0, IOP_HBC1);
VMSNAME("VMS",			vdnVMS,	 IOP_HBC0, IOP_HBC1);

//*
//* Boot Configuration (Temporary)
//*

struct {long divisor;}  bddTimer={10000};

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
	262144, 512, 1048576, 8192, 
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
BOOTENTRY("Transport",	&bddMessenger );
BOOTENTRY("Timer",		&bddTimer );

