/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SmTrace.h
// 
// Description:
// This file defines trace methods for debugging the ScsiMonitor code.
// NOTE: There are no methods for this header, all is done in macros.
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiMonitor/SmTrace.h $ 
// 
// 1     10/11/99 5:35p Cchan
// HSCSI (QL1040B) version of the Drive Monitor
//
/*************************************************************************/

#if !defined(SmTrace_H)
#define SmTrace_H

//#include "Trace.h"		// Trace interface
#include "Nucleus.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif



/*************************************************************************/
//    SM_TRACE_ENTRY Macro Definition
//    This macro should be at the entry to each method.
/*************************************************************************/
#if defined(SM_DEBUG) && defined(_DEBUG)
#define SM_TRACE_ENTRY(method) \
	if (TraceLevel[TRACE_INDEX] >= TRACE_ENTRY_LVL) TRACE_ENTRY(#method);  // special case
#else

// If debugging is not turned on, no code is generated
#define SM_TRACE_ENTRY(method) 
#endif // #ifdef (SM_DEBUG)
		
/*************************************************************************/
//    SM_ASSERT Macro Definition
/*************************************************************************/
#if defined(SM_DEBUG) && defined(_DEBUG)
#define SM_ASSERT(condition, module) \
	if (!(condition)) \
		SM_Log_Error(SM_ERROR_TYPE_FATAL, \
			module, \
			"Assert (" #condition ") Failed", \
			0, \
			0);
#else

// If debugging is not turned on, no code is generated
#define SM_ASSERT(condition, module)
#endif // #ifdef (SM_DEBUG)

/*************************************************************************/
//    SM_XXXX Trace Macro Definitions
//    These macros are defined for debug tracing with level checking
/*************************************************************************/
#if defined(SM_DEBUG) && defined(_DEBUG)
#define SM_PRINT_STRING(level, string) \
	TRACE_STRING(level, string);
	
#define SM_PRINT_HEX(level, string, number) \
	TRACE_HEX(level, string, number);
	
#define SM_PRINT_HEX16(level, string, number) \
	TRACE_HEX16(level, string, number);

#define SM_PRINT_NUMBER(level, string, number) \
	TRACE_NUMBER(level, string, number);

#define SM_PRINT_NUMBER_EOL(level, string, number) \
	TRACE_NUMBER_EOL(level, string, number);

#define SM_DUMP_HEX(level, string, pointer, length) \
	TRACE_DUMP_HEX(level, string, pointer, length);

#else

// If debugging is not turned on, no code is generated
#define SM_PRINT_STRING(level, string) 
#define SM_PRINT_HEX(level, string, number) 
#define SM_PRINT_HEX16(level, string, number) 
#define SM_PRINT_NUMBER(level, string, number)
#define SM_PRINT_NUMBER_EOL(level, string, number)
#define SM_DUMP_HEX(level, string, pointer, length)
#endif // #ifdef (SM_DEBUG)
		
#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* SmTrace_H  */
