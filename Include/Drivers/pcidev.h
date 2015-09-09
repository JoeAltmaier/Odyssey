/*
 *
 * MODULE: pcidev.h - header file for the PCI devices
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



#ifndef		_PCIDEV_
#define		_PCIDEV_

#ifdef  __cplusplus
extern "C" {
#endif


/*
 * Config Header Offsets
 */
#define	PCI_CONF_VENID			0x0		/* Vendor id */
#define	PCI_CONF_DEVID			0x2		/* Device id */
#define	PCI_CONF_COMM			0x4		/* Command register */
#define	PCI_CONF_STAT			0x6		/* Status register */
#define	PCI_CONF_REVID			0x8		/* Revision id */
#define	PCI_CONF_SUBCLASS		0xA		/* Sub-class  */
#define	PCI_CONF_CLASS			0xB		/* Class Code */
#define	PCI_CONF_CLS			0xC		/* Cache line size */
#define	PCI_CONF_LT				0xD		/* Latency timer */
#define	PCI_CONF_HEADER			0xE		/* Header type */
#define	PCI_CONF_BIST			0xF		/* Built-In-Self-Test */
#define	PCI_CONF_BASE0			0x10	/* Base register 0 */
#define	PCI_CONF_BASE1			0x14	/* Base register 1 */
#define	PCI_CONF_BASE2			0x18	/* Base register 2 */
#define	PCI_CONF_BASE3			0x1C	/* Base register 3 */
#define	PCI_CONF_BASE4			0x20	/* Base register 4 */
#define	PCI_CONF_BASE5			0x24	/* Base register 5 */
#define	PCI_CONF_CIS			0x28	/* Cardbus CIS Pointer */
#define	PCI_CONF_SUBVENID		0x2C	/* Subsystem Vendor ID */
#define	PCI_CONF_SUBSYSID		0x2E	/* Subsystem ID */
#define	PCI_CONF_ROM			0x30	/* ROM Base register */
#define	PCI_CONF_ILINE			0x3C	/* Interrupt line */
#define	PCI_CONF_IPIN			0x3D	/* Interrupt pin */
#define	PCI_CONF_MIN_G			0x3E	/* Minimum grant */
#define	PCI_CONF_MAX_L			0x3F	/* Maximum grant */

/* PCI_1 offsets for Galileo */
#define	PCI_1_CONF_VENID		0x80		/* Vendor id */
#define	PCI_1_CONF_DEVID		0x82		/* Device id */
#define	PCI_1_CONF_COMM			0x84		/* Command register */
#define	PCI_1_CONF_STAT			0x86		/* Status register */


/* PCI-to-PCI Bridge */
#define	PCI_BCONF_PRIBUS		0x18	/* Primary Bus Number */
#define	PCI_BCONF_SECBUS		0x19	/* Secondary Bus Number */
#define	PCI_BCONF_SUBBUS		0x1A	/* Secondary Bus Number */
#define	PCI_BCONF_SECLT			0x1B	/* Secondary Latency timer */
#define	PCI_BCONF_IOBASE		0x1C	/* I/O Base Register */
#define	PCI_BCONF_IOLIMIT		0x1D	/* I/O Limit Register */
#define	PCI_BCONF_SECSTAT		0x1E	/* Secondary Status register */
#define	PCI_BCONF_MEMBASE		0x20	/* Memory Base Register */
#define	PCI_BCONF_MEMLIMIT		0x22	/* Memory Limit Register */
#define	PCI_BCONF_PMEMBASE		0x24	/* Prefetch Memory Base Register */
#define	PCI_BCONF_PMEMLIMIT		0x26	/* Prefetch Memory Limit Register */
#define	PCI_BCONF_P32_MEMBASE	0x28	/* Prefetch Upper 32 Memory Base  */
#define	PCI_BCONF_P32_MEMLIMIT	0x2C	/* Prefetch Upper 32 Memory Limit  */
#define	PCI_BCONF_U32_IOBASE	0x30	/* Upper 16 I/O Base  */
#define	PCI_BCONF_U32_IOLIMIT	0x32	/* Upper 16 I/O Base  */
#define	PCI_BCONF_ROM			0x38	/* ROM Base register */
#define	PCI_BCONF_ILINE			0x3C	/* Interrupt line */
#define	PCI_BCONF_IPIN			0x3D	/* Interrupt pin */
#define	PCI_BCONF_CNTL			0x3E	/* Bridge Control Register */
		
/* Bridge Control Register */
#define	PCI_BCNTL_B2B			0x0080	/* Fast Back-to-back enable on sec bus*/

/* Secondary Bus Latency Value */
#define	PCI_BSECLT				0x40
/* PCI Command Register */
#define	PCI_COMM_IO				0x0001	/* IO access enable */
#define	PCI_COMM_MEM			0x0002	/* Memory aceess enable */
#define	PCI_COMM_MASTER			0x0004	/* Bus Master Enable */
#define	PCI_COMM_PARERR			0x0040	/* Parity Error Response */
#define	PCI_COMM_SERR			0x0100	/* System Error Enable */
#define	PCI_COMM_B2B			0x0200	/* Fast Back to Back Enable */

