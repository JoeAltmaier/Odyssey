// BuildSys.cpp
#include "BuildSys.h"

// Size of all available memory
extern "C" U32 gSize_available_memory = 0x00800000;   // 8MB
// Size of the small heap.  The rest of memory is in the large heap.
extern "C" U32 gSize_small_heap = 0x200000;         // 2MB
// Size of memory reserved for old-fashioned Nucleus memory allocation
extern "C" U32 gSizeReserved=0;

//*
//* DDM Classes
//*

// New - 3rd parm is message Queue Size
// Parms - "ClassName", StackSize, QueueSize, LinkName

// 1st four entries are always there
CLASSENTRY("DDM_MANAGER", 10240, 1024, DdmManager);
//CLASSENTRY("HDM_TRANSPORT", 10240, 1024, Transport);
//CLASSENTRY("DDM_TIMER", 10240, 1024, DdmTimer );
//CLASSENTRY("DDM_SYSINFO", 10240, 1024, DdmSysInfo);
CLASSENTRY("HDM_SNMP", 10240, 1024, TestSnmpDdm);

//*
//* Device Initialization
//*
//* Procedures called for initialization at system startup
//*
DEVICEENTRY("DdmManager",   DdmManager);
//DEVICEENTRY("Interrupt",    Interrupt);
//DEVICEENTRY("Dma",          Dma);
//DEVICEENTRY("Transport",    Transport);
//DEVICEENTRY("FailSafe",     FailSafe);


//*
//* System Ddms (No Virtual Device Numbers)
//*
SYSTEMENTRY("DDM_MANAGER");              // Must be first!
//SYSTEMENTRY("DDM_TIMER");
//SYSTEMENTRY("DDM_SYSINFO");
SYSTEMENTRY("HDM_SNMP");

//*
//* Boot Configuration (Temporary)
//*
struct {int iCabinet; TySlot iSlot;} bddDdmManager = {
	0, IOP_HBC0
};

struct {long cbMsg; long sMsg; long cbBuf; long sBuf; long pciWindow; long sWindow; } bddMessenger = {
	256000, 256, 1048536, 8192, 0x2000000, 0x1000000
};

struct {long divisor;}  bddTimer={10000};

BOOTENTRY("DdmManager",	&bddDdmManager );
//BOOTENTRY("Transport",  &bddMessenger );
//BOOTENTRY("Timer",      &bddTimer );
