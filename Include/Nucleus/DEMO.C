/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: erc.c
//
// Description:
// This file contains the Application_Initialize() routine, which is the
// main application level module.  The demo version starts several tasks
// to demonstrate the capabilities of various synchronization methods.
//
//
// Update Log:
// 10/06/98 JSN: Modifications previously made to R4640 source.
//               - Added serial i/o and the demo screen, providing
//                 a convenient place to hang code which can be
//                 executed at a particular keystroke.
//               - Removed original serial i/o and LED display code.
// 11/03/98 JSN: Removed a couple of unused variables from tasks.
// 08/19/99 JSN: Replaced print_string with printf, and hence removed
//               serial_init, convert_number, char_present, get_char,
//               print_char, clreol, and bspace.
// 10/11/99 JSN: Updated gSize buildsysisms and added a description.
/*************************************************************************/


/* Include necessary Nucleus PLUS files.  */

#include  "nucleus.h"
#include  "tc_defs.h"


/* Here are a couple of symbols which our CHAOS-tailored MSL needs:
**  gSize_total_memory = total available physical memory
**  gSize_small_heap = 3X this number for heap, debug heap, pci window
**  gSizeReserved = memory for old-fashioned Nucleus memory allocation    */
unsigned gSize_total_memory = 0x1000000;  /* eval board has 16MB          */
unsigned gSize_small_heap = 0x200000;     /* 2MB => 6MB actually used     */
unsigned gSizeReserved = 0;               /* not using nucleus allocation */


/* Define Application data structures.  */
NU_TASK         Task_0;
NU_TASK         Task_1;
NU_TASK         Task_2;
NU_TASK         Task_3;
NU_TASK         Task_4;
NU_TASK         Task_5;
NU_QUEUE        Queue_0;
NU_SEMAPHORE    Semaphore_0;
NU_EVENT_GROUP  Event_Group_0;
NU_MEMORY_POOL  System_Memory;


/* Allocate global counters. */
UNSIGNED  Task_Time;
UNSIGNED  Task_2_messages_received;
UNSIGNED  Task_2_invalid_messages;
UNSIGNED  Task_1_messages_sent;
NU_TASK  *Who_has_the_resource;
UNSIGNED  Event_Detections;


/* Added so the simple serial i/o knows when to update the demo screen.  */
unsigned  delta0;
unsigned  delta1;
unsigned  delta2;
unsigned  delta34;
unsigned  delta5;


/* Define prototypes for function references.  */
void    task_0(UNSIGNED argc, VOID *argv);
void    task_1(UNSIGNED argc, VOID *argv);
void    task_2(UNSIGNED argc, VOID *argv);
void    task_3_and_4(UNSIGNED argc, VOID *argv);
void    task_5(UNSIGNED argc, VOID *argv);


/* Define the Application_Initialize routine that determines the initial
   Nucleus PLUS application environment.  */
   
void    Application_Initialize(void *first_available_memory)
{
    VOID    *pointer;

    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */
    NU_Create_Memory_Pool(&System_Memory, "SYSMEM", 
                        first_available_memory, 40000, 56, NU_FIFO);
                        
    /* Create each task in the system.  */
    
    /* Create task 0.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    NU_Create_Task(&Task_0, "TASK 0", task_0, 0, NU_NULL, pointer,
                   2000, 1, 20, NU_PREEMPT, NU_START);

    /* Create task 1.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    NU_Create_Task(&Task_1, "TASK 1", task_1, 0, NU_NULL, pointer, 
                   2000, 10, 5, NU_PREEMPT, NU_START);

    /* Create task 2.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    NU_Create_Task(&Task_2, "TASK 2", task_2, 0, NU_NULL, pointer,
                   2000, 10, 5, NU_PREEMPT, NU_START);

    /* Create task 3.  Note that task 4 uses the same instruction area.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    NU_Create_Task(&Task_3, "TASK 3", task_3_and_4, 0, NU_NULL, pointer, 
                   2000, 10, 0, NU_PREEMPT, NU_START);

    /* Create task 4.  Note that task 3 uses the same instruction area.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    NU_Create_Task(&Task_4, "TASK 4", task_3_and_4, 0, NU_NULL, pointer, 
                   2000, 10, 0, NU_PREEMPT, NU_START);

    /* Create task 5.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    NU_Create_Task(&Task_5, "TASK 5", task_5, 0, NU_NULL, pointer, 
                   2000, 7, 0, NU_PREEMPT, NU_START);

    /* Create communication queue.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 100*sizeof(UNSIGNED), 
                       NU_NO_SUSPEND);
    NU_Create_Queue(&Queue_0, "QUEUE 0", pointer, 100, NU_FIXED_SIZE, 
                    1, NU_FIFO);

    /* Create synchronization semaphore.  */
    NU_Create_Semaphore(&Semaphore_0, "SEM 0", 1, NU_FIFO);
    
    /* Create event flag group.  */
    NU_Create_Event_Group(&Event_Group_0, "EVGROUP0");
}


