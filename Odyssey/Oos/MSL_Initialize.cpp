/* MSL_Initialize
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
 *     03/10/99 Joe Altmaier: MSL_Initialize take only pointer.  Get sizes from globals.
 *		Return next available memory.
 *
**/
#include "Odyssey_Trace.h"

#include "OsTypes.h"
#include <__rts_info_t__.h>
#include "Galileo.h"
#include "Address.h"
#include "DeviceId.h"
#include "Os.h"

extern "C" void __call_static_initializers(void (**si)(void));
extern "C" void OsHeap_Initialize(U32 nFragment, U32 aPa[], U32 aCb[], U32 nReserved, U32 aPaRes[], U32 aCbRes[], U32 cbHeapSmall);
extern "C" void bcopy(void *pFrom, void *pTo, int cb);
extern "C" Status Init_Hardware();

extern "C" long long bootblockp;
extern "C" bootblock_t bootblock;
#ifndef _ODYSSEY
extern "C" U32 gSize_total_memory;
#endif
extern "C" U32 gSize_small_heap;
extern U32 gSizeReserved;
extern "C" char _ftext[];			/* 1st location of .text	*/

extern "C" void Init_Address();
extern "C" void *MSL_Initialize(void *first_available_memory);

U32 aPa[20];
U32 aCb[20];
U32 aPaRes[20];
U32 aCbRes[20];


extern "C" void *MSL_Initialize(void *first_available_memory) {
#ifdef _ODYSSEY
	
	Init_Hardware();
	
	Init_Address();

	// The system heap must be initialized first, before any other static initializers.

	// Build a list of reserved memory.
	int nReserved=3 + (gSizeReserved? 1 :0);

	// Pci slave memory is reserved
	aPaRes[0] = memmaps.paSlave;
	aCbRes[0] = memmaps.cbSlave;
	
	// 1MB from zero belongs to boot rom
	aPaRes[1]=0;
	aCbRes[1]=0x00100000;

	// Operating system code
	aPaRes[2]=PA_P(_ftext);
	aCbRes[2]=((char*)first_available_memory - _ftext);

	// Any reserved memory
	if (gSizeReserved) {
		aPaRes[3]=PA_P(first_available_memory);
		aCbRes[3]=gSizeReserved;
		}

	// Copy memmap fragment list to static array so there is room to expand.
	bcopy(&memmaps.aPa[0], &aPa[1], memmaps.nFragment * sizeof(aPa[0]));
	bcopy(&memmaps.aCb[0], &aCb[1], memmaps.nFragment * sizeof(aCb[0]));

	// Pci slave memory is 1st fragment.
	aPa[0]=memmaps.paSlave;
	aCb[0]=memmaps.cbSlave;

	OsHeap_Initialize(memmaps.nFragment+1, aPa, aCb, nReserved, aPaRes, aCbRes, ((bootblock.b_cbHeap && bootblock.b_cbHeap != 0xDEADDEAD)? bootblock.b_cbHeap : gSize_small_heap));

#else	// eval
	// Pci slave memory is 1st fragment.
	aPa[0]=PA_P((U32)first_available_memory + gSizeReserved);
	aCb[0]=gSize_small_heap * 3;

	// Available memory is 2nd fragment
	aPa[1]=aPa[0] + aCb[0];
	aCb[1]=gSize_total_memory - aPa[1];

	// Build a list of reserved memory.
	int nReserved=3 + (gSizeReserved? 1 :0);

	// Pci slave memory is reserved
	aPaRes[0] = aPa[0];
	aCbRes[0] = aCb[0];
	
	// 1MB from zero belongs to boot rom
	aPaRes[1]=0;
	aCbRes[1]=0x00100000;

	// Operating system code
	aPaRes[2]=PA_P(_ftext);
	aCbRes[2]=((char*)first_available_memory - _ftext);

	// Any reserved memory
	if (gSizeReserved) {
		aPaRes[3]=PA_P(first_available_memory);
		aCbRes[3]=gSizeReserved;
		}

	OsHeap_Initialize(2, aPa, aCb, nReserved, aPaRes, aCbRes, gSize_small_heap);
#endif

	__call_static_initializers(__static_init);

	// Some memory left over for debugging.
	return first_available_memory;
	}
	

