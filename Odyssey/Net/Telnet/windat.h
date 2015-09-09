/*************************************************************************/
/*                                                                       */
/*            Copyright (c) 1998 Accelerated Technology, Inc.            */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/
/*
*
* Portions of this program were written by:       */
/***************************************************************************
*                                                                          *
*      part of:                                                            *
*      TCP/IP kernel for NCSA Telnet                                       *
*      by Tim Krauskopf                                                    *
*                                                                          *
*      National Center for Supercomputing Applications                     *
*      152 Computing Applications Building                                 *
*      605 E. Springfield Ave.                                             *
*      Champaign, IL  61820                                                *
*                                                                          *
***************************************************************************/
/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*     windat.h                                               1.1        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      the date structur of the parameters for a telnet session		 */
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
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      MQ qian         01-10-1995      Modified initial version 1.0     */
/*                                                                       */
/*************************************************************************/

#ifndef WINDAT_H
#define WINDAT_H

/*
*  the struct twin contains all the parameter of a telnet session
*/
#define VTEKTYPE 	1
#define DUMBTYPE 	2
#define VTTYPE 		3
#define TEKTYPE 	4
#define RASTYPE 	5

#define VT100TYPE 	6
#define VT220TYPE 	7
#define VT52TYPE 	8

#define NCOLORS 	4

struct twin {
	unsigned short colors[NCOLORS];
	unsigned char mname[16],    	/* name of the machine connected to */
		linemode_mask,          	/* mask for editting modes in linemode */
		linemode[82];               /* line mode buffer for session */
	int pnum,						/* port number associated */
		socket,                     /* the socket index associated with it */
		bkscroll,                   /* scroll back value */
		width,                      /* width of the window */
		rows,                       /* Number of rows in the window */
		telstate,                   /* telnet state for this connection */
		substate,                   /* telnet subnegotiation state */
		termstate,                  /* terminal type for this connection */
		nego_NAWS,                  /* the flag of NWAS negotiation */
		crfollow,            /* what is supposed to follow a CR? NUL or LF? */
		nego_TERMTYPE,      /* the flag about the negotiation on TERMTYPE */
		bksp,                       /* what keycode for backspace ?*/
		del,                        /* for delete? */
		slc[31],                	/* line mode sub-option characters */
		slm[31];                	/* line mode sub-option modes */
	char *ftpopts;					/* FTP cmd line paramters */
	unsigned int mapoutput:1,   /* are we mapping the characters output ? */
		vtwrap:1,               /* line wrapping flag */
		lmflag:1,               /* Are we in linemode? */
		lmedit:1,               /* Edit lines in linemode? */
								/* the following four only for linemode use */
		litflag:1,              /* Is the next character to be send literally */
		litecho:1,              /* Is the next character to be echoed literally */
		softtab:1,              /* Expand tabs on the client side? */
		trapsig:1,              /* Trap signals on the client side? */
		halfdup:1,              /* half duplex mode overrides line mode */
		termsent:1,             /* has terminal type been sent? */
								/* the following four only for binary trans. */
		ibinary:1,              /* negotiate for binary traffic out */
		iwantbinary:1,          /* flag to indicate whether we asked for out */
		ubinary:1,              /* negotiate for binary traffic in */
		uwantbinary:1,          /* flag to indicate whether we asked for in */
		igoahead:1,             /* negotiation for suppress go-ahead */
		ugoahead:1,             /* neg. for his suppress go-ahead */
		echo:1,                 /* line mode or echo mode? */
		timing:1,               /* timing marker sent */
		capon:1,                /* does this session own a capture file? */
		condebug:2;             /* debugging level for console output */
	FILE *capfp;
	struct twin *next,*prev;
};

/*#define NUM_WINDOWS     20*/

#ifdef WINMASTER
	struct twin *current=NULL, *tn_session[NSOCKETS], his_side;
#else
	extern struct twin *current, *tn_session[], his_side;
#endif

#endif
