/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: cache.s
 * 
 * Description:
 * This file contains mips support functions for Cache 
 * 
 * Update Log:
 * 03/23/99 Raghava Kondapalli: Created 
 * 10/19/99 Jeff Nespor: Added support for gp relative addressing (sdata).
 ************************************************************************/

#include <mips_reg.h>

#define	KSEG0_BASE	0x80000000

/*
 * Moved definition of cache globals to mips_util.c due to
 * an apparent sdata metrolimitation.  All of these externs
 * will be in the small data section if sdata >= 4.
 *
 * The boot code is built with sdata = 0.
 */
#ifdef CONFIG_BOOT
	.extern icache_size
	.extern icache_lsize
	.extern icache_set_size
	.extern dcache_size
	.extern dcache_lsize
	.extern dcache_set_size
	.extern scache_size
	.extern scache_lsize
	.extern cpu_type
	.extern cache_nway
#else
	.extern sdata:icache_size
	.extern sdata:icache_lsize
	.extern sdata:icache_set_size
	.extern sdata:dcache_size
	.extern sdata:dcache_lsize
	.extern sdata:dcache_set_size
	.extern sdata:scache_size
	.extern sdata:scache_lsize
	.extern sdata:cpu_type
	.extern sdata:cache_nway
#endif


	.text

/*
 * Configure Cache. A0 contains one of the following
 *	CFG_C_WTNOALLOC
 *	CFG_C_WTALLOC
 *	CFG_C_UNCACHED
 *	CFG_C_WRITEBACK
 * WARNING: No Error checking is done
 */
	.global	config_cache
config_cache:
	.option	reorder off
	mfc0	v0, C0_CONFIG
	nop
	nop
	and		v0, v0, ~CFG_K0C_MASK
	or		v0, v0, a0
	mtc0	v0, C0_CONFIG
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	jr	ra
	nop
	.option	reorder on
/*
 * Return the I Tag for a address in a0
 */
	.global	load_itag
load_itag:
	cache	INDEX_LOAD_TAG_I, 0(a0)
	nop
	nop
	mfc0	v0, C0_TAGLO
	nop
	nop
	nop
	nop
	nop
	jr	ra
	nop

/*
 * Return the D Tag for a address in a0
 */
	.global	load_dtag
load_dtag:
	cache	INDEX_LOAD_TAG_D, 0(a0)
	nop
	nop
	mfc0	v0, C0_TAGLO
	nop
	nop
	nop
	nop
	nop
	jr	ra
	nop

/*
 * lock_icache(addr, len, set);
 * Lock a region is icache. 
 * addr is 32 byte aligned
 */
	.global	lock_icache
lock_icache:
	.option	reorder off

	mfc0	v0, C0_STATUS		# Save status register
	li	t0,~SR_IE		# Disable interrupts
	and	t0, v0
	li	t1, SR_DE
	or	t0, t0, t1		# Disable parity exceptions
	mtc0	t0, C0_STATUS          
	nop
	nop

	mfc0	v1, C0_ECC			# Save ECC
	nop
	nop
	li	t0, ECC_LOCK_I
	or	t0, t0, a2			# Set up lock bit in ECC
	mtc0	t0, C0_ECC
	nop
	nop

	move	t0, a0	
	move	t1, a1
lic_l:
	cache	FILL_I, 0(t0)			# Load the code
	nop
	nop
	sub	t1, t1, 8
	bnez	t1, lic_l
	add	t0, t0, 8

	mtc0	v1, C0_ECC			# Restore ECC
	nop
	nop
	mtc0	v0, C0_STATUS			# Restore Status
	nop
	nop
	jr	ra
	nop
	.option	reorder off
	
#define addr	t0
#define maxaddr	t1
#define mask	t2

/*
 * lock_dcache(addr, len, set);
 * Lock a region is icache. 
 * addr is 32 byte aligned
 */
	.global	lock_dcache
