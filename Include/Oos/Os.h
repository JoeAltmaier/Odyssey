/* Os.h -- OS Initializations
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
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
 * Description:
 *	 This class is an interface to the OS which hides some of the
 *   implementation details and associated OS includes.
 *
**/

// Revision History:
//  6/12/98 Joe Altmaier: Create file
// ** Log at end-of-file **

#ifndef __Os_H
#define __Os_H

#include "OsTypes.h"
#include "Address.h"

class Os {
public:
	static void Initialize(void);

	static REFNUM MakeRefNum();
	static BOOL AddServeEntry(CtorFunc pCtor,REQUESTCODE reqCode,BOOL fVirtual);
	static BOOL AddClassEntry(char *pName,U32 cbStack,U32 sQueue,CtorFunc pCtor,ClassFlags flags);
	static BOOL AddDeviceEntry(char *pName,InitFunc pCtor);
	static BOOL AddSystemEntry(char *pszClassName,VDN vdn=VDNNULL);
	static BOOL AddTestEntry(char *pName,char *pArgs,DID did,TySlot slot);	
	static BOOL AddFailEntry(char *pName,InitFunc pFail);
	static BOOL AddSuspendEntry(char *pName,InitFunc pSuspend,InitFunc pResume);
	static BOOL AddVirtualEntry(char *pszClassName,BOOL fStart,VDN vdn,void *pData,U32 sData,TySlot priSlot,TySlot secSlot,
							   const char *pszTableName,const void *pFieldDefs,U32 sFieldDefs,U32 sTableRecord);
	static BOOL AddBootEntry(char *pszBootName,void *pData,U32 sData);
	static void * GetBootData(char *pszBootName);
	
	static void Fail();
	static void Suspend();
	static void Resume();

	static void DumpTables(char *pTitle);
	static void DumpOsMaps(char *pTitle);
};

typedef Os Oos;		// Oos name is OBSOLETE

#endif	// __Os_H

//********************************************************************************************
// Update Log:
//	$Log: /Gemini/Include/Oos/Os.h $
// 
// 15    2/11/00 1:49p Rbraun
// Added TestTable support
// 
// 14    2/08/00 8:49p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 15    2/08/00 6:12p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 13    12/09/99 1:38a Iowa
// 
// 12    10/14/99 4:16a Iowa
// Iowa merge
// 
// 11    9/16/99 3:04p Tnelson
// Support for PTS
//
//	6/28/99 Joe Altmaier: Added Fail()
//  6/25/99 Tom Nelson:	  Added DumpOsMaps()
//  4/16/99 Tom Nelson:   Added AddPtsEntry()
//  4/14/99 Tom Nelson:   Added MakeRefNum()
//	3/28/99 Tom Nelson:   Added flags to ClassName
//  2/22/99 Tom Nelson:   Renamed module Os.h (was Oos.h)
//  2/17/99 Tom Nelson:   Added ServeEntry(...)
// 11/14/98 Tom Nelson:   Added dynamic device/class tables
//  7/21/98 Joe Altmaier: Add Initialize(...iCab, iSlot)
//  7/20/98 Joe Altmaier: Add Interrupt initialization.  
//  7/13/98 Joe Altmaier: messenger.Initialize takes memory
