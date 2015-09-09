/*
 *
 * MODULE: nflash.h - header file for the System E2PROM Driver
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 11/17/98	- Created by Sudhir
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

#ifndef	_NFLASH_
#define	_NFLASH_
#ifdef  __cplusplus
extern "C" {
#endif

#define	OK						0
#define	NFLASH_BUSY				1
#define	NFLASH_RESET_ERROR		2
#define	NFLASH_ID_ERROR			3
#define	NFLASH_ERASE_ERROR		4
#define	NFLASH_PROGRAM_ERROR	5
#define	NFLASH_READ_ERROR		6
#define	NFLASH_READ_CORRECTED	7
#define	NFLASH_READ_ECC_CORRECTED	8
#define	NFLASH_READ_UNCORRECTABLE	9
#define	NFLASH_SEMA_ERROR		10

#define	NFLASH_TIMEOUT		1000000

#ifdef	CONFIG_E2
#define	NFLASH_CNTL_BASE	0xBC088000		// tested
#define	NFLASH_DATA_BASE	0xBC800004		// tested
#define	NFLASH_CONF_BASE	0xBF010004

#define	NFLASH_MAKER_CODE	0xECECECEC		// tested
#define	NFLASH_DEV_CODE		0x73737373		// tested
		
/* Definitions for CONTROL Signals */
#define	NFLASH_CNTL_CLE		0x01		// tested
#define	NFLASH_CNTL_ALE		0x02		// tested
#define	NFLASH_CNTL_OP		0x04
#define	NFLASH_CNTL_RW		0x08
#define	NFLASH_CNTL_WE		0x10
#define	NFLASH_CNTL_WP		0x20
#define	NFLASH_CNTL_RE		0x40
#define	NFLASH_CNTL_CE		0x80
		
/* Definitions for Command for the NAND Flash */
#define	NFLASH_CMD_RESET		0xFFFFFFFF
#define	NFLASH_CMD_READ1		0x00000000
#define	NFLASH_CMD_READ2		0x01010101
#define	NFLASH_CMD_READ3		0x50505050
#define	NFLASH_CMD_DATAIN		0x80808080
#define	NFLASH_CMD_PROGRAM		0x10101010
#define	NFLASH_CMD_ERASE_SETUP	0x60606060
#define	NFLASH_CMD_ERASE_START	0xD0D0D0D0
#define	NFLASH_CMD_STATUS		0x70707070
#define	NFLASH_CMD_IDREAD		0x90909090		// tested

/*Definitions for the Status */
#define	NFLASH_STATUS_FAIL		0x01010101
#define	NFLASH_STATUS_READY		0x40404040
#define	NFLASH_STATUS_NOPROTECT	0x80808080

#define	NFLASH_PAGE_SIZE	2048
#define	NFLASH_PAGE_MASK	(NFLASH_PAGE_SIZE -1)

		// tested
#define	NFLASH_CNTL_WRITE(X)	{	\
		flash_cntl = (X); 	\
		*((volatile U8 *)NFLASH_CNTL_BASE) = flash_cntl; \
		}

		// tested
#define	NFLASH_CNTL_READ	 flash_cntl
		// tested
#define	NFLASH_DATA_WRITE(X) *((volatile U32 *)NFLASH_DATA_BASE) = (X)
		// tested
#define	NFLASH_DATA_READ	 \
		(U32)(*((volatile unsigned long *)NFLASH_DATA_BASE) & 0xFFFFFFFF)
		// tested
#define	NFLASH_STS_READ		\
		((*((volatile unsigned long *)NFLASH_CONF_BASE) & 0xFF000000) >> 24)

#else	/* CONFIG_E2 */
		
		

#define	NFLASH_CNTL_BASE	0xBC800006
#define	NFLASH_DATA_BASE	0xBC810007
#define	NFLASH_STS_BASE		0xBF000000

#define	NFLASH_MAKER_CODE	0x98
#define	NFLASH_DEV_CODE		0xE6

/* Definitions for CONTROL Signals */
#define	NFLASH_CNTL_CLE		0x8000
#define	NFLASH_CNTL_ALE		0x4000
#define	NFLASH_CNTL_OP		0x2000
#define	NFLASH_CNTL_RW		0x1000
#define	NFLASH_CNTL_WE		0x0800
#define	NFLASH_CNTL_WP		0x0400
#define	NFLASH_CNTL_RE		0x0200
#define	NFLASH_CNTL_CE		0x0100

/* Definitions for Command for the NAND Flash */
#define	NFLASH_CMD_RESET		0xFF
#define	NFLASH_CMD_READ1		0x00
#define	NFLASH_CMD_READ2		0x01
#define	NFLASH_CMD_READ3		0x50
#define	NFLASH_CMD_DATAIN		0x80
#define	NFLASH_CMD_PROGRAM		0x10
#define	NFLASH_CMD_ERASE_SETUP	0x60
#define	NFLASH_CMD_ERASE_START	0xD0
#define	NFLASH_CMD_STATUS		0x70
#define	NFLASH_CMD_IDREAD		0x90

/*Definitions for the Status */
#define	NFLASH_STATUS_FAIL		0x01
#define	NFLASH_STATUS_READY		0x40
#define	NFLASH_STATUS_NOPROTECT	0x80

#define	NFLASH_PAGE_SIZE	512
#define	NFLASH_PAGE_MASK	(NFLASH_PAGE_SIZE -1)

#define	NFLASH_CNTL_WRITE(X)	{	\
		flash_cntl = (X); 	\
		*((volatile U16 *)NFLASH_CNTL_BASE) = flash_cntl; \
		}
		
#define	NFLASH_CNTL_READ	 flash_cntl
#define	NFLASH_DATA_WRITE(X) *((volatile U8 *)NFLASH_DATA_BASE) = (X)
#define	NFLASH_DATA_READ	 \
		(U8)(*((volatile unsigned long long *)NFLASH_STS_BASE) & 0xFF)
#define	NFLASH_STS_READ		\
		(*((volatile unsigned long long *)NFLASH_STS_BASE) & 0x100)

#endif	/* CONFIG_E2 */
extern	U32 NFlashStatus;

void	nflash_release_sema();
U32		nflash_get_id(U32 bank);
STATUS	nflash_reset();
STATUS	nflash_init();
STATUS	nflash_erase_block(U32  block_addr);
STATUS	nflash_write_page(U8 *buf, U32 pagenum);
STATUS	nflash_read_page(U8 *buf, U32 pagenum);


/* Ignore these calls, used for debugging */
#if 0
STATUS	nflash_init_noecc();
STATUS	nflash_read_noecc(U8 *buf, U32 offset, I32 len);
STATUS	nflash_program_noecc(U8 *buf, U32 offset, I32 len);
STATUS	nflash_program_page_noecc(U8 *buf, U32 offset, I32 len);
STATUS	nflash_program_partial_page_noecc(U8 *buf, U32 offset, I32 len);
STATUS	nflash_test();
#endif

#ifdef  __cplusplus
}
#endif
#endif	/* _NFLASH_ */
