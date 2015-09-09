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
/*      IGMP.H                                            4.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      IGMP -     Implements the IGMP protocol.                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This header file holds all the includes and defines used in IGMP.*/
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


typedef struct _IGMP_LAYER
{
    uint8   igmp_type;
    uint8   igmp_unused;
    uint16  igmp_cksum;
    uint32  igmp_group;
} IGMP_LAYER;

/* This is the IP address that all level 2 conforming hosts should be a member of. */
#define IGMP_ALL_HOSTS_GROUP   0xE0000001 

/* IGMP packet types. */
#define IGMP_HOST_MEMBERSHIP_QUERY      0x11
#define IGMP_HOST_MEMBERSHIP_REPORT     0x12


/* This macro is used to send a IGMP report after some time delay. */
#define IGMP_REPORT_EVENT(ipm) \
/* IP_MULTI *ipm; */            \
{ \
    if ((ipm) != NU_NULL) \
    { \
        /* Send a report of the membership.*/ \
        IGMP_Send((IP_MULTI *)(ipm)); \
        ((IP_MULTI *)ipm)->ipm_timer = 0; \
    } \
} /* IGMP_Report_Event */

STATUS  IGMP_Initialize(DV_DEVICE_ENTRY *device);
VOID    IGMP_Join(IP_MULTI *ipm);
VOID    IGMP_Send(IP_MULTI *ipm);
uint32  IGMP_Random_Delay(IP_MULTI *ipm);
VOID    IGMP_Report_Event(IP_MULTI *ipm);
VOID    IGMP_Leave(IP_MULTI *ipm);
STATUS  IGMP_Interpret(NET_BUFFER *buf_ptr, INT hlen);
