/*
 *
 * MODULE: pcimap.c - PCI Interface Module
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 09/29/98	- Created by Sudhir
 * 04/12/99 - Not Initializing Galileo BAR again
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
#include "nucleus.h"
#include "types.h"
#include "pcidev.h"
#include "pcimap.h"
#include "galileo.h"
#include "ispdev.h"
#include "sysflash.h"
#include "pcialloc.h"

#define DEBUG	1
/*
#define	C_ENABLE 1
*/
/* Prototypes */
void print_pcimap();

/* Global Data Structures used by other */
membanks_t membanks;
extern int real_slot_num[];




mmap_t memmaps = {
	/* Board Types */
	{ BOARD_HBC, BOARD_HBC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	  BOARD_SSD, BOARD_SSD, BOARD_SSD, BOARD_SSD, 
	  BOARD_SSD, BOARD_SSD, BOARD_SSD, BOARD_SSD, 
	  BOARD_RAC, 0, 0, BOARD_NIC,
	  BOARD_RAC, 0, 0, BOARD_NIC,
	},
	/* Pci Id */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
	},
	
	/* pci address of each cards window */
	{ PCI_WINDOW_START, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* HBC  */ 
	  PCI_WINDOW_START + M(512),			/* B1 */
	  PCI_WINDOW_START + M(512) + M(64),	/* B2 */
	  PCI_WINDOW_START + M(512) + M(128),	/* B3 */ 
	  PCI_WINDOW_START + M(512) + M(192),	/* B4 */ 
	  PCI_WINDOW_START + M(1024),			/* D1 */ 
	  PCI_WINDOW_START + M(1024)+ M(64),	/* D2 */ 
	  PCI_WINDOW_START + M(1024)+ M(128),	/* D3 */ 
	  PCI_WINDOW_START + M(1024)+ M(192),	/* D4 */ 

	  PCI_WINDOW_START + M(256) + M(192),	/* A4 */ 
	  PCI_WINDOW_START + M(256),			/* A1 */ 
	  PCI_WINDOW_START + M(256) + M(64),	/* A2 */ 
	  PCI_WINDOW_START + M(256) + M(128),	/* A3 */ 

	  PCI_WINDOW_START + M(768) + M(192),	/* C4 */ 
	  PCI_WINDOW_START + M(768),			/* C1 */ 
	  PCI_WINDOW_START + M(768) + M(64),	/* C2 */ 
	  PCI_WINDOW_START + M(768) + M(192),	/* C3 */ 
	},
	
	/* Rest of the stuffs */
	0, 0, 0, 0, 0, 0, 
	{0, 0,},
	{0, 0,},
	{0, 0,}
};

U32	pmap_size[MAX_SLOTS] = { 
		M(64), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		M(64), M(64), M(64), M(64), M(64), M(64), M(64), M(64),
		M(64), M(64), M(64), M(512), M(64), M(64), M(64), M(512)
};

char	*slotname[MAX_SLOTS] = {
	"H0", "H1",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"Unknown Device",
	"B1", "B2", "B3", "B4", "D1", "D2", "D3", "D4", 
	"A4", "A1", "A2", "A3", "C4", "C1", "C2", "C3"
 };

 

void
get_membank_config()
{
	U32	low, high, page;
	
	/* Get Bank 10 parameters */
	low = gt_read(GT_PROC_RAS10_LO_OFF)  & GT_DEC_LOW_MASK;
	page = low & ~GT_DEC_HIGH_MASK;
	high = page | (gt_read(GT_PROC_RAS10_HI_OFF) & GT_DEC_HIGH_MASK);
	membanks.bank10_start = low << 21;
	membanks.bank10_size  = ((high << 21) | 0x1FFFFF) - (low << 21) + 1;

	/* Get Bank 32 parameters */
	low = gt_read(GT_PROC_RAS32_LO_OFF)  & GT_DEC_LOW_MASK;
	page = low & ~GT_DEC_HIGH_MASK;
	high = page | (gt_read(GT_PROC_RAS32_HI_OFF) & GT_DEC_HIGH_MASK);
	membanks.bank32_start = low << 21;
	membanks.bank32_size  = ((high << 21) | 0x1FFFFF) - (low << 21) + 1;
}

void
init_galileo()
{
#ifdef INCLUDE_ODYSSEY
	U32		val;
#endif	INCLUDE_ODYSSEY

	/* Program Galileo as Master */
	pciconf_writew(0x80000000, PCI_CONF_COMM,
			PCI_COMM_IO | PCI_COMM_MEM | PCI_COMM_MASTER | PCI_COMM_B2B);
	
	/* Program the Cache Line Size */
	pciconf_writeb(0x80000000, PCI_CONF_CLS, 0x00);
	
	/* Program the Latency Timer */
	pciconf_writeb(0x80000000, PCI_CONF_LT, 0xFF);

#ifdef INCLUDE_ODYSSEY
	/* Program the Sync Mode */
#if 0
	/* Tclk/Pclk 75/33 */
	gt_write(0xC00, 0x0A);
	/* Tclk/Pclk 75/50 or 75/66*/
	gt_write(0xC00, 0x02);
#else 
	/* Tclk/Pclk unspecified */
	gt_write(0xC00, 0x00);
#endif
#endif	INCLUDE_ODYSSEY
	
	/* Enable the Max Burst Len for DRAM10, DRAM32 */
	gt_write(0xC40, 0x43);
	
	/* Retry Values */

	gt_write(GT_PCI_RETRY_OFF, 0xFFFF);
	
#ifdef INCLUDE_ODYSSEY
	/* Enable Flow Thru - One Sampple */
	gt_write(GT_DRAM0_PARMS_OFF, 0x0861);
	gt_write(GT_DRAM1_PARMS_OFF, 0x0861);
	gt_write(GT_DRAM2_PARMS_OFF, 0x0861);
	gt_write(GT_DRAM3_PARMS_OFF, 0x0861);

	/* Let the Starting address of the PCI Window be 0x800000000
	 * and Map the entire 2G into PCI 
	 */
	gt_write(GT_PROC_PCI_MEM_LO_OFF, (PCI_WINDOW_START >> 21));
	gt_write(GT_PROC_PCI_MEM_HI_OFF, 0x7F);

	gt_write(GT_PROC_PCI0_MEM1_LO_OFF, (PCI_WINDOW_START >> 21));
	gt_write(GT_PROC_PCI0_MEM1_HI_OFF, 0x7F);

	/* Configure the Galileo for 2G PCI Space */
	val = gt_read(GT_CPU_CONFIG_OFF);
	val |= GT_PCI_2GIG;
	gt_write(GT_CPU_CONFIG_OFF, val);

	
	/* CS need not be exposed onto PCI */
	gt_write(GT_PCI_CS20_BSIZE_OFF, 0);
	gt_write(GT_PCI_BOOTCS_BSIZE_OFF, 0);

#endif	INCLUDE_ODYSSEY
}
#ifdef	CONFIG_BOOT
extern	U32	boardSlot;
void
create_memmap()
{
	/* This function is called by HBC only */
	bzero(&memmaps, sizeof(mmap_t));
	/* Initialize the Memrory Sizes */
	get_membank_config();
	memmaps.nFragment	= 2;
	memmaps.aPa[0]		= membanks.bank10_start;
	memmaps.aCb[0]		= membanks.bank10_size;
	memmaps.aPa[1]		= membanks.bank32_start;
	memmaps.aCb[1]		= membanks.bank32_size;

	memmaps.aP[0]		= PTOV(memmaps.aPa[0]);
	memmaps.aP[1]		= PTOV1(memmaps.aPa[1]);
	
	/* Initialize the PCI Memory Map and pci_id */
	/* HBC0 */
	memmaps.aBoardType[0]	= BOARD_HBC;
	memmaps.aIdPci[0]		= 0x80000000;
	memmaps.aPaPci[0]		= PCI_WINDOW_START;

	/* Initialize the PCI Memory Map and pci_id */
	/* HBC1 */
	memmaps.aIdPci[1]		= 0x80000000;
	memmaps.aPaPci[1]		= PCI_WINDOW_START+ M(128);

	/* Init Slot A1 */
	memmaps.aIdPci[25] = PADDR(PCI_A_BUSNUM, 12, 0);
	memmaps.aPaPci[25] = PCI_WINDOW_START + M(256);
	/* Init Slot A2 */
	memmaps.aIdPci[26] = PADDR(PCI_A_BUSNUM, 13, 0);
	memmaps.aPaPci[26] = PCI_WINDOW_START + M(256) + M(64);
	/* Init Slot A3 */
	memmaps.aIdPci[27] = PADDR(PCI_A_BUSNUM, 14, 0);
	memmaps.aPaPci[27] = PCI_WINDOW_START + M(256) + M(128);
	/* Init Slot A4 */
	memmaps.aIdPci[24] = PADDR(PCI_A_BUSNUM, 15, 0);
	memmaps.aPaPci[24] = PCI_WINDOW_START + M(256) + M(192);

	/* Init Slot B1 */
	memmaps.aIdPci[16] = PADDR(PCI_B_BUSNUM, 12, 0);
	memmaps.aPaPci[16] = PCI_WINDOW_START + M(512);
	/* Init Slot B2 */
	memmaps.aIdPci[17] = PADDR(PCI_B_BUSNUM, 13, 0);
	memmaps.aPaPci[17] = PCI_WINDOW_START + M(512) + M(64);
	/* Init Slot B3 */
	memmaps.aIdPci[18] = PADDR(PCI_B_BUSNUM, 14, 0);
	memmaps.aPaPci[18] = PCI_WINDOW_START + M(512) + M(128);
	/* Init Slot B4 */
	memmaps.aIdPci[19] = PADDR(PCI_B_BUSNUM, 15, 0);
	memmaps.aPaPci[19] = PCI_WINDOW_START + M(512) + M(192);
	
	/* Init Slot C1 */
	memmaps.aIdPci[29] = PADDR(PCI_C_BUSNUM, 12, 0);
	memmaps.aPaPci[29] = PCI_WINDOW_START + M(768);
	/* Init Slot C2 */
	memmaps.aIdPci[30] = PADDR(PCI_C_BUSNUM, 13, 0);
	memmaps.aPaPci[30] = PCI_WINDOW_START + M(768) + M(64);
	/* Init Slot C3 */
	memmaps.aIdPci[31] = PADDR(PCI_C_BUSNUM, 14, 0);
	memmaps.aPaPci[31] = PCI_WINDOW_START + M(768) + M(128);
	/* Init Slot C4 */
	memmaps.aIdPci[28] = PADDR(PCI_C_BUSNUM, 15, 0);
	memmaps.aPaPci[28] = PCI_WINDOW_START + M(768) + M(192);

	/* Init Slot D1 */
	memmaps.aIdPci[20] = PADDR(PCI_D_BUSNUM, 12, 0);
	memmaps.aPaPci[20] = PCI_WINDOW_START + M(1024);
	/* Init Slot D2 */
	memmaps.aIdPci[21] = PADDR(PCI_D_BUSNUM, 13, 0);
	memmaps.aPaPci[21] = PCI_WINDOW_START + M(1024) + M(64);
	/* Init Slot D3 */
	memmaps.aIdPci[22] = PADDR(PCI_D_BUSNUM, 14, 0);
	memmaps.aPaPci[22] = PCI_WINDOW_START + M(1024) + M(128);
	/* Init Slot D4 */
	memmaps.aIdPci[23] = PADDR(PCI_D_BUSNUM, 15, 0);
	memmaps.aPaPci[23] = PCI_WINDOW_START + M(1024) + M(192);
	

	memmaps.iCabinet = 0;
	memmaps.iSlot 	 = boardSlot;					/* Slot Number */
	memmaps.cbSlave  = M(64);						/* Size */
	memmaps.pciSlave = memmaps.aPaPci[boardSlot];	/* PCI Address */
	memmaps.paSlave	 = REMAP_ADDRESS;				/* Local Reamp Address */
}
#endif
	

