 /*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class declared Address data.
// 
// Update Log: 
// 11/11/98 Joe Altmaier: Created
/*************************************************************************/


#define _TRACEF
#include "Trace_Index.h"
#include "odyssey_trace.h"

#include "Address.h"

	TySlot Address::iSlotMe;
	U32 Address::pciBase=0;
	U32 Address::paPci=0;
	U32 Address::sPci=0;
	U32 Address::iCabinet=0;
	TySlot Address::iSlotHbcMaster;
	
	Address::TySlotRoute Address::aSlot[NSLOT];

	/*
		// The slot map.
		printf("\n In Hex:\n");
		printf("13 12 11 10       19 1A 1B 18\n");
		printf("            00 01 \n");
		printf("17 16 15 14       1D 1E 1F 1C\n");
		printf("\n Decimal:\n");
		printf("19 18 17 16       25 26 27 24\n");
		printf("            00 01 \n");
		printf("23 22 21 20       29 30 31 28\n");
		printf("\n English:\n");
		printf("B4 B3 B2 B1       A1 A2 A3 A4\n");
		printf("            00 01 \n");
		printf("D4 D3 D2 D1       C1 C2 C3 C4\n");
		
		// The Slot / Failover Partner Map.
		FOP_HBC1 = IOP_HBC0,	// 0
		FOP_HBCO = IOP_HBC1,	// 1
		FOP_SSDL0 = IOP_SSDU0,	// 16
		FOP_SSDL1 = IOP_SSDU1,	// 17
		FOP_SSDL2 = IOP_SSDU2,	// 18
		FOP_SSDL3 = IOP_SSDU3,	// 19
		FOP_SSDU0 = IOP_SSDL0,	// 20
		FOP_SSDU1 = IOP_SSDL1,	// 21
		FOP_SSDU2 = IOP_SSDL2,	// 22
		FOP_SSDU3 = IOP_SSDL3, 	// 23
		FOP_RAC1 = IOP_RAC0,	// 24
		FOP_APP2 = IOP_APP0,	// 25
		FOP_APP3 = IOP_APP1,	// 26
		FOP_NIC1 = IOP_NIC0,	// 27
		FOP_RAC0 = IOP_RAC1,	// 28
		FOP_APP0 = IOP_APP2,	// 29
		FOP_APP1 = IOP_APP3,	// 30
		FOP_NIC0 = IOP_NIC1,	// 31
	*/

	// Declare and initialize a static IOP to FOP Map.
	TySlot Address::aMapIoptoFop[NSLOT] = {
		FOP_HBCO,		// Slot #1 is FOP for IOP_HBC0
		FOP_HBC1,		// Slot #0 is FOP for IOP_HBC1
		IOP_NONE,		// Slot #2 is unused!
		IOP_NONE,		// Slot #3 is unused!
		IOP_NONE,		// Slot #4 is unused!
		IOP_NONE,		// Slot #5 is unused!
		IOP_NONE,		// Slot #6 is unused!
		IOP_NONE,		// Slot #7 is unused!
		IOP_NONE,		// Slot #8 is unused!
		IOP_NONE,		// Slot #9 is unused!
		IOP_NONE,		// Slot #10 is unused!
		IOP_NONE,		// Slot #11 is unused!
		IOP_NONE,		// Slot #12 is unused!
		IOP_NONE,		// Slot #13 is unused!
		IOP_NONE,		// Slot #14 is unused!
		IOP_NONE,		// Slot #15 is unused!
		FOP_SSDU0,		// Slot #20 is FOP for IOP_SSDU0
		FOP_SSDU1,		// Slot #21 is FOP for IOP_SSDU1
		FOP_SSDU2,		// Slot #22 is FOP for IOP_SSDU2
		FOP_SSDU3,		// Slot #23 is FOP for IOP_SSDU3 
		FOP_SSDL0,		// Slot #16 is FOP for IOP_SSDL0
		FOP_SSDL1,		// Slot #17 is FOP for IOP_SSDL1
		FOP_SSDL2,		// Slot #18 is FOP for IOP_SSDL2
		FOP_SSDL3,		// Slot #19 is FOP for IOP_SSDL3 
		FOP_RAC0,		// Slot #28 is FOP for IOP_RAC0
		FOP_APP0,		// Slot #29 is FOP for IOP_APP0
		FOP_APP1,		// Slot #30 is FOP for IOP_APP1
		FOP_NIC0,		// Slot #31 is FOP for IOP_NIC0
		FOP_RAC1,		// Slot #24 is FOP for IOP_RAC1
		FOP_APP2,		// Slot #25 is FOP for IOP_APP2
		FOP_APP3,		// Slot #26 is FOP for IOP_APP3
		FOP_NIC1		// Slot #27 is FOP for IOP_NIC1
	};
	
	// Declare and initialize a static FOP  to IOP Map.
	TySlot	Address::aMapFoptoIop[NSLOT] = {
		IOP_HBC1,		// Slot #0 is IOP for FOP_HBC1 (0)
		IOP_HBC0,		// Slot #1 is IOP for FOP_HBC0 (1)
		IOP_NONE,		// Slot #2 is unused!
		IOP_NONE,		// Slot #3 is unused!
		IOP_NONE,		// Slot #4 is unused!
		IOP_NONE,		// Slot #5 is unused!
		IOP_NONE,		// Slot #6 is unused!
		IOP_NONE,		// Slot #7 is unused!
		IOP_NONE,		// Slot #8 is unused!
		IOP_NONE,		// Slot #9 is unused!
		IOP_NONE,		// Slot #10 is unused!
		IOP_NONE,		// Slot #11 is unused!
		IOP_NONE,		// Slot #12 is unused!
		IOP_NONE,		// Slot #13 is unused!
		IOP_NONE,		// Slot #14 is unused!
		IOP_NONE,		// Slot #15 is unused!
		IOP_SSDU0,		// Slot #16 is IOP for FOP_SSDU0
		IOP_SSDU1,		// Slot #17 is IOP for FOP_SSDU1
		IOP_SSDU2,		// Slot #18 is IOP for FOP_SSDU2
		IOP_SSDU3,		// Slot #19 is IOP for FOP_SSDU3 
		IOP_SSDL0,		// Slot #20 is IOP for FOP_SSDL0
		IOP_SSDL1,		// Slot #21 is IOP for FOP_SSDL1
		IOP_SSDL2,		// Slot #22 is IOP for FOP_SSDL2
		IOP_SSDL3,		// Slot #23 is IOP for FOP_SSDL3 
		IOP_RAC0,		// Slot #24 is IOP for FOP_RAC0
		IOP_APP0,		// Slot #25 is IOP for FOP_APP0
		IOP_APP1,		// Slot #26 is IOP for FOP_APP1
		IOP_NIC0,		// Slot #27 is IOP for FOP_NIC0
		IOP_RAC1,		// Slot #28 is IOP for FOP_RAC1
		IOP_APP2,		// Slot #29 is IOP for FOP_APP2
		IOP_APP3,		// Slot #30 is IOP for FOP_APP3
		IOP_NIC1		// Slot #31 is IOP for FOP_NIC1
	};
