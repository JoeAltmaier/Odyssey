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
/*    NCP.C                                                  2.0            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains the network control protocols used by PPP to       */
/*    negotiate network settings to be used over the link. This             */
/*    implementation only contains the internet protocol control protocol   */
/*    (IPCP). No other network protocols are suppported.                    */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah Pollock                                                         */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  NU_TIMER                            NCP_Restart_Timer                   */
/*  struct          lcp_state_struct    ncp_state                           */
/*  int8                                _ncp_num_transmissions              */
/*  uint8                               ncp_mode                            */
/*  static          uint8      _assigned_peer_ip_address[IP_ADDRESS_LENGTH] */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*  PPP_NCP_IP_Interpret                                                    */
/*  NCP_IP_Send_Config_Req                                                  */
/*  NCP_IP_Req_Sent_State                                                   */
/*  NCP_IP_Ack_Rcvd_State                                                   */
/*  NCP_Timer_Expire                                                        */
/*  NCP_Change_IP_Mode                                                      */
/*  NCP_Configure_Req_Check                                                 */
/*  NCP_Configure_Nak_Check                                                 */
/*  NCP_IP_Ack_Sent_State                                                   */
/*  NCP_IP_Opened_State                                                     */
/*  NCP_IP_Code_Reject_Check                                                */
/*  NCP_IP_Stopping_State                                                   */
/*  NCP_Set_Client_IP_Address                                               */
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
/*  Uriah T. Pollock        11/18/97      Fixed a bug in NCP_Timer_Expire   */
/*                                          - spr 375                       */
/*  Uriah T. Pollock        11/18/97      Removed standard library headers  */
/*                                          from being included - spr 376   */
/*  Uriah T. Pollock        11/18/97      Updated PPP to version 1.1        */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
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

/* Declare globals for NCP negotiation */
NU_TIMER                            NCP_Restart_Timer;
struct          lcp_state_struct    ncp_state;
int8                                _ncp_num_transmissions;
uint8                               ncp_mode;
static          uint8               _assigned_peer_ip_address[IP_ADDRESS_LENGTH];

