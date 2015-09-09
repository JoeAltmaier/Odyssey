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
/*      ps_dir.c                                        WEBSERV 1.0      */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      File that contains the DIR plugin information.                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      list_directory                  Plugin to list all files in the  */
/*                                      in memory or os dependent file   */
/*                                      system.                          */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*     ps_pico.h                        Defines and Data Structures      */
/*                                      related to Nucleus Webserv       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

#include "ps_pico.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef LIST_DIRECTORY

#define OUTBLEN 1024
#define OUTBMAX 900

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      list_directory                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function gets the structure of the file system and outputs  */
/*      to the HTTP client the directory structure and the number of     */
/*      embedded files.  This operation is a plugin and must be          */
/*      registered within the server before being used.                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*     process_GET                      Processes an HTTP response by    */
/*                                      using the GET method.            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_proto_status                 Sets up the HTTP response.       */
/*      ps_outhdr_insert                Inserts the name and value into  */
/*                                      the output header of the HTTP    */
/*                                      response.                        */
/*      ps_proto_start                  Starts the HTTP response.        */
/*      ps_net_write                    Writes data out to the Network.  */
/*      strlen                          Finds the length of a string.    */
/*      sprintf                         Prints a message into a string.  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      tok                             Pointer to Token structure that  */
/*                                      holds current token arguments    */
/*                                      (Name and Value) and pointer to  */
/*                                      next token arguments.            */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void list_directory( Token * tok,Request * req )
{
    int num_ent=0,x;
    char * name;
    char * file;
    char outb[OUTBLEN];

    struct fs_file * f;

    extern struct fs_file * fs_fil;


    /*  Remove Warnings */
    tok = tok;

    ps_proto_status( req,PROTO_OK );
    ps_outhdr_insert( CONTENT_TYPE, TYPE_TXT_HTML,req->response );
    ps_proto_start_response(req);

    name = "/";

    outb[0]=0;
    sprintf( outb+strlen( outb ),"<HTML><HEAD>\n<TITLE>Index of %s", name );
    sprintf( outb+strlen( outb ),"</TITLE>\n</HEAD><BODY>\n" );
    sprintf( outb+strlen( outb ),"<H1>Directory Index of %s</H1>\n", name );
    sprintf( outb+strlen( outb ),"<HTML><HEAD>\n<TITLE>Index of %s",name );
    sprintf( outb+strlen( outb ),"</TITLE>\n</HEAD><BODY>\n" );

    ps_net_write( req,outb,strlen( outb ));

    outb[0]=0;
    f = fs_fil;

    while( f )
    {
	file = f->name; 

	if( ( name[0] == '/' ) && (name[1] == '\0' ) )
	    sprintf( outb+strlen( outb ),"<A HREF=\"%s\"> %s</A><BR>\n",file,file );
	else
	    sprintf( outb+strlen( outb ),
		"<A HREF=\"%s/%s\"> %s</A><BR>\n",name,file,file);
		
		ps_net_write( req,outb,strlen( outb ));
		outb[0]=0;
	
		num_ent++;

	    f = f->next;
    }
    x = strlen( outb );
    if( x )
	ps_net_write( req,outb,x );

    sprintf( outb,"<BR>Total entries %d <BR>\n",num_ent );
    ps_net_write( req,outb,strlen( outb ));
}
#endif /* LIST_DIRECTORY */
