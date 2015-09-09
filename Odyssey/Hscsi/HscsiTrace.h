/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiTrace.h
// 
// Description:
// This file defines trace methods for debugging the HSCSI code. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiTrace.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HSCSITrace_H)
#define HSCSITrace_H

#include <stdio.h>

#include "Nucleus.h"
#include "HSCSIData.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


STATUS	HSCSI_Trace_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Trace_Destroy();

void	HSCSI_Trace_Entry(char* p_module_name);

extern	UNSIGNED HSCSI_if_trace_on;

/*************************************************************************/
//    HSCSI_TRACE_ENTRY Macro Definition
//    This macro should be at the entry to each method.
/*************************************************************************/
#if defined(HSCSI_DEBUG) && defined(_DEBUG)
#define HSCSI_TRACE_ENTRY(method) \
	if (TraceLevel[TRACE_INDEX] >= TRACE_ENTRY_LVL) HSCSI_Trace_Entry(#method);  // special case
#else

// If debugging is not turned on, no code is generated
#define HSCSI_TRACE_ENTRY(method) 
#endif // #ifdef (HSCSI_DEBUG)
		
/*************************************************************************/
//    HSCSI_ASSERT Macro Definition
//    TODO include trace here
/*************************************************************************/
#if defined(HSCSI_DEBUG) && defined(_DEBUG)
#define HSCSI_ASSERT(condition, module) \
	if (!(condition)) \
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL, \
			module, \
			"Assert (" #condition ") Failed", \
			0, \
			0);
#else

// If debugging is not turned on, no code is generated
#define HSCSI_ASSERT(condition, module)
#endif // #ifdef (HSCSI_DEBUG)

/*************************************************************************/
//    HSCSI_XXXX Trace Macro Definitions
//    These macros are defined for debug tracing with level checking
/*************************************************************************/
#if defined(HSCSI_DEBUG) && defined(_DEBUG)
#define HSCSI_PRINT_STRING(level, string) \
	TRACE_STRING(level, string);
	
#define HSCSI_PRINT_HEX(level, string, number) \
	TRACE_HEX(level, string, number);
	
#define HSCSI_PRINT_HEX16(level, string, number) \
	TRACE_HEX16(level, string, number);

#define HSCSI_PRINT_NUMBER(level, string, number) \
	TRACE_NUMBER(level, string, number);

#define HSCSI_DUMP_HEX(level, string, pointer, length) \
	TRACE_DUMP_HEX(level, string, pointer, length);

#else

// If debugging is not turned on, no code is generated
#define HSCSI_PRINT_STRING(level, string) 
#define HSCSI_PRINT_HEX(level, string, number) 
#define HSCSI_PRINT_HEX16(level, string, number) 
#define HSCSI_PRINT_NUMBER(level, string, number)
#define HSCSI_DUMP_HEX(level, string, pointer, length)
#endif // #ifdef (HSCSI_DEBUG)
		
#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HSCSITrace_H  */
