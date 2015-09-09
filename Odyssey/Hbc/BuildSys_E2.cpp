///////////////////////////////////////////////////////////////////////
// BuildSys_E2.cpp -- System Tables for the Gemini Rev E2
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
// Revision History:
//
// $Log: /Gemini/Odyssey/Hbc/BuildSys_E2.cpp $
// 
// 59    2/11/00 1:52p Rbraun
// Added Neptune test suite entries
// Made DdmNet a SYSTEMMASTER
// 
// 58    2/09/00 5:05p Tnelson
// Removed useless include to old "PersistentData.h" file which has been
// deleted from Chaos.
// 
// 57    2/08/00 9:03p Tnelson
// Fixes for SYSTEMMASTER macro
// 
// 56    2/08/00 4:24p Jlane
// Make small heap size global work with debugging heap by setting to
// 22Mb.
// 
// 55    2/02/00 5:39p Jlane
// Make flags for RAC ports just like others as part of changes to remove
// internal drives.
// 
// 54    1/31/00 4:37p Joehler
// Changes made by Jerry, Jaymie and Bob to get Quiesce IOP to work.
// 
// 53    1/27/00 4:49p Jaltmaier
// Profiling.
// 
// 52    1/26/00 2:51p Joehler
// Uncomment upgrade master and file system master.
// 
// 51    1/14/00 6:02p Mpanas
// - Add entries for secondary DriveMonitors, these get enabled when
//   3 or 4 NACs are enabled
// - Cleanup file move all PRECONFIG data to end of file and frame
//   with #ifdef
// - delete unused RAID config stuff
// 
// 50    1/12/00 1:53p Jfrandeen
// Comment out SSD; otherwise, everyone must have an SSD board.
// 
// 49    1/12/00 1:43p Jfrandeen
// Fix SSD slot
// 
// 48    1/12/00 11:00a Jfrandeen
// Move SSD from IOP_APP1 to IOP_SSD_U2
// 
// 47    1/05/00 5:58p Dpatel
// added partition mstr
// 
// 46    1/03/00 6:54p Jlane
// Comment out Asserting UpgradeMaster & its FileSysMaster cohort.
// 
// 45    12/23/99 7:25p Mpanas
// Add flags to enable preconfigured Virtual Circuits and
// 3 or 4 NAC configurations.
// PRECONFIG
// ENABLE_3_NACS
// ENABLE_4_NACS
// Just uncomment the ones you need
// 
// 44    12/23/99 3:17p Legger
// Fixed DriveMon redundancy problem. Jerry did it.
// 
// 43    12/23/99 12:42p Jlane
// Comment out virtual entry macros for stuff in three NAC 16,24,28
// config.  Note that Virtual Entries for IOPs not present result in the
// hang with IOPs in state 7.
// 
// 42    12/21/99 2:36p Mpanas
// Add support for IOP Failover
// - make several modules IOP_LOCAL
// 
// 41    12/17/99 8:15a Jyeaman
// Added CLASSENTRY and SYSTEMENTRY for DdmSnmp.

// 40    12/16/99 2:56a Dpatel
// [ewx]  Added Hot Swap Master to build.
//
// 39    12/13/99 2:49p Vnguyen
// Add EnvironmentDdm.
//
// 38    12/09/99 3:56p Joehler
// Added DdmConsoleHbc to buildsys
//
// 37    12/09/99 1:50a Iowa
//
// 36    11/19/99 7:51p Ewedel
// Removed PTS field def #includes (!).  These are now obtained via the
// CtTables library.
//
// 35    11/18/99 1:15p Joehler
// Added Upgrade Master and Filesys Master to build
//
// 34    11/11/99 2:57p Bbutler
// Added flag to enable/disable call to FF_Create on first HBC use.
//
// 33    11/06/99 4:13p Vnguyen
// Add DdmReporter to build image.
//
// 32    11/05/99 4:24p Mpanas
// Default should be slot 25, 26 and 27 turned off
//
// 31    11/05/99 1:26p Mpanas
// -New DriveMonitor flags allow scanning all devices on
// non-DDH ports
// -Add compile time support for 4 NACs (slots 24, 25, 26, 27)
// Turn on VIRTUAL_ENTRY section for slots desired
//
// 30    10/28/99 7:18p Sgavarre
// ReArranged system entries per Tom N's suggestion.
//
// 29    10/27/99 12:21p Hdo
// Add new cache config records.
//
// 28    10/27/99 11:08a Jlane
// Add DdmPartitionMgr and ChaosFile for persistance support.
//
// 27    10/14/99 4:34a Iowa
// Iowa merge
//
// 26    10/09/99 11:48a Jlane
// Comment out DdmPTSDefault (who loaded up the export table).  Comment
// out default circuit element VIRTUALENTRY macros.  Re-Add SYSTEMENTRY
// macro for DDM_VCM.
//
// 25    10/05/99 7:19p Jlane
// Add in checkin data overwritten by previous checkin
//
// 24    10/04/99 6:10p Hdo
// Add SSD_CONFIG record
// Add a virtualentry for the SSD board
//
// 23    10/04/99 3:39p Agusev
// Decreased small heap to 32Mb
//
// 22    10/03/99 3:59p Agusev
// Bumped up the size of small hep to 64K
//
// 21    10/01/99 5:16p Agusev
// Rolled in the VCM
//
// 18    9/16/99 11:13p Jlane
// Update for new CHAOS with Virtual Manager.
//
// 17    9/15/99 4:31p Mpanas
// Add DM_FLAGS_SPIN_4X flag to all DM_CONFIG records
//
// 16    9/13/99 1:57p Agusev
// Added Raid Master to the system image
//
// 15    9/10/99 6:02p Agusev
// Merged the  SSAPI into the HBC image
//
// 14    9/09/99 4:39p Agusev
// Added more stuff for SSAPI support( still commented out, though)
//
// 13    9/07/99 9:56p Agusev
// Temporarily comment out SSAPI SYSTEMENTRY macros.
//
// 12    9/07/99 5:21p Agusev
// Added SSAPI CLASSENTRY, DEVBICEENTRY and SYSTEMENTRY Macros.
//
// 11    9/06/99 4:09p Jlane
// Added CLASSENTRY and SYSTEMENTRY macros PeekNPoke and its serial Ddm.
//
// 10    9/03/99 10:50a Jlane
// Configured a 10MB mirror for Build.01 functionality.
//
// 9     9/02/99 5:27p Jlane
// Rearrange LUNs so RAM disks appear before disks.
//
// 8     9/01/99 8:05p Iowa
//
// 7     8/25/99 7:07p Jlane
// Added DdmLED.
//
// 6     8/25/99 1:21p Egroff
// Changes to accomodate new record formats.
//
// 5     8/25/99 11:42a Jaltmaier
// gSizeReserved
//
// 4     8/15/99 12:10p Jlane
// Removed Redundancy Mgr.
//
// 3     8/13/99 7:25p Jlane
// Made STS_CONFIG match new structure def'n.
//
// 2     7/29/99 8:00p Mpanas
// Enable CMB Boot
//
// 1     7/27/99 9:08p Mpanas
// New Rev E2 build files
//
//
//  11/14/98	Tom Nelson: Created
//	03/01/99	JFL	Adapted for HBCImage.
//	07/27/99	MGP	Made from BuildSys._Odyssey.cpp for the HBC
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
#include "DdmPartitionMgr.h"

// Tables referenced
#include "ExportTable.h"
#include "DiskDescriptor.h"
#include "StorageRollCallTable.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"

// Turn this on for 3 ports, off for 2 ports
#define	NAC_E2

// Turn this on to enable pre-configured virtual circuits on slot 24 only
//#define	PRECONFIG

// Turn this flag on for four NAC support
// Slots enabled (24), 28, 16 (one failover pair,  all three on seperate buses)
//#define	ENABLE_3_NACS

// Turn this flag on for four NAC support
// Slots enabled (24), 28, 16, 20 (Two failover pairs  all four on seperate buses)
//#define	ENABLE_4_NACS

#if defined(ENABLE_3_NACS) || defined(ENABLE_4_NACS)
#define	REDUNDANT
#endif

// Size of all available memory for actual hardware
extern "C" U32 gSize_available_memory = 0x0C800000;       // 200MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap = 0x01600000;             // 22MB
// Size of memory reserved for old-fashioned Nucleus memory allocation
extern "C" U32 gSizeReserved=0;


//
// DDM Classes
//
// Parms - "ClassName", StackSize, QueueSize, LinkName
//

#define DEF_STK_SIZE	32768

FAILENTRY("Kernel",	Kernel);
FAILENTRY("Interrupt", 	Interrupt);
FAILENTRY("Dma", 		Dma);
FAILENTRY("Pci", 		HeapPci);
FAILENTRY("Net", 		Network);

/// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER",   DEF_STK_SIZE,1024, DdmManager);
CLASSENTRY("PTSLOADER",    DEF_STK_SIZE,1024,DdmPtsLoader);	// Loads Real Pts.
CLASSENTRY("VIRTUALMANAGER", DEF_STK_SIZE,1024, DdmVirtualManager);
CLASSENTRY("VMS",			DEF_STK_SIZE,1024, DdmVirtualMaster);
CLASSENTRY("DDMTRANSPORT",	DEF_STK_SIZE,1024, Transport);
CLASSENTRY("DDMTIMER",		DEF_STK_SIZE,1024, DdmTimer);
CLASSENTRY("DDMSYSINFO",	DEF_STK_SIZE,1024, DdmSysInfo);
#if defined(PRECONFIG)
CLASSENTRY("PTSDEFAULT",	DEF_STK_SIZE, 1024, DdmPtsDefault );	// Populates the Export Table
#endif

// CHAOS Ddm Classes (HBC Only)
CLASSENTRY("HDM_PTS", DEF_STK_SIZE, 1024, DdmPTS);			// The real PTS

// Customize the HBC image here:
//CLASSENTRY("DDM_RedundancyMgr", DEF_STK_SIZE, 1024, DdmRedundancyMgr);
//CLASSENTRY("DDM_Registrar", DEF_STK_SIZE, 1024, DdmRegistrar);
CLASSENTRY("HDM_REPORTER",      DEF_STK_SIZE, 1024, DdmReporter );
CLASSENTRY("CDdmCMB", DEF_STK_SIZE, 1024, CDdmCMB);
CLASSENTRY("DDM_ENV", DEF_STK_SIZE, 1024, EnvironmentDdm);
CLASSENTRY("DDM_BootMgr", DEF_STK_SIZE, 1024, DdmBootMgr);
CLASSENTRY("DDM_LED", DEF_STK_SIZE, 1024, DdmLED);
CLASSENTRY("DDM_NET_MGR", DEF_STK_SIZE, 1024, DdmNet);
CLASSENTRY("DDM_NET_MASTER", DEF_STK_SIZE, 1024, Network);
CLASSENTRY("HDM_SSERV",DEF_STK_SIZE,1024,DdmSocketServer);
CLASSENTRY("HDM_SSAPI",DEF_STK_SIZE,1024,DdmSSAPI);
CLASSENTRY("DDM_LOGMASTER",  DEF_STK_SIZE,1024,DdmLogMaster);
CLASSENTRY("DDM_IOPLOG",  DEF_STK_SIZE,1024,DdmEventLogIOP);
CLASSENTRY("DDM_RMSTR", DEF_STK_SIZE,1024,DdmRAIDMstr);
CLASSENTRY("DDM_ALARM_MASTER", DEF_STK_SIZE, 1024, DdmAlarm );
CLASSENTRY("DDM_UPGRADE_MASTER", DEF_STK_SIZE, 1024, DdmUpgrade );
CLASSENTRY("DDM_FILESYS_MASTER", DEF_STK_SIZE, 1024, DdmFileSystem );
CLASSENTRY("DDM_SSAPI_TEST", DEF_STK_SIZE, 1024, DdmSsapiTest );
CLASSENTRY("DDM_VCM", DEF_STK_SIZE, 1024, DdmVCM );
CLASSENTRY("PARTITIONMGR",  DEF_STK_SIZE,1024,DdmPartitionMgr);
CLASSENTRY("DDM_HOT_SWAP_MASTER", DEF_STK_SIZE, 1024, CDdmHotSwap );
CLASSENTRY("HDM_SNMP",10240,1024,DdmSnmp);
CLASSENTRY("DDM_PMSTR", DEF_STK_SIZE,1024,DdmPartitionMstr);
CLASSENTRY("DDMCONSOLEHBC",  DEF_STK_SIZE,1024,DdmConsoleHbc);
//CLASSENTRY("DDM_PPP", DEF_STK_SIZE, 1024, DdmPPP);
CLASSENTRY("DDM_QUIESCE_MASTER",  DEF_STK_SIZE,1024,DdmQuiesceMaster);
//CLASSENTRY("DDM_IOP_FAILOVER_MASTER",  DEF_STK_SIZE,1024,DdmIopFailoverMaster);
CLASSENTRY("DDM_STATUS",  DEF_STK_SIZE,1024,DdmStatus);
CLASSENTRY("DDM_TEST_MANAGER", DEF_STK_SIZE, 1024, DdmTestManager);
CLASSENTRY("DDM_TEST_CONSOLE", DEF_STK_SIZE, 1024, DdmTestConsole);
CLASSENTRY("DDM_TEST_SAMPLE", DEF_STK_SIZE, 1024, DdmTestSample);

CLASSENTRY("DDMPROFILE"	  ,10240,1024, DdmProfile);
CLASSENTRY("DDMLEAK"	  ,10240,1024, DdmLeak);
CLASSENTRY("DDMCPU"	  	  ,10240,1024, DdmCpu);


//==============================================================================
// SystemVdn.h - Virtual Device Numbers
//
// These numbers will eventually come from the persistant
// table service.
#include "SystemVdn.h"


//
// Device Initialization
//
// Procedures called for initialization at system startup
//

DEVICEENTRY("Interrupt", 	Interrupt);
DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DDMMANAGER", 	DdmManager);
DEVICEENTRY("PIT",			TimerPIT);
DEVICEENTRY("Network",      Network);
DEVICEENTRY("Transport_Pci",Transport_Pci);
DEVICEENTRY("Transport_Net",Transport_Net);
DEVICEENTRY("Transport", 	Transport);
DEVICEENTRY("FailSafe", 	FailSafe);

DEVICEENTRY("Profile", 		Profile);
DEVICEENTRY("Leak", 		Leak);
DEVICEENTRY("Cpu",			CpuStatistics);

//
// System Ddms (No Virtual Device Numbers)
//

