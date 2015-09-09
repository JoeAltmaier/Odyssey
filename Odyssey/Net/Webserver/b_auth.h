#ifndef _B_AUTH_H
#define _B_AUTH_H

/*************************************************************************/
/*                                                                       */
/*       Copyright (c) 1993-1998 Accelerated Technology, Inc.            */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      b_auth.h                                       WEBSERV 1.0       */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      Nucleus WebServ                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This  file contains all defines, data structures, and function   */
/*      prototypes necessary for the Basic Authentication to work        */
/*      properly.                                                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*      Don Sharer Accelerated Technology Incorporated.                  */
/*                                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      wpwlist                                                          */
/*      BPWLIST_INFO                                                     */
/*      BPWLIST_INFO_LIST                                                */
/*                                                                       */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*************************************************************************/

#include "nucleus.h"
#include "ps_pico.h"
#include "target.h"

#include "protocol.h"
#include "ip.h"
#include "externs.h"
#include "socketd.h"                                /* socket interface structures */
#include "tcpdefs.h"



#define BASIC_AUTH_FAILED   -1
#define BASIC_MAX_SIZE      76
/*  Structure to hold the user_id, password and access for Basic Authetication
 *  algorithm
 */

struct wpwlist
{
     CHAR  wuser_id[32];
     CHAR  wpass_word[32];
     CHAR  access_str[32];
};
extern struct wpwlist webpwTable[];

/* This structure defines An entry into the Basic Authetication string.     */
typedef struct _BPWLIST_INFO
{
    struct _BPWLIST_INFO     *wpwlist_next;        /* Next PW Pointer       */
    struct _BPWLIST_INFO     *wpwlist_previous;    /* Previous PW Pointer   */
    CHAR                     wuser[32];            /*  user id              */
    CHAR                     wpass_word[32];       /*  Password             */
    CHAR                     waccess[64];          /*  Access String        */
} BPWLIST_INFO;


/* Define the head of the linked list of the Nucleus WebServ Basic
 * Authetication Pass word structure.
 */
typedef struct _BPWLIST_INFO_LIST
{
    BPWLIST_INFO    *bpwlist_head;         /*  Basic Password List Head      */
    BPWLIST_INFO    *bpwlist_tail;         /*  Basic Passowrd List Tail      */
} BPWLIST_INFO_LIST;


/* Function Prototypes for Nucleus WebServ Basic Authetication */

int16 b_auth_init(void);
int16 b_parse_auth(Request *req);
uchar *base64_decode(uchar *total_char, uchar *final_decode);
int16 b_auth_add_entry(CHAR *user_id, CHAR *password);
int16 b_auth_delete_entry(CHAR *user_id, CHAR *password);
#endif
