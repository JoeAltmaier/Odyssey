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
 | FILE NAME   : sys.c                                  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : System group support functions                            
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
#include "timer.h"
#include "xarp.h"
#include "ipp.h"
#include "1213xxxx.h"

extern	void nc_init(void);
extern	rfc1213_vars_t	rfc1213_vars;
extern 	xsnmp_cfig_t	xsnmp_cfg;

void
SysUpdate( void )
{
i8  *t;
static i8	ObjectID[SNMP_SIZE_BUFCHR];

	x_strcpy((i8 *)(rfc1213_vars.rfc1213_sys.sysDescr), 		&xsnmp_cfg.sys_description[0]);
	x_strcpy((i8 *)(rfc1213_vars.rfc1213_sys.sysContact),	&xsnmp_cfg.sys_contact[0]);
	x_strcpy((i8 *)(rfc1213_vars.rfc1213_sys.sysName),     	&xsnmp_cfg.sys_name[0]);
	x_strcpy((i8 *)(rfc1213_vars.rfc1213_sys.sysLocation), 	&xsnmp_cfg.sys_location[0]);
	rfc1213_vars.rfc1213_sys.sysServices			= xsnmp_cfg.sys_services;
	x_strcpy((i8 *)ObjectID, (i8 *)&xsnmp_cfg.sys_objectid[0]);

	rfc1213_vars.rfc1213_sys.sysObjectIDLen     	= 0;
	if ((t = (i8 *)x_strtok(ObjectID, ".")) != NULL) {
		rfc1213_vars.rfc1213_sys.sysObjectID[rfc1213_vars.rfc1213_sys.sysObjectIDLen++] = x_atol(t);
		while ((t = (i8 *)x_strtok(NULL, ".")) != NULL) {
			rfc1213_vars.rfc1213_sys.sysObjectID[rfc1213_vars.rfc1213_sys.sysObjectIDLen++] = x_atol(t);
		}
	}
}


ul32 
SysTime( void )
{
/*  very costly version: does a MibRequest of sysUpTime  */
/*  snmp_object_t time = {SNMP_PDU_GET,  {1,3,6,1,2,1,1,3,0}, 9};
 * 
 * if (MibRequest(&time) != SNMP_NOERROR)
 *     return 0;
 * else
 *     return time.Syntax.LngUns;
 */

	/* Current ticks minus startup ticks */
	if( (u32)rfc1213_vars.rfc1213_sys.sysUpTime < x_timeticks() )
		return(x_timeticks() - rfc1213_vars.rfc1213_sys.sysUpTime);
	else
		return(x_timeticks());
}

void 
get_system_contact( u8 *buf )
{
	x_bcopy( (i8 *)(rfc1213_vars.rfc1213_sys.sysContact), (i8 *)buf, MAX_SYS_STRINGS );
}

void 
get_system_description( u8 *buf )
{
	x_bcopy( (i8 *)(rfc1213_vars.rfc1213_sys.sysDescr), (i8 *)buf, MAX_SYS_STRINGS );
}

void 
get_system_location( u8 *buf )
{
	x_bcopy( (i8 *)(rfc1213_vars.rfc1213_sys.sysLocation), (i8 *)buf, MAX_SYS_STRINGS );
}

void 
get_system_name( u8 *buf )
{
	x_bcopy( (i8 *)(rfc1213_vars.rfc1213_sys.sysName), (i8 *)buf, MAX_SYS_STRINGS );
}

void 
get_system_objectid( u8 *buf )
{
	x_bcopy( (i8 *)(rfc1213_vars.rfc1213_sys.sysObjectID), (i8 *)buf, MAX_SYS_STRINGS );
}

i32 
get_system_services( void )
{
	return( rfc1213_vars.rfc1213_sys.sysServices );
}

void
sc_init( void )
{
	x_bcopy(MIBII_SYSCONTACT,		(i8 *)(rfc1213_vars.rfc1213_sys.sysContact), MAX_SYS_STRINGS );
	x_bcopy(MIBII_SYSDESCRIPTION, 	(i8 *)(rfc1213_vars.rfc1213_sys.sysDescr), MAX_SYS_STRINGS );
	x_bcopy(MIBII_SYSLOCATION, 		(i8 *)(rfc1213_vars.rfc1213_sys.sysLocation), MAX_SYS_STRINGS );
	x_bcopy(MIBII_SYSNAME,			(i8 *)(rfc1213_vars.rfc1213_sys.sysName), MAX_SYS_STRINGS );
	//x_bcopy((i8 *)MIBII_OBJECTID, 	(i8 *)(rfc1213_vars.rfc1213_sys.sysObjectID), MAX_SYS_STRINGS );
	rfc1213_vars.rfc1213_sys.sysServices 	= MIBII_SERVICES;

	nc_init();
}
