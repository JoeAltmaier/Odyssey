/************************************************************************* 
 |                                                                         
 | FILE NAME   : 1286base.c
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports RFC 1286 dot1dBase Group
 | AUTHOR      : Ryan Braun
 | DATE	       : 8/17/99
 | NOTES	   : This hack supports only a single Bridge object and
 |					initializes the MIB with static information
 |
 *************************************************************************/
 
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"
#include "snmp.h"
#include "agent.h"
#include "link.h"
#include "prott.h"
#include "xsnmp.h"
#include "mac.h"
#include "xcfig.h"
#include "mib.h"
#include "timer.h"
#include "xarp.h"
#include "ipp.h"
#include "1286base.h"
#include "1286xxxx.h"

extern rfc1286_vars_t	rfc1286_vars;

static bool BaseMib( snmp_object_t * obj, u16 idlen );
u16 Get1286BasePortTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag);

static u8 bridgeAddr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

bool
BaseMibInit( void )
{
i8	*t;
static i8	ObjectID[SNMP_SIZE_BUFCHR];

#ifdef INTERNAL_RFC1286_STORAGE
	x_bzero( (i8 *)&(rfc1286_vars), sizeof(rfc1286_vars_t) );
#endif

	/* init base vars */
	memcpy(rfc1286_vars.rfc1286_Base.BaseBridgeAddress, bridgeAddr, MAX_1286_PADDRSIZE);
	rfc1286_vars.rfc1286_Base.BaseType = 2;	/* Transparent Only */
	rfc1286_vars.rfc1286_Base.BasePortTab = NULL;	

	return TRUE;
}

u16
BaseBridgeAddress( snmp_object_t * obj, u16 idlen, void * param )
{

	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

}

u16
BaseNumPorts( snmp_object_t * obj, u16 idlen, void * param )
{

}

u16
BaseType( snmp_object_t * obj, u16 idlen, void * param )
{


}
