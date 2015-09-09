/* BuildSys.cpp
 *
 * Copyright (C) ConvergeNet Technologies, 1998, 1999 
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
 *     11/03/98 Tom Nelson: Created
 *     11/24/98 Michael G. Panas: convert DdmImage to BuildSys model
 *     12/22/98 Michael G. Panas: changes to test a standalone RAC,
 *                                initially no NIC emulation
 *     12/24/98 Michael G. Panas: update to use config data for startup,
 *                                include NONIC option
 *     02/12/99 Michael G. Panas: convert to new DDM model
 *     03/01/99 Michael G. Panas: increase to 20 table entries
 *     03/03/99 Michael G. Panas: rewrite most configs, 8 entries for 
 *                                NT 4.0 Max, split for seperate images
 *                                add transport (configurable)
 *     07/07/99 Michael G. Panas: rework to use new DDMs and new config
 *     09/26/99 Michael G. Panas: rework to use new VirtualManager and VirtualMaster
 *
**/

#include "OsTypes.h"
#include "Odyssey.h"
#include "FC_Loop.h"
#include "CtIDLUN.h"

// Include all field defs here
#include "ExportTable.cpp"
#include "DiskDescriptor.cpp"
#include "StorageRollCallTable.cpp"
#include "LoopDescriptor.cpp"
#include "FCPortDatabaseTable.cpp"
#include "SESDescriptor.cpp"
#include "TapeDescriptor.cpp"
#include "IOPStatusTable.cpp"

// Include all config field defs here
#include "FcpConfig.cpp"
#include "DmConfig.cpp"
#include "LmConfig.cpp"
#include "BsaConfig.cpp"
#include "StsConfig.cpp"
#include "RDConfig.cpp"
#include "EsConfig.cpp"

// Tables referenced here
#include "ExportTable.h"
#include "DiskDescriptor.h"
#include "StorageRollCallTable.h"
#include "LoopDescriptor.h"
#include "FCPortDatabaseTable.h"
#include "SESDescriptor.h"
#include "TapeDescriptor.h"
#include "IOPStatusTable.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

#if defined(RAC_BUILD) || defined(NAC_BUILD)
extern "C" U32 gSizeReserved = 0;
extern "C" U32 gSize_total_memory = 200000000;
// Size of all available memory for actual hardware
extern "C" U32 gSize_available_memory = 0x0C800000;       // 200MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap = 0x00800000;             // 8MB

#else
extern "C" U32 gSizeReserved = 0;
extern "C" U32 gSize_total_memory = 16000000;
// Size of all available memory for eval boards
extern "C" U32 gSize_available_memory = 0x00E00000;       // 14MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap = 0x00200000;             // 2MB
#endif

#define		RAM_DISK_SIZE	((1024*1024) / 512)		// 1Mb in blocks

#define	NEED_TRANSPORT	// turn on transport code
//#define	NEED_REPORTER	// turn on reporter code

// slot and chip defines needed for eval systems
#define	EV_NIC_SLOT		0		// always slot 0 for both
#define	EV_RAC_SLOT		0
#define	EV_NAC_SLOT		0

#if !defined(NIC_TEST_BUILD) && !defined(RAC_TEST_BUILD)
#define	NIC_CHIP		0
#define	RAC_CHIP		1

#else
#define	NIC_CHIP		0		// must be zero so the instance index is zero
#define	RAC_CHIP		0

#endif

#define	NO_SLOT		(TySlot)(-1)

//=========================================================================
//#define	NONIC			// define this if the NIC is not desired
//#define	NORAC			// define this if the RAC is not desired

//#define   FCP_SINGLE_BUILD	// This will build a Single Test image.
//                              // DriveMonitor and RAC Initiator only
//#define	RD_BUILD		// defined when a Ram Disk Image is built.

// The NIC and RAC images run on different Ev Boards, message transport
// between boards must be available.
//#define	NIC_TEST_BUILD	// defined when a NIC Image is built.
//#define	RAC_TEST_BUILD	// defined when a RAC Image is built.

							// NOTE: these flags will be defined in the
							// project prefix file as needed
//=========================================================================

//=========================================================================
//
//	Dummy methods needed

#include "HdmFcpTarget.h"
#include "HdmFcpInit.h"

#ifdef NONIC
VDN HdmFcpTarget::FCP_Find_Next_Virtual_Device(U32 key)
{	
	return(0);
	
}	// FCP_Find_Next_Virtual_Device

#endif


#include "BuildSys.h"

//=========================================================================
// Class Table
// This is the Class Table for the Test setup
// 
// Update Log: 
// 9/3/98 Michael G. Panas: Create file
// 10/4/98: Add EchoScsi
// 10/8/98: Add DriveMon
// 11/24/98: updated to the BuildSys model
//
// Literal names/StackSize/Queue Size/Class for all DDM classes

#include "DdmPTS.h"
#include "EchoScsiIsm.h"
#include "DriveMonitorIsm.h"
#include "BsaIsm.h"
#ifdef SCSI_TARGET_SERVER
#include "ScsiServerIsm.h"
#endif
#include "FcpData.h"
#include "LoopMonitorIsm.h"

CLASSENTRY("HDM_DMGR",		10240, 1024, DdmManager );
CLASSENTRY("HDM_TPT",		10240, 1024, Transport );
CLASSENTRY("HDM_TIMR",		10240, 1024, DdmTimer );

CLASSENTRY("PTSLOADER",		10240,1024, DdmPtsLoader);         // Loads real Pts
CLASSENTRY("VIRTUALMANAGER", 10240,1024, DdmVirtualManager);
CLASSENTRY("DDMCMB",		10240,1024, DdmCmbNull);           // dummy CMB

CLASSENTRY("HDM_BSA",  		10240, 1024, BsaIsm );

#if !defined(NIC_TEST_BUILD)
// HBC only DDMs
CLASSENTRY("PTS",  			10240, 1024, DdmPTS );				// the real PTS
CLASSENTRY("VMS",			10240, 1024, DdmVirtualMaster );
//CLASSENTRY("BOOTMAN",  	10240, 1024, DdmBootMgr );
#endif
#if !defined(NIC_TEST_BUILD) && !defined(FCP_SINGLE_BUILD)
CLASSENTRY("PTSDEFAULT",  10240, 1024, DdmPtsDefault );
#endif

#include "DdmConsoleTest.h"
CLASSENTRY("DDM_CONS",  10240, 1024, DdmConsoleTest );

#ifndef NORAC
CLASSENTRY("HDM_FCP_INIT",  10240, 1024, HdmFcpInit );
#endif
#ifndef NONIC
CLASSENTRY("HDM_FCP_TARG",  10240, 1024, HdmFcpTarget );

#if !defined(SCSI_TARGET_SERVER) && !defined(NORAC) || defined(NIC_TEST_BUILD)
CLASSENTRY("HDM_ECHO_SCSI",  10240, 1024, EchoScsiIsm );
#endif
#endif
#if !defined(NIC_TEST_BUILD)
CLASSENTRY("HDM_DRIVE_MON",  10240, 1024, DriveMonitorIsm );
#endif
#ifdef SCSI_TARGET_SERVER
CLASSENTRY("HDM_STS",  10240, 1024, ScsiServerIsm );
#ifdef NEED_REPORTER
#include "DdmReporter.h"
CLASSENTRY("HDM_REPORT",  10240, 1024, DdmReporter );
#endif
#endif
#if defined(RD_BUILD)
#include "RdIsm.h"
CLASSENTRY("HDM_RD",  10240, 1024, RdIsm );
#endif
CLASSENTRY("HDM_LOOP_MON",  10240, 1024, LoopMonitorIsm );


//============================================================================
// Device Table
//
// These functions are each called once at system startup
// before the DDMs are enabled.

DEVICEENTRY("Interrupt",	Interrupt);
DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DdmManager",	DdmManager);
DEVICEENTRY("PIT",			TimerPIT);
DEVICEENTRY("Transport_Pci",Transport_Pci);
//DEVICEENTRY("Transport_Net",Transport_Net);  // don't use this on Eval Systems
DEVICEENTRY("Transport",        Transport);
//DEVICEENTRY("FailSafe", 	FailSafe);



//=============================================================================
// System DDMs Table
// These have no Virtual Device Numbers
//
SYSTEMENTRY("HDM_DMGR");		// Must be first
SYSTEMENTRY("PTSLOADER");       
SYSTEMENTRY("DDMCMB");       
#if !defined(RAC_BUILD) && !defined(FCP_SINGLE_BUILD)
SYSTEMENTRY("PTSDEFAULT");		// third
#endif
SYSTEMENTRY("HDM_TPT");			// Transport
SYSTEMENTRY("HDM_TIMR");		// Timer
SYSTEMENTRY("DDM_CONS");		// Console DDM
SYSTEMENTRY("VIRTUALMANAGER");	// VM


//==============================================================================
// Virtual Device Numbers
//

// Now use this when building all images
// These numbers will eventually come from the persistant
// table service (on an Odyssey).
#include "SystemVdn.h"
#define	vdnES (vdnLAST+1)
#define	vdnES1 (vdnLAST+2)
#define	vdnES2 (vdnLAST+3)
#define	vdnES3 (vdnLAST+4)
#define	vdnES4 (vdnLAST+5)
#define	vdnES5 (vdnLAST+6)
#define	vdnES6 (vdnLAST+7)
#define	vdnES7 (vdnLAST+8)
#define	vdnES8 (vdnLAST+9)
#define	vdnES9 (vdnLAST+10)
#define	vdnES10 (vdnLAST+11)
#define	vdnES11 (vdnLAST+12)
#define	vdnES12 (vdnLAST+13)
#define	vdnES13 (vdnLAST+14)
#define	vdnES14 (vdnLAST+15)
#define	vdnES15 (vdnLAST+16)
#define	vdnES16 (vdnLAST+17)
#define	vdnES17 (vdnLAST+18)
#define	vdnES18 (vdnLAST+19)
#define	vdnES19 (vdnLAST+20)

//=============================================================================
// PTS Name and VMS Name
//
PTSNAME("PTS", vdnPTS, IOP_HBC0,NO_SLOT);
VMSNAME("VMS", vdnVMS, IOP_HBC0,NO_SLOT);


//=========================================================================
// This section declares the tables for the dummy Persistent Data service
// 
// Update Log: 
// 9/3/98 Michael G. Panas: Create file
// 11/23/98 Michael G. Panas: update to new DiskDescriptor header
// 11/24/98 Michael G. Panas: move to BuildSys.cpp
/*************************************************************************/

#include "PersistentData.h"
#include "FcpConfig.h"
#include "FcpIsp.h"
#include "FcpData.h"


FCP_CONFIG	TargetParms[3] = {
	// Instance 0
	{
	{0,0,0},
	FCP_CONFIG_VERSION,
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
	1024,	// high level interrupt stack size
	8,		// number of Target IDs (max 31)
	8,		// number of LUNs
	30,		// number of PCI window buffers
	
	// this value is changed by the startup code
	// based on the target build
	TARGET_INSTANCE,	// instance number
	FCLOOPINSTANCE(EV_NIC_SLOT, 0),	// calculate the loop instance num
	vdnLM},		// virtual device to throw events to
	
	// Instance 1
	{
	{0,0,0},
	FCP_CONFIG_VERSION,
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
	1024,	// high level interrupt stack size
	8,		// number of Target IDs (max 31)
	8,		// number of LUNs
	30,		// number of PCI window buffers
	
	// this value is changed by the startup code
	// based on the target build
	TARGET_INSTANCE,	// instance number
	FCLOOPINSTANCE(EV_NIC_SLOT, 1),	// calculate the loop instance num
	vdnLM},		// virtual device to throw events to
	
	// Instance 2
	{
	{0,0,0},
	FCP_CONFIG_VERSION,
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
	1024,	// high level interrupt stack size
	8,		// number of Target IDs (max 31)
	8,		// number of LUNs
	30,		// number of PCI window buffers
	
	// this value is changed by the startup code
	// based on the target build
	TARGET_INSTANCE,	// instance number
	FCLOOPINSTANCE(EV_NIC_SLOT, 2),	// calculate the loop instance num
	vdnLM}	// virtual device to throw events to
};
	
