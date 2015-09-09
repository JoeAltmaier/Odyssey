/*************************************************************************/
/*                                                                       */
/*            Copyright (c) 1994 - 1998 Accelerated Technology, Inc.     */
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
/*      dbc.c                                           DBUG+  1.2       */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DEBUG       Nucleus DBUG+                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core debugger routines.                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      DBC_Debug                           Main debugger routine        */
/*      DBC_Display_Status                  Main debugger routine        */
/*      DBC_Task_Fields                     Task fields routine          */
/*      DBC_Task_Infomation                 Task information routine     */
/*      DBC_Mailbox_Fields                  Mailbox fields routine       */
/*      DBC_Mailbox_Information             Mailbox Information routine  */
/*      DBC_Queue_Fields                    Queue field routine          */
/*      DBC_Queue_Information               Queue information routine    */
/*      DBC_Pipe_Fields                     Pipe field routine           */
/*      DBC_Pipe_Information                Pipe information routine     */
/*      DBC_Semaphore_Fields                semaphore field routine      */
/*      DBC_Semaphore_Information           semaphore information routine*/
/*      DBC_Event_Fields                    Event field routine          */
/*      DBC_Event_Information               Event information routine    */
/*      DBC_Signal_Fields                   Signal field routine         */
/*      DBC_Signal_Information              Signal information routine   */
/*      DBC_Timer_Fields                    Timer field routine          */
/*      DBC_Timer_Information               Timer information routine    */
/*      DBC_Partition_Fields                Partition field routine      */
/*      DBC_Partition_Information           Partition information routine*/
/*      DBC_Dynamic_Memory_Fields           Dynamic memory field routine */
/*      DBC_Dynamic_Memory_Information      Dynamic memory infor routine */
/*      DBC_Display_Memory                  Display memory               */
/*      DBC_Set_Memory                      Set memory                   */
/*      DBC_Resume_Task                     Resume task                  */
/*      DBC_Suspend_Task                    Suspend task                 */
/*      DBC_Terminate_Task                  Terminate task               */
/*      DBC_Reset_Task                      Reset task                   */
/*      DBC_Change_Priority                 Change task priority         */
/*      DBC_Broadcast_To_Mailbox            Broadcast to mailbox         */
/*      DBC_Receive_From_Mailbox            Receive from mailbox         */
/*      DBC_Reset_Mailbox                   Reset mailbox                */
/*      DBC_Send_To_Mailbox                 Send message to mailbox      */
/*      DBC_Broadcast_To_Queue              Broadcast to queue           */
/*      DBC_Receive_From_Queue              Receive from queue           */
/*      DBC_Reset_Queue                     Reset queue                  */
/*      DBC_Send_To_Front_Of_Queue          Send to the front of queue   */
/*      DBC_Send_To_Queue                   Send message to queue        */
/*      DBC_Broadcast_To_Pipe               Broadcast to a pipe          */
/*      DBC_Receive_From_Pipe               Receive from a pipe          */
/*      DBC_Reset_Pipe                      Reset pipe                   */
/*      DBC_Send_To_Front_Of_Pipe           Send to the front of pipe    */
/*      DBC_Send_To_Pipe                    Send message to pipe         */
/*      DBC_Obtain_Semaphore                Obtain semaphore             */
/*      DBC_Release_Semaphore               Release semaphore            */
/*      DBC_Reset_Semaphore                 Reset semaphore              */
/*      DBC_Retrieve_Events                 Retrieve events              */
/*      DBC_Set_Events                      Set events                   */
/*      DBC_Send_Signals                    Send signals                 */
/*      DBC_Control_Timer                   Control timer                */
/*      DBC_Reset_Timer                     Reset timer                  */
/*      DBC_Retrieve_Clock                  Retrieve system time         */
/*      DBC_Set_Clock                       Set system time              */
/*      DBC_Allocate_Partition              Allocate partition           */
/*      DBC_Deallocate_Partition            Deallocate partition         */
/*      DBC_Allocate_Memory                 Allocate memory              */
/*      DBC_Deallocate_Memory               Deallocate memory            */
/*      DBC_Build_List                      Build list                   */
/*      DBC_Print_Menu                      Print menu                   */
/*      DBC_Print_Status                    Print status                 */
/*      DBC_Input_Line                      Input line                   */
/*      DBC_Get_Name                        Get name                     */
/*      DBC_Name_Prompt                     Prompt user for name         */
/*      DBC_Get_Token                       Get token                    */
/*      DBC_Print_Line                      Print line                   */
/*      DBC_Check_For_Excess                Check for excess             */
/*      DBC_Items_Per_Width                 Items per width              */
/*      DBC_Items_Per_Height                Items per height             */
/*      DBC_Init_Buffer                     Initialize buffer            */
/*      DBC_Pause                           Screen pause                 */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      nucleus.h                           Nucleus PLUS functions       */
/*      db_defs.h                           Debugger Constants           */
/*      db_extr.h                           Debugger Service functions   */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      tm_extr.h                           Timer Control function       */
/*      qu_extr.h                           Queue Control function       */
/*      pi_extr.h                           Pipe functions               */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*      MQ Qian         05-01-1995      Modified for TELNET              */
/*      MQ Qian         01-22-1996      Modified to kill compiler warning*/
/*      MQ Qian         08-13-1996      Added connection check in        */
/*                                      DBC_Print_Line                   */
/*      MQ Qian         10-22-1996   Added ERR_RETURN in DBC_Input_Line. */
/*      D. Sharer       04-14-1998      Removed Warnings.                */
/*      S. Nguyen       05-05-1998,     Changed int16 to int for the     */
/*                                      variable telnet_socket           */
/*      P. Hill         05-07-1998      Cleaned up externs               */
/*                                                                       */
/*************************************************************************/


#include <stdio.h>
#include <string.h>
#include "nucleus.h"
#include "cs_defs.h"
#include "db_defs.h"
#include "db_extr.h"
#include "tc_defs.h"
#include "tm_defs.h"
#include "qu_defs.h"
#include "pi_defs.h"
#include "mb_extr.h"

#include "externs.h"
//#include "config.h"
#include "windat.h"
#include "n_ansi.h"
#include "protocol.h"
#include "tel_extr.h"

/* the following two MACROs is needed to adjust the size of output to match
    the size of terminal of client */
#undef  COL
#undef  ROW
#define COL his_side.width
#define ROW his_side.rows
/* we need set the maximun row and column for global variables */
#define  MAXCOL  85
#define  MAXROW  25

extern unsigned int      telnet_socket;

extern char *server_ip, *server_name;


/* Define various Nucleus debugger static data structures.   */

char            Input_Line[MAXCOL+1];          /* User input line          */
char            Output_Line[MAXCOL+1];         /* User output line         */
char            Token[MAXCOL+1];               /* Token from input line    */
char            Prompt[] =  "\n\rDBUG+>";       /* Nucleus debugger prompt  */
char            New_Line[] =  "\n\r";       /* New line equate          */
VOID            *list[MAX_ITEMS];           /* Pointers list for all    */
STATUS          status;                     /* Status return value      */
UNSIGNED        total_items;                /* Total items in list      */
UNSIGNED        DBC_Size=0;                     /* Size of message          */

/* Define various error messages.  */

char  Invalid_Name[] =        "\n\rNucleus DBUG+ ERROR: Name not found\n\n\r\r";
char  Invalid_Name_Range[] =  "\n\rNucleus DBUG+ ERROR: Name size\n\n\r";
char  Invalid_Address[] =   "\n\rNucleus DBUG+ ERROR: Not a HEX address\n\n\r";
char  Excess_Chars[] =      "\n\rNucleus DBUG+ ERROR: Excess characters\n\n\r";


/* Define constant portions of the Nucleus help menu.  */
char *Help_Menu[] = {
      "ts [name]   Display Task Status  [task name]\n\r",
      "ms [name]   Display Mailbox Status [mailbox name]\n\r",
      "qs [name]   Display Queue Status [queue name]\n\r",
      "ps [name]   Display Pipe Status [pipe name]\n\r",
      "ss [name]   Display Semaphore Status [semaphore name]\n\r",
      "es [name]   Display Event Status [event group name]\n\r",
      "si [name]   Display Signal Status [task name]\n\r",
      "ti [name]   Display Timer Status [timer name]\n\r",
      "pm [name]   Display Partition Status [partition name]\n\r",
      "dm [name]   Display Dynamic Memory Status [memory pool name]\n\r",
      "m  [a]      Display Memory [(H) address]\n\r",
      "sm  a       Set Memory a = (H) address\n\r",
      "\n\r",
      "\n\r",
      "help        Display this menu again\n\r",
      "exit        exit from DBC_Debug() connection\n\r",
      "\n\r",
      "Available Nucleus Service Routines:\n\r",
      "\n\r",
      "NU_Resume_Task                  Resume a task\n\r",
      "NU_Suspend_Task                 Suspend a task\n\r",
      "NU_Terminate_Task               Terminate a task\n\r",
      "NU_Reset_Task                   Resets a task\n\r",
      "NU_Change_Priority              Change a task's priority\n\r",
      "NU_Broadcast_To_Mailbox         Broadcasts message to mailbox\n\r",
      "NU_Receive_From_Mailbox         Receives message from mailbox\n\r",
      "NU_Reset_Mailbox                Resets mailbox\n\r",
      "NU_Send_To_Mailbox              Sends message to mailbox\n\r",
      "NU_Broadcast_To_Queue           Broadcasts message to queue\n\r",
      "NU_Receive_From_Queue           Receives message from queue\n\r",
      "NU_Reset_Queue                  Resets queue\n\r",
      "NU_Send_To_Front_Of_Queue       Sends message to front of queue\n\r",
      "NU_Send_To_Queue                Sends message to queue\n\r",
      "NU_Broadcast_To_Pipe            Broadcasts message to pipe\n\r",
      "NU_Receive_From_Pipe            Receives message from pipe\n\r",
      "NU_Reset_Pipe                   Resets pipe\n\r",
      "NU_Send_To_Front_Of_Pipe        Sends message to front of pipe\n\r",
      "NU_Send_To_Pipe                 Sends message to pipe\n\r",
      "NU_Obtain_Semaphore             Obtains semaphore\n\r",
      "NU_Release_Semaphore            Releases semaphore\n\r",
      "NU_Reset_Semaphore              Reset semaphore\n\r",
      "NU_Retrieve_Events              Retrieves events from a group\n\r",
      "NU_Set_Events                   Sets events in a group\n\r",
      "NU_Receive_Signals              Receives signals\n\r",
      "NU_Send_Signals                 Send signals\n\r",
      "NU_Control_Timer                Controls timer\n\r",
      "NU_Reset_Timer                  Resets timer\n\r",
      "NU_Retrieve_Clock               Retrieve the current time\n\r",
      "NU_Set_Clock                    Set the Nucleus time\n\r",
      "NU_Allocate_Partition           Allocate a memory partition\n\r",
      "NU_Deallocate_Partition         Deallocate a memory partition\n\r",
      "NU_Allocate_Memory              Allocate dynamic memory\n\r",
      "NU_Deallocate_Memory            Deallocate dynamic memory\n\r",
      "\n\r",
      "use 'exit' to exit from DBC_Debug() connection\n\r",
      0};


/* Define the list of valid commands.  */

struct DB_COMMAND_STRUCT
{
    char  *string;                          /* ASCII command string     */
    int   id;                               /* Command ID               */
};

struct DB_COMMAND_STRUCT  DB_Commands[] =  {
        {"ts",                          TS},
        {"TS",                          TS},
        {"ms",                          MS},
        {"MS",                          MS},
        {"qs",                          QS},
        {"QS",                          QS},
        {"ps",                          PS},
        {"PS",                          PS},
        {"ss",                          SS},
        {"SS",                          SS},
        {"es",                          ES},
        {"ES",                          ES},
        {"si",                          SI},
        {"SI",                          SI},
        {"ti",                          TI},
        {"TI",                          TI},
        {"pm",                          PM},
        {"PM",                          PM},
        {"dm",                          DM},
        {"DM",                          DM},
        {"m",                           M},
        {"M",                           M},
        {"sm",                          SM},
        {"SM",                          SM},
        {"help",                        HELP},
        {"HELP",                        HELP},
        {"NU_Resume_Task",              NU_RESUME_TASK},
        {"NU_Suspend_Task",             NU_SUSPEND_TASK},
        {"NU_Terminate_Task",           NU_TERMINATE_TASK},
        {"NU_Reset_Task",               NU_RESET_TASK},
        {"NU_Change_Priority",          NU_CHANGE_PRIORITY},
        {"NU_Broadcast_To_Mailbox",     NU_BROADCAST_TO_MAILBOX},
        {"NU_Receive_From_Mailbox",     NU_RECEIVE_FROM_MAILBOX},
        {"NU_Reset_Mailbox",            NU_RESET_MAILBOX}, 
        {"NU_Send_To_Mailbox",          NU_SEND_TO_MAILBOX},
        {"NU_Broadcast_To_Queue",       NU_BROADCAST_TO_QUEUE},
        {"NU_Receive_From_Queue",       NU_RECEIVE_FROM_QUEUE},
        {"NU_Reset_Queue",              NU_RESET_QUEUE},
        {"NU_Send_To_Front_Of_Queue",   NU_SEND_TO_FRONT_OF_QUEUE},
        {"NU_Send_To_Queue",            NU_SEND_TO_QUEUE},
        {"NU_Broadcast_To_Pipe",        NU_BROADCAST_TO_PIPE},
        {"NU_Receive_From_Pipe",        NU_RECEIVE_FROM_PIPE},
        {"NU_Reset_Pipe",               NU_RESET_PIPE},
        {"NU_Send_To_Front_Of_Pipe",    NU_SEND_TO_FRONT_OF_PIPE},
        {"NU_Send_To_Pipe",             NU_SEND_TO_PIPE},
        {"NU_Obtain_Semaphore",         NU_OBTAIN_SEMAPHORE},
        {"NU_Release_Semaphore",        NU_RELEASE_SEMAPHORE},
        {"NU_Reset_Semaphore",          NU_RESET_SEMAPHORE},
        {"NU_Retrieve_Events",          NU_RETRIEVE_EVENTS},
        {"NU_Set_Events",               NU_SET_EVENTS},
        {"NU_Send_Signals",             NU_SEND_SIGNALS},
        {"NU_Control_Timer",            NU_CONTROL_TIMER},
        {"NU_Reset_Timer",              NU_RESET_TIMER},
        {"NU_Retrieve_Clock",           NU_RETRIEVE_CLOCK},
        {"NU_Set_Clock",                NU_SET_CLOCK},
        {"NU_Allocate_Partition",       NU_ALLOCATE_PARTITION},
        {"NU_Deallocate_Partition",     NU_DEALLOCATE_PARTITION},
        {"NU_Allocate_Memory",          NU_ALLOCATE_MEMORY},
        {"NU_Deallocate_Memory",        NU_DEALLOCATE_MEMORY},
        {0, -1}};


/* Define the Nucleus Display buffer.  This is used to facilitate the column
   display of Nucleus information.  */

char  Output_Buffer[(MAXCOL+1) * MAXROW];
char  *Line_Pointers[MAXROW];

/* Define the Nucleus Data buffer.  This buffer contains a copy of a
   Nucleus data structure or the contents of a queue item to be sent
   or received by the debugger.  */
   
char  Data_Buffer[DB_BUFFER_SIZE];


/* Define the variable that contains the last HEX address displayed.  */

