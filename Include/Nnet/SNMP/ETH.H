/*@*********************************************************************** 
 |                                                                         
 |             Copyright (c) 1995-1997 XACT Incporated                     
 |                                                                         
 | PROPRIETARY RIGHTS of XACT Incorporated are involved in the subject     
 | matter of this material.  All manufacturing, reproduction, use, and     
 | sales rights pertaining to this subject matter are governed by the      
 | license agreement.  The recipient of this software implicitly accepts   
 | the terms of the license.                                               
 |                                                                         
 |                                                                         
 | FILE NAME   :                                        
 | VERSION     :      
 | COMPONENT   :
 | DESCRIPTION :                                                           
 | AUTHOR      :                                                            
 *************************************************************************/
#ifndef _ETH_H_
#define _ETH_H_

#define ETH_MTU		1500
#define ETH_FRAME	2048
#define ETH_STATS			"/proc/net/dev"
#define ETH_STATS_FORMAT	"%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu"
#define ETH_STATS_SIZE		11

typedef struct eth_descr_s	eth_descr_t;

struct eth_descr_s {
    ul32       Octets; 
    ul32       Pkts; 
    ul32       Pkts64Octets; 
    ul32       Pkts65to127Octets; 
    ul32       Pkts128to255Octets; 
    ul32       Pkts256to511Octets;
    ul32       Pkts512to1023Octets;
    ul32       Pkts1024to1518Octets;
    ul32       LostPkts;
    ul32       BroadcastPkts;
    ul32       MulticastPkts;
    ul32       CRCAlignErrors;         
    ul32       UndersizePkts;            
    ul32       OversizePkts;             
    ul32       Fragments;
    ul32       Jabbers;         
    ul32       BufferedPkts;
    ul32       StackedPkts;
    ul32       DiscardedPkts;
    ul32       TruncatedPkts; 
 	ul32	   OutPkts;
 	ul32       OutOctets;
 	ul32       OutDiscards;
    ul32       OutBroadcastPkts;
    ul32       OutMulticastPkts;
	/* extra interface parameters from kernel memory */
 	ul32	   InErrors;
 	ul32	   OutErrors;
 	ul32	   InDrops;
 	ul32	   OutDrops;
    ul32       Collisions;
};           

bool 	EthInit(void);
void    EthExit(void);

#endif
