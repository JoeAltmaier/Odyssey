/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: mips_util.c
 * 
 * Description:
 * This file contains mips support functions for TLB/Cache 
 * 
 * Update Log:
 * 03/23/99 Raghava Kondapalli: Created 
 * 10/19/99 Jeff Nespor: Added support for gp relative addressing (sdata).
 ************************************************************************/

#include <mips_util.h>

#define	KSEG0_BASE	0x80000000
/*
 * TLB Page Table Entry(PTE)
 */
#define PAGESIZE		MEG(16)
#define PAGEMASK		PM_M_16M	/* 16MB */
#define ROUNDUP(a, n)	((((U32)a) + ((n) - 1)) & ~((n) - 1))
#define MEG(a)	(1024 * 1024 * (a))

STATUS (*_sync_for_cpu)(VOID *, U32);
STATUS (*_sync_for_dev)(VOID *, U32);
static STATUS wtna_sync_for_cpu(VOID *, U32);
static STATUS wta_sync_for_cpu(VOID *, U32);
static STATUS uc_sync_for_cpu(VOID *, U32);
static STATUS wb_sync_for_cpu(VOID *, U32);
static STATUS wtna_sync_for_dev(VOID *, U32);
static STATUS wta_sync_for_dev(VOID *, U32);
static STATUS uc_sync_for_dev(VOID *, U32);
static STATUS wb_sync_for_dev(VOID *, U32);

/*
 * Moved definition of cache globals from cache.s due to
 * an apparent sdata metrolimitation.
 */
U32	icache_size;
U32	icache_lsize;
U32	icache_set_size;
U32	dcache_size;
U32	dcache_lsize;
U32	dcache_set_size;
U32	scache_size;
U32	scache_lsize;
U32	cpu_type;
U32	cache_nway;

/*
 * Initialize cache. Should be the first function to be called.
 */