lock_dcache:
	.option	reorder off

	mfc0	v0, C0_STATUS		# Save status register
	li	t0,~SR_IE		# Disable interrupts
	and	t0, v0
	li	t1, SR_DE
	or	t0, t0, t1		# Disable parity exceptions
	mtc0	t0, C0_STATUS          
	nop
	nop

	mfc0	v1, C0_ECC			# Save ECC
	nop
	nop
	li	t0, ECC_LOCK_D
	or	t0, t0, a2			# Set up lock bit in ECC
	mtc0	t0, C0_ECC
	nop
	nop

	move	t0, a0	
	move	t1, a1
ldc_l:
	ld	t2, 0(t0)			# Load data
	nop
	nop
	sub	t1, t1, 8
	bnez	t1, ldc_l
	add	t0, t0, 8

	mtc0	v1, C0_ECC			# Restore ECC
	nop
	nop
	mtc0	v0, C0_STATUS			# Restore Status
	nop
	nop
	jr	ra
	nop
	.option	reorder off

	.global	flush_cache
flush_cache:
	# Invalidate instruction cache
	lw	t2, icache_size
	li	t8, K0BASE 
	add	t9, t8, t2		# End address 
	subu	t9, 32
fc_l1:
	cache	INDEX_INVALIDATE_I,0(t8)
	bne	t8, t9, fc_l1
	addu	t8, t8, 32 

	# Invalidate data cache
	lw	t3, dcache_size
	li	t8, K0BASE 
	add	t9, t8, t3		# End address 
	subu	t9, 32
fc_l2:
	cache	INDEX_WRITEBACK_INV_D,0(t8)
	bne	t8, t9, fc_l2
	addu	t8, t8, 32

	lw	t9, scache_size
	beq	t9, zero, fc_done	# skip if no secondary cache
	nop

	# Flushing On-chip Vs Off-Chip caches is different
	# On the 7000 we have the cache on chip
	# On 5xxx it is external
	lw	t9, cpu_type
	li	t8, MIPS_RM7000
	bne	t9, t8, ic_flush_5x	# cpu is rm5x
	nop	

	# Invalidate secondary cache on RM7000
	li	t8, K0BASE
	lw	t9, scache_size
fc_l3:
	cache	INDEX_WRITEBACK_INV_S,0(t8)
	subu	t9, 32
	bgtz	t9, fc_l3
	addu	t8, 32
	b	fc_done
	nop

ic_flush_5x:
	mtc0	zero, C0_TAGLO
	li	t8, K0BASE
	lw	t9, scache_size
fc_l4:
	cache	INDEX_INVALIDATE_S_PAGE,0(t8)
	subu	t9, 4096
	bgtz	t9, fc_l4
	addu	t8, 4096
fc_done:
	jr	ra			# return to the caller 
	nop

/*
 * static void _size_cache()
 * Internal routine to determine cache sizes by looking at config
 * register.  Sizes are returned in registers, as follows:
 */

#define icachesize	t2
#define dcachesize	t3
#define scachesize	t4
#define ilinesize	t5
#define dlinesize	t6
#define slinesize	t7
#define cacheflags	t8

__size_cache:


	mfc0	t0,C0_CONFIG
	/* work out primary i-cache size */
	and	t1,t0,CFG_ICMASK
	srl	t1,CFG_ICSHIFT
	li	icachesize,0x1000
	sll	icachesize,icachesize, t1

	/* primary i-cache line size (fixed) */
	li	ilinesize,LINESIZE

	/* work out primary d-cache size */
	and	t1,t0,CFG_DCMASK
	srl	t1,CFG_DCSHIFT
	li	dcachesize,0x1000
	sll	dcachesize, dcachesize, t1

	/* primary d-cache line size (fixed) */
	li	dlinesize,LINESIZE

	move	scachesize,zero
	move	slinesize,zero
	move	cacheflags,zero	
	/*
 	 * Secondary Cache. RM5xxx and RM7xxx differ in how Scache is
 	 * detected 
 	 */
	mfc0	v1, C0_PRID
	nop
	and	v1, 0xff00
	li	t0, (MIPS_RM7000 << 8)
	beq	v1, t0, RM7XXX
	nop

