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
/*    PAP.C                                                  2.0            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains the password authentication protocol that is used  */
/*    to log into a PPP server and authenticate a calling PPP client.       */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock                                                      */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*   none                                                                   */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*   PPP_PAP_Interpret                                                      */
/*   PAP_Send_Authentication                                                */
/*   PAP_Timer_Expire                                                       */
/*   PAP_Check_Login                                                        */
/*   PAP_Send_Authentication_Ack_Nak                                        */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*   nucleus.h                                                              */
/*   externs.h                                                              */
/*   tcp_errs.h"                                                            */
/*   netevent.h                                                             */
/*   target.h                                                               */
/*   data.h                                                                 */
/*   ppp.h                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        11/18/97      Fixed a bug in PAP_Timer_Expire   */
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
#include "target.h"
#include "data.h"
#include "ppp.h"

/* Import all external variables */
extern struct lcp_state_struct  lcp_state;
extern struct lcp_state_struct  ncp_state;
extern int8                     _num_authentication_timeouts;
extern uint8                    ncp_mode;
extern struct pw_list           _passwordlist[];
extern NU_TIMER                 LCP_Echo_Timer;

/* Import the vars to be used for ID and PW */
extern char  _ppp_login_pw[PPP_MAX_PW_LENGTH];
extern char  _ppp_login_name[PPP_MAX_ID_LENGTH];


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_PAP_Interpret                                                     */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function processes the incoming PAP packet. This envolves        */
/* responding to an authentication request and informing the upper layer    */
/* if PAP has succeed or failed.                                            */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_Input                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    PAP_Check_Login                                                       */
/*    PAP_Send_Authentication_Ack_Nak                                       */
/*    NU_Set_Events                                                         */
/*    NU_Control_Timer                                                      */
/*    MEM_Buffer_Chain_Free                                                 */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER                  *buf_ptr    Pointer to the incoming PAP     */
/*                                          packet                          */
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
void PPP_PAP_Interpret (NET_BUFFER *buf_ptr)
{
    char            temp_id [PPP_MAX_ID_LENGTH];
    char            temp_pw [PPP_MAX_PW_LENGTH];
    int             temp_id_len, temp_pw_len, x;
    STATUS          ret_status;
    uchar   HUGE    *pap_pkt = buf_ptr->data_ptr;

#ifdef DEBUG_PRINT
    _PRINT ("\npap interpret ");
#endif

    /* Make sure that LCP is done. */
    if (lcp_state.state == OPENED)
    {
        switch (pap_pkt[0])
        {
            case PAP_AUTHENTICATE_REQUEST :

#ifdef DEBUG_PRINT
                _PRINT ("req ");
#endif

                /* We need to pull out the ID and PW, then check them
                   against our internal password list. */

                /* get the id, we know where it starts in the pkt. */

                /* Get the length of the value. */
                temp_id_len = pap_pkt [PAP_ID_LENGTH_OFFSET];

                /* Copy it over. */
                for (x = 0; (x < temp_id_len) && (x < PPP_MAX_ID_LENGTH); x++)
                   temp_id [x] = pap_pkt [PAP_ID_OFFSET + x];

                /* Null terminate it. */
                temp_id [x] = (char)0;

                /* now get the pw. where it starts in the pkt depends on the
                   id length field. */
                temp_pw_len = pap_pkt [PAP_ID_OFFSET + temp_id_len];

                /* Copy it over. */
                for (x = 0; (x < temp_pw_len) && (x < PPP_MAX_PW_LENGTH); x++)
                   temp_pw [x] = pap_pkt [PAP_ID_OFFSET + temp_id_len + 1 + x];

                /* Null terminate it. */
                temp_pw [x] = (char)0;

                /* Now check it against the password list */
                ret_status = PAP_Check_Login (temp_id, temp_pw);

                /* Is it ok */
                if (ret_status == NU_TRUE)
                {
                    /* Let the CLIENT know that everything checked out. */
                    PAP_Send_Authentication_Ack_Nak(buf_ptr, PAP_AUTHENTICATE_ACK,
                                                    pap_pkt [LCP_ID_OFFSET]);

                    /* Move on to the next phase */
                    NU_Set_Events (&Buffers_Available, AUTHENTICATION_SUCCESS,
                                                                    NU_OR);
                }
                else
                {
                    /* Stop the echo timer since we are leaving the open state */
                    NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

                    /* tell the CLIENT that they failed to login */
                    PAP_Send_Authentication_Ack_Nak(buf_ptr, PAP_AUTHENTICATE_NAK,
                                                    pap_pkt [LCP_ID_OFFSET]);

                    /* Stop the PPP session. */
                    NU_Set_Events (&Buffers_Available, LCP_CONFIG_FAIL, NU_OR);
                }

                break;

            case PAP_AUTHENTICATE_ACK   :
#ifdef DEBUG_PRINT
                _PRINT ("ack ");
#endif

                /* We have received a positive response from the server.
                   If NCP is in need of negotiation let it know, otherwise
                   PAP is done. */

                if (ncp_state.state != OPENED)
                    NU_Set_Events (&Buffers_Available, AUTHENTICATION_SUCCESS,
                                                                    NU_OR);

                break;

            case PAP_AUTHENTICATE_NAK   :
#ifdef DEBUG_PRINT
                _PRINT ("nak ");
#endif
                /* Stop the echo timer since we are leaving the open state */
                NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

                /* We have received a negative response from the server,
                   let the upper layer know. */
                NU_Set_Events (&Buffers_Available, LCP_CONFIG_FAIL, NU_OR);

                break;
        } /* switch */
    } /* if */

    /* Release the buffer space */
    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);


}


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PAP_Send_Authentication                                               */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function sends an authentication request.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_Lower_Layer_Up                                                    */
/*    NU_EventsDispatcher                                                   */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Random_Number                                                       */
/*  PPP_TX_Packet                                                           */
/*  intswap                                                                 */
/*  MEM_Buffer_Dequeue                                                      */
/*  NU_Tcp_Log_Error                                                        */
/*  MEM_Buffer_Enqueue                                                      */
/*  sizeof                                                                  */
/*  strlen                                                                  */
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
void PAP_Send_Authentication (void)
{
    NET_BUFFER  *buf_ptr;
    LCP_LAYER   *pap_pkt;
    uchar       *pap_pkt_ptr;
    uint16      len;
    uint8       pw_id_len, x;

#ifdef DEBUG_PRINT
    _PRINT ("PAP send ");
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
    pap_pkt             = (LCP_LAYER *) buf_ptr->data_ptr;

    len = 0;

    /* Get a pointer to the data part of the packet */
    pap_pkt_ptr = (uchar *)&pap_pkt->data;

    /* Set up the packet */
    pap_pkt->code        = PAP_AUTHENTICATE_REQUEST;
    pap_pkt->identifier  = LCP_Random_Number();

    /* Add length of the name of the user */
    pap_pkt_ptr[len++] = pw_id_len = strlen (_ppp_login_name);

    /* Copy the login name into the pkt. */
    for (x = 0; x < pw_id_len; x++, len++)
        pap_pkt_ptr[len] = _ppp_login_name[x];

    /* Add length the PW of the user */
    pap_pkt_ptr[len++] = pw_id_len = strlen (_ppp_login_pw);

    /* Copy the PW over. */
    for (x = 0; x < pw_id_len; x++)
        pap_pkt_ptr[len++] = _ppp_login_pw [x];

    /* Add on the LCP header to the length and
       put it in the pkt. */
    len             += LCP_HEADER_LENGTH;
    pap_pkt->length  = intswap (len);

    /* Set the packet type for this buffer. */
    buf_ptr->mem_flags = NET_PAP;

    /* Set the length of the data contained in the buffer. */
    buf_ptr->data_len = buf_ptr->mem_total_data_len = len;

    /* Send the pkt. */
    PPP_TX_Packet (buf_ptr->mem_buf_device, buf_ptr);

    /* Release the buffer allocated at the beginning of this function. */
    MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, buf_ptr);

}


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PAP_Timer_Expire                                                      */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function handles the authentication timer expiration. If the     */
/* authentication request has not been sent more than LCP_MAX_AUTHENTICATE  */
/* times then it will get sent again. Otherwise we will consider that the   */
/* authentication failed.                                                   */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Nucleus PLUS                                                          */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    UTL_Timerset                                                          */
/*    NU_Control_Timer                                                      */
/*    NU_Set_Events                                                         */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  UNSIGNED                    dev_ptr     Address of the device structure */
/*                                          that caused this timer event.   */
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
/*  Uriah T. Pollock        11/18/97      Fixed a bug where this timer      */
/*                                         expiration routine would self    */
/*                                         suspend, some of the routines it */
/*                                         called did the actual suspension.*/
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
void PAP_Timer_Expire (UNSIGNED dev_ptr)
{

#ifdef DEBUG_PRINT
    _PRINT ("\npap timer ");
#endif

    /* See if we will try again. */
    if (_num_authentication_timeouts-- > 0)
    {
        /* if we are a CLIENT then we need to retransmit the authentication
           packet. */
        if (ncp_mode == CLIENT)
        {
            /* Set the event that will send an authenticaion packet */
            UTL_Timerset (PAP_SEND_AUTH, dev_ptr, 0, 0);
        }
        else
        {
            /* We do not need to do anything. We are still waiting for
               a response from our CLIENT. */
        }
    }
    else
    {
        /* Stop the echo timer since we are leaving the open state */
        NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

        /* We have tried or waited too many times. Give up. */
        NU_Set_Events (&Buffers_Available, LCP_CONFIG_FAIL, NU_OR);
    }

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PAP_Check_Login                                                       */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Searchs the password list for a correct match.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_PAP_Interpret                                                     */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*     strcmp                                                               */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*     char                     id[]        ID to check                     */
/*     char                     pw[]        pw to check                     */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*     STATUS                               Was a correct match found       */
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
STATUS PAP_Check_Login (char id[PPP_MAX_ID_LENGTH], char pw[PPP_MAX_PW_LENGTH])
{
    STATUS  found_it, wrong_pw;
    uint    index;

    /* Assume an invalid login */
    found_it = NU_FALSE;
    wrong_pw = NU_FALSE;

    /* All we need to do is loop through the password list and compare
       the ID and PW. */
    for (index = 0; (_passwordlist[index].id[0] != 0) && (found_it == NU_FALSE)
                    && (wrong_pw == NU_FALSE); index++)
    {
        /* If the IDs match, check the PW */
        if ((strcmp (id, _passwordlist[index].id)) == 0)
        {
            if ((strcmp (pw, _passwordlist[index].pw)) == 0)

                /* If they both match then we found it. */
                found_it = NU_TRUE;
            else
            {
                /* The password must be wrong. */
                wrong_pw = NU_TRUE;
            }
        }

    }

    return (found_it);
}


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PAP_Send_Authentication_Ack_Nak                                       */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Sends a PAP ack or nak packet                                         */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_PAP_Interpret                                                     */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  dev_start                                                               */
/*  intswap                                                                 */
/*  MEM_Buffer_Dequeue                                                      */
/*  NU_Tcp_Log_Error                                                        */
/*  sizeof                                                                  */
/*  MEM_Buffer_Enqueue                                                      */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER                  *in_buf_ptr Pointer to the incoming PAP     */
/*                                          packet                          */
/*  int                         pkt_type    Which type to send - ACK or NAK */
/*  int                         req_id      ID of the request that caused   */
/*                                          this ACK/NAK                    */
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
void PAP_Send_Authentication_Ack_Nak(NET_BUFFER *in_buf_ptr, int pkt_type, int req_id)
{
    NET_BUFFER  *buf_ptr;
    LCP_LAYER   *pap_pkt;
    uint16      len;

#ifdef DEBUG_PRINT
    if (pkt_type == PAP_AUTHENTICATE_ACK)
        _PRINT ("send ack ");
    else
        _PRINT ("send nak ");
#endif

    len = 0;

    /* Allocate a buffer for the PAP packet. */
    buf_ptr = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

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
    pap_pkt = (LCP_LAYER *) buf_ptr->data_ptr;
               
    /* We will not send a msg. So set the msg len to 0. The msg length
       is the first byte of data. */
    pap_pkt->data = 0;

    /* Set up the packet */
    if (pkt_type == PAP_AUTHENTICATE_ACK)
    {
        pap_pkt->code = PAP_AUTHENTICATE_ACK;
    }
    else
    {
        pap_pkt->code = PAP_AUTHENTICATE_NAK;
    }

    pap_pkt->identifier  = req_id;

    /* Add on the LCP header to the length and
       put it in the pkt. */
    len += LCP_HEADER_LENGTH;
    pap_pkt->length = intswap (len);

    /* Set the lengths for the packet buffer. */
    buf_ptr->data_len = buf_ptr->mem_total_data_len = len;

    /* Set the packet type. */
    buf_ptr->mem_flags = NET_PAP;

    /* Send the pkt. */
    in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device, buf_ptr);

    /* Release the buffer. */
    MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, buf_ptr);

}

