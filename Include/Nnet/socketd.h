/*************************************************************************/
/*                                                                       */
/*    Copyright (c) 1993 - 1996 by Accelerated Technology, Inc.  NET 2.3 */
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
/*      SOCKETD.H                                         4.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      Sockets                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Holds the defines for data structures related to sockets.        */
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

#ifndef SOCKETD_H
#define SOCKETD_H

#ifndef __cplusplus
#ifdef PLUS
    #include "nucleus.h"
#else
    #include "nu_defs.h"    /* added during ATI mods - 10/20/92, bgh */
#endif
#endif

#include "sockdefs.h"     /* socket definitions */
#include "target.h"

#ifdef PLUS
        #define            NU_IGNORE_VALUE  -1   /* Null parameter value     */
#endif

#define      NULL_IP        0   /* Used to initialize ip addresses to NULL */



/* 32-bit structure containing 4-digit ip number */
struct id_struct
{
    uchar is_ip_addrs[4];        /* IP address number */
};

struct addr_struct
{
    int16    family;             /* family = INTERNET */
    uint16   port;               /* machine's port number */
    struct   id_struct id;       /* contains the 4-digit ip number for the host machine */
    char     *name;              /* points to machine's name */
};

struct sockaddr_struct
{
    struct id_struct ip_num;     /* the address = the ip num */
    int16  port_num;             /* the process = the port num */
    int16  pad;
};


typedef struct SCK_SOCKADDR_STRUCT
{
    uint8       sck_len;
    uint8       sck_family;
    int8        sck_data[14];
}SCK_SOCKADDR;

struct SCK_IP_ADDR_STRUCT
{
    uint32      sck_ip_addr;
};


struct SCK_SOCKADDR_IP_STRUCT
{
    uint8           sck_len;
    uint8           sck_family;
    uint16          sck_port;
    uint32          sck_addr;
    int8            sck_unused[8];
};


/* this is the socket 5-tuple */
struct sock_struct
{
  struct sockaddr_struct    local_addr;
  struct sockaddr_struct    foreign_addr;
  NET_BUFFER_HEADER         s_recvlist;     /* List of received packets. */
  NU_TASK                   *s_RXTask;		/* Task pending on a receive. */
  NU_TASK                   *s_TXTask;		/* Task pending on a transmit. */
  uint32                    s_recvbytes;	/* Total bytes in s_recvlist. */
  uint32                    s_recvpackets;	/* Total packets in s_recvlist. */
  IP_MULTI_OPTIONS          *s_moptions;    /* IP multicast options. */
  uint16                    s_state;        /* Internal state flags. */
  uint16                    s_options;      /* Socket options as defined by BSD.  Currently */
                                            /* the only implemented option is SO_BROADCAST. */
  uint16                    s_flags;
  uint16                    protocol;
};

/*
 * Socket state bits.
 */
#define SS_NOFDREF              0x001   /* no file table ref any more */
#define SS_ISCONNECTED          0x002   /* socket connected to a peer */
#define SS_ISCONNECTING         0x004   /* in process of connecting to peer */
#define SS_ISDISCONNECTING      0x008   /* in process of disconnecting */
#define SS_CANTSENDMORE         0x010   /* can't send more data to peer */
#define SS_CANTRCVMORE          0x020   /* can't receive more data from peer */
#define SS_RCVATMARK            0x040   /* at mark on input */
#define SS_PRIV                 0x080   /* privileged for broadcast, raw... */
#define SS_NBIO                 0x100   /* non-blocking ops */
#define SS_ASYNC                0x200   /* async i/o notify */
#define SS_ISCONFIRMING         0x400   /* deciding to accept connection req */

/*
 *  Socket Flag bits.
 */
#define SF_BLOCK                0x0001  /* Indicates blocking or non-blocking */
#define SF_LISTENER             0x0002  /* Is a TCP server listening */

/* task table structure - created during an NU_Listen call to
   store status on x number of connections for a single port number
   from a single task id */

struct TASK_TABLE_STRUCT
{
  struct TASK_TABLE_STRUCT *next;  /* pointer to the next task structure */
#ifdef PLUS
  NU_TASK *Task_ID;
#else
  int16 Task_ID, pad2;     /* task which is willing to accept a connection */
#endif
  short *stat_entry;       /* status of each connection */
  int16 *port_entry;       /* portlist entry number of each connection */
  int16  socketd;
  uint16 local_port_num;   /* port number of server */
  uint16 current_idx;      /* points to oldest entry in the table; a task
                              should service this connection before the others */
  uint16 total_entries;    /* number of backlog queues possible */
  int8   acceptFlag;       /* Used to indicate that the task is suspended in the
                              NU_Accept service. */
  int8   pad[3];
};

/* host structure */
typedef struct NU_Host_Ent
{
  char   *h_name;
  char   **h_alias;        /* unused */
  int16  h_addrtype;
  int16  h_length;
  char   *h_addr;          /* contains the host's 4-digit ip number */
} NU_HOSTENT;

#define NU_Get_Host_by_NAME   NU_Get_Host_By_Name

/* Host information.  Used to match a host name with an address.  Used in
   hosts.c to setup information on foreign hosts. */
struct host
{
        char   name[32];
        uchar   address[4];
};

/* Defines added for the NU_Select service call. */
#define FD_BITS                 32
#define FD_ELEMENTS     (NSOCKETS/FD_BITS)+1

typedef struct fd_set
{
        uint32 words[FD_ELEMENTS];
} FD_SET;


/* Clear the connecting flag and set the connected flag. */
#define		SCK_CONNECTED(desc)	\
        if (socket_list[desc])                              \
        {                                                   \
		  socket_list[desc]->s_state &= (~SS_ISCONNECTING); \
		  socket_list[desc]->s_state |= SS_ISCONNECTED;     \
        }

/* Change the the socket state to disconnecting. */
#define		SCK_DISCONNECTING(desc) \
            if (socket_list[desc])                                  \
            {                                                       \
				socket_list[desc]->s_state &= (~SS_ISCONNECTED);    \
				socket_list[desc]->s_state |= SS_ISDISCONNECTING;   \
            }

/* IP Multicast Request structure. This structure is used when using 
   NU_Setsockopt or NU_Getsockopt to set or get IP multicasting options. */
typedef struct _ip_mreq {
    uint32      sck_multiaddr;      /* IP multicast address. */
    uint32      sck_inaddr;         /* IP address of the interface. */
} IP_MREQ;


/*  Miscellaneuos Defines for application layer interface to DEV, DHCP and BOOTP structures. */
#define  NU_DEVICE              DEV_DEVICE
#define  NU_BOOTP_STRUCT        BOOTP_STRUCT
#define  NU_DHCP_STRUCT         DHCP_STRUCT
#define  NU_Init_Devices        DEV_Init_Devices

/* Option commands for the NU_Ioctl service call. */
#define IOCTL_GETIFADDR     1       /* Get the IP address associated with an 
                                       interface. */

typedef struct _SCK_IOCTL_OPTION
{
    uint8       *s_optval;
    
    union 
    {
        uint8   s_ipaddr[4];
    } s_ret;

} SCK_IOCTL_OPTION;

#define NU_IOCTL_OPTION     SCK_IOCTL_OPTION

#endif  /* SOCKETD_H */
