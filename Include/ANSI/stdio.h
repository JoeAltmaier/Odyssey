/*  Metrowerks Standard Library  Version 2.3a4  1997 December 8  */

/*
 *	stdio.h
 *	
 *		Copyright © 1995-1997 Metrowerks, Inc.
 *		All rights reserved.
 */
// 031099 by Joe Altmaier:  add EOF.
 
#ifndef __cstdio__
#define __cstdio__


#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#include <ansi_parms.h>

__namespace(__stdc_space(stdio))

#include <size_t.h>
#include <null.h>
#include <stdarg.h>

#define EOF				 -1

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

int	printf(const char * format, ...);
int	ttyport_printf(int port, const char * format, ...);
int	sprintf(char * s, const char * format, ...);
int   vprintf(const char * format, va_list args);
int   ttyport_vprintf(int port, const char * format, va_list args);
int   vsprintf(char * s, const char * format, va_list args);
int 	getchar(void);
int 	ttyport_getchar(int);
int 	putchar(int c);
int 	ttyport_putchar(int port, int c);
int	kbhit(void);

/* here're defs which should be in conio.h, but we don't have that header */

int   getch(void);      /* blocking read of char from console, without echo */
int   ttyport_getch(int port);      /* blocking read of char from console, without echo */
int 	putch(int c);     /* blocking write of raw char to console (no mapping) */
int 	ttyport_putch(int port, int c);     /* blocking write of raw char to console (no mapping) */

__end_namespace(stdc_space(stdio))

__import_stdc_into_std_space(stdio)

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* __cstdio__ */
