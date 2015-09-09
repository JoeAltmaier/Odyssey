/*************************************************************************/
// File: CtTtyUtils.h
// 
// Description:
//
// 
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
//
//*************************************************************************
// Update Log 
//	$Log: /Gemini/Odyssey/Util/CtTtyUtils.h $
// 
// 2     1/26/00 2:46p Joehler
// Added ttyport_* string processing functionality.
// 
// 1     12/09/99 4:14p Joehler
// 
/*************************************************************************/

#ifndef	_CtTtyUtils_h
#define	_CtTtyUtils_h

/*************************************************************************/

#include "stdio.h"
#include "simple.h"
#include "tty.h"

// functions
extern void printf_at(int row, int col, char *string);
extern void	Print_Dump(unsigned long *p, int cb);
extern UI64 Get_Hex(char *p_message);
extern U32 Get_U32(char *p_message);
extern void ttyport_printf_at(I32 port, int row, int col, char *string);
extern void	ttyport_Print_Dump(I32 port, unsigned long *p, int cb);
extern UI64 ttyport_Get_Hex(I32 port, char *p_message);
extern U32 ttyport_Get_U32(I32 port, char *p_message);

// useful ascii escape sequences
extern char		*clreol;                /* clear to the end of the line */
extern char		*clrscn;				/* clear the screen */
extern char		*bspace;		        /* backspace */

#endif


