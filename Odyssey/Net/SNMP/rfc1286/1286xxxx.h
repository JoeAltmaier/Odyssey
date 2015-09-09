/************************************************************************* 
 |                                                                         
 | FILE NAME   : 1286xxxx.h
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support file for RFC 1286 Groups
 | AUTHOR      : Ryan Braun
 | DATE	       : 8/17/99
 |
 *************************************************************************/
 
#ifndef _1286XXXx_H_
#define _1286XXXX_H_

/*
 * Typedefs used by the RFC1286 structure.
 * Defined here for portability.  Use of local typedefs
 * should conform to the following definitions
 */
typedef unsigned long			x_u32;		/* unsigned 32 bit */
typedef unsigned char			x_u8;		/* unsigned  8 bit */
typedef signed char				x_i8;		/* signed    8 bit */
typedef long					x_l32;		/* signed   32 bit */
typedef unsigned short			x_u16;		/* unsigned 16 bit */
typedef signed long				x_i32;		/* signed   32 bit */

/*
 * Miscellaneous defines.  These defines should be migrated to
 * a "config structure" so that a particular environment can
 * be set for the stack/mib/snmp
 * For now, these defines are sufficient.
 */
#define MAX_1286_STRSIZE		256
#define MAX_1286_BUFINT			128
#define MAX_1286_OIDSIZE		32
#define MAX_1286_PORTS			MAX_PORTS 
#define MAX_1286_PADDRSIZE		6

/*
 * Base group
 */
typedef struct baseporttab_s {
	x_u32					BasePort;
	x_u32					BasePortIfIndex;
	x_u32					BasePortCircuit;
	x_u32					BasePortDelayExceededDiscards;
	x_u32					BasePortMtuExceededDiscards;
	struct	baseporttab_s	*next;
	struct	baseporttab_s	*last;	
} baseporttab_t;

typedef struct rfc1286_Base_s {
	x_u8					BaseBridgeAddress[MAX_1286_PADDRSIZE];
	x_u32					BaseType;
	baseporttab_t			*BasePortTab;
} rfc1286_Base_t;

/*
 * RFC1286 group/object container
 */
typedef struct rfc1286_vars_s {
	rfc1286_Base_t			rfc1286_Base;
} rfc1286_vars_t;

#endif