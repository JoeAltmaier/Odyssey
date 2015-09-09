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
/*      RTAB.C                                            4.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      Routing                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This module contains the functions for accessing the routing     */
/*      table.                                                           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Kelly Wiles                                                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      <Data Structure> - <Description>                                 */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      RTAB_Add_Route                                                   */
/*      RTAB_Compare_Addresses                                           */
/*      RTAB_Create_Node                                                 */
/*      RTAB_Delete_Node                                                 */
/*      RTAB_Find_Leaf                                                   */
/*      RTAB_Find_Route                                                  */
/*      RTAB_Free                                                        */
/*      RTAB_Get_Default_Route                                           */
/*      RTAB_Init                                                        */
/*      RTAB_Insert_Node                                                 */
/*      RTAB_Network_Class                                               */
/*      RTAB_Root_Node                                                   */
/*      RTAB_Set_Default_Route                                           */
/*      RTAB_Update_Address                                              */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      <Dependency>                                                     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/


#include "nucleus.h"
#include "target.h"
#include "externs.h"
#include "socketd.h"    /* socket interface structures */
#include "tcpdefs.h"
#include "tcp_errs.h"
#include "dev.h"
#include "rip2.h"
#include "rtab.h"
#if SNMP_INCLUDED
#include    "snmp_g.h"
#endif

/* The top or root of the route table tree. */
static ROUTE_NODE *Root_Node = NULL_ROUTE_NODE; 

/* This is the default route to return when no other route can be found. */
ROUTE_NODE *RTAB_Default_Route;

ROUTE_NODE *RTAB_Root_Node( void );
ROUTE_NODE *RTAB_Create_Node( void );
int RTAB_Compare_Addresses( ROUTE_NODE *, ROUTE_NODE * );
int RTAB_Update_Address( ROUTE_NODE *, ROUTE_NODE * );
int16 RTAB_Network_Class(uint8 *);

extern void Hex_Dump_Buffer(uint8 *, uint8 *, uint16);

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Find_Route                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Finds a route to a destination IP address and return the dev.   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      RTAB_Compare_Addresses      Compare two route addresses.         */
/*      DEV_Get_Dev_By_Addr                                              */
/*      RTAB_Network_Class													   */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Returns null if route not found and their is no default route.   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
ROUTE_NODE *RTAB_Find_Route( SCK_SOCKADDR_IP *de )
{
	int             dir = -1;
	RIP2_ENTRY      re;
	ROUTE_NODE      *n, c;
    uint32          ip_addr;

	if( Root_Node == NULL_ROUTE_NODE )
		return( (ROUTE_NODE *)0 );      /* no nodes in tree */

	c.rt_rip2 = &re;
	c.rt_auth = 0;
    memcpy(c.rt_rip2->ip_addr, (char *)&de->sck_addr, 4);

	/* Walk tree searching for a match. */
	n = Root_Node;
	while( n )				/* search for a host match. */
	{
		dir = RTAB_Compare_Addresses( n, &c);
		if ( dir == MIDDLE )
		{
			/* should I check for a route that is of RT_INFINITY? */
			/* should I check for a route that is DOWN? */
			break;
		}
		else
			n = n->rt_child[dir];
	}

	if( dir != MIDDLE )
	{
		/* did not find a host routem now search for a network route. */

        /* Save a copy of the unmodified address. */
        ip_addr = *(uint32 *)c.rt_rip2->ip_addr;

        n = Root_Node;
		while( n )
		{
            /* Use the network mask associated with the device. */
            *(uint32 *)c.rt_rip2->ip_addr = ip_addr 
                & n->rt_device->dev_addr.dev_netmask;

            dir = RTAB_Compare_Addresses( n, &c);
			if ( dir == MIDDLE )
			{
				/* should I check for a route that is of RT_INFINITY? */
				/* should I check for a route that is DOWN? */
				break;
			}
			else
				n = n->rt_child[dir];
		}
	}

    /* If a route was not found then return the default route. */
    if (dir != MIDDLE)
		n = RTAB_Default_Route;

    if (n)
        n->rt_refcnt++;

	return(n);
}	/* RTAB_Find_Route */

