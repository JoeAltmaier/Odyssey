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
/*      ps_up.c                                         WEBSERV 1.0      */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains all files related to the upload plugin. To    */
/*      exclude this feature  see FILE_UPLOAD defined in ps_conf.h.      */
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
/*      None.                                                            */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      file_upload                 Plugin to upload a file into the     */
/*                                  system.                              */
/*      save_upload_file            Saves an uploaded file to memory.    */
/*      save_memfs                  Save file in memory.                 */
/*      save_os_fs                  Saves a file in an os supported file */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*     ps_pico.h                    Defines and Data Structures related  */
/*                                  to Nucleus Webserv.                  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Sharer         05-04-98      Corrected File Upload            */
/*      D. Sharer         08-04-98      Added Nucleus File Support and   */
/*                                      corrected file upload to handle  */
/*                                      a file where the boundary is     */
/*                                      offset between two packets.      */
/*                                                                       */
/*************************************************************************/

#include "ps_pico.h"
#include "nucleus.h"
#include "externs.h"
extern NU_MEMORY_POOL    System_Memory;
#ifdef FILE_UPLOAD

int     save_memfs(Request * req,char *fname,char *filemem,int32 length);
int     save_os_fs(Request *req,char * fname,char * filemem,int32 length);

int     save_upload_file(Request * req, char * fname,char * filemem, int32 length);
int     file_upload(Token * Tok,  Request * req);
char    *find_token(char * token, char * buf, char * last); 
extern char * CRLFCRLF;
extern char * CRLF;

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      file_upload                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine uploads a file to the local file system             */
/*      This feature is implememted as  a plugin. To exclude             */
/*      this feature  see FILE_UPLOAD defined in ps_conf.h .             */
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
/*      save_memfs                  Save file in memory.                 */
/*      find_token                  Finds the position in the second     */
/*                                  string where an occurence of the     */
/*                                  first string starts.                 */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_net_read                 Read data from network once a        */
/*                                  connection has been made.            */
/*      send_client_html_msg        Sends HTML error message to client.  */
/*      save_upload_file            Saves an uploaded file to memory.    */
/*      memcpy                      Copies one location in memory        */
/*                                  to another with a specified          */
/*                                  length.                              */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      tok                         Pointer to Token structure that      */
/*                                  holds current token arguments        */
/*                                  (Name and Value) and pointer to      */
/*                                  next token arguments.                */
/*      req                                                              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      FAILURE                                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*       D. Sharer         04-05-98               Corrected File Upload  */
/*                                                                       */
/*************************************************************************/

