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
#ifndef _AGENT_H_
#define _AGENT_H_

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#define AGENT_BUFFER_SIZE		1472	/* 1500 - 8 (UDP) - 20 (IP) */
#define AGENT_COMMUNITY_SIZE    32/*255*/
#define AGENT_LIST_SIZE         32
#define AGENT_ADDR_SIZE 		16

typedef struct agent_stat_s 	agent_stat_t;
typedef struct agent_comm_s 	agent_comm_t;
typedef struct agent_host_s 	agent_host_t;
typedef struct agent_iface_s  	agent_iface_t;

struct agent_stat_s {
	ul32			InTotalReqVars;
	ul32			InTotalSetVars;
	ul32			InBadCommunityNames;
	ul32			InBadCommunityUses;
	ul32   			datagrams;
	ul32   			errors;
};

struct agent_iface_s {               
	u32     		number;
	u32     		stats;          /* collector, 0=off, 1=on             */
	u32				type;			/* type of interface 				  */
	u32     		history;        /* collector, 0=off, 1=on             */
	u32     		buckets;        /* if history on, then the bucket     */
	u32     		interval;       /* if history on, then the interval   */
	u32     		host;           /* collector, 0=off, 1=on             */
	u32     		matrix;         /* collector, 0=off, 1=on             */
	u32     		info;           /* collector, 0=off, 1=on             */
	agent_iface_t	*next;			/* next mac iface                     */
};              
 
struct agent_comm_s {    
	i32				index;
	i32				mode;			/* 1=public,2=secret,3=trap,4=all,5=empty */
	i32				access;			/* 1=read-only, 2=read-write              */
	u8				comm[255];
	u16				commLen;
	agent_host_t	*hostList;
	agent_comm_t	*next;
};

struct agent_host_s {
	ul32			addr;
	agent_host_t	*next;
};

bool 	 			AgentInit(u32 port);
agent_stat_t		*AgentStatistics(void);
bool      			AgentGetAuthenTraps(void);
void         		AgentSetAuthenTraps(bool enable);
bool      			AgentSendTrap(u8 *comm, u16 commLen, ul32 systime, u16 gen, u16 spec, 
						snmp_object_t *list, u16 listLen);
agent_comm_t		*AgentCommunity(i32 aindex);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif
