/* DeviceId.cpp -- Cabinet/Slot/Id
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 * Copyright (C) Dell Computer, 2000
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/
 
// Revision History: 
// 6/15/98 Joe Altmaier: Create file
//  ** Log at end-of-file **

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include "DeviceId.h"
#include "Messenger.h"
#include "DdmManager.h"


	// Determine if slot number is a processor
	BOOL DeviceId::fIop_iSlot[NSLOT]={
		true, true, false, false, false, false, false, false, 
		false, false, false, false, false, false, false, false, 
		true, true, true, true, true, true, true, true,
		true, true, true, true, true, true, true, true
	};

	// Every slot reachable from every other slot (PCI slots only)
	TySlot DeviceId::iSlot_iSlot[NSLOT]={
		IOP_HBC0, IOP_HBC1,
		// reserved slot IDs
		(TySlot)-1, (TySlot)-1, (TySlot)-1, (TySlot)-1, (TySlot)-1, (TySlot)-1, 
		(TySlot)-1, (TySlot)-1, (TySlot)-1, (TySlot)-1,
		(TySlot)-1, (TySlot)-1, (TySlot)-1, (TySlot)-1,
      // IOP slots (non-HBC)
		IOP_SSDU0, IOP_SSDU1, IOP_SSDU2, IOP_SSDU3, 
		IOP_SSDL0, IOP_SSDL1, IOP_SSDL2, IOP_SSDL3, 
		IOP_RAC0, IOP_APP0, IOP_APP1, IOP_NIC0,
		IOP_RAC1, IOP_APP2, IOP_APP3, IOP_NIC1
	};

	// Return name of processor
	char *DeviceId::st_Slot[] = {
		"HBC0", "HBC1", "", "", "", "","", "",
        "", "", "", "", "", "", "", "",
		"SDDU0", "SDDU1", "SDDU2", "SDDU3", 
		"SDDL0", "SDDL1", "SDDL2", "SDDL3", 
		"RAC0", "APP0", "APP1", "NIC0", 
		"RAC1", "APP2", "APP3", "NIC1"
	};

	BOOL DeviceId::IsLocal() {
		return (ICabinet() == Address::GetCabinet() && ISlot() == Address::GetSlot());
	}
		
	DeviceId::DeviceId(DID did): did(did) {
	}
	
	DID DeviceId::Did(U32 iCabinet, TySlot iSlot, U32 idLocal) {
#ifdef _WIN32
		return ((U32)iCabinet << 29) | ((U32)iSlot << 24) | idLocal;
#else
		union {
			DID did;
			struct {
				unsigned int iCabinet:3;
				TySlot iSlot:5;
				unsigned int idLocal:24;
			} idb;
		};
		idb.iCabinet = (unsigned char) iCabinet;
		idb.iSlot = iSlot;
		idb.idLocal = idLocal;
		return did;
#endif
	}

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DeviceId.cpp $
// 
// 13    2/06/00 2:11p Tnelson
// Update comments
//
// 8/24/99 Joe Altmaier: fPcimap_iSlot -> fIop_iSlot
// 4/26/99 Jerry Lane:   Replace bitfield union code with #ifndef _WIN32.
//						 The new code breaks the MIPS build.
// 4/21/99 Joe Altmaier: Remove bitfield union code - it's not endian-proof.
// 6/03/99 Eric Wedel:   Updated DeviceId::iSlot_iSlot and DeviceId::st_Slot
//                       to reflect revised TySlot definition.
// 6/15/98 Joe Altmaier: Create file
