/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: mips_reg.h
 * 
 * Description:
 * This file contains register definition for Galileo GT-64XXX PCI controllers 
 * 
 * Update Log:
 * XX/XX/98 JSN: Created.
 * 10/16/98  RK: Common files for TRK and nucleus. Added CP0 register bit defs.
 * 05/05/99 JSN: MetroChange: Symbolic definition of register names now need $.
 ************************************************************************/


/****************************************************************************/
/*

FILE
	
	MIPS_reg.h
	
	$Header: /Gemini/Include/mips_reg.h 10    1/12/00 4:06p Rkondapalli $
	
DESCRIPTION

	API for target-specific implementation.
	
	Register numbers for IDT MIPS 3041, 3051/51E/52/52E,
	3071/71E/81/81E, R4400, R4600, R4700 (32 bit only).
	
	Registers are accessed via the kDSReadRegisters and kDSWriteRegisters
	messages.  The register numbers for all registers are defined 
	below.  The registers are rougly categorized into blocks.
	All registers are 32 bits, so register data in messages is transmitted
	as a series of 32 bit values in big-endian format.
	
HISTORY
	
	$History: mips_reg.h $
// 
// *****************  Version 10  *****************
// User: Rkondapalli  Date: 1/12/00    Time: 4:06p
// Updated in $/Gemini/Include
// added watch register definitions
// 
// *****************  Version 9  *****************
// User: Rkondapalli  Date: 10/27/99   Time: 3:50p
// Updated in $/Gemini/Include
// added Set1 ICR register defines
// 
// *****************  Version 8  *****************
// User: Rkondapalli  Date: 5/11/99    Time: 9:33a
// Updated in $/Gemini/Include
// added RM7000 scache values
// 
// *****************  Version 7  *****************
// User: Jnespor      Date: 5/05/99    Time: 10:18a
// Updated in $/Gemini/Include
// The VSS Editor sucks!
// 
// When I made the 990503 MetroChange, it took
// a '#' from line 395 and put it at the end of the file.
// 
// *****************  Version 6  *****************
// User: Jnespor      Date: 5/05/99    Time: 9:24a
// Updated in $/Gemini/Include
// MetroChange for 990503 (backward compatible with 990324):
//   Symbolic definition of register names now need $.
// 
// *****************  Version 5  *****************
// User: Rkondapalli  Date: 4/13/99    Time: 3:20p
// Updated in $/Gemini/Include
// added a cache op
// 
// *****************  Version 4  *****************
// User: Rkondapalli  Date: 4/05/99    Time: 11:54a
// Updated in $/TreeRoot/Include
// added scache support for RM7xxx
// 
// *****************  Version 3  *****************
// User: Skasargod    Date: 12/22/98   Time: 7:49p
// Updated in $/TreeRoot/Include
// modified by raghava for Odyssey
//	
//	*****************  Version 9  *****************
//	User: Smoore       Date: 3/19/98    Time: 12:04a
//	Updated in $/Embedded/TRK_In_Progress/NEC_VR_TRK/MetroTRK/Processor/mips/export
//	Merged in old register scheme still used by the 3k TRK.
//	
//	*****************  Version 8  *****************
//	User: Smoore       Date: 12/12/97   Time: 3:08p
//	Updated in $/Embedded/TRK_In_Progress/NEC_VR_TRK/MetroTRK/Processor/mips/export
//	Removed all of registers in the default block that are just aliases to
//	registers in the extended1 block.
//	
//	Added control register defines pertinent to the NEC VR4100.
//	
//	*****************  Version 7  *****************
//	User: Smoore       Date: 12/08/97   Time: 7:43p
//	Updated in $/Embedded/TRK_In_Progress/NEC_VR_TRK/MetroTRK/Processor/mips/export
//	Added support for 64-bit registers.  Moved several MIPS-specific
//	register-related #defines from targimpl.c to this file.
//	
//	*****************  Version 5  *****************
//	User: Smoore       Date: 8/29/97    Time: 2:42p
//	Updated in $/Embedded/MetroTRK/Processor/mips/export
//	Added PCType, InstructionType, and ExceptionCauseType typedefs
//	to help determine field sizes in the NotifyStopped message.
//	
//	*****************  Version 4  *****************
//	User: Smoore       Date: 8/28/97    Time: 4:59p
//	Updated in $/Embedded/MetroTRK/Processor/mips/export
//	Completed implementation of MIPS TRK.
//	
//	*****************  Version 3  *****************
//	User: Smoore       Date: 8/07/97    Time: 1:37p
//	Updated in $/Embedded/MetroTRK/Processor/mips/export
//	Added register info for IDT MIPS R4xxx processors.
//	
//	*****************  Version 2  *****************
//	User: Smoore       Date: 8/06/97    Time: 4:18p
//	Updated in $/Embedded/MetroTRK/Processor/mips/mips3081e_export
//	Filled in w/MIPS register info for R30xx & R4xxx processors.
//	
//	*****************  Version 1  *****************
//	User: Smoore       Date: 8/05/97    Time: 6:44p
//	Created in $/Embedded/MetroTRK/Processor/mips/mips3081e_export
//	Description of registers for the MIPS 3081E processor.
//	
	
AUTHORS

	(c) 1997 Metrowerks Corporation
	All rights reserved.
	
*/
/****************************************************************************/
#ifndef MIPS_REG_H
#define MIPS_REG_H
#pragma once
#define TRK_DEFAULT_SIZE( x )		(sizeof( DefaultType ))