#ifdef	CONFIG_BOOT
extern	U32 is_master;
#ifdef	CONFIG_E2
STATUS
init_onboard_bridges()
{
	U32		pci_id;
	U32		mbase, mlmt;
	
	printf("\n\r");
	/* Initialize the Bridge E */
	printf("Initializing Bridge E ....");
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_E_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(64);
	mlmt  = mbase - 1 + M(64);
	bridge_init(pci_id, PCI_G_BUSNUM, PCI_E_BUSNUM, PCI_E_SUBNUM, mbase, mlmt); 
	printf("OK\n\r");

	/* Initialize the Bridge F */
	printf("Initializing Bridge F ....");
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_F_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(1024);
	bridge_init(pci_id, PCI_G_BUSNUM, PCI_F_BUSNUM, PCI_F_SUBNUM, mbase, mlmt);
	printf("OK\n\r");

	/* Do not init rest of the Bridges if it is a slave */
	if ( is_master) {
		/* Initialize the Bridge H */
		printf("Initializing Bridge H ....");
		pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 0);
		if ( !pci_id) {
			printf("Fail\n\r");
			return(BRIDGE_H_ACCESS_ERROR);
		}
		mbase = PCI_WINDOW_START+M(256);
		mlmt  = mbase - 1 + M(512);
		bridge_init(pci_id, PCI_F_BUSNUM, PCI_H_BUSNUM, PCI_H_SUBNUM, 
											mbase, mlmt);
		printf("OK\n\r");

		/* Initialize the Bridge C */
		printf("Initializing Bridge C ....");
		pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 1);
		if ( !pci_id) {
			printf("Fail\n\r");
			return(BRIDGE_C_ACCESS_ERROR);
		}
		mbase = PCI_WINDOW_START+M(768);
		mlmt  = mbase - 1 + M(256);
		bridge_init(pci_id, PCI_F_BUSNUM, PCI_C_BUSNUM, PCI_C_SUBNUM, 
											mbase, mlmt);
		printf("OK\n\r");

		/* Initialize the Bridge D */
		printf("Initializing Bridge D ....");
		pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 2);
		if ( !pci_id) {
			printf("Fail\n\r");
			return(BRIDGE_D_ACCESS_ERROR);
		}
		mbase = PCI_WINDOW_START+M(1024);
		mlmt  = mbase - 1 + M(256);
		bridge_init(pci_id, PCI_F_BUSNUM, PCI_D_BUSNUM, PCI_D_SUBNUM, 
											mbase, mlmt);
		printf("OK\n\r");

		/* Initialize the Bridge A */
		printf("Initializing Bridge A ....");
		pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 0);
		if ( !pci_id) {
			printf("Fail\n\r");
			return(BRIDGE_A_ACCESS_ERROR);
		}
		mbase = PCI_WINDOW_START+M(256);
		mlmt  = mbase - 1 + M(256);
		bridge_init(pci_id, PCI_H_BUSNUM, PCI_A_BUSNUM, PCI_A_SUBNUM, 
											mbase, mlmt);
		printf("OK\n\r");

		/* Initialize the Bridge B */
		printf("Initializing Bridge B ....");
		pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 1);
		if ( !pci_id) {
			printf("Fail\n\r");
			return(BRIDGE_B_ACCESS_ERROR);
		}
		mbase = PCI_WINDOW_START+M(512);
		mlmt  = mbase - 1 + M(256);
		bridge_init(pci_id, PCI_H_BUSNUM, PCI_B_BUSNUM, PCI_B_SUBNUM,
												mbase, mlmt);
		printf("OK\n\r");
	}
	
	/* Make Bridge E non-pre-fetchable */
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 0);
	
	/* Read the Bridge E Mem abse and Mem Limit */
	mbase = pciconf_readw(pci_id, PCI_BCONF_PMEMBASE) & 0xFFF0;
	mlmt  = pciconf_readw(pci_id, PCI_BCONF_PMEMLIMIT) | 0xF;
	
	/* Now Disable the Pref Memory */
	pciconf_writew(pci_id, PCI_BCONF_PMEMBASE, (0xFFFFFFFF >> PCI_BMEM_SHIFT));
	pciconf_writew(pci_id, PCI_BCONF_PMEMLIMIT, (0 >> PCI_BMEM_SHIFT));
	
	/* Now Enable the Non-Pref Memory Window */
	pciconf_writew(pci_id, PCI_BCONF_MEMBASE, mbase );
	pciconf_writew(pci_id, PCI_BCONF_MEMLIMIT, mlmt );
	
	/* Configure the local bridge for not support fast back-to-back
	 * on the secondary Bus */
	pciconf_writew(pci_id, PCI_BCONF_CNTL, 0);
		
	return(OK);
}



#else
STATUS
init_onboard_bridges()
{
	U32		pci_id;
	U32		mbase, mlmt;
	
	printf("\n\r");
	/* Initialize the Bridge E */
	printf("Initializing Bridge E ....");
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 4);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_E_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(64);
	mlmt  = mbase - 1 + M(64);
	bridge_init(pci_id, PCI_G_BUSNUM, PCI_E_BUSNUM, PCI_E_SUBNUM, mbase, mlmt); 
	printf("OK\n\r");
	
	/* Initialize the Bridge A */
	printf("Initializing Bridge A ....");
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_A_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(256);
	bridge_init(pci_id, PCI_G_BUSNUM, PCI_A_BUSNUM, PCI_A_SUBNUM, mbase, mlmt);
	printf("OK\n\r");
	
	/* Initialize the Bridge B */
	printf("Initializing Bridge B ....");
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_B_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(512);
	mlmt  = mbase - 1 + M(256);
	bridge_init(pci_id, PCI_G_BUSNUM, PCI_B_BUSNUM, PCI_B_SUBNUM, mbase, mlmt);
	printf("OK\n\r");
	
	/* Initialize the Bridge C */
	printf("Initializing Bridge C ....");
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 2);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_C_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(768);
	mlmt  = mbase - 1 + M(256);
	bridge_init(pci_id, PCI_G_BUSNUM, PCI_C_BUSNUM, PCI_C_SUBNUM, mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge D */
	printf("Initializing Bridge D ....");
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 3);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_D_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(1024);
	mlmt  = mbase - 1 + M(256);
	bridge_init(pci_id, PCI_G_BUSNUM, PCI_D_BUSNUM, PCI_D_SUBNUM, mbase, mlmt);
	printf("OK\n\r");
	
	/* Make Bridge E non-pre-fetchable */
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 4);
	
	/* Read the Bridge E Mem abse and Mem Limit */
	mbase = pciconf_readw(pci_id, PCI_BCONF_PMEMBASE) & 0xFFF0;
	mlmt  = pciconf_readw(pci_id, PCI_BCONF_PMEMLIMIT) | 0xF;
	
	/* Now Disable the Pref Memory */
	pciconf_writew(pci_id, PCI_BCONF_PMEMBASE, (0xFFFFFFFF >> PCI_BMEM_SHIFT));
	pciconf_writew(pci_id, PCI_BCONF_PMEMLIMIT, (0 >> PCI_BMEM_SHIFT));
	
	/* Now Enable the Non-Pref Memory Window */
	pciconf_writew(pci_id, PCI_BCONF_MEMBASE, mbase );
	pciconf_writew(pci_id, PCI_BCONF_MEMLIMIT, mlmt );
	
	/* Configure the local bridge for not support fast back-to-back
	 * on the secondary Bus */
	pciconf_writew(pci_id, PCI_BCONF_CNTL, 0);
	
	return(OK);
}
#endif /* CONFIG_E2 */
#endif /* CONFIG_BOOT */

