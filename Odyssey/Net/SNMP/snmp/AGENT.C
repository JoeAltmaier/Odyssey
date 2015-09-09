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
 | FILE NAME   : agent.c
 | VERSION     : 1.1
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : SNMP agent functions
 | AUTHOR      : Robert Winter
 *************************************************************************/
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"
#include "link.h"
#include "ifudp.h"
#include "snmp.h"
#include "mib.h"
#include "xsnmp.h"
#include "agent.h"
#include "prott.h"
#include "mac.h"
#include "timer.h"
#include "xarp.h"
#include "ipp.h"
#include "udp.h"
#include "xcfig.h"
#include "1213sys.h"

static 	bool      		RcveUdp(udp_descr_t *descr, link_t *link, udp_hdr_t *udpHdr, ip_hdr_t *ipHdr);
static 	bool      		Request(u8 *rqs, u32 rqsLen, u8 **rsp, u32* rspLen, ul32 addr);
static 	agent_comm_t 	*CommFind(u8 *comm, u16 commLen);
static 	bool      		HostInComm(u8 *comm, u16 commLen, ul32 addr);
static 	agent_stat_t 	agentStat;
void 	SendENTTrap( u16 gen, u16 spec, snmp_object_t *list, u16 listLen );

extern	u32				get_hostid(void);
extern	bool			CommIsTrap(i32 unit);
extern	i8				*GetCommName(i32 unit);
extern	u32				GetCommLen(i32 unit);
extern	xsnmp_cfig_t	xsnmp_cfg;

udp_descr_t agentUdp = {
    RcveUdp,
    161,
    UDP_ADDR_ANY,
    UDP_PORT_ANY,
    UDP_ADDR_ANY,
    0
};

void
ClearComms()
{
agent_comm_t	*comm;

	for( comm=xsnmp_cfg.comm_list; comm!=0; comm=comm->next ) {
		x_free(comm);
	}
	xsnmp_cfg.comm_list = 0;
}

void
AddComm( i8 *name, i32 len, i32 access )
{
agent_comm_t	*comm, *icomm, *lcomm;
i32 i = 0;

	icomm = xsnmp_cfg.comm_list;
	if( !icomm ) {
		icomm = (agent_comm_t *)x_malloc(sizeof(agent_comm_t));
		if( icomm ) {
			xsnmp_cfg.comm_list = icomm;
			icomm->index = 1;
			x_bcopy( name, (i8 *)icomm->comm, len );
			icomm->commLen = len;
			icomm->access = access;
			icomm->mode = 0;
			icomm->next = 0;
			icomm->hostList = 0;
		} else {
			x_dbg("XSNMP, AddComm, no memory\n", TRUE);
		}
	} else {
		comm = (agent_comm_t *)x_malloc( sizeof(agent_comm_t));
		if( comm ) {
			while( icomm ) {
				if( !x_bcmp((i8 *)icomm->comm, name, 16) ) {
                    x_free(comm);
					return;
				}
				lcomm = icomm;
				icomm = icomm->next;
				i++;
			}
			lcomm->next     = comm;
			comm->index 	= i;
			x_bcopy( name, (i8 *)comm->comm, len );
			comm->commLen 	= len;
			comm->mode    	= 0;
			comm->access	= access;
			comm->hostList 	= 0;
			comm->next 		= 0;
		} else {
			x_dbg("XSNMP, AddComm, no memory 2\n", TRUE);
		}
	}
}

i32
GetCommIndex( i8 *name )
{
agent_comm_t *comm;

	if( *name ) {
		for( comm=xsnmp_cfg.comm_list; comm!=0; comm=comm->next ) {
			if( !(x_bcmp((i8 *)comm->comm, name, x_strlen( name ))) ) {
				return(comm->index);
			}
		}
	}
	return(0);
}

void
AddHost( i32 idx, ul32 hst )
{
agent_comm_t *comm;
agent_host_t *host, *chost;

	for( comm=xsnmp_cfg.comm_list; comm!=0; comm=comm->next ) {
		if( comm->index == idx ) {
			host = (agent_host_t *)x_malloc( sizeof(agent_host_t));
			if( host ) {
				host->addr 		= hst;
				host->next 		= 0;
				if( !comm->hostList ) {
					comm->hostList = host;
					break;
				} else {
					chost = comm->hostList;
					while( chost->next ) chost = chost->next;
					chost->next = host;
					break;
				}
			} else {
				x_dbg("XSNMP, AddHost, no memory\n", TRUE);
			}
		}
	}
}