SYSTEMENTRY("DDMMANAGER");		// Must be first!
SYSTEMENTRY("DDMTIMER");
SYSTEMENTRY("PARTITIONMGR");
SYSTEMENTRY("DDMPROFILE");
SYSTEMENTRY("DDMLEAK");
SYSTEMENTRY("DDMCPU");
SYSTEMMASTER("HDM_PTS",  vdnPTS);
SYSTEMENTRY("PTSLOADER");		// 
SYSTEMENTRY("CDdmCMB");
SYSTEMMASTER("VMS",		vdnVMS);
SYSTEMENTRY("VIRTUALMANAGER");	// 
SYSTEMENTRY("DDMSYSINFO");
#if defined(PRECONFIG)
SYSTEMENTRY("PTSDEFAULT");	// No export table load for dynamic VC create versions.
#endif
SYSTEMENTRY("HDM_REPORTER");
SYSTEMENTRY("DDM_STATUS");
SYSTEMENTRY("DDM_ENV");
SYSTEMENTRY("DDM_LED");
SYSTEMENTRY("DDM_BootMgr");
// SYSTEMENTRY("DDM_QUIESCE_MASTER");
// SYSTEMENTRY("DDM_IOP_FAILOVER_MASTER");
//SYSTEMENTRY("VIRTUALMANAGER");	//
//SYSTEMENTRY("DDMTRANSPORT");

SYSTEMENTRY("DDM_HOT_SWAP_MASTER");
SYSTEMENTRY("DDM_LOGMASTER");
SYSTEMENTRY("DDM_IOPLOG");
SYSTEMENTRY("DDM_SSAPI_TEST");
SYSTEMMASTER("DDM_NET_MGR", vdn_NetMgr);
SYSTEMENTRY("HDM_SSERV");
SYSTEMENTRY("HDM_SSAPI");
SYSTEMENTRY("DDM_NET_MASTER");
//SYSTEMENTRY("DDM_PPP");
SYSTEMENTRY("DDM_ALARM_MASTER" );
SYSTEMENTRY("DDM_UPGRADE_MASTER" );
SYSTEMENTRY("DDM_FILESYS_MASTER" );
SYSTEMENTRY("DDMTRANSPORT");
SYSTEMENTRY("DDM_QUIESCE_MASTER");
//SYSTEMENTRY("DDM_IOP_FAILOVER_MASTER");
SYSTEMENTRY("DDM_RMSTR" );
SYSTEMENTRY("DDM_PMSTR" );
SYSTEMENTRY("DDM_VCM");
SYSTEMENTRY("HDM_SNMP");

SYSTEMMASTER("DDM_TEST_MANAGER", vdn_TestMgr);
SYSTEMENTRY("DDM_TEST_CONSOLE");
SYSTEMENTRY("DDM_TEST_SAMPLE");

SYSTEMENTRY("DDMCONSOLEHBC");


// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary

//PTSNAME("HDM_PTS",      vdnPTS,  IOP_HBC0, IOP_HBC1);
//VMSNAME("VMS",			vdnVMS,	 IOP_HBC0, IOP_HBC1);

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
#endif

#if true
FF_CONFIG flashConfig;
#else
FF_CONFIG flashConfig = {
						// version of FF_CONFIG record
	FF_CONFIG_VERSION,	// U32 version;

						// Size of FF_CONFIG record in bytes.
						// This need not be set by FF_Initialize, but it is returned
						// by FF_Get_Config.
	sizeof(FF_CONFIG),	// U32		size;

						// Pointer to flash device.
	devFSS,				// Flash_Device *p_device;

						// Pointer to memory to be used by the flash manager.
	NULL,				// void	*p_memory;

						// Amount of memory to be allocated for the flash storage system.
						// The user may call FF_Get_Memory_Size_Required
						// to allocate the proper amount of memory for the flash storage system.
						// If the user does not fill in the memory parameters for the
						// cache config, the left over memory will be used for the cache manager.
	1048576,			// U32 memory_size;

						// Verify write 1 means verify every write.
	true,				// U32 verify_write;

						// Verify erase level 1 means verify block sectors for all ones
						// after erase.
						// Verify erase level level 2 means write and verify block sectors
						// for checkerboard after level 1 check.
						// Verify erase level level 3 means write and verify block sectors
						// for inverted checkerboard after level 2 check.
	false,				// U32 verify_erase;

						// Verify erased before write is normally used only for testing
						// to assure that the erase operation is working properly.
	false,				// U32 verify_page_erased_before_write;

						// When wear_level_threshold pages have been erased, the wear level
						// algorithm is started.  The wear level address is advanced by one
						// block, and the block is erased.
	25,					// U32 wear_level_threshold;

						// Format configuration parameters begin here.

	1,					// U32 percentage_erased_pages;
	1,					// U32 percentage_replacement_pages;

						// When the number of replacement pages falls below a percentage
						// of the total number allocated, no more Write commands
						// are permitted.
	25,					// U32 replacement_page_threshold;

						// If erase_all_pages is true, all pages will be erased when the
						// unit is formatted (a time-consuming operation)
						// otherwise, only erase as many pages
						// as needed when the unit is formatted;
	false,				// U32 erase_all_pages;

						// Set all error tests to go off once every test_all_random
						// at random.
	false,				// U32 test_all_random;

						// Set all error tests to go off once every test_all_static
						// times the test is executed.
	false,				// U32 test_all_static;

						// Simulate a write error every write_error_frequency_value.
	0,					// U32 write_error_frequency_value;

						// Simulate an erase error every erase_error_frequency_value.
	0,					// U32 erase_error_frequency_value;

						// Simulate a read ECC every erase_error_frequency_value.
	0					// U32 read_error_frequency_value;
};
#endif


#if true
CM_CONFIG cacheConfig;
#else
CM_CONFIG cacheConfig = {
						// version of CM_CONFIG record
	CM_CONFIG_VERSION,	//	U32	version;

						// Size of CM_CONFIG record in bytes.
						// This need not be set by FF_Initialize, but it is returned
						// by CM_Get_Config.
	sizeof(CM_CONFIG),	// U32 size;

						// Number of bytes per cache page.
	2048,				// U32	page_size;

						// Number of pages in the cache.
	0,					// U32 num_pages;

						// Pointer to memory to be used for pages.
						// The user must have allocated page_size * num_pages bytes of memory.
	NULL,				// void *p_page_memory;

						// Number of pages in the secondary cache -- 0 if secondary not used.
	0,					// U32 num_pages_secondary;

						// Pointer to memory to be used for pages for secondary cache
						// 0 if secondary cache not used.
						// The user must have allocated page_size * num_pages_secondary bytes of memory.
	NULL,				// void *p_page_memory_secondary;

						// Pointer to memory to be used for internal cache manager tables.
						// The user must have allocated CM_Get_Memory_Size_Required() bytes of memory.
	NULL,				// void *p_table_memory;

						// page_table_size specifies the number of entries in the page table
						// for a cache that uses linear mapping.
	32768,					// U32	page_table_size;

						// hash_table_size specifies the number of entries in the hash table
						// for a cache that uses associative mapping.  page_table_size and
						// hash_table_size are mutually exclusive.
	0,					// U32	hash_table_size;

						// Number of reserve pages in the cache for CM_Open_Page with
						// CM_PRIORITY_RESERVE.
	64,					// U32 num_reserve_pages;

						// Dirty page writeback threshold.
						// When dirty page writeback threshold is reached, user's CM_WRITE_CALLBACK
						// is called to write out a dirty page.
						// This number is a percentage of the total page frames, which must be
						// less than 100.
	60,					// U32 dirty_page_writeback_threshold;

						// Dirty page error threshold.
						// When dirty page error threshold is reached,
						// CM_ERROR_MAX_DIRTY_PAGES is returned.
						// This number is a percentage of the total page frames,
						// which can be 100.
	95,					// U32 dirty_page_error_threshold;

						// Number of pages to prefetch forward when a cache miss is encountered.
	0,					// U32 num_prefetch_forward;

						// Number of pages to prefetch backward when a cache miss is encountered.
	0					// U32 num_prefetch_backward;
};
#endif


