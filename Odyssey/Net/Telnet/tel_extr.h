/*************************************************************************/
/*                                                                       */
/*      Copyright (c) 1993 - 1998 by Accelerated Technology, Inc.        */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/******************************************************************************/
/*                                                                            */
/* FILE NAME                                            VERSION               */
/*                                                                            */
/*   TEL_EXTR.H                                          NET 4.0              */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*   External definitions for functions in Nucleus Telnet.                    */
/*   This include file needs to go after other include files                  */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*  Patricia Hill                                                             */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*      None                                                                  */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*      None                                                                  */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*      NAME            DATE                    REMARKS                       */
/*  P. Hill            5-6-98                   Cleaned up externs            */
/*  P. Hill            5-7-98                   Ported to NET4.0              */
/******************************************************************************/

#ifndef TEL_EXTR_H
#define TEL_EXTR_H



/* Define external referances to global variables */
extern int NU_Telnet_Socket(char *, char *);
extern int NU_Telnet_Server_Accept(int16);
extern int NU_Telnet_Client_Connect(char *, char *);
extern void NU_Telnet_Start_Negotiate(int16, char *);
extern void NU_Telnet_Specific_Negotiate(int16, int);
extern int NU_Telnet_Pick_Up_Ascii(unsigned char *, int);
extern char NU_Telnet_Get_Filtered_Char(int16, int);
extern void NU_Telnet_Init_Parameters(int16);
extern void NU_Telnet_Free_Parameters(int16);
extern int NU_Telnet_Check_Connection(int16, int);
extern int NU_Telnet_Parse(int16, unsigned char *, int, int);
extern int received_exit(char *, int);
extern void parsewrite(char *, int);
extern void set_one_Nego_option(char *, char, char);
extern void nu_parse_subnegotiat(int16, char *);
extern void NU_Install_Negotiate_Options(char *, char *);
extern int NU_Close_and_check_retval(int16, char *);
extern int NU_check_Recv_retval(int16, int, int, int, char *);
extern char NU_Telnet_Get_Filtered_Char(int16, int);
extern int NU_Telnet_Send(int16, char *, ... );
extern int NU_Receive_NVT_Key(int16, char *, char);
extern void NU_Send_NVT_key(int16, unsigned int);

#endif  /* TEL_EXTR_H */
