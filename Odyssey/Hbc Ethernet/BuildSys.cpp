///////////////////////////////////////////////////////////////////////
// BuildSys.cpp -- System Tables for the Gemini HBC Ethernet
//
// Copyright (C) ConvergeNet Technologies, 1999 
//
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
//  08/24/99	Joe Altmaier: Created
//
///////////////////////////////////////////////////////////////////////

#include "BuildSys.h" 

// Size of memory reserved for old-fashioned Nucleus memory allocation
extern "C" U32 gSizeReserved = 0x00080000;				// 512 KB
extern "C" U32 gSize_small_heap = 0x00200000;				// 2 MB

//
// DDM Classes
//
// Parms - "ClassName", StackSize, QueueSize, LinkName
//

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER",   10240,1024, DdmManager);
CLASSENTRY("PTSLOADER",    10240,1024,DdmPtsLoader);	// Loads Real Pts.
CLASSENTRY("VIRTUALMANAGER",10240,1024, DdmVirtualManager);
CLASSENTRY("DDMTRANSPORT", 10240,1024, Transport);
CLASSENTRY("DDMTIMER",     10240,1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO",   10240,1024, DdmSysInfo);
//CLASSENTRY("DDMCMB"		   ,10240,1024, CDdmCMB);
CLASSENTRY("DDMCMB"		   ,10240,1024, DdmCmbNull);	// Use DdmCmbNull on Eval
CLASSENTRY("NULL"		  ,10240,1024, DdmNull);
CLASSENTRY("STRESS", 10240, 1024, DdmStress);
CLASSENTRY("SGL", 10240, 1024, DdmSgl);
CLASSENTRY("TEST", 	10240, 1024, TestDdm);
CLASSENTRY("DDMPROFILE"	  ,10240,1024, DdmProfile);

// CHAOS Ddm Classes (HBC ONLY)
CLASSENTRY("PTS"		  ,10240,1024, DdmPTS);
CLASSENTRY("VMS"		  ,10240,1024, DdmVirtualMaster);
//CLASSENTRY("BOOTMAN"	  ,10240,1024, DdmBootMgr);


//
// Device Initialization
//
// Procedures called for initialization at system startup
//

DEVICEENTRY("Interrupt", 	Interrupt);
DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DDMMANAGER", 	DdmManager);
DEVICEENTRY("PIT",			TimerPIT);
DEVICEENTRY("Network",		Network);
//DEVICEENTRY("SuckBootBlockIfSlave", BootBlucker);
DEVICEENTRY("Transport_Pci",Transport_Pci);
DEVICEENTRY("Transport_Net",Transport_Net);
DEVICEENTRY("Transport", 	Transport);
DEVICEENTRY("FailSafe", 	FailSafe);
DEVICEENTRY("Stress", 		DdmStress);
DEVICEENTRY("Profile", 		Profile);


//
// System Ddms (No Virtual Device Numbers)
//

SYSTEMENTRY("DDMMANAGER");		// Must be first!
SYSTEMENTRY("PTSLOADER");		// 
SYSTEMENTRY("DDMCMB");
SYSTEMENTRY("DDMTIMER");
SYSTEMENTRY("DDMSYSINFO");
SYSTEMENTRY("VIRTUALMANAGER");
SYSTEMENTRY("DDMTRANSPORT");
//SYSTEMENTRY("DDMBOOTBLOCKD");	// Boot block server

SYSTEMENTRY("STRESS");
SYSTEMENTRY("SGL");
SYSTEMENTRY("TEST");
SYSTEMENTRY("DDMPROFILE");


//==============================================================================
// These numbers will eventually come from the persistant
// table service.
enum {
	vdn_PTS	= 1,	// Must not be zero! 
	vdn_VMS,
		
	vdn_Null
};

VIRTUALENTRY("NULL",	TRUE, vdn_Null,		IOP_HBC0,	SLOTNULL, NoPtsTable, NoPtsData);


// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary
PTSNAME("PTS", vdn_PTS, IOP_HBC0, IOP_HBC1);
VMSNAME("VMS", vdn_VMS, IOP_HBC0, IOP_HBC1);


//*
//* Boot Configuration (Temporary)
//*

struct {long divisor;}  bddTimer={10000};
struct {long updateInterval;} bddStress={1000000};

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
	256000, 256, /*1048536*/ 524287, 8192, 
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
BOOTENTRY("Stress",		&bddStress );



//==========================================================================
// Default Virtual Device Table
//
// This is the Virtual Device Table for the Test setup
// must be defined after the config data
/*************************************************************************/
// These Virtual entries will eventually come from the persistant
// table service.

struct {} pddNone[8];	// no config data placeholder

#define	NOSLOT	((TySlot)-1)	// need this for secondary devices

// None


//
// Device cleanup
//
// Procedures called for cleanup at system failure
//

FAILENTRY("Kernel",		Kernel);
FAILENTRY("Interrupt", 	Interrupt);
FAILENTRY("Dma", 		Dma);
FAILENTRY("Pci", 		HeapPci);