/*
** Note: Some register cleanup was done for the MIPS 4k TRK.
** It has not been propagated back into the 3k host plugin,
** so the 3k TRK has to use the old register scheme for now.
*/

#define TRK_DEFAULT_GPR( x )		((x) - 1)	/* r0 not included */
#define TRK_DEFAULT_LO				31
#define TRK_DEFAULT_HI				32
#define TRK_DEFAULT_PC				33

#define TRK_DEFAULT_MIN_REGISTER	TRK_DEFAULT_GPR( 1 )
#define TRK_DEFAULT_MAX_REGISTER	TRK_DEFAULT_PC


#define TRK_FP_SIZE( x )			(sizeof( FPType ))

#define TRK_FP_FPR( x )				(x)
#define TRK_FP_FPCSR				32
#define TRK_FP_FPIR					33	/* read-only */


#define TRK_FP_MIN_REGISTER			TRK_FP_FPR( 0 )
#define TRK_FP_MAX_REGISTER			TRK_FP_FPIR



/****************************************************************************/
/*
 * Extended registers block 1.
 *
 * Note: accessing registers that are not supported on the current
 * processor has no effect and does not result in an error.
 */
/****************************************************************************/
#define TRK_EXTENDED1_SIZE( x )		(sizeof( Extended1Type ))


#define TRK_EXTENDED1_INDEX			0	/* R30xxE & R4xxx only */
#define TRK_EXTENDED1_RANDOM		1	/* R30xxE & R4xxx only (read-only) */

#define TRK_EXTENDED1_ENTRYLO0		2	/* R30xxE & R4xxx only */
#define TRK_EXTENDED1_BUSCTRL		2	/* R3041 only */

#define TRK_EXTENDED1_CONFIG3		3	/* R3041 & R3081 only */
#define TRK_EXTENDED1_ENTRYLO1		3	/* R4xxx only */

#define TRK_EXTENDED1_CONTEXT		4	/* R30xxE & R4xxx only */
#define TRK_EXTENDED1_PAGEMASK		5	/* R4xxx only */
#define TRK_EXTENDED1_WIRED			6	/* R4xxx only */
#define TRK_EXTENDED1_CP0_7			7
#define TRK_EXTENDED1_BADVADDRESS	8
#define TRK_EXTENDED1_COUNT			9	/* R3041 & R4xxx only */

#define TRK_EXTENDED1_ENTRYHI		10	/* R30xxE & R4xxx only */
#define TRK_EXTENDED1_PORTSIZE		10	/* R3041 only */

