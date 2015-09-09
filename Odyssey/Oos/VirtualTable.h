/* VirtualTable.h -- PTS to use until real PTS is active
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
//  11/24/98 Tom Nelson: Changed to dynamic creation
//

#ifndef __VirtualTable_H
#define __VirtualTable_H

#include <String.h>
#include "OsTypes.h"
#include "Address.h" 
#include "DeviceId.h"

class VmsVdd  {
public:	
	char *pszClassName;
	VDN vdn;
	TySlot slotPrimary;
	TySlot slotSecondary; 

	VmsVdd(char *_pszClassName,VDN _vdn,TySlot _primary,TySlot _secondary) {
		pszClassName = _pszClassName;
		vdn = _vdn;
		slotPrimary = _primary;
		slotSecondary = _secondary;
	}

};

class VmsName { 
public:
	static VmsVdd *pVdd;
	
	static BOOL SetVdd(char *_pszClassName,VDN _vdn,TySlot _priSlot,TySlot _secSlot) {
		if (pVdd == NULL) {
			pVdd = new VmsVdd(_pszClassName,_vdn,_priSlot,_secSlot);
			return TRUE;
		}
		return FALSE;
	}
	static VmsVdd *GetVdd() { return pVdd; }
};

class VirtualEntry {
public:
	const char *pszClassName;
	BOOL fStart;
	VDN  vdn;
	TySlot slotPrimary;
	TySlot slotSecondary;
	void *pData;
	U32   sData;
	
	const char *pszTableName;
	const void *pFieldDefs;
	U32   sFieldDefs;
	U32   sTableRecord;
	
	VirtualEntry *pNextEntry;
	VirtualEntry *pPrevEntry;
	
	VirtualEntry(const char *_pszName,BOOL _fStart,VDN _vdn,void *_pData,U32 _sData,TySlot ,TySlot,
			     const char *pszTableName,const void *pFieldDefs,U32 sFieldDefs,U32 sTableRecord);
};


class VirtualTable {
public:
	static VirtualEntry *pFirstEntry;
	static VirtualEntry *pLastEntry;
	static U32 nEntry;
	
	
public:
	VirtualTable()	{ nEntry = 0; pFirstEntry = pLastEntry = NULL; }
	
	static BOOL Add(const char *pszClassName,BOOL fStart,VDN vdn,void *pData,U32 sData,TySlot priSlot,TySlot secSlot,
					const char *pszTableName,const void *pFieldDefs,U32 sFieldDefs,U32 sTableRecord);
	static VirtualEntry * Find(VDN vdn);
	static VirtualEntry * Find(TySlot _slot);
	static VirtualEntry * GetFirst();
	static U32 GetCount()			{ return nEntry;	  }

	// Return only entries with config data
	static VirtualEntry * GetFirstData();
	static VirtualEntry * GetNextData(VirtualEntry *pHere);

	static void Dump();

private:
	static BOOL Insert(VirtualEntry *pHere,VirtualEntry *pEntry);
};

#endif	// __VirtualTable_H
