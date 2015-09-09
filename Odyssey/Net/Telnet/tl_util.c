/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1993 - 1998 by Accelerated Technology, Inc.           */
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
/*  tl_util.c                                                  2.0          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Telnet utilities                                                        */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*  MQ Qian (modified from NCSA code)                                       */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*  NU_Receive_NVT_Key                                                      */
/*  NU_Send_NVT_key                                                         */
/*  received_exit                                                           */
/*  NU_Telnet_Send()                                                        */
/*  NU_Telnet_Init_Parameters()                                             */
/*  NU_Telnet_Free_Parameters()                                             */
/*  NU_Telnet_Client_Connect()                                              */
/*  NU_Telnet_Socket                                                        */
/*  NU_Telnet_Server_Accept                                                 */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*  No other file dependencies                                              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME       DATE      REMARKS                                            */
/*                                                                          */
/*  MQ Qian    01-05-95  initial version 1.0                                */
/*  MQ Qian    01-22-96  modification to pacify compiler warnnings          */
/*  MQ Qian    08-13-96  moved NU_check_Recv_retval into negotiat.c         */
/*  MQ Qian    08-14-96  added NU_Telnet_Socket                             */
/*  D. Sharer  04-14-98  Removed warnings and Modified for Net 4.0          */
/*                                                                          */
/****************************************************************************/

/* c library includes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define WINMASTER

#include "externs.h"
#include "nucleus.h"
#include "socketd.h"    /* socket interface structures */
#include "netevent.h"
#include "tcpdefs.h"
#include "protocol.h"
#include "data.h"
#include "telopts.h"
#include "windat.h"
#include "nkeys.h"
#include "vtkeys.h"
#include "n_ansi.h"
#include "nvt.h"
#include "tel_extr.h"

#define NK_TELNET   23

int tl_test=0;