#define TRK_EXTENDED1_COMPARE		11	/* R3041 only */
#define TRK_EXTENDED1_SR			12	/* same as default block SR */
#define	TRK_EXTENDED1_CR			13	/* same as default block CR */
#define TRK_EXTENDED1_EPC			14	/* same as default block PC */
#define TRK_EXTENDED1_PRID			15	/* read-only */
#define TRK_EXTENDED1_CONFIG4		16	/* R4xxxx only */
#define TRK_EXTENDED1_LLADDR		17	/* R4xxxx only */
#define TRK_EXTENDED1_WATCHLO		18	/* R4xxxx only */
#define TRK_EXTENDED1_WATCHHI		19	/* R4xxxx only */
#define TRK_EXTENDED1_XCONTEXT		20	/* R4xxxx only */
#define TRK_EXTENDED1_CP0_21		21
#define TRK_EXTENDED1_CP0_22		22
#define TRK_EXTENDED1_CP0_23		23
#define TRK_EXTENDED1_CP0_24		24
#define TRK_EXTENDED1_CP0_25		25
#define TRK_EXTENDED1_ECC			26	/* R4xxxx only */
#define TRK_EXTENDED1_CACHEERR		27	/* R4xxxx only */
#define TRK_EXTENDED1_TAGLO			28	/* R4xxxx only */
#define TRK_EXTENDED1_TAGHI			29	/* R4xxxx only */
#define TRK_EXTENDED1_ERROREPC		30	/* R4xxxx only */
#define TRK_EXTENDED1_CP0_31		31


#define TRK_EXTENDED1_MIN_REGISTER	TRK_EXTENDED1_INDEX
#define TRK_EXTENDED1_MAX_REGISTER	TRK_EXTENDED1_CP0_31

/****************************************************************************/
/*
 * Handy, processor-specific register defines.
 */
/****************************************************************************/

/*
** MIPS floating point coprocessor defines
*/

#define FP_COPROCESSOR			1

#define FPC_FPCSR				31
#define FPC_FPIR				0

/*
** CPU Control coprocessor defines.
*/

#define CONTROL_COPROCESSOR			0

#define CR_INDEX				$0
#define CR_RANDOM				$1
#define CR_ENTRYLO0				$2
#define CR_TLBLO				$2
#define CR_TLBLO0				$2
#define CR_TLBLO1				$3
#define CR_BUSCTRL				$2
#define CR_CONFIG3				$3
#define CR_ENTRYLO1				$3
#define CR_CONTEXT				$4
#define CR_PAGEMASK				$5
#define CR_WIRED				$6

#define CR_BADVADDRESS			$8
#define CR_COUNT				$9
#define CR_ENTRYHI				$10
#define CR_TLBHI				$10
#define CR_PORTSIZE				$10
#define CR_COMPARE				$11
#define CR_SR					$12
#define CR_CR					$13
#define CR_EPC					$14
#define CR_PRID					$15
#define CR_CONFIG4				$16
#define CR_LLADDR				$17
#define CR_WATCHLO				$18
#define CR_WATCHHI				$19
#define CR_XCONTEXT				$20
#define CR_ECC					$26
#define CR_CACHEERR				$27
#define CR_TAGLO				$28
#define CR_TAGHI				$29
#define CR_ERROREPC				$30

#define SR_RP					0x08000000
#define SR_ITS					0x01000000
#define SR_IM					0x0000ff00
#define SR_KSU					0x00000018

#define CONFIG4_EC				0x70000000
#define CONFIG4_EP				0x0f000000
#define CONFIG4_AD				0x00800000
#define CONFIG4_BE				0x00008000
#define CONFIG4_CU				0x00000008
#define CONFIG4_K0				0x00000007
#define CONFIG4_K0_CACHE		0x00000001
#define CONFIG4_K0_UNCACHE		0x00000002

/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: mips.h
 * 
 * Description:
 * Header File for mips processors  
 * 
 * Update Log:
 * 10/12/98 Raghava Kondapalli: Created 
 ************************************************************************/

#define _MIPS_H_
/*
 * System Control Coprocessor (CP0) memory-management registers
 */
#define	C0_INDEX	$0		/* TLB Index */
#define	C0_RANDOM	$1		/* TLB Random */
#define	C0_ENTRYLO0	$2		/* TLB EntryLo0 */
#define	C0_ENTRYLO1	$3		/* TLB EntryLo1 */
#define	C0_PAGEMASK	$5		/* TLB PageMask */
#define	C0_WIRED	$6		/* TLB Wired */
#define	C0_ENTRYHI	$10		/* TLB EntryHi */
#define	C0_PRID		$15		/* Processor Revision Indentifier */
#define	C0_CONFIG	$16		/* Config */
#define	C0_LLADDR	$17		/* LLAddr */
#define	C0_WATCH1	$18		/* Watch1 */
#define	C0_WATCH2	$19		/* Watch1 */
#define	C0_TAGLO	$28		/* TagLo */
#define	C0_TAGHI	$29		/* TagHi (always zero on the R4650) */


