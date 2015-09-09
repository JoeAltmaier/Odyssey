/****************************************************************
	IEEE floating point include file	02-06-86
	Copyright 1986 Software Development Systems, Inc.
	All rights reserved
****************************************************************/

/****************************************************************
	Precede all global names with an underscore so they
	do not conflict with names in the user's code.  These
	functions are not intended to be called by the user,
	but will be called by compiler generated code sequences.
****************************************************************/
#ifdef HIDE
#define addd	_addd
#define adds	_adds
#define addul	_addul
#define addull	_addull
#define atod	_atod
#define atos	_atos
#define cmpd	_cmpd
#define cmps	_cmps
#define dblprec _dblprec
#define dbltod	_dbltod
#define dbltox81 _dbltox81
#define divd	_divd
#define divs	_divs
#define divul	_divul
#define divull	_divull
#define dtoa	_dtoa
#define dtodbl	_dtodbl
#define dtol	_dtol
#define dtoprtf	_dtoprtf
#define dtos	_dtos
#define fabs_d	_fabs_d
#define fabs_s	_fabs_s
#define fh1ul	_fh1ul
#define fh1ull	_fh1ull
#define frexp_d	_frexp_d
#define frexp_s	_frexp_s
#define ldexp_d	_ldexp_d
#define ldexp_s	_ldexp_s
#define ltod	_ltod
#define ltos	_ltos
#define modf_d	_modf_d
#define modf_s	_modf_s
#define muld	_muld
#define muls	_muls
#define mulul	_mulul
#define mulull	_mulull
#define negd	_negd
#define negs	_negs
#define normd	_normd
#define norms	_norms
#define nrmul	_nrmul
#define nrmull	_nrmull
#define round	_round
#define sgltos	_sgltos
#define shlul	_shlul
#define shlull	_shlull
#define shrul	_shrul
#define shrull	_shrull
#define stoa	_stoa
#define stod	_stod
#define stol	_stol
#define stoprtf	_stoprtf
#define stosgl	_stosgl
#define subd	_subd
#define subs	_subs
#define tstd	_tstd
#define tstdinf	_tstdinf
#define tstmd	_tstmd
#define tstms	_tstms
#define tsts	_tsts
#define tstsinf	_tstsinf
#define tstx81	_tstx81
#define x81todbl _x81todbl
#define zerod	_zerod
#define zeros	_zeros
#endif

/****************************************************************
		    Some constants.
****************************************************************/
#define MASK32	0xffffffffl	/* not necessarily the same as ~0 */

#define BIT(bit)	((unsigned long)1 << (bit))

#define EMAXS	127	/* maximum exponent for single format */
#define EMINS	(-126)	/* minimum exponent for single format */
#define EBIASS	127	/* exponent bias for single format */

#define EMAXD	1023	/* maximum exponent for double format */
#define EMIND	(-1022)	/* minimum exponent for double format */
#define EBIASD	1023	/* exponent bias for double format */

/****************************************************************
	The SHORT types in this file must hold at least 16 bits,
	and the LONG types must hold exactly 32 bits.  The exact
	requirement permits a structure of two LONGs to be
	mapped over a 64-bit floating point number.  Use of
	the default INT size will be assumed by the templates.

	As the functions in this directory are called directly
	by compiler templates, the data type sizes need not
	be compatible with the type sizes the user is compiling
	code with, except that POINTER-TO-DATA must be the same
	since it is used for argument passing.  The mklib
	script will zero out most type sizes with a -O option
	to the assembler, so that the linker will not complain
	about type changes that do not matter.
****************************************************************/

/****************************************************************
	The following structures hold IEEE format numbers.
	Members "hi" or "lo" hold 32 bits each, with "hi"
	holding the half of the double precision number
	that contains the exponent.  Only two byte orders
	are currently supported: high byte first and
	low byte first (indicated by _LOFIRST being defined).
	The bytes in the longs must be ordered the same as
	the two longs are ordered in the 64-bit double.
	The Z80 and 8086 are _LOFIRST.
****************************************************************/
typedef struct {
# ifdef _LOFIRST
#	define DBLTWO(hi,lo)	lo,hi
    unsigned long lo;
    unsigned long hi;
# else
#	define DBLTWO(hi,lo)	hi,lo
    unsigned long hi;
    unsigned long lo;
# endif
} idbl;

typedef struct {
    unsigned long hi;
} isgl;

/****************************************************************
	The following structure holds an IEEE extended
	precision number.  The exact representation depends
	on the coprocessor involved.
****************************************************************/
typedef struct {
# ifdef _LOFIRST
    unsigned long lo;
    unsigned long mid;
    unsigned long hi;
# else
    unsigned long hi;
    unsigned long mid;
    unsigned long lo;
# endif
} ixtd;