/* Import externals defined in other modules */
extern NU_TIMER                     LCP_Echo_Timer;
extern struct   lcp_state_struct    lcp_state;
extern uint16                       _silent_discards;
extern NU_TIMER                     LCP_Restart_Timer;
extern uint8                        _lcp_num_transmissions;

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_NCP_IP_Interpret                                                  */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Based on the current state of PPP, this function passes the incoming    */
/* packet to the state function. After the packet has been processed it is  */
/* deallocated from the buffer list.                                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  demux                                                                   */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  MDM_Hangup                                                              */
/*  NCP_IP_Stopping_State                                                   */
/*  NCP_IP_Req_Sent_State                                                   */
/*  NCP_Ack_Rcvd_State                                                      */
/*  NCP_Ack_Sent_State                                                      */
/*  NCP_Opened_State                                                        */
/*  LCP_IP_Send_Code_Reject                                                 */
/*  MEM_Buffer_Chain_Free                                                   */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER                  *buf_ptr    Pointer to the incoming NCP     */
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
void PPP_NCP_IP_Interpret (NET_BUFFER *buf_ptr)
{

#ifdef DEBUG_PRINT
    _PRINT ("\nncp interpret ");
#endif

    /* LCP must be in the opened state for NCP negotiation */
    if (lcp_state.state != OPENED)
    {
#ifdef DEBUG_PRINT
        _PRINT ("discarded - lcp not open ");
#endif
        _silent_discards++;
    }
    else
    {
        /* What we do will depends on the state of the NCP */
        switch (ncp_state.state)
        {
            case INITIAL    :

#ifdef DEBUG_PRINT
                _PRINT ("initial ");
#endif
                /* This state not used. Execution will never
                   make it here. */

                break;

            case STARTING   :

#ifdef DEBUG_PRINT
                _PRINT ("starting ");
#endif

                /* This state is ONLY entered after the
                   modem has been hung up. If we ever
                   reach this point then there must be a problem
                   with the modem. So we will try to hang it
                   up again and ignore this packet. */

                MDM_Hangup();

                break;

            case CLOSED     :

#ifdef DEBUG_PRINT
                _PRINT ("closed ");
#endif

                /* This state not used. Execution will never
                   make it here. */

                break;

            case STOPPED    :

#ifdef DEBUG_PRINT
                _PRINT ("stopped ");
#endif

                /* This state not used. Execution will never
                   make it here. */

                break;

            case CLOSING    :

#ifdef DEBUG_PRINT
                _PRINT ("closing ");
#endif
                /* This state not used. Execution will never
                   make it here. */

                break;

            case STOPPING   :

#ifdef DEBUG_PRINT
                _PRINT ("stopping ");
#endif

                /* Call the routine to handle this state */
                NCP_IP_Stopping_State (buf_ptr);

                break;

            case REQ_SENT   :

#ifdef DEBUG_PRINT
                _PRINT ("req_sent ");
#endif      

                /* Call the routine to handle this state */
                NCP_IP_Req_Sent_State (buf_ptr);
                break;

            case ACK_RCVD   :

#ifdef DEBUG_PRINT
                _PRINT ("ack_rcvd ");
#endif

                /* Call the routine to handle this state */
                NCP_IP_Ack_Rcvd_State (buf_ptr);
                break;

            case ACK_SENT   :

#ifdef DEBUG_PRINT
                _PRINT ("ack_sent ");
#endif

                /* Call the routine to handle this state */
                NCP_IP_Ack_Sent_State (buf_ptr);
                break;

            case OPENED     :

#ifdef DEBUG_PRINT
                _PRINT ("opened ");
#endif

                /* Call the routine to handle this state */
                NCP_IP_Opened_State (buf_ptr);
                break;
     
            default :

#ifdef DEBUG_PRINT
                _PRINT ("default ");
#endif

                /* Send a code reject pkt. */
                LCP_Send_Code_Reject (buf_ptr, PPP_IP_CONTROL_PROTOCOL);

                break;
        } /* switch */

    } /* else */
    
    /* Release the buffer space */
    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    NCP_IP_Send_Config_Req                                                */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Sends a configure request packet to the peer. This packet contains      */
/* either an IP address that is being assigned to the peer or a IP address  */
/* of zero, stating that we need to be assigned an IP address.              */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_Lower_Layer_Up                                                      */
/*  NCP_IP_Req_Sent_State                                                   */
/*  NCP_IP_Ack_Rcvd_State                                                   */
/*  NU_EventsDispatcher                                                     */
/*  NCP_IP_Ack_Sent_State                                                   */
/*  NCP_IP_Opened_State                                                     */
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
/*  uchar                   ip_address [4]  IP address to be assigned to the*/
/*                                          peer. If we are in client mode  */
/*                                          this will be zero, stating that */
/*                                          we need to be assigned an ip    */
/*                                          address.                        */
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
void NCP_IP_Send_Config_Req (uchar ip_address[4])
{

    NET_BUFFER          *buf_ptr;
    LCP_LAYER   HUGE    *ncp_pkt;
    uchar       HUGE    *ncp_pkt_ptr;
    uint16              len;

#ifdef DEBUG_PRINT
    _PRINT ("send config req ");
#endif

    len = 0;

    /* Is there a negotiation packet that can be reused? */
    if (ncp_state.negotiation_pkt != NU_NULL)
        buf_ptr = ncp_state.negotiation_pkt;
    else
        /* Allocate a buffer for the LCP packet. */
        ncp_state.negotiation_pkt = buf_ptr = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

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
    ncp_pkt = (LCP_LAYER *) buf_ptr->data_ptr;

    /* Set up the configure pkt. */
    ncp_pkt->code        = LCP_CONFIGURE_REQUEST;
    ncp_pkt->identifier  = LCP_Random_Number();

    /* Get a pointer to the data part of the NCP packet. */
    ncp_pkt_ptr         = &ncp_pkt->data;

    /* NCP IP option */
    ncp_pkt_ptr[len++]  = NCP_IP_ADDRESS;
    ncp_pkt_ptr[len++]  = NCP_IP_ADDRESS_LENGTH;

    /* If we are in client mode we will ask for an IP address by sending
       all zeros. */

    /* Fill in the address */
    ncp_pkt_ptr[len++] = ip_address[0];
    ncp_pkt_ptr[len++] = ip_address[1];
    ncp_pkt_ptr[len++] = ip_address[2];
    ncp_pkt_ptr[len++] = ip_address[3];

    /* Fill in the length for the ncp packet. */
    len                     += LCP_HEADER_LENGTH;
    ncp_pkt->length          = intswap(len);

    /* Set the length for the buffer. */
    buf_ptr->data_len = buf_ptr->mem_total_data_len = len;

    /* Store this identifier for checking receive pkts. */
    ncp_state.identifier     = ncp_pkt->identifier;

    /* Set the packet type. */
    buf_ptr->mem_flags = NET_IPCP;

    /* Ok, now send the pkt and let the automaton begin. */

    /* Bump the transmission count. */
    _ncp_num_transmissions--;

    /* Send the configure request packet. */
    PPP_TX_Packet (buf_ptr->mem_buf_device, buf_ptr);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    NCP_IP_Req_Sent_State                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This functions handles processing of an incoming NCP packet.            */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_NCP_IP_Interpret                                                    */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  NCP_Configure_Req_Check                                                 */
/*  NCP_Configure_Nak_Check                                                 */
/*  NCP_IP_Send_Config_Req                                                  */
/*  LCP_Send_Terminate_Ack                                                  */
/*  NCP_IP_Code_Reject_Check                                                */
/*  MDM_Hangup                                                              */
/*  LCP_Send_Code_Reject                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER                  *buf_ptr    Pointer to the incoming NCP     */
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
void NCP_IP_Req_Sent_State (NET_BUFFER *buf_ptr)
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
            status = NCP_Configure_Req_Check (buf_ptr);

            /* Check the status to see what state we need to switch
               to. */
            if (status == NU_TRUE)  /* all options were ok */
                ncp_state.state = ACK_SENT;

            break;

        case LCP_CONFIGURE_ACK :
#ifdef DEBUG_PRINT
            _PRINT ("config_ack ");
#endif
            /* All we need to do is reset the
               restart counter and change
               states */

            _ncp_num_transmissions = LCP_MAX_CONFIGURE;
            ncp_state.state = ACK_RCVD;
            break;

        case LCP_CONFIGURE_NAK :
#ifdef DEBUG_PRINT
            _PRINT ("config_nak ");
#endif
            /* Init the restart counter and call the routine to
               handle this state. */ 

            _ncp_num_transmissions = LCP_MAX_CONFIGURE;

            /* Check the NAK pkt. If we get a true back then this pkt contains
               our IP address. */
            status = NCP_Configure_Nak_Check (buf_ptr);

            if (status == NU_TRUE)
            {
                /* We need to send a request with our new IP address. */

                NCP_IP_Send_Config_Req (buf_ptr->mem_buf_device->dev_addr.dev_ip_addr);
            }

            break;

        case LCP_TERMINATE_REQUEST :

            /* Just send a terminate ack */
            LCP_Send_Terminate_Ack (buf_ptr, PPP_IP_CONTROL_PROTOCOL);

            break;

        case LCP_TERMINATE_ACK :

            /* do nothing */

            break;

        case LCP_CODE_REJECT :

            /* We need to check what was rejected and make sure we can do
               without it. */
            status = NCP_IP_Code_Reject_Check (buf_ptr);

            /* If we can not do without it. */
            if (status != NU_TRUE)
            {
                /* Change states. */
                ncp_state.state = STOPPED;
                lcp_state.state = STOPPED;

                /* Hang the modem up. This layer finished. */
                MDM_Hangup();

                /* Change states. */
                ncp_state.state = STARTING;
                lcp_state.state = STARTING;

            }

            break;

        default :

            /* If we get here then an un-interpretable
               packet was received, send a
               code-reject */

            LCP_Send_Code_Reject (buf_ptr, PPP_IP_CONTROL_PROTOCOL);
            break;

    } /* switch */

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    NCP_IP_Ack_Rcvd_State                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This functions handles processing of an incoming NCP packet.            */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_NCP_IP_Interpret                                                    */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  NCP_Configure_Req_Check                                                 */
/*  NU_Set_Events                                                           */
/*  NCP_Configure_Nak_Check                                                 */
/*  NCP_IP_Send_Config_Req                                                  */
/*  LCP_Send_Terminate_Ack                                                  */
/*  NCP_IP_Code_Reject_Check                                                */
/*  MDM_Hangup                                                              */
/*  LCP_Send_Code_Reject                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER                  *buf_ptr    Pointer to the incoming NCP     */
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
void NCP_IP_Ack_Rcvd_State (NET_BUFFER *buf_ptr)
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
            status = NCP_Configure_Req_Check (buf_ptr);

			/* Check the status to see what we need do. */
			if (status == NU_TRUE)	/* all options were ok */
			{
				/* Change states and notify the upper layer that
                   the application can now have a shot. */
                ncp_state.state = OPENED;

                NU_Set_Events (&Buffers_Available, NCP_CONFIG_SUCCESS, NU_OR);
			}

			break;

		case LCP_CONFIGURE_ACK		:

#ifdef DEBUG_PRINT
            _PRINT ("config ack ");
#endif

            /* Send a configure req */
            NCP_IP_Send_Config_Req (buf_ptr->mem_buf_device->dev_addr.dev_ip_addr);

			/* Change states */
            ncp_state.state = REQ_SENT;

			break;

		case LCP_CONFIGURE_NAK		:

#ifdef DEBUG_PRINT
            _PRINT ("config nak ");
#endif

            /* Check the NAK pkt. If we get a true back then this pkt contains
               our IP address. */
            status = NCP_Configure_Nak_Check (buf_ptr);

            if (status == NU_TRUE)
            {
                /* We need to send a request with our new IP address. */
                NCP_IP_Send_Config_Req (buf_ptr->mem_buf_device->dev_addr.dev_ip_addr);
            }

			/* Change states */
            ncp_state.state = REQ_SENT;

			break;

		case LCP_TERMINATE_REQUEST	:

            /* Send a terminate ack */
            LCP_Send_Terminate_Ack (buf_ptr, PPP_IP_CONTROL_PROTOCOL);

			/* Change states */
            ncp_state.state = REQ_SENT;

			break;

		case LCP_TERMINATE_ACK		:

			/* All we need to do is change states. */
            ncp_state.state = REQ_SENT;

			break;

        case LCP_CODE_REJECT        :

            /* We need to check what was rejected and make sure we can do
               without it. */
            status = NCP_IP_Code_Reject_Check (buf_ptr);

            /* If we can not do without it. */
            if (status != NU_TRUE)
            {
                /* Change states. */
                ncp_state.state = STOPPED;
                lcp_state.state = STOPPED;

                /* Hang the modem up. This layer finished. */
                MDM_Hangup();

                /* Change states. */
                ncp_state.state = STARTING;
                lcp_state.state = STARTING;

            }
            else
                ncp_state.state = REQ_SENT;


			break;

		default :

			/* If we get here then we have RX an unknown code. So
			   send a reject pkt for that code. */

            LCP_Send_Code_Reject (buf_ptr, PPP_IP_CONTROL_PROTOCOL);

			break;
                     
	} /* switch */
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    NCP_Timer_Expire                                                      */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This is the expiration routine for the NCP negotiation timer. It will   */
/* retransmit a NCP packet or signal PPP that negotiation failed. All is    */
/* based on the current state of NCP.                                       */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  Nucleus PLUS                                                            */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  UTL_Timerset                                                            */
/*  intswap                                                                 */
/*  NU_Control_Timer                                                        */
/*  NU_Reset_Timer                                                          */
/*  NU_Set_Events                                                           */
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
void NCP_Timer_Expire(UNSIGNED dev_ptr)
{

#ifdef DEBUG_PRINT
    _PRINT ("\nncp timer ");
#endif

    /* Check the current state to see what we can do. */
    switch (ncp_state.state)
    {
        case CLOSING    :
#ifdef DEBUG_PRINT
            _PRINT ("closing ");
#endif

			/* See if we get another try at terminating. */
            if (_ncp_num_transmissions-- > 0)
            {
#ifdef DEBUG_PRINT
                _PRINT ("send term req again ");
#endif

                /* Set the event to send the packet again. */
                UTL_Timerset (NCP_RESEND, dev_ptr, 0, 0);
            }
            else
            {
                /* Stop the restart timer. */
                NU_Control_Timer (&NCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Stop the echo timer since we are leaving the open state */
                NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

                /* Change states */
				ncp_state.state = CLOSED;

                /* Now let LCP close the link. */

                /* Reset the reset counter */
                _lcp_num_transmissions = LCP_MAX_TERMINATE;

                /* Reset the reset timer */
                NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Set the event that will get the link closed. */
                UTL_Timerset (LCP_CLOSE_LINK, dev_ptr, 0, 0);

                lcp_state.state = CLOSING;

            }


		break;

        case STOPPING   :
#ifdef DEBUG_PRINT
            _PRINT ("stopping\n");
#endif

            /* See if we get another try at terminating. */
            if (_ncp_num_transmissions-- > 0)
            {
#ifdef DEBUG_PRINT
                _PRINT ("send term req again ");
#endif

                /* Set the event to send the packet again. */
                UTL_Timerset (NCP_RESEND, dev_ptr, 0, 0);
            }
            else
            {
                /* Stop the restart timer. */
                NU_Control_Timer (&NCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Stop the echo timer since we are leaving the open state */
                NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

                /* Otherwise we have tried to many times, set the event
                   that will tell initialization we could not configure. Do
                   this only if LCP is in the open state, this will
                   suggest that we are still negotiating the link and this
                   terminate is probaly because we could not configure. */

                if (lcp_state.state == OPENED)
                    NU_Set_Events (&Buffers_Available, LCP_CONFIG_FAIL, NU_OR);

                /* Change states for both LCP */
                ncp_state.state = STOPPED;

                /* Now let LCP close the link. */

                /* Reset the reset counter */
                _lcp_num_transmissions = LCP_MAX_TERMINATE;

                /* Reset the reset timer */
                NU_Control_Timer (&LCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Set the event that will get the link closed. */
                UTL_Timerset (LCP_CLOSE_LINK, dev_ptr, 0, 0);

                lcp_state.state = CLOSING;

            }

		break;

        case REQ_SENT   :
#ifdef DEBUG_PRINT
            _PRINT ("req_sent ");
#endif

            /* See if we get another try at configuring. */
            if (_ncp_num_transmissions-- > 0)
            {
#ifdef DEBUG_PRINT
                _PRINT ("send config req again ");
#endif

                /* Set the event to send the packet again. */
                UTL_Timerset (NCP_RESEND, dev_ptr, 0, 0);
            }
            else
            {
                /* Stop the restart timer. */
                NU_Control_Timer (&NCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Stop the echo timer since we are leaving the open state */
                NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

                /* Change states */
                ncp_state.state = STOPPED;

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
            if (_ncp_num_transmissions--)
            {

                /* We need to send another configure request with the newest
                   current set of configuration options. */

                UTL_Timerset (NCP_SEND_CONFIG, dev_ptr, 0, 0);

                ncp_state.state = REQ_SENT;
            }
            else
            {
                /* Stop the restart timer. */
                NU_Control_Timer (&NCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Stop the echo timer since we are leaving the open state */
                NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

                /* Change states */
                ncp_state.state = STOPPED;

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
            if (_ncp_num_transmissions-- > 0)
            {

                /* We need to send another configure request with the newest
                   current set of configuration options. */

                UTL_Timerset (NCP_SEND_CONFIG, dev_ptr, 0, 0);

            }
            else
            {
                /* Stop the restart timer. */
                NU_Control_Timer (&NCP_Restart_Timer, NU_DISABLE_TIMER);

                /* Stop the echo timer since we are leaving the open state */
                NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

                /* Change states */
                ncp_state.state = STOPPED;

                /* Otherwise we have tried to many times, set the event
                   that will tell initialization we could not configure. */
                NU_Set_Events (&Buffers_Available, LCP_CONFIG_FAIL, NU_OR);
            }

            break;
    }


}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    NCP_IP_Change_Mode                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function sets the mode of operation for NCP and the rest of PPP.   */
/* The main difference between modes is in CLIENT mode PPP requests an IP   */
/* address and in SERVER mode PPP assigns an IP address.                    */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  MDM_Dial                                                                */
/*  MDM_Wait_For_Client                                                     */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  none                                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  int8                    new_mode        CLIENT or SERVER                */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  NU_SUCCESS if the passed in mode*/
/*                                          was acceptable NU_UNAVAILABLE if*/
/*                                          the passed in mode was          */
/*                                          unacceptable                    */
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
STATUS NCP_Change_IP_Mode(int8 new_mode)
{
    STATUS ret_val;

    /* Make sure it is a valid mode */
    if ((new_mode == CLIENT) || (new_mode == SERVER))
    {

        /* Change the mode the one that was passed in */
        ncp_mode = new_mode;

        ret_val = NU_SUCCESS;
    }
    else
        ret_val = NU_UNAVAILABLE;

    return (ret_val);

 }



/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    NCP_Configure_Req_Check                                               */
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
/*  NCP_IP_Req_Sent_State                                                   */
/*  NCP_IP_Ack_Rcvd_State                                                   */
/*  NCP_IP_Ack_Sent_State                                                   */
/*  NCP_IP_Opened_State                                                     */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  intswap                                                                 */
/*  netsetip                                                                */
/*  memcpy                                                                  */
/*  dev_start                                                               */
/*  sizeof                                                                  */
/*  MEM_Buffer_Dequeue                                                      */
/*  MEM_Buffer_Enqueue                                                      */
/*  NU_Tcp_Log_Error                                                        */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER              *in_buf_ptr     Pointer to the incoming NCP     */
/*                                          packet                          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  Were all the options ok         */
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
STATUS NCP_Configure_Req_Check (NET_BUFFER *in_buf_ptr)
{
    NET_BUFFER          *reject_buf_ptr, *nak_buf_ptr, *ack_buf_ptr;
    LCP_LAYER           *ncp_reject_pkt, *ncp_nak_pkt, *ncp_ack_pkt;
    uchar   HUGE        *ncp_pkt_ptr,
            HUGE        *ncp_reject_ptr,
            HUGE        *ncp_nak_ptr;
    uint8               options_ok, dont_need_to_reject, identifier;
    uint16              length, reject_length, nak_length;
    uint8               temp_ip_address[4];
    uchar   HUGE        *ncp_pkt = in_buf_ptr->data_ptr;

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
    ncp_reject_pkt = (LCP_LAYER *) reject_buf_ptr->data_ptr;
    ncp_nak_pkt    = (LCP_LAYER *) nak_buf_ptr->data_ptr;
    ncp_ack_pkt    = (LCP_LAYER *) ack_buf_ptr->data_ptr;

    /* Point to the nak and reject packet data in case we have to
       reject or nak something */
    ncp_nak_ptr = &ncp_nak_pkt->data;
    nak_length = LCP_HEADER_LENGTH;

    ncp_reject_ptr = &ncp_reject_pkt->data;
    reject_length = LCP_HEADER_LENGTH;

    /* This will be used to tell if all the config options are unacceptable */
    options_ok = NU_TRUE;
    dont_need_to_reject = NU_TRUE;

    /* Get the ID of this pkt */
    identifier = ncp_pkt[LCP_ID_OFFSET];

    /* Put the LCP structure on the packet and get the length */
    ncp_pkt_ptr = ncp_pkt;
    length      = *(ncp_pkt_ptr + LCP_LENGTH_OFFSET) << 8;
    length      =  length | *(ncp_pkt_ptr + LCP_LENGTH_OFFSET + 1);

    /* Save the length before we take off the header. This will be used
       if the options are ok */
    ncp_ack_pkt->length = intswap (length);

    /* Now remove the header */
    length     -= LCP_HEADER_LENGTH;

    /* Get a pointer to the data inside the LCP pkt */
    ncp_pkt_ptr = ncp_pkt_ptr + LCP_DATA_OFFSET;

    while (length > 0)
    {
        /* Check out what the option is and see if we can handle it */
        switch (ncp_pkt_ptr[0])
        {
            case NCP_IP_ADDRESS :

                /* Put the address in a temp var */

                temp_ip_address[0] = ncp_pkt_ptr[LCP_CONFIG_OPTS];
                temp_ip_address[1] = ncp_pkt_ptr[LCP_CONFIG_OPTS + 1];
                temp_ip_address[2] = ncp_pkt_ptr[LCP_CONFIG_OPTS + 2];
                temp_ip_address[3] = ncp_pkt_ptr[LCP_CONFIG_OPTS + 3];

                /* See if the address given to us is zero, this indicates
                   a request for IP assignment. */

                if ((temp_ip_address[0] == 0) && (temp_ip_address[1] == 0) &&
                        (temp_ip_address[2] == 0) && (temp_ip_address[3] == 0))
                {
                    /* We must be in server mode to assign an address */
                    if (ncp_mode == SERVER)
                    {
                        /* This will be done by a NAK pkt. So all we need to
                           do is to replace the zero IP address with the
                           assigned one. */

                        /* Put it in the NAK packet */
                        memcpy (ncp_nak_ptr, ncp_pkt_ptr,
                                            NCP_IP_ADDRESS_LENGTH);

                        /* Replace the zeros with the new IP address */
                        ncp_nak_ptr[LCP_CONFIG_OPTS]
                                                = _assigned_peer_ip_address[0];
                        ncp_nak_ptr[LCP_CONFIG_OPTS + 1]
                                                = _assigned_peer_ip_address[1];
                        ncp_nak_ptr[LCP_CONFIG_OPTS + 2]
                                                = _assigned_peer_ip_address[2];
                        ncp_nak_ptr[LCP_CONFIG_OPTS + 3]
                                                = _assigned_peer_ip_address[3];

                        /* Update the length and the ptr */
                        ncp_pkt_ptr += NCP_IP_ADDRESS_LENGTH;
                        length -= NCP_IP_ADDRESS_LENGTH;

                        /* Set the NAK flag and length */
                        options_ok = NU_FALSE;
                        nak_length += NCP_IP_ADDRESS_LENGTH;
                    }
                    else
                    {
                        /* This should never happen, so just discard it and
                           bump the ptr and length. */
                        _silent_discards++;
                        ncp_pkt_ptr += NCP_IP_ADDRESS_LENGTH;
                        length -= NCP_IP_ADDRESS_LENGTH;
                    }
                } /* if */
                else
                {
                    /* If the ID of this pkt is the same as the ID of our
                       last request then the peer is assigning us an IP,
                       otherwise this is the IP of our server. */
                    if (identifier == ncp_state.identifier)
                    {

                        /* We can only be assigned an address if we are in client
                           mode. */
                        if (ncp_mode == CLIENT)
                        {
                            /* Set our new address */
                            memcpy (in_buf_ptr->mem_buf_device->dev_addr.dev_ip_addr,
                                temp_ip_address, 4);

                            /* Bump the length and ptr */
                            ncp_pkt_ptr += NCP_IP_ADDRESS_LENGTH;
                            length -= NCP_IP_ADDRESS_LENGTH;
                        }
                        else
                        {
                            /* If we get here then our peer is probably confirming
                               the assigned IP address. All we need to do is to
                               bump the ptr and length. */

                            ncp_pkt_ptr += NCP_IP_ADDRESS_LENGTH;
                            length -= NCP_IP_ADDRESS_LENGTH;

                        }
                    }
                    else
                    {
                        /* We should get the IP address out, but at this point
                           there is nothing to do with the servers IP. So just
                           bump the length and ptr. */

                        ncp_pkt_ptr += NCP_IP_ADDRESS_LENGTH;
                        length -= NCP_IP_ADDRESS_LENGTH;
                    }

                } /* else */

                break;

            default                         :

                /* If we make it here then there is a protocol that we
                   do not understand, send a reject. */

                /* Put it in the reject packet */
                memcpy (ncp_reject_ptr, ncp_pkt_ptr,
                                       ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET]);

                /* Update the length and the ptr */
                ncp_reject_ptr += ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];
                reject_length += ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                /* Mark that we need to reject something */
                dont_need_to_reject = NU_FALSE;

                /* The length is usually the second field in the protocol,
                   so minus it off and look at the reset of the pkt. */
                length -= ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];
                ncp_pkt_ptr += ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                break;

        } /* switch */
    } /* while */

    /* See if all the options are ok, if so send an ack */
    if (options_ok && dont_need_to_reject)
    {
        /* Every option is ok. The ack pkt is the same as the config pkt.
           The only difference is that the pkt code is now config ack. */

        /* Change the pkt code and put in the same ID */
        ncp_ack_pkt->code = LCP_CONFIGURE_ACK;
        ncp_ack_pkt->identifier = identifier;

        /* Copy the pkt data over */
        memcpy (&ncp_ack_pkt->data, ncp_pkt + LCP_DATA_OFFSET,
                                    intswap(ncp_ack_pkt->length));

        /* Update the lengths for the buffer. */
        ack_buf_ptr->data_len = ack_buf_ptr->mem_total_data_len =
                intswap(ncp_ack_pkt->length);

        /* Set the packet type. */
        ack_buf_ptr->mem_flags = NET_IPCP;

#ifdef DEBUG_PRINT
        _PRINT ("acking ");
#endif

        /* Send it */
        in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device, ack_buf_ptr);

    }
    else
    {
        /* One or more options were not ok. We must nak or reject them. */

        /* See if we need to nak something. Only NAK if we do not have to
           reject. */
        if ((!options_ok) && (dont_need_to_reject))
        {
            /* Put in the pkt indentifier and type */
            ncp_nak_pkt->identifier     = identifier;
            ncp_nak_pkt->code           = LCP_CONFIGURE_NAK;
            ncp_nak_pkt->length         = intswap (nak_length);

#ifdef DEBUG_PRINT
            _PRINT ("naking ");
#endif
            /* Update the lengths for the buffer. */
            nak_buf_ptr->data_len = nak_buf_ptr->mem_total_data_len =
                nak_length;

            /* Set the packet type. */
            nak_buf_ptr->mem_flags = NET_IPCP;

            /* Send the pkt */
            in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device,
                            nak_buf_ptr);
        }

        /* See if we need to reject something */
        if (!dont_need_to_reject)
        {
            /* Put in the pkt indentifier and type */
            ncp_reject_pkt->identifier  = identifier;
            ncp_reject_pkt->code        = LCP_CONFIGURE_REJECT;
            ncp_reject_pkt->length      = intswap (reject_length);

            /* Update the lengths for the buffer. */
            reject_buf_ptr->data_len = reject_buf_ptr->mem_total_data_len =
                reject_length;

            /* Set the packet type. */
            reject_buf_ptr->mem_flags = NET_IPCP;

#ifdef DEBUG_PRINT
            _PRINT ("rejecting ");
#endif

            /* Send the pkt */
            in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device,
                            reject_buf_ptr);
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
/*    NCP_Configure_Nak_Check                                               */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function checks all naked options in a configure nak packet. It    */
/* is used to assign an address to a peer. If the packet contains an unknown*/
/* option this function will send a reject packet for that option.          */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  NCP_IP_Req_Sent_State                                                   */
/*  NCP_IP_Ack_Rcvd_State                                                   */
/*  NCP_IP_Ack_Sent_State                                                   */
/*  NCP_IP_Opened_State                                                     */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  netsetip                                                                */
/*  memcpy                                                                  */
/*  dev_start                                                               */
/*  sizeof                                                                  */
/*  MEM_Buffer_Dequeue                                                      */
/*  MEM_Buffer_Enqueue                                                      */
/*  NU_Tcp_Log_Error                                                        */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER          *in_buf_ptr         Pointer to the incoming NCP     */
/*                                          packet                          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  Did we get an IP address        */
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
STATUS NCP_Configure_Nak_Check (NET_BUFFER *in_buf_ptr)
{
    NET_BUFFER          *buf_ptr;
    LCP_LAYER           *ncp_reject_pkt;
    uchar   HUGE        *ncp_pkt_ptr,
            HUGE        *ncp_reject_ptr;
    uint8               ip_reply, dont_need_to_reject, identifier;
    uint16              length, reject_length;
    uint8               temp_ip_address[IP_ADDR_LEN];
    uint8               subnet_mask [IP_ADDR_LEN];
    uchar   HUGE        *ncp_pkt                = in_buf_ptr->data_ptr;

    /* This will be used to tell if all the config options are unacceptable */
    ip_reply = NU_FALSE;
    dont_need_to_reject = NU_TRUE;

    /* Allocate a buffer for the reject packet. */
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
    buf_ptr->data_ptr   = (buf_ptr->mem_parent_packet + sizeof (PPP_HEADER));
    ncp_reject_pkt      = (LCP_LAYER *)buf_ptr->data_ptr;

    /* Get the ID of this pkt */
    identifier = ncp_pkt[LCP_ID_OFFSET];

    /* Put the LCP structure on the packet and get the length */
    ncp_pkt_ptr = ncp_pkt;
    length      = *(ncp_pkt_ptr + LCP_LENGTH_OFFSET) << 8;
    length      =  length | *(ncp_pkt_ptr + LCP_LENGTH_OFFSET + 1);

    /* Now remove the header */
    length     -= LCP_HEADER_LENGTH;

    /* Get a pointer to the data inside the LCP pkt */
    ncp_pkt_ptr = ncp_pkt_ptr + LCP_DATA_OFFSET;

    /* Point to the reject packet data in case we have to
       reject something */
    ncp_reject_ptr = &ncp_reject_pkt->data;
    reject_length = LCP_HEADER_LENGTH;

    while (length > 0)
    {
        /* Check out what the option is and see if we can handle it */
        switch (ncp_pkt_ptr[0])
        {
            case NCP_IP_ADDRESS :

                /* The ID of this pkt must be the same as the ID of our
                   config req pkt. */
                if (identifier == ncp_state.identifier)
                {

                    /* Put the address in a temp var */

                    temp_ip_address[0] = ncp_pkt_ptr[LCP_CONFIG_OPTS];
                    temp_ip_address[1] = ncp_pkt_ptr[LCP_CONFIG_OPTS + 1];
                    temp_ip_address[2] = ncp_pkt_ptr[LCP_CONFIG_OPTS + 2];
                    temp_ip_address[3] = ncp_pkt_ptr[LCP_CONFIG_OPTS + 3];

                    /* We can only be assigned an address if we are in client
                       mode. */
                    if (ncp_mode == CLIENT)
                    {
                        /* Get the subnet mask for this type of address. */
                        IP_Get_Net_Mask ((CHAR *)&temp_ip_address, (CHAR *)subnet_mask);

                        /* Set our new address */
                        DEV_Attach_IP_To_Device (in_buf_ptr->mem_buf_device->dev_net_if_name,
                                        temp_ip_address, subnet_mask);

                        /* Set the flag that signals we have been assigned an IP
                        address. */
                        ip_reply = NU_TRUE;

#ifdef DEBUG_PRINT
                        _PRINT ("got IP ");
                        _PRINT ("%d.", temp_ip_address[0]);
                        _PRINT ("%d.", temp_ip_address[1]);
                        _PRINT ("%d.", temp_ip_address[2]);
                        _PRINT ("%d ", temp_ip_address[3]);
#endif

                        /* Bump the length and ptr */
                        ncp_pkt_ptr += NCP_IP_ADDRESS_LENGTH;
                        length -= NCP_IP_ADDRESS_LENGTH;
                    }
                    else
                    {
                        /* We should never get here. If we are in server mode
                           and the client NAKed us with an IP address then there
                           is probably an implementation problem with the
                           client. */

                        ncp_pkt_ptr += NCP_IP_ADDRESS_LENGTH;
                        length -= NCP_IP_ADDRESS_LENGTH;

                    }
                }

                break;

            default                         :

                /* If we make it here then there is a protocol that we
                   do not understand, send a reject. */

                /* Put it in the reject packet */
                memcpy (ncp_reject_ptr, ncp_pkt_ptr,
                                       ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET]);

                /* Update the length and the ptr */
                ncp_reject_ptr += ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];
                reject_length += ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                /* Mark that we need to reject something */
                dont_need_to_reject = NU_FALSE;

                /* The length is usually the second field in the protocol,
                   so minus it off and look at the reset of the pkt. */
                length -= ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];
                ncp_pkt_ptr += ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

                break;

        } /* switch */
    } /* while */

    /* See if we need to reject something */
    if (!dont_need_to_reject)
    {
        /* Put in the pkt indentifier and type */
        ncp_reject_pkt->identifier  = identifier;
        ncp_reject_pkt->code        = LCP_CONFIGURE_REJECT;
        ncp_reject_pkt->length      = intswap (reject_length);

#ifdef DEBUG_PRINT
        _PRINT ("rejecting ");
#endif

        /* Setup the packet type and length */
        buf_ptr->mem_flags      = NET_IPCP;
        buf_ptr->data_len = buf_ptr->mem_total_data_len = reject_length;

        /* Send the pkt */
        in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device,
                                    buf_ptr);
    }

    /* Release the buffer allocated at the beginning of this function. */
    MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, buf_ptr);

    return (ip_reply);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    NCP_IP_Ack_Sent_State                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This functions handles processing of an incoming NCP packet.            */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_NCP_IP_Interpret                                                    */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  NCP_Configure_Req_Check                                                 */
