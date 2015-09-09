///////////////////////////////////////////////////////////////////////
// BuildSys.cpp -- System Tables
//
// Copyright (C) ConvergeNet Technologies, 1998 
//
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Revision History:
//
// $Log: /Gemini/Odyssey/Hbc/BuildSys._Odyssey.cpp $
// 
// 22    9/01/99 8:05p Iowa
// 
// 21    8/25/99 11:41a Jaltmaier
// gSizeReserved
// 
// 20    7/24/99 8:02p Jlane
// NAC changes for a single NAC test
// 
// 19    7/22/99 5:43p Jlane
// Updated export rec initializors to conform to new Export Rec def'n.
// 
// 18    7/20/99 9:15p Jlane
// James Taylor's Raid config changes
// + Export table capacity changes to match
// 
// 17    7/19/99 9:08p Jlane
// Configure the FC Loop Instance in the Export table
// -- wrong slot the first time, need NIC slot
// 
// 16    7/19/99 8:49p Jlane
// Configure the FC Loop Instance in the Export table
// 
// 15    7/15/99 11:55p Mpanas
// Changes to support Multiple FC Instances,
// Move SystemVdn.cpp into here, remove
// SystemVdn.cpp
// 
// 14    7/09/99 5:38p Jlane
// Assorted changes for new CHAOS.
// 
// 13    7/06/99 2:22p Mpanas
// Changes to support SSD in build
// 
// 12    7/02/99 7:13p Mpanas
// Change over to the new Export Table
// 
// 11    7/02/99 3:41p Mpanas
// Changes for new BuildSys model,
// add new DdmPtsDefault library to
// initialize some PTS data
// 
// 10    6/30/99 2:44p Ewedel
// Split PTSNAME() line into PTSNAME() and PTSPROXY(), so that real PTS is
// made system-start (for CMB's use).
// 
// 9     6/29/99 5:59p Mpanas
// Update to match current config setup
// 
// 8     6/29/99 4:12p Jlane
// Remove MYSLOT macro.  Make DdmBootMgr and CMB Ddm SYSTEM Entries.
// 
// 7     6/28/99 9:15p Jlane
// Updated for new CHAOS Single VDT Technology.
//
//  11/14/98	Tom Nelson: Created
//	03/01/99	JFL	Adapted for HBCImage.
//
///////////////////////////////////////////////////////////////////////

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include "Address.h"
#include "OsTypes.h"
#include "BuildSys.h" 
#include "Odyssey.h"
#include "FC_Loop.h"

// Include all field defs here
#include "ExportTable.cpp"
#include "DiskDescriptor.cpp"
#include "StorageRollCallTable.cpp"

// Tables referenced
#include "ExportTable.h"
#include "DiskDescriptor.h"
#include "StorageRollCallTable.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"


#define	NAC_TEST

// Size of all available memory:
extern "C" U32 gSize_available_memory=0x00700000;

// Size of the small heap:
extern "C" U32 gSize_small_heap = 0x00200000;

// Size of memory reserved for old-fashioned Nucleus memory allocation
extern "C" U32 gSizeReserved=0;