/****************************************************************
	The following format is used during floating point
	computations.  When used as single, the value mlo
	is not used.  When used as double, the value mlo
	is an additional 32 bits of mantissa.  In either case,
	the most significant part of the mantissa is the 32
	bits in mhi, with an assumed radix point between
	bits 31 and 30 (lsb is bit 0).  The exponent on any
	operand number is in the range:
		-16382 <= exp <= +16383
	which means that two operand exponents can be added
	together, the result can be incremented or decremented,
	and the result will still fit in exp without overflowing.
	The mantissa parts of a nan are preserved, except to be
	modified if necessary to keep an invented NAN consistent
	with the "nan" structure member information.
	Although the mantissa for single and double here are
	32 and 64 bits, the bits beyond 24 and 53 bits are
	merely used to hold information required for rounding
	to the actual single or double format, and once an
	operation is completed, all those bits are zeroed.
	Thus there are no extended format operations.
	(To implement extended would required keeping three
	bits beyond 32 and 64 bits, two exact, and one that
	indicates if all bits after those two are zero;
	otherwise there is not enough information to do
	all the rounding operations correctly, e.g. the case
	where the unnormalized result has a high bit of zero
	(one bit) and the first bit that would overflow the
	format is one (one bit) and all following bits are
	zero (one fuzzy bit).
****************************************************************/
typedef struct {
    unsigned long mhi;	  /* hi 32 bits of mantissa */
    unsigned long mlo;	  /* lo 32 bits of mantissa (not used for single) */
    short exp;		  /* signed exponent (no bias) */
    unsigned int  sgn :1; /* sign of number, 1 for negative */
    unsigned int  inf :1; /* infinity flag, 1 for infinity */
    unsigned int  nan :2; /* not a number: 0=number, else QNAN or SNAN */
#	define QNAN	1	/* quiet NaN */
#	define SNAN	2	/* signalling NaN */
} dble;

#define rdble register dble

/****************************************************************
	One of the four rounding modes should be in "round".
	The default should be TO_NEAR.
****************************************************************/
#define	TO_NEAR	0	/* round to nearest (default so guarenteed 0) */
#define	TO_PINF	1	/* round toward plus infinity */
#define	TO_MINF	2	/* round toward minus infinity */
#define	TO_ZERO	3	/* round toward zero */

/****************************************************************
	One of the two precision modes should be in dblprec
	when doing double precision arithmetic.
****************************************************************/
#define TO_DBL	0	/* round to double precision (default guarenteed 0) */
#define TO_SGL	1	/* round to single precision */

char *dtoa PROTOE((char *, int, idbl *, int n, int *, int *));
char *stoa PROTOE((char *, int, isgl *, int, int *, int *));

void addd PROTOE((idbl *, idbl *, idbl *));
void adds PROTOE((isgl *, isgl *, isgl *));
void dbltod PROTOE((idbl *, dble *));
void dbltox81 PROTOE((ixtd *, dble *));
void divd PROTOE((idbl *, idbl *, idbl *));
void divs PROTOE((isgl *, isgl *, isgl *));
void divul PROTOE((ulong *, ulong *, ulong *));
void divull PROTOE((ulong *, ulong *, ulong *, ulong *, ulong *, ulong *));
void dtodbl PROTOE((dble *, idbl *));
void dtol PROTOE((long *, idbl *, long));
void dtos PROTOE((isgl *, idbl *));
void fabs_d PROTOE((idbl *));
void fabs_s PROTOE((isgl *));
void frexp_d PROTOE((idbl *, long *));
void frexp_s PROTOE((isgl *, long *));
void ldexp_d PROTOE((idbl *, long *, long *));
void ldexp_s PROTOE((isgl *, long *, long *));
void ltod PROTOE((idbl *, long, long));
void ltos PROTOE((isgl *, long, long));
void modf_d PROTOE((idbl *, idbl *, idbl *));
void modf_s PROTOE((isgl *, isgl *, isgl *));
void muld PROTOE((idbl *, idbl *, idbl *));
void muls PROTOE((isgl *, isgl *, isgl *));
void mulul PROTOE((ulong *, ulong *, ulong *, ulong *));
void mulull PROTOE((ulong *, ulong *, ulong *, ulong *, ulong *, ulong *));
void negd PROTOE((idbl *, idbl *));
void negs PROTOE((isgl *, isgl *));
void normd PROTOE((idbl *, dble *));
void norms PROTOE((isgl *, dble *));
void sgltos PROTOE((isgl *, dble *));
void stod PROTOE((idbl *, isgl *));
void stol PROTOE((long *, isgl *, long));
void stosgl PROTOE((dble *, isgl *));
void subd PROTOE((idbl *, idbl *, idbl *));
void subs PROTOE((isgl *, isgl *, isgl *));
void x81todbl PROTOE((dble *, ixtd *));
void zerod PROTOE((idbl *));
void zeros PROTOE((isgl *));

#ifdef PROTO	/* prune int ones */
int addul PROTOE((ulong *, ulong *));
int addull PROTOE((ulong *, ulong *, ulong *, ulong *));
int atod PROTOE((idbl *, char **, int));
int atos PROTOE((isgl *, char **, int));
int cmpd PROTOE((idbl *, idbl *));
int cmps PROTOE((isgl *, isgl *));
int fh1ul PROTOE((ulong));
int fh1ull PROTOE((ulong, ulong));
int nrmul PROTOE((ulong *));
int nrmull PROTOE((ulong *, ulong *));
int shlul PROTOE((ulong *, int, int));
int shlull PROTOE((ulong *, ulong *, int, int));
int shrul PROTOE((ulong *, int, int));
int shrull PROTOE((ulong *, ulong *, int, int));
int tstd PROTOE((idbl *));
int tstdinf PROTOE((idbl *));
int tstmd PROTOE((idbl *));
int tstms PROTOE((isgl *));
int tsts PROTOE((isgl *));
int tstsinf PROTOE((isgl *));
int tstx81 PROTOE((ixtd *));
#endif

extern const int dblprec;
extern const int round;
