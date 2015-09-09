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
// File: RowId.cpp
// 
// Description:
//  This file contains the bodies of class RowId members.
//  (RowId is defined in include\CtTypes.h.)
// 
// $Log: /Gemini/Odyssey/Util/RowId.cpp $
// 
// 2     9/04/99 4:33p Agusev
// Fixed WIN32 build broken by the conversion to new RowId
// 
// 1     9/03/99 5:38p Ewedel
// Initial revision.
// 
/*************************************************************************/

#include  "CtTypes.h"

#ifndef WIN32
#include  "ansi/stdio.h"         // for printf(), used below
#else
#include  "stdio.h"         // for printf(), used below
#endif


//
//  RowId::PrintYourself ()
//
//  Description:
//    A little debug helper member.
//
//    This routine simply prints the current value of our RowId instance
//    to the console.  We use printf() for the output, because that's
//    what the existing code (SSAPI_Server) which inspired this routine
//    uses.
//
//  Inputs:
//    none
//
//  Outputs:
//    none
//

void  RowId::PrintYourself ()  const
{

   printf ("%d:%d:%d", Table, HiPart, LoPart);

}

