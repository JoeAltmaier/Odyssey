/* FmtSpec.c -- Printf() style formatting function.
 *
 * Copyright (c) 1985,86,87,89,98 by
 *      Tom Nelson  D.B.A. Clockwork Software
 *
 * Ancient History:
 *      Feb 85  MRH     Created using Desmet C
 *      Aug 85  JJA     Added centering
 *      Jan 86  TRN     Fixed leading fractional zeros
 *      Jan 86  TRN     Added %blank format
 *      May 86  TRN     Added precision spec  %.#c
 *      May 86  TRN     Made floating point optional
 *      Jul 86  TRN     Adding cheap rounding of floats
 *      Feb 87  TRN     Added %ls for length strings (1st byte length)
 *      Feb 87  TRN     Changed FuncArg to a pointer.
 *      Jul 87  TRN     Ported to Turbo C
 *      Aug 89  TRN     Added Upper/Lower case hex display.
 *               "      Make format items case intolerant.
 *               "      Fixed non-signed long hex display.
 *               "      Added binary format item '%b'.
 *               "      Added variable width/precision (%*.*) specs.
 *               "      Fixed precision spec for integers.
 *               "      Added pointer format item '%p'.
 *               "      Added Near & Far size override for pointer format.
 *
 * Revision History:
 *		7-01-97	 TRN  Converted to 32 bit MSVC++ v4.2
 *     10-01-97  TRN  Revised to use Stdarg.h 
 *
//
// ConvergeNet History:
// $Log: /Gemini/Odyssey/Util/FmtSpec.c $
// 
// 9     5/18/99 11:27a Jaltmaier
// Fix include duplicate 'UNSIGNED'.
// Add return to methods missing return.
// Prototype methods to avoid warning.
// 
// 8     4/26/99 1:43p Jhatwich
// win32 support
// 
// 7     4/26/99 1:23p Jhatwich
// 
// 6     4/23/99 6:49p Ewedel
// Changed _fmt() to receive argument list tail via standard va_list
// machinery.
// 
// 5     3/24/99 1:38p Ewedel
// Fixed bug with display of 64 bit integers which have the high bit set
// (a little Metrowerks and/or MIPS issue).  Also fixed so "%%" escape
// displays properly.
// 
// 4     3/23/99 11:11a Ewedel
// Fixed bug with rendering of unsigned 32-bit integer values having the
// high bit set.
// 
// 3     3/10/99 8:38a Jfrandeen
// Replace long long with I64 so it will work with GreenHills
// 
// 2     3/01/99 7:39p Ewedel
// Cleaned up some type mismatches, const-strengthened printf() format
// string handling.
//     10-21-98  Tom Nelson: Revised for Long Longs and removed %p format.
//
 *
**/

#include <string.h>
#include <stdarg.h>

#include "ct_fmt.h"


//#define FLOAT           /* Include floating point */
#define ROUNDING        /* Rounding of floats     */

#ifdef _WIN32
typedef __int64 		 LONG64;
typedef unsigned __int64 ULONG64;
#else
typedef I64 LONG64;			/* Type of %Ld 		*/
typedef I64 ULONG64; /* Type of %Lu, %Lx */
#endif

#define IsDigit(ch)     (((ch) >= '0' && (ch) <= '9') ? TRUE : FALSE)

/* Prototypes */

static
BOOL _ChkFor(char ch, const char **ppStr);

static
U32 _GetInt(const char **ppStr);

static
U32 _NtoA(char **ppStr, ULONG64 n, unsigned base, unsigned width, BOOL fCaps);

static
void  DivideBigNumber (ULONG64 n, unsigned int base,
                       ULONG64 *NdivBase, ULONG64 *NmodBase);

long longCheck;

