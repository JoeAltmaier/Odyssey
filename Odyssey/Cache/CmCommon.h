/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmCommon.h
// 
// Description:
// This file contains macros and definitions used in all 
// Cache modules.
// 
// Update Log: 
// 9/11/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(CmCommon_H)
#define CmCommon_H

//#pragma warning( disable : 4103)

#include "Cache.h"
#include "ErrorLog.h"

/*************************************************************************/
// Debugging methods defined in CmDebug.cpp.
/*************************************************************************/
void CM_Break();

// When we return an error code, call a method so we can set a breakpoint.
Status CM_Error(Status status);
#define CM_ERROR(code) CM_Error(CM_ERROR_##code)

/*************************************************************************/
//    CM_TRACE_ENTRY Macro Definition
//    This macro should be at the entry to each method.
/*************************************************************************/
#if defined(_DEBUG)
#define CM_TRACE_ENTRY(method) \
	CM_Trace_Entry(#method);
#else

// If debugging is not turned on, no code is generated
#define CM_TRACE_ENTRY(method) 
#endif // #ifdef (_DEBUG)
		
/*************************************************************************/
//    CM_ASSERT Macro Definition
//    TODO include trace here
/*************************************************************************/
#if defined(_DEBUG)
#define CM_ASSERT(condition, module) \
	if (!(condition)) \
		CT_Log_Error(CM_ERROR_TYPE_FATAL, \
			#module, \
			"Assert (" #condition ") Failed", \
			0, \
			0);
#else

// If debugging is not turned on, no code is generated
#define CM_ASSERT(condition, module)
#endif // #ifdef (_DEBUG)
		
#ifdef _DEBUG
// If we run out of memory, call a method so we can set a breakpoint
// and see where we run out of memory.
Status CM_No_Memory();
#define CM_NO_MEMORY CM_No_Memory()
#else
#define CM_NO_MEMORY CM_ERROR_NO_MEMORY
#endif



#endif /* CmCommon_H  */
