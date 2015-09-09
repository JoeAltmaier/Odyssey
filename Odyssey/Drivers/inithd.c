/*
 *
 * MODULE: inithd.c - Hardware Initialization Module 
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 09/29/98	- Created by Sudhir
 * 04/12/99 - Added the slot specific info initalization to hbc_init() 
 * 07/29/99 - Joe Altmaier: memcpy entire memmap.
 * 08/24/99 - Joe Altmaier: Init_Hardware work only once.
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

#define	PCI_STATIC	1
#include "nucleus.h"
#include "types.h"
#include "pcidev.h"
#include "pcimap.h"
#include "galileo.h"
#include "ispdev.h"
#include "sysflash.h"
#include "gt.h"
#include "pcialloc.h"
#include "bootblock.h"

//#define	CMB_HACK	1


#define DEBUG	1
#ifdef	CONFIG_E2
#define	BUS0_BRIDGE_COUNT	2
#else
#define	BUS0_BRIDGE_COUNT	5
#endif
int	not_an_hbc = 0;
bootblock_t bootblock;
unsigned long long bootblockp = 0xdeaddeaddeaddead;

STATUS init_hbc();
STATUS init_ssd();
STATUS init_nic();
STATUS init_local_devs();
void copy_pcislot_to_pcispace();
void copy_pcispace_to_pcislot();
void init_pcimap();
void init_slot_specific();
void init_all_static();

int	real_slot_num [MAX_PCI_SLOTS] = {0, 1, 
									25, 26, 27, 24,
									16, 17, 18, 19,
									29, 30, 31, 28,
									20, 21, 22, 23
									};

U32
get_slotno()
{
#ifdef	INCLUDE_ODYSSEY
	U32 slot_no;
	int i;
	for(i=0; i < MAX_PCI_SLOTS; i++) {
		if ( real_slot_num[i] == memmaps.iSlot) {
			slot_no = i;
			break;
		}
	}
	if ( i == MAX_PCI_SLOTS )
		slot_no = 0;
	return(slot_no);
#else
	return(0);
#endif
		
}



unsigned long long *hbc_ctl	= (unsigned long long *)0xBF000000;

U32
get_boardtype()
{
#ifdef	INCLUDE_ODYSSEY
	U32 slot_no;
	slot_no = (U32)pciconf_readb(0x80000000, PCI_CONF_CLS); 
	return(slot_no);
#else
	return(BOARD_NIC);
#endif
		
}


#ifdef	CONFIG_BOOT
STATUS
Init_Hardware()
{
	I32	hbc_flag =1;
	U32	board_type;
	STATUS	rc = OK;
	U16	int_mask = 0;
	int	i;
	U32	pci_id;
	int cleared = 0;
	
#ifdef	DEBUG
	printf("Initializing the Hardware\n\r");
#endif
#ifdef	INCLUDE_ODYSSEY
	/* Initalize the TLB */
#ifndef	CONFIG_BOOT
	int_mask = NU_Control_Interrupts(0x0000);
	tlb_init();
	NU_Control_Interrupts(int_mask);
#endif
#endif	INCLUDE_ODYSSEY

	/* Hack to Overcome The CMB Protocol
	 * Since we dont have one yet
	 */
	pciconf_writew(0x80000000, PCI_CONF_COMM,
		PCI_COMM_IO | PCI_COMM_MEM | PCI_COMM_MASTER | PCI_COMM_B2B);

	for(i=0; i < BUS0_BRIDGE_COUNT; i++) {
		pci_id = get_pci_id(0, BRIDGE_21154_ID, i);
		if ( pci_id == 0)
			break;
	}
	if ( pci_id ) {
		/* HBC */
		/* This Board is a HBC, Initialize  */
		board_type = BOARD_HBC;
		rc = init_hbc();
		if ( rc != OK )
			goto Init_Hardware_end;

#if	0
		/* Initialize the IOPs */
		for(i=2; i < 32; i++) {
			if ( memmaps.aIdPci[i]) {
				init_iop(i);
			}
		}
#endif
#ifdef	CMB_HACK
		create_pcislot();
		copy_pcislot_to_pcispace();	
		/* Initialize the Galileo on the other boards*/
		init_pci_galileo();
#endif
	} else {
		/* Non-HBC */

		/* Get the board specific stuff */
#ifdef	CMB_HACK
		init_memmap();
#else
		init_memmap_iop();
#endif
		/* Initilize the Galileo */
		init_galileo();
		
		/* Initalize the Galileo BARS */
		init_galileo_bars();
		
		/* init the PCI specific entire Mem Map  */
		init_pcimap();

		board_type = memmaps.aBoardType[memmaps.iSlot];
		switch(board_type) {
		case BOARD_NIC:
		case BOARD_RAC:
		case BOARD_NIC_RAC:
			rc = init_nic();
			if ( rc != OK )
				goto Init_Hardware_end;
			break;
		case BOARD_SSD:
			rc = init_ssd();
			if ( rc != OK )
				goto Init_Hardware_end;
			break;
		
		default:
			break;
		}

	}

