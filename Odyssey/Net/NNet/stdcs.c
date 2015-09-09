/*************************************************************************/
/*                                                                       */
/*     Copyright (c) 1993 - 1998 Accelerated Technology, Inc.            */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/
#include "stdcs.h"

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  stricmp                                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  The function compares two strings, continuing until a difference is     */
/*  found or the end of the strings is reached.                             */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
int stricmp(s1,s2)
register const char *s1;
register const char *s2;
{
    while( (toupper(*s1) == toupper(*s2)) && (*s1) ) ++s1, ++s2;

    return ((int)(unsigned char)*s1) - ((int)(unsigned char)*s2);
}

#ifdef SNN /* SPR0335 */
/* NOTE: when this function is removed, there should be no conflict when
         SNMP is built.
*/

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  toupper                                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  The function converts a character to upper case.                        */
/*                                                                          */
/****************************************************************************/
int toupper(int ch)
{
    if ( (ch < 'a') || (ch > 'z') )
        return ch;

    ch -= 32;

    return ch;

} /* toupper */
#endif

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  itoa                                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function converts value to a null terminated ascii string.         */
/*                                                                          */
/****************************************************************************/

char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr++ = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr++ = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = d + 0x30;
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr++ = 0;

    return string;

} /* itoa */

int isascii(int c)  
{
    return (unsigned int)c <= 0x7f; 
}
