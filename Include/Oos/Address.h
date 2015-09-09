/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Address.h
// 
// Description:
// This module defines address space conversion macros.
// 
// Update Log: 
// 11/11/98 Joe Altmaier: Create file
// 3/23/99 Joe Altmaier: ISLOCAL_PCI
// 3/31/99 Joe Altmaier: ODYSSEY build
// 4/20/99 Joe Altmaier: WIN32 build
// 6/03/99 Eric Wedel: Updated TySlot per current PCI & CMB defs.
/*************************************************************************/

#ifndef __Address_h
#define __Address_h

#include "Simple.h"



//  the values in CmbPacket.bDestAddr and .bSrcAddr are TySlot enum members
//  The TySlot enumeration is used with the CMB, as well as to identify
//  PCI slots.  On the PCI, addresses refer to slots.  On the CMB, addresses
//  refer to slot/processor pairs, where the processor may be either the
//  slot's MIPS (for HBCs and IOPs) or the slot's AVR microcontroller (all
//  CMB slots).  To distinguish between the two processor targets, the CMB
//  uses the bit 7 of the TySlot value to select processor.  When the bit
//  is set, the address is for the slot's MIPS CPU, when clear it is for
//  the slot's AVR CPU.
#define  CMB_ADDR_MIPS_FLAG   (0x80)      // OR in for CMB MIPS CPU addresses


//  maximum possible PCI slot addresses.  Note that TySlot (see below)
//  defines addresses greater than this value.  But they are valid only
//  on the Card Management Bus, and not on PCI.
#define NSLOT 32

// Define unused parameter types
#define	NOSLOT	((TySlot)-1)	// need this for secondary devices

//  TySlot defines both the physical slot numbers of IOP boards, and the
//  addresses used in CMB packets to access the Atmel Microcontrollers
//  on the various IOPs, DDHs, EVCs and other intelligent FRUs.  In short,
//  it provides slot addresses for both the PCI and Card Management busses.
//  The IOP_* identifiers are used on both PCI and CMB, while the CMB_*
//  identifiers are used only on the CMB.  When forming CMB addresses,
//  refer also to CMB_ADDR_MIPS_FLAG, defined above.
enum TySlot {
		IOP_HBC0=0, IOP_HBC1=1, 
		IOP_UNUSED_LO = 2,
		IOP_UNUSED_HI = 15,
		IOP_SSDU0, IOP_SSDU1, IOP_SSDU2, IOP_SSDU3, 
		IOP_SSDL0, IOP_SSDL1, IOP_SSDL2, IOP_SSDL3, 
		IOP_RAC0, IOP_APP0, IOP_APP1, IOP_NIC0,
		IOP_RAC1, IOP_APP2, IOP_APP3, IOP_NIC1,
		
		// Fail Over Partners' slot (FOP_slot) for each Iop (IOP_slot).
		FOP_HBCO = IOP_HBC1,
		FOP_HBC1 = IOP_HBC0, 
		FOP_SSDU0 = IOP_SSDL0,
		FOP_SSDU1 = IOP_SSDL1,
		FOP_SSDU2 = IOP_SSDL2,
		FOP_SSDU3 = IOP_SSDL3, 
		FOP_SSDL0 = IOP_SSDU0,
		FOP_SSDL1 = IOP_SSDU1,
		FOP_SSDL2 = IOP_SSDU2,
		FOP_SSDL3 = IOP_SSDU3, 
		FOP_RAC0 = IOP_RAC1,
		FOP_APP0 = IOP_APP2,
		FOP_APP1 = IOP_APP3,
		FOP_NIC0 = IOP_NIC1,
		FOP_RAC1 = IOP_RAC0,
		FOP_APP2 = IOP_APP0,
		FOP_APP3 = IOP_APP1,
		FOP_NIC1 = IOP_NIC0,
				
      CMB_EVC0 = 0x20, CMB_EVC1,
      CMB_DDH0 = 0x22, CMB_DDH1, CMB_DDH2, CMB_DDH3,
      CMB_ADDRESS_MAX,        // Max number of CMB adressable thangies.
      CMB_MASTER_MIPS=0xFB,   // CMB pseudo-dest, means master HBC's MIPS CPU
      CMB_NOTIFY=0xFC,        // CMB pseudo-dest, means unsolicited CMB packet
      CMB_SELF=0x7D,          // CMB pseudo-dest, to send to own CMA
      CMB_BROADCAST=0x7E,     // CMB pseudo-dest, broadcast to all CMAs

      IOP_LOCAL=CMB_SELF | CMB_ADDR_MIPS_FLAG,  // Local IOP Slot (0xFD)
      /* 0xFE reserved for future use */
      IOP_NONE=0xFF			// No slot
	};


struct Address {

	enum StateIop {IOP_RESET=0, IOP_BOOTING, IOP_ACTIVE, IOP_STANDBY, IOP_FAILED};

	// Slot descriptors
	typedef struct {
		U32 pciBase;
		U32 syncLast;
		StateIop state;
	} TySlotRoute;

	struct StateSigCount {
		union {
			U32 l;
			struct {
				U8 state:8;
				U8 sig:8;
				U16 cMsg:16;
			} ssc;
		};
	enum {SIGNATURE=0xD2};
	};

	//** Members - These should really be private
	//**
	
