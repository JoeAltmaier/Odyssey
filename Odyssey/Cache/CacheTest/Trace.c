/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpError.c
// 
// Description:
// This file is a general tracing package.  It uses the serial I/O routines
// defined in the SerDrv.
// 
// Update Log 
// 8/18/98 Michael G. Panas: Create file
// 9/29/98 Jim Frandeen: Check stack only if Metrowerks.
// 9/29/98 Michael G. Panas: Changes to support _DEBUG as a global, include
// "SerDrv.h" now, since it was removed from Trace.h
// 1/13/99 Jim Frandeen: Use this old version of Trace for TraceMonitor
// because it uses TraceString.
/*************************************************************************/

#include "Nucleus.h"
#include "I2Odep.h"
//#include "SerDrv.h"
#include "TraceMon.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
void Print_Number(char *p_string, UNSIGNED number);

/*************************************************************************/
// Trace Globals
/*************************************************************************/
long		TraceLevel = 0;						// Tracing turned off
char		Trace_hex[17] = "0123456789ABCDEF";
UNSIGNED	Trace_num_remaining_free_bytes;

/*************************************************************************/
// Print_Number
/*************************************************************************/
void Print_Number(char *p_string, UNSIGNED number)
{
	char	string[20];
	
	Print_String(p_string);
    Convert_Number(string, number, 16, 8, '0');
    Print_String(string);
	
} // Print_Number

/*************************************************************************/
// Print_Hex
/*************************************************************************/
void Print_Hex(char *p_string, UNSIGNED number)
{
	char	string[20];
	int		index;
	char	digit;
	
	Print_String(p_string);
	index = 4;
	while (index)
	{	
		index--;
		digit = (char)number & 0xFF;
		number = number >> 8;
		string[index * 2 + 1] = Trace_hex[digit & 0xF];
		string[index * 2] = Trace_hex[(digit & 0xF0) >> 4];
	}
	string[8] = 0;
    Print_String(string);
	
} // Print_Hex

/*************************************************************************/
// Print_Hex16
// Print 16-bit word
/*************************************************************************/
void Print_Hex16(char *p_string, U16 number)
{
	char	string[20];
	int		index;
	char	digit;
	
	Print_String(p_string);
	index = 2;
	while (index)
	{	
		index--;
		digit = number & 0xFF;
		number = number >> 8;
		string[index * 2 + 1] = Trace_hex[digit & 0xF];
		string[index * 2] = Trace_hex[(digit & 0xF0) >> 4];
	}
	string[4] = 0;
    Print_String(string);
	
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
	
	Print_String(p_string);		// opening string

	string[2] = ' ';
	string[3] = 0;
	
	for (index = 0; index < Length; index++)
	{	
		if ((index & 0xf) == 0 )
			Print_String("\n\r");
		digit = *p_data++;
		string[1] = Trace_hex[digit & 0xF];
		string[0] = Trace_hex[(digit & 0xF0) >> 4];
	    Print_String(string);
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
	Print_String("\n\rTrace_Entry ");
	Print_String(p_method_name);
	Print_String("\n\r");
	
#ifndef _WINDOWS
	// If building under Metrowerks
	// Check for stack overflow	
	Trace_num_remaining_free_bytes = NU_Check_Stack();
#endif
		
} // FCP_Trace_Entry




