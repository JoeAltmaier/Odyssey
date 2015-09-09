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
/*      ICMP.H                                            4.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      ICMP - Internet Control Message Protocol                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Holds the defines for the ICMP protocol.                         */
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

/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/



typedef struct ICMP_LAYER_STRUCT
{
    uint8   icmp_type;                  /* Type of ICMP message. */
    uint8   icmp_code;                  /* Sub code. */
    uint16  icmp_cksum;                 /* Ones complement check sum .*/

    union {                             /* ICMP header union. */
        uint8   ih_pptr;

        uint32  ih_gwaddr;

        struct  ih_idseq {
            int16   icd_id;
            int16   icd_seq;
        } ih_idseq;

        int32   ih_void;

        struct ih_pmtu {
            int16   ipm_void;
            int16   ipm_nextmtu;
        }ih_pmtu;

    }icmp_hun;

    union {
        struct id_ts  {
            uint32  its_otime;
            uint32  its_rtime;
            uint32  its_ttime;
        } id_ts;

        IPLAYER id_ip;

        uint32  id_mask;

        int8    id_data[1];

    } icmp_dun;

}ICMP_LAYER;


/* The following definitions make accessing the fields in the unions easier. */
#define icmp_pptr       icmp_hun.ih_pptr
#define icmp_gwaddr     icmp_hun.ih_gwaddr
#define icmp_id         icmp_hun.ih_idseq.icd_id
#define icmp_seq        icmp_hun.ih_idseq.icd_seq
#define icmp_void       icmp_hun.ih_void
#define icmp_pmvoid     icmp_hun.ih_pmtu.ipm_void
#define icmp_nextmtu    icmp_hun.ih_pmtu.ipm_nextmtu

#define icmp_otime      icmp_dun.id_ts.its_otime
#define icmp_rtime      icmp_dun.id_ts.its_rtime
#define icmp_ttime      icmp_dun.id_ts.its_ttime
#define icmp_ip         icmp_dun.id_ip
#define icmp_mask       icmp_dun.id_mask
#define icmp_data       icmp_dun.id_data


/* ICMP Type definitions. */
#define        ICMP_ECHOREPLY                   0
#define        ICMP_UNREACH                     3
#define        ICMP_SOURCEQUENCH                4
#define        ICMP_REDIRECT                    5
#define        ICMP_ECHO                        8
#define        ICMP_TIMXCEED                    11
#define        ICMP_PARAPROB                    12
#define        ICMP_TIMESTAMP                   13
#define        ICMP_TIMESTAMPREPLY              14
#define        ICMP_INFOREQUEST                 15
#define        ICMP_INFOREPLY                   16

/* ICMP code definitions. */
#define        ICMP_UNREACH_NET                 0
#define        ICMP_UNREACH_HOST                1
#define        ICMP_UNREACH_PROTOCOL            2
#define        ICMP_UNREACH_PORT                3
#define        ICMP_UNREACH_NEEDFRAG            4
#define        ICMP_UNREACH_SRCFAIL             5

#define        ICMP_TIMXCEED_TTL                0
#define        ICMP_TIMXCEED_REASM              1

#define        ICMP_PARAPROB_OFFSET             0

#define        ICMP_REDIRECT_NET                0
#define        ICMP_REDIRECT_HOST               1
#define        ICMP_REDIRECT_TOSNET             2
#define        ICMP_REDIRECT_TOSHOST            3


/* Function Prototypes. */
STATUS ICMP_Interpret (NET_BUFFER *buf_ptr, uint32 *);
VOID ICMP_Send_Error(NET_BUFFER *buf_ptr, INT type, INT code, uint32 dest,
                     DV_DEVICE_ENTRY *device);
VOID ICMP_Reflect(NET_BUFFER *buf_ptr);
STATUS ICMP_Echo_Reply (NET_BUFFER *buf_ptr);