FCP_CONFIG	InitParms[3] = {
	// Instance 0
	{
	{0,0,0},
	FCP_CONFIG_VERSION,
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
	1024,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	
	// these values are changed by the startup code
	// based on the target build
	INITIATOR_INSTANCE,	// instance number
	FCLOOPINSTANCE(EV_RAC_SLOT, 0),	// calculate the loop instance num
	vdnLM},		// virtual device to throw events to
	
	// Instance 1
	{
	{0,0,0},
	FCP_CONFIG_VERSION,
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
	1024,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	
	// these values are changed by the startup code
	// based on the target build
	INITIATOR_INSTANCE,	// instance number
	FCLOOPINSTANCE(EV_RAC_SLOT, 1),	// calculate the loop instance num
	vdnLM},		// virtual device to throw events to
	
	// Instance 2
	{
	{0,0,0},
	FCP_CONFIG_VERSION,
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
	1024,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	
	// these values are changed by the startup code
	// based on the target build
	INITIATOR_INSTANCE,	// instance number
	FCLOOPINSTANCE(EV_RAC_SLOT, 2),	// calculate the loop instance num
	vdnLM}		// virtual device to throw events to
};


#include "LmConfig.h"

LM_CONFIG	LoopMonParms[1] = { 
	{{0,0,0},
	LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
	2,							// number of loops to scan
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
#if defined(NAC_BUILD)
	FCLOOPINSTANCE(EV_RAC_SLOT, 0),	
	//FCLOOPINSTANCE(EV_RAC_SLOT, 1),	
	FCLOOPINSTANCE(EV_RAC_SLOT, 2),	
#else
	FCLOOPINSTANCE(EV_RAC_SLOT, NIC_CHIP),	// loop instance num of NIC
	FCLOOPINSTANCE(EV_RAC_SLOT, RAC_CHIP),	// loop instance num of RAC
#endif
	}
	}
};

#include "DmConfig.h"

DM_CONFIG	DriveMonParms[3] = { 
	{
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	20,							// number of drives to scan
	DM_FLAGS_SPIN_4X,			// DM Flags
	vdnInit,
	FCLOOPINSTANCE(EV_RAC_SLOT, 0),	// calculate the loop instance num
	//{0 },						// DM mode flags (per drive)
	{
#if defined(DDH)
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
	 0  } // rest are all zeros	
	 },
	 
	{
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	20,							// number of drives to scan
	DM_FLAGS_SPIN_4X,			// DM Flags
	vdnInit1,
	FCLOOPINSTANCE(EV_RAC_SLOT, 1),	// calculate the loop instance num
	//{0 },						// DM mode flags (per drive)
	{
#if defined(DDH)
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
	 0  } // rest are all zeros	
	 },
	 
	{
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	20,							// number of drives to scan
	DM_FLAGS_SPIN_4X,			// DM Flags
	vdnInit2,
	FCLOOPINSTANCE(EV_RAC_SLOT, 2),	// calculate the loop instance num
	//{0 },						// DM mode flags (per drive)
	{
#if defined(DDH)
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
	 0  } // rest are all zeros	
	 },
};

#ifdef SCSI_TARGET_SERVER

#include "BsaIsm.h"

BSA_CONFIG BsaParms[] = {
#if defined(NAC_BUILD)
// 	  Version               size               LUN ID  Init ID SMART
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  0, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  1, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  2, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  3, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  4, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  5, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  6, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  7, vdnInit2, 0	},
#else
// 	  Version               size               LUN ID  Init ID SMART
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  0, vdnInit1, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  1, vdnInit1, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  2, vdnInit1, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  3, vdnInit1, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  4, vdnInit1, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  5, vdnInit1, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  6, vdnInit1, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  7, vdnInit1, 0	}
#endif
};

#include "StsConfig.h"

STS_CONFIG	ScsiServerParms[] = 
	{ 
// 	  rid  Version             size                rid  vdnNext
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnBSA0 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnBSA1 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnBSA2 },
#if defined(RD_BUILD)
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnRD0 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnRD1 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnRD2 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnRD3 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnRD4 }
#else
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnBSA3 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnBSA4 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnBSA5 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnBSA6 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnBSA7 }
#endif
};
#endif

// RAM Disk config data
#include "RDConfig.h"

