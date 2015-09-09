/*************************************************************************/
/*                                                                       */
/*    CopyrIght (c)  1993 - 1996 Accelerated Technology, Inc.            */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/
/*
*
* Portions of this program were written by:       */
/****************************************************************************
*                                                                          *
*      part of:                                                            *
*      TCP/UDP/ICMP/IP Network kernel for NCSA Telnet                      *
*      by Tim Krauskopf                                                    *
*                                                                          *
*      National Center for Supercomputing Applications                     *
*      152 Computing Applications Building                                 *
*      605 E. Springfield Ave.                                             *
*      Champaign, IL  61820                                                *
****************************************************************************
*   'protinit' initializes packets to make them ready for transmission.
*   For many purposes, pre-initialized packets are created for use by the
*   protocol routines, especially to save time creating packets for
*   transmit.
*
*   Important note :  Assumes that the hardware has been initialized and has
*   set all the useful addresses such as the hardware addresses.
*
*   As this is a convenient place for it, this file contains many of the
*   data declarations for packets which are mostly static (pre-allocated).
****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*  protinit.c                                                 4.0          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  global compenent data stuctures defined in this file                    */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*  protinit                                                                */
/*  timerinit                                                               */
/*  udpinit                                                                 */
/*  tcpinit                                                                 */
/*  makeport                                                                */
/*  makeuport                                                               */
/*  setupwindow                                                             */
/*  get_unique_port_number                                                  */
/*                                                                          */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*	No other file dependencies												*/
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*	NAME				DATE		REMARKS 								*/
/*                                                                          */
/*   Tim Krauskopf      10/87        Initial source release.                */
/*   JKM                 5/88        Clean up for new release               */
/*   Craig L. Meredith  02/09/94     Release 1.0.G1.B version               */
/*   Glen Johnson       10/26/94     Updated setupwindow for release        */
/*                                   1.0.G1.E                               */
/*   Maiqi Qian         12/06/96     Fixed the time wrap around (spr0229)   */
/*   Uriah T. Pollock                                                       */
/*   and Don Sharer     05/21/98     Fixed a logic bug in makeport (spr488) */
/*                                                                          */
/****************************************************************************/

/*
 *  Includes
 */

#ifdef PLUS
  #include "nucleus.h"
#else
  #include "nu_defs.h"    /* added during ATI mods - 10/20/92, bgh */
  #include "nu_extr.h"
#endif

#include "protocol.h"
#include "net_extr.h"
#include "socketd.h"
#include "data.h"
#include "target.h"
#include "externs.h"
#include "netevent.h"
#include "tcp_errs.h"
#include "tcpdefs.h"
#include "tcp.h"
#include "sockext.h"
#include "dns.h"

#if SNMP_INCLUDED
#include "snmp_g.h"
#endif


/* Local Prototypes */
static  void setupwindow (struct TCP_Window *, uint16);
static  void tcpinit (void);
static  void udpinit (void);
static  void tcp_timerinit (void);
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      tcp_timerinit                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Initialize the head and tail pointers of both the tcp_timerlist     */
/*   and the tcptimer_freelist to 0.                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*      protinit                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
static void tcp_timerinit()
{
    /* Initialize the head and tail pointers to 0 */
    tcptimer_freelist.flink = tcptimer_freelist.blink = (tqe_t *)0;
    tcp_timerlist.flink = tcp_timerlist.blink = (tqe_t *)0;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      protinit                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Calls all the other packet initialization keep this order as some    */
