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
#ifndef _XCFIG_H
#define _XCFIG_H

#define XSNMP_MAX_STR			32
#define MAX_SYS_STRINGS			48
#define SYSTEM_NAME_LENGTH 		48
#define PASSWORD_SIZE			16
#define XSNMP_HOSTNAME_LEN		16
#define XSNMP_COMMNAME_LEN		16
#define DATESTRLENGTH 			18
#define HOST_ADDR_LEN			4
#define XSNMP_MIBLEN			8
#define XSNMP_BUFSIZE			(8*1024)

#define XSNMP_HZ				1000		/* 1 second interval */

#define XSNMP_COMM_PUBLIC		1
#define XSNMP_COMM_SECRET		2
#define XSNMP_COMM_TRAP  		3
#define XSNMP_COMM_ALL  		4
#define XSNMP_COMM_EMPTY		5

#define XSNMP_READ_ONLY			1
#define XSNMP_WRITE_ONLY		2
#define XSNMP_READ_WRITE		3


typedef struct pinfo_s {
	i8		name[XSNMP_HOSTNAME_LEN+1];
	u32		hostid;
	u32		netmask;
} pinfo_t;

typedef struct snmphost_s {
	i8		host_name[XSNMP_HOSTNAME_LEN+1];
	i8		host_comm[XSNMP_COMMNAME_LEN+1];
	ul32	host_ip;
} snmphost_t;

typedef struct comm_s {
	i8		comm_name[XSNMP_COMMNAME_LEN+1];
	u32		comm_get;
	u32		comm_set;
	u32		comm_trap;
} comm_t;

typedef struct xsnmp_cfig_s {
	/* XSNMP System Group */
	i8				sys_contact[XSNMP_MAX_STR];	    /* Company name       */
	i8				sys_description[XSNMP_MAX_STR];	/* XSNMP, sys desc    */
	i8				sys_location[XSNMP_MAX_STR];	/* here, place of sys */
 	i8				sys_name[XSNMP_MAX_STR];	    /* IP host name       */
	u8				sys_objectid[XSNMP_MAX_STR];	/* Enterprise MIB ID  */
	u32				sys_services;					/* MIBII_SERVICES     */

	/* XSNMP Traps */
	u8				trapComm[XSNMP_MAX_STR];		/* trap community name*/
	u32				trapLen;						/* trap name length   */
	u32				authentrap_enable;				/* enable authen traps*/
	u32				coldtrap_enable;				/* enable cold traps */

	/* XSNMP Ports */
	u32				local_port;						/* XSNMP_PORT         */

#ifdef XMIB_RMON1
	/* XRMON1 (RFC1757) Control */
	u32				cbuff_size;						/* 256k               */
	u32				host_maxnrhosts;				/* 2000               */
	u32				matrix_maxnrsrcdsts;			/* 5000               */
	u32				hist_maxnrbuckets;				/* 2000               */
	u32				event_maxnrlogs;				/* 1000               */
	u32				disc_maxnrnodes;				/* 8000               */
	u32				disc_nodetimeout;				/* 10000              */
	u32				topn_maxnrentries;				/* 100                */
#endif
	agent_comm_t	*comm_list;						/* with matched hosts */
	mac_iface_t    	*mac_iface;         			/* macs, description  */
} xsnmp_cfig_t;

#endif
