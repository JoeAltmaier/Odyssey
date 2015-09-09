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
 | FILE NAME   : snmp.c                                 
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : SNMP protocol support functions                           
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"
#include "xsnmp.h"
#include "snmp.h"
#include "agent.h"
#include "link.h"
#include "prott.h"
#include "mac.h"
#include "xcfig.h"
#include "asn1.h"
 
#define SNMP_IPA    0			/* Ipaddress, APPLICATION (0) */
#define SNMP_CNT    1			/* Counter,   APPLICATION (1) */
#define SNMP_GGE    2			/* Gauge,     APPLICATION (2) */
#define SNMP_TIT    3			/* TimeTicks  APPLICATION (3) */
#define SNMP_OPQ    4			/* Opaque,    APPLICATION (4) */

static bool SnmpSyn2TagCls(u32 *Tag, u32 *Cls, i32 Syn);
static bool SnmpTagCls2Syn(u32 Tag, u32 Cls, u16 *Syn);
static bool SnmpObjEnc(asn1_sck_t *Asn1, snmp_object_t *Obj);
static bool SnmpObjDec(asn1_sck_t *Asn1, snmp_object_t *Obj);
static bool SnmpLstEnc(asn1_sck_t *Asn1, snmp_object_t *Lst, u32 LstLen);
static bool SnmpLstDec(asn1_sck_t *Asn1, snmp_object_t *Lst, u32 LstSze, u32 *LstLen);
static bool SnmpRqsEnc(asn1_sck_t *Asn1, snmp_request_t *Rqs);
static bool SnmpRqsDec(asn1_sck_t *Asn1, snmp_request_t *Rqs);
static bool SnmpTrpEnc(asn1_sck_t *Asn1, snmp_trap_t *Trp);
static bool SnmpTrpDec(asn1_sck_t *Asn1, snmp_trap_t *Trp);
static bool SnmpPduEnc(asn1_sck_t *Asn1, snmp_pdu_t *Pdu, snmp_object_t *Lst, u32 LstLen);
static bool SnmpPduDec(asn1_sck_t *Asn1, snmp_pdu_t *Pdu, snmp_object_t *Lst, u32 LstSze, u32 *LstLen);


i32 snmpErrStatus = SNMP_NOERROR;
i32 snmpErrIndex = 0;
asn1_sck_t snmpErrAsn1;

typedef struct snmp_cnv_s snmp_cnv_t;

struct snmp_cnv_s {
	u32	Class;
	u32	Tag;
	i32	Syntax;
};

snmp_stat_t SnmpStat;

const i8  *SnmpTrap[] = {
	"cold start",
	"warm start",
	"link down",
	"link up",
	"authentication failure",
    "neighbor loss",
	"enterprise specific"
};

static snmp_cnv_t SnmpCnv[] = {
    {ASN1_UNI, ASN1_NUL, SNMP_NULL},
    {ASN1_UNI, ASN1_INT, SNMP_INTEGER},
    {ASN1_UNI, ASN1_OTS, SNMP_OCTETSTR},
    {ASN1_UNI, ASN1_OTS, SNMP_DISPLAYSTR},
    {ASN1_UNI, ASN1_OJI, SNMP_OBJECTID},
    {ASN1_APL, SNMP_IPA, SNMP_IPADDR},
    {ASN1_APL, SNMP_CNT, SNMP_COUNTER},
    {ASN1_APL, SNMP_GGE, SNMP_GAUGE},
    {ASN1_APL, SNMP_TIT, SNMP_TIMETICKS},
    {ASN1_APL, SNMP_OPQ, SNMP_OPAQUE},
    {0, 0, -1}
};

bool 
SnmpSyn2TagCls( u32 *Tag, u32 *Cls, i32 Syn)
{
snmp_cnv_t *Cnv;

    Cnv = SnmpCnv;
    while (Cnv->Syntax != -1) {
        if (Cnv->Syntax == Syn) {
            *Tag = Cnv->Tag;
            *Cls = Cnv->Class;
            return TRUE;
        }
        Cnv++;
    }
	snmpErrStatus = SNMP_BADVALUE;
	SnmpStat.OutBadValues++;
    return FALSE;
}

