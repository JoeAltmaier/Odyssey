/*************************************************************************/
/*                                                                       */
/*      Copyright (c) 1993 - 1997 by Accelerated Technology, Inc.        */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/******************************************************************************/
/*                                                                            */
/* FILE NAME                                            VERSION               */
/*                                                                            */
/*   DEV.H                                              NET 4.0               */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*   Definitions for multiple device driver interface.                        */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*      None                                                                  */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*      None                                                                  */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*      NAME                  DATE              REMARKS                       */
/*                                                                            */
/*      Neil F. Henderson     2/10/97           Initial Version                                                */
/*                                                                            */
/******************************************************************************/

#ifndef DEV_H
#define DEV_H

#include "mem_defs.h"
#include "net.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dev_if_address {
        uint8   dev_ip_addr[4];                 /* address of interface */
        uint8   dev_dst_ip_addr[4];             /* other end of p-to-p link */
        uint32  dev_netmask;                    /* used to determine subnet */
        uint32  dev_net;                        /* Network number. */
        uint32  dev_net_brdcast;                /* Network broadcast. */
        IP_MULTI *dev_multiaddrs;
} DEV_IF_ADDRESS;

struct  _DV_DEVICE_ENTRY {

    struct _DV_DEVICE_ENTRY *dev_next;
    struct _DV_DEVICE_ENTRY *dev_previous;
    char                    *dev_net_if_name;   /* Must be unique. */
    int32                   dev_flags;
    uint32                  dev_index;          /* Unique identifier. */
    INT                     dev_irq;
    uint16                  dev_vect;
    uint32                  dev_sm_addr;       /* shared memmory address */
    uint32                  dev_io_addr;
    uint8                   dev_mac_addr[6];   /* Address of device. */

    /* procedure handles */
    STATUS  (*dev_open) (uchar *, DV_DEVICE_ENTRY *);
    STATUS  (*dev_start) (DV_DEVICE_ENTRY *, NET_BUFFER *);
    STATUS  (*dev_receive) (DV_DEVICE_ENTRY *);
    STATUS  (*dev_output) (NET_BUFFER *, DV_DEVICE_ENTRY *,
                            SCK_SOCKADDR_IP *, RTAB_ROUTE *);
    STATUS  (*dev_input) (VOID);
    STATUS  (*dev_ioctl) (DV_DEVICE_ENTRY *, INT, DV_REQ *);

    /* transmit list pointer. */
    NET_BUFFER_HEADER       dev_transq;
    DEV_IF_ADDRESS          dev_addr;    /* Address information. */
    NET_MULTI               *dev_ethermulti; /* List of multicast ethernet
                                                addresses to be received. */

    /* generic interface information */
    uint8           dev_type;       /* ethernet, tokenring, etc */
    uint8           dev_addrlen;    /* media address length */
    uint8           dev_hdrlen;     /* media header length */
    uint32          dev_mtu;        /* maximum transmission unit, excluding media 
                                       header length, i.e., 1500 for ethernet */
    uint32          dev_metric;     /* routing metric (external only) */
    uint32          dev_rip2_sendmode;
    uint32          dev_rip2_recvmode;
    uint32          dev_com_port;
    uint32          dev_baud_rate;
    uint32          dev_data_mode;
    uint32          dev_parity;
    uint32          dev_stop_bits;
    uint32          dev_data_bits;
    
    uint32          user_defined_1; /* Available for users for anything. */
    uint32          user_defined_2; /* Available for users for anything. */
    uint32          system_use_1;   /* Reserverd for System use. */
    uint32          system_use_2;   /* Reserverd for System use. */
};


typedef struct _DV_DEVICE_LIST
{
    struct _DV_DEVICE_ENTRY   *dv_head;
    struct _DV_DEVICE_ENTRY   *dv_tail;
} DV_DEVICE_LIST;

#define     DV_NAME_SIZE    16

struct _DV_REQ
{
    char        dvr_name[DV_NAME_SIZE];
    union 
    {
        uint32      dvru_addr;
        uint32      dvru_dstaddr;
        uint32      dvru_broadaddr;
        uint16      drvu_flags;
        int         dvru_metric;
        uint8       *dvru_data;
    } dvr_dvru;
};