/*
 * System Control Coprocessor (CP0) exception processing registers
 */
#define	C0_CONTEXT	$4		/* Context */
#define	C0_BADVADDR	$8		/* Bad Virtual Address */
#define	C0_COUNT 	$9		/* Count */
#define	C0_COMPARE	$11		/* Compare */
#define	C0_STATUS	$12		/* Processor Status */
#define	C0_CAUSE	$13		/* Exception Cause */
#define	C0_EPC		$14		/* Exception PC */
#define	C0_XCONTEXT	$20		/* XContext */
#define	C0_PERF		$25		/* Perf Count */
#define	C0_ECC		$26		/* ECC */
#define	C0_CACHEERR	$27		/* CacheErr */
#define	C0_ERROREPC	$30		/* ErrorEPC */

/*
 * Config register fields (read only except for K0 cacheability attributes)
 */
#define	CFG_ECMASK	0x70000000	/* System Clock Ratio: */
#define	CFG_ECBY2	0x00000000 	/*  processor clock divided by 2 */
#define	CFG_ECBY3	0x10000000 	/*  processor clock divided by 3 */
#define	CFG_ECBY4	0x20000000 	/*  processor clock divided by 4 */
#define	CFG_ECBY5	0x30000000 	/*  processor clock divided by 5 */
#define	CFG_ECBY6	0x40000000 	/*  processor clock divided by 6 */
#define	CFG_ECBY7	0x50000000 	/*  processor clock divided by 7 */
#define	CFG_ECBY8	0x60000000 	/*  processor clock divided by 8 */
#define	CFG_EC_RESERVED	0x70000000 	/*  (reserved) */
#define	CFG_EPMASK	0x0f000000	/* Writeback pattern: */
#define	CFG_EPD		0x00000000	/*  DDDD (one dword every cycle) */
#define	CFG_EPDDx	0x01000000	/*  DDxDDx (2 dword/3 cyc) */
#define	CFG_EPDDxx	0x02000000	/*  DDxDDx (2 dword/4 cyc) */
#define	CFG_EPDxDx	0x03000000	/*  DxDxDxDx (2 dword/4 cyc) */
#define	CFG_EPDDxxx	0x04000000	/*  DDxxxDDxxx (2 dword/5 cyc) */
#define	CFG_EPDDxxxx	0x05000000	/*  DDxxxxDDxxxx (2 dword/6 cyc) */
#define	CFG_EPDxxDxx	0x06000000	/*  DxxDxxDxxDxx (2 dword/6 cyc) */
#define	CFG_EPDDxxxxx	0x07000000	/*  DDxxxxxDDxxxxx (2 dword/7 cyc) */
#define	CFG_EPDDxxxxxx	0x08000000	/*  DDxxxxxxDDxxxxxx (2 dword/8 cyc) */
#define	CFG_BEMASK	0x00008000	/* Big Endian */
#define	CFG_EMMASK	0x00004000	/* set to 1 => Parity mode enabled */
#define	CFG_EBMASK	0x00002000	/* set to 1 => Sub-block ordering */
#define	CFG_ICMASK	0x00000e00	/* I-cache size = 2**(12+IC) bytes */
#define	CFG_ICSHIFT	9
#define	CFG_DCMASK	0x000001c0	/* D-cache size = 2**(12+DC) bytes */
#define	CFG_DCSHIFT	6
#define	CFG_IBMASK	0x00000020	/* set to 1 => 32 byte I-cache line */
#define	CFG_DBMASK	0x00000010	/* set to 1 => 32 byte D-cache line */
#define	CFG_K0C_MASK	0x00000007	/* KSEG0 cacheability attributes: */
#define	CFG_C_WTNOALLOC	0		/*  write thru, no write allocate */
#define	CFG_C_WTALLOC	1		/*  write thru, write allocate */
#define	CFG_C_UNCACHED	2		/*  uncached */
#define	CFG_C_WRITEBACK	3		/*  writeback, non-coherent */
#ifdef __RM7000__
#define	CFG_SCMASK	0x80000000	/* Secondary Cache */
#define	SCACHE_SIZE	(256 * 1024)
#else
#define	SCACHE_SIZE	(256 * 1024)
#define	CFG_SCMASK	0x00020000	/* Secondary Cache */
#define	CFG_SC		0x00020000
#define	CFG_SE		0x00001000
#define	CFG_SSMASK	0x00300000
#define	CFG_SSSHIFT	20
#define CFG_SS_NONE	0x00300000
#define CFG_SS_512KB	0x00000000
#define CFG_SS_1MB	0x00100000
#define CFG_SS_2MB	0x00200000
#endif
#define	CFG_7K_SC	0x80000000	/* For RM 7000 */
#define	CFG_7K_SE	0x00000008
/* Status register fields */
#define	SR_CUMASK	0xf0000000	/* Coprocessor usable bits */
#define	SR_CU3		0x80000000	/* Coprocessor 3 usable */
#define	SR_CU2		0x40000000	/* coprocessor 2 usable */
#define	SR_CU1		0x20000000	/* Coprocessor 1 usable */
#define	SR_CU0		0x10000000	/* Coprocessor 0 usable */

