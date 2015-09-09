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
/*      ps_serv.c                                       WEBSERV 1.0      */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file holds the routines that are the heart of the Nucleus   */
/*      Webserv product. The function within are used to maintain all    */
/*      facets of the Webserv Product.  At this time the following       */
/*      are not shipping with the Nucleus Webserv product:               */
/*      Authentication and Encryption.  All other features are           */
/*      supported.                                                       */    
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
/*      mime_table                      Holds the mime extensions.       */
/*      fs_fil                          Structure for the embedded file  */
/*                                      system.                          */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      make_mime_proto                 Generate HTTP header with the    */ 
/*                                      correct mime codes.              */
/*      is_authenticated                                                 */
/*      check_timeout                   Checks the timeout.              */
/*      md_send_file                    Send file to browser over        */
/*                                      network.                         */
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*      process_POST                    Processes Post form commands.    */
/*      client_error                    Error Routine for the client.    */
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/*      is_ssi                          Checks for an SSI(Server Side    */
/*                                      Include) tag.                    */
/*      in_string                       Compares string to actual buffer.*/                                      
/*      process_ssi                     Process a Get Operation with the */
/*                                      .SSI extension.                  */
/*      serve_file                      This function sends the file to  */
/*                                      the client over the network.     */
/*      server_error                    Prints an error message from the */                               
/*                                      server.                          */
/*      ps_proto_status                 Sets up the HTTP response.       */
/*      ps_outhdr_insert                Inserts the name and value into  */
/*                                      the output header of the HTTP    */
/*                                      response.                        */
/*      ps_proto_start_response         Starts the HTTP response to the  */
/*                                      client.                          */
/*      alloc_token                     Allocates token structure from a */
/*                                      fixed sized pool.                */
/*      ps_auth_ctl                     Authentication Control routine.  */
/*      ps_server_ctl                   Server Control Routine.          */
/*      find_auth_ip                    Check if the ip address is       */
/*                                      already authenticated.           */
/*      ip_match                        Compares ip addresses for a      */
/*                                      match.                           */
/*      send_client_html_msg            Sends HTML error message to      */
/*                                      client.                          */
/*      ps_proto_redirect               Redirects the  user agent to     */
/*                                      another URL.                     */
/*      setup_pblock                    Parses the  clients plugin       */
/*                                      arguements into tokens.          */
/*      fs_file_match                   Search for client requested file */
/*                                      name.                            */
/*      global_access                   Check what is global access set  */
/*                                      to.                              */
/*      add_authenticated               Adds an authenticated user       */                                 
/*                                      to the auth structure.           */
/*      free_auth                       Frees authentication structure   */
/*                                      for a particular ip address.     */
/*      ps_outhdr_nninsert              Places name and numeric value    */
/*                                      token into output header.        */
/*      ps_reg_plugin                   Register plugins.                */
/*      ps_get_server                   Retireves information about the  */
/*                                      server.                          */
/*      md_file_stat                    Checks to see if the file is     */
/*                                      in memory or os-dependent file   */
/*                                      system.                          */
/*      pico_init                       Initializes Nucleus Web Server.  */
/*      find_token                      Finds a token in a string.       */
/*      hex2                            Convert binary to two hex        */
/*                                      digits.                          */
/*      packed_hex_2bin                 Convert packed hex to binary.    */
/*      ps_a2hex                        Convert hex to binary.           */
/*      hexa                            Converts Ascii Hex nibble to     */
/*                                      binary.                          */
/*      a2hex                           Converts a hex digit into binary.*/
/*      ps_token_findval                Find a the value of a token name.*/
/*      ps_proto_finish_request         Flushes the output buffer.       */
/*      ps_mime_type                    Derives a mime type.             */
/*      ps_proto_uri2url                Converts the uri to full URL.    */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      ps_pico.h                       Defines and Data Structures      */
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
#include "stdlib.h"
#include "string.h"
#ifdef BASIC_AUTH
#include "b_auth.h"
#endif
void        process_GET(Request* req);
void        serve_file(Request * req);
void        process_ssi(Request *req);
int         is_ssi(Request * req);
void        process_POST(Request * req, char* line);
Token       *setup_pblock(char * s);
Token       *alloc_token(void);
int         fs_file_match(char * s, char ** start, int * length, int * type);
int         is_authenticated(Request * req);
int         ip_match(unsigned char * a, unsigned char * b );
void        free_auth(ps_auth * a);


char        *find_token(char * token, char * buf, char * last);

void        send_auth_salt(Request * req);
void        make_new_salt(ps_auth * a);
void        send_auth_client(Request *req,char * salt);
void        auth_connection(Request * req,char * line);

void        md_send_file(Request * req);
int         add_authenticated(Ps_server *server, char * ipaddr);
ps_auth     *find_auth_ip(Ps_server * server , char * ip );
int         global_access(Request * req);
void        check_timeout(Request * req);
unsigned    int a2hex(unsigned char* s);
void        hex2(char c,char * buf);


int         hexa(unsigned char * s); 
void        packed_hex_2bin(char * d, char * s ,int count);

	     

void        client_error(Request * req, char * reason);
void        pico_init(void);

char        *ps_mime_type(Request * req);
void        make_mime_proto(Request * req,int length);


struct      plugin plugins[MAX_PLUGIN];
extern      Ps_server master_server;
extern      void auth_init(void);
int         tokens_allocated=0;
Token       *free_tokens;


/*  Place Plug-in externals Here  for use in ps_INIT */
extern int             ip_addr(Token * env, Request * req);
extern int             task_stat(Token * env, Request * req);
extern int             task_change(Token * env, Request * req);
extern int             process_answer(Token * env, Request * req);


char text_404[]="\
HTTP/1.0 404 Not Found\r\n\
Content-type: text/html\r\n\r\n\
<HEAD><TITLE>404 Not Found</TITLE></HEAD>\r\n\
<BODY><H1>404 Not Found</H1>\r\n\
Url '%s' not found on server<P>\r\n</BODY>";

char *upload_htm[]= {
    "<html>\r\n",
    "<head>\r\n",
    "<title>Nucleus Webserv Example of Uploading a File Via HTTP</title>\r\n",
    "</head>\r\n",
    "<body bgcolor=\"#FFFFFF\">\r\n\
    <h3><font color=\"#980040\" size=\"5\">ATI's Example of a Post Operation for a\
    File Upload!</font></h3>\r\n",
    "<h2><font color=\"#980040\" size=\"4\">To Upload a file via the Web, fill out the required\
    elements below:</font></h2>\r\n",
    "<FORM ENCTYPE=\"multipart/form-data\" ACTION=\"/upload\" METHOD=POST>\r\n",
    "<font color=\"#980040\" size=\"3\">Filename To Be Saved As On Server:</font>\
	<input name=\"save-as-filename\" size=35><p>\r\n",
    "<font color=\"#980040\" size=\"3\">Filename On Your Computer:</font>\
	<INPUT NAME=\"upload-file\" TYPE=\"file\"><p>\r\n",
    "<INPUT TYPE=\"submit\" VALUE=\"Send File\">\r\n",
    "</FORM>\r\n",
    "</body>\r\n",
    "</html>\r\n",
    ""};

/* the mime table maps file types to mime extensions */
extern NU_MEMORY_POOL    System_Memory;
struct fs_file * fs_fil;

struct mime_tab mime_table[]=
{
	"txt",  "text/plain",
	"text", "text/plain",
	"html", "text/html",
	"htm",  "text/html",
	"gif",  "image/gif",
	"jpg",  "image/jpeg",
	"jpeg", "image/jpeg",
	NULL,NULL
};