Init_Hardware_end:
	/* Inititlaize Galileo for Interrupts */
	return(rc);
}
#else
char fInitialized=0;

Init_Hardware()
{
	U16	int_mask = 0;

	if (fInitialized)
		return(OK);
	
	fInitialized=1;
	
	/* Init the Cache */
	mips_init_cache();
	
	/* Init the TLB */
	int_mask = NU_Control_Interrupts(0x0000);
	tlb_init();
	NU_Control_Interrupts(int_mask);

	/* init the PCI specific entire Mem Map  */
	init_pcimap();
	return(OK);
}
#endif

STATUS
init_iop(U32 slot_no)
{
	U32 pci_id;
	U32 devven_id;
	U32 mbase, mlmt, pbus, sbus;
	
	if ( slot_no < 0 || slot_no >= MAX_SLOTS ) {
		printf("Invalid Slot Number\n\r");
		return(OK);
	}
	/* probe the slot to see whether board is present */
	pci_id = memmaps.aIdPci[slot_no];
	printf("Slot %s(%d): ", slotname[slot_no], slot_no);
	
	/* Get the Device and Vendor Id */
	devven_id = pciconf_readl(pci_id, 0);
	
	
	if ( devven_id == BRIDGE_21154_ID) {
		/* Found a Bridge, may be a NIC/RAC/NAC, init the Bridge */
		mbase = memmaps.aPaPci[slot_no];
		mlmt  = mbase - 1 + M(64);
		/* Get the Primary Bus Number */
		pbus  = PCI_GET_BUS(pci_id);
		/* Get the Secondary and Subordinate Bus Number, both are same */
		sbus  = pbus + PCI_GET_DEV(pci_id) - 11;
		bridge_init(pci_id, pbus, sbus, sbus, mbase, mlmt);
		memmaps.aBoardType[slot_no] = BOARD_NAC;
		printf("NAC, Id %08X, Pci mem %08X\n\r", memmaps.aIdPci[slot_no],
												memmaps.aPaPci[slot_no]);
#ifdef	CMB_HACK
		pci_id = get_pci_id(sbus, GAL120_ID, 0);
		if ( pci_id ) {
			unsigned char size_slot, size;
			int i;

			size_slot = 1;
			size_slot = size_slot << 5;
			for(i=0; i < MAX_SLOTS; i++) {
				if ( slot_no == real_slot_num[i])
					break;
			}
			if ( i >= MAX_SLOTS)
				return(OK);
			size_slot |= (i & 0x1F);
			if ( (pciconf_readb(pci_id, PCI_CONF_ILINE) & 0xE0) == 0xE0)
				/* Already Initialized, dont Initialize again */
				return(OK);
			/* Write the Base address register for Galileo */
			pciconf_writel(pci_id, PCI_CONF_BASE0, memmaps.aPaPci[slot_no]);
			/* Write the Slot info and size of PCI Mem */
			pciconf_writeb(pci_id, PCI_CONF_ILINE, size_slot);
			/* Write the type of the board in Cache Line Size Register */
			pciconf_writeb(pci_id, PCI_CONF_CLS,  memmaps.aBoardType[slot_no]& 0x0F);
			
		}
#endif
		return(OK);
	}
	if ( devven_id == GAL120_ID) {
		/* Found galileo, Must be a SSD */
		memmaps.aBoardType[slot_no] = BOARD_SSD;
		printf("SSD, Id %08X, Pci mem %08X\n\r", memmaps.aIdPci[slot_no],
												memmaps.aPaPci[slot_no]);
#ifdef	CMB_HACK
		pci_id = memmaps.aIdPci[slot_no];
		if ( pci_id ) {
			unsigned char size_slot, size;
			int i;
			
			size_slot = 1;
			size_slot = size_slot << 5;
			for(i=0; i < MAX_SLOTS; i++) {
				if ( slot_no == real_slot_num[i])
					break;
			}
			if ( i >= MAX_SLOTS)
				return(OK);
			size_slot |= (i & 0x1F);
			if ( (pciconf_readb(pci_id, PCI_CONF_ILINE) & 0xE0) == 0xE0) 
				/* Already Initialized, dont Initialize again */
				return(OK);
			/* Write the Base address register for Galileo */
			pciconf_writel(pci_id, PCI_CONF_BASE0, memmaps.aPaPci[slot_no]);
			/* Write the Slot info and size of PCI Mem */
			pciconf_writeb(pci_id, PCI_CONF_ILINE, size_slot);
			/* Write the type of the board in Cache Line Size Register */
			pciconf_writeb(pci_id, PCI_CONF_CLS,  memmaps.aBoardType[slot_no]& 0x0F);
			
		}
#endif
		return(OK);
	}
	printf("Empty\n\r");
	return(BOARD_ACCESS_ERROR);
}
#ifdef	CONFIG_BOOT
extern	U32	is_master;
STATUS
init_hbc()
{
	STATUS	rc;
#ifdef	DEBUG
	printf("Initializing the HBC\n\r");
#endif
	/* Turn on the Quick-Switches if master*/
	if ( is_master) {
		*((U8 *)(0xBC0F8000)) = 0;
		delay_ms(1);
		/* Reset the PCI system */
		*((U8 *)(0xBC0D8000)) = 0x0B;
		delay_ms(1);
		*((U8 *)(0xBC0D8000)) = 0x0F;

	} else {
		*((U8 *)(0xBC0F8000)) = 0x1F;
	}

	/* Probe Galileo for PCI_0 */
	if (pciconf_readl(0x80000000, PCI_CONF_VENID) != GAL120_ID ) {
					/* Could not find Galileo PCI_0 */
		return(GALILEO_0_ACCESS_ERROR);
	}
#ifdef	DEBUG
	printf("Galileo PCI_0 .........OK\n\r");
#endif
	
	/* Initialize the Galileo */
	init_galileo();
	
	/* Intialize the On-board Bridges */
	rc = init_onboard_bridges();
	if(rc != OK)
		return(rc);
	
	
	/* create the Memory Map Data Structure */
	create_memmap();
	
	/* Initialize the Galiloe BARS */
	init_galileo_bars();
	
	/*Initialize the Local PCI Bus Devices*/
	init_local_devs();
	
	return(OK);
}
#endif


