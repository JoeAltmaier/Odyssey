/********************************************************
	Copyright 1989 Software Development Systems, Inc.

	Common internal printf routines.
********************************************************/

/************************************************************************
	Put out 'n' copies of the padding character 'c' using the 'put'
	function.  No characters will be output if n <= 0.
	Return nonzero if a 'put' fails.
************************************************************************/
static int
padit PROTO4( (n, c, put, where),
	rint n, rint c, rint (*put)PROTOE((int,char *)), rchar *where )
{
    while( n-- > 0 ) {
	if( (*put)( c, where ) != c ) return 1; }
    return 0;
}

/****************************************************************
	Print any left-sided padding and any
	sign or alternate characters.
	Return nonzero if a 'put' fails.
****************************************************************/
static int
padsign PROTO6( (prtsign, numalt, numpad, pp, put, where),
int		prtsign,	/* whether a sign should be printed */
int		numalt,		/* number of alternate characters to print */
int		numpad,		/* number of pad characters */
struct prflt	*pp,		/* print parameters */
int (*put)PROTOE((int,char *)),	/* routine to put a character */
char		*where )	/* where characters are put */
{
    register int	alt = pp->alternate;
    register int	lp  = pp->lpad;

    /********************************
	Print NON-ZERO padding if
	field is not left justified
    ********************************/
    if( (lp != '0') && !pp->left && padit(numpad, lp, put, where) ) {
	return -1; }

    /****************************************
	Print the optional sign character.
    ****************************************/
    if( prtsign && ((*put)( prtsign, where ) != prtsign) ) return -1;

    /****************************************
	Print any alternate characters.
    ****************************************/
    if( alt && numalt ) {
	if( (*put)( '0', where ) != '0' ) return -1;
	if( (numalt == 2) && ((*put)( alt, where ) != alt) ) return -1; }

    /********************************
	Print zero padding if
	field is not left justified
    ********************************/
    if( (lp == '0') && !pp->left && padit(numpad, lp, put, where) ) {
	return -1; }

    return 0;
}