Token error_token={ {"OUT_OF_TOKEN_SPACE","ERROR"},(struct token *) 0 };

char *CRLFCRLF="\r\n\r\n";      /* handy constant */
char *CRLF="\r\n";              /* Start I.E. Explorer Read */ 

int DEBUG1 = 0;
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      parse_http_request                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      We parse the HTTP request and call                               */
/*      the correct handler. Note that HEAD is exactly                   */
/*      like GET except no date is returned (only the                    */
/*      response headers).                                               */
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
/*                                                                       */
/*      pico_worker                     Receives a connection and        */
/*                                      processes the HTTP request.      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      is_authenticated                                                 */
/*      check_timeout                   Checks the timeout.              */
/*      md_send_file                    Send file to browser over        */
/*                                      network.                         */
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*      process_POST                    Processes Post form commands.    */
/*      client_error                    Error Routine for the client.    */
/*      global_access                   Check what is global access set  */
/*                                      to.                              */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      line                            Contains the HTTP characters that*/
/*                                      are to be parsed.                */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to the HTTP request.             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Always Returns zero.                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int parse_http_request(Request * req,char * line)
{
    int  j;
    char * s;
  	Ps_server* p;

	p = ps_get_server(); 


    j= line[0] + line[1] + line[2];
    req->method= j;



    switch( j )
    {

	case GET:
	case POST:
	case HEAD:
    break;

	default:
#ifdef DEBUG 
	printf("Could not decode request\n");
	printf("C:%s\n",line);
#endif /*DEBUG*/
	server_error("could not decode request");
	return(0);
    }

    check_timeout(req);                             /* check for authentication timeout */


	
    if( (j== HEAD) || (j==POST) || (j==GET ) )
    {
	s=line;
	while (*s != ' ' )s++;
	s++;
	req->fname = s;
	while (*s != ' ')s++;
	*s = '\0';
	line=s+1;

    }
#ifdef BASIC_AUTH
    if (b_parse_auth(req)!= NU_SUCCESS)
        return(BASIC_AUTH_FAILED);
#endif

    /* if the user has not been authenticeated
     * allow them the choice to login.
     */


    if( (is_authenticated(req)==FAILURE) && ((global_access(req)) == FAILURE))
    {
        	ps_auth_ctl(p,A_DISABLE,NULL);
            req->server->master.auth_state= 1;
	        req->fname =req->server->master.auth_uri;
	        md_send_file(req);
	        goto out;
    }

    switch(j)
    {

	case    GET:
	case    HEAD:
	process_GET(req);
	break;

	case    POST:
	process_POST(req,line);
	break;

	default:
#ifdef DEBUG
	printf("Could not decode request\n");
	printf("C:%s\n",line);
#endif
	client_error(req,"can not decode request");
	
    }                                               /* end switch */
				
    /* The request is over
     * do any cleanup and exit
     */
out:

    tokens_allocated = 0;                           /* reclaim token  pool */
	
	
    return(0);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      process_GET                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      The GET request requests the return or                           */
/*      URI on the server. This is the most used                         */
/*      method and is how the client request fixed                       */
/*      most fixed Web Pages.                                            */
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
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_proto_redirect               Redirects the  user agent to     */
/*                                      another URL.                     */
/*      md_file_stat                Checks to see if the file is         */
/*                                  in memory or os-dependent file       */
/*                                  system.                              */
/*      ps_net_write                    Writes data out to the Network.  */
/*      is_ssi                          Checks for an SSI(Server Side    */
/*                                      Include) tag.                    */
/*      in_string                       Compares string to actual buffer.*/                                      
/*      process_ssi                     Process a Get Operation with the */
/*                                      .SSI extension.                  */
/*      serve_file                      This function sends the file to  */
/*                                      the client over the network.     */
/*      server_error                    Prints an error message from the */                               
/*                                      server.                          */
/*      sprintf                         Prints a message into a string.  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*       req                            Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to the HTTP request.             */
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

void  process_GET(Request* req)
{
	
	
    int i,len;
    struct ps_stat stat;
    int (*plugin)(Token *, Request *)= NULL;
    char * s;
    char buf[260];

#ifdef DEBUG
    printf("GET:%s\n",req->fname);
#endif
    s=in_string("?",req->fname);
    if( s )
    {

	*s++='\0';
	req->pg_args = setup_pblock(s);
    }
    else
	req->pg_args = alloc_token();

	/* if we reference the root node "/" of our filesystem
	 * redirect the browser to the default URL.
	 * This is usually defined to be either default.htm 
	 * or index.htm
	 */

	if( (req->fname[0] == '/') && (req->fname[1] == '\0') )
	{
#ifdef HOST_IP
	    ps_proto_redirect(req, ps_proto_uri2url(req,DEFAULT_URI));
	    return;
#else
	    req->fname = DEFAULT_URI;
#endif
	}

	    req->stat = &stat;
	i = md_file_stat(req); 

    switch( i )
	{

	case ENOFILE:
	    sprintf(buf,text_404,req->fname);
	    len= strlen(buf);
	    ps_net_write(req,buf,len);
	    break;

	    case SUCCESS:

	    if( req->stat->flags == PLUGIN )
	    {
		plugin = req->stat->plugin;
		(plugin)(req->pg_args,req);
		break;
	    }

	    if( is_ssi(req) )                       /* check for server side include */ /*FIX SSI document how to */
		{

	      process_ssi(req);
		}
	    else {
    	    serve_file(req);
	   }
	    break;

	    default:
	    server_error("Fallthru get");
	    break;
	}
		
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      serv_file                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends a file to the client(web browser).           */
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
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      md_send_file                    Send file to browser over        */
/*                                      network.                         */
/*      make_mime_proto                 Generate HTTP header with the    */ 
/*                                      correct mime codes.              */
/*      os_send_file                    Write the os file to socket.     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
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

void  serve_file(Request * req)
{
    if( req->stat->flags & INCORE )   {
	/* an in-memory file...simply send down the socket */
        md_send_file(req); 
	} else {

	/* setup the protocol to generate the response headers for the trasfer */

	make_mime_proto(req,req->stat->size);

	/* all that remains  is to copy the data to the socket and return */
	os_send_file(req);                          /* OS-dependent serve file  */ 
    }
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      process_ssi                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function processed server side includes.                    */
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
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      os_read_file                    Copy the OS file to an array.    */
/*      ps_proto_status                 Sets up the HTTP response.       */
/*      ps_outhdr_insert                Inserts the name and value into  */
/*                                      the output header of the HTTP    */
/*                                      response.                        */
/*      ps_net_write                    Writes data out to the Network.  */
/*      ps_proto_start_response         Starts the HTTP response to the  */
/*                                      client.                          */
/*      ps_mfree                        Deallocates memory with a call   */
/*                                      to NU_Deallocate_Memory.         */
/*      strcmp                          Compares one string with another.*/
/*      in_string                       Compares string to actual buffer.*/                                      
/*      strcpy                          Copies one strings contents into */
/*                                      another strings contents.        */
/*      strcat                          Concantenates two strings.       */
/*      find_token                      Finds a token in a string.       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
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

void  process_ssi(Request *req)
{
    char * file;
    char * insert;
    char * p, * q;
    char plugin_s[SSI_NAMELEN];
    char ssierror[SSI_LINE];
    char *last_start;
    int (*plugin)(Token *, Request *)= NULL;
    Token * t=0;
    int i;
    int     ssilen;
    char * last_byte;
    char * ssi_buf = req->rdata->ssi_buf;

    if( (req->stat->flags & INCORE ) == 0 )         /* this file has to be read in */
    {
	file = (char *)ps_malloc( req->stat->size );

	if(file != (char * ) FAILURE)
	{
	    os_read_file(req,file); /*FIX */
	}
	else
				
	    return;
    }
    else
	file = req->stat->address;

    last_byte = file  + req->stat->size;

	/* setup all the protocol for a transfer */

    ps_proto_status(req,PROTO_OK);
    ps_outhdr_insert(CONTENT_TYPE, TYPE_TXT_HTML,req->response);
    ps_proto_start_response(req);

    last_start = file;
    insert = (find_token(SSI_TOKEN, last_start,last_byte));
    while( insert )
    {

	  p =insert;
	  q = ssi_buf;
	  plugin= NULL;

	  while( *p != '>' && ( (p - insert) < SSI_LINE ) )

						    /* copy tag into buffer  */
	    {
		*q++ = *p++;
	    }
			
	    if( *p == '>' )
		*q='\0';
	    else ssi_buf[SSI_LINE-1]='\0';
	    ssilen = strlen(ssi_buf);               /* size of the ssi tag */

	    /* parse the hard way */

	    q = plugin_s;
	    p = ssi_buf;
	    while( *p !='#' ) p++;
	    p++;
	
	    while(*p != ' '){ *q++ = *p++; }
	    *q='\0';


	    /* now we have the plugin name in the plugin[] array */

	
	    q=p;
	
	    while( *q == ' ' )q++;

	    if( in_string("=",q) == FAILURE )
	    {

		req->pg_args = alloc_token();       /* an empty argument list */

	    }
	    else
	    {                                       /* dose it look like NAME=VALUE ?*/

		if( in_string("~",q) )
		{
		    while(*p != '~')p++;            /* terminate argument string */
		    *p='\0'; 
		}

		req->pg_args = setup_pblock(q);     /* process the arguments */
	    } 

	    /* lookup the plugin in the table */
	    p = plugin_s;
	    while(*p == '/') p++;

	    for(i=0; i<MAX_PLUGIN; i++ )
	    {
		if( plugins[i].name )
		{
		    if( strcmp(p,plugins[i].name) == 0 )
		    {
			plugin = plugins[i].plugin;
							
		    }
		}
	    }


	    ps_net_write(req,last_start, (insert - last_start) );
	    last_start = (insert+ssilen)+1;

	    if( plugin == NULL )
	    {
		
		strcpy(ssierror,plugin_s);
			strcat(ssierror," CANT FIND  SSI plugin");
			ps_net_write(req,ssierror,strlen(ssierror));
			break;
	    }

	    (plugin)(t,req);
	    insert = (find_token(SSI_TOKEN, last_start,last_byte));
	}

	/* write out last piece if any */

	if( last_start < (file + req->stat->size) )
	   ps_net_write(req,last_start,(file + req->stat->size) - last_start);

	if( (req->stat->flags & INCORE ) == 0 )
	   ps_mfree(file);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      is_ssi                                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      The function checks to see if the current file name is a file    */
/*      marked for server side include processing.                       */
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
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      in_string                       Compares string to actual buffer.*/                                      
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      This function returns SUCCESS if the ssi tag is found.  It       */
/*      it returns FAILURE if it cannot find the ssi tag.                */  
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/


int is_ssi(Request * req)
{


	if( (in_string(SSI_MARKER,req->fname)) != FAILURE )
		return(SUCCESS);
	else
		return(FAILURE);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       process_POST                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      The POST method specifies a URI and                              */
/*      includes optional user input data from                           */
/*      HTML forms screens.  Plugin's  can be launched                   */
/*      thru here.                                                       */
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
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/* CALLS                                                                 */
/*                                                                       */
/*      client_error                    Function to print out an error   */
/*                                      to the client.                   */
/*      ps_proto_status                 Sets up the HTTP response.       */
/*      ps_outhdr_insert                Inserts the name and value into  */
/*                                      the output header of the HTTP    */
/*                                      response.                        */
/*      ps_proto_start_response         Starts the HTTP response to the  */
/*                                      client.                          */
/*      ps_net_write                    Writes data out to the Network.  */
/*      in_string                       Compares string to actual buffer.*/                                      
/*      strcpy                          Copies one strings contents into */
/*                                      another strings contents.        */
/*      strcat                          Concantenates two strings.       */
/*      strcmp                          Compares one string with another.*/
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*      line                            String to be parced that includes*/
/*                                      value and name arguements.       */
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

void  process_POST(Request * req, char* line)
{
    char * s; 
    int i;
    Token * t=0;
    char posterror[SSI_LINE];
    struct ps_stat ps;
    int (*plugin)(Token *, Request *)= NULL;
   
    req->stat = &ps;
    if( (in_string("multipart/form-data",line)) == FAILURE)
    {
	     if ((s= in_string("\r\n\r\n",line)) != (char *) FAILURE)
         {
             s = s+4;
             req->pg_args = setup_pblock(s);
         }
         else
         {
             t = alloc_token();
             req->pg_args = t;
         }
   }
	
    s = req->fname;
    while(*s =='/') s++;
    for(i=0; i<MAX_PLUGIN; i++ )
    {
	if(plugins[i].name )
	{
	    if( strcmp(s,plugins[i].name) == 0 )
	    {
		plugin =plugins[i].plugin;
		break;
	    }
	}
    }

    if( plugin == NULL )
    {
		
	/* Print error on browser*/
	ps_proto_status(req,PROTO_OK);
	ps_outhdr_insert(CONTENT_TYPE, TYPE_TXT_HTML,req->response);
	ps_proto_start_response(req);
	strcpy(posterror,"Nucleus WebServ: cant find plugin: ");
	strcat(posterror,req->fname);
	ps_net_write(req,posterror,strlen(posterror));

	return;
    }

    i = ((plugin)(t,req));

    /*FIX post plugin error check */
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      send_client_html_msg                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends an error message to a client when called.    */
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
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      client_error                    Error routine for Client.        */
/*      ps_proto_status                 Sets up the HTTP response.       */
/*      ps_outhdr_insert                Inserts the name and value into  */
/*                                      the output header of the HTTP    */
/*                                      response.                        */
/*      ps_proto_start_response         Starts the HTTP response to the  */
/*                                      client.                          */
/*      ps_net_write                    Writes data out to the Network.  */
/*      sprintf                         Prints a message into a string.  */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*      mes                             The message to be sent           */
/*                                      to the client.                   */
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

void  send_client_html_msg(Request * req, char * mes)
{
    char *buf;
    buf = &req->rdata->ssi_buf[0];

#ifdef DEBUG
    printf("mes = %s\n",mes);
#endif

    /* Print error on browser*/
    ps_proto_status(req,PROTO_OK);
    ps_outhdr_insert(CONTENT_TYPE, TYPE_TXT_HTML,req->response);
    ps_proto_start_response(req);
    sprintf(buf,"Nucleus WebServer:  %s",mes);
    ps_net_write(req,buf,strlen(buf));
    ps_proto_finish_request(req);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      setup_pblock                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function Parses and converts the clients plug-in            */
/*      arguments into Token structures.                                 */
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
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*      process_POST                    Processes Post form commands.    */
/*      process_ssi                     Process a Get Operation with the */
/*                                      .SSI extension.                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      alloc_token                     Allocates token structure from a */
/*                                      fixed sized pool.                */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      s                               String that holds the clients    */
/*                                      plugin arguements.               */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      t                               Pointer to Token structure that  */
/*                                      holds current token arguments    */
/*                                      (Name and Value) and pointer to  */
/*                                      next token arguments.            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

Token * setup_pblock(char * s)
{
    char * n=0;
    char * v=0;
    Token * t;
    Token * u;

    t = alloc_token();
    u = t;
    while(*s )
    {

	n = s;

	while( (*s) && (*s != '=') ) s++;
	    if( ! *s ) break;
	    *s++ ='\0';
	    v= s;            

	while( (*s) && (*s != '&') ) s++;
	    if( ! *s ) break;
	    *s++ ='\0';

	    u->arg.name=n;
	    u->arg.value=v;

//#ifdef DEBUG
//	printf("NAME= %s\n",n);
//	printf("VALUE= %s\n",v);
//#endif

		
	u->next = alloc_token();
	u = u->next;
    }

	u->arg.name=n;
	u->arg.value=v;

//#ifdef DEBUG
//printf("NAME= %s\n",n);
//printf("VALUE= %s\n",v);
//#endif
	
	return(t);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      alloc_token                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function allocates a token structure from a fixed           */
/*      sized pool.  If more Tokens are needed                           */
/*      recompile with a larger TOKEN_HEAP.                              */
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
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*      process_ssi                     Process a Get Operation with the */
/*                                      .SSI extension.                  */
/*      setup_pblock                    Parses the  clients plugin       */
/*                                      arguements into tokens.          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      t                               Pointer to Token structure that  */
/*                                      holds current token arguments    */
/*                                      (Name and Value) and pointer to  */
/*                                      next token arguments.            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

Token * alloc_token()
{
    Token * t;

    if( tokens_allocated < TOKEN_HEAP )              
	t = &free_tokens[tokens_allocated++];
    else 
	t = &error_token;

	t->next = NULL;
	t->arg.name = "";
	t->arg.value=0;

	return(t);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      fs_file_match                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function searches through the in memory based file system   */
/*      for a client requested filename.                                 */ 
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
/*      md_file_stat                Checks to see if the file is         */
/*                                  in memory or os-dependent file       */
/*                                  system.                              */
/*      md_send_file                Send file to browser over            */
/*                                  network.                             */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      strcmp                      Compares one string with another.    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      s                           Filename to be examined.             */
/*      start                       Pointer to start location in memory  */
/*                                  of the file.                         */
/*      length                      Pointer to the length of the file.   */
/*      type                        Pointer to the type of file(ie gif,  */
/*                                  htm, jpeg...                         */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      This function returns SUCCESS if the filename was found in the   */
/*      filesystem structure.  It returns FAILURE if it was unable to    */
/*      find the filename within the filesystem structure.               */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int   fs_file_match(char * s, char ** start, int * length, int * type)
{
    struct fs_file *f;

    char * t;

    f=fs_fil;

    if( f == NULL )return (FAILURE);
    while ( *s == '/' ) s++;                        /* bump past any leading /'s */

    f = fs_fil;
    while ( f )
    {

	if( f->length)
	{
	    t= f->name;
	    while ( *t == '/' ) t++;
	    if( strcmp(s,t)==0 )
	    {
		*start = f->addr;
		*length = f->length;
		*type   = f->type;
	
		return(SUCCESS);
	    }
	}
	f= f->next;
    }
    return(FAILURE);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      is_authenticated                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function checks to see if this connection is authenticated. */
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
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      find_auth_ip                    Check if the ip address is       */
/*                                      already authenticated.           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      This function returns SUCCESS if the server  was already         */
/*      authenticated. It returns FAILURE if it was unable to            */
/*      be authenticated.                                                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int is_authenticated(Request * req)
{
    struct ps_auth * a;


    /* is authentication enabled ? */
    

    if(((req->server->master.flags & AUTH_ENABLED) == 0))
    {   
    
#ifdef AUTH_PLUGIN
        /*  Setup login pages to be only access during login */
        if(req->server->master.auth_state == 1)
        {
           if((strcmp(req->fname,"/Login.htm")==0))
               return(SUCCESS);
           if ((strcmp(req->fname,"/Login.class")==0))
               return(SUCCESS);
           if ((strcmp(req->fname,"/ErrorDialog.class")==0))
               return(SUCCESS);
           if ((strcmp(req->fname,"/StreamListener.class")==0))
               return(SUCCESS);
           if ((strcmp(req->fname,"/COM/widget/des/cipher_ctx.class")==0))
               return(SUCCESS);
           if ((strcmp(req->fname,"/COM/widget/des/cipher.class")==0))
               return(SUCCESS);
           if ((strcmp(req->fname,"/COM/widget/util/Hex.class")==0))
               return(SUCCESS);
           if ((strcmp(req->fname,"/ps_rand")==0))
               return(SUCCESS);
           if ((strcmp(req->fname,"/ps_sendurl")==0))
               return(SUCCESS);
           
           return(FAILURE);
        }
#endif
	    return SUCCESS;
    };

    if((a=find_auth_ip(req->server,(char *)req->ip)) ==FAILURE) 
	return FAILURE;
		
    if(a->state & AUTH_GIVEN )
    {
	a->countdown = req->server->master.timeoutval;
	return SUCCESS;
    }
    return SUCCESS;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      find_auth_ip                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to check and see if the ip address is an already        */
/*      authenticated  address.                                          */
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
/*     send_auth_salt                   Sends a packet of random bits to */
/*                                      be combined with the             */
/*                                      authentication data the user     */
/*                                      enters.                          */
/*      is_authenticated                Checks if a particular ip address*/
/*                                      is authenticated.                */
/*      ps_auth_ctl                     Authentication Control routine.  */
/*      check_timeout                   Checks the timeout.              */
/*      pico_init                       Initializes Nucleus Web Server.  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      find_auth_ip                    Check if the ip address is       */
/*                                      already authenticated.           */
/*      ip_match                        Compares ip addresses for a      */
/*                                      match.                           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      server                          Pointer to Ps_server structure   */
/*                                      that contains all local server   */
/*                                      information.                     */
/*      ip                              The ip address to compare with.  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The function returns a if the the ip address is found in the     */
/*      authentication table.  It returns failure if it is not.          */
/*      FAILURE                                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

ps_auth * find_auth_ip(Ps_server * server , char * ip )
{
    ps_auth * a;
	
    /* look for the ip address in the table */

    a =server->master.user_auth;

    while( a )
    {

	if( ip_match((unsigned char *)ip,a->ip) )
	{
	    return(a);
	}                              

	a = a->next;
    }
    return(FAILURE);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      free_auth                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to free the authorization for a particular ip           */
/*      address.                                                         */
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
/*      ps_auth_ctl                     Authentication Control routine.  */
/*      check_timeout                   Checks the timeout.              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      a                               Pointer to authentication        */
/*                                      structure.                       */
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

void  free_auth(ps_auth * a)
{
    a->ip[0]=0;
    a->state = AUTH_FREE;
    a->countdown =0;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      add_authenticated                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to add a client ip address once the user has been       */
/*      found to be authenticated.                                       */
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
/*      ps_auth_ctl                     Authentication Control routine.  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      find_auth_ip                    Traverses through the server     */
/*                                      authentication structure to check*/
/*                                      for an authenticated ip address. */                                
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      server                          Pointer to server structure that */
/*                                      contains list of authenticated   */
/*                                      clients.                         */
/*      ipaddr                          Clients ip address that wishes to*/
/*                                      be authenticated.                */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Function returns SUCCESS if it added the ip address.  It returns */
/*      FAILURE if it was unable to add the autheticated structure.      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int   add_authenticated(Ps_server *server, char * ipaddr)
{
    ps_auth * a;
    int i;

    if( (server->master.flags & AUTH_ENABLED) == 0 ) return(FAILURE);
    if( ( a= find_auth_ip(server, ipaddr) ) == FAILURE)
    {

	a = server->master.user_auth;
	while( a )
	{                                           /* find a free auth structure */

	    if( a->state & AUTH_FREE)
	    break;
	    a = a->next;
	}

	if( a )
	{
	    for(i=0; i< IPADDR; i++)
	    a->ip[i] = ipaddr[i];
	    a->state = AUTH_GIVEN;
	    a->countdown = server->master.timeoutval;
	}
	else
	    return(FAILURE);

    }
	else
	{
	    a->state = AUTH_GIVEN;
	    a->countdown = server->master.timeoutval;

	}
	return(SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      global_access                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Fuction to find out if the requested path is in the global       */
/*      unauthenticated space.   The / means that everything is          */
/*      unauthentciated.                                                 */
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
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      strncmp                         Compares two strings up to a     */
/*                                      specified number of bytes.       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Function returns FAILURE if the client wants a restricted page.  */
/*      It returns SUCCESS if the user has authority to access the page. */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int   global_access(Request * req)
{
    char * g;

    g = req->server->master.auth_public;

    if( (*g == '/') && (*(g+1) == 0) )              /* if "/" is the global dir everthing is global */
	 return(SUCCESS);

    if( (strncmp(g,req->fname,strlen(g))) == 0 )
	 return(SUCCESS);
    else
	 return(FAILURE);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ip_match                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to compare IP addresses to verify a match.              */
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
/*      find_auth_ip                    Traverses through the server     */
/*                                      authentication structure to check*/
/*                                      for an authenticated ip address. */                                
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      a                               Argument 1 ip address.           */
/*      b                               Argument 2 ip address.           */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      r                               = Success if the ip address was  */
/*                                        found.                         */
/*                                      = Failure if the ip address was  */
/*                                        not found.                     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int ip_match(unsigned char * a, unsigned char * b )
{
    register i,r;

    r= SUCCESS;
    for(i=0; i<IPADDR; i++)
    if( (a[i]&0xff) != (b[i]&0xff) ) r=FAILURE;

    return(r);
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_outhdr_nninsert                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function  place a numeric number into the value of slot of the   */
/*      output header.                                                   */
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
/*      memory_dump                     Plugin used to dump a specified  */
/*                                      memory address and length to an  */
/*                                      HTML page.                       */
/*      make_mime_proto                 Generate HTTP header with the    */ 
/*                                      correct mime codes.              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      sprintf                         Prints a message into a string.  */
/*      strcat                          Concantenates two strings.       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      name                            Name of the token.               */
/*      value                           Numeric Value of the token.      */
/*      outhdr                          The buffer for the output header.*/
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

void  ps_outhdr_nninsert( char * name, int value, char * outhdr)
{
    char num[NUM_LEN];

    sprintf(num,"%d",value);
    strcat(outhdr,name);
    strcat(outhdr,num);
    strcat(outhdr,"\r\n");
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_outhdr_insert                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to insert the name and value intoi the output           */
/*      header.                                                          */ 
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
/*      memory_dump                     Plugin used to dump a specified  */
/*                                      memory address and length to an  */
/*                                      HTML page.                       */
/*      process_ssi                     Process a Get Operation with the */
/*                                      .SSI extension.                  */
/*      arg                             Plugin that checks for an        */
/*                                      argument.                        */
/*      process_POST                    Processes Post form commands.    */
/*      send_client_html_msg            Sends HTML error message to      */
/*                                      client.                          */
/*      ps_proto_redirect               Redirects the  user agent to     */
/*                                      another URL.                     */
/*      make_mime_proto                 Generate HTTP header with the    */ 
/*                                      correct mime codes.              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      strcat                          Concantenates two strings.       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      name                            The name token.                  */
/*      value                           The value token.                 */
/*      outhdr                          The output buffer.               */
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

void  ps_outhdr_insert(char * name, char * value, char * outhdr)
{
    strcat(outhdr,name);
    strcat(outhdr,value);
    strcat(outhdr,"\r\n");
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_reg_plugin                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to register a plugin within the plugin structure.  It   */
/*      takes the plugin function name and uri name and places it within */
/*      this plugin structure.  Every plugin must be registered or the   */
/*      web server will not be able to recognize the plugin when it is   */
/*      called within an HTML file.                                      */
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
/*     auth_init                       Initializes Authentication.       */
/*     pico_init                       Initializes Nucleus Web Server.   */
/*     ps_INIT                         Registers all user plugins        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      plug_in                        The name of the plugin function.  */
/*      uri                            The string used to identify       */
/*                                     plugin.                           */
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

void  ps_reg_plugin(int (* plug_in)(Token *, Request *), char* uri)
{
    int i;

    for(i=0; i<MAX_PLUGIN; i++ )
    {
	if( plugins[i].name == (char*)NULL)
	{
	    plugins[i].name = uri;
	    plugins[i].plugin = plug_in;
	    break;
	}
    }
    if( i == MAX_PLUGIN)  
    {
#ifdef DEBUG
    printf("out of plugin table space\n");
#else 
    ;
#endif 
    }
}

extern Ps_server master_server;



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_get_server                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to return information on the Nucleus Webserv structure. */
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
/*      auth_init                       Initializes Authentication.      */
/*      pico_init                       Initializes Nucleus Web Server.  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      master_server                   Pointer to Nucleus Web Server    */
/*                                      structure.                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

Ps_server * ps_get_server(void)
{
    return( & master_server);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_auth_ctl                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function used to control authentication.                         */
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
/*      auth_init                       Initializes Authentication.      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      add_authenticated               Adds an authenticated user       */                                 
/*                                      to the auth structure.           */
/*      strcpy                          Copies one strings contents into */
/*                                      another strings contents.        */
/*      free_auth                       Frees authentication structure   */
/*                                      for a particular ip address.     */
/*      find_auth_ip                    Traverses through the server     */
/*                                      authentication structure to check*/
/*                                      for an authenticated ip address. */                                
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      server                          Pointer to the server control    */
/*                                      structure.                       */
/*      auth_cmd                        The Control command being issued.*/
/*      arg                             The arguments being passed in.   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      r                                                                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int   ps_auth_ctl(Ps_server* server, int auth_cmd, void * arg) 
{
	
    char ** cs;
    ps_auth * a;
    int r=SUCCESS;
    switch( auth_cmd)
    {

	case A_ENABLE:                              /* enable authentication */
	server->master.flags |= AUTH_ENABLED;
	    break;

	case A_DISABLE:
	server->master.flags &= ~AUTH_ENABLED;
	    break;

	case A_TIMEOUT:
	server->master.timeoutval = (int)arg;
	    break;

	case A_LOGOUT:
	
	a = find_auth_ip(server,(char*)arg);       /* blow away authentication */
	if( a )
		free_auth(a);
	else            r = FAILURE;
	    break;   

	case A_CREDS:

	cs = (char**) arg;

	if( *cs     )strcpy(server->master.key, *cs);	
	    break;

	case A_AUTH_URI:                                                                /* URI to send to unauthenticated user */
	strcpy(server->master.auth_uri,(char*)arg);
	    break;
			
	case A_AUTH_ADD:
	r = add_authenticated(server,(char *) arg); /* add this I/P address to authenticated list */
	    break;   

	case A_AUTH_GLOBL:
	strcpy(server->master.auth_public,(char*)arg);
					     /* public branch of directory space */
	    break;

    }

    return(r);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_server_ctl                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to control server parameters and shutdown the           */
/*      server task.  This function is currently not available.          */
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
/*      None.                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      server                          Pointer to the server control    */
/*                                      structure.                       */
/*      arg                             The arguments being passed in.   */
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

void  ps_server_ctl(Ps_server* server , int  command, void * arg)
{

    switch( command)
    {

	case S_SHUTDOWN:
	server->flags |= S_SHUTDOWN;
	    break;

	case S_PORT:
	server->port = (int) arg;
		
	case    S_ONLINE:
	/* not supported yet */
	;

    }
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_proto_finish_request                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
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
/*      send_client_html_msg            Sends HTML error message to      */
/*                                      client.                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_net_flush                    Flushes the output buffer.       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  ps_proto_finish_request(Request * req)
{
    ps_net_flush(req);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_proto_status                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
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
/*      list_directory                  Plugin to list all files in the  */
/*                                      in memory or os dependent file   */
/*                                      system.                          */
/*      process_ssi                     Process a Get Operation with the */
/*                                      .SSI extension.                  */
/*      process_POST                    Processes Post form commands.    */
/*      send_client_html_msg            Sends HTML error message to      */
/*                                      client.                          */
/*      ps_proto_redirect               Redirects the  user agent to     */
/*                                      another URL.                     */
/*      make_mime_proto                 Generate HTTP header with the    */ 
/*                                      correct mime codes.              */
/*      sprintf                         Prints a message into a string.  */
/*      strcat                          Concantenates two strings.       */
/*      memory_dump                     Plugin used to dump a specified  */
/*                                      memory address and length to an  */
/*                                      HTML page.                       */
/*      arg                             Plugin that expects and argument.*/
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*     code                             Code that is used to descibe the */
/*                                      prototype status.                */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*     None.                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  ps_proto_status(Request * req, int code)
{
    sprintf(req->response,"HTTP/1.0 %d Document Follows",code);
    strcat(req->response,"\r\n");
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_proto_start_response                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to start the http response.  It takes the compiled      */
/*      header and outputs it out to the network.                        */
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
/*      list_directory                  Plugin to list all files in the  */
/*                                      in memory or os dependent file   */
/*                                      system.                          */
/*      process_ssi                     Process a Get Operation with the */
/*                                      .SSI extension.                  */
/*      process_POST                    Processes Post form commands.    */
/*      send_client_html_msg            Sends HTML error message to      */
/*                                      client.                          */
/*      ps_proto_redirect               Redirects the  user agent to     */
/*                                      another URL.                     */
/*      make_mime_proto                 Generate HTTP header with the    */ 
/*                                      correct mime codes.              */
/*      memory_dump                     Plugin used to dump a specified  */
/*                                      memory address and length to an  */
/*                                      HTML page.                       */
/*      arg                             Plugin that expects and argument.*/
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      strcat                          Concantenates two strings.       */
/*      ps_net_write                    Writes data out to the Network.  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
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

void  ps_proto_start_response(Request * req)
{
    strcat(req->response,"\r\n");
    ps_net_write(req,req->response,strlen(req->response));
#ifdef DEBUG
    printf("ps_proto_start_response =%s\n\r",req->response);
#endif /* DEBUG */
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_proto_redirect                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Send the user agent (browser) to another URL (may                */
/*      or may not be on this server).                                   */
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
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*      task_change                     Plugin to perform task functions.*/
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_proto_status                 Sets up the HTTP response.       */
/*      ps_outhdr_insert                Inserts the name and value into  */
/*                                      the output header of the HTTP    */
/*                                      response.                        */
/*      ps_proto_start_response         Starts the HTTP response to the  */
/*                                      client.                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*      url                             The URL of where the response    */
/*                                      is to point to.                  */
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

void  ps_proto_redirect(Request * req, char * url)
{
    ps_proto_status(req,PROTO_REDIRECT);
    ps_outhdr_insert(CONTENT_LOCATION,url,req->response);
    ps_proto_start_response(req);

}

char url_buf[URL_LEN];


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_proto_uri2url                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Convert a URI (local pathname) into a full URL spec              */
/*      (http://xx.xxx.xx.xx/uri)                                        */
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
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      sprintf                         Prints a message into a string.  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*      uri                             The name of the file to be       */
/*                                      retrieved.                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      url_buf                         Conatins the complete URL.       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

char * ps_proto_uri2url(Request * req, char * uri)
{
    unsigned char * p;

    p = req->server->ip;                            /* ip address or server */

    sprintf(url_buf,"http://%d.%d.%d.%d/%s", 
	    *p, *(p+1), *(p+2), *(p+3),uri);

    return(url_buf);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_token_findval                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to find the value of a the token name.  It compares     */
/*      a name string until it finds the correct token name and returns  */
/*      the token value.                                                 */
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
/*     ps_sendurl                       Validates User ID and key and    */
/*                                      calls appropriate URI.           */
/*      ran_num                         Plugin to give random number.    */
/*      memory_dump                     Plugin used to dump a specified  */
/*                                      memory address and length to an  */
/*                                      HTML page.                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      strcmp                          Compares one string with another.*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      name                            Name of the token name to find.  */
/*      tok                             Pointer to Token structure that  */
/*                                      holds current token arguments    */
/*                                      (Name and Value) and pointer to  */
/*                                      next token arguments.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The function returns the token value until the token pointer     */
/*      is NULL.  Then the function returns NULL.                        */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

char * ps_token_findval(char * name, Token * tok)
{

    while( tok )
    {
	if( strcmp(name, tok->arg.name) == 0 )
	return( tok->arg.value );

	tok= tok->next;
    }

    return((char * )NULL);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      check_timeout                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to see if it's time to expire one of the authentication */
/*      structures.                                                      */
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
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_gettime                  Get time and convert it to seconds.  */
/*      free_auth                   Frees a particular ip address from   */
/*                                  the authentication structure.        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
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

void  check_timeout(Request * req)
{
    int t,d;
    struct ps_auth * a;

    t = ps_gettime();                               /* time in seconds */
    d = t - req->server->master.last_time;          /* number of seconds elapsed */
    a = req->server->master.user_auth;

    while( a )
    {

	if( (a->countdown) && (a->state != AUTH_FREE) )
	{
	    if( d >= a->countdown )
	    {
#ifdef DEBUG
	    printf("timeing out %d %d %d %d\n",a->ip[0],a->ip[1],a->ip[2],a->ip[3]);
#endif
	    free_auth(a);
	    }
	    else    a->countdown -= d;
	}
	a = a->next;
    }
    req->server->master.last_time =t;               /* remember for next time */
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      md_file_stat                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function that searches for a  particular uri.  The function      */
/*      first looks to see if the file is file is a plugin.  If          */
/*      not then it checks if the file is in memory.  If not then it     */
/*      returns a value that states that the uri is not found.  This     */                                                          
/*      would apply to the os dependent file system, but at this time it */
/*      is not supported.                                                */
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
/*      save_upload_file                Saves an uploaded file to memory.*/
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      fs_file_match                   Search for client requested file */
/*                                      name.                            */
/*      os_stat_file                    OS-dependent check for external  */
/*                                      storage.                         */
/*      strcmp                          Compares one string with another.*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      If the file wsas found it returns SUCCESS.  If the file is not   */
/*      found then it returns ENOFILE.                                   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int   md_file_stat(Request * req)
{
    char * start;
    int length,type;
    int (*plugin)(Token *, Request *)= NULL;
    char * p;
    int i;

    p = req->fname;
    while(*p == '/') p++;

    req->stat->flags = NOT_FOUND;

    for(i=0; i<MAX_PLUGIN; i++ )
    {
	if( plugins[i].name )
	{
	    if( strcmp(p,plugins[i].name) == 0 )
	    {
		plugin =  plugins[i].plugin;
							
	    }
	}
    }

    if( plugin != NULL )
    {
	req->stat->plugin = plugin;
	req->stat->flags = PLUGIN;
	return(SUCCESS);
    }

    if( fs_file_match(req->fname,&start,&length,&type) )
    {
	req->stat->address = start;
	req->stat->size = length;
	req->stat->flags = INCORE|FOUND|NORMAL;
	return( SUCCESS);
    }  
	
    os_stat_file(req);                              /* find out if it's on external storage */


    if( req->stat->flags == NOT_FOUND )
	return(ENOFILE);
    else    return(SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      md_send_file                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to send a file over the network.  It first verifies that*/
/*      the file requested is a match with the one in the file system.   */
/*      If it is then it makes the mime protoype response and depending  */
/*      on whether file compression is used or not the file is           */
/*      decompressed from memory. Finally, it is transferred to the      */
/*      client.                                                          */
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
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/*      serv_file                       Initial function of sending a    */
/*                                      file over the network.           */
/*      fs_file_match                   Search for client requested file */
/*                                      name.                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      fs_file_match                   Search for client requested file */
/*                                      name.                            */
/*      make_mime_proto                 Generate HTTP header with the    */ 
/*                                      correct mime codes.              */
/*      ps_net_write                    Writes data out to the Network.  */
/*      ps_decompress                   Decompresses a file in memory or */
/*                                      magnetic media.                  */
/*                                                                       */
/* INPUTS                                                                */              
/*                                                                       */
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

void  md_send_file(Request * req)
{
    char * start;
    int length,type;
#ifdef FILE_COMPRESSION

    char outbuf[1024];
    int i;
#endif

#ifdef DEBUG
    printf("md_send_file:%s\n",req->fname);
#endif

    if( fs_file_match(req->fname,&start,&length,&type) )
    {
	make_mime_proto(req,length);
#ifdef FILE_COMPRESSION

	for(i=0; i<CHDSZ; i++ )
	{
	
	    if( *(i+start) != (CHD[i]) )
	    {
		ps_net_write(req,start,length);
		return;
	    }
	}
	ps_decompress(req,start+4,outbuf,length-4,1024);
      
#else

    DEBUG1++;
    ps_net_write(req,start,length);
    DEBUG1--;
#endif
     
    }
    return;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      find_token                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      UTILITY FUNCTIONS. Return the position in the second             */
/*      string where an occurence of the first string starts.            */
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
/*      file_upload                     Plugin to upload a file into the */
/*                                      system.                          */
/*      process_ssi                     Process a Get Operation with the */
/*                                      .SSI extension.                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      strncmp                         Compares two strings up to a     */
/*                                      specified number of bytes.       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      token                           Token string to be found.        */
/*      last                            Size of the file.                */
/*      l                               File to look for the token in.   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Function returns l if it found the token.  It returns FAILURE if */
/*      it does not find the token.                                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

char * find_token(char * token, char * l, char * last)
{
    int len = strlen(token);

    if( len == 1)
    {
	while ( *l &&(l < last))
	{
	    if( *token == *l ) 
		return(l);
	    l++;
	}
	return(FAILURE);
    }

    while( *l && (l < last)  )
    {
	if( strncmp(token,l,len) ==0) 
	    return(l);
	l++;
    }
    return((char*)FAILURE);  
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      in_string                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to find a string in a text string.                      */
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
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*      process_ssi                     Process a Get Operation with the */
/*                                      .SSI extension.                  */
/*      is_ssi                          Checks for an SSI(Server Side    */
/*                                      Include) tag.                    */
/*      process_POST                    Processes Post form commands.    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      strncmp                         Compares two strings up to a     */
/*                                      specified number of bytes.       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      s                               The string sequence to be looked */
/*                                      for.                             */
/*      l                               The string that the sequence is  */
/*                                      to be searched for.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The function returns l if it found the string.  It the sequence  */
/*      is not found then it returns FAILURE.                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

char * in_string(char * s, char * l )
{
    int len;

    len = strlen(s);

    if( len == 1 )
    {
	while ( *l )
	{
	    if( *s == *l ) 
		return(l);
	    l++;
	}
	return(FAILURE);
    }

    while( *l++ )
    {

	if( strncmp(s,l,len) ==0) 
	    return(l);
    }
    return(FAILURE);
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_a2hex                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to convert hex to binary.                               */
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
/*      memory_dump                     Plugin used to dump a specified  */
/*                                      memory address and length to an  */
/*                                      HTML page.                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      a2hex                           Converts a hex digit into binary.*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      s                               Hex string to be converted to    */
/*                                      binary.                          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      r                               Binary converted interger.       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int ps_a2hex(char * s)
{
    register int r=0;

    while( *s )
    {

	r |= a2hex((unsigned char *)s);
	s+=2;
	if( *(s) )
	r <<= 8;
    }

    return(r);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      a2hex                                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to convert an ascii hex digit to binary.                */
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
/*      packed_hex_2bin                 Convert packed hex to binary.    */
/*      ps_a2hex                        Convert hex to binary.           */
/*      memory_dump                     Plugin used to dump a specified  */
/*                                      memory address and length to an  */
/*                                      HTML page.                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      hexa                            Converts Ascii Hex nibble to     */
/*                                      binary.                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      s                               Hex character to be converted to */
/*                                      binary.                          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      r                                Binary value.                   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

unsigned int a2hex(unsigned char* s)
{
    unsigned int r;

    r = hexa(s++);
    r = r <<4;
    r = r&0xf0;
    r |= hexa(s);

    return(r);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      hexa                                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Converts an ascii hex character to  a binary nibble.             */
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
/*      a2hex                       Converts ascii hex digit to binary.  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      s                           Character to be converted.           */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      r                           Converted integer.                   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int hexa(unsigned char * s)
{
    unsigned int r;

    r=0;

    if((*s >= 'A' ) && (*s <= 'F' ) )
	r = (*s - 'A') +10;
    else if((*s >= 'a' ) && (*s <= 'f' ) )
	r = (*s - 'a') +10;
    else if((*s >= '0' ) && (*s <= '9' ) )
	r = (*s - '0');
    return(r);

}
char *hex="0123456789ABCDEF";


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      hex2                                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to convert binary byte to two hex digits.               */
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
/*      send_auth_salt                  Sends a packet of random bits to */
/*                                      be combined with the             */
/*                                      authentication data the user     */
/*                                      enters.                          */
/*      make_dump                       Makes the memory dump buffer.    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      c                               Byte to be converted.            */
/*      buf                             Buffer to store converted Hex    */
/*                                      digits.                          */
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

void  hex2(char c,char * buf)
{
    *buf++=hex[(c>>4)&0xf];
    *buf  =hex[c&0xf];
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      packed_hex_2bin                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to convert packed hex to binary.                        */
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
/*     ps_sendurl                       Validates User ID and key and    */
/*                                      calls appropriate URI.           */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      a2hex                           Converts ascii hex digit to      */
/*                                      binary.                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      d                               Destination string address.      */
/*      s                               Source Address                   */
/*      count                           The number of Hex digits.        */
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

void  packed_hex_2bin(char * d, char * s ,int count)
{
    int i,j;

    for(i=0,j=0; i<count; i+=2,j++)
    {
	d[j] = (unsigned  char)a2hex((unsigned char *)s);
	s += 2;
    }
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      server_error                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine is present for future use as a hook                 */
/*      for error logging.                                               */
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
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      s                               Error Message to be printed.     */
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

void  server_error(char * s)
{
    /*  Remove Warnings */
    s = s;
#ifdef DEBUG
    printf("Server Error: %s\n",s);
#endif
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      client_error                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine is present for future use as a hook                 */
/*      for error logging.                                               */
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
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/*      process_POST                    Processes Post form commands.    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*      reason                          Message to explain error.        */
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

void  client_error(Request * req, char * reason)
{
     /* NOT YET */
    /*  Remove warnings */
    req = req;
    reason = reason;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      pico_init                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function initializes all features of the webserver.  This        */
/*      includes registers plugins for all the features found in         */
/*      ps_conf.h.  This function is called early during initialization. */
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
/*      find_auth_ip                    Traverses through the server     */
/*                                      authentication structure to check*/
/*                                      for an authenticated ip address. */                                
/*      ps_reg_plugin                   Register plugins.                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_reg_plugin                   Register plugins.                */
/*      ps_get_server                   Retireves information about the  */
/*                                      server.                          */
/*      auth_init                       Initializes Authentication.      */
/*      strcpy                          Copies one strings contents into */
/*                                      another strings contents.        */
/*      ps_INIT                         Initializes User plugins.        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None.                                                            */
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

void  pico_init()
{

    int i;
        
    Ps_server * this_server;
    ps_auth * ua;
#ifndef FS_IN_MEMORY
    Request *req=0;
	char *filemem;
    int j;
    STATUS status;
#endif
    this_server = ps_get_server();
    ua = &this_server->master.user_auth[0];

    for(i=0; i<MAX_PLUGIN; i++)
    plugins[i].name=(char*)NULL;

    for(i=0; i< AUTH_MAX_USERS-1; i++)
    {
	ua[i].next = &ua[i+1];
	ua[i].state = AUTH_FREE ;
    }

#ifdef FS_IN_MEMORY

						    /* initialize our in memory file system */
    {
	struct fs_file * f;

	extern struct fs_file embed_fs[];
	extern int embedded_files;
	int i;

	f = embed_fs;

	/* ps_fs is generated
	 * automatically by ps_mkfs
	 */
	
	fs_fil = f;


		/* convert an array of fs_file structures
		 * into a linked list
		 * to make it easier to add/remove files
		 */

	for(i=0; i<embedded_files-1; i++)
	{
	    f->next = (f+1);
	    f++;
	}

	f->next = NULL;                             /* last element */
    }

#else /*FS_IN_MEMORY*/

    fs_fil = NULL;
    status = NU_Allocate_Memory(&System_Memory, (VOID **)&filemem, 4000, 
				NU_NO_SUSPEND);
    if (status != NU_SUCCESS)
    {
        return;
    }



    j=0;
    
    /*  Write Upload File to disk */
    for( i = 0; upload_htm[i][0]; i++ )
    { 
         
         strcpy(filemem + j, upload_htm[i]);
         j= strlen(upload_htm[i]) + j;
    }

    j= strlen(filemem);
    os_write_fs(req, "upload.htm",filemem,j);

#endif
	
#ifdef  AUTHENTICATION
    auth_init();
#endif

    /* register "internal" plugins */

#ifdef FILE_UPLOAD
    {
	extern file_upload(Token * env, Request * req);
	ps_reg_plugin(file_upload,"upload");
    }
#endif

#ifdef LIST_DIRECTORY
    {
	extern list_directory(Token * env, Request * req);
	ps_reg_plugin(list_directory,"dir");
    }
#endif

#ifdef BASIC_AUTH

    b_auth_init();
#endif

    ua[i].next = NULL;
    ua[i].state= AUTH_FREE;
    strcpy(this_server->master.auth_public,PUBLIC_DIR);
    this_server->master.timeoutval = AUTH_TIMEOUT;
    this_server->port = 80;                         /* set default port */

    ps_INIT();                                      /* call the user init routine */

}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_INIT                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function registers all user plugins that are used within the*/
/*      Webserver.                                                       */
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
/*      pico_init                       Initializes Nucleus Web Server.  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_reg_plugin                   Register plugins.                */
/*      ps_get_server                   Retireves information about the  */
/*                                      server.                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*         DRS- ATI        2/12/98                 registered Demo       */
/*                                                 plugins               */
/*************************************************************************/

void ps_INIT()
{
    Ps_server* p;
    p = ps_get_server(); 
    /*  Remove Warnings */
    p = p;

    ps_reg_plugin(ip_addr,"ip_addr");
    ps_reg_plugin(task_stat,"task_stat");
    ps_reg_plugin(task_change,"task_change");
    ps_reg_plugin(process_answer,"process_answer");


}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      make_mime_proto                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function that generates a HTTP response header with correct      */
/*      mime codes.                                                      */
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
/*      serve_file                      This function sends the file to  */
/*                                      the client over the network.     */
/*      md_send_file                    Send file to browser over        */
/*                                      network.                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_proto_status                 Sets up the HTTP response.       */
/*      ps_outhdr_insert                Inserts the name and value into  */
/*                                      the output header of the HTTP    */
/*                                      response.                        */
/*      ps_outhdr_nninsert              Places name and numeric value    */
/*                                      token into output header.        */
/*      ps_proto_start_response         Starts the HTTP response to the  */
/*                                      client.                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*      length                          The size of file that is going   */
/*                                      to be transferred.               */
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

void  make_mime_proto(Request * req,int length)
{
    ps_proto_status(req,PROTO_OK);
    ps_outhdr_insert(CONTENT_TYPE, ps_mime_type(req),req->response);
    ps_outhdr_nninsert(CONTENT_LENGTH,length,req->response);
    ps_proto_start_response(req);
}

/* derive a mime type from the file extension  
 *
 * The mime type tells the browser what kind
 * of file it's getting (at least a guess anyway)
 * based on the extension
 */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_mime_type                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function that derives a mime type from the file extension. The   */
/*      mime type tells the browser what kind of file it's getting       */
/*      (at least a guess anyway) based on the extension.                */
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
/*      make_mime_proto                 Generate HTTP header with the    */ 
/*                                      correct mime codes.              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      strcmp                          Compares one string with another.*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      mime_table                      Returns the  mime type in the    */
/*                                      table.                           */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

char * ps_mime_type(Request * req)
{
    char * s;
    int i;
	
    s = req->fname; /* file name */
    while( *s && (*s != '.') )
	   s++;

    /* no extension use the default mimetype */
    if( *s == '\0') return( mime_table[DEFAULT_MIME].mime_type );

    /* search the mime table */
    if( *s == '.' )
    {
	s++;
	for( i=0; mime_table[i].ext; i++)
	if((strcmp(s,mime_table[i].ext)) ==0)
	return(mime_table[i].mime_type);
    }
    return(mime_table[DEFAULT_MIME].mime_type);
}