RD_CONFIG	RamDiskParms[] = 
{ 
	{ {0,0,0}, RD_CONFIG_VERSION, sizeof(RD_CONFIG), RAM_DISK_SIZE },		// RAM DISK 0
	{ {0,0,0}, RD_CONFIG_VERSION, sizeof(RD_CONFIG), RAM_DISK_SIZE },		// RAM DISK 1
	{ {0,0,0}, RD_CONFIG_VERSION, sizeof(RD_CONFIG), RAM_DISK_SIZE },		// RAM DISK 2
	{ {0,0,0}, RD_CONFIG_VERSION, sizeof(RD_CONFIG), RAM_DISK_SIZE },		// RAM DISK 3
	{ {0,0,0}, RD_CONFIG_VERSION, sizeof(RD_CONFIG), RAM_DISK_SIZE },		// RAM DISK 4
	{ {0,0,0}, RD_CONFIG_VERSION, sizeof(RD_CONFIG), RAM_DISK_SIZE }		// RAM DISK 5
};

#include "EsConfig.h"
#if defined(NAC_BUILD)
#define	ESNEXT	vdnInit2
#else
#define	ESNEXT	vdnInit1
#endif

ES_CONFIG EchoScsiIsmParms[] = {
// 	  Version            size               ID  LUN vndNext
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 0,  0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 1,  0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 2,  0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 3,  0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 4,  0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 5,  0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 6,  0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 7,  0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 8,  0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 9,  0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 10, 0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 11, 0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 12, 0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 13, 0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 14, 0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 15, 0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 16, 0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 17, 0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 18, 0, ESNEXT },
	{ {0,0,0}, ES_CONFIG_VERSION, sizeof(ES_CONFIG), 19, 0, ESNEXT }
	};


// This is the Virtual Device Table for the Test setup
// 
// Update Log: 
// 9/3/98 Michael G. Panas: Create file
// 10/2/98 Michael G. Panas: Add the Echo Scsi Ism
// 10/8/98 Michael G. Panas: Add the Drive Monitor Ism
// 11/24/98 Michael G. Panas: update to  BuildSys model
// 02/15/99 Michael G. Panas: update to new Oos/BuildSys model, use 
//                            VIRTUALENTRY() to define DDMs
// 03/01/99 Michael G. Panas: use 20 SCSI Target servers, 8 devices max for NT 4.0
/*************************************************************************/
// Temporary PTS data

struct {} pddNone;

#if defined(NIC_TEST_BUILD) || defined(RAC_TEST_BUILD)
// use these for multiple images (all must be the same)
VIRTUALENTRY("HDM_FCP_TARG", 	TRUE, vdnTarget, IOP_HBC1,NO_SLOT,  FCP_CONFIG, &TargetParms);
VIRTUALENTRY("HDM_FCP_INIT", 	TRUE, vdnInit, 	 IOP_HBC0,NO_SLOT,  FCP_CONFIG, &InitParms);
VIRTUALENTRY("HDM_DRIVE_MON", 	TRUE, vdnDM, 	 IOP_HBC0,NO_SLOT,  DM_CONFIG,  &DriveMonParms[0]);
VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM, 	 IOP_HBC0,NO_SLOT,  LM_CONFIG,  &LoopMonParms[0]);
//VIRTUALENTRY("HDM_REPORT", 	TRUE, vdnReporter, IOP_HBC0,NO_SLOT,  struct {}, &pddNone);

#ifdef SCSI_TARGET_SERVER
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS0,  IOP_HBC1,NO_SLOT, STS_CONFIG, &ScsiServerParms[0]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS1,  IOP_HBC1,NO_SLOT, STS_CONFIG, &ScsiServerParms[1]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS2,  IOP_HBC1,NO_SLOT, STS_CONFIG, &ScsiServerParms[2]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS3,  IOP_HBC1,NO_SLOT, STS_CONFIG, &ScsiServerParms[3]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS4,  IOP_HBC1,NO_SLOT, STS_CONFIG, &ScsiServerParms[4]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS5,  IOP_HBC1,NO_SLOT, STS_CONFIG, &ScsiServerParms[5]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS6,  IOP_HBC1,NO_SLOT, STS_CONFIG, &ScsiServerParms[6]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS7,  IOP_HBC1,NO_SLOT, STS_CONFIG, &ScsiServerParms[7]);

VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA0,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[0]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA1,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[1]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA2,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[2]);
#if !defined(RD_BUILD)
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA3,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[3]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA4,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[4]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA5,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[5]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA6,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[6]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA7,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[7]);
#endif
#endif