#define	SR_FR		0x04000000	/* Enable 32 floating-point registers */
#define	SR_RE		0x02000000	/* Reverse Endian in user mode */

#define	SR_BEV		0x00400000	/* Bootstrap Exception Vector */
#define	SR_TS		0x00200000	/* TLB shutdown (reserved on R4650) */
#define	SR_SR		0x00100000	/* Soft Reset */

#define	SR_CH		0x00040000	/* Cache Hit */
#define	SR_CE		0x00020000	/* ECC register modifies check bits */
#define	SR_DE		0x00010000	/* Disable cache errors */

#define	SR_IMASK	0x0000ff00	/* Interrupt Mask */
#define	SR_IMASK8	0x00000000	/* Interrupt Mask level=8 */
#define	SR_IMASK7	0x00008000	/* Interrupt Mask level=7 */
#define	SR_IMASK6	0x0000c000	/* Interrupt Mask level=6 */
#define	SR_IMASK5	0x0000e000	/* Interrupt Mask level=5 */
#define	SR_IMASK4	0x0000f000	/* Interrupt Mask level=4 */
#define	SR_IMASK3	0x0000f800	/* Interrupt Mask level=3 */
#define	SR_IMASK2	0x0000fc00	/* Interrupt Mask level=2 */
#define	SR_IMASK1	0x0000fe00	/* Interrupt Mask level=1 */
#define	SR_IMASK0	0x0000ff00	/* Interrupt Mask level=0 */

#define	SR_IBIT8	0x00008000	/*  (Intr5) */
#define	SR_IBIT7	0x00004000	/*  (Intr4) */
#define	SR_IBIT6	0x00002000	/*  (Intr3) */
#define	SR_IBIT5	0x00001000	/*  (Intr2) */
#define	SR_IBIT4	0x00000800	/*  (Intr1) */
#define	SR_IBIT3	0x00000400	/*  (Intr0) */
#define	SR_IBIT2	0x00000200	/*  (Software Interrupt 1) */
#define	SR_IBIT1	0x00000100	/*  (Software Interrupt 0) */

#define	SR_KX		0x00000080	/* xtlb in kernel mode */
#define	SR_SX		0x00000040	/* mips3 & xtlb in supervisor mode */
#define	SR_UX		0x00000020	/* mips3 & xtlb in user mode */

#define	SR_KSU_MASK	0x00000018	/* ksu mode mask */
#define	SR_KSU_USER	0x00000010	/* user mode */
#define	SR_KSU_SUPV	0x00000008	/* supervisor mode */
#define	SR_KSU_KERN	0x00000000	/* kernel mode */

#define	SR_ERL		0x00000004	/* error level */
#define	SR_EXL		0x00000002	/* exception level */
#define	SR_IE		0x00000001 	/* interrupt enable */

