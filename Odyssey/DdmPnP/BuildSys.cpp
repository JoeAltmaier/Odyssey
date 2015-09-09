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
//    System / Boot Tables -- A dummy version for testing Ddm.
// 
// $Log: /Gemini/Odyssey/DdmPnP/BuildSys.cpp $
// 
// 7     8/27/99 5:04p Hdo
// 
// 6     7/29/99 5:59p Hdo
// 
// 5     7/27/99 7:04p Hdo
// 
// 4     7/24/99 4:17p Hdo
// 
// 3     7/15/99 11:55a Hdo
// 
// 2     7/14/99 2:18p Hdo
// 
//	  7/14/99 Huy Do: Added PTSPROXY Macro
// 1     6/30/99 10:23a Hdo
// 
// 1     3/19/99 HDo
// Create.
//
/*************************************************************************/

#include "BuildSys.h"      // standard CHAOS buildsys definitions


//  here are a couple of symbols which our CHAOS-tailored MSL needs
extern "C" U32  gSize_available_memory = 0x1000000;		// eval board has 16MB
extern "C" U32  gSize_small_heap = 0x200000;			// small "non-fragmenting" heap
extern "C" U32	gSizeReserved = 0;
extern "C" U32	gSize_total_memory = 0x1000000;			// eval board has 16MB


//*
//* DDM Classes
//*

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER",   10240,1024, DdmManager);
CLASSENTRY("VIRTUALPROXY", 10240,1024, DdmVirtualProxy);
CLASSENTRY("DDMTRANSPORT", 10240,1024, Transport);
CLASSENTRY("DDMTIMER",     10240,1024, DdmTimer); 
CLASSENTRY("DDMSYSINFO",   10240,1024, DdmSysInfo);
CLASSENTRY("PTSPROXY",     10240,1024, DdmPtsProxy);

CLASSENTRY("PTSPROXYLOADER", 10240,1024, DdmPtsProxyLoader);
CLASSENTRY("PTSLOADER",    10240,1024, DdmPtsLoader);

// Customize your test image here DdmPnPTest
CLASSENTRY("HDM_TS",      10240, 1024, DdmPTS);
CLASSENTRY("HDM_PNP",     10240, 1024, DdmPnP);
CLASSENTRY("HDM_SD",      10240, 1024, SerialDdm);
CLASSENTRY("ISM_PNPTEST", 10240, 1024, DdmPnPTest);


//*
//* Device Initialization
//*
//* Procedures called for initialization at system startup
//*

// CHAOS Initialization
DEVICEENTRY("Interrupt", 	Interrupt);
DEVICEENTRY("Dma", 			Dma);
DEVICEENTRY("DdmManager", 	DdmManager);
DEVICEENTRY("PIT",			TimerPIT);
//DEVICEENTRY("Transport", 	Transport);
//DEVICEENTRY("FailSafe", 	FailSafe);


//*
//* System Ddms (No Virtual Device Numbers)
//*

// CHAOS Ddm Startups
SYSTEMENTRY("DDMMANAGER");		// Must be first!
SYSTEMENTRY("PTSPROXYLOADER");   // weird new PTS stuff which Tom needs
SYSTEMENTRY("PTSLOADER");
SYSTEMENTRY("VIRTUALPROXY");	// Should be second
//SYSTEMENTRY("DDMTRANSPORT" );
SYSTEMENTRY("DDMSYSINFO");
SYSTEMENTRY("DDMTIMER");
//  our local DDMs must be system entry
//  the CMB DDM must be "system start," since it helps facilitate boot
SYSTEMENTRY("HDM_PNP");
SYSTEMENTRY("HDM_SD");


//*
//* Virtual Device Data
//*

enum {
   vdn_PTS  = 1,  // Must be one (1)
   vdn_TS,
   vdn_PnP,
   vdn_Sd,
   vdn_PnPTest
};

// PTS Macros:
//		Only the primary slot is needed by the IOP.
// 		namePTS, vdnPTS, slotPrimary, slotSecondary

// Loads the PtsProxy service used VirtualProxy service
PTSPROXY("PTSPROXY", vdn_PTS, IOP_HBC0, IOP_HBC1);
PTSNAME ("HDM_TS",     vdn_TS,  IOP_HBC0, IOP_HBC1);

// VirtualEntry config data

struct {} pddNone;

//*
//* Virtual Device Ddms created at system startup.
//*

// Temporary entries until real PTS is operational (i.e., persistent)
//VIRTUALENTRY("HDM_TS",      TRUE, vdn_TS,      &pddNone, IOP_HBC0, IOP_HBC1);
//VIRTUALENTRY("HDM_PNP",     TRUE, vdn_PnP,     &pddNone, IOP_HBC0, IOP_HBC1);
//VIRTUALENTRY("HDM_SD",      TRUE, vdn_Sd,      &pddNone, IOP_HBC0, IOP_HBC1);
VIRTUALENTRY("ISM_PNPTEST", TRUE, vdn_PnPTest, &pddNone, IOP_HBC0, IOP_HBC1);


//*
//* Boot Configuration (Temporary)
//*
// ...Needed for the Eval System

struct {int iCabinet; TySlot iSlot;} bddDdmManager = {
	0, IOP_HBC0	// This is used only on the Eval Systems
};

struct {long cbMsg; long sMsg; long cbBuf; long sBuf; long pciWindow; long sWindow; } bddMessenger = {
	256000, 256, 1048536, 8192, 0x2000000, 0x1000000
};

BOOTENTRY("DdmManager",	&bddDdmManager );
BOOTENTRY("Transport",	&bddMessenger );
