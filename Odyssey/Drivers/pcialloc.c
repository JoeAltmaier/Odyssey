/*
 *
 * MODULE: pcialloc.c - header file for the PCI devices
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

/*
#define	DEBUG	1
*/
#define	PCI_STATIC	1

#include "types.h"
#include "pcidev.h"
#include "pcimap.h"
#include "galileo.h"
#include "pcialloc.h"

/* Temp Data Structure and routine for Mem Allocation */
#define	NBUF_SIZE	50
node_t	nodebuf[NBUF_SIZE];
int		nodebuf_index = 0;



node_t *
get_nodemem()
{
	int i;
	char *ptr;

	if (nodebuf_index >= NBUF_SIZE)
		return((node_t *)0);
	
	ptr = (char *)&nodebuf[nodebuf_index++];
	for(i=0; i < sizeof(node_t); i++)
		ptr[i] = 0;

	return((node_t *)ptr);
}

/* Global Data Structure Used in other  files also */
node_t	*treenode;
pcislot_t	pcislot[MAX_PCI_SLOTS];
char *bname[] = {   "Unknown",
					"HBC",
					"NAC", 
					"SSD",
					"NIC", 
					"RAC",
					"Bridge_E",
					"Bridge_A",
					"Bridge_B",
					"Bridge_C",
					"Bridge_D"
				  };

/* Global Data Structure Used in this file only */

int		mstart = 0;
int	pcount[MAXNODES + 1] = { 1, 1, 2, 6, 24, 120, 720, 5040, 40320 };
int	pci_slot[MAX_PCI_SLOTS] = 
					{0x80000000,
					 0x80000000,
					 PADDR(PCI_A_BUSNUM, 12, 0),
					 PADDR(PCI_A_BUSNUM, 13, 0),
					 PADDR(PCI_A_BUSNUM, 14, 0),
					 PADDR(PCI_A_BUSNUM, 15, 0),
					 PADDR(PCI_B_BUSNUM, 12, 0),
					 PADDR(PCI_B_BUSNUM, 13, 0),
					 PADDR(PCI_B_BUSNUM, 14, 0),
					 PADDR(PCI_B_BUSNUM, 15, 0),
					 PADDR(PCI_C_BUSNUM, 12, 0),
					 PADDR(PCI_C_BUSNUM, 13, 0),
					 PADDR(PCI_C_BUSNUM, 14, 0),
					 PADDR(PCI_C_BUSNUM, 15, 0),
					 PADDR(PCI_D_BUSNUM, 12, 0),
					 PADDR(PCI_D_BUSNUM, 13, 0),
					 PADDR(PCI_D_BUSNUM, 14, 0),
					 PADDR(PCI_D_BUSNUM, 15, 0)
					};
					
/* Function Prototypes */
int 	get_pci_slot(unsigned long pci_id);
void	perm(node_t *orgbuf[], node_t *resbuf[], int size, int pindex);
node_t *get_node(node_t *node, int type);
void	create_onboard_nodes();
void	create_pci_nodes(int bridge);
int		alloc_size(node_t *tree, int *nic_size, int *rac_size, int *ssd_size);
void	assign_bounds(node_t *node);
node_t  *get_nonleaf(node_t *node, int instance);
int		pci_allocate();
void	update_bridge_nodes(int bridge);
void	update_pcislot_data_structure();
void	print_node(node_t *node);

					
int
get_pci_slot(unsigned long pci_id)
{
	int i;
	
	for(i=0; i < MAX_PCI_SLOTS; i++) {
		if ( pci_id == pci_slot[i])
			return(i);
	}
	return(-1);
}





void
perm(node_t *orgbuf[], node_t *resbuf[], int size, int pindex)
{
	register int	i;
	register int div;
	
	for(i=0; i < size; i++)
		resbuf[i] = orgbuf[i];
	
	for(i=size; i > 1; i--) {
		div = pindex / pcount[i-1] % i;
		ROTATEN(resbuf, i, div);
	}
}


