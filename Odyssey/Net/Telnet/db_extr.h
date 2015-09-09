/*************************************************************************/
/*                                                                       */
/*            Copyright (c) 1998 Accelerated Technology, Inc.            */
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
/*      db_extr.c                                       DBUG+  1.1       */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DEBUG       Nucleus PLUS Debugger                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*    This file contains Nucleus debugger external function refs.        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
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

extern  char  DBT_Get_Char(void);
extern  void  DBT_Put_Char(char alpha);
extern  int   DBT_ASCII_To_Integer(char *string, int *num_ptr);
extern  int   DBT_HEX_ASCII_To_Long(char *string, unsigned long *num_ptr);
extern  int   DBT_Name_Compare(char *string1, char *string2);
extern  int   DBT_String_Compare(char *string1, char *string2);
extern  void  DBT_String_Cat(char *dest, char *source);
extern  void  DBC_Input_Line(char *string);
extern  char *DBC_Get_Token(char *string1, char *string2);
extern  char *DBC_Get_Name(char *string1, char *string2);
extern  int   DBC_Build_List(char **list,int offset, int max, char *name);
extern  void  DBC_Print_Line(char *string);
extern  int   DBC_Check_For_Excess(char *string);
extern  int   DBC_Items_Per_Width(int start, int item_size);
extern  int   DBC_Items_Per_Height(int item_size);
extern  void  DBC_Pause(void);
extern  void  DBC_Debug(UNSIGNED argc, VOID *argv);
extern  void  DBC_Print_Menu(void);
extern  void  DBC_Print_Status(int status);
extern  void  DBC_Print_Spaces(int);
extern  void  DBC_Init_Buffer(void);
extern  void  DBC_Task_Fields(void);
extern  void  DBC_Task_Information(int counter);
extern  void  DBC_Name_Prompt(void);
extern  void  DBC_Display_Status(char *string, UNSIGNED total,
                                int items_per_width,
                                int items_per_height,
                                VOID (*DB_Item_Fields)(void),
                                VOID (*DB_Item_Information)(int loop),
                                int total_fields, int type);
extern  void  DBC_Mailbox_Fields(void);
extern  void  DBC_Mailbox_Information(int counter);
extern  void  DBC_Queue_Fields(void);
extern  void  DBC_Queue_Information(int counter);
extern  void  DBC_Pipe_Fields(void);
extern  void  DBC_Pipe_Information(int counter);
extern  void  DBC_Semaphore_Fields(void);
extern  void  DBC_Semaphore_Information(int counter);
extern  void  DBC_Event_Fields(void);
extern  void  DBC_Event_Information(int counter);
extern  void  DBC_Signal_Fields(void);
extern  void  DBC_Signal_Information(int counter);
extern  void  DBC_Timer_Fields(void);
extern  void  DBC_Timer_Information(int counter);
extern  void  DBC_Partition_Fields(void);
extern  void  DBC_Partition_Information(int counter);
extern  void  DBC_Dynamic_Memory_Fields(void);
extern  void  DBC_Dynamic_Memory_Information(int counter);
extern  void  DBC_Display_Memory(char *string);
extern  void  DBC_Set_Memory(char *string);
extern  void  DBC_Set_List(int type);
extern  void  DBC_Resume_Task(void);
extern  void  DBC_Suspend_Task(void);
extern  void  DBC_Terminate_Task(void);
extern  void  DBC_Reset_Task(void);
extern  void  DBC_Change_Priority(void);
extern  void  DBC_Broadcast_To_Mailbox(void);
extern  void  DBC_Receive_From_Mailbox(void);
extern  void  DBC_Reset_Mailbox(void);
extern  void  DBC_Send_To_Mailbox(void);
extern  void  DBC_Broadcast_To_Queue(void);
extern  void  DBC_Receive_From_Queue(void);
extern  void  DBC_Reset_Queue(void);
extern  void  DBC_Send_To_Front_Of_Queue(void);
extern  void  DBC_Send_To_Queue(void);
extern  void  DBC_Broadcast_To_Pipe(void);
extern  void  DBC_Receive_From_Pipe(void);
extern  void  DBC_Reset_Pipe(void);
extern  void  DBC_Send_To_Front_Of_Pipe(void);
extern  void  DBC_Send_To_Pipe(void);
extern  void  DBC_Obtain_Semaphore(void);
extern  void  DBC_Release_Semaphore(void);
extern  void  DBC_Reset_Semaphore(void);
extern  void  DBC_Retrieve_Events(void);
extern  void  DBC_Set_Events(void);
extern  void  DBC_Send_Signals(void);
extern  void  DBC_Reset_Timer(void);
extern  void  DBC_Control_Timer(void);
extern  void  DBC_Retrieve_Clock(void);
extern  void  DBC_Set_Clock(void);
extern  void  DBC_Allocate_Partition(void);
extern  void  DBC_Deallocate_Partition(void);
extern  void  DBC_Allocate_Memory(void);
extern  void  DBC_Deallocate_Memory(void);