#ifdef	CONFIG_E2
STATUS
init_slave_bridges()
{
	
	U32 pci_id, mbase, mlmt;

	*((U8 *)(0xBC0F8000)) = 0;
	
	/* Initialize the Bridge H */
	printf("Initializing Bridge H ....");
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_H_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(512);
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_H_BUSNUM, PCI_H_SUBNUM, mbase, mlmt);
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
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_C_BUSNUM, PCI_C_SUBNUM, mbase, mlmt);
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
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_D_BUSNUM, PCI_D_SUBNUM, mbase, mlmt);
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
	bridge_init(pci_id, PCI_H_BUSNUM, PCI_A_BUSNUM, PCI_A_SUBNUM, mbase, mlmt);
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
	bridge_init(pci_id, PCI_H_BUSNUM, PCI_B_BUSNUM, PCI_B_SUBNUM, mbase, mlmt);
	printf("OK\n\r");

	return(OK);
}
#endif



STATUS
init_nic()
{
	U32	pci_id;
	U32	NacE1 = 0;

#ifdef	DEBUG
	printf("Initializing the NAC\n\r");
#endif
	init_galileo();
#ifdef	CONFIG_BOOT
	init_memmap_iop();
#else
	init_memmap();
#endif
	init_galileo_bars();
#ifdef	CONFIG_E2
	/* MAC 0 */
	NacE1 = IsNacE1();
	if ( NacE1 )
		pci_id = PADDR(0, 7, 0);
	else
		pci_id = PADDR(0, 2, 0);
	if ( pciconf_readl(pci_id, 0) != ISP2_ID) {
		printf("FC_MAC 0... Fail\n\r");
		pci_id = 0;
	} else {
		printf("FC_MAC 0... OK\n\r");
	}

	if ( pci_id) {
		/* Configure Q-logic as Bus Master and IO Access  */
		pciconf_writew(pci_id, PCI_CONF_COMM, PCI_COMM_IO | PCI_COMM_MASTER
																	| 0x10);
		/* Set up the Base register */
		pciconf_writel(pci_id, PCI_CONF_BASE0, ISP_BASE);
		/* Write the Int Number */
		pciconf_writeb(pci_id, PCI_CONF_ILINE, 9);
	}
	if ( NacE1) {
		/* MAC 1 */
		pci_id = PADDR(0, 8, 0);
		if ( pciconf_readl(pci_id, 0) != ISP2_ID) {
			printf("FC_MAC 1... Fail\n\r");
			pci_id = 0;
		} else {
			printf("FC_MAC 1... OK\n\r");
		}

		if ( pci_id ) {
			/* Configure Q-logic as Bus Master and IO Access  */
			pciconf_writew(pci_id, PCI_CONF_COMM, PCI_COMM_IO | PCI_COMM_MASTER 
																| 0x10);
			/* Set up the Base register */
			pciconf_writel(pci_id, PCI_CONF_BASE0, ISP_BASE+ K(4));
			/* Write the Int Number */
			pciconf_writeb(pci_id, PCI_CONF_ILINE, 8);
		}
	}

	/* MAC 2 */
	if ( NacE1 )
		pci_id = PADDR(0, 9, 0);
	else
		pci_id = PADDR(0, 4, 0);
	if ( pciconf_readl(pci_id, 0) != ISP2_ID) {
		printf("FC_MAC 2... Fail\n\r");
		pci_id = 0;
	} else {
		printf("FC_MAC 2... OK\n\r");
	}

	if ( pci_id ) {
		/* Configure Q-logic as Bus Master and IO Access  */
		pciconf_writew(pci_id, PCI_CONF_COMM, PCI_COMM_IO | PCI_COMM_MASTER 
																| 0x10);
		/* Set up the Base register */
		pciconf_writel(pci_id, PCI_CONF_BASE0, ISP_BASE+ K(8));
		/* Write the Int Number */
		pciconf_writeb(pci_id, PCI_CONF_ILINE, 7);
	}

#else
	pci_id = get_pci_id(0, ISP_ID, 0);
	if ( pci_id ) {
		printf("FC_MAC 0... OK\n\r");
		/* Configure Q-logic as Bus Master and IO Access  */
		pciconf_writew(pci_id, PCI_CONF_COMM, PCI_COMM_IO | PCI_COMM_MASTER 
																| 0x10);
		/* Set up the Base register */
		pciconf_writel(pci_id, PCI_CONF_BASE0, ISP_BASE);
		/* Write the Int Number */
		pciconf_writeb(pci_id, PCI_CONF_ILINE, 2);
	} else {
		printf("FC_MAC 0... Fail\n\r");
	}
#endif

	return(OK);
}

