/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: BuildSys.cpp
// 
// Description:
//    System / Boot Tables -- A dummy version for testing the Hot Swap
//    Master DDM.
// 
// $Log: /Gemini/Odyssey/DdmHotSwap/TestDdm/BuildSys.cpp $
// 
// 1     10/11/99 7:53p Ewedel
// First cut.
// 
/*************************************************************************/


#include "BuildSys.h"      // standard CHAOS buildsys definitions


//  here are a couple of symbols which our CHAOS-tailored MSL needs

//  Yes!  We now simply report the available physical memory on our eval
//  board.  The OS / library then figures out how much is available for
//  heap use after code and data are accounted for.
//  Thank you Iowa!
extern "C" U32  gSize_total_memory = 0x800000;  // min on an eval board: 8MB

//  size of small "non-fragmenting" heap (twice this amount is used
//  by the small heap when in debug mode)
//  [8/25/99: Per Joe A, new boot-block based scheme uses 3X small heap
//   size to cover small heap, debug small heap, and "PCI window" memory
//   on eval systems.]
extern "C" U32  gSize_small_heap = 0x100000;    // 1MB => 3MB consumed

//  Size of memory reserved for old-fashioned Nucleus memory allocation
extern "C" U32  gSizeReserved = 0;


//*
//* Device Initialization
//*
//* Procedures called for initialization at system startup
//*

// DEVICEENTRY(literal,name)

DEVICEENTRY("Interrupt",   Interrupt);
DEVICEENTRY("Dma",         Dma);
DEVICEENTRY("DdmManager",  DdmManager);
DEVICEENTRY("PIT",         TimerPIT);
//DEVICEENTRY("Transport",    Transport);
//DEVICEENTRY("FailSafe",  FailSafe);


//*
//* DDM Classes
//*

// New - 3rd parm is message Queue Size
// Parms - "ClassName", StackSize, QueueSize, LinkName (used to find Ctor)

// CLASSENTRY(literal,cbStack,sQueue,name)

// 1st four entries are always there
CLASSENTRY("DDMMANAGER",   10240,1024, DdmManager);
CLASSENTRY("PTSLOADER",    10240,1024, DdmPtsLoader);
CLASSENTRY("VIRTUALMANAGER",10240,1024, DdmVirtualManager);
//CLASSENTRY("DDMTRANSPORT", 10240,1024, Transport);
CLASSENTRY("DDMTIMER",     10240,1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO",   10240,1024, DdmSysInfo);

//  DdmCmbNull establishes a "valid" IOP Status table, based on entries
//  in buildsys, which permit virtual manager to run.
CLASSENTRY("HDM_CMBNULL", 10240, 1024, DdmCmbNull);   // dummy CMB DDM class

// CHAOS Ddm Classes (HBC ONLY)
CLASSENTRY("PTS"                  ,10240,1024, DdmPTS);
CLASSENTRY("VMS"                  ,10240,1024, DdmVirtualMaster);

//  enable boot man only when running on Odyssey!
//CLASSENTRY("BOOTMAN"      ,10240,1024, DdmBootMgr);

// Customize your test image here (HotSwapTest)
CLASSENTRY("DDM_HOTSWAP",     10240, 1024, CDdmHotSwap);    // real Hot Swapper
CLASSENTRY("DDM_HOTSWAPTEST", 10240, 1024, CHotSwapTest);



//*
//* System Ddms (No Virtual Device Numbers)
//*

SYSTEMENTRY("DDMMANAGER");      // Must be first!

SYSTEMENTRY("PTSLOADER");

SYSTEMENTRY("HDM_CMBNULL");      // keep things happy before CMB is run
      //  this should only be run on eval crates, or on single-board Odysseys
      //  (we can dump this if we add real boot manager)

//  only use bootman on Odyssey hardware
//SYSTEMENTRY("BOOTMAN");

//SYSTEMENTRY("DDMTRANSPORT" );
SYSTEMENTRY("DDMSYSINFO");
SYSTEMENTRY("DDMTIMER");
SYSTEMENTRY("VIRTUALMANAGER");

//  our local DDMs must be system entry

//*
//* Virtual Device Data
//*

enum {
   vdn_PTS  = 1,  // Must not be zero!
   vdn_VMS,
   vdn_HotSwap,
   vdn_HotSwapTest
};


// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary

PTSNAME("PTS", vdn_PTS, IOP_HBC0, IOP_HBC1);
VMSNAME("VMS", vdn_VMS, IOP_HBC0, IOP_HBC1);



//*
//* Virtual Device Ddms created at system startup.
//*


//  this is the way it will be in the real system (!)
VIRTUALENTRY("DDM_HOTSWAP",     TRUE, vdn_HotSwap,     IOP_HBC0, SLOTNULL,
             NoPtsTable, NoPtsData);

//  our test DDM..
VIRTUALENTRY("DDM_HOTSWAPTEST", TRUE, vdn_HotSwapTest, IOP_HBC0, SLOTNULL,
             NoPtsTable, NoPtsData);



//*
//* Boot Configuration (Temporary)
//*

//  define which IOP board we are in the system.  The IOP_... value here
//  *must* match that given as primary in VIRTUALENTRY() defs above.
struct {int iCabinet; TySlot iSlot;} bddDdmManager = {
   0, IOP_HBC0
};

//  
struct {long cbMsg; long sMsg; long cbBuf; long sBuf; long pciWindow; long sWindow; } bddMessenger = {
   256000, 256, 1048536, 8192, 0x2000000, 0x1000000
};
   

BOOTENTRY("DdmManager", &bddDdmManager );
BOOTENTRY("Transport",  &bddMessenger );