/* Cause register fields */
#define	CAUSE_BD	0x80000000	/* Branch Delay */
#define	CAUSE_CEMASK	0x30000000	/* Coprocessor Error */
#define	CAUSE_CESHIFT	28		/* Right justify CE  */
#define	CAUSE_IPMASK	0x0000ff00	/* Interrupt Pending */
#define	CAUSE_IPSHIFT	8		/* Right justify IP  */
#define	CAUSE_IP8	0x00008000	/*  (Intr5) */
#define	CAUSE_IP7	0x00004000	/*  (Intr4) */
#define	CAUSE_IP6	0x00002000	/*  (Intr3) */
#define	CAUSE_IP5	0x00001000	/*  (Intr2) */
#define	CAUSE_IP4	0x00000800	/*  (Intr1) */
#define	CAUSE_IP3	0x00000400	/*  (Intr0) */
#define	CAUSE_SW2	0x00000200	/*  (Software Interrupt 1) */
#define	CAUSE_SW1	0x00000100	/*  (Software Interrupt 0) */
#define	CAUSE_EXCMASK	0x0000007c	/* Exception Code */
#define	CAUSE_EXCSHIFT	2		/* Right justify EXC */

/* Exception Codes */
#define	EXC_INT		0		/* External interrupt */
#define	EXC_MOD		1		/* TLB modification exception */
#define	EXC_TLBL	2		/* TLB miss (Load or Ifetch) */
#define	EXC_TLBS	3		/* TLB miss (Store) */
#define	EXC_ADEL	4		/* Address error (Load or Ifetch) */
#define	EXC_ADES	5		/* Address error (Store) */
#define	EXC_IBE		6		/* Bus error (Ifetch) */
#define	EXC_DBE		7		/* Bus error (data load or store) */
#define	EXC_SYS		8		/* System call */
#define	EXC_BP		9		/* Break point */
#define	EXC_RI		10		/* Reserved instruction */
#define	EXC_CPU		11		/* Coprocessor unusable */
#define	EXC_OVF		12		/* Arithmetic overflow */
#define	EXC_TRAP	13		/* Trap exception */
#define	EXC_FPE		15		/* Floating Point Exception */

/*
 * EntryHi register fields
 */
#define	EH_REGION_MASK	0xc000000000000000	/* 11=krnl, 01=supv, 00=user */
#define	EH_REGION_SHIFT	62
#define	EH_FILL_MASK	0x3fffff0000000000	/* (holds replica of bit 63) */
#define	EH_FILL_SHIFT	40
#define	EH_VPN2_MASK	0x000000ffffffe000	/* Virtual pageno div 2 */
#define	EH_VPN2_SHIFT	13
#define	EH_ASID_MASK	0x00000000000000ff	/* Address space ID */
#define	EH_ASID_SHIFT	0

/*
 * EntryLo register fields
 */
#define	EL_PFN_MASK	0x000000003fffffc0	/* Page Frame Number */
#define	EL_PFN_SHIFT	6
#define	EL_C_MASK	0x0000000000000038	/* Cacheability attributes */
#define	EL_C_SHIFT	3
#define	EL_D_MASK	0x0000000000000004	/* Dirty bit */
#define	EL_D_SHIFT	2
#define	EL_V_MASK	0x0000000000000002	/* Valid bit */
#define	EL_V_SHIFT	1
#define	EL_G_MASK	0x0000000000000001	/* Global bit */
#define	EL_G_SHIFT	0
#define EL_C_C_NC_WT_NWA	0		/* Cacheable, noncoherent
						 * write-through,
						 * no write allocate
						 */
#define EL_C_C_NC_WT_WA		1
#define EL_C_UC			2		/* Uncached */
#define EL_C_C_NC_WB		3		/* Write-back */

/*
 * PageMask register fields
 */
#define	PM_MASK		0x01ffe000	/* Page size mask: */
#define	PM_M_4K		0x00000000	/*  4K bytes */
#define	PM_M_16K	0x00006000	/*  16K bytes */
#define	PM_M_64K	0x0001e000	/*  64K bytes */
#define	PM_M_256K	0x0007e000	/*  256K bytes */
#define	PM_M_1M		0x001fe000	/*  1M bytes */
#define	PM_M_4M		0x007fe000	/*  4M bytes */
#define	PM_M_16M	0x01ffe000	/*  16M bytes */

