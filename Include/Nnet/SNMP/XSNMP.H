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
#ifndef _XSNMP_H
#define _XSNMP_H

#define UNICAST						0 
#define BCAST						1
#define MCAST						2

#define STATISTICS_ERROR            1000L
#define EVENT_ERROR                 9000L
#define ALARM_ERROR                 8000L
#define CHANNEL_ERROR               4000L
#define FILTER_ERROR                3000L
#define CAPTURE_ERROR               5000L
#define HOST_ERROR                  11000L
#define MATRIX_ERROR                13000L
#define TOPN_ERROR                  12000L
#define HISTORY_ERROR               14000L

i8 		*Inet_NtoA(ul32 addr);
l32  	mem2long(u8 *mem);
void 	long2mem(u8 *mem, l32 val);
ul32  	mem2ulong(u8 *mem);
void  	ulong2mem(u8 *mem, u32 val);
u16   	mem2word(u8 *mem);
void  	word2mem(u8 *mem, u16 val);
ul32  	mem2dword(u8 *mem);
void  	dword2mem(u8 *mem, ul32 val);
ul32  	mem2lword(u8 *mem);
void  	lword2mem(u8 *mem, ul32 val);
i32   	mem2int(u8 *mem);
void  	int2mem(u8 *mem, i32 val);
u32   	mem2uint(u8 *mem);
void  	uint2mem(u8 *mem, u32 val);
i32 	strcnt(c_i8* str, c_i8 c);
ul32 	TimeUsec(void);
ul32 	TimeMsec(void);
ul32 	TimeTimeTicks(void);

#define difftime(a,b)	(double)((b)-(a))

#define __(a) a
#define VAR_PROTO       , ...
#define VAR_ARGS        
#define VAR_DCL
#define VAR_LIST        va_list
#define VAR_END(va)     va_end(va)
#define VAR_START(va,f) va_start(va,f)

#ifdef XSTK_BSD 
#define	AF_INET				2				/* internetwork: UDP, TCP, etc. */
#define	SOCK_DGRAM			2				/* datagram socket */
#define	INADDR_ANY			(ul32)0x0
#endif

#define ETHER_HEADER_SIZE	12
#define PKT_QUEUE_FULL		1
#define PKT_QUEUED			0
#define PKT_HEADER			1
#define PKT_POINTER			2

typedef struct pkt_queue_s {
	struct pkt_queue_s	*pqn;
	u8					pktt[ETHER_HEADER_SIZE];	
	u8					*pktp;
	u8					port;
	u8					kind;
	u16					size;
} pkt_queue_t;

typedef struct free_struct {
	struct free_struct *nxt;				/* pointer to next region      */
	i32 				isize;				/* initial size of free region */
	i32 				size;				/* size of free region         */
	i32 				Psize;				/* page size in bytes          */
	i32 				Pmask;				/* logical page mask           */
} freem_t;

#define FREEP(x)		((freem_t *)(x))

#define PAGE_SIZE		0x10

#define FSIZE(f)		(f->Psize - 1)
#define ROUNDUP(f,x)	((x + (f->Psize - 1)) & f->Pmask)
#define ROUND(f,x)		(x & f->Pmask)

typedef struct udpsocks_s {
	u8				address[16];
	u32				port;
} udp_socks_t;

#define MAX_UDP_SOCKS	16

typedef struct udpmstats_s {
	ul32			udpInDatagrams;
	ul32			udpNoPorts;
	ul32			udpInErrors;
	ul32			udpOutDatagrams;
	udp_socks_t		udpAddrPorts[MAX_UDP_SOCKS];
} udp_stats_t;

/*
 * Receive Frame Information template
 * If the ri_stat field is non-zero, there is viable info there;
 */
typedef struct rip_s {
	u16				rip_used;		/* This rip is being used if not zero 	 */
    u16             rip_stat;    	/* Status as reported in the descriptor  */
    u16             rip_size;    	/* Length of the packet                  */
    u16             rip_seq;     	/* Frame sequence number                 */
	u16				rip_unit;		/* the unit this rip came from			 */
    u16             rip_Status;  	/* For RMON Capture group entry          */
    u16             rip_Len;     	/* For RMON Capture group entry          */
    ul32            rip_ID;      	/* For RMON Capture group entry          */
    ul32            rip_Time;    	/* For RMON Capture group entry          */
    u8             *rip_pktp;  		/* pointer to this packet for this RIP.  */
	u8				rip_buff[2048]; /* storage for the packet 				 */
} rip_t;

#endif
