
/****************************************************************************/
/*                                                                          */
/*            Copyright (c) 1993-1998 Accelerated Technology, Inc.          */
/*                                                                          */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the         */
/* subject matter of this material.  All manufacturing, reproduction,       */
/* use, and sales rights pertaining to this subject matter are governed     */
/* by the license agreement.  The recipient of this software implicitly     */
/* accepts the terms of the license.                                        */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*  B_AUTH.C                                                     1.0        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This File Contains several routines used for Initializing and       */
/*		implementing the HTTP 1.0 Basic Authentication Algorithm.  Also 	*/
/*		contains two user plug-ins that are used to add, delete and show	*/
/*      users on the system. To include this feature ps_conf.h must         */
/*      have BASIC_AUTH defined.                                            */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Don Sharer Accelerated Technology Inc.                              */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*		BPWLIST_INFO_LIST													*/
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*		b_auth_init 														*/
/*		b_auth_add_entry													*/
/*		b_auth_delete_entry 												*/
/*		b_parse_auth														*/
/*		base64_decode														*/
/*		b_add_delete_auth													*/
/*		show_users															*/
/*																			*/
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*		b_auth.h															*/
/*		ps_pico.h															*/
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME                            DATE            REMARKS             */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include "nucleus.h"
#include "target.h"
#include "protocol.h"
#include "ip.h"
#include "externs.h"
#include "socketd.h"                                /* socket interface structures */
#include "tcpdefs.h"
#include "ps_pico.h"

#ifdef   BASIC_AUTH
#include "b_auth.h"

char text_401[]="\
HTTP/1.0 401 Unauthorized\r\n\
WWW-Authenticate: Basic realm=\" %s\"\r\n\
Content-type: text/html\r\n\
Content-length: 0\r\n\r\n";

char text_403[]="\
HTTP/1.0 403 Forbidden\r\n\
Server: Nucleus WebServ\r\n\
Content-type: text/html\r\n\
Content-length: 0\r\n\r\n\
<HEAD><TITLE>403 Forbidden</TITLE></HEAD>\r\n\
<BODY><H1>403 Forbidden Access</H1>\r\n";


extern char    *find_token(char * token, char * buf, char * last);

/*  Define User PLug-in for adding and deleting a user. */
int b_add_delete_auth(Token * env, Request *req);

/*  Define Server Side Include Function that will show all users on
 *  the system.
 */
int show_users(Token *env, Request *req);

extern Token  *setup_pblock(char * s);
extern void   client_error(Request * req, char * reason);

/*  Setup Pointers to the linked List structure Basic Auth Link List        */
BPWLIST_INFO_LIST  BPWLIST_info;


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       B_Auth_Init                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Don Sharer Accelerated Technology Inc.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      pico_init                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                                               */
/*      dll_enqueue                                                      */
/*      ps_reg_plugin                                                    */
/*                                                                       */
/*************************************************************************/

