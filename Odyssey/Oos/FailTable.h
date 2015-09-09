/* FailTable.h -- System Failure Tables (Private to Chaos.lib)
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
 *     08/25/99 Joe Altmaier: Created from DeviceTable.h
 *
**/

#ifndef __FailTable_h
#define __FailTable_h


#include <String.h>
#include "Os.h"

class FailEntry {
public:
	char *pszName;
	U32  cbStack;
	InitFunc function;
	
	FailEntry *pNextEntry;
	
	FailEntry(char *_pszName,InitFunc _function) {
		pszName = _pszName;
		function = _function;
		pNextEntry = NULL; 
	}
};

class FailTable {
public:
	static FailEntry *pFirstEntry;
	static FailEntry *pLastEntry;
	static U32 nEntry;

public:
	static BOOL Add(char *pszName,InitFunc function);
	static FailEntry * Find(char *pszName);

	static void Fail(void);

	static void Dump(void);
};

#endif	// __FailTable_H