RM5XXX:
	li	t0, 2
	sw	t0, cache_nway			/* 2 way associative */
	li	t0, MIPS_RM52X0
	sw	t0, cpu_type
	mfc0	t0, C0_CONFIG
	/* no secondary cache if Config.SC == 1 */
	li	t1, CFG_SC
	and	t1,t1,t0
	bnez	t1,_sc_done

	/* also no secondary cache if Config.SS == NONE */
	li	t1, CFG_SSMASK
	and	t1,t1,t0
	li	t0, CFG_SS_NONE
	beq	t1,t0,_sc_done
	
	/* calculate secondary cache size */
	srl	t1,CFG_SSSHIFT
	li	scachesize,512*1024
	sll	scachesize, scachesize,t1
	li	slinesize,LINESIZE
	b	_sc_done
	nop
RM7XXX:

	li	t0, 4
	sw	t0, cache_nway			/* 4 way associative */
	li	t0, MIPS_RM7000
	sw	t0, cpu_type
	mfc0	t0, C0_CONFIG
	li	t1, 0x80000000
	and	t1, t1, t0
	bnez	t1, _sc_done
	nop
	li	slinesize, 32
	li	scachesize, 0x40000		/* 256K of Scache */
_sc_done:
	jr	ra
	nop


/*
 * void init_cache()
 * Work out size of and initialize I, D & S caches.
 */
	.global init_cache
init_cache:
	/*
 	 * Determine the cache sizes
	 */
	move	v0,ra
	jal	__size_cache
	nop

	sw	icachesize,icache_size
	sw	dcachesize,dcache_size
	sw	scachesize,scache_size
	sw	ilinesize,icache_lsize
	sw	dlinesize,dcache_lsize
	sw	slinesize,scache_lsize
	lw	t0, cache_nway
	srl	t0, t0, 1
	srl	icachesize, icachesize, t0
	sw	icachesize, icache_set_size
	srl	dcachesize, dcachesize, t0
	sw	dcachesize, dcache_set_size

	move	ra, v0
	jr	ra
	nop

/*
 * void __flushdcache_7x (addr, len)
 * Flush and invalidate data cache for RM7000 
 */
	.global	__flush_dcache_7x
	.option	reorder off
__flush_dcache_7x:

	lw	a2, dcache_set_size
	srl	a1, a1, 5
_fdc_7x_l1:
	addu	t1, a0, a2			# flush set B.
	cache	INDEX_WRITEBACK_INV_D, 0(t1)
	addu	t1, t1, a2
	cache	INDEX_WRITEBACK_INV_D, 0(t1)	# do set C
	addu	t1, t1, a2			# do set D
	cache	INDEX_WRITEBACK_INV_D, 0(t1)
	cache	INDEX_WRITEBACK_INV_D, 0(a0)	# do set A
	sub	a1, a1, 1
	addu	a0, a0, 32
	bne	a1, zero, _fdc_7x_l1
	nop

	jr	ra
	nop

/*
 * void __flushdcache_5x (addr, len)
 * Flush and invalidate data cache for RM7000 
 */
	.global __flush_dcache_5x
__flush_dcache_5x:

	lw	a2, dcache_set_size
	srl	a1, a1, 5
_fdc_5x_l1:
	addu	t1, a0, a2			# flush set B.
	cache	INDEX_WRITEBACK_INV_D, 0(t1)
	cache	INDEX_WRITEBACK_INV_D, 0(a0)	# do set A
	addu	a0, a0, 32
	sub	a1, a1, 1
	bne	a1, zero, _fdc_5x_l1
	nop
_fdc5x_done:
	.option reorder on
	jr	ra
	nop

/*
 * __flush_hit_wb_dcache(addr, lines)
 * Contents are written back to the memory and cache is invalidated
 */
	.global __flush_hit_wb_dcache
	.option	reorder off
__flush_hit_wb_dcache:
_hwb_dc_l1:
	cache	HIT_WRITEBACK_INV_D, 0(a0)
	addu	a0, a0, 32	
	sub	a1, a1, 1
	bne	a1, zero, _hwb_dc_l1
	nop

	.option reorder on
	jr	ra
	nop

