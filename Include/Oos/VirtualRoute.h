/* VirtualRoute.h -- Describes route to a Ddm
 *
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
//  2/02/00 Tom Nelson: Create file
//  ** Log at end-of-file **

// 100 columns ruler
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#ifndef __VirtualRoute_h
#define __VirtualRoute_h

#include "OsTypes.h"

// VirtualRoute Class
//
struct VirtualRoute {
	enum { MAXSERVES = 64 };
		
	VDN vdn;
	DID did;	// Active Did
	DID didAlternate;
	U32 nServes;
	REQUESTCODE aServes[MAXSERVES];
		
	VirtualRoute() : vdn(VDNNULL),did(DIDNULL),didAlternate(DIDNULL),nServes(0) {}
		
	VirtualRoute(VDN _vdn,DID _did, DID _didAlternate, U32 _nServes, const REQUESTCODE *_paReqCodes)
	: vdn(_vdn), did(_did), didAlternate(_didAlternate) {
		SetServes(_nServes,_paReqCodes);
	}
	VirtualRoute(VDN _vdn, DID _did, DID _didAlternate=DIDNULL)
	: vdn(_vdn), did(_did), didAlternate(_didAlternate), nServes(0) {}
		
	void SetRoute(VDN _vdn,DID _did,DID _didAlternate=DIDNULL) {
		vdn = _vdn;
		did = _did;
		didAlternate = _didAlternate;
	}
	void SetRoute(VDN _vdn,DID _did, DID _didAlternate, U32 _nServes, const REQUESTCODE *_paServes) {
		vdn = _vdn;
		did = _did;
		didAlternate = _didAlternate;
		SetServes(_nServes,_paServes);
	}
	void SetServes(U32 _nServes, const REQUESTCODE *_paServes) {
		nServes = _nServes;
		memcpy(aServes,_paServes,nServes * sizeof(REQUESTCODE));
	}	
};

#endif	// __VirtualRoute_h
	
/*************************************************************************/
// Update Log:
//	$Log: /Iowa branch/Include/Oos/VirtualRoute.h $
// 
// 1     2/08/00 7:43p Tnelson
// 