node_t *
get_node(node_t *node, int type)
{
	int		i;
	node_t	*tnode;
	if ( node->node_type == type) {
		return(node);
	}
	
	if ( node->leaf) {
		return((node_t *)0);
	}
	
	for(i=0; i < node->node_count; i++) {
		tnode = get_node(node->orgnodes[i], type);
		if ( tnode) {
			return(tnode);
		}
	}
	return((node_t *)0);
}

void
create_onboard_nodes()
{
	int	i;

	treenode = get_nodemem();
	treenode->node_type	= NODE_TREE;
	treenode->leaf	= 0;
	treenode->node_count	= 6;
	treenode->mem_size	= MAX_SIZE;
	treenode->mem_start	= 0;
	treenode->nic_count	= 0;
	treenode->rac_count	= 0;
	treenode->ssd_count	= 0;
	
	if ( treenode->node_count >  MAXNODES ) {
		printf("Too Many Devices\n\r");
		treenode->node_count = MAXNODES;
	}
	for(i=0; i < treenode->node_count; i++) {
		treenode->orgnodes[i] = get_nodemem();
		treenode->orgnodes[i]->leaf = 0;
		treenode->orgnodes[i]->node_count = 0;
	}
	
	treenode->orgnodes[0]->node_type = NODE_HBC;
	treenode->orgnodes[0]->pci_id	 = 0x80000000;
	treenode->orgnodes[0]->bus_num	 = 0;
	treenode->orgnodes[0]->leaf = 1;
	treenode->orgnodes[0]->mem_size = 1;

	treenode->orgnodes[1]->node_type = NODE_BRIDGE_E;
	treenode->orgnodes[1]->pci_id 	 = PADDR(0, BRIDGE_E_DEVNUM, 0);
	treenode->orgnodes[1]->bus_num	 = PCI_E_BUSNUM;
	treenode->orgnodes[1]->leaf = 1;
	treenode->orgnodes[1]->mem_size = 1;

	treenode->orgnodes[2]->node_type = NODE_BRIDGE_A;
	treenode->orgnodes[2]->pci_id 	 = PADDR(0, BRIDGE_A_DEVNUM, 0);
	treenode->orgnodes[2]->bus_num	 = PCI_A_BUSNUM;

	treenode->orgnodes[3]->node_type = NODE_BRIDGE_B;
	treenode->orgnodes[3]->pci_id 	 = PADDR(0, BRIDGE_B_DEVNUM, 0);
	treenode->orgnodes[3]->bus_num	 = PCI_B_BUSNUM;

	treenode->orgnodes[4]->node_type = NODE_BRIDGE_C;
	treenode->orgnodes[4]->pci_id 	 = PADDR(0, BRIDGE_C_DEVNUM, 0);
	treenode->orgnodes[4]->bus_num	 = PCI_C_BUSNUM;

	treenode->orgnodes[5]->node_type = NODE_BRIDGE_D;
	treenode->orgnodes[5]->pci_id 	 = PADDR(0, BRIDGE_D_DEVNUM, 0);
	treenode->orgnodes[5]->bus_num	 = PCI_D_BUSNUM;
		
}

void
create_pci_nodes(int bridge)
{
		
	unsigned long pci_id = 0;
	int	index, instance;
	node_t	*node, *tnode;
	

	/* Probe the Bus on Bridge 'bridge' for NICs */
	index = 0;
	instance = 0;
	node = get_node(treenode, bridge);
	pci_id = get_pci_id(node->bus_num, BRIDGE_21154_ID, instance);
	while(pci_id ) {
		if ( index >=  MAXNODES ) {
			printf("Too Many Devices\n\r");
			return;
		}
		tnode = get_nodemem();
		tnode->node_type = NODE_NIC;
		tnode->pci_id 	 = pci_id;
		tnode->leaf 	 = 1;
		tnode->node_count = 0;
		tnode->bus_num	= node->bus_num + index + 1;
		
		node->node_count++;
		node->nic_count++;
		node->orgnodes[index] = tnode;
			
		index++;
		instance++;
		pci_id = get_pci_id(node->bus_num, BRIDGE_21154_ID, instance);
	}

	/* Probe the Bus on Bridge 'bridge' for SSDs */
	instance = 0;
	pci_id = get_pci_id(node->bus_num, GAL120_ID, instance);
	while(pci_id ) {
		if ( index >=  MAXNODES ) {
			printf("Too Many Devices\n\r");
			return;
		}
		tnode = get_nodemem();
		tnode->node_type = NODE_SSD;
		tnode->pci_id 	 = pci_id;
		tnode->leaf 	 = 1;
		tnode->node_count = 0;
		
		node->node_count++;
		node->ssd_count++;
		node->orgnodes[index] = tnode;
			
		index++;
		instance++;
		pci_id = get_pci_id(node->bus_num, GAL120_ID, instance);
	}
}



