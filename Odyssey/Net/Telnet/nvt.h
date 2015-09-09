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
/*     nvt.h                                              1.1        	 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*     negotiation options tables for client and server 		 		 */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      MQ Qian,          Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*	 client_nego_table - an array of even elements, the first element 	 */
/*		   of each pair is the index of telnet negotiatio command,       */
/* 			the second is the telnet nogotiation option.                 */
/*	 server_nego_table - an array as the same as that of client		     */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*	NAME	   DATE	     REMARKS 										 */
/*                                                                       */
/*  MQ Qian    02-10-95  initial version 1.0                    		 */
/*  MQ Qian    01-22-96  modification to pacify compiler warnnings 		 */
/*                                                                       */
/*************************************************************************/

/* the following MACROs are the index of telnet commands: DOTEL, DONTTEL,
  WILLTEL and WONTTEL,
  they are not the telnet commands directly
*/
#define DO          1
#define DONT        2
#define WILL        3
#define WONT        4
#define DONT_WILL   5
#define DONT_WONT   6
#define DO_WONT     7
#define DO_WILL     8
#define NOTHING     9

#define MAX_NEGO_OPTIONS 	5
#define MAX_NEGO_LENGTH 	(MAX_NEGO_OPTIONS*4)
#define MAX_TABLE_ENTRIES	(MAX_NEGO_OPTIONS*2)

/*
 the negotiation tables are designed as a basic condition, if it does not
 match the requirement of the user, the user need to change it.
 However, the user, who changes the table entries, must follow the way the
 tables are organized now, the rule is:
	1. the array must be of even elements;
	2. the first element of each pair is the MACRO of the index of telnet
	   negotiatio command, the second is the telnet nogotiation option;
	3. usually, the users may only changed the command index MACRO, not the
	   option,
	   BE CAREFULLY, the command index MACROs defined in this file are not
	   commands, different from the telnet commands defined in telopts.h,
	   these index MACROs are used to help set_one_Nego_option() (negotiat.c)
	   to set correct telnet command for each option;
	4. if the users do not need an option, just put NOTHING before the option;
	5. if the users, who have to add more options,  must change MAX_NEGO_OPTIONS
	   to match the number of total options.
*/

#ifdef NEGOTIAT_C

unsigned char client_nego_table[MAX_TABLE_ENTRIES]={
/*  the command index,	the option  */
	NOTHING,			BINARY,    /* not binary */
	DO,  				ECHO,      /* require other side support ECHO */
	DO_WILL,			SGA,       /* require other side suppress Go Ahead */
	WILL,				NAWS,      /* this side is willing to tell window size */
	WILL,				TERMTYPE   /* this side is willing to tell terminal type */
};

char server_nego_table[MAX_TABLE_ENTRIES]={
/*  the command index,	the option  */
	NOTHING,			BINARY,    /* not binary */
	WILL,				ECHO,      /* this side support ECHO */
	DO_WILL,			SGA,       /* this side suppress Go Ahead */
	DO,    				NAWS,      /* require other side to tell window size */
	DO,					TERMTYPE   /* require other side to tell terminal type */
};

#else

extern unsigned char client_nego_table[], server_nego_table[];

#endif /* NEGOTIAT_C */
