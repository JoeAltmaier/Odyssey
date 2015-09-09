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
/*    CHAP.C                                                  2.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains the challenge handshake authentication protocol    */
/*    that is used the log into a PPP server and to authenticate a calling  */
/*    PPP client.                                                           */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah Pollock                                                         */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  int8                            _num_authentication_timeouts            */
/*  static uint32                   challenge_value                         */
/*  static uint8                    challenge_identifier                    */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*  PPP_CHAP_Interpret                                                      */
/*  CHAP_MD5_Encrypt                                                        */
/*  CHAP_Timer_Expire                                                       */
/*  CHAP_Send_Challenge                                                     */
/*  CHAP_Respond_To_Challenge                                               */
/*  CHAP_Check_Response                                                     */
/*  CHAP_Send_Success                                                       */
/*  CHAP_Send_Failure                                                       */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*  nucleus.h                                                               */
/*  externs.h                                                               */
/*  tcp_errs.h"                                                             */
/*  netevent.h                                                              */
/*  data.h                                                                  */
/*  target.h                                                                */
/*  ppp.h                                                                   */
/*  global.h                                                                */
/*  md5.h                                                                   */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        11/18/97      Fixed a bug in CHAP_Timer_Expire  */
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
#include "target.h"        /* Compiler typedefs for data type sizes */
#include "data.h"
#include "ppp.h"

/* These are part of the MD5 encryption */
#include "global.h"
#include "md5.h"

/* Import all external variables */
extern NU_TIMER                 Authentication_Timer;
extern NU_TIMER                 LCP_Echo_Timer;
extern struct lcp_state_struct  lcp_state;
extern struct lcp_state_struct  ncp_state;
extern uint8                    ncp_mode;
extern char                     _ppp_login_pw[PPP_MAX_PW_LENGTH];
extern struct pw_list           _passwordlist[];
extern char                     _ppp_login_name[PPP_MAX_ID_LENGTH];

