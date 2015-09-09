/* SystemTable.cpp -- System Startup Tables
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
// 11/24/98 Tom Nelson: Created
// ** Log at end-of-file *

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <String.h>
#include "DeviceId.h"
#include "DdmManager.h"
#include "SystemTable.h"


SystemEntry * SystemTable::pFirstEntry = NULL;
SystemEntry * SystemTable::pLastEntry = NULL;
U32 SystemTable::nEntry = 0;

// .Add -- Add data to System Table ----------------------------------------------------SystemTable-
//
BOOL SystemTable::Add(char *pszClassName,VDN vdn)
{
	SystemEntry *pEntry;

	pEntry = new SystemEntry(pszClassName,vdn);
		
	if (pFirstEntry == NULL)
		pFirstEntry = pLastEntry = pEntry;
	else {
		pLastEntry->pNextEntry = pEntry;
		pLastEntry = pEntry;
	}			
	nEntry++;
			
	return TRUE;
}

// .GetFirst -- Get first entry in persistant table ------------------------------------SystemTable-
//
SystemEntry * SystemTable::GetFirst()
{
//Tracef("SystemTable::GetFirst()\n");
	return pFirstEntry;
}

// .Dump -- Dump data in persistant table ----------------------------------------------SystemTable-
//
void SystemTable::Dump() {

	Tracef("System Table:\n");
	if (pFirstEntry == NULL)
		Tracef("  <empty>\n");

	for (SystemEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry=pEntry->pNextEntry) {
		Tracef("    \"%s\"\n",pEntry->pszClassName);
	}
	Tracef("\n");
}

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/SystemTable.cpp $
// 
// 8     2/08/00 8:53p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 9     2/08/00 6:08p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 

