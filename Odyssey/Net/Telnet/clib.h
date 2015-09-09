/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1996 Accelerated Technology, Inc.           */
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
/*      clib.h                                          CLIB/C54X/T 1.0  */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      Data Types                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data types common to all of the               */
/*      C library functions.                                             */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Kevin E. Pontzloff, Accelerated Technology, Inc.                 */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      K. Pontzloff    02-03-1998       Created initial version 1.0     */
/*                                                                       */
/*************************************************************************/

/* Check to see if this file has been included already.  */

#ifndef CLIB_H
#define CLIB_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


/*  Define data sizes  */

#define _CHARBITS   8 
#define _INTBITS    32 
#define _SHRTBITS   16 
#define _LONGBITS   32 
#define _DBLBITS    64 
#define _FLTBITS    64 
#define _LDBLBITS   64


#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif


