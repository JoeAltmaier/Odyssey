/* BuildSys.h -- Build System Tables
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
//	11-14-98 Tom Nelson: Created
// ** Log at end of file **

#ifndef _BuildSys_H
#define _BuildSys_H

#include "Os.h"

// Must be nested 2 deep to get __LINE__ to expand! //

#define DUMMYNAME3(x,y)				x##y
#define DUMMYNAME2(x,y)				DUMMYNAME3(x,y)
#define DUMMYNAME(x)				DUMMYNAME2(x,__LINE__)


// Macros for Ddm/Driver modules
//
#define CLASSNAME(name,options)		CtorFunc class##name = name::Ctor; ClassFlags flags##name = options

#define SERVELOCAL(name,reqCode)	static BOOL DUMMYNAME(serve) = Os::AddServeEntry(name::Ctor,reqCode,TRUE)

#define SERVEVIRTUAL(name,reqCode)	static BOOL DUMMYNAME(serve) = Os::AddServeEntry(name::Ctor,reqCode,FALSE)

#define DEVICENAME(name,function)	InitFunc device##name = function

#define FAILNAME(name,function)		InitFunc fail##name = function

#define SUSPENDNAME(name,suspendFunc,resumeFunc)		InitFunc suspend##name = suspendFunc; InitFunc resume##name = resumeFunc

// Macros for Buildsys.cpp module
//
#define CLASSENTRY(literal,cbStack,sQueue,name)	extern CtorFunc class##name; extern ClassFlags flags##name; \
						static BOOL DUMMYNAME(dummyClass) = Os::AddClassEntry(literal,cbStack,sQueue,class##name,flags##name)

#define DEVICEENTRY(literal,name)	extern InitFunc device##name;  \
						static BOOL DUMMYNAME(dummyDevice) = Os::AddDeviceEntry(literal,device##name)

#define FAILENTRY(literal,name)	extern InitFunc fail##name;  \
						static BOOL DUMMYNAME(dummyFail) = Os::AddFailEntry(literal,fail##name)

#define SUSPENDENTRY(literal,name)	extern InitFunc suspend##name;  \
						extern InitFunc resume##name;  \
						static BOOL DUMMYNAME(dummySuspend) = Os::AddSuspendEntry(literal,suspend##name,resume##name)

//#define SYSTEMENTRY(pszClassName)	static BOOL DUMMYNAME(dummySysEntry) = Os::AddSystemEntry(pszClassName,VDNNULL)

#define STARTDEVICE(pszClassName)		static BOOL DUMMYNAME(dummySysEntry) = Os::AddSystemEntry(pszClassName)
#define STARTMASTER(pszClassName,vdn)	static BOOL DUMMYNAME(dummySysEntry) = Os::AddSystemEntry(pszClassName,vdn)

#define SYSTEMENTRY(pszClassName)		STARTDEVICE(pszClassName)
#define SYSTEMMASTER(pszClassName,vdn)	STARTMASTER(pszClassName,vdn)

//#define PTSPROXY(pszClassName,vdn,primary,secondary)	static BOOL DUMMYNAME(dummyPtsProxy) = Os::SetPtsProxy(pszClassName,vdn,primary,secondary)

//#define PTSNAME(pszClassName,vdn,primary,secondary)	static BOOL DUMMYNAME(dummyPtsName) = Os::SetPtsName(pszClassName,vdn,primary,secondary)

//#define VMSNAME(pszClassName,vdn,primary,secondary)	static BOOL DUMMYNAME(dummyPtsName) = Os::SetVmsName(pszClassName,vdn,primary,secondary)

class NoPtsTable {
public:
	static char* TableName()	 { return NULL; }
	static void* FieldDefs()	 { return NULL; }
	static U32   FieldDefsSize() { return 0;	}
};

#define NoPtsData	NULL

#define VIRTUALENTRY(pszClassName,fStart,vdn,primary,secondary,configClass,pData)	\
						static BOOL DUMMYNAME(dummyVirtEntry) = Os::AddVirtualEntry(pszClassName,fStart,vdn, pData, sizeof(configClass),primary,secondary, \
															configClass::TableName(),(void*)configClass::FieldDefs(),configClass::FieldDefsSize(),sizeof(configClass))


#define BOOTENTRY(pszBootName,pData)	static BOOL DUMMYNAME(dummyBoot) = Os::AddBootEntry(pszBootName,pData,sizeof(*pData))

#endif	// _BuildSys_H

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Include/Oos/BuildSys.h $
// 
// 20    2/08/00 8:44p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// 
// 21    2/08/00 6:12p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 19    12/09/99 1:37a Iowa
// 
// 17    10/14/99 4:14a Iowa
// Iowa merge
// 
// 16    9/16/99 3:03p Tnelson
// Support for PTS
//
//	8/30/99 Tom Nelson: Added VMSNAME Macros
//  8/28/99 Joe Altmaier: Added FailName/FailEntry macros
//	4/16/99 Tom Nelson: Added PtsEntry macro
//	2/19/99 Tom Nelson: Added Serve macros
//  1/27/99 Tom Nelson: Added sQueue in ClassEntry macro
// 11/14/98 Tom Nelson: Created

