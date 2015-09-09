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
/*      pl_demo.c                                       WEBSERV 1.0      */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains several user plugins that are used in the     */
/*      tutorial demo and the Nucleus Web Server demo that have attached */
/*      HTML files.                                                      */
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
/*      ip_addr                         SSI plugin to find clients ip    */
/*                                      address.                         */
/*      task_stat                       SSI plugin to find Nucleus       */
/*                                      Kernal Tasks Statistics.         */
/*      process_answer                  Plugin that processes an         */
/*                                      argument and sends back a        */
/*                                      response.                        */
/*      task_change                     Plugin to process task requests. */
/*      dummy_task                      Task used for task_change.       */
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



#include "nucleus.h"
#include "ps_pico.h"


char ubuf[600];
void    dummy_task(UNSIGNED  argc, VOID  *argv);
#define Exit(X)  NU_Terminate_Task(NU_Current_Task_Pointer());
NU_TASK DummyTask;
extern NU_MEMORY_POOL    System_Memory;

extern int created;
extern int suspended;    
extern int terminated;

extern Token  *setup_pblock(char * s);
extern void   client_error(Request * req, char * reason);

int             ip_addr(Token * env, Request * req);
int             task_stat(Token * env, Request * req);
int             task_change(Token * env, Request * req);
int             process_answer(Token * env, Request * req);



/* To call this plugin use
 * <!-#ip_addr >
 */
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ip_addr                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Plugin that returns the clients ip address that is connected to  */
/*      the server.  In the tutorial this plugin is used in an SSI script*/
/*      and simply called within the URL to show the CGI effect.         */ 
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
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_net_write                    Writes data out to the Network.  */
/*      sprintf                         Prints a message into a string.  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      env                             Pointer to Token structure that  */
/*                                      holds current token arguments    */
/*                                      (Name and Value) and pointer to  */
/*                                      next token arguments.            */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      REQ_PROCEED                                                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/


int ip_addr(Token * env, Request * req)
{
    unsigned char * p;
    /*  Remove Warnings */
    env = env;
    p = req->ip;
    
    sprintf(ubuf,"<FONT COLOR=\"#980040\"> %d.%d.%d.%d</FONT>",p[0],p[1],p[2],p[3]);
    ps_net_write(req,ubuf,(strlen(ubuf)));
    return(REQ_PROCEED);
}

/* The tag for this looks  like:
 * <!-#task_stat >
 */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      task_stat                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This is a SSI plugin that gets called from HTML and dynamically  */
