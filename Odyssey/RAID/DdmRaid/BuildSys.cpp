/* BuildSys.cpp
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
 *
**/

#include "OsTypes.h"
#include "Odyssey.h"

// Include all field defs here
#include "ExportTable.cpp"
#include "DiskDescriptor.cpp"
#include "StorageRollCallTable.cpp"
#include "LoopDescriptor.cpp"

// Tables referenced
#include "ExportTable.h"
#include "DiskDescriptor.h"
#include "StorageRollCallTable.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"

#if defined(RAC_BUILD)
// Size of all available memory for actual hardware
extern "C" U32 gSize_available_memory = 0x0C800000;       // 200MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap = 0x00800000;             // 8MB

#else
// Size of all available memory for eval boards
extern "C" U32 gSize_available_memory = 0x00E00000;       // 14MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap = 0x00200000;             // 2MB
#endif

#define		RAM_DISK_SIZE	((1024*1024) / 512)		// 1Mb in blocks

#define	NEED_TRANSPORT	// turn on transport code
//#define	NEED_REPORTER	// turn on reporter code

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

#ifdef NONIC

#include "FcpData.h"
#include "FcpEvent.h"
// include dummy functions needed by the FCP Library
// these methods are used only by the Target DDM

extern "C" {
STATUS FCP_Message_Send_Request(FCP_EVENT_CONTEXT *p_context,
	FCP_EVENT_ACTION next_action);
STATUS FCP_Handle_AE_Target(FCP_EVENT_CONTEXT *p_context);
}


STATUS FCP_Message_Send_Request(FCP_EVENT_CONTEXT *p_context,
	FCP_EVENT_ACTION next_action)
{
	return(0);
}

STATUS FCP_Handle_AE_Target(FCP_EVENT_CONTEXT *p_context)
{
	return(0);
}
#endif

#ifdef NORAC

#include "FcpData.h"
#include "FcpEvent.h"
#include "FcpProto.h"
// include dummy functions needed by the FCP Library
// these methods are used only by the Initiator DDM

extern "C" {
STATUS FCP_Message_Send_Response(FCP_EVENT_CONTEXT *p_context,
	FCP_EVENT_ACTION next_action);
STATUS FCP_Handle_AE_Initiator(FCP_EVENT_CONTEXT *p_context);
}


STATUS FCP_Message_Send_Response(FCP_EVENT_CONTEXT *p_context,
	FCP_EVENT_ACTION next_action)
{
	return(0);
}

STATUS FCP_Handle_AE_Initiator(FCP_EVENT_CONTEXT *p_context)
{
	return(0);
}

void FCP_Send_Enable_Reply(PINSTANCE_DATA Id)
{
}
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
#include "HdmFcpNic.h"
#include "HdmFcpRac.h"
#include "EchoScsiIsm.h"
#include "DriveMonitorIsm.h"
#ifdef SCSI_TARGET_SERVER
#include "BsaIsm.h"
#include "ScsiServerIsm.h"
#endif

CLASSENTRY("HDM_DMGR", 10240, 1024, DdmManager );
#if defined(NEED_TRANSPORT)
CLASSENTRY("HDM_TPT",  10240, 1024, Transport );
#endif
CLASSENTRY("HDM_TIMR",  10240, 1024, DdmTimer );

CLASSENTRY("VIRTUALPROXY",  10240, 1024, DdmVirtualProxy );

#if !defined(NIC_TEST_BUILD)
CLASSENTRY("PTS",  10240, 1024, DdmPTS );
CLASSENTRY("PTSPROXY",  10240, 1024, DdmPtsProxy );
#if !defined(RAC_BUILD) && !defined(FCP_SINGLE_BUILD)
CLASSENTRY("PTSDEFAULT",  10240, 1024, DdmPtsDefault );
#endif
#endif

#ifndef NORAC
CLASSENTRY("HDM_FCP_RAC",  10240, 1024, HdmFcpRac );
#endif
#ifndef NONIC
CLASSENTRY("HDM_FCP_NIC",  10240, 1024, HdmFcpNic );

