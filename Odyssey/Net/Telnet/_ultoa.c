/********************************************************
	Copyright 1985 Software Development Systems, Inc.

	Binary-to-ascii conversion for an unsigned long.

	Returns a pointer to a string built somewhere
	in "buf".  The convup parameter should be non-zero
	if upper case letters are wanted for the conversion. 

	This routine will handle bases from 8 to 16.
********************************************************/
#include "clib.h"
#include "abbr.h"

char *_ultoa(rulong val,
	     rint base,
	     rint convup,
	     char buf[] ) /* must be large enough to hold maximum number */
{
#define MAXBUF 15       /* max length of a converted value */

  rchar *p, *str;

  str = convup? "0123456789ABCDEF": "0123456789abcdef";

  p = buf+MAXBUF-1;
  *p = 0;

  do
  { 
    *--p = str[val % base];
  } while( val /= base );

  return p;
}