int16 b_auth_init(VOID)
{
    BPWLIST_INFO    *bpwlist_info;
    int i=0;

    /*  Register the Plug-in to show all users */
    ps_reg_plugin(show_users,"userp");

    /*  Register the plug-in that adds or deletes a user. */
    ps_reg_plugin(b_add_delete_auth,"addelu");
    
    /* Build the Basic Authentication Password List list. */
    for (i = 0; webpwTable[i].wuser_id[0]; i++)
    {
        if (NU_Allocate_Memory (&System_Memory, (VOID **)&bpwlist_info,
                            sizeof (BPWLIST_INFO),
                            NU_NO_SUSPEND) != NU_SUCCESS)
        {
            return (NU_MEM_ALLOC);
        }


        /* Setup the User Id name */
        strcpy(bpwlist_info->wuser, webpwTable[i].wuser_id);


        /* Setup the Password */
        strcpy(bpwlist_info->wpass_word, webpwTable[i].wpass_word);


        /* Setup the Access_String */
        strcpy(bpwlist_info->waccess, webpwTable[i].access_str);

        /* Add this host to the list. */
        dll_enqueue((tqe_t *) &BPWLIST_info, (tqe_t *) bpwlist_info);

    }

               
       return(NU_SUCCESS);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       b_auth_add_entry                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function dynamically adds a user id and password combination*/
/*      into the Nucleus WebServ's database.                             */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Don Sharer Accelerated Technology Inc.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      b_add_delete_auth                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                                               */
/*      dll_enqueue                                                      */
/*                                                                       */
/*************************************************************************/

int16 b_auth_add_entry(CHAR *user_id, CHAR *password)
{
    int16           found=0;
    BPWLIST_INFO    *bpwlist_info;

   /*  First Verify that the user does not exist */
    found=0;
    for(bpwlist_info=BPWLIST_info.bpwlist_head;bpwlist_info;bpwlist_info=bpwlist_info->wpwlist_next)
    {
        if(strcmp(bpwlist_info->wuser,user_id) == 0 )
        {
            if (strcmp(bpwlist_info->wpass_word,password)==0)
            {
                found++;
                break;
            }
        }

    }

    if(found)
       return(BASIC_AUTH_FAILED);
    
    /*  Allocate Memory for the new database entry   */
    if (NU_Allocate_Memory (&System_Memory, (VOID **)&bpwlist_info,
                            sizeof (BPWLIST_INFO),
                            NU_NO_SUSPEND) != NU_SUCCESS)
    {
        return (NU_MEM_ALLOC);
    }


    /* Setup the New User id to add */
    strcpy(bpwlist_info->wuser,user_id);


    /* Setup the password name */
    strcpy(bpwlist_info->wpass_word ,password);



    /* Add this host to the list. */
    dll_enqueue((tqe_t *) &BPWLIST_info, (tqe_t *) bpwlist_info);

    return(NU_SUCCESS);

}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       b_auth_delete_entry                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is used to dynamically delete a combination        */
/*      of user id and password from the Nucleus WebServ's database.     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Don Sharer Accelerated Technology Inc.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Deallocate_Memory                                             */
/*      dll_remove                                                       */
/*      b_add_delete_auth                                                */
/*                                                                       */
/*************************************************************************/
int16 b_auth_delete_entry(CHAR *user_id, CHAR *password)
{
    BPWLIST_INFO    *bpwlist_info;

    int  found=0;

    /*  Searches to see if the name being asked to delete is availble  */
    for(bpwlist_info=BPWLIST_info.bpwlist_head;bpwlist_info;bpwlist_info=bpwlist_info->wpwlist_next)
    {
        if(strcmp(bpwlist_info->wuser,user_id) == 0 )
        {
            if (strcmp(bpwlist_info->wpass_word,password)==0)
            {
                found++;
                break;
            }
        }

    }
    if (found)
    {
        found=0;
        /*  If entry is found remove it from the list and delete it. */
        dll_remove((tqe_t *)&BPWLIST_info,(tqe_t *) bpwlist_info);
        NU_Deallocate_Memory(bpwlist_info);
    }
    else
        return(-1);

    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       b_parse_auth                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is used to process the Basic Authentication        */
/*      method.  This inlcudes pasring the HTTP packet and getting the   */
/*      bas64 encoded user id and password.  It then calls a function    */
/*      that decodes the string into a userid:password format. Then the  */
/*      database checks if the user exists on the system.  If the user   */
/*      does not exist a 403 access forbidden message is sent.  If the   */
/*      Authorization token is not found a 401 message is sent to pop-up */
/*      the browser's network password method. If the user exists the    */
/*      server acts as normal.                                           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Don Sharer Accelerated Technology Inc.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      parse_http_request                                               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                                               */
/*      memset                                                           */
/*      find_token                                                       */
/*      ps_net_write                                                     */
/*      base64_decode                                                    */
/*      NU_Deallocate_Memory                                             */
/*      strcmp                                                           */
/*      sprintf                                                          */
/*                                                                       */
/*************************************************************************/

int16 b_parse_auth(Request *req)
{
    uchar *total_char;
    uchar *final_decode;
    STATUS status;
    uchar *s=0, *l=0;
    char  *req_buf_end;
    char  buf[1000];
    int16  len;
    int count=0,index=0;
    uchar *user_id;
    uchar *password;
    BPWLIST_INFO  *bpwlist;
    int16   found = 0;
    /*  Allocate Memory for total character buffer */
    status = NU_Allocate_Memory(&System_Memory, (VOID **)&total_char,BASIC_MAX_SIZE,NU_NO_SUSPEND);
    if( status != NU_SUCCESS)
    {
#ifdef DEBUG
        printf("Unable to allocate memory\r\n");
#endif
        return(FAILURE);
    }
    /*  Allocate Memory for total character buffer */
    status = NU_Allocate_Memory(&System_Memory, (VOID **)&user_id,BASIC_MAX_SIZE,NU_NO_SUSPEND);
    if( status != NU_SUCCESS)
    {
#ifdef DEBUG
        printf("Unable to allocate memory\r\n");
#endif
        return(FAILURE);
    }

    /*  Allocate Memory for total character buffer */
    status = NU_Allocate_Memory(&System_Memory, (VOID **)&password,BASIC_MAX_SIZE,NU_NO_SUSPEND);
    if( status != NU_SUCCESS)
    {
#ifdef DEBUG
        printf("Unable to allocate memory\r\n");
#endif
        return(FAILURE);
    }
    /*  Allocate Memory for final decode buffer */
    status = NU_Allocate_Memory(&System_Memory, (VOID **)&final_decode,BASIC_MAX_SIZE,NU_NO_SUSPEND);
    if( status != NU_SUCCESS)
    {
#ifdef DEBUG
        printf("Unable to allocate memory\r\n");
#endif
        return(FAILURE);
    }
    memset(user_id,0,BASIC_MAX_SIZE);
    memset(password,0,BASIC_MAX_SIZE);
    memset(final_decode,0,BASIC_MAX_SIZE);
    memset(total_char,0,BASIC_MAX_SIZE);
    /*  Prepare to check first Packet of Incoming HTTP data */
    s = (uchar *)req->rdata->lbuf;

    /*  Set the End of the Packet  */
    req_buf_end = &req->rdata->lbuf[req->nbytes];

    /* Get to the first Sequence delimeter CRLF*/
    while(*s) s++;			

    s++;
    l = s;

    /*  Search for  HTTP 1.0 Authorization Header  */

    s = (uchar *)find_token("Authorization:",(char *)l,(char *)req_buf_end);
    if( s )
    {
        /*  Search for the Basic Authetication Marker  */
        s = (uchar *)find_token("Basic",(char *)s,req_buf_end);
        if(s)
        {
           l = total_char;
           s+= 6;
           while((*s != '\n') && (*s !='\r') && (*s) )
                  *l++= *s++;
                  *l= '\0';
#ifdef DEBUG
       printf("total_char = %s\r\n",total_char);
#endif
        }
        else
        {
            sprintf(buf,text_401,req->fname);
            len= strlen(buf);
            ps_net_write(req,buf,len);
            return(BASIC_AUTH_FAILED);
        }

    }
    else
    {
        sprintf(buf,text_401,req->fname);
        len= strlen(buf);
        ps_net_write(req,buf,len);
        return(BASIC_AUTH_FAILED);
    }

    final_decode = base64_decode(total_char, final_decode);
#ifdef DEBUG
    printf(" final decode = %s \r\n",final_decode);
#endif
    count=0;
    while(final_decode[count] != ':')
    {
        user_id[count] = final_decode[count];
        count++;
    }
    index=0;
    /*  Increment the count passed the : marker */
    count++;
    while(final_decode[count] != NU_NULL)
    {
        password[index] = final_decode[count];
        index++;
        count++;
    }
#ifdef DEBUG
    printf("user_id = %s password = %s \r\n",user_id, password);
#endif
    /*  Traverse through Pw list structure to see if user is verified. */
    for(bpwlist=BPWLIST_info.bpwlist_head; bpwlist;bpwlist=bpwlist->wpwlist_next)
    {
        if(strcmp((char *)bpwlist->wuser,(char *)user_id) == 0 )
        {
            if(strcmp((char *)bpwlist->wpass_word,(char *)password)==0)
            {
                found++;
                break;
            }
        }
    }


    /*  Deallocate Memory used for base64 computation */
    NU_Deallocate_Memory(total_char);
    NU_Deallocate_Memory(final_decode);
    NU_Deallocate_Memory(user_id);
    NU_Deallocate_Memory(password);
    /*  If not found then send Forbidden */
    if( !found )
	{
        sprintf(buf,text_403);
        len= strlen(buf);
        ps_net_write(req,buf,len);
        return(BASIC_AUTH_FAILED);
    }
	else
    {
        /*  If authenticated continue processing */
        found= 0;
        return(NU_SUCCESS);
    }

}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       base64_decode                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is used to decode a base64 encoded string.  It     */
/*      returns the decoded string to the calling routine.               */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Don Sharer Accelerated Technology Inc.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      b_parse_auth                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      strlen                                                           */
/*                                                                       */
/*************************************************************************/

uchar *base64_decode(uchar *total_char, uchar *final_decode)
{
    int16 index=0;
    int16 len = 0;
    int16 end_text=0;
    int16 decode_counter=0;
    int16 count=0;
    int16 ignore_char=0;
    uchar  decode_value[76];
    uchar  tempbits;
    unsigned char ch;

    index=0;
    len= strlen((char *)total_char);
    while((index < len))
    {
        ch= total_char[index];
        if ((ch >= 'A') && (ch <= 'Z'))
             ch= ch- 'A';

        else if((ch >='a') && (ch <= 'z'))
                 ch= ch -'a' +26;

        else if ((ch >='0') && (ch <= '9'))
                  ch = ch - '0' +52;

        else if (ch == '+')
                 ch = 62;

        else if (ch == '=')
        {
            end_text++;
            ch='\0';
        }

        else if (ch == '/')
             ch = 63;

        else
            ignore_char++;

        decode_value[index]= ch;
        index++;
    }
    index= index - end_text;
    count=0;
    decode_counter=0;
    while(count < index)
    {
        final_decode[decode_counter] = decode_value[count] << 2;
        tempbits = (decode_value[count + 1] & 0x30) >> 4;
        final_decode[decode_counter] = final_decode[decode_counter] | tempbits;
        decode_counter++;
        final_decode[decode_counter] = (decode_value[count +1] & 0x0f)<< 4;
        tempbits= (decode_value[count + 2] & 0x3c) >> 2;
        final_decode[decode_counter] = final_decode[decode_counter] | tempbits;
        decode_counter++;
        final_decode[decode_counter] = (decode_value[count +2] & 0x03)<<6;
        final_decode[decode_counter]=  final_decode[decode_counter] | (decode_value[count +3]);
        decode_counter++;
        count=count +4;
    }
    return(final_decode);
}
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       b_add_delete_auth                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      A plug-in used to add and delete users and passwords to and from */
/*      the Nucleus WebServ's database.  This plug-in will redirect the  */
/*      URL to user.ssi if it is successful. If the plug-in is           */
/*      unsuccessful it will give an error message with a tag to return  */
/*      it to the addel.htm web page.                                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Don Sharer Accelerated Technology Inc.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Post forms web page.                                             */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_net_read                                                      */
/*      in_string                                                        */
/*      client_error                                                     */
/*      setup_pblocks                                                    */
/*      strcmp                                                           */
/*      strncmp                                                          */
/*      b_auth_add_entry                                                 */
/*      b_auth_delete_entry                                              */
/*      ps_proto_status                                                  */
/*      ps_outhdr_insert                                                 */
/*      ps_proto_start_response                                          */
/*      strcat                                                           */
/*      ps_net_write                                                     */
/*      ps_proto_redirect                                                */
/*                                                                       */
/*************************************************************************/
int b_add_delete_auth(Token * env, Request *req)
{

    char o[600];
    char user[32];
    char password[32];
    int  add_func=0;
    int  del_func=0;
    int  i = 0;
    Token * t;
    char *s;
    char *line;
    /*  Remove Warnings */
    env = env;

    /*  Initialize variables */
    add_func=0;
    del_func=0;
    memset(o,0,600);

    /*  Set a pointer to the plug-ins arguements */
    t= req->pg_args;

    /*  If the arguemnet is not what is expected then get the next packet */
    /*  This is a Netscape Protocol packet */
    if(!(strcmp(t->arg.name,"user_id")== 0) && !(strcmp(t->arg.name,"password")==0)
        && !(strcmp(t->arg.name,"AddDel")==0))
    {   /* NetScape Protocol */
        i = ps_net_read(req,req->rdata->lbuf,RECEIVE_SIZE,NET_WAIT);
        if (i < 0)
        {
           /*  An Error has occured */
           /*  Remove Annoying Warnings */
           i=i;
           return(i);
        }
        line = req->rdata->lbuf;
        s= in_string("\r\n\r\n",line);
        if(s == (char*) FAILURE )
        {
          client_error(req, "bad POST");
        }
        /*  Get the correct arguements */
        s = s+4;
        req->pg_args = setup_pblock(s);
        t = req->pg_args;
    }

    /*  Travers through the Name and Values to get a match */
    while ( t )
    {
     /*  Check if arguement name is equal to user_id     */
     if(strcmp(t->arg.name,"user_id")== 0)
	 {                                  
         /*  if so copy the user id name */
         strcpy(user,t->arg.value);
	 }
     /*  Check if the name is eual to password */
     else if(strcmp(t->arg.name,"password")== 0)
     {  /*  If so copy the password value */
        strcpy(password,t->arg.value);
     }
     /*  Verify it is an add or delete function */
     else if(strcmp(t->arg.name,"AddDel")== 0)
     {
        /*  Check if it is an add */
        if(strncmp(t->arg.value,"Add",3)==0)
        {
            /*  Then set the add_func to true */
            add_func=1;
        }
        /*  Check if it is a delete function */
        else if(strncmp(t->arg.value,"Delete",6)==0)
        {
            /*  Then set the delete to a true */
            del_func=1;
        }
     }
	 /*  Increment Link list */ 
	 t = t->next; 
    }/*  End While */

    if(add_func)
    {   /*  If add function  then add the entry */
         if(b_auth_add_entry(user,password) != NU_SUCCESS)
         {
            /*  If the add entry fails then send a response. */

            /*  This is setting up a page for the response */
            ps_proto_status(req,PROTO_OK);
            ps_outhdr_insert(CONTENT_TYPE, TYPE_TXT_HTML,req->response);
            ps_proto_start_response(req);
            strcat(o,"<BR><FONT COLOR=\"#980040\"><FONT SIZE=4>\n");

            strcat(o,"User id and password already exists.<BR>");
            strcat(o,"<BR></FONT></FONT>");
            strcat(o,"<p align=\"left\"><a href=\"addel.htm\"><fontsize=\"3\">Back to Add and Deleting</font></a></p>");

            ps_net_write(req,o,strlen(o));

         }
         else
         {  /*  If successful then redirect the URL */
            /*  Redirect to user.ssi script when completed                   */
            ps_proto_redirect(req,"/user.ssi");
         }

    }
    else if(del_func)
    {
         /*  If the user selected Delete then try add delete the user and the
         password */
         if(b_auth_delete_entry(user,password) != NU_SUCCESS)
         {
            /* An error occured, send a response to the Web Browser */
            /*  This is settig up a page for the response */
            ps_proto_status(req,PROTO_OK);
            ps_outhdr_insert(CONTENT_TYPE, TYPE_TXT_HTML,req->response);
            ps_proto_start_response(req);
            strcat(o,"<BR><FONT COLOR=\"#980040\"><FONT SIZE=4>\n");
            strcat(o,"Unable to find user, password combination.<BR></FONT></FONT>");
            strcat(o,"<p align=\"left\"><a href=\"addel.htm\"><fontsize=\"3\">Back to Add and Deleting</font></a></p>");
            ps_net_write(req,o,strlen(o));

         }
         else
         {  /*  Delete was successful Redirect the user to user id passworfd
             *  list.
             */
            ps_proto_redirect(req,"/user.ssi");
         }
    }
    return(REQ_PROCEED);

}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*       show_users                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This plug-in is a server side include function that shows all the*/
/*      user id's and password that are in the Nucleus WebServ's         */
/*      database.                                                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Don Sharer Accelerated Technology Inc.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      .ssi function with the <!# tag.                                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      sprintf                                                          */
/*      ps_net_write                                                     */
/*                                                                       */
/*************************************************************************/
int show_users(Token *env, Request *req)
{
    char ubuf[700];
    BPWLIST_INFO  *bpwlist;
    char password[32];
    int16 len=0;
    env=env;
/*  Set up table for HTMLMarkup  */
     sprintf(ubuf,"<div align=\"center\"><center>");
     ps_net_write(req,ubuf,(strlen(ubuf)));

     sprintf(ubuf,"<table border=\"4\" cellpadding=\"2\" width=\"40%%\">");
     ps_net_write(req,ubuf,(strlen(ubuf)));
	 
/*  Set up the table columns and headers for the SSI User ID/ Password */
	  
     sprintf(ubuf,"<tr><td align=\"center\" width=\"20%%\"> \
                   <FONT COLOR=\"#980040\"> USER ID </font></td> \
                   <td align=\"center\" width=\"20%%\">\
                    <FONT COLOR=\"#980040\">Password</font></td></tr>");
    

     ps_net_write(req,ubuf,(strlen(ubuf)));              

     
    /*  Traverse through Password/user list structure to see if user is verified. */
    for(bpwlist=BPWLIST_info.bpwlist_head; bpwlist;bpwlist=bpwlist->wpwlist_next)
    {
         memset(password,0,32);
         len= strlen((char *)bpwlist->wpass_word);
         memset(password,0x2a,len);

         /*  Print to ubuf  the user id and password name  */
         sprintf(ubuf,"<tr><td align=\"center\" width=\"20%%\"> \
                       <FONT COLOR=\"#980040\">%s</font> </td> \
                       <td align=\"center\" width=\"20%%\"> \
                       <FONT COLOR=\"#980040\">%s</font></td></tr>",
                       bpwlist->wuser,password);
    
     /*  Output teh user id and statistics   */
	     ps_net_write(req,ubuf,(strlen(ubuf)));


     }
     /*  Make end of table HTML and output */
     sprintf(ubuf,"</table></center></div>");
     ps_net_write(req,ubuf,(strlen(ubuf)));
     
     /*  return to Request Proceed */
	return(REQ_PROCEED);

}
#endif/*  BASIC_AUTH */