void
init_all_bridges()
{

	node_t	*tnode;
	int		i, nonleaf =0;
	int pbus, secbus, subbus;
	unsigned long start;
	unsigned long end;
	unsigned long pci_id;

	printf("Initializing the Bridges....");
	tnode = treenode;
	nonleaf = 0;
	/* Traverse the tree starting from the treenode */
	do {
		for(i=0; i < tnode->node_count; i++) {
			/* Skip if the node is not a bridge */
			if (!(tnode->permnodes[i]->node_type & IS_BRIDGE)) 
				continue;
			/* Get the parameters for the bridge */
			start = PCI_WINDOW_START + tnode->permnodes[i]->mem_start * M(64);
			end = start - 1 + tnode->permnodes[i]->mem_size * M(64);
			pbus 	= tnode->bus_num;
			secbus	= tnode->permnodes[i]->bus_num;
			if ( tnode->permnodes[i]->leaf )
				subbus = secbus;
			else
				subbus = secbus + 4;
			pci_id = tnode->permnodes[i]->pci_id;
			if ( (tnode->permnodes[i]->leaf == 0) && 
							(tnode->permnodes[i]->node_count==0)) {
				/* Bridge does not have any boards connected */
				bridge_init(pci_id, pbus, secbus, subbus, 0xFFFFFFFF, 0);
			} else {
				bridge_init(pci_id, pbus, secbus, subbus, start, end);
			}
		}
		/* get the next node which has children */
		tnode = get_nonleaf(treenode, nonleaf);
		nonleaf++;
	} while(nonleaf < 5);
	printf("Done\n\r");
}

void
init_pci_galileo()
{
	int	i, secbus;
	pcislot_t   *slotp;
	unsigned char size_slot, size;
	U32	pci_id;
	
	printf("Initalizing the Galileo of other boards....");
	for(i=2; i < MAX_PCI_SLOTS; i++) {
		slotp = &pcislot[i];
		/* Get the pci_id of the Gailleo */
		if ( slotp->card_type & IS_BRIDGE) {
			/* Galileo is behind this bridge */
			secbus = pciconf_readb(slotp->pci_id, PCI_BCONF_SECBUS);
			pci_id = get_pci_id(secbus, GAL120_ID, 0);
		} else {
			if ( slotp->card_type)
				pci_id = slotp->pci_id;
			else
				pci_id = 0;
		}

		if (  pci_id ) {
			/* size_slot: 5 LSBs slot number, 
			 * 3 MSBs mem_size, 1-64MB, 2-128MB, 3-256MB, 4-512MB
			 */
			size_slot = 0;
			size = slotp->pci_size >> 26;
			while(size) {
				size_slot++;
				size = size >> 1;
			}
			size_slot = size_slot << 5;
			size_slot |= (i & 0x1F);
			
			if ( (pciconf_readb(pci_id, PCI_CONF_ILINE) & 0xE0) == 0xE0) {
				/* Already Initialized, dont Initialize again */
				continue;
			}
			
			/* Write the Base address register for Galileo */
			if ( slotp->card_type != NODE_HBC)
				pciconf_writel(pci_id, PCI_CONF_BASE0, slotp->pci_start);

			/* Write the Slot info and size of PCI Mem */
			pciconf_writeb(pci_id, PCI_CONF_ILINE, size_slot);
			
			/* Write the type of the board in Cache Line Size Register */
			if ( slotp->card_type != NODE_HBC)
				pciconf_writeb(pci_id, PCI_CONF_CLS, slotp->card_type & 0x0F);
		}
	}
	printf("Done\n\r");
}

