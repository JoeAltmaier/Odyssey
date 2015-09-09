/*
 *
 * MODULE: sysflash.h - header file for the System Flash Driver
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 09/04/98	- Created by Sudhir
 *
 *
 *
 * This material is a confidential trade secret and proprietary information
 * of ConvergeNet Technologies, Inc. which may not be reproduced, used, sold
 * or transferred to any third party without the prior written consent of
 * ConvergeNet Technologies, Inc.  This material is also copyrighted as an
 * unpublished work under sections 104 and 408 of Title 17 of the United
 * States Code.  Law prohibits unauthorized use, copying or reproduction.
 *
 */


/* These defines need to be here till we get types.h and  hw.h
 */

typedef	unsigned long		addr_t;				/* Size of pointer (addr) */



#ifdef	INCLUDE_ODYSSEY
#define		SYSFLASH_START			0xBD000000
#else
#define		SYSFLASH_START			0xBF000000
#endif
#define		GT_DEV_BANK3_REG_ADDR	0xB4000468;
#define		GT_DEV_BANK3_VAL		0xffff7f16;



#ifndef		_SYSFLASH_
#define		_SYSFLASH_

#define		FLASH_FAIL					0
#define		FLASH_SUCCESS				1



#ifdef		INCLUDE_ODYSSEY
#define		SYSFLASH_CHIP_SIZEB		0x200000		/* 2MB */
#define		SYSFLASH_SIZEB			0x800000		/* 8MB */
#define		MULTI_SECTOR			0
#define		IMAGE2_SECTOR_START		16
#define		TOTAL_SECTORS			32
#else		/* AMD */
#define		SYSFLASH_CHIP_SIZEB		0x80000			/* 512K */
#define		SYSFLASH_SIZEB			0x200000		/* 2MB */
#define		MULTI_SECTOR			7
#define		IMAGE2_SECTOR_START		4
#define		TOTAL_SECTORS			8
#endif		/* AMD */

#define		SYSFLASH_SECTORX_SIZEW	(32 * 1024)
#define		SYSFLASH_SECTOR16_SIZEW	(16 * 1024)
#define		SYSFLASH_SECTOR04_SIZEW	(4 * 1024)
#define		SYSFLASH_SECTOR08_SIZEW	(8 * 1024)

/* Just treat the flash as an address space from 0-SYSFLASH_CHIP_SIZE,
 * and pass it to the macro FLASH1,2... which will convert it to 
 * address each flash chip
 */


#define	MSADR(a)			( (addr_t)(a) & ~(SYSFLASH_SIZEB-1) )
#define	LSADR(a)			( ((addr_t)(a) & (SYSFLASH_CHIP_SIZEB-1)) << 3 )

#define	FLASHX(NUM, a)		((u16 *)((u8 *)(MSADR(a) | LSADR(a)) + ((NUM)<<1)))		

#define	FLASH_FCMD_ADDR1			0x5555
#define	FLASH_FCMD_ADDR2			0x2AAA
#define	FLASH_FCMD_ADDR3			0x5555
	
#define	FLASH_FCMD1					0x00AA
#define	FLASH_FCMD2					0x0055

#define	FLASH_RESET_CMD				0x00F0
#define	FLASH_SUSPEND_CMD			0x00B0
#define	FLASH_RESUME_CMD			0x0030

#define	FLASH_AUTO_SELECT_CMD		0x0090
#define	FLASH_PROGRAM_CMD			0x00A0
#define	FLASH_ERASE_CMD				0x0080
#define	FLASH_CHIP_ERASE_CMD		0x0010
#define	FLASH_SECTOR_ERASE_CMD		0x0030

#define	FLASH_DATA_POLLING			0x80
#define	FLASH_TOGGLE				0x40
#define	FLASH_TOGGLE_SECTOR			0x04
#define	FLASH_TIMING_LIMIT			0x20
#define	FLASH_SECTORE_ERASE_TIMER	0x08




extern	void 	sysflash_init();
extern	void	sysflash_reset(int);
extern	ulong	sysflash_getmanf(int);
extern	ulong	sysflash_getdevid(int);

extern	void	sysflash_chiperase_cmd(int);
extern	int		sysflash_chiperase_verify(int);
extern	int		sysflash_erase();
extern	int		sysflash_sector_erase(int);

extern	int		sysflash_erase1();
extern	int		sysflash_erase2();
extern	int		sysflash_test();

/* The offset has to be half-word aligned
 * len should be multiple of 2 bytes except 
 * the last buffer of the image to be programmed
 */
int
sysflash_program(u8 *, u32, u32);



#endif		/* _SYSFLASH_  */
