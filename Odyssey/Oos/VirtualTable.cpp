/* VirtualTable.cpp -- PTS to use until real PTS is active
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
// 11/24/98 Tom Nelson: Changed to dynamic creation
//  5/11/99 Eric Wedel: Changed -1 to IDNULL in DeviceId::Did() calls [GH].
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "odyssey_trace.h"

#include <String.h>
#include "VirtualTable.h"
#include "DdmManager.h"

VirtualEntry * VirtualTable::pFirstEntry = NULL;
VirtualEntry * VirtualTable::pLastEntry = NULL;
U32 VirtualTable::nEntry = 0;

VmsVdd * VmsName::pVdd;

// VirtualEntry -- Constructor ----------------------------------------------VirtualEntry-
//
// Can't be in VirtualTable.h since DdmManager.h causes circular include!
//
VirtualEntry::VirtualEntry(const char *_pszName,BOOL _fStart,VDN _vdn,void *_pData,U32 _sData,TySlot _priSlot,TySlot _secSlot,
					   	   const char *_pszTableName,const void *_pFieldDefs,U32 _sFieldDefs, U32 _sTableRecord)
	: pszClassName(_pszName),pszTableName(_pszTableName),pFieldDefs(_pFieldDefs)
{ 
	pNextEntry = pPrevEntry = NULL;
	
	//pszClassName =_pszName;
	fStart = _fStart;
	vdn = _vdn;
	pData = _pData;
	sData = _pData != NULL ? _sData : 0;
	slotPrimary = _priSlot;
	slotSecondary = _secSlot;
	
	//pszTableName =  _pszTableName;
	//pFieldDefs = _pFieldDefs;
	sFieldDefs = _sFieldDefs;
	sTableRecord = _sTableRecord;
}

// .Insert -- Insert into Virtual Table ------------------------------------VirtualTable-
//
// Insert into list before pHere
// If pHere == NULL then link to end of list
//
BOOL VirtualTable::Insert(VirtualEntry *_pHere,VirtualEntry *_pEntry) {
	
	nEntry++;
	
	// End of List
	if (_pHere == NULL) {
		if (pLastEntry == NULL)
			pFirstEntry = pLastEntry = _pEntry;
		else {
			_pEntry->pPrevEntry = pLastEntry;
			pLastEntry->pNextEntry = _pEntry;
			pLastEntry = _pEntry;
		}
	}
	else { // Insert into list
		_pEntry->pNextEntry = _pHere;
		_pEntry->pPrevEntry = _pHere->pPrevEntry;

		if (_pHere->pPrevEntry == NULL)
			pFirstEntry = _pEntry;
		else {
			_pHere->pPrevEntry->pNextEntry = _pEntry;
			_pHere->pPrevEntry = _pEntry;
		}	
		_pHere->pPrevEntry = _pEntry;
	}		
	return TRUE;
}

// .Add -- Add data to Virtual Table ---------------------------------------VirtualsTable-
//
// Must be sorted by assending VDN
//
BOOL VirtualTable::Add(const char *pszClassName,BOOL fStart,VDN vdn,void *pData,U32 sData,TySlot priSlot,TySlot secSlot,
					   const char *pszTableName,const void *pFieldDefs,U32 sFieldDefs,U32 sTableRecord)	// static
{
	VirtualEntry *pHere;
	VirtualEntry *pEntry;

	pEntry = new VirtualEntry(pszClassName,fStart,vdn,pData,sData,priSlot,secSlot,pszTableName,pFieldDefs,sFieldDefs,sTableRecord);
	
	// Insert into list
	for (pHere = pFirstEntry; pHere != NULL; pHere = pHere->pNextEntry) {
		if (pHere->vdn > pEntry->vdn)
			return Insert(pHere,pEntry);
	}
	return Insert(pHere,pEntry);
}

// .Find -- Find VirtualEntry in table by vdn -------------------------------VirtualTable-
//
VirtualEntry * VirtualTable::Find(VDN vdn)
{
//Tracef("VirtualTable::Find(VDN %lx)\n",vdn);

	for (VirtualEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry=pEntry->pNextEntry) {
		if (pEntry->vdn == vdn)
			return pEntry;
	}
			
	return NULL;
}

// .Find -- Find VirtualEntry in table by slot ------------------------------VirtualTable-
//
VirtualEntry * VirtualTable::Find(TySlot _slot)
{
//Tracef("VirtualTable::Find(TySlot %lx)\n",_slot);

	for (VirtualEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry=pEntry->pNextEntry) {
		if (pEntry->slotPrimary == _slot || pEntry->slotSecondary == _slot)
			return pEntry;
	}
			
	return NULL;
}


// .GetFirst -- Get first entry in persistant table -------------------------VirtualTable-
//
VirtualEntry * VirtualTable::GetFirst()
{
//Tracef("VirtualTable::GetFirst()\n");
	return pFirstEntry;
}

// .GetFirstData -- Get first entry with config data -------------------------VirtualTable-
//
VirtualEntry * VirtualTable::GetFirstData()
{
	VirtualEntry *pHere;
	
	for (pHere = pFirstEntry; pHere != NULL && (pHere->sData == 0 || pHere->pFieldDefs == NULL); pHere=pHere->pNextEntry) ;

	return pHere;
}

// .GetNextData -- Get next entry with config data --------------------------VirtualTable-
//
VirtualEntry * VirtualTable::GetNextData(VirtualEntry *pHere)
{
	for (pHere = pHere->pNextEntry; pHere != NULL && (pHere->sData == 0 || pHere->pFieldDefs == NULL); pHere=pHere->pNextEntry) ;

	return pHere;
}

// .Dump -- Dump data in persistant table -----------------------------------VirtualTable-
//
void VirtualTable::Dump() {

	Tracef("Virtual Table\n");
	if (pFirstEntry == NULL)
		Tracef("    <empty>\n");
	else {
		Tracef("     Vdn  Start  Slot1    Slot2    pData   sData sFieldDefs TableName     ClassName\n");

		for (VirtualEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry=pEntry->pNextEntry) {
			Tracef("    %4u  %s %08x %08x %08x %5u     %4u     \"%12s\" \"%s\"\n",
					pEntry->vdn,pEntry->fStart ? " YES " : " NO  ", pEntry->slotPrimary,pEntry->slotSecondary,
					pEntry->pData,pEntry->sData,pEntry->sFieldDefs,(pEntry->pszTableName ? pEntry->pszTableName : ""),
					pEntry->pszClassName);
		}
		Tracef("\n");
	}
}