//
// DDM Classes
//
// Parms - "ClassName", StackSize, QueueSize, LinkName
//

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER",   10240,1024, DdmManager);
CLASSENTRY("PTSPROXYLOADER",10240,1024,DdmPtsProxyLoader);	// Loads Pts Proxy.
CLASSENTRY("PTSLOADER",    10240,1024,DdmPtsLoader);	// Loads Real Pts.
CLASSENTRY("VIRTUALPROXY", 10240,1024, DdmVirtualProxy);
CLASSENTRY("DDMTRANSPORT", 10240,1024, Transport);
CLASSENTRY("DDMTIMER",     10240,1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO",   10240,1024, DdmSysInfo);
CLASSENTRY("PTSDEFAULT",  10240, 1024, DdmPtsDefault );	// Populates Mikes Tables?

// CHAOS Ddm Classes (HBC Only)
CLASSENTRY("PTSPROXY",     10240,1024, DdmPtsProxy);
//CLASSENTRY("FAILOVERPROXY",     10240,1024, DdmFailoverProxy);
CLASSENTRY("HDM_TS", 10240, 1024, DdmPTS);			// The real PTS

// Customize your image here:
CLASSENTRY("DDM_RedundancyMgr", 10240, 1024, DdmRedundancyMgr);
CLASSENTRY("DDM_Registrar", 10240, 1024, DdmRegistrar);
CLASSENTRY("CDdmCMB", 10240, 1024, CDdmCMB);
CLASSENTRY("DDM_BootMgr", 10240, 1024, DdmBootMgr);


//
// Device Initialization
//
// Procedures called for initialization at system startup
//

DEVICEENTRY("Interrupt", 	Interrupt);
DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DdmManager", 	DdmManager);
DEVICEENTRY("PIT",			TimerPIT);
DEVICEENTRY("Transport", 	Transport);
//DEVICEENTRY("FailSafe", 	FailSafe);


//
// System Ddms (No Virtual Device Numbers)
//

SYSTEMENTRY("DDMMANAGER");		// Must be first!
SYSTEMENTRY("DDMTRANSPORT");
SYSTEMENTRY("PTSPROXYLOADER");	// Must be before VIRTUALPROXY
SYSTEMENTRY("PTSLOADER");		// 
SYSTEMENTRY("VIRTUALPROXY");	// Must come after DDM MAnager!
SYSTEMENTRY("PTSDEFAULT");		// 
SYSTEMENTRY("DDMTIMER");
SYSTEMENTRY("DDMSYSINFO");
//SYSTEMENTRY("CDdmCMB");
//SYSTEMENTRY("DDM_BootMgr");


//==============================================================================
// SystemVdn.h - Virtual Device Numbers
//
// These numbers will eventually come from the persistant
// table service.
#include "SystemVdn.h"


// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary

PTSPROXY("PTSPROXY",   vdnPTS, IOP_HBC0, SLOTNULL);

PTSNAME("HDM_TS",      vdnTS,  IOP_HBC0, SLOTNULL);


//*
//* Boot Configuration (Temporary)
//*

struct {long divisor;}  bddTimer={10000};

#if 0
struct {long cbMsg; long sMsg; long cbBuf; long sBuf; long pciWindow; long sWindow; } bddMessenger = {
	256000, 256, /*1048536*/ 524287, 8192, 0x2000000, 0x1000000
};
#else
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
#endif
	
//BOOTENTRY("DdmManager",	&bddDdmManager );  // only used on Eval systems
BOOTENTRY("Transport",	&bddMessenger );
BOOTENTRY("Timer",		&bddTimer );



//////////////////////////////////////////////////////////////////////
// NIC CONFIG DATA
//////////////////////////////////////////////////////////////////////

//=========================================================================
// This section declares the tables for the dummy PersistentData service
// 

#include "PersistentData.h"
#include "FcpConfig.h"
#include "FcpIsp.h"
#include "FcpData.h"


FCP_CONFIG	TargetParms[2] = {
	{FCP_CONFIG_VERSION,
	sizeof(FCP_CONFIG),
	0,		// initiator mode
	1,		// target mode
	0,		// fairness
	1,		// fast posting
	1,		// hard addesses
	IFCB_FULL_DUPLEX,		// options
	128,	// event queue size
	25,		// mb queue size
	8192,	// task stack size
	100,	// priority
	(MAXIMUM_FRAME_SIZE*2),
	128,	// request FIFO size
	128,	// response FIFO size
	2,		// hard address
	{' ', 0, 0, 0xe0, 0x8b, 0, 0, 1},		// node name
	16384,	// high level interrupt stack size
	8,		// number of Target IDs (max 31)
	8,		// number of LUNs
	40,		// number of PCI window buffers
	TRGT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NIC_SLOT, 0),	// calculate the loop instance num
	vdnLM1},		// virtual device to throw events to
	
	{FCP_CONFIG_VERSION,
	sizeof(FCP_CONFIG),
	0,		// initiator mode
	1,		// target mode
	0,		// fairness
	1,		// fast posting
	1,		// hard addesses
	IFCB_FULL_DUPLEX,		// options
	128,	// event queue size
	25,		// mb queue size
	8192,	// task stack size
	100,	// priority
	(MAXIMUM_FRAME_SIZE*2),
	128,	// request FIFO size
	128,	// response FIFO size
	2,		// hard address
	{' ', 0, 0, 0xe0, 0x8b, 0, 0, 1},		// node name
	16384,	// high level interrupt stack size
	8,		// number of Target IDs (max 31)
	8,		// number of LUNs
	40,		// number of PCI window buffers
	TRGT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT, 0),	// calculate the loop instance num
	vdnLM2}		// virtual device to throw events to
	
};

