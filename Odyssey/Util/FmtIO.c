/* FmtIO.C -- Formatted Printf() style I/O
 *
 * Copyright ConvergeNet Technologies (c) 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
//
// Revision History:
// $Log: /Gemini/Odyssey/Util/FmtIO.c $
// 
// 22    1/26/00 2:47p Joehler
// Added ttyport_* string processing functionality.
// 
// 21    12/09/99 4:12p Joehler
// 
// 20    5/07/99 11:16a Jaltmaier
// Please, please,please don't use busy-wait loops!
// 
// 19    4/27/99 1:12p Ewedel
// Added putch() routine, to support unfiltered character output to
// console.
// 
// 18    4/26/99 1:43p Jhatwich
// win32 support
// 
// 17    4/23/99 6:48p Ewedel
// Added check of own prototypes in stdio.h.  Added new routines
// vprintf(), vsprintf() and getch().  Changed getchar() and putchar() to
// better emulate standard library routines in terms of their echo and
// '\n' expansion behavior.  Updated printf() and sprintf() to work with
// modified (va_list aware) _fmt().
// 
// 16    4/06/99 2:36p Rkondapalli
// Added back the while loop for getchar which had been removed
// 
// 15    4/01/99 2:14p Mpanas
// Get rid of that nasty extra <nl> when
// "\n\r" is in your string
// 
// 14    3/17/99 3:56p Agusev
// Changes to make kbhit() work properly
// 
// 13    3/10/99 8:36a Jfrandeen
// Put back getchar and putchar
// 
// 12    3/09/99 11:06a Jfrandeen
// Removed getchar and putchar.  These are defined in cstdio.h.
// 
// 12	 3/09/99 Jim Frandeen
// Removed getchar and putchar.  These are defined in cstdio.h.
// 11    3/05/99 6:57p Ewedel
// Fixed compiler warning.
// 
// 10    3/05/99 8:14p Jaltmaier
// Use Drivers serial interface.
// 
// 8     3/04/99 3:43p Ewedel
// Changed printf() back to using ttyA_out(), since it has dual-platform
// driver support.
// 
// 7     3/01/99 7:38p Ewedel
// Cleaned up some warnings, const-strengthened format string arguments,
// changed printf()'s underlying output from (undefined) ttyA_out() to
// Print_String().
//     9-28-98  TRN  Create for MIPS I/O
// 
 *
**/

#include "stdarg.h"
#include "OsTypes.h"
#include "ct_fmt.h"
#include "TraceMon.h"

#ifndef _WIN32

#include "ansi/stdio.h"    // our own public prototypes


//typedef unsigned long I32;	// hack -- this is needed by tty.h

#include "tty.h"
#include "systty.h"

// ASCII Characters

#define CR		13
#define OK		0


BOOL gfReturn = FALSE;

/* printf -- format to string --------------------------------------------------*/

int printf(const char *pFmt, ...)
{
   va_list  ArgList;


   //  grab variable parameter tail
   va_start (ArgList, pFmt);

   //  and let vprintf() do the work
   return (vprintf (pFmt, ArgList));

}  /* end of printf */

/* ttyport_printf -- format to string --------------------------------------------------*/

int ttyport_printf(int port, const char *pFmt, ...)
{
   va_list  ArgList;


   //  grab variable parameter tail
   va_start (ArgList, pFmt);

   //  and let vprintf() do the work
   return (ttyport_vprintf (port, pFmt, ArgList));

}  /* end of printf */

/* sprintf -- format to string -------------------------------------------------*/

int sprintf(char *pStr, const char *pFmt, ...)
{
    va_list  ArgList;

    //  grab variable parameter tail
    va_start (ArgList, pFmt);

    //  do the format
    _fmt(_movech,&pStr,pFmt,ArgList);
    *pStr = EOS;
    
    return 0;

}  /* end of sprintf */

/* vprintf -- format to stdout, with caller-supplied arg list ------------------*/

int   vprintf(const char * pFmt, va_list ArgList)
{
   
   char ch,*pBuf,buf[512];
   int      i;

	pBuf = buf;

   //  do the format (& hope we don't exceed size of buf[]!)
	_fmt(_movech,&pBuf,pFmt,ArgList);
	*pBuf = EOS;

	pBuf = buf;
	while ((ch = *pBuf++) != EOS) {
		if (ch == '\n' && !gfReturn)
			ttyA_out('\r');
			
		ttyA_out(ch);

//		if ((gfReturn= (ch == '\r')) != 0)
//			ttyA_out('\n');
	}		
    
	return OK;

}  /* end of vprintf */