/*  NU_Set_Events                                                           */
/*  NCP_Configure_Nak_Check                                                 */
/*  netgetip                                                                */
/*  NCP_IP_Send_Config_Req                                                  */
/*  LCP_Send_Terminate_Ack                                                  */
/*  NCP_IP_Code_Reject_Check                                                */
/*  MDM_Hangup                                                              */
/*  LCP_Send_Code_Reject                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER          *in_buf_ptr         Pointer to the incoming NCP     */
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
void NCP_IP_Ack_Sent_State (NET_BUFFER *buf_ptr)
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
            status = NCP_Configure_Req_Check (buf_ptr);

			/* Check the status to see what we need do. */
            if (status != NU_TRUE)  /* all options were not ok */
                ncp_state.state = REQ_SENT;

            break;

		case LCP_CONFIGURE_ACK		:

#ifdef DEBUG_PRINT
            _PRINT ("config_ack ");
#endif

            /* Change states and notify the upper layer that
               the application can now have a shot. */
            ncp_state.state = OPENED;

            NU_Set_Events (&Buffers_Available, NCP_CONFIG_SUCCESS, NU_OR);

			break;

		case LCP_CONFIGURE_NAK		:

#ifdef DEBUG_PRINT
            _PRINT ("config_nak ");
#endif
            /* Init the restart counter and send a NAK */

            _ncp_num_transmissions = LCP_MAX_CONFIGURE;

            /* Check the NAK pkt. If we get a true back then this pkt contains
               our IP address. */
            status = NCP_Configure_Nak_Check (buf_ptr);

            if (status == NU_TRUE)
            {
                /* We need to send a request with our new IP address. */

                NCP_IP_Send_Config_Req (buf_ptr->mem_buf_device->dev_addr.dev_ip_addr);
            }

			break;

        case LCP_TERMINATE_REQUEST  :

