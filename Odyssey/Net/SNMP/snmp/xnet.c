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
 | FILE NAME   : net.c                                  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Generic network access functions                          
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"
#include "xsnmp.h"
#include "snmp.h"
#include "agent.h"
#include "link.h"
#include "prott.h"
#include "mac.h"
#include "xcfig.h"
#include "1213udp.h"


static struct	in_addr tmpaddr;
i32				nwUdpSocket;

static struct sockaddr_in 	*Addr2Sock( i8 *addr);
i8 							*Sock2Addr(struct sockaddr_in *sa);
bool                        IfSendTo( i8 *addr, u8 * frame, i32 length, u16 src_prt);
bool 						IfInit( void );

#define IfaceCount(ifaces)  (sizeof(ifaces) / sizeof(ifaces[0]) - 1)

extern i32  xsnmp_ctl;

bool 
NwInit( void ) 
{
bool rc = TRUE;

	IfInit();

	return rc;
}


bool 
NwDgSendTo( i8 *addr, u8 *frame, i32 length, u16 src_prt )
{
bool result;

	result = IfSendTo(addr, frame, length, src_prt );
	if (result == FALSE) {
		return FALSE;
	}
	return result;
}

bool 
IfInit( void )
{
static bool init = FALSE;
struct sockaddr_in sa;

	if(init) {
		return TRUE;
	}

#ifdef XSTK_XBSD
	nwUdpSocket = x_socket(AF_INET, SOCK_DGRAM, 0);
#endif
	nwUdpSocket = x_socket(AF_INET, SOCK_DGRAM, 0);
	if( nwUdpSocket < 0 ) {
		x_dbg("XSNMP, netudp: init: socket failed\n", TRUE);
		return FALSE;
	}
	sa.sin_family = AF_INET;
	sa.sin_len    = 4;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = x_htons(0);
	if (x_bind((i32)nwUdpSocket, (void *)&sa, sizeof(struct sockaddr_in)) < 0) {
		x_dbg("XSNMP, netudp: init: x_bind failed\n", TRUE);
		return FALSE;
	}

	init = TRUE;

	return init;
}

bool 
IfSendTo( i8 *addr, u8 * frame, i32 length, u16 src_prt )
{
i32 n, s;
struct sockaddr_in *sa;

#if 0
	s = (i32)nwUdpSocket;
#else
    if (src_prt == 161)
        s = (i32)xsnmp_ctl;
    else
        s = (i32)nwUdpSocket;
#endif

	sa = Addr2Sock(addr);
	sa->sin_len = 4;
	if (sa == 0) {
		return FALSE;
	}

	n = x_sendto(s, (i8 *)frame, length, 0, (void *)sa, sizeof(struct sockaddr_in));

	if (n < 0 || n != length) {
		return FALSE;
	}

	return TRUE;
}

struct sockaddr_in *
Addr2Sock( i8 *addr )
{
static struct sockaddr_in sa;
i8 		*p, *q;
static	i8	string[32];

	p = addr;

	q = &string[0];
	while (*p != ':' && *p != 0)
		*q++ = *p++; /*tolower(*p++);  */
	*q++ = 0;

	if(x_strcmp(string, "udp") != 0) {
		x_dbg("XSNMP, netudp: addr2sock: wrong address family\n", TRUE);
		return 0;
	}

	if(*p++ != ':') {
		x_dbg("XSNMP, netudp: addr2sock: colon expected\n", TRUE);
		return 0;
	}

	q = string;
	while (*p != ':' && *p != 0)
		*q++ = *p++;
	*q++ = 0;

	if(x_strcmp(string, "*") == 0) {
		sa.sin_addr.s_addr = INADDR_ANY;
	} else {
#ifdef XSTK_NUCNET
		sa.sin_addr.s_addr = x_htonl(x_inet_addr(string)); 
#else
		sa.sin_addr.s_addr = x_inet_addr(string); 
#endif
	}


	if(*p++ != ':') {
		x_dbg("XSNMP, netudp: addr2sock: colon expected\n", TRUE);
		return 0;
	}

	q = string;
	while (*p != ':' && *p != 0)
		*q++ = *p++;
	*q++ = 0;

	sa.sin_port = x_atoi(string);
	sa.sin_family = AF_INET;

	return &sa;
}

i8 *
Sock2Addr( struct sockaddr_in *sa )
{
static i8 addr[32];

	if(sa->sin_family != AF_INET) {
		x_dbg("XSNMP, netudp: sock2addr: wrong address family\n", TRUE);
		return 0;
	}

#ifdef XLIB_XSNMP
	plist[0].charptr = (i8 *)Inet_NtoA(sa->sin_addr.s_addr);
	plist[1].intval  = (i32)x_ntohs(sa->sin_port);
	x_sprintf(addr, "udp:%s:%d");
#else
	sprintf(addr, "udp:%s:%d", Inet_NtoA(sa->sin_addr.s_addr), x_ntohs(sa->sin_port));
#endif

	return addr;
}

i8 *
Inet_NtoA(ul32 addr)
{
	tmpaddr.s_addr = addr;
	return ((i8 *)x_inet_ntoa(tmpaddr));
}