int
alloc_size(node_t *tree, int *nic_size, int *rac_size, int *ssd_size)
{
		
	int	nic_count = tree->nic_count;
	int	rac_count = tree->rac_count;
	int	ssd_count = tree->ssd_count;
	int	not_done= 1;

	*nic_size = INIT_SIZE;
	*rac_size = INIT_SIZE;
	*ssd_size = INIT_SIZE;
	
	while( (*rac_size != 1) || (*ssd_size != 1)) {
		if ( SIZE_VERIFY(*nic_size, *rac_size, *ssd_size)) {
			not_done = 0;
			break;
		}
		*ssd_size = SIZE_DEC(*ssd_size);
		*rac_size = SIZE_DEC(*rac_size);
	}

	if ( not_done ) {
		while( *nic_size != 1 ) {
			if ( SIZE_VERIFY(*nic_size, *rac_size, *ssd_size)) {
				not_done = 0;
				break;
			}
			*nic_size = SIZE_DEC(*nic_size);
		}
	}
	if ( SIZE_VERIFY(*nic_size, *rac_size, *ssd_size)) {
		not_done = 0;
	}
	if(not_done)
		return(0);
	else
		return(1);
}

void
assign_bounds(node_t *node)
{
	int		i;
	int		bound;
	
	if ( node->leaf) {
		bound = node->mem_size;
		if ( bound == INIT_SIZE)
			bound = 4;

		mstart = ALIGN(mstart, bound);
		node->mem_start = mstart;
		mstart += node->mem_size;
		return;
	}
	
	for(i=0; i < node->node_count; i++) {
		assign_bounds(node->permnodes[i]);
	}
	return;
}

node_t	*
get_nonleaf(node_t *node, int instance)
{
	register int i; 
	register int inst = 0;

	for(i=0; i < node->node_count; i++) {
		if ( node->permnodes[i]->leaf == 0) {
			if ( instance == inst )
				return(node->permnodes[i]);
			else
				inst++;
				
		}
	}
	return((node_t *)0);
		
}
int 	tcount = 0;

int
pci_allocate()
{
	int i, j;
	int nic_size, rac_size, ssd_size;
	node_t *node;
	int	p, q, r, s;
	node_t	*pnode, *qnode, *rnode, *snode;

	nodebuf_index = 0;
	create_onboard_nodes();
	create_pci_nodes(NODE_BRIDGE_A);
	create_pci_nodes(NODE_BRIDGE_B);
	create_pci_nodes(NODE_BRIDGE_C);
	create_pci_nodes(NODE_BRIDGE_D);
	
	for(i=0; i < treenode->node_count; i++) {
		treenode->nic_count	+= treenode->orgnodes[i]->nic_count;
		treenode->rac_count	+= treenode->orgnodes[i]->rac_count;
		treenode->ssd_count	+= treenode->orgnodes[i]->ssd_count;
	}
#ifndef	PCI_STATIC
	if(alloc_size(treenode, &nic_size, &rac_size, &ssd_size))
		printf("Allocate Successful, NIC : %dMB, RAC : %dMB, SSD : %dMB\n\r", 
				nic_size * 64, rac_size * 64, ssd_size * 64);
	else
		printf("Allocate not Successful\n\r");
#else
	alloc_size(treenode, &nic_size, &rac_size, &ssd_size);
#endif

	for(i=0; i < treenode->node_count; i++) {
		node = treenode->orgnodes[i];
		for(j=0; j < node->node_count; j++) {
			if ( node->orgnodes[j]->node_type == NODE_NIC) {
				node->orgnodes[j]->mem_size = nic_size;
			}
			if ( node->orgnodes[j]->node_type == NODE_RAC) {
				node->orgnodes[j]->mem_size = rac_size;
			}
			if ( node->orgnodes[j]->node_type == NODE_SSD) {
				node->orgnodes[j]->mem_size = ssd_size;
			}
		}
	}
	
	/* Try diff permutations and see bounday condition fits */
	for(i=0; i < pcount[treenode->node_count]; i++) {
		perm(treenode->orgnodes, treenode->permnodes, treenode->node_count, i);
		pnode = get_nonleaf(treenode, 0);
		for(p=0; p < pcount[pnode->node_count]; p++) {
			perm(pnode->orgnodes, pnode->permnodes, pnode->node_count, p);
			qnode = get_nonleaf(treenode, 1);
			for(q=0; q < pcount[qnode->node_count]; q++) {
				perm(qnode->orgnodes, qnode->permnodes, qnode->node_count, q);
				rnode = get_nonleaf(treenode, 2);
				for(r=0; r < pcount[rnode->node_count]; r++) {
					perm(rnode->orgnodes,rnode->permnodes,rnode->node_count, r);
					snode = get_nonleaf(treenode, 3);
					for(s=0; s < pcount[snode->node_count]; s++) {
						perm(snode->orgnodes,snode->permnodes,
												snode->node_count, s);
						tcount++;
						mstart = 0;
						assign_bounds(treenode);
						if ( mstart  <= 32) {
							return(1);
								
						}

					}
				}
			}
		}
	}
	return(0);
}

