/* SystemTable.h -- System Class Tables (Private to Oos.lib)
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
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
//     11/14/98 Tom Nelson: Created
// ** Log at end-of-file **

#ifndef __SystemTable_h
#define __SystemTable_h


#include <String.h>
#include "OsTypes.h"
#include "VirtualRoute.h"

class SystemEntry {
public:
	char *pszClassName;
	VDN  vdn;
	
	BOOL fActive;
	VirtualRoute route;	// Created route
	
	SystemEntry *pNextEntry;
	
	SystemEntry(char *_pszClassName,VDN _vdn) 
	: vdn(_vdn),pszClassName(_pszClassName), pNextEntry(NULL),fActive(FALSE) {}
	
	void SetRoute(const VirtualRoute &_route) {
		route = _route;
	}
};


class SystemTable {	
public:
	static SystemEntry *pFirstEntry;
	static SystemEntry *pLastEntry;
	static U32 nEntry;

public:
	SystemTable()	{ nEntry = 0; pFirstEntry = pLastEntry = NULL; }
	
	static BOOL Add(char *pszClassName,VDN _vdn=VDNNULL);
	static SystemEntry * GetFirst();
	static SystemEntry * GetNext(SystemEntry *pLink) {
		return pLink->pNextEntry;
	}
	static SystemEntry * GetNextActive(SystemEntry *pLink) {
		while ((pLink = GetNext(pLink)) != NULL) {
			if (pLink->fActive)
				return pLink;
		}		
		return pLink;
	}
	static void Dump();
};

#endif	// __SystemTable_H

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/SystemTable.h $
// 
// 5     2/08/00 8:54p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 6     2/08/00 6:08p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
