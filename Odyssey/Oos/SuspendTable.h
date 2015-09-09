/* SuspendTable.h -- System Failure Tables (Private to Chaos.lib)
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

#ifndef __SuspendTable_h
#define __SuspendTable_h


#include <String.h>
#include "Os.h"

class SuspendEntry {
public:
	char *pszName;
	U32  cbStack;
	InitFunc suspend;
	InitFunc resume;
	
	SuspendEntry *pNextEntry;
	
	SuspendEntry(char *_pszName,InitFunc _suspend, InitFunc _resume) {
		pszName = _pszName;
		suspend = _suspend;
		resume = _resume;
		pNextEntry = NULL; 
	}
};

class SuspendTable {
public:
	static SuspendEntry *pFirstEntry;
	static SuspendEntry *pLastEntry;
	static U32 nEntry;

public:
	static BOOL Add(char *pszName,InitFunc suspend,InitFunc resume);
	static SuspendEntry * Find(char *pszName);

	static void Suspend(void);
	static void Resume(void);

	static void Dump(void);
};

#endif	// __SuspendTable_H