struct _EchoScsiParms {
	U32	Parms[20];
	VDN	vdnNext; 
} EchoScsiIsmParms = {
	// use this table to translate the LUN coming in to a
	// Target Id going out
	{
	0, 1, 2, 3,
	4, 5, 6, 7,
	8, 9, 10, 11,
	12, 13, 14, 15,
	16, 17, 18, 19},
	vdnInit
	};

#include "StsConfig.h"

STS_CONFIG	ScsiServerParms[] = 
	{ 
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRAID0 },
#if !defined(NAC_TEST)
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRAID1 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRAID2 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRAID3 },
#if defined(RAMDISK)
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD0 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD1 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD2 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD3 },
#else
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA4 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA5 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA6 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA7 },
#endif

#else
// NAC
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA20 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA21 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA22 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD0 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD1 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD2 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD3 },
#endif
};

#define		RAM_DISK_SIZE	(((1024*1024) / 512)*10)	// 10Mb in blocks

// RAM Disk config data
U32		RamDiskParms[] =
			 {
			 RAM_DISK_SIZE,			// RAM DISK 0
			 RAM_DISK_SIZE,			// RAM DISK 1
			 RAM_DISK_SIZE,			// RAM DISK 2
			 RAM_DISK_SIZE,			// RAM DISK 3
			 };

//////////////////////////////////////////////////////////////////////
// END OF NIC CONFIG DATA
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Loop Monitor Parms
// Used on NIC, RAC and NAC
//////////////////////////////////////////////////////////////////////
#include "LmConfig.h"

LM_CONFIG	LoopMonParms[3] =
{ 
	// LM0
	{LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
	1,							// number of loops to scan
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	FCLOOPINSTANCE(RAC_SLOT, 0), 0, 0 },	// loop instance num of RAC
    },
    
    // LM1
	{LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
	1,							// number of loops to scan
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	FCLOOPINSTANCE(NIC_SLOT, 0), 0 ,0 }	// loop instance num of NIC
	},

    // LM2
	{LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
	2,							// number of loops to scan
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	FCLOOPINSTANCE(NAC_SLOT, 0),// loop instance num of chip 0
	//FCLOOPINSTANCE(NAC_SLOT, 1),// loop instance num of chip 1
	FCLOOPINSTANCE(NAC_SLOT, 2)}	// loop instance num of chip 2
	}
};
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// RAC CONFIG DATA
//////////////////////////////////////////////////////////////////////


//=========================================================================
// This section declares the tables for the dummy PersistentData service
// 

#include "PersistentData.h"
#include "FcpConfig.h"
#include "FcpIsp.h"
#include "FcpData.h"


