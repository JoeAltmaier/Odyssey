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
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define PROT_TYPE          0
#define PROT_PKTUNKNOWN    0
#define PROT_PKTXSNMP      1
#define PROT_PKTETHERNET   2
#define PROT_PKTIP         3
#define PROT_PKTTCP        4
#define PROT_PKTUDP        5
#define PROT_PKTICMP       6
#define PROT_PKTARP        7
#define PROT_PKTRARP       8
#define PROT_PKTDNS        9


typedef union   prot_frame_u prot_frame_t;
typedef struct  prot_pkt_s	prot_pkt_t;

struct prot_pkt_s {
	u8				*Ptr;
	prot_frame_t	*Frame;
	prot_pkt_t		*Child;
	u16				DataLen;
	u16				ChildProt;
};

typedef struct prot_obj_s {
	u16				Level;
	l32				Id[SNMP_SIZE_OBJECTID];
	u8				*ProtStr;
	u8				*FieldStr;
	u16				Type;
	snmp_syntax_t	Syntax;
	u16				SyntaxLen;
} prot_obj_t;

typedef struct prot_ptr_s {
	i8				*Name;
	bool			(*Header)(prot_pkt_t *Pkt);
	bool			(*Field)(prot_pkt_t *Pkt, prot_obj_t *Obj);
	bool			(*Print)(prot_obj_t *Obj, u8 **StrPtr);
	i8				**String;
	u16				StringLen;
} prot_ptr_t;

typedef struct prot_xsnmp_s {
	u16				Type;                       /*  1 : SNMP_GAUGE */
	u16				IfIndex;                    /*  2 : SNMP_GAUGE */
	ul32			ID;                         /*  3 : ID */
	u16				Size;                       /*  4 : SNMP_GAUGE */    
	u16				Len;                        /*  5 : SNMP_GAUGE */    
	ul32			Time;                       /*  6 : SNMP_INTEGER */
	u8				*Data;                      /*  7 : SNMP_NULL */
	u16				Status;						/*  8 : SNMP_INTEGER */
} prot_xsnmp_t;                                

typedef struct prot_ethernet_s {
	u8				*DstAdr;                    /*  1  : SNMP_OCTETSTR */
	u8				*SrcAdr;                    /*  2  : SNMP_OCTETSTR */
	u8				*Type;                      /*  3  : SNMP_GAUGE */
	u8				*Data;                      /*  4  : SNMP_NULL */
} prot_ethernet_t;

typedef struct prot_ip_s {
	u8				*Vers;                      /*  1  : SNMP_GAUGE */
	u8				*HLen;                      /*  2  : SNMP_GAUGE */
	u8				*Service;                   /*  3  : SNMP_GAUGE */
	u8				*TLen;                      /*  4  : SNMP_GAUGE */
	u8				*ID;                        /*  5  : SNMP_GAUGE */
	u8				*Flags;                     /*  6  : SNMP_GAUGE */
	u8				*Fragment;                  /*  7  : SNMP_GAUGE */
	u8				*Time;                      /*  8  : SNMP_TIMETICKS */
	u8				*Type;                      /*  9  : SNMP_GAUGE */
	u8				*ChkSum;                    /*  10 : SNMP_GAUGE */
	u8				*IPSrc;                     /*  11 : SNMP_IPADDR */
	u8				*IPDst;                     /*  12 : SNMP_IPADDR */
	u8				*IPOption;                  /*  13 : SNMP_OCTETSTR */
	u8				*Data;                      /*  14 : SNMP_NULL */
} prot_ip_t;

typedef struct prot_arp_s {
	u8				*Hardw;                     /*  1  : SNMP_GAUGE */
	u8				*Type;                      /*  2  : SNMP_GAUGE */
	u8				*HLen;                      /*  3  : SNMP_GAUGE */
	u8				*PLen;                      /*  4  : SNMP_GAUGE */
	u8				*Operation;                 /*  5  : SNMP_GAUGE */
	u8				*HwSender;                  /*  6  : SNMP_OCTETSTR */
	u8				*ProtSender;                /*  7  : SNMP_OCTETSTR */
	u8				*HwTarget;                  /*  8  : SNMP_OCTETSTR */
	u8				*ProtTarget;                /*  9  : SNMP_OCTETSTR */
	u8				*Data;                      /*  10 : SNMP_NULL */
} prot_arp_t;                                         