/************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      RTAB_Network_Class                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      If IP address is calls A, B or C return true.                    */
/*      A class A address, the 32 bit is off.                            */
/*      A class B address, the 32 bit is on and 31 bit is off.           */
/*      A class C address, the 32 bit is on and 31 bit is on.            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Kelly Wiles, Xact Inc.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      RTAB_Find_Route                                                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      returns 1=A, 2=B and 3=C network class and zero for unknown.     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

int16 RTAB_Network_Class(uint8 *ip_addr)
{
	int16 ret = 0;

	if( (ip_addr[0] >> 6) <= 1 )
		ret = 1;
	else
	if( (ip_addr[0] >> 6) == 2 )
		ret = 2;
	else
	if( (ip_addr[0] >> 6) == 3 )
		ret = 3;

	return(ret);
}	/* RTAB_Network_Class */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Free                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Initialize the default_route strucutrie                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      UDP_Cache_Route                                                  */
/*      TCP_Cleanup                                                      */
/*      RTAB_Redirect                                                    */
/*      IP_Forward                                                       */
/*      IP_Send                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_TCP_Log_Error                                                 */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

VOID RTAB_Free(ROUTE_NODE *rt)
{
    if (rt && (rt->rt_refcnt > 0))
        rt->rt_refcnt--;
    else
        NU_Tcp_Log_Error (RTE_ROUTE_FREE, TCP_RECOVERABLE, __FILE__, __LINE__);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Init                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Initialize the default_route strucutrie                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      protinit                                                         */
/*      NU_Rip2                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

VOID RTAB_Init()
{
    RTAB_Default_Route = NU_NULL;
}	/* RTAB_Init */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Set_Default_Route                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Sets the default route, called by the NU_Rip2 function.         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Device Drivers                                                   */
/*      SCK_Add_Degault_Route                                            */
/*      RTAB_Redirect                                                    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                                               */
/*      UTL_Zero                                                         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
VOID RTAB_Set_Default_Route(DV_DEVICE_ENTRY *device, uint32 gw, uint16 flags)
{
    VOID        *pointer;
    STATUS      status;

    if (!device)
        return;

    if (!RTAB_Default_Route)
    {
        status = NU_Allocate_Memory(&System_Memory, (VOID **) &pointer,
                                sizeof(ROUTE_NODE) + sizeof (RIP2_ENTRY) +
                                sizeof(RIP2_AUTH_ENTRY), (UNSIGNED)NU_NO_SUSPEND);

        if (status != NU_SUCCESS)
            return;

        UTL_Zero(pointer, sizeof(ROUTE_NODE) + sizeof (RIP2_ENTRY) + 
                          sizeof(RIP2_AUTH_ENTRY));

        RTAB_Default_Route = (ROUTE_NODE *)pointer;
        RTAB_Default_Route->rt_rip2 = 
                (RIP2_ENTRY *)((char *)pointer + sizeof (ROUTE_NODE));
        RTAB_Default_Route->rt_auth = 
                (RIP2_AUTH_ENTRY *)((char *)RTAB_Default_Route->rt_rip2 + 
                sizeof (RIP2_ENTRY));
    }

	RTAB_Default_Route->rt_flags = flags | RT_UP | RT_GATEWAY;
	RTAB_Default_Route->rt_refcnt = 0;
	RTAB_Default_Route->rt_use = 0;
	RTAB_Default_Route->rt_clock = 0;
	RTAB_Default_Route->rt_lastsent = 0;
	
    RTAB_Default_Route->rt_gateway.sck_addr = gw;
	RTAB_Default_Route->rt_gateway.sck_family = NU_FAMILY_IP;
	RTAB_Default_Route->rt_gateway.sck_len = sizeof (RTAB_Default_Route->rt_gateway);
    
    /* The destination IP address of a default route should be set to 0. */
    *(uint32 *)RTAB_Default_Route->rt_rip2->ip_addr = 0;
    *(uint32 *)RTAB_Default_Route->rt_rip2->nexthop = gw;
    RTAB_Default_Route->rt_rip2->metric = 1;
    
	RTAB_Default_Route->rt_device = device;

}	/* RTAB_Set_Default_Route */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Root_Node                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Returns the Tree top to the calling function.                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      RIP2_Dump25                                                      */
/*      RIP2_Send_Table                                                  */
/*      RIP2_Find_Route                                                  */
/*      NU_RIP2                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

ROUTE_NODE *RTAB_Root_Node()
{
	return(Root_Node);
}	/* RTAB_Root_Node */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Create_Node                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Create a ROUTE_NODE structure that can be inserted into the      */
/*      RIP2 tree.                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory          Allocate memory from memory pool     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

ROUTE_NODE *RTAB_Create_Node()
{
	VOID *pointer;
	STATUS status;
	static ROUTE_NODE *rn;

	/* Allocate space for the node. */
	status = NU_Allocate_Memory (&System_Memory, &pointer,
									sizeof(ROUTE_NODE), NU_SUSPEND);

	if (status != NU_SUCCESS)
		return((ROUTE_NODE *)0);		/* could not get the memory */

	rn = (ROUTE_NODE *)pointer;

	/* init the elements in ROUTE_NODE */
	rn->rt_parent       = 0;
	rn->rt_child[LESS]  = 0;
	rn->rt_child[MORE]  = 0;
	rn->rt_clock        = 0L;
	rn->rt_lastsent     = 0L;
	rn->rt_rip2         = 0;
	rn->rt_auth         = 0;
	rn->rt_device       = 0;
	rn->rt_flags        = 0;
	rn->rt_refcnt       = 0;
	rn->rt_use          = 0;
	memset(&(rn->rt_gateway), 0, sizeof(SCK_SOCKADDR_IP));

	return(rn);
}	/* RTAB_Create_Node */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Find_Leaf                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Find a leaf node, one that does not have children.              */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      RTAB_Insert_Node                                                 */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      RTAB_Compare_Addresses                                           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      n - node used in search.                                         */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Returns pointer to leaf node or NULL.                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

