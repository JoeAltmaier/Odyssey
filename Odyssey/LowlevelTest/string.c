/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: string.c
 * 
 * Description:
 * This file contains string routines as part of C Library
 * 
 * Update Log:
 * 10/12/98 Raghava Kondapalli: Created 
 ************************************************************************/
#include <stdio.h>

char * strccat (char *dst, char c);
int strlen(char *);
char * memcpy(char *to, char *from, int len);
char * strcat (char *dst, char *src);
int	 memcmp(char *s1, char *s2, int size);

int
strlen(char *s1)
{
	int		i = 0;
	while (*s1++)
		i++;
	return (i);
}


char *
strcat(char *s, char *append)
{
	char *save = s;

	for (; *s; ++s)
		;
	while ((*s++ = *append++) != 0)
		;
	return(save);
}

char *
strcpy(char *to, char *from)
{
	char *save = to;

	for (; (*to = *from) != 0; ++from, ++to)
		;
	return(save);
}


char *
strncpy(char *s1, char *s2, int n)
{
	char *ch = s1;

	while ((n--) && (*s2)) {
		*s1++ = *s2++;
	}
    	if (n > 0)
	       	*s1=0;
	return ch;
}

/*
 * Compare strings.
 */
int
strcmp(char *s1, char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == 0)
			return (0);
	return (*(unsigned char *)s1 - *(unsigned char *)--s2);
}

/** char *strccat(dst,c)  concatinate char to dst string */
char *
strccat (char *dst, char c)
{
	int             len;

	if (!dst)
		return (dst);
	len = strlen (dst);
    	dst[len] = c;
    	dst[len + 1] = 0;

    	return (dst);
}

char *
strncat(char * dst, const char * src, size_t n)
{
	const	char * p = src;
	char * q = dst;
	
	while (*q++)
		;
	q--;
	n++;
	while (--n) {
		if (!(*q++ = *p++)) {
			q--;
			break;
		}
	}
	*q = 0;
	return(dst);
}

#ifndef CONFIG_BOOT
/** int strncmp(s1,s2,n) as strcmp, but compares at most n characters */
int 
strncmp (s1, s2, n)
     char           *s1, *s2;
     int             n;
{

    if (!s1 || !s2)
	return (0);

    while (n && (*s1 == *s2)) {
	if (*s1 == 0)
	  return (0);
	s1++;
	s2++;
	n--;
    }
    if (n)
      return (*s1 - *s2);
    return (0);
}


#define isspace(c)	((c == ' ') || (c == '\t') || (c == 0x08))
/** long atol(p) converts p to long */
long 
atol (p)
     char           *p;
{
    int             digit, isneg;
    long            value;

    isneg = 0;
    value = 0;
    for (; isspace (*p); p++) ;	/* gobble up leading whitespace */

/* do I have a sign? */
    if (*p == '-') {
	isneg = 1;
	p++;
    } else if (*p == '+')
	p++;

    for (; *p; p++) {
	value *= 10;
	if (*p >= '0' && *p <= '9')
	    digit = *p - '0';
	else
	    break;
	value += digit;
    }

    if (isneg)
	value = 0 - value;
    return (value);
}

char *
memcpy(char *to, char *from, int len)
{
	int	i;

	for (i = 0; i < len; i++)
		to[i] = from[i];
	return(to);
}


void *
memset(void *s, int c, int n)
{
	unsigned char	*ptr = (unsigned char *)s;
	unsigned char	ch = (unsigned char)c;
	int	i;

	for (i = 0; i < n; i++)
		ptr[i] = ch;
	
	return (s);
}
int	 
memcmp(char *s1, char *s2, int size)
{
	return(bcmp(s1, s2, size));
}
#endif

void *
MSL_Initialize(void *first_available_memory)
{
	return(first_available_memory);
}
