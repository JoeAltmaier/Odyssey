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
//    System / Boot Tables -- A dummy version for testing message compiler-
//    generated code in an Eval crate (sort of Odyssey) MIPS environment.
// 
// $Log: /Gemini/Odyssey/Message Text/Test/BuildSys.cpp $
// 
// 1     7/12/99 6:10p Ewedel
// Initial revision.
//
/*************************************************************************/


#include "BuildSys.h"      // standard CHAOS buildsys definitions


//  here are a couple of symbols which our CHAOS-tailored MSL needs

//  eval board has 8MB, but we're only supposed to report the amount
//  which our load image is not using.. hmm!
extern "C" U32  gSize_available_memory = 0x480000;

//  size of small "non-fragmenting" heap (twice this amount is used
//  by the small heap when in debug mode)
extern "C" U32  gSize_small_heap = 0x100000;


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
CLASSENTRY("VIRTUALPROXY", 10240,1024, DdmVirtualProxy);
//CLASSENTRY("DDMTRANSPORT", 10240,1024, Transport);
CLASSENTRY("DDMTIMER",     10240,1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO",   10240,1024, DdmSysInfo);
CLASSENTRY("PTSPROXY",     10240,1024, DdmPtsProxy);

CLASSENTRY("PTSPROXYLOADER", 10240,1024, DdmPtsProxyLoader);
//CLASSENTRY("PTSLOADER",    10240,1024, DdmPtsLoader);

// Customize your test image here [McOdyTest]
//CLASSENTRY("HDM_TS",      10240, 1024, DdmPTS);
CLASSENTRY("ISM_MCODYTEST", 10240, 1024, CMcOdyTest);


//*
//* System Ddms (No Virtual Device Numbers)
//*

// SYSTEMENTRY(pszClassName,)

SYSTEMENTRY("DDMMANAGER");      // Must be first!

SYSTEMENTRY("PTSPROXYLOADER");   // weird new PTS stuff which Tom needs
//SYSTEMENTRY("PTSLOADER");

SYSTEMENTRY("VIRTUALPROXY");    // Should be second
//SYSTEMENTRY("DDMTRANSPORT" );
SYSTEMENTRY("DDMTIMER");
SYSTEMENTRY("DDMSYSINFO");


//*
//* Virtual Device Data
//*

enum {
   vdn_PTS  = 1,  // Must be one (1)
   vdn_TS,
   vdn_McOdyTest
};


// Only the primary slot is needed by the IOP.
// namePTS, vdnPTS, slotPrimary, slotSecondary

PTSPROXY("PTSPROXY",   vdn_PTS, IOP_HBC0, IOP_HBC1);
//PTSNAME ("HDM_TS",     vdn_TS,  IOP_HBC0, IOP_HBC1);


// Temporary PTS data

struct {} pddNone;
//struct {VDN vd;} pddScc = {vdn_SCC};

//*
//* Virtual Device Ddms created at system startup.
//*

// VIRTUALENTRY(pszClassName,fStart,vdn,pData,primary,secondary)

// Temporary entries until real PTS is operational (i.e., persistent)
//VIRTUALENTRY("HDM_TS",      TRUE, vdn_TS,      &pddNone, IOP_HBC0, IOP_HBC1);
VIRTUALENTRY("ISM_MCODYTEST", TRUE, vdn_McOdyTest, &pddNone, IOP_HBC0, SLOTNULL);


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


