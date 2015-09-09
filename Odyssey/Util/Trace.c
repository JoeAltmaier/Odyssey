/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Trace.c
// 
// Description:
// This file is a general tracing package.  It uses the serial I/O routines
// defined in the SerDrv.
// 
// $Log: /Gemini/Odyssey/Util/Trace.c $
// 
// 24    6/16/99 10:15p Jaltmaier
// LOG must exist if DEBUG undef.
// 
// 23    5/18/99 11:27a Jaltmaier
// Fix include duplicate 'UNSIGNED'.
// Add return to methods missing return.
// Prototype methods to avoid warning.
// 
// 22    5/14/99 2:10p Jaltmaier
// Added 'Log'
// 
// 21    4/28/99 3:49p Cwohlforth
// Put back Odyssey version code removed by josh
// 
// 20    4/27/99 9:51a Jhatwich
// 
// 19    4/27/99 8:46a Jhatwich
// added trace to file for win32
// 
// 18    4/26/99 1:23p Jhatwich
// win32 support
// 
// 17    4/23/99 6:52p Ewedel
// Modified Tracef() and Tracef_Method() so they use vsprintf() instead of
// private (& changed) _fmt().
// 
// 16    3/24/99 1:33p Ewedel
// Fixed TracePuts() so that it doesn't do bad things to '%' chars.
// Also added vss log: keyword.
//
// 02/17/99 Jim Frandeen: Remove #define _DEBUG -- better to put it in
//				a prefix file.
// 02/12/99 Joe Altmaier: TRACE use Tracef_Method, Tracef_Null.  Restore Tracef().
// 02/04/99 Michael G. Panas: move Tracef() here from FmtSpec.c
// 01/25/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
// 01/25/99 Jim Frandeen: Remove unused variables
// 10/24/98 Tom Nelson: TracePuts()  - Used by FmtIO.c
// 9/29/98 Michael G. Panas: Changes to support _DEBUG as a global, include
// 		   "SerDrv.h" now, since it was removed from Trace.h
// 9/29/98 Jim Frandeen: Check stack only if Metrowerks.
// 8/18/98 Michael G. Panas: Create file
/*************************************************************************/

#include "StdArg.h"
#include "StdIo.h"
#include <limits.h>

#ifndef _WIN32
	#include "Nucleus.h"
#endif
#include "Odyssey_Trace.h"
#include "OsTypes.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
void Print_Number(char *p_string, UNSIGNED number);
void Print_Hex(char *p_string, UNSIGNED number);
void Print_Hex16(char *p_string, U16 number);
//STATUS _fmt(STATUS (*pFunc)(void *pBuf,char ch),void *pArg,const char **ppFmt);
//STATUS _movech(void *pArgs,char ch);
int Tracef_Method(int index, int level, char *pFmt, ...);

/*************************************************************************/
// Trace Globals
/*************************************************************************/
long		TraceLevel[200] = {0};			// Tracing array must be
                                            // initialized externally
UNSIGNED	Trace_num_remaining_free_bytes;

char		Trace_hex[17] = "0123456789ABCDEF";


#if defined (_WIN32)

BOOL		bTraceFile = FALSE;
BOOL		bUnerased = TRUE;
char		sTraceFile[256];

void TraceToFile(char* sFile) {
	sprintf(sTraceFile,"%s", sFile);
	bTraceFile = TRUE;
}

#endif

/*************************************************************************/
// TracePuts - Puts() for TRACE output
/*************************************************************************/

int TracePuts(char *pStr)
{
   //  be careful how we use printf() here -- we don't want to munge
   //  any embedded '%' chars in our input string
#if defined(_WIN32)
	if(!bTraceFile) {
		printf("%s", pStr);
	}
	else {

		FILE* fp;
		if(bUnerased) {
			fp = fopen(sTraceFile,"w");
			bUnerased = FALSE;
		}
		else {
			fp = fopen(sTraceFile,"a");
		}

		if(fp) {
			fprintf(fp,"%s",pStr);
			fclose(fp);
		}
	}

#else				// Metrowerks
   //  be careful how we use printf() here -- we don't want to munge
   //  any embedded '%' chars in our input string
	printf("%s", pStr);
#endif
	return OK;
}

#ifdef _DEBUG
	int iLog=0;
	struct TLog {
		U32 tyLog;
		U32 dLog;
		} aLog[1024];
		
	void Log(unsigned long tyLog, unsigned long dLog) { 
		aLog[iLog].tyLog=tyLog;
		aLog[iLog].dLog=dLog;
		iLog=(iLog+1) % 1024;
		aLog[iLog].tyLog='STOP';
		}


/*************************************************************************/
// Print_Number
/*************************************************************************/
void Print_Number(char *p_string, UNSIGNED number)
{
	char sPrint[512];
	sprintf(sPrint,"%s %08lx ", p_string, number);
	TracePuts(sPrint);
	
} // Print_Number

/*************************************************************************/
// Print_Hex
/*************************************************************************/
void Print_Hex(char *p_string, UNSIGNED number)
{
	char sPrint[512];
	sprintf(sPrint,"%s %08lx ", p_string, number);
	TracePuts(sPrint);
} // Print_Hex

/*************************************************************************/
// Print_Hex16
// Print 16-bit word
/*************************************************************************/
void Print_Hex16(char *p_string, U16 number)
{	
	char sPrint[512];
	sprintf(sPrint,"%s %02x ", p_string, number);
	TracePuts(sPrint);
} // Print_Hex16

/*************************************************************************/
// DumpHex
// Print bytes in hex for Length bytes, include <nl> every 16 bytes
/*************************************************************************/
void DumpHex(char *p_string, U8 *p_data, U16 Length)
{
	char	string[4];
	int		index;
	U8		digit;
	
	TracePuts(p_string);		// opening string

	string[2] = ' ';
	string[3] = 0;
	
	for (index = 0; index < Length; index++)
	{	
		if ((index & 0xf) == 0 )
			TracePuts("\n\r");
		digit = *p_data++;
		string[1] = Trace_hex[digit & 0xF];
		string[0] = Trace_hex[(digit & 0xF0) >> 4];
	    TracePuts(string);
	}
	string[4] = 0;
	
} // DumpHex

/*************************************************************************/
// Trace_Entry
// Trace_Entry is called at the begining of each method
/*************************************************************************/
void Trace_Entry(char* p_method_name)
{
	// Print method name
	TracePuts("\n\rTrace_Entry ");
	TracePuts(p_method_name);
	TracePuts("\n\r");

#ifndef _WIN32
#ifndef _WINDOWS
#ifndef THREADX
	// If building under Metrowerks
	// Check for stack overflow	
	Trace_num_remaining_free_bytes = NU_Check_Stack();
#endif
#endif
#endif	

} // FCP_Trace_Entry
#endif // _DEBUG

#undef PRINTF_CLIB
#ifdef PRINTF_CLIB
/*************************************************************************/
// Tracef
// Tracef is a printf style trace support function
/*************************************************************************/

int Tracef(char *pFmt, ...)
{

    char buf[512];
	va_list args;                        /*scm 970709 */

	va_start( args, pFmt );            /*scm 970709 */
	vsnprintf(buf, ULONG_MAX, pFmt, args);

	TracePuts(buf);	/* Debug Output Device */

    return 0;
}

/*************************************************************************/
// Tracef_Method
// Tracef_Method is a printf style trace support function
/*************************************************************************/

int Tracef_Method(int index, int level, char *pFmt, ...)
{

    char buf[512];
	va_list args;

	if (TraceLevel[index] >= level) {

		va_start( args, pFmt );
		vsnprintf(buf, ULONG_MAX, pFmt, args);

		TracePuts(buf);	/* Debug Output Device */
	}

    return 0;
} // Tracef_Method

#else
// Use our own formatting tool.
#include "ct_fmt.h"

/*************************************************************************/
// Tracef
// Tracef is a printf style trace support function
/*************************************************************************/

int Tracef(char *pFmt, ...)
{
#if 1
	char buf[512];
	va_list  ArgList;

	//  format trace message
	va_start (ArgList, pFmt);
    vsprintf (buf, pFmt, ArgList);

	//  and write it to debug output device
	TracePuts(buf);	/* Debug Output Device */
#else
	char *pBuf,buf[512];

	//  format trace message
	va_start (ArgList, pFmt);
    vsprintf (buf, pFmt, ArgList);

	TracePuts(buf);	// Debug Output Device 
#endif
	return OK;
}

/*************************************************************************/
// Tracef_Method
// Tracef_Method is a printf style trace support function
/*************************************************************************/

int Tracef_Method(int index, int level, char *pFmt, ...)
{
#if 1
	char buf[512];
	va_list  ArgList;

	if (TraceLevel[index] >= level) {
		//  format trace message
		va_start (ArgList, pFmt);
	    vsprintf (buf, pFmt, ArgList);



		//  and write it to debug output device
		TracePuts(buf);	/* Debug Output Device */
	}
#else
	char *pBuf,buf[512];

	if (TraceLevel[index] >= level) {
		//  format trace message
		va_start (ArgList, pFmt);
	    vsprintf (buf, pFmt, ArgList);

		TracePuts(buf);	//Debug Output Device 

	}
#endif
    return OK;
} // Tracef_Method
#endif
