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
#ifndef __MAC_H__
#define __MAC_H__

#define MAC_ARP_NONE           		0
#define MAC_ARP_ETHERNET_10MB  		1
#define MAC_ARP_ETHERNET_3MB   		2
#define MAC_ARP_AMATEUR_AX25   		3
#define MAC_ARP_PROTEON        		4
#define MAC_ARP_CHAOS          		5
#define MAC_ARP_IEEE_802       		6
#define MAC_ARP_ARCNET         		7

#define MAC_TYPE_OTHER              1
#define MAC_TYPE_REGULAR_1822       2
#define MAC_TYPE_HDH_1822           3
#define MAC_TYPE_DDN_X25            4
#define MAC_TYPE_RFC877_X25         5
#define MAC_TYPE_ETHERNET_CSMACD    6
#define MAC_TYPE_88023_CSMACD       7
#define MAC_TYPE_88024_TOKENBUS     8
#define MAC_TYPE_88025_TOKENRING    9
#define MAC_TYPE_88026_MAN          10
#define MAC_TYPE_SOFT_LOOPBACK      24

#define MAC_BIT_OVERSIZE			0
#define MAC_BIT_UNDERSIZE			1
#define MAC_BIT_CRCALIGN			2
#define MAC_BIT_DROPS				3
#define MAC_BIT_ORDER				4

#define MAC_STATUS_OVERSIZE			(1 << MAC_BIT_OVERSIZE)
#define MAC_STATUS_UNDERSIZE		(1 << MAC_BIT_UNDERSIZE)
#define MAC_STATUS_CRCALIGN			(1 << MAC_BIT_CRCALIGN)
#define MAC_STATUS_DROPS			(1 << MAC_BIT_DROPS)
#define MAC_STATUS_ORDER			(1 << MAC_BIT_ORDER)

#define MAC_PROT_IP                 0x0800
#define MAC_PROT_ARP                0x0806

#define MAC_ETHER_ADDR_LEN			6

typedef struct mac_ethstats_s		mac_ethstats_t;
typedef struct mac_iface_s 			mac_iface_t;
typedef struct mac_type_s  			mac_type_t;
typedef struct mac_prot_s  			mac_prot_t;
typedef struct mac_coll_s  			mac_coll_t;
typedef struct mac_hdr_s     		mac_hdr_t;
typedef struct mac_stat_s    		mac_stat_t;
typedef struct mac_info_s    		mac_info_t;
typedef struct mac_perf_s  			mac_perf_t;

struct mac_ethstats_s {
/*
 * Support for RFC1757, EtherStats Group
 */
    ul32    Octets; 
    ul32    Pkts; 
    ul32    BroadcastPkts;
    ul32    MulticastPkts;
    ul32    CRCAlignErrors;         
    ul32    UndersizePkts;            
    ul32    OversizePkts;             
    ul32    Fragments;
    ul32    Jabbers;         
    ul32    Collisions;
    ul32    Pkts64Octets; 
    ul32    Pkts65to127Octets; 
    ul32    Pkts128to255Octets; 
    ul32    Pkts256to511Octets;
    ul32    Pkts512to1023Octets;
    ul32    Pkts1024to1518Octets;
/*
 * Support for RFC1757, Host, Stats, Matrix groups
 */
    ul32    LostPkts;
    ul32    DiscardedPkts;
 	ul32	OutPkts;
 	ul32    OutOctets;
    ul32    OutMulticastPkts;
    ul32    OutBroadcastPkts;
 	ul32	OutErrors;
/*
 * Support for RFC1213, Interfaces Group
 */
    ul32    outQLen;
    ul32    outErrors;
    ul32    outDiscards;
    ul32    outNUcastPkts;
    ul32    outUcastPkts; 
	ul32	outOctets;
	ul32	inUcastPkts;
	ul32	inNUcastPkts;
	ul32	inUnknownProtos;
	ul32	inErrors;
	ul32	inDiscards;
	ul32	inOctets;
};           

