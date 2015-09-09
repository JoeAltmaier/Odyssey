/* ServeTable.cpp -- System Serve Tables
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
//  2/17/99 Tom Nelson: Created
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <String.h>
#include "ServeTable.h"
#include "ClassTable.h"

ServeEntry* ServeTable::pFirstEntry = NULL;
ServeEntry* ServeTable::pLastEntry = NULL;
U32 ServeTable::nEntry = 0;

// .Add -- Add Serve Entry to System Serve Table ------------------------------ServeTable-
//
BOOL ServeTable::Add(CtorFunc ctor,REQUESTCODE reqCode,BOOL fLocal)
{
	ServeEntry *pEntry;;

	if ((pEntry = Find(ctor)) == NULL) {
		pEntry = new ServeEntry(ctor);
		
		if (pFirstEntry == NULL)
			pFirstEntry = pLastEntry = pEntry;
		else {
			pLastEntry->pNextEntry = pEntry;
			pLastEntry = pEntry;
		} 
		nEntry++;
	}
	pEntry->AddItem(reqCode,fLocal);
	
	return TRUE;
}


// .Find -- Find Serve in system table by Ctor --------------------------------ServeTable-
//
// Scan Serve tables, return pointer to serve entry or NULL if not found.
//
ServeEntry * ServeTable::Find(CtorFunc pCtor)
{
	for (ServeEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry)
		if (pCtor == pEntry->ctor)
			return pEntry;
				
	return NULL;
}

// .AddItem -- Add Serve Entry to System Serve Table --------------------------ServeEntry-
//
void ServeEntry::AddItem(REQUESTCODE reqCode,BOOL fLocal)
{
	ServeItem *pItem = new ServeItem(reqCode);
	
	if (fLocal)
		LinkLocal(pItem);
	else
		LinkVirtual(pItem);
}

// .Dump -- Dump Serve Tables -------------------------------------------------ServeTable-
//
void ServeTable::Dump(void) {

	ServeEntry *pEntry;
	ClassEntry *pClass;
	ServeItem *pLocal,*pVirtual;
	CtorFunc ctor;
	
	Tracef("Serve Table:\n");
	if (pFirstEntry == NULL)
		Tracef("  <empty>\n");
	else {
		Tracef("    Ctor     Local Serves  Virtual Serves  Class Name\n");
		for (pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry) {
			ctor = pEntry->ctor;
			pLocal = pEntry->pServeLocal;
			pVirtual = pEntry->pServeVirtual;

			do {
				if (ctor != NULL)
					Tracef("    %08x",ctor);
				else
					Tracef("       \"    ");
					
				if (pLocal != NULL) {
					Tracef("  0x%08x",pLocal->reqCode);
					pLocal = pLocal->pNextItem;
				}
				else
					Tracef("            ");
			
				if (pVirtual != NULL) {
					Tracef("     0x%08x ",pVirtual->reqCode);
					pVirtual = pVirtual->pNextItem;
				}
				else
					Tracef("                ");
			
				if (ctor != NULL) {
					pClass = ClassTable::Find(ctor);
					Tracef("  \"%s\"",pClass == NULL ? "Unknown" : pClass->pszName);
					ctor = NULL;
				}
				else
					Tracef("      \"");
				
				Tracef("\n");
			}
			while (pLocal != NULL || pVirtual != NULL);
		}
	}
	Tracef("\n");
}