int file_upload(Token * Tok, Request * req)
{
    char        bound[URI_LEN*2];
    char        fname[URI_LEN];
    char        *s=0,*t=0,*l=0,*f=0;
    char        * fstart=0;
    char        * fend;
    char        *buffer;
    char        *buf;
    int32       content_length= 0;
    char        * content_start=0;
    char        * filemem= 0;
    int         i;
    int         count;
    char        * req_buf_end;
    char        * tem_buf_end;
    STATUS      status;    
    int         temp=0,last_one=0;
    int32       buf_bytes;
	int         more_data=0;
    
#ifdef DEBUG
    printf("file_upload()\n");
#endif
    /*  Remove Warnings */
    Tok=Tok;
    status = status;
    temp = 0;
    more_data = 0;

    /*  Allocate Memory for Receive Buffer */
    status = NU_Allocate_Memory(&System_Memory, (VOID **)&buffer,RECEIVE_SIZE,NU_NO_SUSPEND);
    if( status != NU_SUCCESS)
    {
 	    send_client_html_msg(req,
                    		 "File Upload Error-no memory");
	    return(FAILURE);
    }

    /*  Prepare to check first Packet of Incoming HTTP data */
	s = req->rdata->lbuf;
    
    /*  Set the End of the Packet  */
    req_buf_end = &req->rdata->lbuf[req->nbytes];
            
    /* Get to the first Sequence delimeter CRLF*/
    while(*s) s++;			
    
    /*  Enter Loop to point to data*/
    while(1)
    {

            
            s++; l =s;
            
            /*  Search for  Mulipart/form-data Mime  */
            s = find_token("multipart/form-data",l,req_buf_end);
            if( s )
            {	
                /*  Search for the Content-Length MIME */
	             s = find_token("Content-Length:",s,req_buf_end);
                 if(s)
                 {
                     /*  Increment Pointer to Content String within Buffer */
	                 s = s+16;
                     
                     /* get the length of the multiform data (with file)*/
	                 content_length = atol(s);    
#ifdef DEBUG
                     printf("Content-Length = %lu \n",content_length);
#endif
                     /*  Look for the CRLFCRLF Token  in the packet */
                     s =find_token(CRLFCRLF,s,req_buf_end);

                     /*  Set the Content Start for calculating the Start of 
                      *  incoming data.
                      */
	                 content_start = s+4;

                     /*  Search for the boundary= tag */
	                 s=find_token("boundary=",l,req_buf_end);
	                 if( s )
                     {
                         /*  Increment Pointer to the Boundary Delimeter */
	                     s += 9;            

                         /*  Get the boundary to use for finding the End of the Data */
	                     t = bound;
	                     while( (*s == '"')) s++;
        
                         while(*s &&  (*s != '\n') && (*s != '\r') )
                         {
                            *t++ = *s++;     
                         } 
	                     *t= '\0';

                         /*  Look for bound in the packet */
	                     s = find_token(bound,s,req_buf_end);
	                     if( s )
                         {
                              /*  Search for CRLFCRLF in packet to get to Filename*/
		                      t = find_token(CRLFCRLF,s,req_buf_end);
		                      if( t )
                              {
                                  /*  Increment Pointer to Filename  */
		                          s= t+4;
                  
                                  /*  Get the Filename */
		                          t= fname;
                                  while((*s != '\n') && (*s !='\r') && (*s) )
			                            *t++= *s++;
		                          *t= '\0';
                                  
                                  /*  Look for the CRLFCRLF delimeter to point to the start
                                   *  of actual packet data.
                                   */
		                          
                                  s = find_token(CRLFCRLF,s,req_buf_end);

                                  /* start of the upload data */
		                          fstart = s+4;                 
                                  
                                  /*  Exit out of while loop because we are pointing at data */
                                  break;

                              }
                         }
                         else
                         {
                            
                            /*  Didn't find boundary as expected get Next Packet 
                             *  and check for what is expected.
                             */
                            i = ps_net_read(req,buffer,RECEIVE_SIZE,NET_WAIT);
                            if ( i < 0)
                            {
                           	    send_client_html_msg(req,
                                            		 "File Upload Error-Expected Packet");
                                break;
                            }
                            /*  Set up pointers */
                            s= buffer;
                            req_buf_end= buffer + i;
                            
                            /*  Set content_start variable to point to beginning
                             *  of content.
                             */
                            content_start = s;

                            /*  Look for the boundary token to get to the start of the data */
                            s = find_token(bound,s,req_buf_end);
	                        if( s )
                            {

                               /*  Look for Content-Disposition Mime */
                               s =  find_token("Content-Disposition",s,req_buf_end);

                               /*  Then find the CRLFCRLF ending delimeter.  */
		                       t = find_token(CRLFCRLF,s,req_buf_end);
		                       if( t )
                               {
		                           /*  Increment the point to point to the Filename */
                                   s= t+4;
                  
                                   /*  Get the file name of the file that is being uploaded. */
		                           t= fname;
                                   while((*s != '\n') && (*s !='\r') && (*s) )
			                              *t++= *s++;
		                           *t= '\0';
		                           
                                   /*  Find the CRLFCRLF delimeter to point to the Start of the Data */
                                   s = find_token(CRLFCRLF,s,req_buf_end);

                                   /* start of the upload data */
		                           fstart = s+4;                 
                                   
                                   /*  Break out of loop to get more data */
                                   break;

                              }/*  If t */
                            }/* If s*/
                         }/*  Else */
                     }/*  If bound */
                 }/* s = Content-Length */
            } /*  T equal Multiform Data */

            else
            {
                /*  Didn't get any Relevent Data to start with so Let's try 
                 *  and get another packet.
                 */
                i = ps_net_read(req,buffer,RECEIVE_SIZE,NET_WAIT);
                if ( i < 0)
                {
               	    send_client_html_msg(req,
                    		 "File Upload Error-Expected Packet");
                    break;

                }
                /*  Setup pointers the buffer and the end of buffer */
                s= buffer;
                req_buf_end= &buffer[i];
                /*  Go back to the top of the while loop and try again */
            }/* end else */                                
            
    }/*  while(1) */

    /*  At this point we should be pointing the data in the packet or we got an 
     *  error on reception of data.  If we do not have an error then we should
     *  look  and see if the boundary is in the buffer.  If the boundary is present
     *  then we have the whole file in the first buffer.
     */
    t = find_token(bound,s,req_buf_end);
    if( t )
	{
  
			/* whole file in in the first buffer */

			fend = t-2;
            /*  Save the Upload file */
			save_upload_file(req,fname,fstart,
					         fend - fstart);
					
			
    } /* if t */
    else
    {
          /*  We should have more data to get.  Get the correct Content Length
           *  of the file and start processing.
           */
          more_data = 1;
          content_length -= (fstart - content_start);
             
          /*  Allocate Memory for the Filemem buffer */
          status = NU_Allocate_Memory(&System_Memory, (VOID **)&filemem,content_length,NU_NO_SUSPEND);
          if( status != NU_SUCCESS)
          {
			    send_client_html_msg(req,
				 "File Upload Error-no memory");
			    return(FAILURE);
          }

          /*  Allocate Memory for the tempoary buffer buf */
          status = NU_Allocate_Memory(&System_Memory, (VOID **)&buf,content_length,NU_NO_SUSPEND);
          if( status != NU_SUCCESS)
          {
			    send_client_html_msg(req,
				 "File Upload Error-no memory");
			    return(FAILURE);
          }

 		  /*  Set the number of buffer bytes left in the last packet. */	
		  buf_bytes = (req_buf_end - fstart);

          /*  Copy the data into the filemem buffer */
		  memcpy(filemem,fstart,buf_bytes);

		  /* There is more data in the upload file
		   * we must read some more from the socket
		   */
          
		  count = content_length-buf_bytes;

          /*  Set up buffer pointers */
    

		     while( count > 0  )
             {

#ifdef DEBUG
                printf("reading more data from socket = %d\n",count);
#endif
                /*  Try to get the remaining data */
                i = ps_net_read(req,buf,count,NET_WAIT);
		        if( i < 0 )
                {
#ifdef DEBUG
                    printf("error reading net\n");
#endif
                    /*  If there was an error then send an error message and  
                     *  break out of the loop and exit.
                     */
               	    send_client_html_msg(req,
                                		 "File Upload Error-Expected Packet");

                    break;
			    }

                /*  Copy the nect packet into the Filemem buffer */
                memcpy(filemem + buf_bytes,buf,i);
#ifdef DEBUG
                 printf("Trying to read %d bytes got %d\n",count,i);
#endif

                /*  Reduce the count */
                count -= i;

                /*  Increase the Number of Buffer Bytes */
			    buf_bytes+=i;
                
                /*  Set up buffer pointers */
                l= buf;            
                tem_buf_end=buf + i;

                /*  If count is equal to 0 then we should be at the last packet 
                 *  We need to point to boundary.
                 */
                if (count == 0)
                {
                      
                    f = l;

                    while(temp < i )
                    {
                        /*  Check if the CRLF has been found */
                         if((*f == '\r') &&(*(f+1) == '\n'))
                         {
                              /*  Set the last_one value */
                              last_one = temp;

                              /*  Increment past the CRLF */
                              f+=2;

                              /*  Is the Buffer Pointing to the Start of the boundary ? */
                              if (*f == '-')
                                    /* If yes then break out of the loop */
                                    break;
                              else
                                   /*  Decrement the buffer pointer and continue looking 
                                    *  looking for the next sequence of CRLF.
                                    */

                                    f-=2;
                         }
                         /* Increment Buffer pointer and counter variable temp. */
                         f++;
                         temp++;
                    } /*  End While Loop */

                    /* Increment Last_one twice to point past the CRLF Sequence */
                    last_one+=2;

                    /*  Increment the pointer to point to the boundary string. */
                    l = l + last_one;
#ifdef DEBUG                    
                    printf("BUF_BYTES %lu \n", (buf_bytes - strlen(bound)));
#endif
              
                } /*  end if count = 0 */
                /*  Now look for the boundary token */
                s=find_token(bound,l,tem_buf_end);

                /*  If boundary was found then the file-upload was complete.  Then call 
                 *  save_upload_file to save the file into the file system. 
                 */
			    if( s )
			    {
                     save_upload_file(req,fname,filemem,buf_bytes -(i - last_one + 2)); 
                     
			    }/* end if */
                else
                /*  check for option where file upload is not complete, but the boundary is found. */
                {
                    temp=0;
                    f=filemem;
                    while(temp < buf_bytes )
                    {
                        /*  Check if the CRLF has been found */
                         if((*f == '\r') &&(*(f+1) == '\n'))
                         {
                              /*  Set the last_one value */
                              last_one = temp;

                              /*  Increment past the CRLF */
                              f+=2;

                              /*  Is the Buffer Pointing to the Start of the boundary ? */
                              if (*f == '-')
                                    /* If yes then break out of the loop */
                                    break;
                              else
                                   /*  Decrement the buffer pointer and continue looking 
                                    *  looking for the next sequence of CRLF.
                                    */

                                    f-=2;
                         }
                         /* Increment Buffer pointer and counter variable temp. */
                         f++;
                         temp++;
                    } /*  End While Loop */
                    
                    
                    /*  Set pointer to filemem buffer */ 
                    l= filemem;
                    /*  Now look for the boundary token */
                    s=(char *)find_token(bound,l,(char *)&l[buf_bytes]);
                    if( s )
			        {
                         save_upload_file(req,fname,filemem,(buf_bytes - (buf_bytes- temp))); 
                     
			        }/* end if */
                    
                } /*  end else */

             } /*  end while (count > 0) */
    
	}/*  end else */

    if (more_data )
    {
       /*  Deallocate the Filemem buffer */
       status =NU_Deallocate_Memory((VOID **)filemem);
       if (status != NU_SUCCESS)
          return(status);
    
       
       /*  Deallocate the buf buffer */
       status = NU_Deallocate_Memory((VOID **)buf);
       if (status != NU_SUCCESS)
           return(status);
    }
	 /*  Deallocate the Buffer */
       status = NU_Deallocate_Memory((VOID **)buffer);
       if (status != NU_SUCCESS)
        return(status);
    
    /*  Return from plug-in */
    
    return(0);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      save_upload_file                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function saves a file to memory or to os dependent file     */
/*      system.  The plugin is passed in the request structure.  The     */
/*      file is checked for a previous version and type.  If it did      */
/*      not exist it saves it to memory.  If it did exist it saved in    */
/*      in the location it was defined.                                  */
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
/*      file_upload                 Plugin to upload a file into the     */
/*                                  system.                              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      md_file_stat                                                     */
/*      send_client_html_msg        Sends HTML error message to client.  */
/*      save_memfs                  Save file in memory.                 */
/*      save_os_fs                  Saves a file in an os supported file */
/*                                  system.                              */
/*      sprintf                     Prints a message into a string.      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                         Pointer to Request structure that    */
/*                                  holds all information pertaining     */
/*                                  to  the HTTP request.                */
/*      fname                       File name of the file to be saved.   */
/*      filemem                     The pointer to the input buffer that */
/*                                  contains the file.                   */
/*      length                      The length of the file in bytes.     */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The function returns FAILURE if it was unable to write the file  */
/*      to memory or os dependent file system.  It returns SUCCESS if    */
/*      the saving of the file was complete.  The value of i indicates   */
/*      from the calling function was a SUCCESS or FAILURE.              */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*      D. Sharer          05-04-98             Corrected  Errors        */
/*                                                                       */
/*************************************************************************/

int save_upload_file(Request * req, char * fname,char * filemem, int32 length)

{
    char buf[256];
	
    int i;
    char * fname_save;

#ifdef DEBUG 
    printf("SAVE_UPLOAD_FILE %s (%d)\n",fname,length);

#endif
    fname_save = req->fname;                        /* save plugin name */
    req->fname = fname;                             /* md_file_stat expects it here */
    fname_save = fname_save;

    i = md_file_stat(req);

    /* find out if it exists and if so what kind of file it is */
    switch( i )
    {

	case SUCCESS:
	if( req->stat->flags == PLUGIN )
	{
	    sprintf(buf,
	     "\' %s \'is the name of a plugin.<BR>Upload failed.\n",fname);
	    send_client_html_msg(req,buf);
	    return(FAILURE);
	}

	if( req->stat->flags & INCORE )
	{
	    i = save_memfs(req,fname,filemem,length);
            if (i == FAILURE)
               return(i);
            /*  Not an Error Send Browser Message that it is complete */
            sprintf(buf,"File: %s saved to memory.\n",fname);
            send_client_html_msg(req,buf);

	}
	else
	{
	    i = save_os_fs(req,fname,filemem,length); 
        /*  Not an Error Send Browser Message that it is complete */
        sprintf(buf,"File: %s saved to memory.\n",fname);
        send_client_html_msg(req,buf);
	}
        return(i);
	
	case ENOFILE:                               /* no file by that name */

#ifdef FS_IN_MEMORY

	/* then save it to memory (or flash or whatever ) */

	if( (save_memfs(req,fname,filemem,length)) == FAILURE )
	{
	    sprintf(buf,"Error Saving %s to memory.\n",fname);
	    send_client_html_msg(req,buf);
            return(FAILURE);
	}
	sprintf(buf,"File: %s saved to memory.\n",fname);
	send_client_html_msg(req,buf);
	return(SUCCESS);
#else

	/* else save it to disk (or what ever Mass storage) */
	
	if( (save_os_fs(req,fname,filemem,length)) == FAILURE )
	{
	    sprintf(buf,"Failed saving %s to disk.\n",fname);
	    send_client_html_msg(req,buf);
	}
	else
	    sprintf(buf,"File: %s sucessfully saved to disk.\n",fname);
	    send_client_html_msg(req,buf);
	    return(SUCCESS);
#endif
    }/* end switch */

    return(SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      save_os_fs                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to save the file  to an os dependent mass storage       */
/*      device if it exits.                                              */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*      Don Sharer Accelerated Technology Inc.                           */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      save_upload_file            Saves an uploaded file to memory.    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      send_client_html_msg        Sends HTML error message to client.  */
/*      os_write_fs                 Write to the os dependent mass       */
/*                                  storage medium.                      */
/*      ps_mfree                    Deallocates memory with a call       */
/*                                  to NU_Deallocate_Memory.             */
/*      sprintf                     Prints a message into a string.      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                         Pointer to Request structure that    */
/*                                  holds all information pertaining     */
/*                                  to  the HTTP request.                */
/*      fname                       File name of the file to be saved.   */
/*      filemem                     The pointer to the input buffer that */
/*                                  contains the file.                   */
/*      length                      The length of the file in bytes.     */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Always returns Success.                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int save_os_fs(Request *req,char * fname,char * filemem,int32 length)
{


    os_write_fs(req,fname,filemem,length);
    
    return(SUCCESS);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      save_memfs                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to save an uploaded file into the incore memory file    */
/*      system.  It checks if file compression is enabled.  If it is     */
/*      then it checks if is necessary to compress the file.  A slot to  */
/*      place the file in memory is looked for.  Once it has found its   */
/*      slot in memory it write the file to that memory location.        */
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
/*      save_upload_file            Saves an uploaded file to memory.    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_compress                                                      */
/*      ps_mfree                    Deallocates memory with a call       */
/*                                  to NU_Deallocate_Memory.             */
/*      memcpy                      Copies one location in memory        */
/*                                  to another with a specified          */
/*                                  length.                              */
/*      strcpy                      Copies one strings contents into     */
/*                                  another strings contents.            */
/*      strcat                      Concantenates two strings.           */
/*      strcmp                      Compares one string with another.    */
/*      send_client_html_msg        Sends HTML error message to client.  */
/*      printf                      Prints a string to console if        */
/*                                  supported.                           */
/*      sprintf                     Prints a message into a string.      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                         Pointer to Request structure that    */
/*                                  holds all information pertaining     */
/*                                  to  the HTTP request.                */
/*      fname                       File name of the file to be saved.   */
/*      filemem                     The pointer to the input buffer that */
/*                                  contains the file.                   */
/*      length                      The length of the file in bytes.     */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The function returns FAILURE if it was unable to write the file  */
/*      to memory. It returns SUCCESS if the saving of the file to       */
/*      memory was complete.                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*        D. Sharer       05-04-98              Corrected Logic and Fixed*/
/*                                              Logic problems.          */
/*************************************************************************/

int save_memfs(Request * req,char *fname,char *filemem,int32 length)
{

#ifdef FILE_COMPRESSION
    int i,j;
    char buf[64];
#endif
    char * nbuf= 0;
    char *n;
    struct fs_file * f, *g, *h;
    short compressed;

    char * s,*t;

    extern struct fs_file  *fs_fil;
    int gif =0,jpg = 0,ssi = 0;
    compressed=0; 

     gif = 0;
     jpg = 0;
     ssi = 0;
#ifdef DEBUG

    printf("memory: save:%s (%x) %d\n",fname,filemem,length);

#endif 

#ifdef FILE_COMPRESSION

#ifdef DEBUG
    printf("COMPRESSION enabled\n");
#endif

    /*  Get the Filename Extension */
    n = fname;
    while (*n != '.')
    {
           if (*(n+1) == NU_NULL)
           {
               n = fname;
               break;
           }
           n++;
    }
    
    n++;
    /*  Gifs, ssi's and Jpegs are already compressed as far as they can be     */
    /*  Do not want to take a chance that the compression algorithm gives an   */
    /*  incorrect size.  SSI's should not be compressed because of the dynamic */
    /*  of the calls.                                                          */
    if (strcmp(n,"gif")== 0)
       gif = 1;
    else if (strcmp(n,"jpg")== 0)
       jpg = 1;
    else if (strcmp(n,"jpeg")==0)
       jpg = 1;
    else if (strcmp(n,"ssi")==0)
       ssi = 1;
    else
    {
       jpg = 0;
       gif = 0;
       ssi = 0;
    }

    /* find out what the compressed size would be */
    i = ps_compress(DONT_OUTPUT ,filemem,NULL,length);

#ifdef  DEBUG
    printf("compressed length = %d\n",i);
#endif

         /* dont bother compressing unless there is some size benefit */
         /* or if the file is a jpeg or gif file */
    if( ((i + CHDSZ ) < (length -16)) && !((gif) || (jpg)|| (ssi)))
    {


#ifdef DEBUG
	    printf("compression version selected\n");
#endif
	    nbuf=(char*) ps_malloc( i + CHDSZ );
	    if( nbuf == FAILURE )
	    {
	        sprintf(buf,
	                "Upload of %s failed. Not enough memory.\n",fname);
	        send_client_html_msg(req,buf);
	        return(FAILURE);
	    }
	    /* do the real compression */
	    j = ps_compress(DO_OUTPUT,filemem,nbuf,length);
	    if( i != j )
	    {
#ifdef DEBUG
	        printf("Compression Phase error\n");
#endif
	    }

	    length =j;                                  /* new length (after compression) */
	    filemem = nbuf;

	    compressed=COMPRESSED;
    }
#endif /*FILE_COMPRESSION */

    /* see if the file exists already */

    s = fname;
    f = h = fs_fil;
    g = NULL;

    while ( f )
    {

	/* TRY:
	 * 
	 * 1. see if it fits in the old slot
	 *    (if we are overwriting an existing file)
	 *
	 * 2. else see if it fits in an unused
	 *    (previously deleated slot)
	 *
	 * 3. If we have no choice we allocate new space 
	 *    for the uploaded file
	 */

	if( f->clength)
	{
	    t= f->name;
	    while ( *t == '/' ) t++;
	    if( strcmp(s,t)==0 )
	    {

		/* FILE EXISTS in incore FS  */
                /*  Make sure file is no bigger than the existing file length */


		f->clength = 0; /* delete old file */
		f->name[0]='\0';

		/* will it fit in old slot ? */
		if( length <= f->length )
                {
                    g = f;            /* yes! found our slot */
                    /*  Set the memory to all 0's */
                    memset(g->addr,0,length);               
                }
		else if( (f->type & COMPILED) == 0 )
		{

		    /* a malloced file slot
		     * delete it  (new file wont fit)
		     */

		    h->next = f->next; /* fix links */
		    ps_mfree(f->addr);
		}
                else if ((f->type & COMPILED) == 1)
                {
                     send_client_html_msg(req,
                     "File Upload Error-File to large to fit in compiled Memory Space");
                     if( compressed )
                        ps_mfree(filemem);    /* free compression buffer */

                     return(FAILURE);
                }    
	    }
	}
	else
	{
	    if( f->clength  == 0 )
	    {                                       /* a previously deleated file */
		
		if( length <= f->length)
		g = f;                              /* found our slot */
	    }
	}
	h = f; /* previous link */
	f= f->next;
    }

    if( g  )
    {

	/* we found a place for the file 
	 * will reuse existing file slot
	 */

	/* save the file */

#ifdef DEBUG
	printf("put new file in old fileslot\n");
#endif
        if (compressed)
        {
            /*  Compressed File tag */
            memcpy(g->addr,CHD,CHDSZ);
            memcpy(g->addr+CHDSZ,filemem,length);
        }
        else
            /*  Don't add compressed File tag */
	        memcpy(g->addr,filemem,length);
          
	strcpy(g->name,fname);
	g->clength = length;
	g->type &= ~COMPRESSED;
	g->type |= compressed;
    }
    else


	if( g == NULL )
	{                                           /* no luck, must allocate space */

#ifdef DEBUG
	    printf("ps_malloc new slot for file\n");
#endif
		
        nbuf =(char *)( ps_malloc(CHDSZ+ length + sizeof( struct fs_file) ));
        if(nbuf == FAILURE)
        {
               send_client_html_msg(req,
                                   "File Upload Error-no memory");
               return(FAILURE);
        }      
                  
	    g = (struct fs_file *) nbuf;
	    g->addr = (nbuf+ (sizeof(struct  fs_file)));
	    strcpy(g->name,"/");
	    strcat(g->name,fname);
        if (compressed)
        {
	        memcpy(g->addr,CHD,CHDSZ);
            memcpy(g->addr+CHDSZ,filemem,length);
            g->type = 0;
	        g->type &= ~COMPRESSED;
	        g->type |= compressed;
	        g->length = length+CHDSZ;
        }
        else
        {
	
            memcpy(g->addr,filemem,length);
            g->type = 0;
            g->type &= ~COMPRESSED;
            g->type |= compressed;
            g->length = length;
        }
	    g->clength = length;
	    h->next = g;
	    g->next =NULL;
	}


    return(SUCCESS);
}

#endif /*FILE_UPLOAD */