extern struct port *portlist[];
extern struct sock_struct *socket_list[];

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Receive_NVT_Key                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  translate a special terminal emulation key to a local key value and  */
/*  explain it, the users can modify this function to match their onw    */
/*  typical environment                                                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*  MQ Qian, (modified) ATI                                              */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  string - the key string to be converted                              */
/*  do     - indicate whether execute the function associated with the   */
/*          function key or not (just do mapping)                        */
/*                                                                       */
/* RETURNS                                                               */
/*                                                                       */
/*   the length of the key string                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/* CALLED                                                                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         02-02-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
int NU_Receive_NVT_Key(int16 socket, char *string, char execute)
{
    char len, i=0;
    struct twin *temp_twin;

    /* get the pointer of the parameter data struct of the current sesseion */
    temp_twin = tn_session[socket];

    if (temp_twin->termstate!=VT100TYPE)
        return(0);

    len = strlen(vt100keys[i].key_string);
    while (len)
    {

        if (memcmp(string, vt100keys[i].key_string, len)==0)
        {
            /* the users, who define certain function for these keys, need
              add code here to call the functions associated with the function
              key , the 'execute' flag allows the user to switch whether execute
              the function or not (just do mapping), before enter this routine.
              */

            if (execute)
                message_print("%d", vt100keys[i].key_value);
            return(len);
        }
        else
            len = strlen(vt100keys[++i].key_string);
    }
    message_print("\nWe don't know this keystring (%c%c%c), now.",
        string[0], string[1], string[2]);

    return(0);
} /* NU_Receive_NVT_Key() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Send_NVT_key                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  takes a key value and checks whether it is a mapped key, then checks */
/*  whether it is a 'special' key (i.e. vt100 keys, and maybe other      */
/*  kermit verbs), pass the characters on to the TCP port in pnum.       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*  MQ Qian, (modified) ATI                                              */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  socket  - the index of socket of the telnet connection               */
/*  c - the keycode to check, convert and send                           */
/*                                                                       */
/* RETURNS                                                               */
/*                                                                       */
/*  NULL for a match not found,                                          */
/*  or a pointer to the key_node containing the matched keycode          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/* CALLED                                                                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-03-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
void NU_Send_NVT_key(int16 socket, unsigned int c)
{
    int i=0;
    struct twin *temp_twin;

    /* get the pointer of the parameter data struct of the current sesseion */
    temp_twin = tn_session[socket];
    /* if the key is not mapped, just send it. And, make certain it is
        an ascii char, or we are transmitting binary data */
    if (c<128 || (c<=255 && temp_twin->ibinary))
    {
        if (c==IAC && temp_twin->ibinary)
        /* if in binary mode and it happens that byte value is 0xFF, in order
        to avoid to confuse with IAC, the starter of telnet commands, send IAC
        one more time, NU_Telnet_Parse() will knows that this IAC is a binary
        data */
            NU_Send(socket, (char *)&c, 1, 0);

        NU_Send(socket, (char *)&c, 1, 0);
        return;
    }

    /* if terminal emulation is negotiated and VT100 is supported, we do it.
        the users need to modify this code to match specific application. */
    if (temp_twin->termstate==VT100TYPE)
    {
        while (vt100keys[i].key_value)
        {
            if (c==(unsigned int)vt100keys[i].key_value)
            {
                NU_Send(socket, vt100keys[i].key_string,
                    (uint16)(strlen(vt100keys[i].key_string)), 0);
                return;
            }
            else
                i++;
        }
        message_print("\nWe can't take care this keystroke (%d), now.", c);
    }
}   /* end NU_Send_NVT_key() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*  NU_Telnet_Send                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Write printf()-like formatted data into an output queue.             */
/*  Calls NU_Send to send these data to network.                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*    socket                                                             */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*   the number of bytes sent, or <0 if an error                         */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*   NU_Send()                                                           */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*    User application                                                   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-11-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
int NU_Telnet_Send(int16 socket, char *fmt, ... )
{
    char temp_str[256];     /* do not input a string longer than 255 !!! */
    va_list arg_ptr;
    int str_len;

    va_start(arg_ptr, fmt);
    str_len = vsprintf(temp_str, fmt, arg_ptr);

    if (str_len>255)
    {
        message_print("too long (%d) string in NU_Telnet_Send() !", str_len);
        NU_TN_exit(1);
    }

    va_end(arg_ptr);
    if (str_len>0)      /* good string to transmit */
        return(NU_Send(socket, temp_str, (uint16)str_len, 0));
    else
        return(-3);
}   /* end NU_Telnet_Send() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*    received_exit                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  check "exit" command from client                                     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  data    - the input string to be parsed                              */
/*  cnt     - the length of the string                                   */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*      0 - no exit command found,  1 - find exit command                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/* CALLED                                                                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-18-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
int received_exit(char *data, int cnt)
{
    static  char exit=0, e[5]="exit";

    if (cnt==4)
    {
        if (memcmp(data, e, 4)==0)
            return(1);
    }
    else if (cnt==1)
    {
        if (e[exit]==data[0])
        {
            if (++exit==4)
                return(1);
        }
        else
            exit = 0;
    }

    return(0);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*  NU_Telnet_Free_Parameters                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  free the data struct pointer of the parameters of a telnet session   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  socket  - the index of socket of the telnet connection               */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*   The user's applications                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-05-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
