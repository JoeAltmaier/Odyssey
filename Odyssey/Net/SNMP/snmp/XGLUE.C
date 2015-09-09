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
 | FILE NAME   : xglue.c
 | VERSION     : 1.1
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Miscellaneous glue functions
 | AUTHOR      : Robert Winter
 *************************************************************************/
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"

#ifdef X_PSOS
#include <ctype.h>
#include <pna.h>
#include <probe.h>
#include <prepc.h>
#include <stdio.h>
#endif

#include "xsnmp.h"
#include "snmp.h"
#include "agent.h"
#include "link.h"
#include "prott.h"
#include "mac.h"
#include "xcfig.h"

#ifdef XMEM
extern i8 *XAlloc();
#endif

//extern u32  get_numports(void);
extern i32			xsnmp_initialized;
extern mac_iface_t	*gmac[];

/*
 * The user supplies knowledge of the MAC address size
 * for each port in his system here.
 */
u16
get_macsize( i32 port )
{
	switch(port) {
		default:
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			return(MAC_ETHER_ADDR_LEN);
	}
}

/*
 * The user supplies knowledge of the MAC MTU size
 * for each port in his system here.
 */
ul32
get_macmtu( i32 port )
{
	switch(port) {
		default:
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			return(1500);
	}
}

/*
 * The user supplies knowledge of each interface's speed
 * (in bits per second) for each interface in his system here.
 */
ul32
get_macspeed( i32 port )
{
	switch(port) {
		default:
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			return(10000000L);
	}
}

/*
 * The user supplies knowledge of each interface's MAC address
 * for each interface in his system here.
 */
void
get_macaddr( i32 port, u8 *m )
{
u8 maddr[6] = {0x00,0x20,0x6E,0x00,0x00,0x00};

	x_bcopy( (i8 *)&maddr[0], (i8 *)m, get_macsize(port));
	switch(port) {
		default:
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			*m++ = maddr[0]; *m++ = maddr[1]; *m++ = maddr[2]; *m++ = maddr[3];
			*m++ = maddr[4]; *m++ = (i8)port;
			break;
	}
}

void
get_macbroad( i32 port, u8 *b )
{
u8 broad[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	switch(port) {
		default:
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			x_bcopy( (i8 *)&broad[0], (i8 *)b, get_macsize(port));
			break;
	}
}

mac_iface_t *
GetMAC( i32 unit )
{
reg mac_iface_t       *mac;

    if( !xsnmp_initialized || ((u32)(unit+1) > MAX_PORTS) ) return( (mac_iface_t *)0);

	if( !gmac[unit] ) {
        mac = gmac[unit] = (mac_iface_t *)(MacIfaceGet((u16)(unit+1)));
    } else {
        mac = gmac[unit];
    }
	return( mac );
}

void
SetMAC( i32 unit, mac_iface_t *mac )
{
	gmac[unit] = mac;
}

