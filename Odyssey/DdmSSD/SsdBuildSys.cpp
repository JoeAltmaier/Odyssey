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
//	$Log: /Gemini/Odyssey/DdmSSD/SsdBuildSys.cpp $
// 
// 17    2/09/00 5:11p Tnelson
// Update BuildSys to match latest OS changes.  Removes PTSNAME and
// VMSNAME macros.  Remove DdmPtsLoader from IOPs.
// 
// 16    11/09/99 12:10p Hdo
// Rearrange deviceentries and systementries
// 
// 15    10/25/99 4:26p Hdo
// Add a deviceenrty for theTransport_Pci
// 
// 11    9/06/99 10:56p Jfrandeen
// new cache
// 
// 10    8/25/99 11:36a Jaltmaier
// gSizeReserved
// 
// 9     8/24/99 9:50a Jfrandeen
// Use PTS
// 
// 8     7/20/99 11:05a Jfrandeen
// Remove obsolete includes
// 
// 7     7/06/99 3:45p Jfrandeen
// New buildsys
// 
// 6     6/01/99 2:35p Jfrandeen
// Fix RTTI
// 
// 5     5/17/99 8:34a Jfrandeen
// 
// 4     5/17/99 7:27a Jfrandeen
// RAIDParms pddNone
// 
// 3     5/12/99 9:21p Jfrandeen
// 
// 2     5/12/99 10:17a Jfrandeen
// Reset ISR
// 
// 4     5/11/99 2:01p Mpanas
// cleanup version
// 
// 2     4/26/99 4:37p Mpanas
// match other checked in versions of
// systemVdn
// 
// 1     3/30/99 8:22p Mpanas
// Initial Checkin of the RAC Image
 *
 *     03/26/99 Michael G. Panas: Created BuildSys for the RAC Image
 *
**/

#include "OsTypes.h"
#include "Odyssey.h"

// Include all field defs here
#include "ExportTable.cpp"
#include "DiskDescriptor.cpp"
#include "StorageRollCallTable.cpp"

// Tables referenced
#include "ExportTable.h"
#include "DiskDescriptor.h"
#include "StorageRollCallTable.h"

//#include "ReadTable.h"
//#include "Table.h"
//#include "Rows.h"
//#include "Listen.h"

// Size of all available memory for RAC boards
extern "C" U32 gSize_available_memory = 0x0C800000;       // 200MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap = 		0x02000000;       // 32MB
// Size of memory reserved for old-fashioned Nucleus memory allocation
extern "C" U32 gSizeReserved=0;

//=========================================================================
//#define	NO_TARGET			// define this if the Target code is not desired
//#define	NO_INIT				// define this if the Initiator is not desired

//#define	RAMDISK				// defined when an Image with Ram Disk is built.

							// NOTE: these flags will be defined in the
							// project prefix file as needed
//=========================================================================

#include "BuildSys.h"

//=========================================================================
// Class Table
// This is the Class Table for the SSD Image
// 
// Literal names/StackSize/Queue Size/Class for all DDM classes

//#include "EchoScsiIsm.h"
//#include "DriveMonitorIsm.h"
//#include "RdIsm.h"
//#include "BsaIsm.h"
//#include "ScsiServerIsm.h"

//
// DDM Classes
//

#define DEF_STK_SIZE	32768

// Parms - "ClassName", StackSize, QueueSize, LinkName

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER",	DEF_STK_SIZE, 1024, DdmManager);
//CLASSENTRY("PTSLOADER", 	DEF_STK_SIZE, 1024, DdmPtsLoader);
CLASSENTRY("VIRTUALMANAGER",DEF_STK_SIZE, 1024, DdmVirtualManager);
CLASSENTRY("DDMTRANSPORT",	DEF_STK_SIZE, 1024, Transport);
CLASSENTRY("DDMTIMER",		DEF_STK_SIZE, 1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO",	DEF_STK_SIZE, 1024, DdmSysInfo);
CLASSENTRY("HDM_CMB",		DEF_STK_SIZE, 1024, CDdmCMB);
CLASSENTRY("DDM_LED",		DEF_STK_SIZE, 1024, DdmLED);

// DDMs that run here
CLASSENTRY("HDM_SSD",		DEF_STK_SIZE, 1024, SSD_Ddm );
CLASSENTRY("HDM_FM",		DEF_STK_SIZE, 1024, FlashMonitorIsm );

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


//=============================================================================
// System DDMs Table
//
SYSTEMENTRY("DDMMANAGER");		// Must be first
//SYSTEMENTRY("PTSLOADER");		// 
SYSTEMENTRY("HDM_CMB");
SYSTEMENTRY("DDM_LED");
SYSTEMENTRY("DDMTIMER");
SYSTEMENTRY("DDMSYSINFO");
SYSTEMENTRY("VIRTUALMANAGER");
SYSTEMENTRY("DDMTRANSPORT");


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

//PTSNAME("PTS", vdnPTS, IOP_HBC0, IOP_HBC1);
//VMSNAME("VMS", vdnVMS, IOP_HBC0, IOP_HBC1);

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
	262144, 512, 1048536 /*524287*/, 8192, 0x2000000, 0x04000000,

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

//struct {int iCabinet; TySlot iSlot;} bddDdmManager = {
//	0, IOP_HBC0	// This is used only on the Eval Systems
//};

struct {U32 divisor;}  bddTimer={10000};

//BOOTENTRY("DdmManager",	&bddDdmManager );
BOOTENTRY("Transport",  &bddMessenger );
BOOTENTRY("Timer",      &bddTimer );