void
init_memmap()
{
	int size, ch;
	int	user_input = 0;
	int board_type;
	
	
	printf("Waiting for HBC to Init the Bridge...");
	
	while(!(board_type = get_boardtype())) {
		NU_Sleep(10);
		if (ttyA_poll() == 0)  {
			ch = ttyA_in();
			/* Read the Slot Number */
			user_input = 1;
			break;
		}
	}
	if ( user_input ) {
		memmaps.iCabinet = 0;
		printf("\n\rMake Sure HBC is Up!!!!\n\r");
get_input:
		printf("2 - A1, 3 - A2, 4 - A3, 5 - A4, 6 - B1, 7 - B2, 8 - B3, 9 - B4\n\ra - C1, b - C2, c - C3, d - C4, e - D1, f - D2, g - D3, h - D4\n\r"); 
		while ( ttyA_poll()) ;
		ch = ttyA_in();
		if ( (ch >= 2) || (ch <= 9)) {
			ch =  ch - '0';
		} else if ( (ch >= 'a') && (ch <= 'h')) {
			ch = ch - 'a' + 10;
		} else {
			printf("Invalid Entry\n\r");
			goto get_input; 
		}
		memmaps.iSlot 	 = real_slot_num[ch];
		memmaps.cbSlave  = M(64);
		memmaps.pciSlave = memmaps.aPaPci[memmaps.iSlot]; 
			
	} else {
		memmaps.iCabinet = 0;
		ch = (int)(pciconf_readb(0x80000000, PCI_CONF_ILINE) & 0x1F);
		memmaps.iSlot 	 = real_slot_num[ch]; 
		memmaps.pciSlave = pciconf_readl(0x80000000, PCI_CONF_BASE0) 
									& PCI_BASE_MEMADDR_M;
		size = pciconf_readb(0x80000000, PCI_CONF_ILINE);
		pciconf_writeb(0x80000000, PCI_CONF_ILINE, size | 0xE0);
		size = size >> 5;
		memmaps.cbSlave 	= 1;
		memmaps.cbSlave 	<<= 25;
		while(size--) {
			memmaps.cbSlave 	<<= 1;
		}
	}

	
	get_membank_config();
	/* Initialize the Memrory Sizes */
	memmaps.nFragment	= 2;
	memmaps.aPa[0]		= membanks.bank10_start;
	memmaps.aCb[0]		= membanks.bank10_size;
	memmaps.aPa[1]		= membanks.bank32_start;
	memmaps.aCb[1]		= membanks.bank32_size;

	memmaps.aP[0]		= PTOV(memmaps.aPa[0]);
	memmaps.aP[1]		= PTOV1(memmaps.aPa[1]);
	memmaps.paSlave		= REMAP_ADDRESS;

	printf("\n\rPCI Mem Start %08X, Size %08X, Remap %08X\n\r", memmaps.pciSlave, 
								memmaps.cbSlave, memmaps.paSlave);
		
}
void
init_galileo_bars()
{
	/* Configure the BAR for RAS10 */
	pciconf_writel(0x80000000, PCI_CONF_BASE0, 
					memmaps.pciSlave | PCI_BASE_TYPE_PREF);
	/* Program the PCI remap register for RAS10, this has to be done after
	 * programming the BAR for RAS10
	 */
	gt_write(GT_PCI_RAS10_MAP_OFF, memmaps.paSlave);

	if ( memmaps.cbSlave > memmaps.aCb[0] ) {
		/* Program RAS size registers */
		gt_write(GT_PCI_RAS10_BSIZE_OFF, memmaps.aCb[0] - 1);

		/* Configure the BAR for RAS32 */
		pciconf_writel(0x80000000, PCI_CONF_BASE1, 
					(memmaps.pciSlave + memmaps.aCb[0]) | PCI_BASE_TYPE_PREF);

		/* Program the PCI remap register for RAS32, this has to be done after
	 	 * programming the BAR for RAS32
	 	 */
		gt_write(GT_PCI_RAS32_MAP_OFF, memmaps.aPa[1]);

		/* Program RAS size registers */
		gt_write(GT_PCI_RAS32_BSIZE_OFF, (memmaps.cbSlave-memmaps.aCb[0]) - 1);
	} else {
		/* Program RAS size registers */
		gt_write(GT_PCI_RAS10_BSIZE_OFF, memmaps.cbSlave - 1);

		/* Program RAS32 size to be 0 */
		gt_write(GT_PCI_RAS32_BSIZE_OFF, 0);
	}

}


/* Given the Virtual address, returns the Physical PCI address in the
 * PCI Space
 */
U32
addr_vtopci(U32	addr)
{
	U32	paSlave;
	/* Get the Local Physical Address */
	addr &= 0x1FFFFFFF;
	/* Get the Slave address from the Remap Register
	 * This is same as memmaps.paSlave, I could have got it from
	 * memmaps.paSlave. Instead I am reading from Galileo Register
	 */
	paSlave = gt_read(GT_PCI_RAS10_MAP_OFF);
	if ( addr < paSlave ) {
		printf("addr_vtopci(): the addr is not in PCI Window\n\r");
		return(0xFFFFFFFF);
	}
	
	/* Get the offset in the PCI Window */
	addr -= paSlave;
	return(memmaps.pciSlave + addr);
}
/* Given the Physiocal PCI address, returns the virtual Address
 */
U32
addr_pcitov(U32	addr, U32 seg_addr)
{
	U32	paSlave;

	if ( addr < memmaps.pciSlave ) {
		printf("addr_pcitov(): the addr is not in PCI Space\n\r");
		return(0xFFFFFFFF);
	}
	/* Get the offset in the PCI Window */
	addr -= memmaps.pciSlave;
	/* Get the Slave address from the Remap Register
	 * This is same as memmaps.paSlave, I could have got it from
	 * memmaps.paSlave. Instead I am reading from Galileo Register
	 */
	paSlave = gt_read(GT_PCI_RAS10_MAP_OFF);
	/* Get the Local Physical Address */
	addr += paSlave;
	
	/* Convert it to Virtual Address */
	return(seg_addr | addr);
}
#ifdef CONFIG_BOOT
void
init_memmap_iop()
{
	extern U32	boardSlot;	
	extern U32	pci_window_size;	
	extern U32	pci_window_address;	
	extern U32	slave_window_address;	

	get_membank_config();
	memmaps.iCabinet = 0;
	memmaps.iSlot 	 = boardSlot;		/* Slot Number */
	memmaps.cbSlave  = pci_window_size;	/* Size */
	memmaps.pciSlave = pci_window_address;	/* PCI Address */
	memmaps.paSlave	 = slave_window_address;	/* Local Reamp Address */
	
	/* Initialize the Memrory Sizes */
	memmaps.nFragment	= 2;
	memmaps.aPa[0]		= membanks.bank10_start;
	memmaps.aCb[0]		= membanks.bank10_size;
	memmaps.aPa[1]		= membanks.bank32_start;
	memmaps.aCb[1]		= membanks.bank32_size;

	memmaps.aP[0]		= PTOV(memmaps.aPa[0]);
	memmaps.aP[1]		= PTOV1(memmaps.aPa[1]);

}
#endif