ROUTE_NODE *RTAB_Find_Leaf( ROUTE_NODE *n )
{
	int dir;
	ROUTE_NODE *s, *r;

	r = Root_Node;

	if( r == NULL_ROUTE_NODE || n == NULL_ROUTE_NODE )
		return( NULL_ROUTE_NODE );

	while( r )
	{
		s = r;				/* save current node */
		dir = RTAB_Compare_Addresses( r, n );
		if( dir == MIDDLE )
			return( r );		/* the nodes equal. */
		else
			r = r->rt_child[dir];   /* move to one of the childern */
	}

	return(s);			/* return found leaf. */
}	/* RTAB_Find_Leaf */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Compare_Addresses                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Compare two addresses and return result.                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      RTAB_Insert_Nodes                                                */
/*      RTAB_Find_Leaf                                                   */
/*      RTAB_Find_Route                                                  */
/*      RIP2_Find_Route                                                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      r - node 1                                                       */
/*      n - node 2                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      MIDDLE - than are equal.                                         */
/*      LESS   - node 1 is less than node 2.                             */
/*      MORE  - node 1 is greater than node 2.                           */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

int RTAB_Compare_Addresses( ROUTE_NODE *r, ROUTE_NODE *n)
{
	uint32 addr1, addr2;
	uint32 raddr1, raddr2;
	uint32 mask;

	if( r == NULL_ROUTE_NODE || n == NULL_ROUTE_NODE )
		return( CMP_ERROR );

	/* use the mask from n, for both addresses. */
	/* Formula (n->addr & n->mask) - (r->addr & n->mask) */

	memcpy((char *)&mask, &r->rt_device->dev_addr.dev_netmask, 4);
	memcpy((char *)&raddr1, n->rt_rip2->ip_addr, 4);
	memcpy((char *)&raddr2, r->rt_rip2->ip_addr, 4);

	addr1 = (raddr1 & mask);
	addr2 = (raddr2 & mask);

    if( addr2 == addr1 )
	{
		if( raddr1 == raddr2 )
			return(MIDDLE);
		else
			if( raddr2 < raddr1 )
			return(LESS);
		else
			return(MORE);
	}
	else
	if( addr2 < addr1 )
		return(LESS);
	else
		return(MORE);

}	/* RTAB_Compare_Addresses */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Add_Route                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Add a new route to the routine table.                           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      RTAB_Redirect                                                    */
/*      NU_Rip2                                                          */
/*      IP_Add_Multi                                                     */
/*      DEV_Init_Route                                                   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      RTAB_Insert_Node                                                 */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Returns pointer to newly inserted node.                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