FCP_CONFIG	InitParms[4] = {
	{FCP_CONFIG_VERSION,
	sizeof(FCP_CONFIG),
	1,		// initiator mode
	0,		// target mode
	0,		// fairness
	1,		// fast posting
	1,		// hard addesses
	IFCB_FULL_DUPLEX,		// options
	128,	// event queue size
	25,		// mb queue size
	8192,	// task stack size
	100,	// priority
	MAXIMUM_FRAME_SIZE,
	128,	// request FIFO size
	128,	// response FIFO size
	0x40,	// hard address
	{' ', 0, 0, 0xe0, 0x8b, 0, 0, 1},		// node name
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(RAC_SLOT, 0),	// calculate the loop instance num
	vdnLM},		// virtual device to throw events to

	{FCP_CONFIG_VERSION,
	sizeof(FCP_CONFIG),
	1,		// initiator mode
	0,		// target mode
	0,		// fairness
	1,		// fast posting
	1,		// hard addesses
	IFCB_FULL_DUPLEX,		// options
	128,	// event queue size
	25,		// mb queue size
	8192,	// task stack size
	100,	// priority
	MAXIMUM_FRAME_SIZE,
	128,	// request FIFO size
	128,	// response FIFO size
	0x40,	// hard address
	{' ', 0, 0, 0xe0, 0x8b, 0, 0, 1},		// node name
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT, 0),	// calculate the loop instance num
	vdnLM2},		// virtual device to throw events to

	{FCP_CONFIG_VERSION,
	sizeof(FCP_CONFIG),
	1,		// initiator mode
	0,		// target mode
	0,		// fairness
	1,		// fast posting
	1,		// hard addesses
	IFCB_FULL_DUPLEX,		// options
	128,	// event queue size
	25,		// mb queue size
	8192,	// task stack size
	100,	// priority
	MAXIMUM_FRAME_SIZE,
	128,	// request FIFO size
	128,	// response FIFO size
	0x40,	// hard address
	{' ', 0, 0, 0xe0, 0x8b, 0, 0, 1},		// node name
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT, 1),	// calculate the loop instance num
	vdnLM2},		// virtual device to throw events to

	{FCP_CONFIG_VERSION,
	sizeof(FCP_CONFIG),
	1,		// initiator mode
	0,		// target mode
	0,		// fairness
	1,		// fast posting
	1,		// hard addesses
	IFCB_FULL_DUPLEX,		// options
	128,	// event queue size
	25,		// mb queue size
	8192,	// task stack size
	100,	// priority
	MAXIMUM_FRAME_SIZE,
	128,	// request FIFO size
	128,	// response FIFO size
	0x40,	// hard address
	{' ', 0, 0, 0xe0, 0x8b, 0, 0, 1},		// node name
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT, 2),	// calculate the loop instance num
	vdnLM2},		// virtual device to throw events to
};

#include "DmConfig.h"
#define	DDH
DM_CONFIG	DriveMonParms[4] = { 
	{DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	20,							// number of drives to scan
	0,							// DM Flags
	vdnInit,
	FCLOOPINSTANCE(RAC_SLOT, 0),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
#ifdef DDH
	 0, 1, 2, 3, 4,				// drive ID translation table
	 8, 9, 10, 11, 12,			// for use with Disk Drive Hubs (4)
	 16, 17, 18, 19, 20,
	 24, 25, 26, 27, 28,
#else
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for test systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
#endif
	 0  }},						 // rest are all zeros	for now

	{DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	20,							// number of drives to scan
	0,							// DM Flags
	vdnInit1,
	FCLOOPINSTANCE(NAC_SLOT, 0),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
#ifdef DDH
	 0, 1, 2, 3, 4,				// drive ID translation table
	 8, 9, 10, 11, 12,			// for use with Disk Drive Hubs (4)
	 16, 17, 18, 19, 20,
	 24, 25, 26, 27, 28,
#else
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for test systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
#endif
	 0  }},						 // rest are all zeros	for now

	{DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	20,							// number of drives to scan
	0,							// DM Flags
	vdnInit2,
	FCLOOPINSTANCE(NAC_SLOT, 1),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
#ifdef DDH
	 0, 1, 2, 3, 4,				// drive ID translation table
	 8, 9, 10, 11, 12,			// for use with Disk Drive Hubs (4)
	 16, 17, 18, 19, 20,
	 24, 25, 26, 27, 28,
#else
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for test systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
#endif
	 0  }},						 // rest are all zeros	for now

	{DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	20,							// number of drives to scan
	0,							// DM Flags
	vdnInit3,
	FCLOOPINSTANCE(NAC_SLOT, 2),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
#ifdef DDH
	 0, 1, 2, 3, 4,				// drive ID translation table
	 8, 9, 10, 11, 12,			// for use with Disk Drive Hubs (4)
	 16, 17, 18, 19, 20,
	 24, 25, 26, 27, 28,
#else
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for test systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
#endif
	 0  }}						 // rest are all zeros	for now
	};


#include "BsaConfig.h"

BSA_CONFIG BsaParms[] = {
// 	  Version               size               LUN ID  Init ID SMART
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  0, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  1, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  2, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  3, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  4, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  8, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  9, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  10, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  11, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  12, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  16, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  17, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  18, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  19, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  20, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  24, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  25, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  26, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  27, vdnInit, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  28, vdnInit, 0	},
	// NAC
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  0, vdnInit3, 0	},	// 20
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  1, vdnInit3, 0	},	// 21
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  2, vdnInit3, 0	}	// 22
};

// RAID Config Data

#include "ArrayDescriptor.h"
#include "RAIDMemberTable.h"


typedef struct
{
	VDN						memberVD;		// VD of member -- 
	RAID_MEMBER_STATUS		memberHealth;
	U32						memberIndex;	// relative to this array
	U32						endLBA;			// where host data ends
}	TMP_RAID_ARRAY_MEMBER;

typedef struct
{
	VDN					arrayVDN;
	U32					totalCapacity;		// in 512 byte blks - depends on RAID Level
	U32					memberCapacity;		// in 512 byte blks - smallest member cap.
	U32					dataBlockSize;		// in 512 byte blks (default = 64)
	U32					parityBlockSize;	// in 512 byte blks	(default = DataBlockSize)
											// if parity blk size is different from data blk
											// size, parity size must be a multiple of the 
											// data blk size (matters in RAIDS 3,4,5)
	RAID_LEVEL			raidLevel;			// rsmRAID0, rsmMIRRORED, rsmRAID5
	RAID_ARRAY_STATUS	health;				// OFFLINE,
	RAID_INIT_STATUS	initStatus;			// Initialization Done,
	U32					numberMembers;
	U32					numberUtilities;	// currently running on array
	rowID				utilities[1]; // info for each running util
	rowID		 		members[8]; 
}	TMP_RAID_ARRAY_DESCRIPTOR;

struct TempConfiguration
{
	TMP_RAID_ARRAY_DESCRIPTOR	AD;
	TMP_RAID_ARRAY_MEMBER		MD[32];
};

TempConfiguration RAIDParms[] = {
{
	{
		vdnRAID0,				// arrayVDN
		0x087acc00,				// totalCapacity
		0x021eb300,				// memberCapacity
		128,					// dataBlockSize
		128,					// parityBlockSize
		RAID0,					// RaidLevel;
		RAID_FAULT_TOLERANT,	// Health;
		RAID_INIT_COMPLETE,		// InitStatus
		4,						// NumberMembers;
		0,						// NumberUtilities;

	},
	{				
		{
			vdnBSA0,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			0,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},
		{
			vdnBSA1,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			1,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},
		{
			vdnBSA2,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			2,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},		
		{
			vdnBSA3,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			3,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},		
	},
},	
#if 0
{
	{
		vdnRAID1,				// arrayVDN
		0x087acc00,				// totalCapacity
		0x021eb300,				// memberCapacity
		128,					// dataBlockSize
		128,					// parityBlockSize
		RAID0,					// RaidLevel;
		RAID_FAULT_TOLERANT,	// Health;
		RAID_INIT_COMPLETE,		// InitStatus
		4,						// NumberMembers;
		0,						// NumberUtilities;

	},
	{				
		{
			vdnBSA4,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			0,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},
		{
			vdnBSA5,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			1,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},
		{
			vdnBSA6,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			2,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},	
		{
			vdnBSA7,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			3,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},			
	},
},	
{
	{
		vdnRAID2,				// arrayVDN
		0x087acc00,				// totalCapacity
		0x021eb300,				// memberCapacity
		128,					// dataBlockSize
		128,					// parityBlockSize
		RAID0,					// RaidLevel;
		RAID_FAULT_TOLERANT,	// Health;
		RAID_INIT_COMPLETE,		// InitStatus
		4,						// NumberMembers;
		0,						// NumberUtilities;

	},
	{				
		{
			vdnBSA8,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			0,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},
		{
			vdnBSA9,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			1,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},
		{
			vdnBSA10,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			2,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},		
		{
			vdnBSA11,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			3,					// MemberIndex;
			0x021eb300,			// EndLBA;
		},		
	},
}	

#endif
};	

//////////////////////////////////////////////////////////////////////
// END OF RAC CONFIG DATA
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// SSD Config data
//////////////////////////////////////////////////////////////////////

//===========================================================================
//
#if false
#include "SsdContext.h"