PmConfig bddPartitionMgr = {
	2048,				// U32 cPages;
	2048,				// U16 bytesPerPage;
	flashConfig,		// FF_CONFIG *flashConfig;
	cacheConfig,		// CM_CONFIG *cacheConfig;
	false,				// U32 reformat; // temp for testing
	false				// U32 allowCreate; // temp for testing
};


//BOOTENTRY("DdmManager",	&bddDdmManager );  // only used on Eval systems
BOOTENTRY("Transport",	&bddMessenger );
BOOTENTRY("Timer",		&bddTimer );
BOOTENTRY("DdmPartitionMgr",	&bddPartitionMgr );



//////////////////////////////////////////////////////////////////////
// FCP TARGET CONFIG DATA
//////////////////////////////////////////////////////////////////////

//=========================================================================
// This section declares the tables for the dummy PersistentData service
//

//#include "PersistentData.h"
#include "FcpConfig.h"
#include "FcpIsp.h"
#include "FcpData.h"


FCP_CONFIG	TargetParms[] = {
	{	// TargetParms[0] - for loop 0 on slot 24 (A4)
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
	16384,	// high level interrupt stack size
	8,		// number of Target IDs (max 31)
	8,		// number of LUNs
	40,		// number of PCI window buffers
	TRGT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT, 0),	// calculate the loop instance num
	vdnLM},		// virtual device to throw events to
	
	{	// TargetParms[1] - for loop 0 on slot 27 (A3)
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
	16384,	// high level interrupt stack size
	8,		// number of Target IDs (max 31)
	8,		// number of LUNs
	40,		// number of PCI window buffers
	TRGT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT2, 0),	// calculate the loop instance num
	vdnLM1},		// virtual device to throw events to

	{	// TargetParms[2] - for loop 0 on slot 26 (A2)
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
	16384,	// high level interrupt stack size
	8,		// number of Target IDs (max 31)
	8,		// number of LUNs
	40,		// number of PCI window buffers
	TRGT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT4, 0),	// calculate the loop instance num
	vdnLM2},		// virtual device to throw events to
	
	{	// TargetParms[3] - for loop 0 on slot 25 (A1)
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
	16384,	// high level interrupt stack size
	8,		// number of Target IDs (max 31)
	8,		// number of LUNs
	40,		// number of PCI window buffers
	TRGT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT6, 0),	// calculate the loop instance num
	vdnLM3},		// virtual device to throw events to
	
	{	// TargetParms[4] - for loop 0 on slot 28 (D4)
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
	16384,	// high level interrupt stack size
	8,		// number of Target IDs (max 31)
	8,		// number of LUNs
	40,		// number of PCI window buffers
	TRGT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT1, 0),	// calculate the loop instance num
	vdnLM1},		// virtual device to throw events to
	
	{	// TargetParms[5] - for loop 0 on slot 16 (B1)
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
	16384,	// high level interrupt stack size
	8,		// number of Target IDs (max 31)
	8,		// number of LUNs
	40,		// number of PCI window buffers
	TRGT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(IOP_SSDU0, 0),	// calculate the loop instance num
	vdnLM2}		// virtual device to throw events to

};


//////////////////////////////////////////////////////////////////////
// END OF FCP TARGET CONFIG DATA
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Loop Monitor Parms
// Used on each NAC
//////////////////////////////////////////////////////////////////////
#include "LmConfig.h"

LM_CONFIG	NewLoopMonParms[] =
{ 
	// LM0
	{	// NewLoopMonParms for any Slot.
	{0,0,0},
	LM_CONFIG_VERSION+1,
	sizeof(LM_CONFIG),
#ifdef NAC_E2
	3,							// number of loops to scan
#else
	2,							// number of loops to scan
#endif
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	0,							// was loop instance num of chip 0
#ifdef NAC_E2
	1,							// was loop instance num of chip 1
#endif
	2}							// was loop instance num of chip 2
    }    
};

LM_CONFIG	LoopMonParms[] =
{ 
	// LM0
	{	// LoopMonParms[0] for Slot #24 (A4)
	{0,0,0},
	LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
#ifdef NAC_E2
	3,							// number of loops to scan
#else
	2,							// number of loops to scan
#endif
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	FCLOOPINSTANCE(NAC_SLOT, 0), // loop instance num of chip 0
#ifdef NAC_E2
	FCLOOPINSTANCE(NAC_SLOT, 1), // loop instance num of chip 1
#endif
	FCLOOPINSTANCE(NAC_SLOT, 2)} // loop instance num of chip 2
    },

    // LM1
	{	// LoopMonParms[1] for Slot #27 (A3)
	{0,0,0},
	LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
#ifdef NAC_E2
	3,							// number of loops to scan
#else
	2,							// number of loops to scan
#endif
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	FCLOOPINSTANCE(NAC_SLOT2, 0),// loop instance num of chip 0
#ifdef NAC_E2
	FCLOOPINSTANCE(NAC_SLOT2, 1), // loop instance num of chip 1
#endif
	FCLOOPINSTANCE(NAC_SLOT2, 2)}	// loop instance num of chip 2
	},

    // LM2
	{	// LoopMonParms[2] for Slot #26 (A2)
	{0,0,0},
	LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
#ifdef NAC_E2
	3,							// number of loops to scan
#else
	2,							// number of loops to scan
#endif
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	FCLOOPINSTANCE(NAC_SLOT4, 0),// loop instance num of chip 0
#ifdef NAC_E2
	FCLOOPINSTANCE(NAC_SLOT4, 1), // loop instance num of chip 1
#endif
	FCLOOPINSTANCE(NAC_SLOT4, 2)}	// loop instance num of chip 2
	},

    // LM3
	{	// LoopMonParms[3] for Slot #25 (A1)
	{0,0,0},
	LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
#ifdef NAC_E2
	3,							// number of loops to scan
#else
	2,							// number of loops to scan
#endif
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	FCLOOPINSTANCE(NAC_SLOT6, 0),// loop instance num of chip 0
#ifdef NAC_E2
	FCLOOPINSTANCE(NAC_SLOT6, 1), // loop instance num of chip 1
#endif
	FCLOOPINSTANCE(NAC_SLOT6, 2)}	// loop instance num of chip 2
	},
	
    // LM4
	{	// LoopMonParms[4] for Slot #28 (D4)
	{0,0,0},
	LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
#ifdef NAC_E2
	3,							// number of loops to scan
#else
	2,							// number of loops to scan
#endif
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	FCLOOPINSTANCE(NAC_SLOT1, 0),// loop instance num of chip 0
#ifdef NAC_E2
	FCLOOPINSTANCE(NAC_SLOT1, 1), // loop instance num of chip 1
#endif
	FCLOOPINSTANCE(NAC_SLOT1, 2)}	// loop instance num of chip 2
	},
	
    // LM5
	{
	{0,0,0},
	LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
#ifdef NAC_E2
	3,							// number of loops to scan
#else
	2,							// number of loops to scan
#endif
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	FCLOOPINSTANCE(NAC_SLOT8, 0),// loop instance num of chip 0
#ifdef NAC_E2
	FCLOOPINSTANCE(NAC_SLOT8, 1), // loop instance num of chip 1
#endif
	FCLOOPINSTANCE(NAC_SLOT8, 2)}	// loop instance num of chip 2
	},

    // LM6
	{
	{0,0,0},
	LM_CONFIG_VERSION,
	sizeof(LM_CONFIG),
#ifdef NAC_E2
	3,							// number of loops to scan
#else
	2,							// number of loops to scan
#endif
	0,							// flags
	0,							// FC_MASTER VDN
	{							// array of FC Instance numbers
	FCLOOPINSTANCE(NAC_SLOT9, 0),// loop instance num of chip 0
#ifdef NAC_E2
	FCLOOPINSTANCE(NAC_SLOT9, 1), // loop instance num of chip 1
#endif
	FCLOOPINSTANCE(NAC_SLOT9, 2)}	// loop instance num of chip 2
	},


};
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// FCP INITIATOR CONFIG DATA
//////////////////////////////////////////////////////////////////////