STATUS
init_local_devs()
{
	U32	pci_id, ether0_int, ether1_int, scsi_int;
	U32	mbase;

#ifdef	CONFIG_E2
	/* Create the Bridge E pci id */
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 0);
	ether0_int = 7;
	ether1_int = 8;
	scsi_int   = 1;
#else
	/* Create the Bridge E pci id */
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 4);
	ether0_int = 6;
	/* Ethernet 1 does not exists on E1 */
	ether1_int = 0xFF;
	scsi_int   = 4;
#endif
	/* Read the Bridge E Mem base  */
	mbase = pciconf_readw(pci_id, PCI_BCONF_MEMBASE) & 0xFFF0;
	mbase = mbase << PCI_BMEM_SHIFT;
	

	/* Program SCSI */
	pci_id = get_pci_id(PCI_E_BUSNUM, SCSI_CNTL_ID, 0);
	if ( pci_id ) {
		printf("SCSI       ....OK\n\r");
		/* Configure SCSI as Bus Master and IO Access  */
		pciconf_writew(pci_id, PCI_CONF_COMM, PCI_COMM_MEM | PCI_COMM_MASTER);
		/* Set up the Base register */
		pciconf_writel(pci_id, PCI_CONF_BASE1, mbase);
		mbase += K(4);
		/* Write the Int Number */
		pciconf_writeb(pci_id, PCI_CONF_ILINE, scsi_int);
	} else {
		printf("SCSI       ....Fail\n\r");
		return(SCSI_ACCESS_ERROR);
	}

	/* Program Ethernet 0 */
	pci_id = get_pci_id(PCI_E_BUSNUM, ETHER_CNTL_ID, 0);
	if ( pci_id ) {
		printf("Ethernet 0 ....OK\n\r");
		/* Configure SCSI as Bus Master and IO Access  */
		pciconf_writew(pci_id, PCI_CONF_COMM, PCI_COMM_MEM | PCI_COMM_MASTER);
		/* Set up the Base register */
		pciconf_writel(pci_id, PCI_CONF_BASE1, mbase);
		mbase += K(4);
		/* Write the Int Number */
		pciconf_writeb(pci_id, PCI_CONF_ILINE, ether0_int);
	} else {
		printf("Ethernet 0 ....Fail\n\r");
		return(ETHERNET_ACCESS_ERROR);
	}

	/* Program Ethernet 1 */
	pci_id = get_pci_id(PCI_E_BUSNUM, ETHER_CNTL_ID, 1);
	if ( pci_id ) {
		printf("Ethernet 1 ....OK\n\r");
		/* Configure SCSI as Bus Master and IO Access  */
		pciconf_writew(pci_id, PCI_CONF_COMM, PCI_COMM_MEM | PCI_COMM_MASTER);
		/* Set up the Base register */
		pciconf_writel(pci_id, PCI_CONF_BASE1, mbase);
		mbase += K(4);
		/* Write the Int Number */
		pciconf_writeb(pci_id, PCI_CONF_ILINE, ether1_int);
	} 
	return(OK);
}



