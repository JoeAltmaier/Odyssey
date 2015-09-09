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
 | FILE NAME   : xcfig.c
 | VERSION     : 1.1
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Configuration of XSNMPv1
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

extern	void	AddHost(i32 idx, ul32 hst);
extern	void	AddComm(i8 *name, i32 len, i32 access);
extern	void 	AgentSetAuthenTraps( bool );
extern	void 	AgentSetColdTraps( bool );
extern 	i32 	GetCommIndex( i8 * );
extern  u32     NUM_ACTIVE_PORTS;

/*
 * Set the default configuration of the XSNMPv1 agent in the
 * following section.
 */
/*-------------------- Start of User Definable Region ------------------*/

    u8      cfig_date[DATESTRLENGTH] 					= "January 14, 1953";
    u32     cfig_snmpd 									= XSNMP_SNMP_PORT;
    u32     authentrap_onoff                            = OFF;
    u32     coldtrap_onoff                              = OFF;
    u8      cfig_mac[MAC_ETHER_ADDR_LEN]				= { 0x00, 0x20, 0x6E, 0x00, 0x00, 0x01 };
    ul32    cfig_hostid                                 = 0x00000000UL;
    ul32    cfig_netmask                                = 0xFFFFFF00UL;
	u8		cfig_portname[MAX_PORTS][XSNMP_HOSTNAME_LEN]= { "Port1" };
    u8      cfig_hostname[XSNMP_HOSTNAME_LEN+1]			= "XSNMPv1_Host";

	/*
 	 * Set the communities and permissions
     */
	comm_t	Comms[MAX_COMMS]							= {
			{ "public", TRUE, TRUE, TRUE },
			{ "public", TRUE, TRUE, 0    },
			{ "public", TRUE, TRUE, 0    },
			{ "public", TRUE, TRUE, 0    },
			{ "public", TRUE, TRUE, 0    },
			{ "public", TRUE, TRUE, 0    },
			{ "public", TRUE, TRUE, 0    },
			{ "public", TRUE, TRUE, 0    }
			/*-name----read--write-trap-*/
	} ;

	/*
	 * Set the address and community of the accessing host
     * (SNMP Manager)
  	 * This must match the manager that is running against
  	 * the XSNMPv1 agent
     */
	snmphost_t	Hosts[MAX_HOSTS]						= {
            { {0},      {0},        0,          },
            { {0},      {0},        0,          },
            { {0},      {0},        0,          },
            { {0},      {0},        0,          },
            { {0},      {0},        0,          },
            { {0},      {0},        0,          },
			{ {0},		{0},		0,			},
			{ {0},		{0},		0,			},
			{ {0},		{0},		0,			},
			{ {0},		{0},		0,			},
			{ {0},		{0},		0,			},
			{ {0},		{0},		0,			},
			{ {0},		{0},		0,			},
			{ {0},		{0},		0,			},
			{ {0},		{0},		0,			},
			{ {0},		{0},		0,			}
	};

/*-------------------- End of User Definable Region --------------------*/
/*------------------ Utility routines for general configuration --------*/

void 	get_portname( i32 p, u8 *b ) 	{ x_bcopy( (i8 *)&cfig_portname[p], (i8 *)b, XSNMP_HOSTNAME_LEN ); }
ul32	get_portmask(void)				{ return( cfig_netmask ); }
u32  	get_snmpport(void) 				{ return(cfig_snmpd); }
u32  	get_authentrap(void)			{ return(authentrap_onoff); }
u32  	get_coldstarttrap(void)			{ return(coldtrap_onoff); }
ul32  	get_hostid(void) 				{ return(cfig_hostid); }
u32     get_numports(void)              { return(NUM_ACTIVE_PORTS); }
void 	get_xsnmpname( i8 *b ) 			{ x_bcopy( (i8 *)&cfig_hostname[0], (i8 *)b, XSNMP_HOSTNAME_LEN ); }

void    set_portmask(u32 ip ) {
    cfig_netmask = ip;
    printf("xcfig - netmask changed to %8x\n", ip);
}

void    set_hostid( u32 ip ) {
    cfig_hostid = ip;
    printf("xcfig - ip address changed to %8x\n", ip);
}

void 	set_snmpport( u32 val ) 		{ cfig_snmpd = val; }

void
set_portname( i32 p, i8 *b, i32 len )
{
	x_bzero( (i8 *)&cfig_portname[p], XSNMP_HOSTNAME_LEN );
	x_bcopy( (i8 *)b, (i8 *)cfig_portname[p], len );
}

void
set_authentrap( u32 val )
{
	if( val ) 	authentrap_onoff = 1;
	else 		authentrap_onoff = 0;
}

void
set_coldstarttrap( u32 val )
{
	if( val ) 	coldtrap_onoff = 1;
	else 		coldtrap_onoff = 0;
}

void
set_xsnmpname( i8 *b, i32 len )
{
	x_bzero( (i8 *)&cfig_hostname[0], XSNMP_HOSTNAME_LEN );
	x_bcopy( (i8 *)b, (i8 *)&cfig_hostname[0], len );
}

