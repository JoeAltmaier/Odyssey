/* DeviceTable.h -- System Device Tables (Private to Oos.lib)
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
 *     11/14/98 Tom Nelson: Created
 *
**/

#ifndef __DeviceTable_h
#define __DeviceTable_h


#include <String.h>
#include "Os.h"

class DeviceEntry {
public:
	char *pszName;
	U32  cbStack;
	InitFunc function;
	
	DeviceEntry *pNextEntry;
	
	DeviceEntry(char *_pszName,InitFunc _function) { 
		pszName = _pszName;
		function = _function;
		pNextEntry = NULL; 
	}
};

class DeviceTable {
public:
	static DeviceEntry *pFirstEntry;
	static DeviceEntry *pLastEntry;
	static U32 nEntry;

public:
	static BOOL Add(char *pszName,InitFunc function);
	static DeviceEntry * Find(char *pszName);

	static void Start(void);

	static void Dump(void);
};

#endif	// __DeviceTable_H
