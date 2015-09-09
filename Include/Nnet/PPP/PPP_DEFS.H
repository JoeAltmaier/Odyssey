/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1997 by Accelerated Technology, Inc.                  */
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
/*    PPP_DEFS.H                                              2.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains constant definitions and structure definitions     */
/* to support the file ppp.c                                                */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock                                                      */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  PPP_TEMP_BUFFER                                                         */
/*  PPP_OCTET_ACCESS                                                        */
/*  PPP_HEADER                                                              */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*    none                                                                  */
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

#ifndef PPP_DEFS_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#define PPP_DEFS_H


/* Define the sizes for the max TX/RX unit over the PPP link
   and the sizes of the headers and Fast Checksum Sequence (FCS). Note
   that this value strictly defines our max RX unit since it is used to
   declare the size of internal buffers. This value, though, can change
   during PPP negotiation for the max TX unit. So it is possible to have
   different RX and TX max values.  */
#define PPP_MTU                         1500
#define PPP_FCS_SIZE                    2
#define PPP_MAX_PROTOCOL_SIZE           2
#define PPP_MAX_ADDR_CONTROL_SIZE       2

/* Sum the above up. This will be the size used to create the static
   arrays used by the PPP RX LISR to store the incoming bytes. */
#define PPP_MAX_RX_SIZE                 (PPP_MTU + PPP_FCS_SIZE +   \
                                        PPP_MAX_PROTOCOL_SIZE +     \
                                        PPP_MAX_ADDR_CONTROL_SIZE)

/* Size of the HISR Stack. */
#define PPP_HISR_STACK_SIZE             2000

/* PPP protocol numbers */
#define PPP_IP_PROTOCOL                 0x0021
#define PPP_IP_COMPRESS_PROTOCOL        0x21
#define PPP_IP_CONTROL_PROTOCOL         0x8021
#define PPP_LINK_CONTROL_PROTOCOL       0xc021
#define PPP_CHAP_PROTOCOL               0xc223
#define PPP_PAP_PROTOCOL                0xc023

/* PPP HDLC frame numbers */
#define PPP_HDLC_FRAME                  0x7e
#define PPP_HDLC_ADDRESS                0xff
#define PPP_HDLC_CONTROL                0x03
#define PPP_HDLC_CONTROL_ESCAPE         0x7d
#define PPP_HDLC_TRANSPARENCY           0x20
#define PPP_BIT_MASK                    0x00ff
#define PPP_INIT_FCS16                  0xffff  /* Initial FCS value */
#define PPP_GOOD_FCS16                  0xf0b8  /* Good final FCS value */

/* Various defines used for PPP */
#define PPP_PROTOCOL_START              4
#define PPP_WITH_PROT_COMPRESS_START    3
#define PPP_WITH_BOTH_COMPRESS_START    1
#define PPP_WITH_ADDR_COMPRESS_START    2
#define PPP_MAX_ACCM                    32
#define PPP_ONE                         1
#define PPP_TWO                         2
#define PPP_COMPRESS_HEADER             1
#define PPP_COMPRESS_PROTOCOL_HEADER    3
#define PPP_COMPRESS_ADDRESS_HEADER     2
#define PPP_TIMER_THREAD                42

/* Define service completion status constants.  */
#define PPP_TRUE                   NU_TRUE
#define PPP_FALSE                  NU_FALSE
#define PPP_SUCCESS                NU_SUCCESS
#define PPP_LCP_FAILED             -5
#define PPP_NCP_FAILED             -10
#define PPP_LOGIN_FAILED           -7
#define PPP_INVALID_POINTER        NU_INVALID_POINTER
#define PPP_BUSY                   -75
#define PPP_NETWORK_BUSY           13

/* Service parameters */
#define PPP_FORCE                  85
#define PPP_NO_FORCE               58

/* Map them the NU names to be consistent */
#define NU_INVALID_ADDRESS         -37
#define NU_LCP_FAILED              PPP_LCP_FAILED
#define NU_NCP_FAILED              PPP_NCP_FAILED
#define NU_LOGIN_FAILED            PPP_LOGIN_FAILED
#define NU_NETWORK_BUSY            PPP_NETWORK_BUSY

#define NU_FORCE                   PPP_FORCE
#define NU_NO_FORCE                PPP_NO_FORCE

/* Structure used to hold the packet size and ptr so the LISR can give
   it to the HISR. */
typedef struct _ppp_temp_buffer
{
    struct _DV_DEVICE_ENTRY *device;
    uint16					size;
    uint16                  flags;
    uchar                   buffer[PPP_MAX_RX_SIZE];
} PPP_TEMP_BUFFER;

/* Structure used to access individual bytes of a variable that is two
   bytes long. */
typedef struct _octet_access
{
        uint8 byte1;
        uint8 byte2;
} PPP_OCTET_ACCESS;

/* Define the PPP header */

typedef struct _ppp_header
{
        uint8   address;
        uint8   control;
        uint16  protocol;
} PPP_HEADER;


/* PPP Services */
#define NU_Change_Communication_Mode        MDM_Change_Communication_Mode
#define NU_Terminal_Data_Ready              MDM_Data_Ready
#define NU_Dial_PPP_Server                  PPP_Dial
#define NU_Modem_Control_String             MDM_Control_String
#define NU_Wait_For_PPP_Client              PPP_Wait_For_Client
#define NU_Get_Terminal_Char                MDM_Get_Char
#define NU_Purge_Terminal_Buffer            MDM_Purge_Input_Buffer
#define NU_Put_Terminal_Char                UART_Put_Char
#define NU_Carrier                          UART_Carrier
#define NU_Set_PPP_Login                    PPP_Set_Login
#define NU_PPP_Still_Connected              PPP_Still_Connected
#define NU_PPP_Hangup                       PPP_Hangup
#define NU_Change_IP_Mode                   NCP_Change_IP_Mode
#define NU_Set_PPP_Client_IP_Address        NCP_Set_Client_IP_Address


#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif
