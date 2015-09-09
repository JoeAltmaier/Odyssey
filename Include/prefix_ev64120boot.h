/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: prefix_ev64120_s.h
//
// Description:
// This is a Metrowerks CodeWarrior Prefix File for assembly language
// code modules.  A prefix file is a file that the assembler processes
// before every assembly file in the project. It is as though you put
// the same .include directive at the beginning of every assembly file.
//
//
// Update Log:
// 10/12/98 JSN: Created.
/*************************************************************************/

#ifndef PREFIX_EV64120_H
#define PREFIX_EV64120_H 	1


/*
 * Define one of the following for target system dependancies.
 */
#define	INCLUDE_EV64120  1      // Flag for running on EV64120 systems.
#undef	INCLUDE_ODYSSEY         // Flag for running on ODYSSEY systems.
#define	CONFIG_BOOT		1
#undef	CONFIG_HBC		

#define TRK_TRANSPORT_INT_DRIVEN	1
#pragma once

/*
 *	Call instruction for pc relative and absolute mode
 *
 */
#ifdef __pc_relative__
#define	CALL	bal
#else
#define	CALL	jal
#endif


/*
 * Load and Store registers instructions for the given ISA level
 *
 */
#ifdef __MIPS_ISA3__

#define LOAD		ld					// Load register
#define	STORE		sd					// Store register
#define MOVE		dmove				// Register copy

#ifdef __ptr64__
#define LOADPTR		ld					// Load an address
#define STOREPTR	sd					// Store an address
#else
#define LOADPTR		lw					// Load an address
#define STOREPTR	sw					// Store an address
#endif

#else	// ISA 1

#define LOAD		lw					// Load register
#define	STORE		sw					// Store register
#define MOVE		move				// Register copy
#define LOADPTR		lw					// Load an address
#define STOREPTR	sw					// Store an address
#endif

#ifdef __fpu__
#ifdef __MIPS_ISA2__
#define LOADF		ldc1
#define STOREF		sdc1
#else
#define LOADF		lwc1
#define STOREF		swc1
#endif
#endif

/*
 * Board configurations depend on the IDT monitor.  MetroTRK
 * runs standalone.
 */
#define __IDT79S381__	(0)	/* Board type					*/
#define __IDT79S464__	(0) /* Board type					*/

#define __IDT_SIM__		(0) /* Running under IDT sim7 		*/

/*
** The TRK needs the following definitions.
*/

#define __CWDS__		(1)

#define __NEED_BOOL__

/*
** There are only a few minor differences between the 4xxx
** family and the 5xxx family.  For 5xxx, automatically 
** enable all of the 4xxx-specific code by setting __MIPS_4xxx__ 
** to 1.
*/

#define __MIPS_4xxx__	(1) /* one of the 4xxx family? */
#define __MIPS_5xxx__	(1) /* one of the 5xxx family? */

#define __MIPS_64__		(__MIPS_4xxx__)	/* 64-bit MIPS processor */

/* Board configuration										*/
/************************************************************/
/*															*/
/* R4000 processor configuration 							*/
/*															*/
/************************************************************/
/*
 *	'alignment' is a mask used to ensure that blocks allocated always have
 *	sizes that are multiples of a given power-of-two, in this case: eight. Other
 *	values are possible, but for internal reasons the alignment factor must be
 *	a multiple of four and must also be a multiple of sizeof(long).
 */
#define __ALIGNMENT__	8L
#endif PREFIX_EV64120_H