void
create_pcislot()
{
	int i, slot;
	pcislot_t *slotp;
	
	bzero(pcislot, sizeof(pcislot_t));
	for(i=0; i < MAX_PCI_SLOTS; i++) {
		slotp = &pcislot[i];
		slot  = real_slot_num[i];
		slotp->card_type = memmaps.aBoardType[slot];
		if (   (slotp->card_type == BOARD_RAC)
			|| (slotp->card_type == BOARD_NIC)
			|| (slotp->card_type == BOARD_NIC_RAC) )
			slotp->card_type |= IS_BRIDGE;
		slotp->pci_id    = memmaps.aIdPci[slot];
		slotp->pci_start = memmaps.aPaPci[slot];
		slotp->pci_size  = M(64);
		if ( i >= 2) {
			slotp->slotname[0] = 'A' + (char)((i - 2) / 4);
			slotp->slotname[1] = '1' + (char)((i - 2)% 4);
		} else {
			slotp->slotname[0] = 'H';
			slotp->slotname[1] = 'B';
			slotp->slotname[2] = 'C';
		}
	}
}

/* Bridge Configure Functions */
#ifdef	CONFIG_E2


/* Reset and Initialize Bridge F and the Subtree which
 * includes H, A, B, C, D and bridges on IOP's
 * Returns : STATUS
 */