/* ttyport_vprintf -- format to specific port, with caller-supplied arg list ------------------*/

int  ttyport_vprintf(int port, const char * pFmt, va_list ArgList)
{
   
   char ch,*pBuf,buf[512];
   int      i;

	pBuf = buf;

   //  do the format (& hope we don't exceed size of buf[]!)
	_fmt(_movech,&pBuf,pFmt,ArgList);
	*pBuf = EOS;

	pBuf = buf;
	while ((ch = *pBuf++) != EOS) {
		if (ch == '\n' && !gfReturn)
			ttyout(port, '\r');
			
		ttyout(port, ch);

	}		
    
	return OK;

}  /* end of ttyport_vprintf */

/* vsprintf -- format to string, with caller-supplied arg list -----------------*/

int   vsprintf(char * pszOutBuf, const char * pFmt, va_list ArgList)
{

   //  do the format
   _fmt(_movech,&pszOutBuf,pFmt,ArgList);
   *pszOutBuf = EOS;

   return 0;

}  /* end of vsprintf */

/* getchar -- perform blocking read of next char from console ------------------*/
/*            We echo the char, and map \r into \n                              */

int 	getchar() 
{

int   ch;


   //  spin (!) until a char is ready to read
	while (! kbhit ())
      {
		NU_Sleep(10);
      }

   //  read it
   ch = ttyA_in();

   if (ch == '\r')
      {
      //  user pressed "enter", map this into our standard C newline char
      ch = '\n';
      }

   //  echo char back to console
   putchar (ch);

   //  and let caller know what it is  :-)
   return (ch);

}  /* end of getchar */

int 	ttyport_getchar(int port) 
{

int   ch;


   //  spin (!) until a char is ready to read
/*	while (! ttyhit(port))
      {
		NU_Sleep(10);
      }*/

   //  read it
   ch = ttyin(port);

   if (ch == '\r')
      {
      //  user pressed "enter", map this into our standard C newline char
      ch = '\n';
      }

   //  echo char back to console
   ttyport_putchar (port, ch);

   //  and let caller know what it is  :-)
   return (ch);

}  /* end of ttyport_getchar */


/* getch -- perform blocking read of next char from console --------------------*/
/*          We do not echo the char, and perform no char translation.           */

int  getch (void)
{


   //  spin (!) until a char is ready to read
	while (! kbhit ())
      {
		NU_Sleep(10);
      }

   //  read & return it
   return (ttyA_in());

}  /* end of getch */


/* ttyport_getch -- perform blocking read of next char from port --------------------*/
/*          We do not echo the char, and perform no char translation.           */

int  ttyport_getch (int port)
{
   //  spin (!) until a char is ready to read
/*	while (! ttyhit(port))
      {
		NU_Sleep(10);
      }*/

   //  read & return it
//   return (ttyport_in(port));
	return(ttyin(port));

}  /* end of getch */

/* putchar -- perform blocking write of char to console ------------------------*/
/*            We map \n into a <cr><lf> pair                                    */
	
int 	putchar(int c) 
{

   //  if caller passed a \n, insert implicit <cr>
   if (c == '\n')
      {
      ttyA_out ('\r');
      }

	return ttyA_out(c);

}  /* end of putchar */

/* ttyport_putchar -- perform blocking write of char to port ------------------------*/
/*            We map \n into a <cr><lf> pair                                    */
	
int 	ttyport_putchar(int port, int c) 
{

   //  if caller passed a \n, insert implicit <cr>
   if (c == '\n')
      {
      ttyout (port, '\r');
      }

	return ttyout(port, c);

}  /* end of putchar */


/* putch -- perform blocking write of raw char to console ----------------------*/
	
int 	putch(int c) 
{

	return ttyA_out(c);

}  /* end of putch */

/* ttyport_putch -- perform blocking write of raw char to port ----------------------*/
	
int 	ttyport_putch(int port, int c) 
{

	return ttyout(port, c);

}  /* end of putch */


/* kbhit -- perform non-blocking test of char ready to read from console -------*/

int	kbhit()
{
   //  return TRUE if a char is ready to read from console input
	return ((ttyA_poll()) ? 0 : 1);
}


/* ttyport_kbhit -- perform non-blocking test of char ready to read from port -------*/

/*int	ttyport_kbhit(int port)
{
   //  return TRUE if a char is ready to read from console input
	return ttyhit(port);
}*/

#else 
#include "stdio.h"
#endif

int Print_String(char *pStr);
int Print_String(char *pStr) {
	return printf("%s", pStr);
}