#if !defined(SCSI_TARGET_SERVER) && !defined(NORAC) || defined(NIC_TEST_BUILD)
CLASSENTRY("HDM_ECHO_SCSI",  10240, 1024, EchoScsiIsm );
#endif
#endif
#if !defined(NIC_TEST_BUILD)
CLASSENTRY("HDM_DRIVE_MON",  10240, 1024, DriveMonitorIsm );
#endif
#ifdef SCSI_TARGET_SERVER
CLASSENTRY("HDM_BSA",  10240, 1024, BsaIsm );
CLASSENTRY("HDM_STS",  10240, 1024, ScsiServerIsm );
CLASSENTRY("DDM_RAID", 10240, 1024, DdmRaid );
CLASSENTRY("DDM_RMSTR", 10240,1024, DdmRAIDMstr);
CLASSENTRY("DDM_RMSTR_TEST", 10240,1024, DdmRAIDMstrTest);
#ifdef NEED_REPORTER
#include "DdmReporter.h"
CLASSENTRY("HDM_REPORT",  10240, 1024, DdmReporter );
#endif
#endif
#if defined(RD_BUILD)
#include "RdIsm.h"
CLASSENTRY("HDM_RD",  10240, 1024, RdIsm );
#endif
#include "LoopMonitorIsm.h"
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
#if defined(NEED_TRANSPORT)
DEVICEENTRY("Transport",	Transport);		// may need these later
#endif
//DEVICEENTRY("FailSafe", 	FailSafe);


//=============================================================================
// System DDMs Table
//
SYSTEMENTRY("HDM_DMGR");		// Must be first
SYSTEMENTRY("VIRTUALPROXY");	// second
SYSTEMENTRY("DDM_RMSTR" );	// Must be first!
SYSTEMENTRY("DDM_RMSTR_TEST" );	// Must be first!
#if !defined(RAC_BUILD) && !defined(FCP_SINGLE_BUILD)
SYSTEMENTRY("PTSDEFAULT");		// third
#endif
#if defined(NEED_TRANSPORT)
SYSTEMENTRY("HDM_TPT");
#endif
SYSTEMENTRY("HDM_TIMR");


//==============================================================================
// Virtual Device Numbers
//

#if defined(NIC_TEST_BUILD) || defined(RAC_TEST_BUILD)
// use this when building seperate images
// These numbers will eventually come from the persistant
// table service.
#include "SystemVdn.h"
#define	vdnES (vdnLAST+1)
#else

// These numbers are used for single board image builds
enum {
	vdnPTS = 1,
	vdnTS,
	
#ifndef NONIC
	vdnNIC,		// FCP NIC DDM
#if !defined(SCSI_TARGET_SERVER) && !defined(NORAC)
	vdnES,		// EchoScsi ISM
#endif
#endif
	vdnRAC,		// FCP RAC DDM  (must come after NIC!)
	vdnDM,		// Drive Monitor ISM
	
#ifdef SCSI_TARGET_SERVER
	vdnSTS0,	// SCSI target Server 0
	vdnSTS1,	// SCSI target Server 1
	vdnSTS2,	// SCSI target Server 2
	vdnSTS3,	// SCSI target Server 3
	vdnSTS4,	// SCSI target Server 4
	vdnSTS5,	// SCSI target Server 5
	vdnSTS6,	// SCSI target Server 6
	vdnSTS7,	// SCSI target Server 7

	vdnBSA0,	// BSA ISM 0
	vdnBSA1,	// BSA ISM 1
	vdnBSA2,	// BSA ISM 2
	vdnBSA3,	// BSA ISM 3
	vdnBSA4,	// BSA ISM 4
	vdnBSA5,	// BSA ISM 5
	vdnBSA6,	// BSA ISM 6
	vdnBSA7,	// BSA ISM 7
	vdnRAID1,	// RAID DDM
#endif
	vdnRD0,		// RAM Disk 0
	vdnRD1,		// RAM Disk 1
	vdnRD2,		// RAM Disk 2
	vdnRD3,		// RAM Disk 3
	vdnRD4,		// RAM Disk 4
	
	vdnReporter,
	vdnLoopMonitor,
	vdnLAST		// last entry in table
};
#endif

//=============================================================================
// PTS Proxy and PTS
//
PTSPROXY("PTSPROXY", vdnPTS, IOP_HBC0,IOP_HBC1);
PTSNAME("PTS", vdnTS, IOP_HBC0,IOP_HBC1);


