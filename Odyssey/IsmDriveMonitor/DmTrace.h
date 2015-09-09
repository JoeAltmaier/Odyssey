/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmTrace.h
// 
// Description:
// This file defines trace methods for debugging the DriveMonitor code.
// NOTE: There are no methods for this header, all is done in macros.
// 
// Update Log 
// 10/14/98 Michael G. Panas: Create file
// 01/24/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
/*************************************************************************/

#if !defined(DmTrace_H)
#define DmTrace_H

//#include "Trace.h"		// Trace interface
#include "Nucleus.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif



/*************************************************************************/
//    DM_TRACE_ENTRY Macro Definition
//    This macro should be at the entry to each method.
/*************************************************************************/
#if defined(DM_DEBUG) && defined(_DEBUG)
#define DM_TRACE_ENTRY(method) \
	if (TraceLevel[TRACE_INDEX] >= TRACE_ENTRY_LVL) TRACE_ENTRY(#method);  // special case
#else

// If debugging is not turned on, no code is generated
#define DM_TRACE_ENTRY(method) 
#endif // #ifdef (DM_DEBUG)
		
/*************************************************************************/
//    DM_ASSERT Macro Definition
/*************************************************************************/
#if defined(DM_DEBUG) && defined(_DEBUG)
#define DM_ASSERT(condition, module) \
	if (!(condition)) \
		DM_Log_Error(DM_ERROR_TYPE_FATAL, \
			module, \
			"Assert (" #condition ") Failed", \
			0, \
			0);
#else

// If debugging is not turned on, no code is generated
#define DM_ASSERT(condition, module)
#endif // #ifdef (DM_DEBUG)

/*************************************************************************/
//    DM_XXXX Trace Macro Definitions
//    These macros are defined for debug tracing with level checking
/*************************************************************************/
#if defined(DM_DEBUG) && defined(_DEBUG)
#define DM_PRINT_STRING(level, string) \
	TRACE_STRING(level, string);
	
#define DM_PRINT_HEX(level, string, number) \
	TRACE_HEX(level, string, number);
	
#define DM_PRINT_HEX16(level, string, number) \
	TRACE_HEX16(level, string, number);

#define DM_PRINT_NUMBER(level, string, number) \
	TRACE_NUMBER(level, string, number);

#define DM_PRINT_NUMBER_EOL(level, string, number) \
	TRACE_NUMBER_EOL(level, string, number);

#define DM_DUMP_HEX(level, string, pointer, length) \
	TRACE_DUMP_HEX(level, string, pointer, length);

#else

// If debugging is not turned on, no code is generated
#define DM_PRINT_STRING(level, string) 
#define DM_PRINT_HEX(level, string, number) 
#define DM_PRINT_HEX16(level, string, number) 
#define DM_PRINT_NUMBER(level, string, number)
#define DM_PRINT_NUMBER_EOL(level, string, number)
#define DM_DUMP_HEX(level, string, pointer, length)
#endif // #ifdef (DM_DEBUG)
		
#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* DmTrace_H  */