void
nc_init( void )
{
	if( authentrap_onoff == TRUE ) 	AgentSetAuthenTraps( TRUE );
	else					 		AgentSetAuthenTraps( FALSE );
	if(coldtrap_onoff == TRUE) 		AgentSetColdTraps( TRUE );
	else					 		AgentSetColdTraps( FALSE );
}

/*------------------ Utility routines for Community configuration --------*/

i32
cc_count( void )
{
i32 i, cnt;

	cnt = 0;
	for( i=0; i<MAX_COMMS; i++ )
		if( Comms[i].comm_name[0] ) cnt++;
	return(cnt);
}

i32
cc_index( i32 idx )
{
	if( Comms[idx].comm_name[0] ) return(1);
	else return(0);
}

i8 *
cc_name( i32 idx )
{
	return( &Comms[idx].comm_name[0] );
}

i32
cc_len( i32 idx )
{
	return( x_strlen(&Comms[idx].comm_name[0]) );
}

i32
cc_get( i32 idx )
{
	if( Comms[idx].comm_get ) return(1);
	else return(0);
}

i32
cc_set( i32 idx )
{
	if( Comms[idx].comm_set ) return(1);
	else return(0);
}

i32
cc_trap( i32 idx )
{
	if( Comms[idx].comm_trap ) return(1);
	else return(0);
}

i32
set_cc_name( i32 idx, i8 *c, i32 len )
{
	x_bzero( Comms[idx].comm_name, XSNMP_COMMNAME_LEN );
	x_bcopy( c, Comms[idx].comm_name, len );
	return TRUE;
}

i32
set_cc_get( i32 idx, i32 val )
{
	if( (val != 0) && (val != 1) ) return FALSE;
	if( (idx < 0) || (idx > cc_count()) )  return FALSE;
	Comms[idx].comm_get = val;
	return TRUE;
}

i32
set_cc_set( i32 idx, i32 val )
{
	if( (val != 0) && (val != 1) ) return FALSE;
	if( (idx < 0) || (idx > cc_count()) )  return FALSE;
	Comms[idx].comm_set = val;
	return TRUE;
}

i32
set_cc_trap( i32 idx, i32 val )
{
	if( (val != 0) && (val != 1) ) return FALSE;
	if( (idx < 0) || (idx > cc_count()) )  return FALSE;
	Comms[idx].comm_trap = val;
	return TRUE;
}

bool
CommIsTrap( i32 unit )
{
	if( Comms[unit].comm_trap ) return TRUE;
	return FALSE;
}

i8 *
GetCommName( i32 unit )
{
	return( &Comms[unit].comm_name[0] );
}

u32
GetCommLen( i32 unit )
{
	return( x_strlen( &Comms[unit].comm_name[0]) );
}

void
cc_init(void)
{
i32 i, pm;

	for( i=0; i<MAX_COMMS; i++ ) {
		if( Comms[i].comm_name[0] ) {
			if( Comms[i].comm_get && Comms[i].comm_set ) {
					pm = XSNMP_READ_WRITE;
			} else {
				if( Comms[i].comm_get ) pm = XSNMP_READ_ONLY;
				else 					pm = XSNMP_WRITE_ONLY;
			}
			AddComm( Comms[i].comm_name,x_strlen( Comms[i].comm_name ),pm );
		}
	}
}

/*------------------ Utility routines for Hosts configuration --------*/

i8 		*hc_name( i32 idx ) 	{ return( &Hosts[idx].host_name[0] ); }
i32 	hc_len( i32 idx ) 		{ return( x_strlen(&Hosts[idx].host_name[0]) ); }
ul32	hc_ip( i32 idx ) 		{ return( Hosts[idx].host_ip ); }
i8 		*hc_comm( i32 idx ) 	{ return( &Hosts[idx].host_comm[0] ); }
i32 	hc_commlen( i32 idx ) 	{ return( x_strlen(&Hosts[idx].host_comm[0]) ); }


i32
hc_count()
{
i32 i, cnt;

	cnt = 0;
	for( i=0; i<MAX_HOSTS; i++ )
		if( Hosts[i].host_name[0] ) cnt++;
	return(cnt);
}

i32
hc_index( i32 idx )
{
	if( Hosts[idx].host_name[0] ) return(1);
	else return(0);
}


i32
set_hc_ip( i32 idx, ul32 val )
{
	Hosts[idx].host_ip = val;
	return TRUE;
}

i32
set_hc_name( i32 idx, i8 *c, i32 len )
{
    x_bzero( Hosts[idx].host_name, XSNMP_HOSTNAME_LEN );
    x_bcopy( c, Hosts[idx].host_name, len );
    return TRUE;
}

i32
set_hc_comm( i32 idx, i8 *c, i32 len )
{
    x_bzero( Hosts[idx].host_comm, XSNMP_COMMNAME_LEN );
    x_bcopy( c, Hosts[idx].host_comm,  len );
    return TRUE;
}

void
hc_init(void)
{
i32 i;

	for( i=0; i<MAX_HOSTS; i++ ) {
		if( Hosts[i].host_name[0] ) {
			if( GetCommIndex( Hosts[i].host_comm ) ) {
				AddHost( GetCommIndex( Hosts[i].host_comm ), Hosts[i].host_ip );
			}
		}
	}
}