/* These macros simplify access to the fields in struct _DV_REQ. */
#define dvr_addr        dvr_dvru.dvru_addr
#define dvr_dstaddr     dvr_dvru.dvru_dstaddr
#define dvr_broadaddr   dvr_dvru.dvru_broadaddr
#define dvr_flags       dvr_dvru.dvru_flags
#define dvr_metric      dvr_dvru.dvru_metric
#define dvr_data        dvr_dvru.dvru_data

/* Device ioctl options. */
#define DEV_ADDMULTI        1
#define DEV_DELMULTI        2

/*  Defines for the DEV_NET_IF.dev_flags field.  */

#define DV_UP           0x1     /* interface is up                  */
#define DV_BROADCAST    0x2     /* broadcast address valid          */
#define DV_DEBUG        0x4     /* turn on debugging                */
#define DV_LOOPBACK     0x8     /* is a loopback net                */
#define DV_POINTTOPOINT 0x10    /* interface is point-to-point link */
#define DV_NOTRAILERS   0x20    /* avoid use of trailers            */
#define DV_RUNNING      0x40    /* resources allocated              */
#define DV_NOARP        0x80    /* no address resolution protocol   */
#define DV_PROMISC      0x100   /* receive all packets              */
#define DV_ALLMULTI     0x200   /* receive all multicast packets    */
#define DV_OACTIVE      0x400   /* transmission in progress         */
#define DV_SIMPLEX      0x800   /* can't hear own transmissions     */
#define DV_LINK0        0x1000  /* per link layer defined bit       */
#define DV_LINK1        0x2000  /* per link layer defined bit       */
#define DV_LINK2        0x4000  /* per link layer defined bit       */
#define DV_MULTICAST    0x8000  /* supports multicast               */

/* Device types. */
#define DVT_OTHER       1                      
#define DVT_ETHER       2       /* Ethernet */
#define DVT_LOOP        3       /* Loop back interface. */
#define DVT_SLIP        4       /* Serial Line IP */
#define DVT_PPP         5       /* Point to Point Protocol */

typedef struct _UART_INIT_STRUCT
{
    int32       com_port;
    int32       baud_rate;
    int32       data_mode;
    int32       parity;
    int32       stop_bits;
    int32       data_bits;
} UART_DEV;

typedef struct _ETHER_INIT_STRUCT
{
    uint32      dv_irq;
    uint32      dv_io_addr;
    uint32      dv_shared_addr;
} ETHER_DEV;

struct _DEV_DEVICE
{
    CHAR        *dv_name;
    int32       dv_flags;
    STATUS      (*dv_init) (DV_DEVICE_ENTRY *);
    uint8       dv_ip_addr[4];
    uint8       dv_subnet_mask[4];
    uint8       dv_gw[4];

	/* RIP2 Information */
    uint32			dv_use_rip2;	/* non-zero if RIP2 enabled */
    uint32			dv_ifmetric;	/* cost of using this interface */
    uint32			dv_recvmode;	/* controls receiving of RIP2 packets */
    uint32			dv_sendmode;	/* controls sending RIP2 packets */

    /* This union defines the hardware specific portion of the device
       initialization structure. */
    union _dv_hw
    {
        UART_DEV    uart;
        ETHER_DEV   ether;
    } dv_hw;

};

extern DV_DEVICE_LIST    DEV_Table;

/*  DEV.C Function Prototypes.  */
STATUS DEV_Init_Devices (DEV_DEVICE *devices, INT dev_count);
DV_DEVICE_ENTRY *DEV_Get_Dev_For_Vector( int vector );
int     DEV_Get_Ether_Address(char *name, uchar *ether_addr);
INT     DEV_Attach_IP_To_Device(char *name, uint8 *ip_addr, uint8 *subnet);
INT     DEV_Detach_IP_From_Device(char *name);
STATUS  DEV_Init_Route(DV_DEVICE_ENTRY *device);
DV_DEVICE_ENTRY *DEV_Get_Dev_By_Name( CHAR *name );
DV_DEVICE_ENTRY *DEV_Get_Dev_By_Addr( uint8 *addr );

#ifdef __cplusplus
}
#endif

#endif