unsigned long *DBC_Address;


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Debug"                  */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This is the main function of the Nucleus debug task.  It is      */
/*      mainly responsible for dispatching commands to the various       */
/*      responsible functions.                                           */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Nucleus scheduler                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DBC_Input_Line                      Input a line from user       */
/*      DBC_Get_Token                       Get a token from the line    */
/*      DBC_Init_Buffer                     initialize buffer            */
/*      DBC_Print_Line                      Print prompt                 */
/*      DBC_Print_Menu                      Print help menu              */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*     MQ Qian          01-17-1995      integrated with telnet           */
/*                                                                       */
/*************************************************************************/
void  DBC_Debug(UNSIGNED argc, VOID *argv)
{

char    *string_ptr;                        /* Working string pointer   */
int     i;                                  /* Working variable         */
char    close=0, put_prompt=1;

/* Access argc and argv to avoid compilation warnings */
status = (STATUS) argc + (STATUS) argv;
status++;

    /* since we have negotiated the size of output, it is necessary to check them */
    if (his_side.width>MAXCOL || his_side.rows>MAXROW)
    {
        message_print("\nclient's screen (%dX%d) is bigger than maximum!",
                his_side.width, his_side.rows);
        return;
    }

    /* Initialize the print buffer.  */
    DBC_Init_Buffer();

    /* Print the menu once for the user to see.  */
    DBC_Print_Menu();

    /* Loop forever, processing commands when entered.  */
    while(1)
    {
        /* Prompt the user for a selection.  */
        if (put_prompt)
            DBC_Print_Line(Prompt);
        put_prompt = 1;

        /* Obtain an input line from the user.  */
        DBC_Input_Line(Input_Line);
        /* Break out the first token to see if a valid command has been
           entered.  */
        string_ptr =  DBC_Get_Token(Input_Line, Token);

#ifdef NETWORK
        /* check if client wants to close the connection */
        if (received_exit(Token, strlen(Token)))
        {
            NU_Sleep(10);
            NU_Close_and_check_retval(telnet_socket, "client");
            break;
        }
        /* check the connection, if it closed, suspend DBC_Debug() */
        if (NU_Telnet_Check_Connection(telnet_socket, close)==SCLOSED)
            break;
#endif /* NETWORK */

        if (Token[0]==0)
        {
            /* eliminate the extra prompt */
            put_prompt = 0;
            continue;
        }

        i = 0;
        /* Determine if the token matches any of the commands.  */
        while ((DB_Commands[i].string) &&
               (DBT_String_Compare(Token,DB_Commands[i].string) == DB_FALSE))
            i++;        /* Look at next command.  */

        /* Process according the the command's ID.  */
        switch(DB_Commands[i].id)
        {

#ifndef NO_STATUS        /* NO_STATUS1 directive */
        case    TS:

            /* Task status is selected.  Call the routine responsible for
               the task status command.  */
            DBC_Display_Status(string_ptr, NU_Task_Pointers((NU_TASK **) list,
                            NU_Established_Tasks()),
                            DBC_Items_Per_Width(20, 10),
                            DBC_Items_Per_Height(10), DBC_Task_Fields,
                            DBC_Task_Information, 9, TASK);
            break;

        case    MS:

            /* Mailbox status is selected.  Call the routine responsible
               for the mailbox status command.  */
            DBC_Display_Status(string_ptr,
                            NU_Mailbox_Pointers((NU_MAILBOX **) list,
                            NU_Established_Mailboxes()),
                            DBC_Items_Per_Width(20, 9),
                            DBC_Items_Per_Height(5), DBC_Mailbox_Fields,
                            DBC_Mailbox_Information, 5, MAILBOX);
            break;

        case    QS:

            /* Queue status is selected.  Call the routine responsible for
               the queue status command.  */
            DBC_Display_Status(string_ptr,
                            NU_Queue_Pointers((NU_QUEUE **) list,
                            NU_Established_Queues()),
                            DBC_Items_Per_Width(20, 9),
                            DBC_Items_Per_Height(11), DBC_Queue_Fields,
                            DBC_Queue_Information, 11, QUEUE);
            break;

        case    PS:

            /* Pipe status is selected.  Call the routine responsible for
               the pipe status command.  */
            DBC_Display_Status(string_ptr,
                            NU_Pipe_Pointers((NU_PIPE **) list,
                            NU_Established_Pipes()),
                            DBC_Items_Per_Width(20, 9),
                            DBC_Items_Per_Height(11), DBC_Pipe_Fields,
                            DBC_Pipe_Information, 11, PIPE);
            break;

        case    SS:

            /* Semaphore status is selected.  Call the routine responsible
               for the semaphore status command.  */
            DBC_Display_Status(string_ptr,
                            NU_Semaphore_Pointers((NU_SEMAPHORE **) list,
                            NU_Established_Semaphores()),
                            DBC_Items_Per_Width(20, 9),
                            DBC_Items_Per_Height(5), DBC_Semaphore_Fields,
                            DBC_Semaphore_Information, 5, SEMAPHORE);
            break;

        case    ES:

            /* Event status is selected.  Call the routine responsible for
               the event status command.  */
            DBC_Display_Status(string_ptr,
                            NU_Event_Group_Pointers((NU_EVENT_GROUP **) list,
                            NU_Established_Event_Groups()),
                            DBC_Items_Per_Width(20, 9),
                            DBC_Items_Per_Height(4), DBC_Event_Fields,
                            DBC_Event_Information, 4, EVENT);
            break;

        case    SI:

            /* Signal status is selected.  Call the routine responsible for
               the signal status command.  */
            DBC_Display_Status(string_ptr, NU_Task_Pointers((NU_TASK **) list,
                            NU_Established_Tasks()),
                            DBC_Items_Per_Width(20, 9),
                            DBC_Items_Per_Height(4), DBC_Signal_Fields,
                            DBC_Signal_Information, 4, SIGNAL);

            break;

        case    TI:

            /* Timer status is selected.  Call the routine responsible for
               the timer status command.  */
            DBC_Display_Status(string_ptr, NU_Timer_Pointers((NU_TIMER **) list,
                            NU_Established_Timers()),
                            DBC_Items_Per_Width(20, 9),
                            DBC_Items_Per_Height(4), DBC_Timer_Fields,
                            DBC_Timer_Information, 4, TIMER);
            break;

        case    PM:

            /* Partition status is selected.  Call the routine responsible
               for the partition status command.  */
            DBC_Display_Status(string_ptr,
                       NU_Partition_Pool_Pointers((NU_PARTITION_POOL **) list,
                       NU_Established_Partition_Pools()),
                       DBC_Items_Per_Width(20, 9),
                       DBC_Items_Per_Height(9), DBC_Partition_Fields,
                       DBC_Partition_Information, 9, PARTITION);
            break;

        case    DM:

            /* Memory status is selected.  Call the routine responsible for
               the memory status command.  */
            DBC_Display_Status(string_ptr,
                            NU_Memory_Pool_Pointers((NU_MEMORY_POOL **) list,
                            NU_Established_Memory_Pools()),
                            DBC_Items_Per_Width(20, 9),
                            DBC_Items_Per_Height(8), DBC_Dynamic_Memory_Fields,
                            DBC_Dynamic_Memory_Information, 8, MEMORY);
            break;

        case    M:

            /* Memory status is selected.  Call the routine responsible for
               the memory status command.  */
            DBC_Display_Memory(string_ptr);
            break;

        case    SM:

            /* Set memory status is selected.  Call the routine responsible
               for the set memory status command.  */
            DBC_Set_Memory(string_ptr);
            break;
  #endif    /* NO_STATUS1 directive */
        case    HELP:

            /* Help menu is selected.  Call the routine responsible for
               the help menu command.  */
            DBC_Print_Menu();
            break;

#ifndef NO_SERVICES /* NO_SERVICES1 directive */

        case    NU_RESUME_TASK:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Resume_Task is selected.  Call the routine responsible for
               the NU_Resume_Task service.  */
            DBC_Resume_Task();
            break;

        case    NU_SUSPEND_TASK:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Suspend_Task is selected.  Call the routine responsible for
               the NU_Suspend_Task service.  */
            DBC_Suspend_Task();
            break;

        case    NU_TERMINATE_TASK:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Terminate_Task is selected.  Call the routine responsible
               for the NU_Terminate_Task service.  */
            DBC_Terminate_Task();
            break;

        case    NU_RESET_TASK:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Reset_Task is selected.  Call the routine responsible for
               the NU_Reset_Task service.  */
            DBC_Reset_Task();
            break;

        case    NU_CHANGE_PRIORITY:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Change_Priority is selected. Call the routine responsible
               for the NU_Change_Priority service.  */
            DBC_Change_Priority();
            break;

        case    NU_BROADCAST_TO_MAILBOX:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Broadcast_To_Mailbox is selected. Call the routine 
            responsible for the NU_Broadcast_To_Mailbox service.  */
            DBC_Broadcast_To_Mailbox();
            break;

        case    NU_RECEIVE_FROM_MAILBOX:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Receive_From_Mailbox is selected. Call the routine 
            responsible for the NU_Receive_From_Mailbox service.  */
            DBC_Receive_From_Mailbox();
            break;

        case    NU_RESET_MAILBOX:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Reset_Mailbox is selected. Call the routine 
            responsible for the NU_Reset_Mailbox service.  */
            DBC_Reset_Mailbox();
            break;

        case    NU_SEND_TO_MAILBOX:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Send_To_Mailbox is selected. Call the routine 
            responsible for the NU_Send_To_Mailbox service.  */
            DBC_Send_To_Mailbox();
            break;

        case    NU_BROADCAST_TO_QUEUE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Broadcast_To_Queue is selected. Call the routine 
            responsible for the NU_Broadcast_To_Queue service.  */
            DBC_Broadcast_To_Queue();
            break;

        case    NU_RECEIVE_FROM_QUEUE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Receive_From_Queue is selected. Call the routine 
            responsible for the NU_Receive_From_Queue service.  */
            DBC_Receive_From_Queue();
            break;

        case    NU_RESET_QUEUE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Reset_Queue is selected. Call the routine 
            responsible for the NU_Reset_Queue service.  */
            DBC_Reset_Queue();
            break;

        case    NU_SEND_TO_FRONT_OF_QUEUE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Send_To_Front_Of_Queue is selected. Call the routine 
            responsible for the NU_Send_To_Front_Of_Queue service.  */
            DBC_Send_To_Front_Of_Queue();
            break;

        case    NU_SEND_TO_QUEUE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Send_To_Queue is selected. Call the routine 
            responsible for the NU_Send_To_Queue service.  */
            DBC_Send_To_Queue();
            break;

        case    NU_BROADCAST_TO_PIPE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Broadcast_To_Pipe is selected. Call the routine
            responsible for the NU_Broadcast_To_Pipe service.  */
            DBC_Broadcast_To_Pipe();
            break;

        case    NU_RECEIVE_FROM_PIPE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Receive_From_Pipe is selected. Call the routine
            responsible for the NU_Receive_From_Pipe service.  */
            DBC_Receive_From_Pipe();
            break;

        case    NU_RESET_PIPE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Reset_Pipe is selected. Call the routine
            responsible for the NU_Reset_Pipe service.  */
            DBC_Reset_Pipe();
            break;

        case    NU_SEND_TO_FRONT_OF_PIPE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Send_To_Front_Of_Pipe is selected. Call the routine
            responsible for the NU_Send_To_Front_Of_Pipe service.  */
            DBC_Send_To_Front_Of_Pipe();
            break;

        case    NU_SEND_TO_PIPE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Send_To_Pipe is selected. Call the routine
            responsible for the NU_Send_To_Pipe service.  */
            DBC_Send_To_Pipe();
            break;

        case    NU_OBTAIN_SEMAPHORE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Obtain_Semaphore is selected. Call the routine
            responsible for the NU_Obtain_Semaphore service.  */
            DBC_Obtain_Semaphore();
            break;

        case    NU_RELEASE_SEMAPHORE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Release_Semaphore is selected. Call the routine
            responsible for the NU_Release_Semaphore service.  */
            DBC_Release_Semaphore();
            break;

        case    NU_RESET_SEMAPHORE:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Reset_Semaphore is selected. Call the routine
            responsible for the NU_Reset_Semaphore service.  */
            DBC_Reset_Semaphore();
            break;

        case    NU_RETRIEVE_EVENTS:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Retrieve_Events is selected. Call the routine
            responsible for the NU_Retrieve_Events service.  */
            DBC_Retrieve_Events();
            break;

        case    NU_SET_EVENTS:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Set_Events is selected. Call the routine
            responsible for the NU_Set_Events service.  */
            DBC_Set_Events();
            break;

        case    NU_SEND_SIGNALS:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Send_Signals is selected. Call the routine
            responsible for the NU_Send_Signals service.  */
            DBC_Send_Signals();
            break;

        case    NU_CONTROL_TIMER:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Control_Timer is selected. Call the routine
            responsible for the NU_Control_Timer service.  */
            DBC_Control_Timer();
            break;


        case    NU_RESET_TIMER:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Reset_Timer is selected. Call the routine
            responsible for the NU_Reset_Timer service.  */
            DBC_Reset_Timer();
            break;

        case    NU_RETRIEVE_CLOCK:

            /* NU_Retrieve_Clock is selected. Call the routine
            responsible for the NU_Retrieve_Clock service.  */
            DBC_Retrieve_Clock();
            break;

        case    NU_SET_CLOCK:

            /* NU_Set_Clock is selected. Call the routine
            responsible for the NU_Set_Clock service.  */
            DBC_Set_Clock();
            break;


        case    NU_ALLOCATE_MEMORY:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Allocate_Memory is selected. Call the routine
            responsible for the NU_Allocate_Memory service.  */
            DBC_Allocate_Memory();
            break;

        case    NU_DEALLOCATE_MEMORY:

            /* NU_Deallocate_Memory is selected. Call the routine
            responsible for the NU_Deallocate_Memory service.  */
            DBC_Deallocate_Memory();
            break;

        case    NU_ALLOCATE_PARTITION:

            /* Prompt user for name */
            DBC_Name_Prompt();
            /* NU_Allocate_Partition is selected. Call the routine
            responsible for the NU_Allocate_Partition service.  */
            DBC_Allocate_Partition();
            break;

        case    NU_DEALLOCATE_PARTITION:

            /* NU_Deallocate_Partition is selected. Call the routine
            responsible for the NU_Deallocate_Partition service.  */
            DBC_Deallocate_Partition();
            break;
#endif /* NO_SERVICES */  /* NO_SERVICES1 directive */

        default:
            if (Token[0] != NUL)
            {
                /* Make sure something is done to avoid warning */
                string_ptr++;

                /* Invalid command entered.  */
                DBC_Print_Line("\n\rDBUG+> Invalid Command: ");
                DBC_Print_Line(Token);
            }
        }
    }
}


#ifndef NO_STATUS      /* NO_STATUS2 directive */
/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Display_Status"         */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the item status.                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Debug                           Main debugger function       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DBC_Build_List                      Builds list of items w/ name */
/*      DBC_Set_List                        Sets list of pointers        */
/*      DBC_Get_Name                        Get a token from the string  */
/*      DBC_Items_Per_Width                 Number of items across       */
/*      DBC_Items_Per_Height                Number of items down         */
/*      DBC_Pause                           Pause for user prompt        */
/*      DBC_Print_Line                      Print a line to the user     */
/*      DBT_Put_Char                        Print Char                   */
/*      NU_Established_Tasks                Get total number of tasks    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Display_Status(char *string, UNSIGNED total, int items_per_width,
                        int items_per_height, VOID (*DB_Item_Fields)(void),
                        VOID (*DB_Item_Information)(int loop),
                        int total_fields, int type)
{
int             i,j;                              /* Working variables  */
int             counter=0;                        /* Index              */
VOID            *temp_list[MAX_ITEMS];            /* Backup list        */
    
    /* Get item name */
    string = DBC_Get_Name(string, Token);

    /* Determine if there was another token.  */
    if (Token[0] != NUL)
    {
        for(i=0;i<(int)total;i++)
            temp_list[i] = list[i];

        DBC_Set_List(type);

        /* check for null value */
        if (total_items == NUL)
        {
            /* Print an error message.  */
            DBC_Print_Line(Invalid_Name);

            /* reset total_items to display all */
            total_items = total;

            for(i=0;i<(int)total;i++)
                list[i] = temp_list[i];
        }
    }

    else
        /* Set total_items to display all */
        total_items = total;

    /* Set counter */
    counter = 0;

    /* Print out the title.  */
    DBC_Print_Line(New_Line);
#ifdef NETWORK
    DBC_Print_Spaces(16);
#else
    for (i = 0; i < (COL/2 - 16); i++)
       DBT_Put_Char(' ');
#endif

    switch(type)
    {
        case TASK:

            DBC_Print_Line(" ***** Task Status Display *****\n\n\r");
            break;

        case MAILBOX:

            DBC_Print_Line(" ***** Mailbox Status Display *****\n\n\r");
            break;

        case QUEUE:

            DBC_Print_Line(" ***** Queue Status Display *****\n\n\r");
            break;

        case PIPE:

            DBC_Print_Line(" ***** Pipe Status Display *****\n\n\r");
            break;

        case SEMAPHORE:

            DBC_Print_Line(" ***** Semaphore Status Display *****\n\n\r");
            break;

        case EVENT:

            DBC_Print_Line(" ***** Event Status Display *****\n\n\r");
            break;

        case SIGNAL:

            DBC_Print_Line(" ***** Signal Status Display *****\n\n\r");
            break;

        case TIMER:

            DBC_Print_Line(" ***** Timer Status Display *****\n\n\r");
            break;

        case PARTITION:

            DBC_Print_Line(" ***** Partition Status Display *****\n\n\r");
            break;

        case MEMORY:

            DBC_Print_Line(" ***** Dynamic Memory Status Display *****\n\n\r");
            break;
    }

    /* Loop through all the the items that need to be displayed.  */
    while (total_items)
    {
        /* Loop through the number of items (or groups) that fit down on the
           screen.   */

        i =  0;

        while((i < items_per_height) && (total_items))
        {
            /* Walk through enough items to exhaust the columns and/or
               exhaust the number of items across.  */

            (*(DB_Item_Fields))();
            j = 0;
            while ((j < items_per_width) && (total_items))
            {

                
                (*(DB_Item_Information))(counter);
                counter++;

                /* Increment the j task variable.  */
                j++;

                /* Decrement the total number of items to print.  */
                total_items--;
            }
            
            /* Now we need to print the display lines.  */
            for (j = 0; j < total_fields; j++)
            {
            
                /* Print the line and position to the next line.  */
                DBC_Print_Line(Line_Pointers[j]);
                DBC_Print_Line(New_Line);
            }

            /* Position to another line and increment the counter for the
               number of items down.  */
            DBC_Print_Line(New_Line);
            i++;
        }

        /* If there are still more items to print, call pause to wait for
           the user to signal continuation.  */
        if (total_items)
        
            /* Pause before we continue on to the next page.  */
            DBC_Pause();
    }
}

