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
 *	$Log: /Gemini/Odyssey/Nac/BuildSys.cpp $
// 
// 20    2/08/00 9:03p Tnelson
// Fixes for SYSTEMMASTER macros
// 
// 19    2/08/00 4:46p Szhang
// Added DdmPart. to the project and included in buildsys. Jim T.
// reviewed.
// 
// 18    1/27/00 4:50p Jaltmaier
// Profiling.
// 
// 17    1/11/00 8:42p Mpanas
// remove unused table refs
// 
//// 16    12/21/99 1:48p Mpanas
// Add support for IOP Failover
// - make several modules IOP_LOCAL
// 
// 15    12/03/99 7:22p Jtaylor
// Change gSize_small_heap to 20MB
// 
// 14    12/02/99 2:35p Vnguyen
// Add DdmReporter to image.
// 
// 13    11/19/99 7:47p Ewedel
// Removed PTS field def #includes (!).  These are now located in the
// CtTables library.
// 
// 12    10/14/99 2:34p Jlane
// Add CMB Ddm
// 
// 11    10/14/99 4:41a Iowa
// Iowa merge
// 
// 10    10/07/99 8:11p Mpanas
// First cut of BSA VD Create
// - need to add BsaConfig.cpp
// 
// 8     9/16/99 11:12p Jlane
// Update for new CHAOS
// 
// 7     9/15/99 11:31a Mpanas
// Fix build problem by adding the missing descriptors
// 
// 6     8/25/99 7:08p Jlane
// Added DdmLED.
// 
// 5     8/25/99 11:47a Jaltmaier
// gSizeReserved
// 
// 4     8/14/99 9:47p Mpanas
// Support for new LoopMonitor
// 
// 3     8/12/99 9:17p Mpanas
// Add Listen.h
// 
// 2     7/23/99 1:36p Mpanas
// include Raid Table definitions
// 
// 1     7/21/99 10:12p Mpanas
// First pass at a NAC Image
// 
 *
 *     07/21/99 Michael G. Panas: Created BuildSys for the NAC Image
 *
**/

#include "OsTypes.h"
#include "Odyssey.h"
#include "FC_loop.h"
#include "CTIdLun.h"

// Tables referenced
#include "ExportTable.h"
#include "LoopDescriptor.h"
#include "FCPortDatabaseTable.h"
#include "BsaConfig.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

// Size of all available memory for NAC boards
extern "C" U32 gSize_available_memory = 0x0C800000;		// 200MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap = 		0x01400000;		// 20MB
// Size of memory reserved for old-fashioned Nucleus memory allocation
extern "C" U32 gSizeReserved=0;

//=========================================================================
//#define	NO_TARGET			// define this if the Target code is not desired
//#define	NO_INIT				// define this if the Initiator is not desired

//#define	RAMDISK				// defined when an Image with Ram Disk is built.

							// NOTE: these flags will be defined in the
							// project prefix file as needed
//=========================================================================
//
//	Dummy methods needed

#include "HdmFcpTarget.h"

#ifdef NO_TARGET

VDN HdmFcpTarget::FCP_Find_Next_Virtual_Device(U32 key)
{	
	return(0);
	
}	// FCP_Find_Next_Virtual_Device

#endif


#include "BuildSys.h"

//=========================================================================
// Class Table
// This is the Class Table for the NAC Image
// 
// Literal names/StackSize/Queue Size/Class for all DDM classes

#include "DdmConsoleTest.h"
#include "EchoScsiIsm.h"
#include "HdmFcpInit.h"
#include "HdmFcpTargetInit.h"
#include "DriveMonitorIsm.h"
#include "RdIsm.h"
#include "BsaIsm.h"
#include "ScsiServerIsm.h"
#include "LoopMonitorIsm.h"

//
// DDM Classes
//
#define DEF_STK_SIZE	32768

