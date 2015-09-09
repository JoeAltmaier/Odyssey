//************************************************************************
// FILE:		win_mips_util.cpp
//
// PURPOSE:		Fake implementations were separated into the *.cpp
//				file because multiple inclusions of the header
//				were causoing link errors.
//************************************************************************

#include "CtTypes.h"
#include "OsTypes.h"
#define TYPES_H	// don't include Types.h
#define I32 signed long
#include "mips_util.h"

	VOID	tlb_init(VOID){}			/* Setup TLB entries */

	STATUS	mips_sync_cache(VOID *addr, U32 len, U32 type) { return OK; }
	STATUS	mips_flush_cache(VOID) { return OK; }
	STATUS	mips_flush_dcache(VOID) { return OK; }
	STATUS	mips_flush_icache(VOID) { return OK; }
	STATUS	mips_lock_icache(VOID *addr, U32 len) { return OK; }
	STATUS	mips_lock_dcache(VOID *addr, U32 len) { return OK; }
	STATUS	init_cache(VOID) { return OK; }


	VOID	tlb_flush(VOID) {}
	VOID 	tlb_get_pid(VOID) {}
	U32 	tlb_get_hi(I32) { return 0; }
	VOID 	tlb_get_wired(VOID) {}
	U32 	tlb_get_pagemask(I32) { return 0; }
	U32	tlb_get_lo0(I32) { return 0;}
	U32	tlb_get_lo1(I32) { return 0; }
	VOID	tlb_flush(VOID);
	VOID 	tlb_clear_entry(I32) {}
	VOID	tlb_set_wired(U32 index) {}
	VOID	tlb_set_pid(VOID) {}
	VOID	tlb_set_wired(U32 index);
	VOID	tlb_set_pid(VOID);

	VOID	config_cache(U32) {}
	VOID	flush_cache(VOID) {}
	VOID	lock_dcache(U32, U32, U32) {}
	VOID	lock_icache(U32, U32, U32) {}
	VOID	__flush_hit_nwb_dcache(U32, U32) {}
	VOID	__flush_hit_wb_dcache(U32, U32) {}
	VOID	__flush_hit_nwb_scache(U32, U32) {}
	VOID	__flush_hit_wb_scache(U32, U32) {}
	VOID	__flush_nwb_scache_5x(U32, U32) {}
	VOID	__flush_icache_7x(U32, U32) {}
	VOID	__flush_icache_5x(U32, U32) {}
	VOID	__flush_dcache_7x(U32, U32) {}
	VOID	__flush_dcache_5x(U32, U32) {}

#ifdef CONFIG_BOOT
		STATUS	mips_init_cache(U32 type) { return OK; }
#else
		STATUS	mips_init_cache(void) { return OK; }
#endif