struct mac_iface_s {
    u8        		descr[48];
    u16        		type;
    u16        		index;
    u16        		mtu;
    ul32       		speed;
    bool     		statusAdmin;
    bool     		statusOper;
    u16        		arp;
    u8        		addr[16];
    u8        		addrBroadcast[16];
    u16        		addrLength;   
    ul32       		frameId;
	/*
 	 * mibextensions:
	 * a field which indicates which mib extensions are "on".
	 * None have been defined so far.
	 */
	u8				addrHost[8];
	u8				pad;
	u16				addrHostLen;
    mac_ethstats_t 	*eth;
    mac_iface_t   	*next;
	void			*hostcontrol;
	void			*matrixcontrol;
	void			*channel;
};

struct mac_type_s {
	link_t				*(*Encode)(link_t *link, mac_hdr_t *hdr);
	link_t				*(*Decode)(link_t *link, mac_hdr_t *hdr);
	u16					type;
	struct mac_type_s	*next;
};


struct mac_prot_s {
    bool     			(*Rcve)(mac_iface_t *iface, link_t *link, mac_hdr_t *hdr);
    u16        			type;
    struct mac_prot_s 	*next;
};


struct mac_coll_s {
	bool		(*Rcve)(struct mac_coll_s *coll, prot_pkt_t *pkt);
	u16			ifindex;			
	void		*specific;
	mac_coll_t	*next;
};

struct mac_hdr_s {
	u8			*src;
	u8			*dst;
	u16			type;
	bool		broadcast;
};

struct mac_stat_s {
    ul32           inPkts;
    ul32           inOctets;
    ul32           inUcastPkts;
    ul32           inNUcastPkts;
    ul32           inDiscards;
    ul32           inErrors;
    ul32           inUnknownProtos;
    ul32           outPkts;
    ul32           outOctets;
    ul32           outUcastPkts;
    ul32           outNUcastPkts;
    ul32           outDiscards;
    ul32           outErrors;
    ul32           outQLen;
	ul32		   ifOverflows;
    ul32           LostPkts;
    ul32           Octets;
    ul32           Pkts; 
    ul32           BroadcastPkts; 
    ul32           MulticastPkts;
    ul32           CRCAlignErrors;                 
    ul32           UndersizePkts;                  
    ul32           OversizePkts;                   
    ul32           Fragments;                      
    ul32           Jabbers;                        
    ul32           Collisions; 
    ul32           BufferedPkts; 
    ul32           DiscardedPkts;
    ul32           StackedPkts;
    ul32           TruncatedPkts;
};

struct mac_info_s {
	rip_t			*rip;
	ul32			time;
	u16				length;
	u16				copied;
	bool			promiscuous;
	u16				status;
};

struct mac_perf_s {
	bool			on;
	ul32			pkts;
	ul32			octets;
	ul32			timeTotal;
	ul32			timeMin;
	ul32			timeMax;
};

bool     		MacInit(void);
bool     		MacIfaceRegister(mac_iface_t *iface); 
bool     		MacIfaceRemove(mac_iface_t *iface);
bool     		MacTypeRegister(mac_type_t *mac); 
bool     		MacTypeRemove(mac_type_t *mac);
bool     		MacProtRegister(mac_prot_t *prot); 
bool     		MacProtRemove(mac_prot_t *prot);
bool     		MacCollRegister(mac_coll_t *coll);
bool     		MacCollRemove(mac_coll_t *coll);
bool     		MacSend(mac_iface_t *iface, link_t *link, mac_hdr_t *hdr);
bool     		MacRcve(mac_iface_t *iface, link_t *link, mac_info_t *info);
u16        		MacIfaceCount(void);
bool     		MacIfaceCheck(void);
mac_iface_t   	*MacIfaceFind(u8 *descr);
mac_iface_t   	*MacIfaceGet(u16 mindex);
bool     		MacStatistics(mac_iface_t *iface, mac_stat_t *stats);
bool     		MacPerfStatistics(mac_perf_t *perf);
bool     		MacPerfSwitch(bool on);
bool			MacUndersize(u16 type, u16 len);
bool			MacOversize(u16 type, u16 len);

#endif
