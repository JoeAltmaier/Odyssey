/*************************************************************************/
/*                                                                       */
/*     Copyright (c) 1993 - 1998 Accelerated Technology, Inc.            */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

#ifndef STDCS_H
#define STDCS_H

int     n_stricmp(const char *, const char *);
int     n_toupper(int ch);
char    *n_itoa(int value, char *string, int radix);
int     isascii(int);

#endif /* STDCS_H */

