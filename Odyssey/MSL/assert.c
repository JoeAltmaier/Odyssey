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
// File: assert.c
// 
// Description:
//    This is a replacement for the Metrowerks Standard Library
//    assert.c, which version requires various stdio things which
//    we don't support in the Odyssey environment.
// 
// $Log: /Gemini/Odyssey/MSL/assert.c $
// 
// 7     1/03/00 4:18p Jlane
// Add new code to halt the machine on assert failure.
// 
// 6     9/02/99 2:12p Iowa
// 
// 5     8/30/99 2:57p Iowa
// Large allocations come from heapBig.
// 
// 4     7/29/99 7:04p Mpanas
// [ewx]  Fixed so assert() should trigger a fault more reliably.
// 
// 3     5/13/99 11:33a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 2     4/01/99 12:37p Jaltmaier
// Use new Critical.h
// 
// 1     3/19/99 6:49p Ewedel
// Initial checkin.
//
/*************************************************************************/


#include <assert.h>

#include "odyssey_trace.h"
void write_to_one(void);
asm void write_to_one(void)
{
	sw	zero, 1(zero)
	jr	ra
	nop
}

//
//  __assertion_failed (pszCondition, pszFilename, ulLineno)
//
//  Description:
//    Called by the assert() macro (defined in Metrowerks' assert.h)
//    when the given assertion condition is FALSE.
//
//    We report the assertion failure using the Odyssey Tracef()
//    facility, and then cause a fatal error by assigning through
//    a NULL pointer.  The latter has the effect of breaking us
//    into the debugger (or hosing the system if we're not running
//    under a debugger).
//
//  Inputs:
//    pszCondition - Text string showing exactly what expression
//             evaluated to FALSE to trigger this call.
//    pszFilename - Name of the source file containing the assert()
//             which called us.
//    ulLineno - Line number in *pszFilename of the assert() which 
//             called us.
//
//  Outputs:
//    none
//

void __assertion_failed(char * pszCondition, char * pszFilename, int ulLineno)
{

long  * plBadPtr;


   Tracef ("Assertion (%s) failed in \"%s\" on line %d\n",
           pszCondition, pszFilename, ulLineno);

   write_to_one();
   return;

}  /* end of __assertion_failed */