bool 
SnmpTagCls2Syn( u32 Tag, u32 Cls, u16 *Syn )
{
snmp_cnv_t *Cnv;

    Cnv = SnmpCnv;
    while (Cnv->Syntax != -1) {
        if (Cnv->Tag == Tag && Cnv->Class == Cls) {
            *Syn = Cnv->Syntax;
            return TRUE;
        }
        Cnv++;
    }
	snmpErrStatus = SNMP_BADVALUE;
	SnmpStat.OutBadValues++;
    return FALSE;
}

bool 
SnmpObjEnc( asn1_sck_t *Asn1, snmp_object_t *Obj )
{
u32 Cls, Tag;
u8 *Eoc, *End;

    if(!Asn1EocEnc (Asn1, &Eoc)) {
        return FALSE;
	}
    switch(Obj->Type) {
        case SNMP_INTEGER:
            if(!Asn1IntEncLng (Asn1, &End, Obj->Syntax.LngInt)) {
                return FALSE;
			}
            break;
        case SNMP_OCTETSTR:
        case SNMP_OPAQUE:
            if(!Asn1OtsEnc (Asn1, &End, Obj->Syntax.BufChr, Obj->SyntaxLen)) {
                return FALSE;
			}
            break;
        case SNMP_NULL:
            if(!Asn1NulEnc (Asn1, &End)) {
                return FALSE;
			}
            break;
        case SNMP_OBJECTID:
            if(!Asn1OjiEnc (Asn1, &End, Obj->Syntax.BufInt, Obj->SyntaxLen)) {
                return FALSE;
			}
            break;
        case SNMP_IPADDR:
            if(!Asn1OtsEnc (Asn1, &End, (u8 *)&Obj->Syntax.LngUns, 4)) {
                return FALSE;
			}
            break;
        case SNMP_COUNTER:
        case SNMP_GAUGE:
        case SNMP_TIMETICKS:
            if(!Asn1IntEncLngUns (Asn1, &End, Obj->Syntax.LngUns)) {
                return FALSE;
			}
            break;
        default:
			snmpErrStatus = SNMP_BADVALUE;
			SnmpStat.OutBadValues++;
            return FALSE;
    }
    if(!SnmpSyn2TagCls (&Tag, &Cls, Obj->Type)) {
        return FALSE;
	}
    if(!Asn1HdrEnc (Asn1, End, Cls, ASN1_PRI, Tag)) {
        return FALSE;
	}
    if(!Asn1OjiEnc (Asn1, &End, Obj->Id, Obj->IdLen)) {
        return FALSE;
	}
    if(!Asn1HdrEnc (Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_OJI)) {
        return FALSE;
	}
    if(!Asn1HdrEnc (Asn1, Eoc, ASN1_UNI, ASN1_CON, ASN1_SEQ)) {
        return FALSE;
	}
    return TRUE;
}

