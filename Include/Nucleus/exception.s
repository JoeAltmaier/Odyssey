/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: exception.s
 * 
 * Description:
 * This file contains exception handler for TLB 
 * 
 * Update Log:
 * 03/23/99 Raghava Kondapalli: Created 
 ************************************************************************/



#define __ASM__
#define __DBG__
#include "mips_reg.h"
#define PCI_VADDR_START	0x00000000
#define	PCI_VADDR_END	0x80000000
#define PCI_PADDR_START	0x80000000
#define PAGESIZE	0x01000000

/*
 * TLB Exception Handler: Assumes that exceptions occur only for
 * PCI space. Dram space is hardwired so as not cause an exception.
 * Simple check is done to determine if address is between 
 * 0x00000000 and 0x80000000
 */
	.text
	.globl	tlb_exception_handler
tlb_exception_handler:
	.option	reorder off
        .option no_at_macros on

	mfc0	k0, C0_BADVADDR
	li	k1, PCI_VADDR_END
	subu	k1, k1, k0		/* Check if BadVaddr < 0x80000000 */
	bltz	k1, bad_addr
	nop
	li	k1, 0xFE000000		/* Get offset to physical space */
	and	k1, k0, k1
	mtc0	k1, C0_ENTRYHI
	li	k0, PCI_PADDR_START	
	add	k1, k0, k1
	srl	k1, k1, 6
	li	k0, TLBLO_PFNMASK	/* (addr >> 6) PFN_MASK */
	and	k1, k1, k0
	ori	k1, k1, 0x17		/* Hardcode cache attributes */
	mtc0	k1, C0_ENTRYLO0	
	li	k0, 0x40000		/* just add to get entrylo1 */
	add	k1, k0, k1
	mtc0	k1, C0_ENTRYLO1

	tlbwr

	mfc0	k1, C0_COMPARE
	sub	k1, k1, 100
	mtc0	k1,C0_COUNT

	eret	
	nop

	/*
	 * Not in our page table. Pass the exception to debugger
	 */
bad_addr:	
	la	k0, 0xA0000080
	j	k0
	nop
        .option no_at_macros off