SSD_Context	SSDParms[] = 
	{ 
		{
	// CM_CONFIG(
				CM_CONFIG_VERSION,		// version
				0,						// size need not be set
				SSD_PAGE_SIZE,			// page_size
				SSD_NUM_VIRTUAL_PAGES,	// page_table_size;
				0,						// hash_table_size;
				64,						// num_reserve_pages;
				60,						// dirty_page_writeback_threshold;
				95,						// dirty_page_error_threshold;
				0,						// num_prefetch_forward;
				0,						// num_prefetch_backward;
	// ) CM_CONFIG
	
	// FF_CONFIG(
				FF_CONFIG_VERSION,		// version
				0,						// size need not be set
				// memory_size
				MEMORY_FOR_SSD,
				MEMORY_FOR_CALLBACKS,	
				0,						// verify_write_level
				1,						// verify_erase_level
				5,						// percentage_erased_pages;
				5,						// percentage_replacement_pages;
				25,						// replacement_page_threshold;
				0,						// erase_all_pages;
				1,						// verify_format_level

	// ) FF_FORMAT_CONFIG
	},
									
	{ 0},
	{ 0},
	{ 0},
	{ 0},
	{ 0}
};
#endif

//////////////////////////////////////////////////////////////////////
// DEFAULT EXPORT TABLE
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// DEFAULT EXPORT TABLE
//////////////////////////////////////////////////////////////////////

//===========================================================================
/*

typedef struct ExportTableEntry {
	rowID			rid;				// rowID of this table row.
	U32 			version;			// Version of Export Table record.
	U32				size;				// Size of Export Table record in bytes.
	rowID			ridVCMCommand;		// For VCM use ONLY.  rowID of VCM Command that created this circuit.
	CTProtocolType	ProtocolType;		// FCP, IP, other
	VDN				vdNext;				// First Virtual Device number in the chain
	VDN				vdLegacyBsa;		// Virtual Device number of the legacy BSA
	VDN				vdLegacyScsi;		// Virtual Device number of the legacy SCSI
	U32				ExportedLUN;		// LUN number exported
	U32				InitiatorId;		// Host ID
	U32				TargetId;			// our ID
	U32				FCInstance;			// FC Loop number
	String32		SerialNumber;		// Use a string array for Serial Number
	I64				Capacity;			// Capacity of this Virtual Circuit
	U32				FailState;
	CTReadyState	ReadyState;			// Current state
	CTReadyState	DesiredReadyState;	// Desired Ready state
	String16		WWNName;			// World Wide Name (64 or 128-bit IEEE registered)
	rowID			ridUserInfo;		// rowID of user info for this VC
	rowID			ridEIP;				// rowID of External Initiator Port in FC Port DB. 
	rowID			ridAltExportRec;	// rowID of our Alternate Path's Export Record
	} ExportTableEntry, ExportTableRecord;

*/

// numExportRows and Export[] are used by DDMPTSDEFAULT to load
// default data into the PTS
#if defined(NAC_TEST)
U32		numExportRows = 4;
#else
U32		numExportRows = 8;
#endif

// helpers
#define	LP0		(FCLOOPINSTANCE(NIC_SLOT, 0))
#define	LP1		(FCLOOPINSTANCE(NAC_SLOT, 0))
//#define	LP2		(FCLOOPINSTANCE(NAC_SLOT, 1))
#define	LP3		(FCLOOPINSTANCE(NAC_SLOT, 2))