//=========================================================================
// This section declares the tables for the dummy PersistentData service
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

#ifdef SCSI_TARGET_SERVER
TempConfiguration pddRaid1 =
{
	{
		vdnRAID1,				// arrayVDN
		0x010f59c7,				// totalCapacity
		0x010f59c7,				// memberCapacity
		128,					// dataBlockSize
		128,					// parityBlockSize
		RAID1,					// RaidLevel;
		RAID_FAULT_TOLERANT,	// Health;
		RAID_INIT_COMPLETE,		// InitStatus
		2,						// NumberMembers;
		0,						// NumberUtilities;

	},
	{				
		{
			vdnBSA0,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			0,					// MemberIndex;
			0x010f59c7,			// EndLBA;
		},
		{
			vdnBSA1,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			1,					// MemberIndex;
			0x010f59c7,			// EndLBA;
		},
		{
			vdnBSA2,			// MemberVd;
			RAID_STATUS_UP,		// MemberHealth;
			2,					// MemberIndex;
			0x010f59c7,			// EndLBA;
		},		
	},
};	
#endif

//#if !defined(NONIC) 

FCP_CONFIG	HdmFcpNicParms = {
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
	8,		// number of LUNs
	30,		// number of PCI window buffers
	
	// these values are changed by the startup code
	// based on the target build
	0x10000000,	// base ISP address
	0,			// interrupt number
	TARGET_INSTANCE,	// instance number
	vdnDM		// virtual device to throw events to
};
//#endif
	
FCP_CONFIG	HdmFcpRacParms = {
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
	0,		// number of LUNs
	0,		// number of PCI window buffers
	
	// these values are changed by the startup code
	// based on the target build
	0x10002000,	// base ISP address
	0,			// interrupt number
	INITIATOR_INSTANCE,	// instance number
	vdnDM		// virtual device to throw events to
};

struct _EchoScsiParms {
	U32	Parms[20];
	VDN	vdnNext; 
} EchoScsiIsmParms = {
	// use this table to translate the LUN coming in to a
	// Target Id going out
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19},
	vdnRAC
	};

#include "LmConfig.h"

LM_CONFIG	LoopMonParms[1] = { 
	LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
	2,							// number of loops to scan
	0,							// flags
	0							// FC_MASTER VDN
};

#include "DmConfig.h"

DM_CONFIG	DriveMonParms[1] = { 
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	20,							// number of drives to scan
	0,							// DM Flags
	vdnRAC,
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
	};

#ifdef SCSI_TARGET_SERVER

#include "BsaIsm.h"

BSA_CONFIG BsaParms[] = {
// 	  Version               size               LUN ID  Init ID SMART
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  0, vdnRAC, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  1, vdnRAC, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  2, vdnRAC, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  3, vdnRAC, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  4, vdnRAC, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  5, vdnRAC, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  6, vdnRAC, 0	},
	{ BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  7, vdnRAC, 0	},
};

#include "StsConfig.h"

STS_CONFIG	ScsiServerParms[] = 
	{ 
// 	  Version             size                vdnNext
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRAID1 },
//	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA0 },
//	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA1 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA2 },
#if defined(RD_BUILD)
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD0 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD1 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD2 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD3 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnRD4 }
#else
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA3 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA4 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA5 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA6 },
	{ STS_CONFIG_VERSION, sizeof(STS_CONFIG), vdnBSA7 }
#endif
};
#endif