void
update_bridge_nodes(int bridge)
{
	node_t	*tnode;
	int		i;
	
	tnode = get_node(treenode, bridge);
	if ( tnode->node_count == 0)
		return;
	tnode->mem_start = tnode->permnodes[0]->mem_start;
	for(i=0; i < tnode->node_count; i++) {
		tnode->mem_size += tnode->permnodes[i]->mem_size;
	}
}
void
update_pcislot_data_structure()
{
	node_t	*tnode;
	int		i, pci_slot_no;
	int		nonleaf =0;
	pcislot_t	*pcislotp;

	tnode = treenode;
	nonleaf = 0;
	do {
		for(i=0; i < tnode->node_count; i++) {
			if (tnode->permnodes[i]->leaf == 0)
				continue;
			pci_slot_no = get_pci_slot(tnode->permnodes[i]->pci_id);
			if ( pci_slot_no != -1) {
				pcislotp = &pcislot[pci_slot_no];
				pcislotp->card_type = tnode->permnodes[i]->node_type;
				pcislotp->pci_id = tnode->permnodes[i]->pci_id;
				pcislotp->pci_start = PCI_WINDOW_START + 
						tnode->permnodes[i]->mem_start * M(64);
				pcislotp->pci_size = tnode->permnodes[i]->mem_size * M(64);
				if ( pci_slot_no >= 2) {
					pcislotp->slotname[0] = 'A' + (char)((pci_slot_no - 2) / 4);
					pcislotp->slotname[1] = '1' + (char)((pci_slot_no - 2)% 4);
				} else {
					pcislotp->slotname[0] = '0';
					pcislotp->slotname[1] = '0' + (char)pci_slot_no;
				}
			}	
		}
		tnode = get_nonleaf(treenode, nonleaf);
		nonleaf++;
	} while(nonleaf < 5);
		
}