/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Task_Fields"            */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the task fields.                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Task_Fields()
{

            /* Initialize the various output lines.  */
            sprintf(Line_Pointers[0],"Task Name:          ");
            sprintf(Line_Pointers[1],"Task Status:        ");
            sprintf(Line_Pointers[2],"Scheduled Count (H):");
            sprintf(Line_Pointers[3],"Priority:           ");
            sprintf(Line_Pointers[4],"Preemptable:        ");
            sprintf(Line_Pointers[5],"Time Slice:         ");
            sprintf(Line_Pointers[6],"Stack Base (H):     ");
            sprintf(Line_Pointers[7],"Stack Size:         "); 
            sprintf(Line_Pointers[8],"Minimum Stack:      ");
}


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Task_Information"       */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the task information.                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Task_Information                 Task information function    */
/*      DBT_String_Cat                      String cat function          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      counter                             Current count                */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Task_Information(int counter)
{
OPTION          priority=0;                       /* Task Priority      */
CHAR            name[9];                          /* Task Name          */
DATA_ELEMENT    task_status;                      /* Current Task Status*/
UNSIGNED        scheduled_count;                  /* Scheduled count    */
OPTION          preempt=0;                        /* Task Preempt       */
UNSIGNED        time_slice;                       /* Task Time/slice    */
VOID            *stack_base;                      /* Task stack base    */
UNSIGNED        stack_size;                       /* Task stack size    */
UNSIGNED        minimum_stack;                    /* Minimum stack      */

    /* Get information */
    NU_Task_Information(list[counter], name, &task_status,
                                        &scheduled_count, &priority,
                                        &preempt, &time_slice, &stack_base,
                                        &stack_size, &minimum_stack);
    /* NUL terminate name */
    name[8] = NUL;
                
    /* Build the Task's fields.  */
    sprintf(Output_Line,"%8s ", name);
    DBT_String_Cat(Line_Pointers[0], Output_Line);

    /* Print a readable suspend status.  */
    switch (task_status)
    {
      
        case    NU_READY:
                
        /* Task is ready.  */
        DBT_String_Cat(Line_Pointers[1],"   READY ");
        break;
                    
        case    NU_PURE_SUSPEND:
                
        /* Task is stopped.  */
        DBT_String_Cat(Line_Pointers[1]," STOPPED ");
        break;

        case    NU_FINISHED:
                
        /* Task is finished.  */
        DBT_String_Cat(Line_Pointers[1],"FINISHED ");
        break;
                    
        case    NU_TERMINATED:
                
        /* Task is terminated.  */
        DBT_String_Cat(Line_Pointers[1],"  KILLED ");
        break;
                    
        case    NU_SLEEP_SUSPEND:
                
        /* Task is waiting for a period of time.  */
        DBT_String_Cat(Line_Pointers[1],"   SLEEP ");
        break;

        case    NU_MAILBOX_SUSPEND:
                
        /* Task is waiting on a mailbox.  */
        DBT_String_Cat(Line_Pointers[1]," MAILBOX ");
        break;

        case    NU_QUEUE_SUSPEND:
                
        /* Task is waiting on a queue.  */
        DBT_String_Cat(Line_Pointers[1],"   QUEUE ");
        break;

        case    NU_PIPE_SUSPEND:
                
        /* Task is waiting on a pipe.  */
        DBT_String_Cat(Line_Pointers[1],"    PIPE ");
        break;

        case    NU_EVENT_SUSPEND:
                
        /* Task is waiting on events.  */
        DBT_String_Cat(Line_Pointers[1],"  EVENTS ");
        break;
                
        case    NU_SEMAPHORE_SUSPEND:
                
        /* Task is waiting on a resource.  */
        DBT_String_Cat(Line_Pointers[1],"    SEMA ");
        break;

        case    NU_MEMORY_SUSPEND:
                
        /* Task is waiting for memory.  */
        DBT_String_Cat(Line_Pointers[1],"  MEMORY ");
        break;

        case    NU_PARTITION_SUSPEND:
                
        /* Task is waiting for a partition.  */
        DBT_String_Cat(Line_Pointers[1],"PARTITION");
        break;

        case    NU_DRIVER_SUSPEND:
                
        /* Task is waiting for a driver.  */
        DBT_String_Cat(Line_Pointers[1],"  DRIVER ");
        break;

        default:

        sprintf(Output_Line,"%8s ", task_status);
        DBT_String_Cat(Line_Pointers[1], Output_Line);
        break;
        }

        sprintf(Output_Line,"%8X ", (UNSIGNED) scheduled_count);
        DBT_String_Cat(Line_Pointers[2], Output_Line);
                
        sprintf(Output_Line,"%8u ", (UNSIGNED) priority);
        DBT_String_Cat(Line_Pointers[3], Output_Line);

        /* Print a readable preempt status.  */
        switch (preempt)
        {
      
            case    10:
                
            /* Task is set for preempt.  */
            DBT_String_Cat(Line_Pointers[4],"     YES ");
            break;

            case    8:
                
            /* Task is set for no preempt.  */
            DBT_String_Cat(Line_Pointers[4],"      NO ");
            break;

            default:

            sprintf(Output_Line,"%8u ", (UNSIGNED) preempt);
            DBT_String_Cat(Line_Pointers[4], Output_Line);
            break;

        }

        /* Print a readable time_slice status.  */
        if (time_slice == 0)

         /* No time slice.  */
         DBT_String_Cat(Line_Pointers[5],"DISABLED ");

         else
         {
             sprintf(Output_Line,"%8u ", (UNSIGNED) time_slice);
             DBT_String_Cat(Line_Pointers[5], Output_Line);
         }

         sprintf(Output_Line,"%08lX ",
                     (UNSIGNED) stack_base);
         DBT_String_Cat(Line_Pointers[6], Output_Line);

         sprintf(Output_Line,"%8u ", (UNSIGNED) stack_size);
         DBT_String_Cat(Line_Pointers[7], Output_Line);
                
         sprintf(Output_Line,"%8u ", (UNSIGNED) minimum_stack);
         DBT_String_Cat(Line_Pointers[8], Output_Line);
                
}


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Mailbox_Fields"         */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the mailbox fields.                      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Mailbox_Fields(void)
{

    /* Initialize the various output lines.  */
    sprintf(Line_Pointers[0],"Mailbox Name:       ");
    sprintf(Line_Pointers[1],"Suspend Type:       ");
    sprintf(Line_Pointers[2],"Message Present:    ");
    sprintf(Line_Pointers[3],"Tasks Waiting:      ");
    sprintf(Line_Pointers[4],"First Task Waiting: ");
}


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Mailbox_Information"    */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the mailbox Information.                 */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Mailbox_Information              Mailbox information service  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      counter                             Current count                */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void DBC_Mailbox_Information(int counter)
{
int             k;
CHAR            name[9];                          /* Mailbox Name       */
OPTION          suspend_type;                     /* Mbox suspend type  */
DATA_ELEMENT    message_present;                  /* Message present    */
UNSIGNED        tasks_waiting;                    /* Scheduled count    */
NU_TASK         *first_task;                      /* First task waiting */

    /* Get information */
    NU_Mailbox_Information(list[counter], name, &suspend_type,
                          &message_present, &tasks_waiting,
                          &first_task);

    /* NUL terminate name */
    name[8] = NUL;
                
    /* Build the Mailbox's fields.  */
    sprintf(Output_Line,"%8s ", name);
    DBT_String_Cat(Line_Pointers[0], Output_Line);

    /* Print a readable suspend status.  */
    switch (suspend_type)
    {
      
        case    NU_FIFO:
                
        /* Mailbox is FIFO.  */
        DBT_String_Cat(Line_Pointers[1],"    FIFO ");
        break;
                    
        case    NU_PRIORITY:
                
        /* Mailbox is PRIORITY.  */
        DBT_String_Cat(Line_Pointers[1],"PRIORITY ");
        break;

        default:

        sprintf(Output_Line,"%8u ", (UNSIGNED) suspend_type);
        DBT_String_Cat(Line_Pointers[1], Output_Line);
        break;
    }

    switch (message_present)
    {
      
        case    NU_TRUE:
                
        /* Mailbox has message.  */
        DBT_String_Cat(Line_Pointers[2],"     YES ");
        break;
                    
        case    NU_FALSE:
                
        /* Mailbox doesn't have message.  */
        DBT_String_Cat(Line_Pointers[2],"      NO ");
        break;

        default:

        sprintf(Output_Line,"%8u ", (UNSIGNED) message_present);
        DBT_String_Cat(Line_Pointers[2], Output_Line);
    }
                
    sprintf(Output_Line,"%8u ", (UNSIGNED) tasks_waiting);
    DBT_String_Cat(Line_Pointers[3], Output_Line);

    /* Check for task waiting */
    if (first_task == NUL)
        DBT_String_Cat(Line_Pointers[4],"    NONE ");
                    
    else
    {
        for (k = 0; k < 8; k++)
        {
            name[k] = ((TC_TCB *) first_task)->tc_name[k];
        }
        name[8] = NUL;
        sprintf(Output_Line,"%8s ", name);
        DBT_String_Cat(Line_Pointers[4], Output_Line);
    }
}


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Queue_Fields"           */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the queue fields.                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Queue_Fields(void)
{
    /* Initialize the various output lines.  */
    sprintf(Line_Pointers[0],"Queue Name:         ");
    sprintf(Line_Pointers[1],"Start Address:      ");
    sprintf(Line_Pointers[2],"Queue Size:         ");
    sprintf(Line_Pointers[3],"Available:          ");
    sprintf(Line_Pointers[4],"Messages:           ");
    sprintf(Line_Pointers[5],"Message Type:       ");
    sprintf(Line_Pointers[6],"Message Size:       ");
    sprintf(Line_Pointers[7],"Suspend Type:       ");
    sprintf(Line_Pointers[8],"Tasks Waiting:      ");
    sprintf(Line_Pointers[9],"First Task Waiting: ");
}


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Queue_Information"      */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the queue information.                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Queue_Information                Queue information service    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      counter                             Current counter              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Queue_Information(int counter)
{
int             k;                                /* for loop varibale  */
CHAR            name[9];                          /* Queue Name         */
OPTION          suspend_type;                     /* Queue suspend type */
OPTION          message_type;                     /* Queue message type */
UNSIGNED        message_size;                     /* Message size       */
UNSIGNED        tasks_waiting;                    /* Tasks Waiting      */
UNSIGNED        queue_size;                       /* Queue Size         */
UNSIGNED        available;                        /* Available          */
UNSIGNED        messages;                         /* Messages           */
NU_TASK         *first_task;                      /* First task waiting */
VOID            *start_address;                   /* Queue Start Address*/


    NU_Queue_Information(list[counter],name, &start_address,
                         &queue_size, &available, &messages, &message_type,
                         &message_size, &suspend_type, &tasks_waiting,
                         &first_task);

    /* NUL terminate name */
    name[8] = NUL;
                
    /* Build the queue's fields.  */
    sprintf(Output_Line,"%8s ", name);
    DBT_String_Cat(Line_Pointers[0], Output_Line);

    sprintf(Output_Line,"%8lX ", (UNSIGNED) start_address);
    DBT_String_Cat(Line_Pointers[1], Output_Line);

    sprintf(Output_Line,"%8u ", (UNSIGNED) queue_size);
    DBT_String_Cat(Line_Pointers[2], Output_Line);

    sprintf(Output_Line,"%8u ", (UNSIGNED) available);
    DBT_String_Cat(Line_Pointers[3], Output_Line);

    sprintf(Output_Line,"%8u ", (UNSIGNED) messages);
    DBT_String_Cat(Line_Pointers[4], Output_Line);

    /* Print a readable suspend status.  */
    switch (message_type)
    {
      
        case    NU_FIXED_SIZE:
                
        /* queue is fixed size.  */
        DBT_String_Cat(Line_Pointers[5],"   FIXED ");
        break;
                    
        case    NU_VARIABLE_SIZE:
                
        /* queue is variable size.  */
        DBT_String_Cat(Line_Pointers[5],"VARIABLE ");
        break;

        default:

        sprintf(Output_Line,"%8u ", (UNSIGNED) message_type);
        DBT_String_Cat(Line_Pointers[5], Output_Line);
        break;
    }

    sprintf(Output_Line,"%8u ", (UNSIGNED) message_size);
    DBT_String_Cat(Line_Pointers[6], Output_Line);

    /* Print a readable suspend status.  */
    switch (suspend_type)
    {
      
        case    NU_FIFO:
                
        /* queue is FIFO.  */
        DBT_String_Cat(Line_Pointers[7],"    FIFO ");
        break;
                    
        case    NU_PRIORITY:
                
        /* queue is PRIORITY.  */
        DBT_String_Cat(Line_Pointers[7],"PRIORITY ");
        break;

        default:

        sprintf(Output_Line,"%8u ", (UNSIGNED) suspend_type);
        DBT_String_Cat(Line_Pointers[7], Output_Line);
        break;
    }

    sprintf(Output_Line,"%8u ", (UNSIGNED) tasks_waiting);
    DBT_String_Cat(Line_Pointers[8], Output_Line);

    if (first_task == NUL)
    DBT_String_Cat(Line_Pointers[9],"    NONE ");
                    
    else
    {
        for (k = 0; k < 8; k++)
            name[k] = ((TC_TCB *) first_task)->tc_name[k];

        name[8] = NUL;
        sprintf(Output_Line,"%8s ", name);
        DBT_String_Cat(Line_Pointers[9], Output_Line);
    }
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Pipe_Fields"            */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the pipe information.                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Pipe_Fields(void)
{
    /* Initialize the various output lines.  */
    sprintf(Line_Pointers[0],"Pipe Name:          ");
    sprintf(Line_Pointers[1],"Start Address:      ");
    sprintf(Line_Pointers[2],"Pipe Size:          ");
    sprintf(Line_Pointers[3],"Available:          ");
    sprintf(Line_Pointers[4],"Messages:           ");
    sprintf(Line_Pointers[5],"Message Type:       ");
    sprintf(Line_Pointers[6],"Message Size:       ");
    sprintf(Line_Pointers[7],"Suspend Type:       ");
    sprintf(Line_Pointers[8],"Tasks Waiting:      ");
    sprintf(Line_Pointers[9],"First Task Waiting: ");
}


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Pipe_Information"       */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the pipe information.                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Pipe_Information                Pipe information service      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      counter                             Current counter              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Pipe_Information(int counter)
{
int             k;                                /* Working variables  */
CHAR            name[9];                          /* pipe Name          */
OPTION          suspend_type;                     /* pipe suspend type  */
OPTION          message_type;                     /* pipe message type  */
UNSIGNED        message_size;                     /* Message size       */
UNSIGNED        tasks_waiting;                    /* Tasks Waiting      */
UNSIGNED        pipe_size;                        /* pipe Size          */
UNSIGNED        available;                        /* Available          */
UNSIGNED        messages;                         /* Messages           */
NU_TASK         *first_task;                      /* First task waiting */
VOID            *start_address;                   /* pipe Start Address */

            
    NU_Pipe_Information(list[counter],name, &start_address,
                              &pipe_size,&available, &messages, &message_type, 
                              &message_size, &suspend_type, &tasks_waiting,
                              &first_task);
    name[8] = NUL;
                
    /* Build the pipe's fields.  */
    sprintf(Output_Line,"%8s ", name);
    DBT_String_Cat(Line_Pointers[0], Output_Line);

    sprintf(Output_Line,"%8lX ", (UNSIGNED) start_address);
    DBT_String_Cat(Line_Pointers[1], Output_Line);

    sprintf(Output_Line,"%8u ", pipe_size);
    DBT_String_Cat(Line_Pointers[2], Output_Line);

    sprintf(Output_Line,"%8u ", available);
    DBT_String_Cat(Line_Pointers[3], Output_Line);

    sprintf(Output_Line,"%8u ", messages);
    DBT_String_Cat(Line_Pointers[4], Output_Line);

    /* Print a readable suspend status.  */
    switch (message_type)
    {
      
        case    NU_FIXED_SIZE:
                
        /* pipe is fixed size.  */
        DBT_String_Cat(Line_Pointers[5],"   FIXED ");
        break;
                    
        case    NU_VARIABLE_SIZE:
                
        /* pipe is variable size.  */
        DBT_String_Cat(Line_Pointers[5],"VARIABLE ");
        break;

        default:

        sprintf(Output_Line,"%8u ", (UNSIGNED) message_type);
        DBT_String_Cat(Line_Pointers[5], Output_Line);
        break;
    }

    sprintf(Output_Line,"%8u ", (UNSIGNED) message_size);
    DBT_String_Cat(Line_Pointers[6], Output_Line);

    /* Print a readable suspend status.  */
    switch (suspend_type)
    {
      
        case    NU_FIFO:
                
        /* pipe is ready.  */
        DBT_String_Cat(Line_Pointers[7],"    FIFO ");
        break;
                    
        case    NU_PRIORITY:
                
        /* pipe is stopped.  */
        DBT_String_Cat(Line_Pointers[7],"PRIORITY ");
        break;

        default:

        sprintf(Output_Line,"%8u ", (UNSIGNED) suspend_type);
        DBT_String_Cat(Line_Pointers[7], Output_Line);
        break;
    }

    sprintf(Output_Line,"%8u ", (UNSIGNED) tasks_waiting);
    DBT_String_Cat(Line_Pointers[8], Output_Line);

    if (first_task == NUL)
        DBT_String_Cat(Line_Pointers[9],"    NONE ");
                    
    else
    {
        for (k = 0; k < 8; k++)
            name[k] = ((TC_TCB *) first_task)->tc_name[k];

        name[8] = NUL;
        sprintf(Output_Line,"%8s ", name);
        DBT_String_Cat(Line_Pointers[9], Output_Line);
    }
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Semaphore_Fields"       */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the semaphore fields.                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Semaphore_Fields(void)
{

    /* Initialize the various output lines.  */
    sprintf(Line_Pointers[0],"Semaphore Name:     ");
    sprintf(Line_Pointers[1],"Current Count:      ");
    sprintf(Line_Pointers[2],"Suspend Type:       ");
    sprintf(Line_Pointers[3],"Tasks Waiting:      ");
    sprintf(Line_Pointers[4],"First Task Waiting: ");
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Semaphore_Information"  */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the semaphore information.               */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Semaphore_Information            Semaphore information service*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      counter                             Current counter              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Semaphore_Information(int counter)
{
int             k;                                /* Working variables  */
CHAR            name[9];                          /* Semaphore Name     */
UNSIGNED        current_count;                    /* Current count      */
OPTION          suspend_type;                     /* Sem. suspend type  */
UNSIGNED        tasks_waiting;                    /* Tasks Waiting      */
NU_TASK         *first_task;                      /* First task waiting */

            
    NU_Semaphore_Information(list[counter], name,
                                 &current_count, &suspend_type, &tasks_waiting,
                                 &first_task);
    name[8] = NUL;
                
    /* Build the semaphore's fields.  */
    sprintf(Output_Line,"%8s ", name);
    DBT_String_Cat(Line_Pointers[0], Output_Line);

    sprintf(Output_Line,"%8lX ", current_count);
    DBT_String_Cat(Line_Pointers[1], Output_Line);

    /* Print a readable suspend status.  */
    switch (suspend_type)
    {
      
        case    NU_FIFO:
                
        /* semaphore is fifo.  */
        DBT_String_Cat(Line_Pointers[2],"    FIFO ");
        break;
                    
        case    NU_PRIORITY:
                
        /* semaphore is priority.  */
        DBT_String_Cat(Line_Pointers[2],"PRIORITY ");
        break;

        default:

        sprintf(Output_Line,"%8u ", (UNSIGNED) suspend_type);
        DBT_String_Cat(Line_Pointers[2], Output_Line);
        break;
    }

    sprintf(Output_Line,"%8u ", (UNSIGNED) tasks_waiting);
    DBT_String_Cat(Line_Pointers[3], Output_Line);

    if (first_task == NUL)
        DBT_String_Cat(Line_Pointers[4],"    NONE ");
                    
    else
    {
        for (k = 0; k < 8; k++)
            name[k] = ((TC_TCB *) first_task)->tc_name[k];

        name[8] = NUL;
        sprintf(Output_Line,"%8s ", name);
        DBT_String_Cat(Line_Pointers[4], Output_Line);
    }
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Event_Fields"           */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the event fields.                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Event_Fields(void)
{
    /* Initialize the various output lines.  */
    sprintf(Line_Pointers[0],"Event Group Name:   ");
    sprintf(Line_Pointers[1],"Event Flags:        ");
    sprintf(Line_Pointers[2],"Tasks Waiting:      ");
    sprintf(Line_Pointers[3],"First Task Waiting: ");
}


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Event_Information"      */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the event information.                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Event_Group_Information          Event information service    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      counter                             Current counter              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Event_Information(int counter)
{
int             k;                                /* Working variables  */
CHAR            name[9];                          /* Event Name         */
UNSIGNED        event_flags;                      /* Event flags        */
UNSIGNED        tasks_waiting;                    /* Tasks Waiting      */
NU_TASK         *first_task;                      /* First task waiting */
            
    NU_Event_Group_Information(list[counter], name,
                                    &event_flags, &tasks_waiting, &first_task);
    name[8] = NUL;
                
    /* Build the event's fields.  */
    sprintf(Output_Line,"%8s ", name);
    DBT_String_Cat(Line_Pointers[0], Output_Line);

    sprintf(Output_Line,"%8lX ", event_flags);
    DBT_String_Cat(Line_Pointers[1], Output_Line);

    sprintf(Output_Line,"%8u ", (UNSIGNED) tasks_waiting);
    DBT_String_Cat(Line_Pointers[2], Output_Line);

    if (first_task == NUL)
        DBT_String_Cat(Line_Pointers[3],"    NONE ");
                    
    else
    {
        for (k = 0; k < 8; k++)
            name[k] = ((TC_TCB *) first_task)->tc_name[k];

        name[8] = NUL;
        sprintf(Output_Line,"%8s ", name);
        DBT_String_Cat(Line_Pointers[3], Output_Line);
    }
}


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Signal_Fields"          */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the signal fields.                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Signal_Fields(void)
{
    /* Initialize the various output lines.  */
    sprintf(Line_Pointers[0],"Task Name:          ");
    sprintf(Line_Pointers[1],"Signal Handler (H): ");
    sprintf(Line_Pointers[2],"Signal Mask (H):    ");
    sprintf(Line_Pointers[3],"Signal Present (H): ");
}


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Signal_Information"     */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the signal information.                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      counter                             Current counter              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Signal_Information(int counter)
{
int             k;                                /* Working variables  */
CHAR            name[9];                          /* Task name          */
TC_TCB          *ptr;                             /* TC_TCB pointer     */
            
    ptr = (TC_TCB *) list[counter];
             
    for (k = 0; k < MAX_NAME+1; k++)
        name[k] = ptr->tc_name[k];

    name[8] = NUL;
                
    /* Build the signal's fields.  */
    sprintf(Output_Line,"%8s ", name);
    DBT_String_Cat(Line_Pointers[0], Output_Line);

    if (ptr->tc_signal_handler != NUL)
    {
        sprintf(Output_Line,"%08lX ", (UNSIGNED)ptr->tc_signal_handler);
        DBT_String_Cat(Line_Pointers[1], Output_Line);
    }

    else
        DBT_String_Cat(Line_Pointers[1],"    NONE ");

    sprintf(Output_Line,"%08lX ", (UNSIGNED) ptr->tc_enabled_signals);
    DBT_String_Cat(Line_Pointers[2], Output_Line);
                
    if (ptr->tc_signals != NUL)
    {
        sprintf(Output_Line,"%08lX ", (UNSIGNED) ptr->tc_signals);
        DBT_String_Cat(Line_Pointers[3], Output_Line);
    }
                
    else
        DBT_String_Cat(Line_Pointers[3],"    NONE ");
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Timer_Fields"           */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the timer fields.                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Timer_Fields(void)
{

    /* Initialize the various output lines.  */
    sprintf(Line_Pointers[0],"Timer Name:       ");
    sprintf(Line_Pointers[1],"Enable:           ");
    sprintf(Line_Pointers[2],"Expirations (H):  ");
    sprintf(Line_Pointers[3],"ID:               ");
    sprintf(Line_Pointers[4],"Initial Time:     ");
    sprintf(Line_Pointers[5],"Reschedule Time:  ");
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Timer_Information"      */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the timer information.                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Timer_Information                Timer information service    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      counter                             Current counter              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Timer_Information(int counter)
{
CHAR            name[9];                          /* Timer name         */
OPTION          enable;                           /* Timer enable       */
UNSIGNED        expirations;                      /* Timer expiration   */
UNSIGNED        id;                               /* Timer ID           */
UNSIGNED        initial_time;                     /* Initial timer time */
UNSIGNED        reschedule_time;                  /* Reschedule time    */
            
    NU_Timer_Information(list[counter], name, &enable,
                           &expirations, &id, &initial_time, &reschedule_time);

    name[8] = NUL;
                
    /* Build the event's fields.  */
    sprintf(Output_Line,"%8s ", name);
    DBT_String_Cat(Line_Pointers[0], Output_Line);

    /* Print a readable suspend status.  */
    switch (enable)
    {
      
        case    NU_ENABLE_TIMER:
                
        /* timer is enabled.  */
        DBT_String_Cat(Line_Pointers[1]," ENABLED ");
        break;
                    
        case    NU_DISABLE_TIMER:
                
        /* timer is disabled.  */
        DBT_String_Cat(Line_Pointers[1],"DISABLED ");
        break;

        default:

        sprintf(Output_Line,"%8u ", (UNSIGNED) enable);
        DBT_String_Cat(Line_Pointers[1], Output_Line);
        break;
    }
                
    sprintf(Output_Line,"%08lX ", expirations);
    DBT_String_Cat(Line_Pointers[2], Output_Line);

    sprintf(Output_Line,"%8u ", id);
    DBT_String_Cat(Line_Pointers[3], Output_Line);

    sprintf(Output_Line,"%8u ", initial_time);
    DBT_String_Cat(Line_Pointers[4], Output_Line);

    sprintf(Output_Line,"%8u ", reschedule_time);
    DBT_String_Cat(Line_Pointers[5], Output_Line);
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Partition_Fields"       */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the partition fields.                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Partition_Fields(void)
{
    sprintf(Line_Pointers[0],"Partition Name:     ");
    sprintf(Line_Pointers[1],"Start Address:      ");
    sprintf(Line_Pointers[2],"Pool Size:          ");
    sprintf(Line_Pointers[3],"Partition Size:     ");
    sprintf(Line_Pointers[4],"Available:          ");
    sprintf(Line_Pointers[5],"Allocated:          ");
    sprintf(Line_Pointers[6],"Suspend Type:       ");
    sprintf(Line_Pointers[7],"Tasks Waiting:      ");
    sprintf(Line_Pointers[8],"First Task Waiting: ");
}


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Partition_Information"  */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the partition information.               */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Partition_Pool_Information       Partition information service*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      counter                             Current counter              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Partition_Information(int counter)
{
int                     k;                        /* Working variables  */
CHAR                    name[9];                  /* Partition  Name    */
VOID                    *start_address;           /* Start address      */
UNSIGNED                pool_size;                /* Pool size          */
UNSIGNED                partition_size;           /* Partition Size     */
UNSIGNED                available;                /* Available          */
UNSIGNED                allocated;                /* Allocated          */
OPTION                  suspend_type;             /* Part. suspend type */
UNSIGNED                tasks_waiting;            /* Tasks Waiting      */
NU_TASK                 *first_task;              /* First task waiting */

    NU_Partition_Pool_Information(list[counter++], name,
                                                &start_address, &pool_size, 
                                                &partition_size, &available, 
                                                &allocated, &suspend_type, 
                                                &tasks_waiting, &first_task);
    name[8] = NUL;
                
    /* Build the partition's fields.  */
    sprintf(Output_Line,"%8s ", name);
    DBT_String_Cat(Line_Pointers[0], Output_Line);

    sprintf(Output_Line,"%8lX ", (UNSIGNED) start_address);
    DBT_String_Cat(Line_Pointers[1], Output_Line);

    sprintf(Output_Line,"%8u ", pool_size);
    DBT_String_Cat(Line_Pointers[2], Output_Line);

    sprintf(Output_Line,"%8u ", partition_size);
    DBT_String_Cat(Line_Pointers[3], Output_Line);

    sprintf(Output_Line,"%8u ", available);
    DBT_String_Cat(Line_Pointers[4], Output_Line);

    sprintf(Output_Line,"%8u ", allocated);
    DBT_String_Cat(Line_Pointers[5], Output_Line);

    /* Print a readable suspend status.  */
    switch (suspend_type)
    {
      
        case    NU_FIFO:
                
        /* partition is FIFO.  */
        DBT_String_Cat(Line_Pointers[6],"    FIFO ");
        break;
                    
        case    NU_PRIORITY:
                
        /* partition is PRIORITY.  */
        DBT_String_Cat(Line_Pointers[6],"PRIORITY ");
        break;

        default:

        sprintf(Output_Line,"%8u ", (UNSIGNED) suspend_type);
        DBT_String_Cat(Line_Pointers[6], Output_Line);
        break;
    }

    sprintf(Output_Line,"%8u ", (UNSIGNED) tasks_waiting);
    DBT_String_Cat(Line_Pointers[7], Output_Line);

    if (first_task == NUL)
        DBT_String_Cat(Line_Pointers[8],"    NONE ");
                    
    else
    {
        for (k = 0; k < 8; k++)
            name[k] = ((TC_TCB *) first_task)->tc_name[k];

        name[8] = NUL;
        sprintf(Output_Line,"%8s ", name);
        DBT_String_Cat(Line_Pointers[8], Output_Line);
    }
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Dynamic_Memory_Fields"  */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the dynamic memory fields.               */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Dynamic_Memory_Fields(void)
{

    /* Initialize the various output lines.  */
    sprintf(Line_Pointers[0],"Memory Pool Name:   ");
    sprintf(Line_Pointers[1],"Start Address:      ");
    sprintf(Line_Pointers[2],"Pool Size:          ");
    sprintf(Line_Pointers[3],"Minimum Allocation: ");
    sprintf(Line_Pointers[4],"Available:          ");
    sprintf(Line_Pointers[5],"Suspend Type:       ");
    sprintf(Line_Pointers[6],"Tasks Waiting:      ");
    sprintf(Line_Pointers[7],"First Task Waiting: ");
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                           "DBC_Dynamic_Memory_Information"  */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine prints out the dynamic memory information.          */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Memory_Pool_Information          Dynamic Memory infor service */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      counter                             Current counter              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Dynamic_Memory_Information(int counter)
{
int             k;                                /* Working variables  */
CHAR            name[9];                          /* Memory pool name   */
VOID            *start_address;                   /* Start address      */
UNSIGNED        pool_size;                        /* Pool size          */
UNSIGNED        min_allocation;                   /* Minimum allocation */
UNSIGNED        available;                        /* Available          */
OPTION          suspend_type;                     /* Part. suspend type */
UNSIGNED        tasks_waiting;                    /* Tasks Waiting      */
NU_TASK         *first_task;                      /* First task waiting */

    NU_Memory_Pool_Information(list[counter], name,
                                                &start_address, &pool_size, 
                                                &min_allocation, &available, 
                                                &suspend_type, &tasks_waiting, 
                                                &first_task);
    name[8] = NUL;
                
    /* Build the memory pool's fields.  */
    sprintf(Output_Line,"%8s ", name);
    DBT_String_Cat(Line_Pointers[0], Output_Line);

    sprintf(Output_Line,"%8lX ", (UNSIGNED) start_address);
    DBT_String_Cat(Line_Pointers[1], Output_Line);

    sprintf(Output_Line,"%8u ", (int) pool_size);
    DBT_String_Cat(Line_Pointers[2], Output_Line);

    sprintf(Output_Line,"%8u ", min_allocation);
    DBT_String_Cat(Line_Pointers[3], Output_Line);

    sprintf(Output_Line,"%8u ", available);
    DBT_String_Cat(Line_Pointers[4], Output_Line);

    /* Print a readable suspend status.  */
    switch (suspend_type)
    {
      
        case    NU_FIFO:
                
        /* memory pool is FIFO.  */
        DBT_String_Cat(Line_Pointers[5],"    FIFO ");
        break;
                    
        case    NU_PRIORITY:
                
        /* memory pool is PRIORITY.  */
        DBT_String_Cat(Line_Pointers[5],"PRIORITY ");
        break;

        default:

        sprintf(Output_Line,"%8u ", (UNSIGNED) suspend_type);
        DBT_String_Cat(Line_Pointers[5], Output_Line);
        break;
    }

    sprintf(Output_Line,"%8u ", tasks_waiting);
    DBT_String_Cat(Line_Pointers[6], Output_Line);

    if (first_task == NUL)
        DBT_String_Cat(Line_Pointers[7],"    NONE ");
                    
    else
    {
        for (k = 0; k < 8; k++)
            name[k] = ((TC_TCB *) first_task)->tc_name[k];

        name[8] = NUL;
        sprintf(Output_Line,"%8s ", name);
        DBT_String_Cat(Line_Pointers[7], Output_Line);
    }
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Display_Memory"         */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine displays memory.                                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Debug                           Main debugger function       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DBC_Get_Token                       Get a token from the string  */
/*      DBT_HEX_ASCII_To_Long               Get HEX address              */
/*      DBC_Check_For_Excess                Check for excess stuff       */
/*      DBC_Print_Line                      Print a line to the user     */
/*      DBT_String_Cat                      Append strings               */
/*      DBC_Pause                           Wait for user input          */
/*      DBC_Items_Per_Width                 Number of items across       */
/*      DBC_Items_Per_Height                Number of items down         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Display_Memory(char *string)
{
int            i;
unsigned long  address;


    /* Determine if there is another token, possibly an address.  */
    string =  DBC_Get_Token(string, Token);
    
    /* Determine if there was another token.  */
    if (Token[0] != NUL)
    {
    
        /* A Token is present.  Attempt to convert the token into an 
           integer.  */
        if (DBT_HEX_ASCII_To_Long(Token, &address) == DB_ERROR)
        {
        
            /* The second parameter was not a HEX number.  Invalid for this
               command.  */
            DBC_Print_Line(Invalid_Address);
            return;
        }
        
        /* One last error check, this time for garbage after the end of the
           line.  */
        if (DBC_Check_For_Excess(string))
        {
        
            /* Yup, there is extra junk at the end of the line.  */
            DBC_Print_Line(Excess_Chars);
            return;
        }

        /* Copy the parameter into the address variable.  */
        DBC_Address =  (unsigned long *) address;
    }

    /* Start display on a new line.  */
    DBC_Print_Line(New_Line);

    /* Display memory for the number of rows in display.  */
    for (i = 0; i < (ROW - 2); i++)
    {

        /* Initialize the various output lines.  */
        sprintf(Output_Line,"%08lX  %08lX\n\r", 
            (unsigned long) DBC_Address, (unsigned long) *DBC_Address);

        /* Print the line and position to the next line.  */
        DBC_Print_Line(Output_Line);
        
        /* Increment the address.  */
        DBC_Address++;
    }
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Set_Memory"             */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine sets memory to input value.                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Debug                           Main debugger function       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DBC_Get_Token                       Get a token from the string  */
/*      DBT_HEX_ASCII_To_Long               Get HEX address              */
/*      DBC_Check_For_Excess                Check for excess stuff       */
/*      DBC_Print_Line                      Print a line to the user     */
/*      DBT_String_Cat                      Append strings               */
/*      DBC_Pause                           Wait for user input          */
/*      DBC_Items_Per_Width                 Number of items across       */
/*      DBC_Items_Per_Height                Number of items down         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Set_Memory(char *string)
{
int            i;
unsigned long  address;
unsigned long  value;
unsigned long *addr_ptr;


    /* Determine if there is another token, possibly an address.  */
    string =  DBC_Get_Token(string, Token);
    
    /* Determine if there was another token.  */
    if (Token[0] == NUL)
    {
    
        /* Print an error message.  */
        DBC_Print_Line("\n\r*** No Address Entered ***\n\n\r");
        return;
    }
    else
    {
    
        /* A Token is present.  Attempt to convert the token into an 
           integer.  */
        if (DBT_HEX_ASCII_To_Long(Token, &address) == DB_ERROR)
        {
        
            /* The second parameter was not a HEX number.  Invalid for this
               command.  */
            DBC_Print_Line(Invalid_Address);
            return;
        }
        
        /* One last error check, this time for garbage after the end of the
           line.  */
        if (DBC_Check_For_Excess(string))
        {
        
            /* Yup, there is extra junk at the end of the line.  */
            DBC_Print_Line(Excess_Chars);
            return;
        }

        /* Copy the parameter into the address variable.  */
        addr_ptr =  (unsigned long *) address;
    }

    /* Display memory for the number of rows in display.  */
    for (i = 0; i < (ROW - 2); i++)
    {

        /* Initialize the various output lines.  */
        sprintf(Output_Line,"\n\r%08lX  %08lX  ", 
                                (unsigned long) addr_ptr, 
                                (unsigned long) *addr_ptr);

        /* Print the line and position to the next line.  */
        DBC_Print_Line(Output_Line);
        
        /* Get an input line.  */
        DBC_Input_Line(Input_Line);
        
        /* Isolate the new data value.  */
        string =  DBC_Get_Token(Input_Line, Token);
    
        /* Determine if there was another token.  */
        if (Token[0] != NUL)
        {
    
            /* A Token is present.  Attempt to convert the token into an 
               unsigned long.  */
            if (DBT_HEX_ASCII_To_Long(Token, &value) == DB_ERROR)
            {
        
                /* The second parameter was not a HEX number.  Invalid for this
                   command.  */
                DBC_Print_Line(Invalid_Address);
                return;
            }
        
            /* One last error check, this time for garbage after the end of the
               line.  */
            if (DBC_Check_For_Excess(string))
            {
        
                /* Yup, there is extra junk at the end of the line.  */
                DBC_Print_Line(Excess_Chars);
                return;
            }
            
            /* Modify the memory location.  */
            *addr_ptr =  value;
        }
        else
        
            /* Terminate the loop when no value is specified.  */
            break;

        /* Increment the address.  */
        addr_ptr++;
    }
    
    /* Print a new line.  */
    DBC_Print_Line(New_Line);
}
#endif        /* NO_STATUS2 directive */


/*************************************************************************/
/*                                                                       */
/*  FUNCTION                           "DBC_Name_Prompt"                 */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function prompts user for item name                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Display_Status                  Display status function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DBC_Print_Line                                                   */
/*      DBC_Input_Line                                                   */
/*      DBC_Get_Name                                                     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Name_Prompt()
{

    /* Prompt the user for the task name.  */
    DBC_Print_Line("\n\rEnter Item Name:                  ");
    
    /* Get the new user input line.  */
    DBC_Input_Line(Input_Line);

    /* Determine if there is another token, possibly an name.  */
    DBC_Get_Name(Input_Line, Token);
}



/*************************************************************************/
/*                                                                       */
/*  FUNCTION                           "DBC_Set_List"                    */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sets the list elements to the structure type       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      type                                Structure type               */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void    DBC_Set_List(int type)
{
    switch(type)
    {
        case    MAILBOX:

        total_items = NU_Mailbox_Pointers((NU_MAILBOX **) list,
                                            NU_Established_Mailboxes());
        total_items = DBC_Build_List((char **) list,
                                           sizeof(CS_NODE) + sizeof(UNSIGNED), 
                                           (int) total_items, (char *) Token);
        break;

        case    PIPE:

        total_items = NU_Pipe_Pointers((NU_PIPE **) list,
                                                NU_Established_Pipes());
        total_items = DBC_Build_List((char **) list,
                                           sizeof(CS_NODE) + sizeof(UNSIGNED), 
                                           (int) total_items, (char *) Token);
        break;

        case    QUEUE:

        total_items = NU_Queue_Pointers((NU_QUEUE **) list,
                                            NU_Established_Queues());
        total_items = DBC_Build_List((char **) list,
                                           sizeof(CS_NODE) + sizeof(UNSIGNED), 
                                           (int) total_items, (char *) Token);
        break;

        case    SEMAPHORE:

        total_items = NU_Semaphore_Pointers((NU_SEMAPHORE **) list,
                                            NU_Established_Semaphores());
        total_items = DBC_Build_List((char **) list,
                                           sizeof(CS_NODE) + sizeof(UNSIGNED), 
                                           (int) total_items, (char *) Token);
        break;

        case    EVENT:

        total_items = NU_Event_Group_Pointers((NU_EVENT_GROUP **) list,
                                        NU_Established_Event_Groups());
        total_items = DBC_Build_List((char **) list,
                                           sizeof(CS_NODE) + sizeof(UNSIGNED), 
                                           (int) total_items, (char *) Token);
        break;

        case    TIMER:
        total_items = NU_Timer_Pointers((NU_TIMER **) list,
                                                NU_Established_Timers());
        total_items = DBC_Build_List((char **) list,
                                           sizeof(CS_NODE) + sizeof(UNSIGNED), 
                                           (int) total_items, (char *) Token);
        break;

        case    PARTITION:

        total_items = NU_Partition_Pool_Pointers((NU_PARTITION_POOL **) list,
                                            NU_Established_Partition_Pools());
        total_items = DBC_Build_List((char **) list,
                                           sizeof(CS_NODE) + sizeof(UNSIGNED), 
                                           (int) total_items, (char *) Token);
        break;

        case    MEMORY:

        total_items = NU_Memory_Pool_Pointers((NU_MEMORY_POOL **) list,
                                        NU_Established_Memory_Pools());
        total_items = DBC_Build_List((char **) list,
                       sizeof(CS_NODE) + sizeof(TC_PROTECT) + sizeof(UNSIGNED),
                       (int) total_items, (char *) Token);
        break;

        default:

        total_items = NU_Task_Pointers((NU_TASK **) list,
                                                    NU_Established_Tasks());
        total_items = DBC_Build_List((char **) list,
                                           sizeof(CS_NODE) + sizeof(UNSIGNED), 
                                           (int) total_items, (char *) Token);
    }
}


#ifndef NO_SERVICES /* NO_SERVICES2 directive */
/*************************************************************************/
/*                                                                       */
/*  FUNCTION                                "DBC_Resume_Task"            */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function executes the Nucleus NU_Resume_Task service call.  */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DBC_Debug                           Main debugger function       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DBC_Build_List                      Builds list of task w/name   */
/*      DBC_Get_Name                        Get a token from the string  */
/*      DBC_Input_Line                      Get a line from the user     */
/*      DBC_Print_Line                      Print a line to the user     */
/*      DBC_Print_Status                    Print status of Nucleus call */
/*      NU_Established_Tasks                Get total number of tasks    */
/*      NU_Task_Pointers                    Builds list of task ptrs     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Resume_Task()
{
    DBC_Set_List(TASK);

    if (total_items == 1)
    {
       status =  NU_Resume_Task(list[0]);
       DBC_Print_Status(status);
       return;
    }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Suspend_Task"          */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Suspend_Task service call.*/
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Build_List                      Builds list of task w/name  */
/*      DBC_Get_Name                        Get a token from the string */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*      NU_Established_Tasks                Get total number of tasks   */
/*      NU_Task_information                  Gets information on task   */
/*      NU_Task_Pointers                    Builds list of task ptrs    */
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Suspend_Task()
{
    DBC_Set_List(TASK);

    if (total_items == 1)
    {
       status =  NU_Suspend_Task(list[0]);
       DBC_Print_Status(status);
       return;
    }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Terminate_Task"        */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Terminate_Task            */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Build_List                      Builds list of task w/name  */
/*      DBC_Get_Name                        Get a token from the string */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*      NU_Established_Tasks                Get total number of tasks   */
/*      NU_Task_information                  Gets information on task   */
/*      NU_Task_Pointers                    Builds list of task ptrs    */
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Terminate_Task(void)
{
    DBC_Set_List(TASK);

    if (total_items == 1)
    {
       status =  NU_Terminate_Task(list[0]);
       DBC_Print_Status(status);
       return;
    }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Reset_Task"            */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Reset_Task service call.  */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Build_List                      Builds list of task w/name  */
/*      DBC_Get_Name                        Get a token from the string */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*      NU_Established_Tasks                Get total number of tasks   */
/*      NU_Task_information                  Gets information on task   */
/*      NU_Task_Pointers                    Builds list of task ptrs    */
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Reset_Task(void)
{
UNSIGNED        argc;                          /* Argc value to reset   */
VOID            *argv;                         /* Argv value to reset   */

    argc = NUL;
    argv = NUL;
    
    DBC_Set_List(TASK);

    if (total_items == 1)
    {
       /* Prompt the user for the task name.  */
       DBC_Print_Line("\n\rEnter the value of argc:          ");
    
       /* Get the new user input line.  */
       DBC_Input_Line(Input_Line);
       
       DBC_Get_Token(Input_Line, (CHAR *) &argc);
       DBT_HEX_ASCII_To_Long(Token, &argc);

       /* Prompt the user for the task name.  */
       DBC_Print_Line("\n\rEnter the value of argv:          ");
    
       /* Get the new user input line.  */
       DBC_Input_Line(Input_Line);

       DBC_Get_Token(Input_Line, (CHAR *) argv);
       DBT_HEX_ASCII_To_Long(Token, (UNSIGNED *) &argv);

       status =  NU_Reset_Task(list[0], (UNSIGNED) argc, (VOID *) argv);
       DBC_Print_Status(status);
       return;
    }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Change_Priority"       */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Change_Priority           */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Build_List                      Builds list of task w/name  */
/*      DBC_Get_Name                        Get a token from the string */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      NU_Established_Tasks                Get total number of tasks   */
/*      NU_Task_information                  Gets information on task   */
/*      NU_Task_Pointers                    Builds list of task ptrs    */
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Change_Priority(void)
{
int      priority;                           /* New priority of task    */

  
    DBC_Set_List(TASK);

    if (total_items == 1)
    {

        /* Prompt the user for the new priority.  */
        DBC_Print_Line("\n\rNew priority for the task:        ");
    
        /* Get the new user input line.  */
        DBC_Input_Line(Input_Line);

        /* Determine if there is another token, possibly an priority.  */
        DBC_Get_Token(Input_Line, Token);
    
        if (Token[0] != NUL)
        {
            /* A Token is present.  Attempt to convert the token into an 
            integer.  */

            if (DBT_ASCII_To_Integer(Token, &priority) == DB_ERROR)
            {
               /* The second parameter was not an integer.  Invalid for this
               command.  */
               DBC_Print_Line("\n\r*** Invalid Priority ***\n\n\r");
               return;
            }

            else
            {
                /* Make the service call.  */
                NU_Change_Priority(list[0], (OPTION) priority);
    
                /* Print the result of the call.  */
                DBC_Print_Status(NU_SUCCESS);
                return;
            }
        }
    
    }
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }

}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Broadcast_To_Mailbox"  */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Broadcast_To_Mailbox      */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Broadcast_To_Mailbox(void)
{
int             i;                                /* Index              */
int             item_size;                        /* Item size          */
UNSIGNED        message[4];                       /* Message storage    */
UNSIGNED        temp;                             /* Temp storage       */


    DBC_Set_List(MAILBOX);

    if (total_items == 1)
    {
        item_size = 4;

        /* Otherwise, get the user's message that needs to be placed in the 
        mailbox.  */
    
        /* Prompt the user for the message.  */
        DBC_Print_Line("\n\rEnter the message below (H):");
    
        for (i = 0; i < item_size; i++)
        {
    
            /* Prompt for the word.  */
            sprintf(Output_Line,"\n\rWord  %u:                          ",
                            (UNSIGNED) i);
            DBC_Print_Line(Output_Line);
        
            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly an ID.  */
            DBC_Get_Token(Input_Line, Token);
    
            /* Determine if there was another token.  */
            if (Token[0] == NUL)
            {
    
                /* Print an error message.  */
                DBC_Print_Line("\n\r*** No word entered ***\n\n\r");
                return;
            }
            else 
            {
        
                /* A Token is present.  Attempt to convert the token into an 
                integer.  */
                if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
                {
        
                  /* The second parameter was not an integer.  Invalid for this
                  command.  */
                  DBC_Print_Line("\n\r*** Invalid Message Word ***\n\n\r");
                  return;
                }
            }
        
           /* If we get here everything is still okay.  Copy the message in the
           long variable "temp" into the appropriate slot in the message to
           send.  */
           message[i] = (UNSIGNED)  temp;
        }

        /* Make the service call.  */
        status =  NU_Broadcast_To_Mailbox(list[0], &message[0], NU_NO_SUSPEND);
    
        /* Print the result of the call.  */
        DBC_Print_Status(status);
        return;
        }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }

}




/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Receive_From_Mailbox"  */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Receive_From_Mailbox      */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Receive_From_Mailbox(void)
{
int          i;                             /* Working variables        */
int          item_size;                     /* Number of "ints" in msg  */
UNSIGNED     message[DB_ITEM_SIZE];         /* Message to send          */

    DBC_Set_List(MAILBOX);

    if (total_items == 1)
    {
       item_size = 4;
           
       /* Make the service call.  */
       status =  NU_Receive_From_Mailbox(list[0], &message[0], NU_NO_SUSPEND);
    
       /* Print the message if successful.  */
       if (status >= 0)
       {

           /* Display the retrieved message.  */
           DBC_Print_Line("\n\rThe following message was received (H):");
    
           for (i = 0; i < item_size; i++)
           {
    
               /* Prompt for the word.  */
               sprintf(Output_Line,
                   "\n\rWord  %u:                          %lX", (UNSIGNED) i,
                   message[i]);
               DBC_Print_Line(Output_Line);
           }
        
           /* Print the result of the call.  */
           DBC_Print_Status(status);
           return;
       }
       else
       {
           /* Print the result of the call.  */
           DBC_Print_Status(status);
           return;
       }

    }
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }

}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Reset_Mailbox"         */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Reset_Mailbox             */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Reset_Mailbox(void)
{

    DBC_Set_List(MAILBOX);

    if (total_items == 1)
    {
       /* Make the service call.  */
       status =  NU_Reset_Mailbox(list[0]);
    
       /* Print the result of the call.  */
       DBC_Print_Status(status);
       return;
    }
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }

}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Send_To_Mailbox"       */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Send_To_Mailbox           */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Send_To_Mailbox(void)
{
int             i;                                /* Index              */
int             item_size;                        /* Item size          */
UNSIGNED        message[4];                       /* Message storage    */
UNSIGNED        temp;                             /* Temp storage       */


    DBC_Set_List(MAILBOX);

    if (total_items == 1)
    {
        item_size = 4;

        /* Otherwise, get the user's message that needs to be placed in the 
        mailbox.  */
    
        /* Prompt the user for the message.  */
        DBC_Print_Line("\n\rEnter the message below (H):");
    
        for (i = 0; i < item_size; i++)
        {
    
            /* Prompt for the word.  */
            sprintf(Output_Line,"\n\rWord  %u:                          ", i);
            DBC_Print_Line(Output_Line);
        
            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly an ID.  */
            DBC_Get_Token(Input_Line, Token);
    
            /* Determine if there was another token.  */
            if (Token[0] == NUL)
            {
    
                /* Print an error message.  */
                DBC_Print_Line("\n\r*** No word entered ***\n\n\r");
                return;
            }
            else 
            {
        
                /* A Token is present.  Attempt to convert the token into an 
                integer.  */
                if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
                {
        
                  /* The second parameter was not an integer.  Invalid for this
                  command.  */
                  DBC_Print_Line("\n\r*** Invalid Message Word ***\n\n\r");
                  return;
                }
            }
        
           /* If we get here everything is still okay.  Copy the message in the
           long variable "temp" into the appropriate slot in the message to
           send.  */
           message[i] = (UNSIGNED)  temp;
        }

        /* Make the service call.  */
        status =  NU_Send_To_Mailbox(list[0], &message[0], NU_NO_SUSPEND);
    
        /* Print the result of the call.  */
        DBC_Print_Status(status);
        return;
        }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }

}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Broadcast_To_Queue"    */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Broadcast_To_Queue        */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Broadcast_To_Queue(void)
{
int             i;                                /* Index              */
int             item_size;                        /* Item size          */
UNSIGNED        message[DB_ITEM_SIZE];            /* Message storage    */
UNSIGNED        temp;                             /* Temp storage       */


    DBC_Set_List(QUEUE);

    if (total_items == 1)
    {
        if (((QU_QCB *) list[0])->qu_fixed_size == NU_TRUE)
            item_size = (int) (((QU_QCB *) list[0])->qu_message_size);

        else
        {
            /* Prompt the user for the message.  */
            DBC_Print_Line("\n\rEnter the message size:           ");

            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly a size.  */
            DBC_Get_Token(Input_Line, Token);

            /* A Token is present.  Attempt to convert the token into an 
               integer.  */
            if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Invalid Message size ***\n\n\r");
              return;
            }
            
            else
                item_size = (int) temp;
            
            if (item_size > DB_ITEM_SIZE)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Message size to large ***\n\n\r");
              return;
            }
            
        }
        
        /* Otherwise, get the user's message that needs to be placed in the 
        Queue.  */
    
        /* Prompt the user for the message.  */
        DBC_Print_Line("\n\rEnter the message below (H):");
    
        for (i = 0; i < item_size; i++)
        {
    
            /* Prompt for the word.  */
            sprintf(Output_Line,"\n\rWord  %u:                          ", i);
            DBC_Print_Line(Output_Line);
        
            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly an ID.  */
            DBC_Get_Token(Input_Line, Token);
    
            /* Determine if there was another token.  */
            if (Token[0] == NUL)
            {
    
                /* Print an error message.  */
                DBC_Print_Line("\n\r*** No word entered ***\n\n\r");
                return;
            }
            else 
            {
        
                /* A Token is present.  Attempt to convert the token into an 
                integer.  */
                if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
                {
        
                  /* The second parameter was not an integer.  Invalid for this
                  command.  */
                  DBC_Print_Line("\n\r*** Invalid Message Word ***\n\n\r");
                  return;
                }
            }
        
           /* If we get here everything is still okay.  Copy the message in the
           long variable "temp" into the appropriate slot in the message to
           send.  */
           message[i] = (UNSIGNED)  temp;
        }

        /* Make the service call.  */
        status =  NU_Broadcast_To_Queue(list[0], &message[0], item_size,
                                        NU_NO_SUSPEND);
    
        /* Print the result of the call.  */
        DBC_Print_Status(status);
        return;
        }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Receive_From_Queue"    */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Receive_From_Queue        */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Receive_From_Queue(void)
{
int             i;                                /* Index              */
int             item_size;                        /* Item size          */
UNSIGNED        message[DB_ITEM_SIZE];            /* Message storage    */
UNSIGNED        temp;                             /* Temp storage       */


    DBC_Set_List(QUEUE);

    if (total_items == 1)
    {
       DBC_Size = (UNSIGNED) (((QU_QCB *) list[0]) -> qu_message_size);

       /* Make the service call.  */
       status =  NU_Receive_From_Queue(list[0], &message[0], DBC_Size,
                                        &temp, NU_NO_SUSPEND);
    
       item_size = (int) temp;

       /* Print the message if successful.  */
       if (status >= 0)
       {

           /* Display the received message.  */
           DBC_Print_Line("\n\rThe following message was received (H):");
    
           for (i = 0; i < item_size; i++)
           {
    
               /* Prompt for the word.  */
               sprintf(Output_Line,
                   "\n\rWord  %u:                          %lX", i,
                   message[i]);
               DBC_Print_Line(Output_Line);
           }
        
           /* Print the result of the call.  */
           DBC_Print_Status(status);
           return;
       }
       else
       {
           /* Print the result of the call.  */
           DBC_Print_Status(status);
           return;
       }

    }
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }

}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Reset_Queue"           */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Reset_Queue               */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Reset_Queue(void)
{


    DBC_Set_List(QUEUE);

    if (total_items == 1)
    {
       /* Make the service call.  */
       status =  NU_Reset_Queue(list[0]);
    
       /* Print the result of the call.  */
       DBC_Print_Status(status);
       return;

    }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                               "DBC_Send_To_Front_Of_Queue" */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Send_To_Front_Of_Queue    */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Send_To_Front_Of_Queue(void)
{
int             i;                                /* Index              */
UNSIGNED        item_size;                        /* Item size          */
UNSIGNED        message[DB_ITEM_SIZE];            /* Message storage    */
UNSIGNED        temp;                             /* Temp Storage       */


    DBC_Set_List(QUEUE);

    if (total_items == 1)
    {
        if (((QU_QCB *) list[0])->qu_fixed_size == NU_TRUE)
            item_size = (UNSIGNED) (((QU_QCB *) list[0])->qu_message_size);

        else
        {
            /* Prompt the user for the message.  */
            DBC_Print_Line("\n\rEnter the message size:           ");

            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly a size.  */
            DBC_Get_Token(Input_Line, Token);

            /* A Token is present.  Attempt to convert the token into an 
               integer.  */
            if (DBT_HEX_ASCII_To_Long(Token, &item_size) == DB_ERROR)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Invalid Message size ***\n\n\r");
              return;
            }
            
            if (item_size > DB_ITEM_SIZE)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Message size to large ***\n\n\r");
              return;
            }
            
        }
        
        /* Otherwise, get the user's message that needs to be placed in the 
        Queue.  */
    
        /* Prompt the user for the message.  */
        DBC_Print_Line("\n\rEnter the message below (H):");
    
        for (i = 0; i < item_size; i++)
        {
    
            /* Prompt for the word.  */
            sprintf(Output_Line,"\n\rWord  %u:                          ", i);
            DBC_Print_Line(Output_Line);
        
            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly an ID.  */
            DBC_Get_Token(Input_Line, Token);
    
            /* Determine if there was another token.  */
            if (Token[0] == NUL)
            {
    
                /* Print an error message.  */
                DBC_Print_Line("\n\r*** No word entered ***\n\n\r");
                return;
            }
            else 
            {
        
                /* A Token is present.  Attempt to convert the token into an 
                integer.  */
                if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
                {
        
                  /* The second parameter was not an integer.  Invalid for this
                  command.  */
                  DBC_Print_Line("\n\r*** Invalid Message Word ***\n\n\r");
                  return;
                }
            }
        
           /* If we get here everything is still okay.  Copy the message in the
           long variable "temp" into the appropriate slot in the message to
           send.  */
           message[i] = (UNSIGNED) temp;
        }

        /* Make the service call.  */
        status =  NU_Send_To_Front_Of_Queue(list[0], &message[0],
                                       (UNSIGNED) item_size, NU_NO_SUSPEND);
    
        /* Print the result of the call.  */
        DBC_Print_Status(status);
        return;
        }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                               "DBC_Send_To_Queue"          */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Send_To_Queue             */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
void  DBC_Send_To_Queue(void)
{
int             i;                                /* Index              */
int             item_size;                        /* Item size          */
UNSIGNED        message[DB_ITEM_SIZE];            /* Message to send    */
UNSIGNED        temp;                             /* Temp storage       */


    DBC_Set_List(QUEUE);

    if (total_items == 1)
    {
        if (((QU_QCB *) list[0])->qu_fixed_size == NU_TRUE)
            item_size = (int) (((QU_QCB *) list[0])->qu_message_size);

        else
        {
            /* Prompt the user for the message.  */
            DBC_Print_Line("\n\rEnter the message size:           ");

            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly a size.  */
            DBC_Get_Token(Input_Line, Token);

            /* A Token is present.  Attempt to convert the token into an 
               integer.  */
            if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Invalid Message size ***\n\n\r");
              return;
            }
            
            else
                item_size = (int) temp;
            
            if (item_size > DB_ITEM_SIZE)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Message size to large ***\n\n\r");
              return;
            }
            
        }
        
        /* Otherwise, get the user's message that needs to be placed in the 
        Queue.  */
    
        /* Prompt the user for the message.  */
        DBC_Print_Line("\n\rEnter the message below (H):");
    
        for (i = 0; i < item_size; i++)
        {
    
            /* Prompt for the word.  */
            sprintf(Output_Line,"\n\rWord  %u:                          ", i);
            DBC_Print_Line(Output_Line);
        
            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly an ID.  */
            DBC_Get_Token(Input_Line, Token);
    
            /* Determine if there was another token.  */
            if (Token[0] == NUL)
            {
    
                /* Print an error message.  */
                DBC_Print_Line("\n\r*** No word entered ***\n\n\r");
                return;
            }
            else 
            {
        
                /* A Token is present.  Attempt to convert the token into an 
                integer.  */
                if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
                {
        
                  /* The second parameter was not an integer.  Invalid
                  for this command.  */
                  DBC_Print_Line("\n\r*** Invalid Message Word ***\n\r");
                  return;
                }
            }
        
           /* If we get here everything is still okay.  Copy the message
           in the long variable "temp" into the appropriate slot in the
           message to send.  */
           message[i] = (UNSIGNED) temp;
        }

        /* Make the service call.  */
        status =  NU_Send_To_Queue(list[0], &message[0], item_size,
                                                NU_NO_SUSPEND);
    
        /* Print the result of the call.  */
        DBC_Print_Status(status);
        return;
    }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Broadcast_To_Pipe"     */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Broadcast_To_Pipe         */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Broadcast_To_Pipe(void)
{
int             i;                                 /* Index             */
int             item_size;                         /* Item size         */
UNSIGNED_CHAR   message[DB_ITEM_SIZE];             /* Message to send   */
UNSIGNED        temp;                              /* Temp storage      */


    DBC_Set_List(PIPE);

    if (total_items == 1)
    {
 
        if (((QU_QCB *) list[0])->qu_fixed_size == NU_TRUE)
            item_size = (int) (((QU_QCB *) list[0])->qu_message_size);

        else
        {
            /* Prompt the user for the message.  */
            DBC_Print_Line("\n\rEnter the message size:           ");

            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly a size.  */
            DBC_Get_Token(Input_Line, Token);

            /* A Token is present.  Attempt to convert the token into an 
               integer.  */
            if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Invalid Message size ***\n\n\r");
              return;
            }
            
            else
                item_size = (int) temp;
            
            if (item_size > DB_ITEM_SIZE)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Message size to large ***\n\n\r");
              return;
            }
            
        }
        
        /* Otherwise, get the user's message that needs to be placed in the 
        Pipe.  */
    
        /* Prompt the user for the message.  */
        DBC_Print_Line("\n\rEnter the message below (H):");
    
        for (i = 0; i < item_size; i++)
        {
    
            /* Prompt for the word.  */
            sprintf(Output_Line,"\n\rByte  %u:                          ", i);
            DBC_Print_Line(Output_Line);
        
            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly an ID.  */
            DBC_Get_Token(Input_Line, Token);
    
            /* Determine if there was another token.  */
            if (Token[0] == NUL)
            {
    
                /* Print an error message.  */
                DBC_Print_Line("\n\r*** No word entered ***\n\n\r");
                return;
            }
            else 
            {
        
                /* A Token is present.  Attempt to convert the token into an 
                integer.  */
                if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
                {
        
                  /* The second parameter was not an integer.  Invalid
                  for this command.  */
                  DBC_Print_Line("\n\r*** Invalid Message Word ***\n\n\r");
                  return;
                }
            }
        
           /* If we get here everything is still okay.  Copy the message in the
           long variable "temp" into the appropriate slot in the message to
           send.  */
           message[i] = (UNSIGNED_CHAR) temp;
        }

        /* Make the service call.  */
        status =  NU_Broadcast_To_Pipe(list[0], &message[0], item_size,
                                        NU_NO_SUSPEND);
    
        /* Print the result of the call.  */
        DBC_Print_Status(status);
        return;
        }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Receive_From_Pipe"     */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Receive_From_Pipe         */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Receive_From_Pipe(void)
{
int             i;                                 /* Index             */
int             item_size;                         /* Item size         */
UNSIGNED_CHAR   message[DB_ITEM_SIZE];             /* Message storage   */
UNSIGNED        temp;                              /* Temp storage      */


    DBC_Set_List(PIPE);

    if (total_items == 1)
    {
       /* Set size to actual size */
       DBC_Size = (UNSIGNED) (((PI_PCB *) list[0]) -> pi_message_size);

       /* Make the service call.  */
       status =  NU_Receive_From_Pipe(list[0], &message[0], DBC_Size,
                                        &temp, NU_NO_SUSPEND);
    
       item_size = (int) temp;

       /* Print the message if successful.  */
       if (status >= 0)
       {

           /* Display the received message.  */
           DBC_Print_Line("\n\rThe following message was received (H):");
    
           for (i = 0; i < item_size; i++)
           {
    
               /* Prompt for the word.  */
               sprintf(Output_Line,
                   "\n\rByte  %u:                          %X", i,
                   (int) message[i]);
               DBC_Print_Line(Output_Line);
           }
        
           /* Print the result of the call.  */
           DBC_Print_Status(status);
           return;
       }
       else
       {
           /* Print the result of the call.  */
           DBC_Print_Status(status);
           return;
       }

    }
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }

}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Reset_Pipe"            */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Reset_Pipe                */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Reset_Pipe(void)
{

    DBC_Set_List(PIPE);

    if (total_items == 1)
    {
       /* Make the service call.  */
       status =  NU_Reset_Pipe(list[0]);
    
       /* Print the result of the call.  */
       DBC_Print_Status(status);
       return;

    }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                               "DBC_Send_To_Front_Of_Pipe"  */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Send_To_Front_Of_Pipe     */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Send_To_Front_Of_Pipe(void)
{
int             i;                                 /* Index             */
int             item_size;                         /* Item size         */
UNSIGNED_CHAR   message[DB_ITEM_SIZE];             /* Message to send   */
UNSIGNED        temp;                              /* Temp storage      */

    DBC_Set_List(PIPE);

    if (total_items == 1)
    {
        if (((QU_QCB *) list[0])->qu_fixed_size == NU_TRUE)
            item_size = (int) (((QU_QCB *) list[0])->qu_message_size);

        else
        {
            /* Prompt the user for the message.  */
            DBC_Print_Line("\n\rEnter the message size:           ");

            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly a size.  */
            DBC_Get_Token(Input_Line, Token);

            /* A Token is present.  Attempt to convert the token into an 
               integer.  */
            if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Invalid Message size ***\n\n\r");
              return;
            }
            
            else
                item_size = (int) temp;
            
            if (item_size > DB_ITEM_SIZE)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Message size to large ***\n\n\r");
              return;
            }
            
        }
        
        /* Otherwise, get the user's message that needs to be placed in the 
        Pipe.  */
    
        /* Prompt the user for the message.  */
        DBC_Print_Line("\n\rEnter the message below (H):");
    
        for (i = 0; i < item_size; i++)
        {
    
            /* Prompt for the word.  */
            sprintf(Output_Line,"\n\rByte  %u:                          ", i);
            DBC_Print_Line(Output_Line);
        
            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly an ID.  */
            DBC_Get_Token(Input_Line, Token);
    
            /* Determine if there was another token.  */
            if (Token[0] == NUL)
            {
    
                /* Print an error message.  */
                DBC_Print_Line("\n\r*** No word entered ***\n\r");
                return;
            }
            else 
            {
        
                /* A Token is present.  Attempt to convert the token into an 
                integer.  */
                if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
                {
        
                  /* The second parameter was not an integer.  Invalid for this
                  command.  */
                  DBC_Print_Line("\n\r*** Invalid Message Word ***\n\n\r");
                  return;
                }
            }
        
           /* If we get here everything is still okay.  Copy the message in the
           long variable "temp" into the appropriate slot in the message to
           send.  */
           message[i] = (UNSIGNED_CHAR)  temp;
        }

        /* Make the service call.  */
        status =  NU_Send_To_Front_Of_Pipe(list[0], &message[0], item_size,
                                                NU_NO_SUSPEND);
    
        /* Print the result of the call.  */
        DBC_Print_Status(status);
        return;
        }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                               "DBC_Send_To_Pipe"           */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Send_To_Pipe              */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Send_To_Pipe(void)
{
int             i;                                /* Index              */
int             item_size;                        /* Item size          */
UNSIGNED_CHAR   message[DB_ITEM_SIZE];            /* Message to send    */
UNSIGNED        temp;                             /* Temp storage       */
    

    DBC_Set_List(PIPE);

    if (total_items == 1)
    {
        if (((QU_QCB *) list[0])->qu_fixed_size == NU_TRUE)
            item_size = (int) (((QU_QCB *) list[0])->qu_message_size);

        else
        {
            /* Prompt the user for the message.  */
            DBC_Print_Line("\n\rEnter the message size:           ");

            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly a size.  */
            DBC_Get_Token(Input_Line, Token);

            /* A Token is present.  Attempt to convert the token into an 
               integer.  */
            if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Invalid Message size ***\n\n\r");
              return;
            }
            
            else
                item_size = (int) temp;
            
            if (item_size > DB_ITEM_SIZE)
            {
        
              /* The second parameter was not an integer.  Invalid for this
                 command.  */
              DBC_Print_Line("\n\r*** Message size to large ***\n\n\r");
              return;
            }
            
        }
        
        /* Otherwise, get the user's message that needs to be placed in the 
        Pipe.  */
    
        /* Prompt the user for the message.  */
        DBC_Print_Line("\n\rEnter the message below (H):");
    
        for (i = 0; i < item_size; i++)
        {
    
            /* Prompt for the word.  */
            sprintf(Output_Line,"\n\rByte  %u:                          ", i);
            DBC_Print_Line(Output_Line);
        
            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly an ID.  */
            DBC_Get_Token(Input_Line, Token);
    
            /* Determine if there was another token.  */
            if (Token[0] == NUL)
            {
    
                /* Print an error message.  */
                DBC_Print_Line("\n\r*** No word entered ***\n\n\r");
                return;
            }
            else 
            {
        
                /* A Token is present.  Attempt to convert the token into an 
                integer.  */
                if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
                {
        
                  /* The second parameter was not an integer.  Invalid for this
                  command.  */
                  DBC_Print_Line("\n\r*** Invalid Message Word ***\n\n\r");
                  return;
                }
            }
        
           /* If we get here everything is still okay.  Copy the message in the
           long variable "temp" into the appropriate slot in the message to
           send.  */
           message[i] = (UNSIGNED_CHAR)  temp;
        }

        /* Make the service call.  */
        status =  NU_Send_To_Pipe(list[0], &message[0], item_size,
                                                NU_NO_SUSPEND);
    
        /* Print the result of the call.  */
        DBC_Print_Status(status);
        return;
        }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION                                   "NU_Obtain_Semaphore"    */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Obtain_Semaphore          */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Obtain_Semaphore(void)
{

    DBC_Set_List(SEMAPHORE);

    if (total_items == 1)
    {
        /* Make the service call.  */
        status =  NU_Obtain_Semaphore(list[0], NU_NO_SUSPEND);
    
        /* Print the result of the call.  */
        DBC_Print_Status(status);
        return;
        }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                   "NU_Release_Semaphore"   */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Release_Semaphore         */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Release_Semaphore(void)
{

    DBC_Set_List(SEMAPHORE);

    if (total_items == 1)
    {
        /* Make the service call.  */
        status =  NU_Release_Semaphore(list[0]);
    
        /* Print the result of the call.  */
        DBC_Print_Status(status);
        return;
        }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                   "NU_Reset_Semaphore"     */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Reset_Semaphore           */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Reset_Semaphore(void)
{
UNSIGNED        initial_count;                    /* Initial count      */

    DBC_Set_List(SEMAPHORE);

    if (total_items == 1)
    {

        /* Otherwise, get the user's initial count that needs to be
           placed in the Semaphore  */
    
        /* Prompt the user for the message.  */
        DBC_Print_Line("\n\rEnter the initial count:          ");

        /* Get the new user input line.  */
        DBC_Input_Line(Input_Line);

        /* Determine if there is another token, possibly an ID.  */
        DBC_Get_Token(Input_Line, Token);

        /* Determine if there was another token.  */
        if (Token[0] == NUL)
        {
    
            /* Print an error message.  */
            DBC_Print_Line("\n\r*** No initial count entered ***\n\n\r");
            return;
        }
        else
        {
        

            /* A Token is present.  Attempt to convert the token into an
            integer.  */
            if (DBT_HEX_ASCII_To_Long(Token, &initial_count) == DB_ERROR)
            {

                /* The second parameter was not an integer.  Invalid for this
                command.  */
                DBC_Print_Line("\n\r*** Invalid Count Value ***\n\n\r");
                return;
            }

            /* Make the service call.  */
            status =  NU_Reset_Semaphore(list[0], (int) initial_count);
    
            /* Print the result of the call.  */
            DBC_Print_Status(status);
            return;
        }
    }
    
    else 
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Retrieve_Events"       */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Retrieve_Events           */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Retrieve_Events(void)
{
OPTION          operation;                        /* Operation flag     */
UNSIGNED        events;                           /* Requested event flg*/
UNSIGNED        retrieved;                        /* Actual event flags */

    DBC_Set_List(EVENT);

    if (total_items == 1)
    {

        /* Prompt the user for the actual operation.  */
        DBC_Print_Line("\n\rEnter operation (AND or OR        ");
        DBC_Print_Line("\n\rAND_CONSUME or OR_CONSUME):       ");

        /* Get the new user input line.  */
        DBC_Input_Line(Input_Line);

        /* Determine if there is another token, possibly an ID.  */
        DBC_Get_Token(Input_Line, Token);
    
        /* Determine if there was another token.  */
        if (Token[0] == NUL)
        {
    
            /* Print an error message.  */
            DBC_Print_Line("\n\r*** Operation not entered ***\n\n\r");
            return;
        }
        else
        {
            /* A Token is present.  Determine if it is legal.  */
            if (DBT_String_Compare(Token, "AND"))
        
               /* AND operation is present.  */
               operation =  NU_AND;

            else if (DBT_String_Compare(Token, "and"))
        
               /* AND operation is present.  */
               operation =  NU_AND;

            else if (DBT_String_Compare(Token, "OR"))
        
                /* OR operation is present.  */
                operation =  NU_OR;

            else if (DBT_String_Compare(Token, "or"))
        
                /* OR operation is present.  */
                operation =  NU_OR;

            else if (DBT_String_Compare(Token, "AND_CONSUME"))
        
               /* AND_CONSUME operation is present.  */
               operation =  NU_AND_CONSUME;

            else if (DBT_String_Compare(Token, "and_consume"))
        
               /* AND_CONSUME operation is present.  */
               operation =  NU_AND_CONSUME;

            else if (DBT_String_Compare(Token, "OR_CONSUME"))
        
                /* OR_CONSUME operation is present.  */
                operation =  NU_OR_CONSUME;

            else if (DBT_String_Compare(Token, "or_consume"))
        
                /* OR_CONSUME operation is present.  */
                operation =  NU_OR_CONSUME;

            else
            {
        
                /* The second parameter was not a valid operation.  */
                DBC_Print_Line("\n\r*** Invalid Event Operation ***\n\n\r");
                return;
            }
    
            /* Prompt the user for the actual event flags.  */
            DBC_Print_Line("\n\rEnter the Event Flag value (H):   ");

            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly an ID.  */
            DBC_Get_Token(Input_Line, Token);
    
            /* Determine if there was another token.  */
            if (Token[0] == NUL)
            {
    
                /* Print an error message.  */
                DBC_Print_Line("\n\r*** Event Flags not entered ***\n\n\r");
                return;
            }
            else
            {
                /* A Token is present.  Attempt to convert the token into an
                unsigned long.  */
                if (DBT_HEX_ASCII_To_Long(Token, &events) == DB_ERROR)
                {
        
                    /* The second parameter was not an integer.  Invalid for
                    this command.  */
                    DBC_Print_Line("\n\r*** Invalid Event Flags ***\n\n\r");
                    return;
                }
            }
            /* Finally, at this point the service call can now be made.  */
            status = NU_Retrieve_Events(list[0], (unsigned int) events,
                                    operation, &retrieved, NU_NO_SUSPEND);

            /* Print the results.  */
            DBC_Print_Status(status);
            return;
        }
    }
    else
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}




/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Set_Events"            */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Set_Events                */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Set_Events(void)
{
OPTION          operation;                        /* Operation flag     */
UNSIGNED        events;                           /* Requested event flg*/


    DBC_Set_List(EVENT);

    if (total_items == 1)
    {

        /* Prompt the user for the actual operation.  */
        DBC_Print_Line("\n\rEnter operation (AND or OR):      ");

        /* Get the new user input line.  */
        DBC_Input_Line(Input_Line);

        /* Determine if there is another token, possibly an ID.  */
        DBC_Get_Token(Input_Line, Token);
    
        /* Determine if there was another token.  */
        if (Token[0] == NUL)
        {
    
            /* Print an error message.  */
            DBC_Print_Line("\n\r*** Operation not entered ***\n\n\r");
            return;
        }
        else
        {
            /* A Token is present.  Determine if it is legal.  */
            if (DBT_String_Compare(Token, "AND"))
        
               /* AND operation is present.  */
               operation =  NU_AND;

            else if (DBT_String_Compare(Token, "and"))
        
               /* AND operation is present.  */
               operation =  NU_AND;

            else if (DBT_String_Compare(Token, "OR"))
        
                /* OR operation is present.  */
                operation =  NU_OR;

            else if (DBT_String_Compare(Token, "or"))
        
                /* OR operation is present.  */
                operation =  NU_OR;

            else
            {
        
                /* The second parameter was not a valid operation.  */
                DBC_Print_Line("\n\r*** Invalid Event Operation ***\n\n\r");
                return;
            }
    
            /* Prompt the user for the actual event flags.  */
            DBC_Print_Line("\n\rEnter the Event Flag value (H):   ");

            /* Get the new user input line.  */
            DBC_Input_Line(Input_Line);

            /* Determine if there is another token, possibly an ID.  */
            DBC_Get_Token(Input_Line, Token);
    
            /* Determine if there was another token.  */
            if (Token[0] == NUL)
            {
    
                /* Print an error message.  */
                DBC_Print_Line("\n\r*** Event Flags not entered ***\n\n\r");
                return;
            }
            else
            {
                /* A Token is present.  Attempt to convert the token into an
                unsigned long.  */
                if (DBT_HEX_ASCII_To_Long(Token, &events) == DB_ERROR)
                {
        
                    /* The second parameter was not an integer.  Invalid for
                    this command.  */
                    DBC_Print_Line("\n\r*** Invalid Event Flags ***\n\n\r");
                    return;
                }
            }
            /* Finally, at this point the service call can now be made.  */
            status = NU_Set_Events(list[0], (unsigned int) events,
                                    operation);

            /* Print the results.  */
            DBC_Print_Status(status);
            return;
        }
    }
    else
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Send_Signals"          */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Send_Signals              */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Build_List                      Builds list of signal w/name*/
/*      DBC_Get_Name                        Get a token from the string */
/*      DBC_Pause                           Pause for user prompt       */
/*      DBC_Print_Line                      Print a line to the user    */
/*      NU_Established_Tasks                Get total number of tasks   */
/*      NU_Task_Pointers                    Builds list of task ptrs    */
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Send_Signals(void)
{
UNSIGNED        signal;                           /* Signal to send     */

    DBC_Set_List(TASK);

    if (total_items == 1)
    {
        /* Prompt the user for the actual event flags.  */
        DBC_Print_Line("\n\rEnter the signal (H):             ");

        /* Get the new user input line.  */
        DBC_Input_Line(Input_Line);

        /* Determine if there is another token, possibly an ID.  */
        DBC_Get_Token(Input_Line, Token);
    
        /* Determine if there was another token.  */
        if (Token[0] == NUL)
        {
    
            /* Print an error message.  */
            DBC_Print_Line("\n\r*** Signal not entered ***\n\n\r");
            return;
        }
        else
        {
            /* A Token is present.  Attempt to convert the token into an
            unsigned long.  */
            if (DBT_HEX_ASCII_To_Long(Token, &signal) == DB_ERROR)
            {
        
                /* The second parameter was not an integer.  Invalid for
                this command.  */
                DBC_Print_Line("\n\r*** Invalid signal ***\n\n\r");
                return;
            }

            /* Finally, at this point the service call can now be made.  */
            status = NU_Send_Signals(list[0], signal);

            /* Print the results.  */
            DBC_Print_Status(status);
            return;
        }
    }
    else
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Control_Timer"         */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Control_Timer             */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Build_List                      Builds list of timer w/ name*/
/*      DBC_Get_Name                        Get a token from the string */
/*      DBC_Pause                           Pause for user prompt       */
/*      DBC_Print_Line                      Print a line to the user    */
/*      NU_Established_Timers               Get total number of timers  */
/*      NU_Timer_Pointers                   Builds list of timer ptrs   */
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Control_Timer(void)
{
OPTION          enable;                           /* Timer enable       */

    DBC_Set_List(TIMER);

    if (total_items == 1)
    {
        /* Prompt the user for enable.  */
        DBC_Print_Line("\n\rEnter the enable:                 ");

        /* Get the new user input line.  */
        DBC_Input_Line(Input_Line);

        /* Determine if there is another token, possibly an name.  */
        DBC_Get_Token(Input_Line, Token);

        if (Token[0] != NUL)
        {
            /* A Token is present.  Determine if it is legal.  */
            if (DBT_String_Compare(Token, "NU_ENABLE_TIMER"))
        
                /* NU_ENABLE_TIMER is present.  */
                enable =  NU_ENABLE_TIMER;

            /* A Token is present.  Determine if it is legal.  */
            if (DBT_String_Compare(Token, "nu_enable_timer"))
        
                /* NU_ENABLE_TIMER is present.  */
                enable =  NU_ENABLE_TIMER;

            /* A Token is present.  Determine if it is legal.  */
            if (DBT_String_Compare(Token, "NU_DISABLE_TIMER"))
        
                /* NU_DISABLE_TIMER is present.  */
                enable =  NU_DISABLE_TIMER;

            /* A Token is present.  Determine if it is legal.  */
            if (DBT_String_Compare(Token, "nu_disable_timer"))
        
                /* NU_DISABLE_TIMER is present.  */
                enable =  NU_DISABLE_TIMER;

        }
        else
        {
        
            /* The this parameter was not a valid operation.  */
            DBC_Print_Line("\n\r*** Invalid Enable ***\n\r");
            return;
        }
    
        /* Finally, at this point the service call can now be made.  */
        status = NU_Control_Timer(list[0], enable);

        /* Print the results.  */
        DBC_Print_Status(status);
        return;
    }
    else
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Reset_Timer"           */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Reset_Timer               */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Build_List                      Builds list of timer w/ name*/
/*      DBC_Get_Name                        Get a token from the string */
/*      DBC_Pause                           Pause for user prompt       */
/*      DBC_Print_Line                      Print a line to the user    */
/*      NU_Established_Timers               Get total number of timers  */
/*      NU_Timer_Pointers                   Builds list of timer ptrs   */
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Reset_Timer(void)
{
UNSIGNED        initial_time;                     /* Initial timer time */
UNSIGNED        reschedule_time;                  /* Reschedule time    */
VOID            (*expiration_routine)(UNSIGNED);  /* Timer expiration   */
OPTION          enable;                           /* Timer enable       */

    DBC_Set_List(TIMER);

    if (total_items == 1)
    {
        /* Prompt the user for expiration routine.  */
        DBC_Print_Line("\n\rEnter the address of the          ");
        DBC_Print_Line("\n\rexpiration routine or             ");
        DBC_Print_Line("\n\rreturn for default:               ");

        /* Get the new user input line.  */
        DBC_Input_Line(Input_Line);

        /* Determine if there is another token, possibly an name.  */
        DBC_Get_Token(Input_Line, Token);
    
        /* Determine if there was another token.  */
        if (Token[0] == NUL)
            expiration_routine =
                    (((TM_APP_TCB *) list[0]) -> tm_expiration_routine);

        else
        {
            /* A Token is present.  Attempt to convert the token into an
            integer.  */

            if (DBT_HEX_ASCII_To_Long(Token,
                    (UNSIGNED *) &expiration_routine) == DB_ERROR)
            {
               /* The second parameter was not an integer.  Invalid for this
               command.  */
               DBC_Print_Line("\n\r*** Invalid expiration_routine ***\n\n\r");
               return;
            }
        }
        /* Prompt the user for initial time.  */
        DBC_Print_Line("\n\rEnter the initial time:           ");

        /* Get the new user input line.  */
        DBC_Input_Line(Input_Line);

        /* Determine if there is another token, possibly an name.  */
        DBC_Get_Token(Input_Line, Token);

        if (Token[0] != NUL)
        {
            /* A Token is present.  Attempt to convert the token into an
            integer.  */

            if (DBT_ASCII_To_Integer(Token, (int *) &initial_time) == DB_ERROR)
            {
               /* The second parameter was not an integer.  Invalid for this
               command.  */
               DBC_Print_Line("\n\r*** Invalid Initial Time ***\n\n\r");
               return;
            }
        }
        else
        {
        
            /* The this parameter was not a valid operation.  */
            DBC_Print_Line("\n\r*** Invalid Initial Time ***\n\n\r");
            return;
        }

        /* Prompt the user for reschedule time.  */
        DBC_Print_Line("\n\rEnter the reschedule time:        ");

        /* Get the new user input line.  */
        DBC_Input_Line(Input_Line);

        /* Determine if there is another token, possibly an name.  */
        DBC_Get_Token(Input_Line, Token);

        if (Token[0] != NUL)
        {
            /* A Token is present.  Attempt to convert the token into an
            integer.  */

            if (DBT_ASCII_To_Integer(Token,
                                    (int *) &reschedule_time) == DB_ERROR)
            {
               /* The second parameter was not an integer.  Invalid for this
               command.  */
               DBC_Print_Line("\n\r*** Invalid reschedule time ***\n\n\r");
               return;
            }
        }
        else
        {
        
            /* The this parameter was not a valid operation.  */
            DBC_Print_Line("\n\r*** Invalid Reschedule Time ***\n\n\r");
            return;
        }

        /* Prompt the user for enable.  */
        DBC_Print_Line("\n\rEnter the enable:                 ");

        /* Get the new user input line.  */
        DBC_Input_Line(Input_Line);

        /* Determine if there is another token, possibly an name.  */
        DBC_Get_Token(Input_Line, Token);

        if (Token[0] != NUL)
        {
            /* A Token is present.  Determine if it is legal.  */
            if (DBT_String_Compare(Token, "NU_ENABLE_TIMER"))
        
                /* NU_ENABLE_TIMER is present.  */
                enable =  NU_ENABLE_TIMER;

            /* A Token is present.  Determine if it is legal.  */
            if (DBT_String_Compare(Token, "NU_DISABLE_TIMER"))
        
                /* NU_DISABLE_TIMER is present.  */
                enable =  NU_DISABLE_TIMER;
        }
        else
        {
        
            /* The this parameter was not a valid operation.  */
            DBC_Print_Line("\n\r*** Invalid Enable ***\n\n\r");
            return;
        }
    
        /* Finally, at this point the service call can now be made.  */
        status = NU_Reset_Timer(list[0], expiration_routine,
                                    initial_time, reschedule_time,
                                    enable);

        /* Print the results.  */
        DBC_Print_Status(status);
        return;
    }
    else
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Retrieve_Clock"        */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Retrieve_Clock            */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      sprintf                             Build formatted string      */
/*      DBC_Print_Line                      Print a line to the user    */
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Retrieve_Clock(void)
{
UNSIGNED      time;                         /* New time                 */

    /* Make the service call.  */
    time =  NU_Retrieve_Clock();
    
    /* Print the time.  */
    sprintf(Output_Line,"\n\rNucleus PLUS time is (in ticks):  %u\n\n\r",
                        time);
    DBC_Print_Line(Output_Line);
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Set_Clock"             */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Set_Clock                 */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Set_Clock(void)
{
unsigned int  time;                         /* New time                 */

    /* Prompt the user for the task ID.  */
    DBC_Print_Line("\n\rNew system time (in ticks):       ");
    
    /* Get the new user input line.  */
    DBC_Input_Line(Input_Line);

    /* Determine if there is another token, possibly an ID.  */
    DBC_Get_Token(Input_Line, Token);
    
    /* Determine if there was another token.  */
    if (Token[0] == NUL)
    {
    
        /* Print an error message.  */
        DBC_Print_Line("\n\r*** No Time Entered ***\n\n\r");
        return;
    }
    else 
    {
        /* A Token is present.  Attempt to convert the token into an 
           integer.  */
        if (DBT_ASCII_To_Integer(Token, (int *) &time) == DB_ERROR)
        {
        
            /* The second parameter was not an integer.  Invalid for this
               command.  */
            DBC_Print_Line("\n\r*** Invalid Time Entered ***\n\n\r");
            return;
        }
        
    }

    /* Make the service call.  */
    NU_Set_Clock(time);
    
    /* Print a new line.  */
    DBC_Print_Line(New_Line);

    /* Print a new line.  */
    DBC_Print_Line(New_Line);
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Allocate_Partition"    */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Allocate_Partition        */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Allocate_Partition(void)
{
VOID                    *dest_ptr;          /* Pointer to memory        */

    DBC_Set_List(PARTITION);

    if (total_items == 1)
    {
        /* Make the service call.  */
        status =  NU_Allocate_Partition(list[0], &dest_ptr, NU_NO_SUSPEND);

        /* If the status is successful, just print out the starting address of
        the memory.  */
        if (status == NU_SUCCESS)
        {
    
            /* Print the address of the partition obtained.  */
            sprintf(Output_Line,
                   "\n\rPartition Start Address (H):      %08lX\n\n\r",
                   (unsigned long) dest_ptr);
            DBC_Print_Line(Output_Line);

        }
        else
    
            /* Print the result of the call.  */
            DBC_Print_Status(status);
            return;
    }
    else
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Deallocate_Partition"  */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Deallocate_Partition      */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_HEX_ASCII_To_Long               Convert to HEX long         */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Deallocate_Partition(void)
{
UNSIGNED               temp;                /* Temp storage             */
VOID                   *dest_ptr;           /* Pointer to memory        */

    /* Prompt the user for the partition address.  */
    DBC_Print_Line("\n\rPartition address (H):            ");
    
    /* Get the new user input line.  */
    DBC_Input_Line(Input_Line);

    /* Determine if there is another token, possibly an address.  */
    DBC_Get_Token(Input_Line, Token);
    
    /* Determine if there was another token.  */
    if (Token[0] == NUL)
    {
    
        /* Print an error message.  */
        DBC_Print_Line("\n\r*** No Address Specified ***\n\n\r");
        return;
    }
    else 
    {
        /* A Token is present.  Attempt to convert the token into an 
           unsigned long.  */
        if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
        {
        
            /* The second parameter was not an address.  Invalid for this
               command.  */
            DBC_Print_Line("\n\r*** Invalid Partition Address ***\n\n\r");
            return;
        }
        
    }

    /* Make the service call.  */
    dest_ptr =  (VOID *)  temp;
    status =  NU_Deallocate_Partition((VOID *) dest_ptr);

    /* Print the result of the call.  */
    DBC_Print_Status(status);
    return;
}    





/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Allocate_Memory"       */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_Allocate_Memory service   */
/*      call.                                                           */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_ASCII_To_Integer                Convert to integer          */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Allocate_Memory(void)
{
VOID            *dest_ptr;                  /* Pointer to memory        */


    DBC_Set_List(MEMORY);

    if (total_items == 1)
    {
        /* Prompt the user for size.  */
        DBC_Print_Line("\n\rEnter the size of memory pool:    ");

        /* Get the new user input line.  */
        DBC_Input_Line(Input_Line);

        /* Determine if there is another token, possibly an name.  */
        DBC_Get_Token(Input_Line, Token);

        if (Token[0] != NUL)
        {
            /* A Token is present.  Attempt to convert the token into an
            integer.  */

            if (DBT_ASCII_To_Integer(Token,
                                    (int *) &DBC_Size) == DB_ERROR)
            {
               /* The second parameter was not an integer.  Invalid for this
               command.  */
               DBC_Print_Line("\n\r*** Invalid Size ***\n\n\r");
               return;
            }
        }
        else
        {
        
            /* The this parameter was not a valid operation.  */
            DBC_Print_Line("\n\r*** Invalid Size ***\n\r");
            return;
        }

        /* Make the service call.  */
        status =  NU_Allocate_Memory(list[0], &dest_ptr, DBC_Size, NU_NO_SUSPEND);

        /* If the status is successful, just print out the starting address of
           the memory.  */
        if (status == NU_SUCCESS)
        {
    
            /* Print the address of the memory obtained.  */
            sprintf(Output_Line,
                   "\n\rMemory Start Address (H):         %08lX\n\n\r",
                   (unsigned long) dest_ptr);
            DBC_Print_Line(Output_Line);
        }
        else
    
            /* Print the result of the call.  */
            DBC_Print_Status(status);
            return;
    }
    else
    {
        /* Print an error message.  */
        DBC_Print_Line(Invalid_Name);
        return;
    }
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Deallocate_Memory"     */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Nucleus NU_DeAllocate_Memory service */
/*      call.                                                           */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Input_Line                      Get a line from the user    */
/*      DBC_Get_Token                       Get a token from the string */
/*      DBT_HEX_ASCII_To_Long               Convert to HEX long         */
/*      DBC_Check_For_Excess                Check for excess stuff      */
/*      DBC_Print_Line                      Print a line to the user    */
/*      DBC_Print_Status                    Print status of Nucleus call*/
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Deallocate_Memory(void)
{
VOID            *dest_ptr;                  /* Pointer to memory        */
UNSIGNED        temp;                       /* Temp storage             */

    /* Prompt the user for the memory address.  */
    DBC_Print_Line("\n\rMemory address (H):               ");
    
    /* Get the new user input line.  */
    DBC_Input_Line(Input_Line);

    /* Determine if there is another token, possibly an ID.  */
    DBC_Get_Token(Input_Line, Token);
    
    /* Determine if there was another token.  */
    if (Token[0] == NUL)
    {
    
        /* Print an error message.  */
        DBC_Print_Line("\n\r*** No Address Specified ***\n\n\r");
        return;
    }
    else 
    {
        /* A Token is present.  Attempt to convert the token into an 
           unsigned long.  */
        if (DBT_HEX_ASCII_To_Long(Token, &temp) == DB_ERROR)
        {
        
            /* The second parameter was not an address.  Invalid for this
               command.  */
            DBC_Print_Line("\n\r*** Invalid Memory Address ***\n\n\r");
            return;
        }
        
    }

    /* Make the service call.  */
    dest_ptr =  (VOID *)  temp;
    status =  NU_Deallocate_Memory(dest_ptr);

    /* Print the result of the call.  */
    DBC_Print_Status(status);
    return;
}    
#endif  /* NO_SERVICES2 directive */



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Build_List"            */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function executes the Debugger DBC_Build_List              */
/*      service call.                                                   */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBT_Name_Compare                         Name compare routine   */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      list                                    List of pointers        */
/*      offset                                  Offset into structure   */
/*      max                                     Maximum number of items */
/*      name                                    Name to compare with    */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      int                                     Total number of matches */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
int DBC_Build_List(char **list, int offset, int max, char *name)
{
int     i, j, k, count;
char    *block_name;
char    *temp[10];
char    string[9];

    temp[0] = NUL;
    j = 0;
    count = 0;
    
    for (i = 0; i < max; i++)
    {
        block_name = (char *) (list[i] + offset);
        for (k = 0; k < 9; k++)
            string[k] = block_name[k];
        string[8] = NUL;
            
        if (DBT_Name_Compare(string, name))
            temp[j++] = list[i];
    }

    for (i = 0; i < j; i++)
    {
        list[i] = temp[i];
        count++;
    }

    j++; 

    if (j < (max - 1))
       list[j] = NUL;
    
    return (count);
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Print_Help"            */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function prints the help menu.                             */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debugger function      */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Print_Line                      Print a line to the user    */
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*      MQ Qian         01-16-1995      Verified version 1.2            */
/*                                                                      */
/************************************************************************/
void  DBC_Print_Menu(void)
{
    int i, j;

    /* Define the Nucleus Debugger Help/Greeting menu.  */

    /* Print out the title.  */
    DBC_Print_Line(New_Line);
#ifdef NETWORK
    DBC_Print_Spaces(19);
    DBC_Print_Line("******* Nucleus DBUG+ Help Menu *******  \n\r");
    DBC_Print_Spaces(6);
#else
    for (i = 0; i < (COL/2 - 19); i++)
       DBT_Put_Char(' ');
    DBC_Print_Line("******* Nucleus DBUG+ Help Menu *******  \n\r");
    for (i = 0; i < (COL/2 - 6); i++)
       DBT_Put_Char(' ');
#endif
    DBC_Print_Line("Version 2.0 \n\n\r");

   /* Walk through the menu table until a NUL entry is found.  */
   i =  0;
   j =  6;
   while (Help_Menu[i])
   {
      /* Print line of help menu.  */
      DBC_Print_Line(Help_Menu[i++]);

      /* Determine if we need to pause.  */
      if (j >= (ROW-2))
      {
          /* Pause until user requests completion.  */
          DBC_Print_Line(New_Line);
          DBC_Pause();
          j = 2;
      }
      else
          /* Increment the lines within a page.  */
          j++;
   }
/*   DBC_Print_Line(New_Line);
     printf(" menu is ended."); */
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Print_Status"          */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function prints the Nucleus PLUS status codes.             */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_*                               Nucleus debugger functions  */
/*                                                                      */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBC_Print_Line                      Print a line to the user    */
/*                                                                      */
/* INPUTS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* OUTPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Print_Status(int  status)
{

    /* Print the first part of the status.  */
    DBC_Print_Line("\n\rReturn Status:  ");
    
    /* Determine what the status is and print a corresponding message. */
    switch(status)
    {

    case NU_SUCCESS:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_SUCCESS\n\n\r");
        break;

    case NU_END_OF_LOG:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_END_OF_LOG\n\n\r");
        break;

    case NU_GROUP_DELETED:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_GROUP_DELETED\n\n\r");
        break;

    case NU_INVALID_DELETE:
    
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_DELETE\n\n\r");
        break;

    case NU_INVALID_DRIVER:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_DRIVER\n\n\r");
        break;

    case NU_INVALID_ENABLE:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_ENABLE\n\n\r");
        break;

    case NU_INVALID_ENTRY:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_ENTRY\n\n\r");
        break;

    case NU_INVALID_FUNCTION:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_FUNCTION\n\n\r");
        break;

    case NU_INVALID_GROUP:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_GROUP\n\n\r");
        break;

    case NU_INVALID_HISR:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_HISR\n\n\r");
        break;
        
    case NU_INVALID_MAILBOX:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_MAILBOX\n\n\r");
        break;

    case NU_INVALID_MEMORY:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_MEMORY\n\n\r");
        break;

    case NU_INVALID_MESSAGE:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_MESSAGE\n\n\r");
        break;

    case NU_INVALID_OPERATION:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_OPERATION\n\n\r");
        break;

    case NU_INVALID_PIPE:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_PIPE\n\n\r");
        break;

    case NU_INVALID_POINTER:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_POINTER\n\n\r");
        break;

    case NU_INVALID_POOL:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_POOL\n\n\r");
        break;

    case NU_INVALID_PREEMPT:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_PREEMPT\n\n\r");
        break;

    case NU_INVALID_PRIORITY:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_PRIORITY\n\n\r");
        break;

    case NU_INVALID_QUEUE:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_QUEUE\n\n\r");
        break;

    case NU_INVALID_RESUME:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_RESUME\n\n\r");
        break;

    case NU_INVALID_SEMAPHORE:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_SEMAPHORE\n\n\r");
        break;

    case NU_INVALID_SIZE:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_SIZE\n\n\r");
        break;

    case NU_INVALID_START:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_START\n\n\r");
        break;

    case NU_INVALID_SUSPEND:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_SUSPEND\n\n\r");
        break;

    case NU_INVALID_TASK:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_TASK\n\n\r");
        break;

    case NU_INVALID_TIMER:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_TIMER\n\n\r");
        break;

    case NU_INVALID_VECTOR:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_INVALID_VECTOR\n\n\r");
        break;

    case NU_MAILBOX_DELETED:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_MAILBOX_DELETED\n\n\r");
        break;

    case NU_MAILBOX_EMPTY:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_MAILBOX_EMPTY\n\n\r");
        break;

    case NU_MAILBOX_FULL:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_MAILBOX_FULL\n\n\r");
        break;

    case NU_MAILBOX_RESET:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_MAILBOX_RESET\n\n\r");
        break;

    case NU_NO_MEMORY:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_NO_MEMORY\n\n\r");
        break;

    case NU_NO_MORE_LISRS:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_NO_MORE_LISRS\n\n\r");
        break;

    case NU_NO_PARTITION:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_NO_PARTITION\n\n\r");
        break;

    case NU_NOT_DISABLED:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_NOT_DISABLED\n\n\r");
        break;

    case NU_NOT_PRESENT:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_NOT_PRESENT\n\n\r");
        break;

    case NU_NOT_REGISTERED:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_NOT_REGISTERED\n\n\r");
        break;

    case NU_NOT_TERMINATED:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_NOT_TERMINATED\n\n\r");
        break;

    case NU_PIPE_DELETED:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_PIPE_DELETED\n\n\r");
        break;

    case NU_PIPE_EMPTY:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_PIPE_EMPTY\n\n\r");
        break;

    case NU_PIPE_FULL:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_PIPE_FULL\n\n\r");
        break;

    case NU_PIPE_RESET:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_PIPE_RESET\n\n\r");
        break;

    case NU_POOL_DELETED:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_POOL_DELETED\n\n\r");
        break;

    case NU_QUEUE_DELETED:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_QUEUE_DELETED\n\n\r");
        break;

    case NU_QUEUE_EMPTY:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_QUEUE_EMPTY\n\n\r");
        break;

    case NU_QUEUE_FULL:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_QUEUE_FULL\n\n\r");
        break;        

    case NU_QUEUE_RESET:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_QUEUE_RESET\n\n\r");
        break;        

    case NU_SEMAPHORE_DELETED:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_SEMAPHORE_DELETED\n\n\r");
        break;        

    case NU_SEMAPHORE_RESET:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_SEMAPHORE_RESET\n\n\r");
        break;        

    case NU_TIMEOUT:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_TIMEOUT\n\n\r");
        break;        

    case NU_UNAVAILABLE:
        
        /* Print appropriate error message.  */
        DBC_Print_Line("                  NU_UNAVAILABLE\n\n\r");
        break;        

    default:                                

        /* Print appropriate error message.  */
        DBC_Print_Line("                  UNKNOWN VALUE\n\n\r");
        break;
    }
}




/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Input_Line"            */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function will read in characters and return input          */
/*      in the supplied string area.                                    */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Higher level Debugger routines                                  */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBT_Get_Char                                                    */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      string                              Area to place input string  */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/* MQ Qian   10-22-96   Added ERR_RETURN for the return value for err   */
/*                      from NU_Recv in NU_Telnet_Get_Filtered_Char     */
/*                                                                      */
/************************************************************************/
void DBC_Input_Line(char *string)
{
    int  i=0;                                  /* Working pointers  */
    char alpha;

    do
    {
        alpha = DBT_Get_Char();          /* Read character              */
        if (alpha == BS)                 /* Check for back space        */
        {   /* Now check to make sure that there is something to BS over*/
            if (i > 0)
                i--;                     /* adjust pointer position     */
        }
        else if (alpha != CR)            /* Check for return            */
        {
            string[i] = alpha;           /* String[i] value set to alpha*/
            i++;
        }
    } while (alpha!='\n' && alpha!='\r' && alpha!=ERR_RETURN && i<COL);

    /* Because of "\r\n" or "\n\r", we need to grap another,
      however, it depends on what client sent */
    if (alpha!='\n' || alpha!='\r')
/*      NU_Recv(socket, (char *)&alpha, 1, 0); */
        alpha = DBT_Get_Char();          /* Read character              */

    string[i] = NUL;                     /* End of string set to NUL    */
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Get_Token"             */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function will return a pointer to the next token in the    */
/*      string removing leading spaces and comas.                       */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Higher level debugger routines                                  */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      string1                             Input string                */
/*      string2                             String to place token in    */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      Pointer to next location in source string                       */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*      MQ Qian         02-01-1995      Verified version 1.2            */
/*                                                                      */
/************************************************************************/
char *DBC_Get_Token(char *string1, char *string2)
{
    int i=0, j=0;                                    /* Working pointers         */

    /* Delete spaces and comas. while ((string1[i] == ' ') || (string1[i] == ',')) */
    while ((string1[i]==' ') || (string1[i]==',')
            || (string1[i]=='\n') || (string1[i]=='\r'))
        i++;                                /* Adjust pointer           */

    /* Continues until a NUL, coma, or a Space occurs */
    while (((string1[i] != NUL) && (string1[i] != ',')) && (string1[i] != ' '))
        string2[j++] = string1[i++];

    /* Delete spaces and comas. */
    while ((string1[i] == ' ') || (string1[i] == ','))
        i++;                                /* Adjust pointer           */

    /* Insure that the token string is NUL terminated.  */
    string2[j] = NUL;

    /* Return the last string position.  */
    return(&string1[i]);
}                                           /* End of DBC_Get_Token      */


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Get_Name"              */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function will return a pointer to the next token in the    */
/*      string removing leading spaces and comas.                       */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Higher level debugger routines                                  */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      string1                             Input string                */
/*      string2                             String to place name in     */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      Pointer to next location in source string                       */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
char *  DBC_Get_Name(char *string1,char *string2)
{

int i,j;                                    /* Working pointers         */

    i = 0;
    j = 0;                                  /* initialize pointers      */

    /* Delete spaces and comas. */
    while ((string1[i] == ' ') || (string1[i] == ','))
    {

        i++;                                /* Adjust pointer           */
    }

    /* Continues until a NUL, or a coma occurs */
    while (((string1[i] != NUL) && (string1[i] != ',')) && (j < MAX_NAME))
    {

        string2[j] = string1[i];
        j++;
        i++;
    }

    /* Delete spaces and comas. */
    while ((string1[i] == ' ') || (string1[i] == ','))
    {

        i++;                                /* Adjust pointer           */
    }

    /* Insure that the token string is NUL terminated.  */
    string2[j] = NUL;

    /* Return the last string position.  */
    return(&string1[i]);
}                                           /* End of DBC_Get_Name      */

/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Check_For_Excess"      */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This functions checks for excess garbage (anything other than   */
/*      spaces remaining on the line).                                  */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Higher level debugger routines                                  */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      string                              Remainder of string         */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      return(DB_TRUE)                     If some garbage exists      */
/*      return(DB_FALSE)                    If everything is okay       */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
int  DBC_Check_For_Excess(char *string)
{

int i;                                  /* Working pointer              */

    /* Check characters until a NULL or a non-SPACE is found.  */
    i = 0;
    while ((string[i] != NUL) && (string[i] == ' '))
    {

        /* Increment position in the string.  */
        i++;
    }

    /* Determine what to send back to the caller.  */
    if (string[i] == NUL)

        /* Reached the end of the string, must not have found anything. */
        return(DB_FALSE);
    else

        /* Found garbage, return appropriate value.  */
        return(DB_TRUE);
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Items_Per_Width"       */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function determines how many items will fit within the     */
/*      start and COL defined in DB_DEFS.H.                             */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Higher level debugger routines                                  */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      start                               Starting column             */
/*      item_width                          Width of each item          */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      return(items)                       Number of items that fit    */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
int  DBC_Items_Per_Width(int start, int item_width)
{

int  items = 1;                             /* Number of items that fit */

    /* Loop to determine how many of these buggers will fit.  */
    start = start + item_width;
    while ((start+item_width) < COL)
    {
     
        /* Increment the start and the number of items that will fit.  */
        items++;
        start =  start + item_width;
    }

    /* Return the amount of items that will fit.  */
    return(items);
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Items_Per_Height"      */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function determines how many items will fit within the     */
/*      ROW defined in DB_DEFS.H.                                       */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Higher level debugger routines                                  */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      start                               Starting column             */
/*      item_height                         Height of each item         */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      return(items)                       Number of items that fit    */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
int  DBC_Items_Per_Height(int item_height)
{

int  items = 1;                             /* Number of items that fit */
int  row = 1;                               /* Current working row      */

    /* Loop to determine how many of these buggers will fit.  */
    row = row + item_height;
    while ((row+item_height) < ROW)
    {
     
        /* Increment the start and the number of items that will fit.  */
        items++;
        row =  row + item_height;
    }

    /* Return the amount of items that will fit.  */
    return(items);
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Init_Buffer"           */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function initializes the output buffer for use in the      */
/*      various status reports.                                         */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Debug                           Main debug routine          */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Init_Buffer(void)
{

int     i;
int     offset;

    /* Initialize the output line buffer pointers.   */
    offset =  0;
    for (i = 0; i < ROW; i++)
    {
    
        /* Setup the line pointer structure to point at a line in the 
           output buffer.  */
        Line_Pointers[i] =  &Output_Buffer[offset];
        
        /* Update the offset to the next logical line in the output
           buffer.  */
        offset =  offset + COL + 1;
    }
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Pause"                 */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function waits for user key action before it returns.      */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Higher level debugger routines                                  */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  HISTORY                                                             */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBC_Pause(void)
{

    /* Print message to the user.  */
    DBC_Print_Line("---- Hit any key to continue ----\n\r");
    
    /* Get a character from the user.  */
    DBT_Get_Char();
    
    /* Print a new line after the key is hit.  */
    DBC_Print_Line(New_Line);
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Print_Line"            */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function will print a line, character by character.        */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Higher level debugger routines                                  */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBT_Put_Char                        Print a single character    */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      string                              String to print             */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*      MQ Qian         01-20-1995      Verified version 1.2            */
/*      MQ Qian         08-13-1996      Check connection.               */
/*                                                                      */
/************************************************************************/
void DBC_Print_Line(char *string)
{
    /* Print characters in the string until a NULL is found.  */
#ifdef NETWORK
    int send, sent=0;                                  /* Working pointer              */

    send = strlen(string);

    while (send)
    {
        sent = NU_Send(telnet_socket, string, (uint16)send, 0);
        if (sent==NU_NOT_CONNECTED)
            return;

        /* if the string was not sent through, sleep 1 tick and send again */
        if (send>sent)
            NU_Sleep(1);
        else
            break;
        send -= sent;
        string += sent;
    }
    /* this sleep is just for TD debug purpose  */
    NU_Sleep(1);
#else /* NETWORK */
    int i=0;

    while (string[i] != NUL)
    {
        /* Print one character of the string.  */
        DBT_Put_Char(string[i++]);
    }
#endif /* NETWORK */
} /* DBC_Print_Line() */


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBC_Print_Spaces"          */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function will print a line, character by character.        */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Higher level debugger routines                                  */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      DBT_Print_Line                        Print a string            */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      n      the number of spaces to be put                           */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      MQ Qian         01-25-1995      Created initial version 1.0     */
/*                                                                      */
/************************************************************************/
void DBC_Print_Spaces(int n)
{
    char i, spaces[MAXCOL];

    for (i=0; i<(COL/2-n); i++)
        spaces[i] = ' ';
    spaces[i] = 0;
       DBC_Print_Line(spaces);
} /* DBC_Print_Spaces() */