#ifdef DEBUG_PRINT
            _PRINT ("term req ");
#endif

            /* Send a terminate ack */
            LCP_Send_Terminate_Ack (buf_ptr, PPP_IP_CONTROL_PROTOCOL);

			/* Change states */
            ncp_state.state = REQ_SENT;

			break;

		case LCP_TERMINATE_ACK		:

#ifdef DEBUG_PRINT
            _PRINT ("term ack ");
#endif
            /* do nothing */

			break;

		case LCP_CODE_REJECT		:

#ifdef DEBUG_PRINT
            _PRINT ("code reject ");
#endif
            /* We need to check what was rejected and make sure we can do
               without it. */
            status = NCP_IP_Code_Reject_Check (buf_ptr);

            /* If we can not do without it. */
            if (status != NU_TRUE)
            {
                /* Change states. */
                ncp_state.state = STOPPED;
                lcp_state.state = STOPPED;

                /* Hang the modem up. This layer finished. */
                MDM_Hangup();

                /* Change states. */
                ncp_state.state = STARTING;
                lcp_state.state = STARTING;

            }

			break;

		default :

#ifdef DEBUG_PRINT
            _PRINT ("unknown type ");
#endif
            /* If we get here then we have RX an unknown code. So
			   send a reject pkt for that code. */

            LCP_Send_Code_Reject (buf_ptr, PPP_IP_CONTROL_PROTOCOL);

			break;
                     
	} /* switch */
}


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    NCP_IP_Opened_State                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This functions handles processing of an incoming NCP packet.            */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_NCP_IP_Interpret                                                    */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  NCP_Configure_Req_Check                                                 */
/*  netgetip                                                                */
/*  NCP_IP_Send_Config_Req                                                  */
/*  NCP_Configure_Nak_Check                                                 */
/*  PPP_Kill_All_Open_Sockets                                               */
/*  LCP_Send_Terminate_Ack                                                  */
/*  NCP_IP_Code_Reject_Check                                                */
/*  LCP_Send_Terminate_Req                                                  */
/*  NU_Control_Timer                                                        */
/*  NU_Reset_Timer                                                          */
/*  LCP_Send_Code_Reject                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER          *in_buf_ptr         Pointer to the incoming NCP     */
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
void NCP_IP_Opened_State (NET_BUFFER *buf_ptr)
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

            /* Determine if the options are ok
			   with our side. This function will take care of
			   sending the ACK or NAK. */
            status = NCP_Configure_Req_Check (buf_ptr);

			/* Check the status to see what we need do. */
			if (status == NU_TRUE)	/* all options were ok */
			{
				/* Change states and notify the upper layer that
				   NCP can now have a shot. */
                ncp_state.state = ACK_SENT;

            }
            else
                ncp_state.state = REQ_SENT;

            /* Get our IP address. */

            /* Send a configure req pkt */
            NCP_IP_Send_Config_Req(buf_ptr->mem_buf_device->dev_addr.dev_ip_addr);

			break;

		case LCP_CONFIGURE_ACK		:
