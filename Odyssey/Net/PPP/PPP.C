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
/*    PPP.C                                                  2.0            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains the interface between the UART and the TCP/IP      */
/*    protocol stack. This involes services for the application to start    */
/*    and end a PPP session.                                                */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock                                                      */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  DV_DEVICE_ENTRY         *temp_dev_ptr                                   */
/*  NU_HISR                 PPP_HISR                                        */
/*  PPP_TEMP_BUFFER         _ppp_packet_size[PPP_MAX_HOLDING_PACKETS]       */
/*  uint8                   _ppp_packet_size_read                           */
/*  uint8                   _ppp_packet_size_write                          */
/*  uchar                   _ppp_login_pw[PPP_MAX_PW_LENGTH]                */
/*  uchar                   _ppp_login_name[PPP_MAX_ID_LENGTH]              */
/*  uint16                  _silent_discards                                */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*  PPP_Initialize                                                          */
/*  PPP_RX_Packet                                                           */
/*  PPP_TX_Packet                                                           */
/*  PPP_HISR_Entry                                                          */
/*  PPP_Compute_TX_FCS                                                      */
/*  PPP_Compute_RX_FCS                                                      */
/*  PPP_Lower_Layer_Up                                                      */
/*  PPP_Set_Login                                                           */
/*  PPP_Hangup                                                              */
/*  PPP_Kill_All_Open_Sockets                                               */
/*  PPP_Safe_To_Hangup                                                      */
/*  PPP_Still_Connected                                                     */
/*  PPP_Two_To_Power                                                        */
/*  PPP_Output                                                              */
/*  PPP_Input                                                               */
/*  PPP_Dial                                                                */
/*  PPP_Wait_For_Client                                                     */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*  nucleus.h                                                               */
/*  externs.h                                                               */
/*  tcp_errs.h                                                              */
/*  target.h                                                                */
/*  data.h                                                                  */
/*  rtab.h                                                                  */
/*  rip2.h                                                                  */
/*  ppp.h                                                                   */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        11/18/97      Removed standard library headers  */
/*                                          from being included - spr 376   */
/*  Uriah T. Pollock        11/18/97      Fixed bugs in PPP_Xmit_Packet and */
/*                                          PPP_Receive_Packet - spr 376.   */
/*  Uriah T. Pollock        11/18/97      Recoded PPP_Two_To_Power - spr 376*/
/*  Uriah T. Pollock        11/18/97      Removed the & from the expiration */
/*                                          routine being past to NU_Create_*/
/*                                          Timer in PPP_Initialize- spr 376*/
/*  Uriah T. Pollock        11/18/97      Changed the state set at the start*/
/*                                          of PPP_Lower_Layer_Up - spr 376.*/
/*  Uriah T. Pollock        11/18/97      Removed assert calls from         */
/*                                          PPP_Compute_FCS - spr 376.      */
/*  Uriah T. Pollock        11/18/97      Changed parameters for creating   */
/*                                          the echo timer in PPP_Initialize*/
/*                                          - spr 375.                      */
/*  Uriah T. Pollock        11/18/97      Updated PPP to version 1.1        */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/

#include "nucleus.h"
#include "target.h"        /* Compiler typedefs for data type sizes */
#include "externs.h"
#include "tcp_errs.h"
#include "data.h"
#include "rtab.h"
#include "rip2.h"

#include "ppp.h"

/* debug */
#ifdef DEBUG_PKT_TRACE
char debug_rx_buf[PKT_TRACE_SIZE];
int  debug_rx_index = 0;
char debug_tx_buf[PKT_TRACE_SIZE];
int  debug_tx_index = 0;
#endif

/* Declarations for the interrupt service routines, tasks, and functions that
   are required when interrupts are used. */

DV_DEVICE_ENTRY         *temp_dev_ptr;      /* temp until a better solution
                                               can be found. */
NU_HISR                 PPP_HISR;
PPP_TEMP_BUFFER         _ppp_temp_packet[PPP_MAX_HOLDING_PACKETS];
uint8                   _ppp_temp_packet_read;
uint8                   _ppp_temp_packet_write;
char                    _ppp_login_pw[PPP_MAX_PW_LENGTH];
char                    _ppp_login_name[PPP_MAX_ID_LENGTH];

/* Import all external variables */
extern NU_TIMER                 LCP_Restart_Timer;
extern NU_TIMER                 LCP_Echo_Timer;
extern NU_TIMER                 NCP_Restart_Timer;
NU_TIMER                        Authentication_Timer;
extern struct lcp_opts_struct   lcp_my_options;
extern struct lcp_opts_struct   lcp_peer_options;
extern struct lcp_state_struct  lcp_state;
extern int8                     _lcp_num_transmissions;
extern int8                     _ncp_num_transmissions;
extern int8                     _num_authentication_timeouts;
extern struct lcp_state_struct  ncp_state;
extern uint8                    ncp_mode;
extern uint8                    _assigned_peer_ip_address[4];

/* This is used to keep a record of the number of silently discarded packets
   that we have. */
uint16  _silent_discards;

