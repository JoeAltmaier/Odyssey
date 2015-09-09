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
// This class is the DeviceId structure, which helps in constructing
// and decomposing a did into cabinet, slot and local id.
// 
// Update Log: 
// 6/12/98 Joe Altmaier: Create file
// 6/15/98 Joe Altmaier: Created DeviceId.cpp, remove code from here to there.
// 9/29/98 Jim Frandeen: Use CtTypes.h instead of stddef.h
// 4/21/99 Joe Altmaier: Remove bitfield union code - it's not endian-proof.
// 8/24/99 Joe Altmaier: fPcimap_iSlot -> fIop_iSlot
/*************************************************************************/

#ifndef __DeviceId_h
#define __DeviceId_h

#include "OsTypes.h"
#include "Address.h"

#define IDLOCALMAX 4096
#define IDNULL 0x0FFFFFF
#define St_Slot(iSlot) DeviceId::st_Slot[iSlot]


class DeviceId {
	DID did;
	
	BOOL IsLocal();

public:
	static
	BOOL fIop_iSlot[NSLOT];
	static
	TySlot iSlot_iSlot[NSLOT];
	static
	char *st_Slot[];

public:
	DeviceId(DID did);
	
	DID Did() const { return did; }
	U32 IdLocal() const { return did & 0x00FFFFFF; }
	U32 ICabinet() const { return did >> 29; }
	TySlot ISlot() const { return (TySlot)((did >> 24) & 0x1F); }

	static
	BOOL IsLocal(DID did) {
		return ISlot(did) == Address::GetSlot() && ICabinet(did) == Address::GetCabinet();
	}
	
	static
	U32 IdLocal(DID didArg) { 
		return (U32) (didArg & 0x00FFFFFF);
		}

	static
	TySlot ISlot(DID didArg) { 
		return (TySlot) ( (didArg >> 24) & 0x1F );
	}

	static 
	U32 ICabinet(DID didArg) {
		return didArg >> 29;
	}
	
	BOOL IsValid() const { return (ICabinet() == 0 && fIop_iSlot[ISlot()] && IdLocal() < IDLOCALMAX); }

	static
	BOOL IsValid(DID didArg) { 
		return fIop_iSlot[(didArg >> 24) &0x1F] && ( (didArg & 0xE0FFFFFF) < IDLOCALMAX );
		}
		
	static
	DID Did(U32 iCabinet, TySlot iSlot, U32 idLocal);
};
#endif


