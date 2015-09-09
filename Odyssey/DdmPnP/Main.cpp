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
// File: Main.c
// 
// Description:
//    Demo Ddm Main.  Used to start up CHAOS [OOS] execution.
//
//    This Demo requires that Interrupt 0 on your eval CPU
//    be jumpered for serial interrupts.
// 
// $Log: /Gemini/Odyssey/DdmPnP/Main.cpp $
// 
// 6     7/29/99 5:59p Hdo
// 
// 5     7/27/99 7:04p Hdo
// 
// 4     7/24/99 4:17p Hdo
// 
// 3     7/21/99 7:29p Hdo
// 
// 2     6/30/99 6:38p Hdo
// 
// 1     6/30/99 10:23a Hdo
// 
// Initial checkin.
//
/*************************************************************************/

#include "Os.h"
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void  StartTask(UNSIGNED , VOID *)
{
	Oos::Initialize();
}  /* end of StartTask */