/*
 * Index register fields
 */
#define	IR_P_MASK	0x80000000	/* TLB Probe (TLBP) failure */
#define	IR_INDEX_MASK	0x0000003f	/* Index of TLB entry for TLBR/TLBWI */

/*
 * Random register
 */
#define	RR_INDEX_MASK	0x0000003f	/* Index of TLB entry for TLBWR */
#define	NTLBENTRIES	48		/* Max TLB index is one less */

/*
 * Wired register
 */
#define	WR_INDEX_MASK	0x0000003f	/* Number of wired TLB entries */

/*
 * PrID register fields
 */
#define	PRID_IMP_MASK	(0xff<<8)	/* Implementation number */
#define	PRID_REV_MASK	(0xff<<0)	/* Revision number */

/*
 * Primary Cache TagLo
 */
#define	TAG_PTAG_MASK		0xffffff00      /* P-Cache Tag (Addr 35:12) */
#define	TAG_PTAG_SHIFT          8
#define	TAG_PSTATE_MASK         0x000000c0      /* P-Cache State */
#define	TAG_PSTATE_SHIFT        6
#define	TAG_FIFO_BIT_MASK	0x00000002      /* P-Cache FIFO bit */
#define	TAG_FIFO_BIT_SHIFT	1
#define	TAG_PARITY_MASK		0x00000001      /* P-Cache Tag Parity */
#define	TAG_PARITY_SHIFT        0

#define	PSTATE_INVAL		0		/* Invalid */
#define	PSTATE_SHARED		1		/* Should not occur */
#define	PSTATE_CLEAN_EXCL	2		/* Should not occur */
#define	PSTATE_DIRTY_EXCL	3		/* Dirty exclusive */

/*
 *  RM 7000 Scache Tag states
 */
#define	TAG_7K_SSTATE_MASK         0x000001c0
#define	SSTATE_INVALID		   0x00000000
#define	SSTATE_CLEAN_EXCL	   0x00000100
#define	SSTATE_DIRTY_EXCL	   0x00000140
#define	SSTATE_PENDING		   0x00000180
#define	SSTATE_DIRTY_EXCLD	   0x000001C0
/*
 * CacheErr register
 */
#define	CACHEERR_TYPE		0x80000000	/* reference type */
#define	CACHEERR_LEVEL		0x40000000	/* cache level */
#define	CACHEERR_DATA		0x20000000	/* data field */
#define	CACHEERR_TAG		0x10000000	/* tag field */
#define	CACHEERR_REQ		0x08000000	/* request type */
#define	CACHEERR_BUS		0x04000000	/* error on bus */
#define	CACHEERR_BOTH		0x02000000	/* Data & Instruction error */
#define	CACHEERR_REFILL		0x01000000	/* Error on Refill */
#define	CACHEERR_SIDX_MASK	0x003ffff8	/* PADDR(21..3) */
#define	CACHEERR_SIDX_SHIFT		 0
#define	CACHEERR_PIDX_MASK	0x00000007	/* VADDR(14..12) */
#define	CACHEERR_PIDX_SHIFT		12


/*
 * Floating Point Coprocessor (CP1) registers
 */
#define	C1_IRR		$0		/* Implementation/Revision register */
#define	C1_CSR		$31		/* FPU Control/Status register */

/*
 * Implementation/Revision reg fields
 */
#define	IRR_IMP_MASK	(0xff<<8)	/* Implementation number */
#define	IRR_REV_MASK	(0xff<<0)	/* Revision number */

/*
 * FPU Control/Status register fields
 */
#define	CSR_FS		0x01000000	/* Set to flush denormals to zero */
#define	CSR_C		0x00800000	/* Condition bit (set by FP compare) */

#define	CSR_CMASK	(0x3f<<12)
#define	CSR_CE		0x00020000
#define	CSR_CV		0x00010000
#define	CSR_CZ		0x00008000
#define	CSR_CO		0x00004000
#define	CSR_CU		0x00002000
#define	CSR_CI		0x00001000

#define	CSR_EMASK	(0x1f<<7)
#define	CSR_EV		0x00000800
#define	CSR_EZ		0x00000400
#define	CSR_EO		0x00000200
#define	CSR_EU		0x00000100
#define	CSR_EI		0x00000080

