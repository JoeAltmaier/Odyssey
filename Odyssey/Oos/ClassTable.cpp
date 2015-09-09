/* ClassTable.cpp -- System Class Tables
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
#include "ClassTable.h"

ClassEntry * ClassTable::pFirstEntry = NULL;
ClassEntry * ClassTable::pLastEntry = NULL;
U32 ClassTable::nEntry = 0;

// .Add -- Add Class to System Class Table ------------------------------------ClassTable-
//
// Due to the unreliablity of static initializer call order there may 
// already be an incomplete class entry in the class table.
// 
// No Duplicate Class Names allowed.

BOOL ClassTable::Add(char *pszName,U32 cbStack,U32 sQueue,CtorFunc ctor,ClassFlags flags) {

	ClassEntry *pEntry;;

	if (Find(pszName) != NULL)
		return FALSE;
		
	pEntry = new ClassEntry(pszName,ctor,flags,cbStack,sQueue);
		
	if (pFirstEntry == NULL)
		pFirstEntry = pLastEntry = pEntry;
	else {
		pLastEntry->pNextEntry = pEntry;
		pLastEntry = pEntry;
	}			
	nEntry++;
			
	return TRUE;
}

// .Find -- Find Class in system table by Ctor --------------------------------ClassTable-
//
// Scan Class tables, return pointer to class entry or NULL if not found.
//
ClassEntry * ClassTable::Find(CtorFunc pCtor)
{
	for (ClassEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry)
		if (pCtor == pEntry->ctor)
			return pEntry;
				
	return NULL;
}

// .Find -- Find Class in system table by name --------------------------------ClassTable-
//
// Scan Class tables, return pointer to class entry or NULL if not found.
//
ClassEntry * ClassTable::Find(char *pszName)
{
	for (ClassEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry)
		if (strcmp(pszName, pEntry->pszName) == 0)
			return pEntry;
				
	return NULL;
}

// .Dump -- Dump Serve Tables -------------------------------------------------ServeTable-
//
void ClassTable::Dump(void) {

	ClassEntry *pEntry;
	
	Tracef("Class Table:\n");
	Tracef("    Ctor      sQueue  cbStack  ClassName\n");
	if (pFirstEntry == NULL)
		Tracef("  <empty>\n");
		
	for (pEntry = pFirstEntry; pEntry != NULL; pEntry=pEntry->pNextEntry) {
		Tracef("    %08x %6u   %6u  \"%s\"\n",pEntry->ctor,pEntry->sQueue,pEntry->cbStack,pEntry->pszName);
	}
	Tracef("\n");
}


