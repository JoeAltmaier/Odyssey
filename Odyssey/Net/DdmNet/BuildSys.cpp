/* BuildSys.cpp -- System Tables
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
 *     5/7/99: Created (RCB)
 *
**/

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif

#include "BuildSys.h" 


extern "C" U32 gSize_total_memory = 0x01000000;			// 16 MB
extern "C" U32 gSize_small_heap = 0x00200000;			// 2 MB
extern "C" U32 gSizeReserved = 0x00000000;

//* DDM Classes

// CHAOS Ddm Classes
CLASSENTRY("DDMMANAGER",   10240,1024, DdmManager);
//CLASSENTRY("PTSPROXYLOADER",10240,1024,DdmPtsProxyLoader);	// Loads PtsProxy 
CLASSENTRY("PTSLOADER",	   10240,1024, DdmPtsLoader);		// Loads real Pts
//CLASSENTRY("VIRTUALPROXY", 10240,1024, DdmVirtualProxy);
//CLASSENTRY("DDMTRANSPORT", 10240,1024, Transport);
CLASSENTRY("DDMTIMER",     10240,1024, DdmTimer); 
//CLASSENTRY("DDMSYSINFO",   10240,1024, DdmSysInfo);


// 1st four entries are always there
//CLASSENTRY("HDM_PTS",  10240,1024,PersistentData);
CLASSENTRY("HDM_DMGR", 10240,1024,DdmManager);
//CLASSENTRY("HDM_TIMR", 10240,1024,DdmTimer);
//CLASSENTRY("DDM_PT",   10240,1024,DdmPTS);
// Customize your test image here
CLASSENTRY("HDM_NET",10240,1024,DdmNet);
CLASSENTRY("HDM_ECHOS",10240,1024,DdmEchoServer);


//* Device Initialization

//* Procedures called for initialization at system startup
DEVICEENTRY("DdmManager", 	DdmManager);
DEVICEENTRY("Network",		Network);

//* System Ddms (No Virtual Device Numbers)
SYSTEMENTRY("HDM_DMGR" );	// Must be first!
SYSTEMENTRY("HDM_NET");
SYSTEMENTRY("HDM_ECHOS");

//* Virtual Device Data
enum {
	vdn_PTS	= 1,// Must be one (1)
	vdn_PT
};

// Temporary PTS data
struct {} pddNone;

//* Virtual Device Ddms created at system startup.
VIRTUALENTRY("HDM_PTS",  TRUE, vdn_PTS,  &pddNone,	IOP_HBC0,SLOTNULL);
VIRTUALENTRY("DDM_PT",  TRUE, vdn_PT,  &pddNone,	IOP_HBC0,SLOTNULL);

//* Boot Configuration (Temporary)
struct {int iCabinet; TySlot iSlot;} bddDdmManager = {
	0, IOP_HBC0
};

struct {long cbMsg; long sMsg; long cbBuf; long sBuf; long pciWindow; long sWindow; } bddMessenger = {
	256000, 256, 1048536, 8192, 0x2000000, 0x1000000
};
	
struct {long divisor;}  bddTimer={10000};

BOOTENTRY("DdmManager",	&bddDdmManager );