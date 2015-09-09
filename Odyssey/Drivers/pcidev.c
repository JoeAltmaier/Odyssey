/*
 *
 * MODULE: pcidev.c - Driver for the PCI Interface
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 09/29/98	- Created by Sudhir
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

#include "types.h"
#include "pcidev.h"
#include "pcimap.h"

#define	MODIFY		1



#define	GT_PCI_CONFIG_ADDR_REG 	((U32 *)(0xB4000CF8))
#define GT_PCI_CONFIG_DATA		0xB4000CFC
#define	GT_PCI_1_CONFIG_ADDR_REG ((U32 *)(0xB4000CF0))
#define GT_PCI_1_CONFIG_DATA	0xB4000CF4

#define GT_BASE_ADDR			0xB4000000
#define	PDEV_MAX		256
pdev_t	pdevs[PDEV_MAX];


U32	devven_ids[] = { GAL120_ID,
					 BRIDGE_21154_ID,
					 BRIDGE_21152_ID,
					 ETHER_CNTL_ID,
					 ISP_ID,
					 SCSI_CNTL_ID,
					 ISP2_ID,
					 0x0
					};
char *devname[] = { "Galileo  ",
					"PCIB21154",
					"PCIB21152",
					"Ethr21143", 
					"QISP-2100", 
					"QISP-1040", 
					"QISP-2200" 
				  };
U16	isp_regvals[20] = { 0x0, 0xFFFF, 0x0, 0x400, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x5349, 0x2050, 0x2020, 0x100, 0x800, 0x303, 0x600,
						0x0, 0x0, 0x0, 0x0
					  };
char *get_devname(U32 devven_id);

U32
pciconf_readl(U32 pci_id, I32 reg_num)
{
	U32	val;
	
	*GT_PCI_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	val = *((U32 *)GT_PCI_CONFIG_DATA);
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		return(val);
	else						/* Galileo */
		return(DWSWAP(val));
		
}
U16
pciconf_readw(U32 pci_id, I32 reg_num)
{
	U16	val;
	
	*GT_PCI_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		val = *((U16 *)((GT_PCI_CONFIG_DATA + 2) - (reg_num & 0x2)));
	else						/* Galileo */
		val = WSWAP(*((U16 *)(GT_PCI_CONFIG_DATA +(reg_num & 0x2))));
	return(val);
		
}
U8
pciconf_readb(U32 pci_id, I32 reg_num)
{
	U8	val;
	
	*GT_PCI_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		val = *((U8 *)((GT_PCI_CONFIG_DATA + 3) - (reg_num & 0x3)));
	else						/* Galileo */
		val = *((U8 *)(GT_PCI_CONFIG_DATA + (reg_num & 0x3)));
	return(val);
		
}

void
pciconf_writel(U32 pci_id, I32 reg_num, U32 val)
{
	
	*GT_PCI_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		*((U32 *)GT_PCI_CONFIG_DATA) = val;
	else						/* Galileo */
		*((U32 *)GT_PCI_CONFIG_DATA) = DWSWAP(val);
		
}
void
pciconf_writew(U32 pci_id, I32 reg_num, U16 val)
{
	
	*GT_PCI_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		*((U16 *)((GT_PCI_CONFIG_DATA + 2) - (reg_num & 0x2))) = val;
	else						/* Galileo */
		*((U16 *)(GT_PCI_CONFIG_DATA +(reg_num & 0x2))) = WSWAP(val);
		
}

void
pciconf_writeb(U32 pci_id, I32 reg_num, U8 val)
{
	
	*GT_PCI_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		*((U8 *)((GT_PCI_CONFIG_DATA + 3) - (reg_num & 0x3))) = val;
	else						/* Galileo */
		*((U8 *)(GT_PCI_CONFIG_DATA +(reg_num & 0x3))) = val;
		
}


/* Routines to Access PCI_1 Interface */
U32
pciconf_1_readl(U32 pci_id, I32 reg_num)
{
	U32	val;
	
	*GT_PCI_1_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	val = *((U32 *)GT_PCI_1_CONFIG_DATA);
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		return(val);
	else						/* Galileo */
		return(DWSWAP(val));
		
}
U16
pciconf_1_readw(U32 pci_id, I32 reg_num)
{
	U16	val;
	
	*GT_PCI_1_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		val = *((U16 *)((GT_PCI_1_CONFIG_DATA + 2) - (reg_num & 0x2)));
	else						/* Galileo */
		val = WSWAP(*((U16 *)(GT_PCI_1_CONFIG_DATA +(reg_num & 0x2))));
	return(val);
		
}
U8
pciconf_1_readb(U32 pci_id, I32 reg_num)
{
	U8	val;
	
	*GT_PCI_1_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		val = *((U8 *)((GT_PCI_1_CONFIG_DATA + 3) - (reg_num & 0x3)));
	else						/* Galileo */
		val = *((U8 *)(GT_PCI_1_CONFIG_DATA + (reg_num & 0x3)));
	return(val);
		
}

void
pciconf_1_writel(U32 pci_id, I32 reg_num, U32 val)
{
	
	*GT_PCI_1_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		*((U32 *)GT_PCI_1_CONFIG_DATA) = val;
	else						/* Galileo */
		*((U32 *)GT_PCI_1_CONFIG_DATA) = DWSWAP(val);
		
}
void
pciconf_1_writew(U32 pci_id, I32 reg_num, U16 val)
{
	
	*GT_PCI_1_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		*((U16 *)((GT_PCI_1_CONFIG_DATA + 2) - (reg_num & 0x2))) = val;
	else						/* Galileo */
		*((U16 *)(GT_PCI_1_CONFIG_DATA +(reg_num & 0x2))) = WSWAP(val);
		
}

void
pciconf_1_writeb(U32 pci_id, I32 reg_num, U8 val)
{
	
	*GT_PCI_1_CONFIG_ADDR_REG = DWSWAP(pci_id | (reg_num & PCI_ADDR_REG_SZ));
	if ( pci_id & PCI_ADDR_BUSDEV_M)
		*((U8 *)((GT_PCI_1_CONFIG_DATA + 3) - (reg_num & 0x3))) = val;
	else						/* Galileo */
		*((U8 *)(GT_PCI_1_CONFIG_DATA +(reg_num & 0x3))) = val;
		
}

void
pciconf_print(U32 pci_id)
{
	int i, j;
	
	j = 1;
	for(i=0; i < 0x40; i = i+4) {
		printf("%02X : %08X,	", i, pciconf_readl(pci_id, i));
		if ( (j % 4) == 0 )
			printf("\n\r");
		j++;
	}
}

void
pciconf_print_bridge(U32 pci_id)
{
	int i, j;
	
	j = 1;
	for(i=0; i < 0x44; i = i+4) {
		printf("%02X : %08X,	", i, pciconf_readl(pci_id, i));
		if ( (j % 4) == 0 )
			printf("\n\r");
		j++;
	}
		
		
}

void
gt_write(U32 offset, U32 val)
{
	*((U32 *)(GT_BASE_ADDR + offset)) = DWSWAP(val);
}

U32
gt_read(U32 offset)
{
	U32 val;
	val = *((U32 *)(GT_BASE_ADDR + offset));
	return(DWSWAP(val));
}


void
bridge_init(U32 pci_id, U32 primbus, U32 secbus, U32 subbus,
			U32 pmembase, U32 pmemlmt)
{
	U32	membase = 0xFFFFFFFF;
	U32	memlmt	= 0x0;
						/* Disable the Bus Master */
	pciconf_writew(pci_id, PCI_CONF_COMM, 0);
						/* Program the Primary Bus number */
	pciconf_writeb(pci_id, PCI_BCONF_PRIBUS, primbus);
						/* Program the Secondary Bus number */
	pciconf_writeb(pci_id, PCI_BCONF_SECBUS, secbus);
						/* Program the Sub-ordinate Bus number */
	pciconf_writeb(pci_id, PCI_BCONF_SUBBUS, subbus);
						/* Set the Prefetchable mem base and limit */
	pciconf_writew(pci_id, PCI_BCONF_PMEMBASE, (pmembase >> PCI_BMEM_SHIFT));
	pciconf_writew(pci_id, PCI_BCONF_PMEMLIMIT, (pmemlmt >> PCI_BMEM_SHIFT));

						/* configure the I/O Range from 0x10000000 to
						 * 0x11FFFFFF */
	pciconf_writew(pci_id, PCI_BCONF_IOBASE, 0x0);
	pciconf_writew(pci_id, PCI_BCONF_U32_IOBASE, 0x1000);
	pciconf_writew(pci_id, PCI_BCONF_IOLIMIT, 0xFF00);
	pciconf_writew(pci_id, PCI_BCONF_U32_IOLIMIT, 0x11FF);
						/* Set Mem-mapped IO, to disable make membase=0xFFFF
						 * memlmt = 0
						 */
	pciconf_writew(pci_id, PCI_BCONF_MEMBASE, (membase >> PCI_BMEM_SHIFT));
	pciconf_writew(pci_id, PCI_BCONF_MEMLIMIT, (memlmt >> PCI_BMEM_SHIFT));
						/* Make the bridge Bus Master and enable mem access 
						 * Has to be done after initializing rest of the
						 * stuff
						 */
						/* Configure the bridge for Fast Back to Back */
	pciconf_writew(pci_id, PCI_CONF_COMM, 
				PCI_COMM_MEM | PCI_COMM_MASTER | PCI_COMM_B2B );
	
						/* Configure the Secondary Bus Latency timer */
	pciconf_writeb(pci_id, PCI_BCONF_SECLT, PCI_BSECLT);
						/* Configure the bridge for Fast Back to Back */
	pciconf_writew(pci_id, PCI_BCONF_CNTL, PCI_BCNTL_B2B);
	
	   					/* Program the Cache Line Size */
	pciconf_writeb(pci_id, PCI_CONF_CLS, 0x00);
	
	/* Program the Primary Latency Timer */
	pciconf_writeb(pci_id, PCI_CONF_LT, 0xFF);

	/* Program the Secondary Latency Timer */
	pciconf_writeb(pci_id, PCI_BCONF_SECLT, 0xFF);

#define	PCI_BCNTL_PARERR	0x0001
#define	PCI_BCNTL_SERR		0x0002
#define	PCI_BCNTL_MABORT	0x0020
#define	PCI_BCNTL_PMTIMEOUT	0x0100
#define	PCI_BCNTL_SMTIMEOUT	0x0200
#define	PCI_BCNTL_MTIMEOUTSERR	0x0800
#if 0
	/* Program the Memory Write Disconnect Control Bit */
	pciconf_writeb(pci_id, 0x40, 0x02);
	pciconf_writew(pci_id, PCI_CONF_COMM, 
				PCI_COMM_MEM | PCI_COMM_MASTER | PCI_COMM_B2B 
				| PCI_COMM_PARERR | PCI_COMM_SERR );
	pciconf_writew(pci_id, PCI_BCONF_CNTL, PCI_BCNTL_B2B
				| PCI_BCNTL_PARERR | PCI_BCNTL_SERR 
				);
#endif
}


void 
probe_devs(I32 bus_num, I32 pci_iface)
{
	I32 devnum,funnum,i;
	U8 htype;
	U32 devven_id;
	U32 pci_id;
	I32 pdevnum = 0;
	
	for(i=0; i < 32; i++) {
		pdevs[i].devven_id = 0xFFFFFFFF;
		pdevs[i].pci_id 	= 0;
	}
	for(devnum=0; devnum < 31; devnum++ ) {
		pci_id = PADDR(bus_num, devnum, 0);
		if ( pci_iface == 0)
			devven_id = pciconf_readl(pci_id, PCI_CONF_VENID);
		else
			devven_id = pciconf_1_readl(pci_id, PCI_CONF_VENID);
		if ( devven_id != 0xFFFFFFFF) {
				/* Found a Device */
			if ( pci_iface == 0)
				htype	= pciconf_readb(pci_id, PCI_CONF_HEADER);
			else
				htype	= pciconf_1_readb(pci_id, PCI_CONF_HEADER);
			if ( htype & PCI_HEADER_FUNC_M)
				funnum = 8;
			else
				funnum = 1;
			for(i=0; i < funnum; i++) {
				pci_id = PADDR(bus_num, devnum, i);
				if ( pdevnum > PDEV_MAX ) {
					printf("Num of devs found mode than PDEV_MAX\n\r");
					return;
				}
				pdevs[pdevnum].devven_id	= devven_id;
				pdevs[pdevnum++].pci_id		= pci_id;
			}
		}
	}
}

