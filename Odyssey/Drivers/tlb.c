/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: tlb.c
 * 
 * Description:
 * This file contains TLB routines for Odyssey 
 * 
 * Update Log:
 * 12/17/98 Raghava Kondapalli: Created 
 * 03/24/99: Jerry Lane:		Included Mips_util.h.
 ************************************************************************/

#include "types.h"
#include "mips_reg.h"
#include "pcimap.h"
#include "galileo.h"
#include "Mips_util.h"


#define MEG(a)	(1024 * 1024 * (a))
/*
 * TLB Page Table Entry(PTE)
 */
#define MAX_PTE_ENTRIES		128
#define PAGESIZE		MEG(16)
#define PAGEMASK		PM_M_16M	/* 16MB */
#define REGION1_VADDR_START	(0xC0000000)
#define REGION1_VADDR_END	(0xD0000000)
#define REGION1_PADDR_START	(0x40000000)
#define REGION2_VADDR_START	(0x00000000)
#define REGION2_VADDR_END	(0x80000000)
#define REGION2_PADDR_START	(0x80000000)

#define REGION1_LO0		(REGION1_PADDR_START)
#define REGION1_LO1		(REGION1_PADDR_START + PAGESIZE)
#define PAGE_ATTRIBUTE		((EL_C_UC << EL_C_SHIFT) | 7)
#define LO_ENTRY(addr)	\
	(((addr >> 12 - TLBLO_PFNSHIFT) & TLBLO_PFNMASK) | PAGE_ATTRIBUTE)
#define REGION1_ATTRIBUTE	((EL_C_UC << EL_C_SHIFT) | 7)


U32		tlb_pt_entries = 0;
U32		tlb_index = 0;

extern void printf(const char *fmt, ...);
void	tlb_init(void);
void 	tlb_set_wired(U32);
void 	tlb_set_pid(void);
int		tlb_map_region(U32, U32, U32);
int		tlb_handle_exception(U32);
int 	tlb_mapi(U32 index, U32 vaddr, U32 entrylo0, U32 entrylo1);
int 	tlb_mapr(U32 vaddr, U32 entrylo0, U32 entrylo1);

void
tlb_init(void)
{
	/*
	 * Setup KSSEG Space to map to physical memory
	 */
	tlb_flush();
	tlb_pt_entries = 1;
	tlb_index = 0;

	/*
	 * Map Bank2:3
	 */
	tlb_map_region(REGION1_VADDR_START, REGION1_PADDR_START,
			REGION1_VADDR_END - REGION1_VADDR_START);

	tlb_set_wired(tlb_pt_entries - 1);
#ifndef CONFIG_BOOT
	/*
	 * Map PCI space
	 */
	tlb_map_region(REGION2_VADDR_START, REGION2_PADDR_START,
			REGION2_VADDR_END - REGION2_VADDR_START);
#endif
}

int
tlb_map_region(U32 vaddr, U32 paddr, U32 size)
{
	int		i;
	int		p_num;
	U32		page_size;

	p_num = (size)/ (2 * PAGESIZE);
	page_size = PAGESIZE;
	i = tlb_pt_entries;
	for (; i < (tlb_pt_entries + p_num); i++) {
		if (i < NTLBENTRIES) {
			tlb_mapi(i, vaddr,
					LO_ENTRY(paddr),
					LO_ENTRY(paddr + page_size));
			tlb_index++;
		}
		vaddr += (2 * page_size);
		paddr += (2 * page_size);
	}
	tlb_pt_entries = i;
	return (tlb_pt_entries);
}

int
tlb_map_page(U32 vaddr, U32 paddr)
{
		
	tlb_flush();
	tlb_mapi(1, vaddr, LO_ENTRY(paddr), LO_ENTRY(paddr + PAGESIZE));
	tlb_mapi(2, vaddr + MEG(32), LO_ENTRY(paddr + MEG(32)),
					LO_ENTRY(paddr + MEG(32) + PAGESIZE));
	return (0);
}
	

/*
 * Map Vaddr using INDEX Register. Uses tlbwi
 */
asm int 
tlb_mapi(U32 index, U32 vaddr, U32 entrylo0, U32 entrylo1)
{
	.set noreorder

	mfc0	t1, CR_SR
	mfc0	t2, CR_TLBHI

	li	t0, PAGEMASK
	mtc0	t0, CR_PAGEMASK	
	mtc0	a1, CR_TLBHI		/* Set TLB HI */
	nop
	mtc0	a2, CR_TLBLO0		/* Set TLB Lo0 */
	nop
	mtc0	a3, CR_TLBLO1		/* Set TLB Lo1 */
	nop
	mtc0	a0, CR_INDEX
	nop
	mtc0	zero, CR_RANDOM
	nop
	nop
	nop
	nop
	nop
	nop
	tlbwi
	nop
	nop
	nop
	nop
	nop
	nop
	jr	ra
	li	v0, 1
}

/*
 * Map Vaddr using RANDOM Register. Uses tlbwr
 */
asm int 
tlb_mapr(U32 vaddr, U32 entrylo0, U32 entrylo1)
{
	.set noreorder

	mfc0	t1, CR_SR
	mfc0	t2, CR_TLBHI

	li	t0, PAGEMASK
	mtc0	t0, CR_PAGEMASK	
	mtc0	a0, CR_TLBHI		/* Set TLB HI */
	nop
	mtc0	a1, CR_TLBLO0		/* Set TLB Lo0 */
	nop
	mtc0	a2, CR_TLBLO1		/* Set TLB Lo1 */
	nop
	nop
	nop
	nop
	nop
	nop
	tlbwr
	nop
	nop
	nop
	nop
	nop
	nop
	jr	ra
	li	v0, 1
}
