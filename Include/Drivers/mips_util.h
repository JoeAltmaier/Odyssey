/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: mips_util.h
 * 
 * Description:
 * This file contains mips support functions for TLB/Cache 
 * 
 * Update Log:
 * 03/23/99 Raghava Kondapalli: Created 
 ************************************************************************/

#ifndef MIPS_UTIL_H
#define MIPS_UTIL_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#ifndef MIPS_REG_H
#include "mips_reg.h"
#endif
#ifndef TYPES_H
#include <types.h>
#endif

#define	SYNC_CACHE_FOR_DEV	1
#define	SYNC_CACHE_FOR_CPU	2

#define	CACHE_SET_A	0x00000000	/* For locking */
#define	CACHE_SET_B	0x10000000	/* For locking */


	VOID	tlb_init(VOID);			/* Setup TLB entries */


/*
 * Cache routines
 * Len here is the number of bytes
 * Warning: Cache line on MIPS is 32 bytes. Syncing is done
 * in multiples of 32 bytes. 
 */

	STATUS	mips_sync_cache(VOID *addr, U32 len, U32 type);
	STATUS	mips_flush_cache(VOID);
	STATUS	mips_flush_dcache(VOID);
	STATUS	mips_flush_icache(VOID);
	STATUS	mips_lock_icache(VOID *addr, U32 len);
	STATUS	mips_lock_dcache(VOID *addr, U32 len);
	STATUS	init_cache(VOID);

/*
 * Cache Configuration option on MIPS cpu
 * Do not change these values.
 */
#define	CACHE_WRITE_THRU_NOALLOC	CFG_C_WTNOALLOC
#define	CACHE_WRITE_THRU_ALLOC		CFG_C_WTALLOC
#define	CACHE_UNCACHED			CFG_C_UNCACHED
#define	CACHE_WRITEBACK			CFG_C_WRITEBACK

/*
 * Call this routine first before calling other cache routines.
 */
#ifdef CONFIG_BOOT
		STATUS	mips_init_cache(U32 type);
#else
		STATUS	mips_init_cache(void);
#endif


/*
 * Private Functions. Do not use
 */

	VOID	tlb_flush(VOID);
	VOID 	tlb_get_pid(VOID);
	U32 	tlb_get_hi(I32);
	VOID 	tlb_get_wired(VOID);
	U32 	tlb_get_pagemask(I32);
	U32	tlb_get_lo0(I32);
	U32	tlb_get_lo1(I32);
	VOID	tlb_flush(VOID);
	VOID 	tlb_clear_entry(I32);
	VOID	tlb_set_wired(U32 index);
	VOID	tlb_set_pid(VOID);
	VOID	tlb_set_wired(U32 index);
	VOID	tlb_set_pid(VOID);

	VOID	config_cache(U32);
	VOID	flush_cache(VOID);
	VOID	lock_dcache(U32, U32, U32);
	VOID	lock_icache(U32, U32, U32);
	VOID	__flush_hit_nwb_dcache(U32, U32);
	VOID	__flush_hit_wb_dcache(U32, U32);
	VOID	__flush_hit_nwb_scache(U32, U32);
	VOID	__flush_hit_wb_scache(U32, U32);
	VOID	__flush_nwb_scache_5x(U32, U32);
	VOID	__flush_icache_7x(U32, U32);
	VOID	__flush_icache_5x(U32, U32);
	VOID	__flush_dcache_7x(U32, U32);
	VOID	__flush_dcache_5x(U32, U32);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif MIPS_UTIL_H
