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
#ifndef __UDP_H__
#define __UDP_H__

#define UDP_PORT_ANY	0
#define UDP_ADDR_ANY    0xffffffffL

typedef struct udp_descr_s	udp_descr_t;
typedef struct udp_hdr_s	udp_hdr_t;
typedef struct udp_stat_s	udp_stat_t;

struct udp_descr_s {
	bool    	(*Rcve)(udp_descr_t *descr, link_t *link, udp_hdr_t *udpHdr, ip_hdr_t *ipHdr);
	u16	    	locPort;
	ul32    	locAddr;
	u16    		remPort;
	ul32    	remAddr;
	void    	*specific;
	udp_descr_t	*next;   
	nw_dg_t 	*nw;  
};

struct udp_stat_s {
	ul32   		noPorts;
	ul32   		inDatagrams;
	ul32   		inErrors;
	ul32   		outDatagrams;
	ul32   		outErrors;
};

struct udp_hdr_s {
    u16    		src;
    u16    		dst;
    u16    		length;
    u16    		check;
};

bool UdpInit(void);
bool UdpRegister(udp_descr_t *udp);
bool UdpRemove(udp_descr_t *udp);
bool UdpSend(link_t *link, udp_hdr_t *udpHdr, ip_hdr_t *ipHdr, udp_descr_t *descr);
u16  UdpAnyPort(void);

#endif
