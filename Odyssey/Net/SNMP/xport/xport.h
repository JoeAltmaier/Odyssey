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
 | FILE NAME   : xport.h
 | VERSION     : 1.1
 | COMPONENT   : XFAMILY
 | DESCRIPTION : User system configuration file for XFAMILY
 | AUTHOR      : Robert Winter
 *************************************************************************/
/*
 * This is the start of the user definable region for the XSNMPv1
 * package and it's associated MIB engines.
 *
 * The user must select an operating system define, a TCP/IP stack
 * define and a library define.
 *
 * The user must also select various defines related to certain
 * tunable parameters within each MIB engine.
 *
 *====================== Start of User Defines ==========================*/
/*
 * Define the operating system that the X-files work under
 * Choose one. The truth is out there...
 */
#ifndef _XPORT_H_
#define _XPORT_H_
#define XOS_NUC					/* Nucleus OS from ATI					*/
/*#define XOS_PSOS*/			/* pSOS from ISI      					*/
/*#define XOS_VXWORKS*/			/* VxWorks             					*/
/*#define XOS_XOS*/				/* XOS OS from Xact    					*/

/*
 * Define the TCP/IP stack that the X-files work under
 * Choose one. The truth is still out there...
 */
#define XSTK_NUCNET				/* Nucleus NET TCP/IP from ATI 			*/
/*#define XSTK_PSOS*/			/* pSOS TCP/IP from ISI 				*/
/*#define XSTK_VXWORKS*/		/* VxWorks TCP/IP        				*/
/*#define XSTK_XBSD*/			/* XBSD TCP/IP from Xact				*/

/*
 * Define the Standard library package the X-files work under
 * Choose one. The truth is really, really out there...
 */
#define XLIB_NUC			    /* Nucleus ATI standard libraries		*/
/*#define XLIB_BORLAND*/		/* Borland for Nucleus in DOS			*/
/*#define XLIB_PSOS*/			/* pSOS standard libraries				*/
/*#define XLIB_VXWORKS*/		/* VxWorks standard libraries			*/
/*#define XLIB_XBSD*/			/* XBSD standard libraries				*/
/*#define XLIB_GNU*/			/* GNU library support					*/
/* #define XLIB_XSNMP */		/* Use Internal Library support			*/

/*
 * Define the MIB Engines that this SNMP will have knowledge of
 */
#define XMIB_MIB2				/* RFC1213, MIB II						*/
/*#define XMIB_RMON1*/              /* RFC1757, RMON MIB 1                  */

/*---------------------- XSNMPv1/MIB2 Defines --------------------------*/
/*
 * Define location of RFC1213 storage; DEFINED if local to XSNMPv1
 * UNDEFINED if defined in TCP/IP stack underneath
 */
/* #define INTERNAL_RFC1213_STORAGE */
/*#define XSNMPV1_SHOW_BANNER*/

#define XSNMP_MIB				1,3,6,1,4,1,2993,1
#define XSNMP_SNMP_PORT			161
#define XSNMP_SNMP_TRAP_PORT	162

/* Maximum ports used to be 1, changed to 50 to test - will eventually be (if not
	dynamic) the total boards and interfaces I believe??  I forget because I'm doing
	this some time after and just noticed the change (rcb) */
#define MAX_PORTS               75
#define MAX_COMMS				8
#define MAX_HOSTS				16

#define MIBII_SYSCONTACT        "                       "
#define MIBII_SYSDESCRIPTION    "                       "
#define MIBII_SYSLOCATION       "                       "
#define MIBII_SYSNAME           "                       "
#define MIBII_OBJECTID          "                       "
#define MIBII_SERVICES          72
/*--------------------End of XSNMPv1/MIB2 Defines ----------------------*/
/*------------------- XRMON1/XRMON1-Lite Defines -----------------------*/

#define XRMON_MAXHOSTS			512		/* Maximum size of Host table	*/
#define XRMON_MAXHOSTPAIRS		512		/* Maximum size of Matrix table	*/
#define XRMON_MAXNODES			512		/* Must match above two values	*/
#define XRMON_MAXBUCKETS		10		/* Number of history buckets	*/
#define XRMON_MAXLOGS			10		/* Maximum logs					*/
#define XRMON_MAXTOPNS			512		/* Maximum HostTopNs			*/
#define XRMON_MAXPKTSIZE		2048	/* Buffer size for each capture	*/
#define XRMON_MAXRIPS			128		/* Maximum capture buffers		*/

#define XRMON_CBUFSIZE			(XRMON_MAXRIPS * XRMON_MAXPKTSIZE)
/*------------------- End of XRMON1/XRMON1-Lite Defines -----------------*/

/*====================== End of User Defines ============================*/



/*==================  Operating System Includes/Defines =================*/
/*
 * Globally used structures, defines, typedefs, ...
 */

typedef struct xq_s {
    struct xq_s        *xq_next;
    struct xq_s        *xq_prev;
} xq_t;

/*
 * OS/Stack Specific defines and include files
 */
#ifdef XOS_NUC
#ifndef NU_DEBUG
#define NU_DEBUG
#endif
#ifndef NU_NO_ERROR_CHECKING
#define NU_NO_ERROR_CHECKING
#endif
#include "nucleus.h"

#include "ansi/stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "stdcs.h"
#include "odyssey_trace.h"

#ifdef XOS_NUCDOS
#include "externs.h"
#include <stdio.h>
#include <stdlib.h>
#include <String.h>
#include <mem.h>
#include <alloc.h>
#include <tchar.h>
#include <alloc.h>
#endif

typedef struct tmr_s {
    struct tmr_s    *forw;
    struct tmr_s    *back;
    void            (*func)(void *);
    void            *arg;
    NU_TIMER        *tmr_ptr;
    unsigned char   name[8];
} tmr_t;

#define SIN_ADDR_LEN	4
#endif

#ifdef XOS_PSOS
#include <psos.h>
#include <ctype.h>
#include <probe.h>
#include <prepc.h>
#include <stdio.h>
#define isascii(c)  ((unsigned)(c) <= 0177)
#endif

/*==================  Library Includes/Defines ==========================*/
#ifdef XLIB_GNU
#define isascii(c)  ((unsigned)(c) <= 0177)
#endif
#ifdef XLIB_XSNMP
#define isascii(c)  ((unsigned)(c) <= 0177)
#endif
/*==================  TCP/IP Stack Includes/Defines =====================*/

#ifdef XSTK_NUCNET
#define _BSD_SOCKETS_

#include "target.h"
#include "mem_defs.h"
#include "socketd.h"
#include "xinclude/types.h"
#include "xinclude/socket.h"
#include "xinclude/netdb.h"
#include "xinclude/time.h"
#include "xinclude/errno.h"
#include "xinclude/in.h"



#endif

#ifdef XSTK_PSOS
#include <pna.h>
#endif

#ifdef XSTK_VXWORKS
#endif

#ifdef XSTK_XBSD
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/tftp.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <setjmp.h>
#include <syslog.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <machine/endian.h>
#endif

#endif /* _XPORT_H_ */