#if defined(RD_BUILD)
VIRTUALENTRY("HDM_RD", TRUE, vdnRD0, IOP_HBC0,NO_SLOT, RD_CONFIG, &RamDiskParms[0]);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD1, IOP_HBC0,NO_SLOT, RD_CONFIG, &RamDiskParms[1]);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD2, IOP_HBC0,NO_SLOT, RD_CONFIG, &RamDiskParms[2]);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD3, IOP_HBC0,NO_SLOT, RD_CONFIG, &RamDiskParms[3]);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD4, IOP_HBC0,NO_SLOT, RD_CONFIG, &RamDiskParms[4]);
#endif
#if !defined(SCSI_TARGET_SERVER) && !defined(NORAC)
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES,   IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[0]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES1,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[1]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES2,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[2]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES3,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[3]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES4,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[4]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES5,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[5]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES6,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[6]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES7,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[7]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES8,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[8]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES9,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[9]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES10, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[10]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES11, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[11]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES12, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[12]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES13, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[13]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES14, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[14]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES15, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[15]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES16, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[16]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES17, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[17]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES18, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[18]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES19, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[19]);
#endif



#else
// use these for single board images
#ifndef NORAC
#if defined(NAC_BUILD)
#if defined(FCP_SINGLE_BUILD)
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit,	IOP_HBC0,NO_SLOT, FCP_CONFIG, &InitParms[0]);
//VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit1,	IOP_HBC0,NO_SLOT, FCP_CONFIG, &InitParms[1]);
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit2,	IOP_HBC0,NO_SLOT, FCP_CONFIG, &InitParms[2]);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM,	IOP_HBC0,NO_SLOT, DM_CONFIG,  &DriveMonParms[0]);
//VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM1,	IOP_HBC0,NO_SLOT, DM_CONFIG,  &DriveMonParms[1]);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM2,	IOP_HBC0,NO_SLOT, DM_CONFIG,  &DriveMonParms[2]);
#else
// not a SINGLE - only one (two) initiator configured
//VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit1,	IOP_HBC0,NO_SLOT, FCP_CONFIG, &InitParms[1]);
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit2,	IOP_HBC0,NO_SLOT, FCP_CONFIG, &InitParms[2]);
//VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM1,	IOP_HBC0,NO_SLOT, DM_CONFIG,  &DriveMonParms[1]);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM2,	IOP_HBC0,NO_SLOT, DM_CONFIG,  &DriveMonParms[2]);
#endif
#else
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit1,	IOP_HBC0,NO_SLOT, FCP_CONFIG, &InitParms[RAC_CHIP]);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM,	IOP_HBC0,NO_SLOT, DM_CONFIG,  &DriveMonParms[RAC_CHIP]);
#endif
#endif

#ifndef NONIC
#if defined(NAC_BUILD)
VIRTUALENTRY("HDM_FCP_TARG", TRUE, vdnTarget,	IOP_HBC0,NO_SLOT, FCP_CONFIG, &TargetParms[0]);
#else // not NAC_BUILD
VIRTUALENTRY("HDM_FCP_TARG", TRUE, vdnTarget,	IOP_HBC0,NO_SLOT, FCP_CONFIG, &TargetParms[NIC_CHIP]);
#endif // NAC_BUILD

#if !defined(SCSI_TARGET_SERVER) && !defined(NORAC)
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES,   IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[0]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES1,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[1]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES2,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[2]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES3,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[3]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES4,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[4]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES5,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[5]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES6,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[6]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES7,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[7]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES8,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[8]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES9,  IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[9]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES10, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[10]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES11, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[11]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES12, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[12]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES13, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[13]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES14, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[14]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES15, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[15]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES16, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[16]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES17, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[17]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES18, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[18]);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES19, IOP_HBC0,NO_SLOT, ES_CONFIG, &EchoScsiIsmParms[19]);
#endif

#endif // NONIC

VIRTUALENTRY("HDM_LOOP_MON", TRUE, vdnLM, 	IOP_HBC0,NO_SLOT, LM_CONFIG,  &LoopMonParms[0]);

