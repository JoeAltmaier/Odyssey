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
 | FILE NAME   : xtypes.h
 | VERSION     : 1.1
 | COMPONENT   : XFAMILY
 | DESCRIPTION : Portable types for XFAMILY
 | AUTHOR      : Robert Winter
 *************************************************************************/
#ifndef _XTYPES_H_
#define _XTYPES_H_

/*
 * X-defines
 */
#define	BIT(x)					(1 << (x))
#ifndef LOWORD
#define	LOWORD(x)				(u16)((u32)(x) & 0xffff)
#endif
#ifndef HIWORD
#define	HIWORD(x)				(u16)((u32)(x) >> 16)
#endif
#define	W_SIZ(x)				(sizeof(x)/sizeof(u16))

#define	TRUE					1
#define	FALSE					0
#define	ON						TRUE
#define	OFF						FALSE

#ifndef NULL
#define	NULL					0
#endif

#define	staticf					static
#define	reg						register

#define	A_CNT(arr)				(sizeof(arr)/sizeof(arr[0]))
#define	A_END(arr)				(&arr[A_CNT(arr)])
#define	A_LAST(arr)				(&arr[A_CNT(arr)-1])

#define	nil(x)					((x)0)
#define	nilp(x)					((x *)0)
#define	max(a,b)				(((a) > (b)) ? (a) : (b))
#define	min(a,b)				(((a) < (b)) ? (a) : (b))

#define	align(x,y)				(((u32)x + ((u32)y - 1)) & ~((u32)y - 1))
#define	cntrl(x)				(x & 0x1f)

#define XSNMP_SPRINTF_NUM_PARMS	8
/*
 * X-types
 * DOS lives (and dies) in a world all by itself....
 */
#ifdef XLIB_BORLAND

typedef	signed char				i8;			/* also called CHAR          */
typedef	unsigned char			u8;			/* also called BYTE, UCHAR   */
typedef	signed short			i16;		/* also called SHORT         */
typedef	unsigned short			u16;		/* also called USHORT, WORD  */
typedef	signed long				i32;		/* also called INT           */
#ifndef TYPES_H
typedef	unsigned long			u32;		/* also called UINT          */
typedef	signed long				l32;		/* also called LONG          */
#endif
typedef	unsigned long			ul32;		/* also DWORD, LWORD, ULONG  */
typedef	volatile char			hw_i8;
typedef	volatile unsigned char	hw_u8;
typedef	volatile short			hw_i16;
typedef	volatile unsigned short	hw_u16;
typedef	volatile signed long	hw_i32;
typedef	volatile unsigned long	hw_u32;
typedef	volatile long			hw_l32;
typedef	volatile unsigned long	hw_ul32;
typedef	const char				c_i8;
typedef	const unsigned char		c_u8;
typedef	unsigned short			Bit_t;
typedef	unsigned short			Bit;
typedef	void					Func();
typedef	i32						(*pfi_t)();
typedef	u32						(*pfu_t)();
typedef	void					(*pfv_t)();
typedef	void					(*pfvi_t)( i32 );
typedef	i32						Bool_t;
#ifndef TYPES_H
typedef	i32						boolean;
#endif
typedef	i32						bool;
typedef	const void				c_void;

#else

/*
 * Skip these typedefs if building Driver.lib (EV64120 or ODYSSEY)
 * Leave them alone if building SNMP.lib
 */
#ifndef INCLUDE_EV64120
#ifndef INCLUDE_ODYSSEY
typedef	char					i8;			/* also called CHAR          */
typedef	unsigned char			u8;			/* also called BYTE, UCHAR   */
typedef	signed short			i16;		/* also called SHORT         */
typedef	unsigned short			u16;		/* also called USHORT, WORD  */
typedef	signed int				i32;		/* also called INT           */
#endif  INCLUDE_ODYSSEY
#endif  INCLUDE_EV64120
#ifndef TYPES_H
typedef	unsigned long			u32;		/* also called UINT          */
typedef	signed long				l32;		/* also called LONG          */
#endif
typedef	unsigned long			ul32;		/* also DWORD, LWORD, ULONG  */
typedef	volatile char			hw_i8;
typedef	volatile unsigned char	hw_u8;
typedef	volatile short			hw_i16;
typedef	volatile unsigned short	hw_u16;
typedef	volatile int			hw_i32;
typedef	volatile unsigned int	hw_u32;
typedef	volatile long			hw_l32;
typedef	volatile unsigned long	hw_ul32;
typedef	const char				c_i8;
typedef	const unsigned char		c_u8;
typedef	unsigned short			Bit_t;
typedef	unsigned short			Bit;
typedef	void					Func(void);
typedef	int						(*pfi_t)();
typedef	u32						(*pfu_t)();
typedef	void					(*pfv_t)();
typedef	void					(*pfvi_t)( i32 );
typedef	int						Bool_t;
#ifndef TYPES_H
typedef	int						boolean;
#endif
#ifndef __cplusplus
typedef	int                 bool;
#endif
typedef	const void				c_void;

#endif

typedef struct parm_list_s {
	u32		*longptr;
	u32		longval;
	i32		*intptr;
	i32		intval;
	i8		charval;
	i8		*charptr;
} parm_list_t;

extern	parm_list_t plist[];

/*
 * X-inlines
 */
#if 0
static __inline__
	u32	upper(u32 x) {return ((x >> 16) & 0xFFFF);}

static __inline__
	u32	lower(u32 x) {return (x & 0xFFFF);}

static __inline__
	u32	make32(u32 h, u32 l) {return ((h << 16) | (l & 0xFFFF));}

static __inline__
	u16	make16(u16 h, u16 l) {return ((h << 8) | (l & 0xFF));}

#endif
#endif

