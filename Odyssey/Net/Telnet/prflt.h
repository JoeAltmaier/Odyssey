/****************************************************************
	Definitions related to floating to text conversion.
	Copyright 1989 Software Development Systems, Inc.
	All rights reserved
****************************************************************/

/********************************************************
    Structure to hold floating point printf parameters.
********************************************************/

struct prflt
{
    char lpad, rpad;	/* left and right padding characters */

    char sign;		/* print a sign even if positive: 0 => no, 1 => yes */
    char alternate;	/* alternate form: 0 => no, nonzero => yes */
			/*  when passed to padsign with numalt==2, this */
			/*  must contain the 2nd alternate character */
    char blank;		/* (when !sign) print blank for positive:0=>no,1=>yes*/
    char left;		/* justification: 0 => right, 1 => left */

    int width;		/* field width: for default width use -1 */
    int precision;	/* precision: for default precision use -1 */
};

/* default initializer */
#define PRP_DEFAULT(p)	{\
			p.lpad = ' ';\
			p.rpad = ' ';\
			p.sign = 0;\
			p.alternate = 0;\
			p.blank = 0;\
			p.left = 0;\
			p.width = -1;\
			p.precision = -1; }

/****************************************************************
	MAXNS, MAXND and MAXNDBL are the maximum number of
	significant digits to convert to character form for
	IEEE single precision, IEEE double precision, and C
	"double" type numbers respectively.  (Note: we do not
	depend on MAXNDBL for compilers other than our own.)

	From the IEEE document: "... the implementor may, at his
	option, alter all significant digits after the ninth
	for single and seventeenth for double to other decimal
	digits, typically 0."
****************************************************************/
#define MAXNS	9
#define MAXND	17

#if _DBLBITS==32
#define MAXNDBL	MAXNS
#else
#define MAXNDBL	MAXND
#endif

int stoprtf PROTOE((int(*)PROTOE((int,char*)), char *, isgl *, int,
		    struct prflt *));
int dtoprtf PROTOE((int(*)PROTOE((int,char*)), char *, idbl *, int,
		    struct prflt *));