STATUS RTAB_Add_Route(DV_DEVICE_ENTRY *device, uint32 dest, uint32 mask, uint32 gw, int16 flags)
{
	ROUTE_NODE rn;
	RIP2_ENTRY re;
	RIP2_AUTH_ENTRY ra;
	STATUS st = NU_SUCCESS;

	UTL_Zero(&re, sizeof(RIP2_ENTRY));
	UTL_Zero(&ra, sizeof(RIP2_AUTH_ENTRY));
	UTL_Zero(&rn, sizeof(ROUTE_NODE));

	memcpy(re.ip_addr, (char *)&dest, 4);

	if( mask == 0L )
	{
	    if( (re.ip_addr[0] >> 6) <= 1 )
	    {
		    /* it's a class A network address */
		    re.submask[0] = 255;
		    re.submask[1] = 0;
		    re.submask[2] = 0;
		    re.submask[3] = 0;
	    }
	    else
	    if( (re.ip_addr[0] >> 6) == 2 )
	    {
		    /* it's a class B network address */
		    re.submask[0] = 255;
		    re.submask[1] = 255;
		    re.submask[2] = 0;
		    re.submask[3] = 0;
	    }
	    else
	    if( (re.ip_addr[0] >> 6) == 3 )
	    {
		    /* it's a class C network address */
		    re.submask[0] = 255;
		    re.submask[1] = 255;
		    re.submask[2] = 255;
		    re.submask[3] = 0;
	    }
    }
	else
	{
		memcpy(re.submask, (uint8 *)&mask, sizeof(mask));	
	}


	if( device->dev_metric == 0 )
		re.metric = 1;
	else
		re.metric = device->dev_metric;

	rn.rt_rip2 = &re;
	rn.rt_auth = &ra;

	rn.rt_flags = flags;
	rn.rt_refcnt = 0;
	rn.rt_use = 0;
	rn.rt_device = device;
	memcpy((uint8 *)&rn.rt_gateway.sck_addr, (uint8 *)&gw, sizeof(gw));

	if( RTAB_Insert_Node( &rn ) == -1 )
    {
		st = -1;

        /* Increment the number of routes that could not be added. */
        SNMP_ipRoutingDiscards_Inc;
    }

#if (SNMP_INCLUDED != 0)

    else
    {
        /*
           There are two cases. They are for the type of route. Either a
           direct or indirect. If the next hop is a gateway then the route
           type is indirect. A type of 4 is indirect and 3 is direct. Also
           8 is for the routing protocol type. In this case it is RIP.
        */
        if (flags & RT_GATEWAY)

            /* Add this route to the SNMP routing table */
            SNMP_ipRouteTableUpdate (SNMP_ADD, device->dev_index, (uint8 *)&dest,
                re.metric, -1, -1, -1, -1, (uint8 *)&gw, 4, 8, 0, re.submask, "0, 0");
        else

            /* Add this route to the SNMP routing table */
            SNMP_ipRouteTableUpdate (SNMP_ADD, device->dev_index, (uint8 *)&dest,
                re.metric, -1, -1, -1, -1, (uint8 *)&gw, 3, 8, 0, re.submask, "0, 0");




    }

#endif

	return(st);
}	/* RTAB_Add_Route */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Insert_Node                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Insert a node to either the left or right side of node.         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      RTAB_Add_Route                                                   */
/*      RIP2_Update_Table                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                                               */
/*      NU_Retrieve_Clock                                                */
/*      RTAB_Create_Node                                                 */
/*      RTAB_Find_Leaf                                                   */
/*      RTAB_Compare_Addresses                                           */
/*      RTAB_Update_Address                                              */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      n - node to insert into tree                                     */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Returns TRUE or FALSE.                                           */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

