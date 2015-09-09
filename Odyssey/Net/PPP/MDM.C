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
/*    MDM.C                                                  2.0            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains the modem functions.                               */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Glen Johnson                                                          */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*     MDM_Init                                                             */
/*     MDM_Receive                                                          */
/*     MDM_Data_Ready                                                       */
/*     MDM_Get_Char                                                         */
/*     MDM_Control_String                                                   */
/*     MDM_Delay                                                            */
/*     MDM_Dial                                                             */
/*     MDM_Get_Modem_String                                                 */
/*     MDM_Hangup                                                           */
/*     MDM_Purge_Input_Buffer                                               */
/*     MDM_Change_Communication_Mode                                        */
/*     MDM_Wait_For_Client                                                  */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*     string.h                                                             */
/*     stdlib.h                                                             */
/*     nucleus.h                                                            */
/*     externs.h                                                            */
/*     tcp_errs.h                                                           */
/*     urt_extr.h                                                           */
/*     urt_defs.h                                                           */
/*     mdm_defs.h                                                           */
/*     mdm_extr.h                                                           */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
#include <string.h>
#include <stdlib.h>
#include "nucleus.h"
#include "externs.h"
#include "tcp_errs.h"
#include "mdm_extr.h"
#include "mdm_defs.h"
#include "urt_defs.h"
#include "urt_extr.h"
#include "Odyssey_Trace.h"

MDM_BUFFER      MDM_Recv_Buffer;
CHAR            MDM_Dial_Prefix[41];