bool 
SnmpObjDec( asn1_sck_t *Asn1, snmp_object_t *Obj )
{
u32 Cls, Con, Tag;
u8 *Eoc, *End;

    if(!Asn1HdrDec (Asn1, &Eoc, &Cls, &Con, &Tag))
        return FALSE;
    if(Cls != ASN1_UNI || Con != ASN1_CON || Tag != ASN1_SEQ)
        return FALSE;
    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag))
        return FALSE;
    if(Cls != ASN1_UNI || Con != ASN1_PRI || Tag != ASN1_OJI)
        return FALSE;
    if(!Asn1OjiDec (Asn1, End, Obj->Id, SNMP_SIZE_OBJECTID, &Obj->IdLen))
        return FALSE;
    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag))
        return FALSE;
    if(Con != ASN1_PRI) {
		snmpErrStatus = SNMP_BADVALUE;
		SnmpStat.OutBadValues++;
        return FALSE;
	}
    if(!SnmpTagCls2Syn (Tag, Cls, &Obj->Type))
        return FALSE;
    switch(Obj->Type) {
        case SNMP_INTEGER:
            if(!Asn1IntDecLng (Asn1, End, (u32 *)&Obj->Syntax.LngInt))
                return FALSE;
            break;
        case SNMP_OCTETSTR:
        case SNMP_OPAQUE:
            if(!Asn1OtsDec (Asn1, End, Obj->Syntax.BufChr, SNMP_SIZE_BUFCHR,
                &Obj->SyntaxLen))
                return FALSE;
            break;
        case SNMP_NULL:
            if(!Asn1NulDec (Asn1, End))
                return FALSE;
            break;
        case SNMP_OBJECTID:
            if(!Asn1OjiDec (Asn1, End, Obj->Syntax.BufInt, SNMP_SIZE_BUFINT,
                &Obj->SyntaxLen))
                return FALSE;
            break;
        case SNMP_IPADDR:
            if(!Asn1OtsDec (Asn1, End, (u8 *)&Obj->Syntax.LngUns, 4,
                &Obj->SyntaxLen))
                return FALSE;
            if(Obj->SyntaxLen != 4)
                return FALSE;
            break;
        case SNMP_COUNTER:
        case SNMP_GAUGE:
        case SNMP_TIMETICKS:
            if(!Asn1IntDecLngUns (Asn1, End, &Obj->Syntax.LngUns))
                return FALSE;
            break;
        default:
			snmpErrStatus = SNMP_BADVALUE;
			SnmpStat.OutBadValues++;
            return FALSE;
    }
    if(!Asn1EocDec (Asn1, Eoc))
        return FALSE;
    return TRUE;
}

bool 
SnmpLstEnc( asn1_sck_t *Asn1, snmp_object_t *Lst, u32 LstLen )
{
u8 *Eoc;

    if(!Asn1EocEnc (Asn1, &Eoc)) {
        return FALSE;
	}
    Lst += LstLen;
    while(LstLen-- > 0) {
        if(!SnmpObjEnc (Asn1, --Lst)) {
            return FALSE;
		}
    }
    if(!Asn1HdrEnc (Asn1, Eoc, ASN1_UNI, ASN1_CON, ASN1_SEQ)) {
        return FALSE;
	}
    return TRUE;
}

bool
SnmpLstDec( asn1_sck_t *Asn1, snmp_object_t *Lst, u32 LstSze, u32 *LstLen )
{
u32 Cls, Con, Tag;
u8 *Eoc;

    if(!Asn1HdrDec (Asn1, &Eoc, &Cls, &Con, &Tag))
        return FALSE;
    if(Cls != ASN1_UNI || Con != ASN1_CON || Tag != ASN1_SEQ)
        return FALSE;
    *LstLen = 0;
    while(!Asn1Eoc (Asn1, Eoc)) {
        if(++(*LstLen) > LstSze) {
            snmpErrStatus = SNMP_TOOBIG;
			SnmpStat.OutTooBigs++;
            return FALSE;
        }
        if(!SnmpObjDec (Asn1, Lst++)) {
			snmpErrIndex = *LstLen;
            return FALSE;
		}
    }
    if(!Asn1EocDec (Asn1, Eoc))
        return FALSE;
    return TRUE;
}

bool 
SnmpRqsEnc( asn1_sck_t *Asn1, snmp_request_t *Rqs )
{
u8 *End;

    if(!Asn1IntEncUns (Asn1, &End, Rqs->ErrorIndex))
        return FALSE;
    if(!Asn1HdrEnc (Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_INT))
        return FALSE;
    if(!Asn1IntEncUns (Asn1, &End, Rqs->ErrorStatus))
        return FALSE;
    if(!Asn1HdrEnc (Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_INT))
        return FALSE;
    if(!Asn1IntEncLngUns (Asn1, &End, Rqs->Id))
        return FALSE;
    if(!Asn1HdrEnc (Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_INT))
        return FALSE;
    return TRUE;
}