typedef struct prot_udp_s {
	u8				*SrcPort;                   /*  1  : SNMP_GAUGE */
	u8				*DstPort;                   /*  2  : SNMP_GAUGE */
	u8				*MLen;                      /*  3  : SNMP_GAUGE */
	u8				*ChkSum;                    /*  4  : SNMP_GAUGE */
	u8				*Data;                      /*  5  : SNMP_NULL */
} prot_udp_t;

typedef struct prot_tcp_s {
	u8				*SrcPort;                   /*  1  : SNMP_GAUGE */
	u8				*DstPort;                   /*  2  : SNMP_GAUGE */
	u8				*SeqNr;                     /*  3  : SNMP_GAUGE */
	u8				*AckNr;                     /*  4  : SNMP_GAUGE */
	u8				*HLen;                      /*  5  : SNMP_GAUGE */
	u8				*Reserved;                  /*  6  : SNMP_GAUGE */
	u8				*Code;                      /*  7  : SNMP_GAUGE */
	u8				*Window;                    /*  8  : SNMP_GAUGE */
	u8				*ChkSum;                    /*  9  : SNMP_GAUGE */
	u8				*UrgePtr;                   /*  10 : SNMP_GAUGE */
	u8				*Option;                    /*  11 : SNMP_OCTETSTR */
	u8				*Data;                      /*  12 : SNMP_NULL */
} prot_tcp_t;

typedef struct prot_icmp_s {
	u8				*Type;                      /*  1  : SNMP_GAUGE */
	u8				*Code;                      /*  2  : SNMP_GAUGE */
	u8				*ChkSum;                    /*  3  : SNMP_GAUGE */
	u8				*Data;                      /*  4  : SNMP_NULL */
} prot_icmp_t;

typedef struct prot_dns_s {
	u8				*ID;                        /*  1  : SNMP_GAUGE */
	u8				*Parameter;                 /*  2  : SNMP_GAUGE */
	u8				*NroQuest;                  /*  3  : SNMP_GAUGE */
	u8				*NroAnsw;                   /*  4  : SNMP_GAUGE */
	u8				*NroAuth;                   /*  5  : SNMP_GAUGE */
	u8				*NroAdd;                    /*  6  : SNMP_GAUGE */
	u8				*Entry;                     /*  7,8,9,10  : variable */
	u8				*Data;                      /* 11  : SNMP_NULL */
} prot_dns_t;

union prot_frame_u {
	prot_xsnmp_t	Xsnmp;
	prot_ethernet_t	Ethernet;
	prot_ip_t		Ip;
	prot_tcp_t		Tcp;
	prot_udp_t		Udp;
	prot_icmp_t		Icmp;
	prot_arp_t		Arp;
	prot_dns_t		Dns;
};


#define PROT_NETWORD(Word, BytePtr)                    \
    (Word) = (ul32) (BytePtr)[0];                      \
    (Word) <<= 8;                                      \
    (Word) |= (ul32) (BytePtr)[1];

#define PROT_NETLWORD(LWord, BytePtr)                  \
    (LWord) = (ul32) (BytePtr)[0];                     \
    (LWord) <<= 24;                                    \
    (LWord) |= (ul32) (BytePtr)[1];                    \
    (LWord) <<= 16;                                    \
    (LWord) = (ul32) (BytePtr)[2];                     \
    (LWord) <<= 8;                                     \
    (LWord) |= (ul32) (BytePtr)[3];


void    ProtFree(prot_pkt_t *Pkt);
void    ProtExit(void);
bool 	ProtFrame(prot_pkt_t *Pkt, u8 *frame, u16 size, u16 length, 
				ul32 ptime, u16 status, u16 type, u16 pindex, ul32 id);
bool 	ProtFile(prot_pkt_t *Pkt, u8 *PktPtr);
bool 	ProtGetField(prot_pkt_t *Pkt, prot_obj_t *Obj);
bool 	ProtStringNr(prot_obj_t *Obj);
bool 	ProtPrint(prot_obj_t *Obj, u8 **StrPtr);

#endif