int RTAB_Insert_Node( ROUTE_NODE *n )
{
	int dir;
	int ret = 0;
	VOID *pointer;
	STATUS status;
	ROUTE_NODE *node, *nn, *r;

	r = Root_Node;

	/* If this is the first time called then setup root node */
	if( r == NULL_ROUTE_NODE )
	{
		r = Root_Node = RTAB_Create_Node();

		/* Allocate space for the rip2 structure. */
		status = NU_Allocate_Memory (&System_Memory, &pointer,
									sizeof(RIP2_ENTRY), NU_SUSPEND);

		if (status != NU_SUCCESS)
			return(-1);					/* could not get the memory */

		r->rt_rip2 = (RIP2_ENTRY *)pointer;

		/* Allocate space for the rip2 structure. */
		status = NU_Allocate_Memory (&System_Memory, &pointer,
									sizeof(RIP2_AUTH_ENTRY), NU_SUSPEND);

		if (status != NU_SUCCESS)
			return(-1);					/* could not get the memory */

		r->rt_auth = (RIP2_AUTH_ENTRY *)pointer;

		*(r->rt_rip2) = *(n->rt_rip2);
        r->rt_gateway.sck_addr = n->rt_gateway.sck_addr;
	    r->rt_gateway.sck_family = NU_FAMILY_IP;
	    r->rt_gateway.sck_len = sizeof(r->rt_gateway);
		
        if( n->rt_auth )
			*(r->rt_auth) = *(n->rt_auth);
		else
			r->rt_auth = 0;
		r->rt_device = n->rt_device;
		r->rt_clock = NU_Retrieve_Clock();
		r->rt_refcnt = n->rt_refcnt;
		r->rt_use = n->rt_use;
		r->rt_flags = n->rt_flags;

		return(ret);
	}
	else
	{
		node = RTAB_Find_Leaf(n);
		if( node == NULL_ROUTE_NODE )
			return(-1);
	}
	
	/* find out which side it is to go on. */
	dir = RTAB_Compare_Addresses(node, n);
	switch( dir )
	{
		case CMP_ERROR:
			break;
		case MIDDLE:
			RTAB_Update_Address(node, n);
			nn = n;
			break;
		default:
			/* Do not add infinity routes. (see RFC 1058) */
			if (n->rt_rip2->metric >= RT_INFINITY )
				break;

			/* make sure that child pointer is really null */
			if( node->rt_child[dir] )
				return( -1 );

			/* create a new node */
			nn = RTAB_Create_Node();

			/* make sure that it was allocated */
			if( nn == NULL_ROUTE_NODE )
				return(-1);

			/* Allocate space for the rip2 structure. */
			status = NU_Allocate_Memory (&System_Memory, &pointer,
										sizeof(RIP2_ENTRY), NU_SUSPEND);

			if (status != NU_SUCCESS)
				return(-1);						/* could not get the memory */

			nn->rt_rip2 = (RIP2_ENTRY *)pointer;

			/* Allocate space for the rip2 structure. */
			status = NU_Allocate_Memory (&System_Memory, &pointer,
										sizeof(RIP2_AUTH_ENTRY), NU_SUSPEND);

			if (status != NU_SUCCESS)
				return(-1);						/* could not get the memory */

			nn->rt_auth = (RIP2_AUTH_ENTRY *)pointer;

			/* point mode to new node */
			node->rt_child[dir] =nn;

			/* point new node parent pointer to it's parent */
			nn->rt_parent = node;
            nn->rt_gateway.sck_addr = n->rt_gateway.sck_addr;
	        nn->rt_gateway.sck_family = NU_FAMILY_IP;
	        nn->rt_gateway.sck_len = sizeof (nn->rt_gateway);

			*(nn->rt_rip2) = *(n->rt_rip2);
			if ( n->rt_auth )
				*(nn->rt_auth) = *(n->rt_auth);
			nn->rt_device = n->rt_device;
			nn->rt_clock = NU_Retrieve_Clock();
			nn->rt_refcnt = n->rt_refcnt;
			nn->rt_use = n->rt_use;
			nn->rt_flags = n->rt_flags;
			break;
	}

	return(ret);
}	/* RTAB_Insert_Node */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Update_Address                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Update the node with new data and reset the clock.              */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Retrieve_Clock                                                */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      r - node to update                                               */
/*      n - data to update with.                                         */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Returns 0 for success and -1 for failure.                        */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

