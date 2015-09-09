/*@***********************************************************************
 |
 |             Copyright (c) 1995-1997 XACT Incporated
 |
 | PROPRIETARY RIGHTS of XACT Incorporated are involved in the subject
 | matter of this material.  All manufacturing, reproduction, use, and
 | sales rights pertaining to this subject matter are governed by the
 | license agreement.  The recipient of this software implicitly accepts
 | the terms of the license.
 |
 |
 | FILE NAME   : xport.c
 | VERSION     : 1.1
 | COMPONENT   : XFAMILY
 | DESCRIPTION : Provides porting for XFAMILY products
 | AUTHOR      : Robert Winter
 *************************************************************************/
/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "xport.h"
#include "xtypes.h"

/*------------------- Specific suppprt requirements ---------------------*/
#ifdef XOS_NUC
#define IP_H                /* Exclude the Nucleus NET IP.H header file. */
#include "sockdefs.h"
#include "externs.h"
tmr_t	*tmr_free;
tmr_t	*tmr_active;
u32     NUM_ACTIVE_PORTS = 0;
#define MAX_TIMERS				10
extern NU_MEMORY_POOL   System_Memory;
#endif

/* BASIC atoi and atol until a common one appears */

int atoi (const char *s)
{
	int i = 0;
	while(s && *s && isdigit(*s))
	{
		i = i * 10 + (*s - '0');
		++s;
	}
	return i;
}

long atol (const char *s)
{
	long i = 0;
	while(s && *s && isdigit(*s))
	{
		i = i * 10 + (*s - '0');
		++s;
	}
	return i;
}

#ifdef WINNT
#include <time.h>
#endif

#ifdef XOS_PSOS
extern void *malloc( ul32 size );
extern void free( void *p );
#endif

#ifdef XSTK_NUCNET

static	u16	socket_family[] = {
	(u16)-1,		/* AF_UNSPEC unspecified */
	NU_FAMILY_UNIX,	/* AF_UNIX or AF_LOCAL local to host (pipes, portals) */
	NU_FAMILY_IP,	/* AF_INET  internetwork: UDP, TCP, etc. */
	(u16)-1,		/* AF_IMPLINK arpanet imp addresses */
	(u16)-1,		/* AF_PUP pup protocols: e.g. BSP */
	(u16)-1,		/* AF_CHAOS mit CHAOS protocols */
	(u16)-1,	    /* AF_NS XEROX NS protocols */
	(u16)-1,		/* AF_ISO or AF_OSI ISO protocols */
	(u16)-1,		/* AF_ECMA european computer manufacturers */
	(u16)-1,		/* AF_DATAKIT datakit protocols */
	(u16)-1,		/* AF_CCITT CCITT protocols, X.25 etc */
	(u16)-1,		/* AF_SNA  IBM SNA */
	(u16)-1,		/* AF_DECnet DECnet */
	(u16)-1,		/* AF_DLI DEC Direct data link interface */
	(u16)-1,		/* AF_LAT */
	(u16)-1,		/* AF_HYLINK NSC Hyperchannel */
	(u16)-1,		/* AF_APPLETALK Apple Talk */
	(u16)-1,		/* AF_ROUTE Internal Routing Protocol */
	(u16)-1,		/* AF_LINK Link layer interface */
	(u16)-1,		/* pseudo_AF_XTP eXpress Transfer Protocol (no AF) */
	(u16)-1,		/* AF_COIP connection-oriented IP, aka ST II */
	(u16)-1,		/* AF_CNT Computer Network Technology */
	(u16)-1,		/* pseudo_AF_RTIP Help Identify RTIP packets */
	(u16)-1,		/* AF_IPX Novell Internet Protocol */
	(u16)-1,		/* AF_SIP Simple Internet Protocol */
	(u16)-1,		/* pseudo_AF_PIP Help Identify PIP packets */
	(u16)-2
};
static	u16	socket_types[] = {
	(u16)-1,
	NU_TYPE_STREAM,		/* SOCK_STREAM */
	NU_TYPE_DGRAM,		/* SOCK_DGRAM */
	NU_TYPE_RAW,		/* SOCK_RAW */
	NU_TYPE_RDM,		/* SOCK_RDM */
	NU_TYPE_SEQPACKET,	/* SOCK_SEQPACKET */
	(u16)-2
};

static u32	unused_param;

u32		*NU_Errno(void);
#endif

#ifdef XLIB_XSNMP
void	xsnmp_qsort(void *a, u32 n, u32 es, i32 (*cmp)() );
l32		xsnmp_strtol(c_i8 *nptr, i8 **endptr, reg i32 base);
#endif

parm_list_t	plist[XSNMP_SPRINTF_NUM_PARMS];

void	x_bzero( i8 *s1, u32 len );
void	x_bcopy( i8 *s1, i8 *s2, u32 len );
void	x_dbg(i8 *msg, u32 flag);
i32		_xabs(i32 j);
extern void	timeout_function(ul32 arg);
i32		_xinet_aton( reg c_i8 *cp, struct in_addr *addr);
void 	*x_malloc( ul32 size );
void 	x_sprintf(i8 *buf, c_i8 *fmt);
void	x_remque( void *ein );
void	x_insque( void *ein, void *previn);

/*------------------- Memory Functions (XOS defines apply)---------------*/
u32
_xminit( u32 s )
{
	/*
 	 * User-specific memory bounds checking
	 * may go here, if desired
	 */
	return s;
}

void *
_xmalloc( ul32 n )
{
#ifdef XOS_NUC
STATUS	status;
VOID 	*return_pointer;

	return_pointer = (void *)0;
	if(n) {
        status = NU_Allocate_Memory( &System_Memory, &return_pointer,
					n, NU_NO_SUSPEND );
		if( status != NU_SUCCESS)
			return (void *)0;
		x_bzero( (i8 *)return_pointer, n );
	}
	return return_pointer;
#endif
#ifdef XOS_PSOS
    return((void *)malloc((u32)n));
#endif
#ifdef XOS_VXWORKS
#endif
#ifdef XOS_XOS
    return((void *)malloc((u32)n));
#endif
}

void
_xfree( void *p )
{
#ifdef XOS_NUC
STATUS	status;

	status = NU_Deallocate_Memory(p);
#endif
#ifdef XOS_PSOS
	free(p);
#endif
#ifdef XOS_VXWORKS
#endif
#ifdef XOS_XOS
	free(p);
#endif
}

/*
 * High-level allocation routines
 */
u32
x_meminit( u32 size )
{
    return(_xminit( size ));
}

void *
x_malloc( ul32 size )
{
    if(!size) {
        return(0);
    }
    return( _xmalloc(size) );
}

void
x_free( void *memblock )
{
	if( memblock )
    	_xfree(memblock);
}

void *
x_realloc( void *memblock, ul32 size )
{
#ifdef XOS_NUC
void	*cp;

	if(!memblock)
		return x_malloc(size);

	cp = x_malloc(size);
	if(cp)
		x_bcopy((i8 *)memblock, (i8 *)cp, size);

	NU_Deallocate_Memory(memblock);
	return cp;
#endif
#ifdef XOS_PSOS
    return((void *)realloc((void *)memblock, (u32)size));
#endif
#ifdef XOS_VXWORKS
#endif
#ifdef XOS_XOS
    return((void *)realloc((void *)memblock, (u32)size));
#endif
}
/*---------------------- Library Functions (XLIB Defines Apply) ------------*/