#ifdef CONFIG_BOOT
STATUS
mips_init_cache(U32 type)
{
#else
STATUS
mips_init_cache(void)
{
	U32	type;
#endif

#ifndef CONFIG_BOOT	
	U32		cfg;

	cfg = r5k_get_config();
	type = cfg & CFG_K0C_MASK;
#endif
	switch(type) {
		case CACHE_WRITE_THRU_NOALLOC:
			printf("Setting Cache in \"Write Thru' Mode Without Write Allocate\" Mode\n\r");
			_sync_for_cpu = wtna_sync_for_cpu;
			_sync_for_dev = wtna_sync_for_dev;
			break;
		case CACHE_WRITE_THRU_ALLOC:
			printf("Setting cache in \"Write Thru' Mode With Write Allocate\" Mode\n\r");
			_sync_for_cpu = wta_sync_for_cpu;
			_sync_for_dev = wta_sync_for_dev;
			break;
		case CACHE_UNCACHED:
			printf("Setting cache in \"Uncached\" Mode\n\r"); 
			_sync_for_cpu = uc_sync_for_cpu;
			_sync_for_dev = uc_sync_for_dev;
			break;
		case CACHE_WRITEBACK:
			printf("Setting cache in \"Writeback\" Mode\n\r"); 
			_sync_for_cpu = wb_sync_for_cpu;
			_sync_for_dev = wb_sync_for_dev;
			break;
		default:
			printf("Illegal Cache Mode %d. Setting to Write thru' mode\n\r", type);
			_sync_for_cpu = wtna_sync_for_cpu;
			_sync_for_dev = wtna_sync_for_dev;
	}

#ifdef CONFIG_BOOT
	config_cache(type);
#endif

	init_cache();
	return (0);
}

/*
 * Sync Cache. Function pointers are set in mips-init_cache.
 */
STATUS
mips_sync_cache(VOID *saddr, U32 len, U32 type)
{
	if (type == SYNC_CACHE_FOR_DEV) {
		if (_sync_for_dev) {
			return (_sync_for_dev(saddr, len));
		}
	} else  {
		if (type == SYNC_CACHE_FOR_CPU)
			if (_sync_for_cpu)
	       			return (_sync_for_cpu(saddr, len));
	}
	return (-1);
}

/*
 * Flush Entire I, D & S caches. Invalidates all the entries.
 */
STATUS
mips_flush_cache(VOID)
{
	flush_cache();
	return (0);
}

/*
 * Flush Entire D cache. Invalidates all the entries.
 */
STATUS
mips_flush_dcache(VOID)
{
	int	lines;

	if (cpu_type == MIPS_RM7000) {
		__flush_dcache_7x(KSEG0_BASE, dcache_size/4);
	} else {
		__flush_dcache_5x(KSEG0_BASE, dcache_size/2);
	}
	if (scache_size) {
		lines = scache_size / scache_lsize; 
		__flush_hit_wb_scache(KSEG0_BASE, lines);
	}
	return (0);
}

/*
 * Flush Entire I cache. Invalidates all the entries.
 */
STATUS
mips_flush_icache(VOID)
{
	int	lines;

	if (cpu_type == MIPS_RM7000)
		__flush_icache_7x(KSEG0_BASE, icache_size);
	else
		__flush_icache_5x(KSEG0_BASE, icache_size);
	if (scache_size) {
		lines = scache_size / scache_lsize; 
		__flush_hit_wb_scache(KSEG0_BASE, lines);
	}
	return (0);
}

STATUS
mips_lock_dcache(VOID *addr, U32 len)
{
	static int set = 0xFF;
	
	addr = (VOID *)((U32)addr & 0xFFFFFFFC);
	len = ROUNDUP(len, 32);
	if (set == 0xFF) {
		set = 0;
		lock_dcache((U32)addr, len, CACHE_SET_A);
		return (0);
	} else {
		if (set == 0) {
			set = 1;
			lock_dcache((U32)addr, len, CACHE_SET_B);
			return (0);
		}
	}
	return (-1);
}

STATUS
mips_lock_icache(VOID *addr, U32 len)
{
	static int set = 0xFF;
	
	addr = (VOID *)((U32)addr & 0xFFFFFFFC);
	len = ROUNDUP(len, 32);
	if (set == 0xFF) {
		set = 0;
		lock_icache((U32)addr, len, CACHE_SET_A);
		return (0);
	} else {
		if (set == 0) {
			set = 1;
			lock_icache((U32)addr, len, CACHE_SET_B);
			return (0);
		}
	}
	return (-1);
}

/*
 * tlb_clear_entry(index) Invalidate the  TLB entry specified by index
 */
asm VOID
tlb_clear_entry(I32 index)
{

	
	li	t2,K1BASE&TLBHI_VPN2MASK
	mfc0	t0,CR_TLBHI		/* save current TLBHI */
	mfc0	v0,CR_SR		/* save SR and disable interrupts */
	mtc0	zero,CR_SR
	mtc0	t2,CR_TLBHI		/* invalidate entry */
	mtc0	zero,CR_TLBLO0
	mtc0	zero,CR_TLBLO1
	mtc0	a0,CR_INDEX
	li	t2, PAGEMASK
	mtc0	t2, CR_PAGEMASK
	.set noreorder
	nop
	nop
	nop
	nop
	nop
	.set reorder
	tlbwi
	.set noreorder
	nop
	nop
	nop
	nop
	nop
	.set reorder
	mtc0	t0,CR_TLBHI
	mtc0	v0,CR_SR
	jr	ra
	nop
}

/*
 * Set WIRED Register
 */
asm VOID
tlb_set_wired(U32 index)
{
	mtc0	a0, CR_WIRED
	.set	noreorder
	nop
	nop
	nop
	nop
	nop
	.set	reorder
	jr	ra
	nop
}

/*
 * tlb_getlo0 -- returns the 'entrylo' contents for the TLB
 */
asm U32
tlb_get_lo0(I32)
{
	mtc0	a0,CR_INDEX		/* write to index register */
	.set noreorder
	nop
	nop
	nop
	nop
	nop
	.set reorder
	.set noreorder
	tlbr				/* put tlb entry in entrylo and hi */
	nop
	nop
	nop
	nop
	nop
	mfc0	v0,CR_TLBLO0		/* get the requested entry lo */
	nop
	nop
	nop
	nop
	nop
	.set reorder
	jr	ra
	nop
}

asm U32
tlb_get_lo1(I32)
{
	mtc0	a0,CR_INDEX		/* write to index register */
	.set noreorder
	nop
	nop
	nop
	nop
	nop
	.set reorder
	tlbr				/* put tlb entry in entrylo and hi */
	.set noreorder
	nop
	nop
	nop
	nop
	nop
	mfc0	v0,CR_TLBLO1		/* get the requested entry lo */
	nop
	nop
	nop
	nop
	nop
	.set reorder
	jr	ra
	nop
}

/*
** tlb_get_pagemask() -- return pagemask contents of tlb entry
*/
asm U32
tlb_get_pagemask(I32)
{
	mfc0	v0,CR_PAGEMASK		/* to return value */
	.set	noreorder
	nop
	nop
	nop
	nop
	nop
	.set	reorder
	jr	ra
	nop
}

/*
 * tlb_get_wired(void) -- return wired register
 */
asm VOID
tlb_get_wired(void)
{
	mfc0	v0,CR_WIRED
	.set	noreorder
	nop
	nop
	nop
	nop
	nop
	.set	reorder
	jr	ra
	nop
}

/*
 * tlb_get_hi -- return the tlb entry high content for tlb entry
 * index
 */
asm U32
tlb_get_hi(I32 index)
{
	mtc0	a0,CR_INDEX		/* drop it in C0 register */
	.set noreorder
	nop
	nop
	nop
	nop
	nop
	.set reorder
	tlbr				/* read entry entry hi/lo0/lo1/mask */
	.set noreorder
	nop
	nop
	nop
	nop
	nop
	mfc0	v0,CR_TLBHI		/* to return value */
	nop
	nop
	nop
	nop
	nop
	.set reorder
	jr	ra
	nop
}

/*
 * tlb_get_pid() -- return tlb pid contained in the current entry hi
 */
asm VOID
tlb_get_pid(void)
{
	mfc0	v0,CR_TLBHI		/* to return value */
	.set	noreorder
	nop
	nop
	nop
	nop
	nop
	and	v0,v0, TLBHI_PIDMASK
	.set	noreorder
	jr	ra
	nop
}

VOID
tlb_flush(void)
{
	int		i;

	for (i = 1; i < NTLBENTRIES; i++)
		tlb_clear_entry(i);
}

/*
 * Set current TLBPID.
 *
 */
asm VOID tlb_set_pid(VOID)
{
	mtc0	a0,CR_TLBHI
	.set	noreorder
	nop
	nop
	nop
	nop
	nop
	.set	reorder
	jr	ra
	nop
}


/*
 * Sync DRAM with Cache. Used when CPU is configured for write-thru' with
 * no allocate.
 * In this case we do nothing, as data is written to DRAM on store. 
 */
static STATUS
wtna_sync_for_dev(VOID *addr, U32 len)
{
	/*
	 * To avoid compiler warning
	 */
	addr = addr;
	len = len;
	return (0);
}

/*
 * Sync Cache with DRAM function. Used when CPU is configured for
 * write-thru' with no allocate.
 * In this case we do nothing. 
 */
static STATUS
wtna_sync_for_cpu(VOID *addr, U32 len)
{
	int	lines;

	addr = (VOID *)((U32)addr & (~(dcache_lsize - 1)));	
	lines = ROUNDUP(len, dcache_lsize) / dcache_lsize;
	/*
	 * The cache contents might be stale. So Invalidate cache entries
	 * in dcache and Scache. So the next read will fetch the contents
	 * from main memory. Note Scache is accessed only on line fill
	 */
	__flush_hit_nwb_dcache((U32)addr, lines);

	return (0);
}

/*
 * Sync DRAM with Cache. Used when CPU is configured for write-thru' with
 * write allocate.
 */
static STATUS
wta_sync_for_dev(VOID *addr, U32 len)
{
	addr = addr;
	len = 1;
	return (0);
}

/*
 * Sync Cache with DRAM function. Used when CPU is configured for
 * write-thru' with write allocate.
 * In this case we do nothing. 
 */
static STATUS
wta_sync_for_cpu(VOID *addr, U32 len)
{
	int	lines;

	addr = (VOID *)((U32)addr & (~(dcache_lsize - 1)));	
	lines = ROUNDUP(len, dcache_lsize) / dcache_lsize;
	/*
	 * The cache contents might be stale. So Invalidate cache entries
	 * in dcache and Scache. So the next read will fetch the contents
	 * from memory.
	 */
	__flush_hit_nwb_dcache((U32)addr, lines);
	return (0);
}

/*
 * Sync DRAM with Cache. Used when Cache is configured as uncached
 * In this case we do nothing. 
 */
static STATUS
uc_sync_for_dev(VOID *addr, U32 len)
{
	/*
	 * To avoid compiler warning
	 */
	addr = addr;
	len = len;
	return (0);
}

/*
 * Sync Cache with DRAM function. Used when Cache is configured uncached
 * In this case we do nothing. 
 */
static STATUS
uc_sync_for_cpu(VOID *addr, U32 len)
{
	/*
	 * To avoid compiler warning
	 */
	addr = addr;
	len = len;
	return (0);
}

static STATUS
wb_sync_for_dev(VOID *addr, U32 len)
{
	int	lines;
	int	npages;
	int	page;

	addr = (VOID *)((U32)addr & (~(dcache_lsize - 1)));	
	lines = ROUNDUP(len, dcache_lsize) / dcache_lsize;
	if (cpu_type == MIPS_RM7000) {
		__flush_hit_wb_dcache((U32)addr, lines);
		if (scache_size) {
			__flush_hit_wb_scache((U32)addr, lines);
		}
	} else {
		/*
		 * RM 5xxx operations are different fot Scache.
		 * Flushing of RM5xxx Scache is done i terms of a page.
		 */
		__flush_hit_wb_dcache((U32)addr, lines);
		if (scache_size) {
			page = (U32)addr - ((U32)addr & 4095);
			npages = ROUNDUP(len, 4096) / 4096;
			__flush_nwb_scache_5x((U32)page, npages);
		}
	}
	return (0);
}
	
static STATUS
wb_sync_for_cpu(VOID *addr, U32 len)
{
	int	lines;

	addr = (VOID *)((U32)addr & (~(dcache_lsize - 1)));	
	lines = ROUNDUP(len, dcache_lsize) / dcache_lsize;
	__flush_hit_nwb_dcache((U32)addr, lines);
	__flush_hit_nwb_scache((U32)addr, lines);

	return (0);
}