void
print_devs()
{
	I32 i;
	U32 pci_id;


	if ( pdevs[0].devven_id != 0xFFFFFFFF)
		printf("PCI Devices Found :\n\r");
	else
		printf("PCI Devices not Found :\n\r");
	i = 0;
	while(pdevs[i].devven_id != 0xFFFFFFFF) {
		pci_id = pdevs[i].pci_id;
		if ( get_devname(pdevs[i].devven_id))
			printf(get_devname(pdevs[i].devven_id));
		else {
			printf("%08X ", (pdevs[i].devven_id));
		}
		printf(": pciid :%X, Bus :%x, Idsel :%x", pci_id, PCI_GET_BUS(pci_id),
						PCI_GET_DEV(pci_id));
		if ( pdevs[i].devven_id == 0x00261011 ) {
			printf(", SBus :%x, SubBus :%x", 
					pciconf_readb(pci_id, PCI_BCONF_SECBUS),
					pciconf_readb(pci_id, PCI_BCONF_SUBBUS));
		}
		printf("\n\r");
		i++;
	}
}
void
probe_pci_devs(U32 bus_num)
{
	probe_devs(bus_num, 0);
	print_devs();
}

U32
get_pci_id(U32 bus_num, U32 devven_id, U32 instance)
{
	I32 i;
	U32	inst = 0;

	probe_devs(bus_num, 0);
	i = 0;
	while(pdevs[i].devven_id != 0xFFFFFFFF) {
		if ( pdevs[i].devven_id == devven_id ) {
			if ( instance == inst )
				return(pdevs[i].pci_id);
			else
				inst++;
		}
		i++;
	}
	return(0);
}

U32
get_isp_base(U32 instance)
{
	U32	pci_id;
	
#ifdef	CONFIG_E2
	if ( IsNacE1())
		pci_id = PADDR(0, (7 + instance), 0);
	else
		pci_id = PADDR(0, (2 + instance), 0);
#else
	pci_id = get_pci_id(0, ISP_ID, instance);
#endif

	if ( pci_id ) {
		return(pciconf_readl(pci_id, PCI_CONF_BASE0) & PCI_BASE_IOADDR_M);
	}
	return(0);
}
U32
get_isp_int(U32 instance)
{
	U32	pci_id;
#ifdef	CONFIG_E2
	if ( IsNacE1())
		pci_id = PADDR(0, (7 + instance), 0);
	else
		pci_id = PADDR(0, (2 + instance), 0);
#else
	pci_id = get_pci_id(0, ISP_ID, instance);
#endif
	if ( pci_id ) {
		return((U32)pciconf_readb(pci_id, PCI_CONF_ILINE));
	}
	return(0);
	
}

U32
IsNacE1()
{
#ifdef INCLUDE_ODYSSEY
	if (get_pci_id(0, ISP2_ID, 2))
		return(1);
	else
		return(0);
#else
	return(1);
#endif
}


char *
get_devname(U32 devven_id)
{
	I32 i;
	
	i=0;
	while(devven_ids[i]) {
		if ( devven_id == devven_ids[i] )
			return(devname[i]);
		i++;
	}
	return(0);
		
}
U32
get_isp_pci_id()
{
	I32 i;

	i = 0;
	while(pdevs[i].devven_id != 0xFFFFFFFF) {
		if ( pdevs[i].devven_id == ISP_ID)
			return(pdevs[i].pci_id);
		i++;
	}
	return(0);
}

U32
get_bridge_pci_id(U32 instance)
{
	I32 i;
	U32	inst = 0;

	i = 0;
	while(pdevs[i].devven_id != 0xFFFFFFFF) {
		if ( (pdevs[i].devven_id == BRIDGE_21152_ID) 
				|| (pdevs[i].devven_id == BRIDGE_21154_ID) ) {
			if ( instance == inst )
				return(pdevs[i].pci_id);
			else
				inst++;
		}
		i++;
	}
	return(0);
}

U32
get_galileo_pci_id(U32 instance)
{
	I32 i;
	U32	inst = 0;

	i = 0;
	while(pdevs[i].devven_id != 0xFFFFFFFF) {
		if ( pdevs[i].devven_id == GAL120_ID) { 
			if ( instance == inst )
				return(pdevs[i].pci_id);
			else
				inst++;
		}
		i++;
	}
	return(0);
}

U32
test_isp_regs()
{
	U16	*sptr = (U16 *)PTOV(get_isp_base(0));
	U32	*lptr = (U32 *)PTOV(get_isp_base(0));
	int i;
	

	for(i=0; i < 20; i++) {
		if ( sptr[i] != isp_regvals[i])
			return(0);
	}
	for(i=0; i < 20; i++) {
		if ( lptr[i] != 0xFFFFFFFF)
			return(0);
	}
	return(1);
}