/* PCI Conf Header Reister */
#define	PCI_HEADER_TYPE_M		0x7F	/* Header Type */
#define	PCI_HEADER_FUNC_M		0x80	/* single or multi function */

/* PCI Conf BASE Address Register */
#define	PCI_BASE_IOMEM			0x01
#define	PCI_BASE_TYPE_M			0x06
#define	PCI_BASE_TYPE_32		0x00
#define	PCI_BASE_TYPE_1MB		0x02
#define	PCI_BASE_TYPE_64		0x04
#define	PCI_BASE_TYPE_PREF		0x08
#define	PCI_BASE_MEMADDR_M		0xFFFFFFF0	
#define	PCI_BASE_IOADDR_M		0xFFFFFFFC	


/* Mask for Config Addresses */
#define	PCI_ADDR_REG_M		0x000000FF  /* Register mask */
#define	PCI_ADDR_FUNC_M		0x00000700  /* Function mask */
#define	PCI_ADDR_DEV_M		0x0000F800  /* Device mask */
#define	PCI_ADDR_BUS_M		0x00FF0000  /* Bus number mask */
#define	PCI_ADDR_BUSDEV_M	(PCI_ADDR_BUS_M | PCI_ADDR_DEV_M)

/* Sizes of bit fileds in the Address */ 
#define	PCI_ADDR_REG_SZ		0x000000FF  /* Register mask */
#define	PCI_ADDR_FUNC_SZ	0x00000007  /* Function mask */
#define	PCI_ADDR_DEV_SZ		0x0000001F  /* Device mask */
#define	PCI_ADDR_BUS_SZ		0x000000FF  /* Bus number mask */

#define	PCI_GET_REG(X)		((X) & PCI_ADDR_REG_M)
#define	PCI_GET_FUNC(X)		(((X) & PCI_ADDR_FUNC_M) >> 8)
#define	PCI_GET_DEV(X)		(((X) & PCI_ADDR_DEV_M) >> 11)
#define	PCI_GET_BUS(X)		(((X) & PCI_ADDR_BUS_M) >> 16)

#define PCI_BMEM_ALIGN		0x00100000
#define PCI_BMEM_SHIFT		16


#define DWSWAP(X) ((((X)&0xff)<<24) | (((X)&0xff00)<<8) | (((X)&0xff0000)>>8)    | (((X)&0xff000000)>>24))
#define WSWAP(X) ((((X)&0xff)<<8) | (((X)&0xff00)>>8))

#define	PADDR(B, D, F) ( 0x80000000 \
				| (((B)&PCI_ADDR_BUS_SZ) << 16) \
				| (((D)&PCI_ADDR_DEV_SZ) << 11) \
				| (((F)&PCI_ADDR_FUNC_SZ) << 8) )
		
#define	ALIGN(X,Y)	(((U32)X + ((U32)Y - 1)) & ~((U32)Y - 1))

/*
typedef	struct pdev {
	U32		pci_id;
	U32		devven_id;
		
	U8		pbus_num;
	U8		sbus_num;
	U8		subbus_num;
	
	U8		memsize;
	U8		pref_memsize;

	S32				is_pcibridge;
	S32				is_hostbridge;
	
	struct pdev		*parent;
	struct pdev		*peer;
	struct pdev		*child;
} pdev_t;
*/
typedef	struct pdev {
	U32		devven_id;
	U32		pci_id;
} pdev_t;

#define	GAL120_ID		0x462011AB
#define	BRIDGE_21154_ID	0x00261011
#define	BRIDGE_21152_ID	0x00241011
#define	ISP_ID			0x21001077
#define	ISP2_ID			0x22001077
#define	ETHER_CNTL_ID	0x00191011
#define	SCSI_CNTL_ID	0x10201077

extern 	pdev_t	pdevs[];
extern	U32	 pciconf_readl(U32 pci_id, S32 reg_num);
extern	U16	 pciconf_readw(U32 pci_id, S32 reg_num);
extern	U8	 pciconf_readb(U32 pci_id, S32 reg_num);
extern	void pciconf_writel(U32 pci_id, S32 reg_num, U32 val);
extern	void pciconf_writew(U32 pci_id, S32 reg_num, U16 val);
extern	void pciconf_writeb(U32 pci_id, S32 reg_num, U8 val);
extern	void gt_write(U32 offset, U32 val );
extern	U32  gt_read(U32 offset);
extern	void bridge_init(U32 pci_id, U32 primbus, U32 secbus, U32 subbus,
					     U32 pmembase, U32 pmemlmt);

extern	void	pciconf_print(U32 pci_id);
extern	void	pciconf_print_bridge(U32 pci_id);
extern	void	probe_pci_devs(U32 bus_num);
extern	U32		get_pci_id(U32 bus_num, U32 devven_id, U32 instance);


extern	U32	get_isp_pci_id();
extern	U32 get_bridge_pci_id(U32 instance);
extern	U32 get_galileo_pci_id(U32 instance);
extern	U32	get_isp_base(U32 instance);
extern	U32	get_isp_int(U32 instance);
extern	void	set_isp_base(U32 base);
extern	U32 IsNacE1();

extern	U32 test_isp_regs();

#ifdef  __cplusplus
}
#endif
#endif		/* _PCIDEV_  */
