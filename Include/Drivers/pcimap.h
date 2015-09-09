
#ifndef PCIMAP_H
#define PCIMAP_H
#ifdef  __cplusplus
extern "C" {
#endif

/* PCI Window for PCI_1 */
#define	PCI_1_MEM_LOW		0x12000000
#define	PCI_1_MEM_HIGH		0x13FFFFFF
		

#define	M(X)	((X) * 0x100000)
#define	K(X)	((X) * 1024 )

#define	REMAP_ADDRESS	0

/* Base Address for Q-LOGIC on NIC/RAC Board */

#define	MAX_SLOTS			32
#define	MAX_MSEGS			2

#define	PCI_WINDOW_START	0x80000000
#define	PCI_WINDOW_END		0xFFFFFFFF
#define	LOCAL_PCI_START		0x04000000
#define	ISP_BASE			0x10000000		/* We access ISP with IO Access */

#ifdef	INCLUDE_ODYSSEY
#define	PCITOV(X)				( (X) - 0x80000000)
#define	VTOPCI(X)				( (X) + 0x80000000)
#else
#define	PCITOV(X)				( (X) + 0xA0000000)
#define	VTOPCI(X)				( (X) - 0xA0000000)
#endif

#define	PTOV1(X)				( (X) + 0x80000000)
#define	VTOP1(X)				( (X) - 0x80000000)

		/*
#define	CACHE_CODE 1
*/

#ifdef	CACHE_CODE
#define	PTOV(X)					( (X) + 0x80000000)
#define	VTOP(X)					( (X) - 0x80000000)
#else
#define	PTOV(X)					( (X) + 0xA0000000)
#define	VTOP(X)					( (X) - 0xA0000000)
#endif

typedef struct	mmap {
	/* Describe pci address space */
	U32 aBoardType[MAX_SLOTS];	/* HPB/SSD/NIC/RAC */
	U32 aIdPci[MAX_SLOTS];		/* Pci id of galileo or Bridge */
	U32 aPaPci[MAX_SLOTS];		/* Pa(physical address) of each pci window */
	
	/* Next entries describe this board only */
	U32 iCabinet;		/* zero */
	U32 iSlot;			/* Our Slot (different value on each board) */
	
	U32 pciSlave;		/* Our slave pci window physical addr on the pci bus */
	U32 paSlave;		/* Our slave pci window physical addr on the local bus*/
	U32 cbSlave;		/* Our slave pci window size */
	
	U32 nFragment;		/* Number of physical memory fragments(# elements in 
						   the following arrays */
	U32 aPa[MAX_MSEGS];	/* Array of physical addresses of memory fragments */
	U32 aP[MAX_MSEGS];	/* Array of pointers (virtual addr) of memory fragment*/
	U32 aCb[MAX_MSEGS];	/* Array of cb(count of bytes) of each memory fragment*/
} mmap_t;

typedef	struct	membanks {
	U32		bank10_size;
	U32		bank10_start;
	U32		bank32_size;
	U32		bank32_start;
		
} membanks_t;

#define	GT_DEC_LOW_MASK		0x7FFF
#define	GT_DEC_HIGH_MASK	0x7F
		
		

/* There will be an array 'pcimaps' of above structure with each member 
 * corresponding to each board
 */
extern mmap_t	memmaps;
extern membanks_t membanks;
extern U32		pmap_size[];

extern	char 	*slotname[];
/* Definitions for the board type */
#define	BOARD_HBC			1
#define	BOARD_NAC			2
#define	BOARD_SSD			3
#define	BOARD_NIC			4
#define	BOARD_RAC			5
#define	BOARD_NIC_RAC		BOARD_NAC


#define	PCI_MAPSIZE_FOR_LOCAL	0x04000000

#define	BRIDGE_A_DEVNUM		13
#define	BRIDGE_B_DEVNUM		14
#define	BRIDGE_C_DEVNUM		15
#define	BRIDGE_D_DEVNUM		16
#define	BRIDGE_E_DEVNUM		17


#ifdef	CONFIG_E2
#define	PCI_F_BUSNUM		1
#define	PCI_H_BUSNUM		2
#define	PCI_A_BUSNUM		3
#define	PCI_B_BUSNUM		8
#define	PCI_C_BUSNUM		13
#define	PCI_D_BUSNUM		18
#define	PCI_E_BUSNUM		23
#else
#define	PCI_A_BUSNUM		1
#define	PCI_B_BUSNUM		6
#define	PCI_C_BUSNUM		11
#define	PCI_D_BUSNUM		16
#define	PCI_E_BUSNUM		21
#endif

#define	PCI_G_BUSNUM		0

#define	PCI_A_SUBNUM		(PCI_B_BUSNUM - 1)
#define	PCI_B_SUBNUM		(PCI_C_BUSNUM - 1)
#define	PCI_C_SUBNUM		(PCI_D_BUSNUM - 1)
#define	PCI_D_SUBNUM		(PCI_E_BUSNUM - 1)
#define	PCI_E_SUBNUM		(PCI_E_BUSNUM)
#define	PCI_F_SUBNUM		(PCI_E_BUSNUM - 1)
#define	PCI_H_SUBNUM		(PCI_C_BUSNUM - 1)

#define	OK								0

#define	GALILEO_0_ACCESS_ERROR			1
#define	GALILEO_1_ACCESS_ERROR			2
/* Error Codes for HPB */
#define	BRIDGE_A_ACCESS_ERROR		3
#define	BRIDGE_B_ACCESS_ERROR		4
#define	BRIDGE_C_ACCESS_ERROR		5
#define	BRIDGE_D_ACCESS_ERROR		6
#define	BRIDGE_E_ACCESS_ERROR		7
#define	BRIDGE_F_ACCESS_ERROR		15
#define	BRIDGE_H_ACCESS_ERROR		16

#define	ISP_ACCESS_ERROR				8
#define	ISP_RISC_NOT_READY_ERROR		9	
#define	ISP_MAILBOX_ERROR				10
#define	ETHERNET_ACCESS_ERROR			11
#define	ETHERNET_TEST_ERROR				12
#define	SCSI_ACCESS_ERROR				13
#define	SCSI_TEST_ERROR					14

#define	BOARD_ACCESS_ERROR				17


#define	GT_PCI_2GIG			0x00200000

/* Defines for Bridge Resets */
#define	PCI_RESET_F		0x0B
#define	PCI_RESET_H		0x07
#define	PCI_RESET_A		0x0E
#define	PCI_RESET_B		0x0D
#define	PCI_RESET_C		0x0B
#define	PCI_RESET_D		0x07

#define	PCI_RESET_DEASSERT	0x0F
#define	PCI_RESET_ADDR_FH	0xBC0D8000
#define	PCI_RESET_ADDR_ABCD	0xBC0C8000

extern	void	get_membank_config();
extern	void	init_galileo();
extern	void	init_pci_bridges();
extern	STATUS	init_onboard_bridges();
extern STATUS	Init_Hardware();
extern STATUS	init_slave_bridges();
extern U32		getSlotNumber();

extern STATUS	InitBridgeFTree();
extern STATUS	InitBridgeHTree();
extern STATUS	InitBridgeATree();
extern STATUS	InitBridgeBTree();
extern STATUS	InitBridgeCTree();
extern STATUS	InitBridgeDTree();
extern STATUS	InitBridgeETree();
#ifdef  __cplusplus
}
#endif

#endif		/* PCIMAP_H */
