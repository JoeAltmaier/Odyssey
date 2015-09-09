/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FCPTrace.h
// 
// Description:
// This file defines trace methods for debugging the FCP code. 
// 
// Update Log 
// 5/27/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 8/18/98 Michael G. Panas: Add tracing with levels
// 9/2/98 Michael G. Panas: add C++ stuff
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/

#if !defined(FCPTrace_H)
#define FCPTrace_H

#include <stdio.h>

#include "Nucleus.h"
#include "FCPData.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


STATUS	FCP_Trace_Create(PINSTANCE_DATA Id);
void	FCP_Trace_Destroy();

void	FCP_Trace_Entry(char* p_module_name);

extern	UNSIGNED FCP_if_trace_on;

/*************************************************************************/
//    FCP_TRACE_ENTRY Macro Definition
//    This macro should be at the entry to each method.
/*************************************************************************/
#if defined(FCP_DEBUG) && defined(_DEBUG)
#define FCP_TRACE_ENTRY(method) \
	if (TraceLevel[TRACE_INDEX] >= TRACE_ENTRY_LVL) FCP_Trace_Entry(#method);  // special case
#else

// If debugging is not turned on, no code is generated
#define FCP_TRACE_ENTRY(method) 
#endif // #ifdef (FCP_DEBUG)
		
/*************************************************************************/
//    FCP_ASSERT Macro Definition
//    TODO include trace here
/*************************************************************************/
#if defined(FCP_DEBUG) && defined(_DEBUG)
#define FCP_ASSERT(condition, module) \
	if (!(condition)) \
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL, \
			module, \
			"Assert (" #condition ") Failed", \
			0, \
			0);
#else

// If debugging is not turned on, no code is generated
#define FCP_ASSERT(condition, module)
#endif // #ifdef (FCP_DEBUG)

/*************************************************************************/
//    FCP_XXXX Trace Macro Definitions
//    These macros are defined for debug tracing with level checking
/*************************************************************************/
#if defined(FCP_DEBUG) && defined(_DEBUG)
#define FCP_PRINT_STRING(level, string) \
	TRACE_STRING(level, string);
	
#define FCP_PRINT_HEX(level, string, number) \
	TRACE_HEX(level, string, number);
	
#define FCP_PRINT_HEX16(level, string, number) \
	TRACE_HEX16(level, string, number);

#define FCP_PRINT_NUMBER(level, string, number) \
	TRACE_NUMBER(level, string, number);

#define FCP_DUMP_HEX(level, string, pointer, length) \
	TRACE_DUMP_HEX(level, string, pointer, length);

#else

// If debugging is not turned on, no code is generated
#define FCP_PRINT_STRING(level, string) 
#define FCP_PRINT_HEX(level, string, number) 
#define FCP_PRINT_HEX16(level, string, number) 
#define FCP_PRINT_NUMBER(level, string, number)
#define FCP_DUMP_HEX(level, string, pointer, length)
#endif // #ifdef (FCP_DEBUG)
		
#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FCPTrace_H  */