void
print_node(node_t *node)
{
	node_t	*tnode;
	int i;
	
	tnode = node;
	printf("Root: \n\r");
	for(i=0; i < tnode->node_count; i++) {
		printf("Type: %s,	", bname[tnode->permnodes[i]->node_type & 0x0F]);
		printf("Pci : %08X,	", tnode->permnodes[i]->pci_id);
		printf("Bus : %d,	", tnode->permnodes[i]->bus_num);
		printf("Size : %d,	", tnode->permnodes[i]->mem_size);
		printf("Start : %d	", tnode->permnodes[i]->mem_start);
		printf("Bus : %d,	", PCI_GET_BUS(tnode->permnodes[i]->pci_id));
		printf("Idsel : %d,	\n\r", PCI_GET_DEV(tnode->permnodes[i]->pci_id));
	}
	
	tnode = get_nonleaf(treenode, 0);
	printf("\n\r%s: \n\r", bname[tnode->node_type & 0x0F]);
	for(i=0; i < tnode->node_count; i++) {
		printf("Type: %s,	", bname[tnode->permnodes[i]->node_type & 0x0F]);
		printf("Pci : %08X,	", tnode->permnodes[i]->pci_id);
		printf("Bus : %d,	", tnode->permnodes[i]->bus_num);
		printf("Size : %d,	", tnode->permnodes[i]->mem_size);
		printf("Start : %d	", tnode->permnodes[i]->mem_start);
		printf("Bus : %d,	", PCI_GET_BUS(tnode->permnodes[i]->pci_id));
		printf("Idsel : %d,	\n\r", PCI_GET_DEV(tnode->permnodes[i]->pci_id));
	}

	tnode = get_nonleaf(treenode, 1);
	printf("\n\r%s: \n\r", bname[tnode->node_type & 0x0F]);
	for(i=0; i < tnode->node_count; i++) {
		printf("Type: %s,	", bname[tnode->permnodes[i]->node_type & 0x0F]);
		printf("Pci : %08X,	", tnode->permnodes[i]->pci_id);
		printf("Bus : %d,	", tnode->permnodes[i]->bus_num);
		printf("Size : %d,	", tnode->permnodes[i]->mem_size);
		printf("Start : %d	", tnode->permnodes[i]->mem_start);
		printf("Bus : %d,	", PCI_GET_BUS(tnode->permnodes[i]->pci_id));
		printf("Idsel : %d,	\n\r", PCI_GET_DEV(tnode->permnodes[i]->pci_id));
	}
	tnode = get_nonleaf(treenode, 2);
	printf("\n\r%s: \n\r", bname[tnode->node_type & 0x0F]);
	for(i=0; i < tnode->node_count; i++) {
		printf("Type: %s,	", bname[tnode->permnodes[i]->node_type & 0x0F]);
		printf("Pci : %08X,	", tnode->permnodes[i]->pci_id);
		printf("Bus : %d,	", tnode->permnodes[i]->bus_num);
		printf("Size : %d,	", tnode->permnodes[i]->mem_size);
		printf("Start : %d	", tnode->permnodes[i]->mem_start);
		printf("Bus : %d,	", PCI_GET_BUS(tnode->permnodes[i]->pci_id));
		printf("Idsel : %d,	\n\r", PCI_GET_DEV(tnode->permnodes[i]->pci_id));
	}
	tnode = get_nonleaf(treenode, 3);
	printf("\n\r%s: \n\r", bname[tnode->node_type & 0x0F]);
	for(i=0; i < tnode->node_count; i++) {
		printf("Type: %s,	", bname[tnode->permnodes[i]->node_type & 0x0F]);
		printf("Pci : %08X,	", tnode->permnodes[i]->pci_id);
		printf("Bus : %d,	", tnode->permnodes[i]->bus_num);
		printf("Size : %d,	", tnode->permnodes[i]->mem_size);
		printf("Start : %d	", tnode->permnodes[i]->mem_start);
		printf("Bus : %d,	", PCI_GET_BUS(tnode->permnodes[i]->pci_id));
		printf("Idsel : %d,	\n\r", PCI_GET_DEV(tnode->permnodes[i]->pci_id));
	}
		
	printf("\n\n\n\r");
	printf("Count %d\n\r", tcount);
}



void	
pci_configure()
{
#ifndef	PCI_STATIC
	int		i;
	pcislot_t	*pcislotp;
#endif

	bzero((char *)pcislot, sizeof(pcislot));
	if ( pci_allocate()) {
		update_bridge_nodes(NODE_BRIDGE_A);
		update_bridge_nodes(NODE_BRIDGE_B);
		update_bridge_nodes(NODE_BRIDGE_C);
		update_bridge_nodes(NODE_BRIDGE_D);
		update_pcislot_data_structure();
		
	} else {
		printf("Failed to allocate PCI space for the boards\n\r");
	}
	
#ifndef	PCI_STATIC
	for(i=0; i < MAX_PCI_SLOTS; i++) {
		pcislotp = &pcislot[i];
		if( pcislotp->pci_id ) {
			printf("Slot %s: Card %s, pci_id %08X, pstart %08X, psize %08X\n\r",
			pcislotp->slotname, bname[pcislotp->card_type & 0x0F], 
			pcislotp->pci_id, pcislotp->pci_start, pcislotp->pci_size);
		}
	}
#endif
}

