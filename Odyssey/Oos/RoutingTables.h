/* RoutingTables.h -- VirtualDevice/RequestCode Routing Tables Definitions
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
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
//  3/23/99 Tom Nelson: Create file
// ** Log at end-of-file **

#ifndef __RoutingTables_h
#define __RoutingTables_h

#include "Array_t.h"
#include "CtEvent.h"

//***
//*** VirtualDeviceTable
//***

class DeviceIdCode {
public:
	DID did;
	
	DeviceIdCode() : did(DIDNULL) {}

	BOOL IsValid()	{ return did != DIDNULL; }
};

// Quick Fix using Array_T - Only extends table in increments of 16
//
class VirtualDeviceMap {
	Array_T<DeviceIdCode> aTable;	// Range Lookup
public:	
	ERC Set(VDN vdn,DID did) {
		DeviceIdCode dic;
		
		dic.did = did;
		aTable.Set(vdn,dic);
		
		return OK;
	}

	DID Get(VDN vdn) {
		
		if (vdn < (signed)aTable.Size())
			return aTable[vdn].did;

		return DIDNULL;
	}
	// Return VDN of first table entry
	VDN GetFirst() {
		return GetNext(0);
	}
	// Return VDN of next table entry
	VDN GetNext(REQUESTCODE vdnPrev) {
		VDN vdn = vdnPrev + 1;
		
		while (vdn < (signed)aTable.Size()) {
			if (aTable[vdn].IsValid())
				return vdn;
			else
				++vdn;	// Try next VDN
		}
		return VDNNULL;
	}
};


//***
//*** RequestRouteTable
//***

class RequestRouteCodes {
public:
	VDN vdn;
	DID did;
	
	RequestRouteCodes()	: vdn(VDNNULL), did(DIDNULL) {}

	BOOL IsDid()	{ return did != DIDNULL; }
	BOOL IsVdn()	{ return vdn != VDNNULL; }
	BOOL IsValid()	{ return IsDid() || IsVdn(); }
};

class RequestRouteMap {
	Array_T<RequestRouteCodes> aRange[REQUEST_RANGE_LAST];	// Range Lookup
	
	ERC _Set(REQUESTCODE reqCode,RequestRouteCodes& rrc) {
		if ((reqCode >> 16) >= REQUEST_RANGE_LAST)
			return CTS_CHAOS_UNKNOWN_FUNCTION;

		aRange[reqCode >> 16].Set(reqCode & 0xFFFF,rrc);
		
		return OK;
	}

public:
	ERC Set(REQUESTCODE reqCode,VDN vdn) {
		RequestRouteCodes rrc;
		
		rrc.vdn = vdn;
		rrc.did = DIDNULL;

		return _Set(reqCode,rrc);
	}

	ERC Set(REQUESTCODE reqCode,DID did) {
		RequestRouteCodes rrc;
		
		rrc.vdn = VDNNULL;
		rrc.did = did;

		return _Set(reqCode,rrc);
	}
	
	ERC Clear(REQUESTCODE reqCode) {
		RequestRouteCodes rrc;

		return _Set(reqCode,rrc);
	}
	
	RequestRouteCodes *Get(REQUESTCODE reqCode) {
		static RequestRouteCodes rrcNull;

		if ((reqCode >> 16) < REQUEST_RANGE_LAST) {
			if ((reqCode & 0xFFFF) < aRange[reqCode >> 16].Size())
				return &aRange[reqCode>>16][reqCode & 0xFFFF];
		}
		return &rrcNull;
	 }

	// Return RequestCode of first table entry
	REQUESTCODE GetFirst() {
		return GetNext(0);
	}
	// Return RequestCode of next table entry
	REQUESTCODE GetNext(REQUESTCODE reqCodePrev) {
		REQUESTCODE reqCode = reqCodePrev + 1;
		
		while ((reqCode >> 16) < REQUEST_RANGE_LAST) {
			if ((reqCode & 0xFFFF) < aRange[reqCode >> 16].Size()) {
				if (aRange[reqCode>>16][reqCode & 0xFFFF].IsValid() )
					return reqCode;
				else
					++reqCode;
			}
			else
				reqCode = (reqCode & ~0xFFFF) + 0x10000;	// Next range
		}
		return 0;	// End of table
	}
};

#endif	// __RoutingTables_h

//*************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/RoutingTables.h $
// 
// 5     12/09/99 2:10a Iowa
// 
// 4     9/16/99 3:23p Tnelson
// 
