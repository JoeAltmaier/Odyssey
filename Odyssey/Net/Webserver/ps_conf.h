/*************************************************************************/
/*                                                                       */
/*       Copyright (c) 1993-1996 Accelerated Technology, Inc.            */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      ps_conf.h                                       WEBSERV 1.0      */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains all user modifiable switches that allow       */
/*      extended features to be supported by the Nucleus Web Server.     */
/*      At this time all features are supported except for Authentication*/
/*      and encryption.  If these items are define the source code will  */
/*      Not compile.  Also if the hardware platform you are working on   */
/*      does not support printfs.  Then don't turn on debugging.         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* PicoServer Embedded Web Server                                        */
/*                                                                       */
/* Copywrite (c) 1995 1996 1997 CNiT                                     */
/*                                                                       */
/* Communication and Information Technology                              */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*************************************************************************/





/*      This file is used to configure (turn on/off) various    
 *      Nucleus WebServer features.                     
 */





/* To turn all Nucleus WebServer extended features on  insert the line
 * #define ALL_F_ON
 * after the comment.
 * (Note: this overrides the feature specific defines below)
 */


/* To turn all Nucleus Webserver extended features off  insert the line
 * #define ALL_F_OFF
 * after the comment.
 * (Note: this overrides the feature specific defines below)
 */


/* Nucleus WebServer can support a filesystem on mass storage or
 * a memory based filesystem that is compiled into the system
 * or uploaded useing the file upload feature. 
 * By default the in-memory file system is set up.
 * If you would like to use the File Abstraction layer comment out the
 * #define FS_IN_MEMORY.
 * To enable an in memory filesystem:
 * #define FS_IN_MEMORY
 */

#define FS_IN_MEMORY

 /* Enable the Nucleus WebServ directory command
 * This feature is a plugin that lists
 * the users incore filesystem on the
 * users browser.
 */

#define LIST_DIRECTORY



/* Nucleus WebServ can support file uploading useing multi-part mime
 * To enable file uploading 
 * #define FILE_UPLOAD
 */

#define FILE_UPLOAD             /* include file upload processing*/


/* Nucleus Webserver can support file compression of embedded HTML
 * and other files
 * To support transparent file compression for both mass-storage
 * and in memory filesystems
 *
 * #define FILE_COMPRESSION
 */


#undef FILE_COMPRESSION

 /* Nucleus WebServer can support secure user authentication 
  * useing a Java applet and DES encryption support in the server.
  * To support hard encrypted authentication. To enable DES authentication
  * uncomment the #define AUTHENTICATION and the #define AUTH_40_ENTICATION.
  */

/* #define AUTHENTICATION     */
/* #define AUTH_40_ENTICATION */
/* to select 40bit DES authentication 
  
  *#define AUTH_56_ENTICATION
  * to select 56bit DES authentication
  * 
  * Note: only one of either (40 or 56 bit) authentication
  * may be enabled at one time.
  */

  /*  To select Basic Authtentication as defined in HTTP 1.0 uncomment the
 *  the #define BASIC_AUTH.
 */

/* #define BASIC_AUTH */

/* END OF USER CONFIGURATION SECTION */


#ifdef AUTH_56_ENTICATION
#ifndef AUTH_PLUGIN
#define AUTH_PLUGIN
#endif
#endif /* AUTH_56_ENTICATION */

#ifdef AUTH_40_ENTICATION
#ifndef AUTH_PLUGIN
#define AUTH_PLUGIN
#endif
#endif /* AUTH_40_ENTICATION */

/* TURN ALL THE FEATURES ON IF USER SELECTED ALL_F_ON */

#ifdef ALL_F_ON

#ifndef FS_IN_MEMORY
#define FS_IN_MEMORY
#endif

#ifndef FILE_UPLOAD
#define FILE_UPLOAD
#endif

#ifndef LIST_DIRECTORY
#define LIST_DIRECTORY
#endif

#if 0
#ifndef FILE_COMPRESSION
#define FILE_COMPRESSION
#endif
#endif
#ifndef AUTHENTICATION
#define AUTH_40_ENTICATION
#endif

#ifndef BASIC_AUTH
#define BASIC_AUTH
#endif

#endif

/* TURN ALL THE FEATURES OFF */

#ifdef ALL_F_OFF

#ifdef FS_IN_MEMORY
#undef FS_IN_MEMORY
#endif

#ifdef FILE_UPLOAD
#undef FILE_UPLOAD
#endif

#ifdef LIST_DIRECTORY
#undef LIST_DIRECTORY
#endif

#ifdef FILE_COMPRESSION
#undef FILE_COMPRESSION
#endif

#ifdef AUTHENTICATION
#undef AUTHENTICATION
#endif

#endif


#include "ps_nuc.h"


