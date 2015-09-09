/* Os.cpp -- OS Initializations
 *
 * Copyright ConvergeNet Technologies (c) 1998 
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

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_OOS
#include "Odyssey_Trace.h"

#include "OsTypes.h"
#include "Os.h"

#include "DdmManager.h"
#include "DeviceTable.h"
#include "FailTable.h"
#include "SuspendTable.h"
#include "ClassTable.h"
#include "ServeTable.h"
#include "SystemTable.h"
#include "TestTable.h"
//#include "PtsTable.h"
#include "BootTable.h"
#include "VirtualTable.h"

#ifndef WIN32
	#include "Transport.h"
#endif


// Initialize -- Initialize OS ------------------------------------------------------------------Os-
//
void Os::Initialize(void)	// static
{
	DeviceTable::Start();

	DdmManager::Start();
}
	
// .MakeRefNum -- Make a unique reference number ------------------------------------------------Os-
//
// Zero is an invalid RefNum.
//
REFNUM Os::MakeRefNum() {	// static

	static I64 idLocal = 0;
	
	union { 
		REFNUM num;
		struct {
			DID did;
			U32 idLocal;
		} part;
	} ref;
	
	ref.part.did = DeviceId::Did(Address::GetCabinet(),Address::GetSlot(),0);
	ref.part.idLocal = 0;

	return ref.num | (++idLocal);
}

// AddServeEntry -- Add request code to serve table ---------------------------------------------Os-
//
BOOL Os::AddServeEntry(CtorFunc pCtor,REQUESTCODE reqCode,BOOL fLocal)	// static
{
	return ServeTable::Add(pCtor,reqCode,fLocal);
}

// AddClassEntry -- Add Class Name to system class table ----------------------------------------Os-
//
BOOL Os::AddClassEntry(char *pName,U32 cbStack,U32 sQueue,CtorFunc pCtor,ClassFlags flags)
{
	if (!ClassTable::Add(pName,cbStack,sQueue,pCtor,flags)) {
		Tracef("[WARNING] Duplicate entry in ClassEntry Table (Os::AddClassEntry)\n");
		return FALSE;
	}
	return TRUE;
}

// AddDeviceEntry -- Add Device Function to system device table ---------------------------------Os-
//
BOOL Os::AddDeviceEntry(char *pName,InitFunc entry)	// static
{
	return DeviceTable::Add(pName,entry);
}

// AddFailEntry -- Add Fail Function to system fail table ---------------------------------------Os-
//
BOOL Os::AddFailEntry(char *pName,InitFunc pFail)	// static
{
	return FailTable::Add(pName,pFail);
}

// AddSuspendEntry -- Add Suspend Function to system suspend table ------------------------------Os-
//
BOOL Os::AddSuspendEntry(char *pName,InitFunc pSuspend,InitFunc pResume)	// static
{
	return SuspendTable::Add(pName,pSuspend,pResume);
}

// AddSystemEntry -- Add Entry to System Startup Table ------------------------------------------Os-
//
BOOL Os::AddSystemEntry(char *pName,VDN vdn)	// static
{
	return SystemTable::Add(pName,vdn);
}

// AddTestEntry -- Add Entry to the Test Subsystem Table  ---------------------------------------Os-
//
BOOL Os::AddTestEntry(char *pName,char *pArgs,DID did,TySlot slot)
{
	return TestTable::Add(pName,pArgs,did,slot);
}

#if 0
// AddPtsEntry -- Add Entry to Pts table --------------------------------------------------------Os-
//
BOOL Os::AddPtsEntry(char *pszClassName,BOOL fStart,VDN vdn,void *pData,U32 sData,TySlot priSlot,TySlot secSlot) // static
{
	return PtsTable::Add(pszClassName,fStart,vdn,pData,sData,priSlot,secSlot);
}
#endif

// AddVirtualEntry -- Add Entry to Virtual Device table -----------------------------------------Os-
//
BOOL Os::AddVirtualEntry(char *pszClassName,BOOL fStart,VDN vdn,void *pData,U32 sData,TySlot priSlot,TySlot secSlot,
					   const char *pszTableName,const void *pFieldDefs,U32 sFieldDefs,U32 sTableRecord)	// static
{
	return VirtualTable::Add(pszClassName,fStart,vdn,pData,sData,priSlot,secSlot,pszTableName,pFieldDefs,sFieldDefs,sTableRecord);
}

// AddBootEntry -- Add Entry to Boot Configuration table ----------------------------------------Os-
//
BOOL Os::AddBootEntry(char *pszBootName,void *pData,U32 sData)	// static
{
	return BootTable::Add(pszBootName,pData,sData);
}

// GetBootData -- Get Data from Boot Configuration table ----------------------------------------Os-
//
void * Os::GetBootData(char *pszBootName)	// static
{
	BootEntry *pBe=BootTable::Find(pszBootName);
	
	return (pBe? pBe->pData :NULL);
}
#if 0
// SetVmsName -- Set VMS Name -------------------------------------------------------------------Os-
//
BOOL Os::SetVmsName(char *pszClassName,VDN vdn,TySlot priSlot,TySlot secSlot) 	// static
{
	if (VmsName::GetVdd() != NULL) {
		Tracef("[WARNING] Multiple VMSNAME macros in BuildSys (Os::SetVmsName)\n");
		return FALSE;
	}
	VmsName::SetVdd(pszClassName,vdn,priSlot,secSlot);
	
	return TRUE;
}

// SetPtsName -- Set PTS Name -------------------------------------------------------------------Os-
//
BOOL Os::SetPtsName(char *pszClassName,VDN vdn,TySlot priSlot,TySlot secSlot) 	// static
{
	if (PtsName::GetVdd() != NULL) {
		Tracef("[WARNING] Multiple PTSNAME macros in BuildSys (Os::SetPtsName)\n");
		return FALSE;
	}
	PtsName::SetVdd(pszClassName,vdn,priSlot,secSlot);
	
	return TRUE;
}
#endif
// Fail -- Fail this IOP gracefully -------------------------------------------------------------Os-
//
void Os::Fail() {
#ifndef WIN32
	FailTable::Fail();
#endif
}

// Suspend -- Suspend bus traffic ---------------------------------------------------------------Os-
//
void Os::Suspend() {
#ifndef WIN32
	SuspendTable::Suspend();
#endif
}


// Resume -- Resume bus traffic -----------------------------------------------------------------Os-
//
void Os::Resume() {
#ifndef WIN32
	SuspendTable::Resume();
#endif
}


// DumpTables -- Dump Static Tables -------------------------------------------------------------Os-
//
void Os::DumpTables(char *pTitle) {

	Tracef("%s\n",pTitle);
	
	DeviceTable::Dump();
	FailTable::Dump();
	ClassTable::Dump();
	ServeTable::Dump();
	SystemTable::Dump();
	VirtualTable::Dump();
	BootTable::Dump();
	FailTable::Dump();
	SuspendTable::Dump();
}

// DumpOsMaps -- Dump OS maintained maps --------------------------------------------------------Os-
//
void Os::DumpOsMaps(char *pTitle) {
	
	Tracef("%s\n",pTitle);
	
	DdmManager::DumpMaps();
}

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/Os.cpp $
// 
// 21    2/11/00 1:51p Rbraun
// Added TestTables
// 
// 20    2/09/00 3:08p Tnelson
// Removed references to unused include files.  No code changes.
// 
// 19    2/08/00 8:54p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 20    2/08/00 6:08p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
//  9/02/99 Joe Altmaier: Add SuspendTable
//  2/26/99 Tom Nelson:   Renamed module Os (was Oos)
//  2/17/99 Tom Nelson:   Added AddServeEntry(...)
// 11/14/98 Tom Nelson:   Added dynamic device/class tables
// 10/23/98 Tom Nelson:   Added Startup() & HeapBlock class
// 	7/21/98 Joe Altmaier: Add Initialize(...iCab, iSlot)
// 	7/20/98 Joe Altmaier: Add Interrupt initialization.  Reorder dma init.
// 	7/13/98 Joe Altmaier: messenger.Initialize takes memory
// 	6/12/98 Joe Altmaier: Create file as OOS