/* _Fmt -- perform string formatting --------------------------------------*/
/*
 * This function formats the calling functions arguments using
 * the specified format and calls the given function for each
 * character to output.  Function returns the value from the
 * given function.  Any return from the specified function other
 * than zero will terminate _fmt().
 *
 * Formats:  %[flag][width][.precision][size]operator
 *	 flags:  -,+,=,0
 *   width:  minimum width number or *
 * precision: .number or .*
 *    size:  h,l,L
 * operator: c,s,d,u,x,X,o,b,f
 *
**/
STATUS  _fmt(pfnCharOut fnCharOut, void *pCookie,
             const char *pFmt, va_list pArg)
{
#ifdef FLOAT
    static double powerof10[] = {
        0.0,  10.0, 10e1, 10e2, 10e3, 10e4, 
        10e5, 10e6, 10e7, 10e8, 10e9, 10e10
    };
#ifdef ROUNDING
    static double rounding[] = {
        5.0e-1, 5.0e-2, 5.0e-3, 5.0e-4,  5.0e-5,  5.0e-6, 
        5.0e-7, 5.0e-8, 5.0e-9, 5.0e-10, 5.0e-11, 5.0e-12
    };
    double doubleNum;
#endif
#endif

    STATUS  erc;
    BOOL fCaps, fLeftJust, fCentered, fSigned;
    BOOL fConversion, fPrecSpec, fLongVal,fShortVal,fHugeVal;

#ifdef POINTERS    
    BOOL fFar;
#endif

    char ch, *pData, dataBuf[60], *pNum, fillChar;
    U32 width, precision, base, sData,w;
    U32 leftPad, rightPad, padding;
    LONG64  hugeArg;
    ULONG64 hugeNum;


    while ((ch = *pFmt++) != EOS)
        if (ch != '%') {
            if ((erc=(*fnCharOut)(pCookie,ch)) != OK)
                return erc;
        }  
        else if (*pFmt == '%') {
            //  two percent chars in a row means to output a single '%'
            pFmt++;     // skip second %
            if ((erc=(*fnCharOut)(pCookie,ch)) != OK)
                return erc;
        }
        else {
            fConversion = TRUE;
            fSigned = FALSE;
            fCentered = FALSE;
            fCaps = TRUE;
            pData = pNum = dataBuf;
            sData = base = 0;

            fLeftJust = _ChkFor('-', &pFmt);
            fCentered = _ChkFor( '=', &pFmt );

            fillChar  = _ChkFor('0', &pFmt)? '0' : ' ';
            if (_ChkFor('*',&pFmt)) 
                width = va_arg(pArg,int);
            else
                width = _GetInt(&pFmt);

            fPrecSpec = _ChkFor('.', &pFmt);
            if (fPrecSpec)
                if (_ChkFor('*',&pFmt))
                    precision = va_arg(pArg,int);
                else
                    precision = _GetInt(&pFmt);
            else
                precision = 6;

			fHugeVal = _ChkFor('L',&pFmt);
            fLongVal = _ChkFor('l', &pFmt);
			fShortVal = _ChkFor('h',&pFmt);            

            switch (ch = *pFmt++) {
            case ' ':   /* Blanks */
                fConversion = FALSE;
                while (width--)
                    if ((erc=(*fnCharOut)(pCookie, ' ')) != OK)
                        return erc;
                break;      

            case 'c':   /* Characters */
                if (!fPrecSpec) {
                    *pData = va_arg(pArg,int);
                    sData = 1;
                }
                else {
                    if (precision <= 2) 
                        *(int *) pData = va_arg(pArg,int);
                    else
                        *(long *) pData = va_arg(pArg,long);
                    sData = precision;
                }
                break;

            case 's':   /* Strings */
                pData = va_arg(pArg,char *);
                if (fLongVal || fHugeVal)
                    sData = *pData++;
                else
                    sData = strlen(pData);

                if (fPrecSpec && sData > precision)
                    sData = precision;
                break;

            case 'd':   /* Signed integers */
                fSigned = TRUE; 
            case 'u':   /* Unsigned integers */
                base = 10;
                break;

            case 'x':   /* Hex (Lowercase) */
                fCaps = FALSE;
            case 'X':   /* Hex (Uppercase) */
                base = 16;
                break;

            case 'o':   /* Octal */
                base = 8;
                break;
            case 'b':   /* Binary */
                base = 2;
                break;

#ifdef FLOAT
            case 'e':   /* Float */
            case 'g':
            case 'f':
                doubleNum = va_arg(pArg,double);
                longCheck = (long) doubleNum;
                if (doubleNum < 0.0) {
                    *pNum++ = '-';
                    sData = 1;
                    doubleNum = -doubleNum;
                }
#ifdef ROUNDING
                doubleNum += rounding[precision];   /* Cheap rounding fix */
#endif
                longNum = (long) doubleNum;
                sData += _NtoA(&pNum, longNum, 10, 0, fCaps);
                if (precision > 0) {
                    *pNum++ = '.';
                    ++sData;
                    doubleNum -= longNum;
                    sData += _NtoA(&pNum, (long) (doubleNum *
                                   powerof10[precision]), 10, precision, fCaps);
                }
                break;
#endif
               
            case '\0':
                return OK;

            default:
                fConversion = FALSE;
                if ((erc=(*fnCharOut)(pCookie, ch)) != OK)
                    return erc;     
            }

            if (base != 0) {
            	if (fSigned) {
	           		if (fHugeVal)
    	         	  	hugeArg = va_arg(pArg,LONG64);
              		else if (fLongVal)
            	 	   hugeArg = (LONG64) va_arg(pArg,long);
                	else if (fShortVal)
   	             	   hugeArg = (LONG64) va_arg(pArg,short);
	            	else /* default int */
		         	   hugeArg = (LONG64) va_arg(pArg,int);

	            	if (hugeArg >= 0)
	              	  hugeNum = hugeArg;
	           		else {
	                	*pNum++ = '-';
    	            	sData = 1;
        	        	hugeNum = -hugeArg;
	            	}
            	}
	            else { /* Unsigned */
                  /***BUG*** Metroworks ALWAYS sign extends to ULONG64!! */
	           		if (fHugeVal)
    	           		hugeNum = va_arg(pArg,ULONG64);
              		else if (fLongVal)
            	    	hugeNum = ((ULONG64) va_arg(pArg,unsigned long)) & 0xFFFFFFFF;
                	else if (fShortVal)
   	                	hugeNum = ((ULONG64) va_arg(pArg,unsigned short)) & 0xFFFF;
	            	else { /* default int */
                     //  under Metrowerks, "unsigned int" is 32 bits, but it
                     //  sign-extends to 64 bits (even into a ULONG64, hmm).
                     //  So we must mask it back down to a 32 bit number.
		            	hugeNum = ((ULONG64) va_arg(pArg,unsigned int)) & 0xFFFFFFFF;
                  }
	     		}     
				w = fPrecSpec ? precision : 0;	// MSC does not allow this a parm to func(WORD)
                sData += _NtoA(&pNum, hugeNum, base, w, fCaps);
            }

            if (fConversion) {
                padding = (width < sData) ? 0 : width - sData;
                leftPad = padding;
                rightPad = padding;
                if (fCentered) {
                    leftPad = padding / 2;
                    rightPad = leftPad;
                    if (leftPad + rightPad < padding) {
                        if (fLeftJust)
                            ++rightPad;
                        else
                            ++leftPad;
                    }
                }

                if (!fLeftJust || fCentered)
                    while (leftPad-- > 0) {
                        if ((erc=(*fnCharOut)(pCookie, fillChar)) != OK)
                            return erc;
                    } 
            
                while (sData--) {
                    if ((erc=(*fnCharOut)(pCookie, *pData++)) != OK)
                        return erc;
                }  

                if (fLeftJust || fCentered)
                    while (rightPad-- > 0) {
                        if ((erc=(*fnCharOut)(pCookie, fillChar)) != OK)
                            return erc;
                }
            }
        }
    return OK;

}  /* end of _fmt */


