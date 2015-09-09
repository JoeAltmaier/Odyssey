/*@***********************************************************************
 |
 |             Copyright (c) 1995-1997 XACT Incporated
 |
 | PROPRIETARY RIGHTS of XACT Incorporated are involved in the subject
 | matter of this material.  All manufacturing, reproduction, use, and
 | sales rights pertaining to this subject matter are governed by the
 | license agreement.  The recipient of this software implicitly accepts
 | the terms of the license.
 |
 |
 | FILE NAME   : xtern.c
 | VERSION     : 1.1
 | COMPONENT   : XFAMILY
 | DESCRIPTION : External references
 | AUTHOR      : Robert Winter
 *************************************************************************/
void	x_reboot(void);
void	x_insque(void *ein, void *previn);
void	x_remque(void *ein);
i32		x_meminit(ul32 size);
void	*x_malloc(ul32 size);
void	x_free(void *memblock);
void	*x_realloc(void *memblock, ul32 size);
void	x_bcopy(i8 *s1, i8 *s2, u32 len);
i32		x_bcmp(c_void *s1, c_void *s2, u32 len);
void	x_bzero(i8 *s1, u32 len);
void    x_memcpy(void *s1, c_void *s2, u32 n);
i32		x_memcmp(c_void *s1, c_void *s2, u32 n);
void	*x_memset(void *s1, i32 c, u32 n);
i8		*x_strcat(i8 *d, c_i8 *s);
u32		x_strlen(c_i8 *s);
i32		x_strcmp(c_i8 *s1, c_i8 *s2);
i8		*x_strchr(c_i8 *s, i32 c);
i8		*x_strcpy(i8 *d, c_i8 *s);
i8		*x_strtok(i8 *s1, c_i8 *s2);
i8		*x_strncpy(i8 *d, c_i8 *s, u32 n);
i32		x_putnstr(i8 *s, i32 n);
i32		x_putstr(i8 *s);
ul32	x_htonl(ul32 hostlong);
u16		x_htons(u16 hostshort);
u16		x_ntohs(u16 netshort);
ul32	x_ntohl(ul32 netlong);
ul32	x_inet_addr(c_i8 *cp);
i8		*x_inet_ntoa(const struct in_addr in);
i32		x_socket(i32 domain, i32 type, i32 protocol);
i32		x_connect(i32 s, struct sockaddr *name, i32 namelen);
i32		x_close(i32 s);
i32		x_bind(i32 s, void *name, i32 namelen);
i32		x_sendto(i32 s, c_i8 *msg, i32 len, i32 flags, void *to, i32 totlen);
i32		x_recvfrom(i32 c, i8 *buf, i32 len, i32 flags, void *f, i32 *flen);
i32		x_atoi(c_i8 *str);
l32		x_atol(c_i8 *str);
i32		x_abs(i32 val);
void    x_qsort( void *base, u32 nel, u32 width, i32( *ptr)(const void *, const void *) );
i32		x_gettimeofday(struct timeval *tp, void *p);
void	x_dbg(i8 *msg, u32 flag);
void	x_msg(i8 *msg);
void	x_msgstr(i8 *msg, i8 *str);
ul32	x_timeusec(void);
ul32	x_timemsec(void);
ul32	x_timeticks(void);
ul32	x_timesec(void);
bool	x_timerinit(void);
void	x_timeout(void(*func)(void *), void *arg, i32 ticks);
void	x_untimeout(void(*func)(void *), void *arg);
void	x_sprintf( i8 *buf, c_i8 *fmt);

