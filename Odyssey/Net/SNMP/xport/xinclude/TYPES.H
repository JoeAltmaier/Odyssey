/*-
 * Copyright (c) 1982, 1986, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)types.h	8.4 (Berkeley) 1/21/94
 */

#ifndef _SYS_TYPES_H_
#define	_SYS_TYPES_H_

#ifdef __STDC__
#define __P(x) x
#else
#define __P(x) ( )
#endif /* __STDC__ */

#ifndef _POSIX_SOURCE
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned long	u_int;
typedef	unsigned long	u_long;
#ifndef TARGET_H
typedef	unsigned short	ushort;		/* Sys V compatibility */
typedef	unsigned long	uint;		/* Sys V compatibility */
typedef long		int32;
typedef int		int16;
#endif
#endif

typedef	char *		caddr_t;	/* core address */
typedef	long		daddr_t;	/* disk address */
typedef	unsigned long	dev_t;		/* device number */
typedef unsigned long	fixpt_t;	/* fixed point number */
typedef	unsigned long	gid_t;		/* group id */
typedef	unsigned long	ino_t;		/* inode number */
typedef	unsigned short	mode_t;		/* permissions */
typedef	unsigned short	nlink_t;	/* link count */
typedef	long		pid_t;		/* process id */
typedef	long		segsz_t;	/* segment size */
typedef	long		swblk_t;	/* swap offset */
typedef	unsigned long	uid_t;		/* user id */

#ifndef _BSD_SIZE_T_
#define _BSD_SIZE_T_	unsigned int	/* sizeof() */
typedef _BSD_SIZE_T_	size_t;
#endif

#define _BSD_SSIZE_T_   int             /* byte count or error */
typedef _BSD_SSIZE_T_   ssize_t;

#if 0
/*
 * Select uses bit masks of file descriptors in longs.  These macros
 * manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here should
 * be enough for most uses.
 */
#define	FD_SETSIZE	256

#define NBBY	8		/* Number of bits per byte. */

typedef long	fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */

#define	howmany(x, y)	(((x)+((y)-1))/(y))

typedef	struct fd_set {
	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;
#endif

#if 0
#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define	FD_COPY(f, t)	bcopy(f, t, sizeof(*(f)))
#define	FD_ZERO(p)	bzero(p, sizeof(*(p)))
#else
#define	FD_SET(n, p)	NU_FD_Set( n, p )
#define	FD_CLR(n, p)	NU_FD_Reset( n, p )
#define	FD_ISSET(n, p)	NU_FD_Check( n, p )
#define	FD_COPY(f, t)	memmove(t, f, sizeof(*(f)))
#define	FD_ZERO(p)		NU_FD_Init( p )
#endif

#endif /* !_SYS_TYPES_H_ */
