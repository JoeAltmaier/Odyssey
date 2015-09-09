/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1998 by Accelerated Technology, Inc.                  */
/*                                                                          */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject */
/* matter of this material.  All manufacturing, reproduction, use and sales */
/* rights pertaining to this subject matter are governed by the license     */
/* agreement.  The recipient of this software implicity accepts the terms   */
/* of the license.                                                          */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*    LCP_DEFS.H                                            2.0             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains constant definitions and structure definitions     */
/* to support the file lcp.c                                                */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah Pollock                                                         */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  lcp_opts_struct                                                         */
/*  lcp_packet_layer_struct                                                 */
/*  lcp_state_struct                                                        */
/*  lcp_mru_option                                                          */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*  protocol.h                                                              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        11/18/97      Updated PPP to version 1.1        */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/

#ifndef LCP_DEFS_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#define LCP_DEFS_H

#include "protocol.h"

#define LCP_MAX_DATA_SIZE               100
#define LCP_CLEAR_HIGH                  0x0000ffff
#define LCP_CONFIG_DONE                 12
#define LCP_CONFIG_SUCCESS              4
#define LCP_CONFIG_FAIL                 8
#define AUTHENTICATION_SUCCESS          16
#define AUTHENTICATION_DONE             24
#define NCP_CONFIG_DONE                 40
#define NCP_CONFIG_SUCCESS              32

/* define basic packet lengths */
#define LCP_HEADER_LENGTH               4
#define LCP_MRU_LENGTH                  4
#define LCP_MAGIC_NUMBER_LENGTH         6
#define LCP_ACCM_LENGTH                 6
#define LCP_PROTOCOL_COMPRESS_LENGTH    2
#define LCP_ADDRESS_COMPRESS_LENGTH     2
#define LCP_CHAP_LENGTH                 5
#define LCP_PAP_LENGTH                  4

/* Define offsets into the lcp pkt */
#define LCP_LENGTH_OFFSET               2
#define LCP_CODE_OFFSET                 0
#define LCP_ID_OFFSET                   1
#define LCP_DATA_OFFSET                 4
#define LCP_CONFIG_LENGTH_OFFSET        1
#define LCP_CONFIG_OPTS                 2

/* define the type of LCP packets */
#define LCP_CONFIGURE_REQUEST           1
#define LCP_CONFIGURE_ACK               2
#define LCP_CONFIGURE_NAK               3
#define LCP_CONFIGURE_REJECT            4
#define LCP_TERMINATE_REQUEST           5
#define LCP_TERMINATE_ACK               6
#define LCP_CODE_REJECT                 7
#define LCP_PROTOCOL_REJECT             8
#define LCP_ECHO_REQUEST                9
#define LCP_ECHO_REPLY                  10
#define LCP_DISCARD_REQUEST             11
#define LCP_CHAP_MD5                    5

/* LCP configuration option numbers */
#define LCP_MAX_RX_UNIT                 1
#define LCP_ASYNC_CONTROL_CHAR_MAP      2
#define LCP_AUTHENTICATION_PROTOCOL     3
#define LCP_QUALITY_PROTOCOL            4
#define LCP_MAGIC_NUMBER                5
#define LCP_PROTOCOL_FIELD_COMPRESS     7
#define LCP_ADDRESS_FIELD_COMPRESS      8

/* LCP options structure */
struct lcp_opts_struct
{
        uint16  max_rx_unit;                    /* max that we can rx     */
        uint32  accm;                           /* async control char map */
        uint16  authentication_protocol;
        uint32  magic_number;
        uint8   protocol_field_compression;
        uint8   address_field_compression;
        uint8   use_accm;                       /* will the accm be used  */
        uint8   use_max_rx_unit;                /* will the MRU be used   */
};

/* define the LCP packet structure */
typedef struct lcp_packet_layer_struct
{
        uint8   code;
        uint8   identifier;
        uint16  length;
        uint8   data;
} LCP_LAYER;


/* define the possible LCP states */
#define INITIAL     0
#define STARTING    1
#define CLOSED      2
#define STOPPED     3
#define CLOSING     4
#define STOPPING    5
#define REQ_SENT    6
#define ACK_RCVD    7
#define ACK_SENT    8
#define OPENED      9

/* define the LCP state structure, this will hold the current state
   of the link and the packet that is in current negotiation. */
struct lcp_state_struct
{
    uint8       state;
    uint8       identifier;
    NET_BUFFER  *negotiation_pkt;
};

typedef struct lcp_mru_option
{
        uint8   type;
        uint8   length;
        uint16  mru;
} LCP_MRU_OPTION;

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif
