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
/*    LCP.C                                                  2.0            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains the link control protocol used by PPP to establish */
/*    and negotiate the configuration options that will be over the link.   */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah Pollock                                                         */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  NU_TIMER                LCP_Restart_Timer                               */
/*  NU_TIMER                LCP_Echo_Timer                                  */
/*  struct lcp_opts_struct  lcp_peer_options                                */
/*  struct lcp_opts_struct  lcp_my_options                                  */
/*  struct lcp_state_struct lcp_state                                       */
/*  int8                    _lcp_num_transmissions                          */
/*  static uint8            _echo_counter                                   */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*  LCP_Interpret                                                           */
/*  LCP_Send_Config_Req                                                     */
/*  LCP_Random_Number                                                       */
/*  LCP_Random_Number32                                                     */
/*  LCP_Timer_Expire                                                        */
/*  LCP_Configure_Req_Check                                                 */
/*  LCP_Send_Terminate_Ack                                                  */
/*  LCP_Send_Terminate_Req                                                  */
/*  LCP_Configure_Reject_Check                                              */
/*  LCP_Send_Code_Reject                                                    */
/*  LCP_Ack_Sent_State                                                      */
/*  LCP_Req_Sent_State                                                      */
/*  LCP_Ack_Rcvd_State                                                      */
/*  LCP_Open_State                                                          */
/*  LCP_Send_Echo_Reply                                                     */
/*  LCP_Closing_State                                                       */
/*  LCP_Code_Reject_Check                                                   */
/*  LCP_Configure_Nak_Check                                                 */
/*  LCP_Stopping_State                                                      */
/*  LCP_Echo_Expire                                                         */
/*  LCP_Send_Echo_Req                                                       */
/*  LCP_Send_Protocol_Reject                                                */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*  nucleus.h                                                               */
/*  externs.h                                                               */
/*  tcp_errs.h"                                                             */
/*  netevent.h                                                              */
/*  protocol.h                                                              */
/*  target.h                                                                */
/*  data.h                                                                  */
/*  ppp.h                                                                   */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        11/18/97      Fixed a bug in LCP_Timer_Expire   */
/*                                          and LCP_Echo_Expire - spr 375   */
/*  Uriah T. Pollock        11/18/97      Removed standard library headers  */
/*                                          from being included - spr 376   */
/*  Uriah T. Pollock        11/18/97      Changed LCP_Send_Configure_Req    */
/*                                          to include the ACCM - spr 376   */
/*  Uriah T. Pollock        11/18/97      Updated PPP to version 1.1        */
/*  Uriah T. Pollock        05/18/97      Added changes per CSR1040, beta   */
/*                                          tester.                         */
/*  Uriah T. Pollock        05/20/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/

#include "nucleus.h"
#include "externs.h"
#include "tcp_errs.h"
#include "netevent.h"
#include "protocol.h"
#include "target.h"        /* Compiler typedefs for data type sizes */
#include "data.h"

#include "ppp.h"

/* Declare the global options structure for the LCP. */
NU_TIMER                LCP_Restart_Timer;
NU_TIMER                LCP_Echo_Timer;
struct lcp_opts_struct  lcp_peer_options;
struct lcp_opts_struct  lcp_my_options;
struct lcp_state_struct lcp_state;
int8                    _lcp_num_transmissions;
static uint8            _echo_counter;

/* Import externally defined structures */
extern NU_EVENT_GROUP   Buffers_Available;
extern uint16           _silent_discards;
extern struct           lcp_state_struct    ncp_state;
extern uint8            ncp_mode;
extern DV_DEVICE_ENTRY  *temp_dev_ptr;      /* temp until a better solution
                                               can be found. */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_LCP_Interpret                                                     */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Based on the current state of PPP, this function passes the incoming    */
/* packet to the state function. After the packet has been processed it is  */
/* deallocated from the buffer list.                                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_Input                                                               */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  MDM_Hangup                                                              */
/*  LCP_Stopping_State                                                      */
/*  LCP_Ack_Sent_State                                                      */
/*  LCP_Req_Sent_State                                                      */
/*  LCP_Ack_Rcvd_State                                                      */
/*  LCP_Open_State                                                          */
/*  LCP_Closing_State                                                       */
/*  MEM_Buffer_Chain_Free                                                   */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER          *buf_ptr            Pointer to the buffer that holds*/
/*                                          the incoming packet             */
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
void PPP_LCP_Interpret (NET_BUFFER *buf_ptr)
{

#ifdef DEBUG_PRINT
    _PRINT ("\nlcp interpret ");
#endif

    /* The current state of the LCP will determine what we will
       do with this packet. */
    switch (lcp_state.state)
    {
        case INITIAL    :

#ifdef DEBUG_PRINT
                            _PRINT ("initial ");
#endif

                            /* This state is ONLY entered after the
                               modem has been hung up. If we ever
                               reach this point then there must be a problem
                               with the modem. So we will try to hang it
                               up again and ignore this packet. */

                            MDM_Hangup();

                            break;

        case STARTING   :

#ifdef DEBUG_PRINT
                            _PRINT ("initial ");
#endif

                            /* This state is ONLY entered after the
                               modem has been hung up. If we ever
                               reach this point then there must be a problem
                               with the modem. So we will try to hang it
                               up again and ignore this packet. */

                            MDM_Hangup();

                            break;            /* just drop it for now */

        case CLOSED     :

#ifdef DEBUG_PRINT
                            _PRINT ("closed ");
#endif
                            /* This state is entered when the this layer
                               finished action is done. While PPP is waiting
                               for the modem to hangup it holds the semaphore
                               for the network stack. So if a packet comes in
                               during this state it will not get processed
                               until after the modem is hungup and the
                               semaphore is released. So this state is
                               useless in this implementation. */

                            break;

        case STOPPED    :

#ifdef DEBUG_PRINT
                            _PRINT ("initial ");
#endif
                            /* This state is entered when the this layer
                               finished action is done, in one place it is
                               entered when negotiation has failed.
                               While PPP is waiting for the modem to hangup
                               it holds the semaphore for the network stack.
                               So if a packet comes in during this state it
                               will not get processed until after the modem
                               is hungup and the semaphore is released. So
                               this state is useless in this implementation.
                               If it is entered after negotiation has failed
                               then the modem is going to hangup as well. */

                            break;

        case CLOSING    :

#ifdef DEBUG_PRINT
                            _PRINT ("closing ");
#endif

                            /* Call the routine to handle this state. */
                            LCP_Closing_State(buf_ptr);

                            break;

        case STOPPING   :

#ifdef DEBUG_PRINT
                            _PRINT ("stopping ");
#endif

                            /* Call the routine to handle this state. */
                            LCP_Stopping_State (buf_ptr);

                            break;

        case REQ_SENT   :   /* Call the routine to handle this state */
#ifdef DEBUG_PRINT
                            _PRINT ("req_sent ");
#endif
                            LCP_Req_Sent_State (buf_ptr);
                            break;

        case ACK_RCVD   :   /* Call the routine to handle this state */
#ifdef DEBUG_PRINT
                            _PRINT ("ack_rcvd ");
#endif
                            LCP_Ack_Rcvd_State (buf_ptr);
                            break;

        case ACK_SENT   :   /* Call the routine to handle this state */
#ifdef DEBUG_PRINT
                            _PRINT ("ack_sent ");
#endif
                            LCP_Ack_Sent_State (buf_ptr);
                            break;

        case OPENED     :   /* Call the open state routine */
#ifdef DEBUG_PRINT
                            _PRINT ("opened ");
#endif
                            LCP_Open_State (buf_ptr);
                            break;
     
    }

    /* Release the buffer space */
    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);


}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    LCP_Send_Config_Req                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Sends a configure request packet containing the current set of          */
/* configuration options and their appropriate settings.                    */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_Lower_Layer_Up                                                      */
/*  LCP_Ack_Sent_State                                                      */
/*  LCP_Req_Sent_State                                                      */
/*  LCP_Ack_Recv_State                                                      */
/*  LCP_Open_State                                                          */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Random_Number                                                       */
/*  LCP_Random_Numer32                                                      */
/*  intswap                                                                 */
/*  sizeof                                                                  */
/*  memcpy                                                                  */
/*  PPP_TX_Packet                                                           */
/*  MEM_Buffer_Dequeue                                                      */
/*  NU_Tcp_Log_Error                                                        */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  none                                                                    */
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
/*  Uriah T. Pollock        11/18/97      Added code to send the ACCM as    */
/*                                          part of a configuration request */
/*                                          packet - spr 376                */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
void LCP_Send_Config_Req(void)
{
    LCP_LAYER       *lcp_pkt;
    uchar           *lcp_pkt_ptr, *byte_ptr;
    uint8           len;
    NET_BUFFER      *buf_ptr;

#ifdef DEBUG_PRINT
    _PRINT ("send config req ");
#endif

    len = 0;

    /* Is there a negotiation packet that can be reused? */
    if (lcp_state.negotiation_pkt != NU_NULL)
        buf_ptr = lcp_state.negotiation_pkt;
    else
        /* Allocate a buffer for the LCP packet. */
        lcp_state.negotiation_pkt = buf_ptr = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

    /* Make sure a buffer was available */
    if (buf_ptr == NU_NULL)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_SEVERE,
                          __FILE__, __LINE__);

        /* Get out */
        return;
    }

    /* Set the data ptr */
    buf_ptr->data_ptr = buf_ptr->mem_parent_packet + sizeof (PPP_HEADER);
    lcp_pkt = (LCP_LAYER *) buf_ptr->data_ptr;

    /* Set up the initial configure request packet. This packet will contain
       all the configuration options we need to configure. */
    lcp_pkt->code        = LCP_CONFIGURE_REQUEST;
    lcp_pkt->identifier  = LCP_Random_Number();

    /* Get a pointer to the data part of the LCP packet. */
    lcp_pkt_ptr         = &lcp_pkt->data;

    /* Go through all options and see which need to be configured. */

    /* Check the MRU (or MTU) */
    if (lcp_my_options.use_max_rx_unit)
    {
        lcp_pkt_ptr[len++] = LCP_MAX_RX_UNIT;
        lcp_pkt_ptr[len++] = LCP_MRU_LENGTH;
        lcp_pkt_ptr[len++] = (uint8) ((PPP_MTU >> 8) & PPP_BIT_MASK);
        lcp_pkt_ptr[len++] = (uint8) (PPP_MTU & PPP_BIT_MASK);
    }

    /* Check the Async Control Char Map. */
    if (lcp_my_options.use_accm)
    {
        lcp_pkt_ptr[len++] = LCP_ASYNC_CONTROL_CHAR_MAP;
        lcp_pkt_ptr[len++] = LCP_ACCM_LENGTH;

        byte_ptr           = (uchar *)&lcp_my_options.accm;

#ifdef SWAPPING
        lcp_pkt_ptr[len++]  = byte_ptr[3];
        lcp_pkt_ptr[len++]  = byte_ptr[2];
        lcp_pkt_ptr[len++]  = byte_ptr[1];
        lcp_pkt_ptr[len++]  = byte_ptr[0];
#else
        lcp_pkt_ptr[len++]  = byte_ptr[0];
        lcp_pkt_ptr[len++]  = byte_ptr[1];
        lcp_pkt_ptr[len++]  = byte_ptr[2];
        lcp_pkt_ptr[len++]  = byte_ptr[3];
#endif
    }
    
    /* Check the Authentication Protocol. We will only send a challenge
       if we are in sever mode and there is an authentication protocol to
       be used. */

    if (ncp_mode == SERVER)
    {
        if (lcp_my_options.authentication_protocol == PPP_CHAP_PROTOCOL)
        {
            lcp_pkt_ptr[len++] = LCP_AUTHENTICATION_PROTOCOL;
            lcp_pkt_ptr[len++] = LCP_CHAP_LENGTH;

            /* Put in the type of authentication */
            lcp_pkt_ptr[len++]  = ((lcp_my_options.authentication_protocol >> 8)
                                    & PPP_BIT_MASK);
            lcp_pkt_ptr[len++]  = (lcp_my_options.authentication_protocol
                                    & PPP_BIT_MASK);

            /* Put in the type of algorithm used, ours will be MD5 */
            lcp_pkt_ptr[len++] = LCP_CHAP_MD5;
        }
        else
        {
            if (lcp_my_options.authentication_protocol == PPP_PAP_PROTOCOL)
            {
                lcp_pkt_ptr[len++] = LCP_AUTHENTICATION_PROTOCOL;
                lcp_pkt_ptr[len++] = LCP_PAP_LENGTH;

                /* Put in the type of authentication */
                lcp_pkt_ptr[len++]  = ((lcp_my_options.authentication_protocol >> 8)
                                    & PPP_BIT_MASK);
                lcp_pkt_ptr[len++]  = (lcp_my_options.authentication_protocol
                                    & PPP_BIT_MASK);
            }
        }
    }


    /* Check the Quality protocol. */
    /* There is no support for this at this time. */

    /* Check the Magic Number. */
    if (lcp_my_options.magic_number)
    {
        lcp_pkt_ptr[len++]          = LCP_MAGIC_NUMBER;
        lcp_pkt_ptr[len++]          = LCP_MAGIC_NUMBER_LENGTH;
        lcp_my_options.magic_number = LCP_Random_Number32();
        byte_ptr                    = (uchar *)&lcp_my_options.magic_number;

#ifdef SWAPPING
        lcp_pkt_ptr[len++]  = byte_ptr[3];
        lcp_pkt_ptr[len++]  = byte_ptr[2];
        lcp_pkt_ptr[len++]  = byte_ptr[1];
        lcp_pkt_ptr[len++]  = byte_ptr[0];
#else
        lcp_pkt_ptr[len++]  = byte_ptr[0];
        lcp_pkt_ptr[len++]  = byte_ptr[1];
        lcp_pkt_ptr[len++]  = byte_ptr[2];
        lcp_pkt_ptr[len++]  = byte_ptr[3];
#endif
    }

    /* Protocol Field Compression. */
    if (lcp_my_options.protocol_field_compression)
    {
        lcp_pkt_ptr[len++] = LCP_PROTOCOL_FIELD_COMPRESS;
        lcp_pkt_ptr[len++] = LCP_PROTOCOL_COMPRESS_LENGTH;
    }

    /* Address and Control Compresssion. */
    if (lcp_my_options.address_field_compression)
    {
        lcp_pkt_ptr[len++] = LCP_ADDRESS_FIELD_COMPRESS;
        lcp_pkt_ptr[len++] = LCP_ADDRESS_COMPRESS_LENGTH;
    }

    /* Fill in the length for the LCP packet. */
    len             += LCP_HEADER_LENGTH;
    lcp_pkt->length  = intswap (len);

    /* Set the length for the buffer. */
    buf_ptr->data_len = buf_ptr->mem_total_data_len = len;

    /* Store this identifier for checking receive pkts. */
    lcp_state.identifier    = lcp_pkt->identifier;

    /* Ok, now send the pkt and let the automaton begin. */

    /* Bump the transmission count. */
    _lcp_num_transmissions--;

    /* Set the packet type. */
    buf_ptr->mem_flags = NET_LCP;

    /* Send the configure request packet. */
    PPP_TX_Packet (buf_ptr->mem_buf_device, buf_ptr);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  LCP_Random_Number                                                       */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*	This routine will handle Pseudo-random sequence generation.  The		*/
/*	algorithms and code was taken from the draft of the C standards document*/
/*	dated November 11, 1985.												*/
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*	Craig L. Meredith, Accelerated Technology Inc.							*/
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*	No functions call this function 										*/
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*	No functions called by this function									*/
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*	No inputs to the function												*/
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*	sint : Returns a value in the range of 0 to RAND_MAX.					*/
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*	NAME				DATE		REMARKS 								*/
/*                                                                          */
/*	Craig L. Meredith	04/10/93	Initial version.						*/
/*                                                                          */
/****************************************************************************/

#define LCP_RAND_MAX    256

uint8 LCP_Random_Number(void)
{
    static ulint next = 1;

    next = next * NU_Retrieve_Clock() + 12345;

    return ((uint8) ((next / 32) % (((uint) LCP_RAND_MAX) + 1)));
}  /* end rand routine */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  LCP_Random_Number32                                                     */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*	This routine will handle Pseudo-random sequence generation.  The		*/
/*	algorithms and code was taken from the draft of the C standards document*/
/*	dated November 11, 1985.												*/
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*	Craig L. Meredith, Accelerated Technology Inc.							*/
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*	No functions call this function 										*/
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*	No functions called by this function									*/
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*	No inputs to the function												*/
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*	sint : Returns a value in the range of 0 to RAND_MAX.					*/
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*	NAME				DATE		REMARKS 								*/
/*                                                                          */
/*	Craig L. Meredith	04/10/93	Initial version.						*/
/*                                                                          */
/****************************************************************************/
#define LCP_RAND_MAX32    1000000000ul

