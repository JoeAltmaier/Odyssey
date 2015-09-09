/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1998 Accelerated Technology, Inc.           */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      RTAB.H                                            4.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      routing                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Holds the defines for routing.                                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Accelerated Technology Inc.                                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*  global compenent data stuctures defined in this file                 */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      No functions defined in this file                                */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      No other file dependencies                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      NAME                            DATE            REMARKS          */
/*                                                                       */
/*************************************************************************/
#ifndef _RTAB_H
#define _RTAB_H

#include "socketd.h"
#include "dev.h"

#define SEMA_WAIT_TIME		(3 * TICKS_PER_SECOND)
#define RT_LIFE_TIME		(180 * TICKS_PER_SECOND)

#define MORE		0		/* RIGHT */
#define LESS		1		/* LEFT */
#define MIDDLE		2
#define CMP_ERROR	3

#define ISLEAF(n)		(n->rt_child[LESS] == 0 && n->rt_child[MORE] == 0)
#define ISNODE(n)		(n->rt_child[LESS] != 0 || n->rt_child[MORE] != 0)
#define ISROOT(n)		(n->rt_parent == 0)

#define NULL_ROUTE_NODE		(ROUTE_NODE *)0

struct route_node {
	struct route_node *rt_parent;   /* parent to this node */
	struct route_node *rt_child[2];	/* left and right child of node */

	int16           rt_flags;	    /* Up/Down, Host/Net */
	int16           rt_refcnt;		/* # held references */
	uint32          rt_use;			/* # of packets sent over this route */
	uint32          rt_clock;       /* number of clock ticks of last update */
	uint32          rt_lastsent;    /* if clock != lastsent then broadcast */
	SCK_SOCKADDR_IP rt_gateway;		/* gateway for route, if any */
	RIP2_ENTRY      *rt_rip2;       /* holds rip2 entry structure */
	RIP2_AUTH_ENTRY *rt_auth;       /* holds rip2 auth entry structure */
	DV_DEVICE_ENTRY *rt_device;	    /* pointer to the interface structure */
};

typedef struct route_node ROUTE_NODE;

struct destination_addr {
	uchar ip_addr[4];
	uchar ip_mask[4];
};

typedef struct destination_addr DEST_ADDR;

struct route_entry {
	RIP2_ENTRY *rip2;
};

typedef struct route_entry ROUTE_ENTRY;

struct _RTAB_Route
{
    ROUTE_NODE          *rt_route;
    SCK_SOCKADDR_IP     rt_ip_dest;
};

/* Route Flags */
#define RT_UP          0x1         /* route usable */
#define RT_GATEWAY     0x2         /* destination is a gateway */
#define RT_HOST        0x4         /* host entry (net otherwise) */
#define RT_REJECT      0x8         /* host or net unreachable */
#define RT_DYNAMIC     0x10        /* created dynamically (by redirect) */
#define RT_MODIFIED    0x20        /* modified dynamically (by redirect) */
#define RT_DONE        0x40        /* message confirmed */
#define RT_MASK        0x80        /* subnet mask present */
#define RT_CLONING     0x100       /* generate new routes on use */
#define RT_XRESOLVE    0x200       /* external daemon resolves name */
#define RT_LLINFO      0x400       /* generated by ARP or ESIS */
#define RT_STATIC      0x800       /* manually added */
#define RT_BLACKHOLE   0x1000      /* just discard pkts (during updates) */
#define RT_USED        0x4000      /* This entry in the routing table is */
                                   /* being used. */
#define RT_PROTO1      0x8000      /* protocol specific routing flag */


/* The function prototypes known to the outside world. */
ROUTE_NODE *RTAB_Root_Node(void);
int RTAB_Insert_Node( ROUTE_NODE * );
int RTAB_Delete_Node( ROUTE_NODE * );

VOID RTAB_Init(VOID);
ROUTE_NODE *RTAB_Find_Route( SCK_SOCKADDR_IP * );
STATUS RTAB_Add_Route(DV_DEVICE_ENTRY *, uint32, uint32, uint32, int16);
VOID   RTAB_Free( ROUTE_NODE * );
VOID   RTAB_Set_Default_Route(DV_DEVICE_ENTRY *device, uint32 gw, uint16 flags);
ROUTE_NODE *RTAB_Get_Default_Route(VOID);
VOID RTAB_Redirect(uint32, uint32, INT, uint32);

#endif