STATUS
test_isp(U32 pci_id, I32 pci_iface)
{
	I32	timeout;
	U16	mailbox_0;

	if ( pci_iface == 0 ) {
						/* PCI_0 */
		/* Program Q-logic as Master */
		pciconf_writew(pci_id, PCI_CONF_COMM, PCI_COMM_MEM | PCI_COMM_MASTER );
		pciconf_writel(pci_id, PCI_CONF_BASE1, ISP_BASE);
	} else {
						/* PCI_1 */
		/* Program Q-logic as Master */
		pciconf_1_writew(pci_id, PCI_CONF_COMM, PCI_COMM_MEM |PCI_COMM_MASTER );
		pciconf_1_writel(pci_id, PCI_CONF_BASE1, ISP_BASE);
	}
	
	/* The Q-logic's registers has to be accessed as 16-bit wide only*/

	/* Issue soft reset */
	Write_ISP(ISP_CONTROL_STATUS, 1);
	
	/* During initialization, we run with interrupts off */
	Write_ISP(ISP_PCI_INT_CONTROL, 0);
	
	/* Issue hard reset on RISC processor */
	Write_ISP(ISP_HCCR, HCTLRESETRISC);
	
	/* Release RISC from Reset */
	Write_ISP(ISP_HCCR, HCTLRLSRISC);
	
	/* Disable BIOS access  (Needed by Targets only?) */
	Write_ISP(ISP_HCCR, HCTLDISABLEBIOS);
	

	/* Wait for RISC to be Ready
	 * timeout = FCP_TIMEOUT;  */
	timeout = 10;  
	mailbox_0 = Read_ISP(ISP_MAILBOX_0);
	while(mailbox_0 != 0) {
		mailbox_0 = Read_ISP(ISP_MAILBOX_0);
		if (timeout-- == 0) {
#ifdef	DEBUG
			printf("ISP RISC NOT Ready\n\r");
#endif
			return(ISP_RISC_NOT_READY_ERROR);
		}		
	}
#ifdef	DEBUG
	printf("ISP  RISC Ready \n\r");
#endif
	
	/* Probe Mailbox */
	timeout = 10000;  
	while(Read_ISP(ISP_HCCR) & HCTLH2RINTR) {
		delay_ms(100);
		
		/* wait for ISP to clear host interrupt to RISC */
		if ( timeout-- == 0 ) {
#ifdef	DEBUG
			printf("ISP Mailbox Error\n\r");
#endif
			return(ISP_MAILBOX_ERROR);
		}		
			
	}
#ifdef	DEBUG
	printf("ISP Mailbox OK\n\r");
#endif

	return(OK);
}



STATUS
init_ssd()
{
#ifdef	DEBUG
	printf("Initializing the SSD\n\r");
#endif
	init_galileo();
#ifdef	CONFIG_BOOT
	init_memmap_iop();
#else
	init_memmap();
#endif
	init_galileo_bars();

	return(OK);
}

void
copy_pcislot_to_pcispace()	
{
	unsigned char   *ptr1, *ptr2;
	int size, i;
#if	0
	pcislot_t *slotp;
#endif
	
	size = sizeof(pcislot_t) * MAX_PCI_SLOTS;
	ptr1 = (unsigned char *)(memmaps.aP[0] + K(4));
	ptr2 = (unsigned char *)(pcislot);
	for(i=0; i < size; i++)
		ptr1[i] = ptr2[i];
	/*
#ifdef	PCI_STATIC
*/
#if	0
	slotp = (pcislot_t *)ptr1;
	for(i=2; i < MAX_PCI_SLOTS; i++) {
		if ( !slotp->pci_id) {
			slotp->pci_id = 0x8FFFFFFF;
			slotp->card_type = NODE_NIC;
		}
		slotp++;
	}
	
#endif
		
}

void
copy_pcispace_to_pcislot()
{
	unsigned char   *ptr1, *ptr2;
	int size, i;
	
	size = sizeof(pcislot_t) * MAX_PCI_SLOTS;
	/* Assumes that HBC is always mapped to pci-0x80000000, v- 0x00000000
	 * region */
	ptr1 = (unsigned char *)(0 + K(4));
	ptr2 = (unsigned char *)(pcislot);
	for(i=0; i < size; i++)
		ptr2[i] = ptr1[i];
		
}
/* Initializes all the bridges and pcislot[] to static values */
void
init_all_static()
{
	int i;
		
	pcislot_t *slotp;
	U32	pci_id;
	int	pbus, sbus;
	
	
	for(i=2; i < MAX_PCI_SLOTS; i++) {
		slotp = &pcislot[i];
		slotp->pci_start = PCI_WINDOW_START +  i * M(64);
		slotp->pci_size = M(64);
	}
	/* Initialize the bridges */
	pci_id = PADDR(0, BRIDGE_A_DEVNUM, 0);
	bridge_init(pci_id, 0, PCI_A_BUSNUM, PCI_A_SUBNUM, pcislot[2].pci_start, 
					(pcislot[2].pci_start - 1 + M(256)));
	pci_id = PADDR(0, BRIDGE_B_DEVNUM, 0);
	bridge_init(pci_id, 0, PCI_B_BUSNUM, PCI_B_SUBNUM, pcislot[6].pci_start, 
					(pcislot[6].pci_start - 1 + M(256)));
	pci_id = PADDR(0, BRIDGE_C_DEVNUM, 0);
	bridge_init(pci_id, 0, PCI_C_BUSNUM, PCI_C_SUBNUM, pcislot[10].pci_start, 
					(pcislot[10].pci_start - 1 + M(256)));
	pci_id = PADDR(0, BRIDGE_D_DEVNUM, 0);
	bridge_init(pci_id, 0, PCI_D_BUSNUM, PCI_D_SUBNUM, pcislot[14].pci_start, 
					(pcislot[14].pci_start - 1 + M(256)));
	pci_id = PADDR(0, BRIDGE_E_DEVNUM, 0);
	bridge_init(pci_id, 0, PCI_E_BUSNUM, PCI_E_SUBNUM, PCI_WINDOW_START +M(64), 
					((PCI_WINDOW_START +M(64)) - 1 + M(64)));
	pbus = PCI_A_BUSNUM;
	sbus = pbus + 1;
	for(i=2; i < MAX_PCI_SLOTS; i++) {
		slotp = &pcislot[i];
		if ( slotp->pci_id) {
			if ( (slotp->card_type == NODE_NIC) || 
											(slotp->card_type == NODE_RAC))
				bridge_init(slotp->pci_id, pbus, sbus, sbus, slotp->pci_start, 
							slotp->pci_start - 1 + M(64));
		}
			
		if ( (sbus == 5 ) || ( sbus == 10) || ( sbus == 15)) {
			pbus = sbus + 1;
			sbus = pbus + 1;
		} else {
			sbus++;
		}
	}
	printf("Static PCI Spaces:\n");
	for(i=0; i < MAX_PCI_SLOTS; i++) {
		slotp = &pcislot[i];
		if( slotp->pci_id ) {
			printf("Slot %s: Card %s, pci_id %08X, pstart %08X, psize %08X\n\r",
			slotp->slotname, bname[slotp->card_type & 0x0F],
				slotp->pci_id, slotp->pci_start, slotp->pci_size);
		}
	}
}

void
init_pcimap()
{
#ifdef	CMB_HACK
	int i, slot;
	pcislot_t *slotp;
	
	/* Get the PCI info from the HBC memory */
	copy_pcispace_to_pcislot();

	for(i=0; i < MAX_PCI_SLOTS; i++) {
		slotp = &pcislot[i];
		slot  = real_slot_num[i];
		memmaps.aBoardType[slot] = slotp->card_type & 0x0F;
		memmaps.aIdPci[slot] = slotp->pci_id;
		memmaps.aPaPci[slot] = slotp->pci_start;
	}
#else
#ifdef	CONFIG_BOOT
	extern int boardSlot;
	extern int boardType;
	memmaps.aBoardType[boardSlot] = boardType;
#else
	/* Init the memmap from bootblock here */
#if 0
	int i;
#endif
	mmap_t  *bmap = &(bootblock.b_memmap);
//	bootblockp &= 0xFFFFFFFF;
	bootblockp |= 0xFFFFFFFF00000000;
	memcpy(&bootblock, bootblockp, sizeof(bootblock_t));
	memcpy(&memmaps, bmap, sizeof(memmaps));
#endif
#endif
#if 0
	for(i=0; i < MAX_SLOTS; i++) {
		if ( memmaps.aBoardType[i]) {;
			printf("%s : Pci Mem %08X, id %08X\n\r", 
				bname[memmaps.aBoardType[i]], memmaps.aPaPci[i], 
				memmaps.aIdPci[i] ); 
		}
	}
	printf("This Board's Info:\n\r");
	printf("Slot %s(%d): PCI Mem %08X, Size %08X, Remap %08X\n\r", 
			slotname[memmaps.iSlot], memmaps.iSlot, memmaps.pciSlave,
		    memmaps.cbSlave, memmaps.paSlave); 	
#endif

}


void
init_slot_specific()
{
	U32 pci_id = 0x80000000;
	unsigned char size_slot, size;
	pcislot_t   *slotp = &pcislot[0];
	
	size_slot = 0;
	size = slotp->pci_size >> 26;
	while(size) {
		size_slot++;
		size = size >> 1;
	}
	size_slot = size_slot << 5;
	
	/* Write the Base address register for Galileo */
	pciconf_writel(pci_id, PCI_CONF_BASE0, slotp->pci_start);
	/* Write the Slot info and size of PCI Mem */
	pciconf_writeb(pci_id, PCI_CONF_ILINE, size_slot);
}

STATUS
test_ethernet(U32 pci_id)
{
	U32 i = pci_id;
	return(OK);
}
STATUS
test_scsi(U32 pci_id)
{
	U32 i = pci_id;
	return(OK);
}


unsigned char	*ledp0 = (unsigned char *)0xBC008000;
unsigned char	*ledp1 = (unsigned char *)0xBC028000;
unsigned char	*ledp2 = (unsigned char *)0xBC048000;
unsigned int	timer_int_count = 0;
unsigned char	led0_val = 0;
unsigned char	led1_val = 0;
void led0_flip_flop();


void
led0_flip_flop()
{
	if ( not_an_hbc) {
		if (timer_int_count++ >= 100) {
			*ledp0 = led0_val ^= 0x01;
			timer_int_count = 0;
		}
	}
}