/*      updates Nucleus Tasks Statistics.                                */     
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Don Sharer - Accelerated Technology                              */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*      process_ssi                     Process a Get Operation with the */
/*                                      .SSI extension.                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_net_write                    Writes data out to the Network.  */
/*      NU_Task_Pointers                                                 */
/*      NU_Task_Information                                              */
/*      NU_Current_Task_Pointer                                          */
/*      sprintf                         Prints a message into a string.  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      env                             Pointer to Token structure that  */
/*                                      holds current token arguments    */
/*                                      (Name and Value) and pointer to  */
/*                                      next token arguments.            */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      REQ_PROCEED                                                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int task_stat(Token * env, Request * req)
{
	
     NU_TASK *Pointer_Array[20];
     UNSIGNED number;
     CHAR task_name[9];
     DATA_ELEMENT  task_status;
     UNSIGNED      scheduled_count;
     OPTION        priority;
     OPTION        preempt;
     UNSIGNED      time_slice;
     VOID          *stack_base;
     UNSIGNED      stack_size;
     UNSIGNED      minimum_stack;
     STATUS        status;
     int           len=8;
     NU_TASK       *task_ptr;
     char *test=0;
     UNSIGNED i;

     /*  Remove Warnings */
     env = env;
     status = status;
     /*  Retrieve Number of task pointers  */
     number= NU_Task_Pointers(&Pointer_Array[0],20);
     
/*  Set up table for HTMLMarkup  */ 
     sprintf(ubuf,"<div align=\"center\"><center>");
     ps_net_write(req,ubuf,(strlen(ubuf)));

     sprintf(ubuf,"<table border=\"4\" cellpadding=\"2\" width=\"100%%\">");
     ps_net_write(req,ubuf,(strlen(ubuf)));
	 
/*  Set up the table columns and headers for the SSI Nucleus Kernal Stats */
	  
     sprintf(ubuf,"<tr><td align=\"center\" width=\"10%%\"><FONT COLOR=\"#980040\"> TASK </font></td><td align=\"center\" width=\"20%%\"><FONT COLOR=\"#980040\">TASK STATUS</font></td>    \
		   <td align=\"center\" width=\"10%%\"><FONT COLOR=\"#980040\">#EXECUTED</font></td> <td align=\"center\" width=\"20%%\"><FONT COLOR=\"#980040\"> PRIORITY</font></td>   \
		   <td align=\"center\" width=\"20%%\"><FONT COLOR=\"#980040\">STACKBASE</font></td>  \
		   <td align=\"center\" width=\"20%%\"><FONT COLOR=\"#980040\">STACKSIZE</font></td></tr>");
    

     ps_net_write(req,ubuf,(strlen(ubuf)));              
     

/*  Retrieve all task pointer statistics and send them back to SSI HTML Markup
    in ssi script  */
     
     for ( i=0; i < number; i++)
     {
	 /*  retrieve task information for task pointer in array  */     
	 status = NU_Task_Information(Pointer_Array[i],task_name,
				      &task_status,
				      &scheduled_count,
				      &priority,
				      &preempt,
				      &time_slice,
				      &stack_base,
				      &stack_size,
				      &minimum_stack);

	 /*  Correct output for task name for task that is over 8 characters */
	 len = strlen(task_name);
	 if (len > 7)
	   task_name[8]='\0';
	  
	 /*  Switch statement used to output correct task_status  */
	 switch(task_status)
	 {

	 case NU_READY:             
	     test="READY";
	     len=strlen(test);
	     test[len]='\0';
	      break;
	 case NU_PURE_SUSPEND:
	      test="PURE_SUSPEND";
	     len=strlen(test);
	     test[len]='\0';
	       break;
	 case NU_FINISHED:
	      test="FINISHED";
	     len=strlen(test);
	     test[len]='\0';
		break;
	 case NU_TERMINATED:
	      test="TERMINATED";
	     len=strlen(test);
	     test[len]='\0';
	       break;
	 case NU_SLEEP_SUSPEND:
	      test="SLEEP_SUSPEND";
	     len=strlen(test);
	     test[len]='\0';
	     break;
	 case NU_MAILBOX_SUSPEND:
	      test="MAILBOX_SUSPEND";
	     len=strlen(test);
	     test[len]='\0';
	     break;
	 case NU_QUEUE_SUSPEND:
	      test="QUEUE_SUSPEND";
	     len=strlen(test);
	     test[len]='\0';
	     break;
	 case NU_PIPE_SUSPEND:
	      test="PIPE_SUSPEND";
	     len=strlen(test);
	     test[len]='\0';
	     break;
	 case NU_EVENT_SUSPEND:
	      test="EVENT_SUSPEND";
	     len=strlen(test);
	     test[len]='\0';
	     break;
	 case NU_SEMAPHORE_SUSPEND:
	      test="SEMAPHORE_SUSPEND";
	     len=strlen(test);
	     test[len]='\0';
	     break;
	 case NU_MEMORY_SUSPEND:
	      test="MEMORY_SUSPEND";
	     len=strlen(test);
	     test[len]='\0';
	     break;
	 case NU_PARTITION_SUSPEND:
	      test="PARTITION_SUSPEND";
	     len=strlen(test);
	     test[len]='\0';
	     break;
	 case NU_DRIVER_SUSPEND:
	      test="DRIVER_SUSPEND";
	     len=strlen(test);
	     test[len]='\0';
	     break;
	 default:
	     break;

	 }

	 /*  Retrieve current task status and set the task_status to be running */
	 task_ptr = NU_Current_Task_Pointer();

	 if(task_ptr ==  Pointer_Array[i])
	 {
	     test = "RUNNING";
	     len=strlen(test);
	     test[len]='\0';
	 }
	 /*  Print to ubuf ctask statistics  */
	 sprintf(ubuf,"<tr><td align=\"center\" width=\"10%%\"><FONT COLOR=\"#980040\">%s</font> </td> <td align=\"center\" width=\"20%%\"><FONT COLOR=\"#980040\">%s</font></td>    \
			   <td align=\"center\" width=\"10%%\"><FONT COLOR=\"#980040\">%lu</font></td> <td align=\"center\" width=\"20%%\"><FONT COLOR=\"#980040\"> %d</font></td>  \
			   <td align=\"center\" width=\"20%%\"><FONT COLOR=\"#980040\">%lx</font></td>  \
			   <td align=\"center\" width=\"20%%\"><FONT COLOR=\"#980040\"> %lu</font></td> </tr>",                                            \
				       task_name, test, scheduled_count, priority, (UNSIGNED)stack_base, stack_size);
    
	 /*  Output Task Statistics   */
	     ps_net_write(req,ubuf,(strlen(ubuf)));


     }
     /*  Make end of table HTML and output */
     sprintf(ubuf,"</table></center></div>");
     ps_net_write(req,ubuf,(strlen(ubuf)));
     
     /*  return to Request Proceed */
	return(REQ_PROCEED);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      process_answer                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Plugin to process the answer to the question in the HTML         */
/*      tutorial.  This operation is called with a forms POST operation. */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*      Donald R. Sharer - Accelerated Technology Inc.                   */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      process_POST                    Processes Post form commands.    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_proto_status                 Sets up the HTTP response.       */
/*      ps_outhdr_insert                Inserts the name and value into  */
/*                                      the output header of the HTTP    */
/*                                      response.                        */
/*      ps_proto_start_response         Starts the HTTP response to the  */
/*                                      client.                          */
/*      ps_net_write                    Writes data out to the Network.  */
/*      strcpy                          Copies one strings contents into */
/*                                      another strings contents.        */
/*      strcat                          Concantenates two strings.       */
/*      strcmp                          Compares one string with another.*/
/*      sprintf                         Prints a message into a string.  */
/*      strncmp                         Compares two strings up to a     */
/*                                      specified number of bytes.       */
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
/*      Always returns REQ_PROCEED.                                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int process_answer(Token * env, Request * req)
{
    char o[255];
    char p[80];
    int  i = 0;
    Token * t;
    char *s;
    char *line;
    env = env;
    t= req->pg_args;
    printf("process_answer\n\r");
    /*  If the arguemnet is not what is expected then get the next packet */
    /*  This is a Netscape Protocol packet */
    if(strcmp(t->arg.name,"RADIOB")!= 0)
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
	
	s = s+4;
	req->pg_args = setup_pblock(s);
        t = req->pg_args;
    }

    /*  This is settig up a page for the response */
    ps_proto_status(req,PROTO_OK);
    ps_outhdr_insert(CONTENT_TYPE, TYPE_TXT_HTML,req->response);
    ps_proto_start_response(req);

    o[0]=0;
    strcat(o,"<BR><FONT COLOR=\"#980040\"><FONT SIZE=4>\n");

    strcat(o,"Reply to Post Form Operation:<BR>");
	
    strcat(o,"<BR><FONT COLOR=\"#980040\">");
    ps_net_write(req,o,strlen(o));

    while ( t )
    {
	 /*  Check if arguemnt name is equal to RadioB              */       
	 if(strcmp(t->arg.name,"RADIOB")== 0)
	 {                                  
	      /*  If the value of the Radio Button is equal to YES.  */
	      if(strncmp(t->arg.value,"YES",3)==0)
	      {                         
		  sprintf(p,"You are an Employee.<BR>");
		  ps_net_write(req,p,strlen(p));
	      }
	      /* If the value of the Radio Button is equal to NO.   */
	      if(strncmp(t->arg.value,"NO",2)==0)
	      {              
		  sprintf(p,"You are not an Employee.<BR>");
		  ps_net_write(req,p,strlen(p));
	      }

	 }
	 /*  Increment Link list */ 
	 t = t->next; 
    }
	
    strcpy(o,"<p align=\"left\"><a href=\"index.htm\"><font size=\"3\">Back to Index</font></a></p>");

    ps_net_write(req,o,strlen(o));

    return(REQ_PROCEED);

}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*     task_change                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*     This is a plugin the processes a PUT or GET form request.         */ 
/*     This plugin takes a simple radip button input to do one of the    */
/*     following:  Create a task, Suspend a Task, Resume a Task,         */
/*     Terminate a Task,  and Reset a Task.  This routine redirects the  */
/*     server to load the ssi.ssi script to show the statistics of the   */
/*     net task.                                                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*     Don Sharer - Accelerated Technology Inc.                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*      process_POST                    Processes Post form commands.    */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory              Allocate memory from memory pool.*/
/*      NU_Create_Task                  Create an application task.      */
/*      NU_Reset_Task                   Reset an application task.       */
/*      NU_Resume_Task                  Resume a suspended Application   */
/*                                      Task.                            */
/*      NU_Suspend_Task                 Suspend an Application Task.     */
/*      NU_Terminate_Task               Terminate an Application Task.   */
/*      ps_proto_redirect               Redirects the  user agent to     */
/*                                      another URL.                     */
/*      strcmp                          Compares one string with another.*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      env                             Pointer to Token structure that  */
/*                                      holds current token arguments    */
/*                                      (Name and Value) and pointer to  */
/*                                      next token arguments.            */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Always returns REQ_PROCEED.                                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int task_change(Token * env, Request * req)
{
    VOID           *pointer;
    STATUS         status;
    Token * t;
    char *s;
    char *line;
    int i = 0;

    /*  Remove Warnings */
    env = env;
    /*  Retrive the argument pointer  */
    t = req->pg_args;        
    if(strcmp(t->arg.name,"RADIOB")!= 0)
    {   /* NetScape Protocol */
        i = ps_net_read(req,req->rdata->lbuf,RECEIVE_SIZE,NET_WAIT);
        if (i < 0)
        {
           /*  An Error has occured */
           return(i);
        }
        line = req->rdata->lbuf;
	s= in_string("\r\n\r\n",line);
	if(s == (char*) FAILURE )
	{
	    client_error(req, "bad POST");
	}
	
	s = s+4;
	req->pg_args = setup_pblock(s);
        t = req->pg_args;
    }
    
    /*  Redirect to ssi.ssi script when completed                   */    
    ps_proto_redirect(req,"/ssi.ssi");

    /*  Arguments are split up into name and value parameters.      */
    /*  Scroll through the arguments until the link list is NULL    */
    while ( t )
    {
	 /*  Check if arguemnt name is equal to RadioB              */       
	 if(strcmp(t->arg.name,"RADIOB")== 0)
	 {                    
	      
	      /*  If it is equal to RadioB check if the value is Create.  */
	      if(strncmp(t->arg.value,"CREATE",6)==0)
	      {
			 
		  /*  Verify that the task has not already been created. */
		  if (!created)
		  {
		       /* Allocate stack space for and create each task in the system.  */
		       /* Create dummy_task.  */
		  
		       status = NU_Allocate_Memory(&System_Memory, &pointer, 1000, 
					NU_NO_SUSPEND);

		       if (status != NU_SUCCESS) 
		       {                            
			   Exit (-1);
		       }

			   /* this creates the dummy server process */ 

		       status = NU_Create_Task(&DummyTask, "DummyTsk",dummy_task, 0, 
						       NU_NULL, pointer, 1000, 3, 100, NU_PREEMPT, NU_START);

		       if (status != NU_SUCCESS) 
		       {                            
			   Exit (2);
		       }
	      
		       /*  Set Created to one so the task cannot be created more than once */
		       created = 1;  

		   }

	      }  
	      /*   Checks if the value is equal to SUSPEND  */
	      if(strncmp(t->arg.value,"SUSPEND",7)==0)
	      {
		   /*  Verifies that the module has not already been suspended */
		   if(!suspended)
		   {

		       /*  Suspend the dummy task  */
		       status= NU_Suspend_Task(&DummyTask);
		       /*  Set the suspended flag to 1 so that the task will not be
			   suspended more than once */
		       suspended= 1;
		   }
	      }
	      /*  Checks if the value is equal to Resume  */
	      if(strncmp(t->arg.value,"RESUME",6)==0)
	      {
		  /*  Verify that the task is suspended */
		   if(suspended)
		   {
		       /*  Resume the task */
		       status= NU_Resume_Task(&DummyTask);
		       /*  Then set suspend to 0 so it can be suspended again */
		       suspended = 0;
		   }
	      }
	      /*  Checks if the value is equal to TERMINATE */       
	      if(strncmp(t->arg.value,"TERMINATE",9)==0)
	      {
		   /*  Make sure task is not already Terminated */
		   if (!terminated)
		   {
		       /*  Terminate the dummy Task  */
		       status= NU_Terminate_Task(&DummyTask);
		       /*  Set terminated to one so the termination task will not be
			   called again until it is reset */
		       terminated = 1;
		   }
	      }
	      /* Checks if the value is equal to Reset */
	      if(strncmp(t->arg.value,"RESET",5)==0)
	      {
		  /*  check that the task has been terminated */
		   if(terminated)
		   {
		       /*  Reset the Dummy Task */
		       status= NU_Reset_Task(&DummyTask,0,NULL);
		       /* Set terminated to a 0 so the task can be terminated again */
		       terminated = 0;
		       /*  Set the suspend to 1 so the task can be resumed.  */
		       suspended = 1;
		   }

	      }


	 }

	 /*  Increment Link list */ 
	 t = t->next; 
    }


    /*  Return Request Proceed */
    return(REQ_PROCEED);

}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*     dummy_task                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*     Simple task used with POST and GET form plugin example.           */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*     Don Sharer - Accelerated Technology Inc.                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*     None.                                                             */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Sleep                        Sleeps for specified number of   */
/*                                      timer ticks.                     */
/*                                                                       */
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

void    dummy_task(UNSIGNED  argc, VOID  *argv)
{
    STATUS status=0;
    unsigned count=0;

/*  Use to dismiss warnings */    
    /*  Remove warnings for unused parameters.  */
    status = (STATUS) argc + (STATUS) argv + status;

/*  Never ending Loop */    
    while(1)
    {
	count++;
	NU_Sleep(10);
    }

}