	// Slot decoding
	static TySlotRoute aSlot[NSLOT];
	
	static TySlot iSlotMe;

	static TySlot iSlotHbcMaster;

	static U32 iCabinet;
		
	static U32 pciBase;
		
	static U32 sPci;
		
	static U32 paPci;

	// aMapIoptoFop - Maps IOP slot # to Fail Over Partner (FOP) slot #.
	static TySlot aMapIoptoFop[NSLOT];
	
	// aMapFoptoIop - Maps Fail Over Partner (FOP) slot # to IOP slot #.
	static TySlot aMapFoptoIop[NSLOT];

	//** Accessor Methods
	//**
	static TySlot GetHbcMasterSlot()	{ return iSlotHbcMaster;		}
	static TySlot GetSlot() 			{ return iSlotMe;  				}
	static TySlot GetSlotFop() 			{ return GetFopForIop(iSlotMe); }
	static U32 GetCabinet() 			{ return iCabinet; 				}
	static TySlot GetFopForIop(TySlot Iop)	{ return aMapIoptoFop[Iop];	}
	static TySlot GetIopForFop(TySlot Fop)	{ return aMapFoptoIop[Fop];	}
};


#ifdef WIN32
// Make physical address (pa or pci) from p 
#define PA_P(p) (U32)(p)

// Make pointer from physical address
#define P_PA(pa) (void*)(pa)

// Make cached pointer from physical address
#define PC_PA(pa) (void*)(pa)

#define PCI_WINDOW_BASE 0x00000000


#else // MIPS

#include "pcimap.h"
#include "BootBlock.h"

extern "C" bootblock_t bootblock;

// Translate between address spaces
// pa - physical address to local memory
// pci - physical address to pci window
// p - mips pointer with 8 (kseg0) or a (kseg1) on top, could reference pa or pci

#define USEG_BASE_ADDR 0

#ifdef _ODYSSEY
// pciBase 0x80000000 and up
// paPci   0x0
// sPci    0x04000000 or 0x10000000

#define PCI_WINDOW_BASE 0x80000000

// Make pci address into MIPS pointer to pci address
#define P_PCI(pci) (void*)(pci - PCI_WINDOW_BASE + USEG_BASE_ADDR)

// Make physical address (pa or pci) from p 
#define PA_P(p) (U32)( (U32)(p) < KSEG0_BASE_ADDR ? (U32)(p) - USEG_BASE_ADDR + PCI_WINDOW_BASE \
		: (U32)(p) < KSEG1_BASE_ADDR ? (U32)(p) - KSEG0_BASE_ADDR \
		: (U32)(p) < K2BASE ? (U32)(p) - KSEG1_BASE_ADDR \
		: (U32)(p) - K2BASE + memmaps.aPa[1])

// Make pointer from physical address
#define P_PA(pa) ((void*)( (pa) >= PCI_WINDOW_BASE ? (pa) - PCI_WINDOW_BASE + USEG_BASE_ADDR \
			: (pa) < memmaps.aPa[1] ? (pa) - memmaps.aPa[0] + memmaps.aP[0] \
			: (pa) - memmaps.aPa[1] + memmaps.aP[1] \
			))


#else
// Eval boards
// pciBase 0x02000000 or 0x03000000
// paPci   0x0
// sPci    0x01000000

#define PCI_WINDOW_BASE 0x02000000

// Make pci address into MIPS pointer to pci address
#define P_PCI(pci) (void*)((pci) | KSEG1_BASE_ADDR)

// Make physical address (pa or pci) from p 
#define PA_P(p) ((U32)(p) & 0x1FFFFFFF)

// Make pointer from physical address
#define P_PA(pa) ((void*)((pa) | KSEG1_BASE_ADDR))

#endif


// Make cached pointer from physical address
#ifdef _NO_CACHE
#define PC_PA(pa) (void*)((pa) | KSEG1_BASE_ADDR)
#else
#define PC_PA(pa) (void*)((pa) | KSEG0_BASE_ADDR)
#endif

#endif	// !WIN32



// Make pci address into MIPS pointer to local memory
// pci address must point to our pci window
#define PLOCAL_PCI(pci) P_PA(PALOCAL_PCI(pci))

// Find local physical address that corresponds to pci address
#define PALOCAL_PCI(pci) ( (pci) - Address::pciBase + Address::paPci )

// Make pci address from local physical address
#define PCI_PALOCAL(pa) ( (pa) - Address::paPci + Address::pciBase )

// Is pci a local reference?
#define ISLOCAL_PCI(pci) ((pci) && (pci) >= Address::pciBase \
						&& (pci) < Address::pciBase + Address::sPci)
						
// Is pa a local physical address?
#define IS_PALOCAL(pa) (pa && (pa < PCI_WINDOW_BASE))

// See if local physical address is in pci window
#define IS_PCI(pa) ((pa) >= Address::paPci && (pa) < Address::paPci + Address::sPci )

#define PUNCACHED(p) (p? P_PA(PA_P(p)) :(void*)p)
#define PCACHED(p) (p? PC_PA(PA_P(p)) :(void*)p)

#define IS_CACHED(p) (((U32)p - KSEG0_BASE_ADDR) < (KSEG1_BASE_ADDR - KSEG0_BASE_ADDR))
#endif