STATUS
InitBridgeFTree()
{
	U32		pci_id;
	U32		mbase, mlmt;
	U32		pbus, sbus, devven_id;
	int 	i;

	/* Reset the Bridge F and Subordinate Bridges */
	*((U8 *)(PCI_RESET_ADDR_FH)) = PCI_RESET_F;
	delay_ms(10);
	*((U8 *)(PCI_RESET_ADDR_FH)) = PCI_RESET_DEASSERT;
	
	/* Initialize the Bridge F */
	printf("Initializing Bridge F ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_F_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(1024);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_G_BUSNUM, PCI_F_BUSNUM, PCI_F_SUBNUM, mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge H */
	printf("Initializing Bridge H ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_H_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(512);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_H_BUSNUM, PCI_H_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge C */
	printf("Initializing Bridge C ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_C_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(768);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_C_BUSNUM, PCI_C_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge D */
	printf("Initializing Bridge D ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 2);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_D_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(1024);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_D_BUSNUM, PCI_D_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge A */
	printf("Initializing Bridge A ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_A_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_H_BUSNUM, PCI_A_BUSNUM, PCI_A_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge B */
	printf("Initializing Bridge B ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_B_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(512);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_H_BUSNUM, PCI_B_BUSNUM, PCI_B_SUBNUM,
												mbase, mlmt);
	printf("OK\n\r");
	
	/* Init the Bridge on the IOP */
	for(i=2; i < MAX_SLOTS; i++) {
		if ( memmaps.aIdPci[i]) {
			pci_id = memmaps.aIdPci[i];
			devven_id = pciconf_readl(pci_id, 0);
			if ( devven_id == BRIDGE_21154_ID) {
				/* Found a Bridge, may be a NIC/RAC/NAC, init the Bridge */
				mbase = memmaps.aPaPci[i];
				mlmt  = mbase - 1 + M(64);
				/* Get the Primary Bus Number */
				pbus  = PCI_GET_BUS(pci_id);
				/* Get the Secondary & Subordinate Bus Number, both are same */
				sbus  = pbus + PCI_GET_DEV(pci_id) - 11;
				/* Init the Bridge */
				bridge_init(pci_id, pbus, sbus, sbus, mbase, mlmt);
			}
		}
	}
	return(OK);
}

/* Reset and Initialize Bridge H and the Subtree which
 * includes A, B and bridges on IOP's on Segment A and B
 * Returns : STATUS
 */
STATUS
InitBridgeHTree()
{
	U32		pci_id;
	U32		mbase, mlmt;
	U32		pbus, sbus, devven_id;
	int 	i;

	/* Reset the Bridge F and Subordinate Bridges */
	*((U8 *)(PCI_RESET_ADDR_FH)) = PCI_RESET_H;
	delay_ms(10);
	*((U8 *)(PCI_RESET_ADDR_FH)) = PCI_RESET_DEASSERT;
	

	/* Initialize the Bridge H */
	printf("Initializing Bridge H ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_H_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(512);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_H_BUSNUM, PCI_H_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge A */
	printf("Initializing Bridge A ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_A_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_H_BUSNUM, PCI_A_BUSNUM, PCI_A_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge B */
	printf("Initializing Bridge B ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_B_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(512);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_H_BUSNUM, PCI_B_BUSNUM, PCI_B_SUBNUM,
												mbase, mlmt);
	printf("OK\n\r");
	
	/* Init the Bridge on the IOP on Segment A */
	for(i=24; i < 28; i++) {
		if ( memmaps.aIdPci[i]) {
			pci_id = memmaps.aIdPci[i];
			devven_id = pciconf_readl(pci_id, 0);
			if ( devven_id == BRIDGE_21154_ID) {
				/* Found a Bridge, may be a NIC/RAC/NAC, init the Bridge */
				mbase = memmaps.aPaPci[i];
				mlmt  = mbase - 1 + M(64);
				/* Get the Primary Bus Number */
				pbus  = PCI_GET_BUS(pci_id);
				/* Get the Secondary & Subordinate Bus Number, both are same */
				sbus  = pbus + PCI_GET_DEV(pci_id) - 11;
				/* Init the Bridge */
				bridge_init(pci_id, pbus, sbus, sbus, mbase, mlmt);
			}
		}
	}
	/* Init the Bridge on the IOP on Segment B */
	for(i=16; i < 20; i++) {
		if ( memmaps.aIdPci[i]) {
			pci_id = memmaps.aIdPci[i];
			devven_id = pciconf_readl(pci_id, 0);
			if ( devven_id == BRIDGE_21154_ID) {
				/* Found a Bridge, may be a NIC/RAC/NAC, init the Bridge */
				mbase = memmaps.aPaPci[i];
				mlmt  = mbase - 1 + M(64);
				/* Get the Primary Bus Number */
				pbus  = PCI_GET_BUS(pci_id);
				/* Get the Secondary & Subordinate Bus Number, both are same */
				sbus  = pbus + PCI_GET_DEV(pci_id) - 11;
				/* Init the Bridge */
				bridge_init(pci_id, pbus, sbus, sbus, mbase, mlmt);
			}
		}
	}
	return(OK);
}

/* Reset and Initialize Bridge A and the Subtree which
 * includes bridges on IOP's on Segment A 
 * Returns : STATUS
 */
STATUS
InitBridgeATree()
{
	U32		pci_id;
	U32		mbase, mlmt;
	U32		pbus, sbus, devven_id;
	int 	i;

	/* Reset the Bridge F and Subordinate Bridges */
	*((U8 *)(PCI_RESET_ADDR_ABCD)) = PCI_RESET_A;
	delay_ms(10);
	*((U8 *)(PCI_RESET_ADDR_ABCD)) = PCI_RESET_DEASSERT;
	

	/* Initialize the Bridge A */
	printf("Initializing Bridge A ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_A_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_H_BUSNUM, PCI_A_BUSNUM, PCI_A_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Init the Bridge on the IOP on Segment A */
	for(i=24; i < 28; i++) {
		if ( memmaps.aIdPci[i]) {
			pci_id = memmaps.aIdPci[i];
			devven_id = pciconf_readl(pci_id, 0);
			if ( devven_id == BRIDGE_21154_ID) {
				/* Found a Bridge, may be a NIC/RAC/NAC, init the Bridge */
				mbase = memmaps.aPaPci[i];
				mlmt  = mbase - 1 + M(64);
				/* Get the Primary Bus Number */
				pbus  = PCI_GET_BUS(pci_id);
				/* Get the Secondary & Subordinate Bus Number, both are same */
				sbus  = pbus + PCI_GET_DEV(pci_id) - 11;
				/* Init the Bridge */
				bridge_init(pci_id, pbus, sbus, sbus, mbase, mlmt);
			}
		}
	}
	return(OK);
}

/* Reset and Initialize Bridge B and the Subtree which
 * includes bridges on IOP's on Segment B 
 * Returns : STATUS
 */
STATUS
InitBridgeBTree()
{
	U32		pci_id;
	U32		mbase, mlmt;
	U32		pbus, sbus, devven_id;
	int 	i;

	/* Reset the Bridge F and Subordinate Bridges */
	*((U8 *)(PCI_RESET_ADDR_ABCD)) = PCI_RESET_B;
	delay_ms(10);
	*((U8 *)(PCI_RESET_ADDR_ABCD)) = PCI_RESET_DEASSERT;
	
	/* Initialize the Bridge B */
	printf("Initializing Bridge B ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_B_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(512);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_H_BUSNUM, PCI_B_BUSNUM, PCI_B_SUBNUM,
												mbase, mlmt);
	printf("OK\n\r");

	/* Init the Bridge on the IOP on Segment B */
	for(i=16; i < 20; i++) {
		if ( memmaps.aIdPci[i]) {
			pci_id = memmaps.aIdPci[i];
			devven_id = pciconf_readl(pci_id, 0);
			if ( devven_id == BRIDGE_21154_ID) {
				/* Found a Bridge, may be a NIC/RAC/NAC, init the Bridge */
				mbase = memmaps.aPaPci[i];
				mlmt  = mbase - 1 + M(64);
				/* Get the Primary Bus Number */
				pbus  = PCI_GET_BUS(pci_id);
				/* Get the Secondary & Subordinate Bus Number, both are same */
				sbus  = pbus + PCI_GET_DEV(pci_id) - 11;
				/* Init the Bridge */
				bridge_init(pci_id, pbus, sbus, sbus, mbase, mlmt);
			}
		}
	}
	return(OK);
}

/* Reset and Initialize Bridge C and the Subtree which
 * includes bridges on IOP's on Segment C 
 * Returns : STATUS
 */
STATUS
InitBridgeCTree()
{
	U32		pci_id;
	U32		mbase, mlmt;
	U32		pbus, sbus, devven_id;
	int 	i;

	/* Reset the Bridge F and Subordinate Bridges */
	*((U8 *)(PCI_RESET_ADDR_ABCD)) = PCI_RESET_C;
	delay_ms(10);
	*((U8 *)(PCI_RESET_ADDR_ABCD)) = PCI_RESET_DEASSERT;
	

	/* Initialize the Bridge C */
	printf("Initializing Bridge C ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_C_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(768);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_C_BUSNUM, PCI_C_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Init the Bridge on the IOP on Segment C */
	for(i=20; i < 24; i++) {
		if ( memmaps.aIdPci[i]) {
			pci_id = memmaps.aIdPci[i];
			devven_id = pciconf_readl(pci_id, 0);
			if ( devven_id == BRIDGE_21154_ID) {
				/* Found a Bridge, may be a NIC/RAC/NAC, init the Bridge */
				mbase = memmaps.aPaPci[i];
				mlmt  = mbase - 1 + M(64);
				/* Get the Primary Bus Number */
				pbus  = PCI_GET_BUS(pci_id);
				/* Get the Secondary & Subordinate Bus Number, both are same */
				sbus  = pbus + PCI_GET_DEV(pci_id) - 11;
				/* Init the Bridge */
				bridge_init(pci_id, pbus, sbus, sbus, mbase, mlmt);
			}
		}
	}
	return(OK);
}

/* Reset and Initialize Bridge D and the Subtree which
 * includes bridges on IOP's on Segment D 
 * Returns : STATUS
 */
STATUS
InitBridgeDTree()
{
	U32		pci_id;
	U32		mbase, mlmt;
	U32		pbus, sbus, devven_id;
	int 	i;

	/* Reset the Bridge F and Subordinate Bridges */
	*((U8 *)(PCI_RESET_ADDR_ABCD)) = PCI_RESET_D;
	delay_ms(10);
	*((U8 *)(PCI_RESET_ADDR_ABCD)) = PCI_RESET_DEASSERT;
	
	/* Initialize the Bridge D */
	printf("Initializing Bridge D ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 2);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_D_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(1024);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_D_BUSNUM, PCI_D_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");


	/* Init the Bridge on the IOP on Segment C */
	for(i=28; i < 32; i++) {
		if ( memmaps.aIdPci[i]) {
			pci_id = memmaps.aIdPci[i];
			devven_id = pciconf_readl(pci_id, 0);
			if ( devven_id == BRIDGE_21154_ID) {
				/* Found a Bridge, may be a NIC/RAC/NAC, init the Bridge */
				mbase = memmaps.aPaPci[i];
				mlmt  = mbase - 1 + M(64);
				/* Get the Primary Bus Number */
				pbus  = PCI_GET_BUS(pci_id);
				/* Get the Secondary & Subordinate Bus Number, both are same */
				sbus  = pbus + PCI_GET_DEV(pci_id) - 11;
				/* Init the Bridge */
				bridge_init(pci_id, pbus, sbus, sbus, mbase, mlmt);
			}
		}
	}
	return(OK);
}

/* Initialize Bridge E and Devices on Bus E
 * Returns : STATUS
 */
STATUS
InitBridgeETree()
{
	U32		pci_id;
	U32		mbase, mlmt;
	extern	STATUS	init_local_devs();
	

	/* Initialize the Bridge E */
	printf("Initializing Bridge E ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_E_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(64);
	mlmt  = mbase - 1 + M(64);
	/* Init the Bridge */
	bridge_init(pci_id, PCI_G_BUSNUM, PCI_E_BUSNUM, PCI_E_SUBNUM, mbase, mlmt); 
	printf("OK\n\r");

	/* Make Bridge E non-pre-fetchable */
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 0);
	
	/* Read the Bridge E Mem abse and Mem Limit */
	mbase = pciconf_readw(pci_id, PCI_BCONF_PMEMBASE) & 0xFFF0;
	mlmt  = pciconf_readw(pci_id, PCI_BCONF_PMEMLIMIT) | 0xF;
	
	/* Now Disable the Pref Memory */
	pciconf_writew(pci_id, PCI_BCONF_PMEMBASE, (0xFFFFFFFF >> PCI_BMEM_SHIFT));
	pciconf_writew(pci_id, PCI_BCONF_PMEMLIMIT, (0 >> PCI_BMEM_SHIFT));
	
	/* Now Enable the Non-Pref Memory Window */
	pciconf_writew(pci_id, PCI_BCONF_MEMBASE, mbase );
	pciconf_writew(pci_id, PCI_BCONF_MEMLIMIT, mlmt );
	
	/* Configure the local bridge for not support fast back-to-back
	 * on the secondary Bus */
	pciconf_writew(pci_id, PCI_BCONF_CNTL, 0);
	
	/*Initialize the Local PCI Bus Devices*/
	return(init_local_devs());
}

/* ------------- Debugging Stuff  ------------------------------------------*/
void
bridge_print(U32 pci_id, U32 primbus, U32 secbus, U32 subbus,
			U32 pmembase, U32 pmemlmt)
{
	printf("PStatus %04X ", pciconf_readw(pci_id, PCI_CONF_STAT)); 
	printf("SStatus %04X ", pciconf_readw(pci_id, PCI_BCONF_SECSTAT)); 
	printf("\n\r");
	pciconf_print(pci_id);
		
#pragma unused (primbus)
#pragma unused (secbus)
#pragma unused (subbus)
#pragma unused (pmembase)
#pragma unused (pmemlmt)
		
}
STATUS
printBridgeFTree()
{
	U32		pci_id;
	U32		mbase, mlmt;
	U32		pbus, sbus, devven_id;
	int 	i;

	/* Initialize the Bridge F */
	printf("Status Bridge F ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_F_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(1024);
	/* Init the Bridge */
	bridge_print(pci_id, PCI_G_BUSNUM, PCI_F_BUSNUM, PCI_F_SUBNUM, mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge H */
	printf("Status Bridge H ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_H_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(512);
	/* Init the Bridge */
	bridge_print(pci_id, PCI_F_BUSNUM, PCI_H_BUSNUM, PCI_H_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge C */
	printf("Status Bridge C ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_C_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(768);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_print(pci_id, PCI_F_BUSNUM, PCI_C_BUSNUM, PCI_C_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge D */
	printf("Status Bridge D ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 2);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_D_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(1024);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_print(pci_id, PCI_F_BUSNUM, PCI_D_BUSNUM, PCI_D_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge A */
	printf("Status Bridge A ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_A_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_print(pci_id, PCI_H_BUSNUM, PCI_A_BUSNUM, PCI_A_SUBNUM, 
											mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge B */
	printf("Status Bridge B ....");
	/* Probe the Bridge */
	pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		/* Did not find the Bridge */
		printf("Fail\n\r");
		return(BRIDGE_B_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(512);
	mlmt  = mbase - 1 + M(256);
	/* Init the Bridge */
	bridge_print(pci_id, PCI_H_BUSNUM, PCI_B_BUSNUM, PCI_B_SUBNUM,
												mbase, mlmt);
	printf("OK\n\r");
	
	/* Init the Bridge on the IOP */
	for(i=2; i < MAX_SLOTS; i++) {
		if ( memmaps.aIdPci[i]) {
			pci_id = memmaps.aIdPci[i];
			devven_id = pciconf_readl(pci_id, 0);
			if ( devven_id == BRIDGE_21154_ID) {
				/* Found a Bridge, may be a NIC/RAC/NAC, init the Bridge */
				printf("Status Bridge %s ....", slotname[i]);
				mbase = memmaps.aPaPci[i];
				mlmt  = mbase - 1 + M(64);
				/* Get the Primary Bus Number */
				pbus  = PCI_GET_BUS(pci_id);
				/* Get the Secondary & Subordinate Bus Number, both are same */
				sbus  = pbus + PCI_GET_DEV(pci_id) - 11;
				/* Init the Bridge */
				bridge_print(pci_id, pbus, sbus, sbus, mbase, mlmt);
				/*
				probe_devs(sbus, 0);
				*/
				printf("OK\n\r");
			}
		}
	}
	return(OK);
}
#endif
