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
#ifndef _ARP_H_
#define _ARP_H_

#define ARP_ENTRY_TIMEOUT       300000L
#define ARP_REQUEST_TIMEOUT     2000L
#define ARP_REQUEST_RETRIES     3
#define ARP_LENGTH_PROT 16
#define ARP_LENGTH_HARD 6

typedef struct arp_descr_s		arp_descr_t;
typedef struct arp_request_s	arp_request_t;
typedef struct arp_que_s   		arp_que_t;
typedef struct arp_entry_s 		arp_entry_t;
typedef void (*arp_reply_t)(mac_iface_t *iface, arp_entry_t *entry, void *specific);

struct arp_descr_s {
	mac_iface_t		*iface;
	u16				prot;
	u8				addr[ARP_LENGTH_PROT];
	u16				addrLength;
	arp_entry_t		*entryList;
	arp_request_t	*requestList;
	arp_descr_t		*next;
};

struct arp_request_s {
	arp_descr_t		*descr;
	u8				addr[ARP_LENGTH_PROT];
	u16				retry;
	timer_descr_t	*timer;
	arp_que_t		*queList;
	arp_request_t	*next;
};

struct arp_que_s {
	arp_reply_t		Reply;
	void			*specific;
	arp_que_t		*next;
};

struct arp_entry_s {
	u8				addrProt[ARP_LENGTH_PROT];
	u8				addrHard[ARP_LENGTH_HARD];
	bool			dynamic;
	bool			update;
	arp_entry_t		*next;
};

#endif



