/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Odyssey_Trace.h
// 
// Description:
// This file defines general trace methods for debugging. Replaces Trace.h
// 
// Update Log
//	$Log: /Gemini/Include/odyssey_trace.h $
// 
// 11    9/16/99 4:03p Tnelson
// 
// 10    8/16/99 12:35p Tnelson
// Added TRACE_PROC macro for C++ only
// 
// 9     8/12/99 2:48p Tnelson
// Added TRACE_ENTER and TRACE_EXIT macros
// 
// 8     8/06/99 8:47p Jaltmaier
// New TRACE_MESSAGE
// 
// 7     6/16/99 8:44p Jaltmaier
// Make TRACE work semantically the same when DEBUG is undef.
// Trace_Puts needs to be defined when DEBUG is undef.
// 
// 6     5/19/99 2:23p Jhatwich
// 
// 5     5/19/99 12:34p Jhatwich
// Removed Log that caused failed build
// 
// 4     5/14/99 2:10p Jaltmaier
// Added 'Log'
// 
// 3     5/12/99 3:23p Mpanas
// Fix the TRACEF() macro, remove TRACE and  _TRACE
//
//
// 8/18/98 Michael G. Panas: Create file
// 9/2/98 Michael G. Panas: add C++ stuff
// 9/2/98 Joe Altmaier: Add TRACE_ENABLE for performance monitoring
// 9/29/98 Jim Frandeen: Change TRACE_ENABLE to _DEBUG because MSD
// 			gives us this flag in the debug build.
// 10/14/98 Michael G. Panas: Change TRACE_ENTRY_LVL to level 4 instead of 1.
// 			This gives us 3 user levels without too verbose a display.
// 11/11/98 Tom Nelson: Tracef.
// 12/03/98 Michael G. Panas: Remove all vestiges of Print_String et all,
//                            use printf() instead.
// 01/24/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
// 02/04/99 Michael G. Panas: fixed tracef() somewhat...
// 02/12/99 Joe Altmaier: TRACE use Tracef_Method, Tracef_Null.  Restore Tracef().
// 02/17/99 Jim Frandeen: Remove #define _DEBUG -- better to put it in
//				a prefix file.
// 02/18/99 Joe Altmaier: Tracef always exists.
/*************************************************************************/

#if !defined(Odyssey_Trace_H)
#define Odyssey_Trace_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

// Tracef is like printf, but flushes after each write.
int Tracef(char *pFmt, ...);
int TracePuts(char *pStr);

#if defined(_DEBUG)

#include "Trace_Index.h"

void DumpHex(char *p_string, unsigned char *p_data, unsigned short Length);
void Trace_Entry(char* p_method_name);

extern		long TraceLevel[];

void TraceToFile(char* sFileName);

/*************************************************************************/
//	  Define Trace Levels Allowed
//	  0 - 9 are the current levels, 9 is trace all
/*************************************************************************/
typedef enum {
	TRACE_OFF_LVL		= 0,		// No tracing
	TRACE_L1			= 1,
	TRACE_L2			= 2,
	TRACE_L3			= 3,
	TRACE_ENTRY_LVL		= 4,
	TRACE_L4			= 4,
	TRACE_L5			= 5,
	TRACE_L6			= 6,
	TRACE_L7			= 7,
	TRACE_L8			= 8,
	TRACE_ALL_LVL		= 9			// Tracing on for all levels
} TRACE_LEVELS;

// NOTE: TRACE_INDEX must be defined externally!

#define TRACE_STRING(level, string) \
	if (TraceLevel[TRACE_INDEX] >= level) printf(string);
	
#define TRACE_HEX(level, string, number) \
	if (TraceLevel[TRACE_INDEX] >= level) printf("%s%08x", string, number);
	
#define TRACE_HEX16(level, string, number) \
	if (TraceLevel[TRACE_INDEX] >= level) printf("%s%04x", string, number);

#define TRACE_NUMBER(level, string, number) \
	if (TraceLevel[TRACE_INDEX] >= level) printf("%s%d", string, number);

#define TRACE_NUMBER_EOL(level, string, number) \
	if (TraceLevel[TRACE_INDEX] >= level) printf("%s%d\n\r", string, number);

#define TRACE_DUMP_HEX(level, string, pointer, length) \
	if (TraceLevel[TRACE_INDEX] >= level) DumpHex(string, pointer, length);

#define TRACE_ENTRY(method) \
	if (TraceLevel[TRACE_INDEX] >= TRACE_ENTRY_LVL) Trace_Entry(#method);

// no flush version of TRACEF()
#define TRACEF_NF(level, X) \
	if (TraceLevel[TRACE_INDEX] >= level) printf X;

// TRACEF(level, ("string %d", args)); NOTE: extra parens for var args
#define TRACEF(level, X) \
	if (TraceLevel[TRACE_INDEX] >= level) Tracef X;

#define TRACE(index, level, X) \
	if (TraceLevel[index] >= level) Tracef X;

void Log(unsigned long tyLog, unsigned long dLog);

#ifdef __cplusplus
// TRACE_PROC(MyClass::MyProc)
//    Displays: "ENTER  MyClass::MyProc"
//    then on exit automatically
//    Displays: "EXIT   MyClass::MyProc"
#define TRACE_PROC(method)	struct TraceProc { TraceProc() { TRACE_ENTER(method); } ~TraceProc() { TRACE_EXIT(method); } }; TraceProc traceProc
#endif
// Ddm base class use 
#define TRACEDDM(ddmFlags,X) if (ddmFlags & DDMDEBUG) Tracef X;

#else
#define TRACE_STRING(level, string) if (false) ;
#define TRACE_HEX(level, string, number) if (false) ;
#define TRACE_HEX16(level, string, number) if (false) ;
#define TRACE_NUMBER(level, string, number) if (false) ;
#define TRACE_DUMP_HEX(level, string, pointer, length) if (false) ;
#define TRACE_ENTRY(method) if (false) ;
#define TRACE_NUMBER_EOL(level, string, number)  if (false) ;
#define TRACEF_NF(level, X) if (false) ;
#define TRACEF(level, X) if (false) ;
#define TRACE(index, level, X) if (false) ;
#define TRACE_PROC(method)
#define TRACEDDM(flags,X) if (false) ;

#define Log(x, y)

#endif /* _DEBUG */

#define TRACE_ENTER(method)	TRACEF(TRACE_ENTRY_LVL, ("ENTER  " #method "\n"))	//**Tom
#define TRACE_EXIT(method)	TRACEF(TRACE_ENTRY_LVL, ("EXIT   " #method "\n"))	//**Tom

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* Trace_H  */