/*  packet inits require lower layers already be initialized.            */
/*                                                                       */
/* CALLED BY                                                             */
/*      netinit                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      tcp_timerinit                                                    */
/*      arpint                                                           */
/*      IP_Initialize                                                    */
/*      tcpinit                                                          */
/*      udpinit                                                          */
/*      RTAB_Init                                                        */
/*      DNS_Initialize                                                   */
/*                                                                       */
/*************************************************************************/
STATUS protinit(VOID)
{
#if SNMP_INCLUDED
    SNMP_Initialize();
#endif
    tcp_timerinit();                /* init new timer stuff */
    ARP_Init();                     /* ARP packets          */
    IP_Initialize();                /* ip packets           */
    tcpinit();                      /* tcp packets          */
    udpinit();                      /* udp packets          */
    RTAB_Init();
    return (DNS_Initialize());

}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      udpinit                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Initialize the UDP layer.                                            */
/*                                                                       */
/* CALLED BY                                                             */
/*      protinit                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      UTL_Zero                                                         */
/*                                                                       */
/*************************************************************************/
static void udpinit (void)
{
    /* Zero out the UDP portlist. */
    UTL_Zero((CHAR *)uportlist, sizeof(uportlist));

} /* udpinit */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*    tcpinit                                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  setup for makeport ()                                                */
/*                                                                       */
/* CALLED BY                                                             */
/*      protinit                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      UTL_Zero                                                         */
/*                                                                       */
/*************************************************************************/
static void tcpinit(void)
{
      
    /* Zero out the TCP port list. */
    UTL_Zero((CHAR *)portlist, sizeof(portlist));

    tasks_waiting_to_send = 0;

    /* Initialize the task table pointers.  These are pointers for the list of
       listeners (servers).
    */
    Task_Head = Task_Tail = NU_NULL;

}  /* end tcpinit routine */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      makeport                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This is the intialization for TCP based communication.  When a port */
/*   needs to be created, this routine is called to do as much pre-      */
/*   initialization as possible to save overhead during operation.       */
/*                                                                       */
/*   This structure is created upon open of a port, either listening or  */
/*   wanting to send.                                                    */
/*                                                                       */
/*   A TCP port, in this implementation, includes all of the state data  */
/*   for the port connection, a complete packet for the TCP transmission,*/
/*   and two queues, one each for sending and receiving.  The data       */
/*   associated with the queues is in struct window.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*      netlisten                                                        */
/*      netxopen                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                                               */
/*      setupwindow                                                      */
/*      NU_Tcp_Log_Error                                                 */
/*      UTL_Zero                                                         */
/*      get_unique_port_number                                           */
/*      normalize_ptr                                                    */
/*      intswap                                                          */
/*      longswap                                                         */
/*      n_clicks                                                         */
/*      setupwindow                                                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  NAME                  DATE        REMARKS                            */
/*                                                                       */
/*   Uriah T. Pollock                                                    */
/*   and Don Sharer     05/21/98     Fixed a logic bug in the if statment*/
/*                                   that decides if a port can be re-   */
/*                                   used or not (spr488)                */
/*************************************************************************/
int16 makeport (void)
{
    uint16 i, retval;
    struct port *prt, *q;
    /* added during ATI mods - 10/1/92, bgh */
#ifdef PLUS
    STATUS  status;
#else
    int16 status;              /* status of memory allocation */
#endif

    /*
    *  Check to see if any other connection is done with its port buffer
    *  space.  Indicated by the connection state of SCLOSED
    */

    prt = NU_NULL;
    i = 0;

    /* search for a pre-initialized port to re-use */
    do
    {
        q = portlist[i];
        if (q != NU_NULL)
        {
            if (((q->state == SCLOSED) || (q->state == STWAIT))
                && (INT32_CMP(q->out.lasttime + WAITTIME, n_clicks()) < 0))
            {
               prt = q;

               /* Zero out the port structure. */
               UTL_Zero((CHAR *)prt, sizeof(*prt));

               prt -> pindex = i;
            }  /* end if SCLOSED/STWAIT and timeout < n_clicks */
        }  /* end if q != NU_NULL */
        retval = i++;                   /* port # to return */
    } while ((prt == NU_NULL) && (i < NPORTS));

    /*
    * None available pre-allocated, get a new one, about 8.5 K with a 4K
    * windowsize
    */

    if (prt == NU_NULL)
    {

        for (i = 0; (!(i>= NPORTS) && (portlist[i] != NU_NULL)); i++);
            
        if (i >= NPORTS)
        {
	        return (-1);               /* out of room for ports */
        } /* end if */
      

        /* added during ATI mods - 10/1/92, bgh */
#ifdef PLUS
        status = NU_Allocate_Memory(&System_Memory, (void **) &prt,
                                (UNSIGNED)sizeof(struct port),
                                (UNSIGNED)NU_NO_SUSPEND);
#else
        status = NU_Alloc_Memory (sizeof(struct port),
                              (unsigned int **)&prt, NU_WAIT_FOREVER);
#endif

        /* check status of memory allocation */
        if (status == NU_SUCCESS)
        {
            prt = (struct port *)normalize_ptr(prt);
        }
        else
        {
			/* ERROR memory allocation error.\r\n */
			NU_Tcp_Log_Error (TCP_NO_TCP_PORTS, TCP_RECOVERABLE,
							  __FILE__, __LINE__);
            return (-1);               /* out of room for ports */
        } /* end if */

        /* Zero out the port structure. */
        UTL_Zero((CHAR *)prt, sizeof(*prt));

        prt -> pindex = i;
        portlist[i] = prt;
        retval = i;

    } /* end if */

    /* Initialize those fields in the port structure that should be something
       other than 0. */
    prt->tcpout.hlen = 20<<2;                 /* header length << 2 */

    setupwindow (&prt->in, WINDOWSIZE);       /* queuing parameters */
    setupwindow (&prt->out, WINDOWSIZE);

    /* Build a local port number.  Needs to be unique and greater than
       2048.  */
    i = get_unique_port_number();
    prt->in.port = i;                         /* save for incoming comparison */
    prt->tcpout.source = intswap (i);         /* use it for our port number */
    prt->tcpout.seq = longswap (prt->out.nxt);  /* got from setupwindow() above */
    prt->state = SCLOSED;                     /* connection not established yet */
    prt->credit = WINDOWSIZE;                 /* presaved credit */
    prt->sendsize = MAX_SEGMENT_LEN;          /* static size */
    prt->rto = MINRTO;                        /* static timeout */

    /* added to the initialization of a new portlist entry by ATI - 11/9/92 */
#ifndef PLUS
    prt->TXTask = -1;                       /* initialize user task number */
    prt->RXTask = -1;                       /* initialize user task number */
#endif

    return ((int16)retval);
}  /* end makeport */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      makeuport                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This is the intialization for UDP based communication.  When a port */
/*   needs to be created, this routine is called to do as much pre-      */
/*   initialization as possible to save overhead during operation.       */
/*                                                                       */
/*   This structure is created upon open of a port, either listening or  */
/*   wanting to send.                                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_Recv_From                                                     */
/*      NU_Send_To                                                       */
/*      SEL_Check_Recv                                                   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                                               */
/*      intswap                                                          */
/*      NU_Tcp_Log_Error                                                 */
/*      normalize_ptr                                                    */
/*      UTL_Zero                                                         */
/*      get_unique_port_number                                           */
/*                                                                       */
/*************************************************************************/
int16 makeuport (int16 myport, INT socketd)
{
    int16 i, retval = -1;
    struct uport *p;


    /* added during ATI mods - 10/1/92, bgh */
#ifdef PLUS
    STATUS  status;
#else
    int16 status;              /* status of memory allocation */
#endif
    uint16 *return_ptr;       /* pointer to memory block */

#if SNMP_INCLUDED
   struct sock_struct		*sockptr;
   sockptr = socket_list[socketd];
#endif

    /*
     *  Check to see if any other connection is done with its port buffer
     *  space.  Indicated by the connection state of SCLOSED
     */

    p = NU_NULL;
    i = 0;


    for (i = 0; uportlist[i] != NU_NULL; i++)
    {
        if (i >= NUPORTS)
        {
            NU_Tcp_Log_Error (TCP_NO_TCP_PORTS, TCP_RECOVERABLE,
								  __FILE__, __LINE__);
            return (-1);               /* out of room for ports */
        } /* end if */
    } /* end for */

    if (p == NU_NULL)
    {
        /* added during ATI mods - 10/1/92, bgh */
#ifdef PLUS
        status = NU_Allocate_Memory(&System_Memory, (void **) &return_ptr,
                                (UNSIGNED)sizeof(struct uport),
                                (UNSIGNED)NU_NO_SUSPEND);
#else
        status = NU_Alloc_Memory (sizeof(struct uport),
                              (unsigned int **)&return_ptr, NU_WAIT_FOREVER);
#endif

        /* check status of memory allocation */
        if (status == NU_SUCCESS)
        {
            return_ptr = (uint16 *)normalize_ptr(return_ptr);
            p = (struct uport *)return_ptr;
        }
        else
        {
			/* ERROR memory allocation error.\r\n */
            NU_Tcp_Log_Error (UDP_NO_UDP_PORTS, UDP_RECOVERABLE,
							  __FILE__, __LINE__);
            return (-1);               /* out of room for ports */
        } /* end if */

        uportlist[i] = p;
        retval = i;

    } /* end if */

    /* Clear the UDP port structure. */
    UTL_Zero((char *)p, sizeof(*p));

    /* If my port number has not been specified then find a unique one
       for me. */
    if (myport <= 0)
        /* Build a local port number.  Needs to be unique and greater than
           2048.  */
        myport = (int16)get_unique_port_number();

    p->up_lport = intswap((uint16)myport); /* save for incoming comparison */
    p->in_dgrams = 0;                       /* input not received yet */
    p->out_stale = 0;                       /* output not sent yet */

    p->dgram_list.head = NU_NULL;
    p->dgram_list.tail = NU_NULL;

    /* The socket with which this port is associated is unkown at this time. */
    p->up_socketd = socketd;

    p->up_route.rt_route = NU_NULL;

    /* added to the initialization of a new portlist entry by ATI - 11/9/92 */
#ifdef PLUS
    p->TXTask = NU_NULL;                  /* initialize user task number */
    p->RXTask = NU_NULL;                  /* initialize user task number */
#else
    p->TXTask = -1;                       /* initialize user task number */
    p->RXTask = -1;                       /* initialize user task number */
#endif

#if SNMP_INCLUDED
    /* Update the UDP Listen Table. */
    SNMP_udpListenTableUpdate(SNMP_ADD, sockptr->local_addr.ip_num.is_ip_addrs, myport);
#endif

    return (retval);
}  /* end makeuport */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      setupwindow                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Configure information about a window *w.                            */
/*                                                                       */
/* CALLED BY                                                             */
/*      makeport                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*      n_clicks                                                         */
/*                                                                       */
/*************************************************************************/
static void setupwindow (struct TCP_Window *w, uint16 wsize)
{
    w->packet_list.head = w->packet_list.tail = NU_NULL;
    w->ooo_list.head = w->ooo_list.tail = NU_NULL;
    w->nextPacket = NU_NULL;
    w->num_packets = 0;
    w->contain = 0;                     /* nothing here yet */
    w->lasttime = n_clicks();
    w->size = wsize;
    w->push = 0;
    /*
    *  base this on time of day clock, for uniqueness
    */
    w->ack = w->nxt = (((uint32)w->lasttime << 12) & 0x0fffffff);
}