#ifdef DEBUG_PRINT
            _PRINT ("config_ack ");
#endif
            /* this layer down action */

            /* Get our IP address so we can ACK with it. */

            /* Send a configure req */
            NCP_IP_Send_Config_Req(buf_ptr->mem_buf_device->dev_addr.dev_ip_addr);

            /* Change states */
            ncp_state.state = REQ_SENT;

			break;

		case LCP_CONFIGURE_NAK		:
#ifdef DEBUG_PRINT
            _PRINT ("config_nak ");
#endif
            /* this layer down action */

            /* Check the NAK pkt. */
            status = NCP_Configure_Nak_Check (buf_ptr);

            /* If the status is true then we are not in the loop-back
               condition. Send a configure request with the new set of
               options. */
            if (status == NU_TRUE)
            {
                /* Get our IP address. */

                /* Send the new config req */
                NCP_IP_Send_Config_Req (buf_ptr->mem_buf_device->dev_addr.dev_ip_addr);
            }

            ncp_state.state = REQ_SENT;

            break;

		case LCP_TERMINATE_REQUEST	:
#ifdef DEBUG_PRINT
            _PRINT ("term_req ");
#endif

            /* this layer down */
            PPP_Kill_All_Open_Sockets(buf_ptr->mem_buf_device);

            /* Detach the IP address from this device. */
            DEV_Detach_IP_From_Device (buf_ptr->mem_buf_device->dev_net_if_name);

            /* Zero the restart counter */
            _ncp_num_transmissions = 0;

            /* Change states */
            ncp_state.state = STOPPING;

            /* Send an ack for the terminate request */
            LCP_Send_Terminate_Ack (buf_ptr, PPP_IP_CONTROL_PROTOCOL);

			break;

		case LCP_TERMINATE_ACK		:

#ifdef DEBUG_PRINT
            _PRINT ("term ack ");
#endif

            /* this layer down */

            /* Get our IP address. */

            /* Send a configure req */
            NCP_IP_Send_Config_Req(buf_ptr->mem_buf_device->dev_addr.dev_ip_addr);

            /* Change states */
            ncp_state.state = REQ_SENT;

			break;

		case LCP_CODE_REJECT		:

#ifdef DEBUG_PRINT
            _PRINT ("code reject ");
#endif

            /* We need to check what was rejected and make sure we can do
               without it. */
            status = NCP_IP_Code_Reject_Check (buf_ptr);

            /* If we can not do without it. */
            if (status != NU_TRUE)
            {
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
                ncp_state.state = STOPPING;
                lcp_state.state = STOPPING;

            }

            break;

        default :

#ifdef DEBUG_PRINT
            _PRINT ("unknown code ");
#endif

            /* If we get here then we have RX an unknown code. So
			   send a reject pkt for that code. */

            LCP_Send_Code_Reject(buf_ptr, PPP_IP_CONTROL_PROTOCOL);

			break;
                     
	} /* switch */


}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    NCP_IP_Code_Reject_Check                                              */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Checks the reject packet options to determine if PPP can do without     */
/* the rejected option.                                                     */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  NCP_IP_Req_Sent_State                                                   */
/*  NCP_IP_Ack_Rcvd_State                                                   */
/*  NCP_IP_Ack_Sent_State                                                   */
/*  NCP_IP_Opened_State                                                     */
/*  NCP_IP_Stopping_State                                                   */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  none                                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER          *in_buf_ptr         Pointer to the incoming NCP     */
/*                                          packet                          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  Can PPP live without the        */
/*                                          rejected option                 */
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
STATUS NCP_IP_Code_Reject_Check (NET_BUFFER *in_buf_ptr)
{
    uchar   HUGE        *ncp_pkt_ptr;
    uint8               options_ok, identifier;
    uint16              length;
    uchar   HUGE        *ncp_pkt = in_buf_ptr->data_ptr;

    /* This will be used to tell if all the rejected options are acceptable */
    options_ok = NU_TRUE;

    /* Get the ID of this pkt */
    identifier = ncp_pkt[LCP_ID_OFFSET];

    /* The id must match that of the last sent config pkt. */
    if (identifier == ncp_state.identifier)
    {

        /* Put the LCP structure on the packet and get the length */
        ncp_pkt_ptr = ncp_pkt;
        length      = *(ncp_pkt_ptr + LCP_LENGTH_OFFSET) << 8;
        length      =  length | *(ncp_pkt_ptr + LCP_LENGTH_OFFSET + 1);

        /* Now remove the header */
        length     -= LCP_HEADER_LENGTH;

        /* Get a pointer to the data inside the LCP pkt */
        ncp_pkt_ptr = ncp_pkt_ptr + LCP_DATA_OFFSET;

        while ((length > 0) && options_ok)
        {
            /* Check out what option is rejected and see if we can handle it.
               There is only one option that we support here. */

            switch (ncp_pkt_ptr[0])
            {
                case NCP_IP_ADDRESS :

                    /* This option must be configured. */
                    options_ok = NU_FALSE;

                    break;

                default :

                    /* All other options can be rejected since we don't
                       support them. Just bump the lenth and ptr. */

                    length -= ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];
                    ncp_pkt_ptr += ncp_pkt_ptr[LCP_CONFIG_LENGTH_OFFSET];

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

    return (options_ok);
}


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    NCP_IP_Stopping_State                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This functions handles processing of an incoming NCP packet.            */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_NCP_IP_Interpret                                                    */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Send_Terminate_Ack                                                  */
/*  NU_Control_Timer                                                        */
/*  MDM_Hangup                                                              */
/*  NCP_IP_Code_Reject_Check                                                */
/*  LCP_Send_Code_Reject                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER          *buf_ptr            Pointer to the incoming NCP     */
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
void NCP_IP_Stopping_State (NET_BUFFER *buf_ptr)
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

		case LCP_TERMINATE_REQUEST	:

#ifdef DEBUG_PRINT
            _PRINT ("term_req ");
#endif

            /* Send an ACK */
            LCP_Send_Terminate_Ack(buf_ptr, PPP_IP_CONTROL_PROTOCOL);

			break;

		case LCP_TERMINATE_ACK		:

#ifdef DEBUG_PRINT
            _PRINT ("term_ack ");
#endif

            /* Change states */
            ncp_state.state = STOPPED;

            /* Reset the restart timer */
            NU_Control_Timer (&NCP_Restart_Timer, NU_DISABLE_TIMER);

            /* Hang the modem up. This layer finished. */
            MDM_Hangup();

            /* Change states. */
            ncp_state.state = STARTING;
            lcp_state.state = STARTING;

			break;

		case LCP_CODE_REJECT		:

#ifdef DEBUG_PRINT
            _PRINT ("code reject ");
#endif
            /* We need to check what was rejected and make sure we can do
               without it. */
            status = NCP_IP_Code_Reject_Check (buf_ptr);

            /* If we can not do without it. */
            if (status != NU_TRUE)
            {
                /* Change states. */
                ncp_state.state = STOPPED;

                /* Hang the modem up. This layer finished. */
                MDM_Hangup();

                /* Change states. */
                ncp_state.state = STARTING;
                lcp_state.state = STARTING;
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

            LCP_Send_Code_Reject(buf_ptr, PPP_IP_CONTROL_PROTOCOL);

			break;
                     
	} /* switch */

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    NCP_Set_Client_IP_Address                                             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function sets the IP address to be assigned to a calling client. It*/
/* is only used in SERVER mode.                                             */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  Application                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  netgetip                                                                */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  uint8                   *ip_address     Pointer to the IP address to set*/
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  Was the address passed in set   */
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
STATUS NCP_Set_Client_IP_Address (uint8 *ip_address)
{
    STATUS ret_status;

    if (ip_address == NU_NULL)
        ret_status = NU_INVALID_ADDRESS;
    else
        memcpy (_assigned_peer_ip_address, ip_address, 4);

    return (ret_status);

}