void
RemoveAllHostsForComm( i32 idx )
{
agent_comm_t *comm;

    for( comm = xsnmp_cfg.comm_list; comm != 0; comm = comm->next ) {
		if( comm->index == idx ) {
            comm->hostList = 0;             // little memory leak here, should be insignificant
		}
	}
}

bool
AgentInit( u32 port )
{
static bool init = FALSE;

    if(!init) {
		if (port == 0) {
			port = xsnmp_cfg.local_port;
			agentUdp.locPort = (u16)port;
		} else {
        	agentUdp.locPort = (u16)port;
			xsnmp_cfg.local_port = port;
		}
		init = TRUE;
    }
    return init;
}


agent_stat_t
*AgentStatistics( void )
{
    return &agentStat;
}


bool
AgentGetAuthenTraps( void )
{
    return xsnmp_cfg.authentrap_enable;
}


void
AgentSetAuthenTraps( bool enable )
{
	if( enable == TRUE ) xsnmp_cfg.authentrap_enable = TRUE;
	else 				 xsnmp_cfg.authentrap_enable = FALSE;
}

void
AgentSetColdTraps( bool enable )
{
	if( enable == TRUE ) xsnmp_cfg.coldtrap_enable = TRUE;
	else 				 xsnmp_cfg.coldtrap_enable = FALSE;
}

bool
AgentSendTrap( u8 *comm, u16 commLen, ul32 systime, u16 gen,
                       u16 spec, snmp_object_t *list, u16 listLen )
{
u8				*trapBuffer;
u32				trapLen;
static			u8				sendTrapBuffer[AGENT_BUFFER_SIZE];
static			snmp_pdu_t		sendTrappdu;
static			udp_hdr_t		sendTrapudpHdr;
static			ip_hdr_t		sendTrapipHdr;
static			link_t			sendTrapnew;
static 			snmp_object_t	sendTrapobject = {SNMP_PDU_GET, {1,3,6,1,2,1,1,2,0}, 9};
agent_comm_t	*descr;
agent_host_t	*host;
ul32			addr;
bool			success = FALSE;
u16				dummy;

    if(MibRequest(1, &sendTrapobject, &dummy) == SNMP_NOERROR) {
        sendTrappdu.Type            = SNMP_PDU_TRAP;
        sendTrappdu.Trap.General    = gen;
        sendTrappdu.Trap.Specific   = spec;
        sendTrappdu.Trap.Time       = systime;
        //sendTrappdu.Trap.IdLen      = sendTrapobject.SyntaxLen;
        //x_memcpy(sendTrappdu.Trap.Id, sendTrapobject.Syntax.BufInt,
            //sendTrapobject.SyntaxLen * sizeof(sendTrapobject.Syntax.BufInt[0]));
        sendTrappdu.Trap.IdLen      =  9;
        sendTrappdu.Trap.Id[0]      =  1;
        sendTrappdu.Trap.Id[1]      =  3;
        sendTrappdu.Trap.Id[2]      =  6;
        sendTrappdu.Trap.Id[3]      =  1;
        sendTrappdu.Trap.Id[4]      =  4;
        sendTrappdu.Trap.Id[5]      =  1;
        sendTrappdu.Trap.Id[6]      =  2993;
        sendTrappdu.Trap.Id[7]      =  3;
        sendTrappdu.Trap.Id[8]      =  1;
        descr=CommFind(comm, commLen);
        if(descr!=0) {
            success = TRUE;
            for(host=descr->hostList; host!=0; host=host->next) {
				addr = (u32)get_hostid();
                sendTrappdu.Trap.IpAddress = IpH2NDWord(addr);
                sendTrapudpHdr.src  = UDP_PORT_ANY;
#ifdef XSTK_NUCNET
                sendTrapudpHdr.dst  = 162;
#else
                sendTrapudpHdr.dst  = x_htons(162);
#endif
                sendTrapipHdr.dst   = host->addr;
                sendTrapipHdr.src   = addr;
                sendTrapipHdr.iol   = 0;
                sendTrapipHdr.tos   = 0;
                trapBuffer  = sendTrapBuffer;
                trapLen     = sizeof(sendTrapBuffer);
                if(SnmpEnc(&trapBuffer, &trapLen, &sendTrappdu, comm, commLen,
					list, listLen)) {
                    LinkAlloc(&sendTrapnew, sendTrapBuffer, (u16)sizeof(sendTrapBuffer),
                                            (u16)trapLen, (u16)(trapBuffer-sendTrapBuffer), 0);
                    success = success && UdpSend(&sendTrapnew, &sendTrapudpHdr, &sendTrapipHdr, 0);

                }
            }
        }
    }
    return success;
}


