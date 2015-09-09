/* BootTable.h.h -- Use until real Boot data is available
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
 *     12/02/98 Tom Nelson: Changed to dynamic creation
 *
**/

#ifndef __BootTable_H
#define __BootTable_H

#include <String.h>
#include "OsTypes.h"

#define BOOTTABLESIZE		32

class BootEntry {
public:
	char *pszName;
	void *pData;
	U32   sData;

	BootEntry *pNextEntry;
	
	BootEntry(char *_pszName,void *_pData,U32 _sData) {
		pszName = _pszName;
		pData = _pData;
		sData = _sData;
		pNextEntry = NULL;
	}
};


class BootTable {
public:
	static BootEntry *pFirstEntry;
	static BootEntry *pLastEntry;
	static U32 nEntry;

public:
	BootTable()	{ nEntry = 0; pFirstEntry = pLastEntry = NULL; }

	static BOOL Add(char *pszName,void *pData,U32 sData);
	static BootEntry * Find(char *pName);
	static void Dump();
};

#endif	// __BootTable_H