#define	CSR_FMASK	(0x1f<<2)
#define	CSR_FV		0x00000040
#define	CSR_FZ		0x00000020
#define	CSR_FO		0x00000010
#define	CSR_FU		0x00000008
#define	CSR_FI		0x00000004

#define	CSR_RMODE_MASK	(0x3<<0)
#define	CSR_RM		0x00000003
#define	CSR_RP		0x00000002
#define	CSR_RZ		0x00000001
#define	CSR_RN		0x00000000

/*
 * Cache operations
 */
#define	INDEX_INVALIDATE_I      0x00
#define	INDEX_WRITEBACK_INV_D   0x01
#define	INDEX_WRITEBACK_INV_S   0x03
#define	INDEX_LOAD_TAG_I        0x4
#define	INDEX_LOAD_TAG_D        0x5
#define	INDEX_LOAD_TAG_S        0x7
#define	INDEX_STORE_TAG_I       0x8
#define	INDEX_STORE_TAG_D       0x9
#define	INDEX_STORE_TAG_S       0xB
#define	CREATE_DIRTY_EXC_D      0xD
#define	HIT_INVALIDATE_I        0x10
#define	HIT_INVALIDATE_D        0x11
#define	FILL_I                  0x14
#define	HIT_WRITEBACK_INV_D     0x15
#define	HIT_INVALIDATE_S	0x13
#define	HIT_WRITEBACK_INV_S     0x17
#define	HIT_WRITEBACK_I         0x18
#define	HIT_WRITEBACK_D         0x19
#define	INDEX_INVALIDATE_S_PAGE	0x17

/*
 * ECC bit for Selecting I, D and S caches
 */
#define ECC_LOCK_I		0x08000000
#define ECC_LOCK_D		0x04000000
#define ECC_LOCK_S		0x02000000

/*
 * linesize is fixed at 32 bytes for all caches
 */
#define LINESIZE	32
	
/*
 * TLB Stuff
 */
#define	N_TLB_ENTRIES	48

#define	TLBHI_VPN2MASK	0xffffe000
#define	TLBHI_PIDMASK	0x000000ff
#define	TLBHI_NPID	256

#define	TLBLO_PFNMASK	0x3fffffc0
#define TLBLO_PFNSHIFT	6
#define	TLBLO_D		0x00000004	/* writeable */
#define	TLBLO_V		0x00000002	/* valid bit */
#define	TLBLO_G		0x00000001	/* global access bit */
#define TLBLO_CMASK	0x00000038	/* cache algorithm mask */
#define TLBLO_CSHIFT	3

#define TLBLO_UNCACHED		(CFG_C_UNCACHED<<TLBLO_CSHIFT)
#define TLBLO_NONCOHERENT	(CFG_C_NONCOHERENT<<TLBLO_CSHIFT)
#define TLBLO_COHERENTXCL	(CFG_C_COHERENTXCL<<TLBLO_CSHIFT)
#define TLBLO_COHERENTXCLW	(CFG_C_COHERENTXCLW<<TLBLO_CSHIFT)
#define TLBLO_COHERENTUPD	(CFG_C_COHERENTUPD<<TLBLO_CSHIFT)

#define	TLBINX_PROBE	0x80000000
#define	TLBINX_INXMASK	0x0000003f

#define	TLBRAND_RANDMASK	0x0000003f

#define	TLBCTXT_BASEMASK	0xff800000
#define	TLBCTXT_BASESHIFT	23

#define	TLBCTXT_VPN2MASK	0x007ffff0
#define	TLBCTXT_VPN2SHIFT	4

#define TLBPGMASK_MASK		0x01ffe000


#define	K0BASE 		0x80000000


#define	K0SIZE 		0x20000000
#define	K1BASE 		0xa0000000
#define	K1SIZE 		0x20000000
#define K2BASE          0xc0000000

#define	MIPS_RM52X0	0x28
#define	MIPS_RM7000	0x27

/*
 * Set 1 registers
 */
#define C1_ICR			$20
#define C1_IPLLO		$18
#define C1_IPLHI		$19

#define ICR_INT_MASK	0x0000FF00
#endif MIPS_REG_H