extern void SendHangup();

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Init                                                              */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function initializes the MODEM module.                           */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_Initialize                                                        */
/*    SLIP_Initialize                                                       */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Allocate_Memory                                                    */
/*    NU_Tcp_Log_Error                                                      */
/*    strcpy                                                                */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                                Returns NU_SUCCESS if successful*/
/*                                          initialization, else a negative */
/*                                          value is returned.              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS MDM_Init(VOID)
{

    /* Allocate a block of memory for the receive buffer.  Not that this buffer
       is only used when the driver is being used in terminal mode.  */
    if (NU_Allocate_Memory (&System_Memory,
                            (VOID **)&MDM_Recv_Buffer.mdm_head,
                            MDM_RECV_BUFFER_SIZE,
                            NU_NO_SUSPEND
                           ) != NU_SUCCESS)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL,
                          __FILE__, __LINE__);
        return(-1);
    }

    MDM_Recv_Buffer.mdm_read =
        MDM_Recv_Buffer.mdm_write =
        MDM_Recv_Buffer.mdm_head;

    MDM_Recv_Buffer.mdm_tail = MDM_Recv_Buffer.mdm_head +
                               MDM_RECV_BUFFER_SIZE - 1;

    MDM_Recv_Buffer.mdm_buffer_status = MDM_BUFFER_EMPTY;

    strcpy(MDM_Dial_Prefix, MDM_DIAL_PREFIX);

    return (NU_SUCCESS);

} /* end MDM_Init */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Receive                                                           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function receives characters whenever operating in terminal mode */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    The UART receive LISR                                                 */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    UART_Get_Char                                                         */
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
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
VOID MDM_Receive(VOID)
{

//	Tracef("MDM_Receive\n");
    if (MDM_Recv_Buffer.mdm_buffer_status != MDM_BUFFER_FULL)
    {
        /* Get the character. */
        *MDM_Recv_Buffer.mdm_write = UART_Get_Char();

        /* Point to the location where the next character will be written. */
        MDM_Recv_Buffer.mdm_write++;

        /* Does the write pointer need to be wrapped around to the start of the
           ring buffer. */
        if (MDM_Recv_Buffer.mdm_write > MDM_Recv_Buffer.mdm_tail)
            MDM_Recv_Buffer.mdm_write = MDM_Recv_Buffer.mdm_head;

        /* Set the status field.  We just added some data so the buffer is
           either full or contains at least one byte of data. */
        if(MDM_Recv_Buffer.mdm_write == MDM_Recv_Buffer.mdm_read)
            MDM_Recv_Buffer.mdm_buffer_status = MDM_BUFFER_FULL;
        else
            MDM_Recv_Buffer.mdm_buffer_status = MDM_BUFFER_DATA;
    }
    else
    {
        /* There is no space for the character, so drop it. */
        UART_Get_Char();
    }
} /* end MDM_Receive */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Data_Ready                                                        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function checks to see if there are any characters in the        */
/*    receive buffer.  A status value is returned indicating the whether    */
/*    characters are present in the receive buffer.                         */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    MDM_Get_Modem_String                                                  */
/*    MDM_Wait_For_Client                                                   */
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
/*    STATUS                                The status indicates the        */
/*                                          presence of characters.         */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS MDM_Data_Ready(VOID)
{
    if((MDM_Recv_Buffer.mdm_buffer_status == MDM_BUFFER_DATA) ||
       (MDM_Recv_Buffer.mdm_buffer_status == MDM_BUFFER_FULL))
    {
        return NU_TRUE;
    }
    else
        return NU_FALSE;

} /* end MDM_Data_Ready */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Get_Char                                                          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function returns the next character that is in the receive       */
/*    buffer.                                                               */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    MDM_Get_Modem_String                                                  */
/*    MDM_Wait_For_Client                                                   */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    CHAR                  *c              Pointer to a location to store  */
/*                                          the RX character.               */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                                NU_SUCCESS if a character is    */
/*                                          available. NU_NO_DATA if no     */
/*                                          characters are available.       */
/*                                          NU_INVALID_POINTER if c is NULL */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS MDM_Get_Char(CHAR *c)
{

    if (c == NU_NULL)
        return (NU_INVALID_POINTER);

    /* IS there any data available. */
    if ((MDM_Recv_Buffer.mdm_buffer_status == MDM_BUFFER_DATA) ||
       (MDM_Recv_Buffer.mdm_buffer_status == MDM_BUFFER_FULL))
    {
        /* Store the character to be returned. */
        *c = *MDM_Recv_Buffer.mdm_read;

        /* Point to the next character to be removed from the buffer. */
        MDM_Recv_Buffer.mdm_read++;

        /* Does the read pointer need to be wrapped around to the start of the
           buffer. */
        if (MDM_Recv_Buffer.mdm_read > MDM_Recv_Buffer.mdm_tail)
            MDM_Recv_Buffer.mdm_read = MDM_Recv_Buffer.mdm_head;

        /* Set the status field.  We just removed some data so the buffer is
           either empty or is at least one byte of short of FULL. */
        if(MDM_Recv_Buffer.mdm_write == MDM_Recv_Buffer.mdm_read)
            MDM_Recv_Buffer.mdm_buffer_status = MDM_BUFFER_EMPTY;
        else
            MDM_Recv_Buffer.mdm_buffer_status = MDM_BUFFER_DATA;
    }
    else
    {
        return (NU_NO_DATA);
    }

    return (NU_SUCCESS);
} /* end MDM_Get_Char */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Control_String                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function is used to send control codes to the modem.             */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    MDM_Dial                                                              */
/*    MDM_Hangup                                                            */
/*    MDM_Wait_For_Client                                                   */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    MDM_Delay                                                             */
/*    UART_Put_Char                                                         */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    CHAR                      *string     String containing the control   */
/*                                          code to be sent.                */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                                NU_SUCCESS         If successful*/
/*                                          NU_INVALID_POINTER If string    */
/*                                          pointer is null.                */
/*                                                                          */
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS MDM_Control_String(CHAR *string)
{
    if (string == NU_NULL)
        return NU_INVALID_POINTER;

    while (*string) {                        /* loop for the entire string */
        switch (*string) {
			case '~':					/* if ~, wait a half second */
                MDM_Delay(500);
				break;
			default:
                switch (*string) {
					case '^':			/* if ^, it's a control code */
                        if (string[1]) {     /* send the control code */
                            string++;
                            UART_Put_Char((CHAR)(*string - 64));
						}
						break;
					default:
                        UART_Put_Char(*string); /* send the character */
				}
                MDM_Delay(100);              /* wait 100 ms. for slow modems */
		}
        string++;                            /* bump the string pointer */
	}

    return NU_SUCCESS;

} /* MDM_Control_String */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Delay                                                             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function is used to simply consume CPU time.                     */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    MDM_Control_String                                                    */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Sleep                                                              */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    UNSIGNED              milliseconds    Time in milliseconds to delay.  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
VOID MDM_Delay(UNSIGNED milliseconds)
{
    UNSIGNED ticks;

    /* Compute the approximate number of ticks.  This value does not have to be
       exactly on target. */
    ticks = (TICKS_PER_SECOND * milliseconds)/1000;

    /* Sleep for at least one tick. */
    if (!ticks)
        ticks = 1;

    NU_Sleep(ticks);

} /* MDM_Delay */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Dial                                                              */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function commands the modem to dial a phone number.              */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_Dial                                                              */
/*    SLIP_Dial                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    MDM_Control_String                                                    */
/*    MDM_Get_Modem_String                                                  */
/*    MDM_Change_Communication_Mode                                         */
/*    strcpy                                                                */
/*    NU_Sleep                                                              */
/*    strstr                                                                */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    CHAR                  *number         Pointer to a string that        */
/*                                          contains the number to dial.    */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                                NU_SUCCESS is returned if       */
/*                                          connection to remote machine    */
/*                                          is made, else a negtive value   */
/*                                          is returned.                    */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS MDM_Dial(CHAR *number)
{
    CHAR dial_string[81], response[81], *baud_start;
    uint32 baud_rate;
    STATUS status;

    if (number == NU_NULL)
        return(NU_INVALID_POINTER);

    /* Change to terminal  mode */
    MDM_Change_Communication_Mode(MDM_TERMINAL_COMMUNICATION);

    /* Purge any characters that were already in the receive buffer. */
    MDM_Purge_Input_Buffer();

    strcpy(dial_string, MDM_Dial_Prefix);
    strcpy(&dial_string[strlen(dial_string)], number);
    strcpy(&dial_string[strlen (dial_string)], "^M");

    MDM_Control_String(dial_string);               /* dial the number */

    /* The modem will return the string with a CR on the end, 0xD is
       the ASCII code for a CR. */
    strcpy(dial_string, MDM_Dial_Prefix);
    strcpy(&dial_string[strlen(dial_string)], number);
    dial_string [strlen(dial_string) + 1] = 0;
    dial_string [strlen(dial_string)] = 0xD;

    MDM_Get_Modem_String(response, dial_string);   /* get a response */

    /* get a pointer to the connect string */
    baud_start = strstr(response, "CONNECT");

    /* connection was made */
    if (baud_start)
    {

#ifdef DEBUG_PRINT

        /* read the baud rate for printing purposes */
        baud_rate = atol(baud_start + 8);

        _PRINT ("\nmodem connected at %ld bps\n", baud_rate);
#endif

        /* Set the status to success since the modem has connected. */
        status = NU_SUCCESS;

        /* We must wait a little to let the server begin its session. */
        NU_Sleep(TICKS_PER_SECOND * 2);

    }
    else
        if (strstr(response, "NO CARRIER") != NULL)
            status = NU_NO_CARRIER;
        else
            if (strstr(response, "BUSY") != NULL)
                status = NU_BUSY;
            else
                status = NU_NO_CONNECT;

    return (status);

} /* MDM_Dial */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Get_Modem_String                                                  */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function retrieves the responses from the modem after the modem  */
/*    has been issued a command.                                            */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    MDM_Dial                                                              */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Retrieve_Clock                                                     */
/*    MDM_Data_Ready                                                        */
/*    MDM_Get_Char                                                          */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  CHAR                    *response       Pointer to a location to store  */
/*                                          the modems response.            */
/*  CHAR                    *dial_string    Pointer to the string that was  */
/*                                          sent to the modem.              */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  CHAR                    *               Pointer to a location to store  */
/*                                          the modems response.            */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
CHAR *MDM_Get_Modem_String(CHAR *response, CHAR *dial_string)
{
    CHAR            c;
    int32           time;
    int8            got_dial_string;


    *response = 0;
    got_dial_string = NU_FALSE;

    time = NU_Retrieve_Clock();

    time += 50 * TICKS_PER_SECOND;

    /* Timeout if a response is not received within the specified period of
       time.  Note the test below accounts for the wrapping of the clock. */
    while ((time - (int32)NU_Retrieve_Clock()) > 0)
    {
		/* get a character from the port if one's there */
        if (MDM_Data_Ready())
        {
            /* Retrieve the next character. */
            MDM_Get_Char(&c);

            switch (c)
            {
                case 0xD:                /* CR - return the result string */

                    /* The first CR is from the dial string. If we got it
                       already then return the received string.  */
                    if ((got_dial_string) && (*response))
                        return response;
                    else

                        /* Otherwise set the flag stating that we got
                           the CR from dialing. */
                        got_dial_string = NU_TRUE;

                default:
                    if (c != 10)
                    {      /* add char to end of string */
                        response[strlen(response) + 1] = 0;
                        response[strlen(response)] = (char)c;
						/* ignore RINGING and the dial string */
                        if ( !strcmp(response, "RINGING") ||
                             !strcmp(response, dial_string) )
                        {
                            *response = 0;
                        }
					}
			}
		}
	}

    return response;

} /* MDM_Get_Modem_String */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Hangup                                                            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function does exactly what you would think, hangsup the modem.   */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    NU_EventsDispatcher                                                   */
/*    LCP_Interpret                                                         */
/*    LCP_Ack_Sent_State                                                    */
/*    LCP_Req_Sent_State                                                    */
/*    LCP_Ack_Recv_State                                                    */
/*    LCP_Open_State                                                        */
/*    LCP_Closing_State                                                     */
/*    LCP_Stopping_State                                                    */
/*    LCP_Send_Echo_Req                                                     */
/*    PPP_NCP_IP_Interpret                                                  */
/*    NCP_IP_Req_Sent_State                                                 */
/*    NCP_IP_Ack_Rcvd_State                                                 */
/*    NCP_IP_Ack_Sent_State                                                 */
/*    NCP_IP_Stopping_State                                                 */
/*    PPP_Hangup                                                            */
/*    PPP_Dial                                                              */
/*    PPP_Wait_For_Client                                                   */
/*    SLIP_Hangup                                                           */
/*    SLIP_Dial                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Retrieve_Clock                                                     */
/*    UART_Set_DTR                                                          */
/*    UART_Carrier                                                          */
/*    MDM_Purge_Input_Buffer                                                */
/*    MDM_Control_String                                                    */
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
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
VOID MDM_Hangup(VOID)
{

    UNSIGNED wait_time;
	Tracef("MDM_Hangup\n");
    /* drop DTR */
    UART_Set_DTR(NU_FALSE);

    wait_time = NU_Retrieve_Clock() + (10 * TICKS_PER_SECOND);

    /* loop till loss of carrier */
    while (UART_Carrier() && ( wait_time > NU_Retrieve_Clock()) )
    {
        NU_Sleep(1);
	}

    /* raise DTR */
    UART_Set_DTR(NU_TRUE);

    /* return if disconnect */
    if (!UART_Carrier())
    {
        MDM_Purge_Input_Buffer();
		// Signal DdmPPP
//		Tracef("Sending hangup message..\n");
//		SendHangup();
//		Tracef("Hangup message sent..\n");	
		Tracef("End MDM_Hangup()\n");
		return;
	}
    MDM_Control_String(MDM_HANGUP_STRING); /* send software command */

    wait_time = NU_Retrieve_Clock() + (10 * TICKS_PER_SECOND);

    /* loop till loss of carrier */
    while (UART_Carrier() && ( wait_time > NU_Retrieve_Clock()))
    {
        NU_Sleep(1);
    }

    MDM_Purge_Input_Buffer();
    
    UART_Change_Communication_Mode(MDM_OTHER_COMMUNICATION);

	Tracef("End MDM_Hangup()\n");
	
	// Signal DdmPPP
//	Tracef("Sending hangup message..\n");
//	SendHangup();
//	Tracef("Hangup message sent..\n");	

} /* MDM_Hangup */


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Purge_Input_Buffer                                                */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function resets the receive buffer, purging all characters that  */
/*    still remain in the buffer.                                           */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    MDM_Hangup                                                            */
/*    MDM_Change_Communication_Mode                                         */
/*    Application                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Control_Interrupts                                                 */
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
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
VOID MDM_Purge_Input_Buffer(VOID)
{
    INT     prev_value;

    prev_value = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* Reset the read and write pointers. */
    MDM_Recv_Buffer.mdm_read =
        MDM_Recv_Buffer.mdm_write =
        MDM_Recv_Buffer.mdm_head;

    /* Update the buffer status to empty. */
    MDM_Recv_Buffer.mdm_buffer_status = MDM_BUFFER_EMPTY;

    NU_Control_Interrupts(prev_value);

}   /* MDM_Purge_Input_Buffer */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Change_Communication_Mode                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*   This function changes the communication mode that the UART is operating*/
/*    in.  There are two modes.  Terminal communication mode might be used  */
/*    to log into a dailup router. Network mode is for exchanging IP packets*/
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Applications                                                          */
/*    PPP_Dial                                                              */
/*    PPP_Wait_For_Client                                                   */
/*    SLIP_Dial                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    MDM_Purge_Input_Buffer                                                */
/*    UART_Change_Communication_Mode                                        */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    INT   :  The mode of operation desired.                               */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS :  Indicates success or failure of the operation.              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  G. Johnson              01/09/97        Created initial version.        */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
STATUS MDM_Change_Communication_Mode(INT mode)
{
    /* Is the mode valid. */
    if( (mode != MDM_NETWORK_COMMUNICATION) &&
        (mode != MDM_TERMINAL_COMMUNICATION) )
        return (NU_INVALID_MODE);

    /* Clear the input buffer. */
    MDM_Purge_Input_Buffer();

    /* Switch the mode in the UART component as well. */
    UART_Change_Communication_Mode(mode);

    return NU_SUCCESS;

}   /* MDM_Change_Communication_Mode */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    MDM_Wait_For_Client                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function waits for a client to call. If only returns once a        */
/* successful modem connection is made.                                     */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  PPP_Wait_For_Client                                                     */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  NU_Sleep                                                                */
/*  MDM_Control_String                                                      */
/*  MDM_Change_Communication_Mode                                           */
/*  MDM_Data_Ready                                                          */
/*  MDM_Get_Char                                                            */
/*  strstr                                                                  */
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
/*  NAME                    DATE             REMARKS                        */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97        Created initial version         */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus         */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
VOID MDM_Wait_For_Client (VOID)
{

    char    mdm_string [81];
    char    rx_char;
    int     index, connected;
    uint32  baud_rate;

#ifdef DEBUG_PRINT
    _PRINT ("\nwaiting for client ");
#endif

    /* We are not currently connected. */
    connected = NU_FALSE;
    index     = 0;

    /* wait for modem */
    NU_Sleep (TICKS_PER_SECOND);

    /* Change to terminal mode so we can talk to the modem. */
    MDM_Change_Communication_Mode (MDM_TERMINAL_COMMUNICATION);

    /* Tell the modem to accept a caller. */
    MDM_Control_String (MDM_ACCEPT_CALL);

    /* Clear out the modem buffer. */
    MDM_Purge_Input_Buffer();

    /* Wait for a caller. */
    while (!connected)
    {

        /* wait for modem */
        NU_Sleep (TICKS_PER_SECOND);

        while (MDM_Data_Ready())
        {
            /* Get the char that came in */
            if (MDM_Get_Char (&rx_char) == NU_SUCCESS)
            {
                if ((index + 1) >= 80)
                    index = 0;

                /* Put it in our buffer so we can look at it. */
                mdm_string [index++] = rx_char;

                /* Null the end. */
                mdm_string [index]   = 0;

                /* See if we got a connection. */
                if (strstr (mdm_string, "CONNECT"))
                {

#ifdef DEBUG_PRINT
                    /* Wait for the baud rate. */
                    NU_Sleep(TICKS_PER_SECOND / 18);

                    index = 0;

                    /* read in the baud rate */
                    while (MDM_Data_Ready() && (index <= 80))
                    {
                        MDM_Get_Char (&rx_char);
                        mdm_string[index++] = rx_char;
                        mdm_string[index] = 0;
                    }

                    /* convert the baud rate to a number */
                    baud_rate = atol(mdm_string);

                    _PRINT ("\nmodem connected at %ld bps\n", baud_rate);
#endif

                    /* Set this flag to true so that we will fall out of
                       the loop and exit this function. */
                    connected = NU_TRUE;
                }
            }
        }
    }
}