//=========================================================================
// This section declares the tables for the dummy PersistentData service
// 

//#include "PersistentData.h"
#include "FcpConfig.h"
#include "FcpIsp.h"
#include "FcpData.h"

FCP_CONFIG	NewFcpParms[] = {

	{	// NewFcpParms[0] - for loop 0 on any slot.
	{0,0,0},
	FCP_CONFIG_VERSION+1,
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
	0,		// was FCLOOPINSTANCE(NAC_SLOT, 0),	we will calculate the loop instance num
	vdnLM},	// virtual device to throw events to

	{	// NewFcpParms[1] - for loop 1 on any slot
	{0,0,0},
	FCP_CONFIG_VERSION+1,
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
	1,		// was FCLOOPINSTANCE(NAC_SLOT, 1),	we will calculate the loop instance num
	vdnLM
	},		// virtual device to throw events to

	{	// NewFcpParms[2] - for loop 2 on any slot
	{0,0,0},
	FCP_CONFIG_VERSION+1,
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
	2,		// was FCLOOPINSTANCE(NAC_SLOT, 1), we will calculate the loop instance num
	vdnLM
	}		// virtual device to throw events to
};	// end of NewFcpParms


FCP_CONFIG	InitParms[] = {
	{	// InitParms[0] - for loop 1 on slot 24 (A4)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT, 1),	// calculate the loop instance num
	vdnLM},		// virtual device to throw events to

	{	// InitParms[1] - for loop 2 on slot 24 (A4)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT, 2),	// calculate the loop instance num
	vdnLM},		// virtual device to throw events to

	{	// InitParms[2] - for loop 1 on slot 27 (A3)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT2, 1),	// calculate the loop instance num
	vdnLM1},		// virtual device to throw events to

	{	// InitParms[3] - for loop 2 on slot 27 (A3)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT2, 2),	// calculate the loop instance num
	vdnLM1},		// virtual device to throw events to
	
	{	// InitParms[4] - for loop 1 on slot 26 (A2)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT4, 1),	// calculate the loop instance num
	vdnLM2},		// virtual device to throw events to

	{	// InitParms[5] - for loop 2 on slot 26 (A2)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT4, 2),	// calculate the loop instance num
	vdnLM2},		// virtual device to throw events to

	{	// InitParms[6] - for loop 1 on slot 25 (A1)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT6, 1),	// calculate the loop instance num
	vdnLM2},		// virtual device to throw events to

	{	// InitParms[7] - for loop 2 on slot 25 (A1)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT6, 2),	// calculate the loop instance num
	vdnLM2},		// virtual device to throw events to
	
	{	// InitParms[8] - for loop 1 on slot 28 (D4)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT1, 1),	// calculate the loop instance num
	vdnLM1},		// virtual device to throw events to

	{	// InitParms[9] - for loop 2 on slot 28 (D4)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(NAC_SLOT1, 2),	// calculate the loop instance num
	vdnLM1},		// virtual device to throw events to

	{	// InitParms[10] - for loop 1 on slot 16 (B1)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(IOP_SSDU0, 1),	// calculate the loop instance num
	vdnLM2},		// virtual device to throw events to

	{	// InitParms[11] - for loop 2 on slot 16 (B1)
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
	16384,	// high level interrupt stack size
	0,		// number of Target IDs (max 31)
	0,		// number of LUNs
	0,		// number of PCI window buffers
	INIT_ONLY_INSTANCE,	// instance number
	FCLOOPINSTANCE(IOP_SSDU0, 2),	// calculate the loop instance num
	vdnLM2},		// virtual device to throw events to

};

#include "DmConfig.h"
#define	DDH