extern "C" void Init_Address() {

#ifdef _ODYSSEY

//		Address::iSlotMe = (TySlot)memmaps.iSlot;
		Address::iSlotMe = (TySlot)((bootblock_t *)bootblockp)->b_slot;
		Tracef("got add::slot = %d\n", Address::iSlotMe);
		Address::pciBase = memmaps.pciSlave;
		Address::sPci    = memmaps.cbSlave;
		Address::paPci   = memmaps.paSlave;
		Address::iSlotHbcMaster = (TySlot) bootblock.b_iSlotHbcMaster;

// Initialization done in Init_Hardware in inithd.c,pcimap.c
		// Exception if PCI cycle doesn't complete in limited time.  Default was FFFF.
//		*(U32*)(GT_PCI_RETRY_REG) = GTREG(0x0200);
#else
		// Eval boards

		// Get config info from boot tables
		struct CabSlot {U32 iCabinet; TySlot iSlot;} *pCabSlot = (CabSlot*)Oos::GetBootData("DdmManager");
		Address::iCabinet = pCabSlot->iCabinet;
		Address::iSlotMe = pCabSlot->iSlot;
		Address::sPci    = 0x01000000;
		Address::paPci   = 0;
		Address::iSlotHbcMaster = IOP_HBC0;

//	    *(U32*)(GT_CPU_CONFIG_REG) = GTREG(0x10000); // DDDD write mode
	    *(U32*)(GT_CPU_CONFIG_REG) = GTREG(0x00000); // DXDXDXDX write mode
#define	GT_PCI_BAR_ENABLE_REG			((U32 *)(GALILEO_BASE_ADDR+0xC3C))
	    *(GT_PCI_BAR_ENABLE_REG) = GTREG(0x00EF);

	    /* check our own pci device and vendor id (IDSEL0) */
		*(U32*)(GT_PCI_CONFIG_ADDR_REG) = GTREG(0x80000000);     /* 80000000 = IDSEL0 cfg - device and vendor id */

		if (*(U32*)(GT_PCI_CONFIG_DATA_REG) != GTREG(0x462011AB))/* 462011AB = GT-64120                          */
			{
	        Tracef("ERROR - Galileo 64120 not responding correctly: %lx\n", *(U32*)(GT_PCI_CONFIG_DATA_REG));
    	    return;
			}

		// Program bus sync
//	    *(U32*)(GT_PCI_COMMAND_REG) = GTREG(0x0000);
	    *(U32*)(GT_PCI_COMMAND_REG) = GTREG(0x000A);	/* Tclk/Pclk 75/33 */

		// Exception if PCI cycle doesn't complete in limited time.  Default was FFFF.
		*(U32*)(GT_PCI_RETRY_REG) = GTREG(0x0200);

	    /* enable ourselves as pci bus master/slave */
	    *(U32*)(GT_PCI_CONFIG_ADDR_REG) = GTREG(0x80000004);     /* 80000004 = IDSEL0 cfg - status and command   */
		*(U32*)(GT_PCI_CONFIG_DATA_REG) = GTREG(0x00000006);     /* 00000006 = enable pci bus master/slave mode  */
//	    *(U32*)(GT_PCI_CONFIG_DATA_REG) = GTREG(0x00000007);     /* 00000006 = enable pci bus master/slave mode and i/o  */

	    /* set cache line size to 8 (program 7) */
	    // Sudhir says just 0
    	*(U32*)(GT_PCI_CONFIG_ADDR_REG) = GTREG(0x8000000C);
//		*(U32*)(GT_PCI_CONFIG_DATA_REG) = GTREG(0x00000207);
		*(U32*)(GT_PCI_CONFIG_DATA_REG) = GTREG(0x00000000);

    	/* set internal registers base address */
	    *(U32*)(GT_PCI_CONFIG_ADDR_REG) = GTREG(0x80000020);
		*(U32*)(GT_PCI_CONFIG_DATA_REG) = GTREG(0x14000000);

		// PCI master
		// Program master window low, high
		*(U32*)(GT_PROC_PCI_MEM_LO_REG) = GTREG(pTransportParams->pciWindowLo >> 21);
		*(U32*)(GT_PROC_PCI_MEM_HI_REG) = GTREG(((pTransportParams->pciWindowHi-1) >> 21) & 0x3f);
	
		// PCI slave
		// Program BAR
		*(U32*)(GT_PCI_CONFIG_ADDR_REG) = GTREG(GT_PCI_CONFIG_CYCLE + GT_PCICONF_RAS10_BASE_OFF);
		*(U32*)(GT_PCI_CONFIG_DATA_REG) = GTREG(Address::pciBase + 8);
		// Program SIZE

		*(U32*)(GT_PCI_RAS10_BSIZE_REG) = GTREG(Address::sPci-1);
		// Program MAP so pci memory accesses go to SDRAM
		*(U32*)(GT_PCI_RAS10_MAP_REG) = GTREG(DRAM0_BASE_PHYS_ADDR);

		NU_Control_Interrupts(0xA400);
/*
		Interrupt::SetHandler(Interrupt::tySlvWrErr0, Int_PciError, Interrupt::tySlvWrErr0);
		Interrupt::SetHandler(Interrupt::tySlvRdErr0, Int_PciError, Interrupt::tySlvRdErr0);
		Interrupt::SetHandler(Interrupt::tyRetryCtr0, Int_PciTimeout, Interrupt::tyRetryCtr0);
 -- poll these interrupts
*/
#endif
	}