/* _Chkfor -- ckeck for presence of character ------------------------------*/
BOOL _ChkFor(
char ch,
const char **ppStr
)
{
    if (ch == **ppStr) {
        (*ppStr)++;
        return TRUE;
    }
    return FALSE;
}


/* _GetInt -- Return unsigned integer from string --------------------------*/
U32 _GetInt(
const char **ppStr
)
{
    U32 n = 0;

    while (IsDigit(**ppStr))
        n = n * 10 + *(*ppStr)++ - '0';

    return n;
}


/* _NtoA -- convert unsigned long long number to ascii ----------------------*/
/*
 * Convert unsigned long integer to ASCII in specified buffer.  The buffer
 * pointer is updated to point one past the last character placed in the
 * buffer.  The Function value returns the number of characters placed in
 * the buffer.  The string is NOT terminated with an EOS character.
 *
**/   
U32 _NtoA(
char **ppStr,   /* -> Addr of string buffer    */
ULONG64 n,      /* Number to convert           */
unsigned base,      /* Number Base for converstion */
unsigned width,     /* Field width for lead zeros  */
BOOL fCaps      /* Use upper case for hex      */
)
{
    U32     length;
    ULONG64 NdivBase;      // quotient of n / base
    ULONG64 NmodBase;      // modulus (n, base)


    //  decide whether we've hit the bottom of our recursion.
    if ((n < base) && (n >= 0)) {
        //  at the bottom, so it's time to emit any leading zeros.
        for (length = 1; length < width; length++)  /* Insert leading zeros */
            *(*ppStr)++ = '0';

        //  and display our (leftmost / most-significant) data digit
        if (n < 10) 
            *(*ppStr)++ = (char) n + '0';
        else
            *(*ppStr)++ = (char) n + (fCaps ? ('A' - 10) : ('a' - 10));

        return length;
    }

    //  compute remaining minimum field width
    width = (width==0) ? 0 : width-1;

    //  find our own (right-most) digit, and the most-significant remainder
    if (n >= 0) {
        //  can use regular MIPS / Metrowerks math operators
        NmodBase = n % (ULONG64) base;
        NdivBase = n / (ULONG64) base;
    }
    else {
        //  whoops, got what looks like a negative 64-bit value.
        //  We must do things in a curious way.
        DivideBigNumber (n, base, &NdivBase, &NmodBase);
    }

    //  process & display (most-significant) remainder of number
    length = _NtoA(ppStr, NdivBase, base, width, fCaps);

    //  and display our own (right-most) digit
    //  (note that we "anchor" the starting width at '1')
    _NtoA(ppStr, NmodBase, base, 1, fCaps);

    return length + 1;
}

