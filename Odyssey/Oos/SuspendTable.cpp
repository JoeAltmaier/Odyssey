/* SuspendTable.cpp -- System Suspend Tables
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
// 08/25/99 Joe Altmaier: Created from FailTable.cpp
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <String.h>
#include "SuspendTable.h"

SuspendEntry * SuspendTable::pFirstEntry = NULL;
SuspendEntry * SuspendTable::pLastEntry = NULL;
U32 SuspendTable::nEntry = 0;

// .Add -- Add Suspend to System Suspend Table ---------------------------SuspendTable-
//
BOOL SuspendTable::Add(char *pszName,InitFunc suspend,InitFunc resume)
{
	SuspendEntry *pEntry;;

	pEntry = new SuspendEntry(pszName,suspend,resume);
		
	if (pFirstEntry == NULL)
		pFirstEntry = pLastEntry = pEntry;
	else {
		pLastEntry->pNextEntry = pEntry;
		pLastEntry = pEntry;
	}			
	nEntry++;
			
	return TRUE;
}

// .Find -- Find Suspend in system table by name -----------------------------SuspendTable-
//
// Scan Suspend tables, return pointer to Suspend entry or NULL if not found.
//
SuspendEntry * SuspendTable::Find(char *pszName)
{
	for (SuspendEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry)
		if (strcmp(pszName, pEntry->pszName) == 0)
			return pEntry;
				
	return NULL;
}

// .Start -- Invoke all Suspend functions ------------------------------------SuspendTable-
//
void SuspendTable::Suspend()
{		
	for (SuspendEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry) {
//Tracef("start:\"%s\"\n",pEntries->aEntry[iEntry].pszName);		
			(*(pEntry->suspend))();
	}
}

// .Start -- Invoke all Resume functions -------------------------------------SuspendTable-
//
void SuspendTable::Resume()
{		
	for (SuspendEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry) {
//Tracef("start:\"%s\"\n",pEntries->aEntry[iEntry].pszName);		
			(*(pEntry->resume))();
	}
}

// .Dump -- Dump data in persistant table ------------------------------------SuspendTable-
//
void SuspendTable::Dump() {

	Tracef("Suspend Table:\n");
	Tracef("     Suspend  Resume    SuspendName\n");
	if (pFirstEntry == NULL)
		Tracef("  <empty>\n");

	for (SuspendEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry=pEntry->pNextEntry) {
		Tracef("    %08x  %08x  \"%s\"\n",pEntry->suspend,pEntry->resume,pEntry->pszName);
	}
	Tracef("\n");
}