#ifdef SCSI_TARGET_SERVER
#ifdef NEED_REPORTER
VIRTUALENTRY("HDM_REPORT", TRUE, vdnReporter, IOP_HBC0,NO_SLOT, struct {}, &pddNone);
#endif

VIRTUALENTRY("HDM_STS", TRUE, vdnSTS0,  IOP_HBC0,NO_SLOT, STS_CONFIG, &ScsiServerParms[0]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS1,  IOP_HBC0,NO_SLOT, STS_CONFIG, &ScsiServerParms[1]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS2,  IOP_HBC0,NO_SLOT, STS_CONFIG, &ScsiServerParms[2]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS3,  IOP_HBC0,NO_SLOT, STS_CONFIG, &ScsiServerParms[3]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS4,  IOP_HBC0,NO_SLOT, STS_CONFIG, &ScsiServerParms[4]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS5,  IOP_HBC0,NO_SLOT, STS_CONFIG, &ScsiServerParms[5]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS6,  IOP_HBC0,NO_SLOT, STS_CONFIG, &ScsiServerParms[6]);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS7,  IOP_HBC0,NO_SLOT, STS_CONFIG, &ScsiServerParms[7]);

VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA0,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[0]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA1,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[1]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA2,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[2]);
#if !defined(RD_BUILD)
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA3,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[3]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA4,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[4]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA5,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[5]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA6,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[6]);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA7,  IOP_HBC0,NO_SLOT, BSA_CONFIG, &BsaParms[7]);
#endif
#endif

#if defined(RD_BUILD)
VIRTUALENTRY("HDM_RD", TRUE, vdnRD0, IOP_HBC0,NO_SLOT, RD_CONFIG, &RamDiskParms[0]);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD1, IOP_HBC0,NO_SLOT, RD_CONFIG, &RamDiskParms[1]);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD2, IOP_HBC0,NO_SLOT, RD_CONFIG, &RamDiskParms[2]);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD3, IOP_HBC0,NO_SLOT, RD_CONFIG, &RamDiskParms[3]);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD4, IOP_HBC0,NO_SLOT, RD_CONFIG, &RamDiskParms[4]);
#endif

#endif
#if !defined(FCP_SINGLE_BUILD) && !defined(NONIC)

#endif


//===========================================================================
/*
struct ExportTableEntry {
	rowID			rid;					// rowID of this table row.
	U32 			version;				// Version of Export Table record.
	U32				size;					// Size of Export Table record in bytes.
	rowID			ridVCMCommand;			// For VCM use ONLY.  rowID of VCM Command that created this circuit.
	CTProtocolType	ProtocolType;			// FCP, IP, other
	VDN				vdNext;					// First Virtual Device number in the chain
	VDN				vdLegacyBsa;			// Virtual Device number of the legacy BSA
	VDN				vdLegacyScsi;			// Virtual Device number of the legacy SCSI
	U32				ExportedLUN;			// LUN number exported
	U32				InitiatorId;			// Host ID
	U32				TargetId;				// our ID
	U32				FCInstance;				// FC Loop number
	String32		SerialNumber;			// Use a string array for Serial Number
	I64				Capacity;				// Capacity of this Virtual Circuit
	U32				FailState;
	CTReadyState	ReadyState;				// Current state
	CTReadyState	DesiredReadyState;		// Desired Ready state
	String16		WWNName;				// World Wide Name (64 or 128-bit IEEE registered)
	rowID			ridUserInfo;			// rowID of user info for this VC
	rowID			ridEIP;					// rowID of External Initiator Port in FC Port DB. 
	rowID			ridSTSStatusRec;		// rowID of SCSI Target Server's Status Record
	rowID			ridSTSPerformanceRec;	// rowID of SCSI Target Server's Performance Record
	rowID			ridAltExportRec;		// rowID of our Alternate Path's Export Record
	} ExportTableEntry, ExportTableRecord;
*/

// helpers:
#define	PFC		ProtocolFibreChannel
#define	SZ		sizeof(ExportTableEntry)

#define	LP0		(FCLOOPINSTANCE(EV_RAC_SLOT, 0))
#define	LP1		(FCLOOPINSTANCE(EV_RAC_SLOT, 1))
#define	LP2		(FCLOOPINSTANCE(EV_RAC_SLOT, 2))

// Loop the Target runs on
#define	EX_INST	LP0

#if defined(SCSI_TARGET_SERVER) || defined(NIC_TEST_BUILD) || defined(RAC_TEST_BUILD)

U32		numExportRows = 8;

ExportTableEntry			Export[8] = 	// one for each VirtualCircuit
{
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS0, 0, 0, 0, -1, 2, EX_INST, "00", 0x010f59c7,    0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS1, 0, 0, 1, -1, 2, EX_INST, "01", 0x010f59c7,    0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS2, 0, 0, 2, -1, 2, EX_INST, "02", 0x010f59c7,    0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
#if defined(RD_BUILD)
	// 2 regular disks and one Ram Disk for the RAM DISK BUILD
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS3, 0, 0, 3,  0, 2, EX_INST, "03", RAM_DISK_SIZE, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS4, 0, 0, 4,  0, 2, EX_INST, "04", RAM_DISK_SIZE, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS5, 0, 0, 5,  0, 2, EX_INST, "05", RAM_DISK_SIZE, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS6, 0, 0, 6,  0, 2, EX_INST, "06", RAM_DISK_SIZE, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS7, 0, 0, 7,  0, 2, EX_INST, "07", RAM_DISK_SIZE, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
#else
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS3, 0, 0, 3,  0, 2, EX_INST, "03", 0x010f59c7,    0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS4, 0, 0, 4,  0, 2, EX_INST, "04", 0x010f59c7,    0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS5, 0, 0, 5,  0, 2, EX_INST, "05", 0x010f59c7,    0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS6, 0, 0, 6,  0, 2, EX_INST, "06", 0x010f59c7,    0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS7, 0, 0, 7,  0, 2, EX_INST, "07", 0x010f59c7,    0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
#endif
};

#else

#if !defined(FCP_SINGLE_BUILD) && !defined(NONIC)

U32		numExportRows = 20;

ExportTableEntry			Export[20] = 	// one for each VirtualCircuit
{
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES,   0, 0, 0,  -1, 2, EX_INST, "01", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES1,  0, 0, 1,  -1, 2, EX_INST, "02", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES2,  0, 0, 2,  -1, 2, EX_INST, "03", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES3,  0, 0, 3,  -1, 2, EX_INST, "04", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES4,  0, 0, 4,  -1, 2, EX_INST, "05", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES5,  0, 0, 5,  -1, 2, EX_INST, "06", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES6,  0, 0, 6,  -1, 2, EX_INST, "07", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES7,  0, 0, 7,  -1, 2, EX_INST, "08", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES8,  0, 0, 8,  -1, 2, EX_INST, "09", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES9,  0, 0, 9,  -1, 2, EX_INST, "10", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES10, 0, 0, 10, -1, 2, EX_INST, "11", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES11, 0, 0, 11, -1, 2, EX_INST, "12", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES12, 0, 0, 12, -1, 2, EX_INST, "13", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES13, 0, 0, 13, -1, 2, EX_INST, "14", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES14, 0, 0, 14, -1, 2, EX_INST, "15", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES15, 0, 0, 15, -1, 2, EX_INST, "16", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES16, 0, 0, 16, -1, 2, EX_INST, "17", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES17, 0, 0, 17, -1, 2, EX_INST, "18", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES18, 0, 0, 18, -1, 2, EX_INST, "19", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES19, 0, 0, 19, -1, 2, EX_INST, "20", 0, 0, StateConfigured, StateConfiguredAndActive, " ", {0},{0},{0},{0}}
};

#endif
#endif

//=========================================================================
// Boot Data
//
// These values will eventually come from the Boot ROM
// 
//  Boot Configuration (Temporary)
//

#if defined(NIC_TEST_BUILD)
struct {U32 iCabinet; TySlot iSlot;} bddDdmManager = {
        0, IOP_HBC1
};
#else

struct {U32 iCabinet; TySlot iSlot;} bddDdmManager = {
        0, IOP_HBC0
};
#endif

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

struct {U32 divisor;}  bddTimer={10000};

BOOTENTRY("DdmManager", &bddDdmManager );
BOOTENTRY("Transport",  &bddMessenger );
BOOTENTRY("Timer",      &bddTimer );


