/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1998 Accelerated Technology, Inc.           */
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
/*      n_ansi.h                                              1.1        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      the MACROs for non-ansi function                                 */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian,          Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/* NAME      DATE       REMARKS                                          */
/*                                                                       */
/* MQ qian   01-10-95   Created initial version 1.0                      */
/* MQ Qian   01-22-96   make it for all embedded system                  */
/* MQ Qian   10-22-96   Added ERR_RETURN, the return value for NU_Recv   */
/*                      err in NU_Telnet_Get_Filtered_Char               */
/*                                                                       */
/*************************************************************************/

/*
in this file, the MACROs are defined in order to handle a few non-portable
function, the users can redefine them, or just declare them as dummy function.
for example:
message_print() is basically used for error messsage or other debug purpose,
in our demo, just replace it by printf, the users can replace whatever their
target can do, or set it to be a dummy as follows
*/

#ifdef __BORLANDC__

#define message_print printf
#define telnet_print printf

extern int getkey(void);
extern void DOS_Exit(int);
#define NU_TN_exit DOS_Exit

#else /* not __BORLANDC__ */

#ifdef WINMASTER
void message_print(char *fmt, ...) { }
void telnet_print(char *fmt, ...) { }
#else
extern void message_print(char *, ...);
extern void telnet_print(char *, ...);
#endif

#define NU_TN_exit while

#endif /* __BORLANDC__ */

#define ERR_RETURN 0x8


