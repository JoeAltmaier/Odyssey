/* Simple.h -- Simple types/defines unrelated to Odyssey
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * PLEASE DO NOT ADD ANY INCLUDES TO THIS FILE!!!!
 * This file is the root of all includes.
 *
 * Revision History:
 *     10-05-98  Tom Nelson: Created
 *     11-11-98  Joe Altmaier: include i2odep to get all the U32 etc.
 *     02-18-99  Jim Frandeen: fix I64 for Windows; Remove obsolete types.
 *     02-24-99  Jim Frandeen: Add Status so we can move away from Nucleus STATUS
 *     04-12-99  Jim Frandeen: Add UI64 as unsigned long long
 *     05-07-99  Eric Wedel: changed VOID from typedef to #define for GHills
 *
**/

#ifndef Simple_H
#define Simple_H

#ifndef __INCi2odeph	// Starting to remove i2odep.h
#define __INCi2odeph

#define I2ODEP_REV 1_5_4

/*
 * Pragma macros. These are to assure appropriate alignment between
 * host/IOP as defined by the I2O Specification. Each one of the shared
 * header files includes these macros.
 */

#define PRAGMA_ALIGN_PUSH   /*pragma pack( push, 1 )    */
#define PRAGMA_ALIGN_POP    /*pragma pack( pop )        */
#define PRAGMA_PACK_PUSH    /*pragma pack( push, 1 )    */
#define PRAGMA_PACK_POP     /*pragma pack( pop )        */


/* Setup the basics */


#ifndef NUCLEUS
typedef    unsigned long	   UNSIGNED;
typedef    int                 STATUS;
#endif

typedef    signed char  S8;
typedef    short  S16;

typedef    unsigned char  U8;
typedef    unsigned short  U16;

typedef    unsigned long  U32;
typedef    long  S32;
typedef    int  I32;
typedef    int  Status;  // Make it the same as Nucleus STATUS


/* Bitfields */

typedef    unsigned  BF;

/* VOID */
#ifndef __VOID
#ifdef GREEN_HILLS
#define VOID void    /* #define instead of typedef, for GH, NOT Win */
#else
#ifdef _WIN32
#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif // VOID
#else
typedef void VOID;
#endif
#endif // _WIN32
#define __VOID
#endif //  __VOID

/* Boolean */

#ifndef __BOOL
#define __BOOL

typedef int  BOOL; 
#endif


/* NULL */

//#define NULL  

#endif /* __INCi2odeph */

#ifndef __INCi2otypesh	// Starting to remove i2otypes.h
#define __INCi2otypesh

#define I2OTYPES_REV 1_5_4

/* 64 bit defines */

typedef struct _S64 {
   U32                         LowPart;
   S32                         HighPart;
} S64;

typedef struct _U64 {
   U32                         LowPart;
   U32                         HighPart;
} U64;

/* Pointer to Basics */

typedef    VOID                *PVOID;
typedef    S8                  *PS8;
typedef    S16                 *PS16;
typedef    S32                 *PS32;
typedef    S64                 *PS64;

/* Pointer to Unsigned Basics */

typedef    U8                  *PU8;
typedef    U16                 *PU16;
typedef    U32                 *PU32;
typedef    U64                 *PU64;

/* misc */

typedef S32             I2O_ARG;
typedef U32             I2O_COUNT;
typedef U32             I2O_USECS;
typedef U32             I2O_ADDR32;
typedef U32             I2O_SIZE;

#endif /* __INCi2otypesh */




/* Generic Error Return Codes (Erc) */

#define OK              0           /* Generic No-Error */
#define ERR	            (-1)        /* Generic Error    */

#ifndef NULL
#	define NULL			0L			/* Null pointer		*/
#endif


/* Generic Booleans */

#define TRUE	1				/* Boolean value    */
#define FALSE	0				/* Boolean value    */
#define true 	1				/* If you don't like caps */
#define false 	0

#define EOS     '\0'			/* End-Of-String    */

/* Basic Types */

#ifdef _WIN32
#pragma warning(disable : 4103)
typedef __int64	  I64;
typedef unsigned __int64	  UI64;
#else
#ifdef GREEN_HILLS
// Temporarily, Green Hills does not support 64-bit.
typedef long I64;
typedef unsigned long UI64;
#else
typedef long long I64;
typedef unsigned long long UI64;
#endif
#endif

#ifndef __BOOL
#define __BOOL
typedef int  BOOL; 				/* Boolean 				*/
#endif

typedef int		 ERC;           /* Error/Return Code 	*/
typedef double   REAL;          /* Generic floating pnt */


/* Obsolete, do not use */
#if 0
typedef char	 boolean;
typedef char 	 byte;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
#endif

#endif 	/* Simple_H */
