#include "clib.h"
/***********************************************************************\
*       Generic "putc" routine for strings.                             *
*       copyright 1990 Software Development Systems, Inc.               *
*       all rights reserved                                             *
\***********************************************************************/

/***********************************************************************\
*   This routine acts like the 'fputc' function, except that it works   *
*   on character strings and not streams.  It is primarily intended     *
*   as an interface function to routines common to both strings and     *
*   streams.                                                            *
\***********************************************************************/
int     _sputc (int c, char **s )
{   *(*s)++ = c;
    *(*s)   = 0;
    return (unsigned char)c;
}