bool
SnmpRqsDec( asn1_sck_t *Asn1, snmp_request_t *Rqs )
{
u32 Cls, Con, Tag;
u8 *End;

    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag))
        return FALSE;
    if(Cls != ASN1_UNI || Con != ASN1_PRI || Tag != ASN1_INT)
        return FALSE;
    if(!Asn1IntDecLngUns (Asn1, End, &Rqs->Id))
        return FALSE;
    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag))
        return FALSE;
    if(Cls != ASN1_UNI || Con != ASN1_PRI || Tag != ASN1_INT)
        return FALSE;
    if(!Asn1IntDecUns (Asn1, End, &Rqs->ErrorStatus))
        return FALSE;
    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag))
        return FALSE;
    if(Cls != ASN1_UNI || Con != ASN1_PRI || Tag != ASN1_INT)
        return FALSE;
    if(!Asn1IntDecUns (Asn1, End, &Rqs->ErrorIndex))
        return FALSE;
	/*
 	 * Should not receive any error status
	 */
    if(Rqs->ErrorStatus != SNMP_NOERROR) { 
        switch(Rqs->ErrorStatus) {
        case SNMP_TOOBIG:
             SnmpStat.InTooBigs++;
             break;
        case SNMP_NOSUCHNAME:
             SnmpStat.InNoSuchNames++;
             break;
        case SNMP_BADVALUE:
             SnmpStat.InBadValues++;
             break;
        case SNMP_READONLY:
             SnmpStat.InReadOnlys++;
             break;
        default:
             SnmpStat.InGenErrs++;
             break;
        }
        return FALSE;
    }
#if 0
    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag))
        return FALSE;
    if(Cls != ASN1_UNI || Con != ASN1_PRI || Tag != ASN1_INT)
        return FALSE;
    if(!Asn1IntDecUns (Asn1, End, &Rqs->ErrorIndex))
        return FALSE;
#endif
	x_memcpy(&snmpErrAsn1, Asn1, sizeof(asn1_sck_t));
    return TRUE;
}

bool
SnmpTrpEnc( asn1_sck_t *Asn1, snmp_trap_t *Trp )
{
u8 *End;

    if(!Asn1IntEncLngUns (Asn1, &End, Trp->Time)) {
        return FALSE;
	}
    if(!Asn1HdrEnc (Asn1, End, ASN1_APL, ASN1_PRI, SNMP_TIT)) {
        return FALSE;
	}
    if(!Asn1IntEncUns (Asn1, &End, Trp->Specific)) {
        return FALSE;
	}
    if(!Asn1HdrEnc (Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_INT)) {
        return FALSE;
	}
    if(!Asn1IntEncUns (Asn1, &End, Trp->General)) {
        return FALSE;
	}
    if(!Asn1HdrEnc (Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_INT)) {
        return FALSE;
	}
    if(!Asn1OtsEnc (Asn1, &End, (u8 *)&(Trp->IpAddress), 4)) {
        return FALSE;
	}
    if(!Asn1HdrEnc (Asn1, End, ASN1_APL, ASN1_PRI, SNMP_IPA)) {
        return FALSE;
	}
    if(!Asn1OjiEnc (Asn1, &End, Trp->Id, Trp->IdLen)) {
        return FALSE;
	}
    if(!Asn1HdrEnc (Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_OJI)) {
        return FALSE;
	}
    return TRUE;
}

