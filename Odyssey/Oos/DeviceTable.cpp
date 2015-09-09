/* DeviceTable.cpp -- System Device Tables
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
**/

// Revision History:
// 11/14/98 Tom Nelson: Changed to dynamic tables
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <String.h>
#include "DeviceTable.h"

DeviceEntry * DeviceTable::pFirstEntry = NULL;
DeviceEntry * DeviceTable::pLastEntry = NULL;
U32 DeviceTable::nEntry = 0;

// .Add -- Add Device to System Device Table ---------------------------------DeviceTable-
//
BOOL DeviceTable::Add(char *pszName,InitFunc function)
{
	DeviceEntry *pEntry;;

	pEntry = new DeviceEntry(pszName,function);
		
	if (pFirstEntry == NULL)
		pFirstEntry = pLastEntry = pEntry;
	else {
		pLastEntry->pNextEntry = pEntry;
		pLastEntry = pEntry;
	}			
	nEntry++;
			
	return TRUE;
}

// .Find -- Find Device in system table by name ------------------------------DeviceTable-
//
// Scan Device tables, return pointer to device entry or NULL if not found.
//
DeviceEntry * DeviceTable::Find(char *pszName)
{
	for (DeviceEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry)
		if (strcmp(pszName, pEntry->pszName) == 0)
			return pEntry;
				
	return NULL;
}

// .Start -- Invoke all device initializers ----------------------------------DeviceTable-
//
void DeviceTable::Start()
{		
	for (DeviceEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry) {
//Tracef("start:\"%s\"\n",pEntries->aEntry[iEntry].pszName);		
			(*(pEntry->function))();
	}
}

// .Dump -- Dump data in persistant table ------------------------------------DeviceTable-
//
void DeviceTable::Dump() {

	Tracef("Device Table:\n");
	Tracef("     Function  DeviceName\n");
	if (pFirstEntry == NULL)
		Tracef("  <empty>\n");

	for (DeviceEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry=pEntry->pNextEntry) {
		Tracef("    %08x  \"%s\"\n",pEntry->function,pEntry->pszName);
	}
	Tracef("\n");
}