/*
 * __flush_hit_nwb_dcache(addr, len)
 * Invalidate the contents of the cache in the address range
 * Contents are NOT written back to the memory
 */
	.global __flush_hit_nwb_dcache
	.option	reorder off
__flush_hit_nwb_dcache:
_hnwb_dc_l1:
	cache	HIT_INVALIDATE_D, 0(a0)
	addu	a0, a0, 32	
	subu	a1, a1, 1
	bne	a1, zero, _hnwb_dc_l1
	nop

	.option reorder on
	jr	ra
	nop


/*
 * __flush_hit_wb_scache(addr, len)
 * Contents of Secondary Cache are written back to the memory
 * and cache is invalidated
 */
	.global __flush_hit_wb_scache
	.option	reorder off
__flush_hit_wb_scache:
_hwb_sc_l1:
	cache	HIT_WRITEBACK_INV_S, 0(a0)
	addu	a0, a0, 32	
	sub	a1, a1, 1
	bne	a1, zero, _hwb_sc_l1
	nop

	.option reorder on
	jr	ra
	nop

/*
 * __flush_hit_nwb_scache(addr, len)
 * Invalidate the contents of the secondary cache in the address range
 * Contents are NOT written back to the memory
 */
	.global __flush_hit_nwb_scache
	.option	reorder off
__flush_hit_nwb_scache:
_hnwb_sc_l1:
	cache	HIT_INVALIDATE_S, 0(a0)
	addu	a0, a0, 32	
	sub	a1, a1, 1
	bne	a1, zero, _hnwb_sc_l1
	nop

	.option reorder on
	jr	ra
	nop
/*
 * __flush_nwb_scache_5x(addr, pages)
 * Invalidate the contents of the secondary cache in the 4096 wide pages
 * Contents are NOT written back to the memory
 */
	.global __flush_nwb_scache_5x
	.option	reorder off
__flush_nwb_scache_5x:
_nwb_sc_l1:
	cache	INDEX_INVALIDATE_S_PAGE, 0(a0)
	addu	a0, a0, 4096	
	sub	a1, a1, 1
	bne	a1, zero, _nwb_sc_l1
	nop

	.option reorder on
	jr	ra
	nop

/*
 * __flush_icache_7x(addr, len)
 * Flush Icache
 */
	.global	__flush_icache_7x
	.option	reorder off
__flush_icache_7x:
	
	addu	a1, a1, 127			# Align for unrolling
	and	a0, a0, 0xffff80		# Align start address
	li	t0, KSEG0_BASE
	add	a0, a0, t0
	srl	a1, a1, 7			# Number of unrolled loops
	lw	t0, icache_set_size			# Cache size
_fic_7x_l1:
	sub	a1, a1, 1

	cache	INDEX_INVALIDATE_I, 0(a0)		# Set A
	addu	t1, t0, a0
	cache	INDEX_INVALIDATE_I, 0(t1)		# Set B
	add	t1, t1, t0
	cache	INDEX_INVALIDATE_I, 0(t1)		# Set C
	add	t1, t1, t0
	cache	INDEX_INVALIDATE_I, 0(t1)		# Set D
	bne	a1, zero, _fic_7x_l1
	addu	a0, a0, 32

	.option reorder on
	jr	ra
	nop

/*
 * __flush_icache_5x(addr, len)
 * Flush Icache
 */
	.global	__flush_icache_5x
	.option	reorder off
__flush_icache_5x:
	
	addu	a1, a1, 127			# Align for unrolling
	and	a0, a0, 0xffff80		# Align start address
	li	t0, KSEG0_BASE
	add	a0, a0, t0
	srl	a1, a1, 7			# Number of unrolled loops
	lw	t0, icache_set_size			# Cache size
_fic_5x_l1:
	sub	a1, a1, 1

	cache	INDEX_INVALIDATE_I, 0(a0)	# Set A
	addu	t1, t0, a0
	cache	INDEX_INVALIDATE_I, 0(t1)	# Set B
	
	bne	a1, zero, _fic_5x_l1
	addu	a0, a0, 32

	.option reorder on
	jr	ra
	nop
