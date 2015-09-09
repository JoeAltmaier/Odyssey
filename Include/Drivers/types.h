/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: types.h
 * 
 * Description:
 * This file contains typedefs for simple data types  
 * 
 * Update Log:
 * 10/12/98 Raghava Kondapalli: Created 
 * 11/06/98 Raghava Kondapalli: Addded ulong removed STATUS as it is defined in
 * 				nucleus.h 
 * 02/12/99 Jim Frandeen: combine uses of long long to one definition.
 * 05/10/99 Eric Wedel: changed VOID from typedef to #define (std C, GH).
 ************************************************************************/

/*
 * If including i20types.h don't include this file
 */
#ifndef __INCi2otypesh

#ifndef TYPES_H
#define TYPES_H

#ifndef VOID
# define  VOID    void
#endif

typedef	unsigned int		U32;
typedef	unsigned long long	U64;
typedef	unsigned short		U16;
typedef	unsigned char		U8;

typedef	int			S32;
typedef	int			I32;
typedef	U64			I64;
typedef	short			I16;
typedef	char			I8;

typedef U64			u64;
typedef U64			i64;
typedef U64			uint64;
typedef U64			int64;
typedef U64			u_int64;
typedef unsigned int		u32;
typedef unsigned long		u_long;
typedef unsigned int		u_int32;
typedef unsigned int		u_int;
typedef unsigned short		u16;
typedef unsigned short		u_int16;
typedef unsigned short		u_short;
typedef unsigned char		u8;
typedef unsigned char		u_char;

/* target.h in Nulcues net also defined these types */
#ifndef	TARGET_H
typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned short		uint16;
typedef int					int32;
typedef unsigned int		uint;
typedef unsigned int		uint32;
typedef unsigned long		ulong;
#endif	/* TARGET_H */

typedef int			i32;
typedef int			l32;
typedef short			i16;
typedef char			i8;

typedef long			word;
typedef unsigned long		uword;
typedef U64			udword;
typedef U64			dword;
typedef unsigned long		Word;

typedef U64 			reg_t;
typedef char			boolean;
typedef char			BOOLEAN;

typedef unsigned long		jmp_buf[22];
#ifndef NUCLEUS
typedef int			STATUS;
#endif NUCLEUS
#endif TYPES_H
#endif __INCi2otypesh
