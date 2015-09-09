/********************************************************
	Copyright 1985 Software Development Systems, Inc.

	Common routine called by printf, fprintf, sprintf
********************************************************/
#include "clib.h"
#include "stdarg.h"
#include "string.h"
#include "stdlib.h"
#include "ctype.h"
#include "stdio.h"
#include "abbr.h"
#include "flt.h"
#include "prflt.h"
#include "pad.h"

char *_ultoa PROTOE((ulong,int,int,char[]));
char *_sltoa PROTOE(( long,int,int,char[]));
int _dbltoprtf PROTOE((int (*)PROTOE((int,char *)), char *, double *,
			int, const struct prflt *));

#if 0
#ifdef FLOAT
    char _fltus;  /* '_fltus' will cause linker to pull in float formats */
		  /* For this to work _uprflt.o must be placed in the */
		  /* library BEFORE all files referencing _uprint() */
#endif
#endif

/****************************************************************
	Common print routine for printf, fprintf, sprintf
	put     pointer to a function that 'puts' a character
		args are character, where-to-put-ptr;
		returns the character if successful, -1 otherwise
	where   where to put character (eg. stderr)
	fmt     format string
	varg    variable argument string

	returns: number of characters put out (-1 if 'put' failed)
****************************************************************/
int _uprint(int (*put)PROTOE((int,char *)),
			char *where,
			register const char *fmt,
			register va_list varg)
{
#define VA( type )      va_arg( varg, type )
#define CHRSTR          0       /* print type is character or string */
#define DOUX            1       /* print type is d, o, u, x, X */
#define CONVF           2       /* print type is some sort of float */

    rint c, longarg;
    int  len, pad, prpad, prtsign, numalt, prttype;
    int  retv = 0, getwidth, getprec, convup;
    long val;
    rchar *p;
    char  buf[3];
    char  conv[16];     /* buffer for converted values */
			/* dimension of at least 16 for non-floats (ltoa) */

    struct prflt pp;    /* print parameters */

    /********************************************************
	Valid printf characters (in specific order).
    ********************************************************/
    static const char validc[] = "douxXcsfeEgG";
#       define SIGN_OK          0
#       define UNSIGN_OK        4
#ifdef FLOAT
#       define FLT_OK   7
    double flt;
#endif
	FILE *d;

    buf[1] = 0;

    for( ; (c = *fmt) != 0; ++fmt ) 
    {

	  /**************************************************************
		If the character is not a % or if it is a %%
		    then just put out the character and continue.
	   **************************************************************/
	  if( (c != '%') || ((c = *++fmt) == '%') ) 
	  {
	    ++retv;
	    if( (*put)( c, where ) != c ) return -1;
	    continue; 
	  }

	  /**************************************************************
		Look for zero or more flag arguments:
		-       left-justified conversion
		+       sign conversion will begin with + or -
		blank   sign conversion will begin with blank or -
		#       use alternate form for conversion
			o       force first digit to be a 0
			x       non-zero gets 0x prepended to it
			X       non-zero gets 0X prepended to it
			e,E,f   always show a decimal point
			g,G     always show a decimal point and
				trailing zeroes will NOT be removed
	  **************************************************************/
	  PRP_DEFAULT(pp);
	  do 
	  {
	    if( c == '-' )      pp.left = 1;
	    else if( c == '+' ) pp.sign = 1;
	    else if( c == ' ' ) pp.blank = 1;
	    else if( c == '#' ) pp.alternate = 1;
	    else                break; 
	  } while( ( c = *++fmt ) != 0 );

	  /**************************************************************
		Following the flags, there is an optional decimal
		field width or an * which denotes that the width
		is on the stack.
	   **************************************************************/
	  if ( c == '0' ) 
	  {
	    pp.lpad = '0';              /* zero padding on left */
	    pp.width = 0;               /* in case no more digits are found */
	    c = *++fmt; 
	  }
	  if( (getwidth = (c == '*')) != 0 ) c = *++fmt;
	  else if( isdigit(c) ) 
	  {
	    pp.width = atoi( fmt );
	    while( c = *++fmt, isdigit( c ) ) ; 
	  }

	  /**************************************************************
		Following the field width, there is an optional
		period (.) followed by a decimal precision or an *
		which denotes that the precision is on the stack.
		A null (ie. non-digit) precision string is treated as 0.
		d,o,u,x,X       minimum number of digits
		e,f             # of digits after the decimal point
		g               # of significant digits
		s               maximum # of characters to print
	   **************************************************************/
	  getprec = 0;
	  if( c == '.' ) 
	  {
	    c = *++fmt;
	    if( (getprec = (c == '*')) != 0 ) 
	    {
		  c = *++fmt; 
		}
	    else 
	    {
		  pp.precision = atoi( fmt );
		  while( isdigit( c ) ) c = *++fmt; 
		}
	  }

	  /****************************************************************
		Following the precision, there is an optional l,
		which specifies that the int argument is a long int,
		or an options L, which specifies that the double
		argument is a long double.
	  ****************************************************************/
	  longarg = 0;
	  if ( c == 'l' || c == 'L' )
	  {
	    longarg = c;
	    c = *++fmt; 
	  }

	  /* set defaults */
	  convup = numalt = prtsign = 0;

	  /****************************************************************
		Before executing the switch, check to see if it
		is a valid character so that the width, precision
		and value can be retrieved.
	  ****************************************************************/
	  if( (p = strchr( validc, c )) != 0 ) 
	  {
	    if( getwidth ) 
	    {
		  pp.width = VA(int);
		  if ( pp.width < 0 ) 
		  {   
		      /* negative width means left-just */
		      pp.left = 1;
		      pp.width = -pp.width; 
		  }
		}
	    if( getprec ) 
	    {
		  pp.precision = VA(int);
		  if ( pp.precision < 0 )  
		  {
		    /* negative prec as if not specified */
		    pp.precision = -1; 
		  }
		}

	    if( p <= (validc + SIGN_OK) ) 
	    {
		  val = (longarg=='l'? VA(long): (long)VA(int)); 
		}

	    else if( p <= (validc + UNSIGN_OK) ) 
	    {
		  val = (longarg=='l'?VA(unsigned long):(long)VA(unsigned int));
		}
#ifdef FLOAT
	    else if( p >= (validc + FLT_OK) ) 
	    {
		  if( pp.precision == -1 ) pp.precision = 6;
		  if ( longarg == 'L' ) 
		  {
		    /********************************************************
			NOTE: A "long double" is converted to "double"
			before printing, losing any extra precision.
			It would be possible to print "long double"
			values by changing variable "flt" so it is a
			"long double", and by changing "cvt.c" so that
			it uses "long double" types.  This is not done
			by default because the long double math would
			then always be pulled in from the library.
		    ********************************************************/
		    flt = VA(long double); 
		  }
		else 
		{
		  flt = VA(double); 
		}
	  }
#endif
	}

	switch( c ) 
	{
	  /********************************************************
		Convert a signed decimal argument.  After receiving
		the value, get the sign of the number and determine
		if it should be printed.  The variable prtsign con-
		tains either the sign or a 0 indicating that no sign
		should be printed.  Negative signs are always printed,
		but if the user didn't request a sign (using '+') and
		the value was positive, then either the sign should
		be printed as a blank (using ' ') or it shouldn't be
		printed at all.
	  ********************************************************/
	  case 'd':
	    p = _sltoa( val, 10, 0, conv );
	    prtsign = *p++;
	    if( !pp.sign && (prtsign == '+') ) prtsign = (pp.blank? ' ': 0);
	    prttype = DOUX;
	    break;

	  /********************************************************
		Convert an unsigned octal argument.  The variable,
		numalt, contains the number of alternate characters
		that are printed if the '#' argument was given.
	  ********************************************************/
	  case 'o':
		p = _ultoa( val, 8, 0, conv );
		numalt = 1;
		prttype = DOUX;
		break;

	  /********************************************************
		Convert an unsigned decimal argument.
	   ********************************************************/
	  case 'u':
		p = _ultoa( val, 10, 0, conv );
		prttype = DOUX;
		break;

	  /********************************************************
		Convert an unsigned hedxadecimal argument.  If the
		'X' was given the all the characters will be printed
		in upper case.  The variable, numalt, contains the
		number of characters to be printed if the '#' argument
		was given.
	   ********************************************************/
	  case 'X':
		convup = 1;
		/* fall thru */
	  case 'x':
		p = _ultoa( val, 16, convup, conv );
		numalt = 2;
		prttype = DOUX;
		break;

	  /********************************************************
		Print the argument as a character string.
	   ********************************************************/
	  case 's':
		p = VA( char * );
		prttype = CHRSTR;
		len = strlen( p );
		break;

#ifdef FLOAT
	  /************************************************
		    all floating point formats
	   ************************************************/
	  case 'f':
	  case 'e':
	  case 'E':
	  case 'g':
	  case 'G':
		if ( (len = _dbltoprtf(put, where, &flt, c, &pp)) < 0 )
		    return -1;
		retv += len;
		prttype = CONVF;
		break;
#endif

	  /********************************************************
	    Print the integer argument as a character.  It is
		put in a buffer because characters are treated the
		same as strings when dealing with field widths
		except that the precision is always 1 for characters.
	   ********************************************************/
	  case 'c':
		c = VA(int) & 0xff;
		/* fall thru */

	  /********************************************************
		Treat an unrecognized character as you would treat
		a %c conversion (see that above falls thru).
	   ********************************************************/
	  default:
		buf[0] = c;
		p = buf;
		pp.precision = 1;
		len = 1;
		prttype = CHRSTR; 
	}


	/****************************************************************
		Handle the final printing of conversions for d,o,u,x,X
	****************************************************************/
	if( prttype == DOUX ) 
	{

	  /********************************************************
		In the case where the number of precision digits
		is 0 and the value is 0, then the "number" itself
		should be printed as a null string instead of a 0.
		The alternate characters should not be printed
		when the value is 0.
	   ********************************************************/
	  if( val == 0 ) 
	  {
		pp.alternate = numalt = 0;
		if( pp.precision == 0 )  *p = 0;
	  }

	  len = strlen( p );

	  /********************************************************
		Determine the number of precision padding characters.
		There is a special case in that if the alternate
		type was specified and we are printing octal #'s,
		then decrement the padding so that an extra 0
		is not being printed with octal numbers.
	   ********************************************************/
	  if( (prpad = pp.precision - len) > 0 ) 
	  {
		if( pp.alternate && (numalt == 1) ) --prpad; 
	  }
	  else 
	  { 
	    prpad = 0; 
	  }

	  /********************************************************
		Determine the number of width padding bytes which
		is the total field with for the converted value.
	   ********************************************************/
	  pad = pp.width - len - prpad;

	  if( pp.alternate ) 
	  {
		pad -= numalt;
		retv += numalt;
		pp.alternate = c; 
	  }


	  if( prtsign ) --pad, ++retv;
	  if( pad < 0 ) pad = 0;

	  retv += len + pad + prpad;

	  /****************************
		Emit padding and sign.
	   ****************************/
	  if ( padsign(prtsign, numalt, pad, &pp, put, where) ) return -1;

	  /********************************************************
		Print out any padding characters for the length
		of the number itself. Then print the value, and if
		it was left justified, print any field padding.
	   ********************************************************/
	  if( padit( prpad, '0', put, where ) ) 
	  {             
	    return -1; 
	  }
	  while( (c = *p++) != 0 ) 
	  { 
	    if( (*put)( c, where ) != c )   return -1;
	  }
	  if( pp.left && padit( pad, pp.rpad, put, where ) )  return -1; 
	}

	/********************************************************
		Handle the print of strings and characters.
		If a width is given, it always governs how
		wide the field will be where the precision
		just governs how many characters are printed
		from the string.
	********************************************************/
	else if( prttype == CHRSTR ) 
	{

	  /********************************************************
		'len' is the number of characters to print, except if
		a precision less than 'len' was given, then that is
		the number of characters to print.
		The padding bytes should either be output before
		or after the string depending on whether the
		string is being left justified or not.
	   ********************************************************/
	  if( (pp.precision != -1) && (pp.precision < len) ) 
	  {
		len = pp.precision; 
	  }

	  if( (pad = pp.width - len) < 0 ) pad = 0;

	    retv += len + pad;

	    if( !pp.left && padit( pad, pp.lpad, put, where ) ) return -1;
	    while( len-- ) 
	    {
		  c = *p++;
		  if( (*put)( c, where ) != c ) return -1; 
		}
		if( pp.left  && padit( pad, pp.rpad, put, where ) ) return -1; 
	  }
	}

	return retv;
}
