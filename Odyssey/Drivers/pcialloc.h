/*
 *
 * MODULE: pcialloc.h - header file for the PCI devices
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


#ifndef		_PCIALLOC_
#define		_PCIALLOC_

#ifdef  __cplusplus
extern "C" {
#endif
		
		
#define	MAXNODES		8
#define	IS_BRIDGE		0x80
		
#define	NODE_HBC		BOARD_HBC
#define	NODE_SSD		BOARD_SSD
#define	NODE_NIC		(BOARD_NIC | IS_BRIDGE)
#define	NODE_RAC		(BOARD_RAC | IS_BRIDGE)
#define	NODE_NIC_RAC	(BOARD_NIC_RAC | IS_BRIDGE)

#define	NODE_TREE		10
#define	NODE_BRIDGE_E	(11 | IS_BRIDGE)
#define	NODE_BRIDGE_A	(12 | IS_BRIDGE)
#define	NODE_BRIDGE_B	(13 | IS_BRIDGE)
#define	NODE_BRIDGE_C	(14 | IS_BRIDGE)
#define	NODE_BRIDGE_D	(15 | IS_BRIDGE)
		
typedef	struct	_node {
	int		node_type;
	unsigned long	pci_id;
	int		bus_num;
	int		leaf;
	int		node_count;
	int		mem_size;
	int		mem_start;
	int		nic_count;
	int		rac_count;
	int		ssd_count;
	struct	_node	*orgnodes[MAXNODES];
	struct	_node	*permnodes[MAXNODES];
} node_t;

#define	MAX_PCI_SLOTS	18
typedef	struct	_pcislot {
	int	card_type;
	unsigned long pci_id;
	unsigned long pci_start;
	unsigned long pci_size;
	char	slotname[4];
} pcislot_t;

#define	MAX_SIZE	32
#define	INIT_SIZE	8

#define	SIZE_DEC(X)	( (X) ? (((X) >> 1) ? ((X) >> 1):1) : 0 )
#define	SIZE_VERIFY(X, Y, Z)	\
	((( nic_count*(X) + rac_count*(Y) + ssd_count *(Z) + 2) <= MAX_SIZE)? 1:0)
		
#define	ROTATE1(B, X)	{ register int r1i;	\
		register node_t	*r1temp;			\
		r1temp = (B)[0];					\
		for(r1i=0; r1i < (X)-1; r1i++) {	\
			(B)[r1i] = (B)[r1i+1];			\
		}									\
		(B)[r1i] = r1temp;	}

		
#define	ROTATEN(B, X, N) { register int rni;\
		for(rni=0; rni < (N); rni++) {	\
			ROTATE1((B), (X));			\
		}}
		
		

extern	node_t  *treenode;
extern	pcislot_t   pcislot[];
extern	char *bname[];

extern	void pci_configure();
extern	node_t  *get_nonleaf(node_t *node, int instance);

#ifdef  __cplusplus
}
#endif
#endif		/* _PCIALLOC_  */
