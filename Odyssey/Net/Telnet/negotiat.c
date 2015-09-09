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
/*                                                                          */
/*                                                                          */
/* Portions of NU_Telnet_Parse() were written by:                           */
/*****************************************************************************
*                                                                           *
*     part of:                                                              *
*     TCP/IP kernel for NCSA Telnet                                         *
*     by Quincey Koziol                                                     *
*                                                                           *
*     National Center for Supercomputing Applications                       *
*     152 Computing Applications Building                                   *
*     605 E. Springfield Ave.                                               *
*     Champaign, IL  61820                                                  *
*/
/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*  negotiat.c                                                 2.0          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Telnet negotiation utilities                                            */
/*                                                                          */
/*  in this file, there are two points, the users may need modify them to   */
/*  match their specific situation                                          */
/*  1. about negotiation, here we only follow the NCSA template             */
/*  2. about the check and explanation of special terminal-dependent key    */
/*    '[' and 'O' are the second byte of vt100 key code, however, different */
/*    terminal must have different mapping and convetion                    */
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
/*  parsewrite()                                                            */
/*  NU_Telnet_Start_Negotiate()                                             */
/*  NU_Telnet_Special_Negotiate()                                           */
/*  nu_parse_subnegotiat()                                                  */
/*  NU_Telnet_Parse()                                                       */
/*  NU_Telnet_Get_Filtered_Char()                                           */
/*  NU_Telnet_Pick_Up_Ascii()                                               */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME       DATE      REMARKS                                            */
/*                                                                          */
/*  MQ Qian    01-05-95  initial version 1.0                                */
/*  MQ Qian    01-22-96  modification to pacify compiler warnnings          */
/*  MQ Qian    01-24-96  add default value for NAWS                         */
/*  MQ Qian    08-13-96  fixed the infinite loop of WONTTEL NAWS            */
/*                       modified NU_check_Recv_retval to handle RST        */
/* MQ Qian         10-22-96    Changed the return value for NU_Recv err     */
/*                             in NU_Telnet_Get_Filtered_Char               */
/*                                                                          */
/*  D. Sharer  04-14-1998  Removed Warnings and Modified for Net 4.0        */
/*  P. Hill    05-07-1998  Cleaned up externs                               */
/****************************************************************************/

/* c library includes */

#define NEGOTIAT_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "externs.h"
#include "telopts.h"
#include "windat.h"
#include "n_ansi.h"
#include "nvt.h"
#include "tel_extr.h"