// RAM Disk config data
U32		RamDiskParms[] =
			 {
			 RAM_DISK_SIZE,		// RAM DISK 0
			 RAM_DISK_SIZE,		// RAM DISK 1
			 RAM_DISK_SIZE,		// RAM DISK 2
			 RAM_DISK_SIZE,		// RAM DISK 3
			 RAM_DISK_SIZE,		// RAM DISK 4
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

#define	NO_SLOT		(TySlot)(-1)

#if defined(NIC_TEST_BUILD) || defined(RAC_TEST_BUILD)
// use these for multiple images (all must be the same)
VIRTUALENTRY("HDM_FCP_NIC", TRUE, vdnNIC, &HdmFcpNicParms, IOP_HBC1,NO_SLOT);
VIRTUALENTRY("HDM_FCP_RAC", TRUE, vdnRAC, &HdmFcpRacParms, IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES, &EchoScsiIsmParms, IOP_HBC1,NO_SLOT);
VIRTUALENTRY("HDM_DRIVE_MON", TRUE, vdnDM, &DriveMonParms[0], IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_LOOP_MON", TRUE, vdnLoopMonitor, &LoopMonParms[0], IOP_HBC0,IOP_HBC1);
//VIRTUALENTRY("HDM_REPORT", TRUE, vdnReporter, &pddNone, IOP_HBC0,NO_SLOT);

#ifdef SCSI_TARGET_SERVER
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS0,  &ScsiServerParms[0],  IOP_HBC1,NO_SLOT);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS1,  &ScsiServerParms[1],  IOP_HBC1,NO_SLOT);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS2,  &ScsiServerParms[2],  IOP_HBC1,NO_SLOT);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS3,  &ScsiServerParms[3],  IOP_HBC1,NO_SLOT);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS4,  &ScsiServerParms[4],  IOP_HBC1,NO_SLOT);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS5,  &ScsiServerParms[5],  IOP_HBC1,NO_SLOT);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS6,  &ScsiServerParms[6],  IOP_HBC1,NO_SLOT);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS7,  &ScsiServerParms[7],  IOP_HBC1,NO_SLOT);

VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA0,  &BsaParms[0],  IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA1,  &BsaParms[1],  IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA2,  &BsaParms[2],  IOP_HBC0,NO_SLOT);
#if !defined(RD_BUILD)
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA3,  &BsaParms[3],  IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA4,  &BsaParms[4],  IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA5,  &BsaParms[5],  IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA6,  &BsaParms[6],  IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA7,  &BsaParms[7],  IOP_HBC0,NO_SLOT);
#endif
#endif

#if defined(RD_BUILD)
VIRTUALENTRY("HDM_RD", TRUE, vdnRD0, &RamDiskParms[0], IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD1, &RamDiskParms[1], IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD2, &RamDiskParms[2], IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD3, &RamDiskParms[3], IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD4, &RamDiskParms[4], IOP_HBC0,NO_SLOT);
#endif

#else
// use these for single board images
#ifndef NORAC
VIRTUALENTRY("HDM_FCP_RAC", TRUE, vdnRAC, &HdmFcpRacParms, IOP_HBC0,IOP_HBC1);
#endif
#ifndef NONIC
VIRTUALENTRY("HDM_FCP_NIC", TRUE, vdnNIC, &HdmFcpNicParms, IOP_HBC0,IOP_HBC1);
#if !defined(SCSI_TARGET_SERVER) && !defined(NORAC)
VIRTUALENTRY("HDM_ECHO_SCSI", TRUE, vdnES, &EchoScsiIsmParms, IOP_HBC0,IOP_HBC1);
#endif
#endif
VIRTUALENTRY("HDM_DRIVE_MON", TRUE, vdnDM, &DriveMonParms[0], IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_LOOP_MON", TRUE, vdnLoopMonitor, &LoopMonParms[0], IOP_HBC0,IOP_HBC1);

#ifdef SCSI_TARGET_SERVER
#ifdef NEED_REPORTER
VIRTUALENTRY("HDM_REPORT", TRUE, vdnReporter, &pddNone, IOP_HBC0,NO_SLOT);
#endif

VIRTUALENTRY("HDM_STS", TRUE, vdnSTS0,  &ScsiServerParms[0],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS1,  &ScsiServerParms[1],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS2,  &ScsiServerParms[2],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS3,  &ScsiServerParms[3],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS4,  &ScsiServerParms[4],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS5,  &ScsiServerParms[5],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS6,  &ScsiServerParms[6],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_STS", TRUE, vdnSTS7,  &ScsiServerParms[7],  IOP_HBC0,IOP_HBC1);

VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA0,  &BsaParms[0],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA1,  &BsaParms[1],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA2,  &BsaParms[2],  IOP_HBC0,IOP_HBC1);
#if !defined(RD_BUILD)
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA3,  &BsaParms[3],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA4,  &BsaParms[4],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA5,  &BsaParms[5],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA6,  &BsaParms[6],  IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_BSA", TRUE, vdnBSA7,  &BsaParms[7],  IOP_HBC0,IOP_HBC1);
#endif
VIRTUALENTRY("DDM_RAID", TRUE, vdnRAID1, &pddRaid1, IOP_HBC0,IOP_HBC1);
#endif

#if defined(RD_BUILD)
VIRTUALENTRY("HDM_RD", TRUE, vdnRD0, &RamDiskParms[0], IOP_HBC0,IOP_HBC1);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD1, &RamDiskParms[1], IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD2, &RamDiskParms[2], IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD3, &RamDiskParms[3], IOP_HBC0,NO_SLOT);
VIRTUALENTRY("HDM_RD", TRUE, vdnRD4, &RamDiskParms[4], IOP_HBC0,NO_SLOT);
#endif

#endif


//===========================================================================
/*
typedef struct DiskDescriptor {
	rowID			ridThisRow;				// rowID of this table row.
	U32 			version;				// Version of Disk Object descriptor record.
	U32				size;					// Size of Disk Object descriptor record in bytes.
	U32				FCInstance;				// FC Loop number (index in LoopDescriptor table)
	U32				SlotID;					// physical slot of drive
	U32				FCTargetID;				// actual FCP target ID.
	VDN				vdBSADdm;				// BSA Virtual Device number for this disk
	String32		SerialNumber;			// Device serial number	
	U32				CurrentStatus;			// see DriveStatus enum above.
	U64				Capacity;				// Disk formatted capacity in bytes.
	INQUIRY			InqData;				// read from drive. 43 bytes defined in scsi.h
	rowID			rid_status_record;		// rowID of the status record for this device.
	rowID			rid_performance_record;	// rowID of the performance record for this device.
	} DiskDescriptor, *pDiskDescriptor;
*/

// Dummy Table Data for test
// this table will not be persistant
#ifdef SCSI_TARGET_SERVER

DiskDescriptor	DiskDesc[8] = 	// one for each slot
{
	{ {0,0},0,0, 0, 0,  0,  vdnBSA0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 1,  1,  vdnBSA1, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 2,  2,  vdnBSA2, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 3,  3,  vdnBSA3, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 4,  4,  vdnBSA4, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 5,  5,  vdnBSA5, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 6,  6,  vdnBSA6, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 7,  7,  vdnBSA7, 0, DriveInvalid, 0,0,0,0,0},
};

#else

DiskDescriptor	DiskDesc[MAX_DRIVES] = 	// one for each slot
{
	{ {0,0},0,0, 0, 0,  0,  0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 1,  1,  0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 2,  2,  0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 3,  3,  0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 4,  4,  0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 5,  5,  0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 6,  6,  0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 7,  7,  0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 8,  8,  0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 9,  9,  0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 10, 10, 0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 11, 11, 0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 12, 12, 0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 13, 13, 0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 14, 14, 0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 15, 15, 0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 16, 16, 0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 17, 17, 0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 18, 18, 0, 0, DriveInvalid, 0,0,0,0,0},
	{ {0,0},0,0, 0, 19, 19, 0, 0, DriveInvalid, 0,0,0,0,0}
};

#endif

//===========================================================================
/*
struct ExportTableEntry {
	rowID			ridThisRow;			// rowID of this table row.
	U32 			version;			// Version of Export Table record.
	U32				size;				// Size of Export Table record in bytes.
	CTProtocolType	ProtocolType;		// FCP, IP, other
	U32				CircuitNumber;		// LUN or other
	VDN				vdNext;				// First Virtual Device number in the chain
	VDN				vdLegacyBsa;		// Virtual Device number of the legacy BSA
	VDN				vdLegacyScsi;		// Virtual Device number of the legacy SCSI
	U32				ExportedLUN;		// LUN number exported
	U32				InitiatorId;		// Host ID
	U32				TargetId;			// our ID
	U32				FCInstance;			// FC Loop number
	String32		SerialNumber;		// Use a string array for Serial Number
	long long		Capacity;			// Capacity of this Virtual Circuit
	U32				FailState;
	U32				PrimaryFCTargetOwner;
	U32				SecondaryFCTargetOwner;
	CTReadyState	ReadyState;			// Current state
	CTReadyState	DesiredReadyState;	// Desired Ready state
	String16		WWNName;			// World Wide Name (first 8 bytes used)
	String32		Name;				// Virtual Circuit Name
	};
*/