static bool
RcveUdp( udp_descr_t *descr, link_t *link, udp_hdr_t *udpHdr, ip_hdr_t *ipHdr )
{
u16    	port;
ul32   	addr;
static	u8			RcveUdpBuffer[AGENT_BUFFER_SIZE];
static	link_t  	RcveUdpnew;
u8    	*rqsBuffer;
u16    	rqsLen;
link_t  *rqsLink;
u8    	*rspBuffer;
u32    	rspLen;
link_t  *rspLink;
bool 	success = FALSE;

    rqsLink    = link;
    rqsLen      = LinkLength(rqsLink);
    rqsBuffer   = LinkPop(&rqsLink, rqsLen);

    if (rqsBuffer!=0) {
        rspLink    = &RcveUdpnew;
        rspBuffer   = RcveUdpBuffer;
        rspLen      = sizeof(RcveUdpBuffer);

        if (Request(rqsBuffer, rqsLen, &rspBuffer, &rspLen, ipHdr->src)) {
            LinkAlloc(rspLink, RcveUdpBuffer, (u16)sizeof(RcveUdpBuffer),
                                    (u16)rspLen, (u16)(rspBuffer-RcveUdpBuffer), 0);

            port        = udpHdr->src;
            udpHdr->src = udpHdr->dst;
#ifdef XSTK_NUCNET
            udpHdr->dst = port;
#else
            udpHdr->dst = x_htons(port);
#endif

            addr        = ipHdr->src;
            ipHdr->src  = ipHdr->dst;
            ipHdr->dst  = addr;

            success = UdpSend(rspLink, udpHdr, ipHdr, descr);
        }
        if ( rqsLink != link )
            LinkFree(rqsLink);
    }

    if (success)
        agentStat.datagrams++;
    else
        agentStat.errors++;

    return success;
}