void NU_Telnet_Free_Parameters(int16 socket)
{
    /*  Clear this telnet session parameter list entry.  */
    NU_Deallocate_Memory((uint *) tn_session[socket]);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*  NU_Telnet_Init_Parameters                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  initialize all parameters of a telnet session                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  socket  - the index of socket of the telnet connection               */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*   The user's applications                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-05-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
void NU_Telnet_Init_Parameters(int16 socket)
{
    struct twin *p;
    int status;

    /* get the momery for the parameter data struct of the current sesseion */
    status = NU_Allocate_Memory(&System_Memory, (void *) &tn_session[socket],
                (UNSIGNED)(sizeof(struct twin)), (UNSIGNED)NU_NO_SUSPEND);
    if (status!=NU_SUCCESS)
    {
        message_print("\n\rcan't allocate memory for tn_session[%d].", socket);
        NU_TN_exit(50);
    }

    p = tn_session[socket];
    p->pnum = -1;
    p->socket = socket;
    p->nego_NAWS = 0;       /* the flag about the negotiation on NAWS */
    p->nego_TERMTYPE = 0;   /* the flag about the negotiation on TERMTYPE */
    p->telstate = 0;
    p->substate = 0;
    p->termsent = 0;
    p->termstate = VTTYPE; /*basetype;*/
    p->linemode[0] = 0;
    p->echo = 1;
    p->ibinary = 0;       /* I'm sending NVT ASCII data */
    p->iwantbinary = 0;   /* I did not ask for binary transmission from me */
    p->ubinary = 0;       /* Server is sending NVT ASCII data */
    p->uwantbinary = 0;   /* I did not ask for server to begin binary transmission */
    p->ugoahead = 0;      /* we want goahead suppressed */
    p->igoahead = 0;        /* we want goahead suppressed */
    p->timing = 0;
    p->next = NULL;
    p->prev = NULL;

    p->bksp = 127;
    p->del = 8;
    p->crfollow = 0;
    p->rows = 24;
    p->width = 80;
}   /* end NU_Telnet_Init_Parameters() */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*   NU_check_Close_retval                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  check the return value of NU_Recv()                                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  retval  the return value from NU_Recv()                              */
/*  bytes   the bytes number to check                                    */
/*  excuse  the excuse not to exit                                       */
/*  where   the message tell who calls NU_Recv()                         */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*   the user's application                                              */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-20-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
int NU_Close_and_check_retval(int16 socket, char *who)
{
    int retval;

    if ((retval=NU_Close_Socket(socket))==NU_SUCCESS)
        message_print("\n%s closed this connection.", who);
    else
        message_print("\nError (%d) from NU_Close_Socket.", retval);

    return(retval);
}    /* NU_Close_and_check_retval() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*   NU_Telnet_Client_Connect                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Open a telnet connection with server                                 */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  server_ip - telnet server ip                                         */
/*  server_name - telnet server name                                     */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*  NU_Socket()                                                          */
/*  NU_Allocate_Memory ()                                                */
/*  NU_Connect()                                                         */
/*  NU_Close_Socket()                                                    */
/*                                                                       */
/* CALLED                                                                */
/*                                                                       */
/*      NU_Telnet_Client                                                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-05-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
int NU_Telnet_Client_Connect(char *server_ip, char *server_name)
{
    int16 socket;                    /* the socket descriptor */
    struct addr_struct  *servaddr;  /* holds the server address structure */
    unsigned int        *return_ptr;
    int                 status;

    /* open a connection via the socket interface */
    if ((socket=NU_Socket(NU_FAMILY_IP, NU_TYPE_DGRAM, 0))<0)
    {
        /*  Can't get socket for telnet, get out.  */
        message_print("Cannot get socket for telnet\n\r");
        NU_TN_exit(1);
    }

    status = NU_Allocate_Memory (&System_Memory, (void *) &return_ptr,
                                    sizeof(struct addr_struct), NU_SUSPEND);
    if (status!=NU_SUCCESS)
    {
        /*  Can't allocate memory, get out.  */
        message_print("Cannot allocate memory for addr_struct\n\r");
        NU_TN_exit(1);
    }

    servaddr = (struct addr_struct *)return_ptr;
    /* fill in a structure with the telnet server address */
    servaddr->family    = NU_FAMILY_IP;
    servaddr->port      = NK_TELNET;
    servaddr->id.is_ip_addrs[0]  = server_ip[0];
    servaddr->id.is_ip_addrs[1]  = server_ip[1];
    servaddr->id.is_ip_addrs[2]  = server_ip[2];
    servaddr->id.is_ip_addrs[3]  = server_ip[3];
    servaddr->name = server_name;

    if ((socket=NU_Connect(socket, servaddr, 0))>=0)
    {
        /* start the negotiation
        NU_Telnet_Start_Negotiate(socket, current, client_nego_table); */

        return(socket);
    }
    else
        return(-1);
} /* NU_Telnet_Client_Connect */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*   NU_Telnet_Socket                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Create the first socket for TELNET Server                            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  server_ip - telnet server ip                                         */
/*  server_name - telnet server name                                     */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*  NU_Socket()                                                          */
/*  NU_Allocate_Memory ()                                                */
/*  NU_Bind()                                                            */
/*  NU_Listen()                                                          */
/*                                                                       */
/* CALLED                                                                */
/*                                                                       */
/*   NU_Telnet_Debug_Server                                              */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  NAME        DATE        REMARKS                                      */
/*                                                                       */
/*  MQ Qian     08-14-96  initial version 1.0                            */
/*                                                                       */
/*************************************************************************/
int NU_Telnet_Socket(char *server_ip, char *server_name)
{
    int16  socket=-1;                           /* the socket descriptor */
    struct addr_struct  *servaddr;              /* holds the server address structure */
    void                *pointer;
    int                 status;

    /* open a connection via the socket interface */
    if ((socket=NU_Socket(NU_FAMILY_IP, NU_TYPE_STREAM, 0))<0)
        return(-1);

    status = NU_Allocate_Memory(&System_Memory, &pointer,
                    sizeof(struct addr_struct), NU_SUSPEND);
    if (status!=NU_SUCCESS)
        return(-1);

    servaddr = (struct addr_struct *)pointer;

     /* fill in a structure with the server address */
    servaddr->family    = NU_FAMILY_IP;
    servaddr->port      = NK_TELNET;
    servaddr->id.is_ip_addrs[0]  = server_ip[0];
    servaddr->id.is_ip_addrs[1]  = server_ip[1];
    servaddr->id.is_ip_addrs[2]  = server_ip[2];
    servaddr->id.is_ip_addrs[3]  = server_ip[3];
    servaddr->name = server_name;

    /* make an NU_Bind() call to bind the server's address */
    if ((NU_Bind(socket, servaddr, 0))>=0)
    {
        /* be ready to accept connection requests */
        status = NU_Listen(socket, 10);
        if (status == NU_SUCCESS)
            return(socket);
    }

    return(-1);
} /* NU_Telnet_Socket */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*   NU_Telnet_Server_Accept                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Waiting for a telnet connection from client                          */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  socket  - the first template socket for this server                  */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*  NU_Accept()                                                          */
/*                                                                       */
/* CALLED                                                                */
/*                                                                       */
/*   NU_Telnet_Debug_Server                                              */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  NAME        DATE        REMARKS                                      */
/*                                                                       */
/*  MQ Qian     01-17-1995  initial version 1.0                          */
/*  MQ Qian     01-16-1996  Changed client_addr to structure             */
/*  MQ Qian     08-14-96  seperated socket part as NU_Telnet_Socket      */
/*                                                                       */
/*************************************************************************/
int NU_Telnet_Server_Accept(int16 socket)
{
    int16               newsock;    /* the new socket descriptor */
    struct addr_struct  client_addr;

    /* block in NU_Accept until a client attempts connection */
    newsock = NU_Accept(socket, &client_addr, 0);
    if (newsock >= 0)
    {
        /* FOR TEST ONLY */
        printf("\nClient has connected.\n ");
        /* process the new connection */

        return(newsock);
    } /* end successful NU_Accept */

    return(-1);
} /* NU_Telnet_Server_Accept */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*  NU_Telnet_Check_Connection                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Check the connection on the socket is closed or not, if it is still  */
/*  opening and parameter close_it=1, close.                             */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  server_ip - telnet server ip                                         */
/*  server_name - telnet server name                                     */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*  NU_Socket()                                                          */
/*  NU_GetPnum                                                           */
/*  NU_Close_Socket()                                                    */
/*                                                                       */
/* CALLED                                                                */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-17-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
int NU_Telnet_Check_Connection(int16 socket, int close_it)
{
    struct sock_struct  *sockptr;
    int                 cnt, pnum;

    /*  Validate the socket number.  */
    if (socket<0 || socket>=NSOCKETS || (sockptr=socket_list[socket])==NU_NULL)
        return(SCLOSED);

    /* get the index of portlist of this connection. */
    if ((pnum=NU_GetPnum(sockptr))<0)
        return(SCLOSED);

    if (portlist[pnum]->state==SCLOSED)
    {
        message_print("\nThe connection is stopped.");
        return(SCLOSED);
    }
    else if (close_it)
    {
        if ((cnt=NU_Close_Socket(socket))==NU_SUCCESS)
        {
            message_print("\nNU_Close closed this connection.");
            return(SCLOSED);
        }
        else
        {
            message_print("\nError (%d) from NU_Close_Socket.", cnt);
            return(cnt);
        }
    }
    else
        return(NU_FALSE);
}  /* NU_Telnet_Check_Connection */


