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
#ifndef _IP_H_
#define _IP_H_

#define IP_ADDR_SIZE				4

#define IP_PROT_ICMP    			1
#define IP_PROT_GGP     			3
#define IP_PROT_TCP     			6
#define IP_PROT_EGP     			8
#define IP_PROT_IGP     			9
#define IP_PROT_CHAOS   			16
#define IP_PROT_UDP     			17
#define IP_PROT_TP4     			29
#define IP_ADDR_THIS    			0L
#define IP_TOS_PREC_MASK		    0x07
#define IP_TOS_PREC_ROUTINE	      	0x00
#define IP_TOS_PREC_PRIORITY		0x01
#define IP_TOS_PREC_IMMEDIATE		0x02
#define IP_TOS_PREC_FLASH		    0x03
#define IP_TOS_PREC_FLASH_OVERIDE	0x04
#define IP_TOS_PREC_CRITIC		    0x05
#define IP_TOS_PREC_CTRL		    0x06
#define IP_TOS_PREC_CTRL_NETWORK	0x07
#define	IP_TOS_LOW_DELAY		    0x08
#define	IP_TOS_HIGH_TROUGHPUT		0x10
#define	IP_TOS_HIGH_RELIABILITY 	0x20
#define IP_FLAG_RESERVED            0x04
#define IP_FLAG_DONT_FRAGMENT       0x02
#define IP_FLAG_MORE_FRAGMENTS      0x01
#define IP_OPTION_END               0x00
#define IP_OPTION_NOP               0x01
#define IP_OPTION_SECURITY          0x02
#define IP_OPTION_ROUTE_LOOSE       0x03
#define IP_OPTION_ROUTE_RECORD      0x07
#define IP_OPTION_STREAM            0x08
#define IP_OPTION_ROUTE_STRICT      0x09
#define IP_OPTION_TIME_STAMP        0x44
#define IP_OPTION_COPY              0x80
#define IP_ROUTE_TYPE_OTHER    		1
#define IP_ROUTE_TYPE_INVALID  		2
#define IP_ROUTE_TYPE_DIRECT   		3
#define IP_ROUTE_TYPE_REMOTE   		4
#define IP_ROUTE_PROTO_OTHER   		1
#define IP_ROUTE_PROTO_LOCAL   		2
#define IP_ROUTE_PROTO_MGMT    		3
#define IP_ROUTE_PROTO_ICMP    		4
#define IP_ROUTE_PROTO_EGP     		5
#define IP_ROUTE_PROTO_GGP     		6

typedef struct  ip_prot_s			ip_prot_t;
typedef struct  ip_hdr_s			ip_hdr_t;
typedef struct  ip_iface_s			ip_iface_t;

typedef struct ip_route_entry_s		ip_route_entry_t;
typedef struct ip_route_route_s		ip_route_route_t;

struct ip_prot_s {
	bool		(*Rcve)(link_t *link, ip_hdr_t *ipHdr);
	u8			prot;
	ip_prot_t	*next;
};

struct ip_hdr_s {
    u8        	ver;
    u16       	ihl;
    u8	    	tos;
    u16        	length;
	u16	    	id;
	u8	    	flags;
	u16        	offset;
	u8        	ttl;
	u8        	prot;
	u16        	check;
	ul32       	src;
	ul32       	dst;
	u16        	iol;
	u8        	*options;
};

struct ip_iface_s {
	mac_iface_t	*iface;
	ul32		addr;
	ul32		mask;
	bool		up;
	ul32		addrNet;
	ul32		addrBroadcast;
	arp_descr_t	*arp;
	ip_iface_t	*next;
};

struct ip_route_entry_s {
	ip_iface_t	*iface;
	ul32		addr;
	ul32		addrNextHop;
	ul32		mask;
	u16			metric;
	u16			routingProto;
	u16			type;
	ip_route_entry_t  *next;
};

struct ip_route_route_s {
	ip_iface_t	*iface;
	ul32		addr;
	u16        		metric;
	bool     		broadcast;
};

bool 		IpRouteIsBroadcast(ul32 addr);
bool 		IpRouteFind(ul32 addr, ip_route_route_t *route);
bool     	IpInit(void);
u16      	IpH2NWord(u16 w);
u16      	IpN2HWord(u16 w);
ul32     	IpH2NDWord(ul32 d);
ul32     	IpN2HDWord(ul32 d);
u16      	IpHdrCheck(link_t *link, u16 length);
bool     	IpIfaceRegister(ip_iface_t *iface);
void     	IpIfaceRemove(ip_iface_t *iface);
void     	IpIfaceSwitch(ip_iface_t *iface, bool up);
bool     	IpRouteAdd( ul32 addr, ul32 mask, ul32 addrNextHop, mac_iface_t *iface, u16 metric);
bool     	IpProtRegister(ip_prot_t *prot);
void     	IpProtRemove(ip_prot_t *prot);
bool     	IpSend(link_t *link, ip_hdr_t *ipHdr);

#endif