/* define the lookup table for the frame check sequence computation
   method, 16 bit version. */

   static uint16 fcstab[256] = {
      0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
      0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
      0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
      0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
      0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
      0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
      0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
      0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
      0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
      0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
      0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
      0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
      0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
      0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
      0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
      0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
      0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
      0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
      0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
      0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
      0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
      0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
      0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
      0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
      0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
      0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
      0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
      0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
      0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
      0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
      0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
      0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
   };


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Initialize                                                        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function intializes the media layer of the protocol stack.       */
/*    After exiting from this function PPP will be in a state to begin      */
/*    link negotiation.                                                     */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEV_Init_Devices in file DEV.C                                        */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Allocate_Memory                                                    */
/*    NU_Tcp_Log_Error                                                      */
/*    NU_Create_HISR                                                        */
/*    MDM_Init                                                              */
/*    UART_Init_Port                                                        */
/*    NU_Create_Timer                                                       */
/*    MDM_Change_Communication_Mode                                         */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DV_DEVICE_ENTRY       *dev_ptr        Pointer to the device structure */
/*                                          for this device.                */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                                Returns NU_SUCCESS if           */
/*                                          initialization is successful,   */
/*                                          else a value less than 0 is     */
/*                                          returned.                       */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        11/18/97      Removed & from expiration routines*/
/*                                          on calls to NU_Create_Timer     */
/*                                          - spr 376.                      */
/*  Uriah T. Pollock        11/18/97      When creating the echo timer the  */
/*                                          parameter ID is now used to tell*/
/*                                          if the routine LCP_Echo_Expire  */
/*                                          is being called by the timer or */
/*                                          by some other routine - spr 375.*/
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS PPP_Initialize (DV_DEVICE_ENTRY *dev_ptr)
{
    STATUS      status;
    VOID        *pointer;

    /* Save the device pointer in a temp variable. This will only be used
       by PPP_Hangup and should be removed by the next release of PPP. */
    temp_dev_ptr = dev_ptr;

    /* Allocate a block of memory for the PPP HISR stack. */
    if (NU_Allocate_Memory (&System_Memory,
                            &pointer,
                            PPP_HISR_STACK_SIZE,
                            NU_NO_SUSPEND
                           ) != NU_SUCCESS)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL,
                          __FILE__, __LINE__);
        return(-1);
    }

    /* Create the HISR. */
    if (NU_Create_HISR (&PPP_HISR, "PPPHISR",
                        PPP_HISR_Entry,
                        0, pointer, PPP_HISR_STACK_SIZE) != NU_SUCCESS)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL,
                          __FILE__, __LINE__);
        return(-1);
    }

    /* init the globals */
    _ppp_temp_packet_read       = 0;
    _ppp_temp_packet_write      = 0;
    lcp_state.negotiation_pkt   = NU_NULL;
    ncp_state.negotiation_pkt   = NU_NULL;

    /* Fill in the device */
    dev_ptr->dev_output     = PPP_Output;
    dev_ptr->dev_input      = PPP_Input;
    dev_ptr->dev_start      = PPP_TX_Packet;
    dev_ptr->dev_receive    = PPP_RX_Packet;
    dev_ptr->dev_mtu        = PPP_MTU;
    dev_ptr->dev_hdrlen     = NET_PPP_HEADER_OFFSET_SIZE;

    /* Initialize the MODEM module. */
    if(MDM_Init() != NU_SUCCESS)
        return(-1);

    /* Initialize the UART. */
    if (UART_Init_Port(dev_ptr) != NU_SUCCESS)
        return (-1);

    /* Create the restart timer, it will be used for LCP */
    status = NU_Create_Timer (&LCP_Restart_Timer, "lcptimer", LCP_Timer_Expire,
                   (UNSIGNED) dev_ptr, LCP_TIMEOUT_VALUE, LCP_TIMEOUT_VALUE,
                   NU_DISABLE_TIMER);

    if (status != NU_SUCCESS)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL,
                          __FILE__, __LINE__);
        return(-1);
    }


    /* Create the restart timer, it will be used for NCP. */
    status = NU_Create_Timer (&NCP_Restart_Timer, "ncptimer", NCP_Timer_Expire,
                   (UNSIGNED) dev_ptr, LCP_TIMEOUT_VALUE, LCP_TIMEOUT_VALUE,
                   NU_DISABLE_TIMER);

    if (status != NU_SUCCESS)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL,
                          __FILE__, __LINE__);
        return(-1);
    }

    /* Create the restart timer, it will be used for authentication. */
    status = NU_Create_Timer (&Authentication_Timer, "authtimr", PAP_Timer_Expire,
                   (UNSIGNED) dev_ptr, LCP_TIMEOUT_VALUE, LCP_TIMEOUT_VALUE,
                   NU_DISABLE_TIMER);

    if (status != NU_SUCCESS)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL,
                          __FILE__, __LINE__);
        return(-1);
    }

    /* Create the restart timer, it will be used for authentication. */
    status = NU_Create_Timer (&LCP_Echo_Timer, "echotimr", LCP_Echo_Expire,
                   (UNSIGNED) dev_ptr , LCP_ECHO_VALUE, LCP_ECHO_VALUE,
                   NU_DISABLE_TIMER);

    if (status != NU_SUCCESS)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL,
                          __FILE__, __LINE__);
        return(-1);
    }

    /* Start in terminal mode */
    MDM_Change_Communication_Mode (MDM_TERMINAL_COMMUNICATION);

    return (NU_SUCCESS);

}  /* end NU_Etopen routine */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_RX_Packet                                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function is called from the LISR whenever a                      */
/*    character receive interrupt occurs.  It reads the received character  */
/*    from the UART and adds it to the buffer.  When a complete packet has  */
/*    been received the HISR will be activated to notify the upper layer    */
/*    software that a packet has arrived.                                   */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    the UART receive LISR                                                 */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    UART_Get_Char                                                         */
/*    NU_Activate_HISR                                                      */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DV_DEVICE_ENTRY       *device         Pointer to the device           */
/*                                          structure for the device that   */
/*                                          generated the RX interrupt.     */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                                NU_SUCCESS is always returned   */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        08/18/97      Removed uint8 cast on the call to */
/*                                          PPP_Two_To_Power - spr 376.     */
/*  Uriah T. Pollock        08/18/97      Changed input variable c from type*/
/*                                          char to uchar - spr 376.        */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS PPP_RX_Packet (DV_DEVICE_ENTRY *device)
{
    NET_BUFFER      *tmp_buf_ptr;
    uchar           c, prev_char;
    static INT      received        = 0;
    static INT      esc_received    = NU_FALSE;

//	Tracef("PPP_RX_Packet\n");
//	NU_Modem_Control_String("PPP_RX_Packet\n\r");	
    c = UART_Get_Char();

    /* Was the previous character received an escape character. */
    /* The character following the escape needs to been replaced with
       its XOR value. */
    if (esc_received)
    {
        switch(c)
        {
            case (PPP_HDLC_FRAME ^ PPP_HDLC_TRANSPARENCY):

                if (received < PPP_MAX_RX_SIZE)
                {
                    _ppp_temp_packet[_ppp_temp_packet_write].buffer[received++]
                                    = PPP_HDLC_FRAME;

#ifdef DEBUG_PKT_TRACE
                    if (debug_rx_index == PKT_TRACE_SIZE)
                        debug_rx_index = 0;

                    debug_rx_buf[debug_rx_index++] = PPP_HDLC_FRAME;
#endif
                }

                break;

            case (PPP_HDLC_CONTROL_ESCAPE ^ PPP_HDLC_TRANSPARENCY):

                if (received < PPP_MAX_RX_SIZE)
                {
                    _ppp_temp_packet[_ppp_temp_packet_write].buffer[received++]
                                    = PPP_HDLC_CONTROL_ESCAPE;

#ifdef DEBUG_PKT_TRACE
                    if (debug_rx_index == PKT_TRACE_SIZE)
                        debug_rx_index = 0;

                    debug_rx_buf[debug_rx_index++] = PPP_HDLC_CONTROL_ESCAPE;
#endif
                }

                break;

            default:

                if (received < PPP_MAX_RX_SIZE)
                {
                    _ppp_temp_packet[_ppp_temp_packet_write].buffer[received++]
                                    = (c ^ PPP_HDLC_TRANSPARENCY);

#ifdef DEBUG_PKT_TRACE
                    if (debug_rx_index == PKT_TRACE_SIZE)
                        debug_rx_index = 0;

                    debug_rx_buf[debug_rx_index++] = (c ^ PPP_HDLC_TRANSPARENCY);
#endif
                }

                break;
        }

        /* Reset the escape received flag. */
        esc_received = NU_FALSE;
    }

    /* This is either just another character or its the end of the
       frame. */

    else
    {
        switch (c)
        {
            /* If it's an END character, then we're done with the packet. */

            case PPP_HDLC_FRAME:

                /* A minor optimization:  If there is no data in the packet,
                   ignore it.  This is meant to avoid bothering IP with all the
                   empty packets generated by the duplicate END characters which
                   are in Serial Line IP.
                */

                if(received)
                {
                    /* Store the length of this packet and a pointer to it for
                       the HISR */

                    /*  Store the device for this interrupt. */
                    _ppp_temp_packet[_ppp_temp_packet_write].device = device;

                    /* Update the length of current buffer. */
                    _ppp_temp_packet[_ppp_temp_packet_write].size = received;

                    /* Bump the write pointer. */
                    _ppp_temp_packet_write++;

                    /* Make sure the index wraps around if needed. */
                    if (_ppp_temp_packet_write >= PPP_MAX_HOLDING_PACKETS)
                        _ppp_temp_packet_write = 0;

                    /* Reset the number of bytes received. */
                    received = 0;

                    /* Activate the HISR.  The HISR will inform the upper layer
                       protocols that a packet has arrived. */
                    NU_Activate_HISR (&PPP_HISR);
                }

                break;

            case PPP_HDLC_CONTROL_ESCAPE:

                /* Set the escape received flag. */
                esc_received = NU_TRUE;

                break;

            default:

                /* Store the char if we are under our MTU and if the char
                   is not one that is supposed to be control escaped. */
                if (c < PPP_MAX_ACCM)
                {
                    if ((received < PPP_MAX_RX_SIZE) && (!(PPP_Two_To_Power (c)
                           & lcp_my_options.accm)))
                    {
#ifdef DEBUG_PKT_TRACE
                        if (debug_rx_index == PKT_TRACE_SIZE)
                            debug_rx_index = 0;

                        debug_rx_buf [debug_rx_index++] = c;
#endif

                        _ppp_temp_packet[_ppp_temp_packet_write].buffer[received++] = c;

                    }
                }
                else
                {
                    _ppp_temp_packet[_ppp_temp_packet_write].buffer[received++] = c;
                }

                break;
        }

    }

    return (NU_SUCCESS);

}  /* end PPP_Receive */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_TX_Packet                                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function will send the PPP packet. This includes adding the      */
/* correct PPP header and computing the Frame Check Sequence over the packet*/
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_Output                                                            */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    UART_Put_Char                                                         */
/*    PPP_Compute_TX_FCS                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DEV_DEVICE_ENTRY  *device             Pointer to device structure for */
/*                                          the device to send the packet   */
/*                                          over.                           */
/*    NET_BUFFER        *buf_ptr            Pointer to the buffer that      */
/*                                          holds the packet to be sent.    */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                                NU_SUCCESS is always returned   */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        08/18/97      Removed uint8 cast on the call to */
/*                                          PPP_Two_To_Power - spr 376.     */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS PPP_TX_Packet (DV_DEVICE_ENTRY *device, NET_BUFFER *buf_ptr)
{
    NET_BUFFER          *tmp_buf_ptr = buf_ptr;
    uint16              frame_check, index = 0, pkt_type;
    uchar               *byte_ptr;
    uchar       HUGE    *ppp_header_ptr;
    int16               length;

#ifndef SWAPPING
    uint16              temp_num;
    uint8               *new_num, *org;
#endif

    uint8               more_to_send = NU_TRUE;

    /* Get the type of packet we are encapsulating. */
    if (buf_ptr->mem_flags & NET_IP)
        pkt_type = PPP_IP_PROTOCOL;
    else
        if (buf_ptr->mem_flags & NET_LCP)
            pkt_type = PPP_LINK_CONTROL_PROTOCOL;
        else
            if (buf_ptr->mem_flags & NET_IPCP)
                pkt_type = PPP_IP_CONTROL_PROTOCOL;
            else
                if (buf_ptr->mem_flags & NET_PAP)
                    pkt_type = PPP_PAP_PROTOCOL;
                else
                    if (buf_ptr->mem_flags & NET_CHAP)
                        pkt_type = PPP_CHAP_PROTOCOL;

    /* If we are in the OPENED state and this is not a PPP negotiation pkt
       then maybe we can compress some fields. */
    if ((lcp_state.state == OPENED) && (ncp_state.state == OPENED)
                && (pkt_type == PPP_IP_PROTOCOL))
    {

        /* We have three choices for compression: both the address and
           protocol, and one or the other. */

        /* See if we will do both */
        if ((lcp_peer_options.address_field_compression)
                               && (lcp_peer_options.protocol_field_compression))
        {
            /* Make room for the PPP header and adjust the lengths */
            buf_ptr->data_ptr           -= PPP_COMPRESS_HEADER;
            buf_ptr->data_len           += PPP_COMPRESS_HEADER;
            buf_ptr->mem_total_data_len += PPP_COMPRESS_HEADER;

            ppp_header_ptr = buf_ptr->data_ptr;

            /* Don't add the address and control fields */

            /* Add the compressed packet type */
#ifdef SWAPPING
            byte_ptr                = (uchar*)&pkt_type;
            ppp_header_ptr[index++] = byte_ptr[0];
#else
            byte_ptr                = (uchar*)&pkt_type;
            ppp_header_ptr[index++] = byte_ptr[1];
#endif
        }
        else
        {
            /* Ok. Maybe we will compress one or the other */

            /* See if we want to compress the address fields */
            if (lcp_peer_options.address_field_compression)
            {
                /* Make room for the PPP header and adjust the lengths */
                buf_ptr->data_ptr           -= PPP_COMPRESS_ADDRESS_HEADER;
                buf_ptr->data_len           += PPP_COMPRESS_ADDRESS_HEADER;
                buf_ptr->mem_total_data_len += PPP_COMPRESS_ADDRESS_HEADER;

                ppp_header_ptr = buf_ptr->data_ptr;

                /* Add the uncompressed protocol */
#ifdef SWAPPING
                byte_ptr                = (uchar*)&pkt_type;
                ppp_header_ptr[index++] = byte_ptr[1];
                ppp_header_ptr[index++] = byte_ptr[0];
#else
                byte_ptr                = (uchar*)&pkt_type;
                ppp_header_ptr[index++] = byte_ptr[0];
                ppp_header_ptr[index++] = byte_ptr[1];
#endif

            }
            else
            /* Now see if we want to compress the protocol field */
            if (lcp_peer_options.protocol_field_compression)
            {
                /* Make room for the PPP header and adjust the lengths */
                buf_ptr->data_ptr           -= PPP_COMPRESS_PROTOCOL_HEADER;
                buf_ptr->data_len           += PPP_COMPRESS_PROTOCOL_HEADER;
                buf_ptr->mem_total_data_len += PPP_COMPRESS_PROTOCOL_HEADER;

                ppp_header_ptr = buf_ptr->data_ptr;

                /* Add the PPP header info. */
                ppp_header_ptr[index++] = PPP_HDLC_ADDRESS;
                ppp_header_ptr[index++] = PPP_HDLC_CONTROL;

                /* Add the compress packet type */
#ifdef SWAPPING
                byte_ptr                = (uchar*)&pkt_type;
                ppp_header_ptr[index++] = byte_ptr[0];
#else
                byte_ptr                = (uchar*)&pkt_type;
                ppp_header_ptr[index++] = byte_ptr[1];
#endif
            }
            else
            {

                /* Make room for the PPP header and adjust the lengths */
                buf_ptr->data_ptr           -= sizeof (PPP_HEADER);
                buf_ptr->data_len           += sizeof (PPP_HEADER);
                buf_ptr->mem_total_data_len += sizeof (PPP_HEADER);

                ppp_header_ptr = buf_ptr->data_ptr;

                /* Add the PPP header info. */
                ppp_header_ptr[index++] = PPP_HDLC_ADDRESS;
                ppp_header_ptr[index++] = PPP_HDLC_CONTROL;

                /* Add the packet type in MSB LSB order. */
#ifdef SWAPPING
                byte_ptr                = (uchar*)&pkt_type;
                ppp_header_ptr[index++] = byte_ptr[1];
                ppp_header_ptr[index++] = byte_ptr[0];
#else
                byte_ptr                = (uchar*)&pkt_type;
                ppp_header_ptr[index++] = byte_ptr[0];
                ppp_header_ptr[index++] = byte_ptr[1];
#endif
            }
        }
    }
    else
    {
        /* Since we are not in the opened state we must be sending
           configure pkts, none of these are compressed. */

        /* Make room for the PPP header and adjust the lengths */
        buf_ptr->data_ptr           -= sizeof (PPP_HEADER);
        buf_ptr->data_len           += sizeof (PPP_HEADER);
        buf_ptr->mem_total_data_len += sizeof (PPP_HEADER);

        ppp_header_ptr = buf_ptr->data_ptr;

        /* Add the PPP header info. */
        ppp_header_ptr[index++] = PPP_HDLC_ADDRESS;
        ppp_header_ptr[index++] = PPP_HDLC_CONTROL;

        /* Add the packet type in MSB LSB order. */
#ifdef SWAPPING
        byte_ptr = (uchar*)&pkt_type;
        ppp_header_ptr[index++] = byte_ptr[1];
        ppp_header_ptr[index++] = byte_ptr[0];
#else
        byte_ptr = (uchar*)&pkt_type;
        ppp_header_ptr[index++] = byte_ptr[0];
        ppp_header_ptr[index++] = byte_ptr[1];
#endif
    }

    /* Now compute the FCS. */
    frame_check = PPP_Compute_TX_FCS (PPP_INIT_FCS16, buf_ptr);
    frame_check ^= 0xffff;                  /* get the complement */

    /* Set the index to zero for use during the sending of the packet. */
    index = 0;

    /* Send a FLAG character to flush out any characters the remote host might
       have received because of line noise and to mark the begining of this
       PPP HDLC frame. */

    UART_Put_Char (PPP_HDLC_FRAME);

    /* Loop through all the buffers in the chain and send out each byte. */
    while (more_to_send)
    {
        /* Get the length so that we can loop through the packet and send out
           each byte. */
        length = (uint16) tmp_buf_ptr->data_len;

        /* Get a pointer to the data. */
        ppp_header_ptr = tmp_buf_ptr->data_ptr;

        /* Now loop through the encapsulated packet and send it out */
        while(length-- > 0)
        {
            switch((char)*ppp_header_ptr)
            {
                /* If it's the same code as a FRAME character, then send the special
                   two character code so that the receiver does not think we sent
                   a FRAME.
                */
                case PPP_HDLC_FRAME:
#ifdef DEBUG_PKT_TRACE
                    if (debug_tx_index == PKT_TRACE_SIZE)
                        debug_tx_index = 0;

                    debug_tx_buf [debug_tx_index++] = (PPP_HDLC_FRAME);

#endif

                    UART_Put_Char(PPP_HDLC_CONTROL_ESCAPE);
                    UART_Put_Char(PPP_HDLC_FRAME ^ PPP_HDLC_TRANSPARENCY);
                    break;

                /* If it's the same code as an ESC character, then send the special
                   two character code so that the receiver does not think we sent
                   an ESC.
                */
            case PPP_HDLC_CONTROL_ESCAPE:
#ifdef DEBUG_PKT_TRACE
                    if (debug_tx_index == PKT_TRACE_SIZE)
                        debug_tx_index = 0;

                    debug_tx_buf [debug_tx_index++] = PPP_HDLC_CONTROL_ESCAPE;

#endif

                    UART_Put_Char(PPP_HDLC_CONTROL_ESCAPE);
                    UART_Put_Char(PPP_HDLC_CONTROL_ESCAPE ^ PPP_HDLC_TRANSPARENCY);
                    break;

                /* Otherwise it is just a regular character. We will either send
                   it or encode it depending on the ACCM. */
                default:

#ifdef DEBUG_PKT_TRACE
                        if (debug_tx_index == PKT_TRACE_SIZE)
                            debug_tx_index = 0;

                        debug_tx_buf [debug_tx_index++] = *ppp_header_ptr;
#endif

                    /* Since the ACCM is only for the first 32 chars. see if
                       we even need to check it for this one. */
                    if (*ppp_header_ptr >= PPP_MAX_ACCM)
                    {
                        UART_Put_Char(*ppp_header_ptr);       /* guess not so send it */
                    }
                    else

                        /* else check the character against the map */

                        if (PPP_Two_To_Power (*ppp_header_ptr) & lcp_peer_options.accm)
                        {

                            /* It is in the map so send the two char code. */
                            UART_Put_Char(PPP_HDLC_CONTROL_ESCAPE);
                            UART_Put_Char((CHAR) (*ppp_header_ptr ^ PPP_HDLC_TRANSPARENCY));
                        }
                        else
                        {
                            /* It is not in the map so send it in the clear. */
                            UART_Put_Char(*ppp_header_ptr);
                        }
            }

            /* Move to the next byte. */
            ppp_header_ptr++;
        }

        /* Move the buffer pointer to the next buffer. */
        tmp_buf_ptr = tmp_buf_ptr->next_buffer;

        /* If there are no more buffers then there is no more to send. */
        if (!tmp_buf_ptr)
        {
            more_to_send = NU_FALSE;
        }

    }


    /* Now set the length to two for the two bytes of the FCS that are appended
       to the end of each packet. */
    length = 2;

#ifndef SWAPPING
    /* Swapp the FCS if needed, remember that the FCS is put on LSB first. */
    org     = (uint8 *)&frame_check;
    new_num = (uint8 *)&temp_num;

    /* swap the bytes in the int16 */
    new_num[0] = org[1];
    new_num[1] = org[0];

    frame_check = temp_num;
#endif

    /* Get a pointer to the FCS. */
    ppp_header_ptr = (uchar *) &frame_check;

    /* Put the FCS on the end of the packet. */
    while(length--)
    {
        switch((char)*ppp_header_ptr)
        {
            /* If it's the same code as a FRAME character, then send the special
               two character code so that the receiver does not think we sent
               a FRAME.
            */
            case PPP_HDLC_FRAME:
#ifdef DEBUG_PKT_TRACE
                if (debug_tx_index == PKT_TRACE_SIZE)
                    debug_tx_index = 0;

                debug_tx_buf [debug_tx_index++] = (PPP_HDLC_FRAME);

#endif

                UART_Put_Char(PPP_HDLC_CONTROL_ESCAPE);
                UART_Put_Char(PPP_HDLC_FRAME ^ PPP_HDLC_TRANSPARENCY);
                break;

            /* If it's the same code as an ESC character, then send the special
               two character code so that the receiver does not think we sent
               an ESC.
            */
            case PPP_HDLC_CONTROL_ESCAPE:
#ifdef DEBUG_PKT_TRACE
                if (debug_tx_index == PKT_TRACE_SIZE)
                    debug_tx_index = 0;

                debug_tx_buf [debug_tx_index++] = PPP_HDLC_CONTROL_ESCAPE;

#endif

                UART_Put_Char(PPP_HDLC_CONTROL_ESCAPE);
                UART_Put_Char(PPP_HDLC_CONTROL_ESCAPE ^ PPP_HDLC_TRANSPARENCY);
                break;

            /* Otherwise it is just a regular character. We will either send
               it or encode it depending on the ACCM. */
            default:

#ifdef DEBUG_PKT_TRACE
                    if (debug_tx_index == PKT_TRACE_SIZE)
                        debug_tx_index = 0;

                    debug_tx_buf [debug_tx_index++] = *ppp_header_ptr;
#endif

                /* Since the ACCM is only for the first 32 chars. see if
                   we even need to check it for this one. */
                if (*ppp_header_ptr >= PPP_MAX_ACCM)
                {
                    UART_Put_Char(*ppp_header_ptr);       /* guess not so send it */
                }
                else

                    /* else check the character against the map */

                    if (PPP_Two_To_Power (*ppp_header_ptr) & lcp_peer_options.accm)
                    {

                        /* It is in the map so send the two char code. */
                        UART_Put_Char(PPP_HDLC_CONTROL_ESCAPE);
                        UART_Put_Char((CHAR) (*ppp_header_ptr ^ PPP_HDLC_TRANSPARENCY));
                    }
                    else
                    {
                        /* It is not in the map so send it in the clear. */
                        UART_Put_Char(*ppp_header_ptr);
                    }
        }

        ppp_header_ptr++;
    }

    /* Terminate the packet. */
    UART_Put_Char(PPP_HDLC_FRAME);

    return (NU_SUCCESS);

}  /* end PPP_Xmit routine */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   PPP_HISR_Entry                                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This is the entry function for the PPP HISR.  The HISR will check the */
/*    FCS of the incoming packet. If it passes, an event will be set to     */
/*    notify the upper layer that a packet needs to be processed.           */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Activated from PPP_RX_Packet.                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    PPP_Compute_RX_FCS                                                    */
/*    NU_Set_Events                                                         */
/*    MEM_Buffer_Chain_Dequeue                                              */
/*    memcpy                                                                */
/*    MEM_Buffer_Enqueue                                                    */
/*    NU_Tcp_Log_Error                                                      */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
VOID PPP_HISR_Entry (VOID)
{
    NET_BUFFER  *buf_ptr, *work_buf;
    int32       bytes_left;
    uint8       *buffer, start;

    /* Check the FCS to make sure this is a valid packet before we
      bother the upper layers with it. */
    if (PPP_Compute_RX_FCS (PPP_INIT_FCS16,
        &_ppp_temp_packet[_ppp_temp_packet_read]) != PPP_GOOD_FCS16)
    {
#ifdef DEBUG_PRINT
        _PRINT ("\ndiscard - invalid FCS ");
#endif
        _silent_discards++;

        _ppp_temp_packet_read++;
    }
    else
    {
        /* Make sure that the packet starts on a word boundary.
           This is done by backing up where the start of the PPP 
           packet is copied. If full compression was used then the PPP 
           header is 1 byte and the packet will start 1 byte before a word
           boundary. And so on.
        */

        /* Was full compression used. If so the first byte will 0x21 */
        if (_ppp_temp_packet[_ppp_temp_packet_read].buffer[0] == 0x21)
        {
            start = PPP_WITH_BOTH_COMPRESS_START;
        }

        /* Was just address control field compression used. */
        else if ((_ppp_temp_packet[_ppp_temp_packet_read].buffer[0] == 0x00)
                && (_ppp_temp_packet[_ppp_temp_packet_read].buffer[1] == 0x21))
        {
            start = PPP_WITH_ADDR_COMPRESS_START;
        }

        /* Was just protocol compression used */
        else if ((_ppp_temp_packet[_ppp_temp_packet_read].buffer[0] == 0xFF)
                && (_ppp_temp_packet[_ppp_temp_packet_read].buffer[1] == 0x03)
                && (_ppp_temp_packet[_ppp_temp_packet_read].buffer[2] == 0x21))
        {
            start = PPP_WITH_PROT_COMPRESS_START;
        }
        else
            /* No compression was used. In this case just copy the whole
               thing, ie. start at the beginning. */
            start = sizeof (PPP_HEADER);

        /* Store the total size, removing the two bytes of the FCS. */
        bytes_left = (_ppp_temp_packet[_ppp_temp_packet_read].size - PPP_FCS_SIZE);

        /* Allocate a buffer chain to copy this packet into. */
        buf_ptr = MEM_Buffer_Chain_Dequeue (&MEM_Buffer_Freelist,
                (bytes_left + (NET_PPP_HEADER_OFFSET_SIZE - start)));

        /* Make sure we got some buffers. */
        if (buf_ptr == NU_NULL)
        {
            NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_SEVERE,
                          __FILE__, __LINE__);

            /* Bump the number of packets discarded. */
            _silent_discards++;

            /* Move to the next packet. */
            _ppp_temp_packet_read++;
        }
        else
        {

            /* Set the total length in the parent buffer. */
            buf_ptr->mem_total_data_len = bytes_left;

            /* Set the data pointer. */
            buf_ptr->data_ptr = ((buf_ptr->mem_parent_packet +
                    NET_PPP_HEADER_OFFSET_SIZE) - start);

            /* Get a pointer to the data. */
            buffer = _ppp_temp_packet[_ppp_temp_packet_read].buffer;

            /* Now break this packet into a buffer chain. */

            /* Will it all fit into one buffer? */
            if (bytes_left <= (NET_PARENT_BUFFER_SIZE -
                                (NET_PPP_HEADER_OFFSET_SIZE - start)))
            {
                /* Store the number of bytes held by this buffer, this includes the
                   protocol headers. */
                buf_ptr->data_len = buf_ptr->mem_total_data_len;

                /* Copy the data. */
                memcpy  (buf_ptr->data_ptr, buffer, bytes_left);
            }
            else
            {

                /* Fill the parent buffer in the chain. This one is slightly smaller than
                   the rest in the chain. */
                memcpy (buf_ptr->data_ptr, buffer, NET_PARENT_BUFFER_SIZE -
                    (NET_PPP_HEADER_OFFSET_SIZE - start));

                /* Take off the bytes just copied from the total bytes left. */
                bytes_left -= (NET_PARENT_BUFFER_SIZE - (NET_PPP_HEADER_OFFSET_SIZE - start));

                /* Store the number of bytes in this buffer. */
                buf_ptr->data_len = (NET_PARENT_BUFFER_SIZE -
                    (NET_PPP_HEADER_OFFSET_SIZE - start));

                /* Bump it the number of bytes just copied. */
                buffer += (NET_PARENT_BUFFER_SIZE -
                            (NET_PPP_HEADER_OFFSET_SIZE - start));

                /* Get a work buffer pointer to the buffer chain */
                work_buf = buf_ptr;

                /* Break the rest up into the multiple buffers in the chain. */
                do
                {
                    /* Move to the next buffer in the chain */
                    work_buf = work_buf->next_buffer;

                    /* If the bytes left will fit into one buffer then copy them over */
                    if (bytes_left <= NET_MAX_BUFFER_SIZE)
                    {
                        /* Copy the rest of the data. */
                        memcpy (work_buf->mem_packet, buffer, bytes_left);

                        /* Set the data ptr */
                        work_buf->data_ptr = work_buf->mem_packet;

                        /* Store the number of bytes in this buffer. */
                        work_buf->data_len = bytes_left;
                    }
                    else
                    {
                        /* Copy all that will fit into a single buffer */
                        memcpy (work_buf->mem_packet, buffer, NET_MAX_BUFFER_SIZE);

                        /* Update the buffer pointer */
                        buffer += NET_MAX_BUFFER_SIZE;

                        /* Set the data ptr */
                        work_buf->data_ptr = work_buf->mem_packet;

                        /* Store the number of bytes in this buffer. */
                        work_buf->data_len = NET_MAX_BUFFER_SIZE;
                    }

                    /* Update the data bytes left to copy. */
                    bytes_left -= NET_MAX_BUFFER_SIZE;

                } while (bytes_left > 0);

            } /* end if it will fit into one buffer */

            /* Set the device that this packet was RX on and bump the read
               pointer. */
            buf_ptr->mem_buf_device = _ppp_temp_packet[_ppp_temp_packet_read++].device;

            /* Move the packet onto the buffer list, where the upper
               layer protocols can find it. */
            MEM_Buffer_Enqueue (&MEM_Buffer_List, buf_ptr);

            /* Let the upper layer know a good packet is here. */
            NU_Set_Events(&Buffers_Available, (UNSIGNED)2, NU_OR);
        } /* end if we got a buffer */
    }

    /* Make sure the ring buffer loops. */
    if (_ppp_temp_packet_read >= PPP_MAX_HOLDING_PACKETS)
        _ppp_temp_packet_read = 0;

}  /* end PPP_HISR routine */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Compute_TX_FCS                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function will compute the frame check sequence for the frame     */
/*  contained at the address pointed to by frame_ptr. The computation of    */
/*  the FCS spans over multiple buffers in the chain containing the packet. */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_TX_Packet                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uint16        fcs                     The initial FCS to start with   */
/*    NET_BUFFER    *buf_ptr                Pointer to the buffer that      */
/*                                          holds the packet to be sent.    */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    uint16                                The computed FCS                */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        11/18/97      Removed the calls to assert. They */
/*                                           are no longer needed - spr 376.*/
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
uint16 PPP_Compute_TX_FCS (uint16 fcs, NET_BUFFER *buf_ptr)
{
    NET_BUFFER          *tmp_buf_ptr = buf_ptr;
    int32               len, bytes_done;
    uchar       HUGE    *frame_ptr;

    bytes_done = 0;

    /* Get the length of the buffer and a pointer to the data. */
    len         = tmp_buf_ptr->data_len;
    frame_ptr   = tmp_buf_ptr->data_ptr;
    bytes_done += len;

    while (tmp_buf_ptr)
    {
        /* Compute the FCS over the bytes in this buffer. */
        while (len-- > 0)
            fcs = (fcs >> 8) ^ fcstab[(fcs ^ *frame_ptr++) & 0xff];

        tmp_buf_ptr = tmp_buf_ptr->next_buffer;

        if (tmp_buf_ptr)
        {
            /* Get the length of the buffer and a pointer to the data. */
            len         = tmp_buf_ptr->data_len;
            frame_ptr   = tmp_buf_ptr->data_ptr;
            bytes_done += len;
        }

    }

    return (fcs);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Compute_RX_FCS                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function will compute the frame check sequence for the frame     */
/*  contained at the address pointed to by frame_ptr. The computaion does   */
/*  not span buffers. It is done over a contiguous area of memory only.     */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_HISR_Entry                                                        */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uint16            fcs                 The initial FCS to start with   */
/*    PPP_TEMP_BUFFER   *buf_ptr            Pointer to the packet to        */
/*                                          compute the FCS over.           */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    uint16                                The computed FCS                */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        11/18/97      Removed the calls to assert. They */
/*                                           are no longer needed - spr 376.*/
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
uint16 PPP_Compute_RX_FCS (uint16 fcs, PPP_TEMP_BUFFER *buf_ptr)
{
    uint32 len;
    uchar  HUGE *frame_ptr;

    /* Get the length of the buffer and a pointer to the data. */
    len         = buf_ptr->size;
    frame_ptr   = buf_ptr->buffer;

    /* Compute the FCS. */
    while (len--)
        fcs = (fcs >> 8) ^ fcstab[(fcs ^ *frame_ptr++) & 0xff];

    return (fcs);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Lower_Layer_Up                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function is responsible for controlling the movement of PPP      */
/* through each phase of the link negotiation.                              */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_Dial                                                              */
/*    PPP_Wait_For_Client                                                   */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Retrieve_Events                                                    */
/*    NU_Control_Timer                                                      */
/*    LCP_Send_Config_Req                                                   */
/*    NU_Reset_Timer                                                        */
/*    PAP_Send_Authentication                                               */
/*    CHAP_Send_Challenge                                                   */
/*    NCP_IP_Send_Config_Req                                                */
/*    MEM_Buffer_Enqueue                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                                How the link negotiation ended  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        11/18/97      Changed the state set at the      */
/*                                          beginning of this routine-spr376*/
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS PPP_Lower_Layer_Up(uint8 *ip_address)
{
    STATUS   status;
    UNSIGNED bufs_ava;

    /* init the globals */
    _silent_discards        = 0;

    /* Set the current state of the LCP and NCP. */
    lcp_state.state = REQ_SENT;
    ncp_state.state = REQ_SENT;

    /* Initalize the pkt negotiation holder. A 0 indicates we are not
       waiting on a reply. */
    lcp_state.identifier = 0;
    ncp_state.identifier = 0;

    /* Reset the MRU for this device is case it was changed during the
       last PPP session. */
    temp_dev_ptr->dev_mtu = PPP_MTU;

    /* Fill in the default TX and RX LCP options into the structure that
       will hold them. */
    lcp_my_options.max_rx_unit                  = PPP_MTU;
    lcp_my_options.accm                         = LCP_LOCAL_DEFAULT_ACCM;
    lcp_my_options.authentication_protocol      = LCP_DEFAULT_AUTH_PROTOCOL;
    lcp_my_options.magic_number                 = LCP_DEFAULT_MAGIC_NUMBER;
    lcp_my_options.protocol_field_compression   = LCP_DEFAULT_PROTOCOL_COMPRESS;
    lcp_my_options.address_field_compression    = LCP_DEFAULT_ADDRESS_COMPRESS;
    lcp_my_options.use_accm                     = LCP_USE_ACCM;
    lcp_my_options.use_max_rx_unit              = LCP_USE_MRU;

    /* We don't fill them all in here because our peer will let us know
       which ones it can handle. */
    lcp_peer_options.max_rx_unit                = PPP_MTU;
    lcp_peer_options.accm                       = LCP_FOREIGN_DEFAULT_ACCM;

    /* Reset the number of configure/terminate transmissions. */
    _lcp_num_transmissions = LCP_MAX_CONFIGURE;

    /* Eat any events that may be around from the last session. */
    status = NU_Retrieve_Events(&Buffers_Available, (LCP_CONFIG_DONE |
                            NCP_CONFIG_DONE | AUTHENTICATION_DONE |
                            LCP_CONFIG_FAIL),
                            NU_OR_CONSUME, &bufs_ava, NU_NO_SUSPEND);

    /* Reset the restart timer */
    NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

    /* Start the LCP state automaton. */
    LCP_Send_Config_Req();

    /* Start the timer */
    NU_Reset_Timer (&LCP_Restart_Timer, LCP_Timer_Expire,
                   LCP_TIMEOUT_VALUE, LCP_TIMEOUT_VALUE, NU_ENABLE_TIMER);
#ifdef DEBUG_PRINT
            _PRINT ("Waiting for LCP to be done..\n");
#endif
    /* We wait for the event to tell us that LCP is done. */
    status = NU_Retrieve_Events(&Buffers_Available, LCP_CONFIG_DONE,
                            NU_OR_CONSUME, &bufs_ava, NU_SUSPEND);
#ifdef DEBUG_PRINT
            _PRINT ("LCP done!\n");
#endif
    /* Reset the restart timer */
    NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

    /* Check to see if it completed succcessfully. */
    if (!(!(bufs_ava & LCP_CONFIG_SUCCESS) || (status != NU_SUCCESS)))
    {
        /* LCP is done so start authentication if it is used */

        if (lcp_my_options.authentication_protocol)
        {
#ifdef DEBUG_PRINT
            _PRINT ("\ndone with LCP, starting authentication. \n");
#endif

            /* If we are are using PAP then we must start the timer and
               initiate the authentication */
            if (lcp_my_options.authentication_protocol == PPP_PAP_PROTOCOL)
            {
#ifdef DEBUG_PRINT
                _PRINT ("using PAP ");
#endif
                /* We will only send a PAP packet if we are a CLIENT */
                if (ncp_mode == CLIENT)
                {
                    /* Send our ID and PW */
                    PAP_Send_Authentication();
                }
                else
                {
                    /* Don't need to do anything, the client will send
                       us an authentication pkt. */
                }

                /* Start the timer */
                NU_Reset_Timer (&Authentication_Timer, PAP_Timer_Expire,
                       LCP_TIMEOUT_VALUE, LCP_TIMEOUT_VALUE, NU_ENABLE_TIMER);
            }
            else
            {
#ifdef DEBUG_PRINT
                _PRINT ("using CHAP-MD5 ");
#endif
                /* If we in SERVER mode then we need to challenge the
                   peer for a PW. */
                if (ncp_mode == SERVER)
                {
                    /* Send a challenge pkt. */
                    CHAP_Send_Challenge();
                }
                else
                {
                    /* Don't need to do anything, the server will challenge
                       us. */
                }

                /* Start the timer */
                NU_Reset_Timer (&Authentication_Timer, CHAP_Timer_Expire,
                       LCP_TIMEOUT_VALUE, LCP_TIMEOUT_VALUE, NU_ENABLE_TIMER);

            }

            _num_authentication_timeouts = LCP_MAX_AUTHENTICATE;

            /* We wait for the event to tell us that authentication is done. */
            status = NU_Retrieve_Events(&Buffers_Available, AUTHENTICATION_DONE,
                            NU_OR_CONSUME, &bufs_ava, NU_SUSPEND);

            /* Reset the restart timer */
            NU_Control_Timer (&Authentication_Timer, NU_DISABLE_TIMER);

            /* See if we were successful at logging in */
            if (!(!(bufs_ava & AUTHENTICATION_SUCCESS) || (status != NU_SUCCESS)))
                status = PPP_SUCCESS;
            else
                status = PPP_LOGIN_FAILED;

        } /* if we are using authentication */

        /* Make sure we logged in before try NCP */
        if (status == PPP_SUCCESS)
        {

#ifdef DEBUG_PRINT
            _PRINT ("\ndone with authentication, starting NCP\n");
#endif

            _ncp_num_transmissions = LCP_MAX_CONFIGURE;

            ncp_state.state = REQ_SENT;

            /* Send the NCP configure request pkt. */
            NCP_IP_Send_Config_Req(ip_address);

            /* Start the timer */
            NU_Reset_Timer (&NCP_Restart_Timer, NCP_Timer_Expire,
                   LCP_TIMEOUT_VALUE, LCP_TIMEOUT_VALUE, NU_ENABLE_TIMER);

            /* We wait for the event to tell us that NCP is done. */
            status = NU_Retrieve_Events(&Buffers_Available, NCP_CONFIG_DONE,
                            NU_OR_CONSUME, &bufs_ava, NU_SUSPEND);

            /* Stop the timer */
            NU_Control_Timer (&NCP_Restart_Timer, NU_DISABLE_TIMER);

            /* Check to see if it completed succcessfully. */
            if (!(!(bufs_ava & NCP_CONFIG_SUCCESS) || (status != NU_SUCCESS)))
                status = PPP_SUCCESS;
            else
                status = PPP_NCP_FAILED;
        } /* if login completed successfully */

    } /* if LCP completed successfully */
    else
        status = PPP_LCP_FAILED;

    /* Release the buffers used by the PPP negotiation. */
    if (lcp_state.negotiation_pkt != NU_NULL)
    {
        MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, lcp_state.negotiation_pkt);
        lcp_state.negotiation_pkt = NU_NULL;
    }

    if (ncp_state.negotiation_pkt != NU_NULL)
    {
        MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, ncp_state.negotiation_pkt);
        ncp_state.negotiation_pkt = NU_NULL;
    }

    return (status);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Set_Login                                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Sets the ID and password to be used when dialing up a PPP server.     */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Application                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    strcpy                                                                */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    char[]    : ID to be used                                             */
/*    char[]    : password to be used                                       */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
void PPP_Set_Login (char id[PPP_MAX_ID_LENGTH], char pw[PPP_MAX_PW_LENGTH])
{
    /* Simply copy the ID and PW pair into our internal variables. */
    strcpy (_ppp_login_name, id);
    strcpy (_ppp_login_pw, pw);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Hangup                                                            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Starts the link termination phase and hangs up the physical device.   */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Application                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Control_Timer                                                      */
/*    NU_Obtain_Semaphore                                                   */
/*    PPP_Kill_All_Open_Sockets                                             */
/*    NU_Release_Semaphore                                                  */
/*    PPP_Safe_To_Hangup                                                    */
/*    NU_Reset_Timer                                                        */
/*    LCP_Send_Terminate_Req                                                */
/*    MDM_Hangup                                                            */
/*    NU_Sleep                                                              */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uint8         mode                    Should the link be closed even  */
/*                                          if it is active.                */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                                Was the link closed or not      */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS PPP_Hangup (uint8 mode)
{
    STATUS   ret_status;

#ifdef DEBUG_PRINT
    _PRINT ("\nhangup ");
#endif

    ret_status = PPP_TRUE;

    /* If the caller wants to force it, then we will close the whole network
       without warning. */
    if (mode == PPP_FORCE)
    {
        /* Stop the echo timer since we are leaving the open state */
        NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

        /* Get the network resource. */
        NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);

        /* Close the network down. */
        PPP_Kill_All_Open_Sockets(temp_dev_ptr);

        /* Detach the IP address from this device. */
        DEV_Detach_IP_From_Device (temp_dev_ptr->dev_net_if_name);

        /* Give the resource back. */
        NU_Release_Semaphore(&TCP_Resource);

        /* Hangup the phone. */
        MDM_Hangup();
    }

    else

        /* Otherwise we will see if it is safe to hangup the phone. If any
           sockets are in the process of closing, this function will wait until
           they are closed. */
        ret_status = PPP_Safe_To_Hangup(temp_dev_ptr);

    /* If it is ok to hangup then we will start by terminating the link
       and we will hangup the modem. */
    if (ret_status == PPP_TRUE)
    {
        /* Stop the echo timer since we are leaving the open state */
        NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

        /* Everything depends on our state */
        switch (lcp_state.state)
        {
            case INITIAL    :
            case STARTING   :
            case CLOSED     :   /* Don't need to do anything for these states */
            case STOPPED    :
            case CLOSING    :
            case STOPPING   :   break;

            case OPENED     :

            case REQ_SENT   :   /* These are all the same */
            case ACK_RCVD   :
            case ACK_SENT   :

                /* Get the network resource. */
                NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);

                /* Reset the restart counter */
                _lcp_num_transmissions = LCP_MAX_TERMINATE;

                /* Reset the restart timer */
                NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Send a terminate request */
                LCP_Send_Terminate_Req();

                lcp_state.state = CLOSING;

                /* Start the timer */
                NU_Reset_Timer (&LCP_Restart_Timer, LCP_Timer_Expire,
                   LCP_TIMEOUT_VALUE, LCP_TIMEOUT_VALUE, NU_ENABLE_TIMER);

                /* Give the resource back. */
                NU_Release_Semaphore(&TCP_Resource);

                /* Wait for an ACK or for a timeout. */
                while ((lcp_state.state != INITIAL) &&
                                                (lcp_state.state != STARTING))
                    NU_Sleep(1);
        }
                
        /* Reset the restart timer */
        NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

        /* Detach the IP address from this device. */
        DEV_Detach_IP_From_Device (temp_dev_ptr->dev_net_if_name);

    } /* if */

    return (ret_status);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Kill_All_Open_Sockets                                             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Loops through the port lists closing all ports and waking up their      */
/* associated tasks.                                                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_Hangup                                                              */
/*  LCP_Open_State                                                          */
/*  LCP_Echo_Expire                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  NU_Resume_Task                                                          */
/*  NU_Deallocate_Memory                                                    */
/*  TCP_Cleanup                                                             */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  DV_DEVICE_ENTRY     *dev_ptr            Pointer to the device structure */
/*                                          for the device that is being    */
/*                                          closed.                         */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  none                                                                    */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
void PPP_Kill_All_Open_Sockets (DV_DEVICE_ENTRY *dev_ptr)
{
    uint16 index;
    struct port *prt;
    struct uport *uprt;

#ifdef DEBUG_PRINT
    _PRINT ("killing all sockets ");
#endif

    /* Init the index. */
    index = 0;

    /* Go through the UDP ports first and check for an active port */
    do
    {
        if (uportlist [index] != NU_NULL)
        {
            /* Get a pointer to the UDP port. */
            uprt = uportlist [index];

            /* Make sure it is using this interface before we kill it. */
            if (dev_ptr == uprt->up_route.rt_route->rt_device)
            {
                /* If any tasks are suspended on this socket, restart them. */
                if (uprt->RXTask != NU_NULL)
                {
                    NU_Resume_Task(uprt->RXTask);
                    uprt->RXTask = NU_NULL;
                }

                if (uprt->TXTask != NU_NULL)
                {
                    NU_Resume_Task(uprt->TXTask);
                    uprt->TXTask = NU_NULL;
                }

                /*  Clear this port list entry.  */
                NU_Deallocate_Memory((uint *) uportlist[index]);

                /*  Indicate that this port is no longer used. */
                uportlist[index] = NU_NULL;
            }
        }

    } while (++index < NUPORTS);


    /* Reset the index */
    index = 0;

    /* Look at the TCP ports. */
    do
    {
        /* See if there is an entry. */
        if (portlist [index] != NU_NULL)
        {

            /* Get a pointer to the TCP port. */
            prt = portlist[index];

            /* Make sure this entry is using this device. */
            if (dev_ptr == prt->tp_route.rt_route->rt_device)
            {

                /* See if the port is already in the closed state */
                if ((prt->state != SCLOSED) && (prt->state != STWAIT))
                {

                    /* The connection is  closed.  Cleanup. */
                    TCP_Cleanup(prt);

                    prt->state = SCLOSED;
                
                }
            }
        }

    } while (++index < NUPORTS);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Safe_To_Hangup                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Checks the port lists for active sockets on a particular device.      */
/* Returns TRUE if none are active and FALSE if one or more is active.      */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_Hangup                                                            */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Sleep                                                              */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DV_DEVICE_ENTRY       *dev_ptr        Pointer to the device to check  */
/*                                          for active ports on.            */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                                Was the link active or not      */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS PPP_Safe_To_Hangup (DV_DEVICE_ENTRY *dev_ptr)
{
    uint16 index;
    STATUS ret_status;

    /* Assume all if safe. */
    ret_status = NU_TRUE;

    /* Init the index. */
    index = 0;

    /* Go through the UDP ports first and check for an active port */
    do
    {
        if (uportlist [index] != NU_NULL)
        {
            /* Is this port using this device. */
            if (uportlist [index]->up_route.rt_route->rt_device == dev_ptr)
            {

                /* Set the return value, there is still an active port. */
                ret_status = NU_FALSE;
            }
        }

    } while (ret_status && (++index <= NUPORTS));

    /* Reset the index */
    index = 0;

    /* The UDP ports checked out ok, look at the TCP ports. */
    do
    {
        /* See if there is an entry. */
        if (portlist [index] != NU_NULL)
        {
            /* Is this port using this device. */
            if (portlist [index]->tp_route.rt_route->rt_device == dev_ptr)
            {

                /* See if the port is already in the closed state */
                if ((portlist [index]->state != SCLOSED) &&
                                    (portlist [index]->state != STWAIT))
                {
                    /* It may be in a closing state, if so we will wait for
                       TCP to finish closing. */
                    if ((portlist[index]->state == SFW1) ||
                            (portlist[index]->state == SFW2) ||
                            (portlist[index]->state == SCLOSING) ||
                            (portlist[index]->state == SCWAIT) ||
                            (portlist[index]->state == SLAST))
                    {

                        while (((portlist[index]->state == SFW1) ||
                                (portlist[index]->state == SFW2) ||
                                (portlist[index]->state == SCLOSING) ||
                                (portlist[index]->state == SCWAIT) ||
                                (portlist[index]->state == SLAST)) &&
                                ((portlist[index]->state != SCLOSED) &&
                                (portlist[index]->state != STWAIT)))
                            NU_Sleep(5);
                    }
                    else
                    {
                        /* Otherwise it is in an opening or established state.
                           Return false. */
                        ret_status = NU_FALSE;
                    }
                }
                else
                    index++;
            }
            else
                index++;
        }
        else
            index++;

    } while (ret_status && (index <= NUPORTS));

    return (ret_status);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Still_Connected                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Checks the state of the PPP link and returns TRUE if it is still      */
/* open. FALSE if it is not open.                                           */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Application                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                                Is the link open or not         */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS PPP_Still_Connected (void)
{
    if ((lcp_state.state == INITIAL) || (lcp_state.state == STARTING))
        return (NU_FALSE);
    else
        return (NU_TRUE);
}
/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Two_To_Power                                                      */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Raises two to the power of the passed in variable. This function is   */
/* used instead of POW so that math and floating point libraries do not     */
/* have to be linked in.                                                    */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_TX_Packet                                                         */
/*    PPP_RX_Packet                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uint8         exponent                The exponent to raise two too   */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    uint32                                The computed power              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        11/18/97      Recoded this function to be       */
/*                                          reentrant. The new solution is  */
/*                                          faster than the previous.       */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
uint32 PPP_Two_To_Power (uint8 exponent)
{
    uint32  answer;

#ifdef NU_NON_REENTRANT_CLIB
    INT     old_state;
#endif

    answer = PPP_ONE;

    /* Make sure the exponent is not zero */
    if (exponent != 0)
    {
        /* The bit shifting operation below has been seen to cause
           problems with non-reentrant C library's. To fix this
           turn interrupts off so that the PPP LISR does not
           interrupt the shift operation. */
#ifdef NU_NON_REENTRANT_CLIB
        old_state = NU_Control_Interrupts (NU_DISABLE_INTERRUPTS);
#endif

        answer <<= (uint32) exponent;

#ifdef NU_NON_REENTRANT_CLIB
        NU_Control_Interrupts (old_state);
#endif
    }

    return (answer);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Output                                                            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Makes sure that the device is active and calls the transmit routine   */
/* for the device. Then deallocates the buffer to the appropriate list.     */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  IP_Send                                                                 */
/*  IP_Fragment                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    device->dev_start                     Function pointer to the transmit*/
/*                                          routine for the device.         */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER          *buf_ptr            Pointer to the packet buffer of */
/*                                          the packet to send.             */
/*  DV_DEVICE_ENTRY     *device             Pointer to the device to send   */
/*                                          the packet over.                */
/*  SCK_SOCKADDR_IP     *dest               Not used by this routine. Only  */
/*                                          included in order to comply     */
/*                                          with NET 4.0.                   */
/*  RTAB_ROUTE          *ro                 Not used by this routine. Only  */
/*                                          included in order to comply     */
/*                                          with NET 4.0.                   */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  Was the packet sent?            */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        05/06/98      Created as part of integration    */
/*                                          with NET 4.0.                   */
/*                                                                          */
/****************************************************************************/
STATUS PPP_Output (NET_BUFFER *buf_ptr, DV_DEVICE_ENTRY *device,
                SCK_SOCKADDR_IP *dest, RTAB_ROUTE *ro)
{
    STATUS ret_status;

    /* Verify that the device is up. */
    if ( (device->dev_flags & (DV_UP | DV_RUNNING)) != (DV_UP | DV_RUNNING) )
        ret_status = NU_HOST_UNREACHABLE;

    else    /* send the packet */
    {
        ret_status = NU_SUCCESS;

        /* Send the packet */
        device->dev_start (device, buf_ptr);
    }

    /*  Deallocate this buffer.  */
    MEM_One_Buffer_Chain_Free (buf_ptr, buf_ptr->mem_dlist);

    return (ret_status);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Input                                                             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function pulls the first packet off of the incoming packet list  */
/* and calls the correct protocol routine to handle the encapsulated packet.*/
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    NET_Demux in file NET.C                                               */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  IP_Interpret                                                            */
/*  PPP_NCP_IP_Interpret                                                    */
/*  PPP_PAP_Interpret                                                       */
/*  PPP_LCP_Interpret                                                       */
/*  PPP_CHAP_Interpret                                                      */
/*  MEM_Buffer_Chain_Free                                                   */
/*  LCP_Send_Protocol_Reject                                                */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  none                                                                    */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  NU_SUCCESS                      */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        05/06/98      Created as part of integration    */
/*                                          with NET 4.0.                   */
/*  Uriah T. Pollock        05/18/98      Added changes per CSR1040, beta   */
/*                                          tester.                         */
/*                                                                          */
/****************************************************************************/
STATUS PPP_Input (VOID)
{
    uchar       *ppp_compress_protocol;
    uint16      ppp_protocol;

    /* Get a pointer to the head of the packet. */
    ppp_compress_protocol = (uchar *)MEM_Buffer_List.head->data_ptr;

    /* Now we must check to make sure it is a valid packet before
       it is passed on to get interpreted. We have to check for all
       combinations of compression. For the first check we will assume
       all compression is turned on. */

    /* For address and control field compression they are simple omitted.
       So there is no need to check for them. */

    /* Assume the protocol field is compressed, this will be the
       case most of the time. */
    switch (*ppp_compress_protocol)
    {
        case PPP_IP_COMPRESS_PROTOCOL   :

            /* Point the data ptr to the beginning of the IP header. */
            MEM_Buffer_List.head->data_ptr += PPP_WITH_BOTH_COMPRESS_START;
            MEM_Buffer_List.head->data_len -= PPP_WITH_BOTH_COMPRESS_START;
            MEM_Buffer_List.head->mem_total_data_len
                                            -= PPP_WITH_BOTH_COMPRESS_START;

            IP_Interpret ((IPLAYER *) MEM_Buffer_List.head->data_ptr,
                    MEM_Buffer_List.head->mem_buf_device, MEM_Buffer_List.head);

            break;

        default                         :

            /* Since it failed on the assumption try checking it one
               more time without protocol field compression. */

            /* Get the first byte. */
            ppp_protocol = MEM_Buffer_List.head->data_ptr[0];

            /* Shift it over to the MSB */
            ppp_protocol <<= 8;

            /* OR in the LSB byte */
            ppp_protocol = (ppp_protocol | MEM_Buffer_List.head->data_ptr[1]);

            switch (ppp_protocol)
            {
                case PPP_IP_PROTOCOL            :

                    /* Point the data ptr to the beginning of the IP header. */
                    MEM_Buffer_List.head->data_ptr += PPP_WITH_ADDR_COMPRESS_START;
                    MEM_Buffer_List.head->data_len -= PPP_WITH_ADDR_COMPRESS_START;
                    MEM_Buffer_List.head->mem_total_data_len
                                            -= PPP_WITH_ADDR_COMPRESS_START;

                    IP_Interpret ((IPLAYER *) MEM_Buffer_List.head->data_ptr,
                            MEM_Buffer_List.head->mem_buf_device,
                            MEM_Buffer_List.head);

                    break;

                case PPP_IP_CONTROL_PROTOCOL    :

                    /* Strip off the PPP header. */
                    MEM_Buffer_List.head->data_ptr += PPP_WITH_ADDR_COMPRESS_START;

                    PPP_NCP_IP_Interpret (MEM_Buffer_List.head);
                    break;

                case PPP_CHAP_PROTOCOL          :

                    /* Point the data ptr to the beginning of the IP header. */
                    MEM_Buffer_List.head->data_ptr += PPP_WITH_ADDR_COMPRESS_START;

                    PPP_CHAP_Interpret (MEM_Buffer_List.head);
                    break;

                case PPP_PAP_PROTOCOL          :

                    /* Strip offf the PPP header. */
                    MEM_Buffer_List.head->data_ptr += PPP_WITH_ADDR_COMPRESS_START;

                    PPP_PAP_Interpret (MEM_Buffer_List.head);
                    break;

                case PPP_LINK_CONTROL_PROTOCOL :

                    /* Strip offf the PPP header. */
                    MEM_Buffer_List.head->data_ptr += PPP_WITH_ADDR_COMPRESS_START;

                    PPP_LCP_Interpret (MEM_Buffer_List.head);
                    break;

                default                         :

                    /* Try checking it without address and control field
                       compression. */

                    /* Check the address and control fields. No compression. */
                    if ((MEM_Buffer_List.head->data_ptr[0] == PPP_HDLC_ADDRESS) &&
                        (MEM_Buffer_List.head->data_ptr[1] == PPP_HDLC_CONTROL))

                        /* Assume the protocol field is compressed, this
                           will be the case most of the time. */
                        switch (MEM_Buffer_List.head->data_ptr[2])
                        {
                            case PPP_IP_COMPRESS_PROTOCOL   :

                                /* Strip off the PPP header. */
                                MEM_Buffer_List.head->data_ptr += PPP_WITH_PROT_COMPRESS_START;
                                MEM_Buffer_List.head->data_len -= PPP_WITH_PROT_COMPRESS_START;
                                MEM_Buffer_List.head->mem_total_data_len
                                            -= PPP_WITH_PROT_COMPRESS_START;

                                IP_Interpret ((IPLAYER *) MEM_Buffer_List.head->data_ptr,
                                        MEM_Buffer_List.head->mem_buf_device,
                                        MEM_Buffer_List.head);

                                break;

                            default                         :

                                /* Since it failed on the assumption try checking
                                   it one more time without protocol field
                                   compression. */

                                /* Get the first byte. */
                                ppp_protocol = MEM_Buffer_List.head->data_ptr[2];

                                /* Shift it over to the MSB */
                                ppp_protocol <<= 8;

                                /* OR in the LSB byte */
                                ppp_protocol = (ppp_protocol | MEM_Buffer_List.head->data_ptr[3]);

                                switch (ppp_protocol)
                                {
                                    case PPP_IP_PROTOCOL            :

                                        /* Strip off the PPP header. */
                                        MEM_Buffer_List.head->data_ptr += PPP_PROTOCOL_START;
                                        MEM_Buffer_List.head->data_len -= PPP_PROTOCOL_START;
                                        MEM_Buffer_List.head->mem_total_data_len
                                            -= PPP_PROTOCOL_START;

                                        IP_Interpret ((IPLAYER *) MEM_Buffer_List.head->data_ptr,
                                            MEM_Buffer_List.head->mem_buf_device,
                                            MEM_Buffer_List.head);

                                        break;

                                    case PPP_IP_CONTROL_PROTOCOL    :

                                        /* Strip off the PPP header. */
                                        MEM_Buffer_List.head->data_ptr += PPP_PROTOCOL_START;

                                        PPP_NCP_IP_Interpret (MEM_Buffer_List.head);
                                        break;

                                    case PPP_LINK_CONTROL_PROTOCOL  :

                                        /* Strip off the PPP header. */
                                        MEM_Buffer_List.head->data_ptr += PPP_PROTOCOL_START;

                                        PPP_LCP_Interpret (MEM_Buffer_List.head);
                                        break;

                                    case PPP_CHAP_PROTOCOL          :

                                        /* Strip off the PPP header. */
                                        MEM_Buffer_List.head->data_ptr += PPP_PROTOCOL_START;

                                        PPP_CHAP_Interpret (MEM_Buffer_List.head);
                                        break;

                                    case PPP_PAP_PROTOCOL          :

                                        /* Strip off the PPP header. */
                                        MEM_Buffer_List.head->data_ptr += PPP_PROTOCOL_START;

                                        PPP_PAP_Interpret (MEM_Buffer_List.head);
                                        break;

                                    default                         :

                                        /* This packet is for a protocol
                                           unknown to this implementation.
                                           Reject the protocol. */

                                        LCP_Send_Protocol_Reject (MEM_Buffer_List.head);


                                        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
                                         break;

                                } /* switch - no compression */
                        } /* switch - just protocol field compression */
                    else /* if address and control fields ok */
                    {
                        /* This packet is for a protocol
                           unknown to this implementation.
                           Reject the protocol. */

                        LCP_Send_Protocol_Reject (MEM_Buffer_List.head);

                        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
                    }

            } /* switch - just address and control field compresssion */
    } /* switch - all compression */

    return (NU_SUCCESS);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Dial                                                              */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Dials a PPP server and attempts to establish a connection.              */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Application                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  MDM_Dial                                                                */
/*  NCP_Change_IP_Mode                                                      */
/*  MDM_Change_Communication_Mode                                           */
/*  PPP_Lower_Layer_Up                                                      */
/*  MDM_Hangup                                                              */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  CHAR            *number                 Pointer to the number to dial.  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  Was the connection made. If not */
/*                                          which protocol faild.           */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        05/06/98      Created as part of integration    */
/*                                          with NET 4.0.                   */
/*                                                                          */
/****************************************************************************/
STATUS PPP_Dial (CHAR *number)
{
    STATUS  ret_status;
    uint8   ip_addr[] = {0, 0, 0, 0};       /* zero denotes we will be assigned
                                               an IP address */

    /* Dial the number. */
    ret_status = MDM_Dial (number);

    /* Check to see if we connected. */
    if (ret_status == NU_SUCCESS)
    {
        /* Change to CLIENT mode, we must be a client if we are dialing. */
        NCP_Change_IP_Mode (CLIENT);

        /* Change to network mode */
        MDM_Change_Communication_Mode (MDM_NETWORK_COMMUNICATION);

        /* Start the PPP negotiation */
        ret_status = PPP_Lower_Layer_Up(ip_addr);

        /* If we failed to connect correctly then we need to hangup */
        if (ret_status != NU_SUCCESS)

            /* Hangup the modem */
            MDM_Hangup();
    }
    else
    {
        /* If we failed to connect correctly then we need to hangup */
        if (ret_status != NU_SUCCESS)

            /* Hangup the modem */
            MDM_Hangup();
    }

    return (ret_status);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_Wait_For_Client                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Puts the modem in answer mode and waits for a caller. When the modem has*/
/* connected with another modem a PPP session is attempted to be established*/ 
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Application                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  MDM_Wait_For_Client                                                     */
/*  NCP_Change_IP_Mode                                                      */
/*  MDM_Change_Communication_Mode                                           */
/*  PPP_Lower_Layer_Up                                                      */
/*  IP_Get_Net_Mask                                                         */
/*  DEV_Attach_IP_To_Device                                                 */
/*  MDM_Hangup                                                              */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  uint8           *server_ip_address      IP Address to assume when acting*/
/*                                          as a server for the incoming    */
/*                                          caller.                         */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  NU_SUCCESS                      */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        05/06/98      Created as part of integration    */
/*                                          with NET 4.0.                   */
/*                                                                          */
/****************************************************************************/
STATUS PPP_Wait_For_Client (uint8 *server_ip_addr)
{
    STATUS  ret_status;
    CHAR    subnet_mask [IP_ADDR_LEN];

    /* Assume that there is not a client connected. */
    ret_status = ~NU_SUCCESS;

    while (ret_status != NU_SUCCESS)
    {
        /* Start waiting for a client to call. */
        MDM_Wait_For_Client();

        /* Change to SERVER mode */
        NCP_Change_IP_Mode (SERVER);

        /* Change to network mode */
        MDM_Change_Communication_Mode(MDM_NETWORK_COMMUNICATION);

#ifdef DEBUG_PRINT
        _PRINT ("starting PPP negotiation\n");
#endif

        /* Start the PPP negotiation */
        ret_status = PPP_Lower_Layer_Up(server_ip_addr);

        /* If we connected then attach the server's IP address
           to this device. */
        if (ret_status == NU_SUCCESS)
        {
            /* Get the subnet mask for this type of address. */
            IP_Get_Net_Mask ((CHAR *)&server_ip_addr, (CHAR *)subnet_mask);

            /* Set our new address */
            DEV_Attach_IP_To_Device (temp_dev_ptr->dev_net_if_name,
                                        server_ip_addr, (uint8 *) subnet_mask);
        }
        else
            /* Hangup the modem in case it is still off hook. */
            MDM_Hangup();
    }

    return (ret_status);
}

