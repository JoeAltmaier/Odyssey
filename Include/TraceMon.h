/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: TraceMon.h
// 
// Description:
// The idea is to the the old version of Trace.h if running under
// Windows.  This allows us to use the Trace Monitor. 
// 
// Update Log 
// 1/13/99 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(TraceMon_H)
#define TraceMon_H

#include "Trace_Index.h"
#if !defined(_WINDOWS)

// Not running under windows, use the new version of Trace.h
// that uses Printf
#include "stdio.h"
#include "Odyssey_Trace.h"
#else

// The following is the old version of Trace.h that uses PrintString.
#if defined(_DEBUG)

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

void Print_Number(char *p_string, unsigned long number);
void Print_Hex(char *p_string, unsigned long number);
void Print_Hex16(char *p_string, unsigned short number);
void DumpHex(char *p_string, unsigned char *p_data, unsigned short Length);
void Trace_Entry(char* p_method_name);
void Print_String(char *string);
char*   Convert_Number (char *st, unsigned int number, int base, int length, char fPad);

// Tracef_Method is called by the macro: TRACE(TRACE_INDEX, level, "string", ...);
// 5/14/99 Jim Frandeen: Make Tracef_Method fit the new TRACEF format.
//int Tracef_Method(int index, int level, char *pFmt, ...);
int Tracef_Method(int level, ...);

// Tracef is like printf, but flushes after each write.
int Tracef(char *pFmt, ...);

extern		long TraceLevel;

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

/*************************************************************************/
//	  Define Trace Levels Allowwed
//	  0 - 9 are the current levels, 9 is trace all
/*************************************************************************/
typedef enum {
	TRACE_OFF_LVL		= 0,		// No tracing
	TRACE_L1			= 1,
	TRACE_L2			= 2,
	TRACE_L3			= 3,
	TRACE_ENTRY_LVL		= 4,
	TRACE_L5			= 5,
	TRACE_L6			= 6,
	TRACE_L7			= 7,
	TRACE_L8			= 8,
	TRACE_ALL_LVL		= 9			// Tracing on for all levels
} TRACE_LEVELS;

#define TRACE_STRING(level, string) \
	if (TraceLevel >= level) Print_String(string);
	
#define TRACE_HEX(level, string, number) \
	if (TraceLevel >= level) Print_Hex(string, number);
	
#define TRACE_HEX16(level, string, number) \
	if (TraceLevel >= level) Print_Hex16( string, number);

#define TRACE_NUMBER(level, string, number) \
	if (TraceLevel >= level) Print_Number(string, number);

#define TRACE_NUMBER_EOL(level, string, number) \
	if (TraceLevel >= level) {Print_Number(string, number); Print_String("\n\r");}

#define TRACE_DUMP_HEX(level, string, pointer, length) \
	if (TraceLevel >= level) DumpHex(string, pointer, length);

#define TRACE_ENTRY(method) \
	if (TraceLevel >= TRACE_ENTRY_LVL) Trace_Entry(#method);

// TEMPORARY -- for windows, don't define TRACE until we can implement...
#define TRACEF 1? 0 : Tracef_Method

#else
#define TRACE_STRING(level, string)
#define TRACE_HEX(level, string, number)
#define TRACE_HEX16(level, string, number)
#define TRACE_NUMBER(level, string, number)
#define TRACE_DUMP_HEX(level, string, pointer, length)
#define TRACE_ENTRY(method)
#define TRACE_NUMBER_EOL(level, string, number) 
#define TRACE 1? 0 : Tracef_Method


#endif /* TRACE_ENABLE */

#endif // _WINDOWS

#endif /* TraceMon_H  */
