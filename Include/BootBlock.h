/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: BootBlock.h
 * 
 * Description:
 * This file contains the definition for Boot Block which is passed as  
 * as a parameter to Nucleus from Boot ROM
 * 
 * Update Log:
 * 6/18/99 Raghava Kondapalli: Created 
 ************************************************************************/

#ifndef BOOTBLOCK_H
#define BOOTBLOCK_H

#ifndef __ASM__
#include "pcimap.h"
#endif	/* __ASM__ */ 

#ifdef  __cplusplus
extern "C" {
#endif

#define BOOT_BLOCK_SIZE		2048

#ifndef __ASM__
typedef struct {
	U32	b_slot;				/* Board Slot */
	U32	b_type;				/* Board Type */
	U32	b_dramsize;			/* Total size of DRAM */
	U32	b_bank01_addr;		/* Virtual addr of Banks 01 */
	U32	b_bank01_size;		/* Total Size of Banks 01 */
	U32	b_bank23_addr;		/* Virtual addr of Banks 23 */
	U32	b_bank23_size;		/* Total Size of Banks 23 */
	mmap_t	b_memmap;		/* PCI Map sent from HBC */
	U32 b_cbHeap;			/* Size of small heap */
	U32 b_iSlotHbcMaster;	/* IOP_HBC0 or IOP_HBC1 */
	U8	b_imghdr[128];		/* Image header defined in imghdr.h. 
							 * Change the size if imghdr_t 
							 * exceeds 128
							 */
	U8	pad[1452];			/* Total of 2048 Bytes */
} bootblock_t;


/*
 * IOP configuration information exported to HBC during CMB Boot
 */
typedef struct {
	U32	i_br_majver;		/* Boot ROM Major Version */
	U32	i_br_minver;		/* Boot ROM Minor Version */
	U32	i_bank01_size;		/* Total Size of Banks 01 */
	U32	i_bank23_size;		/* Total Size of Banks 23 */
	U32	i_slot;				/* Board Slot */
	U32	i_type;				/* Board Type */
	U8	pad[488];			/* Total of 512 bytes */
} iop_cfg_t;

#endif	/* __ASM__ */

/*
 * Offset in PCI window where the IOP configuration info is placed
 */
#define IOP_CFG_OFFSET			0x2400

/*
 * Offset where target context is saved during NMI
 */
#define CONTEXT_REGION_OFFSET	0x2800
#define KSEG1_CONTEXT_ADDR		(0xA0000000 + CONTEXT_REGION_OFFSET)

/*
 * Crash log area
 */
#define CRASH_LOG_OFFSET		0x8000
#define KSEG1_CRASH_LOG_ADDR	(0xA0000000 + CRASH_LOG_OFFSET)
#ifdef  __cplusplus
}
#endif

#endif		/* BOOTBLOCK_H */