/*
01    *********************************************************************
02            ConvergeNet Technologies - Nucleus PLUS Demonstration
03    *********************************************************************
04   
05               Task0 <EVENT>     Events Set:        NNNNNNNNNN
06               Task1 <QUEUE>     Messages Sent:     NNNNNNNNNN
07               Task2 <QUEUE>     Messages Received: NNNNNNNNNN
08               Task3 <SEMAPHORE> Task Status:           Active
09               Task4 <SEMAPHORE> Task Status:        Suspended
10               Task5 <EVENT>     Events Retrieved:  NNNNNNNNNN
11   
12               Keystrokes Will Be Echoed Indefinitely:       _
13
14
15
16
17
18
19
20
21
22
23
24
*/


/* Define a task that sleeps for a period of time and then sets 
   an event flag that wakes up task 5.  */  

void   task_0(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
    unsigned  i;
    unsigned  key = 0;
    unsigned  screen_refresh = 1;

    /* useful ascii escape sequences */
    char *clrscn = "\033[H\033[2J\033[0m";  /* clear the screen */
    char *cpecho = "\033[12;64H";           /* cursor position for keystroke echo */

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    /* Set the clock to 0.  This clock ticks every 10 system timer ticks. */
    Task_Time =  0;

    /* Task0 forever loop */
    while(1)
    {
        /* poll 4 times for recent task activity (1 second total) */
        for (i=0; i<4; i++)
        {
            NU_Sleep(23);  /* 100 ticks (4x25) at 10ms with no code would be 1 second */

            if (screen_refresh)
            {
                printf(clrscn);
                printf("\033[01;07H*********************************************************************");
                printf("\033[02;15HConvergeNet Technologies - Nucleus PLUS Demonstration");
                printf("\033[03;07H*********************************************************************");
                printf("\033[05;18HTask0 <EVENT>     Events Set:");
                printf("\033[06;18HTask1 <QUEUE>     Messages Sent:");
                printf("\033[07;18HTask2 <QUEUE>     Messages Received:");
                printf("\033[08;18HTask3 <SEMAPHORE> Task Status:");
                printf("\033[09;18HTask4 <SEMAPHORE> Task Status:");
                printf("\033[10;18HTask5 <EVENT>     Events Retrieved:");
                printf("\033[12;18HKeystrokes Will Be Echoed Indefinitely:");
                printf(cpecho);
                screen_refresh = 0;
            }

            if (delta0)
            {
                printf("\033[05;55H%10d",Task_Time);
                printf(cpecho);
                delta0 = 0;
            }

            if (delta1)
            {
                printf("\033[06;55H%10d",Task_1_messages_sent);
                printf(cpecho);
                delta1 = 0;
            }

            if (delta2)
            {
                printf("\033[07;55H%10d",Task_2_messages_received);
                if (Task_2_invalid_messages)
                    printf(" %10d",Task_2_invalid_messages);
                printf(cpecho);
                delta2 = 0;
            }

            if (delta34)
            {
                if (Who_has_the_resource == &Task_3)
                    printf("\033[08;55H    Active\033[09;55H Suspended");
                else if (Who_has_the_resource == &Task_4)
                    printf("\033[08;55H Suspended\033[09;55H    Active");
                else
                    printf("\033[08;55H          \033[09;55H          ");
                printf(cpecho);
                delta34 = 0;
            }

            if (delta5)
            {
                printf("\033[10;55H%10d",Event_Detections);
                printf(cpecho);
                delta5 = 0;
            }

            if (kbhit())
            {
                switch (key = getch())
                {
                    case ' ':  /* SPACEBAR - redraw the screen */
                        screen_refresh = 1;
                        break;

                    case 0x08:  /* BACKSPACE */
                    case 0x09:  /* TAB */
                    case 0x0D:  /* ENTER */
                    case 0x1B:  /* ESC */
                        putch(' ');
                        putch(7);
                        break;

                    default:
                        putch(key);
                        break;

                }  /* switch (key = Get_Char()) */

                printf(cpecho);

            }  /* if (Char_Present()) */

        }  /* for (i=0; i<4; i++) */

        /* Increment the time.  */
        Task_Time++;
        delta0 = 1;

        /* Set an event flag to lift the suspension on task 5.  */
        status =  NU_Set_Events(&Event_Group_0, 1, NU_OR);

    }  /* while(1) */
}


