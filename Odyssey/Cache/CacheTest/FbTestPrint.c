/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FbTestPrint.cpp
// 
// Description:
// This file implements Print for the Flash Block test. 
// 
// Update Log 
// 
// 9/3/98 Jim Frandeen: Create file
/*************************************************************************/

#include "Nucleus.h"
#include <stdlib.h>
#include <stdio.h>
#include <ostream.h>
#include <string.h>

/* The Convert_Number routine converts an unsigned number into a 
   string so it can be printed.  */
char *
Convert_Number (char *st, unsigned int number, int base, int length, char chPad)
{
   int      i;
   char     aChDigit[]="0123456789ABCDEF";

   /* Build the string, starting at the right hand side.  */
   for (i=length-1; i >= 0; ) {
	  st[i--]=(number? aChDigit[number % base] :chPad? chPad :'0');
	  number /= base;
	  if (!number && !chPad)
	  	break;
   } 
    
   /* Place a NULL at the end of the string.  */
   st[length] =  (unsigned char) 0;
   return &st[i+1];
}