static bool
Request( u8 *rqsBuffer, u32 rqsLen, u8 **rspBuffer, u32* rspLen, ul32 addr )
{
static			snmp_pdu_t		Requestpdu;
snmp_request_t 	*rqs = &Requestpdu.Request;
static			u8				RequestComm[AGENT_COMMUNITY_SIZE];
u32        		commLen;
static			snmp_object_t 	RequestList[AGENT_LIST_SIZE];
u32        		listLen;
u16        		status, aindex;
static			u8				Requestaddress[4];
u16				perm;
static 			u8				Requestbuf[64];

	x_bzero( (i8 *)Requestbuf, 64);

    if (!SnmpDec(rqsBuffer, rqsLen, &Requestpdu, RequestComm, AGENT_COMMUNITY_SIZE, &commLen, RequestList,
        AGENT_LIST_SIZE, &listLen)) {

		rqs->Type = SNMP_PDU_RESPONSE;
		rqs->ErrorStatus = snmpErrStatus;
		rqs->ErrorIndex = snmpErrIndex;

		if (!SnmpErrEnc(rqs)) {
            x_dbg("XSNMP, SNMP error encode 1 failed\n", TRUE);
            return FALSE;
        }

   		x_memcpy(*rspBuffer, rqsBuffer, rqsLen);
		*rspLen = rqsLen;
		return TRUE;
    }

    if (Requestpdu.Type != SNMP_PDU_GET  &&
        Requestpdu.Type != SNMP_PDU_NEXT &&
        Requestpdu.Type != SNMP_PDU_SET) {
        x_dbg("XSNMP, unknown PDU\n", TRUE);
        return FALSE;
    }

    if ((perm = MibProfil(RequestComm, (u16)commLen)) == 0) {
		x_bcopy( (i8 *)RequestComm, (i8 *)&Requestbuf[0], commLen );
		x_dbg("Request: unknown community name\n", TRUE);
		agentStat.InBadCommunityNames++;
        return FALSE;
	}

    if (!HostInComm(RequestComm, (u16)commLen, addr)) {
		dword2mem(Requestaddress, IpH2NDWord(addr));
		x_bcopy( (i8 *)RequestComm, (i8 *)&Requestbuf[0], commLen );
        x_msgstr("XSNMP, authentication failure community = ",(i8 *)&Requestbuf[0]);
		if (xsnmp_cfg.authentrap_enable == TRUE )
			SendENTTrap( SNMP_TRAP_AUTFAILURE, 0, 0, 0 );
        return FALSE;
    }

	switch (Requestpdu.Type) {
	case SNMP_PDU_NEXT:
	case SNMP_PDU_GET:
		if ((perm & MIB_READ) == 0) {
			agentStat.InBadCommunityUses++;
        	return FALSE;
		}
		break;
	case SNMP_PDU_SET:
		if ((perm & MIB_WRITE) == 0) {
			agentStat.InBadCommunityUses++;
        	return FALSE;
		}
		break;
	}

	for (aindex = 0; aindex < listLen; aindex++) {
		RequestList[aindex].Request = (u16)Requestpdu.Type;
	}
	status = MibRequest(listLen, RequestList, &aindex);

	if (status != SNMP_NOERROR) {
		rqs->Type = SNMP_PDU_RESPONSE;
		rqs->ErrorStatus = status;
		rqs->ErrorIndex = aindex;

		if (!SnmpErrEnc(rqs)) {
            x_dbg("XSNMP, SNMP error encode 2 failed\n", TRUE);
            return FALSE;
        }

   		x_memcpy(*rspBuffer, rqsBuffer, rqsLen);
		*rspLen = rqsLen;
		return TRUE;
	}

	switch (Requestpdu.Type) {
		case SNMP_PDU_GET:
		case SNMP_PDU_NEXT:
			agentStat.InTotalReqVars += listLen;
			break;
		case SNMP_PDU_SET:
			agentStat.InTotalSetVars += listLen;
			break;
	}

    rqs->Type = SNMP_PDU_RESPONSE;
    rqs->ErrorStatus = SNMP_NOERROR;
    rqs->ErrorIndex  = 0;

    if (!SnmpEnc(rspBuffer, rspLen, &Requestpdu, RequestComm, commLen, RequestList, listLen)) {
		rqs->Type = SNMP_PDU_RESPONSE;
		rqs->ErrorStatus = snmpErrStatus;
		rqs->ErrorIndex = snmpErrIndex;

		if (!SnmpErrEnc(rqs)) {
            x_dbg("XSNMP, SNMP error encode 3 failed\n", TRUE);
            return FALSE;
        }

   		x_memcpy(*rspBuffer, rqsBuffer, rqsLen);
		*rspLen = rqsLen;
		return TRUE;
    }
    return TRUE;
}


agent_comm_t *
AgentCommunity( i32 aindex )
{
agent_comm_t *p;

    for( p=xsnmp_cfg.comm_list; p!=0; p=p->next ) {
        if( p->index == aindex ) {
            break;
		}
    }
    return p;
}


static agent_comm_t *
CommFind( u8 *comm, u16 commLen )
{
agent_comm_t *p;

    for (p=xsnmp_cfg.comm_list; p!=0; p=p->next) {
        if (p->commLen == commLen &&
            x_memcmp(p->comm, comm, commLen)==0)
            break;
    }
    return p;
}


static bool
HostInComm(u8 *comm, u16 commLen, ul32 addr)
{
agent_comm_t  *p;
agent_host_t  *h;

    p=CommFind(comm, commLen);
    if (p==0) {
        return FALSE;
	}
    if (p->hostList==0)  {
        return TRUE;
	}
    for (h=p->hostList; h!=0; h=h->next) {
        if (h->addr==addr)
            break;
    }
    return (h!=0);
}


/*
 * Send an Enterprise specific (ENT) trap
 */
void
SendENTTrap( u16 gen, u16 spec, snmp_object_t *list, u16 listLen )
{
i32 i;

	for( i=0; i<MAX_COMMS; i++ ) {
		if( CommIsTrap(i) ) {
			AgentSendTrap( (u8 *)GetCommName(i), (u16)GetCommLen(i),  SysTime(),
			gen, spec, list, listLen);
		}
	}
}