/* Define the queue sending task.  Note that the only things that cause
   this task to suspend are queue full conditions and the time slice
   specified in the configuration file.  */

void   task_1(UNSIGNED argc, VOID *argv)
{
    STATUS    status; 
    UNSIGNED  Send_Message;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    /* Initialize the message counter.  */
    Task_1_messages_sent =  0;

    /* Initialize the message contents.  The receiver will examine the 
       message contents for errors.  */
    Send_Message = 0;

    while(1)
    {
         /* Send the message to Queue_0, which task 2 reads from.  Note
            that if the destination queue fills up this task suspends until
            room becomes available.  */
         status =  NU_Send_To_Queue(&Queue_0, &Send_Message, 1, NU_SUSPEND);

         /* Determine if the message was sent successfully.  */
         if (status == NU_SUCCESS)
             Task_1_messages_sent++;

         /* Modify the contents of the next message to send.  */
         Send_Message++;
         delta1 = 1;
    }
}


/* Define the queue receiving task.  Note that the only things that cause
   this task to suspend are queue empty conditions and the time slice
   specified in the configuration file.   */

void   task_2(UNSIGNED argc, VOID *argv)
{
    STATUS    status; 
    UNSIGNED  Receive_Message;
    UNSIGNED  received_size;
    UNSIGNED  message_expected;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    /* Initialize the message counter.  */
    Task_2_messages_received =  0;

    /* Initialize the message error counter.  */
    Task_2_invalid_messages =  0;

    /* Initialize the message contents to expect.  */
    message_expected =  0;
    
    while(1)
    {
         /* Retrieve a message from Queue_0, which task 1 writes to.  Note
            that if the source queue is empty this task suspends until
            something becomes available.  */
         status =  NU_Receive_From_Queue(&Queue_0, &Receive_Message, 1, 
                                &received_size, NU_SUSPEND);
         
         /* Determine if the message was received successfully.  */
         if (status == NU_SUCCESS)
             Task_2_messages_received++;
             
         /* Check the contents of the message against what this task
            is expecting.  */
         if ((received_size != 1) ||
             (Receive_Message != message_expected))
             Task_2_invalid_messages++;
         
         /* Modify the expected contents of the next message.  */
         message_expected++;
         delta2 = 1;
    }
}


/* Tasks 3 and 4 want a single resource.  Once one of the tasks gets the
   resource, it keeps it for 30 clock ticks before releasing it.  During
   this time the other task suspends waiting for the resource.  Note that
   both task 3 and 4 use the same instruction areas but have different 
   stacks.  */
   
void  task_3_and_4(UNSIGNED argc, VOID *argv)
{
    STATUS    status;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    /* Loop to allocate and deallocate the resource.  */
    while(1)
    {
    
         /* Allocate the resource.  Suspend until it becomes available.  */
         status =  NU_Obtain_Semaphore(&Semaphore_0, NU_SUSPEND);
         
         /* If the status is successful, show that this task owns the 
            resource.  */
         if (status ==  NU_SUCCESS)
         {
         
             Who_has_the_resource =  NU_Current_Task_Pointer();
             delta34 = 1;
             
             /* Sleep for 30 ticks to cause the other task to suspend on 
                the resource.  */
             NU_Sleep(30);
             
             /* Release the semaphore.  */
             NU_Release_Semaphore(&Semaphore_0);
        }
    }

}


/* Define the task that waits for the event to be set by task 0.  */

void  task_5(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
    UNSIGNED  event_group;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    /* Initialize the event detection counter.  */
    Event_Detections =  0;
    
    /* Continue this process forever.  */
    while(1)
    {
        /* Wait for an event and consume it.  */
        status =  NU_Retrieve_Events(&Event_Group_0, 1, NU_OR_CONSUME,
                                     &event_group, NU_SUSPEND);
                          
        /* If the status is okay, increment the counter.  */
        if (status == NU_SUCCESS)
           Event_Detections++;

        delta5 = 1;
    }
}