uint32 LCP_Random_Number32(void)
{
    static ulint next = 1;

    next = next * NU_Retrieve_Clock() + 12345;

    return (next % (((uint) LCP_RAND_MAX32) + 1));
}  /* end rand routine */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  LCP_Timer_Expire                                                        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This is the expiration routine for the LCP negotiation timer. It will   */
/* retransmit a LCP packet or signal PPP that negotiation failed. All is    */
/* based on the current state of LCP.                                       */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  Nucleus PLUS                                                            */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  UTL_Timerset                                                            */
/*  NU_Control_Timer                                                        */
/*  NU_Set_Events                                                           */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  UNSIGNED                dev_ptr         Address of the device structure */
/*                                          that the timer expired for.     */
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
/*  Uriah T. Pollock        11/18/97      Fixed a bug where this timer      */
/*                                         expiration routine would self    */
/*                                         suspend, some of the routines it */
/*                                         called did the actual suspension.*/
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
void LCP_Timer_Expire(UNSIGNED dev_ptr)
{

#ifdef DEBUG_PRINT
    _PRINT ("\nlcp_timer ");
#endif

    /* Check the current state to see what we can do. */
    switch (lcp_state.state)
    {
        case CLOSING    :
#ifdef DEBUG_PRINT
            _PRINT ("closing ");
#endif

			/* See if we get another try at terminating. */
            if (_lcp_num_transmissions-- > 0)
            {
#ifdef DEBUG_PRINT
                _PRINT ("send term req again ");
#endif

                /* Set the event to send the packet again. */
                UTL_Timerset (LCP_RESEND, dev_ptr, 0, 0);
            }
            else
            {
                /* Stop the restart timer. */
                NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Change states for both LCP and NCP */
				lcp_state.state = CLOSED;

                /* Set the event to hang the modem up. */
                UTL_Timerset (HANGUP_MDM, dev_ptr, 0, 0);

                /* Change states for both LCP and NCP */
                lcp_state.state = INITIAL;

            }


		break;

        case STOPPING   :
#ifdef DEBUG_PRINT
            _PRINT ("stopping\n");
#endif

            /* See if we get another try at terminating. */
            if (_lcp_num_transmissions-- > 0)
            {
#ifdef DEBUG_PRINT
                _PRINT ("send term req again ");
#endif

                /* Set the event to send the packet again. */
                UTL_Timerset (LCP_RESEND, dev_ptr, 0, 0);
            }
            else
            {
                /* Stop the restart timer. */
                NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Otherwise we have tried to many times, set the event
                   that will tell initialization we could not configure. Do
                   this only if NCP is not in the open state, this will
                   suggest that we are still negotiating the link and this
                   terminate is probaly because we could not authenticate. */

                if (ncp_state.state != OPENED)
                    NU_Set_Events (&Buffers_Available, LCP_CONFIG_FAIL, NU_OR);

                /* Change states for both LCP and NCP */
                lcp_state.state = STOPPED;

                /* Set the event to hang the modem up. */
                UTL_Timerset (HANGUP_MDM, dev_ptr, 0, 0);

                /* Change states */
                lcp_state.state = STARTING;

            }

		break;

        case REQ_SENT   :
#ifdef DEBUG_PRINT
            _PRINT ("req_sent ");
#endif

            /* See if we get another try at configuring. */
            if (_lcp_num_transmissions-- > 0)
            {
#ifdef DEBUG_PRINT
                _PRINT ("send config req again ");
#endif

                /* Set the event to send the packet again. */
                UTL_Timerset (LCP_RESEND, dev_ptr, 0, 0);
            }
            else
            {
                /* Stop the restart timer. */
                NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Change states */
                lcp_state.state = STOPPED;

                /* Otherwise we have tried to many times, set the event
                   that will tell initialization we could not configure. */
                NU_Set_Events (&Buffers_Available, LCP_CONFIG_FAIL, NU_OR);

            }

            break;

        case ACK_RCVD   :
#ifdef DEBUG_PRINT
            _PRINT ("ack_rcvd ");
#endif
            /* See if we get another try at configuring. */
            if (_lcp_num_transmissions-- > 0)
            {

                /* We need to send another configure request with the newest
                   current set of configuration options. Set the event to
                   do this. */

                UTL_Timerset (LCP_SEND_CONFIG, dev_ptr, 0, 0);

                lcp_state.state = REQ_SENT;

            }
            else
            {
                /* Stop the restart timer. */
                NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Change states */
                lcp_state.state = STOPPED;

                /* Otherwise we have tried to many times, set the event
                   that will tell initialization we could not configure. */
                NU_Set_Events (&Buffers_Available, LCP_CONFIG_FAIL, NU_OR);
            }

            break;

        case ACK_SENT   :
#ifdef DEBUG_PRINT
            _PRINT ("ack_sent ");
#endif
            /* See if we get another try at configuring. */
            if (_lcp_num_transmissions-- > 0)
            {

                /* We need to send another configure request with the newest
                   current set of configuration options. Set the event to
                   do this. */

                UTL_Timerset (LCP_SEND_CONFIG, dev_ptr, 0, 0);
            }
            else
            {
                /* Stop the restart timer. */
                NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Change states */
                lcp_state.state = STOPPED;

                /* Otherwise we have tried to many times, set the event
                   that will tell initialization we could not configure. */
                NU_Set_Events (&Buffers_Available, LCP_CONFIG_FAIL, NU_OR);
            }

            break;
            
         case STARTING	:
#ifdef DEBUG_PRINT
			_PRINT ("are we broke?? ");
#endif
			break;
    }
}


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  LCP_Configure_Req_Check                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function checks all configuration options in the incoming configure*/
/* request packet. If everything is ok then it will send an ACK to the peer.*/
/* If any one option is not ok it will NAK those options. If an option is   */
/* unknown it will reject that option.                                      */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Ack_Sent_State                                                      */
/*  LCP_Req_Sent_State                                                      */
/*  LCP_Ack_Rcvd_State                                                      */
/*  LCP_Open_State                                                          */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  intswap                                                                 */
/*  memcpy                                                                  */
/*  longswap                                                                */
/*  LCP_Random_Number32                                                     */
/*  dev_start                                                               */
/*  sizeof                                                                  */
/*  MEM_Buffer_Dequeue                                                      */
/*  MEM_Buffer_Enqueue                                                      */
/*  NU_Tcp_Log_Error                                                        */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *buf_ptr        Pointer to the incoming         */
/*                                          configure request packet        */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  Were the options                */
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
STATUS LCP_Configure_Req_Check (NET_BUFFER *buf_ptr)
{
    NET_BUFFER          *reject_buf_ptr, *nak_buf_ptr, *ack_buf_ptr;
    LCP_LAYER           *lcp_reject_pkt, *lcp_nak_pkt, *lcp_ack_pkt;
    uchar   HUGE        *lcp_pkt_ptr, *lcp_reject_ptr, *lcp_nak_ptr, *byte_ptr;
    uint8               options_ok, dont_need_to_reject, identifier;
    uint16              length, reject_length, nak_length, mru;
    uchar   HUGE        *lcp_pkt = buf_ptr->data_ptr;

    /* Allocate a buffer for each one of the packets that we may need to send. */
    reject_buf_ptr = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);
    nak_buf_ptr    = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);
    ack_buf_ptr    = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

    /* Make sure a buffer was available */
    if ((nak_buf_ptr == NU_NULL) || (ack_buf_ptr == NU_NULL) ||
            (reject_buf_ptr == NU_NULL))
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_SEVERE,
                          __FILE__, __LINE__);

        /* If any buffer were allocated then we need to put them back
           before we return. */
        if (reject_buf_ptr)
            MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, reject_buf_ptr);

        if (nak_buf_ptr)
            MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, nak_buf_ptr);

        if (ack_buf_ptr)
            MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, ack_buf_ptr);

        /* Get out */
        return (NU_FALSE);
    }


    /* Set the data pointer to give room for the PPP header. */
    reject_buf_ptr->data_ptr = (reject_buf_ptr->mem_parent_packet + sizeof (PPP_HEADER));
    nak_buf_ptr->data_ptr    = (nak_buf_ptr->mem_parent_packet + sizeof (PPP_HEADER));
    ack_buf_ptr->data_ptr    = (ack_buf_ptr->mem_parent_packet + sizeof (PPP_HEADER));

    /* Set the lcp layer pointers to point at the data section of the
       packet. */
    lcp_reject_pkt = (LCP_LAYER *) reject_buf_ptr->data_ptr;
    lcp_nak_pkt    = (LCP_LAYER *) nak_buf_ptr->data_ptr;
    lcp_ack_pkt    = (LCP_LAYER *) ack_buf_ptr->data_ptr;

    /* Point to the nak and reject packet data in case we have to
       reject or nak something */
    lcp_nak_ptr = &lcp_nak_pkt->data;
    nak_length  = LCP_HEADER_LENGTH;

    lcp_reject_ptr  = &lcp_reject_pkt->data;
    reject_length   = LCP_HEADER_LENGTH;
                                           
    /* This will be used to tell if all the config options are unacceptable */
    options_ok          = NU_TRUE;
    dont_need_to_reject = NU_TRUE;

    /* Get the ID of this pkt */
    identifier  = lcp_pkt[LCP_ID_OFFSET];

    /* Put the LCP structure on the packet and get the length */
    lcp_pkt_ptr = lcp_pkt;
    length      = *(lcp_pkt_ptr + LCP_LENGTH_OFFSET) << 8;
    length      =  length | *(lcp_pkt_ptr + LCP_LENGTH_OFFSET + 1);

    /* Save the length before we take off the header. This will be used
       if the options are ok */
    lcp_ack_pkt->length = intswap (length);

    /* Now remove the header */
    length     -= LCP_HEADER_LENGTH;

    /* Get a pointer to the data inside the LCP pkt */
    lcp_pkt_ptr = lcp_pkt_ptr + LCP_DATA_OFFSET;

    while (length > 0)
    {
        /* Check out what the option is and see if we can handle it */
        switch (lcp_pkt_ptr[0])
        {
            case LCP_MAX_RX_UNIT            :

                /* Pull the MRU from the packet. First push the pointer
                   to the MRU option. */
                lcp_pkt_ptr += LCP_CONFIG_OPTS;

                /* Now pull out the MSB. */
                mru = (uint8) lcp_pkt_ptr[0];

                /* Shift it to the correct location. */
                mru <<= 8;

                /* Get the LSB and OR it in with the MSB */
                mru |= (uint8) lcp_pkt_ptr[1];

                /* Update the MTU for this device so that we do not
                   attempt to send packets larger than our peer
                   can handle. */
                buf_ptr->mem_buf_device->dev_mtu = mru;

                /* Update the pkt pointer and the packet size */
                lcp_pkt_ptr += (LCP_MRU_LENGTH - LCP_CONFIG_OPTS);
                length -= LCP_MRU_LENGTH;

                break;
                                     

            case LCP_ASYNC_CONTROL_CHAR_MAP :

                /* Store off the char map for our peer. */

                /* Get the high bits */
                lcp_peer_options.accm = *(lcp_pkt_ptr + LCP_CONFIG_OPTS) << 8;
                lcp_peer_options.accm = lcp_peer_options.accm |
                               *(lcp_pkt_ptr + LCP_CONFIG_OPTS + 1);

                /* Move them to the high side of our ACCM var */
                lcp_peer_options.accm = lcp_peer_options.accm << 16;

                /* Now get the low bits */
                lcp_peer_options.accm = lcp_peer_options.accm |
                               (*(lcp_pkt_ptr + LCP_CONFIG_OPTS + 2) << 8);
                lcp_peer_options.accm = lcp_peer_options.accm |
                               *(lcp_pkt_ptr + LCP_CONFIG_OPTS + 3);

                /* Update the pkt pointer and the packet size */
                lcp_pkt_ptr += LCP_ACCM_LENGTH;
                length -= LCP_ACCM_LENGTH;

                break;

            case LCP_AUTHENTICATION_PROTOCOL :

                /* Check to see if the peer is trying to use an authentication
                   protocol that we support. */

                /* Is is chap? */
                if ((lcp_pkt_ptr [LCP_CONFIG_OPTS] ==
                    ((PPP_CHAP_PROTOCOL >> 8) & PPP_BIT_MASK))
                    && ((lcp_pkt_ptr [LCP_CONFIG_OPTS + 1] ==
                    (PPP_CHAP_PROTOCOL & PPP_BIT_MASK))))

                    /* Yes it is, make sure it is with MD5? */
                    if (lcp_pkt_ptr [LCP_DATA_OFFSET] != LCP_CHAP_MD5)
                    {
                        /* At this point the peer is using CHAP but not with
                        MD5. We must NAK and state that we must use MD5. */

                        lcp_pkt_ptr[LCP_DATA_OFFSET] = LCP_CHAP_MD5;

                        /* Put it in the NAK packet */
                        memcpy (lcp_nak_ptr, lcp_pkt_ptr, LCP_CHAP_LENGTH);

                        /* Update the length and the ptr */
                        lcp_nak_ptr += lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];
                        nak_length += lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                        /* Mark that we need to NAK something */
                        options_ok = NU_FALSE;

                    }
                    else
                    {
                        /* If we make it here then our peer is using
                           MD5-CHAP, which is fine with us. Put this in
                           in the options structure. */
                        lcp_my_options.authentication_protocol =
                                                PPP_CHAP_PROTOCOL;
                    }
                else

                /* See if it is PAP */
                if ((lcp_pkt_ptr [LCP_CONFIG_OPTS] ==
                    ((PPP_PAP_PROTOCOL >> 8) & PPP_BIT_MASK))
                    && ((lcp_pkt_ptr [LCP_CONFIG_OPTS + 1] ==
                    (PPP_PAP_PROTOCOL & PPP_BIT_MASK))))
                {
                    /* This is fine. Put it in the options structure. */
                    lcp_my_options.authentication_protocol = PPP_PAP_PROTOCOL;
                }
                else
                {
                    /* At this point the peer does not request CHAP or PAP.
                       We must NAK the authentication protocol and
                       suggest that we use our default protocol.  */

                    lcp_pkt_ptr[LCP_CONFIG_OPTS]  =
                            ((lcp_my_options.authentication_protocol >> 8)
                                    & PPP_BIT_MASK);
                    lcp_pkt_ptr[LCP_CONFIG_OPTS + 1]  =
                            (lcp_my_options.authentication_protocol
                                & PPP_BIT_MASK);

                    /* If the default protocol is CHAP, we must add MD5 as
                       the encryption algoritm used. */
                    if (lcp_my_options.authentication_protocol ==
                                PPP_CHAP_PROTOCOL)
                    {

                        /* Put in the type of algorithm used, ours will be MD5 */
                        lcp_pkt_ptr[LCP_DATA_OFFSET] = LCP_CHAP_MD5;

                        /* Put it in the NAK packet */
                        memcpy (lcp_nak_ptr, lcp_pkt_ptr, LCP_CHAP_LENGTH);

                        /* Add the correct length */
                        lcp_nak_ptr [LCP_CONFIG_LENGTH_OFFSET] = LCP_CHAP_LENGTH;

                    }
                    else
                    {

                        /* Put it in the NAK packet */
                        memcpy (lcp_nak_ptr, lcp_pkt_ptr, LCP_PAP_LENGTH);

                        /* Add the correct length */
                        lcp_nak_ptr [LCP_CONFIG_LENGTH_OFFSET] = LCP_PAP_LENGTH;

                    }

                    /* Update the length and the ptr */
                    nak_length += lcp_nak_ptr[LCP_CONFIG_LENGTH_OFFSET];
                    lcp_nak_ptr += lcp_nak_ptr[LCP_CONFIG_LENGTH_OFFSET];

                    /* Mark that we need to NAK something */
                    options_ok = NU_FALSE;

                }

                length -= lcp_pkt_ptr [LCP_CONFIG_LENGTH_OFFSET];
                lcp_pkt_ptr += lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                break;

            case LCP_QUALITY_PROTOCOL       :

                /* There is no support for this at this time so reject
                   it */

                /* Put it in the reject packet */
                memcpy (lcp_reject_ptr, lcp_pkt_ptr,
                                        lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET]);

                /* Update the length and the ptr */
                lcp_reject_ptr += lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];
                reject_length += lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                /* Mark that we need to reject something */
                dont_need_to_reject = NU_FALSE;

                length -= lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];
                lcp_pkt_ptr += lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                break;

            case LCP_MAGIC_NUMBER           :

                /* Extract the magic number from the packet */

                /* Get the high bits */
                lcp_peer_options.magic_number = *(lcp_pkt_ptr + LCP_CONFIG_OPTS) << 8;
                lcp_peer_options.magic_number = lcp_peer_options.magic_number |
                               *(lcp_pkt_ptr + LCP_CONFIG_OPTS + 1);

                /* Move them to the high side of our magic num var */
                lcp_peer_options.magic_number = lcp_peer_options.magic_number << 16;

                /* Now get the low bits */
                lcp_peer_options.magic_number = lcp_peer_options.magic_number |
                    (LCP_CLEAR_HIGH & (*(lcp_pkt_ptr + LCP_CONFIG_OPTS + 2) << 8));
                lcp_peer_options.magic_number = lcp_peer_options.magic_number |
                    (LCP_CLEAR_HIGH & (*(lcp_pkt_ptr + LCP_CONFIG_OPTS + 3)));

                /* Make sure they are not the same. Note :
                   lcp_my_options.magic_number is stored in the endien order
                   of the processor. Therefore the longswap on it is needed.
                   lcp_peer_options.magic_number is stored from the incomming
                   pkt and will always be in the correct order, so no swapping
                   is needed. */
                if (lcp_peer_options.magic_number ==
                                        longswap (lcp_my_options.magic_number))
                {
                    /* Since they are we must NAK with a new number */

                    /* Put it in the nak packet */
                    memcpy (lcp_nak_ptr, lcp_pkt_ptr,
                                                  LCP_MAGIC_NUMBER_LENGTH);

                    /* Get a new number */
                    lcp_my_options.magic_number = LCP_Random_Number32();

                    /* Move the pointer to the spot where the number goes. */
                    lcp_nak_ptr += LCP_CONFIG_OPTS;

                    /* Fill in the new number */
                    byte_ptr = (uchar*)&lcp_my_options.magic_number;

#ifdef SWAPPING
                    lcp_nak_ptr[0] = byte_ptr[3];
                    lcp_nak_ptr[1] = byte_ptr[2];
                    lcp_nak_ptr[2] = byte_ptr[1];
                    lcp_nak_ptr[3] = byte_ptr[0];
#else
                    lcp_nak_ptr[0] = byte_ptr[0];
                    lcp_nak_ptr[1] = byte_ptr[1];
                    lcp_nak_ptr[2] = byte_ptr[2];
                    lcp_nak_ptr[3] = byte_ptr[3];
#endif

                    /* Update the pkt ptr and length */
                    nak_length += LCP_MAGIC_NUMBER_LENGTH;
                    lcp_nak_ptr += LCP_MAGIC_NUMBER_LENGTH - LCP_CONFIG_OPTS;
                                                             
                    /* Set the status that all options are not ok */
                    options_ok = NU_FALSE;
                }

                /* Update the pkt pointer and the packet size */
                lcp_pkt_ptr += LCP_MAGIC_NUMBER_LENGTH;
                length -= LCP_MAGIC_NUMBER_LENGTH;

                break;

            case LCP_PROTOCOL_FIELD_COMPRESS :

                /* See if we are going to use protocol compression */
                if (lcp_my_options.protocol_field_compression)
                    lcp_peer_options.protocol_field_compression = NU_TRUE;
                else
                {
                    /* We need to reject this option since it is a boolean
                       option */

                    /* Put it in the reject packet */
                    memcpy (lcp_reject_ptr, lcp_pkt_ptr,
                                                LCP_PROTOCOL_COMPRESS_LENGTH);

                    /* Update the length and the ptr */
                    lcp_reject_ptr += LCP_PROTOCOL_COMPRESS_LENGTH;
                    reject_length += LCP_PROTOCOL_COMPRESS_LENGTH;

                    /* Mark that we need to reject something */
                    dont_need_to_reject = NU_FALSE;
                }

                /* Update the pkt pointer and the packet size */
                lcp_pkt_ptr += LCP_PROTOCOL_COMPRESS_LENGTH;
                length -= LCP_PROTOCOL_COMPRESS_LENGTH;

                break;

            case LCP_ADDRESS_FIELD_COMPRESS :

                /* See if we are going to use address compression */
                if (lcp_my_options.address_field_compression)
                    lcp_peer_options.address_field_compression = NU_TRUE;
                else
                {
                    /* We need to reject this option since it is a boolean
                       option */

                    /* Put it in the reject packet */
                    memcpy (lcp_reject_ptr, lcp_pkt_ptr,
                                                LCP_ADDRESS_COMPRESS_LENGTH);

                    /* Update the length and the ptr */
                    lcp_reject_ptr += LCP_ADDRESS_COMPRESS_LENGTH;
                    reject_length += LCP_ADDRESS_COMPRESS_LENGTH;

                    /* Mark that we need to reject something */
                    dont_need_to_reject = NU_FALSE;
                }


                /* Update the pkt pointer and the packet size */
                lcp_pkt_ptr += LCP_ADDRESS_COMPRESS_LENGTH;
                length -= LCP_ADDRESS_COMPRESS_LENGTH;

                break;

            default                         :

                /* If we make it here then there is a protocol that we
                   do not understand, send a reject. */

                /* Put it in the reject packet */
                memcpy (lcp_reject_ptr, lcp_pkt_ptr,
                                       lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET]);

                /* Update the length and the ptr */
                lcp_reject_ptr += lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];
                reject_length += lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                /* Mark that we need to reject something */
                dont_need_to_reject = NU_FALSE;

                /* The length is usually the second field in the protocol,
                   so minus it off and look at the reset of the pkt. */
                length -= lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];
                lcp_pkt_ptr += lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                break;

        } /* switch */
    } /* while */


    /* See if all the options are ok, if so send an ack */
    if (options_ok && dont_need_to_reject)
    {
#ifdef DEBUG_PRINT
        _PRINT ("acking ");
#endif

        /* Every option is ok. The ack pkt is the same as the config pkt.
           The only difference is that the pkt code is now config ack. */

        /* Change the pkt code and put in the same ID */
        lcp_ack_pkt->code = LCP_CONFIGURE_ACK;
        lcp_ack_pkt->identifier = identifier;

        /* Copy the pkt data over */
        memcpy (&lcp_ack_pkt->data, lcp_pkt + LCP_DATA_OFFSET,
                                           intswap(lcp_ack_pkt->length));

        /* Update the lengths for the buffer. */
        ack_buf_ptr->data_len = ack_buf_ptr->mem_total_data_len =
                intswap(lcp_ack_pkt->length);

        /* Set the packet type. */
        ack_buf_ptr->mem_flags = NET_LCP;

        /* Send it */
        buf_ptr->mem_buf_device->dev_start (buf_ptr->mem_buf_device, ack_buf_ptr);

    }

    {
        /* One or more options were not ok. We must nak or reject them. */

        /* See if we need to nak something. Only NAK if we do not have to
           reject. */
        if ((!options_ok) && (dont_need_to_reject))
        {
#ifdef DEBUG_PRINT
            _PRINT ("naking ");
#endif

            /* Put in the pkt indentifier and type */
            lcp_nak_pkt->identifier = identifier;
            lcp_nak_pkt->code = LCP_CONFIGURE_NAK;
            lcp_nak_pkt->length = intswap(nak_length);

            /* Update the lengths for the buffer. */
            nak_buf_ptr->data_len = nak_buf_ptr->mem_total_data_len =
                nak_length;

            /* Set the packet type. */
            nak_buf_ptr->mem_flags = NET_LCP;

            /* Send the pkt */
            buf_ptr->mem_buf_device->dev_start (buf_ptr->mem_buf_device, nak_buf_ptr);
        }

        /* See if we need to reject something */
        if (!dont_need_to_reject)
        {
#ifdef DEBUG_PRINT
            _PRINT ("rejecting ");
#endif
            /* Put in the pkt indentifier and type */
            lcp_reject_pkt->identifier = identifier;
            lcp_reject_pkt->code = LCP_CONFIGURE_REJECT;
            lcp_reject_pkt->length = intswap(reject_length);

            /* Update the lengths for the buffer. */
            reject_buf_ptr->data_len = reject_buf_ptr->mem_total_data_len =
                reject_length;

            /* Set the packet type. */
            reject_buf_ptr->mem_flags = NET_LCP;

            /* Send the pkt */
            buf_ptr->mem_buf_device->dev_start (buf_ptr->mem_buf_device, reject_buf_ptr);
        }
    }

    /* Release the buffers allocated at the beginning of this function. */
    MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, reject_buf_ptr);
    MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, nak_buf_ptr);
    MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, ack_buf_ptr);

    return (options_ok && dont_need_to_reject);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  LCP_Send_Terminate_Ack                                                  */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Sends a terminate ack packet to the peer.                               */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Ack_Sent_State                                                      */
