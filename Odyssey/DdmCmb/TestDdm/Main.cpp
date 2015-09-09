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
// File: Template.c
// 
// Description:
//    Demo Ddm Main.  Used to start up CHAOS [OOS] execution.
//
//    This Demo requires that Interrupt 0 on your eval CPU
//    be jumpered for serial interrupts.
// 
// $Log: /Gemini/Odyssey/DdmCmb/Test/Main.cpp $
// 
// 4     5/12/99 3:50p Ewedel
// Changed from Trace.h to Odyssey_Trace.h (system-wide change).
// 
// 3     4/07/99 12:04p Ewedel
// Added another CheckHeap() call, just for paranioa's sake.
// 
// 2     3/30/99 6:24p Ewedel
// Added more trace levels, & enabled them before calling
// Oos::Initialize().
// 
// 1     3/19/99 7:37p Ewedel
// Initial checkin.
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