/****************************************************************************/
/*                                                                          */
/*  FUNCTION                                "get_unique_port_number"        */
/*                                                                          */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*                                                                          */
/*      This function derives a new port number and then searches           */
/*      both the TCP portlist and the UDP portlist to see if it has         */
/*      already been used.                                                  */
/*                                                                          */
/*  AUTHOR                                                                  */
/*                                                                          */
/*      Neil Henderson              Accelerated Technology, Inc             */
/*                                                                          */
/*  CALLED FROM                                                             */
/*                                                                          */
/*      makeport                                                            */
/*      makeuport                                                           */
/*                                                                          */
/*  ROUTINES CALLED                                                         */
/*                                                                          */
/*      n_clicks                                                            */
/*                                                                          */
/*  INPUTS                                                                  */
/*                                                                          */
/*      None                                                                */
/*                                                                          */
/*  OUTPUTS                                                                 */
/*                                                                          */
/*      port_number                                                         */
/*                                                                          */
/*  NAME                DATE        REMARKS                                 */
/*                                                                          */
/*  MQ                 02/20/96     Fixed bugs.                             */
/*                                                                          */
/****************************************************************************/
uint16 get_unique_port_number(void)
{
    uint16  i=0;
    int16     j;

	while (i==0)
	{
		i = (uint16) n_clicks ();           /* get a unique number */
		i |= 2048;                          /* make sure >= 0x800 */
		i &= 0x3fff;                        /* and <=0x3fff */

		/*  search for an un-used TCP port.  Search as long as we have
		 *  not searched all ports and we have not found an existing port
		 *  of the same number as the new one.
		 */
		for (j=0; j<NPORTS && i; j++)           /* don't check NULL ports */
            if (portlist[j]!=NU_NULL && intswap(i)==portlist[j]->in.port)
				i = 0;
		for (j=0; j<NUPORTS && i; j++)
            if (uportlist[j]!=NU_NULL && intswap(i)==uportlist[j]->up_lport)
				i = 0;
	}

	return(i);
}