int RTAB_Update_Address( ROUTE_NODE *r, ROUTE_NODE *n )
{
	uint32 addr1, addr2, mask1, mask2;

	if( r == NULL_ROUTE_NODE || n == NULL_ROUTE_NODE )
		return( -1 );
	
	/* to see if it really needs to be updated, but always update the clock */

	addr1 = *(uint32 *)r->rt_rip2->ip_addr;
	addr2 = *(uint32 *)n->rt_rip2->ip_addr;
	mask1 = *(uint32 *)r->rt_rip2->submask;
	mask2 = *(uint32 *)n->rt_rip2->submask;
	addr1 = addr1 & mask1;
	addr2 = addr2 & mask2;

        if( r->rt_rip2->metric >  n->rt_rip2->metric || addr1 != addr2 )
	{
		/* What n points to is reused by the caller, so we have to copy it. */
		/* and we do not want to free and reallocate memory. */
		*(r->rt_rip2) = *(n->rt_rip2);
		r->rt_gateway = n->rt_gateway;
		if( n->rt_auth )
			*(r->rt_auth) = *(n->rt_auth);
		else
			r->rt_auth = 0;
		r->rt_device = n->rt_device;  /* only the address to device struct */
	}

	r->rt_clock = NU_Retrieve_Clock();

	return(0);
}	/* RTAB_Update_Address */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       RTAB_Delete_Node                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Delete a node from the left or right side of node.              */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*       Kelly Wiles @ Xact Inc.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Deallocate_Memory      Deallocate memory                      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      dn - node you want deleted.                                      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