int8                            _num_authentication_timeouts;
static uint32                   challenge_value;
static uint8                    challenge_identifier;

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    PPP_CHAP_Interpret                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function processes the incoming CHAP packet. This envolves       */
/* responding to an authentication request and informing the upper layer    */
/* if CHAP has succeed or failed.                                           */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_Input                                                               */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  CHAP_Respond_To_Challenge                                               */
/*  CHAP_Check_Response                                                     */
/*  CHAP_Send_Success                                                       */
/*  NU_Set_Events                                                           */
/*  CHAP_Send_Failure                                                       */
/*  NU_Control_Timer                                                        */
/*  MEM_Buffer_Chain_Free                                                   */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER                  *buf_ptr    Pointer to the incoming CHAP    */
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
VOID PPP_CHAP_Interpret (NET_BUFFER *buf_ptr)
{
    STATUS status;

#ifdef DEBUG_PRINT
    _PRINT ("\nchap interpret ");
#endif

    /* We must be in the opened state in order to authenticate */

    if (lcp_state.state == OPENED)
    {
        switch (*buf_ptr->data_ptr)
        {
            case CHAP_CHALLENGE :
#ifdef DEBUG_PRINT
                _PRINT ("challenge ");
#endif

                /* We will only except a challenge if we are a client */

                if (ncp_mode == CLIENT)
                {
                    CHAP_Respond_To_Challenge(buf_ptr);
                }

                break;

            case CHAP_RESPONSE :
#ifdef DEBUG_PRINT
                _PRINT ("response ");
#endif
                /* We only accept a response if we are in server mode and
                   if the IDs of the challenge and response match */
                if ((ncp_mode == SERVER) &&
                    (challenge_identifier == buf_ptr->data_ptr[LCP_ID_OFFSET]))
                {
                    /* We must check the response to see if it is valid. */
                    status = CHAP_Check_Response (buf_ptr);

                    /* See if the client was successful at logging in. */
                    if (status == NU_TRUE)
                    {
                        /* Tell the client that it passed logging in. */
                        CHAP_Send_Success(buf_ptr);

                        NU_Set_Events (&Buffers_Available,
                                        AUTHENTICATION_SUCCESS, NU_OR);
                    }
                    else
                    {
                        /* Stop the echo timer since we are leaving the open state */
                        NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

                        /* The client failed the authentication. Let it
                           know. */
                        CHAP_Send_Failure(buf_ptr);

                        NU_Set_Events (&Buffers_Available, LCP_CONFIG_FAIL,
                                                                    NU_OR);
                    }
                }

                break;

            case CHAP_SUCCESS :
#ifdef DEBUG_PRINT
                _PRINT ("success ");
#endif

                /* We have received a positive response from the server.
                   If NCP is in need of negotiation let it know, otherwise
                   CHAP is done. */

                if (ncp_state.state != OPENED)
                    NU_Set_Events (&Buffers_Available, AUTHENTICATION_SUCCESS,
                                                                    NU_OR);

                break;

            case CHAP_FAILURE :
#ifdef DEBUG_PRINT
                _PRINT ("failure ");
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
/*    CHAP_MD5_Encrypt                                                      */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function encrypts a string using MD5 encryption.                   */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  CHAP_Respond_To_Challenge                                               */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  strlen                                                                  */
/*  MD5Init                                                                 */
/*  MD5Update                                                               */
/*  MD5Final                                                                */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  char                    string          Pointer to the string to encrypt*/
/*  char                    digest          Pointer to the area in which to */
/*                                          return the encrypted string     */
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
VOID CHAP_MD5_Encrypt (unsigned char *string, unsigned char *digest)
{
  MD5_CTX context;
  unsigned int len = strlen ((char *) string);

  MD5Init (&context);
  MD5Update (&context, string, len);
  MD5Final (digest, &context);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    CHAP_Timer_Expire                                                     */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function handles the authentication timer expiration. If the     */
/* authentication challenge has not been sent more than LCP_MAX_AUTHENTICATE*/
/* times then it will get sent again. Otherwise we will consider that the   */
/* authentication failed.                                                   */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  Nucleus PLUS                                                            */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  NU_Control_Timer                                                        */
/*  NU_Set_Events                                                           */
/*  UTL_Timerset                                                            */
/*  intswap                                                                 */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  UNSIGNED                    dev_ptr     Address of the device structure */
/*                                          for the device that the timer   */
/*                                          is expiring.                    */
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
VOID CHAP_Timer_Expire (UNSIGNED dev_ptr)
{

#ifdef DEBUG_PRINT
    _PRINT ("\nchap timer expire ");
#endif

    /* See if we have waited long enough for a reply. */
    if (!(_num_authentication_timeouts-- > 0))
    {
        /* Since we have not received a response yet we will assume
           that the authentication failed. */

        /* Stop the echo timer since we are leaving the open state */
        NU_Control_Timer (&LCP_Echo_Timer, NU_DISABLE_TIMER);

        NU_Set_Events (&Buffers_Available, LCP_CONFIG_FAIL, NU_OR);
    }
    else
    {

#ifdef DEBUG_PRINT
        _PRINT ("send response again ");
#endif

        if (ncp_mode == CLIENT)
        {
            /* Otherwise set the event that will send the response again */
            UTL_Timerset (CHAP_RESEND, dev_ptr, 0, 0);

        }
        else
        {
            /* Set the event that will send a challenge again */
            UTL_Timerset (CHAP_CHALL, dev_ptr, 0, 0);
        }
    }

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    CHAP_Send_Challenge                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Sends a challenge request to the client that is trying to log into the  */
/* system.                                                                  */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  NU_EventsDispatcher                                                     */
/*  PPP_Lower_Layer_Up                                                      */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  LCP_Random_Number                                                       */
/*  LCP_Random_Number32                                                     */
/*  intswap                                                                 */
/*  sizeof                                                                  */
/*  PPP_TX_Packet                                                           */
/*  NU_Tcp_Log_Error                                                        */
/*  MEM_Buffer_Dequeue                                                      */
/*  MEM_Buffer_Enqueue                                                      */
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
VOID CHAP_Send_Challenge (VOID)
{
    NET_BUFFER          *buf_ptr;
    LCP_LAYER           *lcp_pkt;
    uchar       HUGE    *lcp_pkt_ptr,
                HUGE    *byte_ptr;
    int16               len;

#ifdef DEBUG_PRINT
    _PRINT ("\nchap send challenge ");
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
    lcp_pkt             = (LCP_LAYER *) buf_ptr->data_ptr;

    /* Get a pointer to the data part of the pkt. */
    lcp_pkt_ptr = (uchar *)&lcp_pkt->data;

    /* Init the length */
    len   = 0;

    /* Lets create a challenge pkt. */
    lcp_pkt->code = CHAP_CHALLENGE;

    /* Fill in the ID */
    challenge_identifier = LCP_Random_Number();
    lcp_pkt->identifier = challenge_identifier;

    /* Put in the value size. Ours will be 4 bytes */
    lcp_pkt_ptr [len++] = CHAP_CHALLENGE_VALUE_SIZE;

    /* Get a random value for the challenge value. */
    challenge_value = LCP_Random_Number32();

    /* Put the challenge value in the pkt */
    byte_ptr = (uchar *)&challenge_value;

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

    /* Add on the LCP header to the length and
       put it in the pkt. */
    len             += LCP_HEADER_LENGTH;
    lcp_pkt->length  = intswap (len);

    /* Set the lengths for the packet buffer. */
    buf_ptr->data_len   = buf_ptr->mem_total_data_len = len;

    /* Set the packet type. */
    buf_ptr->mem_flags = NET_CHAP;

    /* Send the pkt. */
    PPP_TX_Packet (buf_ptr->mem_buf_device, buf_ptr);

    /* Release the buffer. */
    MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, buf_ptr);
}


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    CHAP_Respond_To_Challenge                                             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This function responds to a challenge received from the server that */
/* is authenticating. It will encrypt the user password using MD5           */
/* encryption                                                               */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  CHAP_Interpret                                                          */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  strlen                                                                  */
/*  CHAP_MD5_Encrypt                                                        */
/*  intswap                                                                 */
/*  memcpy                                                                  */
/*  NU_Control_Timer                                                        */
/*  dev_start                                                               */
/*  NU_Reset_Timer                                                          */
/*  MEM_Buffer_Dequeue                                                      */
/*  NU_Tcp_Log_Error                                                        */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER                  *buf_ptr    Pointer to the incoming CHAP    */
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
VOID CHAP_Respond_To_Challenge (NET_BUFFER *in_buf_ptr)
{
    NET_BUFFER      *buf_ptr;
    LCP_LAYER       *lcp_pkt;
    uchar   HUGE    *lcp_pkt_ptr;
    uint8           x, pw_id_len, value_len, secret[PPP_MAX_PW_LENGTH +
                                                (CHAP_MD5_VALUE_SIZE * 2)];
    int16           index, len;
    uchar   HUGE    *chap_pkt = in_buf_ptr->data_ptr;

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

    /* Set the data pointer and the reject packet pointer. */
    buf_ptr->data_ptr   = (buf_ptr->mem_parent_packet + sizeof (PPP_HEADER));
    lcp_pkt             = (LCP_LAYER *) buf_ptr->data_ptr;

    /* Get a pointer to the data part of the pkt. */
    lcp_pkt_ptr = (uchar *)&lcp_pkt->data;

    /* Lets create a response pkt with our login and PW. */
    lcp_pkt->code = CHAP_RESPONSE;

    /* The ID is the same as the ID of the challenge pkt. */
    lcp_pkt->identifier = chap_pkt[LCP_ID_OFFSET];

    index = 0;
    len   = 0;

    /* Put in the value size. For MD5 it is 16 */
    lcp_pkt_ptr [len++] = CHAP_MD5_VALUE_SIZE;

    /* Build the string that we will encrypt, this consists of
       the PW, the ID, and a value supplied by the
       authenticator. */

    secret [index++] = lcp_pkt->identifier;

    /* Get the length of the PW to try */
    pw_id_len = strlen (_ppp_login_pw);

    /* Copy the PW over. */
    for (x = 0; x < pw_id_len; x++)
        secret [index++] = _ppp_login_pw [x];

    /* Get the length of the value. */
    value_len = chap_pkt [CHAP_VALUE_LENGTH_OFFSET];

    /* Copy it over. */
    for (x = 0; x < value_len; x++)
        secret [index++] = chap_pkt [CHAP_VALUE_OFFSET + x];

    /* Null terminate it. */
    secret [index] = (char)0;

    /* Encrypt it. */
    CHAP_MD5_Encrypt (secret, (unsigned char *)&lcp_pkt_ptr[len]);

    /* Update the length */
    len += CHAP_MD5_VALUE_SIZE;

    /* Get the length of the ID to try */
    pw_id_len = strlen (_ppp_login_name);

    /* Copy the login name into the pkt. */
    for (x = 0; x < pw_id_len; x++, len++)
        lcp_pkt_ptr[len] = _ppp_login_name[x];

    /* Add on the LCP header to the length and
       put it in the pkt. */
    len             += LCP_HEADER_LENGTH;
    lcp_pkt->length  = intswap (len);

    /* Reset the restart timer */
    NU_Control_Timer (&Authentication_Timer, NU_DISABLE_TIMER);

    /* Set the lengths for the packet buffer. */
    buf_ptr->data_len   = buf_ptr->mem_total_data_len = len;

    /* Set the packet type. */
    buf_ptr->mem_flags = NET_CHAP;

    /* Send the pkt. */
    in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device, buf_ptr);

    /* Start the retransmission timer for it. */
    NU_Reset_Timer (&Authentication_Timer, CHAP_Timer_Expire,
                    LCP_TIMEOUT_VALUE, LCP_TIMEOUT_VALUE, NU_ENABLE_TIMER);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    CHAP_Check_Response                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Checks the ID and encrypted PW against what the encrypted PW should be. */
/* The correct encypted PW is compute from the password list.               */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  CHAP_Interpret                                                          */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  strcmp                                                                  */
/*  CHAP_MD5_Encrypt                                                        */
/*  memcmp                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  NET_BUFFER                  *buf_ptr    Pointer to the incoming CHAP    */
/*                                          packet                          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  STATUS                                  Was the response correct        */
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
STATUS CHAP_Check_Response (NET_BUFFER *buf_ptr)
{
    uchar           client_secret[CHAP_MD5_VALUE_SIZE],
                    our_encrypt_secret[CHAP_MD5_VALUE_SIZE];
    char            client_name[PPP_MAX_ID_LENGTH];
    uchar           *byte_ptr;
    uint8           x, index, pw_index, name_length, found_it, wrong_pw;
    uchar           secret[PPP_MAX_PW_LENGTH + (CHAP_MD5_VALUE_SIZE * 2)];
    uchar   HUGE    *chap_pkt = buf_ptr->data_ptr;

    /* The size of the value must be correct for MD5 encryption. */
    if (CHAP_MD5_VALUE_SIZE == chap_pkt[CHAP_VALUE_LENGTH_OFFSET])
    {
        /* copy out the secret value */
        for (x = 0; x < CHAP_MD5_VALUE_SIZE; x++)
            client_secret [x] = chap_pkt [CHAP_VALUE_OFFSET + x];

        /* Copy out the name of the client. To get its length we must get
           the total length of the pkt and compute what is left. */

        name_length = *(chap_pkt + LCP_LENGTH_OFFSET) << 8;
        name_length =  name_length | *(chap_pkt + LCP_LENGTH_OFFSET + 1);

        /* Now take off the header, value size, and value fields. Note
           that the value size field is only 1 byte, that is the 1 that
           is added below. */
        name_length -= (LCP_HEADER_LENGTH + CHAP_MD5_VALUE_SIZE + 1);

        /* Copy out the name */
        for (x = 0; (x < name_length) && (x < PPP_MAX_ID_LENGTH); x++)
            client_name [x] = chap_pkt [CHAP_VALUE_OFFSET +
                                                CHAP_MD5_VALUE_SIZE + x];
        /* Null it */
        client_name [x] = 0;

        /* Loop through the password list to find this client. */
        for (x = 0, found_it = NU_FALSE, wrong_pw = NU_FALSE;
                (_passwordlist[x].id[0] != 0) && (found_it == NU_FALSE) &&
                (wrong_pw == NU_FALSE); x++)
        {
            /* If the IDs match, then we must compute what the encrypted
               PW should be. */
            if ((strcmp (client_name, _passwordlist[x].id)) == 0)
            {
                index = 0;

                /* First is the pkt ID */
                secret [index++] = challenge_identifier;

                /* Now copy in the password */
                for (pw_index = 0; pw_index < strlen (_passwordlist[x].pw);
                                                                pw_index++)
                    secret [index++] = _passwordlist[x].pw[pw_index];

                /* Lastly put in the challenge value. */
                byte_ptr = (uchar *)&challenge_value;

#ifdef SWAPPING
                secret [index++]  = byte_ptr[3];
                secret [index++]  = byte_ptr[2];
                secret [index++]  = byte_ptr[1];
                secret [index++]  = byte_ptr[0];
#else
                secret [index++]  = byte_ptr[0];
                secret [index++]  = byte_ptr[1];
                secret [index++]  = byte_ptr[2];
                secret [index++]  = byte_ptr[3];
#endif

                /* Compute the expected encrypted password */
                CHAP_MD5_Encrypt (secret, (unsigned char *) our_encrypt_secret);

                /* Compare the two encrypted secrets. They must be the same
                   for a successful login. */
                if (memcmp (our_encrypt_secret, client_secret,
                                        CHAP_MD5_VALUE_SIZE) == 0)
                    found_it = NU_TRUE;
                else
                    wrong_pw = NU_TRUE;

            }


        }

    }

    return (found_it);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    CHAP_Send_Success                                                     */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Sends a CHAP success packet, telling the CLIENT that it has logged in.  */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  CHAP_Interpret                                                          */
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
/*  NET_BUFFER                  *in_buf_ptr Pointer to the incoming CHAP    */
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
VOID CHAP_Send_Success (NET_BUFFER *in_buf_ptr)
{
    NET_BUFFER          *buf_ptr;
    LCP_LAYER   HUGE    *lcp_pkt;
    uint16              len;
    uchar       HUGE    *chap_pkt = in_buf_ptr->data_ptr;

#ifdef DEBUG_PRINT
    _PRINT (" send success");
#endif

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
    buf_ptr->data_ptr   = buf_ptr->mem_parent_packet + sizeof (PPP_HEADER);
    lcp_pkt             = (LCP_LAYER *) buf_ptr->data_ptr;

    /* Fill in the pkt */
    lcp_pkt->code       = CHAP_SUCCESS;

    /* The ID comes from the response pkt */
    lcp_pkt->identifier = chap_pkt [LCP_ID_OFFSET];

    /* Since there is no data in the pkt the size is just that of the
       header. */
    len                 = LCP_HEADER_LENGTH;
    lcp_pkt->identifier = (uchar) intswap (len);

    /* Set the lengths for the packet buffer. */
    buf_ptr->data_len   = buf_ptr->mem_total_data_len = len;

    /* Set the packet type. */
    buf_ptr->mem_flags = NET_CHAP;

    /* Send the pkt. */
    in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device, buf_ptr);

    /* Release the buffer. */
    MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, buf_ptr);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    CHAP_Send_Failure                                                     */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Sends a CHAP failure packet, telling the CLIENT that either the ID or   */
/* PW was wrong and that it can not login.                                  */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_CHAP_Interpret                                                      */
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
/*  NET_BUFFER                  *in_buf_ptr Pointer to the incoming CHAP    */
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
VOID CHAP_Send_Failure (NET_BUFFER *in_buf_ptr)
{
    NET_BUFFER          *buf_ptr;
    LCP_LAYER           *lcp_pkt;
    uint16              len;
    uchar       HUGE    *chap_pkt = in_buf_ptr->data_ptr;

#ifdef DEBUG_PRINT
    _PRINT (" send failure");
#endif

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
    buf_ptr->data_ptr   = buf_ptr->mem_parent_packet + sizeof (PPP_HEADER);
    lcp_pkt             = (LCP_LAYER *) buf_ptr->data_ptr;

    /* Fill in the pkt */
    lcp_pkt->code = CHAP_FAILURE;

    /* The ID comes from the response pkt */
    lcp_pkt->identifier = chap_pkt [LCP_ID_OFFSET];

    /* Since there is no data in the pkt the size is just that of the
       header. */
    len                 = LCP_HEADER_LENGTH;
    lcp_pkt->identifier = (uchar) intswap (len);

    /* Set the lengths for the packet buffer. */
    buf_ptr->data_len   = buf_ptr->mem_total_data_len = len;

    /* Set the packet type. */
    buf_ptr->mem_flags = NET_CHAP;

    /* Send the pkt. */
    in_buf_ptr->mem_buf_device->dev_start (in_buf_ptr->mem_buf_device, buf_ptr);

    /* Release the buffer. */
    MEM_Buffer_Enqueue (&MEM_Buffer_Freelist, buf_ptr);

}