bool
SnmpTrpDec( asn1_sck_t *Asn1, snmp_trap_t *Trp )
{
u32 Cls, Con, Tag;
u8 *End;

    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag))
        return FALSE;
    if(Cls != ASN1_UNI || Con != ASN1_PRI || Tag != ASN1_OJI)
        return FALSE;
    if(!Asn1OjiDec (Asn1, End, Trp->Id, sizeof(Trp->Id)/sizeof(Trp->Id[0]), &Trp->IdLen))
        return FALSE;
    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag))
        return FALSE;
    if(!((Cls == ASN1_APL && Con == ASN1_PRI && Tag == SNMP_IPA) ||
		(Cls == ASN1_UNI && Con == ASN1_PRI && Tag == ASN1_OTS)))	/* needed for Banyan */
        return FALSE;
    if(!Asn1OtsDec (Asn1, End, (u8 *)&(Trp->IpAddress), 4, &Tag))
        return FALSE;
    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag))
        return FALSE;
    if(Cls != ASN1_UNI || Con != ASN1_PRI || Tag != ASN1_INT)
        return FALSE;
    if(!Asn1IntDecUns (Asn1, End, &Trp->General))
        return FALSE;
    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag))
        return FALSE;
    if(Cls != ASN1_UNI || Con != ASN1_PRI || Tag != ASN1_INT)
        return FALSE;
    if(!Asn1IntDecUns (Asn1, End, &Trp->Specific))
        return FALSE;
    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag))
        return FALSE;
    if(!((Cls == ASN1_APL && Con == ASN1_PRI && Tag == SNMP_TIT) ||
		(Cls == ASN1_UNI && Con == ASN1_PRI && Tag == ASN1_INT)))
        return FALSE;
    if(!Asn1IntDecLngUns (Asn1, End, &Trp->Time))
        return FALSE;
    return TRUE;
}

bool
SnmpPduEnc( asn1_sck_t *Asn1, snmp_pdu_t *Pdu, snmp_object_t *Lst, u32 LstLen )
{
u8 *Eoc;

    if(!Asn1EocEnc (Asn1, &Eoc)) {
        return FALSE;
	}
    if(!SnmpLstEnc (Asn1, Lst, LstLen)) {
        return FALSE;
	}
    switch(Pdu->Type) {
        case SNMP_PDU_GET:
        case SNMP_PDU_NEXT:
        case SNMP_PDU_RESPONSE:
        case SNMP_PDU_SET:
            if(!SnmpRqsEnc (Asn1, &Pdu->Request)) {
                return FALSE;
			}
            break;
        case SNMP_PDU_TRAP:
            if(!SnmpTrpEnc (Asn1, &Pdu->Trap)) {
                return FALSE;
			}
            break;
        default:
            return FALSE;
    }
    if(!Asn1HdrEnc (Asn1, Eoc, ASN1_CTX, ASN1_CON, Pdu->Type)) {
        return FALSE;
	}
    return TRUE;
}

bool
SnmpPduDec( asn1_sck_t *Asn1, snmp_pdu_t *Pdu, snmp_object_t *Lst, u32 LstSze, u32 *LstLen )
{
u32 Cls, Con; 
u8 *Eoc;

    if(!Asn1HdrDec (Asn1, &Eoc, &Cls, &Con, &Pdu->Type))
        return FALSE;
    if(Cls != ASN1_CTX || Con != ASN1_CON)
        return FALSE;
    switch(Pdu->Type) {
        case SNMP_PDU_GET:
        case SNMP_PDU_NEXT:
        case SNMP_PDU_RESPONSE:
        case SNMP_PDU_SET:
            if(!SnmpRqsDec (Asn1, &Pdu->Request))
                return FALSE;
            break;
        case SNMP_PDU_TRAP:
            if(!SnmpTrpDec (Asn1, &Pdu->Trap))
                return FALSE;
            break;
        default:
            SnmpStat.InBadTypes++;
            return FALSE;
    }
    if(!SnmpLstDec (Asn1, Lst, LstSze, LstLen))
        return FALSE;
    if(!Asn1EocDec (Asn1, Eoc))
        return FALSE;
    return TRUE;
}

