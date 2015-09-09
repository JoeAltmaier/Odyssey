/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdMain.cpp
// 
// Description:
// This file implements the main entry point for the
// Solid State Drive. 
// 
// Update Log 
// 
// 02/25/99 Jim Frandeen: Create file
/*************************************************************************/

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include "Os.h"
#include "SccDriver.h"
#include <stdio.h>

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);
extern "C" void  Print_String (char *string);

void  StartTask(UNSIGNED , VOID *)
{


	Oos::DumpTables("In StartTask...");
	Oos::Initialize();
	
}