void
print_slot(pcislot_t *pcislotp)
{
	int i;
	for(i=0; i < MAX_PCI_SLOTS; i++) {
		if( pcislotp->pci_id ) {
			printf("Slot %s: Card %s, pstart %08X, psize %08X\n\r",
			pcislotp->slotname, bname[pcislotp->card_type & 0x0F], 
			pcislotp->pci_start, pcislotp->pci_size);
		}
		pcislotp++;
	}
}
void
print_slot_only(pcislot_t *pcislotp)
{
	int i;
	for(i=0; i < MAX_PCI_SLOTS; i++) {
		if( pcislotp->pci_id ) {
			printf("Slot %s: Card %s\n\r",
			pcislotp->slotname, bname[pcislotp->card_type & 0x0F]);
		}
		pcislotp++;
	}
}
void
print_nodeinfo(node_t *tnode)
{
	U32		pci_id, secbus;
	int i;

	for(i=0; i < tnode->node_count; i++) {
		pci_id = tnode->permnodes[i]->pci_id;
		/* Read the Mmode Status */
		printf("Type: %s,	", bname[tnode->permnodes[i]->node_type & 0x0F]);
		printf("Pci : %08X,	", pci_id);
		printf("Cmd : %08X, ", pciconf_readw(pci_id, PCI_CONF_COMM));
		printf("Sts : %08X\n\r", pciconf_readw(pci_id, PCI_CONF_STAT));
		if ( (tnode->permnodes[i]->node_type == NODE_NIC) || 
		 			(tnode->permnodes[i]->node_type == NODE_RAC)) {
			secbus = pciconf_readb(pci_id, PCI_BCONF_SECBUS);
			/* Read Galileo Status */
			pci_id = get_pci_id ( secbus, GAL120_ID, 0);
			if ( pci_id ) {
				printf("       Galileo, Pci : %08X, ", pci_id);
				printf("Cmd : %08X, ", pciconf_readw(pci_id, PCI_CONF_COMM));
				printf("Sts : %08X\n\r", pciconf_readw(pci_id, PCI_CONF_STAT));
			}
			/* Read Q-Logic Status */
			pci_id = get_pci_id ( secbus, ISP_ID, 0);
			if ( pci_id ) {
				printf("       Q-logic, Pci : %08X, ", pci_id);
				printf("Cmd : %08X, ", pciconf_readw(pci_id, PCI_CONF_COMM));
				printf("Sts : %08X\n\r", pciconf_readw(pci_id, PCI_CONF_STAT));
			}
		}
	}
		
}

void
pcistatus_all()
{
	node_t	*tnode;
	
	/* Print Info about the Bridges and Galileo on HBC */
	tnode = treenode;
	printf("Root: \n\r");
	print_nodeinfo(tnode);

	/* Print Info about the Bridges and Galileo on Segment A */
	tnode = get_nonleaf(treenode, 0);
	printf("\n\r%s: \n\r", bname[tnode->node_type & 0x0F]);
	print_nodeinfo(tnode);

	/* Print Info about the Bridges and Galileo on Segment B */
	tnode = get_nonleaf(treenode, 1);
	printf("\n\r%s: \n\r", bname[tnode->node_type & 0x0F]);
	print_nodeinfo(tnode);

	/* Print Info about the Bridges and Galileo on Segment C */
	tnode = get_nonleaf(treenode, 2);
	printf("\n\r%s: \n\r", bname[tnode->node_type & 0x0F]);
	print_nodeinfo(tnode);

	/* Print Info about the Bridges and Galileo on Segment D */
	tnode = get_nonleaf(treenode, 3);
	printf("\n\r%s: \n\r", bname[tnode->node_type & 0x0F]);
	print_nodeinfo(tnode);

}
