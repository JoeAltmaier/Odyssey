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
#ifndef _IFUDP_H_
#define _IFUDP_H_

#define MAC_TYPE_OTHER              	1
#define MAC_TYPE_REGULAR_1822       	2
#define MAC_TYPE_HDH_1822           	3
#define MAC_TYPE_DDN_X25            	4
#define MAC_TYPE_RFC877_X25         	5
#define MAC_TYPE_ETHERNET_CSMACD    	6
#define MAC_TYPE_88023_CSMACD       	7
#define MAC_TYPE_88024_TOKENBUS     	8
#define MAC_TYPE_88025_TOKENRING    	9
#define MAC_TYPE_88026_MAN          	10
#define MAC_TYPE_SOFT_LOOPBACK      	24
#define MAC_MTU_ETHERNET_CSMACD			1500
#define MAC_MTU_88023_CSMACD			1500
#define MAC_SPEED_ETHERNET_CSMACD		10000000
#define MAC_SPEED_88023_CSMACD			10000000
#define MAC_ADDRLEN_ETHERNET_CSMACD		6
#define MAC_ADDRLEN_88023_CSMACD		6
#define NW_UDP_BUFFER_SIZE 				1500

typedef struct NwStat_s {
	ul32 InPkts;
	ul32 InOctets;
    ul32 InDiscards;
    ul32 InErrors;
    ul32 InUnknownProtos;
    ul32 OutPkts;
    ul32 OutOctets;
    ul32 OutDiscards;
    ul32 OutErrors;
    ul32 OutQLen;
} NwStat_t;

typedef struct nw_dg_s				nw_dg_t;
typedef struct nw_co_s				nw_co_t; 
typedef struct nw_ac_s				nw_ac_t; 
typedef struct nw_dg_iface_s		nw_dg_iface_t;
typedef struct nw_co_iface_s		nw_co_iface_t;
typedef struct nw_ifconfig_s		nw_ifconfig_t;
typedef void  (*nw_dg_recv_call_t)	(nw_dg_t *dg, i8 *addr, u8 *frame, i32 length, void *parm);
typedef void  (*nw_co_recv_call_t)	(nw_co_t *co, u8 *frame, i32 length, void *parm);
typedef void* (*nw_co_open_call_t)	(nw_co_t *co, i8 *addr, void *parm);
typedef void  (*nw_co_close_call_t)	(nw_co_t *co, void *parm);

struct nw_dg_iface_s {
	i8			*descr;
	bool		(*init)      (void); 
	bool		(*exit)      (void); 
	nw_dg_t		*(*accept)   (i8 *addr, nw_dg_recv_call_t rcv, void *parm); 
	nw_dg_t		*(*connect)  (i8 *addr, nw_dg_recv_call_t rcv, void *parm);
	bool		(*send)      (nw_dg_t *dg, u8 *frame, i32 length);  
	bool		(*sendto)    (nw_dg_t *dg, i8 *addr, u8 *frame, i32 length);  
	bool		(*close)     (nw_dg_t *dg);
	bool		(*ifconfig)  (nw_dg_t *dg, nw_ifconfig_t *ifconfig);
	NwStat_t	*(*statistics)(i8 *addr);
};

struct nw_co_iface_s {
	i8			*descr;
	bool		(*init)    (void); 
	bool		(*exit)    (void); 
	nw_ac_t		*(*accept)  (i8 *addr, nw_co_recv_call_t rcv, nw_co_close_call_t clse, nw_co_open_call_t opn, void *parm);
	nw_co_t		*(*connect) (i8 *addr, nw_co_recv_call_t rcv, nw_co_close_call_t clse, void *parm);
	bool		(*send)    (nw_co_t *co, u8 *frame, i32 length);
	bool		(*close)   (nw_co_t *co);
	bool		(*release) (nw_ac_t *ac);
	bool		(*ifconfig)(nw_co_t *co, nw_ifconfig_t *ifconfig);
	NwStat_t	*(*statistics)(i8 *addr);
};

struct nw_dg_s {
	nw_dg_iface_t		*iface;   
	i8					addr[256];
	nw_dg_recv_call_t	recv;
	void				*parm;
};

struct nw_co_s {            
	nw_co_iface_t		*iface;
	i8					addr[256];
	nw_co_recv_call_t	recv; 
	nw_co_close_call_t	close;
	void				*parm;
};

struct nw_ac_s {            
	nw_co_iface_t		*iface;
	i8					addr[256];
	nw_co_recv_call_t	recv; 
	nw_co_close_call_t	close;
	nw_co_open_call_t	open;
	void				*parm;  
};

struct nw_ifconfig_s {
	i32					type;
	i32					mtu;
	i32					speed;
	u8					address[16];
	u8					broadcast[16];
	u16					addrlen;
};

i32 IpCheckSum(u16 *ptr, i32 nbytes);

extern nw_dg_iface_t nwUdpIface;

bool			NwInit(void); 
bool			NwExit(void);
NwStat_t		*NwDgStatistics(i8 *addr);
NwStat_t		*NwCoStatistics(i8 *addr);
nw_dg_t			*NwDgAccept  (i8 *addr, nw_dg_recv_call_t rcv, void *parm);
nw_dg_t			*NwDgConnect (i8 *addr, nw_dg_recv_call_t rcv, void *parm);
bool  			NwDgSend(nw_dg_t *dg, u8 *frame, i32 length);
bool            NwDgSendTo( i8 *addr, u8 *frame, i32 length, u16 src_prt );
bool			NwDgClose(nw_dg_t *dg);
bool			NwDgIfConfig(nw_dg_t *dg, nw_ifconfig_t *ifconfig);
nw_ac_t			*NwCoAccept(i8 *addr, nw_co_recv_call_t rcv, nw_co_close_call_t clse, nw_co_open_call_t opn, void *parm);
nw_co_t			*NwCoConnect(i8 *addr, nw_co_recv_call_t rcv, nw_co_close_call_t clse, void *parm);
bool			NwCoSend(nw_co_t *co, u8 *frame, i32 length);
bool			NwCoClose(nw_co_t *co);
bool			NwCoRelease(nw_ac_t *ac);
bool			NwCoIfConfig(nw_co_t *co, nw_ifconfig_t *ifconfig);

#endif