DM_CONFIG	NewDriveMonParms[] = { 
	{	// NewDriveMonParms[0] - for loop 0 (initiator) on any slot.
	{0,0,0},
	DM_CONFIG_VERSION+1,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
#if defined(REDUNDANT)
	DM_FLAGS_REDUNDANT|
#endif
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit,
	0,							// was Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros	for now

	{	// NewDriveMonParms[1] - for loop 1 (initiator) on any slot.
	{0,0,0},
	DM_CONFIG_VERSION+1,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
#if defined(REDUNDANT)
	DM_FLAGS_REDUNDANT|
#endif
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit1,
	1,							// was Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros	for now

	{	// NewDriveMonParms[2] - for loop 2 (initiator) on any slot.
	{0,0,0},
	DM_CONFIG_VERSION+1,
	sizeof(DM_CONFIG),
	126,							// number of drives to scan
#if defined(REDUNDANT)
	DM_FLAGS_REDUNDANT|
#endif
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit2,
	2,							// was Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 8, 9, 10, 11, 12,			// for use with Disk Drive Hubs (4)
	 16, 17, 18, 19, 20,
	 24, 25, 26, 27, 28,
	 0  }},						 // rest are all zeros	for now

};


//======================================================================================
// New config parameters for the secondary DriveMonitors
// These configs are always REDUNDANT
//
DM_CONFIG	NewDriveMonParmsSecondary[] = { 
	{	// NewDriveMonParms[0] - for loop 0 (initiator) on any slot.
	{0,0,0},
	DM_CONFIG_VERSION+1,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SECONDARY|			// These DM instances are for the secondary
	DM_FLAGS_REDUNDANT|
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit,
	0,							// was Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros	for now

	{	// NewDriveMonParms[1] - for loop 1 (initiator) on any slot.
	{0,0,0},
	DM_CONFIG_VERSION+1,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SECONDARY|			// These DM instances are for the secondary
	DM_FLAGS_REDUNDANT|
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit1,
	1,							// was Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros	for now

	{	// NewDriveMonParms[2] - for loop 2 (initiator) on any slot.
	{0,0,0},
	DM_CONFIG_VERSION+1,
	sizeof(DM_CONFIG),
	126l,							// number of drives to scan
	DM_FLAGS_SECONDARY|			// These DM instances are for the secondary
	DM_FLAGS_REDUNDANT|
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit2,
	2,							// was Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 8, 9, 10, 11, 12,			// for use with Disk Drive Hubs (4)
	 16, 17, 18, 19, 20,
	 24, 25, 26, 27, 28,
	 0  }},						 // rest are all zeros	for now

};

//============================================================================================
// Old DriveMonitor parms
DM_CONFIG	DriveMonParms[] = { 
	{	// DriveMonParms[0] - for loop 1 (initiator) slot 24 (A4)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit,
	FCLOOPINSTANCE(NAC_SLOT, 1),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros	for now

	{	// DriveMonParms[1] - for loop 2 (initiator) slot 24 (A4)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	20,							// number of drives to scan
	DM_FLAGS_SPIN_4X,			// DM Flags for DDH port
	vdnInit1,
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
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
#endif
	 0  }},						 // rest are all zeros	for now

	{	// DriveMonParms[2] - for loop 1 (initiator) slot 27 (A3)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit2,
	FCLOOPINSTANCE(NAC_SLOT2, 1),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros	for now

	{	// DriveMonParms[3] - for loop 2 (initiator) slot 27 (A3)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit3,
	FCLOOPINSTANCE(NAC_SLOT2, 2),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros for now

	{	// DriveMonParms[4] - for loop 1 (initiator) slot 26 (A2)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit4,
	FCLOOPINSTANCE(NAC_SLOT4, 1),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros for now

	{	// DriveMonParms[5] - for loop 2 (initiator) slot 26 (A2)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit5,
	FCLOOPINSTANCE(NAC_SLOT4, 2),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros for now

	{	// DriveMonParms[6] - for loop 1 (initiator) slot 25 (a1)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit6,
	FCLOOPINSTANCE(NAC_SLOT6, 1),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros for now

	{	// DriveMonParms[7] - for loop 2 (initiator) slot 25 (A1)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit7,
	FCLOOPINSTANCE(NAC_SLOT6, 2),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros for now

	{	// DriveMonParms[8] - for loop 1 (initiator) slot 28 (D4)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit2,
	FCLOOPINSTANCE(NAC_SLOT1, 1),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
#ifdef DDH
	 0, 1, 2, 3, 4,				// drive ID translation table
	 8, 9, 10, 11, 12,			// for use with Disk Drive Hubs (4)
	 16, 17, 18, 19, 20,
	 24, 25, 26, 27, 28,
#else
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
#endif
	 0  }},						 // rest are all zeros for now

	{	// DriveMonParms[9] - for loop 2 (initiator) slot 28 (D4)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	20,							// number of drives to scan
	DM_FLAGS_SPIN_4X,			// DM Flags for DDH port
	vdnInit3,
	FCLOOPINSTANCE(NAC_SLOT1, 2),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros for now

	{	// DriveMonParms[10] - for loop 1 (initiator) slot 16 (B1)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit4,
	FCLOOPINSTANCE(IOP_SSDU0, 1),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros for now

	{	// DriveMonParms[11] - for loop 2 (initiator) slot 16 (B1)
	{0,0,0},
	DM_CONFIG_VERSION,
	sizeof(DM_CONFIG),
	126,						// number of drives to scan
	DM_FLAGS_SPIN_4X|
	DM_FLAGS_SCAN_ALL|
	DM_FLAGS_SCAN_8_LUNS|
	DM_FLAGS_USE_GET_LUNS,		// DM Flags for ext ports
	vdnInit5,
	FCLOOPINSTANCE(IOP_SSDU0, 2),	// Loop instance
	//{0 },						// DM mode flags (per drive)
	{
	 0, 1, 2, 3, 4,				// drive ID translation table
	 5, 6, 7, 8, 9,				// for legacy systems
	 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19,
	 0  }},						 // rest are all zeros for now

	};



//////////////////////////////////////////////////////////////////////
// END OF FCP INITIATOR CONFIG DATA
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// SSD Config data
//////////////////////////////////////////////////////////////////////

//===========================================================================
//
#include "SsdConfig.h"
#include "SsdContext.h"

SSD_CONFIG SSDParms[] =
{
	{
		{0, 0, 0},				// rid
		SSD_CONFIG_VERSION,		// version
		sizeof(SSD_CONFIG),		// size
		0,						// capacity need not be set
		vdnSSD0,				// vdnBSADdm
		"123456789",			// SerialNumber
		vdnSSD0,				// vdnMonitor

	// CM_CONFIG(
		//CM_CONFIG_VERSION,		// version
		//0,						// size need not be set
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
		//FF_CONFIG_VERSION,		// version
		//0,						// size need not be set
		//MEMORY_FOR_SSD,				// memory_size
		//MEMORY_FOR_CALLBACKS,		// callback_memory_size
		0,						// verify_write_level
		1,						// verify_erase_level
		5,						// percentage_erased_level;
		5,						// percentage_replacement_pages;
		25,						// replacement_page_threshold;
		0,						// erase_all_pages;
		1,						// verify_format_level
	// ) FF_FORMAT_CONFIG
	}
};


#define		RAM_DISK_SIZE	(((1024*1024) / 512)*10)	// 10Mb in blocks

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

// Temporary entries until real PTS is operational
// Slot #0
//VIRTUALENTRY("DDM_RedundancyMgr",	TRUE, vdn_RedundancyMgr,	&pddNone, IOP_HBC0, NOSLOT);

#if true	// new way

// Instantiate target and initiator drivers and Loop Monitors on all NACS.
VIRTUALENTRY("HDM_FCP_TARGET",	TRUE, vdnTarget,	IOP_LOCAL,	NOSLOT, FCP_CONFIG, &NewFcpParms[0] 	);
#ifdef NAC_E2
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit1,		IOP_LOCAL,	NOSLOT, FCP_CONFIG, &NewFcpParms[1]	);
#endif
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit2,		IOP_LOCAL,	NOSLOT, FCP_CONFIG, &NewFcpParms[2]	);

// Instantiate drive monitors on all boards present FOR THE INITIATOR LOOPS ONLY!
// Note: the drive monitors need to be AutoStart because they Poke their vdn in
// the Loop Descriptor for use by the Loop Monitor who receives an event from the 
// FCP (target only?) driver which initiated a LIP at its initialization.  When
// the LIP event comes back the FCP driver call the Loop Monitor who callse the 
// Drive Monitor who initiates the scan.  Soo if the drive monitor never autostarts
// and never pokes its vdn then it will never get called back when the LIP happens
// and the scan will never happen.  I leave AUTOSTART FALSE on Initiator loops 
// andf that prevents the scan 
// 
#if false

// This will work once Drive monitors can run from redundant slots w/o duplicating
// disk descs...I think.
#ifdef NAC_E2
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM1, 		IOP_LOCAL,	NOSLOT,	DM_CONFIG,	&NewDriveMonParms[1]	);
#endif
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM2, 		IOP_LOCAL,	NOSLOT,	DM_CONFIG,	&NewDriveMonParms[2]	);

#else

// ... until then we need to instantiate them explicitely only on boards we want
// them on AND ONLY FOR THE INITIATOR LOOPS!
//.
#ifdef NAC_E2
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM1, 		NAC_SLOT,	NOSLOT,	DM_CONFIG,	&NewDriveMonParms[1]	);
#if defined(ENABLE_3_NACS) || defined(ENABLE_4_NACS)
// These two used for 3 or 4 NAC configurations
// TODO: our failover partner DM turned off until no table conflicts
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM1, 		NAC_SLOT1,	NOSLOT,	DM_CONFIG,	&NewDriveMonParmsSecondary[1]	);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM1, 		NAC_SLOT8,	NOSLOT,	DM_CONFIG,	&NewDriveMonParms[1]	);
#endif
#if defined(ENABLE_4_NACS)
// This device also needed for 4 NAC configurations
// TODO: our failover partner DM turned off until no table conflicts
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM1, 		NAC_SLOT9,	NOSLOT,	DM_CONFIG,	&NewDriveMonParmsSecondary[1]	);
#endif
#endif

VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM2, 		NAC_SLOT,	NOSLOT,	DM_CONFIG,	&NewDriveMonParms[2]	);
#if defined(ENABLE_3_NACS) || defined(ENABLE_4_NACS)
// These two used for 3 or 4 NAC configurations
// TODO: our failover partner DM turned off until no table conflicts
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM2, 		NAC_SLOT1,	NOSLOT,	DM_CONFIG,	&NewDriveMonParmsSecondary[2]	);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM2, 		NAC_SLOT8,	NOSLOT,	DM_CONFIG,	&NewDriveMonParms[2]	);
#endif
#if defined(ENABLE_4_NACS)
// TODO: our failover partner DM turned off until no table conflicts
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM2, 		NAC_SLOT9,	NOSLOT,	DM_CONFIG,	&NewDriveMonParmsSecondary[2]	);
#endif

#endif

// Instantiate loop monitors on all boards present. 
#if true
VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM, 		IOP_LOCAL,	NOSLOT,	LM_CONFIG,	&NewLoopMonParms[0] 	);
#else
// NO ONLY ON THE PRIMARIES
VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM, 		NAC_SLOT,	NOSLOT,	LM_CONFIG,	&NewLoopMonParms[0] 	);
//VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM1, 		NAC_SLOT1,	NOSLOT,	LM_CONFIG,	&NewLoopMonParms[0] 	);
//VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM2, 		NAC_SLOT8,	NOSLOT,	LM_CONFIG,	&NewLoopMonParms[0] 	);
//VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM3, 		NAC_SLOT9,	NOSLOT,	LM_CONFIG,	&NewLoopMonParms[0] 	);
#endif

#else	// old way

// Slot #24
#ifdef NAC_E2
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit,		NAC_SLOT,	NOSLOT, FCP_CONFIG, &InitParms[0]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM, 		NAC_SLOT,	NOSLOT,	DM_CONFIG,	&DriveMonParms[0]	);
#endif
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit1,		NAC_SLOT,	NOSLOT,	FCP_CONFIG, &InitParms[1]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM1, 		NAC_SLOT,	NOSLOT,	DM_CONFIG,	&DriveMonParms[1] 	);
VIRTUALENTRY("HDM_FCP_TARGET",	TRUE, vdnTarget,	NAC_SLOT,	NOSLOT,	FCP_CONFIG,	&TargetParms[0] 	);
VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM, 		NAC_SLOT,	NOSLOT,	LM_CONFIG,	&LoopMonParms[0] 	);

// Slot #28 (NAC_SLOT1 aka IOP_RAC1)
#if true
#ifdef NAC_E2
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit2,		NAC_SLOT1,	NOSLOT, FCP_CONFIG, &InitParms[8]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM2, 		NAC_SLOT1,	NOSLOT,	DM_CONFIG,	&DriveMonParms[8]	);
#endif
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit3,		NAC_SLOT1,	NOSLOT,	FCP_CONFIG, &InitParms[9]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM3, 		NAC_SLOT1,	NOSLOT,	DM_CONFIG,	&DriveMonParms[9] 	);
VIRTUALENTRY("HDM_FCP_TARGET",	TRUE, vdnTarget1,	NAC_SLOT1,	NOSLOT,	FCP_CONFIG,	&TargetParms[4] 	);
VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM1, 		NAC_SLOT1,	NOSLOT,	LM_CONFIG,	&LoopMonParms[4] 	);
#endif

// Slot #16 (IOP_SSDU0)
#if true
#ifdef NAC_E2
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit4,		IOP_SSDU0,	NOSLOT, FCP_CONFIG, &InitParms[10]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM4, 		IOP_SSDU0,	NOSLOT,	DM_CONFIG,	&DriveMonParms[10]	);
#endif
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit5,		IOP_SSDU0,	NOSLOT,	FCP_CONFIG, &InitParms[11]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM5, 		IOP_SSDU0,	NOSLOT,	DM_CONFIG,	&DriveMonParms[11] 	);
VIRTUALENTRY("HDM_FCP_TARGET",	TRUE, vdnTarget2,	IOP_SSDU0,	NOSLOT,	FCP_CONFIG,	&TargetParms[5] 	);
VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM2, 		IOP_SSDU0,	NOSLOT,	LM_CONFIG,	&LoopMonParms[5] 	);
#endif

#endif	// old way

VIRTUALENTRY("HDM_RD", 			TRUE, vdnRD0, 		NAC_SLOT,	NOSLOT,	RD_CONFIG,	&RamDiskParms[0]	);
VIRTUALENTRY("HDM_RD", 			TRUE, vdnRD1, 		NAC_SLOT,	NOSLOT,	RD_CONFIG,	&RamDiskParms[1]	);
VIRTUALENTRY("HDM_RD", 			TRUE, vdnRD2, 		NAC_SLOT,	NOSLOT,	RD_CONFIG,	&RamDiskParms[2]	);
VIRTUALENTRY("HDM_RD", 			TRUE, vdnRD3, 		NAC_SLOT,	NOSLOT,	RD_CONFIG,	&RamDiskParms[3]	);

// Slot #27
#if 0
#ifdef NAC_E2
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit2,		NAC_SLOT2,	NOSLOT, FCP_CONFIG, &InitParms[2]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM2, 		NAC_SLOT2,	NOSLOT,	DM_CONFIG,	&DriveMonParms[2]	);
#endif
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit3,		NAC_SLOT2,	NOSLOT,	FCP_CONFIG, &InitParms[3]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM3, 		NAC_SLOT2,	NOSLOT,	DM_CONFIG,	&DriveMonParms[3] 	);
VIRTUALENTRY("HDM_FCP_TARGET",	TRUE, vdnTarget1,	NAC_SLOT2,	NOSLOT,	FCP_CONFIG,	&TargetParms[4] 	);
VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM1, 		NAC_SLOT2,	NOSLOT,	LM_CONFIG,	&LoopMonParms[1] 	);

#endif


// Slot #26
#if 0
#ifdef NAC_E2
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit4,		NAC_SLOT4,	NOSLOT, FCP_CONFIG, &InitParms[4]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM4, 		NAC_SLOT4,	NOSLOT,	DM_CONFIG,	&DriveMonParms[4]	);
#endif
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit5,		NAC_SLOT4,	NOSLOT,	FCP_CONFIG, &InitParms[5]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM5, 		NAC_SLOT4,	NOSLOT,	DM_CONFIG,	&DriveMonParms[5] 	);
VIRTUALENTRY("HDM_FCP_TARGET",	TRUE, vdnTarget2,	NAC_SLOT4,	NOSLOT,	FCP_CONFIG,	&TargetParms[2] 	);
VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM2, 		NAC_SLOT4,	NOSLOT,	LM_CONFIG,	&LoopMonParms[2] 	);

#endif

// Slot #25
#if 0
#ifdef NAC_E2
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit6,		NAC_SLOT6,	NOSLOT, FCP_CONFIG, &InitParms[6]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM6, 		NAC_SLOT6,	NOSLOT,	DM_CONFIG,	&DriveMonParms[6]	);
#endif
VIRTUALENTRY("HDM_FCP_INIT",	TRUE, vdnInit7,		NAC_SLOT6,	NOSLOT,	FCP_CONFIG, &InitParms[7]		);
VIRTUALENTRY("HDM_DRIVE_MON",	TRUE, vdnDM7, 		NAC_SLOT6,	NOSLOT,	DM_CONFIG,	&DriveMonParms[7] 	);
VIRTUALENTRY("HDM_FCP_TARGET",	TRUE, vdnTarget3,	NAC_SLOT6,	NOSLOT,	FCP_CONFIG,	&TargetParms[3] 	);
VIRTUALENTRY("HDM_LOOP_MON", 	TRUE, vdnLM3, 		NAC_SLOT6,	NOSLOT,	LM_CONFIG,	&LoopMonParms[3] 	);

#endif

#if 0
// When we put this back, everyone must have an SSD.
VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD0,		IOP_SSDU2,	NOSLOT, SSD_CONFIG,	&SSDParms[0]);
#endif

#if 0
// LATER
// NOTE: we can't have any slots listed here that are not really there because the
// os will wait for them.  So, until later we must comment out any that don't exist.
VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD1, 				&SSDParms[1], IOP_SSDU0,NOSLOT);
VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD2, 				&SSDParms[2], IOP_SSDU1,NOSLOT);
VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD3, 				&SSDParms[3], IOP_SSDL0,NOSLOT);
VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD4, 				&SSDParms[4], IOP_SSDL1,NOSLOT);
VIRTUALENTRY("HDM_SSD", 			TRUE, vdnSSD5, 				&SSDParms[5], IOP_SSDL2,NOSLOT);

#endif


//================================================================================================================
// This section turned on when PRECONFIG is true
// this enables preconfigured Virtual Circuits
#if defined(PRECONFIG)