/*  LCP_Req_Sent_State                                                      */
/*  LCP_Ack_Rcvd_State                                                      */
/*  LCP_Open_State                                                          */
/*  LCP_Closing_State                                                       */
/*  LCP_Stopping_State                                                      */
/*  NCP_IP_Req_Sent_State                                                   */
/*  NCP_IP_Ack_Rcvd_State                                                   */
/*  NCP_IP_Ack_Sent_State                                                   */
/*  NCP_IP_Opened_State                                                     */
/*  NCP_IP_Stopping_State                                                   */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  sizeof                                                                  */
/*  memcpy                                                                  */
/*  dev_start                                                               */
/*  NU_Tcp_Log_Error                                                        */
/*  MEM_Buffer_Dequeue                                                      */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  uchar                   *in_buf_ptr     Pointer to the packet that      */
/*                                          requested termination           */
/*  uint16                  protocol_type   Which protocol, LCP or NCP, is  */
/*                                          sending the ack                 */
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
void LCP_Send_Terminate_Ack(NET_BUFFER *in_buf_ptr, uint16 protocol_type)
{
    NET_BUFFER          *buf_ptr;
    uint16              length;
    uchar   HUGE        *lcp_pkt = in_buf_ptr->data_ptr;

#ifdef DEBUG_PRINT
    _PRINT ("\nsend term ack ");
#endif

    /* Is there a negotiation packet that can be reused? */
    if (lcp_state.negotiation_pkt != NU_NULL)
        buf_ptr = lcp_state.negotiation_pkt;
    else
        /* Allocate a buffer for the LCP packet. */
        lcp_state.negotiation_pkt = buf_ptr = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

    /* Make sure a buffer was available */
    if (buf_ptr == NU_NULL)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_SEVERE,
                          __FILE__, __LINE__);

        /* Get out */
        return;
    }

    /* Set the data ptr */
    buf_ptr->data_ptr = (buf_ptr->mem_parent_packet + sizeof (PPP_HEADER));

    /* The pkt to be sent is the same as the one RX except the code field is
       changed to ACK. */

    /* Put in the pkt type */
    lcp_pkt [LCP_CODE_OFFSET] = LCP_TERMINATE_ACK;

    /* Get the length */
    length      = *(lcp_pkt + LCP_LENGTH_OFFSET) << 8;
    length      =  length | *(lcp_pkt + LCP_LENGTH_OFFSET + 1);

    /* Save a copy of this packet for retransmission, if needed. */
    memcpy (buf_ptr->data_ptr, lcp_pkt, length);

    /* Set the protocol type */
    if (protocol_type == PPP_LINK_CONTROL_PROTOCOL)
        buf_ptr->mem_flags = NET_LCP;
    else
        if (protocol_type == PPP_IP_CONTROL_PROTOCOL)
            buf_ptr->mem_flags = NET_IPCP;

    /* Set the length */
    buf_ptr->data_len = buf_ptr->mem_total_data_len = length;

    /* Send the pkt */
    in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device, buf_ptr);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  LCP_Send_Terminate_Req                                                  */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Sends a terminate req packet to the peer.                               */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Open_State                                                          */
/*  NU_EventsDispatcher                                                     */
/*  NCP_IP_Opened_State                                                     */
/*  PPP_Hangup                                                              */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Random_Number                                                       */
/*  intswap                                                                 */
/*  sizeof                                                                  */
/*  memcpy                                                                  */
/*  PPP_TX_Packet                                                           */
/*  MEM_Buffer_Dequeue                                                      */
/*  NU_Tcp_Log_Error                                                        */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  none                                                                    */
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
void LCP_Send_Terminate_Req (void)
{
    NET_BUFFER  *buf_ptr;
    LCP_LAYER   *lcp_pkt;
    uint8       len;

#ifdef DEBUG_PRINT
    _PRINT ("send term req ");
#endif

    len = 0;

    /* Is there a negotiation packet that can be reused? */
    if (lcp_state.negotiation_pkt != NU_NULL)
        buf_ptr = lcp_state.negotiation_pkt;
    else
        /* Allocate a buffer for the LCP packet. */
        lcp_state.negotiation_pkt = buf_ptr = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

    /* Make sure a buffer was available */
    if (buf_ptr == NU_NULL)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_SEVERE,
                          __FILE__, __LINE__);

        /* Get out */
        return;
    }

    /* Set the data pointer for the buffer */
    buf_ptr->data_ptr = (buf_ptr->mem_parent_packet + sizeof (PPP_HEADER));

    /* Point the LCP structure to the data part of the outgoing buffer */
    lcp_pkt = (LCP_LAYER *) buf_ptr->data_ptr;

    /* Set up the terminate pkt. */
    lcp_pkt->code        = LCP_TERMINATE_REQUEST;
    lcp_pkt->identifier  = LCP_Random_Number();

    /* Fill in the length for the lcp packet. */
    len                 += LCP_HEADER_LENGTH;
    lcp_pkt->length      = intswap (len);

    /* Set the type of packet to be TX*/
    buf_ptr->mem_flags = NET_LCP;

    /* Set the size of the buffer. */
    buf_ptr->data_len = buf_ptr->mem_total_data_len = len;

    /* Send the terminate request packet. */
    PPP_TX_Packet (buf_ptr->mem_buf_device, buf_ptr);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  LCP_Configure_Reject_Check                                              */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function checks a configure reject packet and updates the peers    */
/* allowed options based on what was rejected. The next configure request   */
/* sent will reflect these changes.                                         */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Ack_Sent_State                                                      */
/*  LCP_Req_Sent_State                                                      */
/*  LCP_Ack_Rcvd_State                                                      */
/*  LCP_Opened_State                                                        */
/*  LCP_Closing_State                                                       */
/*  LCP_Stopping_State                                                      */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  none                                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *buf_ptr        Pointer to the code reject      */
/*                                          packet                          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  Can PPP live without the reject */
/*                                          option                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/* Uriah T. Pollock         05/18/97       Created initial version per      */
/*                                           CSR1040, beta tester.          */
/*                                                                          */
/****************************************************************************/
VOID LCP_Configure_Reject_Check (NET_BUFFER *buf_ptr)
{
    uchar   HUGE        *lcp_pkt_ptr;
    uint8               identifier;
    uint16              length;
    uchar   HUGE        *lcp_pkt = buf_ptr->data_ptr;

    /* Get the ID of this pkt */
    identifier = lcp_pkt[LCP_ID_OFFSET];

    /* The id must match that of the last sent config pkt. */
    if (identifier == lcp_state.identifier)
    {

        /* Put the LCP structure on the packet and get the length */
        lcp_pkt_ptr = lcp_pkt;
        length      = *(lcp_pkt_ptr + LCP_LENGTH_OFFSET) << 8;
        length      =  length | *(lcp_pkt_ptr + LCP_LENGTH_OFFSET + 1);

        /* Now remove the header */
        length     -= LCP_HEADER_LENGTH;

        /* Get a pointer to the data inside the LCP pkt */
        lcp_pkt_ptr = lcp_pkt_ptr + LCP_DATA_OFFSET;

        while (length > 0)
        {
            /* Check out what option is rejected and see if we can handle it */
            switch (lcp_pkt_ptr[0])
            {
                case LCP_MAX_RX_UNIT            :

                    /* Set the option to false */
                    lcp_my_options.use_max_rx_unit = NU_FALSE;

                    /* Update the pkt pointer and the packet size */
                    lcp_pkt_ptr += LCP_MRU_LENGTH;
                    length -= LCP_MRU_LENGTH;

                    break;

                case LCP_ASYNC_CONTROL_CHAR_MAP :

                    /* Set the option to false */
                    lcp_my_options.use_accm = NU_FALSE;

                    /* Update the pkt pointer and the packet size */
                    lcp_pkt_ptr += LCP_ACCM_LENGTH;
                    length -= LCP_ACCM_LENGTH;
                                     
                    break;

                case LCP_MAGIC_NUMBER           :

                    /* Set this option to false */
                    lcp_my_options.magic_number = NU_FALSE;

                    /* Update the pkt pointer and the packet size */
                    lcp_pkt_ptr += LCP_MAGIC_NUMBER_LENGTH;
                    length -= LCP_MAGIC_NUMBER_LENGTH;

                    break;

                case LCP_PROTOCOL_FIELD_COMPRESS :

                    /* This option is not required. Turn the option off
                       for us and our peer. */
                    lcp_peer_options.protocol_field_compression = NU_FALSE;
                    lcp_my_options.protocol_field_compression = NU_FALSE;

                    /* Update the pkt pointer and the packet size */
                    lcp_pkt_ptr += LCP_PROTOCOL_COMPRESS_LENGTH;
                    length -= LCP_PROTOCOL_COMPRESS_LENGTH;

                    break;

                case LCP_ADDRESS_FIELD_COMPRESS :

                    /* This option is not required. Turn the option off
                       for us and our peer. */
                    lcp_peer_options.address_field_compression = NU_FALSE;
                    lcp_my_options.address_field_compression = NU_FALSE;

                    /* Update the pkt pointer and the packet size */
                    lcp_pkt_ptr += LCP_ADDRESS_COMPRESS_LENGTH;
                    length -= LCP_ADDRESS_COMPRESS_LENGTH;

                    break;

            } /* switch */

        } /* while */

    } /* if */
    else
    {
#ifdef DEBUG_PRINT
        _PRINT ("discarded - invalid ID ");
#endif

        /* This reject is not for the last configure pkt we sent. So
           drop it. */
        _silent_discards++;
    }

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  LCP_Code_Reject_Check                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function checks a code reject packet. If it is rejecting any       */
/* codes that PPP needs to work NU_FALSE is returned. If all rejected codes */
/* are allowed by this implementation then NU_TRUE is returned. Note that   */
/* most codes specified in RFC1661 are required. Any codes that are not     */
/* required by this implementation are never transmitted and thus will never*/
/* be rejected.                                                             */
/*                                                                          */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Ack_Sent_State                                                      */
/*  LCP_Req_Sent_State                                                      */
/*  LCP_Ack_Rcvd_State                                                      */
/*  LCP_Opened_State                                                        */
/*  LCP_Closing_State                                                       */
/*  LCP_Stopping_State                                                      */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  none                                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *buf_ptr        Pointer to the code reject      */
/*                                          packet                          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  Can PPP live without the reject */
/*                                          codes.                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/* Uriah T. Pollock         05/18/97       Update per CSR1040, beta tester. */
/*                                           It turns out that this function*/
/*                                           was actually performing the    */
/*                                           function of Configure_Reject   */
/*                                           _Check. There was some         */
/*                                           confusion of the two in the    */
/*                                           original implementation.       */
/*                                                                          */
/****************************************************************************/
STATUS LCP_Code_Reject_Check (NET_BUFFER *buf_ptr)
{
    uchar   HUGE        *lcp_pkt_ptr;
    uint8               codes_ok;
    uchar   HUGE        *lcp_pkt = buf_ptr->data_ptr;


    /* Assume that the code rejected is ok. */
    codes_ok = NU_TRUE;

    /* Put the LCP structure on the packet */
    lcp_pkt_ptr = lcp_pkt;

    /* Get a pointer to the data inside the LCP pkt */
    lcp_pkt_ptr = lcp_pkt_ptr + LCP_DATA_OFFSET;

    /* Check out what code is rejected and see if we can handle it. If it
       is one of the codes below the link will be terminated.  */
    switch (lcp_pkt_ptr[0])
    {
        case LCP_CONFIGURE_REQUEST  :
        case LCP_CONFIGURE_ACK      :
        case LCP_CONFIGURE_NAK      :
        case LCP_CONFIGURE_REJECT   :
        case LCP_TERMINATE_REQUEST  :
        case LCP_TERMINATE_ACK      :
        case LCP_CODE_REJECT        :       /* This one doesn't make sense. */
        case LCP_PROTOCOL_REJECT    :
        case LCP_ECHO_REQUEST       :
        case LCP_ECHO_REPLY         :       /* This one doesn't make sense. */
        case LCP_DISCARD_REQUEST    :


            /* If it was one of the above codes then we must
               terminate the link. */
            codes_ok = NU_FALSE;

            break;

        default :

            /* It is some unknown code and thus is ok to be rejected. We
               did not send it anyway. This packet is probably a response by an
               incorrect implementation, poor parsing of a received packet. */

            break;

    } /* switch */

    return (codes_ok);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  LCP_Send_Code_Reject                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Sends a code reject to the peer telling it that an option was received  */
/* that is unknown to this implementation.                                  */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Ack_Sent_State                                                      */
/*  LCP_Req_Sent_State                                                      */
/*  LCP_Ack_Rcvd_State                                                      */
/*  LCP_Opened_State                                                        */
/*  LCP_Closing_State                                                       */
/*  LCP_Stopping_State                                                      */
/*  PPP_NCP_IP_Interpret                                                    */
/*  NCP_IP_Req_Sent_State                                                   */
/*  NCP_IP_Stopping_State                                                   */
/*  NCP_IP_Ack_Rcvd_State                                                   */
/*  NCP_IP_Ack_Sent_State                                                   */
/*  NCP_Opened_State                                                        */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Random_Number                                                       */
/*  intswap                                                                 */
/*  sizeof                                                                  */
/*  dev_start                                                               */
/*  MEM_Buffer_Dequeue                                                      */
/*  MEM_Buffer_Enqueue                                                      */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  uchar                   *in_buf_ptr     Pointer to that packet that is  */
/*                                          unknown to PPP                  */
/*  uint16                  protocol_type   Which protocol is sending it,   */
/*                                          LCP or NCP                      */
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
void LCP_Send_Code_Reject (NET_BUFFER *in_buf_ptr, uint16 protocol_type)
{
    NET_BUFFER          *buf_ptr;
    LCP_LAYER           *lcp_reject_pkt;
    uchar               *lcp_pkt_ptr;
    uint8               len;
    uchar   HUGE        *lcp_pkt = in_buf_ptr->data_ptr;

#ifdef DEBUG_PRINT
    _PRINT ("send code reject ");
#endif

    /* Allocate a buffer to send. */
    buf_ptr = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

    /* Make sure a buffer was available */
    if (buf_ptr == NU_NULL)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_SEVERE,
                          __FILE__, __LINE__);

        /* Get out */
        return;
    }

    /* Set the data pointer and the reject packet pointer. */
    buf_ptr->data_ptr   = (buf_ptr->mem_parent_packet + sizeof (PPP_HEADER));
    lcp_reject_pkt      = (LCP_LAYER *) buf_ptr->data_ptr;

    len = 0;

    /* Set up the configure pkt. */
    lcp_reject_pkt->code        = LCP_CODE_REJECT;
    lcp_reject_pkt->identifier  = LCP_Random_Number();

    /* Get the length */
    len         = *(lcp_pkt + LCP_LENGTH_OFFSET) << 8;
    len         =  len | *(lcp_pkt + LCP_LENGTH_OFFSET + 1);

    /* Get a pointer to the data part of the RX lcp packet. */
    lcp_pkt_ptr = &lcp_reject_pkt->data;

    /* Copy the rejected pkt over. */
    memcpy (lcp_pkt_ptr, lcp_pkt, len);

    /* Fill in the length for the lcp packet. */
    len                                            += LCP_HEADER_LENGTH;
    lcp_reject_pkt->length                          = intswap (len);
    buf_ptr->data_len = buf_ptr->mem_total_data_len = len;

    /* Set the protocol type */
    if (protocol_type == PPP_LINK_CONTROL_PROTOCOL)
        buf_ptr->mem_flags = NET_LCP;
    else
        if (protocol_type == PPP_IP_CONTROL_PROTOCOL)
            buf_ptr->mem_flags = NET_IPCP;

    /* Send the code reject packet. */
    in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device, buf_ptr);

    /* Free the buffer. */
    MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, buf_ptr);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    LCP_Ack_Sent_State                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Handles processing of an incoming LCP packet.                           */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Interpret                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Configure_Req_Check                                                 */
