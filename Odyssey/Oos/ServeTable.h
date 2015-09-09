/* ServeTable.h -- System Serve Tables (Private to Oos.lib)
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

#ifndef __ServeTable_h
#define __ServeTable_h


#include <String.h>
#include "OsTypes.h"
#include "Message.h"

class ServeItem {
public:
	REQUESTCODE reqCode;
	ServeItem *pNextItem;	

	ServeItem(REQUESTCODE _reqCode) { reqCode = _reqCode; pNextItem = NULL; }
};

class ServeEntry {
public:
	CtorFunc   ctor;
	ServeItem *pServeLocal;
	U32 nServeLocal;
	ServeItem *pServeVirtual;
	U32 nServeVirtual;
	ServeEntry *pNextEntry;
	
	ServeEntry(CtorFunc _ctor) { ctor = _ctor; pNextEntry = NULL; pServeLocal = pServeVirtual = NULL; nServeLocal = nServeVirtual = 0; }
	void AddItem(REQUESTCODE reqCode,BOOL fLocal);

private:
	void LinkLocal(ServeItem *pItem)  { if (pServeLocal != NULL) pItem->pNextItem = pServeLocal; pServeLocal = pItem; nServeLocal++; }
	void LinkVirtual(ServeItem *pItem) { if (pServeVirtual != NULL) pItem->pNextItem = pServeVirtual; pServeVirtual = pItem; nServeVirtual++; }
};
	
class ServeTable {	
public:
	static ServeEntry *pFirstEntry;
	static ServeEntry *pLastEntry;
	static U32 nEntry;
		
public:
	static BOOL Add(CtorFunc ctor,REQUESTCODE reqCode,BOOL fVirtual);
	static ServeEntry * Find(CtorFunc pCtor);
	static void Dump(void);
};

#endif	// __ServeTable_H