// Parms - "ClassName", StackSize, QueueSize, LinkName

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER",   	DEF_STK_SIZE,1024, DdmManager);
CLASSENTRY("PTSLOADER", 	DEF_STK_SIZE,1024, DdmPtsLoader);
CLASSENTRY("DDMVIRTUALMGR",	DEF_STK_SIZE,1024, DdmVirtualManager);
CLASSENTRY("DDMTRANSPORT", 	DEF_STK_SIZE,1024, Transport);
CLASSENTRY("DDMTIMER",     	DEF_STK_SIZE,1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO",   	DEF_STK_SIZE,1024, DdmSysInfo);
CLASSENTRY("DDMQUIESCEMGR",	DEF_STK_SIZE,1024, DdmQuiesceManager);

// DDMs that run here
CLASSENTRY("DDM_CONS",          DEF_STK_SIZE, 1024, DdmConsoleTest );
CLASSENTRY("HDM_REPORTER",      DEF_STK_SIZE, 1024, DdmReporter );
CLASSENTRY("HDM_LOOP_MON",      DEF_STK_SIZE, 1024, LoopMonitorIsm );
CLASSENTRY("HDM_DRIVE_MON",     DEF_STK_SIZE, 1024, DriveMonitorIsm );
CLASSENTRY("HDM_FCP_INIT",      DEF_STK_SIZE, 1024, HdmFcpInit );
CLASSENTRY("HDM_FCP_TARGET",    DEF_STK_SIZE, 1024, HdmFcpTarget );
CLASSENTRY("HDM_FCP_TARG_INIT", DEF_STK_SIZE, 1024, HdmFcpTargetInit );
CLASSENTRY("HDM_STS",           DEF_STK_SIZE, 1024, ScsiServerIsm );
CLASSENTRY("HDM_BSA",           DEF_STK_SIZE, 1024, BsaIsm );
CLASSENTRY("HDM_RD",            DEF_STK_SIZE, 1024, RdIsm );
CLASSENTRY("HDM_RAID",          DEF_STK_SIZE, 1024, DdmRaid );
//CLASSENTRY("HDM_PART",          DEF_STK_SIZE, 1024, DdmPart );
CLASSENTRY("HDM_CMB",           DEF_STK_SIZE, 1024, CDdmCMB);
CLASSENTRY("DDM_LED",			DEF_STK_SIZE, 1024, DdmLED);
CLASSENTRY("DDM_STATUS",		DEF_STK_SIZE, 1024, DdmStatus);

CLASSENTRY("DDMPROFILE"	  ,10240,1024, DdmProfile);
CLASSENTRY("DDMLEAK"	  ,10240,1024, DdmLeak);
CLASSENTRY("DDMCPU"	  ,10240,1024, DdmCpu);

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
DEVICEENTRY("Transport",	Transport);
DEVICEENTRY("FailSafe", 	FailSafe);

DEVICEENTRY("Profile", 		Profile);
DEVICEENTRY("Leak", 		Leak);
DEVICEENTRY("Cpu",	 		CpuStatistics);

//=============================================================================
// System DDMs Table
//
SYSTEMENTRY("DDMMANAGER");		// Must be first
SYSTEMENTRY("DDMTIMER");
SYSTEMENTRY("DDMPROFILE");
SYSTEMENTRY("DDMLEAK");
SYSTEMENTRY("DDMCPU");
//SYSTEMENTRY("PTSLOADER");		// 
SYSTEMENTRY("DDMVIRTUALMGR");	// 
SYSTEMENTRY("DDM_CONS");
SYSTEMENTRY("HDM_REPORTER");
SYSTEMENTRY("HDM_CMB");
SYSTEMENTRY("DDM_LED");
SYSTEMENTRY("DDMSYSINFO");
SYSTEMENTRY("DDM_STATUS");
//SYSTEMENTRY("DDMVIRTUALMGR");	// 
SYSTEMENTRY("DDMTRANSPORT");
SYSTEMENTRY("DDMQUIESCEMGR");



//==============================================================================
// Virtual Device Numbers
//

// use this when building seperate images
// These numbers will eventually come from the persistant
// table service.
#include "SystemVdn.h"


//=============================================================================
// PTS Proxy and PTS
//
//PTSPROXY("PTSPROXY", vdnPTS, IOP_HBC0,IOP_HBC1);
//PTSNAME("PTS",	vdnPTS,	IOP_HBC0,	IOP_HBC1);
//VMSNAME("VMS",	vdnVMS,	IOP_HBC0,	IOP_HBC1);


//=========================================================================
// Boot Data
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
        262144, 512, 1048576, 65536, 0x02000000, 0x04000000,

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

BOOTENTRY("Transport",  &bddMessenger );
BOOTENTRY("Timer",      &bddTimer );