/*  NU_Control_Timer                                                        */
/*  NU_Reset_Timer                                                          */
/*  NU_Set_Events                                                           */
/*  LCP_Configure_Nak_Check                                                 */
/*  LCP_Send_Config_Req                                                     */
/*  LCP_Send_Terminate_Ack                                                  */
/*  MDM_Hangup                                                              */
/*  LCP_Configure_Reject_Check                                              */
/*  LCP_Send_Code_Reject                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER                  *buf_ptr    Pointer to the incoming LCP     */
/*                                          packet                          */
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
void LCP_Ack_Sent_State (NET_BUFFER *buf_ptr)
{
    STATUS status;

    /* Now see what kind of pkt it is and deal
       with it. */
    switch (*buf_ptr->data_ptr)
    {

        case LCP_CONFIGURE_REQUEST :
#ifdef DEBUG_PRINT
            _PRINT ("config_req ");
#endif
            /* Determine if the options are ok
               with our side */
            status = LCP_Configure_Req_Check (buf_ptr);

            /* Check the status to see what state we need to switch
               to. */
            if (status != NU_TRUE)  /* all options were not ok */
                lcp_state.state = REQ_SENT;

            break;

        case LCP_CONFIGURE_ACK :
#ifdef DEBUG_PRINT
            _PRINT ("config_ack ");
#endif
            /* We need to restart the counter, change states, and
               set the event stating LCP is done. */

            /* Stop the timer. */
            NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

            _lcp_num_transmissions = LCP_MAX_CONFIGURE;
            lcp_state.state = OPENED;

            /* Zero the echo counter since we are entering the opened state */
            _echo_counter = 0;

            /* Start the echo timer */
            NU_Reset_Timer (&LCP_Echo_Timer, LCP_Echo_Expire,
                   LCP_ECHO_VALUE, LCP_ECHO_VALUE, NU_ENABLE_TIMER);

            NU_Set_Events (&Buffers_Available, LCP_CONFIG_SUCCESS, NU_OR);

            break;

        case LCP_CONFIGURE_NAK :
#ifdef DEBUG_PRINT
            _PRINT ("config_nak ");
#endif
            /* Check the NAK pkt. */
            status = LCP_Configure_Nak_Check (buf_ptr);

            /* If the status is true then we are not in the loop-back
               condition. Send a configure request with the new set of
               options. */
            if (status == NU_TRUE)
            {
                /* Init the restart counter */
                _lcp_num_transmissions = LCP_MAX_CONFIGURE;

                /* Send the new config req */
                LCP_Send_Config_Req ();
            }

            break;

        case LCP_CONFIGURE_REJECT :
#ifdef DEBUG_PRINT
            _PRINT ("config reject ");
#endif
            /* We need to check what was rejected and update which options
               will be included in the next configure request sent. */
            LCP_Configure_Reject_Check (buf_ptr);

            /* Init the restart counter */
            _lcp_num_transmissions = LCP_MAX_CONFIGURE;

            /* Send the new config req */
            LCP_Send_Config_Req ();


            break;

        case LCP_TERMINATE_REQUEST :
#ifdef DEBUG_PRINT
            _PRINT ("term req ");
#endif

            /* Just send a terminate ack */

            LCP_Send_Terminate_Ack(buf_ptr, PPP_LINK_CONTROL_PROTOCOL);

            lcp_state.state = REQ_SENT;

            MDM_Hangup();

            lcp_state.state = STARTING;

            break;

        case LCP_TERMINATE_ACK :
#ifdef DEBUG_PRINT
            _PRINT ("term ack ");
#endif

            /* Do nothing */
            break;

        case LCP_CODE_REJECT :
#ifdef DEBUG_PRINT
            _PRINT ("code reject ");
#endif
            /* We need to check what was rejected and make sure we can do
               without it. */
            status = LCP_Code_Reject_Check (buf_ptr);

            /* If we can not do without it. */
            if (status != NU_TRUE)
            {
                /* Change states. */
                lcp_state.state = STOPPED;

                /* Hang the modem up. This layer finished. */
                MDM_Hangup();

                /* Change states. */
                lcp_state.state = INITIAL;
            }

            break;

        case LCP_ECHO_REQUEST :
#ifdef DEBUG_PRINT
            _PRINT ("echo req - discarded ");
#endif

            /* We don't need to do anything. Since we are not in the
               OPEN state we do not exchange echo pkt. Just drop it. */
            _silent_discards++;

            break;

        default :

#ifdef DEBUG_PRINT
            _PRINT ("unknown code ");
#endif
            /* If we get here then an un-interpretable
               packet was received, send a
               code-reject */

            LCP_Send_Code_Reject(buf_ptr, PPP_LINK_CONTROL_PROTOCOL);
            break;

    } /* switch */

}