int RTAB_Delete_Node( ROUTE_NODE *dn )
{
	ROUTE_NODE *q, *rp, *s, *f;

	if( (Root_Node == NULL_ROUTE_NODE) || (dn == NU_NULL) )
		return(-1);

	/* NOTE: if the refcnt is greater than 1 then we silently keep */
	/* the node and mark the interface as being down. */

	if( dn->rt_refcnt > 0 )
	{
		dn->rt_flags &= ~RT_UP;
		return(0);
	}

	if( ISLEAF(dn) )			/* it's a leaf, then just delete it */
	{
		/* node has no childern attached. */
		rp = dn->rt_parent;

		if( dn->rt_rip2 )
			NU_Deallocate_Memory(dn->rt_rip2);
		if( dn->rt_auth )
			NU_Deallocate_Memory(dn->rt_auth);
		
		dn->rt_device = 0;
		dn->rt_parent = 0;
		dn->rt_refcnt = 0;
		dn->rt_use = 0;
		dn->rt_flags = 0;
		memset(&(dn->rt_gateway), 0, sizeof(SCK_SOCKADDR_IP));

		/* which parent child was I deleting */
		if( rp )
		{
			if(rp->rt_child[LESS] == dn)
				rp->rt_child[LESS] = 0;
			else
				rp->rt_child[MORE] = 0;
		}
		else
			Root_Node = NULL_ROUTE_NODE;
	}
	else
	if( dn->rt_child[LESS] != 0 && dn->rt_child[MORE] != 0 )
	{
		/* node has two childern attached. */
		q = dn->rt_parent;
		rp = dn->rt_child[MORE];
		s = rp->rt_child[LESS];
		while( s )
		{
			f = rp;
			rp = s;
			s = rp->rt_child[LESS];
		}
		if( f != dn )
		{
			f->rt_child[LESS] = rp->rt_child[MORE];
			rp->rt_child[MORE] = dn->rt_child[MORE];
		}
        rp->rt_child[LESS] = dn->rt_child[MORE];

		if( q == 0 )
		{
			Root_Node = rp;
			Root_Node->rt_parent = 0;
		}
		else
		{
			if(rp == q->rt_child[LESS])
				q->rt_child[LESS] = rp;
			else
				q->rt_child[MORE] = rp;
			
			rp->rt_parent = q;
		}
	}
	else
	{
		/* only one child is attached to node */
		q = dn->rt_parent;
		rp = dn;

		/* save off the node that exists. */
		if( dn->rt_child[LESS] )
			s = dn->rt_child[LESS];
		else
			s = dn->rt_child[MORE];

		/* If the dn is the top node, then reset Root_Node */
		if( q == 0 )
		{
			Root_Node = s;
			Root_Node->rt_parent = 0;
		}
		else
		{
			/* find which child of the parent is the deleted node */
			if( rp == q->rt_child[LESS] )
				q->rt_child[LESS] = s;
			else
				q->rt_child[MORE] = s;
			
			s->rt_parent = q;
		}
	}


	NU_Deallocate_Memory(dn);

	return(0);
}	/* RTAB_Delete_Node */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Get_Default_Route                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Get the default route for the NU_Rip2 system.                    */
/*      Note: Caller must allocate memory for the rt_rip2, rt_auth and   */
/*            rt_device structures within the default_route they passed  */
/*            in to this function.                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Kelly Wiles, Xact Inc.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_Rip2                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

ROUTE_NODE *RTAB_Get_Default_Route(VOID)
{
    return RTAB_Default_Route;
}	/* NU_Get_Default_Route */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      RTAB_Redirect                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Process an ICMP redirect.                                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ICMP_Interpret                                                   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      RTAB_Add_Route                                                   */
/*      RTAB_Find_Route                                                  */
/*      RTAB_Free                                                        */
/*      RTAB_Set_Default_Route                                           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      <Inputs>                            <Description>                */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      <Outputs>                           <Description>                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/
VOID RTAB_Redirect(uint32 dst, uint32 gateway, INT flags, uint32 src)
{
    ROUTE_NODE          *rt, *rtg;
    SCK_SOCKADDR_IP     ip_dest;
    SCK_SOCKADDR_IP     gw;
    DV_DEVICE_ENTRY     *temp_dev;


    if (!IP_Localaddr(gateway))
        return;

    ip_dest.sck_family = SK_FAM_IP;
    ip_dest.sck_len = sizeof (ip_dest);
    ip_dest.sck_addr = dst;

    /* Find a route to the original IP address that caused the redirect. */
    if ( (rt = RTAB_Find_Route(&ip_dest)) == NU_NULL)
        return;

    /* Verify the gateway is directly reachable.  A route will be returned if
       the gateway is on this network. */
    gw.sck_addr = gateway;
    if ((rtg = RTAB_Find_Route(&gw)) == 0)
        return;

    /* Validate the redirect.  There are several conditions that must be
       checked.
       - The RTE_DONE flag must not be set.
       - The IP address of the router that sent the redirect must equal the
         current rt_gateway for the destination.
       - The interface for the new gateway must equal the current interface for
         the destination, i.e., the new gateway must be on the same network as
         the current gateway.
     */
    if ( (flags & RT_DONE) || (src != rt->rt_gateway.sck_addr) ||
         (rt->rt_device != rtg->rt_device) )
    {
        RTAB_Free(rt);
        return;
    }

    /* Now make sure that the redirect was not to ourself.  That is that the new
       gateway does not match one of our addresses. */
    for(temp_dev = DEV_Table.dv_head;
        temp_dev != NU_NULL;
        temp_dev = temp_dev->dev_next)
    {
        /* Is there an exact match on the IP address. */
        if (*(uint32 *)temp_dev->dev_addr.dev_ip_addr == gateway)
            break;

        /* Is this a broadcast for our network. */
        if (temp_dev->dev_addr.dev_net == gateway)
            break;
    }

    /* If an interface was found with a matching IP address then this is not a
       valid redirect. */
    if (temp_dev)
    {
        RTAB_Free(rt);
        return;
    }

    /* If the old route used is a default route then create a new entry. A
       default route will have the gateway flag set and will have an IP
       address of 0. */
    if ( (rt->rt_flags & RT_GATEWAY) && (*(uint32 *)rt->rt_rip2->ip_addr == 0) )
    {
        /* Create the new route. */
        RTAB_Set_Default_Route(rt->rt_device, gateway, 
                                (int16)(flags | RT_GATEWAY | RT_DYNAMIC));
    }
    /* Else if the old route is to a gateway. */
    else if (rt->rt_flags & RT_GATEWAY)
    {
        /* If the current route is a network route (HOST flag not set) and the
           redirect is for a host, then create a new route.
         */
        if ( ((rt->rt_flags & RT_HOST) == 0) && (flags & RT_HOST) )
        {
            /* Create the new route. */
            RTAB_Add_Route( rt->rt_device, dst, 0, gateway,
                           (int16)(flags | RT_GATEWAY | RT_DYNAMIC) );




        }
        else
        {
            /* Modify the existing route. */
            rt->rt_flags |= RT_MODIFIED;
            rt->rt_gateway.sck_addr = gateway;
            *(uint32 *)rt->rt_rip2->nexthop = gateway;
        }
    }

    /* If a route was located, free it. */
    if (rt)
        RTAB_Free(rt);

} /* RTAB_Redirect */