bool 
SnmpMsgEnc( asn1_sck_t *Asn1, snmp_pdu_t *Pdu, u8 *Com, u32 ComLen,
		snmp_object_t *Lst, u32	LstLen)
{
u8 *Eoc, *End;
    
	if(!Asn1EocEnc (Asn1, &Eoc)) {
        return FALSE;
	}
    if(!SnmpPduEnc (Asn1, Pdu, Lst, LstLen)) {
        return FALSE;
	}
    if(!Asn1OtsEnc (Asn1, &End, Com, ComLen)) {
        return FALSE;
	}
    if(!Asn1HdrEnc (Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_OTS)) {
        return FALSE;
	}
    if(!Asn1IntEncUns (Asn1, &End, SNMP_VERSION)) {
        return FALSE;
	}
    if(!Asn1HdrEnc (Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_INT)) {
        return FALSE;
	}
    if(!Asn1HdrEnc (Asn1, Eoc, ASN1_UNI, ASN1_CON, ASN1_SEQ)) {
        return FALSE;
	}
    return TRUE;
}

bool
SnmpMsgDec( asn1_sck_t *Asn1, snmp_pdu_t *Pdu, u8 *Com, u32 ComSze, u32 *ComLen, 
			snmp_object_t *Lst, u32 LstSze, u32 *LstLen ) 
{
u32 Cls, Con, Tag, Ver;
u8 *Eoc, *End;

    if(!Asn1HdrDec (Asn1, &Eoc, &Cls, &Con, &Tag)) {
        return FALSE;
	}
    if(Cls != ASN1_UNI || Con != ASN1_CON || Tag != ASN1_SEQ) {
        return FALSE;
	}
    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag)) {
        return FALSE;
	}
    if(Cls != ASN1_UNI || Con != ASN1_PRI || Tag != ASN1_INT) {
        return FALSE;
	}
    if(!Asn1IntDecUns (Asn1, End, &Ver)) {
        return FALSE;
	}
    if(!Asn1HdrDec (Asn1, &End, &Cls, &Con, &Tag)) {
        return FALSE;
	}
    if(Cls != ASN1_UNI || Con != ASN1_PRI || Tag != ASN1_OTS) {
        return FALSE;
	}
    if(!Asn1OtsDec (Asn1, End, Com, ComSze, ComLen)) {
        return FALSE;
	}
    if(Ver != SNMP_VERSION) {
        SnmpStat.InBadVersions++;
        return FALSE;
    }
    if(!SnmpPduDec (Asn1, Pdu, Lst, LstSze, LstLen)) {
        return FALSE;
	}
    if(!Asn1EocDec (Asn1, Eoc)) {
        return FALSE;
	}
	return TRUE;
}

bool 
SnmpEnc( u8 **Snmp, u32 *SnmpLen, snmp_pdu_t *Pdu, u8 *Com, u32 ComLen, 
		snmp_object_t *Lst, u32 LstLen )
{
static asn1_sck_t SnmpEncAsn1;

    snmpErrStatus = SNMP_NOERROR;
    snmpErrIndex = 0;

    Asn1Opn (&SnmpEncAsn1, *Snmp, *SnmpLen, ASN1_ENC);
	if(!SnmpMsgEnc(&SnmpEncAsn1, Pdu, Com, ComLen, Lst, LstLen)) {

		if(snmpErrStatus == SNMP_NOERROR) {
			switch(asn1ErrStatus) {
				case ASN1_ERR_ENC_FULL:
				case ASN1_ERR_NOERROR:
				case ASN1_ERR_DEC_EMPTY:
				case ASN1_ERR_DEC_EOC_MISMATCH:
				case ASN1_ERR_DEC_LENGTH_MISMATCH:
				case ASN1_ERR_DEC_BADVALUE:
				case ASN1_ERR_ENC_BADVALUE:
				default:
					snmpErrStatus = SNMP_GENERROR;
					SnmpStat.OutGenErrs++;
					break;

			}
		}
		return FALSE;
	}

	Asn1Cls(&SnmpEncAsn1, Snmp, SnmpLen);

    SnmpStat.OutPkts++;

    switch(Pdu->Type) {
        case SNMP_PDU_GET:
            SnmpStat.OutGetRequests++;
            break;
        case SNMP_PDU_NEXT:
            SnmpStat.OutGetNexts++;
            break;
        case SNMP_PDU_RESPONSE:
            SnmpStat.OutGetResponses++;
            break;
        case SNMP_PDU_SET:
            SnmpStat.OutSetRequests++;
            break;
        case SNMP_PDU_TRAP:
            SnmpStat.OutTraps++;
            break;
    }
    
    return TRUE;
}

