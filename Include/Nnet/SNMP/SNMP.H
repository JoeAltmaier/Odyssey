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
#ifndef _SNMP_H_
#define _SNMP_H_


#define SNMP_VERSION    			0

/* Default Sizes */

#define SNMP_SIZE_COMM              256		/* 256 */
#define SNMP_SIZE_OBJECTID          128 	/* 128 */
#define SNMP_SIZE_BUFCHR            256 	/* 256 */
#define SNMP_SIZE_BUFINT            128		/* 128 */

#define SNMP_SIZE_SMALLOBJECTID     32


/* Requests */

#define SNMP_PDU_GET                0
#define SNMP_PDU_NEXT               1
#define SNMP_PDU_RESPONSE           2
#define SNMP_PDU_SET                3
#define SNMP_PDU_TRAP               4
#define SNMP_PDU_COMMIT				5	/* for internal use only */
#define SNMP_PDU_UNDO				6	/* for internal use only */

/* Errors */

#define SNMP_NOERROR                0
#define SNMP_TOOBIG                 1
#define SNMP_NOSUCHNAME             2
#define SNMP_BADVALUE               3
#define SNMP_READONLY               4
#define SNMP_GENERROR               5

/* General Traps */

#define SNMP_TRAP_COLDSTART          0
#define SNMP_TRAP_WARMSTART          1
#define SNMP_TRAP_LINKDOWN           2
#define SNMP_TRAP_LINKUP             3
#define SNMP_TRAP_AUTFAILURE         4
#define SNMP_TRAP_EQPNEIGHBORLOSS    5
#define SNMP_TRAP_ENTSPECIFIC        6

/* Rmon Status */

#define SNMP_VALID               1
#define SNMP_CREATEREQUEST       2
#define SNMP_UNDERCREATION       3
#define SNMP_INVALID             4

/* Types */

#define SNMP_NULL                0
#define SNMP_INTEGER             1    /* LngInt */
#define SNMP_OCTETSTR            2    /* BufChr */
#define SNMP_OBJECTID            3    /* BufInt */
#define SNMP_IPADDR              4    /* LngUns */
#define SNMP_COUNTER             5    /* LngUns */
#define SNMP_GAUGE               6    /* LngUns */
#define SNMP_TIMETICKS           7    /* LngUns */
#define SNMP_OPAQUE              8    /* BufChr */
#define SNMP_DISPLAYSTR          2    /* BufChr */

/* Typedefs */

typedef struct  snmp_stat_s       	snmp_stat_t;
typedef struct  snmp_request_s		snmp_request_t;
typedef struct  snmp_trap_s       	snmp_trap_t;
typedef union   snmp_pdu_s			snmp_pdu_t;

typedef union   snmp_syntax_s       snmp_syntax_t;
typedef struct  snmp_object_s		snmp_object_t;

#ifdef __cplusplus
extern "C" {
#endif
union snmp_syntax_s {
	l32			LngInt;
	u32			LngUns;
	u8			BufChr [SNMP_SIZE_BUFCHR];
	u32			BufInt [SNMP_SIZE_BUFINT];
	void		*Ptr;
};

struct snmp_object_s {
	u16				Request;
	ul32			Id[SNMP_SIZE_OBJECTID];
	u32				IdLen;
	u16				Type;
	snmp_syntax_t	Syntax;
	u32				SyntaxLen;
};
#ifdef __cplusplus
}
#endif


struct snmp_stat_s {
    ul32            OutPkts;
    ul32            OutTooBigs;
    ul32            OutNoSuchNames;
    ul32            OutBadValues;
    ul32            OutReadOnlys;
    ul32            OutGenErrs;
    ul32            OutGetRequests;
    ul32            OutGetNexts;
    ul32            OutSetRequests;
    ul32            OutGetResponses;
    ul32            OutTraps;
    ul32            InPkts;
    ul32            InTooBigs;
    ul32            InNoSuchNames;
    ul32            InBadValues;
    ul32            InReadOnlys;
    ul32            InGenErrs;
    ul32            InGetRequests;
    ul32            InGetNexts;
    ul32            InSetRequests;
    ul32            InGetResponses;
    ul32            InTraps;
    ul32            InBadVersions;
    ul32            InASNParseErrs;
    ul32            InBadTypes;
};


struct snmp_request_s {
    u32     		Type;
    ul32         	Id;
    u32     		ErrorStatus;
    u32     		ErrorIndex;
};

struct snmp_trap_s {
    u32             Type;
    ul32            Id [SNMP_SIZE_BUFINT];
    u32             IdLen;
    ul32            IpAddress;
    u32             General;
    u32             Specific;
    ul32            Time;
};


union snmp_pdu_s {
    u32					Type;
    struct snmp_trap_s	Trap;
    snmp_request_t		Request;
};


extern i32 snmpErrStatus;
extern i32 snmpErrIndex;
extern const i8 *SnmpTrap[];

bool SnmpEnc(u8 **Snmp, u32      *SnmpLen, snmp_pdu_t *Pdu, u8 *Com,
                u32      ComLen, snmp_object_t *Lst, u32      LstLen);
bool SnmpDec(u8 *Snmp, u32      SnmpLen, snmp_pdu_t *Pdu, u8 *Com,
                u32      ComSze, u32      *ComLen, snmp_object_t *Lst,
                u32      LstSze, u32      *LstLen);
bool SnmpErrEnc(snmp_request_t *Rqs);
bool SnmpCheckDisplayString(u8 *string, u16 len);


#endif
