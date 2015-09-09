/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: Main.cpp
// 
// Description:
//    Main image entry point.  Used to start up CHAOS [OOS] execution.
//
//    This Demo requires that Interrupt 0 on your eval CPU
//    be jumpered for serial interrupts.
// 
// $Log: /Gemini/Odyssey/DdmHotSwap/TestDdm/Main.cpp $
// 
// 1     10/11/99 7:53p Ewedel
// First cut.
// 
/*************************************************************************/

#define _DEBUG

#include  "Os.h"
#include  "Odyssey_Trace.h"

#include  "OsHeap.h"          // heap debug support (from ..\msl)


extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void  StartTask(UNSIGNED , VOID *)
{
   Tracef("Start Task\n");   
   
   //  enable heap debugging, when using MSL-ISA3-noFPU-BE-ADBG.lib (phew!)
#ifdef _DEBUG
   TraceLevel[TRACE_HEAP1]   = 1;
//   TraceLevel[TRACE_DDM_MGR] = 1;
//   TraceLevel[TRACE_DDM]     = 1;
#endif

   Oos::Initialize();

   //  do a baseline integrity check on the heap
   OsHeap::CheckHeap();
   
   Tracef("Here we go...\n");

   OsHeap::CheckHeap();

}  /* end of StartTask */