bool 
SnmpDec( u8 *Snmp, u32 SnmpLen, snmp_pdu_t *Pdu, u8 *Com, u32 ComSze, 
		u32 *ComLen, snmp_object_t *Lst, u32 LstSze, u32 *LstLen )
{
static asn1_sck_t SnmpDecAsn1;

    snmpErrStatus = SNMP_NOERROR;
    snmpErrIndex = 0;

    Asn1Opn(&SnmpDecAsn1, Snmp, SnmpLen, ASN1_DEC);
	if(!SnmpMsgDec(&SnmpDecAsn1, Pdu, Com, ComSze, ComLen, Lst, LstSze, LstLen)) {
		if(snmpErrStatus == SNMP_NOERROR) {
			switch(asn1ErrStatus) {
				case ASN1_ERR_DEC_BADVALUE:
				case ASN1_ERR_DEC_EOC_MISMATCH:
				case ASN1_ERR_DEC_LENGTH_MISMATCH:
				case ASN1_ERR_DEC_EMPTY:
					snmpErrStatus = SNMP_BADVALUE;
					SnmpStat.OutBadValues++;
					SnmpStat.InASNParseErrs++;
					break;
				case ASN1_ERR_ENC_FULL:
				case ASN1_ERR_NOERROR:
				case ASN1_ERR_ENC_BADVALUE:
				default:
					snmpErrStatus = SNMP_GENERROR;
					SnmpStat.OutGenErrs++;
					break;
			}
		}
		return FALSE;
	}

    Asn1Cls(&SnmpDecAsn1, &Snmp, &SnmpLen);
    
    SnmpStat.InPkts++;
    switch(Pdu->Type) {
        case SNMP_PDU_GET:
            SnmpStat.InGetRequests++;
            break;
        case SNMP_PDU_NEXT:
            SnmpStat.InGetNexts++;
            break;
        case SNMP_PDU_RESPONSE:
            SnmpStat.InGetResponses++;
            break;
        case SNMP_PDU_SET:
            SnmpStat.InSetRequests++;
            break;
        case SNMP_PDU_TRAP:
            SnmpStat.InTraps++;
            break;
    }
    
    return TRUE;
}


bool 
SnmpErrEnc( snmp_request_t *Rqs )
{
u8 *End;
asn1_sck_t *Asn1 = &snmpErrAsn1;

    if(!Asn1IntEncUns(Asn1, &End, Rqs->ErrorIndex))
        return FALSE;
    if(!Asn1HdrEnc(Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_INT))
        return FALSE;
    if(!Asn1IntEncUns(Asn1, &End, Rqs->ErrorStatus))
        return FALSE;
    if(!Asn1HdrEnc(Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_INT))
        return FALSE;
    if(!Asn1IntEncLngUns(Asn1, &End, Rqs->Id))
        return FALSE;
    if(!Asn1HdrEnc(Asn1, End, ASN1_UNI, ASN1_PRI, ASN1_INT))
        return FALSE;
    if(!Asn1HdrEnc(Asn1, Asn1->End, ASN1_CTX, ASN1_CON, Rqs->Type))
        return FALSE;
	return TRUE;
}


bool 
SnmpCheckDisplayString( u8 *string, u16 len )
{
i32 i;

	for(i = 0; i < len; i++) {
		if(string[i] > 127)
			return FALSE;
		if(string[i] == 13 && i+1 >= len)
			return FALSE;
		if(string[i] == 13 && string[i+1] != 0 && string[i+1] != 10)
			return FALSE;
	}
	return TRUE;
}

snmp_stat_t *
SnmpStatistics( void )
{
	return &SnmpStat;
}

