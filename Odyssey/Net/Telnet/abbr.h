/****************************************************************
	Abbreviations and linting macros.
	Copyright 1985 Software Development Systems, Inc.
	All rights reserved
****************************************************************/
#define rchar	register char
#define rshort	register short
#define rint	register int
#define rlong	register long

#define uchar	unsigned char
#define ushort	unsigned short
#define uint	unsigned int
#define ulong	unsigned long

#define ruchar	register uchar
#define rushort	register ushort
#define ruint	register uint
#define rulong	register ulong

#ifdef _INTBITS
#  define ldouble	long double
#  define voidchar	void
#else
#  define ldouble	double
#  define voidchar	char
#endif

#define sink	void

/****************************************************************
	Eliminate lint errors for explicit casts between
	any scalar value 'v' and any scalar type 't',
	which must be parenthesized.
	The macro is careful to make lint check its argument.
****************************************************************/
#ifndef L_CAST
#   ifndef lint
#	define L_CAST( t, v ) (t(v))
#   else
#	define L_CAST( t, v ) ((v)? t 0: t 0)
#   endif
#endif

/****************************************************************
	Short version for (char *) casting.
****************************************************************/
#define CS(x)	L_CAST( (char *), (x) )

/****************************************************************
	Eliminate lint errors for explicit casts to (int)
	by using macro SMALL_INT().
****************************************************************/
#define SMALL_INT( x )	L_CAST( (int), (x) )

/****************************************************************
	the construction SINK( function( args ) )  can be
	used when the value of a function is to be ignored,
	so that lint will not complain about that.
****************************************************************/
#ifdef lint
#   define SINK( f )	((void)(f))
#else
#   define SINK( f )	(f)
#endif

/****************************************************************
	PROTOE: macro for prototype function parameters within
	    an external declaration.  E.g.:
		extern char *strcpy PROTOE(( char *, char * ));
	    Note that two sets of parentheses are needed so that
	    the type list is one argument to the macro since
	    macros cannot take variable numbers of arguments.
	PROTOn: macros for prototype function parameters within
	    a definition of a function of taking n parameters
	    and returning a type other than a pointer to an
	    array or function.  E.g.:
		char *strcpy PROTO2( (a,b), char *a, char *b ) {...}
****************************************************************/
#ifdef _INTBITS
#   define PROTO	1
#   define PROTOTYPES	1
#else
#   undef PROTO
#   undef PROTOTYPES
#endif

#ifdef PROTO
# define PROTOE( l )				l

# define PROTO0( l )				( void )
# define PROTO1( l,a )				( a )
# define PROTO2( l,a,b )			( a,b )
# define PROTO3( l,a,b,c )			( a,b,c )
# define PROTO4( l,a,b,c,d )			( a,b,c,d )
# define PROTO5( l,a,b,c,d,e )			( a,b,c,d,e )
# define PROTO6( l,a,b,c,d,e,f )		( a,b,c,d,e,f )
# define PROTO7( l,a,b,c,d,e,f,g )		( a,b,c,d,e,f,g )
# define PROTO8( l,a,b,c,d,e,f,g,h )		( a,b,c,d,e,f,g,h )
# define PROTO9( l,a,b,c,d,e,f,g,h,i )		( a,b,c,d,e,f,g,h,i )
# define PROTO10(l,a,b,c,d,e,f,g,h,i,j )	( a,b,c,d,e,f,g,h,i,j )
#else
# define PROTOE( l )				()

# define PROTO0( l )				l
# define PROTO1( l,a )				l a;
# define PROTO2( l,a,b )			l a;b;
# define PROTO3( l,a,b,c )			l a;b;c;
# define PROTO4( l,a,b,c,d )			l a;b;c;d;
# define PROTO5( l,a,b,c,d,e )			l a;b;c;d;e;
# define PROTO6( l,a,b,c,d,e,f )		l a;b;c;d;e;f;
# define PROTO7( l,a,b,c,d,e,f,g )		l a;b;c;d;e;f;g;
# define PROTO8( l,a,b,c,d,e,f,g,h )		l a;b;c;d;e;f;g;h;
# define PROTO9( l,a,b,c,d,e,f,g,h,i )		l a;b;c;d;e;f;g;h;i;
# define PROTO10(l,a,b,c,d,e,f,g,h,i,j )	l a;b;c;d;e;f;g;h;i;j;
#endif
