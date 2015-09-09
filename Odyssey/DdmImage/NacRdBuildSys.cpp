/* NacRdBuildSys.cpp
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
 *     08/03/99 Michael G. Panas: rework to NAC RAM Disk config only
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

// Tables referenced here
#include "ExportTable.h"
#include "DiskDescriptor.h"
#include "StorageRollCallTable.h"
#include "LoopDescriptor.h"
#include "FCPortDatabaseTable.h"
#include "SESDescriptor.h"
#include "TapeDescriptor.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

// Size of all available memory for actual hardware
extern "C" U32 gSize_available_memory = 0x0C800000;       // 200MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap = 0x00800000;             // 8MB

#define		RAM_DISK_SIZE	(((1024*1024) / 512)*10)		// 10Mb in blocks

#define	NEED_TRANSPORT	// turn on transport code
//#define	NEED_REPORTER	// turn on reporter code

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
#include "ScsiServerIsm.h"
#include "FcpData.h"
#include "LoopMonitorIsm.h"
#include "DdmConsoleTest.h"
#include "RdIsm.h"

CLASSENTRY("HDM_BSA",  10240, 1024, BsaIsm );
CLASSENTRY("HDM_STS",  10240, 1024, ScsiServerIsm );

CLASSENTRY("HDM_DMGR", 10240, 1024, DdmManager );
CLASSENTRY("HDM_TPT",  10240, 1024, Transport );
CLASSENTRY("HDM_TIMR",  10240, 1024, DdmTimer );

CLASSENTRY("PTSPROXYLOADER",10240,1024,DdmPtsProxyLoader);      // Loads PtsProxy 
CLASSENTRY("PTSLOADER",    10240,1024, DdmPtsLoader);           // Loads real Pts
CLASSENTRY("VIRTUALPROXY",  10240, 1024, DdmVirtualProxy );
CLASSENTRY("PTS",  10240, 1024, DdmPTS );						// the real PTS
CLASSENTRY("PTSPROXY",  10240, 1024, DdmPtsProxy );
CLASSENTRY("DDM_CONS",  10240, 1024, DdmConsoleTest );

#if !defined(FCP_SINGLE_BUILD)
CLASSENTRY("PTSDEFAULT",  10240, 1024, DdmPtsDefault );
#endif // FCP_SINGLE_BUILD

CLASSENTRY("HDM_FCP_INIT",  10240, 1024, HdmFcpInit );
CLASSENTRY("HDM_FCP_TARG",  10240, 1024, HdmFcpTarget );
CLASSENTRY("HDM_DRIVE_MON",  10240, 1024, DriveMonitorIsm );

#if !defined(SCSI_TARGET_SERVER)
CLASSENTRY("HDM_ECHO_SCSI",  10240, 1024, EchoScsiIsm );
#endif // SCSI_TARGET_SERVER


CLASSENTRY("HDM_RD",  10240, 1024, RdIsm );
CLASSENTRY("HDM_LOOP_MON",  10240, 1024, LoopMonitorIsm );

#ifdef NEED_REPORTER
#include "DdmReporter.h"
CLASSENTRY("HDM_REPORT",  10240, 1024, DdmReporter );
#endif

//============================================================================
// Device Table
//
// These functions are each called once at system startup
// before the DDMs are enabled.

DEVICEENTRY("Interrupt",	Interrupt);
DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DdmManager",	DdmManager);
DEVICEENTRY("PIT",			TimerPIT);
DEVICEENTRY("Transport",	Transport);	
//DEVICEENTRY("FailSafe", 	FailSafe);


//=============================================================================
// System DDMs Table
//
SYSTEMENTRY("HDM_DMGR");		// Must be first
SYSTEMENTRY("PTSPROXYLOADER");  // Must be before VirtualProxy
SYSTEMENTRY("PTSLOADER");       
SYSTEMENTRY("VIRTUALPROXY");    // Must be after PtsProxyLoader
#if !defined(RAC_BUILD) && !defined(FCP_SINGLE_BUILD)
SYSTEMENTRY("PTSDEFAULT");		// third
#endif
SYSTEMENTRY("HDM_TPT");
SYSTEMENTRY("HDM_TIMR");
SYSTEMENTRY("DDM_CONS");


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
// PTS Proxy and PTS
//
PTSPROXY("PTSPROXY", vdnPTS, NAC_SLOT,NO_SLOT);
PTSNAME("PTS", vdnTS, NAC_SLOT,NO_SLOT);


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
	FCLOOPINSTANCE(NAC_SLOT, 0),	// calculate the loop instance num
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
	FCLOOPINSTANCE(NAC_SLOT, 1),	// calculate the loop instance num
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
	FCLOOPINSTANCE(NAC_SLOT, 2),	// calculate the loop instance num
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
	FCLOOPINSTANCE(NAC_SLOT, 0),	// calculate the loop instance num
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
	FCLOOPINSTANCE(NAC_SLOT, 1),	// calculate the loop instance num
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
	FCLOOPINSTANCE(NAC_SLOT, 2),	// calculate the loop instance num
	vdnLM}		// virtual device to throw events to
};


#include "LmConfig.h"

LM_CONFIG	LoopMonParms[1] = {
	{	 
	{0,0,0},
	LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
	2,							// number of loops to scan
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	FCLOOPINSTANCE(NAC_SLOT, 0),	
	//FCLOOPINSTANCE(NAC_SLOT, 1),	// rev E1 only
	FCLOOPINSTANCE(NAC_SLOT, 2),	
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
	0,							// DM Flags
	vdnInit,
	FCLOOPINSTANCE(NAC_SLOT, 0),	// calculate the loop instance num
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
	0,							// DM Flags
	vdnInit1,
	FCLOOPINSTANCE(NAC_SLOT, 1),	// calculate the loop instance num
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
	0,							// DM Flags
	vdnInit2,
	FCLOOPINSTANCE(NAC_SLOT, 2),	// calculate the loop instance num
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
// 	  Version               size               LUN ID  Init ID SMART
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  0, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  1, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  2, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  3, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  4, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  5, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  6, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  7, vdnInit2, 0	},
};

#include "StsConfig.h"

STS_CONFIG	ScsiServerParms[] = 
	{ 
// 	  rid  Version             size                rid  vdnNext
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnRD0 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnRD1 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnRD2 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnRD3 },
	{ {0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0}, vdnRD4 }
};
#endif // SCSI_TARGET_SERVER

// RAM Disk config data
U32		RamDiskParms[] =
			 {
			 RAM_DISK_SIZE,		// RAM DISK 0
			 RAM_DISK_SIZE,		// RAM DISK 1
			 RAM_DISK_SIZE,		// RAM DISK 2
			 RAM_DISK_SIZE,		// RAM DISK 3
			 RAM_DISK_SIZE,		// RAM DISK 4
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

#if defined(FCP_SINGLE_BUILD)
VIRTUALENTRY("HDM_FCP_INIT", TRUE, vdnInit, &InitParms[0], NAC_SLOT,NO_SLOT);
//VIRTUALENTRY("HDM_FCP_INIT", TRUE, vdnInit1, &InitParms[1], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_FCP_INIT", TRUE, vdnInit2, &InitParms[2], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_DRIVE_MON", TRUE, vdnDM, &DriveMonParms[0], NAC_SLOT,NO_SLOT);
//VIRTUALENTRY("HDM_DRIVE_MON", TRUE, vdnDM1, &DriveMonParms[1], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_DRIVE_MON", TRUE, vdnDM2, &DriveMonParms[2], NAC_SLOT,NO_SLOT);

#else // not FCP_SINGLE_BUILD
// not a SINGLE - only one (two) initiator configured
//VIRTUALENTRY("HDM_FCP_INIT", TRUE, vdnInit1, &InitParms[1], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_FCP_INIT", TRUE, vdnInit2, &InitParms[2], NAC_SLOT,NO_SLOT);
//VIRTUALENTRY("HDM_DRIVE_MON", TRUE, vdnDM1, &DriveMonParms[1], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_DRIVE_MON", TRUE, vdnDM2, &DriveMonParms[2], NAC_SLOT,NO_SLOT);
#endif // FCP_SINGLE_BUILD
VIRTUALENTRY("HDM_LOOP_MON", TRUE, vdnLM, &LoopMonParms[0], NAC_SLOT,NO_SLOT);

VIRTUALENTRY("HDM_FCP_TARG", TRUE, vdnTarget, &TargetParms[0], NAC_SLOT,NO_SLOT);

#if !defined(SCSI_TARGET_SERVER)
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES, &EchoScsiIsmParms[0], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES1, &EchoScsiIsmParms[1], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES2, &EchoScsiIsmParms[2], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES3, &EchoScsiIsmParms[3], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES4, &EchoScsiIsmParms[4], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES5, &EchoScsiIsmParms[5], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES6, &EchoScsiIsmParms[6], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES7, &EchoScsiIsmParms[7], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES8, &EchoScsiIsmParms[8], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES9, &EchoScsiIsmParms[9], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES10, &EchoScsiIsmParms[10], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES11, &EchoScsiIsmParms[11], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES12, &EchoScsiIsmParms[12], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES13, &EchoScsiIsmParms[13], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES14, &EchoScsiIsmParms[14], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES15, &EchoScsiIsmParms[15], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES16, &EchoScsiIsmParms[16], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES17, &EchoScsiIsmParms[17], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES18, &EchoScsiIsmParms[18], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES19, &EchoScsiIsmParms[19], NAC_SLOT,NO_SLOT);
#endif

#ifdef SCSI_TARGET_SERVER
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS0,  &ScsiServerParms[0],  NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS1,  &ScsiServerParms[1],  NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS2,  &ScsiServerParms[2],  NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS3,  &ScsiServerParms[3],  NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS4,  &ScsiServerParms[4],  NAC_SLOT,NO_SLOT);
#endif // SCSI_TARGET_SERVER

#if defined(RD_BUILD)
VIRTUALENTRY("HDM_RD", TRUE, vdnRD0, &RamDiskParms[0], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD1, &RamDiskParms[1], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD2, &RamDiskParms[2], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD3, &RamDiskParms[3], NAC_SLOT,NO_SLOT);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD4, &RamDiskParms[4], NAC_SLOT,NO_SLOT);
#endif // RD_BUILD

#if !defined(FCP_SINGLE_BUILD)

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

#define	LP0		(FCLOOPINSTANCE(NAC_SLOT, 0))
#define	LP1		(FCLOOPINSTANCE(NAC_SLOT, 1))
#define	LP2		(FCLOOPINSTANCE(NAC_SLOT, 2))

// Loop the Target runs on
#define	EX_INST	LP0

#if defined(SCSI_TARGET_SERVER)

U32		numExportRows = 5;

ExportTableEntry			Export[5] = 	// one for each VirtualCircuit
{
	// 2 regular disks and one Ram Disk for the RAM DISK BUILD
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS0, 0, 0, 0,  0, 2, EX_INST, "01", RAM_DISK_SIZE, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS1, 0, 0, 1,  0, 2, EX_INST, "02", RAM_DISK_SIZE, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS2, 0, 0, 2,  0, 2, EX_INST, "03", RAM_DISK_SIZE, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS3, 0, 0, 3,  0, 2, EX_INST, "04", RAM_DISK_SIZE, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC,  vdnSTS4, 0, 0, 4,  0, 2, EX_INST, "05", RAM_DISK_SIZE, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}}
};

#else

#if !defined(FCP_SINGLE_BUILD) 

U32		numExportRows = 20;

ExportTableEntry			Export[20] = 	// one for each VirtualCircuit
{
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES, 0, 0, 0,  -1, 2, EX_INST, "01", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES1, 0, 0, 1,  -1, 2, EX_INST, "02", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES2, 0, 0, 2,  -1, 2, EX_INST, "03", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES3, 0, 0, 3,  -1, 2, EX_INST, "04", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES4, 0, 0, 4,  -1, 2, EX_INST, "05", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES5, 0, 0, 5,  -1, 2, EX_INST, "06", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES6, 0, 0, 6,  -1, 2, EX_INST, "07", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES7, 0, 0, 7,  -1, 2, EX_INST, "08", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES8, 0, 0, 8,  -1, 2, EX_INST, "09", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES9, 0, 0, 9,  -1, 2, EX_INST, "10", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES10, 0, 0, 10, -1, 2, EX_INST, "11", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES11, 0, 0, 11, -1, 2, EX_INST, "12", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES12, 0, 0, 12, -1, 2, EX_INST, "13", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES13, 0, 0, 13, -1, 2, EX_INST, "14", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES14, 0, 0, 14, -1, 2, EX_INST, "15", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES15, 0, 0, 15, -1, 2, EX_INST, "16", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES16, 0, 0, 16, -1, 2, EX_INST, "17", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES17, 0, 0, 17, -1, 2, EX_INST, "18", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES18, 0, 0, 18, -1, 2, EX_INST, "19", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}},
	{{0,0}, EXPORT_TABLE_VERSION, SZ, {0}, PFC, vdnES19, 0, 0, 19, -1, 2, EX_INST, "20", 0, 0, StateOffLine, StateOffLine, " ", {0},{0},{0},{0}}
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
        0, NAC_SLOT
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

#endif