/* DivideBigNumber - compute quotient & modulus of "negative" 64 bit value ----*/

static void  DivideBigNumber (ULONG64 n, unsigned int base,
                              ULONG64 *pNdivBase, ULONG64 *pNmodBase)
{

//                             1111110000000000
//                             5432109876543210    // 64 bits is a lot!
const ULONG64  ulLowBits  =  0x7FFFFFFFFFFFFFFF;
unsigned int   uiShift;          // bits needed to shift away least-sig digit
unsigned int   uiDigitMask;      // bits needed to extract least-sig digit
ULONG64        ulQuotient;
ULONG64        ulQuotScaled;

   //  what makes this challenging is that "ULONG64" actually is treated
   //  as a signed quantity.  So having the most significant bit (bit 63)
   //  set causes no end of confusion.  In essence, we must avoid using
   //  the processor's native division operations (div, mod) since they
   //  would not properly handle our input, which we are supposed to
   //  treat as an unsigned number despite the compiler and/or processor's
   //  predilection otherwise.

   //  Handling this for the power-of-two bases is relatively easy:  we
   //  just use shifts & masks.

   switch (base)
      {
      case 2:
         uiShift = 1;
         uiDigitMask = 0x01;
         break;

      case 8:
         uiShift = 3;
         uiDigitMask = 0x07;
         break;

      case 16:
         uiShift = 4;
         uiDigitMask = 0x0F;
         break;

      default:
         //  assume base == 10
         uiShift = 0;      // flag alternate algorithm for base 10
      }

   //  got our params set up, now do the ops
   if (uiShift > 0)
      {
      //  binary ops, this is easy-ish

      //  grab least-significant digit
      *pNmodBase = n & uiDigitMask;

      //  shift one bit right, and strip any sign extension
      ulQuotient = n >> 1;
      ulQuotient &= ulLowBits;      // bye bye sign extension
      if (uiShift > 1)
         {
         ulQuotient = ulQuotient >> (uiShift - 1);
         }

      *pNdivBase = ulQuotient;
      }
   else
      {
      //  base 10.  This is interesting.  We really want to use native
      //  division, but can't while that sign bit is hanging around.
      //  So instead, we right shift one, and then divide by 5.
      ulQuotient = n >> 1;
      ulQuotient &= ulLowBits;     // ulQuotient == n / 2
      ulQuotient /= 5;             // ulQuotient == n / 10

      //  now to find our least-significant digit, we multiply our remainder
      //  by 10, again being careful to avoid sign overflow.
      ulQuotScaled = ulQuotient * 5;
      ulQuotScaled = ulQuotScaled << 1;     // ulQuotScaled == ulQuotient * 10

      *pNdivBase = ulQuotient;
      *pNmodBase = n - ulQuotScaled;
      }

   return;

}  /* end of DivideBigNumber */

/* _movech -- put character into buffer for _fmt() ----------------------------*/

ERC _movech(void *pArgs,char ch)
{
	char **ppBuf;

	ppBuf = (char **) pArgs;

    *(char *) *ppBuf = ch;
    ((char *) (*ppBuf))++;

    return OK; 
}