#include "BsaConfig.h"

BSA_CONFIG BsaParms[] = {
// 	           Version               size               LUN ID  Init ID SMART
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  0, vdnInit2, 0	},	// all on DDH
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  1, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  2, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  3, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  4, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  8, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  9, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  10, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  11, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  12, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  16, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  17, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  18, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  19, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  20, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  24, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  25, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  26, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  27, vdnInit2, 0	},
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  28, vdnInit2, 0	},
	// NAC #2
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  0, vdnInit3, 0	},	// 20
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  1, vdnInit3, 0	},	// 21
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  2, vdnInit3, 0	},	// 22
	{ {0,0,0}, BSA_CONFIG_VERSION,	sizeof(BSA_CONFIG),	0,  3, vdnInit3, 0	}	// 23
};


#include "StsConfig.h"

STS_CONFIG	ScsiServerParms[] =
	{
	{ {0,0,0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0,0,0}, vdnRD0 },
	{ {0,0,0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0,0,0}, vdnRD1 },
	{ {0,0,0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0,0,0}, vdnRD2 },
	{ {0,0,0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0,0,0}, vdnRD3 },
	{ {0,0,0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0,0,0}, vdnBSA0 },
	{ {0,0,0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0,0,0}, vdnBSA1 },
//	{ {0,0,0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0,0,0}, vdnBSA2 },
//	{ {0,0,0}, STS_CONFIG_VERSION, sizeof(STS_CONFIG), {0,0,0}, vdnRAID0 },

};

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

// helpers

// SLot #24
#define	LP1		(FCLOOPINSTANCE(NAC_SLOT, 0))
#ifdef NAC_E2
#define	LP2		(FCLOOPINSTANCE(NAC_SLOT, 1))
#endif
#define	LP3		(FCLOOPINSTANCE(NAC_SLOT, 2))

// SLot #28
#define	LP4		(FCLOOPINSTANCE(NAC_SLOT2, 0))
#ifdef NAC_E2
#define	LP5		(FCLOOPINSTANCE(NAC_SLOT2, 1))
#endif
#define	LP6		(FCLOOPINSTANCE(NAC_SLOT2, 2))

// SLot #27
#define	LP7		(FCLOOPINSTANCE(NAC_SLOT4, 0))
#ifdef NAC_E2
#define	LP8		(FCLOOPINSTANCE(NAC_SLOT4, 1))
#endif
#define	LP9		(FCLOOPINSTANCE(NAC_SLOT4, 2))


// numExportRows and Export[] are used by DDMPTSDEFAULT to load
// default data into the PTS
U32		numExportRows = 6;

ExportTableEntry			Export[6] = 	// one for each VirtualCircuit
{
	// Slot #24, Four RAM disks 2 regular disks
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS0, 0, 0, 0, -1, 2, LP1, "01", RAM_DISK_SIZE, 0, StateConfigured, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS1, 0, 0, 1, -1, 2, LP1, "02", RAM_DISK_SIZE, 0, StateConfigured, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS2, 0, 0, 2, -1, 2, LP1, "03", RAM_DISK_SIZE, 0, StateConfigured, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS3, 0, 0, 3, -1, 2, LP1, "04", RAM_DISK_SIZE, 0, StateConfigured, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS4, 0, 0, 4, -1, 2, LP1, "05", 0x010f59c7,    0, StateConfigured, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS5, 0, 0, 5, -1, 2, LP1, "06", 0x010f59c7,    0, StateConfigured, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
//	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS6, 0, 0, 6, -1, 2, LP1, "07", 0x010f59c7,    0, StateConfigured, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	// Slot #28, RAID - 4 drive stripe
//	{{0,0}, EXPORT_TABLE_VERSION, sizeof(ExportTableEntry), {0, 0, 0}, ProtocolFibreChannel,  vdnSTS7, 0, 0, 0, -1, 2, LP4, "00", 0x087acc00,    0, StateConfigured, StateConfigured, " ", {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	// Slot #27,
};


//////////////////////////////////////////////////////////////////////
// DEFAULT EXPORT TABLE END
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// DEFAULT VIRTUAL DEVICES USED FOR PRE-CONFIG VC
//////////////////////////////////////////////////////////////////////



VIRTUALENTRY("HDM_STS", 		TRUE, vdnSTS0,		NAC_SLOT,	NOSLOT,	STS_CONFIG,	&ScsiServerParms[0]	);
VIRTUALENTRY("HDM_STS", 		TRUE, vdnSTS1,		NAC_SLOT,	NOSLOT,	STS_CONFIG,	&ScsiServerParms[1]	);
VIRTUALENTRY("HDM_STS", 		TRUE, vdnSTS2,		NAC_SLOT,	NOSLOT,	STS_CONFIG,	&ScsiServerParms[2]	);
VIRTUALENTRY("HDM_STS", 		TRUE, vdnSTS3,		NAC_SLOT,	NOSLOT,	STS_CONFIG,	&ScsiServerParms[3]	);
VIRTUALENTRY("HDM_STS", 		TRUE, vdnSTS4,		NAC_SLOT,	NOSLOT,	STS_CONFIG,	&ScsiServerParms[4]	);
VIRTUALENTRY("HDM_STS", 		TRUE, vdnSTS5,		NAC_SLOT,	NOSLOT,	STS_CONFIG,	&ScsiServerParms[5]	);

// We can't support instantion of a RAID virtual device from here because its descriptor record 
// requires the row IDs of Members descriptors and I don't have a way yet to define those. 
//VIRTUALENTRY("HDM_RAID", 		TRUE, vdnRAID1, 	NAC_SLOT,	NOSLOT,	&RAIDParms[0]	);

VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA0,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[0]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA1,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[1]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA2,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[2]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA3,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[3]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA4,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[4]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA5,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[5]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA6,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[6]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA7,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[7]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA8,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[8]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA9,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[9]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA10,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[10]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA11,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[11]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA12,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[12]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA13,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[13]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA14,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[14]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA15,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[15]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA16,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[16]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA17,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[17]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA18,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[18]	);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA19,		NAC_SLOT,	NOSLOT,	BSA_CONFIG,	&BsaParms[19]	);


#if 0
VIRTUALENTRY("HDM_STS", 		TRUE, vdnSTS7,		NAC_SLOT2,	NOSLOT, STS_CONFIG,	&ScsiServerParms[7]);

VIRTUALENTRY("HDM_RAID", 		TRUE, vdnRAID0, 	NAC_SLOT2,	NOSLOT,	TempConfiguration,	&RAIDParms[1]);

VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA20,		NAC_SLOT2,	NOSLOT,	BSA_CONFIG,	&BsaParms[20]);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA21,		NAC_SLOT2,	NOSLOT,	BSA_CONFIG,	&BsaParms[21]);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA22,		NAC_SLOT2,	NOSLOT,	BSA_CONFIG,	&BsaParms[22]);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA23,		NAC_SLOT2,	NOSLOT,	BSA_CONFIG,	&BsaParms[23]);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA24,		NAC_SLOT2,	NOSLOT,	BSA_CONFIG,	&BsaParms[24]);
VIRTUALENTRY("HDM_BSA", 		TRUE, vdnBSA25,		NAC_SLOT2,	NOSLOT,	BSA_CONFIG,	&BsaParms[25]);

VIRTUALENTRY("HDM_RD", 			TRUE, vdnRD4, 		NAC_SLOT2,	NOSLOT,	RD_CONFIG,	&RamDiskParms[4]);
VIRTUALENTRY("HDM_RD", 			TRUE, vdnRD5, 		NAC_SLOT2,	NOSLOT,	RD_CONFIG,	&RamDiskParms[5]);
#endif

#endif

