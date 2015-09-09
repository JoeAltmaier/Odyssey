/********************************************
	Character type header.
********************************************/

#ifndef __CTYPE
#define __CTYPE

#ifdef _INTBITS
int isalnum( int );
int isalpha( int );
int iscntrl( int );
int isdigit( int );
int isgraph( int );
int islower( int );
int isprint( int );
int ispunct( int );
int isspace( int );
int isupper( int );
int isxdigit(int );
int tolower( int );
int toupper( int );

int isascii( int );
int toascii( int );
#endif

#define isascii(c)	((unsigned int)(c)<=0x7f)
#define toascii(c)	((c) & 0x7f)

#if !_INTBITS || _CHARBITS == 8
/********************************************************************
	Only implement the macros for small chars because the table
	must be directly indexable by values EOF,0...UCHAR_MAX.
	Each character in the array _uctype is represented by a
	mask of 8 bits as defined here.
********************************************************************/
extern const char _uctype[];

#define _U	0x01	/* upper case letter */
#define _L	0x02	/* lower case letter */
#define _N	0x04	/* digit [0 - 9] */
#define _S	0x08	/* white space */
#define _P	0x10	/* all chars that are not control or alphanumeric */
#define _C	0x20	/* control character */
#define _B	0x40	/* just the space (0x20) character */
#define _X	0x80	/* hexadecimal digit */

#define iscntrl(c)	((_uctype+1)[(c)] & _C)
#define isupper(c)	((_uctype+1)[(c)] & _U)
#define islower(c)	((_uctype+1)[(c)] & _L)
#define isdigit(c)	((_uctype+1)[(c)] & _N)
#define isxdigit(c)	((_uctype+1)[(c)] & _X)
#define isspace(c)	((_uctype+1)[(c)] & _S)
#define ispunct(c)	((_uctype+1)[(c)] & _P)
#define isalpha(c)	((_uctype+1)[(c)] & (_U | _L))
#define isalnum(c)	((_uctype+1)[(c)] & (_U | _L | _N))
#define isgraph(c)	((_uctype+1)[(c)] & (_U | _L | _N | _P))
#define isprint(c)	((_uctype+1)[(c)] & (_U | _L | _N | _P | _B))

#endif
#endif