ExportTableEntry			Export[8] = 	// one for each VirtualCircuit
{
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS0, 0, 0, 0, -1, 0, LP0, "00", 0x087acc00,    0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
#if defined(NAC_TEST)
	// 3 regular disks, four Ram Disk for the NAC_TEST
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS1, 0, 0, 0, -1, 0, LP1, "01", 0x010f59c7,    0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS2, 0, 0, 1, -1, 0, LP1, "02", 0x010f59c7,    0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS3, 0, 0, 2, -1, 0, LP1, "03", 0x010f59c7,    0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS4, 0, 0, 4, -1, 0, LP1, "04", RAM_DISK_SIZE, 0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS5, 0, 0, 5, -1, 0, LP1, "05", RAM_DISK_SIZE, 0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS6, 0, 0, 6, -1, 0, LP1, "06", RAM_DISK_SIZE, 0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS7, 0, 0, 7, -1, 0, LP1, "07", RAM_DISK_SIZE, 0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
#else
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS1, 0, 0, 1, -1, 0, LP0, "01", 0x087acc00,    0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS2, 0, 0, 2, -1, 0, LP0, "02", 0x087acc00,    0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS3, 0, 0, 3, -1, 0, LP0, "03", 0x087acc00,    0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
#endif
#if !defined(NAC_TEST)
#if defined(RAMDISK)
	// 2 regular disks, One SSD, One RAID and four Ram Disk for the RAM DISK BUILD
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS4, 0, 0, 4, -1, 0, LP0, "04", RAM_DISK_SIZE, 0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS5, 0, 0, 5, -1, 0, LP0, "05", RAM_DISK_SIZE, 0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS6, 0, 0, 6, -1, 0, LP0, "06", RAM_DISK_SIZE, 0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS7, 0, 0, 7, -1, 0, LP0, "07", RAM_DISK_SIZE, 0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
#else
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS4, 0, 0, 4,  0, 0, LP0, "04", 0x010f59c7,    0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS5, 0, 0, 5,  0, 0, LP0, "05", 0x010f59c7,    0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS6, 0, 0, 6,  0, 0, LP0, "06", 0x010f59c7,    0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS7, 0, 0, 7,  0, 0, LP0, "07", 0x010f59c7,    0, StateInvalid, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
#endif
#endif
};


//==========================================================================
// SystemVdn.cpp - Default Virtual Device Table
//
// This is the Virtual Device Table for the Test setup
// must be defined after the config data
// 
// NOTE: SystemVdn.cpp is removed!
//       The VIRTUALENTRYs are defined here since they are not used
//       anywhere else. Later only the PTS will have them.

/*************************************************************************/
// Temporary PTS data to match the Vdns in the SystemVdn.h file
// Use these for multiple images (all must be the same)
//
// These Virtual entries will eventually come from the persistant
// table service.

struct {} pddNone[8];	// no config data placeholder

// Define unused parameter types
#define	SSDParms pddNone

#define	NOSLOT	((TySlot)-1)	// need this for secondary devices

// Temporary entries until real PTS is operational

VIRTUALENTRY("HDM_FCP_INIT",		TRUE, vdnInit,				&InitParms[0], 		RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_FCP_TARGET",		TRUE, vdnTarget,		 	&TargetParms[0], 	NIC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_DRIVE_MON",		TRUE, vdnDM, 				&DriveMonParms[0], 	RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_LOOP_MON", 		TRUE, vdnLM, 				&LoopMonParms[0], 	RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_LOOP_MON", 		TRUE, vdnLM1, 				&LoopMonParms[1], 	NIC_SLOT,NOSLOT);
#if defined(NAC_TEST)
VIRTUALENTRY("HDM_LOOP_MON", 		TRUE, vdnLM2, 				&LoopMonParms[2], 	NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_DRIVE_MON",		TRUE, vdnDM3, 				&DriveMonParms[3], 	NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_FCP_INIT",		TRUE, vdnInit3,				&InitParms[3], 		NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_FCP_TARGET",		TRUE, vdnTarget1,		 	&TargetParms[1], 	NAC_SLOT,NOSLOT);
#endif

VIRTUALENTRY("DDM_RedundancyMgr",	TRUE, vdn_RedundancyMgr,	&pddNone, IOP_HBC0, NOSLOT);

VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS0,				&ScsiServerParms[0],  NIC_SLOT,NOSLOT);
#if defined(NAC_TEST)
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS1,				&ScsiServerParms[1],  NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS2,				&ScsiServerParms[2],  NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS3,				&ScsiServerParms[3],  NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS4,				&ScsiServerParms[4],  NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS5,				&ScsiServerParms[5],  NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS6,				&ScsiServerParms[6],  NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS7,				&ScsiServerParms[7],  NAC_SLOT,NOSLOT);
#else
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS1,				&ScsiServerParms[1],  NIC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS2,				&ScsiServerParms[2],  NIC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS3,				&ScsiServerParms[3],  NIC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS4,				&ScsiServerParms[4],  NIC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS5,				&ScsiServerParms[5],  NIC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS6,				&ScsiServerParms[6],  NIC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_STS", 			TRUE, vdnSTS7,				&ScsiServerParms[7],  NIC_SLOT,NOSLOT);
#endif

VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA0,				&BsaParms[0],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA1,				&BsaParms[1],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA2,				&BsaParms[2],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA3,				&BsaParms[3],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA4,				&BsaParms[4],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA5,				&BsaParms[5],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA6,				&BsaParms[6],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA7,				&BsaParms[7],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA8,				&BsaParms[8],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA9,				&BsaParms[9],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA10,				&BsaParms[10],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA11,				&BsaParms[11],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA12,				&BsaParms[12],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA13,				&BsaParms[13],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA14,				&BsaParms[14],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA15,				&BsaParms[15],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA16,				&BsaParms[16],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA17,				&BsaParms[17],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA18,				&BsaParms[18],  RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA19,				&BsaParms[19],  RAC_SLOT,NOSLOT);
#if defined(NAC_TEST)
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA20,				&BsaParms[20],  NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA21,				&BsaParms[21],  NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_BSA", 			TRUE, vdnBSA22,				&BsaParms[22],  NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_RD", 				TRUE, vdnRD0, 				&RamDiskParms[0], NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_RD", 				TRUE, vdnRD1, 				&RamDiskParms[1], NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_RD", 				TRUE, vdnRD2, 				&RamDiskParms[2], NAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_RD", 				TRUE, vdnRD3, 				&RamDiskParms[3], NAC_SLOT,NOSLOT);
#else
VIRTUALENTRY("HDM_RD", 				TRUE, vdnRD0, 				&RamDiskParms[0], RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_RD", 				TRUE, vdnRD1, 				&RamDiskParms[1], RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_RD", 				TRUE, vdnRD2, 				&RamDiskParms[2], RAC_SLOT,NOSLOT);
VIRTUALENTRY("HDM_RD", 				TRUE, vdnRD3, 				&RamDiskParms[3], RAC_SLOT,NOSLOT);
#endif

VIRTUALENTRY("HDM_RAID", 			TRUE, vdnRAID0, 			&RAIDParms[0], RAC_SLOT,NOSLOT);

//VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD0, 				&SSDParms[0], IOP_SSDU2,NOSLOT);

#if 0
// NOTE: we can't have any slots listed here that are not really there because the
// os will wait for them.  So, until later we must comment out any that don't exist.
VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD1, &SSDParms[1], IOP_SSDU0,NOSLOT);
VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD2, &SSDParms[2], IOP_SSDU1,NOSLOT);
VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD3, &SSDParms[3], IOP_SSDL0,NOSLOT);
VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD4, &SSDParms[4], IOP_SSDL1,NOSLOT);
VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD5, &SSDParms[5], IOP_SSDL2,NOSLOT);

#endif

// NOTE THE FOLLOWING LINE MUST BE GREATER THAN THE LENGTH OF SYSTEMVDN.CPP OR DUPLICATE
// DUMMY NAMES MAY RESULT FROM THE VIRTUAL ENTRY MACROS.
// MY_SLOT(IOP_HBC0);	// NO LONGER NEEDED

// AND WHAT ABOUT THIS???
//VIRTUALENTRY("DDM_Registrar", TRUE, vdn_Registrar, &pddNone, IOP_HBC0, IOP_HBC0);

// AND WHAT ABOUT THIS???
//VIRTUALENTRY("DDM_Registrar", TRUE, vdn_Registrar, &pddNone, IOP_HBC0, IOP_HBC0);

// AND WHAT ABOUT THIS???
//VIRTUALENTRY("DDM_Registrar", TRUE, vdn_Registrar, &pddNone, IOP_HBC0, IOP_HBC0);

// AND WHAT ABOUT THIS???
//VIRTUALENTRY("DDM_Registrar", TRUE, vdn_Registrar, &pddNone, IOP_HBC0, IOP_HBC0);

// AND WHAT ABOUT THIS???
//VIRTUALENTRY("DDM_Registrar", TRUE, vdn_Registrar, &pddNone, IOP_HBC0, IOP_HBC0);

// AND WHAT ABOUT THIS???
//VIRTUALENTRY("DDM_Registrar", TRUE, vdn_Registrar, &pddNone, IOP_HBC0, IOP_HBC0);

// AND WHAT ABOUT THIS???
//VIRTUALENTRY("DDM_Registrar", TRUE, vdn_Registrar, &pddNone, IOP_HBC0, IOP_HBC0);