/* Global Variables */
extern int tl_test;

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*   NU_check_Recv_retval                                                */
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
/*  NAME       DATE      REMARKS                                         */
/*                                                                       */
/*  MQ Qian    01-20-95  initial version 1.0                             */
/*  MQ Qian    08-13-96  modified NU_check_Recv_retval to handle RST     */
/*                                                                       */
/*************************************************************************/
int NU_check_Recv_retval(int16 socket, int retval, int bytes, int excuse, char *where)
{
    if (retval==NU_NOT_CONNECTED)
    {
        NU_Close_Socket(socket);
        return(1);
    }

    if (retval<0 || (!bytes && retval!=bytes) )
    {
        message_print("\nNU_Recv() got error (%d) inside %s", retval, where);
        if (excuse && (excuse&retval)==retval)
            return(1);
        NU_TN_exit(retval);
    }

    return(0);
}  /* NU_check_Recv_retval() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      parsewrite                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  print all message text on the users screen                           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*   str    -   the data stream to be printed                            */
/*   len    -   the length of the part of the stream to be printed       */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*   NU_Telnet_Parse                                                     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-05-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
void parsewrite(char *str, int len)
{
    char buf;

    /* save the first character after printing length
        before replace it by a string terminate */
    buf = str[len];
    str[len] = 0;
    /* the function telnet_print() is defined in n_ansi.h, it is a MACRO,
        now, only replaced by printf() for demo purpose, if the user's embedded
        system has not a screen, the users can change its definition in n_ansi.h,
        or make it a dummy function, or direct it to other device */
    telnet_print("%s", str);
    /* restore the first character after printing length */
    str[len] = buf;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*  set_one_Nego_option                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  install one negotiation option                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*    buffer    - the buffer for negotiation option                      */
/*    action    - the requirement                                        */
/*    option    - the option of negotiation                              */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*  NU_Install_Negotiate_Options                                         */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         02-10-1995               initial version 1.0     */
/*      MQ Qian         01-16-1996               last modification       */
/*                                                                       */
/*************************************************************************/
void set_one_Nego_option(char *buffer, char action, char option)
{
    char buf[5];

    /*  install one negotiation option  */
    sprintf(buf, "%c%c%c", IAC, action, option);
    /* concatenate this option into whole negotiation string */
    buffer = strcat(buffer, buf);
} /* set_one_Nego_option() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*  NU_Install_Negotiate_Options                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  install all negotiation options                                      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*    nego_buf  - the buffer for negotiation option                      */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*  set_one_Nego_option                                                  */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*   NU_Telnet_Start_Negotiation()                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         02-10-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
void NU_Install_Negotiate_Options(char *nego_buf, char *nego_table)
{
    unsigned char i/*, nego_buf[MAX_NEGO_LENGTH];*/;

    nego_buf[0] = 0;

    for (i=0; i<MAX_NEGO_OPTIONS; i++)
    {
        /* the following cases are designed to match all possiblities,
            do not understand that every option has all these possiblities. */
        switch (nego_table[2*i])
        {
            case  DO:
                /* we require this option */
                set_one_Nego_option(nego_buf, (char)DOTEL, (char)nego_table[2*i+1]);
                break;
            case  DONT:
                /* we do not require this option, or stop it */
                set_one_Nego_option(nego_buf, (char)DONTTEL, (char)nego_table[2*i+1]);
                break;
            case  WILL:
                /* we support this option */
                set_one_Nego_option(nego_buf, (char)WILLTEL, (char)nego_table[2*i+1]);
                break;
            case  WONT:
                /* we do not support this option */
                set_one_Nego_option(nego_buf, (char)WONTTEL, (char)nego_table[2*i+1]);
                break;
            case  DONT_WILL:
                /* we support this option, but do not do it, now */
                set_one_Nego_option(nego_buf, (char)DONTTEL, (char)nego_table[2*i+1]);
                set_one_Nego_option(nego_buf, (char)WILLTEL, (char)nego_table[2*i+1]);
                break;
            case  DONT_WONT:
                /* do not do it and we do not support this option */
                set_one_Nego_option(nego_buf, (char)DONTTEL, (char)nego_table[2*i+1]);
                set_one_Nego_option(nego_buf, (char)WONTTEL, (char)nego_table[2*i+1]);
                break;
            case  DO_WONT:
                /* just list all possibilities here */
                set_one_Nego_option(nego_buf, (char)DOTEL, (char)nego_table[2*i+1]);
                set_one_Nego_option(nego_buf, (char)WONTTEL, (char)nego_table[2*i+1]);
                break;
            case  DO_WILL:
                /* we support this option and do not do it, now */
                set_one_Nego_option(nego_buf, (char)DOTEL, (char)nego_table[2*i+1]);
                set_one_Nego_option(nego_buf, (char)WILLTEL, (char)nego_table[2*i+1]);
                break;
            case  NOTHING:
                break;
        }
    }
}    /* NU_Install_Negotiate_Options() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*  NU_Telnet_Start_Negotiate()                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Send the initial negotiations on the network and print               */
/*               the negotitations to the console screen.                */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  socket  -   the socket index of the current connection               */
/*  tw      -   the structure containing the parameters of telnet        */
/*  nego_table - the negotiation options table                           */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*   NU_Telnet_Send()                                                    */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*   NU_Telnet_Client_Connect                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-05-1995      Modified initial version 1.0     */
/*                                                                       */
/*************************************************************************/
void NU_Telnet_Start_Negotiate(int16 socket, char *nego_table)
{
    char buf[30];

    /* Send the initial tlnet negotiations about the telnet options,
        the options formatiing and the definition of each command are
        defined in RFC 854, 855.
        NU_Install_Negotiate_Options() will install all the negotiation
        options into buf according to nego_table[], which is declared at
        the beginning of the file. nego_table[] is designed as a basic case,
        if it does not match the need of the user, the user need to change it.
    */
    NU_Install_Negotiate_Options(buf, nego_table);

    NU_Send((int16)socket, buf, (uint16)strlen(buf), 0);
}   /* end NU_Telnet_Start_Negotiate() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*  NU_Telnet_Specific_Negotiate()                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  ask the window size of other part                                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*  socket  -   the socket index of the current connection               */
/*  tw      -   the structure containing the parameters of telnet session*/
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*    NU_Telnet_Send()                                                   */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  NAME       DATE      REMARKS                                                                 */
/*                                                                       */
/*  MQ Qian    01-15-95  initial version 1.0                                                     */
/*  MQ Qian    01-24-96  add option field and default value for NAWS     */
/*                                                                       */
/*************************************************************************/
void NU_Telnet_Specific_Negotiate(int16 socket, int option)
{
    int     cnt;               /* the number of incoming data bytes */
    uchar    data[201];        /* this size should be considered later ??? */
    struct twin *tw;

    /* get the pointer of the parameter data struct of the current sesseion */
    tw = tn_session[socket];

    /* this loop will run until received the response from other side of the
        telnet connection about its window size */
    while (1)
    {
        NU_Fcntl(socket, NU_SETFLAG, NU_BLOCK);
        cnt = NU_Recv((int16)socket, (char *)data, 200, 0);
        NU_Fcntl(socket, NU_SETFLAG, NU_FALSE);
        if (cnt<0)
        {
            message_print("\nSomething (%d) is wrong found by NU_Recv()!", cnt);
            continue;
        }

        /* NU_Telnet_Parse() will analysis data to see if there are telnet
          commands and options, if there is, just do it. */
        NU_Telnet_Parse(socket, data, cnt, 0);

        if (option==NAWS)
        {
            /* check if other side send its window size, or not */
            if (tw->nego_NAWS&NEGO_HE_REJECTED)
            {
                /* if other side DOTEL, or WONTTEL NAWS, use default value,
                  but, the user may need to change the default value to match
                  the size of display area of their specific remote host, */
                his_side.width = 80;
                his_side.rows  = 24;
                break;
            }

            if (!(tw->nego_NAWS&NEGO_HE_ACKED))
                /* if we have not got it, keep asking and waiting */
                NU_Telnet_Send(socket, "%c%c%c", IAC, DOTEL, NAWS);
            else
                /* if we get it, stop this loop */
                break;
        }
        /* now, only NAWS option is checked, the user can add more option
            checking here. */

        NU_Sleep(1);
    }

    /* the function message_print() is defined in n_ansi.h, it is a MACRO,
        now, only replaced by printf() for demo purpose, if the user's embedded
        system has not a screen, the users can change its definition in n_ansi.h,
        or make it a dummy function, or direct it to other device */
    message_print("\nnegotiated: (%d,%d,%s)",
        his_side.width, his_side.rows,
        (tw->nego_TERMTYPE&NEGO_HE_ACKED ?
        (tw->termstate==VT100TYPE ? "vt100" : "") : ""));
}   /* NU_Telnet_Specific_Negotiate() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*  nu_parse_subnegotiat()                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Parse the telnet sub-negotiations read into the parsedat array.      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  socket  -   the socket index of the current connection               */
/*  parsedat -  the sub-stream of sub-negotiation                        */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*    NU_Telnet_Send                                                     */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*   NU_Telnet_Parse                                                     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-05-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
void nu_parse_subnegotiat(int16 socket, char *parsedat)
{
    struct twin *tw;

    /* get the pointer of the parameter data struct of the current sesseion */
    tw = tn_session[socket];

    /* the sub negotiation is about terminal type */
    if (parsedat[0]==TERMTYPE)
    {
        if (parsedat[1]==1)
        {
        /* if this byte is 1, we tell other part what terminal type we will
            emulate, basically it is VT100, the users can change it to match his
            requirement */
            NU_Telnet_Send(socket,"%c%c%c%c%s%c%c",
                IAC,SB,TERMTYPE,0,"vt100",IAC,SE);
            /* after told him mine, set ours to be vt100 as default,
            if he wont change it, he agrees */
            tw->termstate = VT100TYPE;
            /* set global flag */
            tw->nego_TERMTYPE = NEGO_I_ACKED;
        }
        else if (parsedat[1]==0)
        {
        /* if parsedat[1]=0, in this packet, other side inform us of his terminal
        emulation type. The users can change it to match his requirement,
        here we only handle vt100, as an example */
            if (strcmp(&parsedat[2], "vt100")==0)
                /* if he simulate vt100, we agree. */
                tw->termstate = VT100TYPE;
            /* set global flag */
            tw->nego_TERMTYPE = NEGO_HE_ACKED;
        }
        return;
    } /* end if */

    /* received Negotiate About Window Size from other side,
        basically, it is client, save the size into the global data structure
        containing the telent session parameters of client */
    if (parsedat[0]==NAWS && parsedat[1]==0)
    {
        his_side.width = parsedat[2];
        his_side.rows  = parsedat[4];
        tw->nego_NAWS |= NEGO_HE_ACKED;
        return;
    } /* end if */

    if (parsedat[0]==LINEMODE)
    {
        /* we do not support LINEMODE */
        NU_Telnet_Send(socket,"%c%c%c%c%c%c",
                IAC,DONTTEL,LINEMODE, IAC,WONTTEL,LINEMODE);
        return;
    }
}   /* end nu_parse_subnegotiat() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Telnet_Parse                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  check the string coming from networking for special sequences        */
/*  for negotiation, or display it on the users screen.                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  socket  -   the socket index of the current connection               */
/*  *st     -   the pointer to the data stream coming                    */
/*  cnt     -   the count of the data stream                             */
/*  terminal -  the flag to indicate whether the other data of non       */
/*              telnet commands and options will be dump on terminal     */
/*              screen or not                                            */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*   return 0 always, now                                                */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*   parsewrite()                                                        */
/*   NU_Telnet_Send                                                      */
/*   NU_Receive_NVT_Key                                                  */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*   NU_Telnet_Recv                                                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  NAME       DATE      REMARKS                                         */
/*                                                                       */
/*  MQ Qian    01-12-95  initial version 1.0                             */
/*  MQ Qian    01-24-96  add the case of DONTEL and WONTTEL for NAWS     */
/*  MQ Qian    08-13-96  fixed the infinite loop of WONTTEL NAWS         */
/*                                                                       */
/*************************************************************************/
int NU_Telnet_Parse(int16 socket, unsigned char *st, int cnt, int terminal)
{
    int len;                      /* local counting variable */
    static int sub_pos; /* the position we are in the subnegotiation parsing */
    unsigned char *mark,*orig;
    unsigned char parsedat[256];      /* buffer for sub-negotiation */
    struct twin *tw;

    /* get the pointer of the parameter data struct of the current sesseion */
    tw = tn_session[socket];

    orig = st;              /* remember beginning point */
    mark = st+cnt;          /* set to end of input string */
    NU_Push(socket);

    /* Traverse the stream of data passed in, looking for telnet commands and
        options keystroke string of the emulation of certain terminal type.
        As to the telnet commands and its convention, please refer to RFC 854.
        About emulation of terminal, basically it is defined as VT100,
        the emulation table is declared in vt100key.h. However, the key values
        of the local keystrokes, if the user's system has a keyboard, will
        determined by the user's application, so user has to set this table to match
        his device. */

    while (st<mark)
    {
        while (tw->telstate!=STNORM && st<mark)
        {
            /* the following switch branch control structure is FSM (state
            machine), run the machime util the state is set to STNORM, or the
            data stream is exhausted */
            switch (tw->telstate)
            {
                case ESCFOUND:
                    parsewrite("\033", 1);        /* send the missing ESC */
                    tw->telstate=STNORM;
                    break;
                case IACFOUND:
                    /* this state means that we get a telnet command , all
                    the telnet command are started with IAC, which is unsigned
                    char of value 255.
                    however, there is only one exception, if the next value is
                    still 255, it means the second IAC is a data.
                    So, change state back to normal */
                    if (*st==IAC)
                    {
                        st++;
                        tw->telstate=STNORM;
                        break;
                    } /* end if */

                    /* the value of the next byte should be telnet command,
                    its value should be greater 239. take this value as the
                    next state. */
                    if (*st>239)
                    {
                        tw->telstate = *st++;
                        break;
                    } /* end if */

                    orig = ++st;
                    tw->telstate = STNORM;
                    break;

                /* each of the states below is one telnet command */
                case DOTEL:
                    /* received a telnet DO negotiation, the following switch
                    control structure is responsible for handling the
                    negotiation */
                    switch (*st)
                    {
                        case BINARY:
                            /* if the other side requires binary transmission,
                            and this side was not in this mode, tell him, we
                            supports it now.
                            if this side is already in this mode, no response */
                            if (!tw->ibinary)
                            {
                                if (!tw->iwantbinary)
                                    /* check whether we asked for this */
                                    NU_Telnet_Send(socket, "%c%c%c",
                                        IAC,WILLTEL,BINARY);
                                else
                                    tw->iwantbinary = 0;  /* turn off this now */
                                tw->ibinary = 1;
                            } /* end if */
                            break;

                        case SGA: /* Suppress go-ahead */
                            /* if he requires to suppress go-ahead, and we are
                            not in this mode, tell him, we suppress go-ahead
                            from now on.
                            if we are already in this mode, no response */
                            if (!tw->igoahead)
                            {
                                NU_Telnet_Send(socket,"%c%c%c",IAC,WILLTEL,SGA);
                                tw->igoahead = 1;
                            }
                            break;

                        case TERMTYPE:      /* terminal type negotiation */
                            /* if the other side requires TERMTYPE, and we are
                            not in this mode, tell him, we support it.
                            if we are already in this mode, no response */
                            if (!tw->termsent)
                            {
                                tw->termsent = 1;
                                NU_Telnet_Send(socket,"%c%c%c",
                                    IAC,WILLTEL,TERMTYPE);
                            } /* end if */
                            break;

                        case NAWS:
                            /* Negotiate About Window Size.
                            if the other side digs out window size,
                            just tells them */
                            NU_Telnet_Send(socket, "%c%c%c%c%c%c%c%c%c",
                                IAC,SB,NAWS,(char)0, (char)tw->width,(char)0,
                                (char)tw->rows,IAC,SE);
                            tw->nego_NAWS |= NEGO_I_ACKED;
                            break;

                        case ECHO:         /* mq added 02/01/95 */
                            /* if the other side requires echo and this side
                            is not in this mode, just changes mode to do echo,
                            if this side is already in this mode, does nothing */
                            if (!tw->echo)
                                tw->echo = 1;
                            break;

                        default:
                            /* we reject other options */
                            NU_Telnet_Send(socket,"%c%c%c",IAC,WONTTEL,*st);
                            break;

                    } /* end switch DOTEL */

                    /* set state back to normal to check rest byte */
                    tw->telstate = STNORM;
                    /* remember the current position */
                    orig = ++st;
                    break;

                case DONTTEL:
                    /* Received a telnet DONT option */
                    if ((*st)==BINARY)
                    {
                        /* if the other side requires to stop binary transmission,
                        and we are in this mode, tell him, we stop.
                        if we are not in this mode, no response */
                        if (tw->ibinary)
                        {   /* binary */
                            if (!tw->iwantbinary)
                            {
                            /* check whether we asked for this */
                                NU_Telnet_Send(socket,"%c%c%c",
                                    IAC,WONTTEL,BINARY);
                            } /* end if */
                            else
                                tw->iwantbinary = 0;  /* turn off this now */
                            tw->ibinary = 0;
                            /* tw->mapoutput = 0;  */  /* turn output mapping off */
                        } /* end if */
                    } /* end if */

                    if ((*st)==NAWS)
                    {
                        tw->nego_NAWS |= NEGO_HE_REJECTED;
                        NU_Telnet_Send(socket,"%c%c%c", IAC,WONTTEL,NAWS);
                    }

                    /* if the other side requires to stop echo and we are
                    in this mode, just stops the mode,
                    if we are in this mode, do nothing */
                    if ((*st)==ECHO && tw->echo)
                        tw->echo = 0;

                    /* set state back to normal to check rest byte */
                    tw->telstate = STNORM;
                    /* remember the current position */
                    orig = ++st;
                    break;

                case WILLTEL:
                    /* received a telnet WILL option */
                    switch (*st)
                    {
                        case BINARY:
                            /* if the other side is willing to do a binary
                            transmission, and we are not in this mode, tell
                            him, we do it from now on.
                            if we are already in this mode, no response */
                            if (!tw->ubinary)
                            {
                                if (!tw->uwantbinary)
                                {
                                /* check whether we asked for this */
                                    NU_Telnet_Send(socket,"%c%c%c",
                                        IAC,DOTEL,BINARY);
                                }
                                else
                                /* turn off this now */
                                    tw->uwantbinary = 0;
                                tw->ubinary = 1;
                            }
                            break;

                        case SGA:
                            /* if the other side is willing to suppress go-ahead,
                            and we are not in this mode, tell him, we suppress
                            go-ahead from now on.
                            if we are already in this mode, no response */
                            if (!tw->ugoahead)
                            {
                                tw->ugoahead = 1;
                                NU_Telnet_Send(socket,"%c%c%c",IAC,DOTEL,SGA);
                            }
                            break;

                        case ECHO:
                            /* if the other side is willing to echo and we are
                            not in this mode, tell him, we do it now,
                            if we are already in this mode, do nothing */
                            if (!tw->echo)
                            {
                                tw->echo = 1;
                                NU_Telnet_Send(socket,"%c%c%c",IAC,DOTEL,ECHO);
                            }
                            break;

                        case NAWS:          /* mq added 02/13/95 */
                            /* if he is willing tell us his window size,
                            let him do it, however, only once. */
                            if (!(tw->nego_NAWS&NEGO_HE_ACKED))
                            {
                                tw->nego_NAWS |= NEGO_HE_ACKED;
                                NU_Telnet_Send(socket, "%c%c%c", IAC,DOTEL,NAWS);
                            }
                            break;

                        case TERMTYPE:      /* mq added 02/10/95 */
                            /* if he is willing to answer TERMTYPE, ask it */
                            NU_Telnet_Send(socket,"%c%c%c%c%c%c",IAC,SB,TERMTYPE,1,IAC,SE);
                            break;

                        case TIMING:
                            /* Timing mark */
                            tw->timing = 0;
                            break;

                        default:
                            NU_Telnet_Send(socket,"%c%c%c", IAC,DONTTEL,*st);
                            break;
                    } /* end switch WILLTEL */

                    /* set state back to normal to check rest byte */
                    tw->telstate = STNORM;
                    /* remember the current position */
                    orig = ++st;
                    break;

                case WONTTEL:
                    /* Received a telnet WONT option */
                    switch (*st)
                    {
                        case BINARY:
                        /* if he is willing to stop binary transmission, and
                        we are in this mode, tell him, we stop it.
                        if we are not in this mode, no response */
                            if (tw->ubinary)
                            {
                                if (!tw->uwantbinary)
                                {
                                /* check whether we asked for this */
                                    NU_Telnet_Send(socket,"%c%c%c",
                                        IAC,DONTTEL,BINARY);
                                }
                                else
                                    tw->uwantbinary = 0; /* turn off this now */
                                tw->ubinary = 0;
                                /* tw->mapoutput = 0;*//* turn output mapping off */
                            } /* end if */
                            break;

                        case ECHO:              /* echo */
                            /* if he is willing to stop echo and we are in this
                            mode, tell him, tell him, we stop it now,
                            if we are not in this mode, do nothing */
                            if (tw->echo)
                            {
                                tw->echo = 0;
                                NU_Telnet_Send(socket,"%c%c%c",IAC,DONTTEL,ECHO);
                            }
                            break;

                        case TIMING:    /* Telnet timing mark option */
                            tw->timing = 0;
                            break;

                        case NAWS:
                            tw->nego_NAWS |= NEGO_HE_REJECTED;
/* During investigation of CSR0535, find that this causes infinitive negotiation.
                            NU_Telnet_Send(socket,"%c%c%c",IAC,DONTTEL,NAWS); */
                            break;

                        default:
                            break;
                    } /* end switch WONTTEL */

                    /* set state back to normal to check rest byte */
                    tw->telstate = STNORM;
                    /* remember the current position */
                    orig = ++st;
                    break;

                case SB:
                    /* we get the start point of telnet sub-options
                    negotiation, change state to collect all sub-negotiate
                    bytes */
                    tw->telstate = NEGOTIATE;
                    /* initialize these variables to receive sub-negotiation */
                    orig = st;
                    sub_pos = tw->substate = 0;

                case NEGOTIATE:
                    /* collect all the sub-negotiate bytes until we get its
                    end: IAC,SE (255,240) */
                    if (tw->substate==0)
                    {
                        if (*st==IAC)
                        {   /* check if we find an IAC before SE */
                            if (*(st+1)==IAC)
                            {  /* skip over double IAC's */
                                st++;
                                parsedat[sub_pos++] = *st++;
                            } /* end if */
                            else
                            {
                                /* put the IAC byte into the sub-state, in order
                                to leave the collection state on the next time
                                of NEGOTIATE state, the main state is intact. */
                                tw->substate = *st++;
                            } /* end else */
                        } /* end if */
                        else
                            /* otherwise, just keep collect the sub-negotiate
                            bytes in sub-negotiate buffer. */
                            parsedat[sub_pos++] = *st++;
                    } /* (tw->substate==0) */
                    else
                    {
                        /* the next byte should be SE */
                        tw->substate = *st++;
                        /* check if we've really ended the sub-negotiations */
                        if (tw->substate==SE)
                            nu_parse_subnegotiat(socket, (char *)parsedat);
                        orig = st;
                        tw->telstate = STNORM;
                    } /* end else */
                    break;

                case AYT:       /* received a telnet Are-You-There command */
                    NU_Send(socket, "YES", 3, 0);
                    tw->telstate = STNORM;
                    /* NJT Mod was ++st, caused loss of 1 character */
                    orig = st;
                    break;
                default:
                    tw->telstate = STNORM;
                    break;
            } /* end switch */
        } /* end while */

        /* quick scan of the remaining string, skip chars while they are
            uninteresting, or they are keystroke string */
        if (tw->telstate==STNORM && st<mark)
        {
            while (st<mark && *st!=27 && *st!=IAC)
            {
                if (!tw->ubinary)
                /* if we are not in binary mode, mask off high bit */
                    *st &= 127;
                st++;
            } /* end while */

            len = st - orig;
            /* debug purpose, normally it should be exhausted. */
            if (len && st!=mark)
                message_print("!");

            if (terminal && !tw->timing && len)
            /* if this application dumps the incoming date on its screen
             or printer, dumps it */
                parsewrite((char *)orig, len);

            /* forget what we have sent already */
            orig = st;
            if (st<mark)
                switch (*st)
                {
                    case IAC:
                    /* check telnet command, if find IAC, the next state is to
                    parse the telnet command */
                        tw->telstate = IACFOUND;
                        st++;
                        break;

                    case 27:            /* ESCape code */
                    /* if this byte is 27, there are four possibilities:
                     1. it is the last byte of the stream,
                     2. it is followed by 12, this is a printer command,
                       12 - Form Feed Moves the printer to the top of the
                       next page, keeping the same horizontal position.
                     3. it is followed by '^', its value is 94, I don't know.
                     4. this is ESC string of vt100 key, it must be followed by
                       other two characters.
                      '[' and 'O' are the second byte of vt100 key code.
                       the users may need modify it to match their specific situation.
                    */
                        if (*(st+1)=='[' || *(st+1)=='O')
                        {
                            /* check this emulation string in key mapping table
                                and explain it */
                            len = NU_Receive_NVT_Key(socket, (char *)st, 1);
                            /* if it maps a key, skip these bytes
                                and go on parsing */
                            if (len)
                                st += len;
                            else
                                st += 3;
                            orig = st;
                            break;
                        }
                        else if (st==mark-1 || *(st+1)==12 || *(st+1)=='^')
                            tw->telstate = ESCFOUND;
                        st++;           /* strip or accept ESC char */
                        break;

                    default:
                        st++;
                        break;
                }   /* end switch */
        }   /* end if */
    } /* end while */

    /* indicate that NU_Telnet_Parse() exhausts this data stream */
    return(0);
}   /* end NU_Telnet_Parse() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Telnet_Pick_Up_Ascii                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  check the string coming from networking for special sequences        */
/*  only pick up the ascii characters to display, or move to where the   */
/*  users want by redeclare telnet_print()                               */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  *st     -   the pointer to the data stream coming                    */
/*  cnt     -   the count of the data stream                             */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*   telnet application                                                  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ Qian         01-18-1995               initial version 1.0     */
/*                                                                       */
/*************************************************************************/
#define SUBNEG_CODE     1
#define IAC_CODE        2
#define ASCII_CODE      4
#define ESC_CODE        8
int NU_Telnet_Pick_Up_Ascii(unsigned char *st, int cnt)
{
    int find=0, count=0, i;
    unsigned char *orig;

    while (cnt)
    {
        if (*st!=27 && *st!=IAC)
        {
/*          if (!tw->ubinary)
                *st &= 127;  */ /* mask off high bit */
            st++;
            cnt--;
            find |= ASCII_CODE;
            continue;
        }

        orig = st;              /* remember beginning point of IAC string */
        if (*st==27)
        {
            /* '[' and 'O' are the second byte of vt100 key code.
                the users may need modify it to match their own emulation */
            if (*(st+1)=='[' || *(st+1)=='O')
            {
                st += 3;
                count += 3;
                find |= ESC_CODE;
            }
            else
                st++;
        }
        else if (*st==IAC)
        {
            find |= IAC_CODE;
            /* IAC must be followed with, at least, one more byte */
            st++;
            count++;

            /* telnet commands and options string is, at least, of 2 bytes */
            if (*st<SE)
            {
                message_print("\nsomething missed (%d) after IAC!", *st);
                orig[0] = 0;
                return(0);
            }

            if (WILLTEL<=*st && *st<=DONTTEL)
            {
                /* after DOTEL, DONTTEL, WILLTEL and WONTTEL, ONLY one byte */
                st++;
                count++;
            }
            else if (*st==SB)
            {
                find |= SUBNEG_CODE;
                /* find all the bytes before end of sub-negotiation. */
                while (*st!=SE)
                {
                    st++;
                    count++;
                }
            }

            /* telnet commands and options string is, at least, of 2 bytes */
            st++;
            count++;
            /* if sub-negotiation, it is, at least, of 5 bytes */
            if (find&SUBNEG_CODE  && count<6)
            {
                message_print("*"); /*\nit's a bad sub negotiation!");*/
                orig[0] = 0;
                return(0);
            }
        }
        /* calculate the totally retained bytes */
        cnt -= count;
        /* in case some bad situation, cnt<0 will cause crash */
        if (cnt<0)
        {
            message_print("\nthere is something missed (%d) %s", cnt,
                "in telnet command!");
            orig[0] = 0;
            return(0);
        }
        for (i=0; i<cnt; i++)
            orig[i] = st[i];
        orig[i] = 0;
        st = orig;
        count = 0;
    }

    /* indicate that which type of data are found in this data stream */
    return(find);
}   /* end NU_Telnet_Pick_Up_Ascii() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*   NU_Telnet_Get_Filtered_Char                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  get a character from telnet client and check if there is tlenet      */
/*  commands and options, if there is, read all of them and execute them */
/*  util get a ascii character                                           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian, (modified) ATI                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*  socket  -   the socket index of the current connection               */
/*  block   -   the parameter determining whether wait for data or not   */
/*                                                                       */
/*  RETURNS                                                              */
/*                                                                       */
/*   a character                                                         */
/*                                                                       */
/*  CALLS                                                                */
/*                                                                       */
/*   NU_Telnet_Parse                                                     */
/*   NU_Receive_NVT_Key                                                  */
/*                                                                       */
/*  CALLED                                                               */
/*                                                                       */
/*   DBT_Get_Char()                                                      */
/*   telnet application                                                  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/* NAME            DATE        REMARKS                                   */
/*                                                                       */
/* MQ Qian         01-18-95    initial version 1.0                       */
/* MQ Qian         08-14-96    Modification for NU_NOT_CONNECTED         */
/* MQ Qian         10-22-96    Changed the return value for NU_Recv err  */
/*                             don't use NU_check_Recv_retval            */
/*                                                                       */
/*************************************************************************/
#define MAX_TN_COMMAND 30  /* so far, the longest command is of 18 bytes */

#undef NU_TN_exit
/* let the control back to higher level */
#define NU_TN_exit(x) return(x+'\n'-x)

char NU_Telnet_Get_Filtered_Char(int16 socket, int block)
{
    unsigned char chr=0, i, telnet_command[MAX_TN_COMMAND+1];
    int retval;
    char *where="NU_Telnet_Get_Filtered_Char()";

    /*  Get a character from telnet,
      turn on the "block during a read" flag according to block value */
    NU_Fcntl(socket, NU_SETFLAG, NU_BLOCK);
    retval = NU_Recv(socket, (char *)&chr, 1, 0);
    NU_Fcntl(socket, NU_SETFLAG, NU_FALSE);
    if (NU_check_Recv_retval(socket, retval, 1, 1, where))
        return(ERR_RETURN);

    if (block && retval<=0)
        return(0);

    /* check telnet commands and options, if there are,
        read through and execute them */
    while (chr==27 || chr==IAC)
    {
        i = 0;
        if (chr==IAC)
        {
            /* here may be a problem how to handle binary data ??? mq, 01-18-95 */
            telnet_command[i++] = IAC;
            /* Because thr next byte must be a telnet commands and options,
              it is need not to turn on the "block" flag, here */
            chr = 0;
            retval = NU_Recv(socket, (char *)&chr, 1, 0);
            if (NU_check_Recv_retval(socket, retval, 1, 1, where))
                return(ERR_RETURN);

            /* SE is the minimum value among telnet commands */
            if (chr<SE)
            {
                message_print("\nthere is something wrong (%X) after IAC!", chr);
                NU_TN_exit(retval);
            }

            telnet_command[i++] = chr;
            if (WILLTEL<=chr && chr<=DONTTEL)
            {
                /* according to the syntax of telent option,
                    there must be one more byte */
                retval = NU_Recv(socket, (char *)&chr, 1, 0);
                if (NU_check_Recv_retval(socket, retval, 1, 1, where))
                    return(ERR_RETURN);

                telnet_command[i++] = chr;
            }
            else if (chr==SB)
            {
                /* if start sub-negotiation, we read through it util end of it */
                while (chr!=SE)
                {
                    /* Because SB must be followed with more bytes,
                      it need not to turn on the "block" flag */
                    retval = NU_Recv(socket, (char *)&chr, 1, 0);
                    if (NU_check_Recv_retval(socket, retval, 1, 1, where))
                        return(ERR_RETURN);

                    telnet_command[i++] = chr;
                    if (i>=MAX_TN_COMMAND)
                    {
                        message_print("\nSE is missed!");
                        NU_TN_exit(i);
                    }
                }
            }

            telnet_command[i] = 0;
            if (i>=MAX_TN_COMMAND)
            {
                message_print("too long (%d) telnet command!", i);
                NU_TN_exit(i);
            }
            else if (i==1)
            {
                message_print("wrong telnet command!");
                NU_TN_exit(i);
            } else if (i>=1)
            /* there are legal telnet commands and options,
                parse and execute them */
            NU_Telnet_Parse(socket, telnet_command, i, 0);
        }
        else if (chr==27)
        {
            /* read ESC string into buffer */
            telnet_command[i++] = chr;
            /* it is need to turn on the "block" flag, here */
            chr = 0;
            NU_Fcntl(socket, NU_SETFLAG, NU_BLOCK);
            retval = NU_Recv(socket, (char *)&chr, 1, 0);
            NU_Fcntl(socket, NU_SETFLAG, NU_FALSE);
            if (NU_check_Recv_retval(socket, retval, 1, 1, where))
                return(ERR_RETURN);

            /* '[' and 'O' are the second byte of vt100 key code.
            the users may need modify it to match their specific emulation */
            if (chr=='O' || chr=='[')
            {
                telnet_command[i++] = chr;
                /* there must be anther ESC string character */
                NU_Fcntl(socket, NU_SETFLAG, NU_BLOCK);
                retval = NU_Recv(socket, (char *)&chr, 1, 0);
                NU_Fcntl(socket, NU_SETFLAG, NU_FALSE);
                if (NU_check_Recv_retval(socket, retval, 1, 1, where))
                    return(ERR_RETURN);

                telnet_command[i++] = chr;
                telnet_command[i] = 0;
                /* check this emulation string in key mapping table and explain it */
                NU_Receive_NVT_Key(socket, (char *)telnet_command, 1);
            }
            else
                return(chr);
        } /* if (chr==\033) */

        /* There may have more telnet commands and options and ESC string,
          or may not, so it is necessary to turn on the "block" flag again
          to wait for and get a character from telnet.  */
        NU_Fcntl(socket, NU_SETFLAG, NU_BLOCK);
        retval = NU_Recv(socket, (char *)&chr, 1, 0);
        if (NU_check_Recv_retval(socket, retval, 1, 1, where))
            return(ERR_RETURN);

        NU_Fcntl(socket, NU_SETFLAG, NU_FALSE);
    }

    return(chr);
} /* NU_Telnet_Get_Filtered_Char() */


int
NU_Telnet_Pick_Up_Mychar(unsigned char *st, int cnt)
{
    int find=0, count=0, i;
    unsigned char *orig;

    while (cnt)
    {
        if (*st!=IAC)
        {
            st++;
            cnt--;
            find |= ASCII_CODE;
            continue;
        }

        orig = st;              /* remember beginning point of IAC string */

        if (*st==IAC) /* HMA */
        {
            find |= IAC_CODE;
            /* IAC must be followed with, at least, one more byte */
            st++;
            count++;

            /* telnet commands and options string is, at least, of 2 bytes */
            if (*st<SE)
            {
                message_print("\nsomething missed (%d) after IAC!", *st);
                orig[0] = 0;
                return(0);
            }

            if (WILLTEL<=*st && *st<=DONTTEL)
            {
                /* after DOTEL, DONTTEL, WILLTEL and WONTTEL, ONLY one byte */
                st++;
                count++;
            }
            else if (*st==SB)
            {
                find |= SUBNEG_CODE;
                /* find all the bytes before end of sub-negotiation. */
                while (*st!=SE)
                {
                    st++;
                    count++;
                }
            }

            /* telnet commands and options string is, at least, of 2 bytes */
            st++;
            count++;
            /* if sub-negotiation, it is, at least, of 5 bytes */
            if (find&SUBNEG_CODE  && count<6)
            {
                message_print("*"); /*\nit's a bad sub negotiation!");*/
                orig[0] = 0;
                return(0);
            }
        }
        /* calculate the totally retained bytes */
        cnt -= count;
        /* in case some bad situation, cnt<0 will cause crash */
        if (cnt<0)
        {
            message_print("\nthere is something missed (%d) %s", cnt,
                "in telnet command!");
            orig[0] = 0;
            return(0);
        }
        for (i=0; i<cnt; i++)
            orig[i] = st[i];
        orig[i] = 0;
        st = orig;
        count = 0;
    }

    /* indicate that which type of data are found in this data stream */
    return(find);
}   /* end NU_Telnet_Pick_Up_Mychar() */