#if defined(SCSI_TARGET_SERVER) || defined(NIC_TEST_BUILD) || defined(RAC_TEST_BUILD)

U32		numExportRows = 8;

ExportTableEntry			Export[8] = 	// one for each VirtualCircuit
{
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  0, vdnSTS0, 0, 0, 0, -1, 0, 0, "00", 0x010f59c7, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 0"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  1, vdnSTS1, 0, 0, 1, -1, 0, 0, "01", 0x010f59c7, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 1"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  2, vdnSTS2, 0, 0, 2, -1, 0, 0, "02", 0x010f59c7, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 2"},
#if defined(RD_BUILD)
	// 2 regular disks and one Ram Disk for the RAM DISK BUILD
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  2, vdnSTS3, 0, 0, 3,  0, 0, 0, "03", RAM_DISK_SIZE, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 3"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  2, vdnSTS4, 0, 0, 4,  0, 0, 0, "04", RAM_DISK_SIZE, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 4"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  2, vdnSTS5, 0, 0, 5,  0, 0, 0, "05", RAM_DISK_SIZE, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 5"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  2, vdnSTS6, 0, 0, 6,  0, 0, 0, "06", RAM_DISK_SIZE, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 6"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  2, vdnSTS7, 0, 0, 7,  0, 0, 0, "07", RAM_DISK_SIZE, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 7"},
#else
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  3, vdnSTS3, 0, 0, 3,  0, 0, 0, "03", 0x010f59c7, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 3"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  4, vdnSTS4, 0, 0, 4,  0, 0, 0, "04", 0x010f59c7, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 4"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  5, vdnSTS5, 0, 0, 5,  0, 0, 0, "05", 0x010f59c7, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 5"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  6, vdnSTS6, 0, 0, 6,  0, 0, 0, "06", 0x010f59c7, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 6"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  7, vdnSTS7, 0, 0, 7,  0, 0, 0, "07", 0x010f59c7, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 7"},
#endif
};

#else

#if !defined(FCP_SINGLE_BUILD) && !defined(NONIC)

U32		numExportRows = 20;

ExportTableEntry			Export[20] = 	// one for each VirtualCircuit
{
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,   0, vdnES, 0, 0, 0, -1, 0, 0, "01", 0, 0, 0, 0, StateInvalid, StateReady, " ",   "LUN 0"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,   1, vdnES, 0, 0, 1, -1, 0, 0, "02", 0, 0, 0, 0, StateInvalid, StateReady, " ",   "LUN 1"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,   2, vdnES, 0, 0, 2, -1, 0, 0, "03", 0, 0, 0, 0, StateInvalid, StateReady, " ",   "LUN 2"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,   3, vdnES, 0, 0, 3, -1, 0, 0, "04", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 3"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,   4, vdnES, 0, 0, 4, -1, 0, 0, "05", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 4"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,   5, vdnES, 0, 0, 5, -1, 0, 0, "06", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 5"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,   6, vdnES, 0, 0, 6, -1, 0, 0, "07", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 6"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,   7, vdnES, 0, 0, 7, -1, 0, 0, "08", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 7"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,   8, vdnES, 0, 0, 8, -1, 0, 0, "09", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 8"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,   9, vdnES, 0, 0, 9, -1, 0, 0, "10", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 9"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  10, vdnES, 0, 0, 10, -1, 0, 0, "11", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 10"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  11, vdnES, 0, 0, 11, -1, 0, 0, "12", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 11"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  12, vdnES, 0, 0, 12, -1, 0, 0, "13", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 12"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  13, vdnES, 0, 0, 13, -1, 0, 0, "14", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 13"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  14, vdnES, 0, 0, 14, -1, 0, 0, "15", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 14"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  15, vdnES, 0, 0, 15, -1, 0, 0, "16", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 15"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  16, vdnES, 0, 0, 16, -1, 0, 0, "17", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 16"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  17, vdnES, 0, 0, 17, -1, 0, 0, "18", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 17"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  18, vdnES, 0, 0, 18, -1, 0, 0, "19", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 18"},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), ProtocolFibreChannel,  19, vdnES, 0, 0, 19, -1, 0, 0, "20", 0, 0, 0, 0, StateInvalid, StateReady, " ", "LUN 19"}
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


