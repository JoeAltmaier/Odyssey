/* BootTable.cpp -- Use until real Boot data is available
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
// 12/02/98 Tom Nelson: Changed to dynamic creation
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <String.h>
#include "DeviceId.h"
#include "DdmManager.h"
#include "BootTable.h"

BootEntry * BootTable::pFirstEntry = NULL;
BootEntry * BootTable::pLastEntry = NULL;
U32 BootTable::nEntry = 0;

// .Add -- Add data to System Boot Table ---------------------------------------BootTable-
//
BOOL BootTable::Add(char *pszName,void *pData,U32 sData)
{
	BootEntry *pEntry;

	pEntry = new BootEntry(pszName,pData,sData);
		
	if (pFirstEntry == NULL)
		pFirstEntry = pLastEntry = pEntry;
	else {
		pLastEntry->pNextEntry = pEntry;
		pLastEntry = pEntry;
	}			
	nEntry++;
			
	return TRUE;
}

// .Find -- Find data in system boot table by name -----------------------------BootTable-
//
// Scan boots tables, return pointer to class entry or NULL if not found.
//
BootEntry * BootTable::Find(char *pszName)
{
	for (BootEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry)
		if ( strcmp(pEntry->pszName,pszName) == 0)
			return pEntry;
				
	return NULL;
}

// .Dump -- Dump data in persistant table --------------------------------------BootTable-
//
void BootTable::Dump() {

	Tracef("Boot Table:\n");
	Tracef("     pData    sData  BootName\n");
	if (pFirstEntry == NULL)
		Tracef("  <empty>\n");

	for (BootEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry=pEntry->pNextEntry) {
		Tracef("    %08x  %5u \"%s\"\n",pEntry->pData,pEntry->sData,pEntry->pszName);
	}
}