void
x_bcopy( i8 *s1, i8 *s2, u32 len )
{
#ifdef XLIB_NUC
    memcpy((void *)s2, (const void *)s1, len);
#endif
#ifdef XLIB_BORLAND
    memcpy((void *)s2, (const void *)s1, len);
#endif
#ifdef XLIB_PSOS
    memcpy((void *)s2, (const void *)s1, len);
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    memcpy((void *)s2, (const void *)s1, len);
#endif
#ifdef XLIB_GNU
    memcpy((void *)s2, (const void *)s1, len);
#endif
#ifdef XLIB_XSNMP
	reg c_i8 *f = s1;
	reg char *t = s2;

	while(len != 0) {
		*t++ = *f++;
		len--;
	}
#endif
}

i32
x_bcmp( c_void *s1, c_void *s2, u32 len )
{
#ifdef XLIB_NUC
    return(memcmp(s1,s2,len));
#endif
#ifdef XLIB_BORLAND
    return(memcmp(s1,s2,len));
#endif
#ifdef XLIB_PSOS
    return(memcmp(s1,s2,len));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return(memcmp(s1,s2,len));
#endif
#ifdef XLIB_GNU
    return(memcmp(s1,s2,len));
#endif
#ifdef XLIB_XSNMP
	reg i8 *p1, *p2;

	if(len == 0) return(0);
	p1 = (i8 *)s1;
	p2 = (i8 *)s2;
	do {
		if(*p1++ != *p2++) break;
	} while(--len);
	return(len);
#endif
}

void
x_bzero( i8 *s1, u32 len )
{
#ifdef XLIB_NUC
    memset((void *)s1, 0, len);
#endif
#ifdef XLIB_BORLAND
    memset((void *)s1, 0, len);
#endif
#ifdef XLIB_PSOS
    memset((void *)s1, 0, len);
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    memset((void *)s1, 0, len);
#endif
#ifdef XLIB_GNU
    memset((void *)s1, 0, len);
#endif
#ifdef XLIB_XSNMP
	reg char *t = s1;

	while(len !=0) {
		*t++ = 0;
		len--;
	}
#endif
}

void
x_memcpy( void *s1, c_void *s2, u32 n )
{
#ifdef XLIB_NUC
    memcpy( s1, s2, n );
    return;
#endif
#ifdef XLIB_BORLAND
    return((void *)memcpy( s1, s2, n ));
#endif
#ifdef XLIB_PSOS
    return((void *)memcpy( s1, s2, n ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return((void *)memcpy( s1, s2, n ));
#endif
#ifdef XLIB_GNU
    return((void *)memcpy( s1, s2, n ));
#endif
#ifdef XLIB_XSNMP
	reg c_i8 *f = s2;
	reg char *t = s1;

	while(n != 0) {
		*t++ = *f++;
		n--;
	}
	return((void *)0);
#endif
}

i32
x_memcmp( c_void *s1, c_void *s2, u32 n )
{
#ifdef XLIB_NUC
    return(memcmp( s1, s2, n ));
#endif
#ifdef XLIB_BORLAND
    return(memcmp( s1, s2, n ));
#endif
#ifdef XLIB_PSOS
    return(memcmp( s1, s2, n ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return(memcmp( s1, s2, n ));
#endif
#ifdef XLIB_GNU
    return(memcmp( s1, s2, n ));
#endif
#ifdef XLIB_XSNMP
	if (n != 0) {
		reg c_u8 *p1 = s1, *p2 = s2;

		do {
			if (*p1++ != *p2++)
				return (*--p1 - *--p2);
		} while (--n != 0);
	}
	return (0);
#endif
}

void *
x_memset( void *s1, i32 c, u32 n )
{
#ifdef XLIB_NUC
    return((void *)memset( s1, c, n ));
#endif
#ifdef XLIB_BORLAND
    return((void *)memset( s1, c, n ));
#endif
#ifdef XLIB_PSOS
    return((void *)memset( s1, c, n ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return((void *)memset( s1, c, n ));
#endif
#ifdef XLIB_GNU
    return((void *)memset( s1, c, n ));
#endif
#ifdef XLIB_XSNMP
	reg u8 *dst = (u8 *)s1;

	while(n != 0) {
		*dst++ = c;
		n--;
	}
	return(s1);
#endif
}

i8 *
x_strcat( i8 *d, c_i8 *s )
{
#ifdef XLIB_NUC
    return((i8 *)strcat( d, s ));
#endif
#ifdef XLIB_BORLAND
    return((i8 *)strcat( d, s ));
#endif
#ifdef XLIB_PSOS
    return((i8 *)strcat( d, s ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return((i8 *)strcat( d, s ));
#endif
#ifdef XLIB_GNU
    return((i8 *)strcat( d, s ));
#endif
#ifdef XLIB_XSNMP
	i8 *save = d;

	for(; *d; ++d);
	while(*d++ = *s++);
	return(save);
#endif
}

u32
x_strlen( c_i8 *s )
{
#ifdef XLIB_NUC
    return(strlen( s ));
#endif
#ifdef XLIB_BORLAND
    return(strlen( s ));
#endif
#ifdef XLIB_PSOS
    return(strlen( s ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return(strlen( s ));
#endif
#ifdef XLIB_GNU
    return(strlen( s ));
#endif
#ifdef XLIB_XSNMP
	reg c_i8 *ss;

	for(ss = s; *ss; ++ss);
	return(ss - s);
#endif
}

i32
x_strcmp( c_i8 *s1, c_i8 *s2 )
{
#ifdef XLIB_NUC
    return(strcmp( s1, s2 ));
#endif
#ifdef XLIB_BORLAND
    return(strcmp( s1, s2 ));
#endif
#ifdef XLIB_PSOS
    return(strcmp( s1, s2 ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return(strcmp( s1, s2 ));
#endif
#ifdef XLIB_GNU
    return(strcmp( s1, s2 ));
#endif
#ifdef XLIB_XSNMP
	while(*s1 == *s2++)
		if(*s1++ == 0)
			return(0);
	return(*(u8 *)s1 - *(u8 *)--s2);
#endif
}

i8 *
x_strchr( c_i8 *s, i32 c )
{
#ifdef XLIB_NUC
    return((i8 *)strchr( s, c ));
#endif
#ifdef XLIB_BORLAND
    return((i8 *)strchr( s, c ));
#endif
#ifdef XLIB_PSOS
    return((i8 *)strchr( s, c ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return((i8 *)strchr( s, c ));
#endif
#ifdef XLIB_GNU
    return((i8 *)strchr( s, c ));
#endif
#ifdef XLIB_XSNMP
	for(;; ++s) {
		if(*s == c)
			return((i8 *)s);
		if(!*s)
			return((i8 *)0);
	}
#endif
}

i8 *
x_strcpy( i8 *d, c_i8 *s )
{
#ifdef XLIB_NUC
    return((i8 *)strcpy( d, s ));
#endif
#ifdef XLIB_BORLAND
    return((i8 *)strcpy( d, s ));
#endif
#ifdef XLIB_PSOS
    return((i8 *)strcpy( d, s ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return((i8 *)strcpy( d, s ));
#endif
#ifdef XLIB_GNU
    return((i8 *)strcpy( d, s ));
#endif
#ifdef XLIB_XSNMP
	i8 *save = d;

	for(; *d = *s; ++s, ++d);
	return(save);
#endif
}

i8 *
x_strtok( i8 *s1, c_i8 *s2 )
{
#ifdef XLIB_NUC
    return((i8 *)strtok( s1, s2 ));
#endif
#ifdef XLIB_BORLAND
    return((i8 *)strtok( s1, s2 ));
#endif
#ifdef XLIB_PSOS
    return((i8 *)strtok( s1, s2 ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return((i8 *)strtok( s1, s2 ));
#endif
#ifdef XLIB_GNU
    return((i8 *)strtok( s1, s2 ));
#endif
#ifdef XLIB_XSNMP
	reg i8 *spanp;
	reg int c, sc;
	i8 *tok;
	static i8 *last;

	if (s1 == NULL && (s1 = last) == NULL)
		return (NULL);

cont:
	c = *s1++;
	for(spanp = (i8 *)s2; (sc = *spanp++) != 0;) {
		if(c == sc)
			goto cont;
	}

	if(c == 0) {
		last = NULL;
		return(NULL);
	}
	tok = s1 - 1;

	for(;;) {
		c = *s1++;
		spanp = (i8 *)s2;
		do {
			if((sc = *spanp++) == c) {
				if(c == 0)
					s1 = NULL;
				else
					s1[-1] = 0;
				last = s1;
				return(tok);
			}
		} while(sc != 0);
	}
#endif
}

i8 *
x_strncpy( i8 *dst, c_i8 *src, u32 n )
{
#ifdef XLIB_NUC
    return((i8 *)strncpy( dst, src, n ));
#endif
#ifdef XLIB_BORLAND
    return((i8 *)strncpy( dst, src, n ));
#endif
#ifdef XLIB_PSOS
    return((i8 *)strncpy( dst, src, n ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return((i8 *)strncpy( dst, src, n ));
#endif
#ifdef XLIB_XGNU
    return((i8 *)strncpy( dst, src, n ));
#endif
#ifdef XLIB_XSNMP
	if(n != 0) {
		reg i8 *d = dst;
		reg c_i8 *s = src;

		do {
			if((*d++ = *s++) == 0) {
				while(--n != 0)
					*d++ = 0;
				break;
			}
		} while(--n != 0);
	}
	return (dst);
#endif
}

i32
x_putchar(i32 c)
{
#ifdef XLIB_NUC
	return(putchar(c));
#endif
#ifdef XLIB_BORLAND
	return(putchar(c));
#endif
#ifdef XLIB_PSOS
	return(putchar(c));
#endif
#ifdef XLIB_VXWORKS
	return(putchar(c));
#endif
#ifdef XLIB_XBSD
	return(putc((u32*)c,stdout));
#endif
#ifdef XLIB_XGNU
	/*return(putchar(c));*/
	return(0);
#endif
#ifdef XLIB_XSNMP
	/*
	 * User supplies local putchar/putc here
	 */
	return(0);
#endif
}

i32
x_putnstr( i8 *s, i32 n )
{
i8 *str;
i32 i;

    str = s;
    for(i=0; i<n; i++) {
        if(*str){
            x_putchar(*str++);
        } else
            break;
    }
    return(0);
}

i32
x_putstr( i8 *s )
{
#ifdef XLIB_NUC
/*    return(puts(s)); */
    return(TracePuts(s));
#endif
#ifdef XLIB_BORLAND
    return(puts(s));
#endif
#ifdef XLIB_PSOS
    return(putstr(s));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return(fputs(s, stdout));
#endif
#ifdef XLIB_GNU
    return(puts(s));
#endif
#ifdef XLIB_XSNMP
	/*
	 * User supplies local puts here
	 */
	return(0);
#endif
}

u32
x_atoi( c_i8 *str )
{
#ifdef XLIB_NUC
    return(atoi( str ));
#endif
#ifdef XLIB_BORLAND
    return(atoi( str ));
#endif
#ifdef XLIB_PSOS
    return(atoi( str ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return(atoi( str ));
#endif
#ifdef XLIB_GNU
    return(atoi( str ));
#endif
#ifdef XLIB_XSNMP
	return((i32)xsnmp_strtol(str,(i8 **)0,10));
#endif
}

l32
x_atol( c_i8 *str )
{
#ifdef XLIB_NUC
    return(atol( str ));
#endif
#ifdef XLIB_BORLAND
    return(atol( str ));
#endif
#ifdef XLIB_PSOS
    return(atol( str ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return(atol( str ));
#endif
#ifdef XLIB_GNU
    return(atol( str ));
#endif
#ifdef XLIB_XSNMP
	return(xsnmp_strtol(str,(i8 **)0, 10));
#endif
}

i32
x_abs( i32 val )
{
#ifdef XLIB_NUC
    return(_xabs( val ));
#endif
#ifdef XLIB_BORLAND
    return(_xabs( val ));
#endif
#ifdef XLIB_PSOS
    return(_xabs( val ));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return(abs( val ));
#endif
#ifdef XLIB_GNU
    return(abs( val ));
#endif
#ifdef XLIB_XSNMP
	return(val<0 ? -val:val);
#endif
}

void
x_qsort( void *base, u32 nel, u32 width, i32( *ptr)(const void *, const void *) )
{
#ifdef XLIB_NUC
unsigned unel,uwidth;
	unel = (unsigned)nel;
	uwidth = (unsigned)width;

    qsort( base, unel, uwidth, ptr );
#endif
#ifdef XLIB_BORLAND
unsigned unel,uwidth;
	unel = (unsigned)nel;
	uwidth = (unsigned)width;

    qsort( base, unel, uwidth, ptr );
#endif
#ifdef XLIB_PSOS
    qsort( base, nel, width, ptr );
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    qsort( base, nel, width, ptr );
#endif
#ifdef XLIB_GNU
    qsort( base, nel, width, ptr );
#endif
#ifdef XLIB_XSNMP
    xsnmp_qsort( base, nel, width, ptr );
#endif
}

i32
x_gettimeofday( struct timeval *tp, void *p )
{
#ifdef XLIB_NUC
	return(0);
#endif
#ifdef WINNT
	time_t ltime;
	static time_t sysStartTime=-1;

	if ( sysStartTime < 0 ) {
		time(&sysStartTime);
		tp->tv_sec=0;
	} else {
		time(&ltime);
		tp->tv_usec=0;
		tp->tv_sec=ltime-sysStartTime;
	}
	return(0);
#endif
#ifdef XLIB_BORLAND
    return(0);
#endif
#ifdef XLIB_PSOS
    return(gettimeofday(tp));
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    return(gettimeofday(tp,p));
#endif
#ifdef XLIB_GNU
	/*
	 * User must supply own "gettimeofday" function
	 */
	return(0);
#endif
#ifdef XLIB_XSNMP
	/*
	 * User must supply own "gettimeofday" function
	 */
	return(0);
#endif
}

/*----------------------- Character Output ------------------------------*/

void
x_dbg( i8 *msg, u32 doit )
{
#ifdef XLIB_NUC
	if(doit) printf("%s\n",msg);
#endif
#ifdef XLIB_BORLAND
    if(doit) printf("%s\n",msg);
#endif
#ifdef XLIB_PSOS
u32 iopb[4], ioretval;

    iopb[0] = strlen(msg);
    iopb[1] = (u32)msg;
    if(doit) de_write((1<<16), iopb, &ioretval);
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    if(doit) kprintf("%s\n",msg);
#endif
#ifdef XLIB_GNU
	/*
	 * User must supply debug output routine
	 */
#endif
#ifdef XLIB_GNU
	/*
	 * User must supply debug output routine
	 */
#endif

}

void
x_msg( i8 *msg )
{
#ifdef XLIB_NUC
    printf("%s",msg);
#endif

#ifdef XLIB_BORLAND
    printf("%s",msg);
#endif
#ifdef XLIB_PSOS
u32 iopb[4], ioretval;

    iopb[0] = strlen(msg);
    iopb[1] = (u32)msg;
    de_write((1<<16), iopb, &ioretval);
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    printf("%s",msg);
#endif
#ifdef XLIB_GNU
	/*
	 * User must supply msg output routine
	 */
#endif
#ifdef XLIB_XSNMP
	/*
	 * User must supply msg output routine
	 */
#endif
}

void
x_msgstr( i8 *msg, i8 *str )
{
#ifdef XLIB_NUC
    printf("%s, %s\n",msg,str);
#endif
#ifdef XLIB_BORLAND
    printf("%s, %s\n",msg,str);
#endif
#ifdef XLIB_PSOS
u32 iopb[4], ioretval;

    iopb[0] = strlen(msg);
    iopb[1] = (u32)msg;
    de_write((1<<16), iopb, &ioretval);
    iopb[0] = strlen(str);
    iopb[1] = (u32)str;
    de_write((1<<16), iopb, &ioretval);
#endif
#ifdef XLIB_VXWORKS
#endif
#ifdef XLIB_XBSD
    printf("%s, %s\n",msg,str);
#endif
#ifdef XLIB_GNU
	/*
	 * User must supply msgstr output routine
	 */
#endif
#ifdef XLIB_XSNMP
	/*
	 * User must supply msgstr output routine
	 */
#endif
}


/*------------------- Stack Functions (XSTK defines apply)---------------*/
ul32
x_htonl( ul32 hostlong )
{
#ifdef XSTK_NUCNET
    return(htonl( hostlong ));
#endif
#ifdef XSTK_PSOS
    return(htonl( hostlong ));
#endif
#ifdef XSTK_VXWORKS
#endif
#ifdef XSTK_XBSD
    return(htonl( hostlong ));
#endif
}

u16
x_htons( u16 hostshort )
{
#ifdef XSTK_NUCNET
    return(htons( hostshort ));
#endif
#ifdef XSTK_PSOS
    return(htons( hostshort ));
#endif
#ifdef XSTK_VXWORKS
#endif
#ifdef XSTK_XBSD
    return(htons( hostshort ));
#endif
}

u16
x_ntohs( u16 netshort )
{
#ifdef XSTK_NUCNET
    return(ntohs( netshort ));
#endif
#ifdef XSTK_PSOS
    return(ntohs( netshort ));
#endif
#ifdef XSTK_VXWORKS
#endif
#ifdef XSTK_XBSD
    return(ntohs( netshort ));
#endif
}

ul32
x_ntohl( ul32 netlong )
{
#ifdef XSTK_NUCNET
    return(ntohl( netlong ));
#endif
#ifdef XSTK_PSOS
    return(ntohl( netlong ));
#endif
#ifdef XSTK_VXWORKS
#endif
#ifdef XSTK_XBSD
    return(ntohl( netlong ));
#endif
}

ul32
x_inet_addr( c_i8 *cp )
{
#ifdef XSTK_NUCNET
ul32 _xinet_addr(reg c_i8 *cp);

    return((ul32)_xinet_addr( cp ));
#endif
#ifdef XSTK_PSOS
ul32 _xinet_addr(reg c_i8 *cp);

    return((ul32)_xinet_addr( cp ));
#endif
#ifdef XSTK_VXWORKS
#endif
#ifdef XSTK_XBSD
	return(inet_addr(cp));
#endif
}

i8 *
x_inet_ntoa( struct in_addr in )
{
#ifdef XSTK_NUCNET
i8 * _xinet_ntoa( struct in_addr in);

    return((i8 *)_xinet_ntoa( in ));
#endif
#ifdef XSTK_PSOS
i8 * _xinet_ntoa( struct in_addr in);

    return((i8 *)_xinet_ntoa( in ));
#endif
#ifdef XSTK_VXWORKS
#endif
#ifdef XSTK_XBSD
    return((i8 *)inet_ntoa( in ));
#endif
}

i32
x_socket( i32 domain, i32 type, i32 protocol )
{
#ifdef XSTK_NUCNET
	i32		socketd;

	if( socket_family[(u16)domain] == (u16)-1 ) {
		errno = EAFNOSUPPORT;
		return (u16)-1;
	}
	if( socket_types[(u16)type] == (u16)-1 ) {
		errno = EPROTOTYPE;
		return (u16)-1;
	}
	if( !protocol )
		protocol = PF_INET;
	if( protocol != PF_INET ) {
		errno = EPFNOSUPPORT;
		return (u16)-1;
	}
	socketd = (i32)NU_Socket( socket_family[(u16)domain],
			socket_types[(u16)type], (u16)protocol );

	/*
	 * Convert from NetPlus Error code to BSD Errno value.
	 */
	errno = 0;
	if( socketd < 0 ) {
		if( socketd == NU_INVALID_PROTOCOL )
			errno = EPROTONOSUPPORT;
		else if( socketd == NU_NO_MEMORY )
			errno = ENOMEM;
		else if( socketd == NU_NO_SOCKET_SPACE )
			errno = ECONNREFUSED;
		else
			errno = -1;
		socketd = -1;
	}

	/* Enable "blocking during a read" BSD default it to block. */
	NU_Fcntl( (u16)socketd, NU_SETFLAG, NU_BLOCK);

	/* Return the socket ID or -1 for an error. */
	return socketd;
#endif
#ifdef XSTK_PSOS
    return(socket( domain, type, protocol ));
#endif
#ifdef XSTK_VXWORKS
#endif
#ifdef XSTK_XBSD
    return(socket( domain, type, protocol ));
#endif
}

i32
x_connect( i32 s, struct sockaddr *name, i32 namelen )
{
#ifdef XSTK_NUCNET
struct addr_struct	Addr;
struct sockaddr_in	* so;
i32		socketd;

	so	= (struct sockaddr_in *)name;

	/*
	 * Build the NetPlus Socket structure form the BSD structure.
	 */
	if( so->sin_family != AF_INET ) {
		errno = EPROTONOSUPPORT;
		return -1;
	}
	Addr.family		= so->sin_family;
	if( so->sin_len < 4 ) {		/* Only support IP type protocols. */
		errno = EPROTOTYPE;
		return -1;
	}
	memmove( Addr.id.is_ip_addrs, &so->sin_addr, SIN_ADDR_LEN );
	Addr.name = "Connect";
	/*
	 * Nucleus Net will already swaps the bytes, but in normal BSD code it
	 * requires the App to do the swapping. Here we swap it back for Nucleus.
	 */
	Addr.port = htons(so->sin_port);

	socketd = (i32)NU_Connect( (u16)s, &Addr, (u16)namelen );

	/*
	 * Convert from NetPlus Error code to BSD Errno value.
	 */
	errno = 0;
	if ( socketd < 0 ) {
		if ( socketd == NU_INVALID_SOCKET )
			errno = ENOTSOCK;
		else if ( socketd == NU_NO_PORT_NUMBER )
			errno = EADDRNOTAVAIL;
		else if ( socketd == NU_NO_HOST_NAME )
			errno = EDESTADDRREQ;
		else if ( socketd == NU_NO_MEMORY )
			errno = ENOMEM;
		else
			errno = -1;
		socketd = -1;
	} else
		socketd = 0;			/* connect return zero on success. */

	return socketd;
#endif
#ifdef XSTK_PSOS
    return(connect( s, (struct sockaddr_in *)name, namelen ));
#endif
#ifdef XSTK_VXWORKS
#endif
#ifdef XSTK_XBSD
    return(connect( s, name, namelen ));
#endif
}


i32
x_bind( i32 s, void *nam, i32 namelen )
{
#ifdef XSTK_NUCNET
struct addr_struct	Addr;
i32		socketd;
const struct sockaddr_in *name = (struct sockaddr_in *) nam;

	/*
	 * Build the NetPlus Socket structure form the BSD structure.
	 */
	if( name->sin_family != AF_INET ) {
		errno = EPROTONOSUPPORT;
		return -1;
	}
	Addr.family		= name->sin_family;
	if( name->sin_len != 4 ) {		/* Only support IP type protocols. */
		errno = EPROTOTYPE;
		return -1;
	}
	memmove( Addr.id.is_ip_addrs, &name->sin_addr, SIN_ADDR_LEN );
	Addr.name = "Bind";
	/*
	 * Nucleus Net will already swaps the bytes, but in normal BSD code it
	 * requires the App to do the swapping. Here we swap it back for Nucleus.
	 */
	Addr.port = htons(name->sin_port);

	socketd = (i32)NU_Bind( (u16)s, &Addr, (u16)namelen );
	/*
	 * Convert from NetPlus Error code to BSD Errno value.
	 */
	errno = 0;
	if ( socketd < 0 ) {
		if ( socketd == NU_INVALID_SOCKET )
			errno = ENOTSOCK;
		else
			errno = -1;
		socketd = -1;
	} else
		socketd = 0;	/* BSD Bind returns zero on success. */

	return socketd;
#endif
#ifdef XSTK_PSOS
    return(bind( s, (struct sockaddr_in *)nam, namelen ));
#endif
#ifdef XSTK_VXWORKS
#endif
#ifdef XSTK_XBSD
    return(bind( s, (struct sockaddr *)nam, namelen ));
#endif
}

i32
x_sendto( i32 s, c_i8 *_buf, i32 len, i32 flags, void *too, i32 tolen )
{
#ifdef XSTK_NUCNET
struct addr_struct	Addr;
i32         err, serr;
u16			ss, slen, sflags, stolen;
struct sockaddr_in *to = (struct sockaddr_in *)too;

	/*
	 * Build the NetPlus Socket structure form the BSD structure.
	 */
	if( to->sin_family != AF_INET ) {
		errno = EPROTONOSUPPORT;
		return -1;
	}
	Addr.family		= to->sin_family;
	if( to->sin_len != 4 ) {		/* Only support IP type protocols. */
		errno = EPROTOTYPE;
		return -1;
	}

	x_bcopy( (i8 *)&(to->sin_addr.s_addr), (i8 *)&(Addr.id.is_ip_addrs), SIN_ADDR_LEN);

	Addr.name = "SendTo";
	/*
	 * Nucleus Net will already swaps the bytes, but in normal BSD code it
	 * requi`res the App to do the swapping. Here we swap it back for Nucleus.
	 */
	Addr.port = to->sin_port;

	ss = (u16)s;
	slen = (u16)len;
	sflags = (u16)flags;
	stolen = (u16)tolen;

	serr = NU_Send_To( ss, (i8 *)_buf, slen, sflags, &Addr, 0 /*stolen*/ );

	/*
	 * Convert from NetPlus Error code to BSD Errno value.
	 */
	errno	= 0;
	err	= 0;
	if ( serr < 0 ) {
		if ( serr == (u16)NU_INVALID_SOCKET )
			errno = (u16)ENOTSOCK;
		else if ( serr == (u16)NU_NO_PORT_NUMBER )
			errno = ECONNREFUSED;
		else if ( serr == (u16)NU_NO_DATA_TRANSFER )
			errno = EIO;
		else if ( serr == (u16)NU_INVALID_ADDRESS )
			errno = EADDRNOTAVAIL;
		err = (u16)-1;
	} else {
		err = serr;
	}

	return err;
#endif
#ifdef XSTK_PSOS
    return(sendto(s, (i8 *)_buf, len, flags, (struct sockaddr *)too, tolen ));
#endif
#ifdef XSTK_VXWORKS
#endif
#ifdef XSTK_XBSD
    return(sendto( s, _buf, len, flags, (struct sockaddr *)too, tolen ));
#endif
}

i32
x_recvfrom( i32 s, i8 *_buf, i32 len, i32 flags, void *fro, i32 *fromlenaddr )
{
#ifdef XSTK_NUCNET
struct addr_struct	Addr;
ssize_t			err;
int16			serr, ss, slen, sflags;
int16			sfromlen;
struct sockaddr_in *from = (struct sockaddr_in *)fro;

	ss = (int16)s;
	slen = (int16)len;
	sflags = (uint16)flags;
	serr = NU_Recv_From( ss, (char *)_buf, slen, sflags, &Addr, &sfromlen );
	*fromlenaddr = (i32)sfromlen;

	/*
	 * Build the BSD Socket structure form the NETPLUS structure.
	 */
	from->sin_family = AF_INET;
	from->sin_addr.s_addr = *((ul32 *)&(Addr.id.is_ip_addrs));
	Addr.name = "RecvFrom";
	/*
	 * Nucleus Net will already swaps the bytes, but in normal BSD code it
	 * requires the App to do the swapping.
	 * Here we swap it back for Nucleus NetPlus.
	 */
	from->sin_port = ntohs(Addr.port);

	/*
	 * Convert from NetPlus Error code to BSD Errno value.
	 */
	errno 	= 0;
	err	= 0;
	if ( serr < 0 ) {
		if ( serr == NU_INVALID_SOCKET )
			errno = ENOTSOCK;
		else if ( serr == NU_NO_PORT_NUMBER )
			errno = EADDRNOTAVAIL;
		else if ( serr == NU_NO_DATA_TRANSFER )
			errno = EIO;
		err = -1;
	} else
		err = serr;

	return err;
#endif
#ifdef XSTK_PSOS
    return(recvfrom( s, _buf, len, flags, (struct sockaddr *)fro, fromlenaddr ));
#endif
#ifdef XSTK_VXWORKS
#endif
#ifdef XSTK_XBSD
    return(recvfrom( s, _buf, len, flags, (struct sockaddr *)fro, fromlenaddr ));
#endif
}


/*------------------------- Timer Subsystem -----------------------------*/
ul32
x_timeusec( void )
{
#ifdef XOS_XOS
struct timeval tp;

    gettimeofday(&tp, NULL);
    return(tp.tv_sec * 1000000UL + tp.tv_usec);
#endif
    return 0;
}


ul32
x_timemsec( void )
{
#ifdef XOS_XOS
struct timeval tp;

    gettimeofday(&tp, NULL);
    return(  (tp.tv_sec * 1000UL) + (tp.tv_usec/1000) );
#endif
#ifdef WINNT
struct timeval tp;

   x_gettimeofday(&tp, NULL);
    return(  (tp.tv_sec * 1000UL) + (tp.tv_usec/1000) );
#else
    return 0;
#endif
}

/* Changed to return ticks in hundredths of a second, not seconds per RFC1213 */
ul32
x_timeticks( void )
{
#ifdef XOS_NUC
	return( NU_Retrieve_Clock() / (TICKS_PER_SECOND / 100) );
#else
    return(x_timemsec()/10);
#endif
}

ul32
x_timesec( void )
{
#ifdef XOS_XOS
struct timeval tp;

    gettimeofday(&tp, NULL);
    return(tp.tv_sec);
#endif
    return 0;
}

bool
x_timerinit()
{
#ifdef XOS_NUC
#if 1
	tmr_t *tp;
	int i;

	tmr_active = (tmr_t *)x_malloc( sizeof(tmr_t) );
	x_bzero( (i8 *)tmr_active, sizeof(tmr_t) );
	tmr_active->forw=tmr_active;
	tmr_active->back=tmr_active;
	tp = (tmr_t *)x_malloc( sizeof(tmr_t) * MAX_TIMERS);
	x_bzero( (i8 *)tp , sizeof(tmr_t) * MAX_TIMERS);

	tmr_free=tp;
	tmr_free->forw=tmr_free;
	tmr_free->back=tmr_free;
	for(i=0; i < MAX_TIMERS; i++) {
		tp++;
		tp->tmr_ptr = (NU_TIMER *)x_malloc( sizeof(NU_TIMER) );
		x_insque( tp, tmr_free->back );
	}

#endif
#endif
    return( TRUE );
}

#ifdef XOS_NUC
#include "target.h"
#endif

void
x_timeout( void(*func)(void *), void *arg, i32 ticks )
{
#ifdef XOS_NUC
#if 1
    STATUS    status;
    tmr_t    *tp;


    if( (tp = tmr_free->forw) == tmr_free ) {
        x_msg("timeout: Timer List is empty.\n");
        return;
    }
    x_remque( tp );

	tp->forw = tp;
	tp->back = tp;
    tp->func = func;
    tp->arg  = arg;

#ifdef XOS_NUC
//	printf("Round %d: %x\n", curTimer+1, tp);
	ticks = ticks * TICKS_PER_SECOND / 1000;
#endif

    status = NU_Create_Timer( (NU_TIMER *)tp->tmr_ptr, (CHAR *)tp->name,
				timeout_function,
                (UNSIGNED)tp, (UNSIGNED)ticks, (UNSIGNED)ticks, NU_ENABLE_TIMER);
    if ( status != NU_SUCCESS ) {
        x_msg("timeout: NU_Create_Timer Failed\n");
		tp->func = 0;
		tp->arg  = 0;
		x_insque( tp, tmr_free->back );
    } 	else {
		x_insque( tp, tmr_active->back );
	}
#endif
//	XI_timeout( func, arg, ticks );
#endif
#ifdef XOS_PSOS
#endif
#ifdef XOS_VXWORKS
#endif
#ifdef XOS_XOS
//	timeout( func, *arg, ticks );
#endif
}

void
x_untimeout( void(*func)(void *), void *arg )
{
#ifdef XOS_NUC
#if 1
    tmr_t    * tp;


    if ( (tp = tmr_active->forw) != tmr_active ) {
        for( tp = tmr_active->forw; tp != tmr_active; tp = tp->forw ) {
            if ( (tp->func == func) && (tp->arg == arg) ) {
                x_remque( tp );
                tp->func = 0;
				tp->arg = 0;
                (void)NU_Control_Timer( tp->tmr_ptr, NU_DISABLE_TIMER );
                (void)NU_Delete_Timer( tp->tmr_ptr );
                x_insque( tp, tmr_free->back );
				break;
            }
        }
    }
#endif
//	XI_untimeout( func, arg );
#endif
#ifdef XOS_PSOS
#endif
#ifdef XOS_VXWORKS
#endif
#ifdef XOS_XOS
	untimeout( func, arg );
#endif
}

/*========================= Port Support Functions ======================*/
#ifdef XSTK_PSOS
#define INADDR_NONE        0xffffffff
/*
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 */
ul32
_xinet_addr(reg c_i8 *cp)
{
    struct in_addr val;

    if (_xinet_aton(cp, &val))
        return (val.s_addr);
    return (INADDR_NONE);
}

/*
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
int
_xinet_aton( reg c_i8 *cp, struct in_addr *addr)
{
    reg ul32 val;
    reg i32 base, n;
    reg i8 c;
    u32 parts[4];
    reg u32 *pp = parts;

    for (;;) {
        /*
         * Collect number up to ``.''.
         * Values are specified as for C:
         * 0x=hex, 0=octal, other=decimal.
         */
        val = 0; base = 10;
        if (*cp == '0') {
            if (*++cp == 'x' || *cp == 'X')
                base = 16, cp++;
            else
                base = 8;
        }
        while ((c = *cp) != '\0') {
            if (isascii(c) && isdigit(c)) {
                val = (val * base) + (c - '0');
                cp++;
                continue;
            }
            if (base == 16 && isascii(c) && isxdigit(c)) {
                val = (val << 4) +
                    (c + 10 - (islower(c) ? 'a' : 'A'));
                cp++;
                continue;
            }
            break;
        }
        if (*cp == '.') {
            /*
             * Internet format:
             *    a.b.c.d
             *    a.b.c    (with c treated as 16-bits)
             *    a.b    (with b treated as 24 bits)
             */
            if (pp >= parts + 3 || val > 0xff)
                return (0);
            *pp++ = val, cp++;
        } else
            break;
    }
    /*
     * Check for trailing characters.
     */
    if (*cp && (!isascii(*cp) || !isspace(*cp)))
        return (0);
    /*
     * Concoct the address according to
     * the number of parts specified.
     */
    n = pp - parts + 1;
    switch (n) {

    case 1:                /* a -- 32 bits */
        break;

    case 2:                /* a.b -- 8.24 bits */
        if (val > 0xffffff)
            return (0);
        val |= parts[0] << 24;
        break;

    case 3:                /* a.b.c -- 8.8.16 bits */
        if (val > 0xffff)
            return (0);
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;

    case 4:                /* a.b.c.d -- 8.8.8.8 bits */
        if (val > 0xff)
            return (0);
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
    }
    if (addr)
        addr->s_addr = htonl(val);
    return (1);
}

/*
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */

i8 *
_xinet_ntoa( struct in_addr in)
{
    static i8 b[18];
    reg i8 *p;

    p = (i8 *)&in;
#define    UC(b)    (((i32)b)&0xff)
/* (void)snprintf(b, sizeof(b),  *** NOTE *** */
#ifdef XLIB_XSNMP
	plist[0].intval = UC(p[0]);
	plist[1].intval = UC(p[1]);
	plist[2].intval = UC(p[2]);
	plist[3].intval = UC(p[3]);
   (void)x_sprintf(b, "%d.%d.%d.%d");
#else
   sprintf(b, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
#endif
    return (b);
}

#endif

#ifdef XOS_PSOS
i32
_xabs( i32 j )
{
    return(j < 0 ? -j : j);
}
#endif

#ifdef XSTK_NUCNET
#ifndef INADDR_NONE
#define INADDR_NONE        0xffffffffUL
#endif
/*
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 */
ul32
_xinet_addr(reg c_i8 *cp)
{
    struct in_addr val;

    if (_xinet_aton(cp, &val))
        return (val.s_addr);
    return (INADDR_NONE);
}

/*
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
i32
_xinet_aton( reg c_i8 *cp, struct in_addr *addr)
{
    reg u32 val;
    reg i32 base, n;
    reg i8 c;
    u32 parts[4];
    reg u32 *pp = parts;

    for (;;) {
        /*
         * Collect number up to ``.''.
         * Values are specified as for C:
         * 0x=hex, 0=octal, other=decimal.
         */
        val = 0; base = 10;
        if (*cp == '0') {
            if (*++cp == 'x' || *cp == 'X')
                base = 16, cp++;
            else
                base = 8;
        }
        while ((c = *cp) != '\0') {
            if (isascii(c) && isdigit(c)) {
                val = (val * base) + (c - '0');
                cp++;
                continue;
            }
            if (base == 16 && isascii(c) && isxdigit(c)) {
                val = (val << 4) +
                    (c + 10 - (islower(c) ? 'a' : 'A'));
                cp++;
                continue;
            }
            break;
        }
        if (*cp == '.') {
            /*
             * Internet format:
             *    a.b.c.d
             *    a.b.c    (with c treated as 16-bits)
             *    a.b    (with b treated as 24 bits)
             */
            if (pp >= parts + 3 || val > 0xff)
                return (0);
            *pp++ = val, cp++;
        } else
            break;
    }
    /*
     * Check for trailing characters.
     */
    if (*cp && (!isascii(*cp) || !isspace(*cp)))
        return (0);
    /*
     * Concoct the address according to
     * the number of parts specified.
     */
    n = pp - parts + 1;
    switch (n) {

    case 1:                /* a -- 32 bits */
        break;

    case 2:                /* a.b -- 8.24 bits */
        if (val > 0xffffffUL)
            return (0);
        val |= parts[0] << 24;
        break;

    case 3:                /* a.b.c -- 8.8.16 bits */
        if (val > 0xffff)
            return (0);
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;

    case 4:                /* a.b.c.d -- 8.8.8.8 bits */
        if (val > 0xff)
            return (0);
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
    }
    if (addr)
        addr->s_addr = htonl(val);
    return (1);
}

/*
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */

i8 *
_xinet_ntoa( struct in_addr in)
{
    static i8 b[18];
    reg u8 *p;

    p = (u8 *)&in;
#ifdef XLIB_XSNMP
	plist[0].intval = p[0];
	plist[1].intval = p[1];
	plist[2].intval = p[2];
	plist[3].intval = p[3];
   (void)x_sprintf(b, "%d.%d.%d.%d");
#else
   (void)sprintf(b, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
#endif
    return (b);
}

u32 *
NU_Errno()
{
	NU_TASK	* task;

	task = NU_Current_Task_Pointer();
	if ( !task )
		return (u32 *)&unused_param;

	return (u32 *)&task->tc_app_reserved_1;
}
#endif

#ifdef XOS_NUC
i32
_xabs( i32 j )
{
    return(j < 0 ? -j : j);
}

VOID
timeout_function( ul32 arg )
{
    tmr_t    * tp = (tmr_t *)arg;

    if ( tp->func )
        (*tp->func)(tp->arg);
}
#endif

/*-------------------- General Support Functions --------------------------*/
void
x_insque( void *ein, void *previn)
{
xq_t *e, *prev;

    e = (xq_t *)ein;
    prev = (xq_t *)previn;

    e->xq_prev = prev;
    e->xq_next = prev->xq_next;
    prev->xq_next->xq_prev = e;
    prev->xq_next = e;
}

void
x_remque( void *ein )
{
xq_t *e;

    e = (xq_t *)ein;

    e->xq_prev->xq_next = e->xq_next;
    e->xq_next->xq_prev = e->xq_prev;
}

#ifdef XLIB_XSNMP

static inline i8 *med3 __P((i8 *, i8 *, i8 *, i32 (*)()));
static inline void	 swapfunc __P((i8 *, i8 *, i32, i32));

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */
#define swapcode(TYPE, parmi, parmj, n) { 		\
	l32 i = (n) / sizeof (TYPE); 			\
	reg TYPE *pi = (TYPE *) (parmi); 		\
	reg TYPE *pj = (TYPE *) (parmj); 		\
	do { 						\
		register TYPE	t = *pi;		\
		*pi++ = *pj;				\
		*pj++ = t;				\
        } while (--i > 0);				\
}

#define SWAPINIT(a, es) swaptype = ((i8 *)a - (i8 *)0) % sizeof(l32) || \
	es % sizeof(l32) ? 2 : es == sizeof(l32)? 0 : 1;

static inline void
swapfunc( i8 *a, i8 *b, i32 n, i32 swaptype)
{
	if(swaptype <= 1)
		swapcode(l32, a, b, n)
	else
		swapcode(i8, a, b, n)
}

#define swap(a, b)					\
	if (swaptype == 0) {				\
		l32 t = *(l32 *)(a);			\
		*(l32 *)(a) = *(l32 *)(b);		\
		*(l32 *)(b) = t;			\
	} else						\
		swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n) 	if ((n) > 0) swapfunc(a, b, n, swaptype)

static inline i8 *
med3(i8 *a, i8 *b, i8 *c, i32 (*cmp)())
{
	return cmp(a, b) < 0 ?
	       (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a ))
              :(cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c ));
}

void
xsnmp_qsort(void *a, u32 n, u32 es, i32 (*cmp)() )
{
	i8 *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	i32 d, r, swaptype, swap_cnt;

loop:	SWAPINIT(a, es);
	swap_cnt = 0;
	if (n < 7) {
		for (pm = a + es; pm < (i8 *) a + n * es; pm += es)
			for (pl = pm; pl > (i8 *) a && cmp(pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		return;
	}
	pm = a + (n / 2) * es;
	if (n > 7) {
		pl = a;
		pn = a + (n - 1) * es;
		if (n > 40) {
			d = (n / 8) * es;
			pl = med3(pl, pl + d, pl + 2 * d, cmp);
			pm = med3(pm - d, pm, pm + d, cmp);
			pn = med3(pn - 2 * d, pn - d, pn, cmp);
		}
		pm = med3(pl, pm, pn, cmp);
	}
	swap(a, pm);
	pa = pb = a + es;

	pc = pd = a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (r = cmp(pb, a)) <= 0) {
			if (r == 0) {
				swap_cnt = 1;
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (r = cmp(pc, a)) >= 0) {
			if (r == 0) {
				swap_cnt = 1;
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swap(pb, pc);
		swap_cnt = 1;
		pb += es;
		pc -= es;
	}
	if (swap_cnt == 0) {  /* Switch to insertion sort */
		for (pm = a + es; pm < (i8 *) a + n * es; pm += es)
			for (pl = pm; pl > (i8 *) a && cmp(pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		return;
	}

	pn = a + n * es;
	r = min(pa - (i8 *)a, pb - pa);
	vecswap(a, pb - r, r);
	r = min(pd - pc, pn - pd - es);
	vecswap(pb, pn - r, r);
	if ((r = pb - pa) > es)
		qsort(a, r / es, es, cmp);
	if ((r = pd - pc) > es) {
		/* Iterate rather than recurse to save stack space */
		a = pn - r;
		n = r / es;
		goto loop;
	}
/*		qsort(pn - r, r / es, es, cmp);*/
}

/*
 * Convert a string to a long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
l32
xsnmp_strtol(c_i8 *nptr, i8 **endptr, reg i32 base)
{
reg c_i8 *s = nptr;
reg u32 acc;
reg i32 c;
reg u32 cutoff;
reg i32 neg = 0, any, cutlim;

/*
 * Machine dependent
 */
#define LONG_MIN	(2147483648)
#define LONG_MAX	(2147483647)
	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
	do {
		c = *s++;
	} while (isspace(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for longs is
	 * [-2147483648..2147483647] and the input base is 10,
	 * cutoff will be set to 214748364 and cutlim to either
	 * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
	 * a value > 214748364, or equal but the next digit is > 7 (or 8),
	 * the number is too big, and we will return a range error.
	 *
	 * Set any if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	cutoff = (ul32)neg ? -(ul32)LONG_MIN : (ul32)LONG_MAX;
	cutlim = cutoff % (ul32)base;
	cutoff /= (ul32)base;
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = neg ? LONG_MIN : LONG_MAX;
	} else if (neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = (i8 *)(any ? s - 1 : nptr);
	return (acc);
}

static  i32		pcount = 0;
static	i32		outcnt = 0;
static	i32		neg;;
static	i8		fill_char;

#define PBUF(c)	outcnt++; \
				*buf++ = c

void
x_sprintf(i8 *buf, c_i8 *fmt)
{
reg		i8 *p;
reg		i32 ch;
ul32	ul;
i32		lflag, width, len, *il;
i8 		*pp, buff[(sizeof(long) * 8 / 3) + 1];

	for (;;) {
		while ((ch = *fmt++) != '%') {
			if (ch == '\0') {
				outcnt = 0;
				pcount = 0;
				*buf = '\0';
				x_bzero((i8 *)plist,(sizeof(parm_list_t)*XSNMP_SPRINTF_NUM_PARMS));
				return;
			}
			PBUF(ch);
		}
		lflag = 0;
		neg = 0;
		width = 0;
		fill_char = ' ';
reswitch:
		switch (ch = *fmt++) {
		case 'l':
			lflag = 1;
			goto reswitch;
			break;
		case 'c':
			ch = (i32)(plist[pcount++].charval);
			PBUF((ch & 0x7f));
			break;
		case 's':
			p = plist[pcount++].charptr;
			len = x_strlen(p);
			if(width && !neg) {
				if(width > len) {
					width = width - len;
					while(width--) {
						PBUF(' ');
					}
				}
			}

			while (ch = *p++) {
				PBUF(ch);
			}

			if(width && neg) {
				if(width > len) {
					width = width - len;
					while(width--) {
						PBUF(' ');
					}
				}
			}
			break;
		case 'n':
			il	= plist[pcount++].intptr;
			*il = outcnt;
			break;
		case 'd':
			ul = lflag ?
			    plist[pcount++].longval : plist[pcount++].intval;
			if ((l32)ul < 0) {
				PBUF('-');
				ul = -(l32)ul;
			}
			pp = buff;
			do {
				*pp++ = "0123456789abcdef"[ul % 10];
			} while (ul /= 10);
			*pp = 0;
			if(width && !neg) {
				if(x_strlen(buff) < width) {
					width = width - strlen(buff);
					while(width--) {
						PBUF(fill_char);
					}
				}
			}
			do {
				PBUF(*--pp);
			} while (pp > buff);
			if(width && neg) {
				if(strlen(buff) < width) {
					width = width - strlen(buff);
					while(width--) {
						PBUF(' ');
					}
				}
			}
			break;
		case 'o':
			ul = plist[pcount++].longval;
			pp = buff;
			do {
				*pp++ = "0123456789abcdef"[ul % 8];
			} while (ul /= 8);
			*pp = 0;
			if(width && !neg) {
				if(x_strlen(buff) < width) {
					width = width - strlen(buff);
					while(width--) {
						PBUF(fill_char);
					}
				}
			}
			do {
				PBUF(*--pp);
			} while (pp > buff);
			if(width && neg) {
				if(strlen(buff) < width) {
					width = width - strlen(buff);
					while(width--) {
						PBUF(' ');
					}
				}
			}
			break;
		case 'u':
			ul = plist[pcount++].longval;
			pp = buff;
			do {
				*pp++ = "0123456789abcdef"[ul % 10];
			} while (ul /= 10);
			*pp = 0;
			if(width && !neg) {
				if(x_strlen(buff) < width) {
					width = width - strlen(buff);
					while(width--) {
						PBUF(fill_char);
					}
				}
			}
			do {
				PBUF(*--pp);
			} while (pp > buff);
			if(width && neg) {
				if(strlen(buff) < width) {
					width = width - strlen(buff);
					while(width--) {
						PBUF(' ');
					}
				}
			}
			break;
		case 'x':
			ul = plist[pcount++].longval;
			pp = buff;
			do {
				*pp++ = "0123456789abcdef"[ul % 16];
			} while (ul /= 16);
			*pp = 0;
			if(width && !neg) {
				if(x_strlen(buff) < width) {
					width = width - strlen(buff);
					while(width--) {
						PBUF(fill_char);
					}
				}
			}
			do {
				PBUF(*--pp);
			} while (pp > buff);
			if(width && neg) {
				if(strlen(buff) < width) {
					width = width - strlen(buff);
					while(width--) {
						PBUF(' ');
					}
				}
			}
			break;
		case '%':
			PBUF('%');
			break;
		case '-':
			neg = 1;
			goto reswitch;
		default:
			if ( ch == '*' || (ch >= '0' && ch <= '9') ) {
				if ( ch == '0' )
					fill_char = '0';
				do {
					if ( ch < '0' || ch > '9' )
						break;
					width = width * 10;
					width = width + (ch - '0');
					ch = *fmt++;
				} while(1);
					fmt--;
				goto reswitch;
			}

			if (lflag) {
				PBUF('l');
			}
			PBUF(ch);
			break;
		}
	}
	outcnt = 0;
	pcount = 0;
	*buf = '\0';
	x_bzero((i8 *)plist,(sizeof(parm_list_t)*XSNMP_SPRINTF_NUM_PARMS));
}

#endif
