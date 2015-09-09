/********************************************************
	Copyright 1985 Software Development Systems, Inc.

	Binary-to-ascii conversion for a signed long.

	Returns a pointer to a string built somewhere
	in "buf".  The convup parameter should be non-zero
	if upper case letters are wanted for the conversion. 

	This routine will handle bases from 8 to 16.
********************************************************/
#include "clib.h"
#include "abbr.h"

extern char *_ultoa PROTOE((ulong,int,int,char[]));

char *_sltoa(rlong val,
			 rint base,
			 rint convup,
			 char buf[] ) /* must be large enough to hold maximum number */
{
    rchar *p, sign = '+';  rulong uval = val;

    if( val < 0 ) 
    { 
      uval = -val; 
      sign = '-';
    }
    p = _ultoa( uval, base, convup, buf );
    *--p = sign;

    return p;
}