/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    LCP_Req_Sent_State                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Handles processing of an incoming LCP packet.                           */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Interpret                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Configure_Req_Check                                                 */
/*  LCP_Configure_Nak_Check                                                 */
/*  LCP_Send_Config_Req                                                     */
/*  LCP_Send_Terminate_Ack                                                  */
/*  MDM_Hangup                                                              */
/*  LCP_Configure_Reject_Check                                              */
/*  LCP_Send_Code_Reject                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *buf_ptr        Pointer to the incoming LCP     */
/*                                          packet                          */
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
void LCP_Req_Sent_State (NET_BUFFER *buf_ptr)
{
    STATUS status;

    /* Now see what kind of pkt it is and deal
       with it. */
    switch (*buf_ptr->data_ptr)
    {

        case LCP_CONFIGURE_REQUEST :
#ifdef DEBUG_PRINT
            _PRINT ("config_req ");
#endif
            /* Determine if the options are ok
               with our side */
            status = LCP_Configure_Req_Check (buf_ptr);

            /* Check the status to see what state we need to switch
               to. */
            if (status == NU_TRUE)  /* all options were ok */
                lcp_state.state = ACK_SENT;

            break;

        case LCP_CONFIGURE_ACK :
#ifdef DEBUG_PRINT
            _PRINT ("config_ack ");
#endif
            /* All we need to do is reset the
               restart counter and change
               states */

            _lcp_num_transmissions = LCP_MAX_CONFIGURE;
            lcp_state.state = ACK_RCVD;
            break;

        case LCP_CONFIGURE_NAK :
#ifdef DEBUG_PRINT
            _PRINT ("config_nak ");
#endif
            /* Check the NAK pkt. */
            status = LCP_Configure_Nak_Check (buf_ptr);

            /* If the status is true then we are not in the loop-back
               condition. Send a configure request with the new set of
               options. */
            if (status == NU_TRUE)
            {
                /* Init the restart counter */
                _lcp_num_transmissions = LCP_MAX_CONFIGURE;

                /* Send the new config req */
                LCP_Send_Config_Req ();
            }
            break;

        case LCP_CONFIGURE_REJECT :
#ifdef DEBUG_PRINT
            _PRINT ("config_reject ");
#endif

            /* We need to check what was rejected and update what options
               will be included in the next configure request. */
            LCP_Configure_Reject_Check (buf_ptr);

            /* Init the restart counter */
            _lcp_num_transmissions = LCP_MAX_CONFIGURE;

            /* Send the new config req */
            LCP_Send_Config_Req ();

            break;

        case LCP_TERMINATE_REQUEST :
#ifdef DEBUG_PRINT
            _PRINT ("term req ");
#endif

            LCP_Send_Terminate_Ack(buf_ptr, PPP_LINK_CONTROL_PROTOCOL);

            MDM_Hangup();

            lcp_state.state = STARTING;

            break;

        case LCP_TERMINATE_ACK :
#ifdef DEBUG_PRINT
            _PRINT ("term ack ");
#endif

            /* Do nothing */

            break;

        case LCP_CODE_REJECT :
#ifdef DEBUG_PRINT
            _PRINT ("code reject ");
#endif
            /* We need to check what was rejected and make sure we can do
               without it. */
            status = LCP_Code_Reject_Check (buf_ptr);

            /* If we can not do without it. */
            if (status != NU_TRUE)
            {
                /* Change states. */
                lcp_state.state = STOPPED;

                /* Hang the modem up. This layer finished. */
                MDM_Hangup();

                /* Change states. */
                lcp_state.state = INITIAL;
            }

            break;

        case LCP_ECHO_REQUEST :
#ifdef DEBUG_PRINT
            _PRINT ("echo req - discarded ");
#endif

            /* We don't need to do anything. Since we are not in the
               OPEN state we do not exchange echo pkt. Just drop it. */
            _silent_discards++;

            break;

        default :

#ifdef DEBUG_PRINT
            _PRINT ("unknown code ");
#endif

            /* If we get here then an un-interpretable
               packet was received, send a
               code-reject */

            LCP_Send_Code_Reject(buf_ptr, PPP_LINK_CONTROL_PROTOCOL);
            break;

    } /* switch */

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*	  LCP_Ack_Rcvd_State													*/
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Handles processing of an incoming LCP packet.                           */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Interpret                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Configure_Req_Check                                                 */
/*  NU_Reset_Timer                                                          */
/*  NU_Set_Events                                                           */
/*  LCP_Send_Config_Req                                                     */
/*  LCP_Configure_Nak_Check                                                 */
/*  LCP_Send_Terminate_Ack                                                  */
/*  MDM_Hangup                                                              */
/*  LCP_Configure_Reject_Check                                              */
/*  LCP_Send_Code_Rejec                                                     */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *buf_ptr        Pointer to the incoming LCP     */
/*                                          packet                          */
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
void LCP_Ack_Rcvd_State (NET_BUFFER *buf_ptr)
{
    STATUS status;

	/* Now see what kind of pkt it is and deal
	   with it. */

    switch (*buf_ptr->data_ptr)
	{
		case LCP_CONFIGURE_REQUEST :
#ifdef DEBUG_PRINT
            _PRINT ("config_req ");
#endif
			/* Determine if the options are ok
			   with our side. This function will take care of
			   sending the ACK or NAK. */
            status = LCP_Configure_Req_Check (buf_ptr);

			/* Check the status to see what we need do. */
			if (status == NU_TRUE)	/* all options were ok */
			{
				/* Change states and notify the upper layer that
				   NCP can now have a shot. */
				lcp_state.state = OPENED;

                /* Zero the echo counter since we are entering the opened state */
                _echo_counter = 0;

                /* Start the echo timer */
                NU_Reset_Timer (&LCP_Echo_Timer, LCP_Echo_Expire,
                   LCP_ECHO_VALUE, LCP_ECHO_VALUE, NU_ENABLE_TIMER);

                NU_Set_Events (&Buffers_Available, LCP_CONFIG_SUCCESS, NU_OR);
			}

			break;

		case LCP_CONFIGURE_ACK		:
#ifdef DEBUG_PRINT
            _PRINT ("config_ack ");
#endif
            /* Send a configure request and change to that state */
            LCP_Send_Config_Req();

            /* Change states */
			lcp_state.state = REQ_SENT;

			break;

		case LCP_CONFIGURE_NAK		:
#ifdef DEBUG_PRINT
            _PRINT ("config_nak ");
#endif
            /* Check the NAK pkt. */
            status = LCP_Configure_Nak_Check (buf_ptr);

            /* If the status is true then we are not in the loop-back
               condition. Send a configure request with the new set of
               options. */
            if (status == NU_TRUE)
            {
                /* Send the new config req */
                LCP_Send_Config_Req ();

                /* Change states */
                lcp_state.state = REQ_SENT;
            }

			break;

        case LCP_CONFIGURE_REJECT   :
#ifdef DEBUG_PRINT
			_PRINT ("config reject ");
#endif

            /* We need to check what was rejected and update what options
               will be included in the next configure request sent. */
            LCP_Configure_Reject_Check (buf_ptr);

            /* Send the new config req */
            LCP_Send_Config_Req ();

            /* Change states */
            lcp_state.state = REQ_SENT;

			break;

		case LCP_TERMINATE_REQUEST	:
#ifdef DEBUG_PRINT
            _PRINT ("term req ");
#endif

            /* Change states and send an ACK */
            LCP_Send_Terminate_Ack (buf_ptr, PPP_LINK_CONTROL_PROTOCOL);

            lcp_state.state = REQ_SENT;

            MDM_Hangup();

            lcp_state.state = STARTING;

            break;

		case LCP_TERMINATE_ACK		:
#ifdef DEBUG_PRINT
            _PRINT ("term ack ");
#endif

            /* All we need to do is change states. */
			lcp_state.state = REQ_SENT;

			break;

		case LCP_CODE_REJECT		:
#ifdef DEBUG_PRINT
            _PRINT ("code rej ");
#endif
            /* We need to check what was rejected and make sure we can do
               without it. */
            status = LCP_Code_Reject_Check (buf_ptr);

            /* If we can not do without it. */
            if (status != NU_TRUE)
            {
                /* Change states. */
                lcp_state.state = STOPPED;

                /* Hang the modem up. This layer finished. */
                MDM_Hangup();

                /* Change states. */
                lcp_state.state = INITIAL;
            }


			break;

		case LCP_ECHO_REQUEST		:
#ifdef DEBUG_PRINT
            _PRINT ("echo req - discarded ");
#endif

            /* We don't need to do anything. Since we are not in the
               OPEN state we do not exchange echo pkt. Just drop it. */
            _silent_discards++;

			break;

		default :

#ifdef DEBUG_PRINT
            _PRINT ("unknown code ");
#endif

            /* If we get here then we have RX an unknown code. So
			   send a reject pkt for that code. */

            LCP_Send_Code_Reject(buf_ptr, PPP_LINK_CONTROL_PROTOCOL);

			break;
                     
	} /* switch */
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    LCP_Open_State                                                        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Handles processing of an incoming LCP packet.                           */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Interpret                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  NU_Control_Timer                                                        */
/*  LCP_Send_Config_Req                                                     */
/*  LCP_Configure_Req_Check                                                 */
/*  LCP_Configure_Nak_Check                                                 */
/*  PPP_Kill_All_Open_Sockets                                               */
/*  LCP_Send_Terminate_Ack                                                  */
/*  MDM_Hangup                                                              */
/*	LCP_Configure_Reject_Check												*/
/*  LCP_Send_Terminate_Req                                                  */
/*  NU_Reset_Timer                                                          */
/*  LCP_Send_Echo_Reply                                                     */
/*  LCP_Send_Code_Reject                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *buf_ptr        Pointer to the incoming LCP     */
/*                                          packet                          */
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
void LCP_Open_State (NET_BUFFER *buf_ptr)
{
    STATUS status;

	/* Now see what kind of pkt it is and deal
	   with it. */

    switch (*buf_ptr->data_ptr)
	{
		case LCP_CONFIGURE_REQUEST :
#ifdef DEBUG_PRINT
            _PRINT ("config_req ");
#endif
            /* This layer down action */

            /* Stop the echo timer since we are leaving the open state */
            NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

            /* Send a configure req pkt */
            LCP_Send_Config_Req();

            /* Determine if the options are ok
			   with our side. This function will take care of
			   sending the ACK or NAK. */
            status = LCP_Configure_Req_Check (buf_ptr);

			/* Check the status to see what we need do. */
			if (status == NU_TRUE)	/* all options were ok */
			{
				/* Change states and notify the upper layer that
				   NCP can now have a shot. */
                lcp_state.state = ACK_SENT;

            }
            else
                lcp_state.state = REQ_SENT;

			break;

		case LCP_CONFIGURE_ACK		:
#ifdef DEBUG_PRINT
            _PRINT ("config_ack ");
#endif
            /* this layer down action */

            /* Stop the echo timer since we are leaving the open state */
            NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

            /* Send a configure req */
            LCP_Send_Config_Req();

            /* Change states */
			lcp_state.state = REQ_SENT;

            break;

		case LCP_CONFIGURE_NAK		:
#ifdef DEBUG_PRINT
            _PRINT ("config_nak ");
#endif
            /* this layer down action */

            /* Stop the echo timer since we are leaving the open state */
            NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

            /* Check the NAK pkt. */
            status = LCP_Configure_Nak_Check (buf_ptr);

            /* If the status is true then we are not in the loop-back
               condition. Send a configure request with the new set of
               options. */
            if (status == NU_TRUE)
            {
                /* Send the new config req */
                LCP_Send_Config_Req ();
            }

            lcp_state.state = REQ_SENT;

            break;

		case LCP_CONFIGURE_REJECT	   :
#ifdef DEBUG_PRINT
			_PRINT ("config_reject ");
#endif

            /* Stop the echo timer since we are leaving the open state */
            NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

            /* We need to check what was rejected and make sure we can do
               without it. */
            LCP_Configure_Reject_Check (buf_ptr);

            /* Send the new config req */
            LCP_Send_Config_Req ();

            lcp_state.state = REQ_SENT;

            break;

		case LCP_TERMINATE_REQUEST	:
#ifdef DEBUG_PRINT
            _PRINT ("term_req ");
#endif
            /* Stop the echo timer since we are leaving the open state */
            NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

            /* this layer down */
            PPP_Kill_All_Open_Sockets(buf_ptr->mem_buf_device);

            /* Detach the IP address from this device. */
            DEV_Detach_IP_From_Device (buf_ptr->mem_buf_device->dev_net_if_name);

            /* Zero the restart counter */
            _lcp_num_transmissions = 0;

            /* Change states */
            lcp_state.state = STOPPING;

            /* Send an ack for the terminate request */
            LCP_Send_Terminate_Ack (buf_ptr, PPP_LINK_CONTROL_PROTOCOL);

            MDM_Hangup();

            lcp_state.state = STARTING;

            break;

		case LCP_TERMINATE_ACK		:

#ifdef DEBUG_PRINT
            _PRINT ("term ack ");
#endif

            /* this layer down */

            /* Stop the echo timer since we are leaving the open state */
            NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

            /* Send a configure req */
            LCP_Send_Config_Req();

            /* Change states */
			lcp_state.state = REQ_SENT;

            break;

		case LCP_CODE_REJECT		:

#ifdef DEBUG_PRINT
            _PRINT ("code reject ");
#endif
            /* We need to check what was rejected and make sure we can do
               without it. */
            status = LCP_Code_Reject_Check (buf_ptr);

            /* If we can not do without it. */
            if (status != NU_TRUE)
            {

                /* Stop the echo timer since we are leaving the open state */
                NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

                /* Close down the network */
                PPP_Kill_All_Open_Sockets (buf_ptr->mem_buf_device);

                /* Detach the IP address from this device. */
                DEV_Detach_IP_From_Device (buf_ptr->mem_buf_device->dev_net_if_name);

                _lcp_num_transmissions = LCP_MAX_TERMINATE;

                /* Send a terminate request */
                LCP_Send_Terminate_Req();

                /* Reset the restart timer */
                NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Start the timer */
                NU_Reset_Timer (&LCP_Restart_Timer, LCP_Timer_Expire,
                    LCP_TIMEOUT_VALUE, LCP_TIMEOUT_VALUE, NU_ENABLE_TIMER);

                /* Change states. */
                lcp_state.state = STOPPING;
            }

            break;

		case LCP_ECHO_REQUEST		:

#ifdef DEBUG_PRINT
            _PRINT ("echo req ");
#endif
            /* Send an echo reply. */
            LCP_Send_Echo_Reply(buf_ptr);

			break;

        case LCP_ECHO_REPLY         :
#ifdef DEBUG_PRINT
            _PRINT ("echo reply");
#endif
             /* All we need to do is zero the echo counter. Once we get one
                reply we know that the modem connection is still alive. */
             _echo_counter = 0;

             break;

		default :

#ifdef DEBUG_PRINT
            _PRINT ("unknown code ");
#endif

            /* If we get here then we have RX an unknown code. So
			   send a reject pkt for that code. */

            LCP_Send_Code_Reject(buf_ptr, PPP_LINK_CONTROL_PROTOCOL);

			break;
                     
	} /* switch */
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    LCP_Send_Echo_Reply                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Sends an echo reply packet as a response to an echo request.            */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Open_State                                                          */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  intswap                                                                 */
/*  sizeof                                                                  */
/*  dev_start                                                               */
/*  MEM_Buffer_Dequeue                                                      */
/*  MEM_Buffer_Enqueue                                                      */
/*  NU_Tcp_Log_Error                                                        */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *in_buf_ptr     Pointer to the request packet   */
/*                                          that caused this reply          */
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
void LCP_Send_Echo_Reply (NET_BUFFER *in_buf_ptr)
{
    NET_BUFFER      *buf_ptr;
    LCP_LAYER       *lcp_reply_pkt;
    uchar   HUGE    *lcp_pkt_ptr, *byte_ptr;
    uint8           len;
    uchar   HUGE    *lcp_pkt = in_buf_ptr->data_ptr;

#ifdef DEBUG_PRINT
    _PRINT ("send echo reply ");
#endif

    len = 0;

    /* Extract the magic number from the packet */

    /* Get the high bits */
    lcp_peer_options.magic_number = *(lcp_pkt + LCP_DATA_OFFSET) << 8;
    lcp_peer_options.magic_number = lcp_peer_options.magic_number |
        *(lcp_pkt + LCP_DATA_OFFSET + 1);

    /* Move them to the high side of our magic num var */
    lcp_peer_options.magic_number = lcp_peer_options.magic_number << 16;

    /* Now get the low bits */
    lcp_peer_options.magic_number = lcp_peer_options.magic_number |
        (LCP_CLEAR_HIGH & (*(lcp_pkt + LCP_DATA_OFFSET + 2) << 8));
    lcp_peer_options.magic_number = lcp_peer_options.magic_number |
        (LCP_CLEAR_HIGH & (*(lcp_pkt + LCP_DATA_OFFSET + 3)));

    /* Only send a reply if the magic number we RX is different than
       our own magic number. If they are the same then we may be talking
       to ourself. In which case not sending the reply will cause the
       link to be broken. */
    if (lcp_peer_options.magic_number != lcp_my_options.magic_number)
    {
        /* Get a buffer to put the reply packet in. */
        buf_ptr = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

        /* Make sure a buffer was available */
        if (buf_ptr == NU_NULL)
        {
            NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_SEVERE,
                          __FILE__, __LINE__);

            /* Get out */
            return;
        }


        /* Setup the data pointer. */
        buf_ptr->data_ptr = (buf_ptr->mem_parent_packet + sizeof (PPP_HEADER));
        lcp_reply_pkt     = (LCP_LAYER *) buf_ptr->data_ptr;

        /* Set up the configure pkt. */
        lcp_reply_pkt->code         = LCP_ECHO_REPLY;
        lcp_reply_pkt->identifier   = lcp_pkt [LCP_ID_OFFSET];

        /* Get a pointer to the data part of the lcp packet. */
        lcp_pkt_ptr                 = &lcp_reply_pkt->data;

        /* Fill in our magic number */
        byte_ptr = (uchar*)&lcp_my_options.magic_number;

#ifdef SWAPPING
        lcp_pkt_ptr[0] = byte_ptr[3];
        lcp_pkt_ptr[1] = byte_ptr[2];
        lcp_pkt_ptr[2] = byte_ptr[1];
        lcp_pkt_ptr[3] = byte_ptr[0];
#else
        lcp_pkt_ptr[0] = byte_ptr[0];
        lcp_pkt_ptr[1] = byte_ptr[1];
        lcp_pkt_ptr[2] = byte_ptr[2];
        lcp_pkt_ptr[3] = byte_ptr[3];
#endif

        /* Update the length */
        len += LCP_MAGIC_NUMBER_LENGTH;

        /* Fill in the length for the lcp packet. */
        len                     += LCP_HEADER_LENGTH;
        lcp_reply_pkt->length    = intswap (len);

        /* Set the packet type for this buffer. */
        buf_ptr->mem_flags = NET_LCP;

        /* Set the length of the data contained in the buffer. */
        buf_ptr->data_len = buf_ptr->mem_total_data_len = len;

        /* Send the configure reply packet. */
        in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device, buf_ptr);

        /* Release the buffer space */
        MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, buf_ptr);
    }

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    LCP_Closing_State                                                     */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Handles processing of an incoming LCP packet.                           */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Interpret                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Send_Terminate_Ack                                                  */
/*  MDM_Hangup                                                              */
/*  NU_Control_Timer                                                        */
/*	LCP_Configure_Reject_Check												*/
/*  LCP_Send_Code_Reject                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *buf_ptr        Pointer to the incoming LCP     */
/*                                          packet                          */
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
void LCP_Closing_State (NET_BUFFER *buf_ptr)
{
    STATUS status;

	/* Now see what kind of pkt it is and deal
	   with it. */

    switch (*buf_ptr->data_ptr)
	{
        case LCP_CONFIGURE_REQUEST  : /* Do not need to do anything for */

#ifdef DEBUG_PRINT
            _PRINT ("config_req ");
#endif
            _silent_discards++;
            break;

        case LCP_CONFIGURE_ACK      : 

#ifdef DEBUG_PRINT
            _PRINT ("config_ack ");
#endif
            _silent_discards++;
            break;

        case LCP_CONFIGURE_NAK      : /* these types of pkts. */

#ifdef DEBUG_PRINT
            _PRINT ("config_nak ");
#endif

            _silent_discards++;
            break;

		case LCP_CONFIGURE_REJECT	:

#ifdef DEBUG_PRINT
			_PRINT ("config_reject ");
#endif

            _silent_discards++;
            break;

		case LCP_TERMINATE_REQUEST	:

#ifdef DEBUG_PRINT
            _PRINT ("term_req ");
#endif

            /* Send an ACK */
            LCP_Send_Terminate_Ack(buf_ptr, PPP_LINK_CONTROL_PROTOCOL);

            MDM_Hangup();

            lcp_state.state = INITIAL;

			break;

		case LCP_TERMINATE_ACK		:

#ifdef DEBUG_PRINT
            _PRINT ("term_ack ");
#endif

            /* Change states */
            lcp_state.state = CLOSED;

            /* Reset the restart timer */
            NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

            /* Hangup the modem */
            MDM_Hangup();

            /* Change states */
            lcp_state.state = INITIAL;

			break;

		case LCP_CODE_REJECT		:

#ifdef DEBUG_PRINT
            _PRINT ("code reject ");
#endif
            /* We need to check what was rejected and make sure we can do
               without it. */
            status = LCP_Code_Reject_Check (buf_ptr);

            /* If we can not do without it. */
            if (status != NU_TRUE)
            {
                /* Change states. */
                lcp_state.state = CLOSED;

                /* Hang the modem up. This layer finished. */
                MDM_Hangup();

                /* Change states. */
                lcp_state.state = INITIAL;
            }

			break;

		case LCP_ECHO_REQUEST		:

#ifdef DEBUG_PRINT
            _PRINT ("echo req - discarded ");
#endif

            /* We don't need to do anything. Since we are not in the
               OPEN state we do not exchange echo pkt. Just drop it. */
            _silent_discards++;

			break;

		default :

#ifdef DEBUG_PRINT
            _PRINT ("unknown code ");
#endif
            /* If we get here then we have RX an unknown code. So
			   send a reject pkt for that code. */

            LCP_Send_Code_Reject(buf_ptr, PPP_LINK_CONTROL_PROTOCOL);

			break;
                     
	} /* switch */
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    LCP_Configure_Nak_Check                                               */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function checks all naked options in a configure nak packet. It    */
/* will change the configuration options accordingly so that next time a    */
/* configure request is sent it will contain the new options. It also checks*/
/* the magic number to make sure there is not a looped back condition       */
/* present. False is returned if the magic numbers match.  True is returned */
/* is the magic numbers are different.                                      */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Ack_Sent_State                                                      */
/*  LCP_Req_Sent_State                                                      */
/*  LCP_Ack_Rcvd_State                                                      */
/*  LCP_Open_State                                                          */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  sizeof                                                                  */
/*  MEM_Buffer_Dequeue                                                      */
/*  NU_Tcp_Log_Error                                                        */
/*  intswap                                                                 */
/*  longswap                                                                */
/*  MEM_Buffer_Enqueue                                                      */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *in_buf_ptr     Pointer to the incoming LCP     */
/*                                          packet                          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS  :   was the magic number ok                                     */
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
STATUS LCP_Configure_Nak_Check (NET_BUFFER *in_buf_ptr)
{
    NET_BUFFER          *buf_ptr;
    LCP_LAYER           *lcp_nak_pkt;
    uchar   HUGE        *lcp_nak_ptr, HUGE *lcp_pkt_ptr, *byte_ptr;
    uint8               identifier, magic_number_ok, len;
    uint16              length, nak_length;
    uchar   HUGE        *lcp_pkt = in_buf_ptr->data_ptr;

    /* If it is used we will assume that the link is not looped-back. */
    magic_number_ok = NU_TRUE;

    /* Get the ID of this pkt */
    identifier = lcp_pkt[LCP_ID_OFFSET];

    /* The id must match that of the last sent config pkt. */
    if (identifier == lcp_state.identifier)
    {

        /* Allocate a buffer for the NAK packet. */
        buf_ptr = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

        /* Make sure a buffer was available */
        if (buf_ptr == NU_NULL)
        {
            NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_SEVERE,
                          __FILE__, __LINE__);

            /* Get out */
            return (NU_FALSE);
        }

        /* Set the data pointer. */
        buf_ptr->data_ptr = (buf_ptr->mem_parent_packet + sizeof (PPP_HEADER));
        lcp_nak_pkt       = (LCP_LAYER *)buf_ptr->data_ptr;

       /* Point to the nak packet data in case we have to
          nak the magic number. */
        lcp_nak_ptr = &lcp_nak_pkt->data;
        nak_length  = LCP_HEADER_LENGTH + LCP_MAGIC_NUMBER_LENGTH;
        len = 0;

        /* Put the LCP structure on the packet and get the length */
        lcp_pkt_ptr = lcp_pkt;
        length      = *(lcp_pkt_ptr + LCP_LENGTH_OFFSET) << 8;
        length      =  length | *(lcp_pkt_ptr + LCP_LENGTH_OFFSET + 1);

        /* Now remove the header */
        length     -= LCP_HEADER_LENGTH;

        /* Get a pointer to the data inside the LCP pkt */
        lcp_pkt_ptr = lcp_pkt_ptr + LCP_DATA_OFFSET;

        while (length > 0)
        {
            /* Check out what option has been NAKed */
            switch (lcp_pkt_ptr[0])
            {
                case LCP_MAX_RX_UNIT            :

                    /* This option can not be changed */

                    /* Update the pkt pointer and the packet size */
                    lcp_pkt_ptr += LCP_MRU_LENGTH;
                    length -= LCP_MRU_LENGTH;

                    break;

                case LCP_ASYNC_CONTROL_CHAR_MAP :

                    /* What this means is that our peer knows of some
                       chars that should be mapped when we RX. Put
                       them in the options structure. */

                    /* Get the high bits */
                    lcp_my_options.accm = *(lcp_pkt_ptr + LCP_CONFIG_OPTS) << 8;
                    lcp_my_options.accm = lcp_my_options.accm |
                               *(lcp_pkt_ptr + LCP_CONFIG_OPTS + 1);

                    /* Move them to the high side of our ACCM var */
                    lcp_my_options.accm = lcp_my_options.accm << 16;

                    /* Now get the low bits */
                    lcp_my_options.accm = lcp_my_options.accm |
                               (*(lcp_pkt_ptr + LCP_CONFIG_OPTS + 2) << 8);
                    lcp_my_options.accm = lcp_my_options.accm |
                               *(lcp_pkt_ptr + LCP_CONFIG_OPTS + 3);

                    /* Update the pkt pointer and the packet size */
                    lcp_pkt_ptr += LCP_ACCM_LENGTH;
                    length -= LCP_ACCM_LENGTH;
                                     
                    break;

                case LCP_AUTHENTICATION_PROTOCOL :

                    /* Check to see which one the peer wants to use. */

                    /* Is it chap? */
                    if ((lcp_pkt_ptr [LCP_CONFIG_OPTS] ==
                        ((PPP_CHAP_PROTOCOL >> 8) & PPP_BIT_MASK))
                        && ((lcp_pkt_ptr [LCP_CONFIG_OPTS + 1] ==
                        (PPP_CHAP_PROTOCOL & PPP_BIT_MASK))))

                        /* Yes it is, make sure it is with MD5? */
                        if (lcp_pkt_ptr [LCP_DATA_OFFSET] != LCP_CHAP_MD5)
                        {
                            /* At this point the peer is using CHAP but not with
                               MD5. We will try PAP. */

                            lcp_my_options.authentication_protocol =
                                                            PPP_PAP_PROTOCOL;
                        }
                        else
                        {
                            /* If we make it here then our peer wants to use
                               MD5-CHAP, which is fine with us. Put this in
                               in the options structure. */
                            lcp_my_options.authentication_protocol =
                                            PPP_CHAP_PROTOCOL;
                        }
                    else
                        /* The only other one we support if PAP. So use it. */
                        {
                            /* This is fine. Put it in the options structure. */
                            lcp_my_options.authentication_protocol = PPP_PAP_PROTOCOL;
                        }

                    length -= lcp_pkt_ptr [LCP_CONFIG_LENGTH_OFFSET];
                    lcp_pkt_ptr += lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                    break;

                case LCP_QUALITY_PROTOCOL       :

                    /* We don't support this protocol. */

                    length -= lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];
                    lcp_pkt_ptr += lcp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                    break;

                case LCP_MAGIC_NUMBER           :

                    /* Being NAKed with this option means that we might
                       be in a looped-back condition. Check to see if
                       they are the same, this could be a response to a
                       previous magic number conflict. */

                    /* Extract the magic number from the packet */

                    /* Get the high bits */
                    lcp_peer_options.magic_number = *(lcp_pkt_ptr + LCP_CONFIG_OPTS) << 8;
                    lcp_peer_options.magic_number = lcp_peer_options.magic_number |
                               *(lcp_pkt_ptr + LCP_CONFIG_OPTS + 1);

                    /* Move them to the high side of our magic num var */
                    lcp_peer_options.magic_number = lcp_peer_options.magic_number << 16;

                    /* Now get the low bits */
                    lcp_peer_options.magic_number = lcp_peer_options.magic_number |
                        (LCP_CLEAR_HIGH & (*(lcp_pkt_ptr + LCP_CONFIG_OPTS + 2) << 8));
                    lcp_peer_options.magic_number = lcp_peer_options.magic_number |
                        (LCP_CLEAR_HIGH & (*(lcp_pkt_ptr + LCP_CONFIG_OPTS + 3)));

                    /* Make sure they are not the same. Note :
                       lcp_my_options.magic_number is stored in the endien order
                       of the processor. Therefore the longswap on it is needed.
                       lcp_peer_options.magic_number is stored from the incomming
                       pkt and will always be in the correct order, so no swapping
                       is needed. */
                    if (lcp_peer_options.magic_number ==
                                        longswap (lcp_my_options.magic_number))
                    {
                        /* Set the ok flag to false, this will stop the caller
                           from sending a configure request. */
                        magic_number_ok = NU_FALSE;

                        /* Since they are the same we need to get a new number
                           and NAK with it. */

                        lcp_nak_ptr[len++]          = LCP_MAGIC_NUMBER;
                        lcp_nak_ptr[len++]          = LCP_MAGIC_NUMBER_LENGTH;
                        lcp_my_options.magic_number = LCP_Random_Number32();
                        byte_ptr                    = (uchar *)&lcp_my_options.magic_number;

#ifdef SWAPPING
                        lcp_nak_ptr[len++]  = byte_ptr[3];
                        lcp_nak_ptr[len++]  = byte_ptr[2];
                        lcp_nak_ptr[len++]  = byte_ptr[1];
                        lcp_nak_ptr[len++]  = byte_ptr[0];
#else
                        lcp_nak_ptr[len++]  = byte_ptr[0];
                        lcp_nak_ptr[len++]  = byte_ptr[1];
                        lcp_nak_ptr[len++]  = byte_ptr[2];
                        lcp_nak_ptr[len++]  = byte_ptr[3];
#endif
                    }

                    /* Update the pkt pointer and the packet size */
                    lcp_pkt_ptr += LCP_MAGIC_NUMBER_LENGTH;
                    length -= LCP_MAGIC_NUMBER_LENGTH;

                    break;

                case LCP_PROTOCOL_FIELD_COMPRESS :

                    /* This option can not be NAKed, but in case the peer's
                       implementation is not correct we will update the
                       pkt pointer and the packet size so as not to
                       cause problems. */
                    lcp_pkt_ptr += LCP_PROTOCOL_COMPRESS_LENGTH;
                    length -= LCP_PROTOCOL_COMPRESS_LENGTH;

                    break;

                case LCP_ADDRESS_FIELD_COMPRESS :

                    /* This option can not be NAKed, but in case the peer's
                       implementation is not correct we will update the
                       pkt pointer and the packet size so as not to
                       cause problems. */
                    lcp_pkt_ptr += LCP_ADDRESS_COMPRESS_LENGTH;
                    length -= LCP_ADDRESS_COMPRESS_LENGTH;

                    break;

            } /* switch */

        } /* while */

        if (!magic_number_ok)
        {

#ifdef DEBUG_PRINT
            _PRINT ("naking ");
#endif

            /* Put in the pkt indentifier and type */
            lcp_nak_pkt->identifier = identifier;
            lcp_nak_pkt->code       = LCP_CONFIGURE_NAK;
            lcp_nak_pkt->length     = intswap (nak_length);

            /* Setup the packet type and length */
            buf_ptr->mem_flags      = NET_LCP;
            buf_ptr->data_len = buf_ptr->mem_total_data_len = nak_length;

            /* Send the pkt */
            in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device, buf_ptr);
        }

        /* Give the buffer back. */
        MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, buf_ptr);

    } /* if */
    else
    {

#ifdef DEBUG_PRINT
        _PRINT ("discarded - invalid ID ");
#endif

        /* This reject is not for the last configure pkt we sent. So
           drop it. */
        _silent_discards++;
    }

    return (magic_number_ok);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    LCP_Stopping_State                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Handles processing of an incoming LCP packet.                           */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  LCP_Interpret                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Send_Terminate_Ack                                                  */
/*  MDM_Hangup                                                              */
/*  NU_Control_Timer                                                        */
/*  LCP_Code_Reject_Check                                                   */
/*  LCP_Send_Code_Reject                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *in_buf_ptr     Pointer to the incoming LCP     */
/*                                          packet                          */
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
void LCP_Stopping_State (NET_BUFFER *buf_ptr)
{
    STATUS status;

	/* Now see what kind of pkt it is and deal
	   with it. */

    switch (*buf_ptr->data_ptr)
	{
        case LCP_CONFIGURE_REQUEST  : /* Do not need to do anything for */

#ifdef DEBUG_PRINT
            _PRINT ("config_req ");
#endif
            _silent_discards++;
            break;

        case LCP_CONFIGURE_ACK      : 

#ifdef DEBUG_PRINT
            _PRINT ("config_ack ");
#endif
            _silent_discards++;
            break;

        case LCP_CONFIGURE_NAK      : /* these types of pkts. */

#ifdef DEBUG_PRINT
            _PRINT ("config_nak ");
#endif

            _silent_discards++;
            break;

        case LCP_CONFIGURE_REJECT   :

#ifdef DEBUG_PRINT
			_PRINT ("config_reject ");
#endif

            _silent_discards++;
            break;

		case LCP_TERMINATE_REQUEST	:

#ifdef DEBUG_PRINT
            _PRINT ("term_req ");
#endif

            /* Send an ACK */
            LCP_Send_Terminate_Ack(buf_ptr, PPP_LINK_CONTROL_PROTOCOL);

            MDM_Hangup();

            lcp_state.state = STARTING;

			break;

		case LCP_TERMINATE_ACK		:

#ifdef DEBUG_PRINT
            _PRINT ("term_ack ");
#endif

            /* Change states */
            lcp_state.state = STOPPED;

            /* Reset the restart timer */
            NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

            /* Hangup the modem - this layer finished */
            MDM_Hangup();

            /* Change states */
            lcp_state.state = STARTING;

			break;

		case LCP_CODE_REJECT		:

#ifdef DEBUG_PRINT
            _PRINT ("code reject ");
#endif
            /* We need to check what was rejected and make sure we can do
               without it. */
            status = LCP_Code_Reject_Check (buf_ptr);

            /* If we can not do without it. */
            if (status != NU_TRUE)
            {
                /* Change states. */
                lcp_state.state = STOPPED;

                /* Hang the modem up. This layer finished. */
                MDM_Hangup();

                /* Change states. */
                lcp_state.state = INITIAL;
            }


			break;

		case LCP_ECHO_REQUEST		:

#ifdef DEBUG_PRINT
            _PRINT ("echo req - discarded ");
#endif

            /* We don't need to do anything. Since we are not in the
               OPEN state we do not exchange echo pkt. Just drop it. */
            _silent_discards++;

			break;

		default :

#ifdef DEBUG_PRINT
            _PRINT ("unknown code ");
#endif
            /* If we get here then we have RX an unknown code. So
			   send a reject pkt for that code. */

            LCP_Send_Code_Reject(buf_ptr, PPP_LINK_CONTROL_PROTOCOL);

			break;
                     
	} /* switch */

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    LCP_Echo_Expire                                                       */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This is expiration routine for the echo timer. All it does it set a     */
/* NET timer event that will cause an echo request packet to be sent.       */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/* Nucleus PLUS                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  UTL_Timerset                                                            */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  UNSIGNED                dev_ptr         Address of the device stucture  */
/*                                          for this echo request to be     */
/*                                          sent over.                      */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  none                                                                    */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        05/06/98      Created as part of integration    */
/*                                          with NET 4.0.                   */
/*                                                                          */
/****************************************************************************/
VOID LCP_Echo_Expire (UNSIGNED dev_ptr)
{
    /* Tell the event dispatcher to send an echo request. */
    UTL_Timerset (LCP_ECHO_REQ, dev_ptr, 0, 0);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    LCP_Send_Echo_Req                                                     */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This is expiration for the echo timer. If we have alread sent           */
/* LCP_MAX_ECHO echo requests then the link will be considered to be down   */
/* and the upper layers will be notified. If not then an echo request will  */
/* be sent.                                                                 */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/* Nucleus PLUS                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Random_Number                                                       */
/*  intswap                                                                 */
/*  sizeof                                                                  */
/*  dev_start                                                               */
/*  NU_Control_Timer                                                        */
/*  PPP_Kill_All_Open_Sockets                                               */
/*  MDM_Hangup                                                              */
/*  MEM_Buffer_Dequeue                                                      */
/*  MEM_Buffer_Enqueue                                                      */
/*  NU_Tcp_Log_Error                                                        */
/*  DEV_Detach_IP_From_Device                                               */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  none                                                                    */
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
/*  Uriah T. Pollock        11/18/97      Fixed a bug where this timer      */
/*                                         expiration routine would self    */
/*                                         suspend, some of the routines it */
/*                                         called did the actual suspension.*/
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
VOID LCP_Send_Echo_Req (DV_DEVICE_ENTRY *dev_ptr)
{
    NET_BUFFER  *buf_ptr;
    LCP_LAYER   *lcp_req_pkt;
    uchar       *lcp_pkt_ptr, *byte_ptr;
    uint8       len;

#ifdef DEBUG_PRINT
    _PRINT ("\nsend echo req ");
#endif

    /* Make sure that we have been getting a reply */
    if (_echo_counter < LCP_MAX_ECHO)
    {
        len = 0;

        /* Get a buffer for the echo packet. */
        buf_ptr = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

        /* Make sure a buffer was available */
        if (buf_ptr == NU_NULL)
        {
            NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_SEVERE,
                          __FILE__, __LINE__);

            /* Get out */
            return;
        }

        /* Setup the data pointers */
        buf_ptr->data_ptr = (buf_ptr->mem_parent_packet + sizeof (PPP_HEADER));

        /* Point the LCP structure to the data part of the buffer. */
        lcp_req_pkt = (LCP_LAYER *) buf_ptr->data_ptr;

        /* Set up the configure pkt. */
        lcp_req_pkt->code       = LCP_ECHO_REQUEST;
        lcp_req_pkt->identifier = LCP_Random_Number();

        /* Get a pointer to the data part of the lcp packet. */
        lcp_pkt_ptr             = &lcp_req_pkt->data;

        /* Fill in our magic number */
        byte_ptr = (uchar*)&lcp_my_options.magic_number;

#ifdef SWAPPING
        lcp_pkt_ptr[0] = byte_ptr[3];
        lcp_pkt_ptr[1] = byte_ptr[2];
        lcp_pkt_ptr[2] = byte_ptr[1];
        lcp_pkt_ptr[3] = byte_ptr[0];
#else
        lcp_pkt_ptr[0] = byte_ptr[0];
        lcp_pkt_ptr[1] = byte_ptr[1];
        lcp_pkt_ptr[2] = byte_ptr[2];
        lcp_pkt_ptr[3] = byte_ptr[3];
#endif

        /* Update the length */
        len += LCP_MAGIC_NUMBER_LENGTH;

        /* Fill in the length for the lcp packet. */
        len                     += LCP_HEADER_LENGTH;
        lcp_req_pkt->length      = intswap (len);

        /* Set the packet type and length */
        buf_ptr->mem_flags = NET_LCP;
        buf_ptr->data_len = buf_ptr->mem_total_data_len = len;

        /* Send the echo req packet. */
        dev_ptr->dev_start (dev_ptr, buf_ptr);

        /* Return the buffer to the buffer pool. */
        MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, buf_ptr);

        /* Bump the number of echo req sent */
        _echo_counter++;
    }
    else
    {
        /* Since we are not getting a reply then we must have lost the modem
           connection. Close everything down. */

        /* Stop the echo timer since we are leaving the open state */
        NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

        PPP_Kill_All_Open_Sockets(dev_ptr);

        /* Detach the IP address from this device. */
        DEV_Detach_IP_From_Device (buf_ptr->mem_buf_device->dev_net_if_name);

        /* Just in case */
        MDM_Hangup();

        /* Change states */
        lcp_state.state = INITIAL;
    }
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    LCP_Send_Protocol_Reject                                              */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Send a protocol reject packet to our peer. This is in response to       */
/* having recevied a packet destined for some unknown network protocol.     */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/* PPP_Input                                                                */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *buf_ptr        Pointer to the unknown protocol */
/*                                          packet.                         */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  none                                                                    */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/* Uriah T. Pollock         05/15/98        Added this function as per a    */
/*                                          beta testers recommendations.   */
/*                                          This function is required by    */
/*                                          RFC1661.                        */
/*                                                                          */
/****************************************************************************/
VOID LCP_Send_Protocol_Reject (NET_BUFFER *buf_ptr)
{

#ifdef DEBUG_PRINT
    _PRINT ("\nsend protocol reject ");
#endif


    /* Since a protocol reject packet is just the unknown packet
       with a protocol reject header, we can reuse this buffer.

       - LCP header (code, ID, length)
       - Protocol Field of the RX packet, or the rejected protocol
       - Data of the RX packet, or the rejected information

       Note that every RX PPP packet is offset into the buffer 16 bytes.
       This means that we still have enough room at the top of this
       buffer to add the LCP header.

    */

    /* First we need to find out what type of comprssion, if any, was used
       on this packet. This will determine how far we have to backup the
       data pointer in order to add the LCP header. */

    /* Are there address and control fields present. If so these can be over
       written. */
    if ((buf_ptr->data_ptr[0] == PPP_HDLC_ADDRESS) &&
            (buf_ptr->data_ptr[1] == PPP_HDLC_CONTROL))
    {

        /* They are present. Push the data pointer past them. */
        buf_ptr->data_ptr += 2;

        /* Adjust the lengths */
        buf_ptr->data_len -= 2;
        buf_ptr->mem_total_data_len -= 2;
    }

    /* Now that the RX unknown protocol packet is set up correctly. Back off
       the data ptr to make room for the LCP header. */
    buf_ptr->data_ptr -= LCP_HEADER_LENGTH;

    /* Add the header to the lengths */
    buf_ptr->data_len += LCP_HEADER_LENGTH;
    buf_ptr->mem_total_data_len += LCP_HEADER_LENGTH;

    /* Fill in the LCP header. We do this byte by byte to avoid alignment
       problems on CPU's that required word alignment. */
    buf_ptr->data_ptr[0]    = LCP_PROTOCOL_REJECT;  /* code field */
    buf_ptr->data_ptr[1]    = LCP_Random_Number();  /* ID field   */

    /* Make sure we do not send more than the peers MRU. */
    if (buf_ptr->mem_total_data_len > lcp_peer_options.max_rx_unit)
        buf_ptr->mem_total_data_len = lcp_peer_options.max_rx_unit;

    /* length field - 2 bytes - MSB */
    buf_ptr->data_ptr[2]    = (uint8) (buf_ptr->mem_total_data_len >> 8);

    /* length field - 2 bytes - LSB */
    buf_ptr->data_ptr[3]    = (uint8) buf_ptr->mem_total_data_len;


    /* Set up the type of buffer. */
    buf_ptr->mem_flags = NET_LCP;

    /* Send the protocl reject packet. */
    buf_ptr->mem_buf_device->dev_start (buf_ptr->mem_buf_device, buf_ptr);

}
